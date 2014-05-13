//===-- AMDILMachinePeephole.cpp - AMDIL Machine Peephole Pass -*- C++ -*-===//
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

#define DEBUG_TYPE "machine_peephole"

#include "AMDIL.h"
#include "AMDILSubtarget.h"
#include "AMDILUtilityFunctions.h"
#include "llvm/InitializePasses.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm
{
  extern void initializeAMDILMachinePeepholePass(llvm::PassRegistry&);
}

using namespace llvm;
namespace
{
  class AMDILMachinePeephole : public MachineFunctionPass
  {
      typedef SmallVector<MachineBasicBlock *, 32> MachineBlockVec;
      typedef SmallVector<MachineInstr *, 4> MachineInstVec;
      typedef std::map<uint32_t, MachineInstVec*> Reg2InstsMap;

    public:
      static char ID;
      AMDILMachinePeephole();
      // virtual ~AMDILMachinePeephole();
      virtual const char *getPassName() const;
      virtual bool runOnMachineFunction(MachineFunction &MF);
      virtual void getAnalysisUsage(AnalysisUsage &AU) const {
          AU.setPreservesCFG();
          MachineFunctionPass::getAnalysisUsage(AU);
        }

    private:
      void insertFence(MachineBasicBlock::iterator &MIB);

      const TargetMachine *TM;
      MachineFunction* MFP;
  }; // AMDILMachinePeephole
} // anonymous namespace

char AMDILMachinePeephole::ID = 0;
INITIALIZE_PASS_BEGIN(AMDILMachinePeephole, "amdil-machine-peephole",
                      "AMDIL Machine Peephole Optimization", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_END(AMDILMachinePeephole, "amdil-machine-peephole",
                    "AMDIL Machine Peephole Optimization", false, false)

namespace llvm
{
FunctionPass *createAMDILMachinePeephole() {
  return new AMDILMachinePeephole();
}
} // llvm namespace

AMDILMachinePeephole::AMDILMachinePeephole()
  : MachineFunctionPass(ID),
    TM(NULL), MFP(NULL) {
  initializeAMDILMachinePeepholePass(*PassRegistry::getPassRegistry());
}

bool AMDILMachinePeephole::runOnMachineFunction(MachineFunction &MF) {
  MFP = &MF;
  TM = &MF.getTarget();

  bool Changed = false;
  const AMDILSubtarget *STM = &TM->getSubtarget<AMDILSubtarget>();
  for (MachineFunction::iterator I = MF.begin(), E = MF.end(); I != E; ++I) {
    MachineBasicBlock &MB = *I;
    for (MachineBasicBlock::iterator MIB = MB.begin(), MIE = MB.end();
         MIB != MIE; ++MIB) {
      MachineInstr *MI = MIB;
      if (isAtomicInst(MI)) {
        StringRef Name = TM->getInstrInfo()->getName(MI->getOpcode());
        // If we don't support the hardware accellerated address spaces,
        // then the atomic needs to be transformed to the global atomic.
        if (Name.find("_L_") != StringRef::npos &&
            STM->usesSoftware(AMDIL::Caps::LocalMem)) {
          BuildMI(MB, MIB, MI->getDebugLoc(),
                  TM->getInstrInfo()->get(AMDIL::ADDi32rr), AMDIL::R1011)
            .addReg(MI->getOperand(1).getReg())
            .addReg(AMDIL::T2);
          MI->getOperand(1).setReg(AMDIL::R1011);
          MI->setDesc(
            TM->getInstrInfo()->get(
              (MI->getOpcode() - AMDIL::ATOM_L_ADD) + AMDIL::ATOM_G_ADD));
        } else if (Name.find("_R_") != StringRef::npos &&
                   STM->usesSoftware(AMDIL::Caps::RegionMem)) {
          llvm_unreachable("Software region memory is not supported!");
          MI->setDesc(
            TM->getInstrInfo()->get(
              (MI->getOpcode() - AMDIL::ATOM_R_ADD) + AMDIL::ATOM_G_ADD));
        }
      } else if (isVolatileInst(MI) &&
                 (isPtrLoadInst(MI) || isPtrStoreInst(MI))) {
        insertFence(MIB);
      }
    }
  }

  return Changed;
}

const char *AMDILMachinePeephole::getPassName() const {
  return "AMDIL Generic Machine Peephole Optimization Pass";
}

void AMDILMachinePeephole::insertFence(MachineBasicBlock::iterator &MIB) {
  MachineInstr *MI = MIB;
  MachineInstr *Fence = BuildMI(*(MI->getParent()->getParent()),
        MI->getDebugLoc(),
        TM->getInstrInfo()->get(AMDIL::FENCEr)).addReg(1);

  MI->getParent()->insert(MIB, Fence);
  Fence = BuildMI(*(MI->getParent()->getParent()),
        MI->getDebugLoc(),
        TM->getInstrInfo()->get(AMDIL::FENCEr)).addReg(1);
  MIB = MI->getParent()->insertAfter(MIB, Fence);
}


