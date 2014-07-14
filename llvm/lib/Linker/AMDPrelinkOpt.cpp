//
// Copyright (c) 2013 Advanced Micro Devices, Inc. All rights reserved.
//

#include "llvm/AMDPrelinkOpt.h"
#include "llvm/AMDLLVMContextHook.h"
#include "llvm/DataLayout.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/LLVMContext.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

namespace llvm {

void AMDPrelinkOpt(Module *M, bool Whole, bool DisableSimplifyLibCalls,
                   bool UnsafeFPMath, const char* UseNative,
                   bool LowerToPreciseFunctions) {
  // Since AMDSimplifyLibCallsPass() behaves slightly differently b/w
  // PreLink and PostLink opt, set flag to tell LLVM pass whether it is
  // in PreLink or PostLink.
  AMDLLVMContextHook *AmdHook  = static_cast<AMDLLVMContextHook*>(
                                   M->getContext().getAMDLLVMContextHook());
  assert(AmdHook != NULL);
  if (!AmdHook) return;

  AmdHook->amdoptions.WholeProgram = Whole;
  AmdHook->amdoptions.IsPreLinkOpt = true;
  AmdHook->amdoptions.UnsafeMathOpt = UnsafeFPMath;

  llvm::PassManager Passes;
  Passes.add(new llvm::DataLayout(M));

  // AliasAnalysis will be created on demand
  // we just need it presence in pass queue
  Passes.add(llvm::createTypeBasedAliasAnalysisPass());
  Passes.add(llvm::createBasicAliasAnalysisPass());

  if (!DisableSimplifyLibCalls) {
    Passes.add(llvm::createAMDSimplifyLibCallsPass(UnsafeFPMath));
  }
  if (UseNative) {
    Passes.add(llvm::createAMDUseNativeCallsPass(UseNative));
  }
  if (LowerToPreciseFunctions) {
    Passes.add(llvm::createAMDLowerToPreciseBuiltinsPass());
  }
  Passes.run(*M);
}
} // namespace llvm
