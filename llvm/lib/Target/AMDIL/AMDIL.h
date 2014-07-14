//===-- AMDIL.h - Top-level interface for AMDIL representation --*- C++ -*-===//
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
// This file contains the entry points for global functions defined in the LLVM
// AMDIL back-end.
//
//===----------------------------------------------------------------------===//

#ifndef AMDIL_H_
#define AMDIL_H_
#ifdef USE_APPLE
#include "AMDILLLVMApple.h"
#else
#include "AMDILLLVMPC.h"
#endif
#include "AMDILInstPrinter.h"

#ifdef AMD_LLVM_INTERNAL
#include "AMDILInternalDeviceFlags.h"
#endif

#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Target/TargetMachine.h"

#define AMDIL_MAJOR_VERSION 3
#define AMDIL_MINOR_VERSION 1
#define AMDIL_REVISION_NUMBER 111
#define AMDIL_20_REVISION_NUMBER 88
#define ARENA_SEGMENT_RESERVED_UAVS 12
#define DEFAULT_ARENA_UAV_ID 8
#define DEFAULT_RAW_UAV_ID 7
#define GLOBAL_RETURN_RAW_UAV_ID 11
#define HW_MAX_NUM_CB 8
#define MAX_NUM_UNIQUE_UAVS 8
#define OPENCL_MAX_NUM_ATOMIC_COUNTERS 8
#define OPENCL_MAX_SAMPLERS 16
#define OPENCL_MAX_NUM_SEMAPHORES 15

// The next two values can never be zero, as zero is the ID that is
// used to assert against.
#define DEFAULT_LDS_ID     1
#define DEFAULT_GDS_ID     1
#define DEFAULT_SCRATCH_ID 1
#define DEFAULT_VEC_SLOTS  8

#define OCL_DEVICE_RV710       0x00000001
#define OCL_DEVICE_RV730       0x00000002
#define OCL_DEVICE_RV770       0x00000004
#define OCL_DEVICE_CEDAR       0x00000008
#define OCL_DEVICE_REDWOOD     0x00000010
#define OCL_DEVICE_JUNIPER     0x00000020
#define OCL_DEVICE_CYPRESS     0x00000040
#define OCL_DEVICE_CAICOS      0x00000080
#define OCL_DEVICE_TURKS       0x00000100
#define OCL_DEVICE_BARTS       0x00000200
#define OCL_DEVICE_CAYMAN      0x00000400
#define OCL_DEVICE_TAHITI      0x00000800
#define OCL_DEVICE_PITCAIRN    0x00001000
#define OCL_DEVICE_CAPEVERDE   0x00002000
#define OCL_DEVICE_TRINITY     0x00004000
#define OCL_DEVICE_OLAND       0x00008000
#define OCL_DEVICE_BONAIRE     0x00010000
#define OCL_DEVICE_TIRAN       0x00020000 //<-- Internal Only!
#define OCL_DEVICE_SPECTRE     0x00040000
#define OCL_DEVICE_SPOOKY      0x00080000
#define OCL_DEVICE_KALINDI     0x00100000
#define OCL_DEVICE_HAINAN      0x00200000
#define OCL_DEVICE_HAWAII      0x00400000
#define OCL_DEVICE_MAUI        0x00800000 //<-- Internal Only!
#define OCL_DEVICE_ICELAND     0x01000000
#define OCL_DEVICE_TONGA       0x02000000
#define OCL_DEVICE_GODAVARI    0x04000000
#define OCL_DEVICE_BERMUDA     0x08000000 //<-- Internal Only!
#define OCL_DEVICE_FIJI        0x10000000 //<-- Internal Only!
#define OCL_DEVICE_CARRIZO     0x20000000 //<-- Internal Only!
#define OCL_DEVICE_ALL         0x1FFFFFFF

#define NUM_EXTRA_SLOTS_PER_IMAGE 1

/// The number of function ID's that are reserved for
/// internal compiler usage.
const unsigned int RESERVED_FUNCS = 1024;

namespace llvm {
class AMDILInstrPrinter;
class AMDILTargetMachine;
class FunctionPass;
class MCAsmInfo;
class raw_ostream;
class Target;
class TargetMachine;

/// Instruction selection passes.
FunctionPass *createAMDILISelDag(AMDILTargetMachine &TM,
                                 CodeGenOpt::Level OptLevel);
FunctionPass *createAMDILBarrierDetect(TargetMachine &TM,
                                       CodeGenOpt::Level OptLevel);
FunctionPass *createAMDILPrintfConvert(TargetMachine &TM,
                                       CodeGenOpt::Level OptLevel);
Pass *createAMDILCreateKernelStubPass();
FunctionPass *createAMDILInlinePass(TargetMachine &TM,
                                    CodeGenOpt::Level OptLevel);
FunctionPass *createAMDILPeepholeOpt(TargetMachine &TM,
                                     CodeGenOpt::Level OptLevel);

/// Pre regalloc passes.
FunctionPass *createAMDILPointerManager(TargetMachine &TM,
                                        CodeGenOpt::Level OptLevel);
FunctionPass *createAMDILMachinePeephole();

/// Pre emit passes.
FunctionPass *createMachinePostDominatorTreePass();
FunctionPass *createAMDILCFGPreparationPass();
FunctionPass *createAMDILCFGStructurizerPass();
FunctionPass *createAMDILLiteralManager(TargetMachine &TM,
                                        CodeGenOpt::Level OptLevel);
FunctionPass *createAMDILIOExpansion(TargetMachine &TM,
                                     CodeGenOpt::Level OptLevel);
FunctionPass *createAMDILSwizzleEncoder(const TargetMachine &TM);
FunctionPass *createAMDILRenumberRegister(TargetMachine &TM,
                                        CodeGenOpt::Level OptLevel);
FunctionPass *createAMDILRegisterUseValidate(TargetMachine &TM,
                                        CodeGenOpt::Level OptLevel);

/// Instruction Emission Passes
AMDILInstPrinter *createAMDILInstPrinter(const MCAsmInfo &MAI,
                                         const MCInstrInfo &MII,
                                         const MCRegisterInfo &MRI);

extern Target TheAMDILTarget;
extern Target TheAMDIL64Target;
} // end namespace llvm;

#define GET_REGINFO_ENUM
#include "AMDILGenRegisterInfo.inc"
#define GET_INSTRINFO_ENUM
#include "AMDILGenInstrInfo.inc"

/// Include device information enumerations
#include "AMDILDeviceInfo.h"

namespace llvm {
// AMDIL Instruction descriptor flags
namespace AMDID {
  enum {
    EMPTY     =  0,
    SEXTLOAD  =  1,
    ZEXTLOAD  =  2,
    LOAD      =  3,
    STORE     =  4,
    TRUNCATE  =  5,
    ATOMIC    =  6,
    ADDR64    =  7,
    GLOBAL    =  8,
    PRIVATE   =  9,
    CONSTANT  = 10,
    CPOOL     = 11,
    REGION    = 12,
    LOCAL     = 13,
    GDS       = 14,
    LDS       = 15,
    CBMEM     = 16,
    SCRATCH   = 17,
    RAWUAV    = 18,
    ARENAUAV  = 19,
    IMAGE     = 20,
    INFO0     = 21,
    INFO1     = 22,
    TXLD      = 23,
    SEMA      = 24,
    APPEND    = 25,
    SWSEXTLD  = 26,
    LOADCONST = 27,
    IEEE      = 28,
    ZEROOP    = 29,
    FLAT      = 30,
    SWZLSHFT  = 31,
    SWZLDST   = (SWZLSHFT + 0),
    SWZLSRC0  = (SWZLSHFT + 1),
    SWZLSRC1  = (SWZLSHFT + 2),
    SWZLSRC2  = (SWZLSHFT + 3),
    SWZLSRC3  = (SWZLSHFT + 4),
    SWZLSRC4  = (SWZLSHFT + 5),
    GWS       = 37,
    PACKED    = 38,
    SUB32BITS = 39,
    TYPEI16   = 40,
    TYPEV4    = 41,
    VECTOR    = 42
  }; // End anonymous enum.
  static const uint64_t SWZLMASK  = (1ULL << SWZLDST)  | (1ULL << SWZLSRC0)
                                  | (1ULL << SWZLSRC1) | (1ULL << SWZLSRC2)
                                  | (1ULL << SWZLSRC3) | (1ULL << SWZLSRC4);
  static const uint64_t AEXTLOAD = (1ULL << SEXTLOAD) | (1ULL << ZEXTLOAD);
  static const uint64_t INFO = (1ULL << INFO0) | (1ULL << INFO1);
  static const uint64_t EXTLOAD  = AEXTLOAD | (1ULL << SWSEXTLD);
  static const uint64_t TYPEMASK  = (1ULL << TYPEI16) | (1ULL << TYPEV4);
  static const uint64_t TYPEV2I8  = 0ULL;
  static const uint64_t TYPEV2I16 = (1ULL << TYPEI16);
  static const uint64_t TYPEV4I8  = (1ULL << TYPEV4);
  static const uint64_t TYPEV4I16 = TYPEMASK;
} // end AMDID namespace.
/// OpenCL uses address spaces to differentiate between
/// various memory regions on the hardware. On the CPU
/// all of the address spaces point to the same memory,
/// however on the GPU, each address space points to
/// a seperate piece of memory that is unique from other
/// memory locations.
namespace AMDILAS {
enum AddressSpaces {
  PRIVATE_ADDRESS  = 0, // Address space for private memory.
  GLOBAL_ADDRESS   = 1, // Address space for global memory.
  CONSTANT_ADDRESS = 2, // Address space for constant memory.
  LOCAL_ADDRESS    = 3, // Address space for local memory.
  REGION_ADDRESS   = 4, // Address space for region memory.
  GLOBAL_HOST_ADDRESS = 5, // Address space with global host endianness.
  CONSTANT_HOST_ADDRESS = 6, // Address space with constant host endianness.
  FLAT_ADDRESS     = 7, // Address space for flat memory.
  ADDRESS_NONE     = 8,  // Address space for unknown memory.

  // Do not re-order the CONSTANT_BUFFER_* enums. Several places depend on this
  // order to be able to dynamically index a constant buffer, for example:
  //
  // ConstantBufferAS = CONSTANT_BUFFER_0 + CBIdx

  CONSTANT_BUFFER_0 = 9,
  CONSTANT_BUFFER_1 = 10,
  CONSTANT_BUFFER_2 = 11,
  CONSTANT_BUFFER_3 = 12,
  CONSTANT_BUFFER_4 = 13,
  CONSTANT_BUFFER_5 = 14,
  CONSTANT_BUFFER_6 = 15,
  CONSTANT_BUFFER_7 = 16,
  CONSTANT_BUFFER_8 = 17,
  CONSTANT_BUFFER_9 = 18,
  CONSTANT_BUFFER_10 = 19,
  CONSTANT_BUFFER_11 = 20,
  CONSTANT_BUFFER_12 = 21,
  CONSTANT_BUFFER_13 = 22,
  CONSTANT_BUFFER_14 = 23
};

// We are piggybacking on the CommentFlag enum in MachineInstr.h to
// set bits in AsmPrinterFlags of the MachineInstruction. We will
// start at bit 16 and allocate down while LLVM will start at bit
// 1 and allocate up.

// This union/struct combination is an easy way to read out the
// exact bits that are needed.
typedef union ResourceRec {
  struct {
#ifdef __BIG_ENDIAN__
    unsigned short CacheableRead : 1;  // Flag to specify if the read is
                                       // cacheable. (Permanent)
    unsigned short HardwareInst  : 1;  // Flag to specify that this instruction
                                       // is a hardware instruction. (Permanent)
    unsigned short ResourceID    : 10; // Flag to specify the resource ID for
                                       // the op. (Permanent)
    unsigned short PointerPath   : 1;  // Flag to specify if the op is on the
                                       // pointer path.
    unsigned short ByteStore     : 1;  // Flag to specify if the op is byte
                                       // store op.
    unsigned short ConflictPtr   : 1;  // Flag to specify that the pointer has
                                       // a conflict.
    unsigned short isImage       : 1;  // Reserved for future use.
#else
    unsigned short isImage       : 1;  // Reserved for future use/llvm.
    unsigned short ConflictPtr   : 1;  // Flag to specify that the pointer has a
                                       // conflict.
    unsigned short ByteStore     : 1;  // Flag to specify if the op is a byte
                                       // store op.
    unsigned short PointerPath   : 1;  // Flag to specify if the op is on the
                                       // pointer path.
    unsigned short ResourceID    : 10; // Flag to specify the resourece ID for
                                       // the op. (Permanent)
    unsigned short HardwareInst  : 1;  // Flag to specify that this instruction
                                       // is a hardware instruction. (Permanent)
    unsigned short CacheableRead : 1;  // Flag to specify if the read is
                                       // cacheable. (Permanent)
#endif
  } bits;
  unsigned short u16all;

  ResourceRec() { u16all = 0; }
} InstrResEnc;

} // namespace AMDILAS

// The OpSwizzle encodes a subset of all possible
// swizzle combinations into a number of bits using
// only the combinations utilized by the backend.
// The lower 128 are for source swizzles and the
// upper 128 or for destination swizzles.
// The valid mappings can be found in the
// getSrcSwizzle and getDstSwizzle functions of
// AMDILUtilityFunctions.cpp.
typedef union SwizzleRec {
  struct {
#ifdef __BIG_ENDIAN__
    unsigned char dst : 1;
    unsigned char swizzle : 7;
#else
    unsigned char swizzle : 7;
    unsigned char dst : 1;
#endif
  } bits;
  unsigned char u8all;
} OpSwizzle;
} // end namespace llvm
#endif // AMDIL_H_
