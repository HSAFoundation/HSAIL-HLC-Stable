//===- Transforms/Utils/AMDArgumentUtils.h - Argument Utils -----*- C++ -*-===//
//
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
// This family of transformations manipulate LLVM functions' arguments list.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_AMDARGUMENTUTILS_H
#define LLVM_TRANSFORMS_UTILS_AMDARGUMENTUTILS_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"

namespace llvm {

  class Type;
  class FunctionType;
  class Value;
  class CallSite;
  class Function;
  class GlobalAlias;
  class Instruction;
  class LoadInst;
  class DataLayout;

  struct NewArgInfo {
    Type *Ty;
    const char *Name;
  };

  /// CreateFunctionWithNewArguments - Create a new function, cloned from the
  /// old one, while the new arguments are added to the beginning of the
  /// arguments list.
  ///
  Function *CreateFunctionWithNewArguments(Function *F,
                                           ArrayRef<NewArgInfo> NewArgs,
                                           bool TakeBlocks);

  /// TransferFunctionUses - Transfer all uses of the old function to the
  /// new one, and adding the new arguments in new function.
  ///
  /// The old function's argument list must be located in the end of the
  /// new function's argument list.
  ///
  void TransferFunctionUses(Function* FOld, Function *FNew);

  void TransferAliasUses(GlobalAlias* A, GlobalAlias *NA);

  /// GetFunctionTypeWithNewArguments - Get a function type that represent a
  /// function with the new arguments list added to the beginning.
  ///
  FunctionType *GetFunctionTypeWithNewArguments(FunctionType *FTy,
                                                ArrayRef<NewArgInfo> NewArgs);
  FunctionType *GetFunctionTypeWithNewArguments(FunctionType *FTy,
                                                ArrayRef<Type*> NewTypes);

  /// AddArgumentsToCallSite - Add the arguments list to the beginning of the
  /// call-site. On return, the full list of argument will be stored in Args.
  ///
  void AddArgumentsToCallSite(CallSite &CS, SmallVectorImpl<Value*> &Args,
                              Value *Callee);

  /// AddArgumentsToFunction - Add the arguments list to the beginning of the
  /// function. This creates a new function and deletes the old one.
  ///
  void AddArgumentsToFunction(Function* &F, ArrayRef<NewArgInfo> NewArgs);


  void EmitLoadAggregateArguments(Value *AggregateArg,
                                  ArrayRef<NewArgInfo> NewArgs,
                                  MutableArrayRef<LoadInst*> EmittedLoads,
                                  unsigned &Offset, DataLayout &DL,
                                  Instruction &InsertBefore);

} // End llvm namespace

#endif
