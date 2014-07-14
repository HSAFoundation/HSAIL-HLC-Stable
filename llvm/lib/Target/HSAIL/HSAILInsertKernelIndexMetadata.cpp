//==-------- AMDHSAILInsertKernelIndexMetadata.cpp --------------*- C++ -*-===//
//
// Copyright(c) 2014 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief Record all the kernels which has enqueue_kernel calls in
///  "opencl.kernels" Metadata and replace the function pointers with the
///  kernel index (temporary approach).
//
///  This will be lowered to the HSAIL RTI metadata. That will be used
///  by the runtime to do some setup for enqueuing the kernel.
//
//===----------------------------------------------------------------------===//

#include "HSAIL.h"
#include "HSAILUtilityFunctions.h"

#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/AMDMetadataUtils.h"
#include "llvm/Transforms/AMDSPIRUtils.h"

using namespace llvm;

namespace {

class HSAILInsertKernelIndexMetadata : public ModulePass {
public:
  static char ID;
  explicit HSAILInsertKernelIndexMetadata() : ModulePass(ID) {};

private:
  virtual bool runOnModule(Module &M);

  virtual const char *getPassName() const {
    return "Insert OpenCL kernel Index Metadata";
  }
};
} // end anonymous namespace
char HSAILInsertKernelIndexMetadata::ID = 0;

typedef std::map<const Function *, unsigned> KernelIndexMap;

INITIALIZE_PASS(HSAILInsertKernelIndexMetadata, "hsail-kernel-idx-metadata",
                "Insert Kernel Index metadata", false, false);
namespace llvm {
ModulePass *createHSAILInsertKernelIndexMetadataPass() {
  return new HSAILInsertKernelIndexMetadata();
}
}

/// \breif Add kernel index metadata. The index is same as the order
/// of kernels mentioned in the
/// TODO: Remove this metadata, when the LDK instruction is ready.
static void AddKernelIndexMetadata(Module &TheModule,
                                   KernelIndexMap &IndexMap) {

  LLVMContext &Context = TheModule.getContext();
  llvm::NamedMDNode *OpenCLMetadata =
      TheModule.getOrInsertNamedMetadata("opencl.kernels");
  MDString *KernelIdxString = MDString::get(Context, "kernel_index");
  SmallVector<MDNode *, 4> NewOpenCLMDNodes;

  for (unsigned KernelIndex = 0, End = OpenCLMetadata->getNumOperands();
       KernelIndex != End; ++KernelIndex) {
    MDNode *KernelData = cast<MDNode>(OpenCLMetadata->getOperand(KernelIndex));

    // Add the kernel --> index to the map.
    Function *KernelFn = cast<Function>(KernelData->getOperand(0));
    IndexMap[KernelFn] = KernelIndex;

    SmallVector<Value *, 4> KernelDataOperands;
    for (unsigned I = 0; I < KernelData->getNumOperands(); ++I) {
      KernelDataOperands.push_back(KernelData->getOperand(I));
    }

    SmallVector<Value *, 4> NewMDNodes;
    NewMDNodes.push_back(KernelIdxString);
    APInt Index = APInt(32, KernelIndex);
    ConstantInt *KernelIndexVal = ConstantInt::get(Context, Index);
    NewMDNodes.push_back(KernelIndexVal);

    KernelDataOperands.push_back(MDNode::get(Context, NewMDNodes));
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

/// \brief Replace all function uses with the kernel index.
static void replaceAllFuncUses(Function *F, unsigned Index) {
  assert(F->getNumUses());

  for (Value::use_iterator UI = F->use_begin(), E = F->use_end(); UI != E;
       ++UI) {

    ConstantExpr *FuncPointerInst = cast<ConstantExpr>(*UI);
    Type *UseType = FuncPointerInst->getType();
    Constant *KernelIndexVal =
            ConstantInt::get(UseType, Index);
    PointerType *UsePtrType  = dyn_cast<PointerType>(UseType);
    if (UsePtrType) {
      KernelIndexVal = ConstantExpr::getIntToPtr(KernelIndexVal, UsePtrType);
    }
    FuncPointerInst->replaceAllUsesWith(KernelIndexVal);
  }
}

/// \brief Replace all the function pointers wiith the corresponding kernel
/// indexes.
static void replaceFuncPointersWithKerneIndex(Module &M,
                                              KernelIndexMap &IndexMap) {
  // find all the block inovocation functions in the module and
  // replace the uses of it with the corresponding block kernel index.
  for (Module::iterator FI = M.begin(), E = M.end(); FI != E; ++FI) {

    if (isKernelFunc(FI))
      continue;

    StringRef FuncName = FI->getName();
    if (!FuncName.startswith("__amd_blocks_func_"))
      continue;

    std::string KernelName = "__OpenCL_" + FuncName.str() + "_kernel";
    Function *KernelFunc = M.getFunction(KernelName);
    assert(KernelFunc && isKernelFunc(KernelFunc));

    // Get index for the kernel.
    KernelIndexMap::iterator KernelIter = IndexMap.find(KernelFunc);
    assert(KernelIter != IndexMap.end());
    replaceAllFuncUses(FI, KernelIter->second);
  }
}

bool HSAILInsertKernelIndexMetadata::runOnModule(Module &M) {
  if (!isOpenCL20Module(M))
    return false;
  // map to find the index for the given kernel.
  KernelIndexMap IndexMap;

  AddKernelIndexMetadata(M, IndexMap);
  replaceFuncPointersWithKerneIndex(M, IndexMap);
  return true;
}
