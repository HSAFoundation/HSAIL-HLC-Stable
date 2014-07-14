//===- AMDLowerToPreciseBuiltins.cpp - Lowers to Precise math Library Calls ------------===//
//
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief Translate CLANG frontend generated math function calls to
/// precise library function calls.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "AMDLowerToPreciseBuiltins"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Instructions.h"
#include "llvm/IRBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/AMDArgumentUtils.h"
#include "llvm/Value.h"
#include "../../lib/Transforms/SPIR/cxa_demangle.h"
#include <sstream>
#include <string>
using namespace llvm;

namespace {
  class AMDLowerToPreciseBuiltins : public ModulePass {

  public:
    static char ID; // Pass identification, replacement for typeid

    AMDLowerToPreciseBuiltins() : ModulePass(ID) {
    }

    virtual bool runOnModule(Module &M);
  private:

    llvm::Function *getFunctionPreciseFSqrt(llvm::Type*, llvm::Module*);
    llvm::Function *getFunctionPreciseFDiv(llvm::Type*, llvm::Module*);
  };
};

INITIALIZE_PASS(AMDLowerToPreciseBuiltins, "fp32-correctly-rounded-divide-sqrt",
                "Lower OpenCL math functions to precise library functions",
                false, false);

char AMDLowerToPreciseBuiltins::ID = 0;

// this pass traverses all call statments and checks if they are calls
// to "sqrt" builtin function. Such calls are changed to
// "__precise_fp32_sqrt_" function calls.

bool AMDLowerToPreciseBuiltins::runOnModule(Module& M) {

  bool Changed = false;
  for (Module::iterator MF = M.begin(), E = M.end(); MF != E; ++MF) {
    for (Function::iterator BB=MF->begin(), MFE=MF->end(); BB != MFE; ++BB) {
      for (BasicBlock::iterator instr = BB->begin(), instr_end = BB->end();
          instr != instr_end; ) {

       SmallVector<llvm::Value*,16> FuncArgs;
       Value *PreciseLibFunc = NULL;
       Instruction* I = cast<Instruction>(instr++);
       Instruction *NonPreciseInst = I;
       IRBuilder<> B(I);
       if (I->getOpcode() == Instruction::FDiv) {
          llvm::Type *llvmtype = I->getOperand(0)->getType();
          if (llvmtype->isFloatTy() ||
            (llvm::isa<llvm::VectorType>(llvmtype) &&
            (llvm::cast<llvm::VectorType>(llvmtype))
                               ->getElementType()->isFloatTy())) {
            FuncArgs.push_back(I->getOperand(0));
            FuncArgs.push_back(I->getOperand(1));
            PreciseLibFunc = getFunctionPreciseFDiv(llvmtype, &M);
            llvm::CallInst *PreciseLibCall = B.CreateCall(PreciseLibFunc,FuncArgs);
            PreciseLibCall->setCallingConv(CallingConv::SPIR_FUNC);
            NonPreciseInst->replaceAllUsesWith(PreciseLibCall);
            NonPreciseInst->eraseFromParent();
            Changed = true;
	    continue;
	  }
       }
        // Check for sqrt call
       CallInst *CI = dyn_cast<CallInst>(I);
       if(!(CI && CI->getCalledFunction()
             && CI->getCalledFunction()->hasName())) {
           continue;
       }
       llvm::Function *CF = CI->getCalledFunction();
       StringRef MangledName = CF->getName();
       int status = 0;

       const char* DemangledName =
            __cxxabiv1::__cxa_demangle(MangledName.data(), 0, 0, &status);

       if (status || !DemangledName) continue;
       StringRef DemangledString(DemangledName);

       size_t FunctionNameEnd = DemangledString.find("(");
       if (FunctionNameEnd == std::string::npos) continue;
       std::string FunctionName = DemangledString.substr(0,FunctionNameEnd);
       llvm::Type *llvmtype = CI->getArgOperand(0)->getType();

       if ((FunctionName.compare("sqrt")==0) &&
           (llvmtype->isFloatTy() ||
            (llvm::isa<llvm::VectorType>(llvmtype) &&
            (llvm::cast<llvm::VectorType>(llvmtype))
                               ->getElementType()->isFloatTy()))) {
          PreciseLibFunc = getFunctionPreciseFSqrt(llvmtype, &M);
          FuncArgs.push_back(CI->getArgOperand(0));
          llvm::CallInst *PreciseLibCall = B.CreateCall(PreciseLibFunc,FuncArgs);
          PreciseLibCall->setCallingConv(CallingConv::SPIR_FUNC);
          NonPreciseInst->replaceAllUsesWith(PreciseLibCall);
          NonPreciseInst->eraseFromParent();
          Changed = true;
       }
      }
    }
  }

  return Changed;
}

// Function to crete the precise function lowering pass
ModulePass *llvm::createAMDLowerToPreciseBuiltinsPass() {
  return new AMDLowerToPreciseBuiltins();
}

// function returns the definition to  "__precise_fp32_sqrt_" llvm function.
llvm::Function*
AMDLowerToPreciseBuiltins::getFunctionPreciseFSqrt(llvm::Type* llvmtype,
                                                  llvm::Module* llvmModule)
{
  std::string funcName("__precise_fp32_sqrt_");

  if (llvm::isa<llvm::VectorType>(llvmtype)) {
    std::stringstream out;
    out << (llvm::cast<llvm::VectorType>(llvmtype))->getNumElements();
    funcName += out.str();
  }

  funcName += "f32";

  llvm::Function* llvmFunc = llvmModule->getFunction(funcName);
  if (llvmFunc) return llvmFunc;

  std::vector<llvm::Type*> argTys;

  argTys.push_back(llvmtype); //src1

  llvm::FunctionType* funcType = llvm::FunctionType::get(llvmtype,argTys,false);
  llvmFunc = llvm::Function::Create(funcType,
                               llvm::Function::ExternalLinkage,
                               funcName, llvmModule);
  llvmFunc->setCallingConv(CallingConv::SPIR_FUNC);

  return llvmFunc;
}

// function returns the definition to  "__precise_fp32_div_" llvm function.
llvm::Function*
AMDLowerToPreciseBuiltins::getFunctionPreciseFDiv(llvm::Type* llvmtype,
                                                  llvm::Module* llvmModule)
{
  std::string funcName("__precise_fp32_div_");

  if (llvm::isa<llvm::VectorType>(llvmtype)) {
    std::stringstream out;
    out << (llvm::cast<llvm::VectorType>(llvmtype))->getNumElements();
    funcName += out.str();
  }

  funcName += "f32";

  llvm::Function* llvmFunc = llvmModule->getFunction(funcName);
  if (llvmFunc) return llvmFunc;

  std::vector<llvm::Type*> argTys;

  argTys.push_back(llvmtype); //src1
  argTys.push_back(llvmtype); //src2

  llvm::FunctionType* funcType = llvm::FunctionType::get(llvmtype,argTys,false);
  llvmFunc = llvm::Function::Create(funcType,
                               llvm::Function::ExternalLinkage,
                               funcName, llvmModule);
  llvmFunc->setCallingConv(CallingConv::SPIR_FUNC);

  return llvmFunc;
}
