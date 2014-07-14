//=== AMDPrintfRuntimeBinding.cpp -- For openCL -- bind Printfs to a kernel arg
//    pointer that will be bound to a buffer later by the runtime ===//
//===----------------------------------------------------------------------===//
// March 2014.
//		This pass traverses the functions in the module and converts
//		each call to printf	to a sequence of operations that
//		store the following into the printf buffer :
//		- format string (passed as a meta data to an internal function _printf_id)
//		- bitwise copies of printf arguments
//		The backend passes will need to set unique IDs and store in the kernel
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "printfToRuntime"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Constants.h"
#include "llvm/DataLayout.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Instructions.h"
#include "llvm/IRBuilder.h"
#include "llvm/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Type.h"
#include "llvm/ADT/SmallString.h"
#define DWORD_ALIGN 4
using namespace llvm;

namespace {
class LLVM_LIBRARY_VISIBILITY AMDPrintfRuntimeBinding : public ModulePass {
public:
  static char ID;
  bool cpuLowering;
  explicit AMDPrintfRuntimeBinding(bool);
  SmallVector<Value*, 32> printfs;
  const char* getPassName() const;
  bool runOnModule(Module &M);
  bool doInitialization(Module &M);
  bool doFinalization(Module &M);
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DataLayout>();
  }
  void getConversionSpecifiers(
              SmallVectorImpl<char> &opConvSpecifiers,
              StringRef fmt,
              size_t num_ops) const;

  bool shouldPrintAsStr(char Specifier, Type* OpType) const;
  const DataLayout* TD;
  bool confirmSpirModule(Module& M) const;
  bool confirmOpenCLVersion200(Module& M) const;
  bool lowerPrintfForGpu(Module &M);
  bool lowerPrintfForCpu(Module &M);
  void collectPrintfsFromModule(Module &M);
  std::string transPrintfVectorFormat(StringRef stref);
};

char AMDPrintfRuntimeBinding::ID = 0;
}

namespace llvm {
ModulePass *createAMDPrintfRuntimeBinding(bool cpuLowering) {
  return new AMDPrintfRuntimeBinding(cpuLowering);
}
}

AMDPrintfRuntimeBinding::AMDPrintfRuntimeBinding(bool cpuLowering)
  : ModulePass(ID), cpuLowering(cpuLowering) { }

bool AMDPrintfRuntimeBinding::confirmOpenCLVersion200(Module& M) const {
  NamedMDNode *OCLVersion = M.getNamedMetadata("opencl.ocl.version");
  if (!OCLVersion) {
    return false;
  }
  if (OCLVersion->getNumOperands() != 1) {
    return false;
  }
  MDNode *ver = OCLVersion->getOperand(0);
  if (ver->getNumOperands() != 2) {
    return false;
  }
  ConstantInt *major = dyn_cast<ConstantInt>(ver->getOperand(0));
  ConstantInt *minor = dyn_cast<ConstantInt>(ver->getOperand(1));
  if (0 == major || 0 == minor) {
    return false;
  }
  if (major->getZExtValue() == 2) {
    return true;
  } else {
    return false;
  }
}

void AMDPrintfRuntimeBinding::getConversionSpecifiers (
  SmallVectorImpl<char> &OpConvSpecifiers,
  StringRef Fmt, size_t NumOps) const {
  // not all format characters are collected.
  // At this time the format characters of interest
  // are %p and %s, which use to know if we
  // are either storing a literal string or a
  // pointer to the printf buffer.
  static const char ConvSpecifiers[] = "cdieEfgGaosuxXp";
  size_t CurFmtSpecifierIdx = 0;
  size_t PrevFmtSpecifierIdx = 0;

  while ((CurFmtSpecifierIdx
            = Fmt.find_first_of(ConvSpecifiers, CurFmtSpecifierIdx))
         != StringRef::npos) {
    bool ArgDump = false;
    StringRef CurFmt = Fmt.substr(PrevFmtSpecifierIdx,
                                  CurFmtSpecifierIdx - PrevFmtSpecifierIdx);
    size_t pTag = CurFmt.find_last_of("%");
    if (pTag != StringRef::npos) {
      ArgDump = true;
      while (pTag && CurFmt[--pTag] == '%') {
        ArgDump = !ArgDump;
      }
    }

    if (ArgDump) {
      OpConvSpecifiers.push_back(Fmt[CurFmtSpecifierIdx]);
    }

    PrevFmtSpecifierIdx = ++CurFmtSpecifierIdx;
  }
}

bool AMDPrintfRuntimeBinding::shouldPrintAsStr(char Specifier,
                                               Type* OpType) const {
  if (Specifier != 's') {
    return false;
  }
  const PointerType *PT = dyn_cast<PointerType>(OpType);
  if (!PT) {
    return false;
  }
  if (PT->getAddressSpace() != 2) {
    return false;
  }
  Type* ElemType = PT->getContainedType(0);
  if (ElemType->getTypeID() != Type::IntegerTyID) {
    return false;
  }
  IntegerType* ElemIType = cast<IntegerType>(ElemType);
  if (ElemIType->getBitWidth() == 8) {
    return true;
  } else {
    return false;
  }
}

bool AMDPrintfRuntimeBinding::confirmSpirModule(Module& M) const {
  NamedMDNode *SPIRVersion = M.getNamedMetadata("opencl.spir.version");
  if (!SPIRVersion) return false;
  else return true;
}

void AMDPrintfRuntimeBinding::collectPrintfsFromModule(Module& M) {
  for (Module::iterator MF = M.begin(), E = M.end(); MF != E; ++MF) {
    if (MF->isDeclaration()) continue;
    BasicBlock::iterator curInstr;
    for (Function::iterator BB = MF->begin(),
             MFE = MF->end(); BB != MFE; ++BB) {
      for (BasicBlock::iterator instr
             = BB->begin(), instr_end = BB->end();
           instr != instr_end; ) {
        CallInst *CI = dyn_cast<CallInst>(instr);
        curInstr = instr;
        instr++;
        if (CI && CI->getCalledFunction()
            && CI->getCalledFunction()->getName() == "printf") {
          printfs.push_back(CI);
        }
      }
    }
  }
}

std::string AMDPrintfRuntimeBinding::transPrintfVectorFormat(StringRef str) {
  SmallVector<StringRef, 32> opndModifiers;
  std::string fmt(str);
  size_t curFmtSpecifierIdx = 0;
  size_t nextFmtSpecifierIdx = 0;
  size_t vecFmtSpecifierIdx = 0;
  bool isVectorFormat = false;
  static const char convSpecifiers[] = "cdieEfgGaosuxXp";
  curFmtSpecifierIdx = fmt.find_first_of('%',curFmtSpecifierIdx);
  std::string transFmt = fmt.substr(0,curFmtSpecifierIdx);
  opndModifiers.push_back("");
  while (curFmtSpecifierIdx != std::string::npos) {
    nextFmtSpecifierIdx = fmt.find_first_of("%",curFmtSpecifierIdx + 1);
    std::string curFmt;
    if (nextFmtSpecifierIdx != std::string::npos) {
      curFmt = fmt.substr(curFmtSpecifierIdx,
                          nextFmtSpecifierIdx - curFmtSpecifierIdx);
    }
    else {
      curFmt = fmt.substr(curFmtSpecifierIdx);
    }
    size_t convSpecifierIdx;
    //get modifier and store it in the opndModifiers
    if ((convSpecifierIdx =
         curFmt.find_first_of(convSpecifiers)) !=  std::string::npos) {
      if (curFmt[convSpecifierIdx - 1] == 'h') {
        if (convSpecifierIdx > 1 && curFmt[convSpecifierIdx - 2] == 'h') {
          opndModifiers.push_back("hh");
        } else {
          opndModifiers.push_back("h");

        }
      } else if (curFmt[convSpecifierIdx - 1] == 'l') {
        if (convSpecifierIdx > 1 && curFmt[convSpecifierIdx - 2] == 'h') {
          opndModifiers.push_back("hl");
        } else {
          opndModifiers.push_back("l");
        }
      } else {
        opndModifiers.push_back("");
      }
    }
    std::string compFmt;
    vecFmtSpecifierIdx = 0;
    // Check if the vector should be printed:
    // one of "v16",v2,"v3",v4","v8" indicate
    // to vector convension specifier and its elemnts count.
    while ((vecFmtSpecifierIdx = curFmt.find_first_of('v',vecFmtSpecifierIdx))
           != std::string::npos) {
      isVectorFormat = true;
      char elmCount = 0;
      char elmFieldSize = 0;
      if ((vecFmtSpecifierIdx + 1) < curFmt.length()) {
        elmCount = curFmt[vecFmtSpecifierIdx + 1];
        if ((elmCount == '1') && ((vecFmtSpecifierIdx + 2) < curFmt.length())
            && (curFmt[vecFmtSpecifierIdx + 2] == '6')) {
          elmCount = 16;
          elmFieldSize = 2;
        } else if ((('2' <= elmCount) && (elmCount <= '4'))
                   || (elmCount == '8')) {
          elmCount -= '0';
          elmFieldSize = 1;
        }
        else {
          // If there is no element count after 'v',
          // continue to look for valid vector specifier.
          elmCount = 0;
          ++vecFmtSpecifierIdx;
          continue;
        }
        // Rebuild the format to contain the
        // convension specifier to each of vector elements.
        if (elmCount) {
          std::string fmtSuffix;
          convSpecifierIdx = curFmt.find_first_of(
            convSpecifiers,vecFmtSpecifierIdx + 1);
          if (curFmt.length() - 1 != convSpecifierIdx) {
            compFmt = curFmt.substr(convSpecifierIdx + 1);
            curFmt = curFmt.erase(convSpecifierIdx + 1);
          }
          if (nextFmtSpecifierIdx == std::string::npos) {
            fmtSuffix = curFmt.substr(convSpecifierIdx + 1);
            curFmt.erase(convSpecifierIdx + 1);
          }
          if (!curFmt.empty()) {
            // If long value is represented by 4 bytes
            // and that llvm long value is represented by 64-bit,
            // the string format should be converted to have
            // "ll" modifier.
            if (opndModifiers.back() == "l" && sizeof(long) == 4)
              curFmt.insert(convSpecifierIdx - 1, "l");
            curFmt.erase(vecFmtSpecifierIdx, elmFieldSize+1);
            // Donot need "hl" modifier for vector arguments formats.
            if (opndModifiers.back() == "hl") {
              curFmt.erase(curFmt.find_first_of("hl"),2);
            }
            for (char i = 0; i < elmCount - 1; ++i) {
              transFmt = transFmt + curFmt + ",";
            }
          }
          else {
            curFmt = fmtSuffix;
            break;
          }
          if (!fmtSuffix.empty()) {
            curFmt += fmtSuffix;
          }
        }
      }
    }
    transFmt += curFmt;
    if (!compFmt.empty()) {
      transFmt += compFmt;
    }
    curFmtSpecifierIdx = nextFmtSpecifierIdx;
  }
  return transFmt;
}

bool AMDPrintfRuntimeBinding::lowerPrintfForCpu(Module &M) {
  for (SmallVectorImpl<Value*>::iterator
         print_iterate = printfs.begin(),
            print_iterate_e = printfs.end();
       print_iterate != print_iterate_e;
       ++print_iterate) {
    CallInst* CI = dyn_cast<CallInst>( *print_iterate);

    unsigned num_ops = CI->getNumArgOperands();

    SmallString<16> opConvSpecifiers;
    Value *op = CI->getArgOperand(0);
    ConstantExpr *const_expr = dyn_cast<ConstantExpr>(op);

    if (const_expr) {
      GlobalVariable *GVar = dyn_cast<GlobalVariable>(
            const_expr->getOperand(0));

      if (GVar && GVar->hasInitializer()) {
        ConstantDataArray *CA = dyn_cast<ConstantDataArray>(
              GVar->getInitializer());
        if (CA->isString()) {
          StringRef str("unknown");
          str = CA->getAsCString();
          DEBUG(dbgs() << "Processing cpu printf format = "
                << str.str() << '\n');
          std::string trans = transPrintfVectorFormat(str);
          if (trans != str) {
            Constant *fmtStrArray =
              ConstantDataArray::getString(M.getContext(), trans.c_str(), true);
            GlobalVariable* newfmt = new GlobalVariable(M,
                                      fmtStrArray->getType(),
                                      true,
                                      GlobalValue::ExternalLinkage,
                                      fmtStrArray, "fmtPrintf",
                                      NULL, GlobalVariable::NotThreadLocal,
                                      GVar->getType()->getAddressSpace());
            DEBUG(dbgs() << "Format after expanding vectors = "
                  << *newfmt << '\n');
            Constant* ncexp = ConstantExpr::getBitCast(newfmt,
                                const_expr->getType());
            const_expr->replaceAllUsesWith(ncexp);
            if (CI->getNumArgOperands() > 1 ) {
              SmallVector<Value*, 32> callargs;
              callargs.push_back(ncexp);
              bool callFix = false;
              Type *I32Ty = Type::getInt32Ty(M.getContext());
              for (unsigned argcount = 1;
                   argcount < CI->getNumArgOperands();
                   argcount++) {
                Value *arg = CI->getArgOperand(argcount);
                Type *argtype = arg->getType();
                if (argtype->getTypeID() == Type::VectorTyID) {
                  callFix = true;
                  uint32_t elemSize =
                    cast<VectorType>(arg->getType())->getNumElements();
                  DEBUG(dbgs() << "Need to extract printf vector = "
                        << *arg << '\n');
                  for (uint32_t idxv = 0; idxv < elemSize; ++idxv) {
                    Value* extr = ExtractElementInst::Create(
                                 arg, ConstantInt::get(I32Ty, idxv, false),
                                 "printfvecext", CI);
                    DEBUG(dbgs() << "printf vector extract = " <<
                          *extr << '\n');
                    DEBUG(dbgs() << "extract's type = "
                          << *extr->getType() << '\n');
                    if (argtype->getScalarType()->isFloatTy() ||
                        argtype->getScalarType()->isHalfTy()) {
                      Type *doublety = Type::getDoubleTy(M.getContext());
                      extr = CastInst::CreateFPCast(extr, doublety,
                                          "defArgPromPrintfVec", CI);
                      DEBUG(dbgs() << "FPext ins = " << *extr << '\n');
                    }
                    callargs.push_back(extr);
                  }
                } else {
                  DEBUG(dbgs() << "nonvector = " << *arg << '\n');
                  callargs.push_back(arg);
                }
              }
              if (callFix) {
                DEBUG(dbgs() << "printf function signature = "
                      << *CI->getCalledFunction() << '\n');
                CallInst *newprintf = CallInst::Create(CI->getCalledFunction(),
                                                    callargs, "printf_", CI);
                DEBUG(dbgs() << "Before transformation of vector = "
                      << *CI << '\n');
                DEBUG(dbgs() << "Now = " << *newprintf << '\n');
                CI->eraseFromParent();
              }
            }
          }
        }
      }
    }
  }
  return true;
}

bool AMDPrintfRuntimeBinding::lowerPrintfForGpu(Module &M) {
  TD = &getAnalysis<DataLayout>();
  bool is64Bit = (TD->getPointerSizeInBits() == 64);

  for (SmallVectorImpl<Value*>::iterator
         print_iterate = printfs.begin(),
            print_iterate_e = printfs.end();
       print_iterate != print_iterate_e;
       ++print_iterate) {
    CallInst* CI = dyn_cast<CallInst>( *print_iterate);

    unsigned num_ops = CI->getNumArgOperands();

    SmallString<16> opConvSpecifiers;
    Value *op = CI->getArgOperand(0);
    ConstantExpr *const_expr = dyn_cast<ConstantExpr>(op);

    if (const_expr) {
      GlobalVariable *GVar = dyn_cast<GlobalVariable>(
            const_expr->getOperand(0));

      StringRef str("unknown");
      if (GVar && GVar->hasInitializer()) {
        ConstantDataArray *CA = dyn_cast<ConstantDataArray>(
              GVar->getInitializer());
        if (CA->isString()) {
          str = CA->getAsCString();
        }
        //
        // we need this call to ascertain
        // that we are printing a string
        // or a pointer. It takes out the
        // specifiers and fills up the first
        // arg
        getConversionSpecifiers( opConvSpecifiers, str, num_ops - 1);
      }
      if (CI->getNumArgOperands() > 1 ) {
        // Add metadata for the string,
        // it is set as a meta data on the __printf_id call
        std::string astreamholder;
        raw_string_ostream sizes(astreamholder);
        int sum = DWORD_ALIGN;
        BasicBlock *origbb = CI->getParent();
        sizes << CI->getNumArgOperands() -1;
        sizes << ':';
        LLVMContext &Ctx = M.getContext();
        for (unsigned argcount = 1;
             argcount < CI->getNumArgOperands()
               && argcount <= opConvSpecifiers.size();
             argcount++) {
          Value *arg = CI->getArgOperand(argcount);
          Type *argtype = arg->getType();
          unsigned argsize = TD->getTypeAllocSizeInBits(argtype);
          argsize = argsize/8;
          if (opConvSpecifiers[argcount - 1] == 'f') {
            ConstantFP *fpCons = dyn_cast<ConstantFP>(arg);
            if (fpCons)
              argsize = 4;
            else {
              FPExtInst *fpext = dyn_cast<FPExtInst>(arg);
              if (fpext && fpext->getType()->isDoubleTy() &&
                  fpext->getOperand(0)->getType()->isFloatTy())
                argsize = 4;
            }
          }
          if (shouldPrintAsStr(opConvSpecifiers[argcount - 1], argtype)) {
            ConstantExpr *strC = dyn_cast<ConstantExpr>(arg);
            if (strC) {
              GlobalVariable *strG
                = dyn_cast<GlobalVariable>(strC->getOperand(0));
              if (strG && strG->hasInitializer()) {
                ConstantDataArray *strCA = dyn_cast<ConstantDataArray>(
                  strG->getInitializer());
                if (strCA->isString()) {
                  size_t size_str = strlen(strCA->getAsCString().data());
                  size_t rem = size_str % DWORD_ALIGN;
                  size_t nsize_str = 0;
                  DEBUG(dbgs() << "Printf string original size = " << size_str << '\n');
                  if (rem) {
                    nsize_str = size_str + (DWORD_ALIGN - rem);
                  } else {
                    nsize_str = size_str;
                  }
                  argsize = nsize_str;
                }
              }
            }
          }
          DEBUG(dbgs() << "Printf argsize (in buffer) = "
                << argsize << " for type: " << *argtype << '\n');
          sizes << argsize << ':';
          sum += argsize;
        }
        DEBUG(dbgs() << "Printf format string in source = "
                     << str.str() << '\n');
        sizes << str.str();

        // Insert the printf_alloc call
        IRBuilder<> Builder(Ctx);
        Builder.SetInsertPoint(CI);
        Builder.SetCurrentDebugLocation(CI->getDebugLoc());

        Attributes::AttrVal AVs[1]
          = { Attributes::NoUnwind };
        AttributeWithIndex AWI[1]
          = { AttributeWithIndex::get(
                Ctx,
                AttrListPtr::FunctionIndex,
                AVs)
            };

        Type *sizetTy
          = is64Bit?    (Type::getInt64Ty(Ctx))
                      : (Type::getInt32Ty(Ctx));
        sizetTy = Type::getInt32Ty(Ctx);

      Type *Tys_alloc[1] = { sizetTy };
      Type *I8Ptr = PointerType::get( Type::getInt8Ty(Ctx), 1);
      FunctionType *FTy_alloc
        = FunctionType::get( I8Ptr, Tys_alloc, false);
      Constant *printf_alloc_fn
        = M.getOrInsertFunction(StringRef("__printf_alloc"), FTy_alloc,
            AttrListPtr::get(Ctx, AWI));
      Function *afn = dyn_cast<Function>(printf_alloc_fn);
      afn->setCallingConv(llvm::CallingConv::SPIR_FUNC);
      DEBUG(dbgs() << "inserting printf_alloc decl, an extern @ pre-link:");
      DEBUG(dbgs() << *afn);

      Type *I32Ty = Type::getInt32Ty(Ctx);
      ArrayRef<Type*> Tys_id;
      FunctionType *FTy_id
        = FunctionType::get(I32Ty, Tys_id, false);
      Constant *printf_id_fn
        = M.getOrInsertFunction(StringRef("__printf_id"), FTy_id,
            AttrListPtr::get(Ctx, AWI));
      Function *affn = dyn_cast<Function>(printf_id_fn);
      // at pre-link the calling conv is SPIR_FUNC
      affn->setCallingConv(llvm::CallingConv::SPIR_FUNC);
      CallInst *pcall = NULL;

      DEBUG(dbgs() << "Printf metadata = " << sizes.str() << '\n');
      str = StringRef(sizes.str().c_str());
      Constant *fmtStrArray
        = ConstantDataArray::getString( Ctx, str, true);

      SmallVector<Value*,1> ZeroIdxList;
      ConstantInt* zeroInt
        = ConstantInt::get( Ctx, APInt( 32, StringRef("0"), 10));
      ZeroIdxList.push_back(zeroInt);

      // Instead of creating global variables, the
      // printf format strings are extracted
      // and passed as metadata. This avoids
      // polluting llvm's symbol tables in this module.
      // Metadata is going to be extracted
      // by the backend passes and inserted
      // into the OpenCL binary as appropriate.
      StringRef amd("llvm.printf.fmts");
      NamedMDNode *metaD = M.getOrInsertNamedMetadata(amd);
      MDNode *myMD = MDNode::get(Ctx,fmtStrArray);
      metaD->addOperand(myMD);
      Value *sumC = ConstantInt::get( sizetTy, sum, false);
      SmallVector<Value*,1> alloc_args;
      alloc_args.push_back(sumC);
      pcall = CallInst::Create( afn, alloc_args,
                            "printf_alloc_fn", CI);
      pcall->setCallingConv(llvm::CallingConv::SPIR_FUNC);

      //
      // Insert code to split basicblock with a
      // piece of hammock code.
      // basicblock splits after buffer overflow check
      //
      ConstantPointerNull *zeroIntPtr
        = ConstantPointerNull::get(PointerType::get(Type::getInt8Ty(Ctx),
            1));
      ICmpInst *cmp
        = dyn_cast<ICmpInst>(
            Builder.CreateICmpNE(pcall, zeroIntPtr, ""));
      BasicBlock* splitBB
        = SplitBlock( CI->getParent(), cmp, this);
      TerminatorInst *brnch
        = SplitBlockAndInsertIfThen( cmp, false);

      Builder.SetInsertPoint(brnch);

      // insert the printf_id call and store its result in the buffer
      // this function does nto result in an execution code.
      // it is thrown out by the backend.
      //
      SmallVector<Value*,2> zeroIdxList;
      zeroIdxList.push_back(zeroInt);
      SmallVector<Value*,1> alloc_id_args;
      CallInst *pcall_id
        = CallInst::Create(affn, alloc_id_args,
                           "printf_id_fn", brnch);
      pcall_id->setMetadata("prnFmt", myMD);

      GetElementPtrInst *buffer_idx
        = dyn_cast<GetElementPtrInst>(
            GetElementPtrInst::Create(
              pcall, zeroIdxList, "PrintBuffID", brnch));

      Type *idPointer
        = PointerType::get( pcall_id->getType(), 1);
      Value *id_gep_cast
        = new BitCastInst( buffer_idx, idPointer,
                           "PrintBuffIdCast", brnch);

      StoreInst* stbuff
        = new StoreInst( pcall_id, id_gep_cast, brnch);

      SmallVector<Value*,2> FourthIdxList;
      ConstantInt* fourInt
        = ConstantInt::get(Ctx, APInt(
            32, StringRef("4"), 10));

      FourthIdxList.push_back(fourInt); // 1st 4 bytes hold the printf_id
      // the following GEP is the buffer pointer
      buffer_idx
        = cast<GetElementPtrInst>(GetElementPtrInst::Create(
              pcall, FourthIdxList, "PrintBuffGep", brnch));

      Type* Int32Ty = Type::getInt32Ty(Ctx);
      Type* Int64Ty = Type::getInt64Ty(Ctx);
      for (unsigned argcount = 1;
           argcount < CI->getNumArgOperands()
             && argcount <= opConvSpecifiers.size();
           argcount++) {
        Value *arg = CI->getArgOperand(argcount);
        Type *argType = arg->getType();
        SmallVector<Value*,32> whatToStore;
        if (argType->isFPOrFPVectorTy()
              && (argType->getTypeID() != Type::VectorTyID)) {
          Type *iType = (argType->isFloatTy()) ?  Int32Ty : Int64Ty;
          if (opConvSpecifiers[argcount - 1] == 'f') {
            ConstantFP *fpCons = dyn_cast<ConstantFP>(arg);
            if (fpCons) {
              APFloat Val(fpCons->getValueAPF());
              bool lost = false;
              Val.convert(APFloat::IEEEsingle,
                          APFloat::rmNearestTiesToEven,
                          &lost);
              arg = ConstantFP::get(Ctx, Val);
              iType = Int32Ty;
            } else {
              FPExtInst *fpext = dyn_cast<FPExtInst>(arg);
              if (fpext && fpext->getType()->isDoubleTy()
                  && fpext->getOperand(0)->getType()->isFloatTy()) {
                arg = fpext->getOperand(0);
                iType = Int32Ty;
              }
            }
          }
          arg = new BitCastInst(arg, iType, "PrintArgFP", brnch);
          whatToStore.push_back(arg);
        } else if (argType->getTypeID() == Type::PointerTyID) {
          if (shouldPrintAsStr(
                opConvSpecifiers[argcount - 1], argType)) {
            ConstantExpr *strC = dyn_cast<ConstantExpr>(arg);
            if (strC) {
              GlobalVariable *strG
                = dyn_cast<GlobalVariable>(strC->getOperand(0));
              if (strG && strG->hasInitializer()) {
                ConstantDataArray *strCA = dyn_cast<ConstantDataArray>(
                                             strG->getInitializer());
                if (strCA->isString()) {
                  size_t size_str = strlen(strCA->getAsCString().data());
                  size_t rem = size_str % DWORD_ALIGN;
                  size_t nsize_str = 0;
                  if (rem) {
                    nsize_str = size_str + (DWORD_ALIGN - rem);
                  } else {
                    nsize_str = size_str;
                  }
                  char *mynewstr = new char[nsize_str]();
                  strcpy(mynewstr, strCA->getAsCString().data());
                  int numints = nsize_str/4;
                  int charc = 0;
                  while(numints) {
                    int anum = *(int*)(mynewstr+charc);
                    charc += 4;
                    numints--;
                    Value *anumV
                      = ConstantInt::get( Int32Ty, anum, false);
                    whatToStore.push_back(anumV);
                  }
                  delete mynewstr;
                }
              }
            }
          } else {
            uint64_t Size = TD->getTypeAllocSizeInBits(argType);
            assert((Size == 32 || Size == 64) && "unsupported size");
            Type* DstType = (Size == 32) ? Int32Ty : Int64Ty;
            arg = new PtrToIntInst(arg, DstType,
                                    "PrintArgPtr", brnch);
            whatToStore.push_back(arg);
          }
        } else if (argType->getTypeID() == Type::VectorTyID) {
          Type *iType = NULL;
          uint32_t eleCount = cast<VectorType>(argType)->getNumElements();
          uint32_t eleSize = argType->getScalarSizeInBits();
          uint32_t totalSize = eleCount * eleSize;
          if (eleCount == 3) {
            IntegerType *int32ty
              = Type::getInt32Ty(argType->getContext());
            Constant* indices[4]
              = { ConstantInt::get(int32ty, 0),
                  ConstantInt::get(int32ty, 1),
                  ConstantInt::get(int32ty, 2),
                  ConstantInt::get(int32ty, 2)
                };
            Constant* mask = ConstantVector::get(indices);
            ShuffleVectorInst* shuffle
              = new ShuffleVectorInst(arg, arg, mask);
            shuffle->insertBefore(brnch);
            arg = shuffle;
            argType = arg->getType();
            totalSize += eleSize;
          }
          switch (eleSize) {
            default:
              eleCount = totalSize / 64;
              iType = dyn_cast<Type>(
                        Type::getInt64Ty(
                          argType->getContext()));
              break;
            case 8:
              if (eleCount >= 8) {
                eleCount = totalSize / 64;
                iType = dyn_cast<Type>(
                          Type::getInt64Ty(
                            argType->getContext()));
              } else if (eleCount >= 3) {
                eleCount = 1;
                iType = dyn_cast<Type>(
                          Type::getInt32Ty(
                            argType->getContext()));
              } else {
                eleCount = 1;
                iType = dyn_cast<Type>(
                          Type::getInt16Ty(
                           argType->getContext()));
              }
              break;
            case 16:
              if (eleCount >= 3) {
                eleCount = totalSize / 64;
                iType = dyn_cast<Type>(
                          Type::getInt64Ty(
                            argType->getContext()));
              } else {
                eleCount = 1;
                iType = dyn_cast<Type>(
                          Type::getInt32Ty(
                            argType->getContext()));
              }
              break;
            }
            if (eleCount > 1) {
              iType = dyn_cast<Type>(
                        VectorType::get(
                          iType, eleCount));
            }
            arg = new BitCastInst(arg, iType, "PrintArgVect", brnch);
            whatToStore.push_back(arg);
          } else {
            whatToStore.push_back(arg);
          }

          for ( SmallVectorImpl<Value*>::iterator
                 w_iterate = whatToStore.begin(),
                 w_iterate_e = whatToStore.end();
                 w_iterate != w_iterate_e; ) {
            Value* thebtcast = *w_iterate;
            unsigned argsize
              = TD->getTypeAllocSizeInBits(thebtcast->getType())/8;
            SmallVector<Value*,1> buffOffset;
            buffOffset.push_back(
              ConstantInt::get( I32Ty, argsize));

            Type *argPointer
              = PointerType::get( thebtcast->getType(), 1);
            Value *casted_gep
              = new BitCastInst( buffer_idx, argPointer,
                                 "PrintBuffPtrCast", brnch);
            StoreInst* stbuff
              = new StoreInst(
                      thebtcast, casted_gep, brnch);
            DEBUG(dbgs() << "inserting store to printf buffer:\n"
                         << *stbuff << '\n');
            ++w_iterate;
            if (w_iterate == w_iterate_e
                && argcount+1 == CI->getNumArgOperands())
              break;
            buffer_idx
                = dyn_cast<GetElementPtrInst>(GetElementPtrInst::Create(
                      buffer_idx, buffOffset, "PrintBuffNextPtr", brnch));
            DEBUG(dbgs() << "inserting gep to the printf buffer:\n"
                         << *buffer_idx << '\n');
          }
        }
      }
    }
  }
  //erase the printf calls
  for (SmallVectorImpl<Value*>::iterator
         print_iterate = printfs.begin(),
            print_iterate_e = printfs.end();
       print_iterate != print_iterate_e;
       ++print_iterate) {
    CallInst* CI
      = dyn_cast<CallInst>( *print_iterate);
    CI->eraseFromParent();
  }
  return true;
}

bool AMDPrintfRuntimeBinding::runOnModule(Module &M) {
  if (!confirmOpenCLVersion200(M)) return false;
  if (!confirmSpirModule(M)) return false;
  if (cpuLowering) {
    bool ret = false;
    collectPrintfsFromModule(M);
    ret = lowerPrintfForCpu(M);
    return ret;
  } else {
    bool ret = false;
    collectPrintfsFromModule(M);
    ret = lowerPrintfForGpu(M);
    return ret;
  }
}

const char* AMDPrintfRuntimeBinding::getPassName() const {
  return "AMD Printf lowering part 1";
}

bool AMDPrintfRuntimeBinding::doInitialization(Module &M) {
  return false;
}

bool AMDPrintfRuntimeBinding::doFinalization(Module &M) {
  return false;
}
