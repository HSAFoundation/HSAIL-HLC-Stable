//===- AMDRemoveNoalias.cpp - Remove noalias attribute if needed ----===//
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
// One solution is to implement AMD OpenCL specific alias analysis pass, which
// is chained after other standard alias analysis passes.
// See AMDAliasAnalysis.cpp for details.
// The other solution is implemented in this file. This file implements the
// AMDRemoveNoalias pass, which removes the "noalias" attribute of a kernel
// pointer if the kernel directly or indirectly calls a barrier/fence function,
// and the pointer is in the same address space as the barrier/fence.
// As an optimization, we detect read-only pointers, and don't remove the
// "noalias" attribute of read-only pointers. This will allow the AMDIL backend
// to mark UAVs accessed exclusively by the read-only pointers as "read-only"
// and allow SC to optimize to optimize loads from read-only UAVs.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "amd-remove-noalias"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AMDFenceInfoAnalysis.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO.h"
#include <map>
#include <set>

using namespace llvm;

namespace {
  // the definition below has to be consistant with the definition in AMDILAS
  // namespace
  enum {
    GLOBAL_ADDRESS   = 1, // Address space for global memory.
    LOCAL_ADDRESS    = 3, // Address space for local memory.
    REGION_ADDRESS   = 4, // Address space for region memory.
  };

  // Helper class that finds out if a pointer type kernel argument is read-only
  class ReadOnlyPointerFinder {
  public:
    ReadOnlyPointerFinder(AliasAnalysis &aa)
      : AA(aa), ArgReadOnlyCache(), ArgReachReturnCache() {}
    // returns if the argument is read-only
    bool isReadOnlyArg(const Argument &Arg);

  private:
    // detect whether an arument is read-only
    bool detectArgReadOnly(const Argument &Arg);

  private:
    AliasAnalysis &AA;
    // Caches argument read-only detection results
    std::map<const Argument*, bool> ArgReadOnlyCache;
    // Caches result of whether a pointer type argument reaches return value
    std::map<const Argument*, bool> ArgReachReturnCache;
  };

  // The AMDRemoveNoalias class
  class AMDRemoveNoalias : public ModulePass {
  public:
    static char ID; // Class identification
    AMDRemoveNoalias() : ModulePass(ID) {
      initializeAMDRemoveNoaliasPass(*PassRegistry::getPassRegistry());
    }
    ~AMDRemoveNoalias() {}

    virtual void getAnalysisUsage(AnalysisUsage &AU) const;

    virtual bool runOnModule(Module &M);

  private:
    bool removeNoalias(Module &M);
  };
}  // End of namespace

bool ReadOnlyPointerFinder::isReadOnlyArg(const Argument &Arg)
{
  std::map<const Argument*, bool>::iterator I
    = ArgReadOnlyCache.find(&Arg);
  if (I != ArgReadOnlyCache.end()) {
    DEBUG(dbgs() << "  cached read-only result for arg " << Arg.getName()
                 << " is " << I->second << "\n");
    return I->second;
  }

  return detectArgReadOnly(Arg);
}

bool ReadOnlyPointerFinder::detectArgReadOnly(const Argument &Arg)
{
  DEBUG(dbgs() << "  Start tracing pointer arg " << Arg.getName() << "\n");

  ArgReachReturnCache[&Arg] = false;

  std::vector<const Value*> Worklist;
  std::set<const Value*> Visited;

  Worklist.push_back(&Arg);
  Visited.insert(&Arg);

  while (!Worklist.empty()) {
    const Value *V = Worklist.back();
    Worklist.pop_back();
    for (Value::const_use_iterator I = V->use_begin(), E = V->use_end();
      I != E; ++I) {
      const User *U = *I;
      const Instruction *Inst = dyn_cast<Instruction>(U);
      if (!Inst) {
        continue;
      }
      if (Visited.count(U)) {
        continue;
      }
      Visited.insert(U);
      bool Terminate = false;
      switch (Inst->getOpcode()) {
      case Instruction::Load:
      case Instruction::ICmp:
      case Instruction::FCmp:
      case Instruction::Switch:
        // Terminates the tracing
        Terminate = true;
        break;;
      case Instruction::Store:
      case Instruction::AtomicCmpXchg:
      case Instruction::AtomicRMW:
        // if "V" is store address, arg is not readonly
        // if "V" is data to be stored, conservatively mark arg as not readonly
        ArgReadOnlyCache[&Arg] = false;
        DEBUG(dbgs() << "  Found write. Arg not read-only\n");
        return false;
      case Instruction::Ret:
        ArgReachReturnCache[&Arg] = true;
        Terminate = true;
        DEBUG(dbgs() << "  Arg reaches return\n");
        break;
      case Instruction::Invoke:
      case Instruction::Call:
      {
        ImmutableCallSite CS(Inst);
        const Function *Callee = CS.getCalledFunction();
        if (!Callee)
          break;
        // handle calls to intrinsics conservatively
        const IntrinsicInst *II = dyn_cast<IntrinsicInst>(CS.getInstruction());
        if (II) {
          AliasAnalysis::ModRefBehavior MR = AA.getModRefBehavior(Callee);
          if (MR == AliasAnalysis::DoesNotAccessMemory ||
              MR == AliasAnalysis::OnlyReadsMemory) {
            ArgReadOnlyCache[&Arg] = true;
          } else {
            ArgReadOnlyCache[&Arg] = false;
            DEBUG(dbgs() << "  Reaches write intrinsic call."
                         << " Arg not read-only\n");
            return false;
          }
          // fallthru to conservatively continue tracing intrinsic returns
        } else {
          // trace into normal callee functions
          unsigned ArgNo = CS.getArgumentNo(I);
          Function::const_arg_iterator AI = Callee->arg_begin();
          Function::const_arg_iterator AE = Callee->arg_end();
          for (unsigned i = 0; AI != AE && i < ArgNo; ++AI, ++i) {
          }
          assert(AI != AE && "no such argument");
          if (!isReadOnlyArg(*AI)) {
            ArgReadOnlyCache[&Arg] = false;
            return false;
          }
          // terminate if the arg does not reach return in callee
          std::map<const Argument*, bool>::iterator RI
            = ArgReachReturnCache.find(AI);
          assert(RI != ArgReachReturnCache.end() && "Callee not traced yet");
          Terminate = !RI->second;
        }
        break;
      }
      default:
        assert(Inst->getOpcode() != Instruction::VAArg && "unimplemented");
        break;
      }
      if (!Terminate)
        Worklist.push_back(U);
        DEBUG(dbgs() << "  Pushed use " << *U << " to worklist\n");
    }
  }
  ArgReadOnlyCache[&Arg] = true;
  DEBUG(dbgs() << "  Arg " << Arg.getName() << " readonly\n");
  return true;
}

// Register this pass...
char AMDRemoveNoalias::ID = 0;
INITIALIZE_PASS(AMDRemoveNoalias, "amd-remove-noalias",
                "Remove Noalias Attr If Needed", false, false)

ModulePass *llvm::createAMDRemoveNoaliasPass() {
  return new AMDRemoveNoalias();
}

void AMDRemoveNoalias::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<AliasAnalysis>();
  AU.addRequired<AMDFenceInfoAnalysis>();
  AU.setPreservesAll();                         // Does not transform code
}

// Remove the "noalias" attribute from a pointer type kernel argument if the
// kernel calls directly or indirectly a barrier or memory fence intrinsic,
// and if the kernel argument is in the same address space as the fence.
// As an optimization, do not remove noalias from read-only pointers.
bool AMDRemoveNoalias::removeNoalias(Module &M) {
  const AMDFenceInfoAnalysis &FenceInfo = getAnalysis<AMDFenceInfoAnalysis>();
  if (!FenceInfo.ModuleHasFenceCalls()) {
    DEBUG(dbgs() << "Module has no fence calls. No need to remove noalias\n");
    return false;
  }

  DEBUG(dbgs() << M << "\n");

  AliasAnalysis &AA = getAnalysis<AliasAnalysis>();
  ReadOnlyPointerFinder ROPFinder(AA);

  bool changed = false;
  for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F) {
    if (F->isDeclaration()) continue;
    unsigned Flags = FenceInfo.getFenceFlags(*F);
    // skip functions that do not call mem fences
    if (Flags == 0) {
      DEBUG(dbgs() << "Function " << F->getName()
                   << " not call fences. No need to remove noalias\n");
      continue;
    }
    DEBUG(dbgs() << "Function " << F->getName() << " fence flags: ";
          AMDFenceInfoAnalysis::dumpFenceFlags(Flags); dbgs() << "\n");
    unsigned i = 1;
    for (Function::arg_iterator Arg = F->arg_begin(), AE = F->arg_end();
      Arg != AE; ++Arg, ++i) {
      // only pointer can have noalias attribute
      PointerType *ArgTy = dyn_cast<PointerType>(Arg->getType());
      if (!ArgTy) continue;
      // skip arguments without NoAlias attribute
      if (!F->doesNotAlias(i)) continue;

      // skip arguments whose address space is not the same as the fences
      unsigned AS = ArgTy->getAddressSpace();
      if ((AS == GLOBAL_ADDRESS &&
           (Flags & AMDFenceInfoAnalysis::GLOBAL_MEM_FENCE) == 0) ||
          (AS == LOCAL_ADDRESS &&
           (Flags & AMDFenceInfoAnalysis::LOCAL_MEM_FENCE) == 0) ||
          (AS == REGION_ADDRESS &&
           (Flags & AMDFenceInfoAnalysis::REGION_MEM_FENCE) == 0)) {
        DEBUG(dbgs() << "  Arg " << Arg->getName() << " not in same address "
                     << "space as fence. No need to remove noalias\n");
        continue;
      }

      // skip read-only pointer arguments
      if (ROPFinder.isReadOnlyArg(*Arg)) {
        continue;
      }

      AttrBuilder B;
      B.addAttribute(Attributes::NoAlias);

      // Remove the NoAlias attribute from the pointer arg
      F->removeAttribute(i, Attributes::get(M.getContext(), B));

      DEBUG(dbgs() << "  Removed noalias for arg " << Arg->getName() << "\n");
      changed = true;
    }
  }
  return changed;
}

bool AMDRemoveNoalias::runOnModule(Module &M) {
  bool changed = removeNoalias(M);
  return changed;
}








