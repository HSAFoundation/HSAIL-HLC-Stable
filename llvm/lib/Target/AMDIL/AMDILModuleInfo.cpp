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
#include "AMDILModuleInfo.h"
#include "AMDILAlgorithms.tpp"
#include "AMDILAsmPrinter.h"
#include "AMDILKernel.h"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILSubtarget.h"
#include "AMDILUtilityFunctions.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/Support/FormattedStream.h"
#include <cstdio>

using namespace llvm;

static inline uint32_t AlignDown_32(uint32_t Value, uint32_t Alignment) {
  return Value & ~(Alignment - 1);
}

static inline uint32_t AlignUp_32(uint32_t Value, uint32_t Alignment) {
  return AlignDown_32(Value + Alignment - 1, Alignment);
}

AMDILModuleInfo::AMDILModuleInfo(const MachineModuleInfo &MMI)
    : mMMI(&MMI),
      mKernelCallInitialized(false),
      SymTab(NULL),
      mSTM(NULL),
      TM(NULL),
      mReservedBuffs(0),
      mCurrentCPOffset(0),
      mPrintfOffset(0),
      mNumLocalBuffers(1),
      mNumRegionBuffers(1),
      mProcessed(false),
      mNextLiteralId(11),
      mPtrEqSet(NULL) {}

AMDILModuleInfo::~AMDILModuleInfo() {
  for (StringMap<AMDILKernel *>::iterator I = mKernels.begin(),
         E = mKernels.end(); I != E; ++I) {
    StringMapEntry<AMDILKernel *> Cur = *I;
    AMDILKernel *Ptr = Cur.getValue();
    delete Ptr;
  }
  delete mPtrEqSet;
}

static const AMDILConstPtr *getConstPtr(const AMDILKernel *Kernel,
                                        StringRef Arg,
                                        StringMap<AMDILConstPtr> *ConstMems) {
  if (!Kernel) {
    if (ConstMems) {
      for (StringMap<AMDILConstPtr>::iterator cpb = ConstMems->begin(),
          cpe = ConstMems->end(); cpb != cpe; ++cpb) {
        const AMDILConstPtr &c = cpb->second;
        if (Arg == StringRef(c.name)) {
          return &c;
        }
      }
    }
    return NULL;
  }

  for (ArrayRef<AMDILConstPtr>::const_iterator I = Kernel->constPtr.begin(),
         E = Kernel->constPtr.end(); I != E; ++I) {
    const AMDILConstPtr &ACP = *I;
    if (Arg == StringRef(ACP.name)) {
      return &ACP;
    }
  }
  return NULL;
}

void AMDILModuleInfo::processModule(const Module *M,
                                    const AMDILTargetMachine *mTM) {
  Module::const_global_iterator GI;
  Module::const_global_iterator GE;
  mSTM = mTM->getSubtargetImpl();
  TM = mTM;
  if (mProcessed) {
    return;
  }

  for (GI = M->global_begin(), GE = M->global_end(); GI != GE; ++GI) {
    const GlobalValue *GV = GI;
    llvm::StringRef Name = GV->getName();

    if (Name.startswith("sgv")) {
      mKernelArgs[Name] = parseSGV(GV);
    } else if (Name.startswith("fgv")) {
      // We can ignore this since we don't care about the filename
      // string
    } else if (!Name.empty() && (Name.front() == 'l' || Name.front() == 'r')
               && Name.drop_front(1).startswith("vgv")) {
      // "lvgv" or "rvgv"
      mLocalArgs[Name] = parseXVGV(GV);
    } else if (Name.startswith("llvm.image.annotations")) {
      parseImageAnnotate(GV);
    } else if (Name.startswith("llvm.global.annotations")) {
      parseGlobalAnnotate(GV);
    } else if (Name.startswith("llvm.constpointer.annotations")) {
      parseConstantPtrAnnotate(GV);
    } else if (Name.startswith("llvm.sampler.annotations")) {
      parseSamplerAnnotate(GV);
    } else if (Name.startswith("llvm.argtypename.annotations")) {
      parseIgnoredGlobal(GV);
    } else if (Name.startswith("llvm.argtypeconst.annotations")) {
      parseIgnoredGlobal(GV);
    } else if (Name.startswith("llvm.readonlypointer.annotations")) {
      parseIgnoredGlobal(GV);
    } else if (Name.startswith("llvm.signedOrSignedpointee.annotations")) {
      parseIgnoredGlobal(GV);
    } else if (Name.startswith("llvm.restrictpointer.annotations")) {
      parseIgnoredGlobal(GV);
    } else if (Name.startswith("llvm.volatilepointer.annotations")) {
      parseIgnoredGlobal(GV);
#ifdef USE_APPLE
    } else if (GV->getType()->getAddressSpace() == 3) { // *** Match cl_kernel.h local AS #
#else
    } else if (Name.find("cllocal") != StringRef::npos) {
#endif
      parseAutoArray(GV, false);
    } else if (Name.find("clregion") != StringRef::npos) {
      parseAutoArray(GV, true);
    } else if (!GV->use_empty()
               && mIgnoreStr.find(Name) == mIgnoreStr.end()) {
      parseConstantPtr(GV);
    }
  }

  allocateGlobalCB();

  safeForEach(M->begin(), M->end(),
      std::bind1st(
        std::mem_fun(&AMDILModuleInfo::checkConstPtrsUseHW),
        this));
  // Make sure we only process the module once even though this function
  // is called everytime a MachineFunctionInfo object is instantiated.
  mProcessed = true;
}

void AMDILModuleInfo::allocateGlobalCB(void) {
  uint32_t maxCBSize = mSTM->getMaxCBSize();
  uint32_t offset = 0;
  uint32_t curCB = 0;
  uint32_t swoffset = 0;
  for (StringMap<AMDILConstPtr>::iterator cpb = mConstMems.begin(),
       cpe = mConstMems.end(); cpb != cpe; ++cpb) {
    bool constHW = mSTM->usesHardware(AMDIL::Caps::ConstantMem);
    AMDILConstPtr &c = cpb->second;
    c.usesHardware = false;
    if (constHW) {
      // If we have a limit on the max CB Size, then we need to make sure that
      // the constant sizes fall within the limits.
      if (c.size <= maxCBSize) {
        offset = AlignUp_32(offset, c.align);
        if (offset + c.size > maxCBSize) {
          offset = 0;
          curCB++;
        }
        if (curCB < mSTM->getMaxNumCBs()) {
          c.cbNum = curCB + CB_BASE_OFFSET;
          c.offset = offset;
          offset += c.size;
          c.usesHardware = true;
          continue;
        }
      }
    }
    swoffset = AlignUp_32(swoffset, c.align);
    c.cbNum = 0;
    c.offset = swoffset;
    swoffset += c.size;
  }
  if (!mConstMems.empty()) {
    mReservedBuffs = curCB + 1;
  }
}

bool AMDILModuleInfo::checkConstPtrsUseHW(llvm::Module::const_iterator *FCI) {
  Function::const_arg_iterator AI, AE;
  const Function *Func = *FCI;
  StringRef Name = Func->getName();
  AMDILKernel *Kernel = mKernels[Name];
  if (!Kernel || !Kernel->mKernel) {
    return false;
  }
  if (mSTM->usesHardware(AMDIL::Caps::ConstantMem)) {
    for (AI = Func->arg_begin(), AE = Func->arg_end();
         AI != AE; ++AI) {
      const Argument *Arg = &(*AI);
      const PointerType *P = dyn_cast<PointerType>(Arg->getType());
      if (!P) {
        continue;
      }
      if (P->getAddressSpace() != AMDILAS::CONSTANT_ADDRESS) {
        continue;
      }
      const AMDILConstPtr *Ptr = getConstPtr(Kernel, Arg->getName(), NULL);
      if (Ptr) {
        continue;
      }
      AMDILConstPtr ConstAttr;
      ConstAttr.name = Arg->getName();
      ConstAttr.size = this->mSTM->getMaxCBSize();
      ConstAttr.base = Arg;
      ConstAttr.isArgument = true;
      ConstAttr.isArray = false;
      ConstAttr.offset = 0;
      ConstAttr.align = 16;
      ConstAttr.usesHardware =
        mSTM->usesHardware(AMDIL::Caps::ConstantMem);
      if (ConstAttr.usesHardware) {
        ConstAttr.cbNum = Kernel->constPtr.size() + 2;
      } else {
        ConstAttr.cbNum = 0;
      }
      Kernel->constPtr.push_back(ConstAttr);
    }
  }
  // Now lets make sure that only the N largest buffers
  // get allocated in hardware if we have too many buffers
  uint32_t NumPtrs = Kernel->constPtr.size();
  if (NumPtrs > (this->mSTM->getMaxNumCBs() - mReservedBuffs)) {
    // TODO: Change this routine so it sorts
    // AMDILConstPtr instead of pulling the sizes out
    // and then grab the N largest and disable the rest
    llvm::SmallVector<uint32_t, 16> sizes;
    for (uint32_t x = 0; x < NumPtrs; ++x) {
      sizes.push_back(Kernel->constPtr[x].size);
    }
    std::sort(sizes.begin(), sizes.end());
    uint32_t NumToDisable = NumPtrs - (mSTM->getMaxNumCBs() -
                                       mReservedBuffs);
    uint32_t SafeSize = sizes[NumToDisable - 1];
    for (uint32_t I = 0; I < NumPtrs && NumToDisable; ++I) {
      if (Kernel->constPtr[I].size <= SafeSize) {
        Kernel->constPtr[I].usesHardware = false;
        --NumToDisable;
      }
    }
  }
  // Renumber all of the valid CB's so that
  // they are linear increase
  uint32_t CBid = 2 + mReservedBuffs;
  for (uint32_t I = 0; I < NumPtrs; ++I) {
    if (Kernel->constPtr[I].usesHardware) {
      Kernel->constPtr[I].cbNum = CBid++;
    }
  }
  for (StringMap<AMDILConstPtr>::iterator cpb = mConstMems.begin(),
       cpe = mConstMems.end(); cpb != cpe; ++cpb) {
    if (cpb->second.usesHardware) {
      Kernel->constPtr.push_back(cpb->second);
    }
  }
  for (uint32_t x = 0; x < Kernel->constPtr.size(); ++x) {
    AMDILConstPtr &c = Kernel->constPtr[x];
    uint32_t cbNum = c.cbNum - CB_BASE_OFFSET;
    if (cbNum < HW_MAX_NUM_CB && c.cbNum >= CB_BASE_OFFSET) {
      if ((c.size + c.offset) > Kernel->constSizes[cbNum]) {
        Kernel->constSizes[cbNum] = AlignUp_32(c.size + c.offset, 16);
      }
    } else {
      Kernel->constPtr[x].usesHardware = false;
    }
  }
  return false;
}

int32_t AMDILModuleInfo::getArrayOffset(StringRef A) const {
  StringMap<AMDILArrayMem>::const_iterator It = mArrayMems.find(A);
  return It == mArrayMems.end() ? -1 : It->second.offset;
}

int32_t AMDILModuleInfo::getConstOffset(StringRef A) const {
  StringMap<AMDILConstPtr>::const_iterator It = mConstMems.find(A);
  return It == mConstMems.end() ? -1 : It->second.offset;
}

bool AMDILModuleInfo::getConstHWBit(StringRef Name) const {
  StringMap<AMDILConstPtr>::const_iterator It = mConstMems.find(Name);
  return It == mConstMems.end() ? false : It->second.usesHardware;
}

const Value *AMDILModuleInfo::getConstBase(StringRef A) const {
  StringMap<AMDILConstPtr>::const_iterator It = mConstMems.find(A);
  return It == mConstMems.end() ? NULL : It->second.base;
}

void AMDILModuleInfo::parseGroupSize(uint32_t GroupSize[3], StringRef Str) {
  SmallVector<StringRef, 3> Parts;
  Str.split(Parts, ",", 3, false);
  for (unsigned I = 0, N = Parts.size(); I < std::min(3u, N); ++I) {
    Parts[I].getAsInteger(10, GroupSize[I]);
  }
}

// As of right now we only care about the required group size
// so we can skip the variable encoding
AMDILKernelAttr AMDILModuleInfo::parseSGV(const GlobalValue *G) {
  AMDILKernelAttr nArg;
  const GlobalVariable *GV = dyn_cast<GlobalVariable>(G);
  for (int x = 0; x < 3; ++x) {
    nArg.reqGroupSize[x] = mSTM->getDefaultSize(x);
    nArg.reqRegionSize[x] = mSTM->getDefaultSize(x);
    nArg.groupSizeHint[x] = mSTM->getDefaultSize(x);
  }
  nArg.mHasRWG = false;
  nArg.mHasRWR = false;
  nArg.mHasWGH = false;
  nArg.mHasVTH = false;

  if (!GV || !GV->hasInitializer()) {
    return nArg;
  }
  const Constant *CV = GV->getInitializer();
  const ConstantDataArray *CA = dyn_cast_or_null<ConstantDataArray>(CV);
  if (!CA || !CA->isString()) {
    return nArg;
  }

  StringRef init = CA->getAsCString();
  size_t pos = init.find("RWG");
  if (pos != llvm::StringRef::npos) {
    parseGroupSize(nArg.reqGroupSize, init.drop_front(pos + 3));
    nArg.mHasRWG = true;
  }

  pos = init.find("RWR");
  if (pos != llvm::StringRef::npos) {
    parseGroupSize(nArg.reqRegionSize, init.drop_front(pos + 3));
    nArg.mHasRWR = true;
  }

  pos = init.find("WGH");
  if (pos != llvm::StringRef::npos) {
    pos += 3;
    std::string WGH = init.substr(pos, init.size() - pos);
    const char *lws = WGH.c_str();
    sscanf(lws, "%d,%d,%d", &(nArg.groupSizeHint[0]),
           &(nArg.groupSizeHint[1]),
           &(nArg.groupSizeHint[2]));
    nArg.mHasWGH = true;
  }

  pos = init.find("VTH");
  if (pos != llvm::StringRef::npos) {
    pos += 3;
    nArg.vecTypeHint = init.substr(pos, init.size() - pos);
    nArg.mHasVTH = true;
  }
  return nArg;
}

AMDILLocalArg AMDILModuleInfo::parseXVGV(const GlobalValue *G) {
  AMDILLocalArg nArg;
  const GlobalVariable *GV = dyn_cast<GlobalVariable>(G);
  nArg.name = "";
  if (!GV || !GV->hasInitializer()) {
    return nArg;
  }
  const ConstantArray *CA =
    dyn_cast_or_null<ConstantArray>(GV->getInitializer());
  if (!CA) {
    return nArg;
  }
  for (size_t x = 0, y = CA->getNumOperands(); x < y; ++x) {
    const Value *local = CA->getOperand(x);
    const ConstantExpr *CE = dyn_cast_or_null<ConstantExpr>(local);
    if (!CE || !CE->getNumOperands()) {
      continue;
    }
    nArg.name = (*(CE->op_begin()))->getName();
    if (mArrayMems.find(nArg.name) != mArrayMems.end()) {
      nArg.local.push_back(&(mArrayMems[nArg.name]));
    }
  }
  return nArg;
}

void AMDILModuleInfo::parseSamplerAnnotate(const GlobalValue *G) {
  const GlobalVariable *GV = dyn_cast_or_null<GlobalVariable>(G);
  const ConstantArray *CA =
    dyn_cast_or_null<ConstantArray>(GV->getInitializer());
  if (!CA) {
    return;
  }
  uint32_t numOps = CA->getNumOperands();
  for (uint32_t x = 0; x < numOps; ++x) {
    const ConstantExpr *nameField = dyn_cast<ConstantExpr>(CA->getOperand(x));
    const GlobalVariable *nameGV =
      dyn_cast<GlobalVariable>(nameField->getOperand(0));
    const ConstantDataArray *nameArray =
      dyn_cast<ConstantDataArray>(nameGV->getInitializer());
    std::string nameStr = nameArray->getAsString();
    mSamplerSet[GV->getName()].insert(nameStr.substr(0, nameStr.size()-1));
    // Lets add this string to the set of strings we should ignore processing
    mIgnoreStr.insert(nameGV->getName());
    if (mConstMems.find(nameGV->getName())
        != mConstMems.end()) {
      // If we already processesd this string as a constant, lets remove it from
      // the list of known constants.  This way we don't process unneeded data
      // and don't generate code/metadata for strings that are never used.
      mConstMems.erase(mConstMems.find(nameGV->getName()));
    }

  }
}

void AMDILModuleInfo::parseIgnoredGlobal(const GlobalValue *G) {
  const GlobalVariable *GV = dyn_cast_or_null<GlobalVariable>(G);
  if (!GV->hasInitializer())
    return;

  const ConstantArray *CA = dyn_cast<ConstantArray>(GV->getInitializer());
  if (!CA)
    return;

  for (unsigned I = 0, N = CA->getNumOperands(); I < N; ++I) {
    const ConstantExpr *NameField = dyn_cast<ConstantExpr>(CA->getOperand(I));
    const GlobalVariable *NameGV
      = dyn_cast<GlobalVariable>(NameField->getOperand(0));

    StringRef Name = NameGV->getName();
    // Let's add this string to the set of strings we should ignore processing
    mIgnoreStr.insert(Name);
    llvm::StringMap<AMDILConstPtr>::iterator It = mConstMems.find(Name);
    if (It != mConstMems.end()) {
      // If we already processesd this string as a constant, let's remove it
      // from the list of known constants.  This way we don't process unneeded
      // data and don't generate code/metadata for strings that are never used.
      mConstMems.erase(It);
    }
  }
}

std::set<std::string> *AMDILModuleInfo::getSamplerForKernel(StringRef Ref) {
  llvm::StringMap<std::set<std::string> >::iterator It = mSamplerSet.find(Ref);
  return It == mSamplerSet.end() ? NULL : &It->second;
}

void AMDILModuleInfo::parseConstantPtrAnnotate(const GlobalValue *G) {
  const GlobalVariable *GV = dyn_cast_or_null<GlobalVariable>(G);
  const ConstantArray *CA =
    dyn_cast_or_null<ConstantArray>(GV->getInitializer());
  if (!CA) {
    return;
  }
  for (uint32_t I = 0, N = CA->getNumOperands(); I < N; ++I) {
    const Value *V = CA->getOperand(I);
    const ConstantStruct *CS = dyn_cast_or_null<ConstantStruct>(V);
    if (!CS) {
      continue;
    }
    assert(CS->getNumOperands() == 2 && "There can only be 2"
           " fields, a name and size");
    const ConstantExpr *NameField = dyn_cast<ConstantExpr>(CS->getOperand(0));
    const ConstantInt *SizeField = dyn_cast<ConstantInt>(CS->getOperand(1));
    assert(NameField && "There must be a constant name field");
    assert(SizeField && "There must be a constant size field");
    const GlobalVariable *NameGV =
      dyn_cast<GlobalVariable>(NameField->getOperand(0));
    const ConstantDataArray *NameArray =
      dyn_cast<ConstantDataArray>(NameGV->getInitializer());

    StringRef Name = NameGV->getName();
    // Let's add this string to the set of strings we should ignore processing
    mIgnoreStr.insert(Name);

    llvm::StringMap<AMDILConstPtr>::iterator It = mConstMems.find(Name);
    if (It != mConstMems.end()) {
      // If we already processesd this string as a constant, lets remove it from
      // the list of known constants.  This way we don't process unneeded data
      // and don't generate code/metadata for strings that are never used.
      mConstMems.erase(It);
    } else {
      mIgnoreStr.insert(CS->getOperand(0)->getName());
    }
    AMDILConstPtr ConstAttr;
    ConstAttr.name = NameArray->getAsString();
    ConstAttr.size = AlignUp_32(SizeField->getZExtValue(), 16);
    ConstAttr.base = CS;
    ConstAttr.isArgument = true;
    ConstAttr.isArray = false;
    ConstAttr.cbNum = 0;
    ConstAttr.offset = 0;
    ConstAttr.align = 16;
    ConstAttr.usesHardware = (ConstAttr.size <= mSTM->getMaxCBSize());
    // Now that we have all our constant information,
    // lets update the AMDILKernel
    llvm::StringRef AMDILKernelName = G->getName().drop_front(30);
    AMDILKernel *K;
    if (mKernels.find(AMDILKernelName) != mKernels.end()) {
      K = mKernels[AMDILKernelName];
      K->mName = AMDILKernelName;
    } else {
      K = new AMDILKernel(AMDILKernelName, false);
    }
    ConstAttr.cbNum = K->constPtr.size() + 2;
    K->constPtr.push_back(ConstAttr);
    mKernels[AMDILKernelName] = K;
  }
}

void AMDILModuleInfo::parseImageAnnotate(const GlobalValue *G) {
  const GlobalVariable *GV = dyn_cast<GlobalVariable>(G);
  if (!GV || !GV->hasInitializer())
    return;

  const ConstantArray *CA = dyn_cast<ConstantArray>(GV->getInitializer());
  if (!CA) {
    return;
  }
  if (isa<GlobalValue>(CA)) {
    return;
  }
  uint32_t e = CA->getNumOperands();
  if (e == 0) {
    return;
  }
  AMDILKernel *K;
  llvm::StringRef Name = G->getName().drop_front(23);
  if (mKernels.find(Name) != mKernels.end()) {
    K = mKernels[Name];
    K->mName = Name;
  } else {
    K = new AMDILKernel(Name, false);
  }

  for (uint32_t i = 0; i != e; ++i) {
    const Value *V = CA->getOperand(i);
    const Constant *C = dyn_cast<Constant>(V);
    const ConstantStruct *CS = dyn_cast<ConstantStruct>(C);
    if (CS && CS->getNumOperands() == 2) {
      if (mConstMems.find(CS->getOperand(0)->getOperand(0)->getName()) !=
          mConstMems.end()) {
        // If we already processesd this string as a constant, lets remove it
        // from the list of known constants.  This way we don't process unneeded
        // data and don't generate code/metadata for strings that are never
        // used.
        mConstMems.erase(
            mConstMems.find(CS->getOperand(0)->getOperand(0)->getName()));
      } else {
        mIgnoreStr.insert(CS->getOperand(0)->getOperand(0)->getName());
      }
      const ConstantInt *CI = dyn_cast<ConstantInt>(CS->getOperand(1));
      uint32_t val = (uint32_t)CI->getZExtValue();
      if (val == 1) {
        K->readOnly.insert(i);
      } else if (val == 2) {
        K->writeOnly.insert(i);
      } else {
        llvm_unreachable("Unknown image type value!");
      }
    }
  }
  mKernels[Name] = K;
}

void AMDILModuleInfo::parseAutoArray(const GlobalValue *GV, bool isRegion) {
  const GlobalVariable *G = dyn_cast<GlobalVariable>(GV);
  AMDILArrayMem tmp;
  tmp.base = GV;
  tmp.isHW = true;
  tmp.offset = 0;

  if (G == NULL) {
    tmp.vecSize = 0;
    tmp.align = 0;
  } else {
    // dereference the pointer type because GlobalVariable is always a pointer
    // type, and we want to calculate the size of the memory that the
    // GlobalVariable pointer points to
    PointerType *pTy = dyn_cast<PointerType>(G->getType());
    assert(pTy && "Global Variable not pointer type");
    Type *ty = pTy->getElementType();
    tmp.vecSize = TM->getDataLayout()->getTypeAllocSize(ty);
    tmp.align = std::max(G->getAlignment(), 16U);
  }
  tmp.isRegion = isRegion;
  tmp.resourceID = 0;
  mArrayMems[GV->getName()] = tmp;
}

void AMDILModuleInfo::parseConstantPtr(const GlobalValue *GV) {
  const GlobalVariable *G = dyn_cast<GlobalVariable>(GV);
  AMDILConstPtr ConstAttr;
  ConstAttr.name = G->getName();
  if (G == NULL) {
    ConstAttr.size = 0;
  } else {
    // dereference the pointer type because GlobalVariable is always a pointer
    // type, and we want to calculate the size of the memory that the
    // GlobalVariable pointer points to
    PointerType *pTy = dyn_cast<PointerType>(G->getType());
    assert(pTy && "Global Variable not pointer type");
    Type *ty = pTy->getElementType();
    ConstAttr.size = TM->getDataLayout()->getTypeAllocSize(ty);
  }
  ConstAttr.base = GV;
  ConstAttr.isArgument = false;
  ConstAttr.isArray = true;
  ConstAttr.offset = 0;
  ConstAttr.align = std::max(G->getAlignment(), 16U);
  ConstAttr.cbNum = 0;
  ConstAttr.usesHardware = false;
  mConstMems[GV->getName()] = ConstAttr;
}

void AMDILModuleInfo::parseGlobalAnnotate(const GlobalValue *G) {
  const GlobalVariable *GV = dyn_cast<GlobalVariable>(G);
  if (!GV->hasInitializer()) {
    return;
  }
  const Constant *CT = GV->getInitializer();
  if (!CT || isa<GlobalValue>(CT)) {
    return;
  }
  const ConstantArray *CA = dyn_cast<ConstantArray>(CT);
  if (!CA) {
    return;
  }

  unsigned int nKernels = CA->getNumOperands();
  for (unsigned int i = 0, e = nKernels; i != e; ++i) {
    parseKernelInformation(CA->getOperand(i));
  }
}

void AMDILModuleInfo::addArrayMemSize(AMDILKernel *kernel, AMDILArrayMem *a) {
  uint32_t *curSize;
  if (a->isRegion) {
    curSize = a->isHW ? &kernel->curHWRSize : &kernel->curRSize;
  } else {
    curSize = a->isHW ? &kernel->curHWSize : &kernel->curSize;
  }

  a->offset = AlignUp_32(*curSize, a->align);
  *curSize = a->offset + a->vecSize;
}

AMDILLocalArg* AMDILModuleInfo::parseKernelLRInfo(AMDILKernel *kernel, const Constant *CV) {
  llvm::StringRef xvgvName = "";  // lvgv or rvgv

  assert(CV);

  if (CV->getNumOperands()) {
    xvgvName = (*(CV->op_begin()))->getName();
  }

  // There can be multiple local or region arrays, so we
  // need to handle each one separately

  AMDILLocalArg *ptr = NULL;
  if (mLocalArgs.find(xvgvName) != mLocalArgs.end()) {
    ptr = &mLocalArgs[xvgvName];

    llvm::SmallVector<AMDILArrayMem *, DEFAULT_VEC_SLOTS>::iterator ib, ie;
    for (ib = ptr->local.begin(), ie = ptr->local.end(); ib != ie; ++ib) {
      AMDILArrayMem *a = *ib;
      addArrayMemSize(kernel, a);

      if (!a->isRegion) {
        // by default all local arrays are allocated in the default local buffer
        a->resourceID = mSTM->getResourceID(AMDIL::LDS_ID);
      }
    }
  }

  return ptr;
}

void AMDILModuleInfo::parseKernelInformation(const Value *V) {
  if (isa<GlobalValue>(V)) {
    return;
  }
  const ConstantStruct *CS = dyn_cast_or_null<ConstantStruct>(V);
  if (!CS) {
    return;
  }
  uint32_t N = CS->getNumOperands();
  assert((N == 5 || N == 6) && "Expected 5 or 6 operands");

  // The first operand is always a pointer to the AMDILKernel.
  const Constant *CV = dyn_cast<Constant>(CS->getOperand(0));
  llvm::StringRef AMDILKernelName("");
  if (CV->getNumOperands()) {
    AMDILKernelName = (*CV->op_begin())->getName();
  }

  AMDILKernel *Kernel;
  // If we have images, then we have already created the AMDILKernel and we just
  // need to get the AMDILKernel information.
  if (mKernels.find(AMDILKernelName) != mKernels.end()) {
    Kernel = mKernels[AMDILKernelName];
    Kernel->mKernel = true;
    Kernel->mName = AMDILKernelName;
  } else {
    Kernel = new AMDILKernel(AMDILKernelName, true);
  }

  // The second operand is SGV, there can only be one so we don't need to worry
  // about parsing out multiple data points.
  CV = dyn_cast<Constant>(CS->getOperand(1));

  llvm::StringRef sgvName;
  if (CV->getNumOperands()) {
    sgvName = (*CV->op_begin())->getName();
  }

  llvm::StringMap<AMDILKernelAttr>::iterator It = mKernelArgs.find(sgvName);
  if (It != mKernelArgs.end()) {
    Kernel->sgv = &It->second;
  }

#ifdef USE_APPLE
  Function *kf = dyn_cast<Function>(CS->getOperand(0)->stripPointerCasts());
  if (kf) {
    Function::arg_iterator aib, aie;
    unsigned int j = 0;
    // This is a function, pull out the arg info string.
    Value *field1 = CS->getOperand(1)->stripPointerCasts();
    GlobalVariable *sgv = cast<GlobalVariable>(field1);
    std::string args_desc;
    if (ConstantArray *f1arr = dyn_cast<ConstantArray>(
          sgv->getInitializer())) {
      if (f1arr->isString()) {
        args_desc = f1arr->getAsString();
      }
    }
    unsigned int i = 0; // this is the image id
    for (aib = kf->arg_begin(), aie = kf->arg_end(); aib != aie; ++aib, ++j) {
      Type *T = aib->getType();
      unsigned desc = args_desc.empty() ? 0 : args_desc[j];
      if (dyn_cast<PointerType>(T)) {
        if (desc == '5' || desc == '7') {
          kernel->readOnly.insert(i++);
        } else if (desc == '6') {
          kernel->writeOnly.insert(i++);
        }
      }
    }
  }
#endif

  // The third operand is FGV, which is skipped

  // The fourth operand is LVGV
  Kernel->lvgv = parseKernelLRInfo(Kernel, dyn_cast<Constant>(CS->getOperand(3)));

  // The possibly missing (e.g. on Apple) fifth operand is RVGV
  if (N >= 5) {
    Kernel->rvgv = parseKernelLRInfo(Kernel, dyn_cast<Constant>(CS->getOperand(4)));
  }

  // The last (fifth or sixth) operand is NULL

  mKernels[AMDILKernelName] = Kernel;
}

AMDILKernel *AMDILModuleInfo::getKernel(StringRef Name) {
  StringMap<AMDILKernel*>::iterator iter = mKernels.find(Name);
  if (iter == mKernels.end()) {
    return NULL;
  } else {
    return iter->second;
  }
}

bool AMDILModuleInfo::isKernel(StringRef Name) const {
  return (mKernels.find(Name) != mKernels.end());
}

bool AMDILModuleInfo::isWriteOnlyImage(StringRef Name, uint32_t IID) const {
  const StringMap<AMDILKernel *>::const_iterator It = mKernels.find(Name);
  if (It == mKernels.end() || !It->second)
    return false;
  return It->second->writeOnly.count(IID);
}

bool AMDILModuleInfo::isReadOnlyImage(StringRef Name,
                                      uint32_t IID) const {
  const StringMap<AMDILKernel *>::const_iterator It = mKernels.find(Name);
  if (It == mKernels.end() || !It->second)
    return false;
  return It->second->readOnly.count(IID);
}

uint32_t AMDILModuleInfo::getRegion(StringRef Name,
                                    uint32_t Dim) const {
  StringMap<AMDILKernel*>::const_iterator It = mKernels.find(Name);
  if (It != mKernels.end() && It->second->sgv) {
    AMDILKernelAttr *SGV = It->second->sgv;
    switch (Dim) {
    default:
      break;
    case 0:
    case 1:
    case 2:
      return SGV->reqRegionSize[Dim];
      break;
    case 3:
      return SGV->reqRegionSize[0] *
             SGV->reqRegionSize[1] *
             SGV->reqRegionSize[2];
    };
  }
  switch (Dim) {
  default:
    return 1;
  case 3:
    return mSTM->getDefaultSize(0) *
           mSTM->getDefaultSize(1) *
           mSTM->getDefaultSize(2);
  case 2:
  case 1:
  case 0:
    return mSTM->getDefaultSize(Dim);
    break;
  };
  return 1;
}

StringMap<AMDILConstPtr>::iterator AMDILModuleInfo::consts_begin() {
  return mConstMems.begin();
}

StringMap<AMDILConstPtr>::iterator AMDILModuleInfo::consts_end() {
  return mConstMems.end();
}

bool AMDILModuleInfo::consts_empty() {
  return mConstMems.empty();
}

bool AMDILModuleInfo::byteStoreExists(StringRef S) const {
  return mByteStore.find(S) != mByteStore.end();
}

bool AMDILModuleInfo::usesHWConstant(const AMDILKernel *Kernel, StringRef Arg) {
  const AMDILConstPtr *CurConst = getConstPtr(Kernel, Arg, NULL);
  return CurConst ? CurConst->usesHardware : false;
}

uint32_t AMDILModuleInfo::getConstPtrSize(const AMDILKernel *Kernel,
                                          StringRef Arg) {
  const AMDILConstPtr *CurConst = getConstPtr(Kernel, Arg, NULL);
  return CurConst ? CurConst->size : 0;
}

uint32_t AMDILModuleInfo::getConstPtrOff(const AMDILKernel *Kernel,
                                         StringRef Arg) {
  const AMDILConstPtr *CurConst = getConstPtr(Kernel, Arg, NULL);
  return CurConst ? CurConst->offset : 0;
}

uint32_t AMDILModuleInfo::getConstPtrCB(const AMDILKernel *Kernel,
                                        StringRef Arg) {
  const AMDILConstPtr *CurConst = getConstPtr(Kernel, Arg, &mConstMems);
  return CurConst ? CurConst->cbNum : 0;
}

void AMDILModuleInfo::calculateCPOffsets(const MachineFunction *MF,
                                         AMDILKernel *Kernel) {
  const MachineConstantPool *MCP = MF->getConstantPool();
  if (!MCP) {
    return;
  }

  ArrayRef<MachineConstantPoolEntry> Consts(MCP->getConstants());
  size_t NumConsts = Consts.size();
  const DataLayout *DL = TM->getDataLayout();
  for (size_t I = 0; I < NumConsts; ++I) {
    const Constant* ConstVal = Consts[I].Val.ConstVal;
    Kernel->CPOffsets.push_back(std::make_pair(mCurrentCPOffset, ConstVal));
    // Align the size to the vector boundary
    uint32_t Alignment = 16;
    const GlobalValue *GV = dyn_cast<GlobalValue>(ConstVal);
    Type* Ty = ConstVal->getType();
    if (GV) {
      Alignment = std::max(GV->getAlignment(), 16U);
      // dereference the pointer type because GlobalVariable is always a pointer
      // type, and we want to calculate the size of the memory that the
      // GlobalVariable pointer points to
      PointerType *PTy = dyn_cast<PointerType>(Ty);
      assert(PTy && "GlovalVariable not pointer type");
      Ty = PTy->getElementType();
    }
    size_t CurSize = DL->getTypeAllocSize(Ty);
    CurSize = AlignUp_32(CurSize, Alignment);
    mCurrentCPOffset += CurSize;
  }
}

bool AMDILModuleInfo::isConstPtrArray(const AMDILKernel *Kernel,
                                      StringRef Arg) {
  const AMDILConstPtr *CurConst = getConstPtr(Kernel, Arg, NULL);
  return CurConst ? CurConst->isArray : false;
}

bool AMDILModuleInfo::isConstPtrArgument(const AMDILKernel *Kernel,
                                         StringRef Arg) {
  const AMDILConstPtr *CurConst = getConstPtr(Kernel, Arg, NULL);
  return CurConst ? CurConst->isArgument : false;
}

const Value *AMDILModuleInfo::getConstPtrValue(const AMDILKernel *Kernel,
                                               StringRef Arg) {
  const AMDILConstPtr *CurConst = getConstPtr(Kernel, Arg, NULL);
  return CurConst ? CurConst->base : NULL;
}

static size_t dumpZeroElements(StructType *const T, raw_ostream &O,
                               bool AsBytes);
static size_t dumpZeroElements(IntegerType *const T, raw_ostream &O,
                               bool AsBytes);
static size_t dumpZeroElements(ArrayType *const T, raw_ostream &O,
                               bool AsBytes);
static size_t dumpZeroElements(VectorType *const T, raw_ostream &O,
                               bool AsBytes);
static size_t dumpZeroElements(Type *const T, raw_ostream &O, bool AsBytes);

// Return # of bytes dumped if AsBytes is true.
// Otherwise, return # of elements dumped.
size_t dumpZeroElements(Type * const T, raw_ostream &O, bool AsBytes) {
  if (!T) {
    return 0;
  }
  switch(T->getTypeID()) {
  case Type::X86_FP80TyID:
  case Type::FP128TyID:
  case Type::PPC_FP128TyID:
  case Type::LabelTyID:
    llvm_unreachable("These types are not supported by this backend");
  default:
  case Type::DoubleTyID:
    if (AsBytes) {
      O << ":0:0:0:0:0:0:0:0";
      return 8;
    }
    O << ":0";
    return 1;
  case Type::FloatTyID:
  case Type::PointerTyID:
  case Type::FunctionTyID:
    if (AsBytes) {
      O << ":0:0:0:0";
      return 4;
    }
    O << ":0";
    return 1;
  case Type::IntegerTyID:
    return dumpZeroElements(dyn_cast<IntegerType>(T), O, AsBytes);
  case Type::StructTyID:
    {
      const StructType *ST = cast<StructType>(T);
      if (!ST->isOpaque()) {
        return dumpZeroElements(dyn_cast<StructType>(T), O, AsBytes);
      }
    }

    // Opaque type
    if (AsBytes) {
      O << ":0:0:0:0";
      return 4;
    }
    O << ":0";
    return 1;
  case Type::ArrayTyID:
    return dumpZeroElements(dyn_cast<ArrayType>(T), O, AsBytes);
  case Type::VectorTyID:
    return dumpZeroElements(dyn_cast<VectorType>(T), O, AsBytes);
  };
}

// Return # of bytes dumped if AsBytes is true.
// Otherwise, return # of elements dumped.
size_t dumpZeroElements(StructType * const ST, raw_ostream &O, bool AsBytes) {
  if (!ST) {
    return 0;
  }
  Type *curType;
  size_t dumpSize = 0;
  StructType::element_iterator eib = ST->element_begin();
  StructType::element_iterator eie = ST->element_end();
  for (;eib != eie; ++eib) {
    curType = *eib;
    dumpSize += dumpZeroElements(curType, O, AsBytes);
  }
  return dumpSize;
}

// Return # of bytes dumped if AsBytes is true.
// Otherwise, return # of elements dumped.
size_t dumpZeroElements(IntegerType * const IT, raw_ostream &O, bool AsBytes) {
  if (AsBytes) {
    unsigned byteWidth = (IT->getBitWidth() >> 3);
    for (unsigned x = 0; x < byteWidth; ++x) {
      O << ":0";
    }
    return byteWidth;
  }
  O << ":0";
  return 1;
}

// Return # of bytes dumped if AsBytes is true.
// Otherwise, return # of elements dumped.
size_t dumpZeroElements(ArrayType * const AT, raw_ostream &O, bool AsBytes) {
  size_t n = AT->getNumElements();
  size_t dumpSize = 0;
  for (size_t x = 0; x < n; ++x) {
    dumpSize += dumpZeroElements(AT->getElementType(), O, AsBytes);
  }
  return dumpSize;
}

// Return # of bytes dumped if AsBytes is true.
// Otherwise, return # of elements dumped.
size_t dumpZeroElements(VectorType * const VT, raw_ostream &O, bool AsBytes) {
  size_t n = VT->getNumElements();
  size_t dumpSize = 0;
  size_t x = 0;
  for (; x < n; ++x) {
    dumpSize += dumpZeroElements(VT->getElementType(), O, AsBytes);
  }
  for (; x < (size_t)RoundUpToAlignment(n, 2); ++x) {
    dumpSize += dumpZeroElements(VT->getElementType(), O, AsBytes);
  }
  return dumpSize;
}

// Return # of bytes dumped if AsBytes is true.
// Otherwise, return # of elements dumped.
size_t AMDILModuleInfo::printConstantValue(const Constant *CAval,
                                           raw_ostream &O, bool AsBytes) {
  if (const ConstantFP *CFP = dyn_cast<ConstantFP>(CAval)) {
    bool isDouble = &CFP->getValueAPF().getSemantics()==&APFloat::IEEEdouble;
    if (isDouble) {
      double val = CFP->getValueAPF().convertToDouble();
      union dtol_union {
        double d;
        uint64_t l;
        char c[8];
      } conv;
      conv.d = val;
      if (!AsBytes) {
        O << ":";
        O.write_hex(conv.l);
        return 1;
      }
      for (int i = 0; i < 8; ++i) {
        O << ":";
        O.write_hex((unsigned)conv.c[i] & 0xFF);
      }
      return 8;
    }
    float val = CFP->getValueAPF().convertToFloat();
    union ftoi_union {
      float f;
      uint32_t u;
      char c[4];
    } conv;
    conv.f = val;
    if (!AsBytes) {
      O << ":";
      O.write_hex(conv.u);
      return 1;
    }
    for (int i = 0; i < 4; ++i) {
      O << ":";
      O.write_hex((unsigned)conv.c[i] & 0xFF);
    }
    return 4;
  }
  if (const ConstantInt *CI = dyn_cast<ConstantInt>(CAval)) {
    uint64_t zVal = CI->getValue().getZExtValue();
    if (!AsBytes) {
      O << ":";
      O.write_hex(zVal);
      return 1;
    }
    switch (CI->getBitWidth()) {
    default: {
        union ltob_union {
          uint64_t l;
          char c[8];
        } conv;
        conv.l = zVal;
        for (int i = 0; i < 8; ++i) {
          O << ":";
          O.write_hex((unsigned)conv.c[i] & 0xFF);
        }
        return 8;
      }
    case 8:
      O << ":";
      O.write_hex(zVal & 0xFF);
      return 1;
    case 16: {
        union stob_union {
          uint16_t s;
          char c[2];
        } conv;
        conv.s = (uint16_t)zVal;
        O << ":";
        O.write_hex((unsigned)conv.c[0] & 0xFF);
        O << ":";
        O.write_hex((unsigned)conv.c[1] & 0xFF);
        return 2;
      }
    case 32: {
        union itob_union {
          uint32_t i;
          char c[4];
        } conv;
        conv.i = (uint32_t)zVal;
        for (int i = 0; i < 4; ++i) {
          O << ":";
          O.write_hex((unsigned)conv.c[i] & 0xFF);
        }
        return 4;
      }
    }
  }
  size_t dumpSize = 0;
  if (const ConstantVector *CV = dyn_cast<ConstantVector>(CAval)) {
    size_t y = CV->getNumOperands();
    size_t x = 0;
    for (; x < y; ++x) {
      dumpSize += printConstantValue(CV->getOperand(x), O, AsBytes);
    }
    for (; x < (size_t)RoundUpToAlignment(y, 2); ++x) {
      dumpSize += dumpZeroElements(CV->getOperand(0)->getType(), O, AsBytes);
    }
    return dumpSize;
  }
  if (const ConstantStruct *CS = dyn_cast<ConstantStruct>(CAval)) {
    const DataLayout *DL = TM->getDataLayout();
    const StructLayout* layout = DL->getStructLayout(CS->getType());
    int y = CS->getNumOperands();
    assert(AsBytes && "struct not dumped as bytes");
    for (int x = 0; x < y; ++x) {
      const Constant* elem = CS->getOperand(x);
      dumpSize += printConstantValue(elem, O, AsBytes);
      // add padding
      uint64_t nextOffset = x + 1 < y
                            ? layout->getElementOffset(x + 1)
                            : layout->getSizeInBytes();
      if (dumpSize < nextOffset) {
        ArrayType* arrayTy
          = ArrayType::get(Type::getInt32Ty(CS->getContext()),
                           nextOffset - dumpSize);
        dumpSize += dumpZeroElements(arrayTy, O, false/*not AsBytes*/);
      }
    }
    return dumpSize;
  }
  if (const ConstantAggregateZero *CAZ
      = dyn_cast<ConstantAggregateZero>(CAval)) {
    int y = CAZ->getNumOperands();
    if (y > 0) {
      int x = 0;
      for (; x < y; ++x) {
        dumpSize += printConstantValue((llvm::Constant *)CAZ->getOperand(x),
            O, AsBytes);
      }
      return dumpSize;
    }
    if (AsBytes) {
      return dumpZeroElements(CAval->getType(), O, AsBytes);
    }
    y = getNumElements(CAval->getType())-1;
    for (int x = 0; x < y; ++x) {
      O << ":0";
    }
    O << ":0";
    return y + 1;
  }
  if (const ConstantArray *CA = dyn_cast<ConstantArray>(CAval)) {
    int y = CA->getNumOperands();
    int x = 0;
    for (; x < y; ++x) {
      dumpSize += printConstantValue(CA->getOperand(x), O, AsBytes);
    }
    return dumpSize;
  }
  if (const ConstantDataArray *CDA = dyn_cast<ConstantDataArray>(CAval)) {
    int y = CDA->getNumElements();
    int x = 0;
    for (; x < y; ++x) {
      dumpSize += printConstantValue(CDA->getElementAsConstant(x), O, AsBytes);
    }
    return dumpSize;
  }
  if (const ConstantDataVector *CDV = dyn_cast<ConstantDataVector>(CAval)) {
    size_t y = CDV->getNumElements();
    size_t x = 0;
    for (; x < y; ++x) {
      dumpSize += printConstantValue(CDV->getElementAsConstant(x), O, AsBytes);
    }
    for (; x < (size_t)RoundUpToAlignment(y, 2); ++x) {
      dumpSize +=
        dumpZeroElements(CDV->getElementAsConstant(0)->getType(), O, AsBytes);
    }
    return dumpSize;
  }
  if (isa<ConstantPointerNull>(CAval) ||
      isa<ConstantExpr>(CAval) ||
      isa<UndefValue>(CAval)) {
    O << ":0";
    return 1;
  }

  llvm_unreachable("Hit condition which was not expected");
  return 0;
}

static bool isStruct(Type *const T) {
  if (!T) {
    return false;
  }
  switch (T->getTypeID()) {
  default:
    return false;
  case Type::PointerTyID:
    return isStruct(T->getContainedType(0));
  case Type::StructTyID:
    return true;
  case Type::ArrayTyID:
  case Type::VectorTyID:
    return isStruct(dyn_cast<SequentialType>(T)->getElementType());
  }
}

void AMDILModuleInfo::dumpDataToCB(raw_ostream &O,
                                   AMDILMachineFunctionInfo *mfi,
                                   uint32_t id) {
  uint32_t size = 0;
  for (StringMap<AMDILConstPtr>::iterator cmb = consts_begin(),
      cme = consts_end(); cmb != cme; ++cmb) {
    AMDILConstPtr& c = cmb->second;
    if (id == c.cbNum) {
      if ((c.size + c.offset) > size) {
        size = AlignUp_32(c.size + c.offset, 16);
      }
    }
  }
  const DataLayout *DL = TM->getDataLayout();
  if (id == 0) {
    O << ";#DATASTART:" << (size + mCurrentCPOffset) << "\n";
    if (mCurrentCPOffset) {
      for (StringMap<AMDILKernel*>::iterator kcpb = mKernels.begin(),
          kcpe = mKernels.end(); kcpb != kcpe; ++kcpb) {
        const AMDILKernel *k = kcpb->second;
        if (!k) continue;
        size_t numConsts = k->CPOffsets.size();
        for (size_t x = 0; x < numConsts; ++x) {
          size_t offset = k->CPOffsets[x].first;
          const Constant *C = k->CPOffsets[x].second;
          Type *Ty = C->getType();
          if (isa<GlobalVariable>(C)) {
            // dereference the pointer type because GlobalVariable is always
            // a pointer type, and we want to calculate the size of the memory
            // that the GlobalVariable pointer points to
            PointerType* pTy = dyn_cast<PointerType>(Ty);
            assert(pTy && "GlobalVariable not pointer type");
            Ty = pTy->getElementType();
          }
          size_t size = (isStruct(Ty) ? DL->getTypeAllocSize(Ty)
                                      : getNumElements(Ty));
          O << ";#" << getTypeName(Ty, SymTab, mfi, true) << ":";
          O << offset << ":" << size ;
          size_t dumpSize = printConstantValue(C, O, isStruct(Ty));
          // Add padding for struct
          if (isStruct(Ty) && DL->getTypeStoreSize(Ty) < size) {
            ArrayType* arrayTy
              = ArrayType::get(Type::getInt32Ty(C->getContext()),
                               size - DL->getTypeStoreSize(Ty));
            dumpSize += dumpZeroElements(arrayTy, O, false/*not AsBytes*/);
          }
          O << "\n";
          assert(dumpSize == size && "bad dump size");
        }
      }
    }
  } else {
    O << ";#DATASTART:" << id << ":" << size << "\n";
  }

  for (StringMap<AMDILConstPtr>::iterator cmb = consts_begin(), cme = consts_end();
       cmb != cme; ++cmb) {
    if (cmb->second.cbNum != id) {
      continue;
    }
    const GlobalVariable *G = dyn_cast<GlobalVariable>(cmb->second.base);
    Type *Ty = (G) ? G->getType() : NULL;
    size_t offset = cmb->second.offset;
    size_t size = (isStruct(Ty)
        ? cmb->second.size
        : getNumElements(Ty));
    O << ";#" << getTypeName(Ty, SymTab, mfi, true) << ":";
    if (!id) {
      O << (offset + mCurrentCPOffset) << ":" << size;
    } else {
      O << offset << ":" << size;
    }

    if (!G->hasInitializer()) {
      O << " <uninitalized constant>\n";
      continue;
    }

    if (const Constant *C = G->getInitializer()) {
      size_t dumpSize = printConstantValue(C, O, isStruct(Ty));
      // Add padding for struct
      if (isStruct(Ty) && DL->getTypeStoreSize(C->getType()) < size) {
        ArrayType* arrayTy
          = ArrayType::get(Type::getInt32Ty(C->getContext()),
                           size - DL->getTypeStoreSize(C->getType()));
        dumpSize += dumpZeroElements(arrayTy, O, false/*not AsBytes*/);
      }
      assert(dumpSize == size && "bad dump size");
    }
    O <<"\n";
  }
  if (id == 0) {
    O << ";#DATAEND\n";
  } else {
    O << ";#DATAEND:" << id << "\n";
  }
}

void AMDILModuleInfo::dumpDataSection(raw_ostream &O,
                                      AMDILMachineFunctionInfo *mfi) {
  if (consts_empty() && !mCurrentCPOffset) {
    return;
  }

  llvm::DenseSet<uint32_t> const_set;
  for (StringMap<AMDILConstPtr>::iterator cmb = consts_begin(), cme = consts_end();
       cmb != cme; ++cmb) {
    const_set.insert(cmb->second.cbNum);
  }
  if (mCurrentCPOffset) {
    const_set.insert(0);
  }
  for (llvm::DenseSet<uint32_t>::iterator setb = const_set.begin(),
         sete = const_set.end(); setb != sete; ++setb) {
    dumpDataToCB(O, mfi, *setb);
  }
}

/// Create a function ID if it is not known or return the known
/// function ID.
uint32_t AMDILModuleInfo::getOrCreateFunctionID(const GlobalValue* Func) {
  StringRef FuncName = Func->getName();
  if (!FuncName.empty()) {
    return getOrCreateFunctionID(FuncName);
  }

  uint32_t ID;
  if (mFuncPtrNames.find(Func) == mFuncPtrNames.end()) {
    ID = mFuncPtrNames.size() + RESERVED_FUNCS + mFuncNames.size();
    mFuncPtrNames[Func] = ID;
  } else {
    ID = mFuncPtrNames[Func];
  }

  return ID;
}

uint32_t AMDILModuleInfo::getOrCreateFunctionID(StringRef Func) {
  llvm::StringMap<uint32_t>::iterator It = mFuncNames.find(Func);
  if (It != mFuncNames.end()) {
    return It->second;
  }

  uint32_t ID = mFuncNames.size() + RESERVED_FUNCS + mFuncPtrNames.size();
  mFuncNames[Func] = ID;
  return ID;
}

// populate the next local buffer with a set of local pointers
// return the resource ID of the local buffer populated
uint32_t AMDILModuleInfo::populateNextLocalBuffer(
  const SmallSet<const Value *, 1>& Locals, bool IsDefaultBuf) {
  uint32_t DefLocalId = mSTM->getResourceID(AMDIL::LDS_ID);
  uint32_t ResId = IsDefaultBuf ? DefLocalId : DefLocalId + mNumLocalBuffers;
  if (!IsDefaultBuf)
    ++mNumLocalBuffers;
  unsigned Offset = 0;
  for (SmallSet<const Value *, 1>::iterator I = Locals.begin(),
         E = Locals.end(); I != E; ++I) {
    const Value* V = *I;

    // Skip kernel arguments
    if (!isa<GlobalValue>(V))
      continue;

    const PointerType *PT = dyn_cast<PointerType>(V->getType());
    assert(PT && PT->getAddressSpace() == AMDILAS::LOCAL_ADDRESS && "sanity");
    llvm::StringRef GVName = V->getName();
    StringMap<AMDILArrayMem>::iterator It = mArrayMems.find(GVName);
    assert(It != mArrayMems.end() && "undefined local buffer?");
    AMDILArrayMem &Local = It->second;
    assert(Local.isHW && !Local.isRegion && "sanity");
    Local.resourceID = ResId;
    Local.offset = AlignUp_32(Offset, Local.align);
    Offset = Local.offset + Local.vecSize;
  }

  return ResId;
}

uint32_t AMDILModuleInfo::reserveShared(AMDILKernel *Kernel,
                                        uint32_t Size,
                                        uint32_t Alignment,
                                        StringRef Name,
                                        bool Region) {
  uint32_t DefLocalId = mSTM->getResourceID(Region ? AMDIL::GDS_ID : AMDIL::LDS_ID);
  uint32_t ResId = DefLocalId + (Region ? mNumRegionBuffers++ : mNumLocalBuffers++);

  AMDILArrayMem &ArrayMem = mReservedArrayMems.GetOrCreateValue(Name).getValue();
  ArrayMem.base = NULL;
  ArrayMem.vecSize = Size;
  ArrayMem.offset = 0;
  ArrayMem.align = Alignment;
  ArrayMem.resourceID = ResId;

  AMDIL::Caps::CapType Caps
    = Region ? AMDIL::Caps::RegionMem : AMDIL::Caps::LocalMem;
  ArrayMem.isHW = mSTM->usesHardware(Caps);
  ArrayMem.isRegion = Region;

  // Add reserved contribution to kernel total
  addArrayMemSize(Kernel, &ArrayMem);

  Kernel->reservedLocals.local.push_back(&ArrayMem);
  Kernel->reservedLocals.name = Kernel->mName;

  return ArrayMem.offset;
}

uint32_t AMDILModuleInfo::findReservedSharedResourceID(StringRef MemName) const {
  llvm::StringMap<AMDILArrayMem>::const_iterator It
    = mReservedArrayMems.find(MemName);
  return It == mReservedArrayMems.end() ? 0 : It->second.resourceID;
}

bool AMDILModuleInfo::getGlobalValueRID(const AMDILAsmPrinter *ASM,
                                        const Value *value,
                                        uint32_t& RID) const {
  assert(isa<GlobalValue>(value) && "not GlobalValue");

  unsigned addrspace;
  PointerType *Ty = dyn_cast<PointerType>(value->getType());
  if (!Ty) {
    // No addressSpace info, ignored.
    return false;
  }

  // void @__OpenCL_foo_kernel(i32 addrspace(1)* %p, i32 addrspace(3)* %l)
  //   %ll = alloca i32 addrspace(3)*, align 4
  //
  // Case (1) type of %p :  i32 addrspace(1)*
  //      (2) type of %ll:  i32 addrspace(3)**
  if (PointerType *Ty1 = dyn_cast<PointerType>(Ty->getElementType())) {
    // Case (2)
    addrspace = Ty1->getAddressSpace();
  } else {
    // case (1)
    addrspace = Ty->getAddressSpace();
  }
  addrspace = ASM->correctDebugAS(addrspace, value);

  switch (addrspace) {
  case AMDILAS::CONSTANT_ADDRESS:
  {
    RID = 0;
    const GlobalVariable *GV = dyn_cast<GlobalVariable>(value);
    AMDILModuleInfo *AMI = &(ASM->MMI->getObjFileInfo<AMDILModuleInfo>());
    // The AMDILModuleInfo class has the global variable to resource ID
    // mapping for constant buffers.
    if (AMI && GV) {
      RID = AMI->getConstPtrCB(NULL, GV->getName());
    }
    if (!RID) {
      RID = mSTM->getResourceID(AMDIL::CONSTANT_ID);
    }
    break;
  }
  case AMDILAS::LOCAL_ADDRESS:
    RID = mSTM->getResourceID(AMDIL::LDS_ID);
    break;
  case AMDILAS::GLOBAL_ADDRESS:
    if (mSTM->getGeneration() <= AMDIL::NORTHERN_ISLANDS) {
      RID = mSTM->getResourceID(AMDIL::ARENA_UAV_ID);
    } else {
      RID = mSTM->getResourceID(AMDIL::RAW_UAV_ID);
    }
    break;
  case AMDILAS::PRIVATE_ADDRESS:
    // Ignore private address because that is the default.
    return false;
  default:
    // Ignore it for now.
    return false;
  }
  return true;
}
void AMDILModuleInfo::calculateKernelHasCalls() {
  const TargetIntrinsicInfo *TII = TM->getIntrinsicInfo();
  const Module *M = mMMI->getModule();
  for (Module::const_iterator I = M->begin(), E = M->end(); I != E; ++I) {
    // Intrinsics have already been lowered, so ignore them
    if (I->isIntrinsic()) {
      continue;
    }
    // look for target specific intrinsics
    unsigned IntrinsicID = TII->getIntrinsicID(const_cast<Function*>(&(*I)));
    if (IntrinsicID != Intrinsic::not_intrinsic) {
      continue;
    }

    for (Function::const_use_iterator UI = I->use_begin(), UE = I->use_end();
      UI != UE; ++UI) {
      const User *U = *UI;
      if (isa<CallInst>(U) || isa<InvokeInst>(U)) {
        ImmutableCallSite CS(cast<Instruction>(U));
        const Function* caller = CS.getCaller();
        if (isKernel(caller->getName())) {
          mHasCallKernels.insert(caller);
        }
      }
    }
  }
}
