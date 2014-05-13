//===- AMDILCreateKernelStub.cpp - lower TIB usage ----------------------===//
//
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
// This pass creates a stub function for each kernel function.
// The stub function does nothing but make a call to the kernel function.
// The parameters passed to the callee will be null values. These
// values will be ignored during lowering.
// This is so that the setup of kernel args can be done during lowering
// of the call to the kernel function.
//
// E.g. if the kernel is: __kernel void foo(__global int* out);
// The stub being generated will be:
//   void __OpenCL_foo_stub() {
//     foo(NULL);
//   }
//===----------------------------------------------------------------------===//

#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "AMDIL.h"
#include "../../Transforms/IPO/AMDSymbolName.h"
using namespace llvm;

namespace llvm {

//===--------------------------------------------------------------------===//
// AMDILCreateKernelStub pass implementation.
class AMDILCreateKernelStub : public ModulePass {
private:

public:
  static char ID; // Pass identification, replacement for typeid
  AMDILCreateKernelStub() : ModulePass(ID) {
    initializeAMDILCreateKernelStubPass(*PassRegistry::getPassRegistry());
  }

  virtual bool runOnModule(Module& M);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
  }

private:
  void createStubForKernel(Function &F);
};
} // end anonymous namespace

char AMDILCreateKernelStub::ID = 0;
INITIALIZE_PASS(AMDILCreateKernelStub, "amdcreatekernelstub",
                "AMD Create Kernel Stubs", false, false)

Pass *llvm::createAMDILCreateKernelStubPass() {
  return new AMDILCreateKernelStub();
}

bool AMDILCreateKernelStub::runOnModule(Module& M) {
  typedef SmallVector<Function*, 1> FunctionListTy;

  // Find all kernels defined within the module
  FunctionListTy Kernels;
  for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
    Function &F = *I;
    if (AMDSymbolNames::isKernelFunctionName(F.getName())) {
      Kernels.push_back(&F);
    }
  }

  // Create a stub for each kernel function
  for (FunctionListTy::iterator FI = Kernels.begin(), FE = Kernels.end();
    FI != FE; ++FI) {
    createStubForKernel(**FI);
  }

  return true;
}

void AMDILCreateKernelStub::createStubForKernel(Function &KernelF) {
  Module &M = *KernelF.getParent();
  LLVMContext &Ctx = M.getContext();

  // Stub's type is void f().
  Type *VoidTy = Type::getVoidTy(Ctx);
  FunctionType *StubFTy = FunctionType::get(VoidTy, false/*not vararg*/);

  // Form the name of the stub function.
  StringRef BareName
    = AMDSymbolNames::undecorateKernelFunctionName(KernelF.getName());
  std::string StubName = AMDSymbolNames::decorateStubFunctionName(BareName);
  assert(M.getFunction(StubName) == NULL && "stub already exist");

  // Create the stub function.
  Function *StubF
    = Function::Create(StubFTy, Function::ExternalLinkage, StubName, &M);
  StubF->setDoesNotThrow();

  // Create the entry block.
  BasicBlock *EntryBB = BasicBlock::Create(Ctx, "entry", StubF);

  // Create the arg list will all-null values.
  SmallVector<Value*, 8> Args;
  for (Function::arg_iterator I = KernelF.arg_begin(), E = KernelF.arg_end();
    I != E; ++I) {
    Argument *Arg = I;
    Type *ArgTy = Arg->getType();
    PointerType *PTy = dyn_cast<PointerType>(ArgTy);
    Value *ArgVal = NULL;
    // A byval struct type kernel arg is passed in by a pointer to private mem.
    // This was done in the fronend.
    // Here we reserve private memory on the stack which will be used
    // to pass the struct value to the kernel.
    if (PTy && PTy->getAddressSpace() == AMDILAS::PRIVATE_ADDRESS) {
      Type *ETy = PTy->getElementType();
      const StructType *STy = dyn_cast<StructType>(ETy);
      // skip opaque types
      bool IsOpaqueTy;
      IsOpaqueTy = STy && STy->isOpaque();
      if (!IsOpaqueTy) {
        ArgVal = new AllocaInst(ETy, "", EntryBB);
      }
    }
    if (ArgVal == NULL) {
      ArgVal = Constant::getNullValue(ArgTy);
    }
    Args.push_back(ArgVal);
  }

  // Generate a call to the kernel.
  CallInst *CallK = CallInst::Create(&KernelF, Args, "", EntryBB);
  CallK->setCallingConv(KernelF.getCallingConv());

  // Prevents inlining the kernel into the stub.
  CallK->setIsNoInline();

  // Create the return instruction.
  ReturnInst::Create(Ctx, EntryBB);
}
