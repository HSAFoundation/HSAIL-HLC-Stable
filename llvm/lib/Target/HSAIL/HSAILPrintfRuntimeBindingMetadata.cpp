//=== HSAILPrintfRuntimeBindingMetadata.cpp -- For openCL -- bind
//    Printf specific calls inserted at llvm pre-link to a kernel arg
//    pointer.
//===----------------------------------------------------------------------===//
//

#define DEBUG_TYPE "printfToRuntime"
#include "HSAIL.h"
#include "HSAILMachineFunctionInfo.h"
#include "HSAILModuleInfo.h"
#include "HSAILUtilityFunctions.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/AMDLLVMContextHook.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/Function.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/AMDMetadataUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

namespace {
  class HSAILPrintfRuntimeBindingMetadata : public FunctionPass {
  public:
    static char ID;
    explicit HSAILPrintfRuntimeBindingMetadata() :
      FunctionPass(ID), HSATM(0) {};
    explicit HSAILPrintfRuntimeBindingMetadata(HSAILTargetMachine *HSATM)
      : FunctionPass(ID), HSATM(HSATM) {};
    virtual bool runOnFunction(Function &F);
    bool doInitialization(Module &M){
      return false;
    }
    bool doFinalization(Module &M){
      return false;
    }
    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<MachineFunctionAnalysis>();
      FunctionPass::getAnalysisUsage(AU);
      AU.setPreservesAll();
    }
  private:
    const HSAILTargetMachine *HSATM;

    virtual const char* getPassName() const {
      return "HSAIL Printf lowering part 3";
    }
  };
  char HSAILPrintfRuntimeBindingMetadata::ID = 0;

} // end anonymous namespace

INITIALIZE_PASS(HSAILPrintfRuntimeBindingMetadata,
"hsail-printf-binding-metadata",
"Insert printf buffer HSAIL kernargs",
false, false)

FunctionPass* llvm::createHSAILPrintfRuntimeBindingMetadata(
  HSAILTargetMachine &HSATM) {
  return new HSAILPrintfRuntimeBindingMetadata(&HSATM);
}

bool HSAILPrintfRuntimeBindingMetadata::runOnFunction(Function &F) {
  HSAILMachineFunctionInfo * mMFI
    = getAnalysis<MachineFunctionAnalysis>().getMF()
        .getInfo<HSAILMachineFunctionInfo>();
  if (!isKernelFunc(&F)) return false;

  int printf_uniqnum = 0;
  for (Function::iterator bb_it = F.begin(),
      bb_end = F.end(); bb_it != bb_end; ++bb_it) {
    for (BasicBlock::iterator inst_it = bb_it->begin(),
         inst_end = bb_it->end(); inst_it != inst_end;) {
      Instruction* inst = &*inst_it++;
      MDNode *idmd = inst->getMetadata("prnFmt");
      if (idmd) {
        printf_uniqnum++;
        ConstantDataArray *CA
          = dyn_cast<ConstantDataArray>(idmd->getOperand(0));
        assert(CA && "printf_id meta data required in ConstantDataArray");
        StringRef str = CA->getAsCString();
        mMFI->addMetadata(str.str(),true);
        DEBUG(dbgs() << "created metadata for printf fmtstring "
              << str.str() << '\n');
      }
    }
  }
  DEBUG(dbgs() << "number of printf format strings:"
        << printf_uniqnum << '\n');
  return true;
}
