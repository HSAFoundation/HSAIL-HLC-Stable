//===- AMDILInstrInfo.h - AMDIL Instruction Information ---------*- C++ -*-===//
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
// to the U.S. Export Administration Regulations (“EAR”), (15 C.F.R. Sections
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
// Industry and Security’s website at http://www.bis.doc.gov/.
//
//==-----------------------------------------------------------------------===//
//
// This file contains the AMDIL implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef AMDILINSTRUCTIONINFO_H_
#define AMDILINSTRUCTIONINFO_H_

#include "AMDIL.h"
#include "llvm/Target/TargetInstrInfo.h"

#include "AMDILRegisterInfo.h"

#define GET_INSTRINFO_HEADER
#include "AMDILGenInstrInfo.inc"

namespace llvm {
  // AMDIL - This namespace holds all of the target specific flags that
  // instruction info tracks.
  //
  //class AMDILTargetMachine;
class AMDILInstrInfo : public AMDILGenInstrInfo {
private:
  const AMDILRegisterInfo RI;
  AMDILTargetMachine &TM;
  bool getNextBranchInstr(MachineBasicBlock::iterator &Iter,
                          MachineBasicBlock &MBB) const;
  unsigned getBranchInstr(const MachineOperand &Op) const;
public:
  explicit AMDILInstrInfo(AMDILTargetMachine &tm);

  // getRegisterInfo - TargetInstrInfo is a superset of MRegister
  // info.  As such, whenever a client has an instance of instruction
  // info, it should always be able to get register info as well
  // (through this method).
  const AMDILRegisterInfo &getRegisterInfo() const;

  // Return true if the instruction is a register to register move and
  // leave the source and dest operands in the passed parameters.
  bool isMoveInstr(const MachineInstr &MI,
                   unsigned &SrcReg,
                   unsigned &DstReg,
                   unsigned &SrcSubIdx,
                   unsigned &DstSubIdx) const;

  bool isCoalescableExtInstr(const MachineInstr &MI,
                             unsigned &SrcReg,
                             unsigned &DstReg,
                             unsigned &SubIdx) const;

  unsigned isLoadFromStackSlot(const MachineInstr *MI,
                               int &FrameIndex) const;
  unsigned isLoadFromStackSlotPostFE(const MachineInstr *MI,
                                     int &FrameIndex) const;
  bool hasLoadFromStackSlot(const MachineInstr *MI,
                            const MachineMemOperand *&MMO,
                            int &FrameIndex) const;
  unsigned isStoreToStackSlot(const MachineInstr *MI,
                              int &FrameIndex) const;
  unsigned isStoreToStackSlotPostFE(const MachineInstr *MI,
                                      int &FrameIndex) const;
  bool hasStoreToStackSlot(const MachineInstr *MI,
                           const MachineMemOperand *&MMO,
                           int &FrameIndex) const;

  void reMaterialize(MachineBasicBlock &MBB,
                     MachineBasicBlock::iterator MI,
                     unsigned DestReg, unsigned SubIdx,
                     const MachineInstr *Orig,
                     const TargetRegisterInfo &TRI) const;

  MachineInstr *duplicate(MachineInstr *Orig,
                          MachineFunction &MF) const;

  MachineInstr *convertToThreeAddress(MachineFunction::iterator &MFI,
                                      MachineBasicBlock::iterator &MBBI,
                                      LiveVariables *LV) const;

  MachineInstr *commuteInstruction(MachineInstr *MI,
                                   bool NewMI = false) const;
  bool findCommutedOpIndices(MachineInstr *MI, unsigned &SrcOpIdx1,
                             unsigned &SrcOpIdx2) const;
  bool produceSameValue(const MachineInstr *MI0,
                        const MachineInstr *MI1,
                        const MachineRegisterInfo *MRI = NULL) const;

  bool AnalyzeBranch(MachineBasicBlock &MBB,
                     MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify) const;

  unsigned RemoveBranch(MachineBasicBlock &MBB) const;

  unsigned InsertBranch(MachineBasicBlock &MBB,
                        MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB,
                        const SmallVectorImpl<MachineOperand> &Cond,
                        DebugLoc DL) const;

  virtual bool isProfitableToIfCvt(
    MachineBasicBlock &MBB, unsigned, unsigned,
    const BranchProbability &) const LLVM_OVERRIDE;

  virtual bool
  isProfitableToIfCvt(MachineBasicBlock &TMBB,
                      unsigned NumTCycles, unsigned ExtraTCycles,
                      MachineBasicBlock &FMBB,
                      unsigned NumFCycles, unsigned ExtraFCycles,
                      const BranchProbability &) const LLVM_OVERRIDE;

  virtual bool
  isProfitableToDupForIfCvt(MachineBasicBlock &MBB, unsigned NumCycles,
                            const BranchProbability &) const LLVM_OVERRIDE;

  virtual bool isProfitableToUnpredicate(
    MachineBasicBlock &,
    MachineBasicBlock &) const LLVM_OVERRIDE;

  bool copyRegToReg(MachineBasicBlock &MBB,
                    MachineBasicBlock::iterator I,
                    unsigned DestReg, unsigned SrcReg,
                    const TargetRegisterClass *DestRC,
                    const TargetRegisterClass *SrcRC,
                    DebugLoc DL) const;
  void copyPhysReg(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI, DebugLoc DL,
                           unsigned DestReg, unsigned SrcReg,
                           bool KillSrc) const;

  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI,
                           unsigned SrcReg,
                           bool IsKill,
                           int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI) const;
  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI,
                            unsigned DestReg,
                            int FrameIndex,
                            const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI) const;
  void emitAddri(MachineBasicBlock &MBB,
                 MachineBasicBlock::iterator MI,
                 unsigned DestReg,
                 unsigned BaseReg,
                 int imm,
                 unsigned RegClassID) const;
  void emitSPUpdate(MachineBasicBlock &MBB,
                    MachineBasicBlock::iterator MI,
                    int NumBytes) const;
protected:
#if 0
  MachineInstr *foldMemoryOperandImpl(MachineFunction &MF,
                                      MachineInstr *MI,
                                      const SmallVectorImpl<unsigned> &Ops,
                                      int FrameIndex) const;
  MachineInstr *foldMemoryOperandImpl(MachineFunction &MF,
                                      MachineInstr *MI,
                                      const SmallVectorImpl<unsigned> &Ops,
                                      MachineInstr *LoadMI) const;
#endif
public:
#if 0
  bool canFoldMemoryOperand(const MachineInstr *MI,
                            const SmallVectorImpl<unsigned> &Ops) const;
  bool unfoldMemoryOperand(MachineFunction &MF,
                           MachineInstr *MI,
                           unsigned Reg,
                           bool UnfoldLoad,
                           bool UnfoldStore,
                           SmallVectorImpl<MachineInstr *> &NewMIs) const;
  bool unfoldMemoryOperand(SelectionDAG &DAG,
                           SDNode *N,
                           SmallVectorImpl<SDNode *> &NewNodes) const;
  unsigned getOpcodeAfterMemoryUnfold(unsigned Opc,
                                      bool UnfoldLoad,
                                      bool UnfoldStore,
                                      unsigned *LoadRegIndex = 0) const;
#endif
  bool areLoadsFromSameBasePtr(SDNode *Load1,
                               SDNode *Load2,
                               int64_t &Offset1,
                               int64_t &Offset2) const;
  bool shouldScheduleLoadsNear(SDNode *Load1,
                               SDNode *Load2,
                               int64_t Offset1,
                               int64_t Offset2,
                               unsigned NumLoads) const;

#ifndef USE_APPLE
  /// Schedule BARRIER instructions differently.
  /// Schedule this instruction based entirely on it's Sethi-Ullman number,
  /// without raising or lowering it's priority based on use or def numbers.
  /// What this really says is that the instruction has some effect on execution
  /// that is not modeled in the DAG. (For instance, a multi-thread execution
  /// barrier.) On the GPU AMDIL backend, moving these instructions too far up
  /// or down in the execution can artificially constrain the scheduling in the
  /// shared compiler.
  bool shouldScheduleWithNormalPriority(SDNode* Instruction) const;
#endif

  bool ReverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const;
  void insertNoop(MachineBasicBlock &MBB,
                  MachineBasicBlock::iterator MI) const;
  bool isPredicated(const MachineInstr *MI) const;

  bool isUnpredicatedTerminator(const MachineInstr *MI) const;
  bool PredicateInstruction(MachineInstr *MI,
                            const SmallVectorImpl<MachineOperand> &Pred) const;

  bool SubsumesPredicate(const SmallVectorImpl<MachineOperand> &Pred1,
                         const SmallVectorImpl<MachineOperand> &Pred2) const;
  bool DefinesPredicate(MachineInstr *MI,
                        std::vector<MachineOperand> &Pred) const;
  bool isPredicable(MachineInstr *MI) const;
  bool isSafeToMoveRegClassDefs(const TargetRegisterClass *RC) const;
  unsigned GetInstSizeInBytes(const MachineInstr *MI) const;

  unsigned GetFunctionSizeInBytes(const MachineFunction &MF) const;
  unsigned getInlineAsmLength(const char *Str,
                              const MCAsmInfo &MAI) const;
  };
}

#endif // AMDILINSTRINFO_H_
