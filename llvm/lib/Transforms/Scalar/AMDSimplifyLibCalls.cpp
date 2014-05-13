//===- SimplifyLibCalls.cpp - Optimize specific well-known library calls --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements a simple pass that applies a variety of small
// optimizations for calls to specific well-known function calls (e.g. runtime
// library functions).   Any optimization that takes the very simple form
// "replace call to library function with simpler code that provides the same
// result" belongs in this file.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "simplifylib"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/AMDLibCalls.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/IRBuilder.h"
#include "llvm/DataLayout.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

//===----------------------------------------------------------------------===//
// AMDSimplifyLibCalls Pass Implementation
//===----------------------------------------------------------------------===//

namespace {
  class SimplifyLibCalls : public AMDLibCalls {
  protected:
    void replaceCall(Value *With) {
      CI->replaceAllUsesWith(With);
      CI->eraseFromParent();
    }
  };

  class AMDSimplifyLibCalls : public FunctionPass {
    SimplifyLibCalls *Simplifier;
    
  public:
    static char ID; // Pass identification
    AMDSimplifyLibCalls(bool UnsafeMath = false) : FunctionPass(ID) {
      initializeAMDSimplifyLibCallsPass(*PassRegistry::getPassRegistry());
      Simplifier = new SimplifyLibCalls();
      if (UnsafeMath)
        Simplifier->enableUnsafeMathOpt();
    }
    ~AMDSimplifyLibCalls() {
      if (Simplifier != NULL)
        delete Simplifier;
    }
    bool runOnFunction(Function &F);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<AliasAnalysis>();
    }
  };

  class AMDUseNativeCalls : public FunctionPass {
    SimplifyLibCalls *Simplifier;

  public:
    static char ID; // Pass identification
    AMDUseNativeCalls(const char* V = NULL) : FunctionPass(ID) {
      initializeAMDUseNativeCallsPass(*PassRegistry::getPassRegistry());
      Simplifier = new SimplifyLibCalls();
      Simplifier->setFuncNames(V);
    }
    ~AMDUseNativeCalls() {
      if (Simplifier != NULL)
        delete Simplifier;
    }
    bool runOnFunction(Function &F);
  };

} // end anonymous namespace.

char AMDSimplifyLibCalls::ID = 0;
char AMDUseNativeCalls::ID = 0;

INITIALIZE_PASS(AMDSimplifyLibCalls, "simplifylib",
                "Simplify well-known library calls", false, false)
INITIALIZE_PASS(AMDUseNativeCalls, "usenative",
                "Replace builtin math calls with that native versions.",
                false, false)

// Public interface to the Simplify LibCalls pass.
FunctionPass *llvm::createAMDSimplifyLibCallsPass(bool UnsafeMath) {
  return new AMDSimplifyLibCalls(UnsafeMath);
}
FunctionPass *llvm::createAMDUseNativeCallsPass(const char* FNameList) {
  return new AMDUseNativeCalls(FNameList);
}

bool AMDSimplifyLibCalls::runOnFunction(Function &F) {

  const DataLayout *DL = getAnalysisIfAvailable<DataLayout>();

  AliasAnalysis& AA = getAnalysis<AliasAnalysis>();

  bool Changed = false;
  for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
    for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ) {
      // Ignore non-calls.
      CallInst *CI = dyn_cast<CallInst>(I);
      ++I;
      if (!CI) continue;

      // Ignore indirect calls.
      Function *Callee = CI->getCalledFunction();
      if (Callee == 0) continue;

      // Our library function name starts with "__"
      if (!Callee->getName().startswith("__")) continue;

      if(Simplifier->fold(CI, DL,&AA)) {
        Changed = true;
      }
    }
  }
  return Changed;
}

bool AMDUseNativeCalls::runOnFunction(Function &F) {
  const DataLayout *DL = getAnalysisIfAvailable<DataLayout>();

  bool Changed = false;
  for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
    for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ) {
      // Ignore non-calls.
      CallInst *CI = dyn_cast<CallInst>(I);
      ++I;
      if (!CI) continue;

      // Ignore indirect calls.
      Function *Callee = CI->getCalledFunction();
      if (Callee == 0) continue;

      // Our library function name starts with "__"
      if (!Callee->getName().startswith("__")) continue;

      if(Simplifier->useNative(CI, DL)) {
        Changed = true;
      }
    }
  }
  return Changed;
}

