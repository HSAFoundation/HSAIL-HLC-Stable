//===----- AMDILBarrierDetect.cpp - Barrier Detect pass -*- C++ -*- ------===//
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

#define DEBUG_TYPE "barrierdetect"

#include "llvm/Support/Debug.h"
#include "AMDILAlgorithms.tpp"
#include "AMDILCompilerWarnings.h"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILSubtarget.h"
#include "AMDILTargetMachine.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Target/TargetMachine.h"
using namespace llvm;

// The barrier detect pass determines if a barrier has been duplicated in the
// source program which can cause undefined behaviour if more than a single
// wavefront is executed in a group. This is because LLVM does not have an
// execution barrier and if this barrier function gets duplicated, undefined
// behaviour can occur. In order to work around this, we detect the duplicated
// barrier and then make the work-group execute in a single wavefront mode,
// essentially making the barrier a no-op.

namespace
{
  class LLVM_LIBRARY_VISIBILITY AMDILBarrierDetect : public FunctionPass {
    TargetMachine &TM;
    static char ID;
  public:
    AMDILBarrierDetect(TargetMachine &TM, CodeGenOpt::Level OptLevel);
    ~AMDILBarrierDetect();
    const char *getPassName() const;
    bool runOnFunction(Function &F);
    bool doInitialization(Module &M);
    bool doFinalization(Module &M);
    void getAnalysisUsage(AnalysisUsage &AU) const;
  private:
    bool detectBarrier(BasicBlock::iterator *BBI);
    bool mChanged;
    SmallVector<int64_t, DEFAULT_VEC_SLOTS> bVecMap;
    const AMDILSubtarget *mStm;

    // Constants used to define memory type.
    static const unsigned int LOCAL_MEM_FENCE = 1<<0;
    static const unsigned int GLOBAL_MEM_FENCE = 1<<1;
    static const unsigned int REGION_MEM_FENCE = 1<<2;
  };
  char AMDILBarrierDetect::ID = 0;
} // anonymous namespace

namespace llvm
{
  FunctionPass *createAMDILBarrierDetect(TargetMachine &TM,
                                         CodeGenOpt::Level OptLevel) {
    return new AMDILBarrierDetect(TM, OptLevel);
  }
} // llvm namespace

AMDILBarrierDetect::AMDILBarrierDetect(TargetMachine &TM,
                                       CodeGenOpt::Level OptLevel)
  : FunctionPass(ID),
    TM(TM) {

}

AMDILBarrierDetect::~AMDILBarrierDetect() {

}

bool AMDILBarrierDetect::detectBarrier(BasicBlock::iterator *BBI) {
  Instruction *Inst = *BBI;
  CallInst *CI = dyn_cast<CallInst>(Inst);
  if (!CI) {
    return false;
  }

  const Function *FuncVal = CI->getCalledFunction();
  if (!FuncVal) {
    return false;
  }

  StringRef FuncName = FuncVal->getName();

  if (!FuncName.startswith("__amdil_gws")) {
    return false;
  }

  // The runtime needs to know when a GWS instruction is used to avoid
  // running kernels with the normal enqueue function when no GDS is
  // used
  AMDILMachineFunctionInfo *MFI =
    getAnalysis<MachineFunctionAnalysis>().getMF()
    .getInfo<AMDILMachineFunctionInfo>();
  MFI->addMetadata(";memory:gws");

  // If the kernel uses reqd_work_region_size, we can use the actual
  // GWS instruction. Otherwise to get a dynamic region size, we need
  // to use the alternative implementation using semaphores
  AMDILKernel* kernel = MFI->getKernel();
  if (kernel->sgv->mHasRWR || mStm->hasDynamicRegionSize()) {
    return false;
  }

  MFI->addMetadata(";memory:swgws");
  return false;
}

bool AMDILBarrierDetect::runOnFunction(Function &MF) {
  mChanged = false;
  bVecMap.clear();
  mStm = &TM.getSubtarget<AMDILSubtarget>();
  Function *F = &MF;
  safeNestedForEach(F->begin(), F->end(), F->begin()->begin(),
      std::bind1st(
        std::mem_fun(
          &AMDILBarrierDetect::detectBarrier), this));
  return mChanged;
}

const char *AMDILBarrierDetect::getPassName() const {
  return "AMDIL Barrier Detect Pass";
}

bool AMDILBarrierDetect::doInitialization(Module &M) {
  return false;
}

bool AMDILBarrierDetect::doFinalization(Module &M) {
  return false;
}

void AMDILBarrierDetect::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<MachineFunctionAnalysis>();
  FunctionPass::getAnalysisUsage(AU);
  AU.setPreservesAll();
}
