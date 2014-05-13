//===- AMDLowerAtomics.cpp ------------===//
//
// Copyright(c) 2014 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief Convert atomic intrinsic calls to LLVM IR instructions.
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "amdloweratomic"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Value.h"
#include "llvm/Module.h"
#include "llvm/Instructions.h"
#include "llvm/IRBuilder.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "../../lib/Transforms/SPIR/cxa_demangle.h"

using namespace llvm;

// Declarations for lowering OCL 1.x atomics
namespace AMDOCL1XAtomic {
llvm::cl::opt<unsigned> OCL1XAtomicOrder("amd-ocl1x-atomic-order",
    llvm::cl::init(unsigned(llvm::Acquire)),
    llvm::cl::Hidden,
    llvm::cl::desc("AMD OCL 1.x atomic ordering for x86/x86-64"));

  llvm::Value* LowerOCL1XAtomic(IRBuilder<> &llvmBuilder,
      llvm::Instruction* Inst);
}

// Pass for lowering OCL 2.0 atomics
namespace llvm {
class AMDLowerAtomics : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  AMDLowerAtomics() : ModulePass(ID) {
      initializeAMDLowerAtomicsPass(*PassRegistry::getPassRegistry());
  }
  virtual bool runOnModule(Module &M);
private:
  Module *mod;
  void setModule(Module *M);
  Value * lowerAtomic(StringRef name, CallInst *instr);
  Value * lowerAtomicLoad(IRBuilder<> llvmBuilder, CallInst *instr);
  Value * lowerAtomicStore(IRBuilder<> llvmBuilder, StringRef name,
                           CallInst *instr);
  Value * lowerAtomicCmpXchg(IRBuilder<> llvmBuilder, CallInst *instr);
  Value * lowerAtomicRMW(IRBuilder<> llvmBuilder, StringRef name,
                         CallInst *instr);
  Value * lowerAtomicInit(IRBuilder<> llvmBuilder, CallInst *instr);
};
}

INITIALIZE_PASS(AMDLowerAtomics, "amd-lower-opencl-atomic-builtins",
                "Convert OpenCL atomic intrinsic calls into LLVM IR instructions ",
                false, false);

char AMDLowerAtomics::ID = 0;

namespace llvm {
ModulePass *createAMDLowerAtomicsPass() {
  return new AMDLowerAtomics();
}
}


void AMDLowerAtomics::setModule(Module *M) {
  mod = M;
}

static bool isOCLAtomicLoad(StringRef funcName) {
  if(!(funcName.startswith("atomic_load")))
    return false;
  return true;
}

static bool isOCLAtomicStore(StringRef funcName) {
  if(!(funcName.startswith("atomic_store")))
    return false;
  return true;
}

static bool isOCLAtomicCmpXchg(StringRef funcName) {
  if(!(funcName.startswith("atomic_compare_exchange_strong") ||
       funcName.startswith("atomic_compare_exchange_weak")))
    return false;
  return true;
}

static bool isOCLAtomicRMW(StringRef funcName) {
  if(!(funcName.startswith("atomic_fetch_add") ||
       funcName.startswith("atomic_fetch_sub") ||
       funcName.startswith("atomic_fetch_or") ||
       funcName.startswith("atomic_fetch_xor") ||
       funcName.startswith("atomic_fetch_and") ||
       funcName.startswith("atomic_fetch_min") ||
       funcName.startswith("atomic_fetch_max") ||
       funcName.startswith("atomic_exchange")))
    return false;
  return true;
}

static bool isOCLAtomicTestAndSet(StringRef funcName) {
  if(!(funcName.startswith("atomic_flag_test_and_set")))
    return false;
  return true;
}

static bool isOCLAtomicFlagClear(StringRef funcName) {
  if(!(funcName.startswith("atomic_flag_clear")))
    return false;
  return true;
}

static bool isOCLAtomicInit(StringRef funcName) {
  if(!(funcName.startswith("atomic_init")))
    return false;
  return true;
}

static AtomicOrdering MemoryOrderSpir2LLVM(Value * spirMemOrd) {
  typedef enum memory_order {
    memory_order_relaxed = 0,
    memory_order_acquire,
    memory_order_release,
    memory_order_acq_rel,
    memory_order_seq_cst
  } memory_order;
  unsigned memOrd = dyn_cast<ConstantInt>(spirMemOrd)->getZExtValue();
  switch(memOrd) {
    case memory_order_relaxed: return Monotonic;
    case memory_order_acquire: return Acquire;
    case memory_order_release: return Release;
    case memory_order_acq_rel: return AcquireRelease;
    case memory_order_seq_cst: return SequentiallyConsistent;
    default: return NotAtomic;
  }
}

enum MemoryScope {
  MEM_SCOPE_NONE = 0,
  MEM_SCOPE_WAVEFRONT,
  MEM_SCOPE_WORKGROUP,
  MEM_SCOPE_COMPONENT,
  MEM_SCOPE_SYSTEM
};

static unsigned MemoryScopeOpenCL2LLVM(Value *openclMemScope) {
  enum memory_scope {
   memory_scope_work_item = 0,
   memory_scope_work_group,
   memory_scope_device,
   memory_scope_all_svm_devices,
  };
  unsigned memScope = dyn_cast<ConstantInt>(openclMemScope)->getZExtValue();
  switch(memScope) {
    case memory_scope_work_item:
        llvm_unreachable("memory_scope_work_item not valid for atomic builtins");
    case memory_scope_work_group: return MEM_SCOPE_WORKGROUP;
    case memory_scope_device: return MEM_SCOPE_COMPONENT;
    case memory_scope_all_svm_devices: return MEM_SCOPE_SYSTEM;
    default: llvm_unreachable("unknown memory scope");
  }
}

static unsigned getDefaultMemScope(Value *ptr) {
  unsigned addrSpace = dyn_cast<PointerType>(ptr->getType())->getAddressSpace();
  // for atomics on local pointers, memory scope is wg
  if(addrSpace == 3) return MEM_SCOPE_WORKGROUP;
  return MEM_SCOPE_COMPONENT;
}

static AtomicOrdering getMemoryOrder(CallInst *inst, unsigned memOrderPos) {
  return inst->getNumArgOperands() > memOrderPos ?
         MemoryOrderSpir2LLVM(inst->getArgOperand(memOrderPos)):
         SequentiallyConsistent;
}

static unsigned getMemoryScope(CallInst *inst, unsigned memScopePos) {
  return inst->getNumArgOperands() > memScopePos ?
         MemoryScopeOpenCL2LLVM(inst->getArgOperand(memScopePos)):
         getDefaultMemScope(inst->getOperand(0));
}

Value * AMDLowerAtomics::lowerAtomic(StringRef name, CallInst *instr) {
  IRBuilder<> llvmBuilder(mod->getContext());
  llvmBuilder.SetInsertPoint(instr);

  int status = 0;
  const char *result = abi::__cxa_demangle(name.data(), 0, 0, &status);
  if(result == NULL) result = name.data();
  StringRef demangledName(result);

  if(isOCLAtomicLoad(demangledName))
    return lowerAtomicLoad(llvmBuilder, instr);
  if(isOCLAtomicStore(demangledName) || isOCLAtomicFlagClear(demangledName))
    return lowerAtomicStore(llvmBuilder, demangledName, instr);
  if(isOCLAtomicCmpXchg(demangledName))
    return lowerAtomicCmpXchg(llvmBuilder, instr);
  if(isOCLAtomicRMW(demangledName) || isOCLAtomicTestAndSet(demangledName))
    return lowerAtomicRMW(llvmBuilder, name, instr);
  if(isOCLAtomicInit(demangledName))
    return lowerAtomicInit(llvmBuilder, instr);
  return AMDOCL1XAtomic::LowerOCL1XAtomic(llvmBuilder, instr);
}

static void setMemScopeMD(Value *instr, unsigned memScope) {
  LLVMContext& llvmContext =
      cast<Instruction>(instr)->getParent()->getParent()->getParent()->getContext();
  unsigned memScopeMDKind = llvmContext.getMDKindID("mem.scope");
  Value* mdnodeOpnds[1];
  mdnodeOpnds[0] = ConstantInt::get(IntegerType::get(llvmContext, 32), memScope);
  MDNode *memScopeMD = MDNode::get(llvmContext, mdnodeOpnds);
  dyn_cast<Instruction>(instr)->setMetadata(memScopeMDKind, memScopeMD);
}

Value * AMDLowerAtomics::lowerAtomicLoad(IRBuilder<> llvmBuilder,
                                         CallInst *inst) {
  Value *ptr = inst->getArgOperand(0);
  AtomicOrdering memOrd = getMemoryOrder(inst, 1);
  unsigned memScope = getMemoryScope(inst, 2);
  Type *ptrType = ptr->getType();
  Type *valType = ptr->getType()->getPointerElementType();
  if(valType->isFloatTy() || valType->isDoubleTy()) {
    unsigned addrSpace = dyn_cast<PointerType>(ptrType)->getAddressSpace();
    Type *intType = IntegerType::get(mod->getContext(),
                    valType->getPrimitiveSizeInBits());
    Type *intPtrType = PointerType::get(intType, addrSpace);
    ptr = llvmBuilder.CreateCast(Instruction::BitCast, ptr, intPtrType);
  }
  Value *ldInst = llvmBuilder.CreateLoad(ptr, true);
  dyn_cast<LoadInst>(ldInst)->setOrdering(memOrd);
  setMemScopeMD(ldInst, memScope);
  dyn_cast<LoadInst>(ldInst)->setAlignment(valType->getPrimitiveSizeInBits() / 8);
  if(valType->isFloatTy() || valType->isDoubleTy()) {
    ldInst = llvmBuilder.CreateCast(Instruction::BitCast, ldInst, valType);
  }
  return ldInst;
}

Value * AMDLowerAtomics::lowerAtomicStore(IRBuilder<> llvmBuilder,
                                          StringRef funcName,
                                          CallInst *inst) {
  bool isAtomicClear = funcName.startswith("atomic_flag_clear") ?
                       true : false;
  Value *ptr = inst->getArgOperand(0);
  Value *val = isAtomicClear?
               ConstantInt::get(IntegerType::get(mod->getContext(), 32), 0) :
               inst->getArgOperand(1);
  AtomicOrdering memOrd = isAtomicClear? getMemoryOrder(inst, 1) :
                          getMemoryOrder(inst, 2);
  unsigned memScope = isAtomicClear? getMemoryScope(inst, 2) :
                      getMemoryScope(inst, 3);
  Type *valType = val->getType();
  if(valType->isFloatTy() || valType->isDoubleTy()) {
    unsigned addrSpace = dyn_cast<PointerType>(ptr->getType())->getAddressSpace();
    Type *intType = IntegerType::get(mod->getContext(),
                                     valType->getPrimitiveSizeInBits());
    Type *intPtrType = PointerType::get(intType, addrSpace);
    ptr = llvmBuilder.CreateCast(Instruction::BitCast, ptr, intPtrType);
    val = llvmBuilder.CreateCast(Instruction::BitCast, val, intType);
  }
  Value *stInst = llvmBuilder.CreateStore(val, ptr, true);
  dyn_cast<StoreInst>(stInst)->setOrdering(memOrd);
  setMemScopeMD(stInst, memScope);
  dyn_cast<StoreInst>(stInst)->setAlignment(valType->getPrimitiveSizeInBits() / 8);
  return stInst;
}

Value * AMDLowerAtomics::lowerAtomicCmpXchg(IRBuilder<> llvmBuilder,
                                            CallInst *inst) {
  LLVMContext& llvmContext = mod->getContext();
  Value *ptr = inst->getArgOperand(0);
  Value *expected = inst->getArgOperand(1);
  Value *desired = inst->getArgOperand(2);
  AtomicOrdering memOrdSuccess = getMemoryOrder(inst, 3);
  AtomicOrdering memOrdFailure = getMemoryOrder(inst, 4);
  unsigned memScope = getMemoryScope(inst, 5);
  Value *orig_expected = llvmBuilder.CreateLoad(expected, true);
  Value *cas = llvmBuilder.CreateAtomicCmpXchg(ptr, orig_expected, desired,
                                               memOrdSuccess, CrossThread);
  setMemScopeMD(cas, memScope);
  Value *store = llvmBuilder.CreateStore(cas, expected, true);
  Type *valType = expected->getType();
  Value *cmp = NULL;
  if(valType->isFloatTy() || valType->isDoubleTy())
    cmp = llvmBuilder.CreateFCmp(FCmpInst::FCMP_OEQ, cas, orig_expected);
  else
    cmp = llvmBuilder.CreateICmp(ICmpInst::ICMP_EQ, cas, orig_expected);
  cmp = llvmBuilder.CreateCast(Instruction::BitCast, cmp,
                               IntegerType::get(llvmContext, 1));
  Value *select = llvmBuilder.CreateSelect(cmp,
                          ConstantInt::get(IntegerType::get(llvmContext, 1), 1) ,
                          ConstantInt::get(IntegerType::get(llvmContext, 1), 0));
  return select;
}

static AtomicRMWInst::BinOp atomicFetchBinOp(StringRef name, bool isSigned) {
  const char *funcName = name.data();
  if(strstr(funcName,"atomic_fetch_add")) return AtomicRMWInst::Add;
  if(strstr(funcName,"atomic_fetch_sub")) return AtomicRMWInst::Sub;
  if(strstr(funcName,"atomic_fetch_and")) return AtomicRMWInst::And;
  if(strstr(funcName,"atomic_fetch_or")) return AtomicRMWInst::Or;
  if(strstr(funcName,"atomic_fetch_xor")) return AtomicRMWInst::Xor;
  if(strstr(funcName,"atomic_fetch_max")) return isSigned? AtomicRMWInst::Max
                                     : AtomicRMWInst::UMax;
  if(strstr(funcName,"atomic_fetch_min")) return isSigned? AtomicRMWInst::Min
                                     : AtomicRMWInst::UMin;
  if(strstr(funcName,"atomic_exchange") || strstr(funcName,"atomic_flag_test_and_set"))
    return AtomicRMWInst::Xchg;
  assert(0 && "internal error");
  return AtomicRMWInst::BAD_BINOP;
}

Value * AMDLowerAtomics::lowerAtomicRMW(IRBuilder<> llvmBuilder,
                                        StringRef funcName,
                                        CallInst *inst) {
  LLVMContext& llvmContext = mod->getContext();
  bool testAndSet = strstr(funcName.data(), "atomic_flag_test_and_set") ?
                    true : false;
  Value *ptr = inst->getArgOperand(0);
  Value *val = testAndSet ?
               ConstantInt::get(IntegerType::get(llvmContext,32), 1) :
               inst->getArgOperand(1);
  AtomicOrdering memOrd = testAndSet? getMemoryOrder(inst, 1) :
                          getMemoryOrder(inst, 2);
  unsigned memScope = testAndSet? getMemoryScope(inst, 2) :
                      getMemoryScope(inst, 3);
  Type *ptrType = ptr->getType();
  Type *valType = val->getType();
  if(valType->isFloatTy() || valType->isDoubleTy()) {
    unsigned addrSpace = dyn_cast<PointerType>(ptrType)->getAddressSpace();
    Type *intType = IntegerType::get(llvmContext,
                                     valType->getPrimitiveSizeInBits());
    Type *intPtrType = PointerType::get(intType, addrSpace);
    ptr = llvmBuilder.CreateCast(Instruction::BitCast, ptr, intPtrType);
    val = llvmBuilder.CreateCast(Instruction::BitCast, val, intType);
  }
  bool isSigned = true;
  // check if the arguments are unsigned
  if(funcName.endswith("jj") || funcName.endswith("S_S_"))
    isSigned = false;
  AtomicRMWInst::BinOp BinOp = atomicFetchBinOp(funcName, isSigned);
  Value *atomicRMW = llvmBuilder.CreateAtomicRMW(BinOp, ptr, val,
                                                 memOrd, CrossThread);
  setMemScopeMD(atomicRMW, memScope);
  if(valType->isFloatTy() || valType->isDoubleTy()) {
    atomicRMW = llvmBuilder.CreateCast(Instruction::BitCast,
                                       atomicRMW, valType);
  }
  if(testAndSet) {
    atomicRMW = llvmBuilder.CreateCast(Instruction::Trunc,
                                       atomicRMW, IntegerType::get(llvmContext, 1));
  }
  return atomicRMW;
}


Value * AMDLowerAtomics::lowerAtomicInit(IRBuilder<> llvmBuilder,
                                         CallInst *inst) {
  LLVMContext& llvmContext = mod->getContext();
  Value *ptr = inst->getArgOperand(0);
  Value *val = inst->getArgOperand(1);
  Value *stInst = llvmBuilder.CreateStore(val, ptr, true);
  return stInst;
}

static bool donotLowerAtomics(Module& M) {
  llvm::Triple triple(M.getTargetTriple());
  return triple.getArch() == llvm::Triple::amdil ||
      triple.getArch() == llvm::Triple::amdil64;
}

static bool targetRequiresScope(Module& M) {
  llvm::Triple triple(M.getTargetTriple());
  return triple.getArch() == llvm::Triple::hsail ||
      triple.getArch() == llvm::Triple::hsail_64 ||
      triple.getArch() == llvm::Triple::spir ||
      triple.getArch() == llvm::Triple::spir64;
}

bool AMDLowerAtomics::runOnModule(Module& M) {
  if (donotLowerAtomics(M))
    return false;

  setModule(&M);
  bool Changed = false;
  for(Module::iterator MF = M.begin(), E = M.end(); MF != E; ++MF) {
    for(Function::iterator BB = MF->begin(), MFE = MF->end(); BB != MFE; ++BB) {
      for(BasicBlock::iterator instr = BB->begin(), instr_end = BB->end();
          instr != instr_end; ) {
        CallInst *CI = dyn_cast<CallInst>(instr);
        instr++;
        if(!(CI && CI->getCalledFunction()
              && CI->getCalledFunction()->hasName())) {
          continue;
        }
        Value *newAtomicInstr = lowerAtomic(CI->getCalledFunction()->getName(), CI);
        if (newAtomicInstr) {
          CI->replaceAllUsesWith(newAtomicInstr);
          CI->eraseFromParent();
          Changed = true;
        }
      }
    }
  }
  return Changed;
}

// Functions for lowering OCL 1.x atomics
namespace AMDOCL1XAtomic{
using namespace llvm;

enum InstType {
  RMW,
  CMPXCHG,
  BAD
};
struct Entry {
  const char* name;
  InstType type;
  AtomicRMWInst::BinOp op;
  unsigned nop;
};

static Entry table[] = {
    {"add", RMW, AtomicRMWInst::Add, 2},
    {"sub", RMW, AtomicRMWInst::Sub, 2},
    {"xchg", RMW, AtomicRMWInst::Xchg, 2},
    {"inc", RMW, AtomicRMWInst::Add, 1},
    {"dec", RMW, AtomicRMWInst::Sub, 1},
    {"min", RMW, AtomicRMWInst::Min, 2},
    {"max", RMW, AtomicRMWInst::Max, 2},
    {"and", RMW, AtomicRMWInst::And, 2},
    {"or", RMW, AtomicRMWInst::Or, 2},
    {"xor", RMW, AtomicRMWInst::Xor, 2},
    {"cmpxchg", CMPXCHG, AtomicRMWInst::BAD_BINOP, 3}
};

bool parseSPIRAtomic(StringRef N, InstType& TP, AtomicRMWInst::BinOp &OP,
    unsigned& NOP) {
  DEBUG(dbgs() << "[parseSPIRAtomic] " << N << '\n');
  int status = 0;
  const char *result = abi::__cxa_demangle(N.data(), 0, 0, &status);
  if (result == NULL) return false;
  StringRef demangledName(result);
  if (demangledName.startswith("atomic")) {
    demangledName = demangledName.drop_front(7);
  } else if (demangledName.startswith("atom")) {
    demangledName = demangledName.drop_front(5);
  } else {
    return false;
  }
  DEBUG(dbgs() << "[parseSPIRAtomic] " << demangledName << '\n');

  int i;
  int n = sizeof(table)/sizeof(table[0]);
  for (i = 0; i < n; ++i) {
    if (demangledName.startswith(table[i].name)) {
      break;
    }
  }

  if (i == n) return false;

  demangledName = demangledName.drop_front(strlen(table[i].name));
  DEBUG(dbgs() << "[parseSPIRAtomic] " << demangledName << '\n');
  if (!demangledName.startswith("(")) return false;

  OP = table[i].op;
  TP = table[i].type;
  NOP = table[i].nop;
  DEBUG(dbgs() << "[parseSPIRAtomic] " << "type: " << TP << " op: " <<
      OP << " nop: " << NOP << '\n');
  return true;
}

bool parseEDGAtomic(StringRef N, InstType& TP, AtomicRMWInst::BinOp &OP,
    unsigned& NOP) {
  DEBUG(dbgs() << "[parseEDGAtomic] " << N << '\n');
  if (N.startswith("__atomic")) {
    N = N.drop_front(9);
  } else if (N.startswith("__atom")) {
    N = N.drop_front(7);
  } else {
    return false;
  }
  DEBUG(dbgs() << "[parseEDGAtomic] " << N << '\n');

  int i;
  int n = sizeof(table)/sizeof(table[0]);
  for (i = 0; i < n; ++i) {
    if (N.startswith(table[i].name)) {
      break;
    }
  }

  if (i == n) return false;

  N = N.drop_front(strlen(table[i].name)+1);
  DEBUG(dbgs() << "[parseEDGAtomic] " << N << '\n');
  if (N.front() != 'g' && N.front() != 'l') return false;

  N = N.drop_front(1);
  DEBUG(dbgs() << "[parseEDGAtomic] " << N << '\n');
  if (N.front() != 'u' && N.front() != 'i' &&
      !(N.front() == 'f' && table[i].op == AtomicRMWInst::Xchg)) return false;

  N = N.drop_front(1);
  DEBUG(dbgs() << "[parseEDGAtomic] " << N << '\n');
  if (!N.startswith("64") && !N.startswith("32")) return false;

  N = N.drop_front(2);
  DEBUG(dbgs() << "[parseEDGAtomic] " << N << '\n');
  if (!N.empty()) return false;

  OP = table[i].op;
  TP = table[i].type;
  NOP = table[i].nop;
  DEBUG(dbgs() << "[parseEDGAtomic] " << "type: " << TP << " op: " <<
      OP << " nop: " << NOP << '\n');
  return true;
}

static bool needLowerOCL1xAtomics(Module& M) {
  llvm::Triple triple(M.getTargetTriple());
  return triple.getArch() == llvm::Triple::x86 ||
      triple.getArch() == llvm::Triple::x86_64 ||
      triple.getArch() == llvm::Triple::hsail ||
      triple.getArch() == llvm::Triple::hsail_64 ||
      triple.getArch() == llvm::Triple::spir ||
      triple.getArch() == llvm::Triple::spir64;
}

Value* LowerOCL1XAtomic(IRBuilder<> &Builder, Instruction* Inst) {
  Module &M = *Inst->getParent()->getParent()->getParent();
  if (!needLowerOCL1xAtomics(*Inst->getParent()->getParent()->getParent()))
    return NULL;

  CallSite CS(Inst);
  Function* F = CS.getCalledFunction();
  if (!F->hasName()) {
    return NULL;
  }

  InstType type = BAD;
  AtomicRMWInst::BinOp op = AtomicRMWInst::BAD_BINOP;
  unsigned numOp = 0;

  if(!parseEDGAtomic(F->getName(), type, op, numOp)){
    if(!parseSPIRAtomic(F->getName(), type, op, numOp)) {
      return NULL;
    }
  }
  assert(CS.arg_size() == numOp && "Incorrect number of arguments");

  llvm::Value *P = CS.getArgument(0);
  llvm::Value *NI = NULL;
  AtomicOrdering order = AtomicOrdering(OCL1XAtomicOrder.getValue());
  if (type == RMW) {
    llvm::Value *V =
        numOp == 2 ?
            CS.getArgument(1) :
            ConstantInt::get(P->getType()->getPointerElementType(), 1);
    bool needCast = !V->getType()->isIntegerTy();
    if (needCast) {
      assert(op == AtomicRMWInst::Xchg && "Invalid atomic instruction");
      LLVMContext &context = Inst->getParent()->getContext();
      V = Builder.CreateBitCast(V, Type::getInt32Ty(context));
      P = Builder.CreateBitCast(P,
          Type::getInt32PtrTy(context, P->getType()->getPointerAddressSpace()));
    }
    NI = Builder.CreateAtomicRMW(op, P, V, order, CrossThread);
    if(targetRequiresScope(*Inst->getParent()->getParent()->getParent()))
      setMemScopeMD(NI, MEM_SCOPE_WORKGROUP);
    if (needCast) {
      NI = Builder.CreateBitCast(NI, Inst->getType());
    }
  } else if (type == CMPXCHG) {
    NI = Builder.CreateAtomicCmpXchg(P, CS.getArgument(1), CS.getArgument(2),
        order, CrossThread);
    if(targetRequiresScope(*Inst->getParent()->getParent()->getParent()))
      setMemScopeMD(NI, MEM_SCOPE_WORKGROUP);
  } else {
    llvm_unreachable("Invalid atomic builtin");
  }

  DEBUG(dbgs() << *Inst << " => " << *NI << '\n');
  return NI;
}
}
