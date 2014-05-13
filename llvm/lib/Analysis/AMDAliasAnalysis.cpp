//===- AMDAliasAnalysis.cpp - Implement AliasAnalysis for libcalls ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// In OpenCL, the "barrier()" function, as well as various memory fence AMDIL
// intrinsics, should prevent loads/stores of the relevant address space
// from being moved across the barrier() or fence intrinsics.
// Kernel pointers with "restrict" attributes are implemented by marking
// the pointer "noalias" in the LLVMIR. In LLVM, "noalias" pointers are not
// affected by memory fence instructions.
// To make sure all loads/stores, including those accessing "restrict" pointers,
// are not moved across barrier/fence functions, two solutions are implemented.
// Enable only one of them to solve the problem.
// One solution is the AMDRemoveNoalias pass, which removes the "noalias"
// attribute of a kernel arg pointer if the kernel directly or indirectly calls
// a barrier/fence function, and the pointer is in the same address space as
// the barrier/fence. See AMDRemoveNoAlias.cpp for details.
// The other solution is to implement AMD OpenCL specific alias analysis pass,
// which is chained before other standard alias analysis passes.
// This file implements the AMDAliasAnalysis pass, which does AMD OpenCL
// specific alias analysis that is not handled by the standard alias analysises.
// This pass implements the getModRefInfo() interface function to
// report that AMDIL memory fence instrinsics, or any functions that call any
// of them, to clobber any memory in the same address space as the fence.
// This pass also implements the alias() interface function to return NoAlias
// for pointers that belong to different address spaces.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/AMDFenceInfoAnalysis.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Function.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace {
  // the definition below has to be consistant with the definition in AMDILAS
  // namespace
  enum {
    GLOBAL_ADDRESS   = 1, // Address space for global memory.
    LOCAL_ADDRESS    = 3, // Address space for local memory.
    REGION_ADDRESS   = 4, // Address space for region memory.
  };
  struct AMDAliasAnalysis : public ModulePass, public AliasAnalysis {
    static char ID; // Class identification

    AMDAliasAnalysis(bool BarrierClobberMem = true)
      : ModulePass(ID), FenceInfo(),
        _BarrierClobberMem(BarrierClobberMem) {
      initializeAMDAliasAnalysisPass(*PassRegistry::getPassRegistry());
    }
    ~AMDAliasAnalysis() {}

    virtual ModRefResult getModRefInfo(ImmutableCallSite CS,
                               const Location &Loc);

    virtual AliasResult alias(const Location &LocA, const Location &LocB);

    //virtual ModRefBehavior getModRefBehavior(const Function *F);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const;

    virtual bool runOnModule(Module &M) {
      InitializeAliasAnalysis(this);                 // set up super class
      FenceInfo = &getAnalysis<AMDFenceInfoAnalysis>();
      return false;
    }

    /// getAdjustedAnalysisPointer - This method is used when a pass implements
    /// an analysis interface through multiple inheritance.  If needed, it
    /// should override this to adjust the this pointer as needed for the
    /// specified pass info.
    virtual void *getAdjustedAnalysisPointer(const void *PI) {
      if (PI == &AliasAnalysis::ID)
        return (AliasAnalysis*)this;
      return this;
    }

  private:
    AMDFenceInfoAnalysis *FenceInfo;
    bool _BarrierClobberMem;
  };
}  // End of llvm namespace

// Register this pass...
char AMDAliasAnalysis::ID = 0;
INITIALIZE_AG_PASS(AMDAliasAnalysis, AliasAnalysis, "amd-opencl-aa",
                   "AMD OpenCL Alias Analysis", false, true, false)

ModulePass *llvm::createAMDAliasAnalysisPass(bool BarrierClobberMem) {
  return new AMDAliasAnalysis(BarrierClobberMem);
}

void AMDAliasAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AliasAnalysis::getAnalysisUsage(AU);
  AU.addRequired<AMDFenceInfoAnalysis>();
  AU.setPreservesAll();                         // Does not transform code
}

// getModRefInfo - Check to see if the specified callsite can clobber the
// specified memory object.
//
AliasAnalysis::ModRefResult
AMDAliasAnalysis::getModRefInfo(ImmutableCallSite CS, const Location &Loc) {
  ModRefResult MRInfo = NoModRef;

  // If _BarrierClobberMem is true, we use alias analysis to ensure IO
  // instructions are not moved across barriers/mem fences.
  if (_BarrierClobberMem) {
    if (const Function *F = CS.getCalledFunction()) {
      unsigned Flags = FenceInfo->getFenceFlags(*F);
      if (Flags) {
        unsigned AS = Loc.Ptr->getType()->getPointerAddressSpace();
        if ((AS == GLOBAL_ADDRESS) &&
            (Flags & AMDFenceInfoAnalysis::GLOBAL_MEM_FENCE)) {
          return Mod;
        }
        if ((AS == LOCAL_ADDRESS) &&
            (Flags & AMDFenceInfoAnalysis::LOCAL_MEM_FENCE)) {
          return Mod;
        }
      }
    }
  }

  // Forwarding to the next alias analysis
  return (ModRefResult)(MRInfo | AliasAnalysis::getModRefInfo(CS, Loc));
}

// Pointers to different address spaces don't alias
// TODO: when opencl2.0's generic address space is supported, handle the
// generic address space appropriately here.
AliasAnalysis::AliasResult
AMDAliasAnalysis::alias(const Location &LocA, const Location &LocB)
{
  if (LocA.Ptr->getType()->getPointerAddressSpace()
      != LocB.Ptr->getType()->getPointerAddressSpace())
    return NoAlias; // pointers to different address space don't alias

  // Forward the query to the next alias analysis.
  return AliasAnalysis::alias(LocA, LocB);
}

// Get the ModRef behavior for AMDIL intrinsics
// TODO: AMDILGenIntrinsics.inc is currently under AMDIL backend directory.
// So we cannot include it from an optimizer pass.
// Enable this when AMDILGenIntrinsics.inc is moved under llvm's include
// directory.
#if 0
AliasAnalysis::ModRefBehavior
AMDAliasAnalysis::getModRefBehavior(const Function *F) {
  // For intrinsics, we can check the table.
  if (unsigned iid = F->getIntrinsicID()) {
#define GET_INTRINSIC_MODREF_BEHAVIOR
#include "AMDILGenIntrinsics.inc"
#undef GET_INTRINSIC_MODREF_BEHAVIOR
  }

  ModRefBehavior Min = UnknownModRefBehavior;

  // Otherwise be conservative.
  return ModRefBehavior(AliasAnalysis::getModRefBehavior(F) & Min);
}
#endif
