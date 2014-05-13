//===--- AMDSPIRMutator.cpp - helper objects that mutate SPIR types -------===//
//===----------------------------------------------------------------------===//
// Mutate spir data types into device specific LLVM datatypes.
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "spirloader"
#include "AMDSPIRMutator.h"
#include "AMDSPIRLoader.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/GlobalValue.h"
#include "llvm/InstrTypes.h"
#include "llvm/Instructions.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include <set>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <functional>
#include "cxxabi.h"
#include <stdio.h>
using namespace llvm;
using namespace spir;
namespace spir {
  // The name of the functions we want to detect and convert
  // into normal IR instructions.
static const char *functionNames[] =
  {
    "__spir_sizet_convert_size_t",  "__spir_sizet_convert_i1",
    "__spir_sizet_convert_i8",      "__spir_sizet_convert_i16",
    "__spir_sizet_convert_i32",     "__spir_sizet_convert_i64",
    "__spir_sizet_convert_half",    "__spir_sizet_convert_float",
    "__spir_sizet_convert_double",  "__spir_sizet_convert_ptr0",
    "__spir_sizet_convert_ptr1",    "__spir_sizet_convert_ptr2",
    "__spir_sizet_convert_ptr3",    "__spir_sizet_convert_ptr4",
    "__spir_sizet_convert_ptr5",    "__spir_sizet_add",
    "__spir_sizet_sub",             "__spir_sizet_mul",
    "__spir_sizet_udiv",            "__spir_sizet_sdiv",
    "__spir_sizet_urem",            "__spir_sizet_srem",
    "__spir_sizet_or",              "__spir_sizet_and",
    "__spir_sizet_xor",             "__spir_sizet_not",
    "__spir_sizet_shl",             "__spir_sizet_lshr",
    "__spir_sizet_ashr",            "__spir_sizet_cmp",
    "__spir_sizet_add_ptr0",        "__spir_sizet_add_ptr1",
    "__spir_sizet_add_ptr2",        "__spir_sizet_add_ptr3",
    "__spir_sizet_add_ptr4",        "__spir_sizet_add_ptr5",
    "__spir_sizet_alloca",          "__spir_size_of_sizet",
    "__spir_size_of_pointer",       "__spir_sampler_initialize",
    "__spir_globals_initializer",   "__spir_eventt_null",
    "__spir_get_null_ptr0",         "__spir_get_null_ptr1",
    "__spir_get_null_ptr2",         "__spir_get_null_ptr3",
    "__spir_get_null_ptr4",         "__spir_get_null_ptr5",
  }; // end function names table.

  typedef enum _ctl_state_enum {
    ctl_Finish = 0, // Finish the processing for this item.
    ctl_Jump = 1, // Jump to the next location in the table.
    ctl_Rename = 2, // Just rename the function to the string in the index of the next integer.
    ctl_ReplaceLit = 3, // Replace the function with a literal.
    ctl_Assert = 4, // Entered an invalid state, assert.
    ctl_RetType = 5, // Match the return type, if the next token matches the return type(consumes one or two tokens), jump the next token number of tokens, else jump to the next token after.
    ctl_Type = 6, // Match the type for argument N, if the next token matches the next type, jump the third token number of tokens, else jump to the fourth token.
    ctl_Inst = 7, // The target instruction.
    ctl_TargetType = 8, // Only execute if the target type matches.
    ctl_Copy = 9, // Copy the source to destination.
    ctl_Check = 10, // Check that the call matches the function indicated by the next token, otherwise jump over the number of tokens indicated by the third token.
    ctl_SizeT = 11, // Check that the current token is size_t.
    ctl_Mutate = 12, // Mutate the instruction
    ctl_CheckBegin = 13, // Only the check the beginning of the instruction.
    ctl_Last
  } ctl_state;

  // name of the states when emitting debug information.
  static const char* stateNames[ctl_Last + 1] = {
    "ctl_Finish",     "ctl_Jump",       "ctl_Rename", "ctl_ReplaceLit",
    "ctl_Assert",     "ctl_RetType",    "ctl_Type",   "ctl_Inst",
    "ctl_TargetType", "ctl_Copy",       "ctl_Check",  "ctl_SizeT",
    "ctl_Mutate",     "ctl_CheckBegin", "ctl_Last"
  };

  // The format of this table is that the first N entries, where N is the
  // number of names in the function names table. Each entry at this location
  // points to the offset in the callTransformLookup table for how to
  // transform the spir function into its appropriate function call.
  // The comment at the beginning of the line indicate the offset into the table.
  // The comment at the end of the line indicate the number of tokens in that line.
  // So as a sanity check, the comment at the beginning + the comment at the end should equal to
  // the comment on the next line.
  static uint32_t callTransformLookup[] =
  {
      /*  0 */  ctl_CheckBegin, 0,  91, ctl_Mutate,   /*  4 */
      /*  4 */  ctl_RetType,    Type::IntegerTyID,  1, 3, ctl_Jump, 4,   /*  6 */
      /*  10 */  ctl_Inst,       Instruction::ZExt,  ctl_Finish,   /*  3 */
      /*  13 */  ctl_RetType,    Type::IntegerTyID,  8, 3, ctl_Jump, 4,   /*  6 */
      /*  19 */  ctl_Inst,       Instruction::ZExt,  ctl_Finish,   /*  3 */
      /*  22 */  ctl_RetType,    Type::IntegerTyID, 16, 3, ctl_Jump, 4,   /*  6 */
      /*  28 */  ctl_Inst,       Instruction::ZExt,  ctl_Finish,   /*  3 */
      /*  31 */  ctl_TargetType, Type::IntegerTyID, 32, 3, ctl_Jump, 10,   /*  6 */
      /*  37 */  ctl_Type, 0,    Type::IntegerTyID, 32, 3, ctl_Jump, 3,   /*  7 */
      /*  44 */  ctl_Copy,       ctl_Finish,   /*  2 */
      /*  46 */  ctl_TargetType, Type::IntegerTyID, 64, 3, ctl_Jump, 11,   /*  6 */
      /*  52 */  ctl_Type, 0,    Type::IntegerTyID, 32, 3, ctl_Jump, 4,   /*  7 */
      /*  59 */  ctl_Inst,       Instruction::ZExt,  ctl_Finish,   /*  3 */
      /*  62 */  ctl_TargetType, Type::IntegerTyID, 32, 3, ctl_Jump, 11,   /*  6 */
      /*  68 */  ctl_Type, 0,    Type::IntegerTyID, 64, 3, ctl_Jump, 4,   /*  7 */
      /*  75 */  ctl_Inst,       Instruction::Trunc, ctl_Finish,   /*  3 */
      /*  78 */  ctl_TargetType, Type::IntegerTyID, 64, 3, ctl_Jump, 10,   /*  6 */
      /*  84 */  ctl_Type, 0,    Type::IntegerTyID, 64, 3, ctl_Jump, 3,   /*  7 */
      /*  91 */  ctl_Copy,       ctl_Finish,   /*  2 */
      /*  93 */  ctl_Check,  1,  5,   /*  3 */
      /*  96 */  ctl_Mutate, ctl_Inst, Instruction::Trunc,  ctl_Finish,   /*  4 */
      /*  100 */  ctl_Check,  2,  5,   /*  3 */
      /*  103 */  ctl_Mutate, ctl_Inst, Instruction::Trunc,  ctl_Finish,   /*  4 */
      /*  107 */  ctl_Check,  3,  5,   /*  3 */
      /*  110 */  ctl_Mutate, ctl_Inst, Instruction::Trunc,  ctl_Finish,   /*  4 */
      /*  114 */  ctl_Check,  4, 20,   /*  3 */
      /*  117 */  ctl_TargetType, Type::IntegerTyID, 32, 3, ctl_Jump, 4,   /*  6 */
      /*  123 */  ctl_Mutate, ctl_Copy,  ctl_Finish,   /*  3 */
      /*  126 */  ctl_TargetType, Type::IntegerTyID, 64, 3, ctl_Jump, 5,   /*  6 */
      /*  132 */  ctl_Mutate, ctl_Inst, Instruction::ZExt,  ctl_Finish,   /*  4 */
      /*  136 */  ctl_Check,  5, 20,   /*  3 */
      /*  139 */  ctl_TargetType, Type::IntegerTyID, 32, 3, ctl_Jump, 5,   /*  6 */
      /*  145 */  ctl_Mutate, ctl_Inst, Instruction::Trunc, ctl_Finish,   /*  4 */
      /*  149 */  ctl_TargetType, Type::IntegerTyID, 64, 3, ctl_Jump, 4,   /*  6 */
      /*  155 */  ctl_Mutate, ctl_Copy,  ctl_Finish,   /*  3 */
      /*  158 */  ctl_Check,  6,  5,   /*  3 */
      /*  161 */  ctl_Mutate, ctl_Inst, Instruction::UIToFP,   ctl_Finish,   /*  4 */
      /*  165 */  ctl_Check,  7,  5,   /*  3 */
      /*  168 */  ctl_Mutate, ctl_Inst, Instruction::UIToFP,   ctl_Finish,   /*  4 */
      /*  172 */  ctl_Check,  8,  5,   /*  3 */
      /*  175 */  ctl_Mutate, ctl_Inst, Instruction::UIToFP,   ctl_Finish,   /*  4 */
      /*  179 */  ctl_Check,  9,  5,   /*  3 */
      /*  182 */  ctl_Mutate, ctl_Inst, Instruction::IntToPtr, ctl_Finish,   /*  4 */
      /*  186 */  ctl_Check, 10,  5,   /*  3 */
      /*  189 */  ctl_Mutate, ctl_Inst, Instruction::IntToPtr, ctl_Finish,   /*  4 */
      /*  193 */  ctl_Check, 11,  5,   /*  3 */
      /*  196 */  ctl_Mutate, ctl_Inst, Instruction::IntToPtr, ctl_Finish,   /*  4 */
      /*  200 */  ctl_Check, 12,  5,   /*  3 */
      /*  203 */  ctl_Mutate, ctl_Inst, Instruction::IntToPtr, ctl_Finish,   /*  4 */
      /*  207 */  ctl_Check, 13,  5,   /*  3 */
      /*  210 */  ctl_Mutate, ctl_Inst, Instruction::IntToPtr, ctl_Finish,   /*  4 */
      /*  214 */  ctl_Check, 14,  5,   /*  3 */
      /*  217 */  ctl_Mutate, ctl_Inst, Instruction::IntToPtr, ctl_Finish,   /*  4 */
      /*  221 */  ctl_Check, 15,  5,   /*  3 */
      /*  224 */  ctl_Mutate, ctl_Inst, Instruction::Add, ctl_Finish,   /*  4 */
      /*  228 */  ctl_Check, 16,  5,   /*  3 */
      /*  231 */  ctl_Mutate, ctl_Inst, Instruction::Sub, ctl_Finish,   /*  4 */
      /*  235 */  ctl_Check, 17,  5,   /*  3 */
      /*  238 */  ctl_Mutate, ctl_Inst, Instruction::Mul, ctl_Finish,   /*  4 */
      /*  242 */  ctl_Check, 18,  5,   /*  3 */
      /*  245 */  ctl_Mutate, ctl_Inst, Instruction::UDiv, ctl_Finish,   /*  4 */
      /*  249 */  ctl_Check, 19,  5,   /*  3 */
      /*  252 */  ctl_Mutate, ctl_Inst, Instruction::SDiv, ctl_Finish,   /*  4 */
      /*  256 */  ctl_Check, 20,  5,   /*  3 */
      /*  259 */  ctl_Mutate, ctl_Inst, Instruction::URem, ctl_Finish,   /*  4 */
      /*  263 */  ctl_Check, 21,  5,   /*  3 */
      /*  266 */  ctl_Mutate, ctl_Inst, Instruction::SRem, ctl_Finish,   /*  4 */
      /*  270 */  ctl_Check, 22,  5,   /*  3 */
      /*  273 */  ctl_Mutate, ctl_Inst, Instruction::Or, ctl_Finish,   /*  4 */
      /*  277 */  ctl_Check, 23,  5,   /*  3 */
      /*  280 */  ctl_Mutate, ctl_Inst, Instruction::And, ctl_Finish,   /*  4 */
      /*  284 */  ctl_Check, 24,  5,   /*  3 */
      /*  287 */  ctl_Mutate, ctl_Inst, Instruction::Xor, ctl_Finish,   /*  4 */
      /*  291 */  ctl_Check, 25,  5,   /*  3 */
      /*  294 */  ctl_Mutate, ctl_Inst, 0, ctl_Finish,   /*  4 */
      /*  298 */  ctl_Check, 26,  5,   /*  3 */
      /*  301 */  ctl_Mutate, ctl_Inst, Instruction::Shl, ctl_Finish,   /*  4 */
      /*  305 */  ctl_Check, 27,  5,   /*  3 */
      /*  308 */  ctl_Mutate, ctl_Inst, Instruction::LShr, ctl_Finish,   /*  4 */
      /*  312 */  ctl_Check, 28,  5,   /*  3 */
      /*  315 */  ctl_Mutate, ctl_Inst, Instruction::AShr, ctl_Finish,   /*  4 */
      /*  319 */  ctl_Check, 29,  5,   /*  3 */
      /*  322 */  ctl_Mutate, ctl_Inst, Instruction::ICmp, ctl_Finish,   /*  4 */
      /*  326 */  ctl_Check, 30,  5,   /*  3 */
      /*  329 */  ctl_Mutate, ctl_Inst, Instruction::GetElementPtr, ctl_Finish,   /*  4 */
      /*  333 */  ctl_Check, 31,  5,   /*  3 */
      /*  336 */  ctl_Mutate, ctl_Inst, Instruction::GetElementPtr, ctl_Finish,   /*  4 */
      /*  340 */  ctl_Check, 32,  5,   /*  3 */
      /*  343 */  ctl_Mutate, ctl_Inst, Instruction::GetElementPtr, ctl_Finish,   /*  4 */
      /*  347 */  ctl_Check, 33,  5,   /*  3 */
      /*  350 */  ctl_Mutate, ctl_Inst, Instruction::GetElementPtr, ctl_Finish,   /*  4 */
      /*  354 */  ctl_Check, 34,  5,   /*  3 */
      /*  357 */  ctl_Mutate, ctl_Inst, Instruction::GetElementPtr, ctl_Finish,   /*  4 */
      /*  361 */  ctl_Check, 35,  5,   /*  3 */
      /*  364 */  ctl_Mutate, ctl_Inst, Instruction::GetElementPtr, ctl_Finish,   /*  4 */
      /*  368 */  ctl_Check, 36,  5,   /*  3 */
      /*  371 */  ctl_Mutate, ctl_Inst, Instruction::Alloca, ctl_Finish,   /*  4 */
      /*  375 */  ctl_Check, 37, 15,   /*  3 */
      /*  378 */  ctl_TargetType, Type::IntegerTyID, 32, 3, ctl_ReplaceLit, 32, ctl_Finish,   /*  7 */
      /*  385 */  ctl_TargetType, Type::IntegerTyID, 64, 3, ctl_ReplaceLit, 64, ctl_Finish,   /*  7 */
      /*  392 */  ctl_Check, 38, 15,   /*  3 */
      /*  395 */  ctl_TargetType, Type::IntegerTyID, 32, 3, ctl_ReplaceLit, 32, ctl_Finish,   /*  7 */
      /*  402 */  ctl_TargetType, Type::IntegerTyID, 64, 3, ctl_ReplaceLit, 64, ctl_Finish,   /*  7 */
      /*  409 */  ctl_Check, 39,  4,   /*  3 */
      /*  412 */  ctl_Mutate, ctl_Copy, ctl_Finish,   /*  3 */
      /*  415 */  ctl_Check, 40,  4,   /*  3 */
      /*  418 */  ctl_Mutate, ctl_Copy, ctl_Finish,   /*  3 */
      /*  421 */  ctl_Check, 41,  5,   /*  3 */
      /*  424 */  ctl_Mutate, ctl_ReplaceLit, 0, ctl_Finish,   /*  4 */
      /*  428 */  ctl_Check, 42,  5,   /*  3 */
      /*  431 */  ctl_Mutate, ctl_ReplaceLit, 0, ctl_Finish,   /*  4 */
      /*  435 */  ctl_Check, 43,  5,   /*  3 */
      /*  438 */  ctl_Mutate, ctl_ReplaceLit, 0, ctl_Finish,   /*  4 */
      /*  442 */  ctl_Check, 44,  5,   /*  3 */
      /*  445 */  ctl_Mutate, ctl_ReplaceLit, 0, ctl_Finish,   /*  4 */
      /*  449 */  ctl_Check, 45,  5,   /*  3 */
      /*  452 */  ctl_Mutate, ctl_ReplaceLit, 0, ctl_Finish,   /*  4 */
      /*  456 */  ctl_Check, 46,  5,   /*  3 */
      /*  459 */  ctl_Mutate, ctl_ReplaceLit, 0, ctl_Finish,   /*  4 */
      /*  463 */  ctl_Check, 47,  5,   /*  3 */
      /*  466 */  ctl_Mutate, ctl_ReplaceLit, 0, ctl_Finish,   /*  4 */
      /*  470 */  ctl_Finish  /*  1 */
  }; // end call Lookup table.
}
///////////////////////////////////////////////////////////////////////////////
// Value Mutator functions
///////////////////////////////////////////////////////////////////////////////
ValueMutator::ValueMutator(Type **tbl)
  : type_table(tbl), mDebug(false)
{
}

ValueMutator::ValueMutator(const ValueMutator &tm)
{
  type_table = tm.type_table;
  mDebug = tm.mDebug;
}

void
ValueMutator::operator()(Value *V)
{
  mutate(V);
}

void
ValueMutator::operator()(Value &V)
{
  mutate(&V);
}
// If the current value is part of the type table as a key,
// mutate the type of the value to the corresponding IR type.
void 
ValueMutator::mutate(Value* V)
{
  uint32_t x = 0;
  if (!V) return;
  if (isSPIRType(type_table, V->getType(), x)) 
    V->mutateType(type_table[x + 1]);
  if (mDebug && type_table[x] && type_table[x + 1]) {
      type_table[x]->dump();
      dbgs() << " ==> ";
      type_table[x + 1]->dump();
      dbgs() << "\n";
  }
}

// Special case of the mutate function that deals with Call Instructions.
// This is required so that the mutatation of functions and signatures are
// handled correctly.
void 
ValueMutator::mutate(CallInst *CI)
{
  uint32_t x = 0;
  if (!CI) return;
  Function *oldFunc = CI->getCalledFunction();
  FunctionType *oldFT = oldFunc->getFunctionType();
  Type *ret = oldFunc->getReturnType();
  SmallVector<Type *, 10> args(oldFT->param_begin(), oldFT->param_end());
  // Lets mutate the return value.
  if (isSPIRType(type_table, ret, x)) 
    ret = type_table[x + 1];

  // Lets mutate all of the arguments
  for (SmallVector<Type *, 10>::iterator iter = args.begin() ;
                  iter != args.end();
                  ++iter)
  {
    x = 0;
    if (isSPIRType(type_table, *iter, x))
      *iter = type_table[x + 1];
  }
  // Todo: remove the following code between if/endif if no regressions are found
  // changing function name should not be done in call instruction
  // it should be done at function definition/declarition
#if 0
  // Create the new function type and function and replace it
  // in the call instruction. We append the '_spir' name on
  // all new functions since name mangling is not supported yet and
  // not doing so will cause a name collision with the one already 
  // in the file. These names get fixed up later in the loader pass.
  FunctionType *newFT = FunctionType::get(ret, args, oldFT->isVarArg());
  std::string newName = oldFunc->getName();
  // FIXME: This needs to be removed once the frontend/libraries
  // use the correct name mangling scheme.
  newName += std::string("_spir");
  Function *newFunc = CI->getParent()->getParent()
    ->getParent()->getFunction(newName);
  if (newFunc == NULL) {
    newFunc = Function::Create(newFT,
        oldFunc->getLinkage(), newName,
        oldFunc->getParent());
    newFunc->setCallingConv(oldFunc->getCallingConv());
  }
  if (mDebug) {
      oldFunc->dump();
      dbgs() << " ==> ";
      newFunc->dump();
      dbgs() << "\n";
  }
  mutate(dyn_cast<Value>(CI));
  CI->setCalledFunction(newFunc);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Type Mutator handles the mutation of various different types that
// derive from the value class(Instruction, Function, etc..)
///////////////////////////////////////////////////////////////////////////////
bool 
TypeMutator<Instruction>::typesMatch(
    LLVMContext &ctx, Type *type, uint32_t &curIdx)
{
  uint32_t cType = callTransformLookup[++curIdx];
  Type *expType = NULL;
  if (cType == Type::IntegerTyID) {
    expType = Type::getIntNTy(ctx, callTransformLookup[++curIdx]);
  } else if (cType == Type::FloatTyID) {
    expType = Type::getFloatTy(ctx);
  } else if (cType == Type::DoubleTyID) {
    expType = Type::getDoubleTy(ctx);
  } else if (cType == Type::StructTyID) {
    cType = callTransformLookup[++curIdx];
    if (cType == ctl_SizeT) {
      expType = type_table[0];
    }
  } /* else if (cType == Type::HalfTyID) { // Enable once LLVM 3.1 shows up.
       expType = Type::getHalfTy(ctx);
       } */
  if (mDebug) {
    dbgs() << " Types check ";
    type->dump();
    dbgs () << "(" << type << ") =? ";
    expType->dump();
    dbgs() << "(" << expType << ") ";
  }
  return type == expType;
}
  void 
TypeMutator<Instruction>::clear()
{
  for (std::set<Instruction*>::iterator bi = deadInsts.begin(),
      be = deadInsts.end(); bi != be; ++bi) {
    if (mDebug) {
      dbgs() << "Erasing: ";
      (*bi)->dump();
      dbgs() << "\n";
    }
    (*bi)->eraseFromParent();
  }
  deadInsts.clear();
}
// Replace the current instruction with the unary instruction if the
// types of the operation and the requested types match.
template <typename T>
static Value *replaceUnaryInst(Value *op, Type *type, CallInst *CI) {
  if (op->getType() != type) {
    CastInst *cast = new T(op, type, "spir_unary", CI);
    return cast;
  } else {
    return op;
  }
}

void 
TypeMutator<Instruction>::mutate(CallInst *CI) {
  if (!CI) return;
  Function* func = dyn_cast<Function>(CI->getCalledFunction());
  if (func == NULL)
    assert(!"calling NULL function");
  StringRef mangledName= func->getName();
  int status = 0;
  const char* tmpname = __cxxabiv1::__cxa_demangle(mangledName.str().c_str(), 0, 0, &status);
  StringRef name;
  std::string funcName;
  if (!status) {
    std::string unmangledName(tmpname);
    funcName = unmangledName.substr(0, unmangledName.find('('));
    name = funcName;
  } else {
    name = mangledName;
  }
  // FIXME: This needs to be changed once the frontend/libraries
  // use the correct name mangling scheme.
  if (!name.startswith("__spir")) {
    ValueMutator::mutate(CI);
    return;
  }
  if (mDebug) {
    CI->dump();
    dbgs() << "\n";
  }
  unsigned curIdx = 0;
  while (1) {
    uint32_t state = callTransformLookup[curIdx];
    if (mDebug) {
      char buffer[5];
      sprintf(buffer, "%05d", curIdx);
      dbgs() << buffer << ":[";
      dbgs() << "" << stateNames[state] << "]: ";
    }
    if (state == ctl_Assert) {
      CI->dump();
      assert(!"Found an invalid state!");
      return;
    } else if (state == ctl_Finish) {
      if (mDebug) {
        CI->dump();
        dbgs() << "\n";
      }
      break;
    } else if (state == ctl_Jump) {
      uint32_t jump = callTransformLookup[++curIdx];
      if (mDebug) {
        dbgs() << "jumping " << jump << " spots to " << curIdx + jump << "\n";
      }
      curIdx += jump;
    } else if (state == ctl_Rename) {
      assert(!"State not implemented yet!");
    } else if (state == ctl_CheckBegin) {
      if (mDebug) {
        dbgs() << "Name " << name << " and "
          << functionNames[callTransformLookup[curIdx + 1]];
      }
      if (!name.startswith(functionNames[callTransformLookup[++curIdx]])) {
        uint32_t jump = callTransformLookup[++curIdx];
        if (mDebug) {
          dbgs() << " does not match, jumping " << jump << " spots to " << curIdx + jump << "\n";
        }
        curIdx += jump;
        continue;
      }
      if (mDebug) {
        dbgs() << " matches, jumping to " << curIdx + 2<< ".\n";
      }
      curIdx += 2;
    } else if (state == ctl_Check) {
      if (mDebug) {
        dbgs() << "Name " << name << " and "
          << functionNames[callTransformLookup[curIdx + 1]];
      }
      if (name != StringRef(functionNames[callTransformLookup[++curIdx]])) {
        uint32_t jump = callTransformLookup[++curIdx];
        if (mDebug) {
          dbgs() << " does not match, jumping " << jump << " spots to " << curIdx + jump << "\n";
        }
        curIdx += jump;
        continue;
      }
      if (mDebug) {
        dbgs() << " matches, jumping to " << curIdx + 2<< ".\n";
      }
      curIdx += 2;
    } else if (state == ctl_ReplaceLit) {
      assert(!"State not implemented yet!");
      ++curIdx;
    } else if (state == ctl_RetType) {
      Type *retType = CI->getCalledFunction()->getReturnType();
      bool tf = typesMatch(CI->getContext(), retType, curIdx);
      uint32_t jump = (tf ? callTransformLookup[++curIdx] : 2);
      if (mDebug) {
        dbgs() << "jumping " << jump << " spots to " << curIdx + jump << "\n";
      }
      curIdx += jump;
    } else if (state == ctl_Type) {
      Type *argType = CI->getOperand(callTransformLookup[++curIdx])->getType();
      bool tf = typesMatch(CI->getContext(), argType, curIdx);
      uint32_t jump = (tf ? callTransformLookup[++curIdx] : 2);
      if (mDebug) {
        dbgs() << "jumping " << jump << " spots to " << curIdx + jump << "\n";
      }
      curIdx += jump;
    } else if (state == ctl_Inst) {
      uint32_t opcode = callTransformLookup[++curIdx];
      Value *op = CI->getArgOperand(0);
      Type *type = CI->getCalledFunction()->getReturnType();
      switch (opcode) {
        default:
          CI->dump();
          assert(!"Unknown opcode detected!");
        case Instruction::Trunc:
          op = replaceUnaryInst<TruncInst>(op, type, CI); break;
        case Instruction::ZExt:
          op = replaceUnaryInst<ZExtInst>(op, type, CI); break;
        case Instruction::IntToPtr:
          op = replaceUnaryInst<IntToPtrInst>(op, type, CI); break;
        case Instruction::UIToFP:
          op = replaceUnaryInst<UIToFPInst>(op, type, CI); break;
        case Instruction::Add:
        case Instruction::Sub:
        case Instruction::Mul:
        case Instruction::Shl:
        case Instruction::SDiv:
        case Instruction::SRem:
        case Instruction::UDiv:
        case Instruction::URem:
        case Instruction::AShr:
        case Instruction::LShr:
        case Instruction::And:
        case Instruction::Or:
        case Instruction::Xor:
          op = BinaryOperator::Create(
              (Instruction::BinaryOps)opcode, op,
              CI->getArgOperand(1), "spir_binary", CI);
          break;
        case 0:
          op = BinaryOperator::CreateNot(op, "spir_unary", CI);
          break;
        case Instruction::ICmp:
          {
            ConstantInt *cmp_code = dyn_cast<ConstantInt>(CI->getArgOperand(2));
            CmpInst *cmpOp = CmpInst::Create(Instruction::ICmp,
                cmp_code->getZExtValue(),
                CI->getOperand(0), CI->getOperand(1),
                "spir_compare", CI);
            op = replaceUnaryInst<SExtInst>(cmpOp, type, CI); break;
            op = cmpOp;
            break;
          }
        case Instruction::GetElementPtr:
          {
            SmallVector<Value*, 2> args;
            args.push_back(CI->getArgOperand(1));
            GetElementPtrInst *gepOp = GetElementPtrInst::Create(op,
                args, "spir_gep", CI);
            op = gepOp;
            break;
          }
        case Instruction::Alloca:
            assert(dyn_cast<PointerType>(type)
                && "Type must be a pointer type to work with alloca!");
            op = new AllocaInst(dyn_cast<PointerType>(type)->getElementType()
                , "spir_alloca", CI);
          break;
      }
      deadInsts.insert(CI);
      CI->replaceAllUsesWith(op);
      ++curIdx;
    } else if (state == ctl_TargetType) {
      bool tf = typesMatch(CI->getContext(), type_table[1], curIdx);
      uint32_t jump = (tf ? callTransformLookup[++curIdx] : 2);
      if (mDebug) {
        dbgs() << "Jumping " << jump << " spots to " << curIdx + jump << "\n";
      }
      curIdx += jump;
    } else if (state == ctl_Copy) {
      assert(CI->getNumArgOperands() == 1
          && "called ctl_Copy on non-unary function call!");
      CI->replaceAllUsesWith(CI->getOperand(0));
      deadInsts.insert(CI);
      if (mDebug) {
        dbgs() << "\n";
      }
      ++curIdx;
    } else if (state == ctl_Mutate) {
      ValueMutator::mutate(CI);
      ++curIdx;
      if (mDebug) {
        dbgs() << "\n";
      }
    } else {
      assert(!"Unknown state encountered!");
      ++curIdx;
    }
  }
  if (mDebug) {
    dbgs() << "\n";
  }
}
