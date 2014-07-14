//==- AMDX86Adapter.cpp - Fix x86 builtin calls for user Module -*- C++ -*-===//
//
// Copyright(c) 2014 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief Fix all the "undefined" x86 builtin function calls in the user kernel
///  to match the defintion in the Clang compiled x86 builtins library.
//
///  The user kernel can be compiled either by EDG or Clang. In both the cases,
///  user kernel will have the Clang name mangling scheme which includes the
///  Address spaces as well. But the builtin library will be compiled by
///  Clang and this will drop all the address spaces and it will always have
///  address space 0 for the x86 target.
//
///  This pass defines all the "undefined" x86 builtin functions (which has
///  non-default address space pointer arguments) to call the the function,
///  which matches the function signature defined in the x86 builtins library.
//
//===----------------------------------------------------------------------===//

#define DEBUGTYPE "AMDX86Adapter"

#include "llvm/ADT/StringRef.h"
#include "llvm/Attributes.h"
#include "llvm/DataLayout.h"
#include "llvm/Instructions.h"
#include "llvm/IRBuilder.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/AMDSPIRUtils.h"

#include <string>

using namespace llvm;

namespace llvm {
class AMDX86Adapter : public ModulePass {

public:
  static char ID;

  AMDX86Adapter() : ModulePass(ID) {
    initializeAMDX86AdapterPass(*PassRegistry::getPassRegistry());
  }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DataLayout>();
  }

  virtual bool runOnModule(Module &M);
};
}

INITIALIZE_PASS(AMDX86Adapter, "amd-opencl-x86-adapter",
                " Fix name mangling for x86 builtins", false, false);

char AMDX86Adapter::ID = 0;

namespace llvm {
ModulePass *createAMDX86AdapterPass() { return new AMDX86Adapter(); }
}

/// \brief Check wehther the type is a pointer and returns its
/// address space.
/// returns 0 for opaque type pointers.
static size_t getPtrAddrSpace(Type *Ty) {
  PointerType *PtrType = dyn_cast<PointerType>(Ty);
  if(!PtrType) return 0;
  StructType* StrType = dyn_cast<StructType>(PtrType->getElementType());
  if(StrType && StrType->isOpaque()) return 0;
  return (PtrType->getAddressSpace() );
}

/// \brief Check wehther the type is a pointer and also whether it points to
/// non-default address space.If it is not an opaque type, return true.
/// Always skip opaque types because they are not "real" pointers.
static bool isNonDefaultAddrSpacePtr(Type *Ty) {
  PointerType *PtrType = dyn_cast<PointerType>(Ty);
  if(!PtrType) return false;
  StructType* StrType = dyn_cast<StructType>(PtrType->getElementType());
  if(StrType && StrType->isOpaque()) return false;
  return (PtrType->getAddressSpace() != 0);
}

/// \brief Check whether the Function signature has any of the
/// non-default address space pointers as arguments. If yes,
/// this funtion will return true.
static bool hasNonDefaultAddrSpaceArg(Function *F) {

  for (Function::arg_iterator AI = F->arg_begin(), E = F->arg_end();
       AI != E; ++AI)
    if (isNonDefaultAddrSpacePtr(AI->getType()))
      return true;
  return false;
}

/// \brief Locate the position of the function name in the mangled OpenCL
/// builtin function. Returns true on failure.
static bool locateFuncName(StringRef FuncName, size_t &FuncNameStart,
                           size_t &FuncNameSize) {

  // Find the first non-digit number in the mangled name of the
  // builtin.
  // The search should start from "2" because first two characters
  // are "_Z" in the mangling scheme.
  size_t NumStartPos = 2;
  FuncNameStart = FuncName.find_first_not_of("0123456789", NumStartPos);
  // Extract the integer, which is equal to the number of chars
  // in the function name.
  StringRef SizeInChar = FuncName.slice(NumStartPos, FuncNameStart);
  return SizeInChar.getAsInteger(/*radix=*/10, FuncNameSize);
}

/// \brief Returns the declaration of the builtin function
///  with all the address space of the arguments are "0".
///  Name mangling is also modified accordingly to match the
///  defintion in the x86 builtins library.
static Function *getNewX86BuiltinFuncDecl(Function *OldFunc, DataLayout &DL) {

  size_t FuncNameStart, FuncNameSize;
  std::string MangledFuncName = OldFunc->getName();
  unsigned LocalAS  = 1;
  unsigned GlobalAS = 3;
  bool Failure = locateFuncName(OldFunc->getName(),FuncNameStart,FuncNameSize);
  assert(!Failure);

  std::string FuncName = MangledFuncName.substr(FuncNameStart,FuncNameSize);
  std::string NewFuncName =  MangledFuncName;
  if (FuncName.compare("async_work_group_strided_copy") == 0) {
     Function::arg_iterator AI = OldFunc->arg_begin();
     assert(AI != OldFunc->arg_end() && "Invalid number of arguments");
     PointerType *PtrType = dyn_cast<PointerType>(AI->getType());
     assert(PtrType && "Invalid argument");
     size_t AllocatedSize = DL.getTypeAllocSize(PtrType->getElementType());
     unsigned Arg1AddrSpace = getPtrAddrSpace(PtrType);
     AI++;
     assert(AI != OldFunc->arg_end() && "Invalid number of arguments");
     PtrType = dyn_cast<PointerType>(AI->getType());
     assert(PtrType && "Invalid argument");
     unsigned Arg2AddrSpace = getPtrAddrSpace(AI->getType());
     std::string AllocatedSizeStr = APInt(32,AllocatedSize).toString(10,false);
     if (Arg1AddrSpace == GlobalAS && Arg2AddrSpace == LocalAS)
       NewFuncName = "__" + FuncName + "_l2g_" + AllocatedSizeStr ;
     else if (Arg1AddrSpace == LocalAS && Arg2AddrSpace == GlobalAS)
       NewFuncName = "__" + FuncName + "_g2l_" + AllocatedSizeStr ;
  } else {
    size_t StartIndexPos = FuncNameStart + FuncNameSize;
    while (true) {
      // Find the Address space pointer arguments in the mangled name.
      StartIndexPos = NewFuncName.find("U3AS", StartIndexPos);
      if (StartIndexPos == std::string::npos)
        break;
      NewFuncName.erase(StartIndexPos, 5);
    }
  }

  // Create the arguments vector for new Function.
  SmallVector<llvm::Type *, 4> NewFuncArgs;
  for (Function::arg_iterator AI = OldFunc->arg_begin(), E = OldFunc->arg_end();
       AI != E; ++AI) {
    Type *ArgType = AI->getType();

    if (!isNonDefaultAddrSpacePtr(ArgType)) {
      NewFuncArgs.push_back(ArgType);
      continue;
    }

    PointerType *PtrType = cast<PointerType>(ArgType);
    Type *EleType = PtrType->getElementType();
    PointerType *NewPtrType = PointerType::get(EleType, (unsigned)0);
    NewFuncArgs.push_back(NewPtrType);
  }

  FunctionType *NewFuncType = FunctionType::get(
      OldFunc->getReturnType(), NewFuncArgs, OldFunc->isVarArg());
  Module *M = OldFunc->getParent();
  Value *NewFunc = M->getOrInsertFunction(NewFuncName, NewFuncType);
  if (Function *Fn = dyn_cast<Function>(NewFunc->stripPointerCasts())) {
    Fn->setCallingConv(OldFunc->getCallingConv());
    Fn->setLinkage(OldFunc->getLinkage());
    return Fn;
  }
  assert( 0 && "X86 builtin function could not be declared");
  return NULL;
}

/// \brief Define the x86 OpenCL builtin called by the user to call the
/// OpenCL builtin which has only default address space arguments.
void createX86BuiltinFuncDefn(Function *OldFunc, Function *NewFunc) {

  // Adding alwaysinline and No-unwind attribute for the function.
  OldFunc->addFnAttr(Attributes::AlwaysInline);
  OldFunc->addFnAttr(Attributes::NoUnwind);

  BasicBlock *EntryBlock =
      BasicBlock::Create(OldFunc->getContext(), "entry", OldFunc);
  IRBuilder<> BBBuilder(EntryBlock);
  SmallVector<llvm::Value *, 4> NewFuncCallArgs;

  for (Function::arg_iterator AI = OldFunc->arg_begin(), E = OldFunc->arg_end();
       AI != E; ++AI) {
    if (!isNonDefaultAddrSpacePtr(AI->getType())) {
      NewFuncCallArgs.push_back(AI);
      continue;
    }

    PointerType *PtrType = cast<PointerType>(AI->getType());
    Type *EleType = PtrType->getElementType();
    PointerType *NewPtrType = PointerType::get(EleType, (unsigned)0);

    // Bitcast all non-default addr space pointer arguments to default addr
    // space pointers. Note that this bitcast will result in no-op.
    Value *BitCastVal = BBBuilder.CreateBitCast(AI, NewPtrType, "bitcast");
    NewFuncCallArgs.push_back(BitCastVal);
  }

  Value *CallInstVal = BBBuilder.CreateCall(NewFunc, NewFuncCallArgs);
  
  if (CallInstVal->getType()->isVoidTy()) {
    BBBuilder.CreateRetVoid();
	return;
  }
  BBBuilder.CreateRet(CallInstVal);
}

/// \brief Generate right function calls for all "undefined" x86 OpenCL builtins
/// in the whole Module. Returns true if atleast one of the x86 OpenCL builtin
/// has been modified.
static bool findAndDefineBuitlinCalls(Module &M, DataLayout &DL) {
  bool isModified = false;
  for (Module::iterator FI = M.begin(), E = M.end(); FI != E; ++FI) {

    // Search only for used, undefined OpenCL builtin functions,
    // which has non-default addr space pointer arguments.
    if (!FI->empty() || FI->use_empty() || !isOpenCLBuiltinFunction(FI) ||
        !hasNonDefaultAddrSpaceArg(FI))
      continue;

    isModified = true;
    DEBUG(dbgs() << "\n Modifying Func " << FI->getName());
    // Get the new Function declaration.
    Function *NewFunc = getNewX86BuiltinFuncDecl(FI,DL);
    DEBUG(dbgs() << " to call " << NewFunc->getName() << " Function");
    createX86BuiltinFuncDefn(FI, NewFunc);
  }
  return isModified;
}

bool AMDX86Adapter::runOnModule(Module &M) {
  DEBUG(dbgs() << "\nAMD X86 Adapter Pass\n");
  return findAndDefineBuitlinCalls(M, getAnalysis<DataLayout>());
}
