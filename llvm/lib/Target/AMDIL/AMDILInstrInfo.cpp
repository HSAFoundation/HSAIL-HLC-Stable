//===- AMDILInstrInfo.cpp - AMDIL Instruction Information -------*- C++ -*-===//
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
//
// This file contains the AMDIL implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "amdilinstrinfo"

#include "AMDIL.h"
#include "AMDILInstrInfo.h"
#include "AMDILUtilityFunctions.h"
#define GET_INSTRINFO_CTOR
#define GET_INSTRINFO_MC_DESC
#include "AMDILGenInstrInfo.inc"
#include "AMDILMachineFunctionInfo.h"
#include "llvm/Instructions.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

AMDILInstrInfo::AMDILInstrInfo(AMDILTargetMachine &tm)
  : AMDILGenInstrInfo(AMDIL::ADJCALLSTACKDOWN, AMDIL::ADJCALLSTACKUP),
    RI(tm, *this),
    TM(tm) {
}

const AMDILRegisterInfo &AMDILInstrInfo::getRegisterInfo() const {
  return RI;
}

/// Return true if the instruction is a register to register move and leave the
/// source and dest operands in the passed parameters.
bool AMDILInstrInfo::isMoveInstr(const MachineInstr &MI,
    unsigned &SrcReg,
    unsigned &DstReg,
    unsigned &SrcSubIdx,
    unsigned &DstSubIdx) const {
  // FIXME: we should look for:
  //    add with 0
  //llvm_unreachable("is Move Instruction has not been implemented yet!");
  //return true;
  if (MI.getOpcode() == TargetOpcode::COPY) {
    return false;
  }

  if (!MI.getOperand(0).isReg() || !MI.getOperand(1).isReg()) {
    return false;
  }

  SrcReg = MI.getOperand(1).getReg();
  DstReg = MI.getOperand(0).getReg();
  DstSubIdx = 0;
  SrcSubIdx = 0;
  return true;
}

bool AMDILInstrInfo::isCoalescableExtInstr(const MachineInstr &MI,
    unsigned &SrcReg,
    unsigned &DstReg,
    unsigned &SubIdx) const {
  return false;
  SubIdx = AMDIL::NoSubRegister;

  switch (MI.getOpcode()) {
  default:
    return false;
  case AMDIL::DHIf64r:
  case AMDIL::LHIi64r:
    if (MI.getOperand(0).getSubReg() || MI.getOperand(1).getSubReg()) {
      // Be conservative.
      return false;
    }

    SrcReg = MI.getOperand(1).getReg();
    DstReg = MI.getOperand(0).getReg();
    SubIdx = AMDIL::sub_y;
    break;
  case AMDIL::DLOf64r:
  case AMDIL::LLOi64r:

    if (MI.getOperand(0).getSubReg() || MI.getOperand(1).getSubReg()) {
      // Be conservative.
      return false;
    }

    SrcReg = MI.getOperand(1).getReg();
    DstReg = MI.getOperand(0).getReg();
    SubIdx = AMDIL::sub_x;
    break;
  case AMDIL::VEXTRACTv2f64r:
  case AMDIL::VEXTRACTv2i64r:
    if (MI.getOperand(0).getSubReg() || MI.getOperand(1).getSubReg()) {
      // Be conservative.
      return false;
    }

    SrcReg = MI.getOperand(1).getReg();
    DstReg = MI.getOperand(0).getReg();
    assert(MI.getOperand(2).isImm()
        && "Operand 2 must be an immediate value!");

    switch (MI.getOperand(2).getImm()) {
    case 0:
      SubIdx = AMDIL::sub_xy;
      break;
    case 1:
      SubIdx = AMDIL::sub_zw;
      break;
    default:
      return false;
    }

  case AMDIL::VEXTRACTv2f32r:
  case AMDIL::VEXTRACTv2i32r:
  case AMDIL::VEXTRACTv2i16r:
  case AMDIL::VEXTRACTv2i8r:
  case AMDIL::VEXTRACTv4f32r:
  case AMDIL::VEXTRACTv4i32r:
  case AMDIL::VEXTRACTv4i16r:
  case AMDIL::VEXTRACTv4i8r:
    if (MI.getOperand(0).getSubReg() || MI.getOperand(1).getSubReg()) {
      // Be conservative.
      return false;
    }

    SrcReg = MI.getOperand(1).getReg();
    DstReg = MI.getOperand(0).getReg();
    assert(MI.getOperand(2).isImm()
        && "Operand 2 must be an immediate value!");

    switch (MI.getOperand(2).getImm()) {
    case 0:
      SubIdx = AMDIL::sub_x;
      break;
    case 1:
      SubIdx = AMDIL::sub_y;
      break;
    case 2:
      SubIdx = AMDIL::sub_z;
      break;
    case 3:
      SubIdx = AMDIL::sub_w;
      break;
    default:
      return false;
    };
  };

  return SubIdx != AMDIL::NoSubRegister;
}

unsigned AMDILInstrInfo::isLoadFromStackSlot(const MachineInstr *MI,
                                             int &FrameIndex) const {
  if (isPrivateInst(MI) && isPtrLoadInst(MI) && MI->getOperand(1).isFI()) {
    FrameIndex = MI->getOperand(1).getIndex();
    return MI->getOperand(0).getReg();
  }

  return 0;
}

unsigned AMDILInstrInfo::isLoadFromStackSlotPostFE(const MachineInstr *MI,
                                                   int &FrameIndex) const {
  if (isPrivateInst(MI) && isPtrLoadInst(MI) && MI->getOperand(1).isFI()) {
    FrameIndex = MI->getOperand(1).getIndex();
    return MI->getOperand(0).getReg();
  }

  return 0;
}

bool AMDILInstrInfo::hasLoadFromStackSlot(const MachineInstr *MI,
                                          const MachineMemOperand *&MMO,
                                          int &FrameIndex) const {
  for (MachineInstr::mmo_iterator I = MI->memoperands_begin(),
         E = MI->memoperands_end(); I != E; ++I) {
    MachineMemOperand *Operand = *I;
    if (Operand->isLoad() && Operand->getValue())
      if (const FixedStackPseudoSourceValue *Value =
          dyn_cast<const FixedStackPseudoSourceValue>(Operand->getValue())) {
        FrameIndex = Value->getFrameIndex();
        MMO = Operand;
        return true;
      }
  }

  return false;
}
unsigned AMDILInstrInfo::isStoreToStackSlot(const MachineInstr *MI,
                                            int &FrameIndex) const {
  if (isPrivateInst(MI) && isPtrStoreInst(MI) && MI->getOperand(1).isFI()) {
    FrameIndex = MI->getOperand(1).getIndex();
    return MI->getOperand(0).getReg();
  }

  return 0;
}
unsigned AMDILInstrInfo::isStoreToStackSlotPostFE(const MachineInstr *MI,
                                                  int &FrameIndex) const {
  if (isPrivateInst(MI) && isPtrStoreInst(MI) && MI->getOperand(1).isFI()) {
    if (unsigned Reg = isStoreToStackSlot(MI, FrameIndex)) {
      return Reg;
    }

    const MachineMemOperand *Dummy = NULL;

    return hasStoreToStackSlot(MI, Dummy, FrameIndex);
  }

  return 0;
}

bool AMDILInstrInfo::hasStoreToStackSlot(const MachineInstr *MI,
                                         const MachineMemOperand *&MMO,
                                         int &FrameIndex) const {
  for (MachineInstr::mmo_iterator I = MI->memoperands_begin(),
         E = MI->memoperands_end(); I != E; ++I) {
    MachineMemOperand *Operand = *I;
    if (Operand->isStore() && Operand->getValue())
      if (const FixedStackPseudoSourceValue *Value =
          dyn_cast<const FixedStackPseudoSourceValue>(Operand->getValue())) {
        FrameIndex = Value->getFrameIndex();
        MMO = Operand;
        return true;
      }
  }

  return false;
}

void AMDILInstrInfo::reMaterialize(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator MI,
                                   unsigned DestReg,
                                   unsigned SubIdx,
                                   const MachineInstr *Orig,
                                   const TargetRegisterInfo &TRI) const {
  // TODO: Implement this function
}

MachineInstr *AMDILInstrInfo::duplicate(MachineInstr *Orig,
                                        MachineFunction &MF) const {
  // TODO: Implement this function
  return MF.CloneMachineInstr(Orig);
}

MachineInstr *AMDILInstrInfo::convertToThreeAddress(
  MachineFunction::iterator &MFI,
  MachineBasicBlock::iterator &MBBI,
  LiveVariables *LV) const {
  // TODO: Implement this function
  return NULL;
}

MachineInstr *AMDILInstrInfo::commuteInstruction(MachineInstr *MI,
    bool NewMI) const {
  // TODO: Implement this function
  return NULL;
}

bool AMDILInstrInfo::findCommutedOpIndices(MachineInstr *MI,
                                           unsigned &SrcOpIdx1,
                                           unsigned &SrcOpIdx2) const {
  // TODO: Implement this function
  return false;
}

bool AMDILInstrInfo::produceSameValue(const MachineInstr *MI0,
                                      const MachineInstr *MI1,
                                      const MachineRegisterInfo *MRI) const {
  // TODO: Implement this function
  return false;
}

bool AMDILInstrInfo::AnalyzeBranch(MachineBasicBlock &MBB,
                                   MachineBasicBlock *&TBB,
                                   MachineBasicBlock *&FBB,
                                   SmallVectorImpl<MachineOperand> &Cond,
                                   bool AllowModify) const {
  MachineBasicBlock::iterator I = MBB.end();

  // If the block has no terminators, it just falls into the block after it.
  if (I == MBB.begin() || !isUnpredicatedTerminator(--I)) {
    return false;
  }

  // Get the last instruction in the block.
  MachineInstr *LastInst = I;

  // If there is only one terminator instruction, process it.
  if (I == MBB.begin() || !isUnpredicatedTerminator(--I)) {
    if (LastInst->isUnconditionalBranch()) {
      TBB = LastInst->getOperand(0).getMBB();
      return false;
    } else if (LastInst->isConditionalBranch()) {
      // Block ends with fall-through condbranch.
      TBB = LastInst->getOperand(0).getMBB();
      Cond.push_back(LastInst->getOperand(1));
      return false;
    }

    // Otherwise, don't know what this is.
    return true;
  }

  // Get the instruction before it if it's a terminator.
  MachineInstr *SecondLastInst = I;

  // If there are three terminators, we don't know what sort of block this is.
  if (SecondLastInst && I != MBB.begin() &&
      isUnpredicatedTerminator(--I)) {
    return true;
  }

  // If the block ends with a conditional and unconditional branch, handle it.
  if (SecondLastInst->isConditionalBranch()
      && LastInst->isUnconditionalBranch()) {
    DEBUG(dbgs() << "Pushing SecondLastInst:         ");
    DEBUG(SecondLastInst->dump());

    TBB = SecondLastInst->getOperand(0).getMBB();
    Cond.push_back(SecondLastInst->getOperand(1));
    FBB = LastInst->getOperand(0).getMBB();

    return false;
  }

  // If the block ends with two unconditional branches, handle it.  The second
  // one is not executed, so remove it.
  if (SecondLastInst->isUnconditionalBranch()
      && LastInst->isUnconditionalBranch()) {
    TBB = SecondLastInst->getOperand(0).getMBB();
    I = LastInst;

    if (AllowModify) {
      I->eraseFromParent();
    }

    return false;
  }

  // Otherwise, can't handle this.
  return true;
}

unsigned AMDILInstrInfo::getBranchInstr(const MachineOperand &Op) const {
  const MachineInstr *MI = Op.getParent();
  switch (MI->getDesc().OpInfo[0].RegClass) {
  default:
    return AMDIL::BRANCHi32br;
  case AMDIL::GPRI8RegClassID:
    return AMDIL::BRANCHi8br;
  case AMDIL::GPRI16RegClassID:
    return AMDIL::BRANCHi16br;
  case AMDIL::GPR_32RegClassID:
    return AMDIL::BRANCHi32br;
  case AMDIL::GPR_64RegClassID:
    return AMDIL::BRANCHi64br;
  }
}

unsigned AMDILInstrInfo::InsertBranch(
  MachineBasicBlock &MBB,
  MachineBasicBlock *TBB,
  MachineBasicBlock *FBB,
  const SmallVectorImpl<MachineOperand> &Cond,
  DebugLoc DL) const {
  assert(TBB && "InsertBranch must not be told to insert a fallthrough");

  assert((Cond.size() == 1 || Cond.size() == 0)
      && "AMDIL Branch conditions only have one component");

  if (FBB) {
    BuildMI(&MBB, DL, get(getBranchInstr(Cond[0])))
    .addMBB(TBB).addOperand(Cond[0]);
    BuildMI(&MBB, DL, get(AMDIL::BRANCHb))
    .addMBB(FBB);
    return 2;
  }

  // One way branch
  if (Cond.empty()) {
    // Unconditional branch
    BuildMI(&MBB, DL, get(AMDIL::BRANCHb))
    .addMBB(TBB);
  } else {
    BuildMI(&MBB, DL, get(getBranchInstr(Cond[0])))
    .addMBB(TBB).addOperand(Cond[0]);
  }

  return 1;
}

unsigned AMDILInstrInfo::RemoveBranch(MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator I = MBB.end();

  if (I == MBB.begin()) {
    return 0;
  }

  --I;

  while (I->isDebugValue()) {
    if (I == MBB.begin()) {
      return 0;
    }

    --I;
  }

  if (!I->isUnconditionalBranch() && !I->isConditionalBranch()) {
    return 0;
  }

  DEBUG(dbgs() << "Removing branch:         ");
  DEBUG(I->dump());

  // Remove the branch.
  I->eraseFromParent();
  I = MBB.end();

  if (I == MBB.begin()) {
    return 1;
  }

  --I;

  if (!I->isConditionalBranch()) {
    return 1;
  }

  DEBUG(dbgs() << "Removing second branch:         ");
  DEBUG(I->dump());

  // Remove the branch.
  I->eraseFromParent();
  return 2;
}

bool AMDILInstrInfo::isProfitableToIfCvt(MachineBasicBlock &,
                                         unsigned, unsigned,
                                         const BranchProbability &) const {
  return true;
}

bool AMDILInstrInfo::isProfitableToIfCvt(MachineBasicBlock &,
                                         unsigned, unsigned,
                                         MachineBasicBlock &,
                                         unsigned, unsigned,
                                         const BranchProbability &) const {
  return true;
}

bool AMDILInstrInfo::isProfitableToDupForIfCvt(MachineBasicBlock &,
                                               unsigned,
                                               const BranchProbability &) const {
  return true;
}

bool AMDILInstrInfo::isProfitableToUnpredicate(MachineBasicBlock &,
                                               MachineBasicBlock &) const {
  return false;
}

MachineBasicBlock::iterator skipFlowControl(MachineBasicBlock *MBB) {
  MachineBasicBlock::iterator tmp = MBB->end();

  if (MBB->empty()) {
    return MBB->end();
  }

  while (--tmp) {
    if (tmp->getOpcode() == AMDIL::ENDLOOP
        || tmp->getOpcode() == AMDIL::ENDIF
        || tmp->getOpcode() == AMDIL::ELSE) {
      if (tmp == MBB->begin()) {
        return tmp;
      } else {
        continue;
      }
    } else {
      return ++tmp;
    }
  }

  return MBB->end();
}

bool AMDILInstrInfo::copyRegToReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I,
                                  unsigned DestReg,
                                  unsigned SrcReg,
                                  const TargetRegisterClass *DestRC,
                                  const TargetRegisterClass *SrcRC,
                                  DebugLoc DL) const {
  // If we are adding to the end of a basic block we can safely assume that the
  // move is caused by a PHI node since all move instructions that are non-PHI
  // have already been inserted into the basic blocks Therefor we call the skip
  // flow control instruction to move the iterator before the flow control
  // instructions and put the move instruction there.
  //bool Phi = (DestReg < 1025) || (SrcReg < 1025);
  int MovInst = TargetOpcode::COPY;

  MachineBasicBlock::iterator ITemp
    = (I == MBB.end()) ? skipFlowControl(&MBB) : I;

  if (DestRC == SrcRC) {
    BuildMI(MBB, ITemp, DL, get(MovInst), DestReg)
      .addReg(SrcReg);
    return true;
  }

  size_t DestSize = DestRC->getSize();
  size_t SrcSize = SrcRC->getSize();

  if (DestSize > SrcSize) {
    // Elements are going to get duplicated.
    BuildMI(MBB, ITemp, DL, get(MovInst), DestReg)
      .addReg(SrcReg);
  } else if (DestSize == SrcSize) {
    // Direct copy, conversions are not handled.
    BuildMI(MBB, ITemp, DL, get(MovInst), DestReg)
      .addReg(SrcReg);
  } else if (DestSize < SrcSize) {
    // Elements are going to get dropped.
    BuildMI(MBB, ITemp, DL, get(MovInst), DestReg)
      .addReg(SrcReg);
  }

  return true;
}

void AMDILInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 DebugLoc DL,
                                 unsigned DestReg,
                                 unsigned SrcReg,
                                 bool KillSrc) const  {
  BuildMI(MBB, MI, DL, get(TargetOpcode::COPY), DestReg)
    .addReg(SrcReg, getKillRegState(KillSrc));
  return;
#if 0
  DEBUG(dbgs() << "Cannot copy " << RI.getName(SrcReg)
      << " to " << RI.getName(DestReg) << '\n');
  llvm_unreachable("Cannot emit physreg copy instruction");
#endif
}

void AMDILInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator MI,
                                         unsigned SrcReg,
                                         bool IsKill,
                                         int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI) const {
  unsigned Opc = 0;
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = *MF.getFrameInfo();

  DebugLoc DL;

  switch (RC->getID()) {
  default:
    Opc = AMDIL::PRIVATESTOREv4i32r;
    break;
  case AMDIL::GPRI16RegClassID:
    Opc = AMDIL::PRIVATESTOREi16r;
    break;
  case AMDIL::GPR_32RegClassID:
    Opc = AMDIL::PRIVATESTOREi32r;
    break;
  case AMDIL::GPRI8RegClassID:
    Opc = AMDIL::PRIVATESTOREi8r;
    break;
  case AMDIL::GPR_64RegClassID:
    Opc = AMDIL::PRIVATESTOREi64r;
    break;
  case AMDIL::GPRV2I16RegClassID:
    Opc = AMDIL::PRIVATESTOREv2i16r;
    break;
  case AMDIL::GPRV2I32RegClassID:
    Opc = AMDIL::PRIVATESTOREv2i32r;
    break;
  case AMDIL::GPRV2I8RegClassID:
    Opc = AMDIL::PRIVATESTOREv2i8r;
    break;
  case AMDIL::GPRV2I64RegClassID:
    Opc = AMDIL::PRIVATESTOREv2i64r;
    break;
  case AMDIL::GPRV4I16RegClassID:
    Opc = AMDIL::PRIVATESTOREv4i16r;
    break;
  case AMDIL::GPRV4I32RegClassID:
    Opc = AMDIL::PRIVATESTOREv4i32r;
    break;
  case AMDIL::GPRV4I8RegClassID:
    Opc = AMDIL::PRIVATESTOREv4i8r;
    break;
  }

  if (MI != MBB.end()) {
    DL = MI->getDebugLoc();
  }

  MachineMemOperand *MMO =
    new MachineMemOperand(
    MachinePointerInfo::getFixedStack(FrameIndex),
    MachineMemOperand::MOStore,
    MFI.getObjectSize(FrameIndex),
    MFI.getObjectAlignment(FrameIndex));

  if (MI != MBB.end()) {
    DL = MI->getDebugLoc();
  }

  MachineInstr *nMI = BuildMI(MBB, MI, DL, get(Opc))
      .addReg(SrcReg, getKillRegState(IsKill))
      .addFrameIndex(FrameIndex)
      .addMemOperand(MMO);
  AMDILMachineFunctionInfo *FuncInfo = MF.getInfo<AMDILMachineFunctionInfo>();
  FuncInfo->setUsesScratch();
  AMDILAS::InstrResEnc CurRes;
  CurRes.bits.ResourceID
    = TM.getSubtargetImpl()->getResourceID(AMDIL::SCRATCH_ID);
  setAsmPrinterFlags(nMI, CurRes);
}

void AMDILInstrInfo::emitAddri(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator MI,
                               unsigned DestReg,
                               unsigned BaseReg,
                               int imm,
                               unsigned RegClassID) const {
  assert((RegClassID == AMDIL::GPR_32RegClassID ||
          RegClassID == AMDIL::GPR_64RegClassID) && "unexpected reg class");
  unsigned Op = (RegClassID == AMDIL::GPR_32RegClassID)
                ? AMDIL::ADDi32ri : AMDIL::ADDi64ri;
  BuildMI(MBB, MI, DebugLoc(), get(Op), DestReg)
    .addReg(BaseReg)
    .addImm(imm);
}

void AMDILInstrInfo::emitSPUpdate(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator MI,
                                  int NumBytes) const {
  bool Is64Bit = TM.getSubtarget<AMDILSubtarget>().is64bit();
  unsigned RegClassID
    = Is64Bit ? AMDIL::GPR_64RegClassID : AMDIL::GPR_32RegClassID;
  emitAddri(MBB, MI, AMDIL::SP, AMDIL::SP, NumBytes, RegClassID);
}

void AMDILInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator MI,
                                          unsigned DestReg,
                                          int FrameIndex,
                                          const TargetRegisterClass *RC,
                                          const TargetRegisterInfo *TRI) const {
  unsigned Opc = 0;
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = *MF.getFrameInfo();
  DebugLoc DL;

  switch (RC->getID()) {
  default:
    Opc = AMDIL::PRIVATELOADv4i32r;
    break;
  case AMDIL::GPRI16RegClassID:
    Opc = AMDIL::PRIVATELOADi16r;
    break;
  case AMDIL::GPR_32RegClassID:
    Opc = AMDIL::PRIVATELOADi32r;
    break;
  case AMDIL::GPRI8RegClassID:
    Opc = AMDIL::PRIVATELOADi8r;
    break;
  case AMDIL::GPR_64RegClassID:
    Opc = AMDIL::PRIVATELOADi64r;
    break;
  case AMDIL::GPRV2I16RegClassID:
    Opc = AMDIL::PRIVATELOADv2i16r;
    break;
  case AMDIL::GPRV2I32RegClassID:
    Opc = AMDIL::PRIVATELOADv2i32r;
    break;
  case AMDIL::GPRV2I8RegClassID:
    Opc = AMDIL::PRIVATELOADv2i8r;
    break;
  case AMDIL::GPRV2I64RegClassID:
    Opc = AMDIL::PRIVATELOADv2i64r;
    break;
  case AMDIL::GPRV4I16RegClassID:
    Opc = AMDIL::PRIVATELOADv4i16r;
    break;
  case AMDIL::GPRV4I32RegClassID:
    Opc = AMDIL::PRIVATELOADv4i32r;
    break;
  case AMDIL::GPRV4I8RegClassID:
    Opc = AMDIL::PRIVATELOADv4i8r;
    break;
  }

  MachineMemOperand *MMO =
    new MachineMemOperand(
    MachinePointerInfo::getFixedStack(FrameIndex),
    MachineMemOperand::MOLoad,
    MFI.getObjectSize(FrameIndex),
    MFI.getObjectAlignment(FrameIndex));

  if (MI != MBB.end()) {
    DL = MI->getDebugLoc();
  }

  AMDILMachineFunctionInfo *mfinfo = MF.getInfo<AMDILMachineFunctionInfo>();
  mfinfo->setUsesScratch();
  MachineInstr *nMI = BuildMI(MBB, MI, DL, get(Opc))
      .addReg(DestReg, RegState::Define)
      .addFrameIndex(FrameIndex)
      .addMemOperand(MMO);
  AMDILAS::InstrResEnc CurRes;
  CurRes.bits.ResourceID
    = TM.getSubtargetImpl()->getResourceID(AMDIL::SCRATCH_ID);
  setAsmPrinterFlags(nMI, CurRes);

}
#if 0
MachineInstr *AMDILInstrInfo::foldMemoryOperandImpl(MachineFunction &MF,
    MachineInstr *MI,
    const SmallVectorImpl<unsigned> &Ops,
    int FrameIndex) const {
  // TODO: Implement this function
  return 0;
}

MachineInstr *AMDILInstrInfo::foldMemoryOperandImpl(MachineFunction &MF,
    MachineInstr *MI,
    const SmallVectorImpl<unsigned> &Ops,
    MachineInstr *LoadMI) const {
  // TODO: Implement this function
  return 0;
}
#endif

#if 0
bool AMDILInstrInfo::canFoldMemoryOperand(
  const MachineInstr *MI,
  const SmallVectorImpl<unsigned> &Ops) const {
  // TODO: Implement this function
  return TargetInstrInfoImpl::canFoldMemoryOperand(MI, Ops);
}

bool AMDILInstrInfo::unfoldMemoryOperand(
  MachineFunction &MF,
  MachineInstr *MI,
  unsigned Reg,
  bool UnfoldLoad,
  bool UnfoldStore,
  SmallVectorImpl<MachineInstr *> &NewMIs) const {
  // TODO: Implement this function
  return false;
}

bool AMDILInstrInfo::unfoldMemoryOperand(
  SelectionDAG &DAG,
  SDNode *N,
  SmallVectorImpl<SDNode *> &NewNodes) const {
  // TODO: Implement this function
  return false;
}

unsigned AMDILInstrInfo::getOpcodeAfterMemoryUnfold(
  unsigned Opc,
  bool UnfoldLoad,
  bool UnfoldStore,
  unsigned *LoadRegIndex) const {
  // TODO: Implement this function
  return 0;
}
#endif

bool AMDILInstrInfo::areLoadsFromSameBasePtr(SDNode *Load1,
                                             SDNode *Load2,
                                             int64_t &Offset1,
                                             int64_t &Offset2) const {
  if (!Load1->isMachineOpcode() || !Load2->isMachineOpcode()) {
    return false;
  }

  const MachineSDNode *MLoad1 = dyn_cast<MachineSDNode>(Load1);
  const MachineSDNode *MLoad2 = dyn_cast<MachineSDNode>(Load2);

  if (!MLoad1 || !MLoad2) {
    return false;
  }

  if (MLoad1->memoperands_empty() || MLoad2->memoperands_empty()) {
    return false;
  }

  MachineMemOperand *MemOp1 = *MLoad1->memoperands_begin();
  MachineMemOperand *MemOp2 = *MLoad2->memoperands_begin();
  const Value *MV1 = MemOp1->getValue();
  const Value *MV2 = MemOp2->getValue();

  if (MV1 == NULL || MV2 == NULL)
    return false;

  if (!MemOp1->isLoad() || !MemOp2->isLoad()) {
    return false;
  }

  const DataLayout *DL = TM.getDataLayout();

  SmallVector<Value *, 2> MV1Objects;
  SmallVector<Value *, 2> MV2Objects;

  GetUnderlyingObjects(const_cast<Value *>(MV1), MV1Objects, DL, 0);
  GetUnderlyingObjects(const_cast<Value *>(MV2), MV2Objects, DL, 0);

  if (MV1Objects.size() == 1 &&
      MV2Objects.size() == 1 &&
      MV1Objects.front() == MV2Objects.front()) {
    if (isa<GetElementPtrInst>(MV1) && isa<GetElementPtrInst>(MV2)) {
      const GetElementPtrInst *GEP1 = dyn_cast<GetElementPtrInst>(MV1);
      const GetElementPtrInst *GEP2 = dyn_cast<GetElementPtrInst>(MV2);

      if (!GEP1 || !GEP2) {
        return false;
      }

      if (GEP1->getNumOperands() != GEP2->getNumOperands()) {
        return false;
      }

      for (unsigned I = 0, E = GEP1->getNumOperands() - 1; I < E; ++I) {
        const Value *Op1 = GEP1->getOperand(I);
        const Value *Op2 = GEP2->getOperand(I);

        if (Op1 != Op2) {
          // If any value except the last one is different, return false.
          return false;
        }
      }

      unsigned Size = GEP1->getNumOperands() - 1;

      if (!isa<ConstantInt>(GEP1->getOperand(Size))
          || !isa<ConstantInt>(GEP2->getOperand(Size))) {
        return false;
      }

      Offset1 = dyn_cast<ConstantInt>(GEP1->getOperand(Size))->getSExtValue();
      Offset2 = dyn_cast<ConstantInt>(GEP2->getOperand(Size))->getSExtValue();
      return true;
    } else if (isa<Argument>(MV1) && isa<Argument>(MV2)) {
      return false;
    } else if (isa<GlobalValue>(MV1) && isa<GlobalValue>(MV2)) {
      return false;
    }
  }

  return false;
}

bool AMDILInstrInfo::shouldScheduleLoadsNear(SDNode *Load1,
                                             SDNode *Load2,
                                             int64_t Offset1,
                                             int64_t Offset2,
                                             unsigned NumLoads) const {
  LoadSDNode *LoadSD1 = dyn_cast<LoadSDNode>(Load1);
  LoadSDNode *LoadSD2 = dyn_cast<LoadSDNode>(Load2);

  if (!LoadSD1 || !LoadSD2) {
    return false;
  }

  // We only care about scheduling loads near for global address space.
  if (dyn_cast<PointerType>(LoadSD1->getSrcValue()->getType())
      ->getAddressSpace() != AMDILAS::GLOBAL_ADDRESS) {
    return false;
  }

  // We only care about scheduling loads near for global address space.
  if (dyn_cast<PointerType>(LoadSD2->getSrcValue()->getType())
      ->getAddressSpace() != AMDILAS::GLOBAL_ADDRESS) {
    return false;
  }

  assert(Offset2 > Offset1
      && "Second offset should be larger than first offset!");
  // If we have less than 16 loads in a row, and the offsets are within 16,
  // then schedule together.
  // TODO: Make the loads schedule near if it fits in a cacheline
  return (NumLoads < 16 && (Offset2 - Offset1) < 16);
}

#ifndef USE_APPLE
bool AMDILInstrInfo::shouldScheduleWithNormalPriority(SDNode *Instruction) const {
  if (Instruction->isMachineOpcode()) {
    switch (Instruction->getMachineOpcode()) {
    case AMDIL::BARRIER:
    case AMDIL::BARRIER_LOCAL:
    case AMDIL::BARRIER_GLOBAL:
    case AMDIL::BARRIER_REGION:
    case AMDIL::BARRIER_GLOBAL_LOCAL:
    case AMDIL::BARRIER_GLOBAL_REGION:
    case AMDIL::BARRIER_REGION_LOCAL:
    case AMDIL::FENCEr:
    case AMDIL::FENCE_Lr:
    case AMDIL::FENCE_Mr:
    case AMDIL::FENCE_Gr:
    case AMDIL::FENCE_LMr:
    case AMDIL::FENCE_LGr:
    case AMDIL::FENCE_MGr:
    case AMDIL::FENCE_ROr:
    case AMDIL::FENCE_RO_Lr:
    case AMDIL::FENCE_RO_Mr:
    case AMDIL::FENCE_RO_Gr:
    case AMDIL::FENCE_RO_LMr:
    case AMDIL::FENCE_RO_LGr:
    case AMDIL::FENCE_RO_MGr:
    case AMDIL::FENCE_WOr:
    case AMDIL::FENCE_WO_Lr:
    case AMDIL::FENCE_WO_Mr:
    case AMDIL::FENCE_WO_Gr:
    case AMDIL::FENCE_WO_LMr:
    case AMDIL::FENCE_WO_LGr:
    case AMDIL::FENCE_WO_MGr:
    case AMDIL::FENCE_Sr:
    case AMDIL::FENCE_MSr:
    case AMDIL::FENCE_LSr:
    case AMDIL::FENCE_GSr:
    case AMDIL::FENCE_LMSr:
    case AMDIL::FENCE_MGSr:
    case AMDIL::FENCE_LGSr:
      return true;  // Maybe other instructions will need to be added to this?
    default:
      return false;
    }
  }

  return false;
}
#endif

bool AMDILInstrInfo::ReverseBranchCondition(
  SmallVectorImpl<MachineOperand> &Cond) const {
  // TODO: Implement this function
  return true;
}

void AMDILInstrInfo::insertNoop(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const {
  // TODO: Implement this function
}

bool AMDILInstrInfo::isPredicated(const MachineInstr *MI) const {
  // TODO: Implement this function
  return false;
}

bool AMDILInstrInfo::isUnpredicatedTerminator(const MachineInstr *MI) const {
  if (!MI->isTerminator()) {
    return false;
  }

  // Conditional branch is a special case.
  if (MI->isBranch() && !MI->isBarrier()) {
    return true;
  }

  if (!MI->isPredicable()) {
    return true;
  }

  return !isPredicated(MI);
}

bool AMDILInstrInfo::PredicateInstruction(
  MachineInstr *MI,
  const SmallVectorImpl<MachineOperand> &Pred) const {
  // TODO: Implement this function
  return false;
}

bool AMDILInstrInfo::SubsumesPredicate(
  const SmallVectorImpl<MachineOperand> &Pred1,
  const SmallVectorImpl<MachineOperand> &Pred2) const {
  // TODO: Implement this function
  return false;
}

bool AMDILInstrInfo::DefinesPredicate(MachineInstr *MI,
    std::vector<MachineOperand> &Pred) const {
  // TODO: Implement this function
  return false;
}

bool AMDILInstrInfo::isPredicable(MachineInstr *MI) const {
  // TODO: Implement this function
  return MI->getDesc().isPredicable();
}

bool AMDILInstrInfo::isSafeToMoveRegClassDefs(const TargetRegisterClass *RC) const {
  // TODO: Implement this function
  return true;
}

unsigned AMDILInstrInfo::GetInstSizeInBytes(const MachineInstr *MI) const {
  // TODO: Implement this function
  return 0;
}

unsigned AMDILInstrInfo::GetFunctionSizeInBytes(const MachineFunction &MF) const {
  // TODO: Implement this function
  return 0;
}

unsigned AMDILInstrInfo::getInlineAsmLength(const char *Str,
    const MCAsmInfo &MAI) const {
  // TODO: Implement this function
  return 0;
}

