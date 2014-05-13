//===  HSAILPrintfConvert.cpp - Printf Conversion Pass -===//
//===----------------------------------------------------------------------===//
//
//		HSA implementation of printf needs support in Device compiler.
//		This pass traverses the entire program and converts 
//		each call to printf	to a sequence of operations that store the following into the printf buffer :
//		- size of pointer
//		- address of format string 
//		- printf argument size
//		- printf argument value
//    - call to syscall intrinsic:
//     syscall 0x40,start address of printf buffer,end address of printf buffer,0
//
//     Printf Buffer Format :
//     <sizeof(void *), &fmt_str, sizeof(arg_val1), arg_val1, sizeof(arg_val2), arg_val2, ...> 
//
// Example : An Opencl kernel with printf
//  __kernel void test0(void){
//     printf("%5d\n",10);
//  }
//  Will translate to HSAIL:
//  readonly_u8 &__fmtStr0[6] = {37, 53, 100, 10, 0}  // ascii representation of "%5d\n"
//  kernel &__OpenCL_test0_kernel() 
//  {
// align 4 private_u8 %privateStack[16];
//
//@__OpenCL_test0_kernel_entry:
//        // BB#0:                                // %entry
//        lda_private_u32 $s0, [%privateStack];
//        mov_b32 $s1, 4;
//        st_private_u32  $s1, [$s0];
//        lda_readonly_u32        $s2, [&__fmtStr0];
//        st_private_u32  $s2, [$s0+4];
//        st_private_u32  $s1, [$s0+8];
//        mov_b32 $s1, 10;
//        st_private_u32  $s1, [$s0+12];
//        add_u32 $s1, $s0, 16;
//        mov_b32 $s2, 0;
//        syscall_u32     $s0, 64, $s0, $s1, $s2;
//        ret;
//  };
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "printfconvert"
#include "HSAILUtilityFunctions.h"
#include "llvm/Instructions.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Module.h"
#include "llvm/Type.h"
#include "llvm/Constants.h"
#include "llvm/CodeGen/Passes.h"
#define DWORD_ALIGN 4

using namespace llvm;
namespace {
class LLVM_LIBRARY_VISIBILITY HSAILPrintfConvert : public ModulePass {
public:
  static char ID;
  HSAILPrintfConvert(HSAILTargetMachine &tm);
  const char* getPassName() const;
  bool runOnModule(Module &M);
  bool doInitialization(Module &M);
  bool doFinalization(Module &M);
  void setModule(Module &M);
  Value *createPrivatePrintfBuffer(Module::iterator &MF);
  void processPrintfArguments(CallInst *CI, Value *printfBufPtr);
  void storeToPrintfBuf(Value *val, Value *printfBuf, int index1, int index2, 
                        Instruction *insertBefore, int bufIncr);
  void processFormatString(StringRef fmtStr, Value *printfBufPtr,
                           Instruction *insertBefore);
  void createSyscall(Value *printfBufPtr, Instruction *insertBefore);
private:
  Module *mod;
  HSAILTargetMachine &TM;
  int fmtStringNum;
  int printfBufIdx;
  int printfBufStartIdx;
  int eleOffset;
};
char HSAILPrintfConvert::ID = 0;
}

namespace llvm {
ModulePass *createHSAILPrintfConvert(HSAILTargetMachine &TM) {
  return new HSAILPrintfConvert(TM);
}
}

HSAILPrintfConvert::HSAILPrintfConvert(HSAILTargetMachine &tm)
  : ModulePass(ID), TM(tm), fmtStringNum(0), 
    printfBufIdx(0), printfBufStartIdx(0), eleOffset(0) {
}

void HSAILPrintfConvert::setModule(Module &M) {
  mod = &M;
}

// Routine to create printf buffer in private memory 
// that stores address of format strings and argument values of all printfs in the kernel
Value* HSAILPrintfConvert::createPrivatePrintfBuffer(Module::iterator &MF) {
  int numOfPrintsInKernel = 0, totalArgSize = 0, numArgs = 0, printfBufSize = 0;
  for (Function::iterator BB = MF->begin(), E = MF->end(); BB != E; ++BB) {
    for (BasicBlock::iterator I = BB->begin(), BBE = BB->end(); I != BBE; ++I) {
      CallInst *CI = dyn_cast<CallInst>(I);
      if (!(CI && CI->getCalledFunction()
            && CI->getCalledFunction()->getName() == "printf")) continue;
      numOfPrintsInKernel++;
      assert(eleOffset % DWORD_ALIGN == 0); 
      eleOffset += sizeof(int);
      eleOffset += TM.getDataLayout()->getPointerSize();  
      for (unsigned int x = 1; x < CI->getNumArgOperands(); ++x) {
        Value *op = CI->getArgOperand(x);
        Type *oType = op->getType();
        int eleCount = HSAILgetNumElements(oType);
        int eleSize = (int)oType->getScalarSizeInBits();
        if (0 == eleSize) eleSize = 32;
        if (0 == eleCount) eleCount = 1;
        // Calculate the printf argument size,
        // the offset of the printf argument should be at dword boundaries
        int argSize = (eleCount * (eleSize / 8 ));
        // Compute if the printf arg size is divisible by dword width
        // If not, add padding bytes, so that printf buffer is dword aligned
        int rem = argSize % DWORD_ALIGN;
        int padding = rem == 0 ? 0 : DWORD_ALIGN - rem;
        argSize += padding;
        totalArgSize += argSize;
        eleOffset += sizeof(int);
        eleOffset += argSize;
        numArgs++;
      }
    }
  }
  if (0 == numOfPrintsInKernel) return NULL;
  printfBufSize = 4 * numOfPrintsInKernel + 
                  TM.getDataLayout()->getPointerSize() * numOfPrintsInKernel +
                  4 * numArgs + totalArgSize;
  //Create printf buffer in private memory to hold 
  //address of format string and argument values
  ArrayType *printfBufType = ArrayType::get(IntegerType::get(mod->getContext(), 8),
                                            printfBufSize);
  Value *printfBuf = new AllocaInst(printfBufType, "printf_buf", 
                                    MF->getEntryBlock().getFirstInsertionPt());
  return printfBuf;
}

// Routine to store values to printf buffer
void HSAILPrintfConvert::storeToPrintfBuf(Value *val, Value *printfBuf,
                                          int index1, int index2,
                                          Instruction *insertBefore,
                                          int bufIncr) {
  std::vector<Value *> printfBufIndices;
  ConstantInt* constIntIdx1 = ConstantInt::get(mod->getContext(), 
                                              APInt(32, index1, false));
  ConstantInt* constIntIdx2 = ConstantInt::get(mod->getContext(), 
                                              APInt(32, index2, false));
  printfBufIndices.push_back(constIntIdx1);
  printfBufIndices.push_back(constIntIdx2);
  Value *printfBufPtr = GetElementPtrInst::Create(printfBuf, 
                                                  printfBufIndices, "", 
                                                  insertBefore);
  CastInst *castInst = CastInst::Create(Instruction::BitCast, printfBufPtr, 
                                        PointerType::get(val->getType(), 0), "",
                                        insertBefore);
  StoreInst *storeInst = new StoreInst(val, castInst, insertBefore);
  // Compute if the offset at which 
  // the printf argument will be stored (printfBufIdx + bufIncr),
  // is dword aligned, if not add padding bytes to ensure alignment
  int rem = (printfBufIdx + bufIncr) % DWORD_ALIGN;
  int padding = rem == 0 ? 0 : DWORD_ALIGN - rem;   
  printfBufIdx += bufIncr + padding;
}

// Routine to create format string and store its address in printf buffer
void HSAILPrintfConvert::processFormatString(StringRef fmtStr,
                                             Value *printfBuf,
                                             Instruction *insertBefore) {
  char buffer[256];
  sprintf(buffer, "__fmtStr%d",fmtStringNum);
  Constant *fmtStrArray = ConstantDataArray::getString(mod->getContext(), fmtStr.data(), true);
  // Create format string literal in constant memory
  GlobalVariable* fmtStringGvar = new GlobalVariable (*mod, fmtStrArray->getType(), 
                                                      true, 
                                                      GlobalValue::ExternalLinkage,
                                                      fmtStrArray, buffer, 
                                                      NULL, GlobalVariable::NotThreadLocal, 
                                                      HSAILAS::CONSTANT_ADDRESS);
  std::vector<Value *> fmtStrIdx;
  ConstantInt* zeroInt = ConstantInt::get(mod->getContext(),
                                          APInt(32, StringRef("0"), 10));
  fmtStrIdx.push_back(zeroInt);
  fmtStrIdx.push_back(zeroInt);
  Instruction *formatStringGep = GetElementPtrInst::Create(fmtStringGvar,
                                                           fmtStrIdx, "", 
                                                           insertBefore);
  // store the size of address in printf bufffer
  storeToPrintfBuf(ConstantInt::getSigned(
                   Type::getInt32Ty(mod->getContext()), 
                   TM.getDataLayout()->getPointerSize()), 
                   printfBuf, 0, printfBufIdx, insertBefore, 
                   /* size of int */ 4);
  // Store the address of format string literal in private printf buffer 
  storeToPrintfBuf(formatStringGep, printfBuf, 0, printfBufIdx, insertBefore, 
                   TM.getDataLayout()->getPointerSize());
}

//Routine to replace call to printf by sequence of 
//stores of argument values to printf buffer
void HSAILPrintfConvert::processPrintfArguments(CallInst *CI, Value *printfBuf) {
  for (unsigned x = 1; x < CI->getNumArgOperands(); x++) {
    Value *op = CI->getArgOperand(x);
    Type *oType = op->getType();
    int size = oType->getPrimitiveSizeInBits();
    if (size == 0) {
      const PointerType *PT = dyn_cast<PointerType>(oType);
	  size = TM.getDataLayout()->getPointerSize() * 8;
      if (PT->getAddressSpace() == HSAILAS::CONSTANT_ADDRESS
          && PT->getContainedType(0)->getScalarSizeInBits() == 8
          && HSAILgetNumElements(PT->getContainedType(0)) == 1) {
        op = new BitCastInst(op, Type::getInt8PtrTy(oType->getContext(),
                             HSAILAS::CONSTANT_ADDRESS), "printfPtrCast", CI);
        oType = op->getType();
      }  else {    
        op = new PtrToIntInst(op, Type::getIntNTy(oType->getContext(),
                              TM.getDataLayout()->getPointerSize() * 8),
                              "printfPtrCast", CI);
      }  
    }
    // store the argument size in the printf buffer
    storeToPrintfBuf(ConstantInt::getSigned(
                     Type::getInt32Ty(mod->getContext()),size/8), 
                     printfBuf, 0, printfBufIdx, CI, /* size of int */4);
    // store the argument in the printf buffer
    storeToPrintfBuf(op, printfBuf, 0, printfBufIdx, CI, size/8);
  }
  // delete the call to printf
  Constant *newConst = ConstantInt::getSigned(CI->getType(), 0);
  CI->replaceAllUsesWith(newConst);
  CI->eraseFromParent();
}

// Routine to create a call to syscall intrinsic
void HSAILPrintfConvert::createSyscall(Value *printfBuf, 
                                       Instruction *insertBefore) {
  // Compute start and end address of printfBuf
  ConstantInt* zeroInt = ConstantInt::get(mod->getContext(), 
                                          APInt(32, StringRef("0"), 10));
  ConstantInt* constIntBegIdx = ConstantInt::get(mod->getContext(),
                                                APInt(32, printfBufStartIdx,
                                                      false));
  std::vector<Value *> begIndices;
  begIndices.push_back(zeroInt);
  begIndices.push_back(constIntBegIdx);
  std::vector<Value *> endIndices;
  ConstantInt* constIntEndIdx = ConstantInt::get(mod->getContext(),
                                                APInt(32, printfBufIdx, false));
  endIndices.push_back(zeroInt);
  endIndices.push_back(constIntEndIdx);
  Instruction *printfBufBegPtr = GetElementPtrInst::Create(printfBuf, 
                                                           begIndices,
                                                           "", insertBefore);
  Instruction *printfBufEndPtr = GetElementPtrInst::Create(printfBuf,
                                                           endIndices,
                                                           "", insertBefore);
  Instruction *begPtrToInt = new PtrToIntInst(printfBufBegPtr, 
                                              Type::getInt32Ty(mod->getContext()),
                                              "", insertBefore);
  Instruction *endPtrToInt = new PtrToIntInst(printfBufEndPtr,
                                              Type::getInt32Ty(mod->getContext()),
                                              "", insertBefore);
  // Identifier 0x40 for printf in syscall
  Constant *printfId = ConstantInt::get(mod->getContext(), APInt(32, StringRef("40"), 16));
  Constant *zeroArg = ConstantInt::get(mod->getContext(), APInt(32, 0, false));
  // Create call to syscall intrinsic with arguments:
  // 0,start and end address of printf buffer,0
  Type * argTypes[] = {printfId->getType(), begPtrToInt->getType(), 
                       endPtrToInt->getType(), zeroArg->getType()};
  const HSAILIntrinsicInfo *intrnInfo = TM.getIntrinsicInfo();
  Value *syscallIntrinsic = intrnInfo->getDeclaration(mod,
                                                      HSAILIntrinsic::HSAIL_syscall,
                                                      argTypes, 4);
  Value *args[] = {printfId, begPtrToInt, endPtrToInt, zeroArg};
  CallInst::Create(syscallIntrinsic, args, "", insertBefore);
}

bool HSAILPrintfConvert::runOnModule(Module &M) {
  setModule(M);
  
  for (Module::iterator MF = M.begin(), E = M.end(); MF != E; ++MF) {
    if (MF->isDeclaration()) continue;
    BasicBlock::iterator curInstr;
    // Create printf buffer in private memory
    Value *printfBuf = createPrivatePrintfBuffer(MF);
    if (NULL == printfBuf) continue;
    for (Function::iterator BB = MF->begin(), MFE = MF->end(); BB != MFE; ++BB) {
      for (BasicBlock::iterator instr = BB->begin(), instr_end = BB->end(); 
           instr != instr_end; ) {
        CallInst *CI = dyn_cast<CallInst>(instr);
        curInstr = instr;
        instr++;
        if (!(CI && CI->getCalledFunction() 
              && CI->getCalledFunction()->getName() == "printf")) continue;
        Value *op = CI->getArgOperand(0);
        ConstantExpr *GEPinst = dyn_cast<ConstantExpr>(op);
        if (GEPinst) {
          GlobalVariable *GVar = dyn_cast<GlobalVariable>(GEPinst->getOperand(0));
          if (GVar && GVar->hasInitializer()) {
            ConstantDataArray *CA = dyn_cast<ConstantDataArray>(GVar->getInitializer());
            if (CA->isString()) {
              std::string str = CA->getAsCString(); 
              StringRef fmtStr(str);
              // Store addrress of format string in printf buffer
              printfBufStartIdx = printfBufIdx;
              processFormatString(fmtStr, printfBuf, curInstr);
              fmtStringNum++;
            }
          }
        }
        // Replace call to printf by stores of arg values into printf buffer
        processPrintfArguments(CI, printfBuf);
        curInstr = instr;
        // Create a call to syscall intrinsic
        createSyscall(printfBuf, curInstr);
      }
    }
  }
  // Delete the dead printf declaration
  Function *printfDeclaration = M.getFunction(StringRef("printf"));
  if (printfDeclaration && printfDeclaration->isDeclaration() 
      && printfDeclaration->use_empty()) printfDeclaration->eraseFromParent();
  return true;
}

const char* HSAILPrintfConvert::getPassName() const {
  return "HSAIL Printf Conversion Pass";
}

bool HSAILPrintfConvert::doInitialization(Module &M) {
  return false;
}

bool HSAILPrintfConvert::doFinalization(Module &M) {
  return false;
}
