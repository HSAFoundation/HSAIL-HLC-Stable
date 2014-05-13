#if 1 || defined (AMD_OPENCL_MEMCOMBINE)
//===- AMDAlignmentAnalysis.cpp - alignment analysis ---------------===//
//===----------------------------------------------------------------------===//
//
// This file will implement a version of alignment analysis that relies on
// function argument being aligned by a default value.
// 
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "AMDAlignmentAnalysis"

#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/Analysis/AMDAlignmentAnalysis.h"
#include "llvm/Analysis/AMDOpenCLSymbols.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/GetElementPtrTypeIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/DataLayout.h"
#include <set>
#include <sstream>
#include <stack>

using namespace llvm;

static cl::opt<unsigned>
KernelArgAlignment(
  "kernel-arg-alignment", cl::init(1),
  cl::value_desc("bytes"),
  cl::desc("Opencl kernel function arguments' minimal alignments"));

// Register the AlignmentAnalysis interface, providing a nice name to refer to.
INITIALIZE_ANALYSIS_GROUP(AlignmentAnalysis, "Alignment Analysis",
                          AMDAlignmentAnalysis)

char AlignmentAnalysis::ID = 0;

namespace {
  // Values used in alignment analysis to represent the alignment of a value
  // Try to compact the fields of this class so objects of this class
  // can be passed around by value.
  //
  // The lattice that this class represents has three levels:
  // Bottom:
  //   Cannot be 100% sure statically about the alignment
  // Aligned:
  //   Known to be multiple of some alignment
  // Top:
  //   Variable's alignment is not known yet.
  class AlignmentValue {
    private:
      // use alignment value 0 to represent Top of the lattice (unknown)
      // use alignment value 1 to represent Bottom of the lattice (unaligned)
      enum { Top = 0U, Bottom = 1U };

    public:
      static AlignmentValue makeUnaligned() { return AlignmentValue(Bottom); }
      static AlignmentValue makeAligned(uint64_t align) {
        return AlignmentValue(align);
      }
      static AlignmentValue makeUnknown() { return AlignmentValue(Top); }

    public:
      bool isUnaligned() const { return alignment == Bottom; }
      bool isUnknown() const { return alignment == Top; }
      bool isAligned() const { return !isUnaligned() && !isUnknown(); }

      uint64_t getAlignment() {
        return alignment;
      }

      bool equals(const AlignmentValue& v2) const {
        return alignment == v2.alignment;
      }
      std::string toString(void) const;

    private:
      AlignmentValue(uint64_t align)
        : alignment(align)
      {}

    private:
      uint64_t alignment;
  };

#if 0
  struct AlignmentMapTyConfig : public ValueMapConfig<const Value *> {
    typedef void *ExtraData;
    static void onDelete(ExtraData, const Value*) { 
      assert(false && "AlignmentMapTy doesn't handle Delete yet");
    }
    static void onRAUW(ExtraData, const Value*, const Value*) {
      assert(false && "AlignmentMapTy doesn't handle RAUW yet");
    }
  };
#endif

  // Map a Value to its AlignmentValue.
  // Used to cache the AlignmentValue for each Value
  class AlignmentMapTy {
      typedef ValueMap<const Value*, AlignmentValue,
                       ValueMapConfig<const Value*> > ValueMapTy;

    private:
      ValueMapTy Map;

    public:
      AlignmentMapTy(void) : Map() {}
      ~AlignmentMapTy() {
        clear();
      }
      void clear() {
        Map.clear();
      }
      AlignmentValue lookup(const Value& V) {
        ValueMapTy::iterator it = Map.find(&V);
        if (it == Map.end())
          return AlignmentValue::makeUnknown();
        return it->second;
      }

      void updateValue(Value *key, AlignmentValue newav) {
        ValueMapTy::iterator it = Map.find(key);
        if (it == Map.end())
          Map.insert(std::make_pair(key, newav));
        else
          it->second = newav;
      }

      void dump();
  };

  class AMDAlignmentAnalysis : public FunctionPass, public AlignmentAnalysis {
    public:
      static char ID;

    private:
      AlignmentMapTy alignmentMap;
      // Alignment (#bytes) of OPENCL kernel arguments, passed in from runtime
      uint64_t KernelArgAlign;
      bool isKernel;
      bool isStub;

    public:
      AMDAlignmentAnalysis(uint64_t kernelArgAlign = KernelArgAlignment)
        : FunctionPass(ID), alignmentMap(), KernelArgAlign(kernelArgAlign),
          isKernel(false), isStub(false)
      {
        initializeAMDAlignmentAnalysisPass(*PassRegistry::getPassRegistry());
        if (KernelArgAlign == 0) KernelArgAlign = 1;
        assert(isPowerOf2_64(KernelArgAlign)
               && "KernelArgAlignment not power of 2");
      }
      virtual bool runOnFunction(Function& F);
      virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.addRequired<OpenCLSymbols>();
        AU.addPreserved<OpenCLSymbols>();
      }
      /// getAdjustedAnalysisPointer - This method is used when a pass
      /// implements
      /// an analysis interface through multiple inheritance.  If needed, it
      /// should override this to adjust the this pointer as needed for the
      /// specified pass info.
      virtual void *getAdjustedAnalysisPointer(const void *ID) {
        if (ID == &AlignmentAnalysis::ID)
          return (AlignmentAnalysis*)this;
        return this;
      }

      virtual uint64_t getAlignment(Value& addr);

    private:
      uint64_t getPointerAlignment(const Value &ptr);
      AlignmentValue findAlignmentValue(const Value& v) {
        return alignmentMap.lookup(v);
      }
      AlignmentValue createAlignmentValueForPhi(PHINode& phi);
      AlignmentValue createAlignmentValueForAdd(BinaryOperator& BO);
      AlignmentValue createAlignmentValueForShl(BinaryOperator& BO);
      AlignmentValue createAlignmentValueForMul(BinaryOperator& BO);
      AlignmentValue createAlignmentValueForOr(BinaryOperator& BO);
      AlignmentValue createAlignmentValueForGEPI(GetElementPtrInst& gepi);
      inline bool isHandledOp(Value& v);
      AlignmentValue createAlignmentValue(Value& v);
      AlignmentValue getAlignmentValue(Value& v);
      uint64_t getGEPIAlignment(GetElementPtrInst& gepi);
  };
}

static uint64_t getMaxAlignment() {
  return (uint64_t)1U << (sizeof(uint64_t) * CHAR_BIT -1);
}

// Calculate the alignment of the given value.
// If value is 0, return the maximum alignment.
static uint64_t calAlignment(unsigned i) {
  if (i == 0) return getMaxAlignment();
  uint64_t align = 1U << CountTrailingZeros_64(i);
  return align;
}

raw_ostream& operator<<(raw_ostream &O, AlignmentValue av) {
  return O << av.toString();
}

std::string AlignmentValue::toString(void) const {
  if (isUnaligned()) {
    return std::string("B");
  }
  else if (isUnknown()) {
    return std::string("T");
  }
  else {
    assert(isAligned() && "unexpected AlignmentValue type");
    std::stringstream out;
    out << " alignment " << alignment;
    return out.str();
  }
  return "";
}

void AlignmentMapTy::dump() {
  ValueMapTy::iterator it = Map.begin(), itEnd = Map.end();
  for (; it != itEnd; ++it) {
    errs() << it->first << ":" << it->second << "\n";
  }
}

//
// Return the alignment of given pointer in element size
//
uint64_t AMDAlignmentAnalysis::getPointerAlignment(const Value &ptr)
{
  // We can only assume kernels having aligned pointer-typed parameters.
  if (!isKernel && !isStub) return 0;

  const Type *tptr = ptr.getType();
  if (!tptr || !isa<PointerType>(tptr)) return 0;

  if (isa<GlobalValue>(ptr))
    return dyn_cast<GlobalValue>(&ptr)->getAlignment();

  if (!isa<Argument>(ptr)) {
    if (!isStub) return 0;

    // Try to recognize by pattern matching the case where an OCL kernel
    // was inlined into a stub function. In that case, the kernel function's
    // arguments are not Arguments anymore, instead, they are loaded from
    // the stub function's arguments.
    if (const LoadInst* load = dyn_cast<LoadInst>(&ptr)) {
      const Value* addr = load->getPointerOperand();
      const Value* src = GetUnderlyingObject(addr, DL);
      if (!isa<Argument>(src)) return 0;
    }
  }

  return KernelArgAlign;
}

// Meet(Unknown,Any) = Any
// Meet(Unaligned,Any) = Unaligned
// Meet(Aligned,Aligned) = Aligned:alignment=min(alignment1, alignment2)
static AlignmentValue meet(AlignmentValue& av1, AlignmentValue& av2)
{
  if (av1.equals(av2)) return av1;

  // Meet(Unaligned,Any) = Unaligned
  if (av1.isUnaligned() || av2.isUnaligned())
    return AlignmentValue::makeUnaligned();

  // Meet(Unknown,Any) = Any
  if (av2.isUnknown()) return av1;
  if (av1.isUnknown()) return av2;

  assert(av1.isAligned() && av2.isAligned()
         && "unexpected AlignmentValue type");

  uint64_t avAlign = av1.getAlignment();
  uint64_t av2Align = av2.getAlignment();

  uint64_t newAlign = std::min(avAlign, av2Align);
  if (newAlign > 1)
    return AlignmentValue::makeAligned(newAlign);
  else
    return AlignmentValue::makeUnaligned();
}

// AlignmentValue(phi) = meet(AlignmentValue(phi-args))
AlignmentValue AMDAlignmentAnalysis::createAlignmentValueForPhi(PHINode& phi)
{
  AlignmentValue av = AlignmentValue::makeUnknown();
  for (unsigned i = 0; i < phi.getNumIncomingValues(); ++i) {
    Value* oper = phi.getIncomingValue(i);
    AlignmentValue av2 = findAlignmentValue(*oper);
    av = meet(av, av2);
  }
  return av;
}

// Unaligned + Any = Unaligned
// Unknown + Any = Unknown
// Aligned + Aligned = Aligned : alignment=min(alignment1, alignment2)
AlignmentValue AMDAlignmentAnalysis::createAlignmentValueForAdd(
  BinaryOperator& BO)
{
  assert(BO.getOpcode() == Instruction::Add && "not Add inst");

  Value *op0 = BO.getOperand(0);
  Value *op1 = BO.getOperand(1);
  AlignmentValue avop0 = findAlignmentValue(*op0);
  AlignmentValue avop1 = findAlignmentValue(*op1);

  // Unaligned + Any = Unaligned
  if (avop0.isUnaligned() || avop1.isUnaligned())
    return AlignmentValue::makeUnaligned();

  // Unknown + Any = Unknown
  if (avop0.isUnknown() || avop1.isUnknown())
    return AlignmentValue::makeUnknown();

  assert(avop0.isAligned() && avop1.isAligned()
         && "unexpected AlignmentValue type");

  uint64_t avAlign = avop0.getAlignment();
  uint64_t av2Align = avop1.getAlignment();

  uint64_t newAlign = std::min(avAlign, av2Align);
  if (newAlign > 1)
    return AlignmentValue::makeAligned(newAlign);
  else
    return AlignmentValue::makeUnaligned();
}

// Any << constInt = Aligned : alignment = op1.alignment * 2^constInt
AlignmentValue AMDAlignmentAnalysis::createAlignmentValueForShl(
  BinaryOperator& BO)
{
  assert(BO.getOpcode() == Instruction::Shl && "not Shl inst");

  Value *op0 = BO.getOperand(0);
  Value *op1 = BO.getOperand(1);
  AlignmentValue avop0 = findAlignmentValue(*op0);

  // Absractly interprets the left shift expression.
  // For those already known to be Aligned, return with Aligned and
  // alignment = the variables' alignment * 2^(shift amnt)
  // For anything else that has > 1 constant shift amount,
  // return with a Aligned with alignment=2^(shift amnt)
  uint64_t alignment = 1;
  if (avop0.isAligned())
    alignment = avop0.getAlignment();

  if (ConstantInt* constOp1 = dyn_cast<ConstantInt>(op1)) {
    uint64_t val1 = constOp1->getLimitedValue();
    alignment = ((uint64_t)1U << val1) * alignment;
  }

  if (alignment > 1)
    return AlignmentValue::makeAligned(alignment);

  if (avop0.isUnknown())
    return AlignmentValue::makeUnknown();

  return AlignmentValue::makeUnaligned();
}

// alignment = op1.alignment * op2.alignment
AlignmentValue AMDAlignmentAnalysis::createAlignmentValueForMul(
  BinaryOperator& BO)
{
  assert(BO.getOpcode() == Instruction::Mul && "not Shl inst");

  Value *op0 = BO.getOperand(0);
  Value *op1 = BO.getOperand(1);
  AlignmentValue avop0 = findAlignmentValue(*op0);
  AlignmentValue avop1 = findAlignmentValue(*op1);

  int alignment = 1;
  if (avop0.isAligned()) {
    alignment = avop0.getAlignment();
  }

  if (avop1.isAligned()) {
    uint64_t op1Align = avop1.getAlignment();
    if (op1Align > 1) alignment *= op1Align;
  }

  if (alignment > 1)
    return AlignmentValue::makeAligned(alignment);

  if (avop0.isUnknown() || avop1.isUnknown())
    return AlignmentValue::makeUnknown();

  return AlignmentValue::makeUnaligned();
}

// Unaligned | Any = Unaligned
// Unknown | Any = Unknown
// Aligned | Aligned = Aligned : alignment=min(alignment1, alignment2)
//                     if Op1 & Op2 == 0
// Aligned | Aligned = Unaligned if Op1 & Op2 != 0
AlignmentValue AMDAlignmentAnalysis::createAlignmentValueForOr(
  BinaryOperator& BO)
{
  assert(BO.getOpcode() == Instruction::Or && "not Shl inst");

  Value *op0 = BO.getOperand(0);
  Value *op1 = BO.getOperand(1);
  AlignmentValue avop0 = findAlignmentValue(*op0);
  AlignmentValue avop1 = findAlignmentValue(*op1);

  if (avop0.isUnaligned() || avop1.isUnaligned())
    return AlignmentValue::makeUnaligned();

  if (avop0.isUnknown() || avop1.isUnknown())
    return AlignmentValue::makeUnknown();

  assert(avop0.isAligned() && avop1.isAligned() && "unexpected align type");
  if (ConstantInt* constOp1 = dyn_cast<ConstantInt>(op1)) {
    // X|C == X+C if all the bits in C are unset in X.  Otherwise we can't
    // analyze it.
    if (MaskedValueIsZero(op0, constOp1->getValue(), DL)) {
      uint64_t align = std::min(avop0.getAlignment(), avop1.getAlignment());
      if (align > 1)
        return AlignmentValue::makeAligned(align);
    }
  }

  return AlignmentValue::makeUnaligned();
}

// GEPI(Unaligned, Any) = Unaligned
// GEPI(Unknown, Any) = Unknown
// GEPI(Aligned, Aligned) = Aligned : alignment=min(base-alignment,
//                                                  index-alignment)
//                          if min(base-alignment, index-alignment) > 1
// GEPI(Aligned, Aligned) = Unaligned
//                          if min(base-alignment, index-alignment) < 2
AlignmentValue AMDAlignmentAnalysis::createAlignmentValueForGEPI(
  GetElementPtrInst& gepi)
{
  Value* ptr = gepi.getPointerOperand();
  AlignmentValue av = findAlignmentValue(*ptr);

  if (av.isUnaligned() || av.isUnknown())
    return av;

  assert(av.isAligned() && "unexpected AlignmentValue type");

  int64_t offset = 0;
  gep_type_iterator GTI = gep_type_begin(&gepi);
  User::const_op_iterator I = gepi.op_begin()+1, E = gepi.op_end();
  for (; I != E; ++I, ++GTI) {
    Value *Index = *I;

    // Compute the (potentially symbolic) offset in bytes for this index.
    if (StructType *STy = dyn_cast<StructType>(*GTI)) {
      // For a struct, add the member offset.
      unsigned FieldNo = cast<ConstantInt>(Index)->getZExtValue();
      if (FieldNo == 0) continue;

      offset += DL->getStructLayout(STy)->getElementOffset(FieldNo);
      continue;
    }

    // get element size
    assert(isa<SequentialType>(*GTI) && "unexptected type");
    Type *IndexedType = GTI.getIndexedType();
    if (!IndexedType || !IndexedType->isSized())
      return AlignmentValue::makeUnaligned();
    unsigned elementSize = DL->getTypeAllocSize(IndexedType);

    // For an array/pointer, add the element offset, explicitly scaled.
    if (ConstantInt *CIdx = dyn_cast<ConstantInt>(Index)) {
      if (CIdx->isZero()) continue;
      offset += CIdx->getSExtValue() * elementSize;
      continue;
    }

    AlignmentValue avop = findAlignmentValue(*Index);
    if (avop.isUnknown()) return avop;

    uint64_t thisAlign = avop.isAligned() ? avop.getAlignment() : 1;
    uint64_t elementSizeAlign = calAlignment(elementSize);
    if (thisAlign > getMaxAlignment()/elementSizeAlign)
      thisAlign = getMaxAlignment();
    else
      thisAlign *= elementSizeAlign;

    uint64_t align = std::min(av.getAlignment(), thisAlign);
    if (align <= 1)
      return AlignmentValue::makeUnaligned();
    av = AlignmentValue::makeAligned(align);
  }

  // add the alignment of the combined offset
  if (offset != 0) {
    uint64_t offsetAlign = calAlignment(abs64(offset));
    uint64_t align = std::min(av.getAlignment(), offsetAlign);
    if (align <= 1)
      return AlignmentValue::makeUnaligned();
    av = AlignmentValue::makeAligned(align);
  }

  return av;
}

// the following must match createAlignmentValue()
inline bool AMDAlignmentAnalysis::isHandledOp(Value& v)
{
  if (isa<ConstantInt>(&v)) return true;
  if (isa<CastInst>(&v) || isa<PHINode>(&v)) return true;
  if (isa<GetElementPtrInst>(&v)) return true;

  BinaryOperator *BO = dyn_cast<BinaryOperator>(&v);
  if (!BO) return false;

  switch (BO->getOpcode()) {
    case Instruction::Add:
    case Instruction::Shl:
    case Instruction::Mul:
    case Instruction::Or:
      return true;
    default:;
  }
  return false;
}

// Create AlignmentValue for the given Value
AlignmentValue AMDAlignmentAnalysis::createAlignmentValue(Value& v)
{
  DEBUG(errs() << "  createAlignmentValue begin for " << v << "\n");
  if (ConstantInt *constOp1 = dyn_cast<ConstantInt>(&v)) {
    uint64_t val1 = constOp1->getLimitedValue();
    uint64_t align = calAlignment(val1);
    return AlignmentValue::makeAligned(align);
  }

  if (PHINode *phi = dyn_cast<PHINode>(&v)) {
    return createAlignmentValueForPhi(*phi);
  }

  if (CastInst *CI = dyn_cast<CastInst>(&v)) {
    // Casts do not alter lower bits hence we
    // return a copy from its operand's AlignmentValue
    AlignmentValue av = findAlignmentValue(*CI->getOperand(0));
    return av;
  }

  if (GetElementPtrInst* gepi = dyn_cast<GetElementPtrInst>(&v)) {
    return createAlignmentValueForGEPI(*gepi);
  }

  if (!isa<BinaryOperator>(&v)) {
    uint64_t alignment = getPointerAlignment(v);
    if (alignment > 1)
      return AlignmentValue::makeAligned(alignment);
    return AlignmentValue::makeUnaligned();
  }

  BinaryOperator *BO = dyn_cast<BinaryOperator>(&v);
  switch (BO->getOpcode()) {
    case Instruction::Add:
    {
      return createAlignmentValueForAdd(*BO);
    }
    case Instruction::Shl:
    {
      return createAlignmentValueForShl(*BO);
    }
    case Instruction::Mul:
    {
      return createAlignmentValueForMul(*BO);
    }
    case Instruction::Or:
    {
      return createAlignmentValueForOr(*BO);
    }
    default: ;
  }

  return AlignmentValue::makeUnaligned();
}

// Lookup AlignmentValue for the given Value. If not exist, create
// AlignmentValue for it.
AlignmentValue AMDAlignmentAnalysis::getAlignmentValue(Value& v)
{
  {
    AlignmentValue av = alignmentMap.lookup(v);
    if (!av.isUnknown()) return av;
  }

  // AlignmentValue for v not created yet. Do a DFS on the DFG to create
  // AlignmentValue for it. In order to handle cycles in the DFG, when an
  // instruction's AlignmentValue is changed, we push its uses that belong to
  // the DFG onto a worklist and repeat until no AlignmentValue changes anymore.
  std::stack<Value*> dfsStack;
  std::set<Value*> visited;
  std::vector<Value*> worklist;

  Value* value = &v;
  visited.insert(value);

  while (true) {
    if (isHandledOp(*value) && isa<User>(value)) {
      User* user = dyn_cast<User>(value);
      User::op_iterator it = user->op_begin();
      User::op_iterator end = user->op_end();
      while (it != end) {
        if (isa<BasicBlock>(*it) || visited.find(*it) != visited.end()) {
          ++it;
          continue;
        }

        visited.insert(*it);
        dfsStack.push(user);
        value = *it;
        break;
      }
      if (it != end) continue;
    }

    AlignmentValue oldAlignmentValue = alignmentMap.lookup(*value);
    AlignmentValue newAlignmentValue = createAlignmentValue(*value);
    if (!oldAlignmentValue.equals(newAlignmentValue)) {
      alignmentMap.updateValue(value, newAlignmentValue);
      DEBUG(errs() << "  new alignment value created for " << *value << " ["
                   << newAlignmentValue << "]\n");
      for (Value::use_iterator useIt = value->use_begin(),
           useEnd = value->use_end();
           useIt != useEnd;
           ++useIt)
      {
        Value* use = *useIt;
        if (visited.find(use) != visited.end())
          worklist.push_back(use);
      }
    }
    else {
      DEBUG(errs() << "  alignment value remain for " << *value << " ["
                   << newAlignmentValue << "]\n");
    }

    if (dfsStack.empty()) break;
    value = dfsStack.top();
    dfsStack.pop();
  }

  while (!worklist.empty()) {
    Value* value = worklist.back();
    worklist.pop_back();

    AlignmentValue oldAlignmentValue = alignmentMap.lookup(*value);
    AlignmentValue newAlignmentValue = createAlignmentValue(*value);
    if (!oldAlignmentValue.equals(newAlignmentValue)) {
      alignmentMap.updateValue(value, newAlignmentValue);
      DEBUG(errs() << "  new alignment value created for " << *value << " ["
                   << newAlignmentValue << "]\n");
      for (Value::use_iterator useIt = value->use_begin(),
           useEnd = value->use_end();
           useIt != useEnd;
           ++useIt)
      {
        Value* use = *useIt;
        if (visited.find(use) != visited.end())
          worklist.push_back(use);
      }
    }
    else {
      DEBUG(errs() << "  alignment value remain for " << *value << " ["
                   << newAlignmentValue << "]\n");
    }
  }

  // by now a AlignmentValue should have been created for v, look it up again
  AlignmentValue av = alignmentMap.lookup(v);
  assert(!av.isUnknown() && "bad AlignmentValue created");
  return av;
}

inline uint64_t AMDAlignmentAnalysis::getAlignment(Value& addr)
{
  DEBUG(errs() << "getAlignment begin for " << addr << "\n");
  AlignmentValue av = getAlignmentValue(addr);
  uint64_t alignment = av.getAlignment();
  DEBUG(errs() << "getAlignment return [Align="<< alignment <<"]\n");
  return alignment;
}

bool AMDAlignmentAnalysis::runOnFunction(Function& F)
{
  const OpenCLSymbols &OCLS = getAnalysis<OpenCLSymbols>();
  // We can only assume kernels having aligned pointer-typed parameters.
  // Also, skip stubs
  isKernel = OCLS.isKernel(&F);
  isStub = OCLS.isStub(&F);
  DL = getAnalysisIfAvailable<DataLayout>();
  return false;
}

char AMDAlignmentAnalysis::ID = 0;
INITIALIZE_AG_PASS(AMDAlignmentAnalysis, AlignmentAnalysis,
                   "amdalignmentanalysis",
                   "AMD Alignment Analysis", false, true, true);

FunctionPass *llvm::createAMDAlignmentAnalysisPass()
{
  return new AMDAlignmentAnalysis();
}

#endif // AMD_OPENCL_MEMCOMBINE
