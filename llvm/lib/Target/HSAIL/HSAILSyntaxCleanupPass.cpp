#include "HSAIL.h"
#include "HSAILUtilityFunctions.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/Module.h"

using namespace llvm;

namespace {
  /// \brief Fix LLVM various identifiers to satisfy HSAIL syntax
  ///
  /// This is intended to be a last-minute cleanup, just before
  /// instruction selection is invoked. Any issues with syntax should
  /// be implemented as additional functions that are called within
  /// runOnModule.
  class HSAILSyntaxCleanupPass : public ModulePass {
  public:
    static char ID;
    explicit HSAILSyntaxCleanupPass() : ModulePass(ID) {}

  private:
    virtual bool runOnModule(Module &M);
    virtual const char* getPassName() const {
      return "Fix identifiers for HSAIL syntax";
    }
  };

  char HSAILSyntaxCleanupPass::ID = 0;
}

ModulePass* llvm::createHSAILSyntaxCleanupPass() {
  return new HSAILSyntaxCleanupPass();
}
  // Register pass in passRegistry so that the pass info gets populated for printing debug info 
  INITIALIZE_PASS(HSAILSyntaxCleanupPass, "hsail-synid",
                "Fix identifiers for HSAIL syntax", false, false)

/// \brief Conservatively rename global variables 
/// and function names to satisfy HSAIL syntax.
///
/// Global variables that are automatically created by the compiler
/// tend to have dots "." in them, which is not allowed in HSAIL. We
/// rename these to remove all characters that are outside the HSAIL
/// syntax. We attempt this only on global variables that have local
/// scope, and only if they will be emitted to HSAIL.
static bool sanitizeGlobalNamesInModule(Module &M)
{
  bool Changed = false;

  for (Module::global_iterator I = M.global_begin(), E = M.global_end();
       I != E; ++I) {
    GlobalVariable *GV = I;
    if (!GV->hasLocalLinkage())
      continue;
    if (isIgnoredGV(GV))
      continue;
    
    Changed |= sanitizeGlobalValueName(GV);
  }

  // sanitize function names
  for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
    if (F->isDeclaration()) continue;
    Changed |= sanitizeGlobalValueName(F);
  }

  return Changed;
}  

bool HSAILSyntaxCleanupPass::runOnModule(Module &M)
{
  return sanitizeGlobalNamesInModule(M);
}

