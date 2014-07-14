//===--- AMDILLiteralManager.cpp - AMDIL Literal Manager Pass --*- C++ -*--===//
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

#define DEBUG_TYPE "literal_manager"
#include "AMDIL.h"
#include "AMDILAlgorithms.tpp"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILSubtarget.h"
#include "AMDILTargetMachine.h"
#include "AMDILUtilityFunctions.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;


// AMDIL Literal Manager traverses through all of the LOADCONST
// instructions and converts them from an immediate value to the
// literal index. The literal index is valid IL, but the immediate
// values are not. The Immediate values must be aggregated and
// declared for clarity and to reduce the number of literals that are
// used. It is also illegal to declare the same literal twice, so this
// keeps that from occuring.

namespace {
  class AMDILLiteralManager : public MachineFunctionPass {
  public:
    static char ID;
    AMDILLiteralManager(TargetMachine &tm, CodeGenOpt::Level OL);
    virtual const char *getPassName() const;

    bool runOnMachineFunction(MachineFunction &MF);
  private:
    bool trackLiterals(MachineBasicBlock::iterator *bbb);
    TargetMachine &TM;
    const AMDILSubtarget *mSTM;
    const AMDILModuleInfo *mAMI;
    AMDILMachineFunctionInfo *mMFI;
    bool mChanged;
  };
  char AMDILLiteralManager::ID = 0;
}

namespace llvm {
  FunctionPass *createAMDILLiteralManager(TargetMachine &TM_,
                                          CodeGenOpt::Level OL) {
    return new AMDILLiteralManager(TM_, OL);
  }
}

AMDILLiteralManager::AMDILLiteralManager(TargetMachine &TM_,
                                         CodeGenOpt::Level OL)
  : MachineFunctionPass(ID),
    TM(TM_),
    mSTM(NULL),
    mAMI(NULL),
    mMFI(NULL),
    mChanged(false) {
}

bool AMDILLiteralManager::runOnMachineFunction(MachineFunction &MF) {
  mChanged = false;
  DEBUG(MF.dump());
  mMFI = MF.getInfo<AMDILMachineFunctionInfo>();
  const AMDILTargetMachine *amdtm =
    reinterpret_cast<const AMDILTargetMachine *>(&TM);
  mSTM = dynamic_cast<const AMDILSubtarget *>(amdtm->getSubtargetImpl());
  mAMI = &(MF.getMMI().getObjFileInfo<AMDILModuleInfo>());
  safeNestedForEach(MF.begin(), MF.end(), MF.begin()->begin(),
      std::bind1st(std::mem_fun(&AMDILLiteralManager::trackLiterals), this));
  DEBUG(MF.dump());
  return mChanged;
}

bool AMDILLiteralManager::trackLiterals(MachineBasicBlock::iterator *BBB) {
  MachineInstr *MI = *BBB;
  uint32_t Opcode = MI->getOpcode();
  for (unsigned I = 0, N = MI->getNumOperands(); I < N; ++I) {
    MachineOperand &LitOp = MI->getOperand(I);
    // for globally defined constant or private arrays, declare the literals
    // for their offsets
    if (LitOp.getType() == MachineOperand::MO_GlobalAddress) {
      const GlobalValue *GV = LitOp.getGlobal();
      int64_t Offset = mAMI->getArrayOffset(GV->getName());
      if (Offset == -1) {
        Offset = mAMI->getConstOffset(GV->getName());
      }
      if (Offset == -1) {
        continue;
      }
      unsigned RegClass = MI->getDesc().OpInfo[I].RegClass;
      if (RegClass == AMDIL::GPR_32RegClassID) {
        mMFI->addi32Literal(static_cast<uint32_t>(Offset));
      } else {
        assert(RegClass == AMDIL::GPR_64RegClassID && "unexpected op type");
        mMFI->addi64Literal(static_cast<uint64_t>(Offset));
      }
      continue;
    }
    if ((!LitOp.isImm() && !LitOp.isFPImm())
        || isBypassedLiteral(MI, I)
        || isSkippedLiteral(MI, I)
        || !MI->getDesc().OpInfo) {
      continue;
    }
    /*
    assert(Opcode <= AMDIL::LOADCONSTf64 && Opcode >= AMDIL::LOADCONSTi8
        && "Found a loadconst instruction!");
        */
    uint32_t Idx;
    if (LitOp.isFPImm()) {
      const ConstantFP *fpVal = LitOp.getFPImm();
      const fltSemantics &fpSem = fpVal->getValueAPF().getSemantics();
      if (&fpSem == &APFloat::IEEEsingle) {
        Idx = mMFI->addf32Literal(fpVal);
      } else if (&fpSem == &APFloat::IEEEdouble) {
        Idx = mMFI->addf64Literal(fpVal);
      } else {
        llvm_unreachable("Found a case we don't handle!");
      }
    } else if (LitOp.isImm()) {
      unsigned RegClass = MI->getDesc().OpInfo[I].RegClass;
      if (RegClass == ~0U) {
        StringRef Name = TM.getInstrInfo()->getName(Opcode);
        RegClass = AMDILRegisterInfo::getRegClassFromName(Name);
      }
      int64_t ImmVal = LitOp.getImm();
      switch (RegClass) {
        default:
          Idx = Opcode == AMDIL::LOADCONSTi64
            ? mMFI->addi64Literal(ImmVal)
            : mMFI->addi32Literal(static_cast<int>(ImmVal));
          break;
        case AMDIL::GPR_32RegClassID:
        case AMDIL::GPRV2I32RegClassID:
        case AMDIL::GPRV4I32RegClassID:
          Idx = mMFI->addi32Literal(static_cast<int>(ImmVal));
          break;
        case AMDIL::GPR_64RegClassID:
        case AMDIL::GPRV2I64RegClassID:
          Idx = mMFI->addi64Literal(ImmVal);
          break;
      }
    } else {
      llvm_unreachable("Should never hit here unless a new literal type was added!");
    }
    LitOp.ChangeToImmediate(Idx);
  }

  return false;
}

const char *AMDILLiteralManager::getPassName() const {
  return "AMDIL Literal Manager";
}


