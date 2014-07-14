//===- AMDILUtilityFunctions.h - AMDIL Utility Functions Header -*- C++ -*-===//
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
// This file provides declarations for functions that are used across different
// classes and provide various conversions or utility to shorten the code
//
//===----------------------------------------------------------------------===//
#ifndef AMDILUTILITYFUNCTIONS_H_
#define AMDILUTILITYFUNCTIONS_H_

#include "AMDIL.h"
#include "AMDILTargetMachine.h"

// Utility functions from ID
//
namespace llvm {
class TargetRegisterClass;
class TargetMachine;
class Value;
class Type;
class TypeSymbolTable;
class StructType;
class IntegerType;
class FunctionType;
class VectorType;
class ArrayType;
class PointerType;
class OpaqueType;
class MachineInstr;
class AMDILMachineFunctionInfo;

}
enum SrcSwizzles {
  AMDIL_SRC_DFLT = 0,
  AMDIL_SRC_X000,
  AMDIL_SRC_0X00,
  AMDIL_SRC_00X0,
  AMDIL_SRC_000X,
  AMDIL_SRC_Y000,
  AMDIL_SRC_0Y00,
  AMDIL_SRC_00Y0,
  AMDIL_SRC_000Y,
  AMDIL_SRC_Z000,
  AMDIL_SRC_0Z00,
  AMDIL_SRC_00Z0,
  AMDIL_SRC_000Z,
  AMDIL_SRC_W000,
  AMDIL_SRC_0W00,
  AMDIL_SRC_00W0,
  AMDIL_SRC_000W,
  AMDIL_SRC_XY00,
  AMDIL_SRC_00XY,
  AMDIL_SRC_ZW00,
  AMDIL_SRC_00ZW,
  AMDIL_SRC_XYZ0,
  AMDIL_SRC_0XYZ,
  AMDIL_SRC_XZXZ,
  AMDIL_SRC_YWYW,
  AMDIL_SRC_X0Y0,
  AMDIL_SRC_0X0Y,
  AMDIL_SRC_0YZW,
  AMDIL_SRC_X0ZW,
  AMDIL_SRC_XY0W,
  AMDIL_SRC_XXXX,
  AMDIL_SRC_YYYY,
  AMDIL_SRC_ZZZZ,
  AMDIL_SRC_WWWW,
  AMDIL_SRC_XYXY,
  AMDIL_SRC_ZWZW,
  AMDIL_SRC_YZW0,
  AMDIL_SRC_Z0W0,
  AMDIL_SRC_0Z0W,
  AMDIL_SRC_LAST
};

enum DstSwizzles {
  AMDIL_DST_DFLT = 0,
  AMDIL_DST_X___,
  AMDIL_DST__Y__,
  AMDIL_DST___Z_,
  AMDIL_DST____W,
  AMDIL_DST_XY__,
  AMDIL_DST___ZW,
  AMDIL_DST_XYZ_,
  AMDIL_DST_LAST
};

// Function to get the correct src swizzle string from ID
const char *getSrcSwizzle(unsigned);

// Function to get the correct dst swizzle string from ID
const char *getDstSwizzle(unsigned);

// get the vector register which is the super register of the given register
unsigned getVectorReg(unsigned Reg, const TargetRegisterInfo *TRI);

// Function to check address space
bool check_type(const llvm::Value *Ptr, unsigned int AddrSpace);

// Group of functions that recursively calculate the number of
// elements of a structure based on it's sub-types.
size_t getNumElements(llvm::Type *const T);
size_t getNumElements(llvm::StructType *const ST);
size_t getNumElements(llvm::IntegerType *const IT);
size_t getNumElements(llvm::FunctionType *const FT);
size_t getNumElements(llvm::ArrayType *const AT);
size_t getNumElements(llvm::VectorType *const VT);
size_t getNumElements(llvm::PointerType *const PT);
size_t getNumElements(llvm::OpaqueType *const OT);

const llvm::Value *getMemOpUnderlyingObject(const llvm::MachineInstr *MI,
                                            const DataLayout *DL = 0);

const char *getTypeName(llvm::Type *Ptr,
    const char *SymTab,
    llvm::AMDILMachineFunctionInfo *MFI,
    bool SignedType);
bool isImageType(const Type *Ty);

// Helper functions that check the opcode for status information
bool isLoadInst(const llvm::MachineInstr *MI);
bool isPtrLoadInst(const llvm::MachineInstr *MI);
bool isExtLoadInst(const llvm::MachineInstr *MI);
bool isSWSExtLoadInst(const llvm::MachineInstr *MI);
bool isSExtLoadInst(const llvm::MachineInstr *MI);
bool isZExtLoadInst(const llvm::MachineInstr *MI);
bool isAExtLoadInst(const llvm::MachineInstr *MI);
bool isStoreInst(const llvm::MachineInstr *MI);
bool isPtrStoreInst(const llvm::MachineInstr *MI);
bool isTruncStoreInst(const llvm::MachineInstr *MI);
bool isAtomicInst(const llvm::MachineInstr *MI);
bool isVolatileInst(const llvm::MachineInstr *MI);
bool isGlobalInst(const llvm::MachineInstr *MI);
bool isPrivateInst(const llvm::MachineInstr *MI);
bool isConstantInst(const llvm::MachineInstr *MI);
bool isConstantPoolInst(const llvm::MachineInstr *MI);
bool isRegionInst(const llvm::MachineInstr *MI);
bool isGWSInst(const llvm::MachineInstr *MI);
bool isLocalInst(const llvm::MachineInstr *MI);
bool isImageInst(const llvm::MachineInstr *MI);
bool is64BitImageInst(const llvm::MachineInstr *MI);
bool isWriteImageInst(const llvm::MachineInstr *MI);
bool isReadImageInst(const llvm::MachineInstr *MI);
bool isImageInfoInst(const llvm::MachineInstr *MI);
bool isImageInfo0Inst(const llvm::MachineInstr *MI);
bool isImageInfo1Inst(const llvm::MachineInstr *MI);
bool isImageTXLDInst(const llvm::MachineInstr *MI);
bool isAppendInst(const llvm::MachineInstr *MI);
bool isSemaphoreInst(const llvm::MachineInstr *MI);
bool isRegionAtomic(const llvm::MachineInstr *MI);
bool is64BitRegionAtomic(const llvm::MachineInstr *MI);
bool isLocalAtomic(const llvm::MachineInstr *MI);
bool is64BitLocalAtomic(const llvm::MachineInstr *MI);
bool isGlobalAtomic(const llvm::MachineInstr *MI);
bool is64BitGlobalAtomic(const llvm::MachineInstr *MI);
bool isArenaAtomic(const llvm::MachineInstr *MI);
bool isArenaInst(const llvm::MachineInstr *MI);
bool is64bitLSOp(const llvm::MachineInstr *MI);
bool isLDSInst(const llvm::MachineInstr *MI);
bool isGDSInst(const llvm::MachineInstr *MI);
bool isUAVArenaInst(const llvm::MachineInstr *MI);
bool isUAVRawInst(const llvm::MachineInstr *MI);
bool isCBInst(const llvm::MachineInstr *MI);
bool isScratchInst(const llvm::MachineInstr *MI);
bool is64BitInst(const llvm::MachineInstr *MI);
bool isPackedInst(const llvm::MachineInstr *MI);
bool isSub32BitIOInst(const llvm::MachineInstr *MI);
bool isPackV2I8Inst(const llvm::MachineInstr *MI);
bool isPackV2I16Inst(const llvm::MachineInstr *MI);
bool isPackV4I8Inst(const llvm::MachineInstr *MI);
bool isPackV4I16Inst(const llvm::MachineInstr *MI);
bool isUnpackV2I8Inst(const llvm::MachineInstr *MI);
bool isUnpackV2I16Inst(const llvm::MachineInstr *MI);
bool isUnpackV4I8Inst(const llvm::MachineInstr *MI);
bool isUnpackV4I16Inst(const llvm::MachineInstr *MI);
bool isVectorOpInst(const llvm::MachineInstr *MI);

inline bool isAddriInst(const llvm::MachineInstr *MI) {
  return MI->getOpcode() == llvm::AMDIL::ADDrp
      || MI->getOpcode() == llvm::AMDIL::ADDi64rp;
}
inline bool isLoadConstInst(const llvm::MachineInstr *MI) {
  return (MI->getDesc().TSFlags & (1ULL << AMDID::LOADCONST));
}

bool isSkippedLiteral(const llvm::MachineInstr *MI, uint32_t Op);
bool isBypassedLiteral(const llvm::MachineInstr *MI, uint32_t Op);
// Helper functions that check a registe for status information.
bool isXComponentReg(unsigned);
bool isYComponentReg(unsigned);
bool isZComponentReg(unsigned);
bool isWComponentReg(unsigned);
bool isXYComponentReg(unsigned);
bool isZWComponentReg(unsigned);


bool commaPrint(int I, raw_ostream &O);
/// Helper function to get the currently get/set flags.
void getAsmPrinterFlags(llvm::MachineInstr *MI,
    llvm::AMDILAS::InstrResEnc &CurRes);
void setAsmPrinterFlags(llvm::MachineInstr *MI,
    llvm::AMDILAS::InstrResEnc &CurRes);

#endif // AMDILUTILITYFUNCTIONS_H_
