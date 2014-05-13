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
#include "AMDILMachineFunctionInfo.h"
#include "AMDILModuleInfo.h"
#include "AMDILCompilerErrors.h"
#include "AMDILModuleInfo.h"
#include "AMDILSubtarget.h"
#include "AMDILTargetMachine.h"
#include "AMDILUtilityFunctions.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/Support/FormattedStream.h"
#include <cstdio>
#include <ostream>
#include <algorithm>
#include <string>
#include <queue>
#include <list>
#include <utility>
using namespace llvm;

static const AMDILConstPtr *getConstPtr(const AMDILKernel *Kernel,
                                        const std::string &Arg) {
  if (!Kernel) {
    return NULL;
  }

  llvm::SmallVector<AMDILConstPtr, DEFAULT_VEC_SLOTS>::const_iterator I, E;
  for (I = Kernel->constPtr.begin(), E = Kernel->constPtr.end();
       I != E; ++I) {
    const AMDILConstPtr &ConstPtr = *I;
    if (ConstPtr.name == Arg) {
      return &ConstPtr;
    }
  }
  return NULL;
}

void PrintfInfo::addOperand(size_t Idx, uint32_t Size) {
  mOperands.resize((unsigned)(Idx + 1));
  mOperands[(unsigned)Idx] = Size;
}

uint32_t PrintfInfo::getPrintfID() {
  return mPrintfID;
}

void PrintfInfo::setPrintfID(uint32_t ID) {
  mPrintfID = ID;
}

size_t PrintfInfo::getNumOperands() {
  return mOperands.size();
}

uint32_t PrintfInfo::getOperandID(uint32_t idx) {
  return mOperands[idx];
}

AMDILMachineFunctionInfo::AMDILMachineFunctionInfo()
  : CalleeSavedFrameSize(0), BytesToPopOnReturn(0),
  DecorationStyle(None), ReturnAddrIndex(0),
  TailCallReturnAddrDelta(0), SRetReturnReg(0), mArgSize(-1), mScratchSize(-1),
  mStackSize(-1), mArgNumVecRegs(-1), mRetNumVecRegs(-1),
  mLits(), mReservedLDS(0), mSamplerMap(),
  mValueIDMap(), mHasOutputInst(false), mKernel(NULL),
  mMF(NULL), mAMI(NULL), mSTM(NULL),
  UsesLDS(false), LDSArg(false),
  UsesGDS(false), GDSArg(false),
  UsesScratch(false), ScratchArg(false),
  UsesConstant(false), ConstantArg(false) {
  memset(mUsedMem, 0, sizeof(mUsedMem));
}

AMDILMachineFunctionInfo::AMDILMachineFunctionInfo(MachineFunction &MF)
  : CalleeSavedFrameSize(0), BytesToPopOnReturn(0),
  DecorationStyle(None), ReturnAddrIndex(0),
  TailCallReturnAddrDelta(0), SRetReturnReg(0), mArgSize(-1), mScratchSize(-1),
  mStackSize(-1), mArgNumVecRegs(-1), mRetNumVecRegs(-1),
  mLits(), mReservedLDS(0), mSamplerMap(),
  mValueIDMap(), mHasOutputInst(false), mMF(&MF),
  UsesLDS(false), LDSArg(false),
  UsesGDS(false), GDSArg(false),
  UsesScratch(false), ScratchArg(false),
  UsesConstant(false), ConstantArg(false) {
  memset(mUsedMem, 0, sizeof(mUsedMem));
  const Function *F = MF.getFunction();
  MachineModuleInfo &MMI = MF.getMMI();
  const AMDILTargetMachine *TM =
      reinterpret_cast<const AMDILTargetMachine*>(&MF.getTarget());
  mAMI = &MMI.getObjFileInfo<AMDILModuleInfo>();
  mSTM = TM->getSubtargetImpl();
  mKernel = mAMI->getKernel(F->getName());
}

AMDILMachineFunctionInfo::~AMDILMachineFunctionInfo() {
  for (std::map<std::string, PrintfInfo*>::iterator I = printf_begin(),
      E = printf_end(); I != E; ++I) {
    delete I->second;
  }
}

unsigned AMDILMachineFunctionInfo::getCalleeSavedFrameSize() const {
  return CalleeSavedFrameSize;
}

void AMDILMachineFunctionInfo::setCalleeSavedFrameSize(unsigned Bytes) {
  CalleeSavedFrameSize = Bytes;
}

unsigned  AMDILMachineFunctionInfo::getBytesToPopOnReturn() const {
  return BytesToPopOnReturn;
}

void AMDILMachineFunctionInfo::setBytesToPopOnReturn(unsigned Bytes) {
  BytesToPopOnReturn = Bytes;
}

NameDecorationStyle AMDILMachineFunctionInfo::getDecorationStyle() const {
  return DecorationStyle;
}

void AMDILMachineFunctionInfo::setDecorationStyle(NameDecorationStyle Style) {
  DecorationStyle = Style;
}

int AMDILMachineFunctionInfo::getRAIndex() const {
  return ReturnAddrIndex;
}

void AMDILMachineFunctionInfo::setRAIndex(int Index) {
  ReturnAddrIndex = Index;
}

int AMDILMachineFunctionInfo::getTCReturnAddrDelta() const {
  return TailCallReturnAddrDelta;
}

void AMDILMachineFunctionInfo::setTCReturnAddrDelta(int Delta) {
  TailCallReturnAddrDelta = Delta;
}

unsigned AMDILMachineFunctionInfo::getSRetReturnReg() const {
  return SRetReturnReg;
}

void AMDILMachineFunctionInfo::setSRetReturnReg(unsigned Reg) {
  SRetReturnReg = Reg;
}

bool AMDILMachineFunctionInfo::usesHWConstant(const std::string &Name) const {
  const AMDILConstPtr *CurConst = getConstPtr(mKernel, Name);
  return CurConst ? CurConst->usesHardware : false;
}

bool AMDILMachineFunctionInfo::isKernel() const {
  return mKernel != NULL && mKernel->mKernel;
}

AMDILKernel *AMDILMachineFunctionInfo::getKernel() {
  return mKernel;
}

StringRef AMDILMachineFunctionInfo::getName() {
  return mMF ? mMF->getFunction()->getName() : "";
}

uint32_t AMDILMachineFunctionInfo::getArgSize() {
  if (mArgSize == -1) {
    const AMDILTargetMachine *TM =
      reinterpret_cast<const AMDILTargetMachine*>(&mMF->getTarget());
    Function::const_arg_iterator I = mMF->getFunction()->arg_begin();
    Function::const_arg_iterator Ie = mMF->getFunction()->arg_end();
    uint32_t Counter = 0;
    while (I != Ie) {
      Type* curType = I->getType();
      if (curType->isIntegerTy() || curType->isFloatingPointTy()) {
        ++Counter;
      } else if (const VectorType *VT = dyn_cast<VectorType>(curType)) {
        Type *ET = VT->getElementType();
        int numEle = VT->getNumElements();
        switch (ET->getPrimitiveSizeInBits()) {
          default:
            if (numEle == 3) {
              Counter++;
            } else {
              Counter += ((numEle + 2) >> 2);
            }
            break;
          case 64:
            if (numEle == 3) {
              Counter += 2;
            } else {
              Counter += (numEle >> 1);
            }
            break;
          case 16:
          case 8:
            switch (numEle) {
              default:
                Counter += ((numEle + 2) >> 2);
              case 2:
                Counter++;
                break;
            }
            break;
        }
      } else if (const PointerType *PT = dyn_cast<PointerType>(curType)) {
        Type *CT = PT->getElementType();
        const StructType *ST = dyn_cast<StructType>(CT);
        if (ST && ST->isOpaque()) {
          bool i1d  = ST->getName().startswith("struct._image1d_t");
          bool i1da = ST->getName().startswith("struct._image1d_array_t");
          bool i1db = ST->getName().startswith("struct._image1d_buffer_t");
          bool i2d  = ST->getName().startswith("struct._image2d_t");
          bool i2da = ST->getName().startswith("struct._image2d_array_t");
          bool i3d  = ST->getName().startswith("struct._image3d_t");
          bool is_image = i1d || i1da || i1db || i2d || i2da || i3d;

          if (is_image) {
            if (mSTM->isSupported(AMDIL::Caps::Images)) {
              Counter += 2;
            } else {
              addErrorMsg(amd::CompilerErrorMessage[NO_IMAGE_SUPPORT]);
            }
          } else {
            Counter++;
          }
        } else if (CT->isStructTy()
            && PT->getAddressSpace() == AMDILAS::PRIVATE_ADDRESS) {
          StructType *ST = dyn_cast<StructType>(CT);
          const DataLayout* DL = TM->getDataLayout();
          Counter += DL->RoundUpAlignment(DL->getTypeAllocSize(ST), 16) >> 4;
        } else if (CT->isIntOrIntVectorTy()
            || CT->isFPOrFPVectorTy()
            || CT->isArrayTy()
            || CT->isPointerTy()
            || PT->getAddressSpace() != AMDILAS::PRIVATE_ADDRESS) {
          ++Counter;
        } else {
          llvm_unreachable("Current type is not supported!");
          addErrorMsg(amd::CompilerErrorMessage[INTERNAL_ERROR]);
        }
      } else {
        llvm_unreachable("Current type is not supported!");
        addErrorMsg(amd::CompilerErrorMessage[INTERNAL_ERROR]);
      }
      ++I;
    }
    // Convert from slots to bytes by multiplying by 16(shift by 4).
    mArgSize = Counter << 4;
  }
  return (uint32_t)mArgSize;
}

uint32_t AMDILMachineFunctionInfo::getScratchSize() {
  if (mScratchSize != -1)
    return static_cast<uint32_t>(mScratchSize);

  mScratchSize = 0;

  const Function *F = mMF->getFunction();
  const DataLayout *DL = mMF->getTarget().getDataLayout();

  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I) {
    Type *ArgTy = I->getType();
    mScratchSize += DL->RoundUpAlignment(DL->getTypeAllocSize(ArgTy), 16);
  }

  // FIXME: Doubling the size for no apparent reason. There used to be code here
  // trying to redundantly round up to 16-byte alignment, but in doing so it was
  // unnecessarily doubling it. This is hiding some unoptimized test failures.
  mScratchSize += DL->RoundUpAlignment(mScratchSize, 16);
  assert((mScratchSize % 16 == 0) &&
         "Total should have remained a multiple of alignment");

  return static_cast<uint32_t>(mScratchSize);
}

uint32_t AMDILMachineFunctionInfo::getStackSize() {
  if (mStackSize != -1)
    return static_cast<uint32_t>(mStackSize);

  const TargetMachine &TM = mMF->getTarget();
  bool addStackSize = TM.getOptLevel() == CodeGenOpt::None;
  if (!addStackSize) {
    // Add the stack size if any arguments are structs.
    const Function *F = mMF->getFunction();
    for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
         I != E; ++I) {
      Type *ArgTy = I->getType();
      // XXX - Should probably be checking for byval
      if (PointerType *PT = dyn_cast<PointerType>(ArgTy)) {
        if (PT->getAddressSpace() == AMDILAS::PRIVATE_ADDRESS &&
            PT->getElementType()->isStructTy()) {
          addStackSize = true;
          break;
        }
      }
    }
  }

  const MachineFrameInfo *MFI = mMF->getFrameInfo();
  uint32_t privSize = MFI->getOffsetAdjustment() + MFI->getStackSize();
  if (addStackSize) {
    privSize += getScratchSize();
  }

  mStackSize = DataLayout::RoundUpAlignment(privSize, 16);
  return static_cast<uint32_t>(mStackSize);
}

uint32_t AMDILMachineFunctionInfo::addi32Literal(uint32_t val, int Opcode) {
  // Since we have emulated 16/8/1 bit register types with a 32bit real
  // register, we need to sign extend the constants to 32bits in order for
  // comparisons against the constants to work correctly, this fixes some issues
  // we had in conformance failing for saturation.
  if (Opcode == AMDIL::LOADCONSTi16) {
    val = (((int32_t)val << 16) >> 16);
  } else if (Opcode == AMDIL::LOADCONSTi8) {
    val = (((int32_t)val << 24) >> 24);
  }
  uint64_t val64b = ((uint64_t)val | (uint64_t)val << 32U);
  return addLiteral(val64b, val64b);
}

uint32_t AMDILMachineFunctionInfo::addi64Literal(uint64_t Val) {
  return addLiteral(Val, Val);
}

uint32_t AMDILMachineFunctionInfo::addi128Literal(uint64_t val_lo,
                                                  uint64_t val_hi) {
  return addLiteral(val_lo, val_hi);
}

uint32_t AMDILMachineFunctionInfo::addLiteral(uint64_t val_lo,
                                              uint64_t val_hi) {
  std::pair<uint64_t, uint64_t> a;
  a.first = val_lo;
  a.second = val_hi;
  if (mLits.find(a) == mLits.end()) {
    mLits[a] = mAMI->getUniqueLiteralId();
  }
  return mLits[a];
}

uint32_t AMDILMachineFunctionInfo::addf32Literal(uint32_t val) {
  uint64_t Val64b = ((uint64_t)val | ((uint64_t)val << 32));
  return addLiteral(Val64b, Val64b);
}

uint32_t AMDILMachineFunctionInfo::addf32Literal(const ConstantFP *CFP) {
  uint32_t val = (uint32_t)CFP->getValueAPF().bitcastToAPInt().getZExtValue();
  return addf32Literal(val);
}

uint32_t AMDILMachineFunctionInfo::addf64Literal(uint64_t val) {
  return addLiteral(val, val);
}

uint32_t AMDILMachineFunctionInfo::addf64Literal(const ConstantFP *CFP) {
  union dtol_union {
    double d;
    uint64_t ul;
  } dval;
  const APFloat &APF = CFP->getValueAPF();
  if (&APF.getSemantics() == (const llvm::fltSemantics *)&APFloat::IEEEsingle) {
    float fval = APF.convertToFloat();
    dval.d = (double)fval;
  } else {
    dval.d = APF.convertToDouble();
  }
  return addLiteral(dval.ul, dval.ul);
}

uint32_t AMDILMachineFunctionInfo::getLitIdx(uint32_t Val) {
  uint64_t Val64 = ((uint64_t)Val | ((uint64_t)Val << 32));
  std::pair<uint64_t, uint64_t> ValPair(Val64, Val64);
  assert(mLits.find(ValPair) != mLits.end() && "literal not created yet");
  return mLits[ValPair];
}

uint32_t AMDILMachineFunctionInfo::getLitIdx(uint64_t Val) {
  std::pair<uint64_t, uint64_t> ValPair(Val, Val);
  assert(mLits.find(ValPair) != mLits.end() && "literal not created yet");
  return mLits[ValPair];
}

void AMDILMachineFunctionInfo::addReservedLDS(uint32_t Size) {
  mReservedLDS += Size;
}

uint32_t AMDILMachineFunctionInfo::addSampler(std::string name, uint32_t Val) {
  if (mSamplerMap.find(name) != mSamplerMap.end()) {
    SamplerInfo NewVal = mSamplerMap[name];
    NewVal.val = Val;
    mSamplerMap[name] = NewVal;
    return mSamplerMap[name].idx;
  } else {
    SamplerInfo CurVal;
    CurVal.name = name;
    CurVal.val = Val;
    CurVal.idx = mSamplerMap.size();
    mSamplerMap[name] = CurVal;
    return CurVal.idx;
  }
}

void AMDILMachineFunctionInfo::setUsesMem(unsigned ID) {
  assert(ID < AMDIL::MAX_IDS &&
      "Must set the ID to be less than MAX_IDS!");
  mUsedMem[ID] = true;
}

bool AMDILMachineFunctionInfo::usesMem(unsigned ID) {
  assert(ID < AMDIL::MAX_IDS &&
      "Must set the ID to be less than MAX_IDS!");
  return mUsedMem[ID];
}

void AMDILMachineFunctionInfo::addErrorMsg(const char *Msg, ErrorMsgEnum Val) {
  if (Val == DEBUG_ONLY) {
#if !defined(NDEBUG)
    mErrors.insert(Msg);
#endif
  } else if (Val == RELEASE_ONLY) {
#if defined(NDEBUG)
    mErrors.insert(Msg);
#endif
  } else if (Val == ALWAYS) {
    mErrors.insert(Msg);
  }
}

uint32_t AMDILMachineFunctionInfo::addPrintfString(StringRef name,
                                                   unsigned Offset) {
  if (mPrintfMap.find(name) != mPrintfMap.end()) {
    return mPrintfMap[name]->getPrintfID();
  } else {
    PrintfInfo *Info = new PrintfInfo;
    Info->setPrintfID(mPrintfMap.size() + Offset);
    mPrintfMap[name] = Info;
    return Info->getPrintfID();
  }
}

void AMDILMachineFunctionInfo::addPrintfOperand(StringRef name,
                                                size_t Idx,
                                                uint32_t Size) {
  mPrintfMap[name]->addOperand(Idx, Size);
}

void AMDILMachineFunctionInfo::addMetadata(StringRef MD,
                                           bool KernelOnly) {
  if (KernelOnly) {
    mMetadataKernel.push_back(MD.str());
  } else {
    mMetadataFunc.insert(MD.str());
  }
}

size_t AMDILMachineFunctionInfo::get_num_write_images() {
  return write_image3d_size() + write_image2d_size()
    + write_image2d_array_size() + write_image1d_array_size()
    + write_image1d_size() + write_image1d_buffer_size();
}

bool AMDILMachineFunctionInfo::isSignedIntType(const Value* ptr) {
  if (!mSTM->supportMetadata30())
    return true;
  std::string signedNames = "llvm.signedOrSignedpointee.annotations.";
  std::string argName = ptr->getName();
  if (!mMF)
    return false;

  signedNames += mMF->getFunction()->getName();
  const GlobalVariable *GV =
    mMF->getFunction()->getParent()->getGlobalVariable(signedNames);
  if (!GV || !GV->hasInitializer())
    return false;

  const ConstantArray *CA = dyn_cast<ConstantArray>(GV->getInitializer());
  if (!CA)
    return false;

  for (uint32_t start = 0, stop = CA->getNumOperands(); start < stop; ++start) {
    const ConstantExpr *nameField = dyn_cast<ConstantExpr>(CA->getOperand(start));
    if (!nameField) continue;

    const GlobalVariable *nameGV =
      dyn_cast<GlobalVariable>(nameField->getOperand(0));
    if (!nameGV || !nameGV->hasInitializer()) continue;

    const ConstantDataArray *nameArray =
      dyn_cast<ConstantDataArray>(nameGV->getInitializer());
    if (!nameArray) continue;

    std::string nameStr = nameArray->getAsString();
    // We don't want to include the newline
    if (!nameStr.compare(0, nameStr.length() - 1, argName))
      return true;
  }

  return false;
}

bool AMDILMachineFunctionInfo::isVolatilePointer(const Value* ptr) {
  if (!mSTM->supportMetadata30())
    return false;
  std::string signedNames = "llvm.volatilepointer.annotations.";
  std::string argName = ptr->getName();
  if (!mMF)
    return false;
  signedNames += mMF->getFunction()->getName();
  const GlobalVariable *GV =
    mMF->getFunction()->getParent()->getGlobalVariable(signedNames);
  if (!GV || !GV->hasInitializer())
    return false;
  const ConstantArray *CA = dyn_cast<ConstantArray>(GV->getInitializer());
  if (!CA)
    return false;
  for (uint32_t start = 0, stop = CA->getNumOperands(); start < stop; ++start) {
    const ConstantExpr *NameField
      = dyn_cast<ConstantExpr>(CA->getOperand(start));
    if (!NameField)
      continue;

    const GlobalVariable *nameGV =
      dyn_cast<GlobalVariable>(NameField->getOperand(0));
    if (!nameGV || !nameGV->hasInitializer()) continue;

    const ConstantDataArray *nameArray =
      dyn_cast<ConstantDataArray>(nameGV->getInitializer());
    if (!nameArray) continue;

    std::string nameStr = nameArray->getAsString();
    // We don't want to include the newline
    if (!nameStr.compare(0, nameStr.length()-1, argName)) return true;
  }
  return false;
}
bool
AMDILMachineFunctionInfo::isRestrictPointer(const Value* ptr)
{
  if (!mSTM->supportMetadata30()) return false;
  std::string signedNames = "llvm.restrictpointer.annotations.";
  std::string argName = ptr->getName();
  if (!mMF) return false;
  signedNames += mMF->getFunction()->getName();
  const GlobalVariable *GV =
    mMF->getFunction()->getParent()->getGlobalVariable(signedNames);
  if (!GV || !GV->hasInitializer()) return false;
  const ConstantArray *CA = dyn_cast<ConstantArray>(GV->getInitializer());
  if (!CA) return false;
  for (uint32_t start = 0, stop = CA->getNumOperands(); start < stop; ++start) {
    const ConstantExpr *NameField = dyn_cast<ConstantExpr>(CA->getOperand(start));
    if (!NameField) continue;

    const GlobalVariable *nameGV =
      dyn_cast<GlobalVariable>(NameField->getOperand(0));
    if (!nameGV || !nameGV->hasInitializer()) continue;

    const ConstantDataArray *nameArray =
      dyn_cast<ConstantDataArray>(nameGV->getInitializer());
    if (!nameArray) continue;

    std::string nameStr = nameArray->getAsString();
    // We don't want to include the newline
    if (!nameStr.compare(0, nameStr.length()-1, argName)) return true;
  }
  return false;
}

bool AMDILMachineFunctionInfo::isConstantArgument(const Value* Ptr) {
  if (!mSTM->supportMetadata30())
    return false;

  SmallString<128> SignedNames("llvm.argtypeconst.annotations.");
  StringRef ArgName = Ptr->getName();
  if (!mMF)
    return false;

  SignedNames += mMF->getFunction()->getName();
  const GlobalVariable *GV =
    mMF->getFunction()->getParent()->getGlobalVariable(SignedNames);
  if (!GV || !GV->hasInitializer())
    return false;

  const ConstantArray *CA = dyn_cast<ConstantArray>(GV->getInitializer());
  if (!CA)
    return false;

  for (uint32_t I = 0, E = CA->getNumOperands(); I < E; ++I) {
    const ConstantExpr *NameField = dyn_cast<ConstantExpr>(CA->getOperand(I));
    if (!NameField)
      continue;

    const GlobalVariable *NameGV =
      dyn_cast<GlobalVariable>(NameField->getOperand(0));
    if (!NameGV || !NameGV->hasInitializer())
      continue;

    const ConstantDataArray *NameArray =
      dyn_cast<ConstantDataArray>(NameGV->getInitializer());
    if (!NameArray)
      continue;

    // We don't want to include the newline
    StringRef NameStr = NameArray->getAsString().drop_back(1);
    if (NameStr == ArgName)
      return true;
  }
  return false;
}

// If the value is not known, then the uav is set, otherwise the mValueIDMap
// is used.
void AMDILMachineFunctionInfo::setUAVID(const Value *value, uint32_t ID) {
  assert((isa<Argument>(value) || isa<GlobalValue>(value)) &&
         "UAV ids should be a definition of an object");
  if (value) {
    mValueIDMap[value] = ID;
  }
}

uint32_t AMDILMachineFunctionInfo::getUAVID(const Value *Value) const {
  value_id_iterator I = mValueIDMap.find(Value);
  if (I != mValueIDMap.end()) {
    return I->second;
  }

  if (mSTM->getGeneration() <= AMDIL::NORTHERN_ISLANDS)
    return mSTM->getResourceID(AMDIL::ARENA_UAV_ID);
  return mSTM->getResourceID(AMDIL::RAW_UAV_ID);
}
