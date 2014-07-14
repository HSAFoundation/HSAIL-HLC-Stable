//==- AMDGenerateDevEnqMetadata.cpp - Generate DevEnq Metadata --*- C++ -*-===//
//
// Copyright(c) 2014 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief Record all the kernels which has enqueue_kernel calls in
///  "opencl.kernels" Mmtadata.
//
///  This will be lowered to the HSAIL RTI metadata. That will be used
///  by the runtime to do some setup for enqueuing the kernel.
//
//===----------------------------------------------------------------------===//

#define DEBUGTYPE "AMDGenerateDevEnqMetadata"

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Attributes.h"
#include "llvm/DataLayout.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Intrinsics.h"
#include "llvm/IRBuilder.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/AMDSPIRUtils.h"

#include <list>

using namespace llvm;

namespace llvm {
class AMDGenerateDevEnqMetadata : public ModulePass {

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DataLayout>();
  }

public:
  static char ID;

  AMDGenerateDevEnqMetadata() : ModulePass(ID) {
    initializeAMDGenerateDevEnqMetadataPass(*PassRegistry::getPassRegistry());
  }

  virtual bool runOnModule(Module &M);
};
}

INITIALIZE_PASS(AMDGenerateDevEnqMetadata, "amd-opencl-gen-dev-enq-metadata",
                "Generate Dev Enq metadata", false, false);

char AMDGenerateDevEnqMetadata::ID = 0;

namespace llvm {
ModulePass *createAMDGenerateDevEnqMetadataPass() {
  return new AMDGenerateDevEnqMetadata();
}
}

/// \brief Find all the kernels by traversing all the uses of the
/// functions till the kernels doesn't have any use and emit the metadata.
/// Using BFS alogrithm to traverse the call graph in the bottom-up order.
static void findAndEmitEnqKernelMetadata(Module &TheModule,
                                         std::list<Function *> &FnList) {
  // Record all the kernels to find the metadata.
  LLVMContext &Context = TheModule.getContext();
  SmallSet<Function *, 4> KernelsList;
  SmallSet<Function *, 8> FuncsProcessed;
  while (FnList.size()) {

    Function *CurrFn = FnList.front();
    FnList.pop_front();
    // Func already handled.
    if (FuncsProcessed.count(CurrFn))
      continue;

    FuncsProcessed.insert(CurrFn);
    if (CurrFn->getCallingConv() == CallingConv::SPIR_KERNEL)
      KernelsList.insert(CurrFn);

    for (Value::use_iterator FUI = CurrFn->use_begin(), E = CurrFn->use_end();
         FUI != E; ++FUI) {

      CallInst *CI = dyn_cast<CallInst>(*FUI);
      if (!CI)
        continue;

      BasicBlock *BBUsed = CI->getParent();
      Function *FuncUsed = BBUsed->getParent();
      FnList.push_back(FuncUsed);
    }
  }
  llvm::NamedMDNode *OpenCLMetadata =
      TheModule.getOrInsertNamedMetadata("opencl.kernels");
  MDString *DevEnqData = MDString::get(Context, "device_enqueue");
  SmallVector<MDNode *, 4> NewOpenCLMDNodes;

  // find the opencl.kernels metadata for the kernel.
  for (unsigned OpNum = 0, End = OpenCLMetadata->getNumOperands(); OpNum != End;
       ++OpNum) {
    MDNode *KernelData = cast<MDNode>(OpenCLMetadata->getOperand(OpNum));

    if (!KernelsList.count(cast<Function>(KernelData->getOperand(0)))) {
      NewOpenCLMDNodes.push_back(KernelData);
      continue;
    }
    assert(KernelData);

    SmallVector<Value *, 4> KernelDataOperands;
    for (unsigned I = 0; I < KernelData->getNumOperands(); ++I) {
      KernelDataOperands.push_back(KernelData->getOperand(I));
    }
    KernelDataOperands.push_back(MDNode::get(Context, DevEnqData));

    NewOpenCLMDNodes.push_back(MDNode::get(Context, KernelDataOperands));
  }

  OpenCLMetadata->eraseFromParent();
  llvm::NamedMDNode *NewOpenCLMetadata =
      TheModule.getOrInsertNamedMetadata("opencl.kernels");
  for (SmallVector<MDNode *, 4>::iterator MDI = NewOpenCLMDNodes.begin(),
                                          E = NewOpenCLMDNodes.end();
       MDI != E; ++MDI) {
    NewOpenCLMetadata->addOperand(*MDI);
  }
}

/// \brief Find all the kernels which has enqueue_kernel calls
/// directly or indirectly ( via other fucntion calls) and record
/// it in metadata.
static bool generateEnqKernelFnUseMetadata(Module &M) {

  // List all the functions which has enqueue_kernel() call.
  std::list<Function *> FunctionList;

  for (Module::iterator Fn = M.begin(), FE = M.end(); Fn != FE; ++Fn) {
    if (!Fn->empty())
      continue;
    // Search for the library function used for the
    StringRef FuncName = Fn->getName();
    if (!FuncName.startswith("__enqueue_internal_"))
      continue;

    FunctionList.push_back(Fn);
  }

  if (!FunctionList.size())
    return false;

  findAndEmitEnqKernelMetadata(M, FunctionList);
  return true;
}

bool AMDGenerateDevEnqMetadata::runOnModule(Module &M) {
  DEBUG(dbgs() << "AMD Generate Device Enqueue Related Metadata pass");
  return generateEnqKernelFnUseMetadata(M);
}
