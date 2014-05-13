//===--------------- AMDILMachineModuleInfo.h -------------------*- C++ -*-===//
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
//
// This is an MMI implementation for AMDIL targets.
//
//===----------------------------------------------------------------------===//

#ifndef _AMDIL_MACHINE_MODULE_INFO_H_
#define _AMDIL_MACHINE_MODULE_INFO_H_
#include "AMDIL.h"
#include "AMDILKernel.h"
#include "llvm/Module.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <set>
#define CB_BASE_OFFSET 2

namespace llvm {
  class AMDILKernel;
  class Argument;
  class TypeSymbolTable;
  class GlobalValue;
  class MachineFunction;
  class GlobalValue;
  class AMDILAsmPrinter;

  class AMDILMachineFunctionInfo;
  class AMDILModuleInfo : public MachineModuleInfoImpl {
    protected:
      const MachineModuleInfo *mMMI;
    public:
      class Entity {
      public:
        virtual ~Entity(){}
      };
      AMDILModuleInfo(const MachineModuleInfo &);
      virtual ~AMDILModuleInfo();

      void processModule(const Module *MF, const AMDILTargetMachine* mTM);

      /// Process the given module and parse out the global variable metadata passed
      /// down from the frontend-compiler

      /// Returns true if the image ID corresponds to a read only image.
      bool isReadOnlyImage(StringRef Name, uint32_t IID) const;

      /// Returns true if the image ID corresponds to a write only image.
      bool isWriteOnlyImage(StringRef Name, uint32_t IID) const;

      /// Gets the group size of the kernel for the given dimension.
      uint32_t getRegion(StringRef Name, uint32_t Dim) const;

      /// Get the offset of the array for the global value name.
      int32_t getArrayOffset(StringRef Name) const;

      /// Get the offset of the const memory for the global value name.
      int32_t getConstOffset(StringRef Name) const;

      /// Get the boolean value if this particular constant uses HW or not.
      bool getConstHWBit(StringRef Name) const;

      /// Get the base pointer of the const memory for the global value name.
      const Value *getConstBase(StringRef Name) const;

      /// Get a reference to the kernel metadata information for the given function
      /// name.
      AMDILKernel *getKernel(StringRef Name);
      bool isKernel(StringRef Name) const;

      /// Dump the data section to the output stream for the given kernel.
      void dumpDataSection(raw_ostream &O, AMDILMachineFunctionInfo *MFI);

      /// Iterate through the constants that are global to the compilation unit.
      StringMap<AMDILConstPtr>::iterator consts_begin();
      StringMap<AMDILConstPtr>::iterator consts_end();
      bool consts_empty();

      /// Query if the kernel has a byte store.
      bool byteStoreExists(llvm::StringRef S) const;

      /// Query if the constant pointer is an argument.
      bool isConstPtrArgument(const AMDILKernel *Kernel, StringRef Arg);

      /// Query if the constant pointer is an array that is globally scoped.
      bool isConstPtrArray(const AMDILKernel *Kernel, StringRef Arg);

      /// Query if the constant argument uses hardware or not
      bool usesHWConstant(const AMDILKernel *Kernel, StringRef Arg);

      /// Query the size of the constant pointer.
      uint32_t getConstPtrSize(const AMDILKernel *Kernel, StringRef Arg);

      /// Query the offset of the constant pointer.
      uint32_t getConstPtrOff(const AMDILKernel *Kernel, StringRef Arg);

      /// Query the constant buffer number for a constant pointer.
      uint32_t getConstPtrCB(const AMDILKernel *Kernel, StringRef Arg);

      /// Query the Value* that the constant pointer originates from.
      const Value *getConstPtrValue(const AMDILKernel *Kernel,
                                    StringRef Arg);

      /// Get the unique function ID for the specific function name
      /// and create a new unique ID if it is not found.
      uint32_t getOrCreateFunctionID(const GlobalValue *Func);
      uint32_t getOrCreateFunctionID(StringRef Func);

      /// Calculate the offsets of the constant pool for the given
      /// kernel and machine function.
      void calculateCPOffsets(const MachineFunction *MF,
                              AMDILKernel *Kernel);

      void add_printf_offset(uint32_t offset) { mPrintfOffset += offset; }
      uint32_t get_printf_offset() { return mPrintfOffset; }
      uint32_t populateNextLocalBuffer(
        const SmallSet<const Value*, 1>& Locals, bool IsDefaultBuf);
      std::set<std::string> *getSamplerForKernel(StringRef KernelName);
      uint32_t numLocalBuffers() const { return mNumLocalBuffers; }
      uint32_t numRegionBuffers() const { return mNumRegionBuffers; }

      // Add the AMDILArrayMem size to the kernel's total usage and set the offset
      // of the item
      void addArrayMemSize(AMDILKernel *Kernel, AMDILArrayMem *A);

      // Reserve LDS or GDS for the given kernel. Returns the offset.
      uint32_t reserveShared(AMDILKernel* Kernel,
                             uint32_t Size,
                             uint32_t Alignment,
                             StringRef Name,
                             bool Region);

      // Find the resource ID for the reserved LDS/GDS
      uint32_t findReservedSharedResourceID(StringRef MemName) const;

      // Get the next unique literal id
      uint32_t getUniqueLiteralId()  { return mNextLiteralId++; }

      // FIXME: For debug use (should merge with getUAVID())
      bool getGlobalValueRID(const AMDILAsmPrinter *, const Value *Value,
                             uint32_t& RID) const;

      // Returns whether the given kernel contains calls
      bool kernelHasCalls(const Function* K) {
        if (!mKernelCallInitialized) {
          calculateKernelHasCalls();
          mKernelCallInitialized = true;
        }
        return mHasCallKernels.count(K);
      }

      // Returns whether the module has kernels that contain calls
      bool moduleHasCalls() {
        if (!mKernelCallInitialized) {
          calculateKernelHasCalls();
          mKernelCallInitialized = true;
        }
        return !mHasCallKernels.empty();
      }

      Entity *getPtrEqSet() { return mPtrEqSet;}
      void setPtrEqSet(Entity *ptrEqSet) { mPtrEqSet = ptrEqSet;}
    private:
      /// Various functions that parse global value information and store them in
      /// the global manager. This approach is used instead of dynamic parsing as it
      /// might require more space, but should allow caching of data that gets
      /// requested multiple times.
      AMDILKernelAttr parseSGV(const GlobalValue *GV);

      // Read the LVGV or RVGV annotation
      AMDILLocalArg parseXVGV(const GlobalValue *GV);
      void parseGlobalAnnotate(const GlobalValue *G);
      void parseImageAnnotate(const GlobalValue *G);
      void parseSamplerAnnotate(const GlobalValue *GV);
      void parseConstantPtrAnnotate(const GlobalValue *G);
      void parseIgnoredGlobal(const GlobalValue *G);
      size_t printConstantValue(const Constant *CAVal,
                                raw_ostream& O,
                                bool AsByte);

      void parseGroupSize(uint32_t GroupSize[3], StringRef Str);

      // parse the local and region operands for parseKernelInformation
      AMDILLocalArg* parseKernelLRInfo(AMDILKernel *Kernel,
                                       const Constant *CV);
      void parseKernelInformation(const Value *V);
      void parseAutoArray(const GlobalValue *G, bool IsRegion);
      void parseConstantPtr(const GlobalValue *G);
      void allocateGlobalCB();
      void dumpDataToCB(raw_ostream &O,
                        AMDILMachineFunctionInfo *MFI,
                        uint32_t ID);
      bool checkConstPtrsUseHW(Module::const_iterator *F);
      void calculateKernelHasCalls();

      llvm::StringMap<AMDILKernel *> mKernels;
      llvm::StringMap<AMDILKernelAttr> mKernelArgs;
      llvm::StringMap<AMDILArrayMem> mArrayMems;
      llvm::StringMap<AMDILConstPtr> mConstMems;
      llvm::StringMap<AMDILLocalArg> mLocalArgs;
      llvm::StringMap<AMDILArrayMem> mReservedArrayMems;
      llvm::StringMap<uint32_t> mFuncNames;
      llvm::DenseMap<const GlobalValue*, uint32_t> mFuncPtrNames;
      llvm::DenseMap<uint32_t, llvm::StringRef> mImageNameMap;
      llvm::StringMap<std::set<std::string> > mSamplerSet;
      std::set<llvm::StringRef> mByteStore;
      std::set<llvm::StringRef> mIgnoreStr;
      // Kernels that have calls
      SmallSet<const Function*, 1> mHasCallKernels;
      bool mKernelCallInitialized;
#if LLVM_VERSION > 3316
      const char *SymTab;
#else
      const TypeSymbolTable *SymTab;
#endif
      const AMDILSubtarget *mSTM;
      const TargetMachine *TM;
      uint32_t mReservedBuffs;
      uint32_t mCurrentCPOffset;
      uint32_t mPrintfOffset;
      uint32_t mNumLocalBuffers;
      uint32_t mNumRegionBuffers;
      bool mProcessed;
      uint32_t mNextLiteralId;
      Entity *mPtrEqSet;
  };
} // end namespace llvm

#endif // _AMDIL_MACHINE_MODULE_INFO_H_
