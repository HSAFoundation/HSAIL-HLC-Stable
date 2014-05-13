//===- AMDOpenCLSymbols.cpp - Find OpenCL symbols -------------------------===//
//
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
// Find all AMD OpenCL specific symbols.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "AMDOpenCLSymbols"

#include "llvm/Analysis/AMDOpenCLSymbols.h"
#include "llvm/CallingConv.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Pass.h"
#include "../../Transforms/IPO/AMDSymbolName.h"
using namespace llvm;

namespace {
  class AMDOpenCLSymbols : public ModulePass, public OpenCLSymbols {
  public:
    static char ID; // Pass identification, replacement for typeid

    AMDOpenCLSymbols() : ModulePass(ID) {
        initializeAMDOpenCLSymbolsPass(*PassRegistry::getPassRegistry());
    }

    virtual bool runOnModule(Module &M) {
      Kernels.clear();
      Stubs.clear();
      Reserved.clear();

      for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
        Function *F = I;
        if (!F->hasName())
          continue;

        const StringRef FName = F->getName();
        if (AMDSymbolNames::isKernelFunctionName(FName)
            || F->getCallingConv() == CallingConv::SPIR_KERNEL) {
          if (AMDSymbolNames::isStubFunctionName(FName))
            Stubs.insert(F);
          else
            Kernels.insert(F);
        }
        else if (AMDSymbolNames::isReservedFunctionName(FName)) {
          Reserved.insert(F);
        }
      }

      for (Module::const_global_iterator I = M.global_begin(), E = M.global_end();
           I != E; ++I) {
        const GlobalValue *GV = I;
        if (GV->hasName() && AMDSymbolNames::isReservedGlobalName(GV->getName()))
          Reserved.insert(GV);
      }

      for (Module::const_alias_iterator I = M.alias_begin(), E = M.alias_end();
           I != E; ++I) {
        const GlobalAlias *GA = I;
        if (GA->hasName() && AMDSymbolNames::isReservedGlobalName(GA->getName()))
          Reserved.insert(GA);
      }

      return false;
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    }

    /// getAdjustedAnalysisPointer - This method is used when a pass implements
    /// an analysis interface through multiple inheritance.  If needed, it should
    /// override this to adjust the this pointer as needed for the specified pass
    /// info.
    virtual void *getAdjustedAnalysisPointer(AnalysisID PI) {
      if (PI == &OpenCLSymbols::ID)
        return (OpenCLSymbols*)this;
      return this;
    }
  };
} //End anonymous namespace


INITIALIZE_ANALYSIS_GROUP(OpenCLSymbols, "OpenCL Symbols",
                          AMDOpenCLSymbols)

INITIALIZE_AG_PASS(AMDOpenCLSymbols, OpenCLSymbols, "amdopenclsymbols",
                   "AMD OpenCL Symbols", false, true, true)

char OpenCLSymbols::ID = 0;
char AMDOpenCLSymbols::ID = 0;

/// replaceKernel - Replace the kernel function by another.
void OpenCLSymbols::replaceKernel(const Function *From, const Function *To)
{
  if (Kernels.remove(From))
    Kernels.insert(To);
}

/// replaceStub - Replace the stub function by another.
void OpenCLSymbols::replaceStub(const Function *From, const Function *To)
{
  if (Stubs.remove(From))
    Stubs.insert(To);
}

/// replaceGlobal - Replace the global value by another.
void OpenCLSymbols::replaceGlobal(const GlobalValue *From, const GlobalValue *To)
{
  if (isa<Function>(From) && isa<Function>(To)) {
    const Function *FFrom = cast<Function>(From);
    const Function *FTo = cast<Function>(To);
    if (Kernels.remove(FFrom)) {
      Kernels.insert(FTo);
      return;
    }

    if (Stubs.remove(FFrom)) {
      Stubs.insert(FTo);
      return;
    }
  }

  if (Reserved.remove(From))
    Reserved.insert(To);
}


const Function *OpenCLSymbols::getStubOfKernel(const Function *Kernel) const
{
  const Module *M = Kernel->getParent();
  if (!isKernel(Kernel) || !M)
    return 0;

  return M->getFunction(AMDSymbolNames::decorateStubFunctionName(
              AMDSymbolNames::undecorateKernelFunctionName(Kernel->getName())));
}

const Function *OpenCLSymbols::getKernelOfStub(const Function *Stub) const
{
  const Module *M = Stub->getParent();
  if (!isStub(Stub) || !M)
    return 0;

  return M->getFunction(AMDSymbolNames::decorateKernelFunctionName(
                  AMDSymbolNames::undecorateStubFunctionName(Stub->getName())));
}
