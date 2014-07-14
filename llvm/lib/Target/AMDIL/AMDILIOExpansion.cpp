//===----------- AMDILIOExpansion.cpp - IO Expansion Pass -----------------===//
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
// The AMDIL IO Expansion class expands pseudo IO instructions into a sequence
// of instructions that produces the correct results. These instructions are
// not expanded earlier in the pass because any pass before this can assume to
// be able to generate a load/store instruction. So this pass can only have
// passes that execute after it if no load/store instructions can be generated.
//===----------------------------------------------------------------------===//
#include "AMDILIOExpansion.h"
#include "AMDILCIIOExpansion.h"
#include "AMDILCompilerErrors.h"
#include "AMDILCompilerWarnings.h"
#include "AMDIL.h"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILTargetMachine.h"
#include "AMDILUtilityFunctions.h"
#include "AMDILSIIOExpansion.h"
#include "llvm/DerivedTypes.h"
#include "llvm/GlobalValue.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Value.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/Support/DebugLoc.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetRegisterInfo.h"
using namespace llvm;

namespace llvm {
  FunctionPass *createAMDILIOExpansion(TargetMachine& TM,
                                       CodeGenOpt::Level OptLevel) {
    const AMDILSubtarget &STM = TM.getSubtarget<AMDILSubtarget>();

    switch (STM.getGeneration()) {
    case AMDIL::EVERGREEN:
    case AMDIL::NORTHERN_ISLANDS:
      return new AMDILEGIOExpansion();
    case AMDIL::SOUTHERN_ISLANDS:
      return new AMDILSIIOExpansion();
    case AMDIL::SEA_ISLANDS:
    case AMDIL::VOLCANIC_ISLANDS:
    case AMDIL::HDTEST:
      return new AMDILCIIOExpansion();

    case AMDIL::INVALID_GPU_FAMILY:
    default:
      llvm_unreachable("Bad generation");
    }
  }
}

AMDILIOExpansionImpl::AMDILIOExpansionImpl(MachineFunction& MF_)
  : MF(MF_), mBB(NULL),TM(MF.getTarget()) {
  mSTM = &TM.getSubtarget<AMDILSubtarget>();
  mMFI = MF.getInfo<AMDILMachineFunctionInfo>();
  mTRI = TM.getRegisterInfo();
  mTII = TM.getInstrInfo();
}

bool AMDILIOExpansionImpl::run() {
  for (MachineFunction::iterator MFI = MF.begin(), MFE = MF.end();
       MFI != MFE; ++MFI) {
    MachineBasicBlock *MBB = MFI;
    for (MachineBasicBlock::iterator MBI = MBB->begin(), MBE = MBB->end();
         MBI != MBE; ++MBI) {
      MachineInstr *MI = MBI;
      if (isIOInstruction(MI)) {
        mBB = MBB;
        saveInst = false;
        expandIOInstruction(MI);
        if (!saveInst) {
          // erase returns the instruction after
          // and we want the instruction before
          MBI = MBB->erase(MI);
          --MBI;
        }
      }
    }
  }
  return false;
}

bool AMDILIOExpansionImpl::isIOInstruction(MachineInstr *MI) {
  if (!MI) {
    return false;
  }
  if (isPtrLoadInst(MI) || isPtrStoreInst(MI)) {
    return true;
  }
  return false;
}

void AMDILIOExpansionImpl::expandIOInstruction(MachineInstr *MI) {
  assert(isIOInstruction(MI) && "Must be an IO instruction to "
                                "be passed to this function!");
  if (isPtrLoadInst(MI)) {
    if (isGlobalInst(MI)) {
      expandGlobalLoad(MI);
    } else if (isRegionInst(MI)) {
      expandRegionLoad(MI);
    } else if (isPrivateInst(MI)) {
      expandPrivateLoad(MI);
    } else if (isLocalInst(MI)) {
      expandLocalLoad(MI);
    } else if (isConstantInst(MI)) {
      if (isConstantPoolInst(MI)) {
        expandConstantPoolLoad(MI);
      } else {
        expandConstantLoad(MI);
      }
    } else {
      llvm_unreachable("Found an unsupported load instruction!");
    }
  } else if (isPtrStoreInst(MI)) {
    if (isGlobalInst(MI)) {
      expandGlobalStore(MI);
    } else if (isRegionInst(MI)) {
      expandRegionStore(MI);
    } else if (isPrivateInst(MI)) {
      expandPrivateStore(MI);
    } else if (isLocalInst(MI)) {
      expandLocalStore(MI);
    } else {
      llvm_unreachable("Found an unsupported load instruction!");
    }
  } else {
    llvm_unreachable("Found an unsupported IO instruction!");
  }
}

void AMDILIOExpansionImpl::expandConstantPoolLoad(MachineInstr *MI) {
  if (!isStaticCPLoad(MI)) {
    return expandConstantLoad(MI);
  }

  uint32_t DataReg = MI->getOperand(0).getReg();
  uint32_t Idx = MI->getOperand(1).getIndex();
  const MachineConstantPool *MCP
    = MI->getParent()->getParent()->getConstantPool();
  const std::vector<MachineConstantPoolEntry> &Consts = MCP->getConstants();
  const Constant *C = Consts[Idx].Val.ConstVal;
  emitCPInst(MI, C, 0, isExtendLoad(MI), DataReg);
}

bool AMDILIOExpansionImpl::isAddrCalcInstr(MachineInstr *MI) {
  if (isPrivateInst(MI) && isPtrLoadInst(MI)) {
    // This section of code is a workaround for the problem of
    // globally scoped constant address variables. The problems
    // comes that although they are declared in the constant
    // address space, all variables must be allocated in the
    // private address space. So when there is a load from
    // the global address, it automatically goes into the private
    // address space. However, the data section is placed in the
    // constant address space so we need to check to see if our
    // load base address is a global variable or not. Only if it
    // is not a global variable can we do the address calculation
    // into the private memory ring.

    MachineMemOperand& memOp = (**MI->memoperands_begin());
    const Value *V = memOp.getValue();
    if (V) {
      const GlobalValue *GV = dyn_cast<GlobalVariable>(V);
      return mSTM->usesSoftware(AMDIL::Caps::PrivateMem)
        && !(GV);
    } else {
      return false;
    }
  } else if (isConstantPoolInst(MI) && isPtrLoadInst(MI)) {
    return MI->getOperand(1).isReg();
  } else if (isPrivateInst(MI) && isPtrStoreInst(MI)) {
    return mSTM->usesSoftware(AMDIL::Caps::PrivateMem);
  } else if (isLocalInst(MI) && (isPtrStoreInst(MI) || isPtrLoadInst(MI))) {
    return mSTM->usesSoftware(AMDIL::Caps::LocalMem);
  }
  return false;
}

bool AMDILIOExpansionImpl::isExtendLoad(MachineInstr *MI) {
  return isSExtLoadInst(MI) || isZExtLoadInst(MI) || isAExtLoadInst(MI);
}

bool AMDILIOExpansionImpl::isHardwareRegion(MachineInstr *MI) {
  return (isRegionInst(MI) && (isPtrLoadInst(MI) || isPtrStoreInst(MI)) &&
        mSTM->usesHardware(AMDIL::Caps::RegionMem));
}

bool AMDILIOExpansionImpl::isHardwareLocal(MachineInstr *MI) {
  return (isLocalInst(MI) && (isPtrLoadInst(MI) || isPtrStoreInst(MI)) &&
        mSTM->usesHardware(AMDIL::Caps::LocalMem));
}

bool AMDILIOExpansionImpl::isStaticCPLoad(MachineInstr *MI) {
  if (isConstantPoolInst(MI) && isPtrLoadInst(MI)) {
    for (unsigned I = 0, N = MI->getNumOperands(); I < N; ++I) {
      if (MI->getOperand(I).isCPI()) {
        return true;
      }
    }
  }
  return false;
}

bool AMDILIOExpansionImpl::isHardwareInst(MachineInstr *MI) {
  AMDILAS::InstrResEnc CurInst;
  getAsmPrinterFlags(MI, CurInst);
  return CurInst.bits.HardwareInst;
}

REG_PACKED_TYPE AMDILIOExpansionImpl::getPackedID(MachineInstr *MI) {
  if (isPackV2I8Inst(MI))
    return PACK_V2I8;
  if (isPackV4I8Inst(MI))
    return PACK_V4I8;
  if (isPackV2I16Inst(MI))
    return PACK_V2I16;
  if (isPackV4I16Inst(MI))
    return PACK_V4I16;
  if (isUnpackV2I8Inst(MI))
    return UNPACK_V2I8;
  if (isUnpackV4I8Inst(MI))
    return UNPACK_V4I8;
  if (isUnpackV2I16Inst(MI))
    return UNPACK_V2I16;
  if (isUnpackV4I16Inst(MI))
    return UNPACK_V4I16;
  return NO_PACKING;
}

uint32_t AMDILIOExpansionImpl::getPointerID(MachineInstr *MI) {
  AMDILAS::InstrResEnc curInst;
  getAsmPrinterFlags(MI, curInst);
  return curInst.bits.ResourceID;
}

uint32_t AMDILIOExpansionImpl::getShiftSize(MachineInstr *MI) {
  switch (getPackedID(MI)) {
    default:
      return 0;
    case PACK_V2I8:
    case PACK_V4I8:
    case UNPACK_V2I8:
    case UNPACK_V4I8:
      return 1;
    case PACK_V2I16:
    case PACK_V4I16:
    case UNPACK_V2I16:
    case UNPACK_V4I16:
      return 2;
  }
  return 0;
}

uint32_t AMDILIOExpansionImpl::getMemorySize(MachineInstr *MI) {
  if (MI->memoperands_empty()) {
    return 4;
  }
  return (uint32_t)((*MI->memoperands_begin())->getSize());
}

void AMDILIOExpansionImpl::expandTruncData(MachineInstr *MI, uint32_t &DataReg) {
  if (!isTruncStoreInst(MI)) {
    return;
  }

  DebugLoc DL = MI->getDebugLoc();
  switch (MI->getOpcode()) {
    default:
      MI->dump();
      llvm_unreachable("Found a trunc store instructions we don't handle!");
      break;
    case AMDIL::GLOBALTRUNCSTORE64i64i8r:// case AMDIL::GLOBALTRUNCSTORE64i64i8i:
    case AMDIL::LOCALTRUNCSTORE64i64i8r:// case AMDIL::LOCALTRUNCSTORE64i64i8i:
    case AMDIL::REGIONTRUNCSTORE64i64i8r:// case AMDIL::REGIONTRUNCSTORE64i64i8i:
    case AMDIL::PRIVATETRUNCSTORE64i64i8r:// case AMDIL::PRIVATETRUNCSTORE64i64i8i:
    case AMDIL::GLOBALTRUNCSTOREi64i8r:// case AMDIL::GLOBALTRUNCSTOREi64i8i:
    case AMDIL::LOCALTRUNCSTOREi64i8r:// case AMDIL::LOCALTRUNCSTOREi64i8i:
    case AMDIL::REGIONTRUNCSTOREi64i8r:// case AMDIL::REGIONTRUNCSTOREi64i8i:
    case AMDIL::PRIVATETRUNCSTOREi64i8r:// case AMDIL::PRIVATETRUNCSTOREi64i8i:
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::LLOi64r), AMDIL::Rx1011)
          .addReg(DataReg);
      DataReg = AMDIL::Rx1011;
    case AMDIL::GLOBALTRUNCSTORE64i16i8r:// case AMDIL::GLOBALTRUNCSTORE64i16i8i:
    case AMDIL::LOCALTRUNCSTORE64i16i8r:// case AMDIL::LOCALTRUNCSTORE64i16i8i:
    case AMDIL::REGIONTRUNCSTORE64i16i8r:// case AMDIL::REGIONTRUNCSTORE64i16i8i:
    case AMDIL::PRIVATETRUNCSTORE64i16i8r:// case AMDIL::PRIVATETRUNCSTORE64i16i8i:
    case AMDIL::GLOBALTRUNCSTOREi16i8r:// case AMDIL::GLOBALTRUNCSTOREi16i8i:
    case AMDIL::LOCALTRUNCSTOREi16i8r:// case AMDIL::LOCALTRUNCSTOREi16i8i:
    case AMDIL::REGIONTRUNCSTOREi16i8r:// case AMDIL::REGIONTRUNCSTOREi16i8i:
    case AMDIL::PRIVATETRUNCSTOREi16i8r:// case AMDIL::PRIVATETRUNCSTOREi16i8i:
    case AMDIL::GLOBALTRUNCSTORE64i32i8r:// case AMDIL::GLOBALTRUNCSTORE64i32i8i:
    case AMDIL::LOCALTRUNCSTORE64i32i8r:// case AMDIL::LOCALTRUNCSTORE64i32i8i:
    case AMDIL::REGIONTRUNCSTORE64i32i8r:// case AMDIL::REGIONTRUNCSTORE64i32i8i:
    case AMDIL::PRIVATETRUNCSTORE64i32i8r:// case AMDIL::PRIVATETRUNCSTORE64i32i8i:
    case AMDIL::GLOBALTRUNCSTOREi32i8r:// case AMDIL::GLOBALTRUNCSTOREi32i8i:
    case AMDIL::LOCALTRUNCSTOREi32i8r:// case AMDIL::LOCALTRUNCSTOREi32i8i:
    case AMDIL::REGIONTRUNCSTOREi32i8r:// case AMDIL::REGIONTRUNCSTOREi32i8i:
    case AMDIL::PRIVATETRUNCSTOREi32i8r:// case AMDIL::PRIVATETRUNCSTOREi32i8i:
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1011)
        .addReg(DataReg)
        .addImm(mMFI->addi32Literal(0xFF));
      DataReg = AMDIL::Rx1011;
      break;
    case AMDIL::GLOBALTRUNCSTORE64v2i64i8r:// case AMDIL::GLOBALTRUNCSTORE64v2i64i8i:
    case AMDIL::LOCALTRUNCSTORE64v2i64i8r:// case AMDIL::LOCALTRUNCSTORE64v2i64i8i:
    case AMDIL::REGIONTRUNCSTORE64v2i64i8r:// case AMDIL::REGIONTRUNCSTORE64v2i64i8i:
    case AMDIL::PRIVATETRUNCSTORE64v2i64i8r:// case AMDIL::PRIVATETRUNCSTORE64v2i64i8i:
    case AMDIL::GLOBALTRUNCSTOREv2i64i8r:// case AMDIL::GLOBALTRUNCSTOREv2i64i8i:
    case AMDIL::LOCALTRUNCSTOREv2i64i8r:// case AMDIL::LOCALTRUNCSTOREv2i64i8i:
    case AMDIL::REGIONTRUNCSTOREv2i64i8r:// case AMDIL::REGIONTRUNCSTOREv2i64i8i:
    case AMDIL::PRIVATETRUNCSTOREv2i64i8r:// case AMDIL::PRIVATETRUNCSTOREv2i64i8i:
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::LLOv2i64r), AMDIL::Rxy1011)
          .addReg(DataReg);
      DataReg = AMDIL::Rxy1011;
    case AMDIL::GLOBALTRUNCSTORE64v2i16i8r:// case AMDIL::GLOBALTRUNCSTORE64v2i16i8i:
    case AMDIL::LOCALTRUNCSTORE64v2i16i8r:// case AMDIL::LOCALTRUNCSTORE64v2i16i8i:
    case AMDIL::REGIONTRUNCSTORE64v2i16i8r:// case AMDIL::REGIONTRUNCSTORE64v2i16i8i:
    case AMDIL::PRIVATETRUNCSTORE64v2i16i8r:// case AMDIL::PRIVATETRUNCSTORE64v2i16i8i:
    case AMDIL::GLOBALTRUNCSTOREv2i16i8r:// case AMDIL::GLOBALTRUNCSTOREv2i16i8i:
    case AMDIL::LOCALTRUNCSTOREv2i16i8r:// case AMDIL::LOCALTRUNCSTOREv2i16i8i:
    case AMDIL::REGIONTRUNCSTOREv2i16i8r:// case AMDIL::REGIONTRUNCSTOREv2i16i8i:
    case AMDIL::PRIVATETRUNCSTOREv2i16i8r:// case AMDIL::PRIVATETRUNCSTOREv2i16i8i:
    case AMDIL::GLOBALTRUNCSTORE64v2i32i8r:// case AMDIL::GLOBALTRUNCSTORE64v2i32i8i:
    case AMDIL::LOCALTRUNCSTORE64v2i32i8r:// case AMDIL::LOCALTRUNCSTORE64v2i32i8i:
    case AMDIL::REGIONTRUNCSTORE64v2i32i8r:// case AMDIL::REGIONTRUNCSTORE64v2i32i8i:
    case AMDIL::PRIVATETRUNCSTORE64v2i32i8r:// case AMDIL::PRIVATETRUNCSTORE64v2i32i8i:
    case AMDIL::GLOBALTRUNCSTOREv2i32i8r:// case AMDIL::GLOBALTRUNCSTOREv2i32i8i:
    case AMDIL::LOCALTRUNCSTOREv2i32i8r:// case AMDIL::LOCALTRUNCSTOREv2i32i8i:
    case AMDIL::REGIONTRUNCSTOREv2i32i8r:// case AMDIL::REGIONTRUNCSTOREv2i32i8i:
    case AMDIL::PRIVATETRUNCSTOREv2i32i8r:// case AMDIL::PRIVATETRUNCSTOREv2i32i8i:
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::ANDv2i32rr), AMDIL::Rxy1011)
        .addReg(DataReg)
        .addImm(mMFI->addi32Literal(0xFF));
      DataReg = AMDIL::Rxy1011;
      break;
    case AMDIL::GLOBALTRUNCSTORE64v4i16i8r:// case AMDIL::GLOBALTRUNCSTORE64v4i16i8i:
    case AMDIL::LOCALTRUNCSTORE64v4i16i8r:// case AMDIL::LOCALTRUNCSTORE64v4i16i8i:
    case AMDIL::REGIONTRUNCSTORE64v4i16i8r:// case AMDIL::REGIONTRUNCSTORE64v4i16i8i:
    case AMDIL::PRIVATETRUNCSTORE64v4i16i8r:// case AMDIL::PRIVATETRUNCSTORE64v4i16i8i:
    case AMDIL::GLOBALTRUNCSTOREv4i16i8r:// case AMDIL::GLOBALTRUNCSTOREv4i16i8i:
    case AMDIL::LOCALTRUNCSTOREv4i16i8r:// case AMDIL::LOCALTRUNCSTOREv4i16i8i:
    case AMDIL::REGIONTRUNCSTOREv4i16i8r:// case AMDIL::REGIONTRUNCSTOREv4i16i8i:
    case AMDIL::PRIVATETRUNCSTOREv4i16i8r:// case AMDIL::PRIVATETRUNCSTOREv4i16i8i:
    case AMDIL::GLOBALTRUNCSTORE64v4i32i8r:// case AMDIL::GLOBALTRUNCSTORE64v4i32i8i:
    case AMDIL::LOCALTRUNCSTORE64v4i32i8r:// case AMDIL::LOCALTRUNCSTORE64v4i32i8i:
    case AMDIL::REGIONTRUNCSTORE64v4i32i8r:// case AMDIL::REGIONTRUNCSTORE64v4i32i8i:
    case AMDIL::PRIVATETRUNCSTORE64v4i32i8r:// case AMDIL::PRIVATETRUNCSTORE64v4i32i8i:
    case AMDIL::GLOBALTRUNCSTOREv4i32i8r:// case AMDIL::GLOBALTRUNCSTOREv4i32i8i:
    case AMDIL::LOCALTRUNCSTOREv4i32i8r:// case AMDIL::LOCALTRUNCSTOREv4i32i8i:
    case AMDIL::REGIONTRUNCSTOREv4i32i8r:// case AMDIL::REGIONTRUNCSTOREv4i32i8i:
    case AMDIL::PRIVATETRUNCSTOREv4i32i8r:// case AMDIL::PRIVATETRUNCSTOREv4i32i8i:
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::ANDv4i32rr), AMDIL::R1011)
        .addReg(DataReg)
        .addImm(mMFI->addi32Literal(0xFF));
      DataReg = AMDIL::R1011;
      break;
    case AMDIL::GLOBALTRUNCSTORE64i64i16r:// case AMDIL::GLOBALTRUNCSTORE64i64i16i:
    case AMDIL::LOCALTRUNCSTORE64i64i16r:// case AMDIL::LOCALTRUNCSTORE64i64i16i:
    case AMDIL::REGIONTRUNCSTORE64i64i16r:// case AMDIL::REGIONTRUNCSTORE64i64i16i:
    case AMDIL::PRIVATETRUNCSTORE64i64i16r:// case AMDIL::PRIVATETRUNCSTORE64i64i16i:
    case AMDIL::GLOBALTRUNCSTOREi64i16r:// case AMDIL::GLOBALTRUNCSTOREi64i16i:
    case AMDIL::LOCALTRUNCSTOREi64i16r:// case AMDIL::LOCALTRUNCSTOREi64i16i:
    case AMDIL::REGIONTRUNCSTOREi64i16r:// case AMDIL::REGIONTRUNCSTOREi64i16i:
    case AMDIL::PRIVATETRUNCSTOREi64i16r:// case AMDIL::PRIVATETRUNCSTOREi64i16i:
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::LLOv2i64r), AMDIL::Rxy1011)
          .addReg(DataReg);
      DataReg = AMDIL::Rxy1011;
    case AMDIL::GLOBALTRUNCSTORE64i32i16r:// case AMDIL::GLOBALTRUNCSTORE64i32i16i:
    case AMDIL::LOCALTRUNCSTORE64i32i16r:// case AMDIL::LOCALTRUNCSTORE64i32i16i:
    case AMDIL::REGIONTRUNCSTORE64i32i16r:// case AMDIL::REGIONTRUNCSTORE64i32i16i:
    case AMDIL::PRIVATETRUNCSTORE64i32i16r:// case AMDIL::PRIVATETRUNCSTORE64i32i16i:
    case AMDIL::GLOBALTRUNCSTOREi32i16r:// case AMDIL::GLOBALTRUNCSTOREi32i16i:
    case AMDIL::LOCALTRUNCSTOREi32i16r:// case AMDIL::LOCALTRUNCSTOREi32i16i:
    case AMDIL::REGIONTRUNCSTOREi32i16r:// case AMDIL::REGIONTRUNCSTOREi32i16i:
    case AMDIL::PRIVATETRUNCSTOREi32i16r:// case AMDIL::PRIVATETRUNCSTOREi32i16i:
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1011)
        .addReg(DataReg)
        .addImm(mMFI->addi32Literal(0xFFFF));
      DataReg = AMDIL::Rx1011;
      break;
    case AMDIL::GLOBALTRUNCSTORE64v2i64i16r:// case AMDIL::GLOBALTRUNCSTORE64v2i64i16i:
    case AMDIL::LOCALTRUNCSTORE64v2i64i16r:// case AMDIL::LOCALTRUNCSTORE64v2i64i16i:
    case AMDIL::REGIONTRUNCSTORE64v2i64i16r:// case AMDIL::REGIONTRUNCSTORE64v2i64i16i:
    case AMDIL::PRIVATETRUNCSTORE64v2i64i16r:// case AMDIL::PRIVATETRUNCSTORE64v2i64i16i:
    case AMDIL::GLOBALTRUNCSTOREv2i64i16r:// case AMDIL::GLOBALTRUNCSTOREv2i64i16i:
    case AMDIL::LOCALTRUNCSTOREv2i64i16r:// case AMDIL::LOCALTRUNCSTOREv2i64i16i:
    case AMDIL::REGIONTRUNCSTOREv2i64i16r:// case AMDIL::REGIONTRUNCSTOREv2i64i16i:
    case AMDIL::PRIVATETRUNCSTOREv2i64i16r:// case AMDIL::PRIVATETRUNCSTOREv2i64i16i:
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::LLOv2i64r), AMDIL::Rxy1011)
          .addReg(DataReg);
      DataReg = AMDIL::Rxy1011;
    case AMDIL::GLOBALTRUNCSTORE64v2i32i16r:// case AMDIL::GLOBALTRUNCSTORE64v2i32i16i:
    case AMDIL::LOCALTRUNCSTORE64v2i32i16r:// case AMDIL::LOCALTRUNCSTORE64v2i32i16i:
    case AMDIL::REGIONTRUNCSTORE64v2i32i16r:// case AMDIL::REGIONTRUNCSTORE64v2i32i16i:
    case AMDIL::PRIVATETRUNCSTORE64v2i32i16r:// case AMDIL::PRIVATETRUNCSTORE64v2i32i16i:
    case AMDIL::GLOBALTRUNCSTOREv2i32i16r:// case AMDIL::GLOBALTRUNCSTOREv2i32i16i:
    case AMDIL::LOCALTRUNCSTOREv2i32i16r:// case AMDIL::LOCALTRUNCSTOREv2i32i16i:
    case AMDIL::REGIONTRUNCSTOREv2i32i16r:// case AMDIL::REGIONTRUNCSTOREv2i32i16i:
    case AMDIL::PRIVATETRUNCSTOREv2i32i16r:// case AMDIL::PRIVATETRUNCSTOREv2i32i16i:
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::ANDv2i32rr), AMDIL::Rxy1011)
        .addReg(DataReg)
        .addImm(mMFI->addi32Literal(0xFFFF));
      DataReg = AMDIL::Rxy1011;
      break;
    case AMDIL::GLOBALTRUNCSTORE64v4i32i16r:// case AMDIL::GLOBALTRUNCSTORE64v4i32i16i:
    case AMDIL::LOCALTRUNCSTORE64v4i32i16r:// case AMDIL::LOCALTRUNCSTORE64v4i32i16i:
    case AMDIL::REGIONTRUNCSTORE64v4i32i16r:// case AMDIL::REGIONTRUNCSTORE64v4i32i16i:
    case AMDIL::PRIVATETRUNCSTORE64v4i32i16r:// case AMDIL::PRIVATETRUNCSTORE64v4i32i16i:
    case AMDIL::GLOBALTRUNCSTOREv4i32i16r:// case AMDIL::GLOBALTRUNCSTOREv4i32i16i:
    case AMDIL::LOCALTRUNCSTOREv4i32i16r:// case AMDIL::LOCALTRUNCSTOREv4i32i16i:
    case AMDIL::REGIONTRUNCSTOREv4i32i16r:// case AMDIL::REGIONTRUNCSTOREv4i32i16i:
    case AMDIL::PRIVATETRUNCSTOREv4i32i16r:// case AMDIL::PRIVATETRUNCSTOREv4i32i16i:
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::ANDv4i32rr), AMDIL::R1011)
        .addReg(DataReg)
        .addImm(mMFI->addi32Literal(0xFFFF));
      DataReg = AMDIL::R1011;
      break;
    case AMDIL::GLOBALTRUNCSTORE64i64i32r:// case AMDIL::GLOBALTRUNCSTORE64i64i32i:
    case AMDIL::LOCALTRUNCSTORE64i64i32r:// case AMDIL::LOCALTRUNCSTORE64i64i32i:
    case AMDIL::REGIONTRUNCSTORE64i64i32r:// case AMDIL::REGIONTRUNCSTORE64i64i32i:
    case AMDIL::PRIVATETRUNCSTORE64i64i32r:// case AMDIL::PRIVATETRUNCSTORE64i64i32i:
    case AMDIL::GLOBALTRUNCSTOREi64i32r:// case AMDIL::GLOBALTRUNCSTOREi64i32i:
    case AMDIL::LOCALTRUNCSTOREi64i32r:// case AMDIL::LOCALTRUNCSTOREi64i32i:
    case AMDIL::REGIONTRUNCSTOREi64i32r:// case AMDIL::REGIONTRUNCSTOREi64i32i:
    case AMDIL::PRIVATETRUNCSTOREi64i32r:// case AMDIL::PRIVATETRUNCSTOREi64i32i:
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::LLOi64r), AMDIL::Rx1011)
          .addReg(DataReg);
      DataReg = AMDIL::Rx1011;
      break;
    case AMDIL::GLOBALTRUNCSTORE64v2i64i32r:// case AMDIL::GLOBALTRUNCSTORE64v2i64i32i:
    case AMDIL::LOCALTRUNCSTORE64v2i64i32r:// case AMDIL::LOCALTRUNCSTORE64v2i64i32i:
    case AMDIL::REGIONTRUNCSTORE64v2i64i32r:// case AMDIL::REGIONTRUNCSTORE64v2i64i32i:
    case AMDIL::PRIVATETRUNCSTORE64v2i64i32r:// case AMDIL::PRIVATETRUNCSTORE64v2i64i32i:
    case AMDIL::GLOBALTRUNCSTOREv2i64i32r:// case AMDIL::GLOBALTRUNCSTOREv2i64i32i:
    case AMDIL::LOCALTRUNCSTOREv2i64i32r:// case AMDIL::LOCALTRUNCSTOREv2i64i32i:
    case AMDIL::REGIONTRUNCSTOREv2i64i32r:// case AMDIL::REGIONTRUNCSTOREv2i64i32i:
    case AMDIL::PRIVATETRUNCSTOREv2i64i32r:// case AMDIL::PRIVATETRUNCSTOREv2i64i32i:
      BuildMI(*mBB, MI, DL,
          mTII->get(AMDIL::LLOv2i64r), AMDIL::Rxy1011)
          .addReg(DataReg);
      DataReg = AMDIL::Rxy1011;
      break;
    case AMDIL::GLOBALTRUNCSTORE64f64f32r:// case AMDIL::GLOBALTRUNCSTORE64f64f32i:
    case AMDIL::LOCALTRUNCSTORE64f64f32r:// case AMDIL::LOCALTRUNCSTORE64f64f32i:
    case AMDIL::REGIONTRUNCSTORE64f64f32r:// case AMDIL::REGIONTRUNCSTORE64f64f32i:
    case AMDIL::PRIVATETRUNCSTORE64f64f32r:// case AMDIL::PRIVATETRUNCSTORE64f64f32i:
    case AMDIL::GLOBALTRUNCSTOREf64f32r:// case AMDIL::GLOBALTRUNCSTOREf64f32i:
    case AMDIL::LOCALTRUNCSTOREf64f32r:// case AMDIL::LOCALTRUNCSTOREf64f32i:
    case AMDIL::REGIONTRUNCSTOREf64f32r:// case AMDIL::REGIONTRUNCSTOREf64f32i:
    case AMDIL::PRIVATETRUNCSTOREf64f32r:// case AMDIL::PRIVATETRUNCSTOREf64f32i:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::DTOFr),
          AMDIL::Rx1011).addReg(DataReg);
      DataReg = AMDIL::Rx1011;
      break;
    case AMDIL::GLOBALTRUNCSTORE64v2f64f32r:// case AMDIL::GLOBALTRUNCSTORE64v2f64f32i:
    case AMDIL::LOCALTRUNCSTORE64v2f64f32r:// case AMDIL::LOCALTRUNCSTORE64v2f64f32i:
    case AMDIL::REGIONTRUNCSTORE64v2f64f32r:// case AMDIL::REGIONTRUNCSTORE64v2f64f32i:
    case AMDIL::PRIVATETRUNCSTORE64v2f64f32r:// case AMDIL::PRIVATETRUNCSTORE64v2f64f32i:
    case AMDIL::GLOBALTRUNCSTOREv2f64f32r:// case AMDIL::GLOBALTRUNCSTOREv2f64f32i:
    case AMDIL::LOCALTRUNCSTOREv2f64f32r:// case AMDIL::LOCALTRUNCSTOREv2f64f32i:
    case AMDIL::REGIONTRUNCSTOREv2f64f32r:// case AMDIL::REGIONTRUNCSTOREv2f64f32i:
    case AMDIL::PRIVATETRUNCSTOREv2f64f32r:// case AMDIL::PRIVATETRUNCSTOREv2f64f32i:
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::DTOFr),
          AMDIL::Rx1011).addReg(getCompReg(DataReg, AMDIL::sub_xy));
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::DTOFr),
          AMDIL::Ry1011).addReg(getCompReg(DataReg, AMDIL::sub_zw));
      DataReg = AMDIL::Rxy1011;
      break;
  }
}

uint32_t AMDILIOExpansionImpl::getPackedReg(uint32_t &DataReg, uint32_t ID) {
  switch (ID) {
    default:
      return DataReg;
    case UNPACK_V2I8:
    case UNPACK_V2I16:
    case UNPACK_V4I8:
      return get1stI32SubReg(DataReg);
    case UNPACK_V4I16:
      return get1stI64SubReg(DataReg);
  }
}

void AMDILIOExpansionImpl::expandAddressCalc(MachineInstr *MI,
                                             uint32_t &AddyReg) {
  if (!isAddrCalcInstr(MI)) {
    return;
  }
  DebugLoc DL = MI->getDebugLoc();
  bool Is64Bit = is64bitLSOp(MI);
  uint32_t NewReg = Is64Bit ? AMDIL::Rxy1010 : AMDIL::Rx1010;
  uint32_t AddInst = Is64Bit ? AMDIL::ADDi64rr : AMDIL::ADDi32rr;
  if (isPrivateInst(MI)
      && (isPtrLoadInst(MI)
          || (isPtrStoreInst(MI)
              && mSTM->usesSoftware(AMDIL::Caps::PrivateMem)))) {
      BuildMI(*mBB, MI, DL, mTII->get(AddInst),
          NewReg).addReg(AddyReg).addReg(AMDIL::T1);
      AddyReg = NewReg;
  } else if (isLocalInst(MI) && (isPtrStoreInst(MI) || isPtrLoadInst(MI))) {
    BuildMI(*mBB, MI, DL, mTII->get(AddInst),
            NewReg).addReg(AddyReg).addReg(AMDIL::T2);
    AddyReg = NewReg;
  } else if (isConstantPoolInst(MI)
             && isPtrLoadInst(MI)
             && MI->getOperand(1).isReg()) {
    BuildMI(*mBB, MI, DL, mTII->get(AddInst),
            NewReg).addReg(AddyReg).addReg(AMDIL::SDP);
    AddyReg = NewReg;
  }
}

void AMDILIOExpansionImpl::expandLoadStartCode(MachineInstr *MI,
                                               uint32_t &AddyReg) {
  expandAddressCalc(MI, AddyReg);
}

void AMDILIOExpansionImpl::emitStaticCPLoad(MachineInstr* MI,
                                            int Swizzle,
                                            int ID,
                                            bool ExtFPLoad,
                                            uint32_t &DataReg) {
  DebugLoc DL = MI->getDebugLoc();

  switch (Swizzle) {
    default:
      BuildMI(*mBB, MI, DL, mTII->get(ExtFPLoad ? AMDIL::DTOFr : AMDIL::COPY),
              DataReg)
        .addImm(ID);
      break;
    case 1:
    case 2:
    case 3:
      BuildMI(*mBB, MI, DL,
              mTII->get(ExtFPLoad ? AMDIL::DTOFr : AMDIL::COPY),
              AMDIL::Rx1001)
        .addImm(ID);
      BuildMI(*mBB, MI, DL, mTII->get(AMDIL::VINSERTv4i32rr),
          DataReg)
        .addReg(DataReg)
        .addReg(AMDIL::Rx1001)
        .addImm(Swizzle + 1);
      break;
  }
}

void AMDILIOExpansionImpl::emitCPInst(MachineInstr* MI,
                                      const Constant* C,
                                      int Swizzle,
                                      bool ExtFPLoad,
                                      uint32_t &DataReg) {
  if (const ConstantFP* CFP = dyn_cast<ConstantFP>(C)) {
    if (CFP->getType()->isFloatTy()) {
      uint32_t Val = (uint32_t)(CFP->getValueAPF().bitcastToAPInt()
          .getZExtValue());
      uint32_t ID = mMFI->addi32Literal(Val);
      if (ID == 0) {
        const APFloat &APF = CFP->getValueAPF();
        union dtol_union {
          double d;
          uint64_t ul;
        } Conv;
        if (&APF.getSemantics()
            == (const llvm::fltSemantics*)&APFloat::IEEEsingle) {
          float FVal = APF.convertToFloat();
          Conv.d = (double)FVal;
        } else {
          Conv.d = APF.convertToDouble();
        }
        ID = mMFI->addi64Literal(Conv.ul);
      }
      emitStaticCPLoad(MI, Swizzle, ID, ExtFPLoad, DataReg);
    } else {
      const APFloat &APF = CFP->getValueAPF();
      union ftol_union {
        double d;
        uint64_t ul;
      } Conv;
      if (&APF.getSemantics()
          == (const llvm::fltSemantics*)&APFloat::IEEEsingle) {
        float FVal = APF.convertToFloat();
        Conv.d = (double)FVal;
      } else {
        Conv.d = APF.convertToDouble();
      }
      uint32_t ID = mMFI->getLitIdx(Conv.ul);
      if (ID == 0) {
        ID = mMFI->getLitIdx((uint32_t)Conv.ul);
      }
      emitStaticCPLoad(MI, Swizzle, ID, ExtFPLoad, DataReg);
    }
  } else if (const ConstantInt* CI = dyn_cast<ConstantInt>(C)) {
    int64_t Val = CI ? CI->getSExtValue() : 0;
    if (CI->getBitWidth() == 64) {
      emitStaticCPLoad(MI, Swizzle, mMFI->addi64Literal(Val), ExtFPLoad, DataReg);
    } else {
      emitStaticCPLoad(MI, Swizzle, mMFI->addi32Literal(Val), ExtFPLoad, DataReg);
    }
  } else if (const ConstantArray* CA = dyn_cast<ConstantArray>(C)) {
    uint32_t Size = CA->getNumOperands();
    assert(Size < 5 && "Cannot handle a constant array where size > 4");
    if (Size > 4) {
      Size = 4;
    }
    for (uint32_t I = 0; I < Size; ++I) {
      emitCPInst(MI, CA->getOperand(0), I, ExtFPLoad, DataReg);
    }
  } else if (const ConstantAggregateZero* CAZ
             = dyn_cast<ConstantAggregateZero>(C)) {
    if (CAZ->isNullValue()) {
      emitStaticCPLoad(MI, Swizzle, mMFI->addi32Literal(0), ExtFPLoad, DataReg);
    }
  } else if (const ConstantStruct* CS = dyn_cast<ConstantStruct>(C)) {
    uint32_t Size = CS->getNumOperands();
    assert(Size < 5 && "Cannot handle a constant array where size > 4");
    if (Size > 4) {
      Size = 4;
    }
    for (uint32_t I = 0; I < Size; ++I) {
      emitCPInst(MI, CS->getOperand(0), I, ExtFPLoad, DataReg);
    }
  } else if (const ConstantVector* CV = dyn_cast<ConstantVector>(C)) {
    // TODO: Make this handle vectors natively up to the correct
    // size
    uint32_t Size = CV->getNumOperands();
    assert(Size < 5 && "Cannot handle a constant array where size > 4");
    if (Size > 4) {
      Size = 4;
    }
    for (uint32_t I = 0; I < Size; ++I) {
      emitCPInst(MI, CV->getOperand(0), I, ExtFPLoad, DataReg);
    }
  } else if (const ConstantDataVector* CV = dyn_cast<ConstantDataVector>(C)) {
    // TODO: Make this handle vectors natively up to the correct
    // size
    uint32_t Size = CV->getNumElements();
    assert(Size < 5 && "Cannot handle a constant array where size > 4");
    if (Size > 4) {
      Size = 4;
    }
    for (uint32_t I = 0; I < Size; ++I) {
      emitCPInst(MI, CV->getElementAsConstant(0), I, ExtFPLoad, DataReg);
    }
  } else {
    // TODO: Do we really need to handle ConstantPointerNull?
    // What about BlockAddress, ConstantExpr and Undef?
    // How would these even be generated by a valid CL program?
    llvm_unreachable("Found a constant type that I don't know how to handle");
  }
}

// get the first of the listed sub-registers
uint32_t AMDILIOExpansionImpl::getCompReg(uint32_t Reg,
                                          uint32_t SubIdx0,
                                          uint32_t SubIdx1) {
  uint32_t SubReg = 0;
  if (SubIdx0) {
    SubReg = mTRI->getSubReg(Reg, SubIdx0);
    if (SubReg)
      return SubReg;
  }

  if (SubIdx1) {
    SubReg = mTRI->getSubReg(Reg, SubIdx1);
    if (SubReg)
      return SubReg;
  }

  llvm_unreachable("Found a case where the register does not have either sub-index!");

  // Just incase we hit this assert, lets as least use a valid register so
  // we don't have possible crashes in release mode.
  return Reg;
}

// Return the 1st I32 component of the register.
// If the given register is a I32 register, return itself.
// If the given register is a register larger than i32, then return
// the first I32 sub-register of the given registern.
uint32_t AMDILIOExpansionImpl::get1stI32SubReg(uint32_t Reg) {
  if (mTRI->getRegClass(AMDIL::GPR_32RegClassID)->contains(Reg))
    return Reg;
  return getCompReg(Reg, AMDIL::sub_x, AMDIL::sub_z);
}

// Return the 1st I64 component of the register.
// If the given register is a I64 register, return itself.
// If the given register is a register larger than i64, then return
// the first I64 sub-register of the given registern.
uint32_t AMDILIOExpansionImpl::get1stI64SubReg(uint32_t Reg) {
  if (mTRI->getRegClass(AMDIL::GPR_64RegClassID)->contains(Reg))
    return Reg;
  return getCompReg(Reg, AMDIL::sub_xy, AMDIL::sub_zw);
}

bool AMDILIOExpansionImpl::is32BitRegister(uint32_t Reg) {
  return mTRI->getRegClass(AMDIL::GPR_32RegClassID)->contains(Reg);
}

bool AMDILIOExpansionImpl::is64BitRegister(uint32_t Reg) {
  return mTRI->getRegClass(AMDIL::GPR_64RegClassID)->contains(Reg);
}

bool AMDILIOExpansionImpl::is128BitRegister(uint32_t Reg) {
  return mTRI->getRegClass(AMDIL::GPRV2I64RegClassID)->contains(Reg);
}

void AMDILIOExpansionImpl::expandImageLoad(MachineBasicBlock *MBB,
                                           MachineInstr *MI) {
  uint32_t ImageID = getPointerID(MI);
  MI->getOperand(1).ChangeToImmediate(ImageID);
  saveInst = true;
}

void AMDILIOExpansionImpl::expandImageStore(MachineBasicBlock *MBB,
                                            MachineInstr *MI) {
  uint32_t ImageID = getPointerID(MI);
  MI->getOperand(0).ChangeToImmediate(ImageID);
  saveInst = true;
}

void AMDILIOExpansionImpl::expandImageParam(MachineBasicBlock *MBB,
                                            MachineInstr *MI) {
  uint32_t ID = getPointerID(MI);
  DebugLoc DL = MI->getDebugLoc();
  BuildMI(*MBB, MI, DL, mTII->get(AMDIL::CB32LOAD), MI->getOperand(0).getReg())
    .addImm(ID)
    .addImm(1);
}

// This function does address calculations modifications to load from a vector
// register type instead of a dword addressed load.
  void
AMDILIOExpansionImpl::emitVectorAddressCalc(MachineInstr *MI,
                                            bool is32bit,
                                            bool needsSelect,
                                            uint32_t &addyReg)
{
  DebugLoc DL = MI->getDebugLoc();
  // This produces the following pseudo-IL:
  // ishr r1007.x___, r1010.xxxx, (is32bit) ? 2 : 3
  // iand r1008.x___, r1007.xxxx, (is32bit) ? 3 : 1
  // ishr r1007.x___, r1007.xxxx, (is32bit) ? 2 : 1
  BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHRi32i32rr), AMDIL::Rx1007)
    .addReg(addyReg)
    .addImm(mMFI->addi32Literal((is32bit) ? 0x2 : 3));
  BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1008)
    .addReg(AMDIL::Rx1007)
    .addImm(mMFI->addi32Literal((is32bit) ? 3 : 1));
  BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHRi32i32rr), AMDIL::Rx1007)
    .addReg(AMDIL::Rx1007)
    .addImm(mMFI->addi32Literal((is32bit) ? 2 : 1));
  if (needsSelect) {
    // If the component selection is required, the following
    // pseudo-IL is produced.
    // iadd r1008, r1008.x, (is32bit) ? {0, -1, -2, -3} : {0, 0, -1, -1}
    // ieq r1008, r1008, 0
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ADDv4i32rr), AMDIL::R1008)
      .addReg(AMDIL::Rx1008)
      .addImm(mMFI->addi128Literal((is32bit) ? 0xFFFFFFFFULL << 32 : 0ULL,
            (is32bit) ? 0xFFFFFFFEULL | (0xFFFFFFFDULL << 32) :
            -1ULL));
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::IEQv4i32rr), AMDIL::R1008)
      .addReg(AMDIL::R1008)
      .addImm(mMFI->addi32Literal(0));
  }
  addyReg = AMDIL::Rx1007;
}

// We have a 128 bit load but a 8/16/32bit value, so we need to
// select the correct component and make sure that the correct
// bits are selected. For the 8 and 16 bit cases we need to
// extract from the component the correct bits and for 32 bits
// we just need to select the correct component.
void AMDILIOExpansionImpl::emitDataLoadSelect(MachineInstr *MI,
                                              uint32_t DstReg,
                                              uint32_t AddyReg) {
  DebugLoc DL = MI->getDebugLoc();
  assert(mTRI->getRegClass(AMDIL::GPR_32RegClassID)->contains(DstReg) &&
         "dst register not i32 register");
  if (getMemorySize(MI) == 1) {
    emitComponentExtract(MI, AMDIL::R1011, AMDIL::Rx1011, false);
    // This produces the following pseudo-IL:
    // iand r1006.x___, addyReg.xxxx, l14.xxxx
    // iadd r1006, r1006.x, {0, -1, 2, 3}
    // ieq r1008, r1006, 0
    // ishr r1011, r1011.x, {0, 8, 16, 24}
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1006)
      .addReg(AddyReg)
      .addImm(mMFI->addi32Literal(3));
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ADDv4i32rr), AMDIL::R1006)
      .addReg(AMDIL::Rx1006)
      .addImm(mMFI->addi128Literal(0xFFFFFFFFULL << 32,
                                   (0xFFFFFFFEULL | (0xFFFFFFFDULL << 32))));
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::IEQv4i32rr), AMDIL::R1008)
      .addReg(AMDIL::R1006)
      .addImm(mMFI->addi32Literal(0));
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHRv4i32i32rr), AMDIL::R1011)
      .addReg(AMDIL::Rx1011)
      .addImm(mMFI->addi128Literal(8ULL << 32, 16ULL | (24ULL << 32)));
    emitComponentExtract(MI, AMDIL::R1011, DstReg, false);
  } else if (getMemorySize(MI) == 2) {
    emitComponentExtract(MI, AMDIL::R1011, AMDIL::Rx1011, false);
    // This produces the following pseudo-IL:
    // ishr r1007.x___, AddyReg.xxxx, 1
    // iand r1008.x___, r1007.xxxx, 1
    // ishr r1007.x___, r1011.xxxx, 16
    // cmov_logical r1011.x___, r1008.xxxx, r1007.xxxx, r1011.xxxx
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHRi32i32rr), AMDIL::Rx1007)
      .addReg(AddyReg)
      .addImm(mMFI->addi32Literal(1));
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ANDi32rr), AMDIL::Rx1008)
      .addReg(AMDIL::Rx1007)
      .addImm(mMFI->addi32Literal(1));
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SHRi32i32rr), AMDIL::Rx1007)
      .addReg(AMDIL::Rx1011)
      .addImm(mMFI->addi32Literal(16));
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), DstReg)
      .addReg(AMDIL::Rx1008)
      .addReg(AMDIL::Rx1007)
      .addReg(AMDIL::Rx1011);
  } else {
    emitComponentExtract(MI, AMDIL::R1011, DstReg, false);
  }
}

// This code produces the following pseudo-IL:
// cmov_logical $src.x___, r1008.y, $src.y, $src.x
// cmov_logical $src.x___, r1008.z, $src.x, $src.z
// cmov_logical $dst.x___, r1008.w, $src.x, $src.w
void AMDILIOExpansionImpl::emitComponentExtract(MachineInstr *MI,
                                                unsigned Src,
                                                unsigned Dst,
                                                bool Before) {
  DebugLoc DL = MI->getDebugLoc();
  BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr),
      getCompReg(Src, AMDIL::sub_x))
    .addReg(AMDIL::Ry1008)
    .addReg(getCompReg(Src, AMDIL::sub_y))
    .addReg(getCompReg(Src, AMDIL::sub_x));
  BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr),
      getCompReg(Src, AMDIL::sub_x))
    .addReg(AMDIL::Rz1008)
    .addReg(getCompReg(Src, AMDIL::sub_z))
    .addReg(getCompReg(Src, AMDIL::sub_x));
  BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CMOV_LOGICALi32rrr), Dst)
    .addReg(AMDIL::Rw1008)
    .addReg(getCompReg(Src, AMDIL::sub_w))
    .addReg(getCompReg(Src, AMDIL::sub_x));
}

// This function emits a switch statement and writes 32bit/64bit
// value to a 128bit vector register type.
void AMDILIOExpansionImpl::emitVectorSwitchWrite(MachineInstr *MI,
                                                 bool Is32Bit,
                                                 uint32_t &AddyReg,
                                                 uint32_t &DataReg) {
  uint32_t xID = getPointerID(MI);
  assert(xID &&
      "Found a scratch store that was incorrectly marked as zero ID!\n");
  // This section generates the following pseudo-IL:
  // switch r1008.x
  // default
  //   mov x1[$AddyReg.x].(Is32Bit) ? x___ : xy__, r1011.x{y}
  // break
  // case 1
  //   mov x1[$AddyReg.x].(Is32Bit) ? _y__ : __zw, r1011.x{yxy}
  // break
  // if Is32Bit is true, case 2 and 3 are emitted.
  // case 2
  //   mov x1[$AddyReg.x].__z_, r1011.x
  // break
  // case 3
  //   mov x1[$AddyReg.x].___w, r1011.x
  // break
  // endswitch
  DebugLoc DL = MI->getDebugLoc();
  BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SWITCH))
    .addReg(AMDIL::Rx1008);
  BuildMI(*mBB, MI, DL, mTII->get(AMDIL::DEFAULT));
  BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SCRATCH32STORE),
          (Is32Bit ? AddyReg
                   : (AddyReg - AMDIL::Rx1) + AMDIL::Rxy1))
    .addReg(DataReg).addImm(xID);
  BuildMI(*mBB, MI, DL, mTII->get(AMDIL::BREAK));
  BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CASE)).addImm(1);
  BuildMI(*mBB, MI, DL,
      mTII->get(AMDIL::SCRATCH32STORE),
          (Is32Bit ? (AddyReg - AMDIL::Rx1) + AMDIL::Ry1
                   : (AddyReg - AMDIL::Rx1) + AMDIL::Rzw1))
    .addReg(DataReg).addImm(xID);
  BuildMI(*mBB, MI, DL, mTII->get(AMDIL::BREAK));
  if (Is32Bit) {
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CASE)).addImm(2);
    BuildMI(*mBB, MI, DL,
        mTII->get(AMDIL::SCRATCH32STORE),
        (AddyReg - AMDIL::Rx1) + AMDIL::Rz1)
      .addReg(DataReg).addImm(xID);
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::BREAK));
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::CASE)).addImm(3);
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::SCRATCH32STORE),
        (AddyReg - AMDIL::Rx1) + AMDIL::Rw1)
      .addReg(DataReg).addImm(xID);
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::BREAK));
  }
  BuildMI(*mBB, MI, DL, mTII->get(AMDIL::ENDSWITCH));
}

void AMDILIOExpansionImpl::expandStoreSetupCode(MachineInstr *MI,
                                                uint32_t &AddyReg,
                                                uint32_t &DataReg) {
  if (MI->getOperand(0).isUndef()) {
    DEBUG_WITH_TYPE("udef_type", dbgs()
      <<"<undef> type: "<< mTRI->getName(DataReg)
      << " set as undefined\n");
  }
  expandTruncData(MI, DataReg);
  expandAddressCalc(MI, AddyReg);
  expandPackedData(MI, DataReg, DataReg);
}

