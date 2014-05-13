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
#include "AMDILKernelManager.h"
#include "AMDILAlgorithms.tpp"
#include "AMDILAsmPrinter.h"
#include "AMDILDeviceInfo.h"
#include "AMDILCompilerErrors.h"
#include "AMDILKernel.h"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILModuleInfo.h"
#include "AMDILSubtarget.h"
#include "AMDILTargetMachine.h"
#include "AMDILUtilityFunctions.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Type.h"
#include "../../Transforms/IPO/AMDSymbolName.h"
#include <cstdio>
#include <algorithm>
#include <string>
#include <queue>
#include <list>
#include <utility>
using namespace llvm;

static cl::opt<unsigned>
AMDILCallStackSize("amdil-call-stack-size", cl::init(0), cl::Hidden,
  cl::desc("The call stack size for AMDIL"));

//#if 0
void printRegName(AMDILAsmPrinter *RegNames,
                  unsigned Reg,
                  raw_ostream &O,
                  bool Dst,
                  bool Dupe = false) {
  if (isXComponentReg(Reg)) {
    O << RegNames->getRegisterName(Reg) << ".x";
  } else if (isYComponentReg(Reg)) {
    O << RegNames->getRegisterName(Reg) << ".y";
  } else if (isZComponentReg(Reg)) {
    O << RegNames->getRegisterName(Reg) << ".z";
  } else if (isWComponentReg(Reg)) {
    O << RegNames->getRegisterName(Reg) << ".w";
  } else if (isXYComponentReg(Reg)) {
    O << RegNames->getRegisterName(Reg)
      << (Dst ? ".xy__" : (Dupe ? ".xyxy" : ".xy00"));
  } else if (isZWComponentReg(Reg)) {
    O << RegNames->getRegisterName(Reg)
      << (Dst ? ".__zw" : (Dupe ? ".zwzw" : ".00zw"));
  } else {
    O << RegNames->getRegisterName(Reg);
  }
}

// For the given register used for formal arguments, return the corresponding
// register used for actual arguments at call sites.
// FIXME: Create the wrapper function for kernels before lowering so that
// setup of kernel arguments are done during lowering instead of being done
// during AsmPrinter pass.
unsigned AMDILKernelManager::getActualArgReg(unsigned FormalReg) {
  if (!mSTM->isSupported(AMDIL::Caps::UseMacroForCall)) {
    return FormalReg;
  }

  if (isXComponentReg(FormalReg))
    return FormalReg - AMDIL::INx0 + AMDIL::Rx1;
  if (isYComponentReg(FormalReg))
    return FormalReg - AMDIL::INy0 + AMDIL::Ry1;
  if (isZComponentReg(FormalReg))
    return FormalReg - AMDIL::INz0 + AMDIL::Rz1;
  if (isWComponentReg(FormalReg))
    return FormalReg - AMDIL::INw0 + AMDIL::Rw1;
  if (isXYComponentReg(FormalReg))
    return FormalReg - AMDIL::INxy0 + AMDIL::Rxy1;
  if (isZWComponentReg(FormalReg))
    return FormalReg - AMDIL::INzw0 + AMDIL::Rzw1;
  return FormalReg - AMDIL::IN0 + AMDIL::R1;
}

const char* getFirstComponent(unsigned Reg, unsigned FCall) {
  if (isXComponentReg(Reg) || isYComponentReg(Reg) ||
      isZComponentReg(Reg) || isWComponentReg(Reg)) {
    return ".x";
  } else if (isXYComponentReg(Reg)) {
    switch (FCall) {
      case 1090:
      case 1091:
      case 1092:
        return ".xx";
      default:
        return ".xy";
    }
  } else if (isZWComponentReg(Reg)) {
    switch (FCall) {
      case 1090:
      case 1091:
      case 1092:
        return ".00xx";
      default:
        return ".00xy";
    }
  } else {
    switch (FCall) {
      case 1090:
      case 1091:
        return ".xxxx";
      case 1092:
      case 1093:
        return ".xxyy";
      default:
        return ".xyzw";
    }
  }
}
//#endif

static bool errorPrint(const char *Ptr, raw_ostream &O) {
  if (Ptr[0] == 'E') {
    O << ";error:" << Ptr << '\n';
  } else {
    O << ";warning:" << Ptr << '\n';
  }
  return false;
}

static bool semaPrint(uint32_t Val, raw_ostream &O) {
  O << "dcl_semaphore_id(" << Val << ")\n";
  return false;
}

static bool arenaPrint(uint32_t Val, raw_ostream &O) {
  if (Val >= ARENA_SEGMENT_RESERVED_UAVS) {
    O << "dcl_arena_uav_id(" << Val << ")\n";
  }
  return false;
}

static bool uavPrint(uint32_t Val, raw_ostream &O) {
  if (Val < 8 || Val == 11) {
    O << "dcl_raw_uav_id(" << Val << ")\n";
  }
  return false;
}

static bool printfPrint(std::pair<const std::string, PrintfInfo *> &Data,
                        raw_ostream &O) {
  O << ";printf_fmt:" << Data.second->getPrintfID();
  // Number of operands
  O << ":" << Data.second->getNumOperands();
  // Size of each operand
  for (size_t I = 0, E = Data.second->getNumOperands(); I < E; ++I) {
    O << ":" << (Data.second->getOperandID(I) >> 3);
  }
  const char *Ptr = Data.first.c_str();
  uint32_t Size = Data.first.size() - 1;
  // The format string size
  O << ":" << Size << ":";
  for (size_t I = 0; I < Size; ++I) {
    if (Ptr[I] == '\r') {
      O << "\\r";
    } else if (Ptr[I] == '\n') {
      O << "\\n";
    } else {
      O << Ptr[I];
    }
  }
  O << ";\n";   // c_str() is cheap way to trim
  return false;
}

void AMDILKernelManager::updatePtrArg(Function::const_arg_iterator Ip,
                                      int NumWriteImages,
                                      int raw_uav_buffer,
                                      int Counter,
                                      const Function *F) {
  assert(F && "Cannot pass a NULL Pointer to F!");
  assert(Ip->getType()->isPointerTy() &&
         "Argument must be a pointer to be passed into this function!\n");
  std::string PtrArg(";pointer:");
  const char *SymTab = "NoSymTab";
  uint32_t PtrId = mMFI->getUAVID(Ip);
  PointerType *PT = cast<PointerType>(Ip->getType());
  uint32_t Align = 4;
  const char *MemType = "uav";
  if (PT->getElementType()->isSized()) {
    Align = mTM->getDataLayout()->getTypeAllocSize(PT->getElementType());
    if ((Align & (Align - 1)))
      Align = NextPowerOf2(Align);
  }
  PtrArg += Ip->getName().str() + ":" + getTypeName(PT, SymTab, mMFI,
      mMFI->isSignedIntType(Ip)) + ":1:1:" +
            itostr(Counter * 16) + ":";
  if (mSTM->overridesFlatAS()) {
    MemType = "flat";
    PtrId = 0;
  } else {
    switch (PT->getAddressSpace()) {
      case AMDILAS::FLAT_ADDRESS:
        if (!mSTM->isSupported(AMDIL::Caps::FlatMem)) {
          mMFI->addErrorMsg(amd::CompilerErrorMessage[NO_FLAT_SUPPORT]);
        }
        MemType = "flat";
        PtrId = 0;
        break;

      case AMDILAS::ADDRESS_NONE:
        //O << "No Address space qualifier!";
        mMFI->addErrorMsg(amd::CompilerErrorMessage[INTERNAL_ERROR]);
        break;

      case AMDILAS::GLOBAL_ADDRESS:
        if (mSTM->isSupported(AMDIL::Caps::ArenaSegment)) {
          if (PtrId >= ARENA_SEGMENT_RESERVED_UAVS) {
            PtrId = 8;
          }
        }
        mMFI->uav_insert(PtrId);
        break;

    case AMDILAS::CONSTANT_ADDRESS:
      if (mSTM->usesHardware(AMDIL::Caps::ConstantMem)) {
        const AMDILKernel* T = mAMI->getKernel(mKernelName);
        if (mAMI->usesHWConstant(T, Ip->getName())) {
          MemType = /*(isSI) ? "uc\0" :*/ "hc\0";
          PtrId = mAMI->getConstPtrCB(T, Ip->getName());
          mMFI->setUsesConstant();
        } else {
          MemType = "c\0";
          mMFI->uav_insert(PtrId);
        }
      } else {
        MemType = "c\0";
        mMFI->uav_insert(PtrId);
      }
      break;

    default:
    case AMDILAS::PRIVATE_ADDRESS:
      if (mSTM->usesHardware(AMDIL::Caps::PrivateMem)) {
        MemType = mSTM->isSupported(AMDIL::Caps::PrivateUAV)
                ? "up\0" : "hp\0";
        mMFI->setUsesScratch();
      } else {
        MemType = "p\0";
        mMFI->uav_insert(PtrId);
      }
      break;

    case AMDILAS::REGION_ADDRESS:
      if (mSTM->usesHardware(AMDIL::Caps::RegionMem)) {
        MemType = "hr\0";
        PtrId = 0;
        mMFI->setUsesGDS();
      } else {
        MemType = "r\0";
        mMFI->uav_insert(PtrId);
      }
      break;

    case AMDILAS::LOCAL_ADDRESS:
      if (mSTM->usesHardware(AMDIL::Caps::LocalMem)) {
        MemType = "hl\0";
        // size of local mem pointed to by ptr type args are unknown,
        // so go to default lds buffer
        PtrId = DEFAULT_LDS_ID;
        mMFI->setUsesLDS();
      } else {
        MemType = "l\0";
        mMFI->uav_insert(PtrId);
      }
      break;
    }
  }
  PtrArg += std::string(MemType) + ":";
  PtrArg += itostr(PtrId) + ":";
  PtrArg += itostr(Align) + ":";
  const Value *Ptr = Ip;
  if (mMFI->read_ptr_count(Ptr)) {
    PtrArg += "RO";
  // FIXME: add write-only pointer detection.
  //} else if (mMFI->write_ptr_count(ptr)) {
  //  PtrArg += "WO";
  } else {
    PtrArg += "RW";
  }
  PtrArg += mMFI->isVolatilePointer(Ip) ? ":1" : ":0";
  PtrArg += mMFI->isRestrictPointer(Ip) ? ":1" : ":0";
  mMFI->addMetadata(PtrArg, true);
}

AMDILKernelManager::AMDILKernelManager(MachineFunction *MF)
  : mUniqueID(0), mIsKernel(false), mWasKernel(false),
    mMF(MF)
{
  mTM = reinterpret_cast<const AMDILTargetMachine*>(&MF->getTarget());
  mSTM = mTM->getSubtargetImpl();
  mMFI = MF->getInfo<AMDILMachineFunctionInfo>();
  mAMI = &(MF->getMMI().getObjFileInfo<AMDILModuleInfo>());

  mKernelName = MF->getName();
  mName = AMDSymbolNames::undecorateKernelFunctionName(mKernelName);
  mStubName = mSTM->isSupported(AMDIL::Caps::UseMacroForCall)?
      AMDSymbolNames::decorateStubFunctionName(mName):mName;

}

AMDILKernelManager::~AMDILKernelManager() {
}

bool AMDILKernelManager::useCompilerWrite(const MachineInstr *MI) {
  return (MI->getOpcode() == AMDIL::RETURN && wasKernel() &&
    !mMFI->getOutputInst());
}

void AMDILKernelManager::processArgMetadata(raw_ostream &O, uint32_t Buf)
{
  const Function *F = mMF->getFunction();
  const char *SymTab = "NoSymTab";
  Function::const_arg_iterator Ip = F->arg_begin();
  Function::const_arg_iterator Ep = F->arg_end();

  if (F->hasStructRetAttr()) {
    assert(Ip != Ep && "Invalid struct return function!");
    ++Ip;
  }
  uint32_t mCBSize = 0;
  int raw_uav_buffer = mSTM->getResourceID(AMDIL::RAW_UAV_ID);
  bool MultiUAV = mSTM->isSupported(AMDIL::Caps::MultiUAV);
  bool ArenaSegment = mSTM->isSupported(AMDIL::Caps::ArenaSegment);
  int NumWriteImages = mMFI->get_num_write_images();
  if (NumWriteImages == OPENCL_MAX_WRITE_IMAGES || MultiUAV || ArenaSegment) {
    if (mSTM->getGeneration() <= AMDIL::NORTHERN_ISLANDS) {
      raw_uav_buffer = mSTM->getResourceID(AMDIL::ARENA_UAV_ID);
    }
  }
  uint32_t CounterNum = 0;
  uint32_t SemaNum = 1;
  uint32_t ROArg = 0;
  uint32_t WOArg = 0;
  uint32_t NumArg = 0;
  while (Ip != Ep) {
    Type *CType = Ip->getType();
    if (CType->isIntOrIntVectorTy() || CType->isFPOrFPVectorTy()) {
      std::string argMeta(";value:");
      argMeta += Ip->getName().str() + ":" + getTypeName(CType, SymTab, mMFI
          , mMFI->isSignedIntType(Ip)) + ":";
      int BitSize = CType->getPrimitiveSizeInBits();
      int NumEle = 1;
      if (CType->getTypeID() == Type::VectorTyID) {
        NumEle = cast<VectorType>(CType)->getNumElements();
      }
      argMeta += itostr(NumEle) + ":1:" + itostr(mCBSize << 4);
      mMFI->addMetadata(argMeta, true);

      // FIXME: simplify
      if ((BitSize / NumEle) < 32) {
        BitSize = NumEle >> 2;
      } else {
        BitSize >>= 7;
      }
      if (!BitSize) {
        BitSize = 1;
      }

      mCBSize += BitSize;
    } else if (const PointerType *PT = dyn_cast<PointerType>(CType)) {
      Type *CT = PT->getElementType();
      const StructType *ST = dyn_cast<StructType>(CT);
      if (ST && ST->isOpaque()) {
        StringRef Name = ST->getName();
        bool i1d  = Name.startswith( "struct._image1d_t" );
        bool i1da = Name.startswith( "struct._image1d_array_t" );
        bool i1db = Name.startswith( "struct._image1d_buffer_t" );
        bool i2d  = Name.startswith( "struct._image2d_t" );
        bool i2da = Name.startswith( "struct._image2d_array_t" );
        bool i3d  = Name.startswith( "struct._image3d_t" );
        bool c32  = Name.startswith( "struct._counter32_t" );
        bool c64  = Name.startswith( "struct._counter64_t" );
        bool sema = Name.startswith( "struct._sema_t" );
        if (i1d || i1da || i1db || i2d | i2da || i3d) {
          if (mSTM->isSupported(AMDIL::Caps::Images)) {
            SmallString<16> ImageArg(";image:");
            ImageArg.append(Ip->getName());
            ImageArg.append(":");
            if (i1d)
              ImageArg += "1D:";
            else if (i1da)
              ImageArg += "1DA:";
            else if (i1db)
              ImageArg += "1DB:";
            else if (i2d)
              ImageArg += "2D:";
            else if (i2da)
              ImageArg += "2DA:";
            else if (i3d)
              ImageArg += "3D:";

              if (mAMI->isReadOnlyImage (mMF->getFunction()->getName(),
                                        (ROArg + WOArg))) {
                ImageArg += "RO:" + itostr(ROArg);
                O << "dcl_resource_id(" << ROArg << ")_type(";
                if (i1d)
                  O << "1d";
                else if (i1da)
                  O << "1darray";
                else if (i1db)
                  O << "buffer";
                else if (i2d)
                  O << "2d";
                else if (i2da)
                  O << "2darray";
                else if (i3d)
                  O << "3d";

                O << ")_fmtx(unknown)_fmty(unknown)"
                  << "_fmtz(unknown)_fmtw(unknown)\n";
                ++ROArg;
              } else if (mAMI->isWriteOnlyImage(mMF->getFunction()->getName(),
                                               (ROArg + WOArg))) {
                uint32_t offset = 0;
                offset += WOArg;
                ImageArg += "WO:" + itostr(offset & 0x7);
                O << "dcl_uav_id(" << ((offset) & 0x7) << ")_type(";
                if (i1d)
                  O << "1d";
                else if (i1da)
                  O << "1darray";
                else if (i1db)
                  O << "buffer";
                else if (i2d)
                  O << "2d";
                else if (i2da)
                  O << "2darray";
                else if (i3d)
                  O << "3d";

                O << ")_fmtx(uint)\n";
                ++WOArg;
              } else {
                ImageArg += "RW:" + itostr(ROArg + WOArg);
              }
            ImageArg += ":1:" + itostr(mCBSize * 16);
            mMFI->addMetadata(ImageArg, true);
            mMFI->addi32Literal(mCBSize);
            mCBSize += NUM_EXTRA_SLOTS_PER_IMAGE + 1;
          } else {
            mMFI->addErrorMsg(amd::CompilerErrorMessage[NO_IMAGE_SUPPORT]);
          }
        } else if (c32 || c64) {
          std::string CounterArg(";counter:");
          CounterArg += Ip->getName().str() + ":"
            + itostr(c32 ? 32 : 64) + ":"
            + itostr(CounterNum++) + ":1:" + itostr(mCBSize * 16);
          mMFI->addMetadata(CounterArg, true);
          ++mCBSize;
        } else if (sema) {
          std::string semaArg(";sema:");
          semaArg += Ip->getName().str() + ":" + itostr(SemaNum++)
            + ":1:" + itostr(mCBSize * 16);
          mMFI->addMetadata(semaArg, true);
          ++mCBSize;
        } else {
          updatePtrArg(Ip, NumWriteImages, raw_uav_buffer, mCBSize, F);
          ++mCBSize;
        }
      } else if (CT->getTypeID() == Type::StructTyID &&
                 PT->getAddressSpace() == AMDILAS::PRIVATE_ADDRESS) {
        const DataLayout *DL = mTM->getDataLayout();
        const StructLayout *SL = DL->getStructLayout(dyn_cast<StructType>(CT));
        int ByteSize = SL->getSizeInBytes();
        int ReservedSize = (ByteSize + 15) & ~15;
        int NumSlots = ReservedSize >> 4;
        if (!NumSlots) {
          NumSlots = 1;
        }

        SmallString<16> StructArg(";value:");
        StructArg.append(Ip->getName());
        StructArg += ":struct:" + itostr(ByteSize) + ":1:" + itostr(mCBSize * 16);
        mMFI->addMetadata(StructArg, true);
        mCBSize += NumSlots;
      } else if (CT->isIntOrIntVectorTy()
                 || CT->isFPOrFPVectorTy()
                 || CT->getTypeID() == Type::ArrayTyID
                 || CT->getTypeID() == Type::PointerTyID
                 || PT->getAddressSpace() != AMDILAS::PRIVATE_ADDRESS) {
        updatePtrArg(Ip, NumWriteImages, raw_uav_buffer, mCBSize, F);
        ++mCBSize;
      } else {
        llvm_unreachable("Cannot process current pointer argument");
        mMFI->addErrorMsg(amd::CompilerErrorMessage[INTERNAL_ERROR]);
      }
    } else {
      llvm_unreachable("Cannot process current kernel argument");
      mMFI->addErrorMsg(amd::CompilerErrorMessage[INTERNAL_ERROR]);
    }
    if (mMFI->isConstantArgument(Ip)) {
      std::string ConstArg(";constarg:");
      ConstArg += itostr(NumArg) + ":" + Ip->getName().str();
      mMFI->addMetadata(ConstArg, true);
    }
    ++NumArg;
    ++Ip;
  }
}

void AMDILKernelManager::printWrapperHeader(AMDILAsmPrinter *AsmPrinter,
                                            raw_ostream &O) {
  const std::string &KernelName
    = (mSTM->isApple() ? "__OpenCL_" + mKernelName + "_kernel"
                       : mKernelName);

  int KernelId = mAMI->getOrCreateFunctionID(KernelName);
  O << "func " << KernelId << " ; " << KernelName << "\n";
}

static void addLocalBufferSizes(std::vector<unsigned>& LocalBufferSizes,
                                const AMDILLocalArg* Locals,
                                bool Region) {
  llvm::SmallVector<AMDILArrayMem *, DEFAULT_VEC_SLOTS>::const_iterator I, E;
  for (I = Locals->local.begin(), E = Locals->local.end(); I != E; ++I) {
    const AMDILArrayMem* Local = *I;
    if (!Local->isHW || (Local->isRegion != Region)) {
      continue;
    }

    assert(Local->resourceID != 0 && "bad resourceID");
    uint32_t Size = (Local->vecSize + 3) & ~3;
    LocalBufferSizes[Local->resourceID - DEFAULT_LDS_ID] += Size;
  }
}

void AMDILKernelManager::printGroupSize(raw_ostream &O) {
  // If the launch size is specified via a kernel attribute, we print it
  // here. Otherwise we use the the default size.
  const AMDILKernel *Kernel = mAMI->getKernel(mKernelName);

    // Otherwise we generate for devices that support 3D launch natively.  If
    // the reqd_workgroup_size attribute was specified, then we can specify the
    // exact launch dimensions.
    if (Kernel && Kernel->sgv) {
      if (Kernel->sgv->mHasRWG) {
        O << "dcl_num_thread_per_group "
          << Kernel->sgv->reqGroupSize[0] << ", "
          << Kernel->sgv->reqGroupSize[1] << ", "
          << Kernel->sgv->reqGroupSize[2] << "\n";
      } else {
        // Otherwise we specify the largest workgroup size that can be launched.
          O << "dcl_max_thread_per_group " <<
            Kernel->sgv->reqGroupSize[0]
            * Kernel->sgv->reqGroupSize[1]
            * Kernel->sgv->reqGroupSize[2] << "\n";
        }

      if (Kernel->sgv->mHasRWR) {
        O << "dcl_gws_thread_count " <<
          Kernel->sgv->reqRegionSize[0]
          * Kernel->sgv->reqRegionSize[1]
          * Kernel->sgv->reqRegionSize[2] << "\n";
      }
    } else {
      O << "dcl_max_thread_per_group " << mSTM->getWavefrontSize() << "\n";
    }


  // Now that we have specified the workgroup size, lets declare the local
  // memory size. If we are using hardware and we know the value at compile
  // time, then we need to declare the correct value. Otherwise we should just
  // declare the maximum size.
  DEBUG_WITH_TYPE("func_supp", dbgs() << "[decl lds] " << mName <<
      " usesLDS = " << mMFI->usesLDS() <<
      " curHWSize = " << Kernel->curHWSize << '\n');
  if (mSTM->usesHardware(AMDIL::Caps::LocalMem)
      && (mMFI->usesLDS() || Kernel->curHWSize > 0)) {
    size_t KernelLocalSize = (Kernel->curHWSize + 3) & ~3;
    if (KernelLocalSize > mSTM->getMaxLDSSize()) {
      mMFI->addErrorMsg(amd::CompilerErrorMessage[INSUFFICIENT_LOCAL_RESOURCES]);
    }

    // declare non-default local buffers
    unsigned nLocals = mAMI->numLocalBuffers();
    std::vector<unsigned> LocalBufferSizes(nLocals, 0);

    addLocalBufferSizes(LocalBufferSizes, Kernel->lvgv, false);
    addLocalBufferSizes(LocalBufferSizes, &Kernel->reservedLocals, false);

    unsigned NDefSize = 0;
    for (unsigned i = 1; i < nLocals; ++i) {
      unsigned Size = LocalBufferSizes[i];
      O << "dcl_lds_id(" << DEFAULT_LDS_ID + i << ") " << Size << "\n";
      NDefSize += Size;
    }

    // If there is a local pointer as a kernel argument, we don't know the size
    // at compile time, so we reserve all of the space.
    unsigned DefLocalSize = LocalBufferSizes[0];
    if (mMFI->hasLDSArg()) {
      DefLocalSize = mSTM->getMaxLDSSize() - NDefSize;
    }

    // Declare the default local buffer
    if (DefLocalSize > 0) {
      O << "dcl_lds_id(" << DEFAULT_LDS_ID << ") " << DefLocalSize << "\n";
    }
    mMFI->setUsesMem(AMDIL::LDS_ID);
  }

  // TODO: GDS should use same per-pointer optimization as LDS

  // If the device supports the region memory extension, which maps to our
  // hardware GDS memory, then lets declare it so we can use it later on.
  if (mSTM->usesHardware(AMDIL::Caps::RegionMem)
      && mMFI->usesGDS()) {
    size_t KernelGDSSize = (Kernel->curHWRSize + 3) & ~3;
    if (KernelGDSSize > mSTM->getMaxGDSSize()) {
      mMFI->addErrorMsg(amd::CompilerErrorMessage[INSUFFICIENT_REGION_RESOURCES]);
    }
    // If there is a region pointer as a kernel argument, we don't know the size
    // at compile time, so we reserved all of the space.
    if (mMFI->usesGDS() && (mMFI->hasGDSArg() || !KernelGDSSize)) {
      O << "dcl_gds_id(" << DEFAULT_GDS_ID <<
        ") " << mSTM->getMaxGDSSize() << "\n";
      mMFI->setUsesMem(AMDIL::GDS_ID);
    } else if (KernelGDSSize) {
      // We know the size, so lets declare it.
      O << "dcl_gds_id(" << DEFAULT_GDS_ID <<
        ") " << KernelGDSSize << "\n";
      mMFI->setUsesMem(AMDIL::GDS_ID);
    }
  }
}

void AMDILKernelManager::printWavefrontSize(raw_ostream &O) {
  if (mSTM->isSupported(AMDIL::Caps::CrossThreadOps)) {
    O << "dcl_wavesize " << mSTM->getWavefrontSize() << '\n';
  }
}

void AMDILKernelManager::printDecls(AMDILAsmPrinter *AsmPrinter,
                                    raw_ostream &O) {
  // If we are supporting multiple uav's view the MultiUAV capability,
  // then we need to print out the declarations here. MultiUAV
  // conflicts with write images, so they only use 8 - NumWriteImages
  // uav's. Therefor only pointers with ID's < 8 will get printed.
  if (mSTM->isSupported(AMDIL::Caps::MultiUAV)) {
    binaryForEach(mMFI->uav_begin(), mMFI->uav_end(), uavPrint, O);
    mMFI->setUsesMem(AMDIL::RAW_UAV_ID);
  }
  // If arena segments are supported, then we should emit them now.  Arena
  // segments are similiar to MultiUAV, except ArenaSegments are virtual and up
  // to 1024 of them can coexist. These are more compiler hints for CAL and thus
  // cannot overlap in any form.  Each ID maps to a seperate piece of memory and
  // CAL determines whether the load/stores should go to the fast path/slow path
  // based on the usage and instruction.
  if (mSTM->isSupported(AMDIL::Caps::ArenaSegment)) {
    binaryForEach(mMFI->uav_begin(), mMFI->uav_end(), arenaPrint, O);
  }

  if (mMFI->sema_size()
      && !mSTM->usesHardware(AMDIL::Caps::Semaphore)) {
    mMFI->addErrorMsg(amd::CompilerErrorMessage[NO_SEMAPHORE_SUPPORT]);
  } else {
    binaryForEach(mMFI->sema_begin(), mMFI->sema_end(), semaPrint, O);
  }
  // Now that we have printed out all of the arena and multi uav
  // declaration, now we must print out the default raw uav id. This
  // always exists on HD5XXX and HD6XXX hardware. The reason is that
  // the hardware supports 12 UAV's and 11 are taken up by
  // MultiUAV/Write Images and Arena.  However, if we do not have UAV
  // 11 as the raw UAV and there are 8 write images, we must revert
  // everything to the arena and not print out the default raw uav id.
  if (mSTM->getGeneration() == AMDIL::EVERGREEN
      || mSTM->getGeneration() == AMDIL::NORTHERN_ISLANDS) {
    if ((mSTM->getResourceID(AMDIL::RAW_UAV_ID) < 11 &&
          mMFI->get_num_write_images()
         != OPENCL_MAX_WRITE_IMAGES
         && !mSTM->isSupported(AMDIL::Caps::MultiUAV))
        || mSTM->getResourceID(AMDIL::RAW_UAV_ID) == 11) {
      if (!mMFI->usesMem(AMDIL::RAW_UAV_ID) &&
          mMFI->uav_count(mSTM->getResourceID(AMDIL::RAW_UAV_ID))) {
        O << "dcl_raw_uav_id("
          << mSTM->getResourceID(AMDIL::RAW_UAV_ID);
        O << ")\n";
        mMFI->setUsesMem(AMDIL::RAW_UAV_ID);
      }
    }
    // If we have not printed out the arena ID yet, then do so here.
    if (!mMFI->usesMem(AMDIL::ARENA_UAV_ID) &&
        mSTM->usesHardware(AMDIL::Caps::ArenaUAV)) {
      O << "dcl_arena_uav_id("
        << mSTM->getResourceID(AMDIL::ARENA_UAV_ID) << ")\n";
      mMFI->setUsesMem(AMDIL::ARENA_UAV_ID);
    }
  } else if (mSTM->getGeneration() > AMDIL::NORTHERN_ISLANDS
    && !mSTM->overridesFlatAS()) {
    // Build a map that store whether a UAV is read-only by going through
    // the pointer-to-id map and see if the pointer is read-only.
    std::map<uint32_t, bool> UAVReadOnlyMap;
    for (AMDILMachineFunctionInfo::value_id_iterator I
      = mMFI->value_id_map_begin(), E = mMFI->value_id_map_end(); I != E; ++I)
    {
            unsigned UAVID = I->second;
            bool PtrReadOnly = mMFI->read_ptr_count(I->first);
      std::map<uint32_t, bool>::iterator I2 = UAVReadOnlyMap.find(UAVID);
      if (I2 == UAVReadOnlyMap.end())
        UAVReadOnlyMap[UAVID] = PtrReadOnly;
            else
              I2->second = I2->second && PtrReadOnly;
          }
    for (llvm::uav_iterator i = mMFI->uav_begin(), e = mMFI->uav_end();
      i != e; ++i) {
        bool read_only = false;
        if (!mSTM->usesHardware(AMDIL::Caps::ConstantMem)
          && mSTM->getResourceID(AMDIL::CONSTANT_ID) == *i) {
            read_only = true;
        } else {
        std::map<uint32_t, bool>::iterator I2 = UAVReadOnlyMap.find(*i);
        if (I2 != UAVReadOnlyMap.end())
            read_only = I2->second;
        }
        O << "dcl_typeless_uav_id(" << *i << ")_stride(4)_length(4)_access("
          << (read_only ? "read_only)\n" : "read_write)\n");
      }
      mMFI->setUsesMem(AMDIL::RAW_UAV_ID);
  }
}

void AMDILKernelManager::getIntrinsicSetup(AMDILAsmPrinter *AsmPrinter,
                                           raw_ostream &O) {
  if (!mIsKernel)
    return;

  // declare the itemp array to store the global values calculated below
  O << "dcl_indexed_temp_array x0[" << NUM_ITEMP_SLOTS << "]\n";

  if (mSTM->is64bit()) {
    O << "mov " << AsmPrinter->getRegisterName(AMDIL::SDP) << ", cb0[8].xy\n";
  } else {
    O << "mov " << AsmPrinter->getRegisterName(AMDIL::SDP) << ", cb0[8].x\n";
  }
  O << "mov " << AsmPrinter->getRegisterName(AMDIL::SP) << ", l0.0000\n";
  O << "mov " << AsmPrinter->getRegisterName(AMDIL::FP) << ", l0.0000\n";

  O << "mov r0.__z_, vThreadGrpIdFlat0.x\n"
    << "mov r1022.xyz0, vTidInGrp0.xyz\n"
    << "mov r1023.xyz0, vThreadGrpId0.xyz\n";

  // Calculates the global id.
  const AMDILKernel *Kernel = mAMI->getKernel(mKernelName);
  if (Kernel && Kernel->sgv && Kernel->sgv->mHasRWG) {
    // Anytime we declare a literal, we need to reserve it, if it is not emitted
    // in emitLiterals.
    O << "dcl_literal l9, "
      << Kernel->sgv->reqGroupSize[0] << ", "
      << Kernel->sgv->reqGroupSize[1] << ", "
      << Kernel->sgv->reqGroupSize[2] << ", "
      << "0xFFFFFFFF\n";
    O << "imad r1021.xyz0, r1023.xyzz, l9.xyzz, r1022.xyzz\n";
  } else {
    O << "dcl_literal l9, "
      << mSTM->getDefaultSize(0) << ", "
      << mSTM->getDefaultSize(1) << ", "
      << mSTM->getDefaultSize(2) << ", "
      << "0xFFFFFFFF\n";
    // This umax is added so that on SI or later architectures, the
    // ISA generator can do value range analysis to determine that cb0[1]
    // is a positive value or not.
    if (mSTM->getGeneration() > AMDIL::NORTHERN_ISLANDS) {
      O << "umin r1023.xyz0, r1023.xyzz, l9.w\n";
      O << "umin r1021.xyz0, cb0[1].xyzz, l1.x\n";
      O << "imad r1021.xyz0, r1023.xyzz, r1021.xyzz, r1022.xyzz\n";
    } else {
      O << "imad r1021.xyz0, r1023.xyzz, cb0[1].xyzz, r1022.xyzz\n";
    }
  }

  // These umax's are added so that on SI or later architectures, the
  // ISA generator can do value range analysis to determine that cb0[1]
  // is a positive value or not.
  // Add the global/group offset for multi-launch support.
  if (mSTM->getGeneration() > AMDIL::NORTHERN_ISLANDS) {
    O << "umin r1024.xyz0, cb0[6].xyzz, l9.w\n"
      << "iadd r1021.xyz0, r1021.xyz0, r1024.xyz0\n"
      << "umin r1024.xyz0, cb0[7].xyzz, l9.w\n"
      << "iadd r1023.xyz0, r1023.xyz0, r1024.xyz0\n";
  } else {
    O << "iadd r1021.xyz0, r1021.xyz0, cb0[6].xyz0\n"
      << "iadd r1023.xyz0, r1023.xyz0, cb0[7].xyz0\n";
  }
  // moves the flat group id.
  O << "mov r1023.___w, r0.z\n";
  const char *TmpReg2 = AsmPrinter->getRegisterName(AMDIL::R2);
  if (mSTM->usesSoftware(AMDIL::Caps::LocalMem)) {
    if (mSTM->is64bit()) {
      O << "umul " << TmpReg2 << ".x0__, r1023.w, cb0[4].z\n"
        << "i64add " << TmpReg2 << ".xy__, " << TmpReg2
        << ".xyyy, cb0[4].xyyy\n";

    } else {
      O << "imad " << TmpReg2 << ".x___, r1023.w, cb0[4].y, cb0[4].x\n";
    }
    O << "mov x0[" << ITEMP_SLOT_T2 << "], " << TmpReg2 << "\n";
  }
  // Shift the flat group id to be in bytes instead of dwords.
  O << "ishl r1023.___w, r1023.w, l0.z\n";
  const char *TmpReg1 = AsmPrinter->getRegisterName(AMDIL::R1);
  if (mSTM->usesSoftware(AMDIL::Caps::PrivateMem)) {
    if (mSTM->is64bit()) {
      O << "umul " << TmpReg1 << ".x0__, vAbsTidFlat.x, cb0[3].z\n"
        << "i64add " << TmpReg1 << ".xy__, " << TmpReg1
        << ".xyyy, cb0[3].xyyy\n";

    } else {
      O << "imad " << TmpReg1 << ".x___, vAbsTidFlat.x, cb0[3].y, cb0[3].x\n";
    }
  } else {
    O << "mov " << TmpReg1 << ".x___, l0.0000\n";
  }
  O << "mov x0[" << ITEMP_SLOT_T1 << "], " << TmpReg1 << "\n";

  if (mSTM->isSupported(AMDIL::Caps::RegionMem)) {
    O << "udiv r1024.xyz_, r1021.xyzz, cb0[10].xyzz\n";
    if (Kernel && Kernel->sgv && Kernel->sgv->mHasRWR) {
      O << "dcl_literal l10,"
        << Kernel->sgv->reqRegionSize[0] << ", "
        << Kernel->sgv->reqRegionSize[1] << ", "
        << Kernel->sgv->reqRegionSize[2] << ", "
        << "0\n"
        << "imad r1025.xyz0, r1023.xyzz, l10.xyzz, r1022.xyzz\n";
    } else {
      O << "imad r1025.xyz0, r1023.xyzz, cb0[10].xyzz, r1022.xyzz\n";
    }
  }
  if (!mMFI->printf_empty()) {
    O << "mov " << AsmPrinter->getRegisterName(AMDIL::PRINTF) << ".x, l0.y\n";
    O << "mov x0[" << ITEMP_SLOT_PRIVATE_OFFSET << "], " << TmpReg1 << "\n";
  }
  O << "mov x0[" << ITEMP_SLOT_GLOBAL_ID << "], r1021\n";
  O << "mov x0[" << ITEMP_SLOT_GROUP_ID << "], r1023\n";
}

void AMDILKernelManager::printWrapperFooter(raw_ostream &O) {
  O << "ret\n";
  if (mSTM->isApple()) {
    O << "endfunc ; __OpenCL_" << mKernelName << "_kernel\n";
  } else {
    O << "endfunc ; " << mKernelName << "\n";
  }
}

void AMDILKernelManager::printMetaData(raw_ostream &O,
                                       uint32_t ID,
                                       bool IsWrapper) {
  printKernelArgs(O, IsWrapper);
}

void AMDILKernelManager::setKernel(bool Kernel) {
  mIsKernel = Kernel;
  if (Kernel) {
    mWasKernel = mIsKernel;
  }
}

void AMDILKernelManager::setID(uint32_t ID) {
  mUniqueID = ID;
}

void AMDILKernelManager::setName(StringRef Name) {
  mName = Name;
}

bool AMDILKernelManager::wasKernel() {
  return mWasKernel;
}
//#if 0
void AMDILKernelManager::printConstantToRegMapping(
       AMDILAsmPrinter *RegNames,
       unsigned &LII,
       raw_ostream &O,
       uint32_t &Counter,
       uint32_t Buffer,
       uint32_t N,
       const char *Lit,
       uint32_t FCall,
       bool IsImage,
       bool IsHWCB) {
  // TODO: This needs to be enabled or SC will never statically index into the
  // CB when a pointer is used.
  if (mSTM->usesHardware(AMDIL::Caps::ConstantMem) && IsHWCB) {
    O << "mov ";
    printRegName(RegNames, getActualArgReg(mMFI->getArgReg(LII)), O, true);
    O << ", l5.x\n";
    ++LII;
    Counter++;
    return;
  }

  for (uint32_t I = 0; I < N; ++I) {
    uint32_t Reg = getActualArgReg(mMFI->getArgReg(LII));
    const char *RegName = RegNames->getRegisterName(Reg);
    O << "mov ";
    printRegName(RegNames, Reg, O, true);
    if (IsImage) {
      O << ", l" << mMFI->getLitIdx(Counter++) << "\n";
    } else {
      O << ", cb" <<Buffer<< "[" <<Counter++<< "]"
        << getFirstComponent(Reg, FCall) << "\n";
    }
    switch (FCall) {
    case 1093:
    case 1092:
      O << "ishr ";
      printRegName(RegNames, Reg, O, true);
      O << ", ";
      printRegName(RegNames, Reg, O, false);
      O << ", l3.0y0y\n";
      if (!Lit) {
        O << "ishl " << RegName << ", " << RegName<< ", l3.z\n";
        O << "ishr " << RegName << ", " << RegName<< ", l3.z\n";
      }
      break;
    case 1091:
      O << "ishr ";
      printRegName(RegNames, Reg, O, true);
      O << ", ";
      printRegName(RegNames, Reg, O, false);
      O << ", l3.0zyx\n";
      if (!Lit) {
        O << "ishl " << RegName << ", " << RegName<< ", l3.x\n";
        O << "ishr " << RegName << ", " << RegName<< ", l3.x\n";
      }
      break;
    case 1090:
      O << "ishr ";
      printRegName(RegNames, Reg, O, true);
      O << ", ";
      printRegName(RegNames, Reg, O, false);
      O << ", l3.0z0z\n";
      if (!Lit) {
        O << "ishl " << RegName << ", " << RegName<< ", l3.x\n";
        O << "ishr " << RegName << ", " << RegName<< ", l3.x\n";
      }
      break;
    default:
      break;
    }

    if (Lit) {
      O << "ishl " ;
      printRegName(RegNames, Reg, O, true);
      O << ", ";
      printRegName(RegNames, Reg, O, false, true);
      O << ", " << Lit << "\nishr ";
      printRegName(RegNames, Reg, O, true);
      O << ", ";
      printRegName(RegNames, Reg, O, false, true);
      O << ", " << Lit << "\n";
    }
    ++LII;
    if (IsImage) {
      Counter += NUM_EXTRA_SLOTS_PER_IMAGE;
    }
  }
}

void AMDILKernelManager::printCopyStructPrivate(const StructType *ST,
                                                raw_ostream &O,
                                                size_t stackSize,
                                                uint32_t Buffer,
                                                uint32_t mLitIdx,
                                                uint32_t &Counter) {
  size_t n = ((stackSize + 15) & ~15) >> 4;
  for (size_t x = 0; x < n; ++x) {
    if (mSTM->usesHardware(AMDIL::Caps::PrivateUAV)) {
      O << "uav_raw_store_id(" <<
        mSTM->getResourceID(AMDIL::SCRATCH_ID)
        << ") mem0, r0.x, cb" << Buffer << "[" << Counter++ << "]\n";
    } else if (mSTM->usesHardware(AMDIL::Caps::PrivateMem)) {
      O << "ishr r0.y, r0.x, l5.y\n";
      O << "mov x" << mSTM->getResourceID(AMDIL::SCRATCH_ID)
        <<"[r0.y], cb" << Buffer << "[" << Counter++ << "]\n";
    } else {
      O << "uav_raw_store_id(" <<
        mSTM->getResourceID(AMDIL::GLOBAL_ID)
        << ") mem0, r0.x, cb" << Buffer << "[" << Counter++ << "]\n";
    }
    O << "iadd r0.x, r0.x, l" << mLitIdx << ".z\n";
  }
}
//#endif

void AMDILKernelManager::printKernelArgs(raw_ostream &O, bool IsWrapper) {
  std::string Version(";version:");
  Version += itostr(mSTM->supportMetadata30() ? AMDIL_MAJOR_VERSION : 2) + ":"
    + itostr(AMDIL_MINOR_VERSION) + ":"
    + itostr(mSTM->supportMetadata30()
        ? AMDIL_REVISION_NUMBER : AMDIL_20_REVISION_NUMBER);
  const AMDILKernel *Kernel = mAMI->getKernel(mKernelName);

  StringRef Name = IsWrapper ? mKernelName : mName;
  if (mSTM->isApple() && IsWrapper) {
    Name = std::string("__OpenCL_") + mName + std::string("_kernel");
    }

  O << ";ARGSTART:" << Name<< '\n';
  if (IsWrapper) {
    O << Version << '\n';
    O << ";device:" << mSTM->getDeviceName() << '\n';
  }
  unsigned UniqueID = IsWrapper ?
                      mAMI->getOrCreateFunctionID(Name) : mUniqueID;
  O << ";uniqueid:" << UniqueID << '\n';

  if (Kernel && IsWrapper) {
    size_t Region = Kernel->curRSize;
    size_t HWRegion = (Kernel->curHWRSize + 3) & (~0x3);
    bool UseHWRegion = mSTM->usesHardware(AMDIL::Caps::RegionMem);
    if (!mSTM->overridesFlatAS()) {
      size_t Local = Kernel->curSize;
      size_t HWLocal = (Kernel->curHWSize + 3) & (~0x3);
      bool UseHWLocal = mSTM->usesHardware(AMDIL::Caps::LocalMem);
      bool UseHWPrivate = mSTM->usesHardware(AMDIL::Caps::PrivateMem);
      bool UseUAVPrivate = mSTM->isSupported(AMDIL::Caps::PrivateUAV);
      O << ";memory:"
        << (UseHWPrivate ? (UseUAVPrivate ? "uav" : "hw") : "") << "private:"
        << getReservedStackSize() << "\n";
      DEBUG_WITH_TYPE("noinlines", llvm::dbgs() << "[private uav size] " <<
        Name << ": " << getReservedStackSize() << '\n');
      O << ";memory:" << (UseHWLocal ? "hw" : "") << "local:"
          << (UseHWLocal ? HWLocal : HWLocal + Local) << "\n";
    }
    if (mSTM->isSupported(AMDIL::Caps::RegionMem)) {
      O << ";memory:" << (UseHWRegion ? "hw" : "") << "region:"
        << (UseHWRegion ? HWRegion : HWRegion + Region) << "\n";
    }

    if (Kernel->sgv) {
      if (Kernel->sgv->mHasRWG) {
        O << ";cws:"
          << Kernel->sgv->reqGroupSize[0] << ":"
          << Kernel->sgv->reqGroupSize[1] << ":"
          << Kernel->sgv->reqGroupSize[2] << "\n";
      }
      if (Kernel->sgv->mHasRWR) {
        O << ";crs:"
          << Kernel->sgv->reqRegionSize[0] << ":"
          << Kernel->sgv->reqRegionSize[1] << ":"
          << Kernel->sgv->reqRegionSize[2] << "\n";
      }
      if (Kernel->sgv->mHasWGH) {
        O << ";wsh:"
          << Kernel->sgv->groupSizeHint[0] << ":"
          << Kernel->sgv->groupSizeHint[1] << ":"
          << Kernel->sgv->groupSizeHint[2] << '\n';
       }
      if (Kernel->sgv->mHasVTH) {
        O << ";vth:"
          << Kernel->sgv->vecTypeHint << '\n';
      }
    }
  }
  if (IsWrapper) {
    for (std::vector<std::string>::iterator I = mMFI->kernel_md_begin(),
           E = mMFI->kernel_md_end(); I != E; ++I) {
      O << *I << '\n';
    }
  }
  for (std::set<std::string>::iterator I = mMFI->func_md_begin(),
         E = mMFI->func_md_end(); I != E; ++I) {
    O << *I << '\n';
  }
  if (IsWrapper) {
    // printing for the kernel wrapper.
    uint32_t WrapperID = mAMI->getOrCreateFunctionID(mKernelName);
    uint32_t StubID = mAMI->getOrCreateFunctionID(mStubName);
    O << ";function:" << 1 << ":" << StubID << '\n';
  } else {
  if (!mMFI->func_empty()) {
    O << ";function:" << mMFI->func_size();
    binaryForEach(mMFI->func_begin(), mMFI->func_end(), commaPrint, O);
    O << '\n';
  }
  }

  if (!mSTM->isSupported(AMDIL::Caps::MacroDB)
      && !mMFI->intr_empty()) {
    O << ";intrinsic:" << mMFI->intr_size();
    binaryForEach(mMFI->intr_begin(), mMFI->intr_end(), commaPrint, O);
    O << '\n';
  }

  if (!IsWrapper) {
    binaryForEach(mMFI->printf_begin(), mMFI->printf_end(), printfPrint, O);
    mMF->getMMI().getObjFileInfo<AMDILModuleInfo>().add_printf_offset(
        mMFI->printf_size());
  } else {
    for (StringMap<SamplerInfo>::iterator
           smb = mMFI->sampler_begin(),
           sme = mMFI->sampler_end(); smb != sme; ++ smb) {
      O << ";sampler:" << (*smb).second.name << ":" << (*smb).second.idx
        << ":" << ((*smb).second.val == (uint32_t)-1 ? 0 : 1)
        << ":" << ((*smb).second.val != (uint32_t)-1 ? (*smb).second.val : 0)
        << '\n';
    }
  }
  if (mSTM->is64bit()) {
    O << ";memory:64bitABI\n";
  }

  if (!mMFI->errors_empty()) {
    binaryForEach(mMFI->errors_begin(), mMFI->errors_end(), errorPrint, O);
  }
  // This has to come last
  if (IsWrapper && !mSTM->overridesFlatAS()) {
    uint32_t ID = (mMFI->uav_size() ? *(mMFI->uav_begin()) : 0);
    if (mSTM->getGeneration() <= AMDIL::NORTHERN_ISLANDS) {
      if (mSTM->getResourceID(AMDIL::RAW_UAV_ID) >
          mSTM->getResourceID(AMDIL::ARENA_UAV_ID)) {
        if (mMFI->uav_size() == 1) {
          if (mSTM->isSupported(AMDIL::Caps::ArenaSegment)
              && *(mMFI->uav_begin()) >= ARENA_SEGMENT_RESERVED_UAVS) {
            ID = mSTM->getResourceID(AMDIL::ARENA_UAV_ID);
          }
        } else if (mMFI->uav_count(mSTM->getResourceID(AMDIL::RAW_UAV_ID))) {
          ID = mSTM->getResourceID(AMDIL::RAW_UAV_ID);
        } else {
          ID = mSTM->getResourceID(AMDIL::ARENA_UAV_ID);
        }
      } else if ((mMFI->get_num_write_images()) != OPENCL_MAX_WRITE_IMAGES &&
                 !mSTM->isSupported(AMDIL::Caps::ArenaSegment) &&
                 mMFI->uav_count(mSTM->getResourceID(AMDIL::RAW_UAV_ID))) {
        ID = mSTM->getResourceID(AMDIL::RAW_UAV_ID);;
      } else if (mMFI->uav_size() > 1){
        ID = mSTM->getResourceID(AMDIL::ARENA_UAV_ID);
      }
      O << ";uavid:" << ID << "\n";
    } else if (mSTM->getGeneration() > AMDIL::NORTHERN_ISLANDS) {
      ID = mSTM->getResourceID(AMDIL::GLOBAL_ID);
      // FIXME - Function Call Support: always declare the default global uav
      // until pointer manager is fixed to trace non-kernel functions
      // if (mMFI->uav_count(ID)) {
        O << ";uavid:" << ID << "\n";
      // }
      ID = mSTM->getResourceID(AMDIL::PRINTF_ID);
      // FIXME - Function Call Support: always declare the default global uav
      // until pointer manager is fixed to trace non-kernel functions
      //if (mMFI->uav_count(ID)) {
        O << ";printfid:" << ID << "\n";
      //}
      if (!mSTM->usesHardware(AMDIL::Caps::ConstantMem)) {
        ID = mSTM->getResourceID(AMDIL::CONSTANT_ID);
        // Always issue ;cbid for all kernels, regardless of whether the kernel
        // actually accesses any constant buffers. This will not trigger runtime
        // to actually allocate any memory, unless the compiler sends constant
        // data to the runtime. This is to avoid the messiness of having to
        // analyze all the functions that a kernel calls directly or indirectly
        // to see if any accesses a software constant buffer.
        O << ";cbid:" << ID << "\n";
      }
    }
  }
  if (IsWrapper) {
    O << ";privateid:" << mSTM->getResourceID(AMDIL::SCRATCH_ID)
      << "\n";
  }
  if (IsWrapper) {
    SmallString<32> ArgKernel("llvm.argtypename.annotations.");
    ArgKernel.append(Name);

    GlobalVariable *GV =
      mMF->getFunction()->getParent()->getGlobalVariable(ArgKernel);
    if (GV && GV->hasInitializer()) {
      const ConstantArray *NameArray
        = dyn_cast_or_null<ConstantArray>(GV->getInitializer());
      if (NameArray) {
        for (unsigned I = 0, N = NameArray->getNumOperands(); I < N; ++I) {
          const GlobalVariable *GV= dyn_cast_or_null<GlobalVariable>(
                NameArray->getOperand(I)->getOperand(0));
            const ConstantDataArray *ArgName =
              dyn_cast_or_null<ConstantDataArray>(GV->getInitializer());
            if (!ArgName) {
              continue;
            }
            StringRef ArgStr = ArgName->getAsString().drop_back(1);
            O << ";reflection:" << I << ":" << ArgStr << '\n';
        }
      }
    }
  }
  O << ";ARGEND:" << Name << "\n";
}

void AMDILKernelManager::printArgCopies(raw_ostream &O,
                                        AMDILAsmPrinter *RegNames) {
  Function::const_arg_iterator I = mMF->getFunction()->arg_begin();
  Function::const_arg_iterator Ie = mMF->getFunction()->arg_end();
  uint32_t Counter = 0;
  uint32_t ArgSize = mMFI->getArgSize();
  bool CopyArg = !mSTM->isSupported(AMDIL::Caps::UseMacroForCall);

  if (CopyArg) {
    O << "dcl_literal l3, 0x00000018, 0x00000010, 0x00000008, 0xFFFFFFFF\n";
    O << "dcl_literal l5, 0x00000000, 0x00000004, 0x00000008, 0x0000000C\n";
  }

  if (mMFI->getArgSize()) {
    O << "dcl_cb cb1";
    O << "[" << (ArgSize >> 4) << "]\n";
    mMFI->setUsesMem(AMDIL::CONSTANT_ID);
  }

  const Function *F = mMF->getFunction();
  // Get the stack size
  uint32_t StackSize = mMFI->getStackSize();
  uint32_t PrivateSize = mMFI->getScratchSize();
  uint32_t StackOffset = PrivateSize;
  assert(StackSize % 16 == 0 && "Stack should be 16-byte aligned");
  assert(StackOffset % 16 == 0 && "Stack should be 16-byte aligned");

  uint32_t literalId = uint32_t(-1);

  if (mSTM->usesHardware(AMDIL::Caps::PrivateMem)
      && !mSTM->overridesFlatAS()) {
    // TODO: If the size is too large, we need to fall back to software emulated
    // instead of using the hardware capability.

    uint64_t Size;
    if (CopyArg) {
      Size = (StackSize >> 4) +
      (mSTM->isSupported(AMDIL::Caps::Debug) ? 1 : 0);
    } else {
      Size = getReservedStackSize() >> 4;
    }

    if (Size > 4096) {
      mMFI->addErrorMsg(amd::CompilerErrorMessage[INSUFFICIENT_PRIVATE_RESOURCES]);
    }

    if (Size > 0) {
      uint32_t ResID = mSTM->getResourceID(AMDIL::SCRATCH_ID);

      // For any stack variables, we need to declare the literals for them so that
      // we can use them when we copy our data to the stack.
      // Anytime we declare a literal, we need to reserve it, if it is not emitted
      // in emitLiterals.
      if (mSTM->usesHardware(AMDIL::Caps::PrivateUAV)) {
        O << "dcl_typeless_uav_id(" << ResID
          << ")_stride(4)_length(" << (Size << 4) << ")_access(private)\n";
        if (CopyArg && mSTM->isSupported(AMDIL::Caps::Debug)) {
          int NewSize = (Size - 1) << 4;
          uint32_t id = mAMI->getUniqueLiteralId();
          O << "dcl_literal l" << id << ", "
            << NewSize << ","
            << NewSize << ","
            << NewSize << ","
            << NewSize << "\n";
          O << "uav_raw_store_id(" << ResID << ") mem0, l"
            << id << ", r1023\n";
        }
      } else {
        O << "dcl_indexed_temp_array x"
          << ResID << "["
          << Size << "]\n";
        if (CopyArg && mSTM->isSupported(AMDIL::Caps::Debug)) {
          O << "mov x" << ResID << "[" << Size - 1 << "], r1023\n";
        }
      }
      if (CopyArg) {
      literalId = mAMI->getUniqueLiteralId();
        O << "dcl_literal l" << literalId << ", " << StackSize << ", "
          << PrivateSize << ", 16, "
          << ((StackSize == PrivateSize) ? 0 : StackOffset) << "\n";

        O << "iadd r0.x, " << RegNames->getRegisterName(AMDIL::T1) << ".x, l"
        << literalId << ".w\n"
        << "mov " << RegNames->getRegisterName(AMDIL::FP)
        << ", l" << literalId << ".0\n";
    }
  }
  }
  if (CopyArg) {
  I = mMF->getFunction()->arg_begin();
  unsigned CurReg = 0;
  for (I = mMF->getFunction()->arg_begin(); I != Ie; ++I) {
    Type *CurType = I->getType();
    unsigned int Buffer = 1;
    O << "; Kernel arg setup: " << I->getName().str() << "\n";
    if (CurType->isIntegerTy() || CurType->isFloatingPointTy()) {
      switch (CurType->getPrimitiveSizeInBits()) {
      default:
        printConstantToRegMapping(RegNames, CurReg, O, Counter, Buffer, 1);
        break;
      case 16:
        printConstantToRegMapping(RegNames, CurReg, O, Counter, Buffer, 1,
                                  "l3.y" );
        break;
      case 8:
        printConstantToRegMapping(RegNames, CurReg, O, Counter, Buffer, 1, "l3.x" );
        break;
      }
    } else if (const VectorType *VT = dyn_cast<VectorType>(CurType)) {
      Type *ET = VT->getElementType();
      int NumEle = VT->getNumElements();
      switch (ET->getPrimitiveSizeInBits()) {
      default:
        printConstantToRegMapping(RegNames, CurReg, O, Counter, Buffer,
                                  (NumEle + 2) >> 2);
        break;
      case 64:
        if (NumEle == 3) {
          O << "mov ";
          printRegName(RegNames, getActualArgReg(mMFI->getArgReg(CurReg++)), O, true);
          O << ", cb" << Buffer << "[" << Counter << "].xy\n";
          O << "mov ";
          printRegName(RegNames, getActualArgReg(mMFI->getArgReg(CurReg++)), O, true);
          O << ", cb" << Buffer << "[" << Counter << "].zw\n";
          ++Counter;
          O << "mov ";
          printRegName(RegNames, getActualArgReg(mMFI->getArgReg(CurReg++)), O, true);
          O << ", cb" << Buffer << "[" << Counter << "].xy\n";
          Counter++;
        } else {
          printConstantToRegMapping(RegNames, CurReg, O, Counter, Buffer,
                                    NumEle >> 1);
        }
        break;
      case 16:
        if (NumEle == 2) {
          printConstantToRegMapping(RegNames, CurReg, O, Counter,
                                    Buffer, 1, "l3.y", 1092);
        } else {
          printConstantToRegMapping(RegNames, CurReg, O, Counter,
                                    Buffer, (NumEle + 2) >> 2, "l3.y", 1093);
        }
        break;
      case 8:
        if (NumEle == 2) {
          printConstantToRegMapping(RegNames, CurReg, O, Counter,
                                    Buffer, 1, "l3.x", 1090);
        } else {
          printConstantToRegMapping(RegNames, CurReg, O, Counter,
                                    Buffer, (NumEle + 2) >> 2, "l3.x", 1091);
        }
        break;
      }
    } else if (const PointerType *PT = dyn_cast<PointerType>(CurType)) {
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
            printConstantToRegMapping(RegNames, CurReg, O, Counter, Buffer,
                1, NULL, 0, is_image);
          } else {
            mMFI->addErrorMsg(
                amd::CompilerErrorMessage[NO_IMAGE_SUPPORT]);
            ++CurReg;
          }
        } else {
          printConstantToRegMapping(RegNames, CurReg, O, Counter, Buffer, 1);
        }
      } else if (CT->isStructTy()
                 && PT->getAddressSpace() == AMDILAS::PRIVATE_ADDRESS) {
        StructType *ST = dyn_cast<StructType>(CT);
        bool i1d  = ST->getName().startswith("struct._image1d_t");
        bool i1da = ST->getName().startswith("struct._image1d_array_t");
        bool i1db = ST->getName().startswith("struct._image1d_buffer_t");
        bool i2d  = ST->getName().startswith("struct._image2d_t");
        bool i2da = ST->getName().startswith("struct._image2d_array_t");
        bool i3d  = ST->getName().startswith("struct._image3d_t");
        bool is_image = i1d || i1da || i1db || i2d || i2da || i3d;

        if (is_image) {
          if (mSTM->isSupported(AMDIL::Caps::Images)) {
            printConstantToRegMapping(RegNames, CurReg, O, Counter, Buffer,
                1, NULL, 0, is_image);
          } else {
            mMFI->addErrorMsg(amd::CompilerErrorMessage[NO_IMAGE_SUPPORT]);
            ++CurReg;
          }
        } else {
          const DataLayout *DL = mTM->getDataLayout();
          size_t StructSize
            = DL->RoundUpAlignment(DL->getTypeAllocSize(ST), 16);

          //StackOffset += StructSize;
          O << "mov ";
          printRegName(RegNames, getActualArgReg(mMFI->getArgReg(CurReg)), O, true);
          O << ", r0.x\n";
          assert(literalId != uint32_t(-1) && "Use undefined literal");
          printCopyStructPrivate(ST, O, StructSize,
                                 Buffer, literalId, Counter);
          ++CurReg;
        }
      } else if (CT->isIntOrIntVectorTy()
                 || CT->isFPOrFPVectorTy()
                 || CT->isArrayTy()
                 || CT->isPointerTy()
                 || PT->getAddressSpace() != AMDILAS::PRIVATE_ADDRESS) {
        if (PT->getAddressSpace() == AMDILAS::CONSTANT_ADDRESS) {
          const AMDILKernel *Kernel = mAMI->getKernel(mKernelName);
          printConstantToRegMapping(RegNames, CurReg, O, Counter, Buffer,
              1, NULL, 0, false,
              mAMI->usesHWConstant(Kernel, I->getName()));
        } else if (PT->getAddressSpace() == AMDILAS::REGION_ADDRESS) {
          // TODO: If we are Region address space, the first Region pointer, no
          // array pointers exist, and hardware RegionMem is enabled then we can
          // zero out register as the initial offset is zero.
          printConstantToRegMapping(RegNames, CurReg, O, Counter, Buffer, 1);
        } else if (PT->getAddressSpace() == AMDILAS::LOCAL_ADDRESS) {
          // TODO: If we are local address space, the first local pointer, no
          // array pointers exist, and hardware LocalMem is enabled then we can
          // zero out register as the initial offset is zero.
          printConstantToRegMapping(RegNames, CurReg, O, Counter, Buffer, 1);
        } else {
          printConstantToRegMapping(RegNames, CurReg, O, Counter, Buffer, 1);
        }
      } else {
        llvm_unreachable("Current type is not supported!");
        mMFI->addErrorMsg(amd::CompilerErrorMessage[INTERNAL_ERROR]);
        ++CurReg;
      }
    } else {
      llvm_unreachable("Current type is not supported!");
      mMFI->addErrorMsg(amd::CompilerErrorMessage[INTERNAL_ERROR]);
      ++CurReg;
    }
  }
  }
  if (mSTM->usesHardware(AMDIL::Caps::ConstantMem)) {
    const AMDILKernel *Kernel = mAMI->getKernel(mKernelName);
    uint32_t ConstNum = 0;
    for (unsigned I = 0, MaxCBs = mSTM->getMaxNumCBs();
         I < MaxCBs; ++I) {
      if (Kernel->constSizes[I]) {
        O << "dcl_cb cb" << I + CB_BASE_OFFSET;
        O << "[" << (((Kernel->constSizes[I] + 15) & ~15) >> 4) << "]\n";
        ++ConstNum;
        mMFI->setUsesMem(AMDIL::CONSTANT_ID);
      }
    }
    // TODO: If we run out of constant resources, we need to push some of the
    // constant pointers to the software emulated section.
    if (ConstNum > mSTM->getMaxNumCBs()) {
      llvm_unreachable("Max constant buffer limit passed!");
      mMFI->addErrorMsg(amd::CompilerErrorMessage[INSUFFICIENT_CONSTANT_RESOURCES]);
    }
  }
}

void AMDILKernelManager::emitLiterals(raw_ostream &O) {
  char buffer[256];
  std::map<std::pair<uint64_t, uint64_t>, uint32_t>::iterator vlb, vle;
  for (vlb = mMFI->lit_begin(), vle = mMFI->lit_end(); vlb != vle; ++vlb) {
    uint32_t v[2][2];
    uint64_t a = vlb->first.first;
    uint64_t b = vlb->first.second;
    memcpy(v[0], &a, sizeof(uint64_t));
    memcpy(v[1], &b, sizeof(uint64_t));
    O << "dcl_literal l" << vlb->second << ", ";
    sprintf(buffer, "0x%08X, 0x%08X, 0x%08X, 0x%08X; f128:i128 ",
            v[0][0], v[0][1], v[1][0], v[1][1]);
    O << buffer << vlb->first.first << vlb->first.second << "\n";
  }
}

// Return the size of space to reserve for the call stack
// If call stack size not set, estimate it as double the kernel stack size.
// TODO: Find maximum stack size by call graph.
uint64_t AMDILKernelManager::getReservedStackSize() {
  assert(mIsKernel);
  uint64_t CallStackSize = AMDILCallStackSize;
  uint64_t KernelStackSize = mTM->getDataLayout()->RoundUpAlignment(
      mMFI->getStackSize(), 16) +
      (mSTM->isSupported(AMDIL::Caps::Debug) ? 16 : 0);
  if (!mAMI->kernelHasCalls(mMF->getFunction()) &&
      CallStackSize == 0) {
    return KernelStackSize;
  }
  if (mAMI->kernelHasCalls(mMF->getFunction())) {
    KernelStackSize += mTM->getDataLayout()->RoundUpAlignment(
      mMFI->getScratchSize(), 16);
  }
  if (CallStackSize == 0) {
    CallStackSize = KernelStackSize * 2;
  }
  if (KernelStackSize > CallStackSize) {
    CallStackSize = KernelStackSize;
  }
  return CallStackSize;
}

