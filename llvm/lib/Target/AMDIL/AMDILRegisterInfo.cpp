//===- AMDILRegisterInfo.cpp - AMDIL Register Information -------*- C++ -*-===//
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
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the AMDIL implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "AMDILRegisterInfo.h"
#include "AMDIL.h"
#include "AMDILUtilityFunctions.h"
#include "llvm/Function.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/FormattedStream.h"

#define GET_REGINFO_MC_DESC
#define GET_REGINFO_TARGET_DESC
#include "AMDILGenRegisterInfo.inc"


using namespace llvm;

AMDILRegisterInfo::AMDILRegisterInfo(AMDILTargetMachine &TM_,
    const TargetInstrInfo &TII_)
: AMDILGenRegisterInfo(0), // RA???
  TM(TM_), TII(TII_) {
  baseOffset = 0;
  nextFuncOffset = 0;
}

const uint16_t *AMDILRegisterInfo::getCalleeSavedRegs(
  const MachineFunction *MF) const {
  return CSR_CSR_SaveList;
}

const uint32_t* AMDILRegisterInfo::getCallPreservedMask(
  CallingConv::ID CC) const {
  return CSR_CC_RegMask;
}

BitVector AMDILRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
   const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();
  // Set the frame pointer, stack pointer, RA, Stack Data Pointer as reserved.
  if (TFI->hasFP(MF)) {
    Reserved.set(AMDIL::FP);
  }
  Reserved.set(AMDIL::SP);
  Reserved.set(AMDIL::SDP);
  Reserved.set(AMDIL::RA);

  // Set temps T1-T5 as reserved.
  Reserved.set(AMDIL::T1);
  Reserved.set(AMDIL::T2);
  Reserved.set(AMDIL::T3);
  Reserved.set(AMDIL::T4);
  Reserved.set(AMDIL::T5);

  // Set the mem register as reserved.
  Reserved.set(AMDIL::MEMx);
  Reserved.set(AMDIL::MEMxy);
  Reserved.set(AMDIL::MEM);

  // Set CFG1-CFG10 as reserved.
  Reserved.set(AMDIL::CFG1);
  Reserved.set(AMDIL::CFG2);
  Reserved.set(AMDIL::CFG3);
  Reserved.set(AMDIL::CFG4);
  Reserved.set(AMDIL::CFG5);
  Reserved.set(AMDIL::CFG6);
  Reserved.set(AMDIL::CFG7);
  Reserved.set(AMDIL::CFG8);
  Reserved.set(AMDIL::CFG9);
  Reserved.set(AMDIL::CFG10);

  // Set PRINTF register as reserved.
  Reserved.set(AMDIL::PRINTF);
#if 0
  // Reserve the live-ins for the function.
  MachineBasicBlock::livein_iterator LII = MF.begin()->livein_begin();
  MachineBasicBlock::livein_iterator LIE = MF.begin()->livein_end();
  while (LII != LIE) {
    Reserved.set(*LII);
    ++LII;
  }
#endif
  return Reserved;
}

const TargetRegisterClass* const*
AMDILRegisterInfo::getCalleeSavedRegClasses(const MachineFunction *MF) const {
  static const TargetRegisterClass * const CalleeSavedRegClasses[] = { 0 };
  // TODO: Keep in sync with getCalleeSavedRegs
  //TODO(getCalleeSavedRegClasses);
  return CalleeSavedRegClasses;
}

void AMDILRegisterInfo::eliminateCallFramePseudoInstr(
    MachineFunction &MF,
    MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();
  const AMDILFrameLowering *AFL = static_cast<const AMDILFrameLowering*>(TFI);
  const AMDILInstrInfo *AII = static_cast<const AMDILInstrInfo*>(&TII);
  if (!AFL->hasReservedCallFrame(MF)) {
    // If the stack pointer can be changed after prologue, turn the
    // adjcallstackup instruction into a 'iadd SP, -<amt>' and the
    // adjcallstackdown instruction into 'iadd SP, <amt>'
    MachineInstr *Old = I;
    DebugLoc DL = Old->getDebugLoc();
    unsigned Amount = Old->getOperand(0).getImm();
    if (Amount != 0) {
      // We need to keep the stack aligned properly.  To do this, we round the
      // amount of space needed for the outgoing arguments up to the next
      // alignment boundary.
      unsigned Align = TFI->getStackAlignment();
      Amount = (Amount+Align-1)/Align*Align;
      unsigned Opc = Old->getOpcode();
      if (Opc == AMDIL::ADJCALLSTACKDOWN) {
        AII->emitSPUpdate(MBB, I, Amount);
      } else {
        assert(Opc == AMDIL::ADJCALLSTACKUP);
        AII->emitSPUpdate(MBB, I, -Amount);
      }
    }
  }
  MBB.erase(I);
}

// For each frame index we find, we store the offset in the stack which is
// being pushed back into the global buffer. The offset into the stack where
// the value is stored is copied into a new register and the frame index is
// then replaced with that register.
void AMDILRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                            int SPAdj,
                                            RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");
  MachineInstr &MI = *II;
  MachineBasicBlock &MB = *MI.getParent();
  MachineFunction &MF = *MB.getParent();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();
  DebugLoc DL = MI.getDebugLoc();
  bool Def = isPtrStoreInst(&MI);
  for (unsigned I = 0, N = MI.getNumOperands(); I < N; ++I) {
    if (!MI.getOperand(I).isFI()) {
      continue;
    }
    int FrameIndex = MI.getOperand(I).getIndex();
    unsigned FrameReg;
    int64_t Offset = TFI->getFrameIndexReference(MF, FrameIndex, FrameReg);
    unsigned Reg;
    if (MI.getOpcode() != AMDIL::LOADFIi32) {
      Reg = (Def && !I) ? AMDIL::T5 : AMDIL::DFP;
    } else {
      assert(MI.getOperand(0).isDef() && MI.getOperand(0).isReg() &&
             "unexpected LOADFI format");
      Reg = MI.getOperand(0).getReg();
    }
    // If using FP instead of SP as base address, create ADDri instead of
    // LOADFI, because LOADFI uses SP as base address.
    if (FrameReg != AMDIL::SP) {
      BuildMI(MB, II, DL, TII.get(AMDIL::ADDi32ri), Reg)
        .addReg(FrameReg)
        .addImm(Offset);
    } else if (MI.getOpcode() != AMDIL::LOADFIi32) {
      BuildMI(MB, II, DL, TII.get(AMDIL::LOADFIi32), Reg).addImm(Offset);
    }
    //int64_t Size = MF.getFrameInfo()->getObjectSize(FrameIndex);
    // An optimization is to only use the offsets if the size
    // is larger than 4, which means we are storing an array
    // instead of just a pointer. If we are size 4 then we can
    // just do register copies since we don't need to worry about
    // indexing dynamically
    // FIXME: This needs to embed the literals directly instead of
    // using DFP.
    if (MI.getOpcode() != AMDIL::LOADFIi32) {
      assert(!MI.getOperand(I).isReg());
      MI.getOperand(I).ChangeToRegister(Reg, false);
    } else {
      if (FrameReg != AMDIL::SP) {
        MI.eraseFromParent();
      }
      MI.getOperand(1).ChangeToImmediate(Offset);
      break;
    }
  }
}
const TargetRegisterClass * AMDILRegisterInfo::getPointerRegClass(
  const MachineFunction &MF,
  unsigned Kind) const {
  assert(!Kind && "Unknown register class pointer specified!");
  return TM.getSubtargetImpl()->is64bit()
       ? &AMDIL::GPR_64RegClass
       : &AMDIL::GPR_32RegClass;
}

void AMDILRegisterInfo::processFunctionBeforeFrameFinalized(
  MachineFunction &MF) const {
  //TODO(processFunctionBeforeFrameFinalized);
  // Here we keep track of the amount of stack that the current function
  // uses so
  // that we can set the offset to the end of the stack and any other
  // function call
  // will not overwrite any stack variables.
  // baseOffset = nextFuncOffset;
  MachineFrameInfo *MFI = MF.getFrameInfo();

  for (uint32_t I = 0, N = MFI->getNumObjects(); I < N; ++I) {
    int64_t Size = MFI->getObjectSize(I);
    if (!(Size % 4) && Size > 1) {
      nextFuncOffset += Size;
    } else {
      nextFuncOffset += 16;
    }
  }
}

unsigned AMDILRegisterInfo::getRARegister() const {
  return AMDIL::RA;
}

unsigned AMDILRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();
  return TFI->hasFP(MF) ? AMDIL::FP : AMDIL::SP;
}

unsigned AMDILRegisterInfo::getEHExceptionRegister() const {
  llvm_unreachable("What is the exception register");
  return 0;
}

unsigned AMDILRegisterInfo::getEHHandlerRegister() const {
  llvm_unreachable("What is the exception handler register");
  return 0;
}

int64_t AMDILRegisterInfo::getStackSize() const {
  return nextFuncOffset - baseOffset;
}

bool AMDILRegisterInfo::canRealignStack(const MachineFunction &MF) const {
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  const MachineRegisterInfo *MRI = &MF.getRegInfo();
  if (!MF.getTarget().Options.RealignStack)
    return false;

  // Stack realignment requires a frame pointer.  If we already started
  // register allocation with frame pointer elimination, it is too late now.
  if (!MRI->canReserveReg(AMDIL::FP))
    return false;

  assert(!MFI->hasVarSizedObjects() && "unsupported");
  return true;
}

bool AMDILRegisterInfo::needsStackRealignment(const MachineFunction &MF)
  const {
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  const Function *F = MF.getFunction();
  unsigned StackAlign = MF.getTarget().getFrameLowering()->getStackAlignment();
  bool requiresRealignment =
    ((MFI->getMaxAlignment() > StackAlign) ||
     F->getFnAttributes().hasAttribute(Attributes::StackAlignment));

  return requiresRealignment && canRealignStack(MF);
}

const TargetRegisterClass *AMDILRegisterInfo::getRegClassFromID(unsigned ID) {
  switch (ID) {
  default:
    llvm_unreachable("Passed in ID does not match any register classes.");
    return NULL;
  case AMDIL::GPR_32RegClassID:
    return &AMDIL::GPR_32RegClass;
  case AMDIL::GPR_64RegClassID:
    return &AMDIL::GPR_64RegClass;
  case AMDIL::GPRV4I32RegClassID:
    return &AMDIL::GPRV4I32RegClass;
  case AMDIL::GPRV2I32RegClassID:
    return &AMDIL::GPRV2I32RegClass;
  case AMDIL::GPRV2I64RegClassID:
    return &AMDIL::GPRV2I64RegClass;
  }
}

const TargetRegisterClass *AMDILRegisterInfo::getRegClassFromType(MVT VT) {
  switch (VT.SimpleTy) {
  default:
      llvm_unreachable("Passed in type does not match any register classes.");
  case MVT::i32:
  case MVT::f32:
    return &AMDIL::GPR_32RegClass;
  case MVT::i64:
  case MVT::f64:
    return &AMDIL::GPR_64RegClass;
  case MVT::v4i32:
  case MVT::v4f32:
    return &AMDIL::GPRV4I32RegClass;
  case MVT::v2i32:
  case MVT::v2f32:
    return &AMDIL::GPRV2I32RegClass;
  case MVT::v2i64:
  case MVT::v2f64:
    return &AMDIL::GPRV2I64RegClass;
  }
}

unsigned AMDILRegisterInfo::getRegClassFromName(StringRef Name) {
  if (Name.find("v4i32") != StringRef::npos) {
    return AMDIL::GPRV4I32RegClassID;
  } else if (Name.find("v2i32") != StringRef::npos) {
    return AMDIL::GPRV2I32RegClassID;
  } else if (Name.find("i32") != StringRef::npos) {
    return AMDIL::GPR_32RegClassID;
  } else if (Name.find("v4f32") != StringRef::npos) {
    return AMDIL::GPRV4I32RegClassID;
  } else if (Name.find("v2f32") != StringRef::npos) {
    return AMDIL::GPRV2I32RegClassID;
  } else if (Name.find("f32") != StringRef::npos) {
    return AMDIL::GPR_32RegClassID;
  } else if (Name.find("v2i64") != StringRef::npos) {
    return AMDIL::GPRV2I64RegClassID;
  } else if (Name.find("i64") != StringRef::npos) {
    return AMDIL::GPR_64RegClassID;
  } else if (Name.find("v2f64") != StringRef::npos) {
    return AMDIL::GPRV2I64RegClassID;
  } else if (Name.find("f64") != StringRef::npos) {
    return AMDIL::GPR_64RegClassID;
  } else {
    //llvm_unreachable("Found a Name that I couldn't determine a class for!");
    return AMDIL::GPR_32RegClassID;
  }
}
