//===-------- AMDILSwizzleEncoder.cpp - Encode the swizzle information ----===//
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
// The implementation of the AMDIL Swizzle Encoder. The swizzle encoder goes
// through all instructions in a machine function and all operands and
// encodes swizzle information in the operands. The AsmParser can then
// use the swizzle information to print out the swizzles correctly.
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "swizzleencoder"

#include "AMDILSwizzleEncoder.h"
#include "AMDILAlgorithms.tpp"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILUtilityFunctions.h"
#include "AMDILRegisterInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

/// Get the swizzle id for the dst swizzle that corresponds to the
/// current instruction.
static OpSwizzle getDstSwizzleID(MachineInstr *MI);

/// Determine if the custom source swizzle or the
/// default swizzle for the specified operand should be used.
static bool isCustomSrcInst(MachineInstr *MI, unsigned OpNum);

/// Determine if the custom destination swizzle or the
/// default swizzle should be used for the instruction.
static bool isCustomDstInst(MachineInstr *MI);

/// Get the custom destination swizzle that corresponds tothe
/// instruction.
static OpSwizzle getCustomDstSwizzle(MachineInstr *MI);

/// Encode the new swizzle for the vector instruction.
static void encodeVectorInst(MachineInstr *MI);
/// Helper function to dump the operand for the machine instruction
/// and the relevant target flags.
static void dumpOperand(MachineInstr *MI, unsigned OpNum);

namespace llvm {
  FunctionPass *createAMDILSwizzleEncoder(const TargetMachine &TM) {
    return new AMDILSwizzleEncoder(TM);
  }
}

AMDILSwizzleEncoder::AMDILSwizzleEncoder(const TargetMachine &TM) :
  MachineFunctionPass(ID),
  TRI(TM.getRegisterInfo()) {

}

const char *AMDILSwizzleEncoder::getPassName() const {
  return "AMD IL Swizzle Encoder Pass";
}

bool AMDILSwizzleEncoder::runOnMachineFunction(MachineFunction &MF) {
  // Encode swizzles in instruction operands.
  encodeSwizzles(MF);
  return true;
}

/// Dump the operand swizzle information to the dbgs() stream.
void dumpOperand(MachineInstr *MI, unsigned OpNum) {
  OpSwizzle SwizID;

  const AMDILMachineFunctionInfo *MFI =
      MI->getParent()->getParent()->getInfo<AMDILMachineFunctionInfo>();
  SwizID.u8all = MFI->getMOSwizzle( MI->getOperand(OpNum) );
  dbgs() << '\t' << (SwizID.bits.dst ? "Dst" : "Src")
      << " Operand: " << OpNum << " SwizID: "
      << (unsigned)SwizID.bits.swizzle
      << " Swizzle: " << (SwizID.bits.dst
          ? getDstSwizzle(SwizID.bits.swizzle)
          : getSrcSwizzle(SwizID.bits.swizzle)) << '\n';
}

// This function checks for instructions that don't have
// normal swizzle patterns to their source operands. These have to be
// handled on a case by case basis.
bool isCustomSrcInst(MachineInstr *MI, unsigned OpNum) {
  if (MI->getDesc().getNumOperands() == 0)
    return false;
  unsigned RegClass = MI->getDesc().OpInfo[0].RegClass;
  if ((isPtrLoadInst(MI) || isPtrStoreInst(MI))
      && (isScratchInst(MI)
        || isCBInst(MI)
        || isUAVArenaInst(MI))
      && (RegClass == AMDIL::GPRI16RegClassID
        || RegClass == AMDIL::GPRI8RegClassID
        || RegClass == AMDIL::GPR_32RegClassID)
      && !isExtLoadInst(MI)
      && !isTruncStoreInst(MI)) {
      return true;
  }
  uint32_t MaskVal = (MI->getDesc().TSFlags & AMDID::SWZLMASK) >> AMDID::SWZLSHFT;
  return (MaskVal ? (MaskVal & (1ULL << OpNum)) : false);
}
#define GENERATE_1ARG_CASE(A) \
  case A##r: \
  case A##i:
#define GENERATE_2ARG_CASE(A) \
             GENERATE_1ARG_CASE(A##r) \
             GENERATE_1ARG_CASE(A##i)
#define GENERATE_3ARG_CASE(A) \
             GENERATE_2ARG_CASE(A##r) \
             GENERATE_2ARG_CASE(A##i)
#define GENERATE_4ARG_CASE(A) \
             GENERATE_3ARG_CASE(A##r) \
             GENERATE_3ARG_CASE(A##i)


// This function returns the OpSwizzle with the custom swizzle set
// correclty for source operands.
OpSwizzle AMDILSwizzleEncoder::getCustomSrcSwizzle(MachineInstr *MI,
                                                   unsigned OpNum) const {
  OpSwizzle OpSwiz;
  OpSwiz.u8all = 0;
  if (!MI->getOperand(OpNum).isReg())
    return OpSwiz;
  unsigned OpCode = MI->getOpcode();
  unsigned Reg = MI->getOperand(OpNum).getReg();
  unsigned RegClass = MI->getDesc().OpInfo[0].RegClass;
  if ((isPtrLoadInst(MI) || isPtrStoreInst(MI))
      && (isScratchInst(MI)
        || isCBInst(MI)
        || isUAVArenaInst(MI))
      && (RegClass == AMDIL::GPRI16RegClassID
        || RegClass == AMDIL::GPRI8RegClassID
        || RegClass == AMDIL::GPR_32RegClassID)
      && !isExtLoadInst(MI)
      && !isTruncStoreInst(MI)) {
    if (isUAVArenaInst(MI)) {
      if (isXComponentReg(Reg)) {
        OpSwiz.bits.swizzle = AMDIL_SRC_XXXX;
      } else if (isYComponentReg(Reg)) {
        OpSwiz.bits.swizzle = AMDIL_SRC_YYYY;
      } else if (isZComponentReg(Reg)) {
        OpSwiz.bits.swizzle = AMDIL_SRC_ZZZZ;
      } else if (isWComponentReg(Reg)) {
        OpSwiz.bits.swizzle = AMDIL_SRC_WWWW;
      } else {
        llvm_unreachable("unexpected reg component");
      }
      if (OpNum != 1) {
        OpSwiz.bits.swizzle = AMDIL_SRC_DFLT;
      }
    } else {
      OpSwiz.bits.swizzle = (OpNum == 1) ? AMDIL_SRC_XXXX: AMDIL_SRC_DFLT;
    }
    return OpSwiz;
  }
  if (isSemaphoreInst(MI)
      || isAppendInst(MI)
      || OpCode == AMDIL::CALL
      || OpCode == AMDIL::RETURN
      || OpCode == AMDIL::RETDYN) {
    OpSwiz.bits.swizzle = AMDIL_SRC_DFLT;
    return OpSwiz;
  }

  if (MI->isSelect()) {
    assert(OpNum == 1 && "Only operand number 1 is custom!");

    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    if (RC->getSize() != 8)
      return OpSwiz;

    if (isZWComponentReg(Reg)) {
      OpSwiz.bits.swizzle = AMDIL_SRC_ZZZZ;
    } else {
      assert(isXYComponentReg(Reg) && "unexpected reg component");
      OpSwiz.bits.swizzle = AMDIL_SRC_XXXX;
    }

    return OpSwiz;
  }

  switch (OpCode) {
  default:
    break;
  case AMDIL::DHIf64r:
  case AMDIL::LLOi64r:
    if (isZWComponentReg(Reg)) {
      OpSwiz.bits.swizzle = AMDIL_SRC_Z000;
    } else {
      assert(isXYComponentReg(Reg) && "unexpected reg component");
      OpSwiz.bits.swizzle = AMDIL_SRC_X000;
    }
    Reg = MI->getOperand(0).getReg();
    if (isYComponentReg(Reg)) {
      OpSwiz.bits.swizzle += 1;
    } else if (isZComponentReg(Reg)
               || isZWComponentReg(Reg)) {
      OpSwiz.bits.swizzle += 2;
    } else if (isWComponentReg(Reg)) {
      OpSwiz.bits.swizzle += 3;
    }
    break;
  case AMDIL::DHIv2f64r:
  case AMDIL::LLOv2i64r:
    OpSwiz.bits.swizzle = AMDIL_SRC_XZXZ;
    break;
  case AMDIL::DLOf64r:
  case AMDIL::LHIi64r:
    if (isZWComponentReg(Reg)) {
      OpSwiz.bits.swizzle = AMDIL_SRC_W000;
    } else {
      assert(isXYComponentReg(Reg) && "unexpected reg component");
      OpSwiz.bits.swizzle = AMDIL_SRC_Y000;
    }
    Reg = MI->getOperand(0).getReg();
    if (isYComponentReg(Reg)) {
      OpSwiz.bits.swizzle += 1;
    } else if (isZComponentReg(Reg)) {
      OpSwiz.bits.swizzle += 2;
    } else if (isWComponentReg(Reg)) {
      OpSwiz.bits.swizzle += 3;
    } else if (isZWComponentReg(Reg)) {
      OpSwiz.bits.swizzle += 2;
    }
    break;
  case AMDIL::DLOv2f64r:
  case AMDIL::LHIv2i64r:
    OpSwiz.bits.swizzle = AMDIL_SRC_YWYW;
    break;
  case AMDIL::DCREATEf64rr: {
    unsigned Swiz = AMDIL_SRC_X000;
    if (isWComponentReg(Reg)) {
      Swiz = AMDIL_SRC_W000;
    } else if (isYComponentReg(Reg)) {
      Swiz = AMDIL_SRC_Y000;
    } else if (isZComponentReg(Reg)) {
      Swiz = AMDIL_SRC_Z000;
    } else {
      assert(isXComponentReg(Reg) && "unexpected reg component");
    }
    Reg = MI->getOperand(0).getReg();
    if (isZWComponentReg(Reg)) {
      Swiz += 2;
    } else {
      assert(isXYComponentReg(Reg) && "unexpected reg component");
    }
    OpSwiz.bits.swizzle = Swiz + (OpNum == 1);
  }
    break;
  case AMDIL::DCREATEv2f64rr:
    OpSwiz.bits.swizzle = (OpNum == 1) ? AMDIL_SRC_0X0Y : AMDIL_SRC_X0Y0;
    break;
  case AMDIL::LCREATEi64rr: {
    unsigned Swiz1 = (OpNum == 1) ? AMDIL_SRC_X000 : AMDIL_SRC_0X00;
    if (MI->getOperand(OpNum).isReg()) {
      Reg = MI->getOperand(OpNum).getReg();
      if (isWComponentReg(Reg)) {
        Swiz1 += 12;
      } else if (isYComponentReg(Reg)) {
        Swiz1 += 4;
      } else if (isZComponentReg(Reg)) {
        Swiz1 += 8;
      } else {
        assert(isXComponentReg(Reg) && "unexpected reg component");
      }
    }
    Reg = MI->getOperand(0).getReg();
    if (isZWComponentReg(Reg)) {
      Swiz1 += 2;
    } else {
      assert(isXYComponentReg(Reg) && "unexpected reg component");
    }
    OpSwiz.bits.swizzle = Swiz1;
    break;
  }

 case AMDIL::LCREATEv2i64rr:
   if (isXYComponentReg(Reg)) {
     OpSwiz.bits.swizzle = OpNum + AMDIL_SRC_YWYW;
   } else {
     OpSwiz.bits.swizzle = OpNum + AMDIL_SRC_YZW0;
   }
   break;
 case AMDIL::CONTINUE_LOGICALNZf64r:
 case AMDIL::BREAK_LOGICALNZf64r:
 case AMDIL::IF_LOGICALNZf64r:
 case AMDIL::CONTINUE_LOGICALZf64r:
 case AMDIL::BREAK_LOGICALZf64r:
 case AMDIL::IF_LOGICALZf64r:
 case AMDIL::CONTINUE_LOGICALNZi64r:
 case AMDIL::BREAK_LOGICALNZi64r:
 case AMDIL::IF_LOGICALNZi64r:
 case AMDIL::CONTINUE_LOGICALZi64r:
 case AMDIL::BREAK_LOGICALZi64r:
 case AMDIL::IF_LOGICALZi64r:
   assert(OpNum == 0
          && "Only operand numbers 0 is custom!");
 case AMDIL::SWITCH:
   if (isXYComponentReg(Reg)) {
     OpSwiz.bits.swizzle = AMDIL_SRC_XXXX;
   } else if (isZWComponentReg(Reg)) {
     OpSwiz.bits.swizzle = AMDIL_SRC_ZZZZ;
   } else {
     llvm_unreachable("Found a case we don't handle!");
   }
   break;

 GENERATE_4ARG_CASE(AMDIL::UBIT_INSERTi32)
   assert((OpNum == 1 || OpNum == 2)
          && "Only operand numbers 1 or 2 is custom!");
   if (isXComponentReg(Reg)) {
     OpSwiz.bits.swizzle = AMDIL_SRC_XXXX;
   } else if (isYComponentReg(Reg)) {
     OpSwiz.bits.swizzle = AMDIL_SRC_YYYY;
   } else if (isZComponentReg(Reg)) {
     OpSwiz.bits.swizzle = AMDIL_SRC_ZZZZ;
   } else if (isWComponentReg(Reg)) {
     OpSwiz.bits.swizzle = AMDIL_SRC_WWWW;
   } else {
     llvm_unreachable("Found a case we don't handle!");
   }
   break;

  GENERATE_4ARG_CASE(AMDIL::UBIT_INSERTv2i32)
    assert((OpNum == 1 || OpNum == 2)
            && "Only operand numbers 1 or 2 is custom!");
    if (isXYComponentReg(Reg)) {
      OpSwiz.bits.swizzle = AMDIL_SRC_XYXY;
    } else if (isZWComponentReg(Reg)) {
      OpSwiz.bits.swizzle = AMDIL_SRC_ZWZW;
    } else {
      llvm_unreachable("Found a case we don't handle!");
    }
    break;

  GENERATE_4ARG_CASE(AMDIL::UBIT_INSERTv4i32)
    assert((OpNum == 1 || OpNum == 2)
            && "Only operand numbers 1 or 2 is custom!");
    OpSwiz.bits.swizzle = AMDIL_SRC_DFLT;
    break;
  case AMDIL::HILO_BITORv4i16rr:
    OpSwiz.bits.swizzle = AMDIL_SRC_XZXZ + (OpNum - 1);
    break;
  case AMDIL::HILO_BITORv2i32rr:
    if (isXComponentReg(Reg)) {
      OpSwiz.bits.swizzle = AMDIL_SRC_X000;
    } else if (isYComponentReg(Reg)) {
      OpSwiz.bits.swizzle = AMDIL_SRC_Y000;
    } else if (isZComponentReg(Reg)) {
      OpSwiz.bits.swizzle = AMDIL_SRC_Z000;
    } else if (isWComponentReg(Reg)) {
      OpSwiz.bits.swizzle = AMDIL_SRC_W000;
    } else {
      llvm_unreachable("Found a case we don't handle!");
    }
    Reg = MI->getOperand(0).getReg();
    if (isYComponentReg(Reg)) {
      OpSwiz.bits.swizzle += 1;
    } else if (isZComponentReg(Reg)) {
      OpSwiz.bits.swizzle += 2;
    } else if (isWComponentReg(Reg)) {
      OpSwiz.bits.swizzle += 3;
    } else {
      assert(isXComponentReg(Reg) && "Found a case we don't handle!");
    }
    break;
  case AMDIL::HILO_BITORv2i64rr: {
    unsigned Offset = 0;
    if (isXYComponentReg(Reg)) {
      Offset = AMDIL_SRC_XY00;
    } else if (isZWComponentReg(Reg)) {
      Offset = AMDIL_SRC_ZW00;
    } else {
      llvm_unreachable("Found a case we don't handle!");
    }
    Reg = MI->getOperand(0).getReg();
    if (isZWComponentReg(Reg)) {
      Offset += 1;
    } else {
      assert(isXYComponentReg(Reg) && "Found a case we don't handle!");
    }
    OpSwiz.bits.swizzle = Offset;
    break;
  }
 }

 return OpSwiz;
}
#undef GENERATE_3ARG_CASE
#undef GENERATE_4ARG_CASE

// This function checks for instructions that don't have
// normal swizzle patterns to their destination operand.
// These have to be handled on a case by case basis.
bool isCustomDstInst(MachineInstr *MI) {
  if (MI->getDesc().getNumOperands() == 0)
    return false;

  if ((isPtrLoadInst(MI) || isPtrStoreInst(MI))
      && ( (isLDSInst(MI)
          && (MI->getDesc().OpInfo[1].RegClass == AMDIL::GPRI16RegClassID
            || MI->getDesc().OpInfo[1].RegClass == AMDIL::GPRI8RegClassID
            || MI->getDesc().OpInfo[1].RegClass == AMDIL::GPR_32RegClassID
            || MI->getDesc().OpInfo[1].RegClass == AMDIL::GPR_64RegClassID))
        || isGDSInst(MI)
        || isScratchInst(MI)
        || isCBInst(MI)
        || isUAVArenaInst(MI))
      && !isExtLoadInst(MI)
      && !isTruncStoreInst(MI)) {
      return true;
  }
  return MI->getDesc().TSFlags & (1ULL << AMDID::SWZLDST);
}

// This function returns the OpSwizzle with the custom swizzle set
// correclty for destination operands.
OpSwizzle getCustomDstSwizzle(MachineInstr *MI) {
  OpSwizzle OpSwiz;
  OpSwiz.u8all = 0;
  unsigned OpCode = MI->getOpcode();
  OpSwiz.bits.dst = 1;
  unsigned Reg = MI->getOperand(0).isReg() ? MI->getOperand(0).getReg() : 0;
  if (((isPtrLoadInst(MI) || isPtrStoreInst(MI))
        && ((isLDSInst(MI)
          && (MI->getDesc().OpInfo[1].RegClass == AMDIL::GPRI16RegClassID
            || MI->getDesc().OpInfo[1].RegClass == AMDIL::GPRI8RegClassID
            || MI->getDesc().OpInfo[1].RegClass == AMDIL::GPR_32RegClassID))
          || isGDSInst(MI)
          || isScratchInst(MI)
          || isCBInst(MI)
          || isUAVArenaInst(MI))
        && !isExtLoadInst(MI)
        && !isTruncStoreInst(MI))
      || isSemaphoreInst(MI)
      || isAppendInst(MI)) {
      OpSwiz.bits.dst = 0;
      if (isXComponentReg(Reg)) {
        OpSwiz.bits.swizzle = AMDIL_SRC_XXXX;
      } else if (isYComponentReg(Reg)) {
        OpSwiz.bits.swizzle = AMDIL_SRC_YYYY;
      } else if (isZComponentReg(Reg)) {
        OpSwiz.bits.swizzle = AMDIL_SRC_ZZZZ;
      } else if (isWComponentReg(Reg)) {
        OpSwiz.bits.swizzle = AMDIL_SRC_WWWW;
      }
      if (isPtrStoreInst(MI) && isScratchInst(MI)) {
        if (isXYComponentReg(Reg)) {
          OpSwiz.bits.dst = 1;
          OpSwiz.bits.swizzle = AMDIL_DST_XY__;
        } else if (isZWComponentReg(Reg)) {
          OpSwiz.bits.dst = 1;
          OpSwiz.bits.swizzle = AMDIL_DST___ZW;
        }
      }
    return OpSwiz;
  }

  switch (OpCode) {
  case AMDIL::HILO_BITORv4i16rr:
  case AMDIL::HILO_BITORv2i64rr:
    if (isXYComponentReg(Reg)) {
      OpSwiz.bits.swizzle = AMDIL_DST_XY__;
    } else {
      OpSwiz.bits.swizzle = AMDIL_DST___ZW;
    }
    break;
  default:
    llvm_unreachable("getCustomDstSwizzle hit opcode it doesn't understand!");
    if (isXComponentReg(Reg)) {
      OpSwiz.bits.swizzle = AMDIL_DST_X___;
    } else if (isYComponentReg(Reg)) {
      OpSwiz.bits.swizzle = AMDIL_DST__Y__;
    } else if (isZComponentReg(Reg)) {
      OpSwiz.bits.swizzle = AMDIL_DST___Z_;
    } else if (isWComponentReg(Reg)) {
      OpSwiz.bits.swizzle = AMDIL_DST____W;
    }
  }
  return OpSwiz;
}

OpSwizzle AMDILSwizzleEncoder::getSrcSwizzleID(MachineInstr *MI,
                                               unsigned OpNum) const {
  assert(OpNum < MI->getNumOperands() &&
      "Must pass in a valid operand number.");
  OpSwizzle CurSwiz;
  CurSwiz.u8all = 0;
  CurSwiz.bits.dst = 0; // We need to reset the dst bit.
  unsigned Reg = 0;
  if (MI->getOperand(OpNum).isReg()) {
    Reg = MI->getOperand(OpNum).getReg();
  }
  if (isCustomSrcInst(MI, OpNum)) {
    CurSwiz = getCustomSrcSwizzle(MI, OpNum);
  } else if (isXComponentReg(Reg)) {
    CurSwiz.bits.swizzle = AMDIL_SRC_XXXX;
  } else if (isYComponentReg(Reg)) {
    CurSwiz.bits.swizzle = AMDIL_SRC_YYYY;
  } else if (isZComponentReg(Reg)) {
    CurSwiz.bits.swizzle = AMDIL_SRC_ZZZZ;
  } else if (isWComponentReg(Reg)) {
    CurSwiz.bits.swizzle = AMDIL_SRC_WWWW;
  } else if (isXYComponentReg(Reg)) {
    CurSwiz.bits.swizzle = AMDIL_SRC_XYXY;
  } else if (isZWComponentReg(Reg)) {
    CurSwiz.bits.swizzle = AMDIL_SRC_ZWZW;
  } else if (Reg == AMDIL::R1011 && MI->getOpcode() == TargetOpcode::COPY) {
    Reg = MI->getOperand(0).getReg();
    if (isXComponentReg(Reg) || isYComponentReg(Reg)
        || isZComponentReg(Reg) || isWComponentReg(Reg)) {
      CurSwiz.bits.swizzle = AMDIL_SRC_XXXX;
    } else if (isXYComponentReg(Reg) || isZWComponentReg(Reg)) {
      CurSwiz.bits.swizzle = AMDIL_SRC_XYXY;
    }
  } else {
    CurSwiz.bits.swizzle = AMDIL_SRC_DFLT;
  }
  return CurSwiz;
}

OpSwizzle getDstSwizzleID(MachineInstr *MI) {
  OpSwizzle CurSwiz;
  CurSwiz.bits.dst = 1;
  CurSwiz.bits.swizzle = AMDIL_DST_DFLT;
  unsigned Reg = 0;
  if (MI->getOperand(0).isReg()) {
    Reg = MI->getOperand(0).getReg();
  }
  if (isCustomDstInst(MI)) {
    CurSwiz = getCustomDstSwizzle(MI);
  } else if (isXComponentReg(Reg)) {
    CurSwiz.bits.swizzle = AMDIL_DST_X___;
  } else if (isYComponentReg(Reg)) {
    CurSwiz.bits.swizzle = AMDIL_DST__Y__;
  } else if (isZComponentReg(Reg)) {
    CurSwiz.bits.swizzle = AMDIL_DST___Z_;
  } else if (isWComponentReg(Reg)) {
    CurSwiz.bits.swizzle = AMDIL_DST____W;
  } else if (isXYComponentReg(Reg)) {
    CurSwiz.bits.swizzle = AMDIL_DST_XY__;
  } else if (isZWComponentReg(Reg)) {
    CurSwiz.bits.swizzle = AMDIL_DST___ZW;
  } else {
    CurSwiz.bits.swizzle = AMDIL_DST_DFLT;
  }

  return CurSwiz;
}

void encodeVectorInst(MachineInstr *MI) {
  assert(isVectorOpInst(MI) && "Only a vector instruction can be"
      " used to generate a new vector instruction!");
  unsigned OpCode = MI->getOpcode();
  // For all of the opcodes, the destination swizzle is the same.
  OpSwizzle SwizID = getDstSwizzleID(MI);
  OpSwizzle SrcID;
  SrcID.u8all = 0;

  AMDILMachineFunctionInfo *MFI =
    MI->getParent()->getParent()->getInfo<AMDILMachineFunctionInfo>();
  MFI->setMOSwizzle( MI->getOperand(0), SwizID.u8all );

  unsigned Offset = 0;
  unsigned Reg = MI->getOperand(0).getReg();
  switch (OpCode) {
    GENERATE_2ARG_CASE(AMDIL::VCONCATv2f32)
    GENERATE_2ARG_CASE(AMDIL::VCONCATv2i32)
    GENERATE_2ARG_CASE(AMDIL::VCONCATv2i16)
    GENERATE_2ARG_CASE(AMDIL::VCONCATv2i8)
      if (isZWComponentReg(Reg)) {
        Offset = 2;
      }

      for (unsigned I = 1; I < 3; ++I) {
        unsigned Offset2 = 0;
        if (MI->getOperand(I).isReg()) {
          Reg = MI->getOperand(I).getReg();
        }
        if (isXComponentReg(Reg)) {
          Offset2 = 0;
        } else if (isYComponentReg(Reg)) {
          Offset2 = 4;
        } else if (isZComponentReg(Reg)) {
          Offset2 = 8;
        } else if (isWComponentReg(Reg)) {
          Offset2 = 12;
        }
        SrcID.bits.swizzle = AMDIL_SRC_X000 + Offset + (I - 1) + Offset2;
        MFI->setMOSwizzle( MI->getOperand(I), SrcID.u8all );
      }
      break;

    GENERATE_2ARG_CASE(AMDIL::VCONCATv2f64)
    GENERATE_2ARG_CASE(AMDIL::VCONCATv2i64)
    GENERATE_2ARG_CASE(AMDIL::VCONCATv4f32)
    GENERATE_2ARG_CASE(AMDIL::VCONCATv4i32)
    GENERATE_2ARG_CASE(AMDIL::VCONCATv4i16)
    GENERATE_2ARG_CASE(AMDIL::VCONCATv4i8)
      for (unsigned I = 1; I < 3; ++I) {
        if (MI->getOperand(I).isReg()) {
          Reg = MI->getOperand(I).getReg();
        }
        if (isZWComponentReg(Reg)) {
          SrcID.bits.swizzle = AMDIL_SRC_ZW00 + (I - 1);
        } else {
          SrcID.bits.swizzle = AMDIL_SRC_XY00 + (I - 1);
        }

        MFI->setMOSwizzle( MI->getOperand(I), SrcID.u8all );
      }
    break;

    GENERATE_1ARG_CASE(AMDIL::VEXTRACTv2f32)
    GENERATE_1ARG_CASE(AMDIL::VEXTRACTv2i32)
    GENERATE_1ARG_CASE(AMDIL::VEXTRACTv2i16)
    GENERATE_1ARG_CASE(AMDIL::VEXTRACTv2i8)
      assert(MI->getOperand(2).getImm() <= 2
             && "Invalid immediate value encountered for this formula!");
      if (isXComponentReg(Reg)) {
        Offset = 0;
      } else if (isYComponentReg(Reg)) {
        Offset = 1;
      } else if (isZComponentReg(Reg)) {
        Offset = 2;
      } else if (isWComponentReg(Reg)) {
        Offset = 3;
      }
      assert(MI->getOperand(2).getImm() <= 4
             && "Invalid immediate value encountered for this formula!");
      if (MI->getOperand(1).isReg()) {
        Reg = MI->getOperand(1).getReg();
      }
      if (isZWComponentReg(Reg)) {
        SrcID.bits.swizzle = AMDIL_SRC_Z000;
      } else {
        SrcID.bits.swizzle = AMDIL_SRC_X000;
      }
      SrcID.bits.swizzle += Offset + (MI->getOperand(2).getImm() - 1) * 4;

      MFI->setMOSwizzle( MI->getOperand(1), SrcID.u8all);
      MFI->setMOSwizzle( MI->getOperand(2), 0);
      break;

    GENERATE_1ARG_CASE(AMDIL::VEXTRACTv4f32)
    GENERATE_1ARG_CASE(AMDIL::VEXTRACTv4i32)
    GENERATE_1ARG_CASE(AMDIL::VEXTRACTv4i16)
    GENERATE_1ARG_CASE(AMDIL::VEXTRACTv4i8)
      if (isXComponentReg(Reg)) {
        Offset = 0;
      } else if (isYComponentReg(Reg)) {
        Offset = 1;
      } else if (isZComponentReg(Reg)) {
        Offset = 2;
      } else if (isWComponentReg(Reg)) {
        Offset = 3;
      } else if (isXYComponentReg(Reg)) {
        Offset = 0;
      } else if (isZWComponentReg(Reg)) {
        Offset = 2;
      }
      assert(MI->getOperand(2).getImm() <= 4
             && "Invalid immediate value encountered for this formula!");
      SrcID.bits.swizzle = ((MI->getOperand(2).getImm() - 1) * 4) + 1 + Offset;

      MFI->setMOSwizzle( MI->getOperand(1), SrcID.u8all);
      MFI->setMOSwizzle( MI->getOperand(2), 0);
      break;

    GENERATE_1ARG_CASE(AMDIL::VEXTRACTv2f64)
    GENERATE_1ARG_CASE(AMDIL::VEXTRACTv2i64)
      assert(MI->getOperand(2).getImm() <= 2
             && "Invalid immediate value encountered for this formula!");
      if (isZWComponentReg(Reg)) {
        Offset = 1;
      }
      SrcID.bits.swizzle
        = AMDIL_SRC_XY00 + ((MI->getOperand(2).getImm() - 1) * 2) + Offset;

      MFI->setMOSwizzle( MI->getOperand(1), SrcID.u8all);
      MFI->setMOSwizzle( MI->getOperand(2), 0);
      break;

    GENERATE_2ARG_CASE(AMDIL::VINSERTv2f32)
    GENERATE_2ARG_CASE(AMDIL::VINSERTv2i32)
    GENERATE_2ARG_CASE(AMDIL::VINSERTv2i16)
    GENERATE_2ARG_CASE(AMDIL::VINSERTv2i8) {
      unsigned SwizVal = (unsigned)MI->getOperand(4).getImm();
      OpSwizzle Src2ID;
      Src2ID.u8all = 0;
      if (Reg >= AMDIL::Rzw1 && Reg < AMDIL::SDP) {
        Offset = 2;
      }

      unsigned Offset1 = 0;
      if (MI->getOperand(1).isReg()) {
        Reg = MI->getOperand(1).getReg();
        if (isZWComponentReg(Reg)) {
          Offset1 = 8;
        }
      }

      unsigned Offset2 = 0;
      if (MI->getOperand(2).isReg()) {
        Reg = MI->getOperand(2).getReg();
        if (isYComponentReg(Reg)) {
          Offset2 = 4;
        } else if (isZComponentReg(Reg)) {
          Offset2 = 8;
        } else if (isWComponentReg(Reg)) {
          Offset2 = 12;
        }
      }
      if (((SwizVal >> 8) & 0xFF) == 1) {
        SrcID.bits.swizzle = AMDIL_SRC_X000 + Offset1 + Offset;
        Src2ID.bits.swizzle = AMDIL_SRC_0X00 + Offset2 + Offset;
      } else {
        SrcID.bits.swizzle = AMDIL_SRC_0Y00 + Offset1 + Offset;
        Src2ID.bits.swizzle = AMDIL_SRC_X000 + Offset2 + Offset;
      }

      MFI->setMOSwizzle( MI->getOperand(1), SrcID.u8all);
      MFI->setMOSwizzle( MI->getOperand(2), Src2ID.u8all);
      MFI->setMOSwizzle( MI->getOperand(3), 0);
      MFI->setMOSwizzle( MI->getOperand(4), 0);
    }
    break;

    GENERATE_2ARG_CASE(AMDIL::VINSERTv4f32)
    GENERATE_2ARG_CASE(AMDIL::VINSERTv4i32)
    GENERATE_2ARG_CASE(AMDIL::VINSERTv4i16)
    GENERATE_2ARG_CASE(AMDIL::VINSERTv4i8) {
      unsigned SwizVal = (unsigned)MI->getOperand(4).getImm();
      OpSwizzle Src2ID;
      Src2ID.u8all = 0;
      if (Reg >= AMDIL::Rzw1 && Reg < AMDIL::SDP) {
        Offset = 2;
      }
      unsigned Offset2 = 0;
      if (MI->getOperand(2).isReg()) {
        Reg = MI->getOperand(2).getReg();
        if (isYComponentReg(Reg)) {
          Offset2 = 4;
        } else if (isZComponentReg(Reg)) {
          Offset2 = 8;
        } else if (isWComponentReg(Reg)) {
          Offset2 = 12;
        } else if (isZWComponentReg(Reg)) {
          Offset2 = 2;
        }
      }
      if ((SwizVal >> 8 & 0xFF) == 1) {
        SrcID.bits.swizzle = Offset ? AMDIL_SRC_XYZ0 : AMDIL_SRC_X0ZW;
        Src2ID.bits.swizzle = AMDIL_SRC_0X00 + Offset2 + Offset;
      } else if ((SwizVal >> 16 & 0xFF) == 1) {
        SrcID.bits.swizzle = AMDIL_SRC_XY0W;
        Src2ID.bits.swizzle = AMDIL_SRC_00X0 + Offset2;
      } else if ((SwizVal >> 24 & 0xFF) == 1) {
        SrcID.bits.swizzle = AMDIL_SRC_XYZ0;
        Src2ID.bits.swizzle = AMDIL_SRC_000X + Offset2;
      } else {
        SrcID.bits.swizzle = Offset ? AMDIL_SRC_XY0W : AMDIL_SRC_0YZW;
        Src2ID.bits.swizzle = AMDIL_SRC_X000 + Offset2 + Offset;
      }

      MFI->setMOSwizzle( MI->getOperand(1), SrcID.u8all);
      MFI->setMOSwizzle( MI->getOperand(2), Src2ID.u8all);
      MFI->setMOSwizzle( MI->getOperand(3), 0);
      MFI->setMOSwizzle( MI->getOperand(4), 0);
      break;
    }

    GENERATE_2ARG_CASE(AMDIL::VINSERTv2f64)
    GENERATE_2ARG_CASE(AMDIL::VINSERTv2i64) {
      unsigned SwizVal = (unsigned)MI->getOperand(4).getImm();
      OpSwizzle Src2ID;
      Src2ID.u8all = 0;
      if (MI->getOperand(2).isReg()) {
        Reg = MI->getOperand(2).getReg();
        if (isZWComponentReg(Reg)) {
          Offset = 2;
        }
      }
      if (((SwizVal >> 8) & 0xFF) == 1) {
        SrcID.bits.swizzle = AMDIL_SRC_XY00;
        Src2ID.bits.swizzle = AMDIL_SRC_00XY + Offset;
      } else {
        SrcID.bits.swizzle = AMDIL_SRC_00ZW;
        Src2ID.bits.swizzle = AMDIL_SRC_XY00 + Offset;
      }

      MFI->setMOSwizzle( MI->getOperand(1), SrcID.u8all);
      MFI->setMOSwizzle( MI->getOperand(2), Src2ID.u8all);
      MFI->setMOSwizzle( MI->getOperand(3), 0);
      MFI->setMOSwizzle( MI->getOperand(4), 0);
    }
    break;
  }

  DEBUG(
    for (unsigned I = 0, N = MI->getNumOperands(); I < N; ++I) {
      dumpOperand(MI, I);
    }
    dbgs() << '\n';
  );
}

// This function loops through all of the instructions, skipping function
// calls, and encodes the swizzles in the operand.
void AMDILSwizzleEncoder::encodeSwizzles(MachineFunction &MF) const {
  for (MachineFunction::iterator MFI = MF.begin(), MFE = MF.end();
      MFI != MFE; ++MFI) {
    MachineBasicBlock *MBB = MFI;
    for (MachineBasicBlock::iterator MBI = MBB->begin(), MBE = MBB->end();
        MBI != MBE; ++MBI) {
      MachineInstr *MI = MBI;
      if (MI->getOpcode() == AMDIL::RETDYN
          || MI->getOpcode() == AMDIL::RETURN
          || MI->getOpcode() == AMDIL::DBG_VALUE) {
        continue;
      }
      DEBUG(
        dbgs() << "Encoding instruction: ";
        MI->print(dbgs());
      );
      if (isVectorOpInst(MI)) {
        encodeVectorInst(MI);
        continue;
      }

      for (unsigned a = 0, z = MI->getNumOperands(); a < z; ++a) {
        OpSwizzle SwizID;
        MachineOperand &MO = MI->getOperand(a);
        if (MO.isReg() && MO.isDef() && !MO.isImplicit()) {
          SwizID = getDstSwizzleID(MI);
        } else {
          SwizID = getSrcSwizzleID(MI, a);
        }

        MI->getParent()->getParent()->getInfo<AMDILMachineFunctionInfo>()->
        setMOSwizzle( MI->getOperand(a), SwizID.u8all);
        DEBUG(dumpOperand(MI, a));
      }

      DEBUG(dbgs() << '\n');
    }
  }
}
