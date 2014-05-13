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
#include "HSAILKernelManager.h"
#include "HSAILAlgorithms.tpp"
#include "HSAILAsmPrinter.h"
#include "HSAILDeviceInfo.h"
#include "HSAILDevices.h"
#include "HSAILCompilerErrors.h"
#include "HSAILKernel.h"
#include "HSAILMachineFunctionInfo.h"
#include "HSAILModuleInfo.h"
#include "HSAILSubtarget.h"
#include "HSAILTargetMachine.h"
#include "HSAILUtilityFunctions.h"
#include "HSAILOpaqueTypes.h"
#include "HSAILLLVMVersion.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/MathExtras.h"
#include <cstdio>
#include <ostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <queue>
#include <list>
#include <utility>
using namespace llvm;
#define NUM_EXTRA_SLOTS_PER_IMAGE 1

static bool errorPrint(const char *ptr, OSTREAM_TYPE &O) {
  if (ptr[0] == 'E') {
    O << ";error:" << ptr << "\n";
  } else {
    O << ";warning:" << ptr << "\n";
  }
  return false;
}

static bool
printfPrint(std::pair<const std::string, HSAILPrintfInfo *> &data, OSTREAM_TYPE &O) {
  O << ";printf_fmt:" << data.second->getPrintfID();
  // Number of operands
  O << ":" << data.second->getNumOperands();
  // Size of each operand
  for (size_t i = 0, e = data.second->getNumOperands(); i < e; ++i) {
    O << ":" << (data.second->getOperandID(i) >> 3);
  }
  const char *ptr = data.first.c_str();
  uint32_t size = data.first.size() - 1;
  // The format string size
  O << ":" << size << ":";
  for (size_t i = 0; i < size; ++i) {
    if (ptr[i] == '\r') {
      O << "\\r";
    } else if (ptr[i] == '\n') {
      O << "\\n";
    } else {
      O << ptr[i];
    }
  }
  O << ";\n";   // c_str() is cheap way to trim
  return false;
}

void HSAILKernelManager::updatePtrArg(Function::const_arg_iterator Ip,
                                      int numWriteImages, int raw_uav_buffer,
                                      int counter, bool isKernel,
                                      const Function *F) {
  assert(F && "Cannot pass a NULL Pointer to F!");
  assert(Ip->getType()->isPointerTy() &&
         "Argument must be a pointer to be passed into this function!\n");
  std::string ptrArg("pointer:");
  const char *symTab = "NoSymTab";
  uint32_t ptrID = getUAVID(Ip);
  PointerType *PT = cast<PointerType>(Ip->getType());
  uint32_t Align = 4;
  const char *MemType = "uav";
  if (PT->getElementType()->isSized()) {
    Align = mTM->getDataLayout()->getTypeAllocSize(PT->getElementType());
    if ((Align & (Align - 1))) Align = NextPowerOf2(Align);
  }
  ptrArg += Ip->getName().str() + ":" + 
    HSAILgetTypeName(PT, symTab, mMFI, 
                     mMFI->isSignedIntType(Ip)) + ":1:1:" +
    itostr(counter * 16) + ":";
  switch (PT->getAddressSpace()) {
  case HSAILAS::ADDRESS_NONE:
    //O << "No Address space qualifier!";
    mMFI->addErrorMsg(hsa::CompilerErrorMessage[INTERNAL_ERROR]);
    assert(1);
    break;
  case HSAILAS::GLOBAL_ADDRESS:
    if (mSTM->device()->isSupported(HSAILDeviceInfo::ArenaSegment)) {
      if (ptrID >= ARENA_SEGMENT_RESERVED_UAVS) {
        ptrID = 8;
      }
    }
    mMFI->uav_insert(ptrID);
    break;
  case HSAILAS::CONSTANT_ADDRESS: {
    if (isKernel && mSTM->device()->usesHardware(HSAILDeviceInfo::ConstantMem)){
      const HSAILKernel* t = mAMI->getKernel(F->getName());
      if (mAMI->usesHWConstant(t, Ip->getName())) {
        MemType = /*(isSI) ? "uc\0" :*/ "hc\0";
        ptrID = mAMI->getConstPtrCB(t, Ip->getName());
      } else {
        MemType = "c\0";
        mMFI->uav_insert(ptrID);
      }
    } else {
      MemType = "c\0";
      mMFI->uav_insert(ptrID);
    }
    break; 
  }
  default:
  case HSAILAS::PRIVATE_ADDRESS:
    if (mSTM->device()->usesHardware(HSAILDeviceInfo::PrivateMem)) {
      MemType = (mSTM->device()->isSupported(HSAILDeviceInfo::PrivateUAV)) 
        ? "up\0" : "hp\0";
    } else {
      MemType = "p\0";
      mMFI->uav_insert(ptrID);
    }
    break;
  case HSAILAS::REGION_ADDRESS:
    mMFI->setUsesRegion();
    if (mSTM->device()->usesHardware(HSAILDeviceInfo::RegionMem)) {
      MemType = "hr\0";
      ptrID = 0;
    } else {
      MemType = "r\0";
      mMFI->uav_insert(ptrID);
    }
    break;
  case HSAILAS::GROUP_ADDRESS:
    mMFI->setUsesLocal();
    if (mSTM->device()->usesHardware(HSAILDeviceInfo::LocalMem)) {
      MemType = "hl\0";
      ptrID = 1;
    } else {
      MemType = "l\0";
      mMFI->uav_insert(ptrID);
    }
    break;
  };
  ptrArg += std::string(MemType) + ":";
  ptrArg += itostr(ptrID) + ":";
  ptrArg += itostr(Align) + ":";
  const Value* ptr = Ip;
  if (mMFI->read_ptr_count(ptr)) {
    ptrArg += "RO";
  // FIXME: add write-only pointer detection.
  //} else if (mMFI->write_ptr_count(ptr)) {
  //  ptrArg += "WO";
  } else {
    ptrArg += "RW";
  }
  ptrArg += (mMFI->isVolatilePointer(Ip)) ? ":1" : ":0";
  ptrArg += (mMFI->isRestrictPointer(Ip)) ? ":1" : ":0";
  mMFI->addMetadata(ptrArg, true);
}

HSAILKernelManager::HSAILKernelManager(HSAILTargetMachine *TM)
{
  mTM = TM;
  mSTM = mTM->getSubtargetImpl();
  mMFI = NULL;
  mAMI = NULL;
  mMF = NULL;
  clear();
}

HSAILKernelManager::~HSAILKernelManager() {
  clear();
}

void 
HSAILKernelManager::setMF(MachineFunction *MF)
{
  mMF = MF;
  mMFI = MF->getInfo<HSAILMachineFunctionInfo>();
  mAMI = &(MF->getMMI().getObjFileInfo<HSAILModuleInfo>());
}

void HSAILKernelManager::clear() {
  mUniqueID = 0;
  mWasKernel = false;
  mHasImageWrite = false;
  mHasOutputInst = false;
}

void HSAILKernelManager::processArgMetadata(OSTREAM_TYPE &ignored,
                                            uint32_t buf,
                                            bool isKernel) 
{
  const Function *F = mMF->getFunction();
  const char * symTab = "NoSymTab";
  Function::const_arg_iterator Ip = F->arg_begin();
  Function::const_arg_iterator Ep = F->arg_end();
  
  if (F->hasStructRetAttr()) {
    assert(Ip != Ep && "Invalid struct return fucntion!");
    mMFI->addErrorMsg(hsa::CompilerErrorMessage[INTERNAL_ERROR]);
    ++Ip;
  }
  uint32_t mCBSize = 0;
  int raw_uav_buffer = mSTM->device()->getResourceID(HSAILDevice::RAW_UAV_ID);
  bool MultiUAV = mSTM->device()->isSupported(HSAILDeviceInfo::MultiUAV);
  bool ArenaSegment =
    mSTM->device()->isSupported(HSAILDeviceInfo::ArenaSegment);
  int numWriteImages = mMFI->get_num_write_images();
  if (numWriteImages == OPENCL_MAX_WRITE_IMAGES || MultiUAV || ArenaSegment) {
    if (mSTM->device()->getGeneration() <= HSAILDeviceInfo::HD6XXX) {
      raw_uav_buffer = mSTM->device()->getResourceID(HSAILDevice::ARENA_UAV_ID);
    }
  }
  uint32_t CounterNum = 0;
  uint32_t SemaNum = 0;
  uint32_t ROArg = 0;
  uint32_t WOArg = 0;
  uint32_t NumArg = 0;
  uint32_t SamplerNum = 0;
  while (Ip != Ep) {
    Type *cType = Ip->getType();
    if (cType->isIntOrIntVectorTy() || cType->isFPOrFPVectorTy()) {
      std::string argMeta("value:");
      argMeta += Ip->getName().str() + ":" + HSAILgetTypeName(cType, symTab, mMFI
          , mMFI->isSignedIntType(Ip)) + ":";
      int bitsize = cType->getPrimitiveSizeInBits();
      int numEle = 1;
      if (cType->getTypeID() == Type::VectorTyID) {
        numEle = cast<VectorType>(cType)->getNumElements();
      }
      argMeta += itostr(numEle) + ":1:" + itostr((int64_t)mCBSize << 4);
      mMFI->addMetadata(argMeta, true);

      // FIXME: simplify
      if ((bitsize / numEle) < 32) {
        bitsize = numEle >> 2;
      } else {
        bitsize >>= 7;
      }
      if (!bitsize) {
        bitsize = 1;
      }

      mCBSize += bitsize;
    } else if (const PointerType *PT = dyn_cast<PointerType>(cType)) {
      Type *CT = PT->getElementType();
      const StructType *ST = dyn_cast<StructType>(CT);
      if (ST && ST->isOpaque()) {
        OpaqueType OT = GetOpaqueType(ST);
        if (IsImage(OT)) {
          if (!mSTM->device()->isSupported(HSAILDeviceInfo::Images)) {
            mMFI->addErrorMsg(hsa::CompilerErrorMessage[NO_IMAGE_SUPPORT]);
          }
           
          std::string imageArg("image:");
          imageArg += Ip->getName().str() + ":";
          switch (OT) {
            case I1D:   imageArg += "1D:";   break;
            case I1DA:  imageArg += "1DA:";  break;
            case I1DB:  imageArg += "1DB:";  break;
            case I2D:   imageArg += "2D:";   break;
            case I2DA:  imageArg += "2DA:";  break;
            case I3D:   imageArg += "3D:";   break;
            case I2DDepth: imageArg += "2DDepth:"; break;
            case I2DADepth: imageArg += "2DADepth:"; break;
            default:    llvm_unreachable("unknown image type"); break;
          }
          if (isKernel) {
            if (mAMI->isReadOnlyImage (mMF->getFunction()->getName(),
                                       (ROArg + WOArg))) {
              imageArg += "RO:" + itostr(ROArg);
              ++ROArg;
            } else if (mAMI->isWriteOnlyImage(mMF->getFunction()->getName(),
                                              (ROArg + WOArg))) {
              uint32_t offset = 0;
              offset += WOArg;
              imageArg += "WO:" + itostr(offset & 0x7);
              ++WOArg;
            } else {
              imageArg += "RW:" + itostr(ROArg + WOArg);
            }
          }
          imageArg += ":1:" + itostr(mCBSize * 16);
          mMFI->addMetadata(imageArg, true);
          mMFI->addi32Literal(mCBSize);
          mCBSize += NUM_EXTRA_SLOTS_PER_IMAGE + 1;
        } else if (OT == C32 || OT == C64) {
          std::string counterArg("counter:");
          counterArg += Ip->getName().str() + ":"
            + itostr(OT == C32 ? 32 : 64) + ":"
            + itostr(CounterNum++) + ":1:" + itostr(mCBSize * 16);
          mMFI->addMetadata(counterArg, true);
          ++mCBSize;
        } else if (OT == Sema) {
          std::string semaArg("sema:");
          semaArg += Ip->getName().str() + ":" + itostr(SemaNum++)
            + ":1:" + itostr(mCBSize * 16);
          mMFI->addMetadata(semaArg, true);
          ++mCBSize;
        } else if (OT == Sampler) {
          std::string samplerArg("sampler:");
          samplerArg += Ip->getName().str() + ":" + itostr(SamplerNum++)
            + ":1:" + itostr(mCBSize * 16);
          mMFI->addMetadata(samplerArg, true);
          ++mCBSize;
        } else {
          updatePtrArg(Ip, numWriteImages, raw_uav_buffer, mCBSize, isKernel,
                       F);
          ++mCBSize;
        }
      } else if (CT->getTypeID() == Type::StructTyID 
                 && Ip->hasByValAttr()) { // To distinguish pass-by-value from pass-by-ptr.
        // When struct is passed-by-value, the pointer to the struct copy
        // is passed to the kernel. Relevant RTI is generated here (value...struct).
        // [Informative: RTI for pass-by-pointer case (pointer...struct) is generated
        // in the next "else if" block.]     
        const DataLayout *dl = mTM->getDataLayout();
        const StructLayout *sl = dl->getStructLayout(dyn_cast<StructType>(CT));
        int bytesize = sl->getSizeInBytes();
        int reservedsize = (bytesize + 15) & ~15;
        int numSlots = reservedsize >> 4;
        if (!numSlots) {
          numSlots = 1;
        }
        std::string structArg("value:");
        structArg += Ip->getName().str() + ":struct:"
          + itostr(bytesize) + ":1:" + itostr(mCBSize * 16);
        mMFI->addMetadata(structArg, true);
        mCBSize += numSlots;
      } else if (CT->isIntOrIntVectorTy()
                 || CT->isFPOrFPVectorTy()
                 || CT->getTypeID() == Type::ArrayTyID
                 || CT->getTypeID() == Type::PointerTyID
                 || PT->getAddressSpace() != HSAILAS::PRIVATE_ADDRESS) {
        updatePtrArg(Ip, numWriteImages, raw_uav_buffer, mCBSize, isKernel, F);
        ++mCBSize;
      } else {
        assert(0 && "Cannot process current pointer argument");
        mMFI->addErrorMsg(hsa::CompilerErrorMessage[INTERNAL_ERROR]);
      }
    } else {
      assert(0 && "Cannot process current kernel argument");
      mMFI->addErrorMsg(hsa::CompilerErrorMessage[INTERNAL_ERROR]);
    }
    if (mMFI->isConstantArgument(Ip)) {
      std::string constArg("constarg:");
      constArg += itostr(NumArg) + ":" + Ip->getName().str();
      mMFI->addMetadata(constArg, true);
    }
    ++NumArg;
    ++Ip;
  }
}

void HSAILKernelManager::printHeader(const std::string &name) 
{
  mName = name;
  std::string kernelName = name;
  int kernelId = mAMI->getOrCreateFunctionID(kernelName);
}

/** 
 *
 * HSAIL format for emitting runtime information: 
 * block "rti"
 * blockstring "<metadata>";
 * endblock;
 *
 * @param O 
 * @param id 
 * @param kernel 
 */

void
HSAILKernelManager::printMetaData(OSTREAM_TYPE &O, uint32_t id, bool kernel) {
  if (kernel) {
    int kernelId = mAMI->getOrCreateFunctionID(mName);
    mMFI->addCalledFunc(id);
    mUniqueID = kernelId;
    mIsKernel = true;
  }
  printKernelArgs(O);
  if (kernel) {
    mIsKernel = false;
    mMFI->eraseCalledFunc(id);
    mUniqueID = id;
  }
}

void HSAILKernelManager::setKernel(bool kernel) {
  mIsKernel = kernel;
  if (kernel) {
    mWasKernel = mIsKernel;
  }
}

void HSAILKernelManager::setID(uint32_t id)
{
  mUniqueID = id;
}

void HSAILKernelManager::setName(const std::string &name) {
  mName = name;
}

class BlockString {
    std::string m_str;
    HSAIL_ASM::BrigContainer&  m_bc;
    mutable llvm::raw_string_ostream m_os;
public:
    BlockString(HSAIL_ASM::BrigContainer& bc) : m_bc(bc),m_os(m_str) {}

    ~BlockString() {
        const std::string& str = m_os.str();
        if (!str.empty()) {
            m_bc.append< HSAIL_ASM::BlockString>().string() = str;
        }
    }

    llvm::raw_string_ostream& os() const { return m_os; }
};

template <typename T>
const BlockString& operator << (const BlockString& os, const T& s)    { os.os() << s; return os; } 
const BlockString& operator << (const BlockString& os, const char *s) { os.os() << s; return os; } 

void HSAILKernelManager::brigEmitMetaData(HSAIL_ASM::BrigContainer& bc, uint32_t id, bool isKernel) {

    // Initialization block related to current function being processed
    int kernelId = id;
    if (isKernel) {
      kernelId = mAMI->getOrCreateFunctionID(mName);
      mMFI->addCalledFunc(id);
      mUniqueID = kernelId;
      mIsKernel = true;
    }

    const HSAILKernel *kernel = mAMI->getKernel(mName);  
    
    if (kernel && isKernel && kernel->sgv) {
      if (kernel->sgv->mHasRWG) {
          HSAIL_ASM::OperandImmed i1 = bc.append< HSAIL_ASM::OperandImmed>();
          HSAIL_ASM::setImmed(i1,kernel->sgv->reqGroupSize[0],Brig::BRIG_TYPE_U32);
          HSAIL_ASM::OperandImmed i2 = bc.append< HSAIL_ASM::OperandImmed>();
          HSAIL_ASM::setImmed(i2,kernel->sgv->reqGroupSize[1],Brig::BRIG_TYPE_U32);
          HSAIL_ASM::OperandImmed i3 = bc.append< HSAIL_ASM::OperandImmed>();
          HSAIL_ASM::setImmed(i3,kernel->sgv->reqGroupSize[2],Brig::BRIG_TYPE_U32);
          HSAIL_ASM::DirectiveControl dc = bc.append< HSAIL_ASM::DirectiveControl>();
          dc.code() = bc.insts().end();
          dc.control() = Brig::BRIG_CONTROL_REQUIREDWORKGROUPSIZE;
          dc.type() = Brig::BRIG_TYPE_U32;
          dc.values().push_back(i1);
          dc.values().push_back(i2);
          dc.values().push_back(i3);
      }
    }

    if (isKernel) {
      std::string emptyStr("");
      std::string &refEmptyStr(emptyStr);
      llvm::raw_string_ostream oss(refEmptyStr);
      // metadata block start
      HSAIL_ASM::BlockStart sBlock = bc.append< HSAIL_ASM::BlockStart>();
      sBlock.name() = "rti";
      sBlock.code() = bc.insts().end();
      // function name
      BlockString(bc)  << "ARGSTART:" << mName;
      if(isKernel) {
        // version
        BlockString(bc) << "version:" << itostr(mSTM->supportMetadata30() ? HSAIL_MAJOR_VERSION : 2) << ":"
                        << itostr(HSAIL_MINOR_VERSION) + ":" 
                        << itostr(mSTM->supportMetadata30() ? HSAIL_REVISION_NUMBER : HSAIL_20_REVISION_NUMBER);
        // device info
        BlockString(bc) << "device:" << mSTM->getDeviceName();
      }
      BlockString(bc) << "uniqueid:" << kernelId;
      if (kernel) {
        size_t local = kernel->curSize;
        size_t hwlocal = ((kernel->curHWSize + 3) & (~0x3));
        size_t region = kernel->curRSize;
        size_t hwregion = ((kernel->curHWRSize + 3) & (~0x3));
        bool usehwlocal = mSTM->device()->usesHardware(HSAILDeviceInfo::LocalMem);
        bool usehwprivate = mSTM->device()->usesHardware(HSAILDeviceInfo::PrivateMem);
        bool usehwregion = mSTM->device()->usesHardware(HSAILDeviceInfo::RegionMem);
        bool useuavprivate = mSTM->device()->isSupported(HSAILDeviceInfo::PrivateUAV);
        // private memory
        BlockString(bc) << "memory:" << ((usehwprivate) ?  (useuavprivate) ? "uav" : "hw" : "" ) << "private:" << (((mMFI->getStackSize() + mMFI->getPrivateSize() + 15) & (~0xF)));
        // region memory
        BlockString(bc) << "memory:" << ((usehwregion) ? "hw" : "") << "region:" << ((usehwregion) ? hwregion : hwregion + region);
        // local memory
        BlockString(bc) << "memory:" << ((usehwlocal) ? "hw" : "") << "local:" << ((usehwlocal) ? hwlocal : hwlocal + local)+mMFI->getGroupSize();
        if (kernel && isKernel && kernel->sgv) {
          if (kernel->sgv->mHasRWG) {
            BlockString(bc) << "cws:" << kernel->sgv->reqGroupSize[0] << ":" << kernel->sgv->reqGroupSize[1] << ":" << kernel->sgv->reqGroupSize[2];
          }
          if (kernel->sgv->mHasRWR) {
            BlockString(bc) << "crs:" << kernel->sgv->reqRegionSize[0] << ":" << kernel->sgv->reqRegionSize[1] << ":" << kernel->sgv->reqRegionSize[2];
          }
        }
      }
      if (isKernel) {
        for (std::vector<std::string>::iterator ib = mMFI->kernel_md_begin(), ie = mMFI->kernel_md_end(); ib != ie; ++ib) {
          std::string md = *ib;
          if ( md.find("argmap") == std::string::npos ) {
            BlockString(bc) << (*ib);
          }
        }
      }

    for (std::set<std::string>::iterator ib = mMFI->func_md_begin(), ie = mMFI->func_md_end(); ib != ie; ++ib) {
      BlockString(bc) << (*ib);
    }

    if (!mMFI->func_empty()) {
      oss.str().clear();  
      oss << "function:" << mMFI->func_size();
      binaryForEach(mMFI->func_begin(), mMFI->func_end(), HSAILcommaPrint, oss);
      BlockString(bc) << oss.str();
    }

    if (!mSTM->device()->isSupported(HSAILDeviceInfo::MacroDB) && !mMFI->intr_empty()) {
      oss.str().clear();  
      oss << "intrinsic:" << mMFI->intr_size();
      binaryForEach(mMFI->intr_begin(), mMFI->intr_end(), HSAILcommaPrint, oss);
      BlockString(bc) << oss.str();
    }
  
    if (!isKernel) {
      oss.str().clear();  
      binaryForEach(mMFI->printf_begin(), mMFI->printf_end(), printfPrint, oss);
      if ( ! oss.str().empty() ) {
        BlockString(bc) << oss.str();
      }
      mMF->getMMI().getObjFileInfo<HSAILModuleInfo>().add_printf_offset(mMFI->printf_size());
    } else {
      for (StringMap<SamplerInfo>::iterator 
        smb = mMFI->sampler_begin(),
        sme = mMFI->sampler_end(); smb != sme; ++ smb) {
        BlockString(bc) << "sampler:" << (*smb).second.name << ":" << (*smb).second.idx
          << ":" << ((*smb).second.val == (uint32_t)-1 ? 0 : 1) 
          << ":" << ((*smb).second.val != (uint32_t)-1 ? (*smb).second.val : 0);
      }
    }
    if (mSTM->is64Bit()) {
      BlockString(bc) << "memory:64bitABI";
    }

    if (mMFI->errors_empty()) {
      oss.str().clear();  
      binaryForEach(mMFI->errors_begin(), mMFI->errors_end(), errorPrint, oss);
      if ( ! oss.str().empty() ) {
        BlockString(bc) << oss.str();
      }
    }
    if (isKernel && mSTM->device()->getGeneration() <= HSAILDeviceInfo::HD6XXX) {
      if (mSTM->device()->getResourceID(HSAILDevice::RAW_UAV_ID) > mSTM->device()->getResourceID(HSAILDevice::ARENA_UAV_ID)) {
        if (mMFI->uav_size() == 1) {
          if (mSTM->device()->isSupported(HSAILDeviceInfo::ArenaSegment) && *(mMFI->uav_begin()) >= ARENA_SEGMENT_RESERVED_UAVS) {
            BlockString(bc) << "uavid:";
          } else {
            BlockString(bc) << "uavid:" << *(mMFI->uav_begin());
          }
        } else if (mMFI->uav_count(mSTM->device()-> getResourceID(HSAILDevice::RAW_UAV_ID))) {
          BlockString(bc) << "uavid:" << mSTM->device()->getResourceID(HSAILDevice::RAW_UAV_ID);
        } else {
          BlockString(bc) << "uavid:" << mSTM->device()->getResourceID(HSAILDevice::ARENA_UAV_ID);
        }
      } else if ((mMFI->get_num_write_images()) != OPENCL_MAX_WRITE_IMAGES && !mSTM->device()->isSupported(HSAILDeviceInfo::ArenaSegment)
                 && mMFI->uav_count(mSTM->device()-> getResourceID(HSAILDevice::RAW_UAV_ID))) {
        BlockString(bc) << "uavid:" << mSTM->device()->getResourceID(HSAILDevice::RAW_UAV_ID);
      } else if (mMFI->uav_size() == 1) {
        BlockString(bc) << "uavid:" << *(mMFI->uav_begin());
      } else {
        BlockString(bc) << "uavid:" << mSTM->device()->getResourceID(HSAILDevice::ARENA_UAV_ID);
      }
    } else if (isKernel && mSTM->device()->getGeneration() > HSAILDeviceInfo::HD6XXX) {
        if (mMFI->printf_size() > 0) {
          BlockString(bc) << ";uavid:" << mSTM->device()->getResourceID(HSAILDevice::GLOBAL_ID);
        }
    }
    if (isKernel) {
      BlockString(bc) << "privateid:" << mSTM->device()->getResourceID(HSAILDevice::SCRATCH_ID);
    }
    if (kernel) {
      for (unsigned I = 0, E = kernel->ArgTypeNames.size(); I != E; ++I) {
        BlockString(bc) << "reflection:" << I << ":" << kernel->ArgTypeNames[I];
      }
    }

    BlockString(bc) << "ARGEND:" << mName;

    // we're done - gen end_of_block
    HSAIL_ASM::BlockEnd eBlock = bc.append<HSAIL_ASM::BlockEnd>();
  }

  // De-initialization block
  if (isKernel) {
    mIsKernel = false;
    mMFI->eraseCalledFunc(id);
    mUniqueID = id;
  }
}

void HSAILKernelManager::printKernelArgs(OSTREAM_TYPE &O) {
  std::string version("version:");
  version += itostr(mSTM->supportMetadata30() ? HSAIL_MAJOR_VERSION : 2) + ":"
    + itostr(HSAIL_MINOR_VERSION) + ":" 
    + itostr(mSTM->supportMetadata30() 
        ? HSAIL_REVISION_NUMBER : HSAIL_20_REVISION_NUMBER);
  const HSAILKernel *kernel = mAMI->getKernel(mName);
  bool isKernel = (kernel) ? kernel->mKernel : false;
  if (isKernel) {
    O << "\tblock " << "\"rti\"" << "\n";
    O << "\tblockstring " << "\"";
    O << "ARGSTART:" << mName;
    O << "\"" << ";" << "\n";
    if (isKernel) {
      O << "\tblockstring " << "\"";
      O << version;
      O << "\"" << ";" << "\n";

      O << "\tblockstring " << "\"";
      O << "device:" << mSTM->getDeviceName();
      O << "\"" << ";" << "\n";
    }
    O << "\tblockstring " << "\"";
    O << "uniqueid:" << mUniqueID;
    O << "\"" << ";" << "\n";

    if (kernel) {
      size_t local = kernel->curSize+ mMFI->getGroupSize();
      size_t hwlocal = ((kernel->curHWSize + 3) & (~0x3));
      size_t region = kernel->curRSize;
      size_t hwregion = ((kernel->curHWRSize + 3) & (~0x3));
      bool usehwlocal = mSTM->device()->usesHardware(HSAILDeviceInfo::LocalMem);
      bool usehwprivate = mSTM->device()->usesHardware(HSAILDeviceInfo::PrivateMem);
      bool usehwregion = mSTM->device()->usesHardware(HSAILDeviceInfo::RegionMem);
      bool useuavprivate = mSTM->device()->isSupported(HSAILDeviceInfo::PrivateUAV);
      if (isKernel) {
        O << "\tblockstring " << "\"";
        O << "memory:" << ((usehwprivate) ? 
                           (useuavprivate) ? "uav" : "hw" : "" ) << "private:"
          << (((mMFI->getStackSize() + mMFI->getPrivateSize() + 15) & (~0xF)));
        O << "\"" << ";" << "\n";
      }
      if (mSTM->device()->isSupported(HSAILDeviceInfo::RegionMem)) {
        O << "\tblockstring " << "\"";
        O << "memory:" << ((usehwregion) ? "hw" : "") << "region:"
          << ((usehwregion) ? hwregion : hwregion + region);
        O << "\"" << ";" << "\n";
      }
      O << "\tblockstring " << "\"";
      O << "memory:" << ((usehwlocal) ? "hw" : "") << "local:"
        << ((usehwlocal) ? hwlocal : hwlocal + local) + mMFI->getGroupSize();
      O << "\"" << ";" << "\n";

      if (kernel && isKernel && kernel->sgv) {
        if (kernel->sgv->mHasRWG) {
          O << "\tblockstring " << "\"";
          O << "cws:"
            << kernel->sgv->reqGroupSize[0] << ":"
            << kernel->sgv->reqGroupSize[1] << ":"
            << kernel->sgv->reqGroupSize[2];
          O << "\"" << ";" << "\n";
        }
        if (kernel->sgv->mHasRWR) {
          O << "\tblockstring " << "\"";
          O << "crs:"
            << kernel->sgv->reqRegionSize[0] << ":"
            << kernel->sgv->reqRegionSize[1] << ":"
            << kernel->sgv->reqRegionSize[2];
          O << "\"" << ";" << "\n";
        }
      }
    }

    if (isKernel) {
      for (std::vector<std::string>::iterator ib = mMFI->kernel_md_begin(),
          ie = mMFI->kernel_md_end(); ib != ie; ++ib) {
        O << "\tblockstring " << "\"";
        O << (*ib);
        O << "\"" << ";" << "\n";
      }
    }

    for (std::set<std::string>::iterator ib = mMFI->func_md_begin(),
           ie = mMFI->func_md_end(); ib != ie; ++ib) {
      O << "\tblockstring " << "\"";
      O << (*ib);
      O << "\"" << ";" << "\n";
    }

    if (!mMFI->func_empty()) {
      O << "\tblockstring " << "\"";
      O << "function:" << mMFI->func_size();
      binaryForEach(mMFI->func_begin(), mMFI->func_end(), HSAILcommaPrint, O);
      O << "\"" << ";" << "\n";
    }

    if (!mSTM->device()->isSupported(HSAILDeviceInfo::MacroDB)
        && !mMFI->intr_empty()) {
      O << "\tblockstring " << "\"";
      O << "intrinsic:" << mMFI->intr_size();
      binaryForEach(mMFI->intr_begin(), mMFI->intr_end(), HSAILcommaPrint, O);
      O << "\"" << ";" << "\n";
    }

    if (!isKernel) {
      binaryForEach(mMFI->printf_begin(), mMFI->printf_end(), printfPrint, O);
      mMF->getMMI().getObjFileInfo<HSAILModuleInfo>().add_printf_offset(
          mMFI->printf_size());
    } else {
      for (StringMap<SamplerInfo>::iterator 
          smb = mMFI->sampler_begin(),
          sme = mMFI->sampler_end(); smb != sme; ++ smb) {
        O << "\tblockstring " << "\"";
        O << "sampler:" << (*smb).second.name << ":" << (*smb).second.idx
          << ":" << ((*smb).second.val == (uint32_t)-1 ? 0 : 1) 
          << ":" << ((*smb).second.val != (uint32_t)-1 ? (*smb).second.val : 0);
        O << "\"" << ";" << "\n";
      }
    }
    if (mSTM->is64Bit()) {
      O << "\tblockstring " << "\"";
      O << "memory:64bitABI";
      O << "\"" << ";" << "\n";
    }

    if (mMFI->errors_empty()) {
      binaryForEach(mMFI->errors_begin(), mMFI->errors_end(), errorPrint, O);
    }

    // This has to come last
    if (isKernel
        && mSTM->device()->getGeneration() <= HSAILDeviceInfo::HD6XXX) {
      if (mSTM->device()->getResourceID(HSAILDevice::RAW_UAV_ID) >
          mSTM->device()->getResourceID(HSAILDevice::ARENA_UAV_ID)) {
        if (mMFI->uav_size() == 1) {
          if (mSTM->device()->isSupported(HSAILDeviceInfo::ArenaSegment)
              && *(mMFI->uav_begin()) >= ARENA_SEGMENT_RESERVED_UAVS) {
            O << "\tblockstring " << "\"";
            O << "uavid:";
            // O << mSTM->device()->getResourceID(HSAILDevice::ARENA_UAV_ID);
            O << "\"" << ";" << "\n";
          } else {
            O << "\tblockstring " << "\"";
            O << "uavid:" << *(mMFI->uav_begin());
            O << "\"" << ";" << "\n";
          }
        } else if (mMFI->uav_count(mSTM->device()->
              getResourceID(HSAILDevice::RAW_UAV_ID))) {
          O << "\tblockstring " << "\"";
          O << "uavid:"
            << mSTM->device()->getResourceID(HSAILDevice::RAW_UAV_ID);
          O << "\"" << ";" << "\n";
        } else {
          O << "\tblockstring " << "\"";
          O << "uavid:";
          O << mSTM->device()->getResourceID(HSAILDevice::ARENA_UAV_ID);
          O << "\"" << ";" << "\n";
        }
      } else if ((mMFI->get_num_write_images()) !=
                 OPENCL_MAX_WRITE_IMAGES
                 && !mSTM->device()->isSupported(HSAILDeviceInfo::ArenaSegment)
                 && mMFI->uav_count(mSTM->device()->
                                    getResourceID(HSAILDevice::RAW_UAV_ID))) {
        O << "\tblockstring " << "\"";
        O << "uavid:";
        O << mSTM->device()->getResourceID(HSAILDevice::RAW_UAV_ID);
        O << "\"" << ";" << "\n";
      } else if (mMFI->uav_size() == 1) {
        O << "\tblockstring " << "\"";
        O << "uavid:" << *(mMFI->uav_begin());
        O << "\"" << ";" << "\n";
      } else {
        O << "\tblockstring " << "\"";
        O << "uavid:";
        O << mSTM->device()->getResourceID(HSAILDevice::ARENA_UAV_ID);
        O << "\"" << ";" << "\n";
      }
    } else if (isKernel 
	       && mSTM->device()->getGeneration() > HSAILDeviceInfo::HD6XXX) {
        if (mMFI->printf_size() > 0) {
            O << ";uavid:"
              << mSTM->device()->getResourceID(HSAILDevice::GLOBAL_ID);
            O << "\n";
        }
    }
    if (isKernel) {
      O << "\tblockstring " << "\"";
      O << "privateid:" << mSTM->device()->getResourceID(HSAILDevice::SCRATCH_ID);
      O << "\"" << ";" << "\n";
    }
    if (isKernel) {
      std::string argKernel = "llvm.argtypename.annotations.";
      argKernel.append(mName);
      GlobalVariable *GV = mMF->getFunction()->getParent()
        ->getGlobalVariable(argKernel);
      if (GV && GV->hasInitializer()) {
        const ConstantArray *nameArray
          = dyn_cast_or_null<ConstantArray>(GV->getInitializer());
        if (nameArray) {
          for (unsigned x = 0, y = nameArray->getNumOperands(); x < y; ++x) {
            const GlobalVariable *gV= dyn_cast_or_null<GlobalVariable>(
                  nameArray->getOperand(x)->getOperand(0));
              const ConstantDataArray *argName =
                dyn_cast_or_null<ConstantDataArray>(gV->getInitializer());
              if (!argName) {
                continue;
              }
              std::string argStr = argName->getAsString();
              O << "\tblockstring " << "\"";
              O << "reflection:" << x << ":";
              O << argStr.substr(0, argStr.length()-1);
              O << "\"" << ";" << "\n";
          }
        }
      }
    }
    O << "\tblockstring " << "\"";
    O << "ARGEND:" << mName;
    O << "\"" << ";" << "\n";
    O << "\tendblock" << ";" << "\n";
  }

  if (kernel && isKernel && kernel->sgv) {
    if (kernel->sgv->mHasRWG) {
      O << "\titemsperworkgroup \t"
        << kernel->sgv->reqGroupSize[0] << ", "
        << kernel->sgv->reqGroupSize[1] << ", "
        << kernel->sgv->reqGroupSize[2] << ";\n";
    }
  }

}

uint32_t HSAILKernelManager::getUAVID(const Value *value) {
  if (mValueIDMap.find(value) != mValueIDMap.end()) {
    return mValueIDMap[value];
  }

  if (mSTM->device()->getGeneration() <= HSAILDeviceInfo::HD6XXX) {
    return mSTM->device()->getResourceID(HSAILDevice::ARENA_UAV_ID);
  } else {
    return mSTM->device()->getResourceID(HSAILDevice::RAW_UAV_ID);
  }
}
