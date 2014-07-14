//
// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// If you use the software (in whole or in part), you shall adhere to all
// applicable U.S., European, and other export laws, including but not limited
// to the U.S. Export Administration Regulations (EAR), (15 C.F.R. Sections
// 730 through 774), and E.U. Council Regulation (EC) No 1334/2000 of 22 June
// 2000.  Further, pursuant to Section 740.6 of the EAR, you hereby certify
// that, except pursuant to a license granted by the United States Department
// of Commerce Bureau of Industry and Security or as otherwise permitted
// pursuant to a License Exception under the U.S. Export Administration
// Regulations ("EAR"), you will not (1) export, re-export or release to a
// national of a country in Country Groups D:1, E:1 or E:2 any restricted
// technology, software, or source code you receive hereunder, or (2) export to
// Country Groups D:1, E:1 or E:2 the direct product of such technology or
// software, if such foreign produced direct product is subject to national
// security controls as identified on the Commerce Control List (currently
// found in Supplement 1 to Part 774 of EAR).  For the most current Country
// Group listings, or for additional information about the EAR or your
// obligations under those regulations, please refer to the U.S. Bureau of
// Industry and Securitys website at http://www.bis.doc.gov/.
//
//==-----------------------------------------------------------------------===//
// @file AMDILEGIOExpansion.cpp
// @details Implementation of IO expansion class for evergreen and NI devices.
//
#include "AMDILIOExpansion.h"
#include "AMDILCompilerErrors.h"
#include "AMDILCompilerWarnings.h"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILTargetMachine.h"
#include "AMDILUtilityFunctions.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Value.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/Support/DebugLoc.h"
#include <cstdio>
using namespace llvm;

namespace llvm
{
  extern void initializeAMDILEGIOExpansionPass(llvm::PassRegistry&);
}

char AMDILEGIOExpansion::ID = 0;
INITIALIZE_PASS(AMDILEGIOExpansion, "eg-io-expansion",
                "AMDIL EG/NI IO Expansion", false, false)

AMDILEGIOExpansion::AMDILEGIOExpansion()
  : MachineFunctionPass(ID) {
  initializeAMDILEGIOExpansionPass(*PassRegistry::getPassRegistry());
}

const char *AMDILEGIOExpansion::getPassName() const {
  return "AMDIL EG/NI IO Expansion Pass";
}

bool AMDILEGIOExpansion::runOnMachineFunction(MachineFunction& MF) {
  AMDILEGIOExpansionImpl Impl(MF);
  return Impl.run();
}

bool AMDILEGIOExpansionImpl::isIOInstruction(MachineInstr *MI) {
  if (!MI) {
    return false;
  }

  if (isImageInst(MI)) {
    return true;
  }

  return AMDILIOExpansionImpl::isIOInstruction(MI);
}

void AMDILEGIOExpansionImpl::expandIOInstruction(MachineInstr *MI) {
  assert(isIOInstruction(MI) && "Must be an IO instruction to "
      "be passed to this function!");
  if (isReadImageInst(MI) || isImageTXLDInst(MI)) {
    expandImageLoad(mBB, MI);
  } else if (isWriteImageInst(MI)) {
    expandImageStore(mBB, MI);
  } else if (isImageInfoInst(MI)) {
    expandImageParam(mBB, MI);
  } else {
    AMDILIOExpansionImpl::expandIOInstruction(MI);
  }
}

bool AMDILEGIOExpansionImpl::isCacheableOp(MachineInstr *MI) {
  AMDILAS::InstrResEnc CurRes;
  getAsmPrinterFlags(MI, CurRes);
  // We only support caching on UAV11 - JeffG
  if (CurRes.bits.ResourceID == 11) {
    return CurRes.bits.CacheableRead;
  } else {
    return false;
  }
}

bool AMDILEGIOExpansionImpl::isArenaOp(MachineInstr *MI) {
  AMDILAS::InstrResEnc CurRes;
  getAsmPrinterFlags(MI, CurRes);
  return CurRes.bits.ResourceID == mSTM->getResourceID(AMDIL::ARENA_UAV_ID) ||
    CurRes.bits.ResourceID >= ARENA_SEGMENT_RESERVED_UAVS;
}

void AMDILEGIOExpansionImpl::expandLongExtend(MachineInstr *MI,
                                              uint32_t NumComps,
                                              uint32_t Size,
                                              bool SignedShift,
                                              uint32_t SrcReg,
                                              uint32_t &DstReg) {
  DebugLoc DL = MI->getDebugLoc();
  switch (Size) {
    default:
      llvm_unreachable("Found a case we don't handle!");
      break;
    case 8:
      if (NumComps == 1) {
        expandLongExtendSub32(MI, AMDIL::SHLi8i32rr, AMDIL::SHRv2i32i32rr,
            AMDIL::USHRi8i32rr,
            24, (24ULL | (31ULL << 32)), 24, AMDIL::LCREATEi64rr, SignedShift,
            false, SrcReg, DstReg);
      } else if (NumComps == 2) {
        expandLongExtendSub32(MI, AMDIL::SHLv2i8i32rr, AMDIL::SHRv4i32i32rr,
            AMDIL::USHRv2i8i32rr,
            24, (24ULL | (31ULL << 32)), 24, AMDIL::LCREATEv2i64rr, SignedShift,
            true, SrcReg, DstReg);
      } else {
        llvm_unreachable( "Found a case we don't handle!");
      }
      break;
    case 16:
      if (NumComps == 1) {
        expandLongExtendSub32(MI, AMDIL::SHLi16i32rr, AMDIL::SHRv2i32i32rr,
            AMDIL::USHRi16i32rr,
            16, (16ULL | (31ULL << 32)), 16, AMDIL::LCREATEi64rr, SignedShift,
            false, SrcReg, DstReg);
      } else if (NumComps == 2) {
        expandLongExtendSub32(MI, AMDIL::SHLv2i16i32rr, AMDIL::SHRv4i32i32rr,
            AMDIL::USHRv2i16i32rr,
            16, (16ULL | (31ULL << 32)), 16, AMDIL::LCREATEv2i64rr, SignedShift,
            true, SrcReg, DstReg);
      } else {
        llvm_unreachable( "Found a case we don't handle!");
      }
      break;
    case 32:
      if (NumComps == 1) {
        assert(is32BitRegister(SrcReg) && "bad SrcReg");
        if (SignedShift) {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHRi32i32rr), DstReg)
            .addReg(SrcReg)
            .addImm(mMFI->addi64Literal((0ULL | (31ULL << 32))));
        } else {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LCREATEi64rr), DstReg)
            .addReg(SrcReg)
            .addImm(mMFI->addi32Literal(0));
        }
      } else if (NumComps == 2) {
        assert(is64BitRegister(SrcReg) && "bad SrcReg");
        if (SignedShift) {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHRv2i32i32rr), AMDIL::Rxy1012)
            .addReg(SrcReg)
            .addImm(mMFI->addi64Literal(31));
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LCREATEv2i64rr), DstReg)
            .addReg(SrcReg)
            .addReg(AMDIL::Rxy1012);
        } else {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LCREATEv2i64rr), DstReg)
            .addReg(SrcReg)
            .addImm(mMFI->addi32Literal(0));
        }
      } else {
        llvm_unreachable( "Found a case we don't handle!");
      }
  }
}

void AMDILEGIOExpansionImpl::expandLongExtendSub32(MachineInstr *MI,
                                                   unsigned SHLop,
                                                   unsigned SHRop,
                                                   unsigned USHRop,
                                                   unsigned SHLimm,
                                                   uint64_t SHRimm,
                                                   unsigned USHRimm,
                                                   unsigned LCRop,
                                                   bool SignedShift,
                                                   bool Vec2,
                                                   uint32_t SrcReg,
                                                   uint32_t &DstReg) {
  DebugLoc DL = MI->getDebugLoc();
  unsigned OrigReg = DstReg;
  DstReg = Vec2 ? AMDIL::Rxy1011 : AMDIL::Rx1011;
  BuildMI(*mBB, MI, DL, mTII->get(SHLop), DstReg)
    .addReg(SrcReg)
    .addImm(mMFI->addi32Literal(SHLimm));
  if (SignedShift) {
    BuildMI(*mBB, MI, DL, mTII->get(LCRop),
        Vec2 ? AMDIL::R1011 : AMDIL::Rxy1011)
      .addReg(DstReg).addReg(DstReg);
    DstReg = Vec2 ? AMDIL::R1011 : AMDIL::Rxy1011;
    BuildMI(*mBB, MI, DL, mTII->get(SHRop),
        OrigReg).addReg(DstReg)
      .addImm(mMFI->addi64Literal(SHRimm));
  } else {
    BuildMI(*mBB, MI, DL, mTII->get(USHRop),
        DstReg).addReg(DstReg)
      .addImm(mMFI->addi32Literal(USHRimm));
    BuildMI(*mBB, MI, MI->getDebugLoc(), mTII->get(LCRop),
        OrigReg)
      .addReg(DstReg)
      .addImm(mMFI->addi32Literal(0));
  }
}

void AMDILEGIOExpansionImpl::expandIntegerExtend(MachineInstr *MI,
                                                 unsigned SHLop,
                                                 unsigned SHRop,
                                                 unsigned Offset,
                                                 unsigned SrcReg,
                                                 unsigned DstReg) {
  DebugLoc DL = MI->getDebugLoc();
  Offset = mMFI->addi32Literal(Offset);
  BuildMI(*mBB, MI, DL, mTII->get(SHLop), DstReg)
    .addReg(SrcReg).addImm(Offset);
  BuildMI(*mBB, MI, DL, mTII->get(SHRop), DstReg)
    .addReg(DstReg).addImm(Offset);
}

void AMDILEGIOExpansionImpl::expandExtendLoad(MachineInstr *MI,
                                              uint32_t SrcReg,
                                              uint32_t &DstReg) {
  if (!isExtendLoad(MI)) {
    return;
  }
  Type *AType = NULL;
  if (!MI->memoperands_empty()) {
    MachineMemOperand *MemOp = *MI->memoperands_begin();
    const Value *MOVal = MemOp ? MemOp->getValue() : NULL;
    AType = MOVal ? MOVal->getType() : NULL;
  }

  bool IsFloat = false;

  unsigned OpCode = 0;
  DebugLoc DL = MI->getDebugLoc();
  if (isExtLoadInst(MI)) {
    switch(MI->getDesc().OpInfo[0].RegClass) {
    default:
      llvm_unreachable( "Found an extending load that we don't handle!");
      break;
    case AMDIL::GPRI16RegClassID:
      if (!isHardwareLocal(MI)
          || mSTM->usesSoftware(AMDIL::Caps::ByteLDSOps)) {
        OpCode = isSExtLoadInst(MI) ? AMDIL::SHRi16i32rr : AMDIL::USHRi16i32rr;
        expandIntegerExtend(MI, AMDIL::SHLi16i32rr, OpCode, 24, SrcReg, DstReg);
      }
      break;
    case AMDIL::GPRV2I16RegClassID:
      OpCode = isSExtLoadInst(MI) ? AMDIL::SHRv2i16i32rr : AMDIL::USHRv2i16i32rr;
      expandIntegerExtend(MI, AMDIL::SHLv2i16i32rr, OpCode, 24, SrcReg, DstReg);
      break;
    case AMDIL::GPRV4I8RegClassID:
      OpCode = isSExtLoadInst(MI) ? AMDIL::SHRv4i8i32rr : AMDIL::USHRv4i8i32rr;
      expandIntegerExtend(MI, AMDIL::SHLv4i8i32rr, OpCode, 24, SrcReg, DstReg);
      break;
    case AMDIL::GPRV4I16RegClassID:
      OpCode = isSExtLoadInst(MI) ? AMDIL::SHRv4i16i32rr : AMDIL::USHRv4i16i32rr;
      expandIntegerExtend(MI, AMDIL::SHLv4i16i32rr, OpCode, 24, SrcReg, DstReg);
      break;

    case AMDIL::GPR_32RegClassID:
      if (IsFloat) {
        BuildMI(*mBB, MI, DL,
                mTII->get(AMDIL::HTOFf32r), DstReg)
          .addReg(SrcReg);
        break;
      }

      // We can be a i8 or i16 bit sign extended value
      if (getMemorySize(MI) == 1) {
        OpCode = isSExtLoadInst(MI) ? AMDIL::SHRi32i32rr : AMDIL::USHRi32i32rr;
        expandIntegerExtend(MI, AMDIL::SHLi32i32rr, OpCode, 24, SrcReg, DstReg);
      } else if (getMemorySize(MI) == 2) {
        OpCode = isSExtLoadInst(MI) ? AMDIL::SHRi32i32rr : AMDIL::USHRi32i32rr;
        expandIntegerExtend(MI, AMDIL::SHLi32i32rr, OpCode, 16, SrcReg, DstReg);
      } else {
        llvm_unreachable( "Found an extending load that we don't handle!");
      }
      break;
    case AMDIL::GPRV2I32RegClassID:
      if (IsFloat) {
        BuildMI(*mBB, MI, DL,
                mTII->get(AMDIL::HTOFv2f32r), DstReg)
          .addReg(SrcReg);
        break;
      }

      // We can be a v2i8 or v2i16 bit sign extended value
      if (getMemorySize(MI) == 2) {
        OpCode = isSExtLoadInst(MI) ?
                 AMDIL::SHRv2i32i32rr : AMDIL::USHRv2i32i32rr;
        expandIntegerExtend(MI, AMDIL::SHLv2i32i32rr, OpCode, 24, SrcReg,
                            DstReg);
      } else if (getMemorySize(MI) == 4) {
        OpCode = isSExtLoadInst(MI) ?
                 AMDIL::SHRv2i32i32rr : AMDIL::USHRv2i32i32rr;
        expandIntegerExtend(MI, AMDIL::SHLv2i32i32rr, OpCode, 16, SrcReg,
                            DstReg);
      } else {
        llvm_unreachable( "Found an extending load that we don't handle!");
      }
      break;
    case AMDIL::GPRV4I32RegClassID:
      if (IsFloat) {
        BuildMI(*mBB, MI, DL,
                mTII->get(AMDIL::HTOFv4f32r), DstReg)
          .addReg(SrcReg);
        break;
      }

      // We can be a v4i8 or v4i16 bit sign extended value
      if (getMemorySize(MI) == 4) {
        OpCode = isSExtLoadInst(MI) ?
                 AMDIL::SHRv4i32i32rr : AMDIL::USHRv4i32i32rr;
        expandIntegerExtend(MI, AMDIL::SHLv4i32i32rr, OpCode, 24, SrcReg,
                            DstReg);
      } else if (getMemorySize(MI) == 8) {
        OpCode = isSExtLoadInst(MI) ?
                 AMDIL::SHRv4i32i32rr : AMDIL::USHRv4i32i32rr;
        expandIntegerExtend(MI, AMDIL::SHLv4i32i32rr, OpCode, 16, SrcReg,
                            DstReg);
      } else {
        llvm_unreachable( "Found an extending load that we don't handle!");
      }
      break;
    case AMDIL::GPR_64RegClassID:
      if (IsFloat) {
        BuildMI(*mBB, MI, DL,
                mTII->get(AMDIL::FTODr), DstReg)
          .addReg(SrcReg);
        break;
      }

      // We can be a i8, i16 or i32 bit sign extended value
      if (getMemorySize(MI) == 1) {
        expandLongExtend(MI, 1, 8, isSExtLoadInst(MI), SrcReg, DstReg);
      } else if (getMemorySize(MI) == 2) {
        expandLongExtend(MI, 1, 16, isSExtLoadInst(MI), SrcReg, DstReg);
      } else if (getMemorySize(MI) == 4) {
        expandLongExtend(MI, 1, 32, isSExtLoadInst(MI), SrcReg, DstReg);
      } else {
        llvm_unreachable( "Found an extending load that we don't handle!");
      }
      break;
    case AMDIL::GPRV2I64RegClassID:
      if (IsFloat) {
        assert((isXYComponentReg(SrcReg) || isZWComponentReg(SrcReg)) &&
               "bad SrcReg");
        assert(is128BitRegister(DstReg) && "bad DstReg");
        if (isXYComponentReg(SrcReg)) {
          BuildMI(*mBB, MI, DL,
                  mTII->get(AMDIL::FTODr), getCompReg(DstReg, AMDIL::sub_zw))
            .addReg(getCompReg(SrcReg, AMDIL::sub_y));
          BuildMI(*mBB, MI, DL,
                  mTII->get(AMDIL::FTODr), getCompReg(DstReg, AMDIL::sub_xy))
            .addReg(getCompReg(SrcReg, AMDIL::sub_x));
        } else {
          BuildMI(*mBB, MI, DL,
                  mTII->get(AMDIL::FTODr), getCompReg(DstReg, AMDIL::sub_xy))
            .addReg(getCompReg(SrcReg, AMDIL::sub_z));
          BuildMI(*mBB, MI, DL,
                  mTII->get(AMDIL::FTODr), getCompReg(DstReg, AMDIL::sub_zw))
            .addReg(getCompReg(SrcReg, AMDIL::sub_w));
        }
        break;
      }

      // We can be a v2i8, v2i16 or v2i32 bit sign extended value
      if (getMemorySize(MI) == 2) {
        expandLongExtend(MI, 2, 8, isSExtLoadInst(MI), SrcReg, DstReg);
      } else if (getMemorySize(MI) == 4) {
        expandLongExtend(MI, 2, 16, isSExtLoadInst(MI), SrcReg, DstReg);
      } else if (getMemorySize(MI) == 8) {
        expandLongExtend(MI, 2, 32, isSExtLoadInst(MI), SrcReg, DstReg);
      } else {
        llvm_unreachable( "Found an extending load that we don't handle!");
      }
      break;
    }
  }
}

void AMDILEGIOExpansionImpl::expandPackedData(MachineInstr *MI,
                                              uint32_t SrcReg,
                                              uint32_t &DstReg) {
  if (!isPackedInst(MI)) {
    return;
  }

  DebugLoc DL = MI->getDebugLoc();
  // If we have packed data, then the shift size is no longer
  // the same as the load size and we need to adjust accordingly
  switch (getPackedID(MI)) {
    default:
      break;
    case PACK_V2I8:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UBIT_INSERTi32rrrr), AMDIL::Rx1011)
        .addImm(mMFI->addi32Literal(8)).addImm(mMFI->addi32Literal(8))
        .addReg(getCompReg(SrcReg, AMDIL::sub_y, AMDIL::sub_w))
        .addReg(getCompReg(SrcReg, AMDIL::sub_x, AMDIL::sub_z));
      DstReg = AMDIL::Rx1011;
      break;
    case PACK_V4I8:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LHIv2i64r), AMDIL::Rxy1012)
        .addReg(SrcReg);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LLOv2i64r), AMDIL::Rxy1011)
        .addReg(SrcReg);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UBIT_INSERTv2i32rrrr),
          AMDIL::Rxy1011)
        .addImm(mMFI->addi64Literal(8ULL | (8ULL << 32)))
        .addImm(mMFI->addi64Literal(8ULL | (8ULL << 32)))
        .addReg(AMDIL::Rxy1012).addReg(AMDIL::Rxy1011);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UBIT_INSERTi32rrrr), AMDIL::Rx1011)
        .addImm(mMFI->addi32Literal(16)).addImm(mMFI->addi32Literal(16))
        .addReg(AMDIL::Ry1011).addReg(AMDIL::Rx1011);
      DstReg = AMDIL::Rx1011;
      break;
    case PACK_V2I16:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UBIT_INSERTi32rrrr), AMDIL::Rx1011)
        .addImm(mMFI->addi32Literal(16)).addImm(mMFI->addi32Literal(16))
        .addReg(getCompReg(SrcReg, AMDIL::sub_y, AMDIL::sub_w))
        .addReg(getCompReg(SrcReg, AMDIL::sub_x, AMDIL::sub_z));
      DstReg = AMDIL::Rx1011;
      break;
    case PACK_V4I16:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LHIv2i64r), AMDIL::Rxy1012)
        .addReg(SrcReg);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LLOv2i64r), AMDIL::Rxy1011)
        .addReg(SrcReg);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UBIT_INSERTv2i32rrrr), AMDIL::Rxy1011)
        .addImm(mMFI->addi64Literal(16ULL | (16ULL << 32)))
        .addImm(mMFI->addi64Literal(16ULL | (16ULL << 32)))
        .addReg(AMDIL::Rxy1012).addReg(AMDIL::Rxy1011);
      DstReg = AMDIL::Rxy1011;
      break;
    case UNPACK_V2I8:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UBIT_EXTRACTv2i32rrr), DstReg)
        .addImm(mMFI->addi32Literal(8))
        .addImm(mMFI->addi64Literal(8ULL << 32))
        .addReg(SrcReg);
      break;
    case UNPACK_V4I8:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UBIT_EXTRACTv4i32rrr), DstReg)
        .addImm(mMFI->addi32Literal(8))
        .addImm(mMFI->addi128Literal(8ULL << 32, (16ULL | (24ULL << 32))))
        .addReg(SrcReg);
        break;
    case UNPACK_V2I16:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UBIT_EXTRACTv2i32rrr), DstReg)
        .addImm(mMFI->addi32Literal(16))
        .addImm(mMFI->addi64Literal(16ULL << 32))
        .addReg(SrcReg);
        break;
    case UNPACK_V4I16:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::USHRv2i32i32rr), AMDIL::Rxy1012)
        .addReg(SrcReg)
        .addImm(mMFI->addi32Literal(16));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LCREATEv2i64rr), DstReg)
        .addReg(SrcReg).addReg(AMDIL::Rxy1012);
      break;
  };
}

static bool isAlignedInst(MachineInstr *MI) {
  if (!MI->memoperands_empty()) {
    return ((*MI->memoperands_begin())->getAlignment()
        & ((*MI->memoperands_begin())->getSize() - 1)) == 0;
  }
  return true;
}

void AMDILEGIOExpansionImpl::expandGlobalLoad(MachineInstr *MI) {
  bool UsesArena = isArenaOp(MI);
  bool Cacheable = isCacheableOp(MI);
  bool Aligned = isAlignedInst(MI);
  uint32_t ID = getPointerID(MI);
  mMFI->setOutputInst();
  uint32_t AddyReg = MI->getOperand(1).getReg();
  uint32_t DataReg = MI->getOperand(0).getReg();
  uint32_t Reg = getPackedReg(DataReg, getPackedID(MI));
  // These instructions are generated before the current MI.
  expandLoadStartCode(MI, AddyReg);
  expandArenaSetup(MI, AddyReg);
  DebugLoc DL = MI->getDebugLoc();
  if (getMemorySize(MI) == 1) {
    Reg = get1stI32SubReg(Reg);
    if (UsesArena) {
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32LOADi8), Reg)
        .addReg(AddyReg)
        .addImm(ID);
    } else {
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1008)
        .addReg(AddyReg)
        .addImm(mMFI->addi32Literal(3));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1010)
        .addReg(AddyReg)
        .addImm(mMFI->addi32Literal(0xFFFFFFFC));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ADDv4i32rr), AMDIL::R1008)
        .addReg(AMDIL::Rx1008)
        .addImm(mMFI->addi128Literal(0xFFFFFFFFULL << 32,
              (0xFFFFFFFEULL | (0xFFFFFFFDULL << 32))));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::IEQv4i32rr), AMDIL::R1012)
        .addReg(AMDIL::R1008)
        .addImm(mMFI->addi32Literal(0));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rx1008)
        .addReg(AMDIL::Rx1012)
        .addImm(mMFI->addi32Literal(0))
        .addImm(mMFI->addi32Literal(24));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rx1008)
        .addReg(AMDIL::Ry1012)
        .addImm(mMFI->addi32Literal(8))
        .addReg(AMDIL::Rx1008);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rx1008)
        .addReg(AMDIL::Rz1012)
        .addImm(mMFI->addi32Literal(16))
        .addReg(AMDIL::Rx1008);
      if (Cacheable) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVRAW32LOADCACHEDi32),
            AMDIL::Rx1011).addReg(AMDIL::Rx1010).addImm(ID);

      } else {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVRAW32LOADi32),
            AMDIL::Rx1011).addReg(AMDIL::Rx1010).addImm(ID);

      }
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::IBIT_EXTRACTi32rrr), Reg)
          .addImm(mMFI->addi32Literal(8))
          .addReg(AMDIL::Rx1008)
          .addReg(AMDIL::Rx1011);
    }
  } else if (getMemorySize(MI) == 2) {
    Reg = get1stI32SubReg(Reg);
    if (UsesArena) {
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32LOADi16), Reg)
        .addReg(AddyReg)
        .addImm(ID);
    } else {
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1008)
        .addReg(AddyReg)
        .addImm(mMFI->addi32Literal(3));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHRi32i32rr), AMDIL::Rx1008)
        .addReg(AMDIL::Rx1008)
        .addImm(mMFI->addi32Literal(1));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1010)
        .addReg(AddyReg)
        .addImm(mMFI->addi32Literal(0xFFFFFFFC));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rx1008)
        .addReg(AMDIL::Rx1008)
        .addImm(mMFI->addi32Literal(16))
        .addImm(mMFI->addi32Literal(0));
      if (Cacheable) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVRAW32LOADCACHEDi32),
            AMDIL::Rx1011).addReg(AMDIL::Rx1010).addImm(ID);

      } else {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVRAW32LOADi32),
            AMDIL::Rx1011).addReg(AMDIL::Rx1010).addImm(ID);

      }
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::IBIT_EXTRACTi32rrr), Reg)
          .addImm(mMFI->addi32Literal(16))
          .addReg(AMDIL::Rx1008)
          .addReg(AMDIL::Rx1011);
    }
  } else if (getMemorySize(MI) == 4) {
    uint32_t opc = AMDIL::UAVRAW32LOADi32;
    if (UsesArena) {
      opc = AMDIL::UAVARENA32LOADi32;
    } else if (Cacheable) {
      opc = AMDIL::UAVRAW32LOADCACHEDi32;
    }
    Reg = get1stI32SubReg(Reg);
    if (!isXComponentReg(Reg))
      Reg = AMDIL::Rx1011;
    BuildMI(*mBB, MI, DL, mTII->get(opc), Reg)
      .addReg(AddyReg)
      .addImm(ID);
  } else if (getMemorySize(MI) == 8) {
    Reg = get1stI64SubReg(Reg);
    if (UsesArena) {
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32LOADi32),
        getCompReg(Reg, AMDIL::sub_x, AMDIL::sub_z))
        .addReg(getCompReg(AddyReg, AMDIL::sub_x))
        .addImm(ID);
      if (mSTM->usesHardware(AMDIL::Caps::ArenaVectors)) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32LOADi32),
          getCompReg(Reg, AMDIL::sub_y, AMDIL::sub_w))
          .addReg(getCompReg(AddyReg, AMDIL::sub_y))
          .addImm(ID);
      } else {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::VEXTRACTv4i32r), AMDIL::Rx1007)
          .addReg(AddyReg)
          .addImm(2);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32LOADi32), AMDIL::Rx1008)
          .addReg(AMDIL::Rx1007)
          .addImm(ID);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LCREATEi64rr), Reg)
          .addReg(getCompReg(Reg, AMDIL::sub_x, AMDIL::sub_z))
          .addReg(AMDIL::Rx1008);
      }
    } else {
      if (!isXYComponentReg(Reg))
        Reg = AMDIL::Rxy1011;
      uint32_t opc
        = Cacheable ? (Aligned ? AMDIL::UAVRAW32LOADCACHEDALIGNEDv2i32
                               : AMDIL::UAVRAW32LOADCACHEDv2i32)
                    : AMDIL::UAVRAW32LOADv2i32;
      BuildMI(*mBB, MI, DL, mTII->get(opc), Reg).addReg(AddyReg).addImm(ID);
    }
  } else {
    assert(is128BitRegister(Reg) && "not 128bit register");
    if (UsesArena) {
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32LOADi32), getCompReg(Reg, AMDIL::sub_x))
        .addReg(getCompReg(AddyReg, AMDIL::sub_x))
        .addImm(ID);
      if (mSTM->usesHardware(AMDIL::Caps::ArenaVectors)) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32LOADi32), getCompReg(Reg, AMDIL::sub_y))
          .addReg(getCompReg(AddyReg, AMDIL::sub_y))
          .addImm(ID);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32LOADi32), getCompReg(Reg, AMDIL::sub_z))
          .addReg(getCompReg(AddyReg, AMDIL::sub_z))
          .addImm(ID);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32LOADi32), getCompReg(Reg, AMDIL::sub_w))
          .addReg(getCompReg(AddyReg, AMDIL::sub_w))
          .addImm(ID);
      } else {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::VEXTRACTv4i32r), AMDIL::Rx1007)
          .addReg(AddyReg)
          .addImm(2);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32LOADi32), AMDIL::Rx1008)
          .addReg(AMDIL::Rx1007)
          .addImm(ID);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LCREATEi64rr), getCompReg(Reg, AMDIL::sub_xy))
          .addReg(getCompReg(Reg, AMDIL::sub_x))
          .addReg(AMDIL::Rx1008);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::VEXTRACTv4i32r), AMDIL::Rx1007)
          .addReg(AddyReg)
          .addImm(3);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32LOADi32), AMDIL::Rx1008)
          .addReg(AMDIL::Rx1007)
          .addImm(ID);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::VEXTRACTv4i32r), AMDIL::Rx1007)
          .addReg(AddyReg)
          .addImm(4);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32LOADi32), AMDIL::Rx1006)
          .addReg(AMDIL::Rx1007)
          .addImm(ID);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LCREATEi64rr), getCompReg(Reg, AMDIL::sub_zw))
          .addReg(AMDIL::Rx1006)
          .addReg(AMDIL::Rx1008);
      }
    } else {
      uint32_t opc
        = Cacheable ? (Aligned ? AMDIL::UAVRAW32LOADCACHEDALIGNEDv4i32
                               : AMDIL::UAVRAW32LOADCACHEDv4i32)
                    : AMDIL::UAVRAW32LOADv4i32;
      BuildMI(*mBB, MI, DL, mTII->get(opc), Reg).addReg(AddyReg).addImm(ID);
    }
  }

  if (isPackedInst(MI)) {
    expandPackedData(MI, Reg, DataReg);
    Reg = DataReg;
  }
  if (isExtendLoad(MI)) {
    expandExtendLoad(MI, Reg, DataReg);
    MI->getOperand(0).setReg(DataReg);
  } else if (Reg != DataReg) {
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::COPY), DataReg).addReg(Reg);
  }
}

void AMDILEGIOExpansionImpl::expandConstantLoad(MachineInstr *MI) {
  if (!isHardwareInst(MI) || MI->memoperands_empty()) {
    return expandGlobalLoad(MI);
  }
  uint32_t cID = getPointerID(MI);
  if (cID < 2) {
    return expandGlobalLoad(MI);
  }
  if (!mMFI->usesConstant() && mMFI->isKernel()) {
    mMFI->addErrorMsg(amd::CompilerErrorMessage[MEMOP_NO_ALLOCATION]);
  }

  assert(mSTM->getGeneration() <= AMDIL::NORTHERN_ISLANDS &&
         "This should not be used on SI");

  DebugLoc DL = MI->getDebugLoc();
  uint32_t OrigReg = MI->getOperand(1).getReg();
  uint32_t AddyReg = MI->getOperand(1).getReg();
  uint32_t DataReg = MI->getOperand(0).getReg();
  uint32_t Reg = getPackedReg(DataReg, getPackedID(MI));
  // These instructions go before the current MI.
  expandLoadStartCode(MI, AddyReg);
  switch (getMemorySize(MI)) {
    default:
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::SHRi32i32rr), AMDIL::Rx1010)
        .addReg(AddyReg)
        .addImm(mMFI->addi32Literal(4));
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::CB32LOAD), Reg)
        .addReg(AMDIL::Rx1010)
        .addImm(cID);
      break;
    case 1:
    case 2:
    case 4:
      emitVectorAddressCalc(MI, true, true, AddyReg);
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::CB32LOAD), AMDIL::R1011)
        .addReg(AddyReg)
        .addImm(cID);
      Reg = get1stI32SubReg(Reg);
      emitDataLoadSelect(MI, Reg, OrigReg);
      break;
    case 8:
      emitVectorAddressCalc(MI, false, true, AddyReg);
      Reg = get1stI64SubReg(Reg);
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::CB32LOAD), AMDIL::R1011)
        .addReg(AddyReg)
        .addImm(cID);
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::CMOV_LOGICALi64rrr), Reg)
        .addReg(AMDIL::Rxy1008)
        .addReg(AMDIL::Rxy1011)
        .addReg(AMDIL::Rzw1011);
      break;
  }

  if (isPackedInst(MI)) {
    expandPackedData(MI, Reg, DataReg);
    Reg = DataReg;
  }

  if (isExtendLoad(MI)) {
    expandExtendLoad(MI, Reg, DataReg);
    MI->getOperand(0).setReg(DataReg);
  } else {
    assert(Reg == DataReg && "inconsistent register");
  }
}

void AMDILEGIOExpansionImpl::expandRegionLoad(MachineInstr *MI) {
  bool HWRegion = mSTM->usesHardware(AMDIL::Caps::RegionMem);
  if (!mSTM->isSupported(AMDIL::Caps::RegionMem)) {
    mMFI->addErrorMsg(
        amd::CompilerErrorMessage[REGION_MEMORY_ERROR]);
    return;
  }
  if (!HWRegion || !isHardwareRegion(MI)) {
    return expandGlobalLoad(MI);
  }
  if (!mMFI->usesGDS() && mMFI->isKernel()) {
    mMFI->addErrorMsg(amd::CompilerErrorMessage[MEMOP_NO_ALLOCATION]);
  }
  DebugLoc DL = MI->getDebugLoc();
  unsigned mulOp = 0;
  uint32_t gID = getPointerID(MI);
  assert(gID && "Found a GDS load that was incorrectly marked as zero ID!\n");
  if (!gID) {
    gID = mSTM->getResourceID(AMDIL::GDS_ID);
    mMFI->addErrorMsg(amd::CompilerWarningMessage[RECOVERABLE_ERROR]);
  }
  uint32_t AddyReg = MI->getOperand(1).getReg();
  uint32_t DataReg = MI->getOperand(0).getReg();
  uint32_t Reg = getPackedReg(DataReg, getPackedID(MI));
  // These instructions are generated before the current MI.
  expandLoadStartCode(MI, AddyReg);
  switch (getMemorySize(MI)) {
    default:
      assert(is128BitRegister(Reg) && "not 128bit register");
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ADDv4i32rr), AMDIL::R1010)
        .addReg(AddyReg)
        .addImm(mMFI->addi128Literal(4ULL << 32, 8ULL | (12ULL << 32)));

      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32LOADi32r), getCompReg(Reg, AMDIL::sub_x))
        .addReg(AMDIL::Rx1010)
        .addImm(gID);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32LOADi32r), getCompReg(Reg, AMDIL::sub_y))
        .addReg(AMDIL::Ry1010)
        .addImm(gID);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32LOADi32r), getCompReg(Reg, AMDIL::sub_z))
        .addReg(AMDIL::Rz1010)
        .addImm(gID);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32LOADi32r), getCompReg(Reg, AMDIL::sub_w))
        .addReg(AMDIL::Rw1010)
        .addImm(gID);
      break;
    case 1:
      Reg = get1stI32SubReg(Reg);
      if (!mSTM->usesHardware(AMDIL::Caps::ByteGDSOps)) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1008)
          .addReg(AddyReg)
          .addImm(mMFI->addi32Literal(3));
        mulOp = (mSTM->usesSoftware(AMDIL::Caps::RegionMem))
          ? AMDIL::UMULi32rr : AMDIL::UMUL24i32rr;
        BuildMI(*mBB, MI, DL, mTII->get(mulOp), AMDIL::Rx1008)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi32Literal(8));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1010)
          .addReg(AddyReg)
          .addImm(mMFI->addi32Literal(0xFFFFFFFC));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32LOADi32r), Reg)
          .addReg(AMDIL::Rx1010)
          .addImm(gID);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::IBIT_EXTRACTi32rrr), Reg)
          .addImm(mMFI->addi32Literal(8))
          .addReg(AMDIL::Rx1008)
          .addReg(Reg);
      } else {
        if (isSExtLoadInst(MI)) {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32LOADi8r), Reg)
            .addReg(AddyReg)
            .addImm(gID);
        } else {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32LOADu8r), Reg)
            .addReg(AddyReg)
            .addImm(gID);
        }
      }
      break;
    case 2:
      Reg = get1stI32SubReg(Reg);
      if (!mSTM->usesHardware(AMDIL::Caps::ByteGDSOps)) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1008)
          .addReg(AddyReg)
          .addImm(mMFI->addi32Literal(3));
        mulOp = (mSTM->usesSoftware(AMDIL::Caps::RegionMem))
          ? AMDIL::UMULi32rr : AMDIL::UMUL24i32rr;
        BuildMI(*mBB, MI, DL, mTII->get(mulOp), AMDIL::Rx1008)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi32Literal(8));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1010)
          .addReg(AddyReg)
          .addImm(mMFI->addi32Literal(0xFFFFFFFC));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32LOADi32r), Reg)
          .addReg(AMDIL::Rx1010)
          .addImm(gID);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::IBIT_EXTRACTi32rrr), Reg)
          .addImm(mMFI->addi32Literal(16))
          .addReg(AMDIL::Rx1008)
          .addReg(Reg);
      } else {
        if (isSExtLoadInst(MI)) {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32LOADi16r), Reg)
            .addReg(AddyReg)
            .addImm(gID);
        } else {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32LOADu16r), Reg)
            .addReg(AddyReg)
            .addImm(gID);
        }
      }
      break;
    case 4:
      Reg = get1stI32SubReg(Reg);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32LOADi32r), Reg)
        .addReg(AddyReg)
        .addImm(gID);
      break;
    case 8:
      Reg = get1stI64SubReg(Reg);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ADDv2i32rr), AMDIL::Rxy1010)
        .addReg(AddyReg)
        .addImm(mMFI->addi64Literal(4ULL << 32));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32LOADi32r), getCompReg(Reg, AMDIL::sub_x, AMDIL::sub_z))
        .addReg(AddyReg)
        .addImm(gID);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32LOADi32r), getCompReg(Reg, AMDIL::sub_y, AMDIL::sub_w))
        .addReg(AMDIL::Ry1010)
        .addImm(gID);
      break;
  };

  if (isPackedInst(MI)) {
    expandPackedData(MI, Reg, DataReg);
    Reg = DataReg;
  }
  if (isExtendLoad(MI)) {
    expandExtendLoad(MI, Reg, DataReg);
    MI->getOperand(0).setReg(DataReg);
  } else if (Reg != DataReg) {
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::COPY), DataReg).addReg(Reg);
  }
}

void AMDILEGIOExpansionImpl::expandLocalLoad(MachineInstr *MI) {
  bool HWLocal = mSTM->usesHardware(AMDIL::Caps::LocalMem);
  if (!HWLocal || !isHardwareLocal(MI)) {
    return expandGlobalLoad(MI);
  }
  if (!mMFI->usesLDS() && mMFI->isKernel()) {
    mMFI->addErrorMsg(amd::CompilerErrorMessage[MEMOP_NO_ALLOCATION]);
  }
  uint32_t lID = getPointerID(MI);
  assert(lID && "Found a LDS load that was incorrectly marked as zero ID!\n");
  if (!lID) {
    lID = mSTM->getResourceID(AMDIL::LDS_ID);
    mMFI->addErrorMsg(amd::CompilerWarningMessage[RECOVERABLE_ERROR]);
  }
  DebugLoc DL = MI->getDebugLoc();
  unsigned mulOp = 0;
  uint32_t AddyReg = MI->getOperand(1).getReg();
  uint32_t DataReg = MI->getOperand(0).getReg();
  uint32_t Reg = getPackedReg(DataReg, getPackedID(MI));
  // These instructions are generated before the current MI.
  expandLoadStartCode(MI, AddyReg);
  switch (getMemorySize(MI)) {
    default:
      assert(is128BitRegister(Reg) && "not 128bit register");
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LDS32LOADv4i32r), Reg)
        .addReg(AddyReg)
        .addImm(lID);
      break;
    case 8:
      {
        Reg = get1stI64SubReg(Reg);
        if (!isXYComponentReg(Reg))
          Reg = AMDIL::Rxy1011;

	unsigned Opcode = (mSTM->getGeneration() >= AMDIL::SOUTHERN_ISLANDS &&
	  (*MI->memoperands_begin())->getAlignment() >= 8) ?
	  AMDIL::LDS32LOADi64r : AMDIL::LDS32LOADv2i32r;

        BuildMI(*mBB, MI, DL, mTII->get(Opcode), Reg)
          .addReg(AddyReg)
          .addImm(lID);
      }
      break;
    case 4:
      {
        Reg = get1stI32SubReg(Reg);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LDS32LOADi32r), Reg)
          .addReg(AddyReg)
          .addImm(lID);
      }
      break;
    case 1:
      Reg = get1stI32SubReg(Reg);
      if (!mSTM->usesHardware(AMDIL::Caps::ByteLDSOps)) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1008)
          .addReg(AddyReg)
          .addImm(mMFI->addi32Literal(3));
        mulOp = (mSTM->usesSoftware(AMDIL::Caps::LocalMem))
          ? AMDIL::UMULi32rr : AMDIL::UMUL24i32rr;
        BuildMI(*mBB, MI, DL, mTII->get(mulOp), AMDIL::Rx1008)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi32Literal(8));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1010)
          .addReg(AddyReg)
          .addImm(mMFI->addi32Literal(0xFFFFFFFC));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LDS32LOADi32r), Reg)
          .addReg(AddyReg)
          .addImm(lID);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::IBIT_EXTRACTi32rrr), Reg)
          .addImm(mMFI->addi32Literal(8))
          .addReg(AMDIL::Rx1008)
          .addReg(Reg);
      } else {
        if (isSWSExtLoadInst(MI)) {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LDS32LOADi8r), Reg)
            .addReg(AddyReg)
            .addImm(lID);
        } else {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LDS32LOADu8r), Reg)
            .addReg(AddyReg)
            .addImm(lID);
        }
      }
      break;
    case 2:
      Reg = get1stI32SubReg(Reg);
      if (!mSTM->usesHardware(AMDIL::Caps::ByteLDSOps)) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1008)
          .addReg(AddyReg)
          .addImm(mMFI->addi32Literal(3));
        mulOp = (mSTM->usesSoftware(AMDIL::Caps::LocalMem))
          ? AMDIL::UMULi32rr : AMDIL::UMUL24i32rr;
        BuildMI(*mBB, MI, DL, mTII->get(mulOp), AMDIL::Rx1008)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi32Literal(8));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1010)
          .addReg(AddyReg)
          .addImm(mMFI->addi32Literal(0xFFFFFFFC));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LDS32LOADi32r), Reg)
          .addReg(AMDIL::Rx1010)
          .addImm(lID);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::IBIT_EXTRACTi32rrr), Reg)
          .addImm(mMFI->addi32Literal(16))
          .addReg(AMDIL::Rx1008)
          .addReg(Reg);
      } else {
        if (isSExtLoadInst(MI)) {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LDS32LOADi16r), Reg)
            .addReg(AddyReg)
            .addImm(lID);
        } else {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LDS32LOADu16r), Reg)
            .addReg(AddyReg)
            .addImm(lID);
        }
      }
      break;
  }

  if (isPackedInst(MI)) {
    expandPackedData(MI, Reg, DataReg);
    Reg = DataReg;
  }
  if (isExtendLoad(MI)) {
    expandExtendLoad(MI, Reg, DataReg);
    MI->getOperand(0).setReg(DataReg);
  } else if (Reg != DataReg) {
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::COPY), DataReg).addReg(Reg);
  }
}

void AMDILEGIOExpansionImpl::expandGlobalStore(MachineInstr *MI) {
  bool UsesArena = isArenaOp(MI);
  uint32_t ID = getPointerID(MI);
  mMFI->setOutputInst();
  DebugLoc DL = MI->getDebugLoc();
  uint32_t AddyReg = MI->getOperand(1).getReg();
  uint32_t DataReg = 0;
  if (MI->getOperand(0).isReg()) {
    DataReg = MI->getOperand(0).getReg();
  }
  // These instructions are expandted before the current MI.
  expandStoreSetupCode(MI, AddyReg, DataReg);
  expandArenaSetup(MI, AddyReg);
  switch (getMemorySize(MI)) {
    default:
      if (UsesArena) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32STOREi32),
            getCompReg(AddyReg, AMDIL::sub_x))
          .addReg(getCompReg(DataReg, AMDIL::sub_x))
          .addImm(ID);
        if (mSTM->usesHardware(AMDIL::Caps::ArenaVectors)) {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32STOREi32),
              getCompReg(AddyReg, AMDIL::sub_y))
            .addReg(getCompReg(DataReg, AMDIL::sub_y))
            .addImm(ID);
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32STOREi32),
              getCompReg(AddyReg, AMDIL::sub_z))
            .addReg(getCompReg(DataReg, AMDIL::sub_z))
            .addImm(ID);
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32STOREi32),
              getCompReg(AddyReg, AMDIL::sub_w))
            .addReg(getCompReg(DataReg, AMDIL::sub_w))
            .addImm(ID);
        } else {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::VEXTRACTv4i32r), AMDIL::Rx1007)
            .addReg(AddyReg)
            .addImm(2);
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::VEXTRACTv4i32r), AMDIL::Rx1008)
            .addReg(DataReg)
            .addImm(2);
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32STOREi32), AMDIL::Rx1007)
            .addReg(AMDIL::Rx1008)
            .addImm(ID);
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::VEXTRACTv4i32r), AMDIL::Rx1007)
            .addReg(AddyReg)
            .addImm(3);
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::VEXTRACTv4i32r), AMDIL::Rx1008)
            .addReg(DataReg)
            .addImm(3);
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32STOREi32), AMDIL::Rx1007)
            .addReg(AMDIL::Rx1008)
            .addImm(ID);
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::VEXTRACTv4i32r), AMDIL::Rx1007)
            .addReg(AddyReg)
            .addImm(4);
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::VEXTRACTv4i32r), AMDIL::Rx1008)
            .addReg(DataReg)
            .addImm(4);
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32STOREi32), AMDIL::Rx1007)
            .addReg(AMDIL::Rx1008)
            .addImm(ID);
        }
      } else {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVRAW32STOREv4i32), AMDIL::MEM)
          .addReg(AddyReg)
          .addReg(DataReg)
          .addImm(ID);
      }
      break;
    case 1:
      if (UsesArena) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), DataReg)
          .addReg(DataReg)
          .addImm(mMFI->addi32Literal(0xFF));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32STOREi8), AddyReg)
          .addReg(DataReg)
          .addImm(ID);
      } else {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVRAW32STOREi32), AMDIL::MEMx)
          .addReg(AddyReg)
          .addReg(DataReg)
          .addImm(ID);
      }
      break;
    case 2:
      if (UsesArena) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), DataReg)
          .addReg(DataReg)
          .addImm(mMFI->addi32Literal(0xFFFF));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32STOREi16), AddyReg)
          .addReg(DataReg)
          .addImm(ID);
      } else {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVRAW32STOREi32), AMDIL::MEMx)
          .addReg(AddyReg)
          .addReg(DataReg)
          .addImm(ID);
      }
      break;
    case 4:
      if (UsesArena) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32STOREi32), AddyReg)
          .addReg(DataReg)
          .addImm(ID);
      } else {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVRAW32STOREi32), AMDIL::MEMx)
          .addReg(AddyReg)
          .addReg(DataReg)
          .addImm(ID);
      }
      break;
    case 8:
      if (UsesArena) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32STOREi32),
            getCompReg(AddyReg, AMDIL::sub_x, AMDIL::sub_z))
          .addReg(getCompReg(DataReg, AMDIL::sub_x, AMDIL::sub_z))
          .addImm(ID);
        if (mSTM->usesHardware(AMDIL::Caps::ArenaVectors)) {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32STOREi32),
              getCompReg(AddyReg, AMDIL::sub_y, AMDIL::sub_w))
            .addReg(getCompReg(DataReg, AMDIL::sub_y, AMDIL::sub_w))
            .addImm(ID);
        } else {
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::VEXTRACTv4i32r), AMDIL::Rx1007)
            .addReg(AddyReg)
            .addImm(2);
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::VEXTRACTv4i32r), AMDIL::Rx1008)
            .addReg(DataReg)
            .addImm(2);
          BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVARENA32STOREi32), AMDIL::Rx1007)
            .addReg(AMDIL::Rx1008)
            .addImm(ID);
        }
      } else {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UAVRAW32STOREv2i32), AMDIL::MEMxy)
          .addReg(AddyReg)
          .addReg(DataReg)
          .addImm(ID);
      }
      break;
  };
}

void AMDILEGIOExpansionImpl::expandRegionStore(MachineInstr *MI) {
  bool HWRegion
      = mSTM->usesHardware(AMDIL::Caps::RegionMem);
  if (!HWRegion || !isHardwareRegion(MI)) {
    return expandGlobalStore(MI);
  }
  mMFI->setOutputInst();
  if (!mMFI->usesGDS() && mMFI->isKernel()) {
    mMFI->addErrorMsg(amd::CompilerErrorMessage[MEMOP_NO_ALLOCATION]);
  }
  uint32_t gID = getPointerID(MI);
  assert(gID
     && "Found a GDS store that was incorrectly marked as zero ID!\n");
  if (!gID) {
    gID = mSTM->getResourceID(AMDIL::GDS_ID);
    mMFI->addErrorMsg(amd::CompilerWarningMessage[RECOVERABLE_ERROR]);
  }
  DebugLoc DL = MI->getDebugLoc();
  unsigned MulOp = HWRegion ? AMDIL::UMUL24i32rr : AMDIL::UMUL24i32rr;
  uint32_t AddyReg = MI->getOperand(1).getReg();
  uint32_t DataReg = MI->getOperand(0).getReg();
  // These instructions are expandted before the current MI.
  expandStoreSetupCode(MI, AddyReg, DataReg);
  switch (getMemorySize(MI)) {
    default:

      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ADDv4i32rr), AMDIL::R1010)
        .addReg(AddyReg)
        .addImm(mMFI->addi128Literal(4ULL << 32, 8ULL | (12ULL << 32)));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32STOREi32r), AMDIL::Rx1010)
        .addReg(getCompReg(DataReg, AMDIL::sub_x))
        .addImm(gID);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32STOREi32r), AMDIL::Ry1010)
        .addReg(getCompReg(DataReg, AMDIL::sub_y))
        .addImm(gID);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32STOREi32r), AMDIL::Rz1010)
        .addReg(getCompReg(DataReg, AMDIL::sub_z))
        .addImm(gID);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32STOREi32r), AMDIL::Rw1010)
        .addReg(getCompReg(DataReg, AMDIL::sub_w))
        .addImm(gID);
      break;
    case 1:
      if (!mSTM->usesHardware(AMDIL::Caps::ByteGDSOps)) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1011)
          .addReg(DataReg)
          .addImm(mMFI->addi32Literal(0xFF));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1012)
          .addReg(AddyReg)
          .addImm(mMFI->addi32Literal(3));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ADDv4i32rr), AMDIL::R1008)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi128Literal(0xFFFFFFFFULL << 32,
                (0xFFFFFFFEULL | (0xFFFFFFFDULL << 32))));
        BuildMI(*mBB, MI, DL, mTII->get(MulOp), AMDIL::Rx1006)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi32Literal(8));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rx1007)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi32Literal(0xFFFFFF00))
          .addImm(mMFI->addi32Literal(0x00FFFFFF));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Ry1007)
          .addReg(AMDIL::Ry1008)
          .addReg(AMDIL::Rx1007)
          .addImm(mMFI->addi32Literal(0xFF00FFFF));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rz1012)
          .addReg(AMDIL::Rz1008)
          .addReg(AMDIL::Rx1007)
          .addImm(mMFI->addi32Literal(0xFFFF00FF));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHLi32i32rr), AMDIL::Rx1011)
          .addReg(AMDIL::Rx1011)
          .addReg(AMDIL::Rx1007);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ATOM_R_MSKOR_NORET))
          .addReg(AMDIL::Rx1010)
          .addImm(mMFI->addi32Literal(0))
          .addReg(AMDIL::Rx1012)
          .addReg(AMDIL::Rx1011)
          .addImm(gID);
      } else {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32STOREi8r), AddyReg)
          .addReg(DataReg)
          .addImm(gID);
      }
      break;
    case 2:
      if (!mSTM->usesHardware(AMDIL::Caps::ByteGDSOps)) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1011)
          .addReg(DataReg)
          .addImm(mMFI->addi32Literal(0x0000FFFF));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1008)
          .addReg(AddyReg)
          .addImm(mMFI->addi32Literal(3));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHRi32i32rr), AMDIL::Rx1008)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi32Literal(1));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rx1012)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi32Literal(0x0000FFFF))
          .addImm(mMFI->addi32Literal(0xFFFF0000));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rx1008)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi32Literal(16))
          .addImm(mMFI->addi32Literal(0));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHLi32i32rr), AMDIL::Rx1011)
          .addReg(AMDIL::Rx1011)
          .addReg(AMDIL::Rx1008);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ATOM_R_MSKOR_NORET))
          .addReg(AMDIL::Rx1010)
          .addImm(mMFI->addi32Literal(0))
          .addReg(AMDIL::Rx1012)
          .addReg(AMDIL::Rx1011)
          .addImm(gID);
      } else {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32STOREi16r), AddyReg)
          .addReg(DataReg)
          .addImm(gID);
      }
      break;
    case 4:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32STOREi32r), AddyReg)
        .addReg(DataReg)
        .addImm(gID);
      break;
    case 8:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ADDv2i32rr), AMDIL::Rxy1010)
        .addReg(AddyReg)
        .addImm(mMFI->addi64Literal(4ULL << 32));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32STOREi32r), AMDIL::Rx1010)
        .addReg(getCompReg(DataReg, AMDIL::sub_x))
        .addImm(gID);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::GDS32STOREi32r), AMDIL::Ry1010)
        .addReg(getCompReg(DataReg, AMDIL::sub_y))
        .addImm(gID);
      break;
  };
}

void AMDILEGIOExpansionImpl::expandLocalStore(MachineInstr *MI) {
  bool HWLocal = mSTM->usesHardware(AMDIL::Caps::LocalMem);
  if (!HWLocal || !isHardwareLocal(MI)) {
    return expandGlobalStore(MI);
  }
  DebugLoc DL = MI->getDebugLoc();
  if (!mMFI->usesLDS() && mMFI->isKernel()) {
    mMFI->addErrorMsg(amd::CompilerErrorMessage[MEMOP_NO_ALLOCATION]);
  }
  uint32_t lID = getPointerID(MI);
  assert(lID && "Found a LDS store that was incorrectly marked as zero ID!\n");
  if (!lID) {
    lID = mSTM->getResourceID(AMDIL::LDS_ID);
    mMFI->addErrorMsg(amd::CompilerWarningMessage[RECOVERABLE_ERROR]);
  }
  unsigned MulOp = HWLocal ? AMDIL::UMUL24i32rr : AMDIL::UMUL24i32rr;
  uint32_t AddyReg = MI->getOperand(1).getReg();
  uint32_t DataReg = MI->getOperand(0).getReg();
  // These instructions are expandted before the current MI.
  expandStoreSetupCode(MI, AddyReg, DataReg);
  switch (getMemorySize(MI)) {
    default:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LDS32STOREv4i32r), AMDIL::MEM)
        .addReg(AddyReg)
        .addReg(DataReg)
        .addImm(lID);
      break;
    case 8:
      if (mSTM->getGeneration() >= AMDIL::SOUTHERN_ISLANDS &&
        (*MI->memoperands_begin())->getAlignment() >= 8) {
	BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LDS32STOREi64r), AddyReg)
	  .addReg(DataReg)
	  .addImm(lID);
      } else {
	BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LDS32STOREv2i32r), AMDIL::MEMxy)
	  .addReg(AddyReg)
	  .addReg(DataReg)
	  .addImm(lID);
      }
      break;
    case 4:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LDS32STOREi32r), AddyReg)
        .addReg(DataReg)
        .addImm(lID);
      break;
    case 1:
      if (!mSTM->usesHardware(AMDIL::Caps::ByteLDSOps)) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1011)
          .addReg(DataReg)
          .addImm(mMFI->addi32Literal(0xFF));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1012)
          .addReg(AddyReg)
          .addImm(mMFI->addi32Literal(3));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ADDv4i32rr), AMDIL::R1008)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi128Literal(0xFFFFFFFFULL << 32,
                (0xFFFFFFFEULL | (0xFFFFFFFDULL << 32))));
        BuildMI(*mBB, MI, DL, mTII->get(MulOp), AMDIL::Rx1006)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi32Literal(8));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rx1007)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi32Literal(0xFFFFFF00))
          .addImm(mMFI->addi32Literal(0x00FFFFFF));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rx1007)
          .addReg(AMDIL::Ry1008)
          .addReg(AMDIL::Rx1007)
          .addImm(mMFI->addi32Literal(0xFF00FFFF));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rx1012)
          .addReg(AMDIL::Rz1008)
          .addReg(AMDIL::Rx1007)
          .addImm(mMFI->addi32Literal(0xFFFF00FF));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHLi32i32rr), AMDIL::Rx1011)
          .addReg(AMDIL::Rx1011)
          .addReg(AMDIL::Rx1006);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ATOM_L_MSKOR_NORET))
          .addReg(AddyReg)
          .addImm(mMFI->addi32Literal(0))
          .addReg(AMDIL::Rx1012)
          .addReg(AMDIL::Rx1011)
          .addImm(lID);
      } else {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LDS32STOREi8r), AddyReg)
          .addReg(DataReg)
          .addImm(lID);
      }
      break;
    case 2:
      if (!mSTM->usesHardware(AMDIL::Caps::ByteLDSOps)) {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1011)
          .addReg(DataReg)
          .addImm(mMFI->addi32Literal(0x0000FFFF));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1008)
          .addReg(AddyReg)
          .addImm(mMFI->addi32Literal(3));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHRi32i32rr), AMDIL::Rx1008)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi32Literal(1));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rx1012)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi32Literal(0x0000FFFF))
          .addImm(mMFI->addi32Literal(0xFFFF0000));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rx1008)
          .addReg(AMDIL::Rx1008)
          .addImm(mMFI->addi32Literal(16))
          .addImm(mMFI->addi32Literal(0));
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHLi32i32rr), AMDIL::Rx1011)
          .addReg(AMDIL::Rx1011)
          .addReg(AMDIL::Rx1008);
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ATOM_L_MSKOR_NORET))
          .addReg(AddyReg)
          .addImm(mMFI->addi32Literal(0))
          .addReg(AMDIL::Rx1012)
          .addReg(AMDIL::Rx1011)
          .addImm(lID);
      } else {
        BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LDS32STOREi16r), AddyReg)
          .addReg(DataReg)
          .addImm(lID);
      }
      break;
  }
}

void AMDILEGIOExpansionImpl::expandPrivateLoad(MachineInstr *MI) {
  if (!mMFI->usesScratch() && mMFI->isKernel()) {
    mMFI->addErrorMsg(amd::CompilerErrorMessage[MEMOP_NO_ALLOCATION]);
  }
  uint32_t xID = getPointerID(MI);
  assert(xID &&
      "Found a scratch load that was incorrectly marked as zero ID!\n");
  if (!xID) {
    xID = mSTM->getResourceID(AMDIL::SCRATCH_ID);
    mMFI->addErrorMsg(amd::CompilerWarningMessage[RECOVERABLE_ERROR]);
  }
  DebugLoc DL = MI->getDebugLoc();
  uint32_t origReg = MI->getOperand(1).getReg();
  uint32_t AddyReg = MI->getOperand(1).getReg();
  uint32_t DataReg = MI->getOperand(0).getReg();
  uint32_t Reg = getPackedReg(DataReg, getPackedID(MI));
  // These instructions go before the current MI.
  expandLoadStartCode(MI, AddyReg);
  switch (getMemorySize(MI)) {
    default:
      // Since the private register is a 128 bit aligned, we have to align
      // the address first, since our source address is 32bit aligned and then
      // load the data. This produces the following pseudo-IL:
      // ishr r1010.x___, r1010.xxxx, 4
	    // mov r1011, x1[r1010.x]
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::SHRi32i32rr), AMDIL::Rx1010)
        .addReg(AddyReg)
        .addImm(mMFI->addi32Literal(4));
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::SCRATCH32LOAD), Reg)
        .addReg(AMDIL::Rx1010)
        .addImm(xID);
      break;
    case 1:
    case 2:
    case 4:
      emitVectorAddressCalc(MI, true, true, AddyReg);
      // This produces the following pseudo-IL:
      // mov r1011, x1[r1007.x]
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::SCRATCH32LOAD), AMDIL::R1011)
        .addReg(AddyReg)
        .addImm(xID);
      // These instructions go after the current MI.
      Reg = get1stI32SubReg(Reg);
      emitDataLoadSelect(MI, Reg, origReg);
     break;
    case 8:
      emitVectorAddressCalc(MI, false, true, AddyReg);
      Reg = get1stI64SubReg(Reg);
      // This produces the following pseudo-IL:
      // mov r1011, x1[r1007.x]
      // cmov_logical r1011.xy__, r1008.xxxx, r1011.xy, r1011.zw
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::SCRATCH32LOAD), AMDIL::R1011)
        .addReg(AddyReg)
        .addImm(xID);
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::CMOV_LOGICALi64rrr), Reg)
        .addReg(AMDIL::Rxy1008)
        .addReg(AMDIL::Rxy1011)
        .addReg(AMDIL::Rzw1011);
     break;
  }

  if (isPackedInst(MI)) {
    expandPackedData(MI, Reg, DataReg);
    Reg = DataReg;
  }

  if (isExtendLoad(MI)) {
    expandExtendLoad(MI, Reg, DataReg);
    MI->getOperand(0).setReg(DataReg);
  } else {
    assert(Reg == DataReg && "inconsistent register");
  }
}
void AMDILEGIOExpansionImpl::expandPrivateStore(MachineInstr *MI) {
  if (!mMFI->usesScratch() && mMFI->isKernel()) {
    mMFI->addErrorMsg(amd::CompilerErrorMessage[MEMOP_NO_ALLOCATION]);
  }
  uint32_t xID = getPointerID(MI);
  assert(xID &&
      "Found a scratch store that was incorrectly marked as zero ID!\n");
  if (!xID) {
    xID = mSTM->getResourceID(AMDIL::SCRATCH_ID);
    mMFI->addErrorMsg(amd::CompilerWarningMessage[RECOVERABLE_ERROR]);
  }
  DebugLoc DL = MI->getDebugLoc();
  uint32_t DataReg = MI->getOperand(0).getReg();
  uint32_t AddyReg = MI->getOperand(1).getReg();
  if (AddyReg == AMDIL::DFP) {
    BuildMI(*mBB, MI, DL, mTII->get(TargetOpcode::COPY), AMDIL::Rx1010)
            .addReg(AddyReg);
    AddyReg = AMDIL::Rx1010;
  }
   // These instructions go before the current MI.
  expandStoreSetupCode(MI, AddyReg, DataReg);
  uint32_t OrigReg = AddyReg;
  switch (getMemorySize(MI)) {
    default:
      // This section generates the following pseudo-IL:
      // ishr r1010.x___, r1010.xxxx, 4
	    // mov x1[r1010.x], r1011
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::SHRi32i32rr), AMDIL::Rx1010)
        .addReg(AddyReg)
        .addImm(mMFI->addi32Literal(4));
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::SCRATCH32STORE), AMDIL::Rx1010)
        .addReg(DataReg)
        .addImm(xID);
      break;
    case 1:
      emitVectorAddressCalc(MI, true, true, AddyReg);
      // This section generates the following pseudo-IL:
      // mov r1002, x1[r1007.x]
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::SCRATCH32LOAD), AMDIL::R1002)
        .addReg(AddyReg)
        .addImm(xID);
      emitComponentExtract(MI, AMDIL::R1002, AMDIL::Rx1002, true);
      // This section generates the following pseudo-IL:
      // iand r1003.x, r1010.x, 3
      // iadd r1001, r1003.x, {0, -1, -2, -3}
      // ieq r1001, r1001, 0
      // ishr r1002, r1002.x, {0, 8, 16, 24}
      // cmov_logical r1002, r1001, r1011.x, r1002
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1003)
        .addReg(OrigReg)
        .addImm(mMFI->addi32Literal(3));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ADDv4i32rr), AMDIL::R1001)
        .addReg(AMDIL::Rx1003)
        .addImm(mMFI->addi128Literal(0xFFFFFFFFULL << 32,
              (0xFFFFFFFEULL | (0xFFFFFFFDULL << 32))));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::IEQv4i32rr), AMDIL::R1001)
        .addReg(AMDIL::R1001)
        .addImm(mMFI->addi32Literal(0));
     BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHRv4i32i32rr), AMDIL::R1002)
      .addReg(AMDIL::Rx1002)
      .addImm(mMFI->addi128Literal(8ULL << 32, 16ULL | (24ULL << 32)));
     BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALv4i32rrr), AMDIL::R1002)
        .addReg(AMDIL::R1001)
        .addReg(DataReg)
        .addReg(AMDIL::R1002);

     // This section generates the following pseudo-IL:
     // mov r1001.xy, r1002.yw
     // mov r1002.xy, r1002.xz
     // ubit_insert r1002.xy, 8, 8, r1001.xy, r1002.xy
     // ubit_insert r1011.x, 16, 16, r1002.y, r1002.x
     BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LHIv2i64r), AMDIL::Rxy1001)
       .addReg(AMDIL::R1002);
     BuildMI(*mBB, MI, DL, mTII->get(AMDIL::LLOv2i64r), AMDIL::Rxy1002)
       .addReg(AMDIL::R1002);
     BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UBIT_INSERTv2i32rrrr),
             AMDIL::Rxy1002)
       .addImm(mMFI->addi32Literal(8))
       .addImm(mMFI->addi32Literal(8))
       .addReg(AMDIL::Rxy1001)
       .addReg(AMDIL::Rxy1002);
     BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UBIT_INSERTi32rrrr),
             AMDIL::Rx1011)
       .addImm(mMFI->addi32Literal(16))
       .addImm(mMFI->addi32Literal(16))
       .addReg(AMDIL::Ry1002)
       .addReg(AMDIL::Rx1002);

      DataReg = AMDIL::Rx1011;
      emitVectorAddressCalc(MI, true, false, OrigReg);
      emitVectorSwitchWrite(MI, true, OrigReg, DataReg);
      break;
    case 2:
      emitVectorAddressCalc(MI, true, true, AddyReg);
      // This section generates the following pseudo-IL:
      // mov r1002, x1[r1007.x]
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::SCRATCH32LOAD), AMDIL::R1002)
        .addReg(AddyReg)
        .addImm(xID);
      emitComponentExtract(MI, AMDIL::R1002, AMDIL::Rx1002, true);
      // This section generates the following pseudo-IL:
      // ishr r1003.x, $OrigReg, 1
      // iand r1003.x, r1003.x, 1
      // ishr r1001.x, r1002.x, 16
      // cmov_logical r1002.x, r1003.x, r1002.x, $OrigReg
      // cmov_logical r1001.x, r1003.x, $OrigReg, r1001.x
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHRi32i32rr), AMDIL::Rx1003)
        .addReg(OrigReg)
        .addImm(mMFI->addi32Literal(1));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1003)
        .addReg(AMDIL::Rx1003)
        .addImm(mMFI->addi32Literal(1));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHRi32i32rr), AMDIL::Rx1001)
        .addReg(AMDIL::Rx1002)
        .addImm(mMFI->addi32Literal(16));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rx1002)
        .addReg(AMDIL::Rx1003)
        .addReg(AMDIL::Rx1002)
        .addReg(DataReg);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), AMDIL::Rx1001)
        .addReg(AMDIL::Rx1003)
        .addReg(DataReg)
        .addReg(AMDIL::Rx1001);
      // This section generates the following pseudo-IL:
      // ubit_insert r1011.x, 16, 16, r1001.x, r1002.x
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::UBIT_INSERTi32rrrr),
              AMDIL::Rx1011)
        .addImm(mMFI->addi32Literal(16))
        .addImm(mMFI->addi32Literal(16))
        .addReg(AMDIL::Rx1001)
        .addReg(AMDIL::Rx1002);
      DataReg = AMDIL::Rx1011;

      emitVectorAddressCalc(MI, true, false, OrigReg);
      emitVectorSwitchWrite(MI, true, OrigReg, DataReg);
      break;
    case 4:
      emitVectorAddressCalc(MI, true, false, AddyReg);
      emitVectorSwitchWrite(MI, true, AddyReg, DataReg);
      break;
    case 8:
      emitVectorAddressCalc(MI, false, false, AddyReg);
      emitVectorSwitchWrite(MI, false, AddyReg, DataReg);
      break;
  };
}

void AMDILEGIOExpansionImpl::expandArenaSetup(MachineInstr *MI,
                                              uint32_t &AddyReg) {
  if (!isArenaOp(MI)) {
    return;
  }

  const MCInstrDesc &TID = MI->getDesc();
  const MCOperandInfo &TOI = TID.OpInfo[0];
  unsigned short RegClass = TOI.RegClass;
  DebugLoc DL = MI->getDebugLoc();
  switch (RegClass) {
    case AMDIL::GPRV4I16RegClassID:
    case AMDIL::GPR_64RegClassID:
    case AMDIL::GPRV2I32RegClassID:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ADDv2i32rr), AMDIL::Rxy1010)
        .addReg(AddyReg)

        .addImm(mMFI->addi64Literal(4ULL << 32));
      AddyReg = AMDIL::Rxy1010;
      break;
    default:

      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ADDv4i32rr), AMDIL::R1010)
        .addReg(AddyReg)
        .addImm(mMFI->addi128Literal(4ULL << 32, 8ULL | (12ULL << 32)));
      AddyReg = AMDIL::R1010;
      break;
    case AMDIL::GPRI8RegClassID:
    case AMDIL::GPRV2I8RegClassID:
    case AMDIL::GPRI16RegClassID:
    case AMDIL::GPRV2I16RegClassID:
    case AMDIL::GPRV4I8RegClassID:
    case AMDIL::GPR_32RegClassID:
      break;
  }
}

