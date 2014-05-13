//===--- AMDILRegisterUseValidate.cpp - AMDIL Register Use Validate ------===//
//===--- *- C++ -*                                                      --===//
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

#define DEBUG_TYPE "register_validate"
#include "llvm/ADT/SetOperations.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"
#include "AMDIL.h"
#include "AMDILAlgorithms.tpp"
#include "AMDILAsmPrinter.h"
#include "AMDILRegisterInfo.h"
#include "AMDILTargetMachine.h"
#include "AMDILUtilityFunctions.h"

using namespace llvm;
using namespace AMDIL;

// Validate there is no def-use of registers across function call
// This is to validate "scratch" reserved registers are not used across
// function calls and that read-only reserved registers are not ever
// written-to. This is to ensure that it's safe to not re-number them
// in callees.
namespace {
  class AMDILRegisterUseValidate : public MachineFunctionPass {
    typedef llvm::DenseSet<unsigned> RegSet;

  public:
    static char ID;
    AMDILRegisterUseValidate(TargetMachine &tm, CodeGenOpt::Level OL);
    virtual const char *getPassName() const;

    bool runOnMachineFunction(MachineFunction &MF);
  private:
    bool trace(const MachineBasicBlock& bBlock, RegSet &bbLiveReg);

  private:
    TargetMachine &TM;
    const TargetRegisterClass *tempRegClass;
    const TargetRegisterClass *readonlyRegClass;
  };
  char AMDILRegisterUseValidate::ID = 0;
}

namespace llvm {
  FunctionPass *createAMDILRegisterUseValidate(TargetMachine &TM_,
                                          CodeGenOpt::Level OL) {
    return new AMDILRegisterUseValidate(TM_, OL);
  }
}

template<typename T>
raw_ostream& operator<<(raw_ostream& O, DenseSet<T> set) {
  for (typename DenseSet<T>::iterator I = set.begin(), E = set.end(); I != E; ++I) {
    O << *I << " ";
  }
  return O;
}

AMDILRegisterUseValidate::AMDILRegisterUseValidate(TargetMachine &TM_,
                                         CodeGenOpt::Level OL)
  : MachineFunctionPass(ID),
    TM(TM_) {
  tempRegClass =  TM.getRegisterInfo()->getRegClass(
      AMDIL::ReservedTempRegClassID);
  readonlyRegClass = TM.getRegisterInfo()->getRegClass(
      AMDIL::ReservedReadonlyRegClassID);
}

// The def/use of the scratch registers are intended to be local.
// However, we have encounter cases where def of a scratch register is
// hoisted to predecessor by Control Flow Optimizer.
// So we have to trace liveness of scratch registers across block boundaries.
bool AMDILRegisterUseValidate::runOnMachineFunction(MachineFunction &MF) {
  // map a block number to its live set
  typedef std::map<unsigned, RegSet> Num2RegSetMap;
  Num2RegSetMap BBLives;

  DEBUG(dbgs() << "Check function " << MF.getFunction()->getName() << "\n");
  //DEBUG(MF.dump());

  typedef std::vector<const MachineBasicBlock*> BlockVec;
  BlockVec Worklist;
  DenseSet<unsigned> Visited;

  // Start from return blocks and trace backwards for
  // liveness of "scratch" registers.
  for (MachineFunction::const_iterator I = MF.begin(), E = MF.end();
    I != E; ++I) {
    const MachineBasicBlock &BB = *I;
    if (BB.succ_empty())
      Worklist.push_back(&BB);
  }

  while (!Worklist.empty()) {
    const MachineBasicBlock *BB = Worklist.back();
    Worklist.pop_back();
    RegSet &Lives = BBLives[BB->getNumber()];
    bool changed = false;
    // LivSet(BB) = Sum(LiveSet[Succ])
    for (MachineBasicBlock::const_succ_iterator SI = BB->succ_begin(),
      SE = BB->succ_end(); SI != SE; ++SI) {
      RegSet &SuccLives = BBLives[(*SI)->getNumber()];
      changed |= set_union(Lives, SuccLives);
    }
    if (Visited.count(BB->getNumber()) && !changed) {
      continue;
    }
    trace(*BB, Lives);
    Visited.insert(BB->getNumber());
    for (MachineBasicBlock::const_pred_iterator I = BB->pred_begin(),
      E = BB->pred_end(); I != E; ++I) {
      Worklist.push_back(*I);
    }
  }
  return false;
}

bool AMDILRegisterUseValidate:: trace(const MachineBasicBlock& bBlock,
                                      RegSet &bbLiveReg)
{
  for (MachineBasicBlock::const_reverse_iterator BI = bBlock.rbegin(),
      BE = bBlock.rend(); BI != BE; ++BI)  {
    const MachineInstr *MI = &(*BI);
    if (MI->isCall()) {
      DEBUG(dbgs() << "Call " << *MI <<
          "live register: " << bbLiveReg <<
          "\n");
      assert (bbLiveReg.empty() &&
          "Reserved temp register def/use cross function call" != NULL);
    }
    uint32_t Opcode = MI->getOpcode();
    for (unsigned I = 0, N = MI->getNumOperands(); I < N; ++I) {
      const MachineOperand &RegOp = MI->getOperand(I);
      if (!RegOp.isReg()) continue;

      // Verifies that the read-only registers are never written into
      unsigned number = RegOp.getReg();
      if (RegOp.isDef()) {
        assert(!readonlyRegClass->contains(number) &&
               "Readonly registers are changed" != NULL);
      }

      // trace liveness of "scratch" registers
      if (!tempRegClass->contains(number)) continue;
      if (RegOp.isDef()) {
        bbLiveReg.erase(number);
      } else {
        bbLiveReg.insert(number);
      }
    }
  }

  return false;
}

const char *AMDILRegisterUseValidate::getPassName() const {
  return "AMDIL Register Use Validate";
}

