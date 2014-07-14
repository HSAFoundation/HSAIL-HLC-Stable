//===-- HSAILNullPtrInsertion.cpp - Replace NULL with nullptr ins  -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// \file
// \brief This file implements a ModulePass to replace NULL
// with a nullptr_segment_uLength instruction as detailed
// in HSAIL Specification.
//===----------------------------------------------------------------------===//

#include "HSAIL.h"
#include "HSAILOpaqueTypes.h"
#include "HSAILUtilityFunctions.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/CodeGen/Passes.h"

using namespace llvm;
namespace {
class LLVM_LIBRARY_VISIBILITY HSAILNullPtrInsertion : public ModulePass {
public:
  static char ID;
  HSAILNullPtrInsertion() : ModulePass(ID) {};
  const char* getPassName() const;
  bool runOnModule(Module &M);
};
char HSAILNullPtrInsertion::ID = 0;
}

INITIALIZE_PASS(HSAILNullPtrInsertion, "hsail-nullptr-segment",
                "Insert nullptr instruction for null",
                false, false);


namespace llvm {
ModulePass *createHSAILNullPtrInsertionPass() {
  return new HSAILNullPtrInsertion();
}
}

const char* HSAILNullPtrInsertion::getPassName() const {
  return "HSAIL Nullptr generation Pass";
}

/// \brief Select the proper intrinsic based on the address space.
static StringRef selectNullPtrIntName(unsigned OutAS) {
  switch (OutAS) {
  case HSAILAS::FLAT_ADDRESS:
    return "__hsail_nullptr_flat";
  case HSAILAS::GLOBAL_ADDRESS:
    return "__hsail_nullptr_global";
  case HSAILAS::GROUP_ADDRESS:
    return "__hsail_nullptr_group";
  case HSAILAS::PRIVATE_ADDRESS:
    return "__hsail_nullptr_private";
  default:
    llvm_unreachable("Invalid address space");
  }
}

/// \brief Get a new pointer type of <i8, SrcTy->getAddressSpace()>. This is
/// used to help generate Nullptr intrinsics.
static Type *getAnyPtrType (Module *M,Type *SrcTy) {
  PointerType *SrcPtrTy = cast<PointerType>(SrcTy);
  return PointerType::get(Type::getInt8Ty(M->getContext()),
                          SrcPtrTy->getAddressSpace());
}
/// \brief Replace 'null' with nullptr_segment_uLength instruction.
static void replaceNullWithIntrinsic(Module *M, Instruction *I,
                                                     uint32_t operandNum) {
  BitCastInst *bitcastIns = NULL;

  // construct an intrinsic call depending on the NULL type
  ConstantPointerNull *Cpn = cast<ConstantPointerNull>(I->getOperand(operandNum));
  PointerType *InPtrTy = cast<PointerType>(Cpn->getType());
  // The LLVM null in case of opaque types is actually the integer zero and it
  // will be translated correctly by the backend. We should not replace it
  // with the nullptr instruction
  OpaqueType OT = GetOpaqueType(InPtrTy);
  // If the Opque type is not related to image types, then we assert
  assert(OT != UnknownOpaque);
  if (OT != NotOpaque) return;

  // If the NULL is not opaque type related to images, insert
  // a nullptr instruction
  Type *InValueAnyPtrTy = getAnyPtrType(M, Cpn->getType());
  StringRef nullptrIntName = selectNullPtrIntName(InPtrTy->getAddressSpace());

  // Add the nullptr_segment intrinsic call of prototype
  // i8 addr_space(X) *__hsail_nullptr_segment(void) before NULL instruction.
  Constant *CST1 =
      M->getOrInsertFunction(nullptrIntName, InValueAnyPtrTy, (Type *)0);
  Function *nullptrpFunc = cast<Function>(CST1);
  nullptrpFunc->addFnAttr(Attributes::ReadNone);

  if ( I->getOpcode() == Instruction::PHI ) {
    // For a phi instruction with NULL operand, insert the intrinsic call
    // in the corresponding predecessor Block.
    PHINode *Phin = cast<PHINode>(I);
    BasicBlock *Bbp = Phin->getIncomingBlock(operandNum);
    // Insert the intrinsic call in this block that
    // is suitable for inserting a non-PHI instruction
    Instruction *Terins = Bbp->getFirstInsertionPt();
    CallInst *CI1 = CallInst::Create(nullptrpFunc, "", Terins);
    bitcastIns = new BitCastInst(CI1,
                     Cpn->getType(), "", Terins);
  }
  else {
    CallInst *CI1 = CallInst::Create(nullptrpFunc, "", I);
    // Cast the return type to NULL return type
    bitcastIns = new BitCastInst(CI1,Cpn->getType(), "", I);
  }
  // Replace the NULL with new intrinsic call
  I->setOperand(operandNum, bitcastIns);
}

bool HSAILNullPtrInsertion::runOnModule(Module &M) {
  bool Changed = false;
  for (Module::iterator MF = M.begin(), E = M.end(); MF != E; ++MF) {
    if (MF->isDeclaration()) continue;
    for (Function::iterator B = MF->begin(), BE = MF->end(); B != BE; ++B) {
      for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I) {
        for(uint32_t i=0; i < I->getNumOperands();i++ ) {
          if (isa<ConstantPointerNull>(I->getOperand(i)))
            // For sure we have a NULL operand in an instruction.
            // Generate an intrinsic call __hsail_nullptr_Segment and
            // Replace NULL instruction with the call
            replaceNullWithIntrinsic(&M, I, i);
            Changed = true;
        }
      }
    }
  }
  return Changed;
}