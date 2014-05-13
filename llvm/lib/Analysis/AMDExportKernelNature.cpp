//===- AMDExportKernelNature.cpp - export kernels' nature -----------------===//
//
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "amdexportkernelnature"
#include "llvm/Analysis/Passes.h"
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Constants.h"
#include "llvm/Analysis/AMDOpenCLSymbols.h"
#include "llvm/Support/CallSite.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "../Transforms/IPO/AMDSymbolName.h"

typedef unsigned int uint;
typedef uint32_t cl_mem_fence_flags;
#include "../../../api/opencl/amdocl/cl_kernel.h"
#include <set>
using namespace llvm;

namespace {
  //===--------------------------------------------------------------------===//
  // AMDExportKernelNature pass implementation.
  class AMDExportKernelNature : public ModulePass {
  public:
    static char ID;
    AMDExportKernelNature(bool WorkGroupLevel = false) : ModulePass(ID),
                                                         WGLevel(WorkGroupLevel) {
      initializeAMDExportKernelNaturePass(*PassRegistry::getPassRegistry());
    }

    virtual bool runOnModule(Module& M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<OpenCLSymbols>();
      AU.setPreservesCFG();
    }

  private:
    bool WGLevel;
    StructType *NatureTy;

    void initializeNatureStructure(Module& M) {
      LLVMContext &C = M.getContext();
      IntegerType *Int32Ty = Type::getInt32Ty(C);
      Type *TElems[] = { Int32Ty, Int32Ty };
      NatureTy = StructType::get(C, TElems, true);
    }

    void exportBarrierUser(Function *FStub) const {
      GlobalVariable *GV = cast<GlobalVariable>(
        FStub->getParent()->getOrInsertGlobal(
          AMDSymbolNames::decorateNatureName(
            AMDSymbolNames::undecorateStubFunctionName(FStub->getName())), NatureTy));

      IntegerType *Int32Ty = Type::getInt32Ty(FStub->getContext());
      Constant *CElems[] = {
        ConstantInt::get(Int32Ty, KN_HAS_BARRIER |
                                 (WGLevel ? KN_WG_LEVEL : 0)),
        ConstantInt::get(Int32Ty, 0)
      };
      GV->setInitializer(ConstantStruct::get(NatureTy, CElems));
    }

    void exportWGLevelStub(Function *FStub) const {
      GlobalVariable *GV = cast<GlobalVariable>(
        FStub->getParent()->getOrInsertGlobal(
          AMDSymbolNames::decorateNatureName(
            AMDSymbolNames::undecorateStubFunctionName(FStub->getName())), NatureTy));

      IntegerType *Int32Ty = Type::getInt32Ty(FStub->getContext());
      Constant *CElems[] = {
        ConstantInt::get(Int32Ty, KN_WG_LEVEL),
        ConstantInt::get(Int32Ty, 0)
      };
      GV->setInitializer(ConstantStruct::get(NatureTy, CElems));
    }

    void setNoInline(Function *F) const {
      if (WGLevel && !F->getFnAttributes().hasAttribute(Attributes::NoInline)) {
        LLVMContext &C = F->getParent()->getContext();

        AttrBuilder B;
        B.addAttribute(Attributes::AlwaysInline);
        F->removeFnAttr(Attributes::get(C, B));
        F->addFnAttr(Attributes::NoInline);
      }
    }

    void setAlwaysInline(Function *F) const {
      if (WGLevel && !F->getFnAttributes().hasAttribute(Attributes::AlwaysInline)) {
        LLVMContext &C = F->getParent()->getContext();

        AttrBuilder B;
        B.addAttribute(Attributes::NoInline);
        F->removeFnAttr(Attributes::get(C, B));
        F->addFnAttr(Attributes::AlwaysInline);
      }
    }
  };

} // end anonymous namespace

INITIALIZE_PASS(AMDExportKernelNature, "amdexportkernelnature",
                "AMD Export Kernel Nature", false, false);

char AMDExportKernelNature::ID = 0;

bool AMDExportKernelNature::runOnModule(Module& M)
{
  initializeNatureStructure(M);

  const OpenCLSymbols &OCLS = getAnalysis<OpenCLSymbols>();
  if (WGLevel) {
    for (OpenCLSymbols::stubs_iterator I = OCLS.stubs_begin(),
         E = OCLS.stubs_end(); I != E; ++I)
      exportWGLevelStub(const_cast<Function *>(*I));
  }

  Function *Caller = NULL;
  // TODO: The following macro should be removed when Clang becomes the
  // default compiler for building x86 built-ins. Only the check for the
  // mangled names should be retained.
#ifdef BUILD_X86_WITH_CLANG
  Caller = M.getFunction("_Z7barrierj");
#else
  Caller = M.getFunction("barrier");
#endif

  if (!Caller) {
    DEBUG(dbgs() << "No use of barrier in module.\n");
    return true;
  }

  setNoInline(Caller);

  SmallPtrSet<User *, 16> Users;
  SmallVector<User *, 64> Pending;

  DEBUG(dbgs() << "Indirect/direct barrier users:\n");
  Pending.push_back(Caller);
  while (!Pending.empty()) {
    User *U = Pending.pop_back_val();

    for (Value::use_iterator I = U->use_begin(), E = U->use_end();
         I != E; ++I) {
      User *UI = *I;
      Caller = 0;
      if (isa<CallInst>(UI) || isa<InvokeInst>(UI)) {
        Caller = cast<Instruction>(UI)->getParent()->getParent();
      }
      // Only allowed non Call/Invoke instruction user is Constant.
      else if (isa<Constant>(UI)) {
        if (!isa<GlobalValue>(UI) && Users.insert(UI))
          Pending.push_back(UI);
        continue;
      }

      assert(Caller && "Indirect function calls are not allowed in OpenCL!");

      DEBUG(dbgs() << "    " << (Caller->hasName() ? Caller->getName() : "N/A") << "\n");
      if (OCLS.isStub(Caller)) {
        exportBarrierUser(Caller);
      }
      else if (Users.insert(Caller)) {
        Pending.push_back(Caller);
        setAlwaysInline(Caller);
      }
    }
  }
  DEBUG(dbgs() << "    (" << Users.size() << " users)\n");
  return true;
}

ModulePass *llvm::createAMDExportKernelNaturePass(bool WorkGroupLevel) {
  return new AMDExportKernelNature(WorkGroupLevel);
}
