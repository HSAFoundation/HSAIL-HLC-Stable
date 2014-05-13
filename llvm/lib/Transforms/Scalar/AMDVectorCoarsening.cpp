#if 1 || defined(AMD_OPENCL)
//===- AMDVectorCoarsening.cpp - Coarsen vector operations ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// Currently on GPU the codegen of the opencl compiler only allow operations
// on vectors of up to four elements. So an operation on a bigger vector
// are split into multiple operations.
// For example, a load instruction: load <16 x i8> %addr
// will be split into four load <4 x i8> instructions.
//
// This transformation is to coarsen vectors so that the final operations are
// as few as possible.
// For example, the load instruction: %result = load <16 x i8> %addr
// will be transformed into:
//   %newaddr = bitcast <16 x i8>* %addr to <4 x i32>*
//   %data = load <4 x i32>* %newaddr
//   %result = bitcast <4 x i32>* %data to <16 x i8>
// so only one load will be generated in the end.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "vector-coarsen"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

namespace {

class VectorCoarsening : public FunctionPass {
public:
  static char ID;
  VectorCoarsening(unsigned MinVecLen=0) : FunctionPass(ID),
    MinCoarseningVecLen(MinVecLen) {
    initializeVectorCoarseningPass(*PassRegistry::getPassRegistry());
  }
  virtual ~VectorCoarsening() {}

  virtual bool runOnFunction(Function& F);
  virtual void getAnalysisUsage(AnalysisUsage& AU) const;

private:
  inline bool mayCoarsenForInst(const Instruction& inst) const;
  std::pair<unsigned,unsigned> shouldCoarsenTo(const Type& type);
  Instruction& coarsenValue(Value& v,
                            std::pair<unsigned,unsigned> bitWidthNumElts,
                            LLVMContext& context,
                            Instruction& insertBefore,
                            bool isPointerType=false,
                            unsigned addressSpace=0);
  Instruction* tryCoarsenForInst(Instruction& inst);
  
private:
  const unsigned MinCoarseningVecLen;
};

}

#define MACHINE_WORD_SIZE_IN_BITS 32

char VectorCoarsening::ID = 0;
INITIALIZE_PASS_BEGIN(VectorCoarsening, "vector-coarsen",
                      "Vector Coarsening", false, false)
INITIALIZE_PASS_END(VectorCoarsening, "vector-coarsen",
                      "Vector Coarsening", false, false)

FunctionPass *llvm::createVectorCoarseningPass(unsigned MinVecLen/*=0*/) {
  return new VectorCoarsening(MinVecLen);
}

void VectorCoarsening::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.setPreservesAll();
}

// Is it safe to do vector coarsening for the given instruction?
inline bool VectorCoarsening::mayCoarsenForInst(const Instruction& inst) const
{
  switch (inst.getOpcode()) {
    case Instruction::Load:
    case Instruction::Store:
    case Instruction::And:
    case Instruction::Or:
    case Instruction::Xor:
      return true;
    default:
      return false;
  }
  return false;
}

// If the type is a vector of iN where N is a power of 2, and the vector's
// elements can be combined to wider elements, return the new bitwidth of
// vector elements and new # elements.
// If the vector's elements cannot be combined, return (0,0)
std::pair<unsigned,unsigned> VectorCoarsening::shouldCoarsenTo(const Type& type)
{
  const VectorType* vectorType = dyn_cast<VectorType>(&type);
  if (!vectorType) return std::pair<unsigned,unsigned>(0, 0);

  const Type* eltType = vectorType->getElementType();
  if (!eltType->isIntegerTy()) return std::pair<unsigned,unsigned>(0, 0);

  const IntegerType* eltIntType = cast<IntegerType>(eltType);
  unsigned eltBitWidth = eltIntType->getBitWidth();
  if (!eltIntType->isPowerOf2ByteWidth())
    return std::pair<unsigned,unsigned>(0, 0);

  unsigned numElts = vectorType->getNumElements();
  unsigned vecBitWidth = numElts * eltBitWidth;

  // HSA sets up MinCoarseningVecLen to 4. This is done to 
  // avoid generating unnecessary unpacks for loads and stores.
  if (numElts <= MinCoarseningVecLen)
    return std::pair<unsigned,unsigned>(0, 0);

  unsigned newBitWidth = MACHINE_WORD_SIZE_IN_BITS;
  unsigned newNumElts = 1;
  while (newBitWidth > eltBitWidth) {
    // vector bit width must be a multiple of new bit width
    if ((vecBitWidth > newBitWidth) && (vecBitWidth % newBitWidth) != 0)
      return std::pair<unsigned,unsigned>(0, 0);
    newNumElts = vecBitWidth / newBitWidth;
    if (newNumElts) break;
    newBitWidth >>= 1;
  }

  if (eltBitWidth >= newBitWidth) return std::pair<unsigned,unsigned>(0, 0);

  return std::pair<unsigned,unsigned>(newBitWidth, newNumElts);
}

// Create a BitCastInst to cast the value into a vector (or pointer to vector)
// with the new element bitwidth and # elements.
Instruction& VectorCoarsening::coarsenValue(
  Value& v, std::pair<unsigned,unsigned> bitWidthNumElts,
  LLVMContext& context, Instruction& insertBefore,
  bool isPointerType, unsigned addressSpace)
{
  Type *newEltType = IntegerType::get(context, bitWidthNumElts.first);
  Type* newType = NULL;
  if (bitWidthNumElts.second == 1)
    newType = newEltType;
  else
    newType = VectorType::get(newEltType, bitWidthNumElts.second);
  if (isPointerType)
    newType = PointerType::get(newType, addressSpace);
  BitCastInst* newInst = new BitCastInst(&v, newType, "castvec", &insertBefore);
  return *newInst;
}

// If the given instruction's operands can be coarsened, coarsen them,
// and return a new instruction to replace the original instruction.
// Otherwise, return NULL.
Instruction* VectorCoarsening::tryCoarsenForInst(Instruction& inst)
{
  assert(inst.getNumOperands() <= 2
         && "unexpected instruction type to be coarsened");

  Instruction* newInst = NULL;

  LoadInst* load = dyn_cast<LoadInst>(&inst);
  StoreInst* store = dyn_cast<StoreInst>(&inst);
  if (load || store) {
    Value* ptrOper
      = load ? load->getPointerOperand() : store->getPointerOperand();
    const PointerType* ptrType = dyn_cast<PointerType>(ptrOper->getType());
    const Type *ptrBaseType = ptrType->getElementType();
    std::pair<unsigned,unsigned> pair = shouldCoarsenTo(*ptrBaseType);
    if (pair.first == 0) return NULL; // shouldCoarsenTo returns false
    if (load) {
      // coarsen load's operand first
      Value& newOper
        = coarsenValue(*ptrOper, pair, ptrBaseType->getContext(), *load,
                        true/*isPointerType*/, ptrType->getAddressSpace());
      // create a new load using coarsened operand
      newInst = new LoadInst(&newOper, load->getName(), load->isVolatile(),
                             load->getAlignment(), load);
    }
    else {
      Value* valueOper = store->getValueOperand();
      const Type* valueType = valueOper->getType();
      assert(valueType == ptrBaseType && "store's oper types inconsistent");
      // coarsen store's operands first
      Value& newValueOper
        = coarsenValue(*valueOper, pair, valueType->getContext(), *store);
      Value& newPtrOper
        = coarsenValue(*ptrOper, pair, ptrBaseType->getContext(), *store,
                        true/*isPointerType*/, ptrType->getAddressSpace());
      // create a new store using coarsened operands
      newInst = new StoreInst(&newValueOper, &newPtrOper, store->isVolatile(),
                              store->getAlignment(), store);
    }
  }
  else {
    Value* oper0 = inst.getOperand(0);
    Value* oper1 = inst.getOperand(1);
    const Type* type0 = oper0->getType();
    const Type* type1 = oper1->getType();
    assert(type0 == type1 && "oper types inconsistent");
    std::pair<unsigned,unsigned> pair = shouldCoarsenTo(*type0);
    if (pair.first == 0) return NULL; // shouldCoarsenTo returns false
    // coarsen inst's operands first
    Value& newOper0 = coarsenValue(*oper0, pair, type0->getContext(), inst);
    Value& newOper1 = coarsenValue(*oper1, pair, type1->getContext(), inst);

    // create a new inst using coarsened operands
    BinaryOperator* binOp = dyn_cast<BinaryOperator>(&inst);
    assert(binOp && "unexpected instruction type");
    newInst = BinaryOperator::Create(binOp->getOpcode(), &newOper0, &newOper1,
                                     binOp->getName(), binOp);
  }

  DebugLoc DbgLoc = inst.getDebugLoc();
  if (newInst && !DbgLoc.isUnknown()) {
    newInst->setDebugLoc(DbgLoc);
  }

  Instruction* result = newInst;
  if (newInst->getType() != inst.getType()) {
    result = new BitCastInst(newInst, inst.getType(), "", &inst);
  }

  return result;
}

bool VectorCoarsening::runOnFunction(Function& F) {
  for (Function::iterator I = F.begin(), E = F.end(); I != E; ++I) {
    BasicBlock& BB = *I;
    for (BasicBlock::iterator I1 = BB.begin(), E1 = BB.end(); I1 != E1;) {
      Instruction& inst = *I1++;
      if (!mayCoarsenForInst(inst)) continue;
      Instruction* newInst = tryCoarsenForInst(inst);
      if (!newInst) continue;
      inst.replaceAllUsesWith(newInst);
      inst.eraseFromParent();
    }
  }
  return false;
}

#endif // 1 || defined(AMD_OPENCL)
