//===--- AMDILRenumberRegister.cpp - AMDIL Renumber Register Pass --*- C++ -*--===//
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

#define DEBUG_TYPE "renumber_register"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "AMDIL.h"
#include "AMDILAlgorithms.tpp"
#include "AMDILRegisterInfo.h"
#include "AMDILTargetMachine.h"
#include "AMDILUtilityFunctions.h"

using namespace llvm;
using namespace AMDIL;

// Noinline functions are emitted as function calls in AMDIL. Since they do not
// have a stack, they need to use a different set of registers than the caller
// to avoid overwriting registers used by caller.
// AMDIL Renumber Register traverses through all of the instructions of a
// noinline function and renumber temporary registers and CFG registers to
// a disjoint set.

namespace {
  class AMDILRenumberRegister : public MachineFunctionPass {
  public:
    static char ID;
    AMDILRenumberRegister(TargetMachine &tm, CodeGenOpt::Level OL);
    virtual const char *getPassName() const;

    bool runOnMachineFunction(MachineFunction &MF);
  private:
    bool runOn(MachineBasicBlock::iterator *bbb);
    TargetMachine &TM;
    bool mChanged;
  };
  char AMDILRenumberRegister::ID = 0;
}

namespace llvm {
  FunctionPass *createAMDILRenumberRegister(TargetMachine &TM_,
                                          CodeGenOpt::Level OL) {
    return new AMDILRenumberRegister(TM_, OL);
  }
}

AMDILRenumberRegister::AMDILRenumberRegister(TargetMachine &TM_,
                                         CodeGenOpt::Level OL)
  : MachineFunctionPass(ID),
    TM(TM_) {
}

bool AMDILRenumberRegister::runOnMachineFunction(MachineFunction &MF) {
  mChanged = false;
  if (!MF.getFunction()->getFnAttributes().hasAttribute(Attributes::NoInline))
    return mChanged;

  DEBUG(MF.dump());
  safeNestedForEach(MF.begin(), MF.end(), MF.begin()->begin(),
      std::bind1st(std::mem_fun(&AMDILRenumberRegister::runOn), this));
  DEBUG(MF.dump());
  return mChanged;
}

// Renumber registers for noinline functions.
// This function should only be run with noinline functions.
// registers [33-64]: for noinline functions, should not appear here
// registers [65-96]: for inline functions and kernels, renumbered to [33-64]
// registers [97-160]: for inline functions and kernels, renumbered to [192-255]
// registers [161-255]: out of mapping range, should not appear here
// registers CFG1-CFG10: for inline functions, renumbered to CFG11-CFG20
// registers CFG11-CFG20: for noinline functions, should not appear here
// do not renumber other registers
bool AMDILRenumberRegister::runOn(MachineBasicBlock::iterator *BBB) {
  struct RenumberTableEntry {
    unsigned start;
    unsigned end;
    unsigned newStart;
  };

#define DEFINE_ABNORMAL_MAP(start, end, newStart) \
  {R##start,         R##end,        newStart}, \
  {Rx##start,        Rx##end,       newStart}, \
  {Ry##start,        Ry##end,       newStart}, \
  {Rz##start,        Rz##end,       newStart}, \
  {Rw##start,        Rw##end,       newStart}, \
  {Rxy##start,       Rxy##end,      newStart}, \
  {Rzw##start,       Rzw##end,      newStart}

#define DEFINE_NORMAL_MAP(start, end, newStart) \
  {R##start,         R##end,        R##newStart}, \
  {Rx##start,        Rx##end,       Rx##newStart}, \
  {Ry##start,        Ry##end,       Ry##newStart}, \
  {Rz##start,        Rz##end,       Rz##newStart}, \
  {Rw##start,        Rw##end,       Rw##newStart}, \
  {Rxy##start,       Rxy##end,      Rxy##newStart}, \
  {Rzw##start,       Rzw##end,      Rzw##newStart}

  static RenumberTableEntry table[] = {
      DEFINE_NORMAL_MAP(33, 160, 161),
      DEFINE_ABNORMAL_MAP(161, 288, NoRegister),
      {CFG1,        CFG10,          CFG11},
      {CFG11,       CFG20,          NoRegister},
      {NoRegister,  NoRegister, NoRegister},
  };
  MachineInstr *MI = *BBB;
  uint32_t Opcode = MI->getOpcode();
  for (unsigned I = 0, N = MI->getNumOperands(); I < N; ++I) {
    MachineOperand &RegOp = MI->getOperand(I);
    if (!RegOp.isReg())
      continue;
    unsigned number = RegOp.getReg();
    DEBUG(dbgs() << "register " << number);
    for (RenumberTableEntry *I = table; I->start != NoRegister; ++I) {
      if (I->start <= number && number <= I->end) {
        assert (I->newStart != NoRegister &&
            "invalid register allocation found");
        unsigned newNumber = number - I->start + I->newStart;
        RegOp.setReg(newNumber);
        DEBUG(dbgs() << " => " << newNumber);
        mChanged = true;
      }
    }
    DEBUG(dbgs() << "\n");
  }

  return false;
}

const char *AMDILRenumberRegister::getPassName() const {
  return "AMDIL Renumber Register";
}


