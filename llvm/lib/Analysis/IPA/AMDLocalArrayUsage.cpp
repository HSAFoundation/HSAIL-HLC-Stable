//===- AMDLocalArrayUsage.cpp - Check opencl local array usage ------------===//
//
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief Check OpenCL kernels containing local arrays are not called by 
/// another kernel.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "AMDLocalArrayUsage"

#include "llvm/ADT/ValueMap.h"
#include "llvm/Analysis/AMDLocalArrayUsage.h"
#include "llvm/CallingConv.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <string>
using namespace llvm;

namespace {
  class AMDLocalArrayUsage : public ModulePass {

    // The local address space definition below has to match SPIR, 
    // AMDIL and HSAIL.
    enum {
      LOCAL_ADDRESS    = 3, // Same as GROUP_ADDRESS for HSAIL
    };

  public:
    static char ID; // Pass identification, replacement for typeid

    AMDLocalArrayUsage() : ModulePass(ID) {
        initializeAMDLocalArrayUsagePass(*PassRegistry::getPassRegistry());
    }

    virtual bool runOnModule(Module &M);
    bool hasError() { return !errorMessage.empty(); }
    std::string getErrorMessage() { return errorMessage;}

  private:
    ValueMap<const Function*, bool> isCalledByKernelMap;
    std::string errorMessage;

    // Check usage of local array is valid
    bool checkLocalArray(GlobalVariable& GV);
  };
} //End anonymous namespace


INITIALIZE_PASS(AMDLocalArrayUsage, "amdlocalarrayusage",
                "AMD Local Array Usage", false, false);

char AMDLocalArrayUsage::ID = 0;

bool AMDLocalArrayUsage::runOnModule(Module& M) {
  errorMessage.clear();
  isCalledByKernelMap.clear();

  for (Module::global_iterator GI = M.global_begin(), GE = M.global_end();
      GI != GE; ++ GI) {
    if (PointerType* PT = dyn_cast<PointerType>(GI->getType())) {
      if (PT->getAddressSpace() != LOCAL_ADDRESS) {
        continue;
      }

      if (!checkLocalArray(*GI)) {
        return false;
      }
    }
  }
  return false;
}

// Check if a function is kernel
static bool isKernel(const Function* F) {
  return F->getCallingConv()==CallingConv::SPIR_KERNEL ||
      (F->getName().startswith("__OpenCL_") &&
      F->getName().endswith("_kernel"));
}

static StringRef undecorateKernelFunctionName (const Function* F) {
  StringRef FunctionName = F->getName();
  if (F->getCallingConv() == CallingConv::SPIR_KERNEL) {
    return FunctionName;
  }
  return FunctionName.substr(strlen("__OpenCL_"),
           FunctionName.size() - strlen("_kernel") - strlen("__OpenCL_"));
}

// Check if a function F is called directly or indirectly by a kernel.
// If F is called by a kernel function, caller is returned by callerFunction.
//
// This function can identify the situation that a kernel is called by
// a non-kernel function which in turn is called by a kernel.
static bool isCalledByKernel(const Function* F,
    ValueMap<const Function*, bool>& work,
    const Function** callerFunction = NULL) {
  ValueMap<const Function*, bool>::iterator loc = work.find(F);
  if (loc != work.end())
    return loc->second;
  for (Function::const_use_iterator I = F->use_begin(), E = F->use_end(); I != E;
      ++I) {
    const User *UI = *I;
    if (isa<CallInst>(UI) || isa<InvokeInst>(UI)) {
      ImmutableCallSite CS(cast<Instruction>(UI));
      const Function* caller = CS.getCaller();
      if (isKernel(caller) || isCalledByKernel(caller, work)) {
        work[F] = true;
        if (callerFunction) {
          *callerFunction = caller;
        }
        return true;
      }
    }
  }
  work[F] = false;
  return false;
}

bool AMDLocalArrayUsage::checkLocalArray(GlobalVariable& GV) {
  for (llvm::Value::use_iterator UI = GV.use_begin(), UE = GV.use_end();
      UI != UE; ++UI) {
    User* user = *UI;
    if (Instruction* inst = dyn_cast<Instruction>(user)) {
      Function* usingFunction = inst->getParent()->getParent();
      const Function* caller;
      if (isKernel(usingFunction) && isCalledByKernel(usingFunction,
          isCalledByKernelMap, &caller)) {
        errorMessage = std::string("Kernel ") +
            undecorateKernelFunctionName(usingFunction).str() +
            " uses local array. It should not be called by another kernel " +
            undecorateKernelFunctionName(caller).str() + ".";
        DEBUG(llvm::dbgs() << errorMessage << "\n");
        return false;
      }
    }
  }
  return true;
}

bool llvm::AMDCheckLocalArrayUsage(
    const llvm::Module &M,
    std::string *errorMessage)
{
  PassManager PM;
  AMDLocalArrayUsage *P = new AMDLocalArrayUsage();
  PM.add(P);
  PM.run(const_cast<Module&>(M));

  bool hasError = P->hasError();
  if (errorMessage && hasError)
    *errorMessage = P->getErrorMessage();
  return !hasError;
}
