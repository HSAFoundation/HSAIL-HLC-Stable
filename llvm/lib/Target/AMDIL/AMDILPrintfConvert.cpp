//===-- AMDILPrintfConvert.cpp - Printf Conversion pass --===//
// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// If you use the software (in whole or in part), you shall adhere to all
// applicable U.S., European, and other export laws, including but not limited
// to the U.S. Export Administration Regulations (EAR), (15 C.F.R. Sections
// 730 through 774), and E.U. Council Regulation (EC) No 1334/2000 of 22 June
// 2000.  Further, pursuant to Section 740.6 of the EAR, you hereby certify
// that, except pursuant to a license granted by the United States Department
// of Commerce Bureau of Industry and Security or as otherwise permitted
// pursuant to a License Exception under the U.S. Export Administration
// Regulations ("EAR"), you will not (1) export, re-export or release to a
// national of a country in Country Groups D:1, E:1 or E:2 any restricted
// technology, software, or source code you receive hereunder, or (2) export to
// Country Groups D:1, E:1 or E:2 the direct product of such technology or
// software, if such foreign produced direct product is subject to national
// security controls as identified on the Commerce Control List (currently
// found in Supplement 1 to Part 774 of EAR).  For the most current Country
// Group listings, or for additional information about the EAR or your
// obligations under those regulations, please refer to the U.S. Bureau of
// Industry and Securitys website at http://www.bis.doc.gov/.
//
//==-----------------------------------------------------------------------===//

#define DEBUG_TYPE "printfconvert"
#include "llvm/Support/Debug.h"

#include "AMDILAlgorithms.tpp"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILModuleInfo.h"
#include "AMDILUtilityFunctions.h"
#include "AMDILTargetMachine.h"
#include "llvm/Instructions.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Module.h"
#include "llvm/Type.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include <cstdio>
#include <string>
using namespace llvm;
namespace
{
    class LLVM_LIBRARY_VISIBILITY AMDILPrintfConvert : public FunctionPass
    {
        public:
            TargetMachine &TM;
            static char ID;
            AMDILPrintfConvert(TargetMachine &tm, CodeGenOpt::Level OL);
            ~AMDILPrintfConvert();
            const char* getPassName() const;
            bool runOnFunction(Function &F);
            bool doInitialization(Module &M);
            bool doFinalization(Module &M);
            void getAnalysisUsage(AnalysisUsage &AU) const;
            static void getConversionSpecifiers(
              SmallVectorImpl<char> &opConvSpecifiers,
              StringRef fmt,
              size_t num_ops);

        private:
            bool expandPrintf(BasicBlock::iterator *bbb);
            AMDILMachineFunctionInfo *mMFI;
            const DataLayout* DL;
            bool mChanged;
            SmallVector<int64_t, DEFAULT_VEC_SLOTS> bVecMap;
    };
    char AMDILPrintfConvert::ID = 0;
} // anonymous namespace

namespace llvm
{
  FunctionPass *createAMDILPrintfConvert(TargetMachine &TM, CodeGenOpt::Level OL) {
    return new AMDILPrintfConvert(TM, OL);
  }
} // llvm namespace

AMDILPrintfConvert::AMDILPrintfConvert(TargetMachine &tm, CodeGenOpt::Level OL)
    : FunctionPass(ID), TM(tm), DL(NULL) {
}
AMDILPrintfConvert::~AMDILPrintfConvert()
{
}

// should the given op be printed as string?
static bool shouldPrintAsStr(char Specifier, Type* OpType)
{
  if (Specifier != 's') return false;
  const PointerType *PT = dyn_cast<PointerType>(OpType);
  if (!PT) return false;
  if (PT->getAddressSpace() != 2) return false;
  Type* ElemType = PT->getContainedType(0);
  if (ElemType->getTypeID() != Type::IntegerTyID) return false;
  IntegerType* ElemIType = cast<IntegerType>(ElemType);
  return ElemIType->getBitWidth() == 8;
}

    bool
AMDILPrintfConvert::expandPrintf(BasicBlock::iterator *bbb)
{
    Instruction *inst = (*bbb);
    CallInst *CI = dyn_cast<CallInst>(inst);
    if (!CI) {
      return false;
    }

    unsigned num_ops = CI->getNumArgOperands();
    if (num_ops == 0) {
      return false;
    }

    if (CI->getCalledFunction()->getName() != "printf") {
      return false;
    }

    SmallString<16> opConvSpecifiers;
    Function *mF = inst->getParent()->getParent();
    Module *M = mF->getParent();
    LLVMContext &Ctx = M->getContext();
    uint64_t bytes = 0;
    mChanged = true;
    if (num_ops == 1) {
        ++(*bbb);
        Constant *newConst = ConstantInt::getSigned(CI->getType(), bytes);
        CI->replaceAllUsesWith(newConst);
        CI->eraseFromParent();
        return mChanged;
    }

    Type* Int32Ty = Type::getInt32Ty(mF->getContext());
    Type* Int64Ty = Type::getInt64Ty(mF->getContext());
    // Deal with the string here
    Value *op = CI->getArgOperand(0);
    ConstantExpr *GEPinst = dyn_cast<ConstantExpr>(op);
    if (GEPinst) {
        GlobalVariable *GVar
            = dyn_cast<GlobalVariable>(GEPinst->getOperand(0));
        StringRef str("unknown");
        if (GVar && GVar->hasInitializer()) {
          ConstantDataArray *CA
            = dyn_cast<ConstantDataArray>(GVar->getInitializer());
          if (CA->isString()) {
            str = CA->getAsString();
          }
          getConversionSpecifiers(opConvSpecifiers,
                                  str,
                                  num_ops - 1);

          /*
          if (num_ops - 1 != opConvSpecifiers.size()) {
            mMFI->addErrorMsg(
              "Format specifier count does not match number of arguments");
          }
          */
        }
        uint64_t id = (uint64_t)mMFI->addPrintfString(str,
            getAnalysis<MachineFunctionAnalysis>().getMF()
            .getMMI().getObjFileInfo<AMDILModuleInfo>().get_printf_offset());
        StringRef name("___dumpStringID");

        Type *Tys[1] = { Int32Ty };
        Attributes::AttrVal AVs[1] = { Attributes::NoUnwind };
        AttributeWithIndex AWI[1] = {
          AttributeWithIndex::get(Ctx, AttrListPtr::FunctionIndex, AVs)
        };

        FunctionType *DumpStringFTy
          = FunctionType::get(Type::getVoidTy(Ctx), Tys, false);
        Constant *nF = M->getOrInsertFunction(name, DumpStringFTy,
                                              AttrListPtr::get(Ctx, AWI));
        Constant *C = ConstantInt::get(Int32Ty, id, false);
        CallInst *nCI = CallInst::Create(nF, C, "", CI);
        bytes = str.size();
        for (uint32_t x = 1, y = num_ops; x < y; ++x) {
            Type *oType = CI->getOperand(x)->getType();
            uint64_t Size = DL->getTypeAllocSizeInBits(oType);
            assert(Size && "empty size");
            mMFI->addPrintfOperand(str, x - 1, (uint32_t)Size);
        }
    }

    for (uint32_t x = 1, y = num_ops; x < y; ++x) {
        op = CI->getArgOperand(x);
        Type *oType = op->getType();
        uint64_t Size = 0;
        char buffer[256];
        buffer[0] = '\0';
        if (oType->isFPOrFPVectorTy()
                && (oType->getTypeID() != Type::VectorTyID)) {
            Type *iType = (oType->isFloatTy()) ?  Int32Ty : Int64Ty;
            op = new BitCastInst(op, iType, "printfBitCast", CI);
        } else if (oType->getTypeID() == Type::PointerTyID) {
          if (shouldPrintAsStr(opConvSpecifiers[x - 1], oType)) {
            sprintf(buffer, "___dumpBytes_v1bs");
          }
          else {
            uint64_t Size = DL->getTypeAllocSizeInBits(oType);
            assert((Size == 32 || Size == 64) && "unsupported size");
            Type* DstType = (Size == 32) ? Int32Ty : Int64Ty;
            op = new PtrToIntInst(op, DstType, "printfPtrCast", CI);
          }
        } else if (VectorType *VT = dyn_cast<VectorType>(oType)) {
            Type *iType = nullptr;
            uint32_t eleCount = VT->getVectorNumElements();
            uint32_t eleSize = VT->getScalarSizeInBits();
            uint32_t totalSize = eleCount * eleSize;
            if (eleCount == 3) {
                IntegerType *int32ty = Type::getInt32Ty(oType->getContext());
                Constant* indices[4] = {
                    ConstantInt::get(int32ty, 0),
                    ConstantInt::get(int32ty, 1),
                    ConstantInt::get(int32ty, 2),
                    ConstantInt::get(int32ty, 2)
                };
                Constant* mask = ConstantVector::get(indices);
                ShuffleVectorInst* shuffle = new ShuffleVectorInst(op, op, mask);
                shuffle->insertBefore(CI);
                op = shuffle;
                oType = op->getType();
                totalSize += eleSize;
            }
            switch (eleSize) {
                default:
                    eleCount = totalSize / 64;
                    iType = dyn_cast<Type>(
                            Type::getInt64Ty(oType->getContext()));
                    break;
                case 8:
                    if (eleCount >= 8) {
                        eleCount = totalSize / 64;
                        iType = dyn_cast<Type>(
                                Type::getInt64Ty(oType->getContext()));
                    } else if (eleCount >= 3) {
                        eleCount = 1;
                        iType = dyn_cast<Type>(
                                Type::getInt32Ty(oType->getContext()));
                    } else {
                        eleCount = 1;
                        iType = dyn_cast<Type>(
                                Type::getInt16Ty(oType->getContext()));
                    }
                    break;
                case 16:
                    if (eleCount >= 3) {
                        eleCount = totalSize / 64;
                        iType = dyn_cast<Type>(
                                Type::getInt64Ty(oType->getContext()));
                    } else {
                        eleCount = 1;
                        iType = dyn_cast<Type>(
                                Type::getInt32Ty(oType->getContext()));
                    }
                    break;
            }
            if (eleCount > 1) {
                iType = dyn_cast<Type>(
                    VectorType::get(iType, eleCount));
            }
            op = new BitCastInst(op, iType, "printfBitCast", CI);
        }
        if (buffer[0] == '\0') {
          Size = DL->getTypeAllocSizeInBits(oType);
          assert(Size && "empty size");
          sprintf(buffer, "___dumpBytes_v1b%u", (uint32_t)Size);
        }

        StringRef name(buffer);
        Type *Tys[1] = { op->getType() };
        Attributes::AttrVal AVs[1] = { Attributes::NoUnwind };
        AttributeWithIndex AWI[1] = {
          AttributeWithIndex::get(Ctx, AttrListPtr::FunctionIndex, AVs)
        };

        FunctionType *FTy = FunctionType::get(Int32Ty, Tys, false);
        Constant *nF
          = M->getOrInsertFunction(name, FTy, AttrListPtr::get(Ctx, AWI));
        CallInst *nCI = CallInst::Create(nF, op, "", CI);
        bytes += (Size >> 3);
    }

    ++(*bbb);
    Constant *newConst = ConstantInt::getSigned(CI->getType(), bytes);
    CI->replaceAllUsesWith(newConst);
    CI->eraseFromParent();
    return mChanged;
}
    bool
AMDILPrintfConvert::runOnFunction(Function &MF)
{
    mChanged = false;
    mMFI = getAnalysis<MachineFunctionAnalysis>().getMF()
          .getInfo<AMDILMachineFunctionInfo>();
    DL = getAnalysisIfAvailable<DataLayout>();
    bVecMap.clear();
    safeNestedForEach(MF.begin(), MF.end(), MF.begin()->begin(),
            std::bind1st(
                std::mem_fun(
                    &AMDILPrintfConvert::expandPrintf), this));
    return mChanged;
}

const char*
AMDILPrintfConvert::getPassName() const
{
    return "AMDIL Printf Conversion Pass";
}
bool
AMDILPrintfConvert::doInitialization(Module &M)
{
    return false;
}

bool
AMDILPrintfConvert::doFinalization(Module &M)
{
    return false;
}

void
AMDILPrintfConvert::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.addRequired<MachineFunctionAnalysis>();
  FunctionPass::getAnalysisUsage(AU);
  AU.setPreservesAll();
}

void AMDILPrintfConvert::getConversionSpecifiers(
  SmallVectorImpl<char> &OpConvSpecifiers,
  StringRef Fmt,
  size_t NumOps) {
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
