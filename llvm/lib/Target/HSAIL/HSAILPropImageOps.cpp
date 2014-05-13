//===-- llvm/CodeGen/ExpandISelPseudos.cpp ----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Propagate argument position of image object args (image_t and sampler_t)
// to uses in image function call instructions. Re-write image operands in 
// to hold encoded argument position. 
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "hsail-propagate-image-operands"

#include "HSAIL.h"
#include "HSAILOpaqueTypes.h"

#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <map>

using namespace llvm;

namespace {
  class HSAILPropagateImageOperands : public FunctionPass {
  public:
    static char ID; // Pass identification, replacement for typeid

    explicit HSAILPropagateImageOperands() : FunctionPass(ID) {}

  private:
    std::map<std::string, unsigned> ImageArgMap;
    SmallVector<CallSite, 16> CallSites;

    virtual bool runOnFunction(Function &F);

    virtual const char *getPassName() const { 
      return "HSAIL Propagate Image Operands";
    }

    bool findImageArgs(Function &F);
    bool findImageCallSites(Function &F);
    bool replaceImageOperands();
    void dump();
  };

  char HSAILPropagateImageOperands::ID = 0;
} 
  // Register pass in passRegistry so that the pass info gets populated for printing debug info 
  INITIALIZE_PASS(HSAILPropagateImageOperands, "hsail-propagate-image-operands",
                "HSAIL Propagate Image Operands", false, false)

FunctionPass *llvm::createHSAILPropagateImageOperandsPass() {
  return new HSAILPropagateImageOperands();
}

bool HSAILPropagateImageOperands::runOnFunction(Function &F) {
  bool Changed = false;

  //errs() << "HSAIL Propagate Image Operands for kernel: ";
  //errs().write_escaped(F.getName()) << "\n";

  Changed = findImageArgs(F);
  //errs() << "After findImageArgs\n";
  //dump();

  if (Changed) {
    Changed = findImageCallSites(F);
  }
  //errs() << "After findImageCallSites\n";
  //dump();

  if (Changed) {
    Changed = replaceImageOperands();
  }

  ImageArgMap.clear();
  CallSites.clear();

  return Changed;
}

// Scan function args for image_t and sampler_t arguments. 
// Build maps of arg name -> image arg index.
bool HSAILPropagateImageOperands::findImageArgs(Function &F) {
  unsigned imageArgIndex = IMAGE_ARG_BIAS;

  for (Function::arg_iterator I = F.arg_begin(), E = F.arg_end(); 
       I != E; ++I) {
    const Type *T = I->getType();
    OpaqueType OT = GetOpaqueType(T);

    if (!IsImage(OT) && OT != Sampler)
      continue;
    
    // Map arg name to an image arg index. The image arg index is a
    // constant we use retrieve hsail arg symbols from a table of
    // "image handles" that is created later in
    // HSAILISelLowering::LowerFormalArguments(). The index value is
    // biased by IMAGE_ARG_BIAS.
    ImageArgMap[I->getName()] = imageArgIndex++;
  }

  return !ImageArgMap.empty();
}

// Collect instructions that call image functions.
bool HSAILPropagateImageOperands::findImageCallSites(Function &F) {
  for (Function::iterator I = F.begin(), E = F.end(); I != E; ++I) {
    BasicBlock &BB = *I;
    for (BasicBlock::iterator BBI = BB.begin(), BBE = BB.end(); 
      BBI != BBE; ++BBI) {
      CallSite CS(cast<Value>(&*BBI));
      // If this isn't a call, or if it's a call to an intrinsic, skip the instr.
      if (!CS || isa<IntrinsicInst>(&*BBI)) {
        continue;
      }

      const Function *callee = CS.getCalledFunction();
      // Make sure callee is a function without a body.
      if (!callee->isDeclaration()) {
        continue;
      }

      // Is callee an hsail image function?
      StringRef calleeName = callee->getName();
      if (!(calleeName.startswith("__hsail_rdimage") ||
        calleeName.startswith("__hsail_ldimage")  ||
        calleeName.startswith("__hsail_stimage") ||
        calleeName.startswith("__hsail_query"))) {
          continue;
      }

      CallSites.push_back(CS);
    }
  }

  return !CallSites.empty();
}

#define E2L_ADDR_SUFFIX ".addr"
// Replace image operands in call instructions with int constant that represents
// respective kernel arg positions. We'll use these arg-position constants in isel
// to correctly lower image_t and sampler_t operands in image call instructions.
bool HSAILPropagateImageOperands::replaceImageOperands() {
  assert(!ImageArgMap.empty() && "Expected image or sampler args");
  assert(!CallSites.empty() && "Expected call sites for image functions");

  for (unsigned i = 0; i < CallSites.size(); i++) {
    CallSite CS = CallSites[i];
    unsigned argNum = 0;

    // Scan for image operands 
    for (CallSite::arg_iterator I = CS.arg_begin(), E = CS.arg_end(); 
      I != E; ++I, ++argNum) {
      // Lookup arg name in arg map
      std::string argName = (*I)->getName();
      std::map<std::string, unsigned>::iterator it = ImageArgMap.find(argName);
      Value* intToPtr = 0;
      if (it == ImageArgMap.end()) {
        if (!isa<LoadInst>(I)) continue;
        LoadInst *pLoad = cast<LoadInst>(I);
        if (!isa<AllocaInst>(pLoad->getPointerOperand())) continue;
        AllocaInst *pAlloca = cast<AllocaInst>(pLoad->getPointerOperand());
        OpaqueType OT = GetOpaqueType(pAlloca->getAllocatedType());

        if (Sampler == OT) { // process samplers
          for (Value::use_iterator U = pAlloca->use_begin(),
              B = pAlloca->use_end(); U != B; ++U) {
            if (isa<StoreInst>(*U)){  // find actual value of sampler
              Value *pStoredVal = cast<StoreInst>(*U)->getValueOperand();
              // null pointer or int to ptr generated by clc
              if (isa<ConstantPointerNull>(pStoredVal) ||
                (isa<ConstantExpr>(pStoredVal) &&
                 Instruction::IntToPtr == cast<ConstantExpr>(pStoredVal)->getOpcode())) {
                  intToPtr = pStoredVal;
                  continue;
              }
            }
          }
        } else if (IsImage( OT )) { // process images
          StringRef PointerName = pAlloca->getName();
          if (!PointerName.endswith( E2L_ADDR_SUFFIX )) continue;
          it = ImageArgMap.find( PointerName.drop_back( strlen( E2L_ADDR_SUFFIX )));
          if (it == ImageArgMap.end()) continue;
        } else { // otherwise - skip
            continue;
        }
      }

      if (!intToPtr) {
        // Create an inttoptr expression that based on args's image arg index.
        APInt* indexVal = new APInt(32,  it->second);
        Constant* indexConst = 
          ConstantInt::get((*I)->getType()->getContext(), *indexVal);
        intToPtr = ConstantExpr::getCast(Instruction::IntToPtr,
          indexConst, (*I)->getType());
      }
      // Replace image operand with inttoptr expression.
      CS.setArgument(argNum, intToPtr);
    }
  }
  return true;
}

void HSAILPropagateImageOperands::dump() {
  errs() << "image arg map: ";
  if (ImageArgMap.empty()) {
    errs() << " <empty>";
  } else {
    for (std::map<std::string, unsigned>::iterator it = ImageArgMap.begin();
      it != ImageArgMap.end(); it++) {
        errs() << " <" << it->first << ", " << it->second << ">";
    }
  }
  errs() << "\n";
  if (!CallSites.empty()) {
    for (unsigned i = 0; i < CallSites.size(); i++) {
      errs() << "image callee: ";
      errs().write_escaped(CallSites[i].getCalledFunction()->getName()) << "\n";
    }
  }
}
