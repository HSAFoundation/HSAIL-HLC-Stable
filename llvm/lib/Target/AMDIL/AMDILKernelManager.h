//==------------------------------------------------------------*- C++ -*--===//
//
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
// @file AMDILKernelManager.h
// @details Class that handles the metadata/abi management for the
// ASM printer. Handles the parsing and generation of the metadata
// for each kernel and keeps track of its arguments.
//
#ifndef _AMDILKERNELMANAGER_H_
#define _AMDILKERNELMANAGER_H_

#include "AMDIL.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/Function.h"

#include <map>
#include <set>
#include <string>

#define IMAGETYPE_2D 0
#define IMAGETYPE_3D 1
#define RESERVED_LIT_COUNT 6

namespace llvm {
class AMDILSubtarget;
class AMDILMachineFunctionInfo;
class AMDILModuleInfo;
class AMDILTargetMachine;
class AMDILAsmPrinter;
class StructType;
class Value;
class TypeSymbolTable;
class MachineFunction;
class MachineInstr;
class ConstantFP;
class PrintfInfo;


class AMDILKernelManager {
public:
  typedef enum {
    RELEASE_ONLY,
    DEBUG_ONLY,
    ALWAYS
  } ErrorMsgEnum;

  // There are a few values that we store in global registers so that
  // they can be used from any functions.
  // Since we use outlined macros to implement function calls, and all temp
  // registers are lexically scoped to the macro, we cannot use temp
  // registers to store these values. Instead, we use indexed temp arrays.
  // As long as we always use literals to access the item array slots,
  // they will be promoted to registers by SC.
  // These enums define the slot id in the itemp array where each global
  // value is stored.
  enum {
    ITEMP_SLOT_SP,
    ITEMP_SLOT_GLOBAL_ID,
    ITEMP_SLOT_GROUP_ID,
    ITEMP_SLOT_LOCAL_OFFSET,
    ITEMP_SLOT_PRIVATE_OFFSET,
    ITEMP_SLOT_PRINTF,
    ITEMP_SLOT_SDP,
    ITEMP_SLOT_T1,
    ITEMP_SLOT_T2,
    ITEMP_SLOT_FP,
    NUM_ITEMP_SLOTS
  };
  AMDILKernelManager(MachineFunction *MF);
  virtual ~AMDILKernelManager();

  /// Process the specific kernel parsing out the parameter
  /// information for the kernel.
  void processArgMetadata(raw_ostream &O, uint32_t Buf);

  /// Prints the header for the kernel wrapper which includes the groupsize
  /// declaration and calculation of the local/group/global id's.
  void printWrapperHeader(AMDILAsmPrinter *AsmPrinter,
                          raw_ostream &O);

  virtual void printDecls(AMDILAsmPrinter *AsmPrinter, raw_ostream &O);
  virtual void printGroupSize(raw_ostream &O);
  virtual void printWavefrontSize(raw_ostream &O);

  /// Copies the data from the runtime setup constant buffers into
  /// registers so that the program can correctly access memory or
  /// data that was set by the host program.
  void printArgCopies(raw_ostream &O, AMDILAsmPrinter* RegNames);

  /// Prints out the end of the kernel wrapper function.
  void printWrapperFooter(raw_ostream &O);

  /// Prints out the metadata for the specific function depending if
  /// it is a kernel or not.
  void printMetaData(raw_ostream &O,
                     uint32_t ID,
                     bool IsWrapper = false);

  /// Set bool value on whether to consider the function a kernel or a
  /// normal function.
  void setKernel(bool Kernel);

  /// Set the unique ID of the kernel/function.
  void setID(uint32_t ID);

  uint32_t getID() const { return mUniqueID; }

  /// Set the name of the kernel/function.
  void setName(StringRef Name);

  /// Flag that specifies whether this function has a kernel wrapper.
  bool wasKernel();

  void getIntrinsicSetup(AMDILAsmPrinter *AsmPrinter, raw_ostream &O);

  // Returns whether a compiler needs to insert a write to memory or
  // not.
  bool useCompilerWrite(const MachineInstr *MI);

  // Return whether a region_barrier is used
  bool useRegionBarrier(const MachineInstr *MI);

  void emitLiterals(raw_ostream &O);

  StringRef getStubName() const { return mStubName; }

private:

  /// Helper function that prints the actual metadata and should only
  /// be called by printMetaData.
  void printKernelArgs(raw_ostream &O, bool IsWrapper);
//#if 0
  void printCopyStructPrivate(const StructType *ST,
                              raw_ostream &O,
                              size_t StackSize,
                              uint32_t Buffer,
                              uint32_t mLitIdx,
                              uint32_t &counter);

  virtual void printConstantToRegMapping(AMDILAsmPrinter *RegNames,
                                         unsigned &LII,
                                         raw_ostream &O,
                                         uint32_t &Counter,
                                         uint32_t Buffer,
                                         uint32_t N,
                                         const char *Lit = NULL,
                                         uint32_t FCall = 0,
                                         bool IsImage = false,
                                         bool IsHWCB = false);
//#endif
  void updatePtrArg(llvm::Function::const_arg_iterator Ip,
                    int NumWriteImages,
                    int Raw_uav_buffer,
                    int Counter,
                    const Function *F);
  uint64_t getReservedStackSize();
  unsigned getActualArgReg(unsigned FormalReg);

  /// Name of the current kernel.
  std::string mName;
  std::string mKernelName;
  std::string mStubName;
  uint32_t mUniqueID;
  bool mIsKernel;
  bool mWasKernel;

  const AMDILTargetMachine *mTM;
  const AMDILSubtarget *mSTM;
  /// This is the global offset of the printf string id's.
  const MachineFunction *mMF;
  AMDILMachineFunctionInfo *mMFI;
  AMDILModuleInfo *mAMI;
}; // class AMDILKernelManager

} // llvm namespace
#endif // _AMDILKERNELMANAGER_H_
