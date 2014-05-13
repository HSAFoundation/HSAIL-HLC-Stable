//===----------------------- AMDILFrameLowering.cpp -----------------*- C++ -*-===//
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
// Interface to describe a layout of a stack frame on a AMDIL target machine
//
//===----------------------------------------------------------------------===//
#include "AMDILFrameLowering.h"
#include "AMDILInstrInfo.h"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;
AMDILFrameLowering::AMDILFrameLowering(StackDirection D, unsigned StackAl,
    int LAO, unsigned TransAl)
  : TargetFrameLowering(D, StackAl, LAO, TransAl) {

}

AMDILFrameLowering::~AMDILFrameLowering() {

}

// Returns if the given frame index is for storing a callee saved register
static bool isCSRFrameIndex(const MachineFunction &MF, int FI) {
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  const std::vector<CalleeSavedInfo> &CSI = MFI->getCalleeSavedInfo();
  for (std::vector<CalleeSavedInfo>::const_iterator I = CSI.begin(),
    E = CSI.end(); I != E; ++I) {
    if (I->getFrameIdx() == FI) {
      return true;
    }
  }
  return false;
}

/// getFrameIndexOffset - Returns the base register + displacement from
// the base register to the stack frame of the specified index.
int AMDILFrameLowering::getFrameIndexReference(
  const MachineFunction &MF, int FI, unsigned &FrameReg) const {
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  const TargetRegisterInfo *RegInfo = MF.getTarget().getRegisterInfo();
  const AMDILMachineFunctionInfo *FuncInfo
    = MF.getInfo<AMDILMachineFunctionInfo>();
  FrameReg = AMDIL::SP;
  int Offset = MFI->getObjectOffset(FI);

  assert(hasReservedCallFrame(MF) && "unimplemented");
  if (hasFP(MF)) {
    // Callee saved registeres are pushed to the stack before stack pointers
    // are updated. So the offset is getObjectOffset(FI)-CallFrameSize
    if (isCSRFrameIndex(MF, FI)) {
      return Offset - FuncInfo->getBytesToPopOnReturn();
    }
    // If stack pointer has been realigned, need to use frame pointer to
    // access stack args pushed by the caller, which is below the frame pointer.
    if (FI < 0 && RegInfo->needsStackRealignment(MF)) {
      FrameReg = RegInfo->getFrameRegister(MF);
      return Offset - FuncInfo->getBytesToPopOnReturn();
    }
  }

  // SP has been updated to top of stack at function prolog. Since our stack
  // grows upwards, start of current stack frame is at SP-StackSize.
  return Offset - MFI->getStackSize();
}

/// getFrameIndexOffset - Returns the displacement from the frame register to
/// the stack frame of the specified index.
int AMDILFrameLowering::getFrameIndexOffset(const MachineFunction &MF,
                                            int FI) const {
  unsigned FrameReg;
  return getFrameIndexReference(MF, FI, FrameReg);
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  // SP has been updated to top of stack at function prolog. Since out stack
  // grows upwards, start of current stack frame is at SP-StackSize.
  return MFI->getObjectOffset(FI) - MFI->getStackSize();
}

#if 0
uint64_t AMDILFrameLowering::getStackSizeWOCallFrame(
  const MachineFunction &MF) const {
  if (targetHandlesStackFrameRounding())
    return 0;

  const MachineFrameInfo *MFI = MF.getFrameInfo();
  // If we have reserved argument space for call sites in the function
  // immediately on entry to the current function, count it as part of the
  // overall stack size.
  if (!MFI->adjustsStack() || !hasReservedCallFrame(MF))
  return 0;

  uint64_t MaxFrameSize = MFI->getMaxCallFrameSize();

  // Round up the size to a multiple of the alignment.  If the function has
  // any calls or alloca's, align to the target's StackAlignment value to
  // ensure that the callee's frame or the alloca data is suitably aligned;
  // otherwise, for leaf functions, align to the TransientStackAlignment
  // value.

  // If the frame pointer is eliminated, all frame offsets will be relative to
  // SP not FP. Align to MaxAlign so this works.
  unsigned StackAlign
    = std::max(getStackAlignment(), MFI->getMaxAlignment());
  unsigned AlignMask = StackAlign - 1;
  MaxFrameSize = (MaxFrameSize + AlignMask) & ~uint64_t(AlignMask);
  return MFI->getStackSize() - MaxFrameSize;
}
#endif

void AMDILFrameLowering::emitPrologue(MachineFunction &MF) const {
  MachineBasicBlock &MBB = MF.front();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  const AMDILInstrInfo &TII =
      *static_cast<const AMDILInstrInfo*>(MF.getTarget().getInstrInfo());
  AMDILMachineFunctionInfo *FuncInfo
      = MF.getInfo<AMDILMachineFunctionInfo>();
  const TargetRegisterInfo *RegInfo = MF.getTarget().getRegisterInfo();

  // Add the prolog code after the store of callee saved registers,
  // because FP needs to be preserved first before it is updated.
  MachineBasicBlock::iterator MBBI, E;
  for (MBBI = MBB.begin(), E = MBB.end(); MBBI != E; ++MBBI) {
    int FI;
    if (!(TII.isStoreToStackSlot(MBBI, FI) && isCSRFrameIndex(MF, FI))) {
      break;
    }
  }

  // If frame pointer is not emilinated, setup the frame pointer
  if (hasFP(MF)) {
    unsigned FramePtr = RegInfo->getFrameRegister(MF);
    TII.copyPhysReg(MBB, MBBI, DebugLoc(), FramePtr, AMDIL::SP, false);
  }
  assert(!MFI->hasVarSizedObjects() && "unsupported");

  // StackSize includes bytes pushed by caller as call arguments.
  // So only need to adjust SP by StackSize - BytesCallerPushed
  unsigned NumBytesCallerPushed = FuncInfo->getBytesToPopOnReturn();
  uint64_t Size = MFI->getStackSize() - NumBytesCallerPushed;

  // Realign the SP: step 1: SP += (MaxAlign-1)
  if (RegInfo->needsStackRealignment(MF)) {
    assert(hasFP(MF) && "frame pointer eliminated");
    Size += MFI->getMaxAlignment() - 1;
  }

  // Adjust the stack pointer
  if (Size != 0) {
    TII.emitSPUpdate(MBB, MBBI, Size);
  }

  // Realign the SP: step 2: SP &= ~(MaxAlign-1)
  if (RegInfo->needsStackRealignment(MF)) {
    unsigned Mask = ~(MFI->getMaxAlignment() - 1);
    BuildMI(MBB, MBBI, DebugLoc(), TII.get(AMDIL::ANDi32ri), AMDIL::SP)
      .addReg(AMDIL::SP)
      .addImm(Mask);
  }
}

void AMDILFrameLowering::emitEpilogue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  MachineFrameInfo *MFI = MF.getFrameInfo();
  const AMDILInstrInfo &TII =
      *static_cast<const AMDILInstrInfo*>(MF.getTarget().getInstrInfo());
  AMDILMachineFunctionInfo *FuncInfo
      = MF.getInfo<AMDILMachineFunctionInfo>();
  const TargetRegisterInfo *RegInfo = MF.getTarget().getRegisterInfo();

  // Insert the epilog code before the restore of callee saved registers,
  // because FP needs to be copied to SP first before it is restored.
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  assert(MBBI->isReturn() && "Can only insert epilog into returning blocks");
  MachineBasicBlock::iterator E = MBB.begin();
  for (;;) {
    int FI;
    if (!(TII.isLoadFromStackSlot(MBBI, FI) && isCSRFrameIndex(MF, FI)) &&
      !MBBI->isReturn()) {
      ++MBBI;
      break;
    }
    if (MBBI == E)
      break;
    --MBBI;
  }

  // If frame pointer is not emilinated, restore the stach pointer from
  // the frame pointer
  if (hasFP(MF)) {
    unsigned FramePtr = RegInfo->getFrameRegister(MF);
    TII.copyPhysReg(MBB, MBBI, DebugLoc(), AMDIL::SP, FramePtr, false);
  } else {
  // StackSize includes bytes pushed by caller as call arguments.
  // So only need to update SP with StackSize - BytesCallerPushed
  unsigned NumBytesCallerPushed = FuncInfo->getBytesToPopOnReturn();
  uint64_t Size = MFI->getStackSize() - NumBytesCallerPushed;
  if (Size != 0) {
    TII.emitSPUpdate(MBB, MBBI, -Size);
  }
  }
}

bool AMDILFrameLowering::hasFP(const MachineFunction &MF) const {
  // assert that stack frames are always eliminated
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  const TargetRegisterInfo *RegInfo = MF.getTarget().getRegisterInfo();
  bool CannotEliminateFramePointer =
    MF.getTarget().Options.DisableFramePointerElim(MF) ||
    RegInfo->needsStackRealignment(MF) || MFI->hasVarSizedObjects() ||
    MFI->isFrameAddressTaken();
  return CannotEliminateFramePointer;
}

bool AMDILFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  return !MFI->hasVarSizedObjects();
}

void AMDILFrameLowering::processFunctionBeforeCalleeSavedScan(
  MachineFunction &MF, RegScavenger *RS) const {
  const TargetRegisterInfo *RegInfo = MF.getTarget().getRegisterInfo();

  // Spill the frame pointer if it's used.
  if (hasFP(MF))
    MF.getRegInfo().setPhysRegUsed(RegInfo->getFrameRegister(MF));
}
