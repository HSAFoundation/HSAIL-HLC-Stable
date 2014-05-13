#define DEBUG_TYPE "inline"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/CallingConv.h"
#include "llvm/Instructions.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Module.h"
#include "llvm/Type.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/AMDOpenCLSymbols.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/InlinerPass.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/GlobalAlias.h"
#include "llvm/ADT/SmallPtrSet.h"

using namespace llvm;

namespace {

  // If wholeProgram is true, AMDInlineAll tries to inline all calls; if
  // wholeProgram is false, it tries to inline all calls except those to
  // weak functions.
  class AMDInlineAll : public Inliner {
    bool wholeProgram; // Whether this is a whole program
    InlineCostAnalyzer CA;

    // If any of F's aliases are dead, remove them.
    bool removeDeadFunctionAliasUsers(Function *F);

  public:
    // Use extremely low threshold.
    AMDInlineAll(bool isWhole=false) : Inliner(ID, -2000000000, true),
                                       wholeProgram(isWhole)
    {
      initializeAMDInlineAllPass(*PassRegistry::getPassRegistry());
    }
    static char ID; // Pass identification, replacement for typeid
    InlineCost getInlineCost(CallSite CS) {
      Function *Callee = CS.getCalledFunction();

      if (!wholeProgram) {
        if (Callee->mayBeOverridden())
          return InlineCost::getNever();
      }

      Function *Caller = CS.getCaller();
      // Support function call in amdil
      if (!Caller->getFnAttributes().hasAttribute(Attributes::NoInline) &&
          Callee->getFnAttributes().hasAttribute(Attributes::NoInline)) {
        DEBUG_WITH_TYPE("noinline",
          llvm::dbgs() << "[AMDILInlineAll::getInlineCost] "
            << CS.getCalledFunction()->getName()
            << " (noinline) is called by "
            << CS.getCaller()->getName()
            << " (non noinline). Do not inline.\n");
        return InlineCost::getNever();
      }
      return InlineCost::getAlways();
    }
    float getInlineFudgeFactor(CallSite CS) {
      return 1.0f; // The return value should not matter.
    }
	  void growCachedCostInfo(Function *Caller, Function *Callee) {
	  //	CA.growCachedCostInfo(Caller, Callee);
	  }
    void getAnalysisUsage(AnalysisUsage &AU) const {
      Inliner::getAnalysisUsage(AU);
      AU.addRequired<OpenCLSymbols>();
      AU.addPreserved<OpenCLSymbols>();
    }
    virtual bool doFinalization(CallGraph &CG);
  };
}

char AMDInlineAll::ID = 0;
INITIALIZE_PASS(AMDInlineAll, "inline-all", "Inliner for inlining all functions", false, false);

Pass *llvm::createAMDInlineAllPass(bool isWhole) {
  return new AMDInlineAll(isWhole);
}

bool AMDInlineAll::removeDeadFunctionAliasUsers(Function *F)
{
  bool changed = false;
  if (F->use_empty())
    return changed;

  SmallPtrSet<GlobalAlias *, 16> FuncAliasToRemove;
  for (Value::use_iterator I = F->use_begin(), E = F->use_end();
       I != E; ++I) {
    if (GlobalAlias *aUser = dyn_cast<GlobalAlias>(*I)) {
      if (aUser->use_empty())
        FuncAliasToRemove.insert(aUser);
    }
  }

  for (SmallPtrSet<GlobalAlias*, 16>::iterator I = FuncAliasToRemove.begin(),
       E = FuncAliasToRemove.end(); I != E; ++I) {
      DEBUG(errs() << "<InlineAll> " << (*I)->getName()
                   << " (function alias): non-kernel function, removed!\n");
      (*I)->eraseFromParent();
      changed = true;
  }

  return changed;
}

bool AMDInlineAll::doFinalization(CallGraph &CG) {
  bool Changed = false;

  if (!wholeProgram)
    Changed = removeDeadFunctions(CG);
  else {
    const OpenCLSymbols &OCLS = getAnalysis<OpenCLSymbols>();
    SmallPtrSet<CallGraphNode*, 16> FunctionsToRemove;

    // Delete all functions except kernel functions. A kernel function
    // is one whose name starts with __OPENCL_ and ends with _kernel, ie,
    //    _OPENCL_<name>_kernel
    for (CallGraph::iterator I = CG.begin(), E = CG.end(); I != E; ++I) {
      CallGraphNode *CGN = I->second;
      if (CGN == 0 || CGN->getFunction() == 0)
        continue;

      Function *F = CGN->getFunction();

      // Allow library functions to be not in this module.
      if (!F || F->isDeclaration())
        continue;

      // Skip kernel and reserved functions
      if (OCLS.isKnown(F))
        continue;

      F->removeDeadConstantUsers();

      removeDeadFunctionAliasUsers(F);

      // If F still has users (unlikely), don't delete it.
      if (!F->use_empty())
        continue;

      // Remove any call graph edges from the function to its callees.
      CGN->removeAllCalledFunctions();

      // Remove any edges from the external node to the function's call graph
      // node.  These edges might have been made irrelegant due to
      // optimization of the program.
      CG.getExternalCallingNode()->removeAnyCallEdgeTo(CGN);

      // Removing the node for callee from the call graph and delete it.
      FunctionsToRemove.insert(CGN);
    }

    // Now that we know which functions to delete, do so.  We didn't want to do
    // this inline, because that would invalidate our CallGraph::iterator
    // objects. :(
    //
    // Note that it doesn't matter that we are iterating over a non-stable set
    // here to do this, it doesn't matter which order the functions are deleted
    // in.
    for (SmallPtrSet<CallGraphNode*, 16>::iterator I = FunctionsToRemove.begin(),
         E = FunctionsToRemove.end(); I != E; ++I) {
      DEBUG(errs() << "<InlineAll> " << (*I)->getFunction()->getName()
                   << ": non-kernel function, removed!\n");
      delete CG.removeFunctionFromModule(*I);
      Changed = true;
    }
  }

  return Changed;
}
