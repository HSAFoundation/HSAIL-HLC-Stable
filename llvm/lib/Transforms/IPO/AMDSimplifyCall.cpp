#define DEBUG_TYPE "simplifycall"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;

namespace {

  class AMDSimplifyCall : public ModulePass {
  public:
    static char ID; // Pass identification, replacement for typeid
    AMDSimplifyCall()
      : ModulePass(ID) {
      initializeAMDSimplifyCallPass(*PassRegistry::getPassRegistry());
    }

    virtual ~AMDSimplifyCall() {}

    bool runOnModule(Module &M);

    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    }
  };
}

char AMDSimplifyCall::ID = 0;
INITIALIZE_PASS(AMDSimplifyCall, "directcall",
  "Make calls direct by removing func alias, etc", false, false);

ModulePass *llvm::createAMDSimplifyCallPass() {
  return new AMDSimplifyCall();
}

bool AMDSimplifyCall::runOnModule(Module &M)
{
  bool changed = false;
  for (Module::alias_iterator I = M.alias_begin(), E = M.alias_end();
       I != E; ++I) {
    if (I->isWeakForLinker()) {
      continue;
    }
    GlobalAlias *Aliaser = I;
    if (Constant *Aliasee = Aliaser->getAliasee()) {
      if (isa<Function>(Aliasee)) {
        Constant *NewAliasee = 
          ConstantExpr::getBitCast(Aliasee, Aliaser->getType());
        Aliaser->replaceAllUsesWith(NewAliasee);
        changed = true;

        DEBUG(errs() << "<AMDSimplifyCall> func alias replaced with its aliasee: "
                     << *I << "\n");
      }
    }
  }
  return changed;
}
