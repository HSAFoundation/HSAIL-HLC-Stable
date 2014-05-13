//===-           Scalar replacement of aggregate array elements           -===//
//===----------------------------------------------------------------------===//
//
// It's a known issue that LDS accesses of larger than 32-bits cause bank
// collisions in the GPU.  For example, a float2 read will cause bank
// collisions because threads 0-31 will access banks
// 0,2,4,6,...,30,0,2,4,...,30 and then 1,3,5,...,31,1,3,5,...,31
// causing collisions.
//
// To address this, we reorganize the data to avoid bank collisions.
// Instead of performing a 64-bit store, it is converted to two
// 32-bit stores as follows:
// - All threads write first 32-bits to DWORD address specified by user
// - All threads write second 32-bits to DWORD address specified by user
//   + work group size
//
// In this way, we will avoid collisions because threads will write to
// consecutive addresses.
//
// To retrieve data, we just perform the same address calculations to
// fetch each 32-bit component.
//
// This can be used for doubles or even for accesses that are larger than
// 64-bits with some simple modifications.
//
// If the user typecasts the local pointer to another vector width and does
// dynamic addressing, we do not optimize that case because the addressing
// will become more complex and might add too much overhead.
//
// TODO: handle the case when the address to the local buffer is passed in
// through a kernel argument.
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "amd-scalar-repl-array-elem"
#include "llvm/Constants.h"
#include "llvm/Instructions.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/DataLayout.h"
#include "llvm/Transforms/Scalar.h"
#include <map>
#include <set>

using namespace llvm;

// options for "opt" driver
static cl::opt<int>
SRAEThreshold(
  "SRAE-threshold", cl::init(1024),
  cl::value_desc("bytes"),
  cl::desc("Maximum size of the local array element aggregrates that"
  " will be scalar replaced."));

static cl::opt<int>
BankWidth("bank-width", cl::init(4),
          cl::value_desc("bytes"),
          cl::desc("bank width"));

static cl::opt<int>
HalfWavefront("half-wavefront", cl::init(32),
          cl::value_desc("work-items"),
          cl::desc("# of work-items in a half-wavefront"));

static cl::opt<bool>
StressSRAE("stress-SRAE", cl::init(false),
          cl::desc("# of work-items in a half-wavefront"));

namespace {

#define LOCAL_ADDRESS_SPACE 3u

// A map data structure that maps a node to a list of nodes
class NodeMap {
private:
  typedef SmallVector<Value*, 4> ValueVec;
  typedef std::map<const Value*, ValueVec*> ValueMap;

private:
  ValueMap _map;
  // # of nodes that each key node map to
  unsigned _valueVecLen;

public:
  NodeMap(unsigned n) : _valueVecLen(n) {}
  ~NodeMap() { clear(); }
  Value* map(const Value* oldNode, Value* newNode, unsigned idx) {
    ValueMap::iterator it = _map.find(oldNode);
    ValueVec* vec;
    if (it == _map.end()) {
      vec = new ValueVec(_valueVecLen);
      _map[oldNode] = vec;
    }
    else {
      vec = it->second;
    }
    return (*vec)[idx] = newNode;
  }

  bool contains(const Value* oldNode) {
    ValueMap::iterator it = _map.find(oldNode);
    if (it == _map.end()) return false;
    return true;
  }

  Value* get(const Value* oldNode, unsigned idx) {
    ValueMap::iterator it = _map.find(oldNode);
    if (it == _map.end()) return NULL;
    ValueVec* vec = it->second;
    assert (vec && "sanity");
    return (*vec)[idx];
  }
  unsigned valueVecLen() const { return _valueVecLen; }
  void clear() {
    for (ValueMap::iterator I = _map.begin(), E = _map.end(); I != E; ++I) {
      ValueVec* vec = I->second;
      delete vec;
    }
    _map.clear();
  }
};

// The main body of this pass
class ScalarReplArrayElem: public ModulePass {
  private:
    // bank width (# of bytes)
    unsigned _bankWidth;
    // # of work-items in a half wavefront
    unsigned _halfWavefront;
    // Max size of local array element aggregrates that will be scalar replaced
    unsigned _sraeThreshold;
    const DataLayout* DL;
    ScalarEvolution* SE;
    // current module being worked on
    Module* _module;

    // --- below are transient values set and used within the algorithm ---

    // map from a old node to the list of new nodes created to replace the old
    NodeMap* _nodeMap;
    // current GlobalVariable being worked on
    GlobalVariable* _currentGV;
    // nodes that become dead during the optimization
    std::vector<Value*> _deadNodes;

  private:
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<ScalarEvolution>();
    }
    bool shouldAttemptScalarRepl(const GlobalVariable& GV) const;
    bool shouldTerminate(const Value& val) const {
      return isa<LoadInst>(&val)
             || isa<StoreInst>(&val)
             || isa<ConstantExpr>(&val);
#if 0
             || isa<ExtractValueInst>(&val)
             || isa<InsertValueInst>(&val)
#endif
    }
    bool shouldRewrite(Value& val, const Value& baseAddr);
    bool isBitcast4Annotation(const Value& v) const;
    bool isExpectedGVUser(const Value& user, const GlobalValue* GV) const
    {
      const GetElementPtrInst* GEPI = dyn_cast<GetElementPtrInst>(&user);
      if (GEPI
          && GEPI->getPointerOperand() == GV
          && GEPI->getNumOperands() == 3) {
        return true;
      }
      return isBitcast4Annotation(user);
    }

    bool shouldAbort(const Value& val) const {
      return isa<BitCastInst>(&val)
             || isa<PtrToIntInst>(&val)
             || isa<IntToPtrInst>(&val)
             || isa<CallInst>(&val);
    }
    bool isInt8Type(const Type* ty) const {
      const IntegerType* ity = dyn_cast<IntegerType>(ty);
      return (ity && ity->getBitWidth() == 8);
    }
    bool isInt8PtrType(const Type* ty) const {
      const PointerType* pty = dyn_cast<PointerType>(ty);
      if (!pty) return false;
      return isInt8Type(pty->getElementType());
    }

    bool isSafeForScalarRepl(GlobalVariable& GV);
    void rewriteGEPI(GetElementPtrInst& oldGEPI);
    Value* combinePowerOf2ValuesWithShuffleVec(Value& oldValue,
                                               uint64_t bIdx,
                                               uint64_t nValues,
                                               Instruction* insertBefore);
    void splitPowerOf2ValuesWithShuffleVec(Value& value,
                                           uint64_t nValues,
                                           uint64_t nExpectedValues,
                                           SmallVector<Value*, 4>& leafValues,
                                           Instruction* insertBefore);
    Value* combineNewLoadedValues(LoadInst& oldLoad);
    Value* combineNewLoadedStructValues(LoadInst& oldLoad);
    void splitValue(Value& value,
                    Type* newType,
                    unsigned nNewValues,
                    SmallVector<Value*, 4>& splitValues,
                    Instruction* insertBefore);
    void splitStructValue(Value& value,
                          SmallVector<Value*, 4>& splitValues,
                          Instruction* insertBefore);
    void rewriteLoad(LoadInst& oldLoad);
    void rewriteStore(StoreInst& oldStore);
    bool rewritePhi(PHINode& phi);
#if 0
    void rewriteInsertValue(InsertValueInst& oldInsert);
    void rewriteExtractValue(ExtractValueInst& oldExtract);
#endif
    void rewriteConstBitcast(ConstantExpr& bitcast);
    bool rewriteUse(Value& use);
    void rewriteForScalarRepl(GlobalVariable& GV);
    void eraseDeadNodes();
    void scalarReplaceGV(GlobalVariable& GV);
    void scalarReplaceStructArrayGV(GlobalVariable& GV);
    void doScalarReplacement(GlobalVariable& GV);

  public:
    static char ID; // Pass identification, replacement for typeid
    ScalarReplArrayElem(unsigned bankWidth = BankWidth,
                        unsigned halfWavefront = HalfWavefront,
                        unsigned sraeThreshold = SRAEThreshold);

    virtual bool runOnModule(Module& M);
    virtual ~ScalarReplArrayElem() {}
  };
}

static Type* getIntType(unsigned nBytes, LLVMContext& context)
{
  switch (nBytes) {
  case 1: return Type::getInt8Ty(context);
  case 2: return Type::getInt16Ty(context);
  case 4: return Type::getInt32Ty(context);
  case 8: return Type::getInt64Ty(context);
  default: return NULL;
  }
  return NULL;
}


char ScalarReplArrayElem::ID = 0;
INITIALIZE_PASS_BEGIN(ScalarReplArrayElem,
                      "amd-scalar-repl-array-elem",
                      "AMD Scalar Replacement Of Array Elements",
                      false,
                      false);
INITIALIZE_PASS_DEPENDENCY(ScalarEvolution);
INITIALIZE_PASS_END(ScalarReplArrayElem,
                    "amd-scalar-repl-array-elem",
                    "AMD Scalar Replacement Of Array Elements",
                    false,
                    false);

ScalarReplArrayElem::ScalarReplArrayElem(unsigned bankWidth,
                                         unsigned halfWavefront,
                                         unsigned sraeThreshold)
  : ModulePass(ID),
    _bankWidth(bankWidth),
    _halfWavefront(halfWavefront),
    _sraeThreshold(sraeThreshold),
    DL(NULL),
    SE(NULL),
    _module(NULL),
    _nodeMap(NULL),
    _currentGV(NULL)
{
  initializeScalarReplArrayElemPass(*PassRegistry::getPassRegistry());
}

// Should we attempt to do scalar replacement for the given global variable?
bool ScalarReplArrayElem::shouldAttemptScalarRepl(
  const GlobalVariable& GV) const
{
  DEBUG(errs() << "  Should attempt scalar replacement?\n");

  if (GV.isDeclaration()) {
    DEBUG(errs() << "    GV is declaration\n");
    return false;
  }

  const PointerType* GVPType = dyn_cast<PointerType>(GV.getType());
  if (!GVPType) {
    DEBUG(errs() << "    GV not pointer type\n");
    return false;
  }

  unsigned addrspace = GVPType->getAddressSpace();
  if (!StressSRAE && addrspace != LOCAL_ADDRESS_SPACE) {
    DEBUG(errs() << "    GV not LOCAL\n");
    return false;
  }
  if (!GV.hasLocalLinkage()) {
    DEBUG(errs() << "    GV linkage not local\n");
    return false;
  }

  // for simplicity, only handle zero initializer
  const Constant* initializer = GV.getInitializer();
  if (!initializer || !isa<ConstantAggregateZero>(initializer)) {
    DEBUG(errs() << "    GV does not have zero initializer\n");
    return false;
  }

  // Local data structures should normally be declared as arrays.
  // Note: if we need to consider vector type. then we need to take care
  // of transforming a lot of instruciton types, such as add, which takes
  // vector of integers as arguments.
  ArrayType* arrayType = dyn_cast<ArrayType>(GVPType->getElementType());
  if (!arrayType) {
    DEBUG(errs() << "    GV not array\n");
    return false;
  }

  // look for elements that are aggregate types or large scalar types
  // so that they can be re-laidout
  Type* elementType = arrayType->getElementType();
  if (isa<CompositeType>(elementType)
      && !isa<ArrayType>(elementType)
      && !isa<VectorType>(elementType)
      && !isa<StructType>(elementType)) {
    DEBUG(errs() << "    GV array element not array or vector or struct\n");
    return false;
  }

  uint64_t bankWidthInBits = _bankWidth * 8;

  // would cause bank conflict only if elementSize > BankWidth
  uint64_t elementSizeInBits = DL->getTypeSizeInBits(elementType);
  if (elementSizeInBits <= bankWidthInBits) {
    DEBUG(errs() << "    GV array element <= bank-width\n");
    return false;
  }
  if (elementSizeInBits % bankWidthInBits != 0) {
    DEBUG(errs() << "    GV array element not multiple of bank-width\n");
    return false;
  }

  // 1. If array elements are structs, optimize it only if target is little
  //    endian, because if the truct has fields wider than bank width, we will
  //    split it before storing and putting it back after loading from LDS.
  // 2. For simplicity, only handle structs that do not have composite
  //    type fields.
  // 3. NOTE: If the struct has fields smaller than bank width, and we split
  //    the struct by fields, then we may end up doing more reads after
  //    optimize it.
  //    If we split up the struct by bank width, then we have to generate extra
  //    extract instructions to get the small fields from the loaded data.
  //    In either case, it is not clear whether the optimization would
  //    out-weight the loss of performance by either doing more reads or
  //    extracts.
  //    So for now, optimize a struct only if all its fields are at least as
  //    wide as the bank width.
  if (isa<StructType>(elementType)) {
    if (!DL->isLittleEndian()) {
      DEBUG(errs() << "    GV array element is struct\n");
      return false;
    }
    StructType* structTy = dyn_cast<StructType>(elementType);
    unsigned nElts = structTy->getNumElements();
    if (nElts == 0) {
      DEBUG(errs() << "    struct GV array element is empty\n");
      return false;
    }
    for (unsigned i = 0; i < nElts; ++i) {
      Type* eet = structTy->getTypeAtIndex(i);
      if (isa<CompositeType>(eet)) {
        DEBUG(errs() << "    struct GV array element has composite field\n");
        return false;
      }
      uint64_t eetSizeInBits = DL->getTypeSizeInBits(eet);
      if (eetSizeInBits < bankWidthInBits) {
        if (!StressSRAE) {
          DEBUG(errs() << "    struct has small fields\n");
          return false;
        }
        else if (bankWidthInBits % eetSizeInBits != 0) {
          DEBUG(errs() << "    eetSize % bank-width != 0\n");
          return false;
        }
      }

      if (eetSizeInBits > bankWidthInBits) {
        if (eetSizeInBits % bankWidthInBits != 0) {
          DEBUG(errs() << "    eetSize % bank-width != 0\n");
          return false;
        }
      }
    }
  }
  else {
    // for now, only handle if array elements' elements are non-composite type
    SequentialType* elemSType = dyn_cast<SequentialType>(elementType);
    Type* eet = elementType;
    if (elemSType) {
      eet = elemSType->getElementType();
      if (isa<CompositeType>(eet)) {
        DEBUG(errs() << "    GV array elements' elements composite type\n");
        return false;
      }
    }

    // If eetSize < _bankWidth, we do the transformation only if the local
    // array's elements are vectors, and target is little endian.
    // e.g. original local array is: <n x <8 x i8>>
    // we will transform it to:
    //   <n x <4 x i8>>
    //   <n x <4 x i8>>
    // Loads/stores to the array elements <4 x i8> will later be transformed
    // by AMDVectorCoarsening to loads/stores to i32, so that the resulting
    // reads/writes to the array elements are a single reads/writes,
    // and the reads/writes by threads in the workgroup will be consecutive.
    // Don't do the transformation if local array's elements are arrays, because
    // bitcast does not allow bit-casting from/to arrays, so we won't be able to
    // convert accesses to [4 x i8] into i32.
    uint64_t eetSizeInBits = DL->getTypeSizeInBits(eet);
    if (eetSizeInBits != bankWidthInBits) {
      if (isa<CompositeType>(elementType) && !isa<VectorType>(elementType)) {
        DEBUG(errs() << "    eetSize != bankWidth && array type\n");
        return false;
      }

      if (!DL->isLittleEndian()) {
        DEBUG(errs() << "    eetSize != bankWidth && !littleEndian\n");
        return false;
      }

      if (eetSizeInBits < bankWidthInBits) {
        if (bankWidthInBits % eetSizeInBits != 0) {
          DEBUG(errs() << "    bank-width % eetSize != 0\n");
          return false;
        }
      }

      if (eetSizeInBits > bankWidthInBits) {
        if (eetSizeInBits % bankWidthInBits != 0) {
          DEBUG(errs() << "    eetSize % bank-width != 0\n");
          return false;
        }
      }
    }
  }

  // if the array element aggregrate is too large, don't try to
  // scalar replace it
  uint64_t elemSize = DL->getTypeStoreSize(elementType);
  if (!StressSRAE && elemSize > _sraeThreshold) {
    DEBUG(errs() << "    GV array elements too large\n");
    return false;
  }

  uint64_t numElements = arrayType->getNumElements();
  if (!StressSRAE && numElements < _halfWavefront) {
    DEBUG(errs() << "  GV array length < half-wavefront\n");
    return false;
  }
  if (!StressSRAE && numElements % _halfWavefront != 0) {
    DEBUG(errs() << "    GV array length not multiple of half-wavefront\n");
    return false;
  }

  DEBUG(errs() << "    yes\n");
  return true;
}

#ifdef CHECK_LID_EXPR
// TODO: peek past zext, sext, and trunc
// TODO: handle expression of multi-dimentions of globa/local ids
static bool isLocalOrGlobalID(const SCEV* scev)
{
  const SCEVUnknown* unkSCEV = dyn_cast<SCEVUnknown>(scev);
  if (unkSCEV == NULL) return false;
  Value* val = unkSCEV->getValue();
  ExtractElementInst* extract = dyn_cast<ExtractElementInst>(val);
  if (extract == NULL) return false;
  Value* vec = extract->getVectorOperand();
  CallInst* call = dyn_cast<CallInst>(vec);
  if (call == NULL) return false;
  Function* func = call->getCalledFunction();
  if (func == NULL) return false;
  StringRef funcName = func->getName();
  return funcName.startswith("__amdil_get_local_id")
         || funcName.startswith("__amdil_get_global_id");
}
#endif

// we should rewrite a load/store only if the given base address is
// the pointer operand of the load/store
bool ScalarReplArrayElem::shouldRewrite(Value& val,
                                        const Value& baseAddr)
{
  Value* addr = NULL;
  if (LoadInst* load = dyn_cast<LoadInst>(&val)) {
    if (!load->isSimple()) return false;
    addr = load->getPointerOperand();
  }
  else if (StoreInst* store = dyn_cast<StoreInst>(&val)) {
    if (!store->isSimple()) return false;
    addr = store->getPointerOperand();
  }
  else if (InsertValueInst* insert = dyn_cast<InsertValueInst>(&val)) {
    addr = insert->getAggregateOperand();
  }
  else if (ExtractValueInst* extract = dyn_cast<ExtractValueInst>(&val)) {
    addr = extract->getAggregateOperand();
  }
  else if (ConstantExpr* c = dyn_cast<ConstantExpr>(&val)) {
    addr = c->getOperand(0);
  }
  else {
    assert(0 && "unexpected instruction type");
    return false;
  }

  if (addr != &baseAddr) return false;

  return true;

#ifdef CHECK_LID_EXPR
  // TODO: check load/store index is expression of get_local_id()
  Instruction* inst = dyn_cast<Instruction>(&val);
  if (!inst) return true;

  // return true only if the address is a linear expression of
  // global or local id
  Function* F = inst->getParent()->getParent();
  SE = &getAnalysis<ScalarEvolution>(*F);
  GetElementPtrInst* GEPI = dyn_cast<GetElementPtrInst>(addr);
  Value* arrayIndex = GEPI->getOperand(2);
  const SCEV* scev = SE->getSCEV(arrayIndex);
  DEBUG(errs() << "SCEV for " << *arrayIndex << ":\n    " << *scev << "\n");
  bool foundID = false;
  if (const SCEVAddExpr* addSCEV = dyn_cast<SCEVAddExpr>(scev)) {
    SCEVNAryExpr::op_iterator it, end;
    for (it = addSCEV->op_begin(), end = addSCEV->op_end(); it != end; ++it) {
      const SCEV* op = *it;
      if (!isLocalOrGlobalID(*it)) continue;
      if (!foundID) {
        foundID = true;
      }
      else {
        return false;
      }
    }
    return foundID;
  }
  if (const SCEVAddRecExpr* addRecSCEV = dyn_cast<SCEVAddRecExpr>(scev)) {
    const SCEV* init = addRecSCEV->getStart();
    return isLocalOrGlobalID(init);
  }
  return false;
#endif
}

// check if the given value is only used for llvm.global.annotation global
// variable
bool ScalarReplArrayElem::isBitcast4Annotation(const Value& v) const
{
  const ConstantExpr* c = dyn_cast<ConstantExpr>(&v);
  if (!c) return false;

  // is c a bitcast to i8*?
  if (c->getOpcode() != Instruction::BitCast) return false;
  if (!isInt8PtrType(c->getType())) return false;

  // is this a chain of constants with no use of the final constant?
  const Value* value = &v;
  while (true) {
    if (!isa<Constant>(value)) return false;
    if (value->use_empty()) return true;
    if (!value->hasOneUse()) return false;
    value = value->use_back();
  }
  return false;
}

// Check if it is safe to scalar replace GV's aggregrate array elements
// We do the check by checking all the direct and indirect uses of GV
// and return true only if all the direct and indirect uses can be
// safely rewritten to handle the transformed GV.
bool ScalarReplArrayElem::isSafeForScalarRepl(GlobalVariable& GV)
{
  DEBUG(errs() << "  Is safe to scalar replace elements of GV?\n");

  typedef std::pair<Value*, const Value*> InstValPair;
  std::set<const Value*> visited;
  std::vector<InstValPair> worklist;

  SmallVector<Constant*, 2> deadConsts;

  // set of pointers pointing to one of the GV's element
  std::set<const Value*> GVElemPtrs;

  // set of phi nodes that have been optimistically set as pointing to
  // one of the GV's element
  std::set<const PHINode*> optPhis;

  // push GV's uses to worklist
  for (Value::use_iterator U = GV.use_begin(), E = GV.use_end();
       U != E; ++U) {
    User* user = *U;
    ConstantExpr* c = dyn_cast<ConstantExpr>(const_cast<User*>(user));
    if (c && c->use_empty()) {
      deadConsts.push_back(c);
      continue;
    }
    if (!isExpectedGVUser(*user, &GV)) {
      DEBUG(errs() << "    " << *user << "not expected user of GV\n");
      return false;
    }
    if (visited.find(user) == visited.end()) {
      worklist.push_back(InstValPair(user, &GV));
      visited.insert(user);
      DEBUG(errs() << "    push " << *user << "to worklist\n");
    }
  }

  // remove dead constants
  while (!deadConsts.empty()) {
    Constant* c = deadConsts.back();
    deadConsts.pop_back();
    DEBUG(errs() << "    Destroying dead constant: " << *c << "...\n");
    c->destroyConstant();
  }

  // if any of GV's direct or indirect use is an unhandled instructions,
  // return false.
  while (!worklist.empty()) {
    InstValPair pair = worklist.back();
    Value& val = *pair.first;
    const Value& baseAddr = *pair.second;
    worklist.pop_back();
    DEBUG(errs() << "    pop " << val << "from worklist\n");

    if (shouldTerminate(val)) {
      if (!shouldRewrite(val, baseAddr)) {
        DEBUG(errs() << "    cannot rewrite instruction\n");
        return false;
      }
      // fallthru
    }
    else if (const GetElementPtrInst* GEPI = dyn_cast<GetElementPtrInst>(&val))
    {
      // check if the GEPI points to one of the GV's element
      const Value* base = GEPI->getPointerOperand();
      size_t numOpers = GEPI->getNumOperands();
      if ((base == &GV && numOpers == 3)
          || ( GVElemPtrs.find(base) != GVElemPtrs.end() && numOpers == 2)) {
        GVElemPtrs.insert(&val);
      }
      else {
        DEBUG(errs() << "    not pointer to GV's element\n");
        return false;
      }
      // fallthru
    }
    else if (const PHINode* PHI = dyn_cast<PHINode>(&val)) {
      bool hasGVElemPtrOper = false;
      bool hasUnvisitedOper = false;
      unsigned n = PHI->getNumIncomingValues();
      for (unsigned i = 0; i < n; ++i) {
        const Value* oper = PHI->getIncomingValue(i);
        if (GVElemPtrs.find(oper) != GVElemPtrs.end()) {
          hasGVElemPtrOper = true;
        }
        else if (visited.find(oper) == visited.end()) {
          hasUnvisitedOper = true;
        }
        else {
          DEBUG(errs() << "    not pointer to GV's element\n");
          return false;
        }
      }
      // If at least one of the phi's oper is a pointer to the GV's element,
      // and all other opers have not been processed yet, optimistically
      // assume the phi is also a pointer to the GV's element, so that if
      // the phi is in a cycle, the cycle can be processed.
      if (hasGVElemPtrOper) {
        GVElemPtrs.insert(PHI);
        if (hasUnvisitedOper && optPhis.find(PHI) == optPhis.end()) {
          optPhis.insert(PHI);
        }
      }
      // If the phi has opers that are not visited yet, clear it from the
      // visited set, so that it can be visited again once its unvisited
      // oper is visited.
      if (hasUnvisitedOper) {
        visited.erase(PHI);
      }
      // fallthru
    }
    else if (shouldAbort(val)) {
      DEBUG(errs() << "    cannot rewrite instruction\n");
      return false;
    }
    else {
      assert(0 && "unexpected instruction type encountered");
      DEBUG(errs() << "    unexpected instruction type\n");
      return false;
    }

    if (shouldTerminate(val)) continue;

    // push val's uses to worklist
    for (Value::use_iterator U = val.use_begin(), E = val.use_end();
         U != E; ++U) {
      User* user = *U;
      if (visited.find(user) == visited.end()) {
        worklist.push_back(InstValPair(user, &val));
        visited.insert(user);
        DEBUG(errs() << "    push " << *user << "to worklist\n");
      }
    }
  }

  // Now that all GV's direct and indirect users have been visited,
  // we check the phis that have been optimistically assumed pointing
  // to the GV's element and see if all their opers point to the GV's element
  for (std::set<const PHINode*>::iterator it = optPhis.begin(),
       end = optPhis.end(); it != end; ++it) {
    const PHINode* PHI = *it;
    unsigned n = PHI->getNumIncomingValues();
    for (unsigned i = 0; i < n; ++i) {
      const Value* oper = PHI->getIncomingValue(i);
      if (GVElemPtrs.find(oper) == GVElemPtrs.end()) {
        DEBUG(errs() << *PHI << " not pointer to GV's element\n");
        return false;
      }
    }
  }

  DEBUG(errs() << "    yes\n");
  return true;
}

// Suppose GV has been replaced by GV1, ..., GVn
// Now rewrite GEPI GV, i32 0, i32 i to:
//     GEPI1 GV1, i32 0, i32 i
//     ...
//     GEPIn GVn, i32 0, i32 i
void ScalarReplArrayElem::rewriteGEPI(GetElementPtrInst& oldGEPI)
{
#ifdef NDEBUG
  unsigned nOpers = oldGEPI.getNumOperands();
  if (nOpers == 3) {
    const Value* index0 = oldGEPI.getOperand(1);
    const ConstantInt* constIndex0 = dyn_cast<ConstantInt>(index0);
    assert(constIndex0 && constIndex0->getLimitedValue() == 0
           && "first index not 0");
  }
#endif // NDEBUG
  Value* oldBase = oldGEPI.getPointerOperand();
  for (uint64_t i = 0; i < _nodeMap->valueVecLen(); ++i) {
    Value* newBase = _nodeMap->get(oldBase, i);
    assert(newBase && "base not cloned yet");
    SmallVector<Value*, 2> indices(oldGEPI.idx_begin(), oldGEPI.idx_end());
    GetElementPtrInst* newGEPI
      = GetElementPtrInst::Create(newBase, makeArrayRef(indices), "", &oldGEPI);
    _nodeMap->map(&oldGEPI, newGEPI, i);
    DEBUG(errs() << "    created new " << *newGEPI << "\n");
  }
  _deadNodes.push_back(&oldGEPI);
}

// A load has been replace by N loads, where N is power-of-2.
// Now create shufflevec instructions to combine the N loads into
// a vector of the same type as that of the original load's value.
Value* ScalarReplArrayElem::combinePowerOf2ValuesWithShuffleVec(
  Value& oldValue, uint64_t bIdx, uint64_t nValues, Instruction* insertBefore)
{
  assert(isPowerOf2_64(nValues) && "not power-of-2");
  if (nValues == 1) {
    return _nodeMap->get(&oldValue, bIdx);
  }

  // recursively combine the left subtree
  Value* left = combinePowerOf2ValuesWithShuffleVec(oldValue,
                                                    bIdx,
                                                    nValues/2,
                                                    insertBefore);
  // recursively combine the right subtree
  Value* right = combinePowerOf2ValuesWithShuffleVec(oldValue,
                                                     bIdx+nValues/2,
                                                     nValues/2,
                                                     insertBefore);

  // create shuffle vector instruction to combine the two subtrees
  IntegerType* int32ty = Type::getInt32Ty(oldValue.getContext());
  VectorType* vtype = dyn_cast<VectorType>(left->getType());
  unsigned nVecElem = 1;
  if (vtype != NULL) {
    nVecElem = vtype->getNumElements();
  }
  SmallVector<Constant*,4> indices;
  for (unsigned i = 0; i < nVecElem<<1; ++i) {
    indices.push_back(ConstantInt::get(int32ty, i));
  }
  Constant* mask = ConstantVector::get(indices);
  Value* shuffle = new ShuffleVectorInst(left, right, mask, "", insertBefore);
  DEBUG(errs() << "    created new " << *shuffle << "\n");
  return shuffle;
}

// A store needs to be replace by N stores, where N is power-of-2.
// Before doing that, first create shufflevec instructions to split
// the value to be stored by the original store into N values
// of the same type as that of values to be stored by the new store's.
void ScalarReplArrayElem::splitPowerOf2ValuesWithShuffleVec(
  Value& value,
  uint64_t nValues,
  uint64_t nExpectedValues,
  SmallVector<Value*, 4>& leafValues,
  Instruction* insertBefore)
{
  if (nValues == nExpectedValues) {
    leafValues.push_back(&value);
    return;
  }

  // create shuffle vector instruction to split the two subtrees
  IntegerType* int32ty = Type::getInt32Ty(value.getContext());
  VectorType* vtype = dyn_cast<VectorType>(value.getType());
  assert(vtype != NULL && "sanity");
  Value* undefVec = UndefValue::get(vtype);
  unsigned nVecElem = vtype->getNumElements();
  assert(nVecElem > 1 && nVecElem % 2 == 0 && "sanity");
  SmallVector<Constant*,4> indices1;
  SmallVector<Constant*,4> indices2;
  for (unsigned i = 0; i < nVecElem; ++i) {
    if (i < (nVecElem >> 1)) {
      indices1.push_back(ConstantInt::get(int32ty, i));
    }
    else {
      indices2.push_back(ConstantInt::get(int32ty, i));
    }
  }
  Constant* mask1 = ConstantVector::get(indices1);
  Constant* mask2 = ConstantVector::get(indices2);
  Value* left
    = new ShuffleVectorInst(&value, undefVec, mask1, "", insertBefore);
  Value* right
    = new ShuffleVectorInst(&value, undefVec, mask2, "", insertBefore);
  DEBUG(errs() << "    created new " << *left << "\n");
  DEBUG(errs() << "    created new " << *right << "\n");

  // recursively split the left subtree
  splitPowerOf2ValuesWithShuffleVec(*left,
                                    nValues << 1,
                                    nExpectedValues,
                                    leafValues,
                                    insertBefore);
  // recursively split the right subtree
  splitPowerOf2ValuesWithShuffleVec(*right,
                                    nValues << 1,
                                    nExpectedValues,
                                    leafValues,
                                    insertBefore);
}

// A load has been replace by N loads.
// Now create insertelement, insertvalue, or shufflevec instructions
// to combine the N loads into a vector of the same type as that of
// the original load's value.
Value* ScalarReplArrayElem::combineNewLoadedValues(LoadInst& oldLoad)
{
  IntegerType *int32ty = Type::getInt32Ty(oldLoad.getContext());
  const Value* oldAddr = oldLoad.getPointerOperand();
  Type* oldPType = oldAddr->getType();
  Type* oldType = dyn_cast<PointerType>(oldPType)->getElementType();
  uint64_t oldSize = DL->getTypeStoreSize(oldType);
  LoadInst* newLoad= dyn_cast<LoadInst>(_nodeMap->get(&oldLoad, 0));
  Type* newPType = newLoad->getPointerOperand()->getType();
  Type* newType = dyn_cast<PointerType>(newPType)->getElementType();
  assert(DL->getTypeStoreSize(newType) == _bankWidth && "sanity");
  // Rewrite:
  //   V = load <m x ee>* GEPI
  // to:
  //   V1 = load ee* GEPI1
  //   ...
  //   Vm = load ee* GEPIm
  //   vec1 = insertelement <m x ee> undef, V1, 0
  //   ...
  //   V = insertelement <m x ee> vec<m-1>, Vm, m-1
  if (!isa<CompositeType>(newType)) {
    Type* localVecTy;
    if (isa<ArrayType>(oldType)) {
      localVecTy = ArrayType::get(newType, oldSize / _bankWidth);
    }
    else {
      localVecTy = VectorType::get(newType, oldSize / _bankWidth);
    }
    Value* localVec = UndefValue::get(localVecTy);
    for (unsigned i = 0; i < _nodeMap->valueVecLen(); ++i) {
      Value* newLoad = _nodeMap->get(&oldLoad, i);
      Value* idxConst = ConstantInt::get(int32ty, i);
      Instruction* insertElem = NULL;
      if (isa<VectorType>(localVecTy)) {
        insertElem = InsertElementInst::Create(localVec, newLoad, idxConst);
      }
      else {
        assert(isa<ArrayType>(localVecTy) && "checked before");
        insertElem = InsertValueInst::Create(localVec, newLoad, i);
      }
      DEBUG(errs() << "    created new " << *insertElem << "\n");
      insertElem->insertBefore(&oldLoad);
      localVec = insertElem;
    }
    if (localVecTy == oldType) {
      return localVec;
    }
    Instruction* bitcast = new BitCastInst(localVec, oldType, "", &oldLoad);
    DEBUG(errs() << "    created new " << *bitcast << "\n");
    return bitcast;
  }

  Value* localVec = UndefValue::get(oldType);
  unsigned nNewValues = _nodeMap->valueVecLen();
  if (isa<VectorType>(newType) && isPowerOf2_64(nNewValues)) {
    return combinePowerOf2ValuesWithShuffleVec(oldLoad,
                                               0,
                                               nNewValues,
                                               &oldLoad);
  }

  // e.g.
  // if original local array of ARR = [n x <8 x i8>] is transformed to:
  //   ARR1 = [n x <4 x i8>]
  //   ARR2 = [n x <4 x i8>]
  // then the original load:
  //  GEPI = getelementptr [n x <8 x i8>]* %ARR, 0, 0
  //  L = load <8 x i8>* %GEPI
  // has been transformed into:
  //  GEPI1 = getelementptr [n x <4 x i8>]* %ARR1, 0, 0
  //  GEPI2 = getelementptr [n x <4 x i8>]* %ARR2, 0, 0
  //  L1 = load <4 x i8>* %GEPI1
  //  L2 = load <4 x i8>* %GEPI2
  // If the target machine is little endian, we now generate the following
  // sequence of codes to combine the values loaded by the new loads:
  //  VAL1 = bitcast <4 x i8>* %L1 to i32
  //  VAL2 = bitcast <4 x i8>* %L2 to i32
  //  V1 = insertelement <2 x i32> %undef, %VAL1, 0
  //  V2 = insertelement <2 x i32> %V2, %VAL2, 1
  //  V = bitcast <2 x i32>* %V2 to <8 x i8>*
  if (DL->isLittleEndian()) {
    Type* bankTy = getIntType(_bankWidth, _module->getContext());
    assert(bankTy && "unsupported bank width");
    assert(isa<VectorType>(oldType) && "checked before");
    Type* localVecTy = VectorType::get(bankTy, _nodeMap->valueVecLen());
    Value* localVec = UndefValue::get(localVecTy);
    for (unsigned i = 0; i < _nodeMap->valueVecLen(); ++i) {
      Value* newLoad = _nodeMap->get(&oldLoad, i);
      newLoad = new BitCastInst(newLoad, bankTy, "", &oldLoad);
      DEBUG(errs() << "    created new " << *newLoad << "\n");
      Value* idxConst = ConstantInt::get(int32ty, i);
      Instruction* insertElem
        = InsertElementInst::Create(localVec, newLoad, idxConst);
      DEBUG(errs() << "    created new " << *insertElem << "\n");
      insertElem->insertBefore(&oldLoad);
      localVec = insertElem;
    }
    return new BitCastInst(localVec, oldType, "", &oldLoad);
  }

  // the slow path to fall back to
  // e.g.
  // Original GV: [n x <4 x i16>] has been rewriten to:
  //    [n x <2 x i16>]
  //    [n x <2 x i16>]
  // Rewrite:
  //   V = load <4 x i16>* GEPI
  // to:
  //   V1 = load <2 x i16>* GEPI1
  //   V2 = load <2 x i16>* GEPI2
  //   val11 = extractelement <2 x i16> V1, 0
  //   val12 = extractelement <2 x i16> V1, 1
  //   vec11 = insertelement <4 x i16>> undef, val11, 0
  //   vec12 = insertelement <4 x i16>> vec11, val12, 1
  //   val21 = extractelement <2 x i16> V2, 0
  //   val22 = extractelement <2 x i16> V2, 1
  //   vec21 = insertelement <4 x i16>> vec12, val21, 2
  //   vec22 = insertelement <4 x i16>> vec21, val22, 3
  for (unsigned i = 0; i < _nodeMap->valueVecLen(); ++i) {
    uint64_t newNumEET
      = isa<VectorType>(newType)
        ? dyn_cast<VectorType>(newType)->getNumElements()
        : dyn_cast<ArrayType>(newType)->getNumElements();
    Value* newLoad = _nodeMap->get(&oldLoad, i);
    for (uint64_t j = 0; j < newNumEET; ++j) {
      Instruction* extractElem;
      Instruction* insertElem;
      if (isa<ArrayType>(newType)) {
        extractElem = ExtractValueInst::Create(newLoad, j);
        insertElem
          = InsertValueInst::Create(localVec, extractElem, i*newNumEET+j);
      }
      else {
        Value* idxConst = ConstantInt::get(int32ty, j);
        extractElem = ExtractElementInst::Create(newLoad, idxConst);
        idxConst = ConstantInt::get(int32ty, i*newNumEET+j);
        insertElem = InsertElementInst::Create(localVec, extractElem, idxConst);
      }
      DEBUG(errs() << "    created new " << *extractElem << "\n");
      DEBUG(errs() << "    created new " << *insertElem << "\n");
      extractElem->insertBefore(&oldLoad);
      insertElem->insertBefore(&oldLoad);
      localVec = insertElem;
    }
  }
  return localVec;
}

// create InsertValue to composite the separated load values into
// the original struct type.
// e.g.
// if original local array of ARR = [n x {i8, i16, i64}] is transformed to:
//   ARR1 = [n x i8]
//   ARR2 = [n x i16]
//   ARR3 = [n x i32]
//   ARR4 = [n x i32]
// then the original load:
//  GEPI = getelementptr [n x {i8, i16, i64}]* %ARR, 0, 0
//  L = load {i8, i16, i64}* %GEPI
// has been transformed into:
//  GEPI1 = getelementptr [n x i8]* %ARR1, 0, 0
//  GEPI2 = getelementptr [n x i16]* %ARR2, 0, 0
//  GEPI3 = getelementptr [n x i32]* %ARR3, 0, 0
//  GEPI4 = getelementptr [n x i32]* %ARR4, 0, 0
//  L1 = load i8* %GEPI1
//  L2 = load i16* %GEPI2
//  L3 = load i32* %GEPI3
//  L4 = load i32* %GEPI4
// If the target machine is little endian, we now generate the following
// sequence of codes to combine the values loaded by the new loads:
//  S1 = insertvalue {i8, i16, i64} %undef, %L1, 0
//  S2 = insertvalue {i8, i16, i64} %S1, %L2, 1
//  V1 = insertelement <2 x i32> %undef, %L3, 0
//  V2 = insertelement <2 x i32> %V2, %L4, 1
//  V = bitcast <2 x i32> %V2 to i64
//  S3 = insertvalue {i8, i16, i64} %S2, %V, 2
Value* ScalarReplArrayElem::combineNewLoadedStructValues(LoadInst& oldLoad)
{
  IntegerType *int32ty = Type::getInt32Ty(oldLoad.getContext());
  const Value* oldAddr = oldLoad.getPointerOperand();
  Type* oldPType = oldAddr->getType();
  Type* oldType = dyn_cast<PointerType>(oldPType)->getElementType();
  assert(isa<StructType>(oldType) && "checked before");
  Value* localStruct = UndefValue::get(oldType);
  const StructType* structTy = dyn_cast<StructType>(oldType);
  unsigned idxLoad = 0;
  for (unsigned i = 0, n = structTy->getNumElements(); i < n; ++i) {
    Type* eetTy = structTy->getElementType(i);
    uint64_t eetSize = DL->getTypeStoreSize(eetTy);
    Value* value;
    if (eetSize <= _bankWidth) {
      Value* newLoad = _nodeMap->get(&oldLoad, idxLoad);
      assert(eetTy == newLoad->getType() && "sanity");
      value = newLoad;
      ++idxLoad;
    }
    else {
      unsigned span = (eetSize + _bankWidth - 1) / _bankWidth;
      Type* bankTy = getIntType(_bankWidth, _module->getContext());
      Type* localVecTy = VectorType::get(bankTy, span);
      Value* localVec = UndefValue::get(localVecTy);
      for (unsigned j = 0; j < span; ++j, ++idxLoad) {
        Value* newLoad = _nodeMap->get(&oldLoad, idxLoad);
        assert(eetTy != newLoad->getType() && "sanity");
        Value* idxConst = ConstantInt::get(int32ty, j);
        Instruction* insertElem
          = InsertElementInst::Create(localVec, newLoad, idxConst);
        DEBUG(errs() << "    created new " << *insertElem << "\n");
        insertElem->insertBefore(&oldLoad);
        localVec = insertElem;
      }
      Instruction* bitcast = new BitCastInst(localVec, eetTy, "", &oldLoad);
      DEBUG(errs() << "    created new " << *bitcast << "\n");
      value = bitcast;
    }
    Instruction* insertValue = InsertValueInst::Create(localStruct, value, i);
    DEBUG(errs() << "    created new " << *insertValue << "\n");
    insertValue->insertBefore(&oldLoad);
    localStruct = insertValue;
  }

  return localStruct;
}

// A store needs to be replace by N stores.
// Before doing that, first create extractelement, extractvalue, or
// shufflevec instructions to split the value to be stored by the
// original store into N values of the same type as that of values
// to be stored by the new store's.
void ScalarReplArrayElem::splitValue(
  Value& value,
  Type* newType,
  unsigned nNewValues,
  SmallVector<Value*, 4>& splitValues,
  Instruction* insertBefore)
{
  IntegerType *int32ty = Type::getInt32Ty(value.getContext());
  Type* oldType = value.getType();
  uint64_t oldSize = DL->getTypeStoreSize(oldType);
  // Rewrite:
  //   store <m x ee> V, <m x ee>* GEPI
  // is rewritten to:
  //   V1 = extractelement <m x ee> V, 0
  //   store ee V1, ee* gep1
  //   ...
  //   Vm = extractelement <m x ee> V, m-1
  //   store ee Vm, ee* GEPIm
  if (!isa<CompositeType>(newType)) {
    Type* localVecTy;
    if (isa<ArrayType>(oldType)) {
      localVecTy = ArrayType::get(newType, oldSize / _bankWidth);
    }
    else {
      localVecTy = VectorType::get(newType, oldSize / _bankWidth);
    }
    Value* localVec;
    if (localVecTy != oldType) {
      localVec = new BitCastInst(&value, localVecTy, "", insertBefore);
      DEBUG(errs() << "    created new " << *localVec << "\n");
    }
    else {
      localVec = &value;
    }
    for (unsigned i = 0; i < nNewValues; ++i) {
      Value* idxConst = ConstantInt::get(int32ty, i);
      Instruction* extractElem = NULL;
      if (isa<VectorType>(localVecTy)) {
        extractElem = ExtractElementInst::Create(localVec, idxConst);
      }
      else {
        assert(isa<ArrayType>(localVecTy) && "checked before");
        extractElem = ExtractValueInst::Create(localVec, i);
      }
      DEBUG(errs() << "    created new " << *extractElem << "\n");
      extractElem->insertBefore(insertBefore);
      splitValues.push_back(extractElem);
    }
    return;
  }

  if (isa<VectorType>(newType) && isPowerOf2_64(nNewValues)) {
    splitPowerOf2ValuesWithShuffleVec(value,
                                      1,
                                      nNewValues,
                                      splitValues,
                                      insertBefore);
    return;
  }

  // e.g.
  // if original local array of ARR = [n x <8 x i8>] is transformed to:
  //   ARR1 = [n x <4 x i8>]
  //   ARR2 = [n x <4 x i8>]
  // the original store is:
  //  GEPI = getelementptr [n x <8 x i8>]* %ARR, 0, 0
  //  store <8 x i8> %VAL, <8 x i8>* %GEPI
  // If the target machine is little endian, we now generate the following
  // sequence of codes to split the value to be stored:
  //  V = bitcast <8 x i8>* %VAL to <2 x i32>*
  //  VAL1 = extractelement <2 x i32> %V, 0
  //  VAL2 = extractelement <2 x i32> %V, 1
  //  V1 = bitcast i32 %VAL1 to <4 x i8>*
  //  V2 = bitcast i32 %VAL2 to <4 x i8>*
  // Then the original store will be transformed into:
  //  GEPI1 = getelementptr [n x <4 x i8>]* %ARR1, 0, 0
  //  GEPI2 = getelementptr [n x <4 x i8>]* %ARR2, 0, 0
  //  store <4 x i8> %V1, <4 x i8>* %GEPI1
  //  store <4 x i8> %V2, <4 x i8>* %GEPI2
  if (DL->isLittleEndian()) {
    Type* bankTy = getIntType(_bankWidth, _module->getContext());
    assert(bankTy && "unsupported bank width");
    assert(isa<VectorType>(oldType) && "checked before");
    Type* tmpVecTy = VectorType::get(bankTy, _nodeMap->valueVecLen());
    Value* tmpVec = new BitCastInst(&value, tmpVecTy, "", insertBefore);
    for (unsigned i = 0; i < nNewValues; ++i) {
      Value* idxConst = ConstantInt::get(int32ty, i);
      assert(isa<VectorType>(oldType) && "checked before");
      Instruction* extractElem = ExtractElementInst::Create(tmpVec, idxConst);
      DEBUG(errs() << "    created new " << *extractElem << "\n");
      extractElem->insertBefore(insertBefore);
      Value* newValue = new BitCastInst(extractElem, newType, "", insertBefore);
      DEBUG(errs() << "    created new " << *newValue << "\n");
      splitValues.push_back(newValue);
    }
    return;
  }

  // the slow path to fall back to
  // e.g.
  // Original GV: [n x <4 x i16>] has been rewriten to:
  //    [n x <2 x i16>]
  //    [n x <2 x i16>]
  // Rewrite
  //   store V, <4 x i16>* GEPI
  // to:
  //   val11 = extractelement <4 x i16> V, 0
  //   val12 = extractelement <4 x i16> V, 1
  //   vec11 = insertelement <2 x i16>> undef, val11, 0
  //   vec12 = insertelement <2 x i16>> vec11, val12, 1
  //   val21 = extractelement <4 x i16> V, 2
  //   val22 = extractelement <4 x i16> V, 3
  //   vec21 = insertelement <2 x i16>> undef, val21, 0
  //   vec22 = insertelement <2 x i16>> vec21, val22, 1
  //   store vec12, <2 x i16>* GEPI1
  //   store vec22, <2 x i16>* GEPI2
  uint64_t newNumEET
    = isa<ArrayType>(newType)
      ? dyn_cast<ArrayType>(newType)->getNumElements()
      : dyn_cast<VectorType>(newType)->getNumElements();
  unsigned n = nNewValues / newNumEET;
  for (unsigned i = 0; i < nNewValues; ++i) {
    Value* localVec = UndefValue::get(newType);
    for (uint64_t j = 0; j < newNumEET; ++j) {
      Instruction* extractElem;
      Instruction* insertElem;
      if (isa<ArrayType>(newType)) {
        extractElem = ExtractValueInst::Create(&value, i*newNumEET+j);
        insertElem = InsertValueInst::Create(localVec, extractElem, j);
      }
      else {
        Value* idxConst = ConstantInt::get(int32ty, i*newNumEET+j);
        extractElem = ExtractElementInst::Create(&value, idxConst);
        idxConst = ConstantInt::get(int32ty, j);
        insertElem = InsertElementInst::Create(localVec, extractElem, idxConst);
      }
      extractElem->insertBefore(insertBefore);
      insertElem->insertBefore(insertBefore);
      DEBUG(errs() << "    created new " << *extractElem << "\n");
      DEBUG(errs() << "    created new " << *insertElem << "\n");
      localVec = insertElem;
    }
    splitValues.push_back(localVec);
  }
}

void ScalarReplArrayElem::splitStructValue(
  Value& value,
  SmallVector<Value*, 4>& splitValues,
  Instruction* insertBefore)
{
  IntegerType *int32ty = Type::getInt32Ty(value.getContext());
  StructType* structTy = dyn_cast<StructType>(value.getType());
  for (unsigned i = 0, n = structTy->getNumElements(); i < n; ++i) {
    Type* eetTy = structTy->getElementType(i);
    uint64_t eetSize = DL->getTypeStoreSize(eetTy);
    Instruction* extractValue = ExtractValueInst::Create(&value, i);
    DEBUG(errs() << "    created new " << *extractValue << "\n");
    extractValue->insertBefore(insertBefore);
    if (eetSize <= _bankWidth) {
      splitValues.push_back(extractValue);
    }
    else {
      unsigned span = (eetSize + _bankWidth - 1) / _bankWidth;
      Type* bankTy = getIntType(_bankWidth, _module->getContext());
      Type* localVecTy = VectorType::get(bankTy, span);
      Instruction* bitcast
        = new BitCastInst(extractValue, localVecTy, "", insertBefore);
      DEBUG(errs() << "    created new " << *bitcast << "\n");
      for (unsigned j = 0; j < span; ++j) {
        Value* idxConst = ConstantInt::get(int32ty, j);
        Instruction* extractElm = ExtractElementInst::Create(bitcast, idxConst);
        DEBUG(errs() << "    created new " << *extractElm << "\n");
        extractElm->insertBefore(insertBefore);
        splitValues.push_back(extractElm);
      }
    }
  }
}

// Suppose GV and GEPI has already been re-written.
// Now rewrite load.
void ScalarReplArrayElem::rewriteLoad(LoadInst& oldLoad)
{
  Value* oldAddr = oldLoad.getPointerOperand();
  unsigned origAlign = oldLoad.getAlignment();
  Value* newGV = _nodeMap->get(_currentGV, 0);
  PointerType* newGVPType = dyn_cast<PointerType>(newGV->getType());
  Type* newGVType = newGVPType->getElementType();
  unsigned newGVSize = DL->getTypeAllocSize(newGVType);
  // rewrite the load inst
  for (unsigned i = 0; i < _nodeMap->valueVecLen(); ++i) {
    Value* newAddr = _nodeMap->get(oldAddr, i);
    assert(newAddr && "addr not cloned yet");
    LoadInst* newLoad = new LoadInst(newAddr, "", &oldLoad);
    DEBUG(errs() << "    created new " << *newLoad << "\n");
    unsigned align = i == 0 ? origAlign : std::min(origAlign, i * newGVSize);
    newLoad->setAlignment(align);
    _nodeMap->map(&oldLoad, newLoad, i);
  }

  Value* loadedValue;
  const PointerType* dataPType = dyn_cast<PointerType>(oldAddr->getType());
  const Type* dataType = dataPType->getElementType();
  if (isa<StructType>(dataType)) {
    loadedValue = combineNewLoadedStructValues(oldLoad);
  }
  else {
    loadedValue = combineNewLoadedValues(oldLoad);
  }
  oldLoad.replaceAllUsesWith(loadedValue);
  _deadNodes.push_back(&oldLoad);
}

void ScalarReplArrayElem::rewriteStore(StoreInst& oldStore)
{
  Value* oldAddr = oldStore.getPointerOperand();
  unsigned origAlign = oldStore.getAlignment();
  Value* newGV = _nodeMap->get(_currentGV, 0);
  PointerType* newGVPType = dyn_cast<PointerType>(newGV->getType());
  Type* newGVType = newGVPType->getElementType();
  unsigned newGVSize = DL->getTypeAllocSize(newGVType);
  SmallVector<Value*, 4> newStoreValues;

  Value* value = oldStore.getValueOperand();
  if (isa<StructType>(value->getType())) {
    splitStructValue(*value, newStoreValues, &oldStore);
  }
  else {
    Value* newAddr = _nodeMap->get(oldAddr, 0);
    PointerType* newBasePType = dyn_cast<PointerType>(newAddr->getType());
    Type* newBaseType = newBasePType->getElementType();
    unsigned nNewValues = _nodeMap->valueVecLen();
    splitValue(*value, newBaseType, nNewValues, newStoreValues, &oldStore);
  }

  // rewrite the store inst
  for (unsigned i = 0; i < _nodeMap->valueVecLen(); ++i) {
    Value* newAddr = _nodeMap->get(oldAddr, i);
    assert(newAddr && "addr not cloned yet");
    StoreInst* newStore = new StoreInst(newStoreValues[i], newAddr, &oldStore);
    unsigned align = i == 0 ? origAlign : std::min(origAlign, i * newGVSize);
    newStore->setAlignment(align);
    DEBUG(errs() << "    created new " << *newStore << "\n");
    _nodeMap->map(&oldStore, newStore, i);
  }
  _deadNodes.push_back(&oldStore);
}

// If new phi's are not created for the phi yet, create them.
// Don't set the new phis' incoming values until all of the phi opers
// have been rewritten.
// Returns whether this phi node needs to be re-visited. This phi node needs
// to be re-visited if not all of its opers have been rewritten.
bool ScalarReplArrayElem::rewritePhi(PHINode& phi)
{
  // Check if all of the phi's opers have been rewritten. If not, find one
  // that has been rewritten.
  bool needRevisit = false;
  bool newPhiCreated = _nodeMap->contains(&phi);
  Value* visitedOper = NULL;
  unsigned n = phi.getNumIncomingValues();
  for (unsigned i = 0; i < n; ++i) {
    Value* oldOper = phi.getIncomingValue(i);
    if (!_nodeMap->contains(oldOper)) {
      needRevisit = true;
      if (newPhiCreated || visitedOper) break;
      continue;
    }
    visitedOper = oldOper;
    if (needRevisit) break;
  }

  // if new phis not created yet, create them
  if (!newPhiCreated && visitedOper) {
    for (unsigned i = 0; i < _nodeMap->valueVecLen(); ++i) {
      Value* newOper = _nodeMap->get(visitedOper, i);
      assert(newOper && "new opers created not complete");

      PHINode* newPhi = PHINode::Create(newOper->getType(), n, "", &phi);
      DEBUG(errs() << "    created new " << *newPhi << "\n");
      _nodeMap->map(&phi, newPhi, i);
    }
  }

  if (needRevisit) {
    return needRevisit;
  }

  // if all opers have been rewritten, set new phis' opers
  for (unsigned i = 0; i < n; ++i) {
    Value* oldOper = phi.getIncomingValue(i);
    BasicBlock* block = phi.getIncomingBlock(i);
    for (unsigned j = 0; j < _nodeMap->valueVecLen(); ++j) {
      Value* newOper = _nodeMap->get(oldOper, j);
      assert(newOper && "oper not rewritten yet");

      PHINode* newPhi = dyn_cast<PHINode>(_nodeMap->get(&phi, j));
      assert(newPhi && "new phis created not complete");
      newPhi->addIncoming(newOper, block);
    }
  }
  // remove old phi's opers to break any cycle so phi can be removed
  for (unsigned i = n-1; ; --i) {
    phi.removeIncomingValue(i, false);
    if (i == 0) break;
  }
  _deadNodes.push_back(&phi);
  return needRevisit;
}

#if 0
// Rewrite:
//   insertvalue [n x <m x ee>] GV, <m x ee> V, idx
// to:
//   V1 = extractelement <m x ee> V, 0
//   insertvalue [n x ee] GV1, ee V1, idx
//   ...
//   Vm = extractelement <m x ee> V, m-1
//   insertvalue [n x ee] GVm, ee Vm, idx
void ScalarReplArrayElem::rewriteInsertValue(InsertValueInst& oldInsert)
{
  assert(0 && "ShoudNotReach");
  const IntegerType *int32ty = Type::getInt32Ty(oldInsert.getContext());
  Value* oldAggregate = oldInsert.getAggregateOperand();
  const Type* aggrtype = oldAggregate->getType();
  const Type* elemType = dyn_cast<ArrayType>(aggrtype)->getElementType();
  Value* value = oldInsert.getInsertedValueOperand();
  for (uint64_t i = 0; i < _nodeMap->valueVecLen(); ++i) {
    Value* idxConst = ConstantInt::get(int32ty, i);
    Instruction* extractElem = NULL;
    if (isa<VectorType>(elemType)) {
      extractElem = ExtractElementInst::Create(value, idxConst);
    }
    else {
      assert(isa<ArrayType>(elemType) && "checked before");
      extractElem = ExtractValueInst::Create(value, i);
    }
    DEBUG(errs() << "    created new " << *extractElem << "\n");
    extractElem->insertBefore(&oldInsert);
    Value* newAggregate = _nodeMap->get(oldAggregate, i);
    assert(newAggregate && "addr not cloned yet");
    InsertValueInst* newInsert
      = InsertValueInst::Create(newAggregate, extractElem, i, "", &oldInsert);
    DEBUG(errs() << "    created new " << *newInsert << "\n");
    _nodeMap->map(&oldInsert, newInsert, i);
  }
  _deadNodes.push_back(&oldInsert);
}

// Rewrite
//   V = extractvalue [n x <m x ee>] GV, idx
// to:
//   V1 = extractvalue [n x ee] GV1, idx
//   ...
//   Vm = extractvalue [n x ee] GVm, idx
//   vec1 = insertelement <m x ee> undef, V1, 0
//   ...
//   V = insertelement <m x ee> vec<m-1>, Vm, m-1
//
void ScalarReplArrayElem::rewriteExtractValue(ExtractValueInst& oldExtract)
{
  assert(0 && "ShoudNotReach");
  const IntegerType *int32ty = Type::getInt32Ty(oldExtract.getContext());
  Value* oldAggregate = oldExtract.getAggregateOperand();
  const Type* aggrtype = oldAggregate->getType();
  const Type* elemType = dyn_cast<ArrayType>(aggrtype)->getElementType();
  Value* localVec = UndefValue::get(elemType);
  for (uint64_t i = 0; i < _nodeMap->valueVecLen(); ++i) {
    Value* newAggregate = _nodeMap->get(oldAggregate, i);
    assert(newAggregate && "addr not cloned yet");
    ExtractValueInst* newExtract
      = ExtractValueInst::Create(newAggregate, i, "", &oldExtract);
    DEBUG(errs() << "    created new " << *newExtract << "\n");
    _nodeMap->map(&oldExtract, newExtract, i);
    Value* idxConst = ConstantInt::get(int32ty, i);
    Instruction* insertElem = NULL;
    if (isa<VectorType>(elemType)) {
      insertElem = InsertElementInst::Create(localVec, newExtract, idxConst);
    }
    else {
      insertElem = InsertValueInst::Create(localVec, newExtract, i);
    }
    DEBUG(errs() << "    created new " << *insertElem << "\n");
    insertElem->insertBefore(&oldExtract);
    localVec = insertElem;
  }
  oldExtract.replaceAllUsesWith(localVec);
  _deadNodes.push_back(&oldExtract);
}
#endif

// original global variables for annotation:
//
// @lds_arr = addressspace(3) global [n x <m x eet>]
// @lvgv = constant [1 x i8*] [i8* bitcast ([n x <m x eet>] addrspace(3)* @lds_arr to i8*)]
// @llvm.global.annotations = ..., i8* bitcast ([1 x i8*]* @lvgv to i8*), ...
//
// we convert them to:
//
// @lds_arr1 = addressspace(3) global [n x eet]
// ...
// @lds_arrm = addressspace(3) global [n x eet]
// @lvgv = constant [m x i8*] [i8* bitcast ([n x <m x eet>] addrspace(3)* @lds_arr1 to i8*), ..., i8* bitcast ([n x eet] addrspace(3)* @lds_arrm to i8*)]
// @llvm.global.annotations = ..., i8* bitcast ([m x i8*]* @lvgv to i8*), ...
//
void ScalarReplArrayElem::rewriteConstBitcast(ConstantExpr& bitcast)
{
  // rewrite the const bitcast itself
  assert(bitcast.getOpcode() == Instruction::BitCast && "checked before");
  for (uint64_t i = 0; i < _nodeMap->valueVecLen(); ++i) {
    Constant* newGV = dyn_cast<Constant>(_nodeMap->get(_currentGV, i));
    Constant* newBitcast = ConstantExpr::getBitCast(newGV, bitcast.getType());
    DEBUG(errs() << "    created new " << *newBitcast << "\n");
    _nodeMap->map(&bitcast, newBitcast, i);
  }
  _deadNodes.push_back(&bitcast);

  // rewrite bitcast's use which is a const array
  assert(bitcast.hasOneUse() && "checked before");
  User* user = bitcast.use_back();
  assert(isa<ConstantArray>(user) && "unexpected bitcast user");
  ConstantArray* array = dyn_cast<ConstantArray>(user);
  std::vector<Constant*> arrayElts;
  for (unsigned i = 0, e = array->getNumOperands(); i < e; ++i) {
    Constant* elt = array->getOperand(i);
    if (elt == &bitcast) {
      for (uint64_t j = 0; j < _nodeMap->valueVecLen(); ++j) {
        Value* newBitcast = _nodeMap->get(&bitcast, j);
        arrayElts.push_back(dyn_cast<Constant>(newBitcast));
      }
    }
    else {
      arrayElts.push_back(array->getOperand(i));
    }
  }
  ArrayType* aty = ArrayType::get(bitcast.getType(), arrayElts.size());
  Constant* newConstArr = ConstantArray::get(aty, arrayElts);
  DEBUG(errs() << "    created new " << *newConstArr << "\n");
  _deadNodes.push_back(user);

  // rewrite const array's use which is lvgv global variable
  assert(user->hasOneUse() && "checked before");
  GlobalVariable* lvgv = dyn_cast<GlobalVariable>(user->use_back());
  assert(lvgv && "unexpected const array user");
  std::string lvgvName = lvgv->getName().str();
  // rename old lvgv so that new lvgv can take old lvgv's name
  lvgv->setName("orig_lvgv");
  GlobalVariable* newLvgv = new GlobalVariable(*_module,
                                               newConstArr->getType(),
                                               lvgv->isConstant(),
                                               lvgv->getLinkage(),
                                               newConstArr,
                                               lvgvName,
                                               lvgv,/*InsertBefore*/
                                               lvgv->getThreadLocalMode());
  DEBUG(errs() << "    created new " << *newLvgv << "\n");
  _deadNodes.push_back(lvgv);

  // rewrite lvgv's use which is a bitcast
  assert(lvgv->hasOneUse() && "checked before");
  ConstantExpr* lvgvCast = dyn_cast<ConstantExpr>(lvgv->use_back());
  assert(lvgvCast && lvgvCast->getOpcode() == Instruction::BitCast
         && "unexpected const array user");
  Constant* newBitcast = ConstantExpr::getBitCast(newLvgv, lvgvCast->getType());
  DEBUG(errs() << "    created new " << *newBitcast << "\n");
  lvgvCast->replaceAllUsesWith(newBitcast);
  _deadNodes.push_back(lvgvCast);
}

bool ScalarReplArrayElem::rewriteUse(Value& use)
{
  DEBUG(errs() << "  rewriting " << use << "\n");
  if (isa<GetElementPtrInst>(&use)) {
    GetElementPtrInst* GEPI = dyn_cast<GetElementPtrInst>(&use);
    rewriteGEPI(*GEPI);
    return false;
  }
  else if (isa<LoadInst>(&use)) {
    LoadInst* load = dyn_cast<LoadInst>(&use);
    rewriteLoad(*load);
    return false;
  }
  else if (isa<StoreInst>(&use)) {
    StoreInst* store = dyn_cast<StoreInst>(&use);
    rewriteStore(*store);
    return false;
  }
  else if (isa<PHINode>(&use)) {
    PHINode* phi = dyn_cast<PHINode>(&use);
    return rewritePhi(*phi);
  }
#if 0
  else if (isa<InsertValueInst>(&use)) {
    InsertValueInst* insert = dyn_cast<InsertValueInst>(&use);
    rewriteInsertValue(*insert);
  }
  else if (isa<ExtractValueInst>(&use)) {
    ExtractValueInst* extract = dyn_cast<ExtractValueInst>(&use);
    rewriteExtractValue(*extract);
  }
#endif
  else if (isa<ConstantExpr>(&use)) {
    ConstantExpr* c = dyn_cast<ConstantExpr>(&use);
    rewriteConstBitcast(*c);
    return false;
  }
  assert(0 && "should not reach here");
  return false;
}

// rewrite GV's direct and indiret uses to handle the transformed GV
void ScalarReplArrayElem::rewriteForScalarRepl(GlobalVariable& GV)
{
  std::set<const Value*> visited;
  std::vector<Value*> worklist;

  // push GV's uses to worklist
  for (Value::use_iterator U = GV.use_begin(), E = GV.use_end();
       U != E; ++U) {
    User* user = *U;
    assert(isExpectedGVUser(*user, &GV) && "checked before");
    if (visited.find(user) == visited.end()) {
      worklist.push_back(user);
      visited.insert(user);
    }
  }

  while (!worklist.empty()) {
    Value& use = *worklist.back();
    worklist.pop_back();

    assert((shouldTerminate(use) || isa<GetElementPtrInst>(use)
            || isa<PHINode>(use)) && "sanity");

    bool needRevisit = rewriteUse(use);
    if (needRevisit) {
      visited.erase(&use);
    }

    if (shouldTerminate(use)) continue;

    // push use's uses to worklist
    for (Value::use_iterator U = use.use_begin(), E = use.use_end();
         U != E; ++U) {
      Value* useUse = *U;
      if (visited.find(useUse) == visited.end()) {
        worklist.push_back(useUse);
        visited.insert(useUse);
      }
    }
  }
}

void ScalarReplArrayElem::eraseDeadNodes()
{
  while (!_deadNodes.empty()) {
    Value* val = _deadNodes.back();
    _deadNodes.pop_back();
    // if its use is not empty yet, all uses should be on the _deadNodes list.
    // push current val to the front of the list so its use get removed first.
    if (!val->use_empty()) {
#ifndef NDEBUG
      for (Value::use_iterator U = val->use_begin(), E = val->use_end();
           U != E; ++U) {
        assert(std::find(_deadNodes.begin(), _deadNodes.end(), *U)
               != _deadNodes.end() && "erasing dead value while use not empty");
      }
#endif // NDEBUG
      _deadNodes.insert(_deadNodes.begin(), val);
      continue;
    }
    DEBUG(errs() << "  erasing dead " << *val << "\n");
    if (GlobalVariable* gv = dyn_cast<GlobalVariable>(val)) {
      gv->eraseFromParent();
    }
    else if (Instruction* inst = dyn_cast<Instruction>(val)) {
      inst->eraseFromParent();
    }
    else {
      Constant* c = dyn_cast<Constant>(val);
      c->destroyConstant();
    }
  }
}

// Scalar replace GV's aggregate array elements.
// eg1.
// GV = [n x <m x i32>]
// Scalar replace GV's elements to:
//   GV1 = [n x i32]
//   ...
//   GVm = [n x i32]
//
// eg2.
// GV = [n x i64]
// Scalar replace GV's elements to:
//   GV1 = [n x i32]
//   GVm = [n x i32]
//
// eg3.
// GV = [n x <4 x i16>]
// Scalar replace GV's elements to:
//   GV1 = [n x <2 x i16>]
//   GVm = [n x <2 x i16>]
void ScalarReplArrayElem::scalarReplaceGV(GlobalVariable& GV)
{
  PointerType* GVPType = GV.getType();
  unsigned addrspace = GVPType->getAddressSpace();
  ArrayType* arrayType = dyn_cast<ArrayType>(GVPType->getElementType());
  assert(arrayType && "not array type");
  uint64_t numElements = arrayType->getNumElements();
  Type* elementType = arrayType->getElementType();
  assert((!isa<CompositeType>(elementType)
          || isa<ArrayType>(elementType)
          || isa<VectorType>(elementType))
          && "sanity");

  uint64_t newNumElts = DL->getTypeStoreSize(elementType) / _bankWidth;
  Type* eet;
  if (!isa<CompositeType>(elementType)) {
    eet = elementType;
  }
  else if (const SequentialType* seqEltTy
           = dyn_cast<SequentialType>(elementType)) {
    eet = seqEltTy->getElementType();
  }
  uint64_t eetSize = DL->getTypeStoreSize(eet);
  Type* newEltTy;
  if (eetSize == _bankWidth) {
    newEltTy = eet;
  }
  else if (eetSize < _bankWidth) {
    if (isa<ArrayType>(elementType)) {
      newEltTy = ArrayType::get(eet, _bankWidth / eetSize);
    }
    else {
      assert(isa<VectorType>(elementType) && "checked before");
      newEltTy = VectorType::get(eet, _bankWidth / eetSize);
    }
  }
  else {
    newEltTy = getIntType(_bankWidth, _module->getContext());
  }

  Type* newType = ArrayType::get(newEltTy, numElements);

  _nodeMap = new NodeMap(newNumElts);

  for (uint64_t i = 0; i < newNumElts; ++i) {
    Twine newName(GV.getName());
    newName = newName.concat(Twine(i));
    Constant* initializer = ConstantAggregateZero::get(newType);
    GlobalVariable* newGV = new GlobalVariable(*_module,
                                               newType,
                                               GV.isConstant(),
                                               GV.getLinkage(),
                                               initializer,
                                               newName,
                                               &GV,/*InsertBefore*/
                                               GV.getThreadLocalMode(),
                                               addrspace);
    _nodeMap->map(&GV, newGV, i);
    DEBUG(errs() << "    created new global variable " << *newGV << "\n");
  }

  _deadNodes.push_back(&GV);
}

// GV is array of structs.
// Scalar replace GV's aggregate array elements (structs).
// eg1.
// GV = [n x {i32, float, i64}]
// Scalar replace GV's elements to:
//   GV3 = [n x i32]
//   GV3 = [n x float]
//   GV3 = [n x i32]
//   GV4 = [n x i32]
void ScalarReplArrayElem::scalarReplaceStructArrayGV(GlobalVariable& GV)
{
  const PointerType* GVPType = GV.getType();
  unsigned addrspace = GVPType->getAddressSpace();
  const ArrayType* arrayType = dyn_cast<ArrayType>(GVPType->getElementType());
  assert(arrayType && "not array type");
  uint64_t numElements = arrayType->getNumElements();
  Type* elementType = arrayType->getElementType();
  assert(isa<StructType>(elementType) && "sanity");
  StructType* structTy = dyn_cast<StructType>(elementType);
  const StructLayout* structLayout = DL->getStructLayout(structTy);

  // calculate how many new GVs we are going devide this GV into
  uint64_t newNumElts = 0;
  for (unsigned i = 0, n = structTy->getNumElements(); i < n; ++i) {
    Type* eetTy = structTy->getElementType(i);
    uint64_t eetSize = DL->getTypeStoreSize(eetTy);
    newNumElts += (eetSize + _bankWidth - 1) / _bankWidth;
  }

  _nodeMap = new NodeMap(newNumElts);
  unsigned idxStruct = 0;
  Type* eetTy = structTy->getElementType(idxStruct);
  uint64_t eetSize = DL->getTypeStoreSize(eetTy);
  uint64_t sizeLeft = eetSize;

  for (unsigned i = 0; i < newNumElts; ++i) {
    Type* newEltTy = NULL;

    if (eetSize <= _bankWidth) {
      newEltTy = eetTy;
    }
    else {
      newEltTy = getIntType(_bankWidth, _module->getContext());
    }

    if (sizeLeft <= _bankWidth) {
      ++idxStruct; // go to next field of struct
      if (idxStruct < structTy->getNumElements()) {
        eetTy = structTy->getElementType(idxStruct);
        eetSize = DL->getTypeStoreSize(eetTy);
        sizeLeft = eetSize;
      }
      else {
        assert(i == newNumElts-1 && "sanity");
      }
    }
    else {
      sizeLeft -= _bankWidth;
    }

    Type* newType = ArrayType::get(newEltTy, numElements);

    Twine newName(GV.getName());
    newName = newName.concat(Twine(i));
    Constant* initializer = ConstantAggregateZero::get(newType);
    GlobalVariable* newGV = new GlobalVariable(*_module,
                                               newType,
                                               GV.isConstant(),
                                               GV.getLinkage(),
                                               initializer,
                                               newName,
                                               &GV,/*InsertBefore*/
                                               GV.getThreadLocalMode(),
                                               addrspace);
    _nodeMap->map(&GV, newGV, i);
    DEBUG(errs() << "    created new global variable " << *newGV << "\n");
  }

  _deadNodes.push_back(&GV);
}

// Do scalar replacement for the given global variable by replacing
// the global variable array's elements with scalar types, then
// rewriting the global variable's users to use the newly created
// global variables instead.
void ScalarReplArrayElem::doScalarReplacement(GlobalVariable& GV)
{
  DEBUG(errs() << "  Scalar replacing array element for " << GV << "\n");

  _currentGV = &GV;

  const ArrayType* arrayType
    = dyn_cast<ArrayType>(GV.getType()->getElementType());
  const Type* elementType = arrayType->getElementType();
  if (isa<StructType>(elementType)) {
    scalarReplaceStructArrayGV(GV);
  }
  else {
    scalarReplaceGV(GV);
  }

  // rewrite GV's direct and indiret uses to handle the transformed GV
  rewriteForScalarRepl(GV);

  eraseDeadNodes();

  delete _nodeMap;
  _nodeMap = NULL;
  _currentGV = NULL;
}

// driver of this class
// This is the main transformation entry point for a module.
bool ScalarReplArrayElem::runOnModule(Module& M) {
  if (_sraeThreshold == 0) return false;

  DL = getAnalysisIfAvailable<DataLayout>();
  _module = &M;

  // we can't do all in one loop where we iterate the GV list while scalar
  // replacing some GVs, because scalar replacing a GV modifies the GV
  // list's iterator.
  // So we have to push candidate GV's into a temporary worklist first,
  // them scalar replacing them.

  std::vector<GlobalVariable*> worklist;
  for (Module::global_iterator I = M.global_begin(), E = M.global_end();
       I != E; ++I) {
    GlobalVariable* GV = I;

    if (GV->use_empty()) {
      continue;
    }

    DEBUG(errs() << "Processing: " << *GV << "\n");
    if (!shouldAttemptScalarRepl(*GV)) continue;

    worklist.push_back(GV);
  }

  while (!worklist.empty()) {
    GlobalVariable& GV = *worklist.back();
    worklist.pop_back();

    DEBUG(errs() << "Processing: " << GV << "\n");
    if (!isSafeForScalarRepl(GV)) {
      continue;
    }
    doScalarReplacement(GV);
  }

  return false;
}

// createAMDScalarReplArrayElemPass - The public interface to this file...
ModulePass *llvm::createAMDScalarReplArrayElemPass(unsigned bankWidth,
                                                   unsigned halfWavefront,
                                                   unsigned SRAEThreshold)
{
  return new ScalarReplArrayElem(bankWidth, halfWavefront, SRAEThreshold);
}
