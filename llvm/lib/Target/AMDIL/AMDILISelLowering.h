//===-- AMDILISelLowering.h - AMDIL DAG Lowering Interface ------*- C++ -*-===//
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
// This file defines the interfaces that AMDIL uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef AMDIL_ISELLOWERING_H_
#define AMDIL_ISELLOWERING_H_
#include "AMDIL.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Support/Compiler.h"

namespace llvm
{
  namespace AMDILISD
  {
    enum
    {
      FIRST_NUMBER = ISD::BUILTIN_OP_END,
      UBIT_EXTRACT, // Extract a range of bits with zero extension to 32-bits.
      IBIT_EXTRACT, // Extract a range of bits with sign extension to 32-bits.
      UBIT_INSERT, // Insert a range of bits into a 32-bit integer.
      BFI,         // (src0 & src1) | (~src0 & src2)
      DP_TO_FP,    // Conversion from 64bit FP to 32bit FP
      FP_TO_DP,    // Conversion from 32bit FP to 64bit FP
      VBUILD,      // scalar to vector mov instruction
      VEXTRACT,    // extract vector components
      VINSERT,     // insert vector components
      VCONCAT,     // concat a single vector to another vector
      CALL,        // Function call based on a single integer
      RET,         // Return from a function call
      SELECT_CC,   // Select the correct conditional instruction
      LCREATE,     // Create a 64bit integer from two 32 bit integers
      LCOMPHI,     // Get the hi 32 bits from a 64 bit integer
      LCOMPLO,     // Get the lo 32 bits from a 64 bit integer
      DCREATE,     // Create a 64bit float from two 32 bit integers
      DCOMPHI,     // Get the hi 32 bits from a 64 bit float
      DCOMPLO,     // Get the lo 32 bits from a 64 bit float
      LCREATE2,     // Create a 64bit integer from two 32 bit integers
      LCOMPHI2,     // Get the hi 32 bits from a 64 bit integer
      LCOMPLO2,     // Get the lo 32 bits from a 64 bit integer
      DCREATE2,     // Create a 64bit float from two 32 bit integers
      DCOMPHI2,     // Get the hi 32 bits from a 64 bit float
      DCOMPLO2,     // Get the lo 32 bits from a 64 bit float
      UMUL,        // 32bit unsigned multiplication
      RET_FLAG,
      BRANCH_COND,
      ADDADDR,
      // ATOMIC Operations
      // Global Memory
      ATOM_G_ADD = ISD::FIRST_TARGET_MEMORY_OPCODE,
      ATOM_G_AND,
      ATOM_G_CMPXCHG,
      ATOM_G_DEC,
      ATOM_G_INC,
      ATOM_G_MAX,
      ATOM_G_UMAX,
      ATOM_G_MIN,
      ATOM_G_UMIN,
      ATOM_G_OR,
      ATOM_G_SUB,
      ATOM_G_RSUB,
      ATOM_G_XCHG,
      ATOM_G_XOR,
      ATOM_G_STORE,
      ATOM_G_LOAD,
      ATOM_G_ADD_NORET,
      ATOM_G_AND_NORET,
      ATOM_G_CMPXCHG_NORET,
      ATOM_G_DEC_NORET,
      ATOM_G_INC_NORET,
      ATOM_G_MAX_NORET,
      ATOM_G_UMAX_NORET,
      ATOM_G_MIN_NORET,
      ATOM_G_UMIN_NORET,
      ATOM_G_OR_NORET,
      ATOM_G_SUB_NORET,
      ATOM_G_RSUB_NORET,
      ATOM_G_XCHG_NORET,
      ATOM_G_XOR_NORET,
      // Local Memory
      ATOM_L_ADD,
      ATOM_L_AND,
      ATOM_L_CMPXCHG,
      ATOM_L_DEC,
      ATOM_L_INC,
      ATOM_L_MAX,
      ATOM_L_UMAX,
      ATOM_L_MIN,
      ATOM_L_UMIN,
      ATOM_L_OR,
      ATOM_L_MSKOR,
      ATOM_L_SUB,
      ATOM_L_RSUB,
      ATOM_L_XCHG,
      ATOM_L_XOR,
      ATOM_L_ADD_NORET,
      ATOM_L_AND_NORET,
      ATOM_L_CMPXCHG_NORET,
      ATOM_L_DEC_NORET,
      ATOM_L_INC_NORET,
      ATOM_L_MAX_NORET,
      ATOM_L_UMAX_NORET,
      ATOM_L_MIN_NORET,
      ATOM_L_UMIN_NORET,
      ATOM_L_OR_NORET,
      ATOM_L_MSKOR_NORET,
      ATOM_L_SUB_NORET,
      ATOM_L_RSUB_NORET,
      ATOM_L_XCHG_NORET,
      ATOM_L_XOR_NORET,
      // Region Memory
      ATOM_R_ADD,
      ATOM_R_AND,
      ATOM_R_CMPXCHG,
      ATOM_R_DEC,
      ATOM_R_INC,
      ATOM_R_MAX,
      ATOM_R_UMAX,
      ATOM_R_MIN,
      ATOM_R_UMIN,
      ATOM_R_OR,
      ATOM_R_MSKOR,
      ATOM_R_SUB,
      ATOM_R_RSUB,
      ATOM_R_XCHG,
      ATOM_R_XOR,
      ATOM_R_ADD_NORET,
      ATOM_R_AND_NORET,
      ATOM_R_CMPXCHG_NORET,
      ATOM_R_DEC_NORET,
      ATOM_R_INC_NORET,
      ATOM_R_MAX_NORET,
      ATOM_R_UMAX_NORET,
      ATOM_R_MIN_NORET,
      ATOM_R_UMIN_NORET,
      ATOM_R_OR_NORET,
      ATOM_R_MSKOR_NORET,
      ATOM_R_SUB_NORET,
      ATOM_R_RSUB_NORET,
      ATOM_R_XCHG_NORET,
      ATOM_R_XOR_NORET,
      // Append buffer
      APPEND_ALLOC,
      APPEND_CONSUME,
      // 2D Images
      IMAGE2D_READ,
      IMAGE2D_WRITE,
      IMAGE2D_INFO0,
      IMAGE2D_INFO1,
      // 3D Images
      IMAGE3D_READ,
      IMAGE3D_WRITE,
      IMAGE3D_INFO0,
      IMAGE3D_INFO1,
      ATOM_F_ADD,
      ATOM_F_AND,
      ATOM_F_CMPXCHG,
      ATOM_F_DEC,
      ATOM_F_INC,
      ATOM_F_MAX,
      ATOM_F_UMAX,
      ATOM_F_MIN,
      ATOM_F_UMIN,
      ATOM_F_OR,
      ATOM_F_SUB,
      ATOM_F_XCHG,
      ATOM_F_XOR,

      LAST_ISD_NUMBER
    };
  } // AMDILISD

  class MachineBasicBlock;
  class MachineInstr;
  class DebugLoc;
  class TargetInstrInfo;

  class AMDILTargetLowering : public TargetLowering {
  private:
    int VarArgsFrameOffset;   // Frame offset to start of varargs area.

    void setFloatCondCodeActions(MVT::SimpleValueType VT);
    void setIntCondCodeActions(MVT::SimpleValueType VT);

  public:
      AMDILTargetLowering(TargetMachine &TM);

      virtual MVT getShiftAmountTy(EVT LHSTy) const LLVM_OVERRIDE {
        return MVT::i32;
      }

      virtual SDValue
        LowerOperation(SDValue Op, SelectionDAG &DAG) const LLVM_OVERRIDE;

     virtual void ReplaceNodeResults(SDNode *N,
                                     SmallVectorImpl<SDValue> &Results,
                                     SelectionDAG &DAG) const LLVM_OVERRIDE;

      virtual SDValue
        PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const LLVM_OVERRIDE;

      int
        getVarArgsFrameOffset() const;

      /// computeMaskedBitsForTargetNode - Determine which of
      /// the bits specified
      /// in Mask are known to be either zero or one and return them in
      /// the
      /// KnownZero/KnownOne bitsets.
      virtual void
        computeMaskedBitsForTargetNode(
            const SDValue Op,
            APInt &KnownZero,
            APInt &KnownOne,
            const SelectionDAG &DAG,
            unsigned Depth = 0
            ) const LLVM_OVERRIDE;

      virtual bool
        getTgtMemIntrinsic(IntrinsicInfo &Info,
                           const CallInst &I,
                           unsigned Intrinsic) const LLVM_OVERRIDE;
      virtual const char*
        getTargetNodeName(
            unsigned Opcode
            ) const LLVM_OVERRIDE;

    virtual bool isLegalAddressingMode(const AddrMode &AM,
                                       Type *Ty) const LLVM_OVERRIDE;

    virtual bool isTruncateFree(Type *Src, Type *Dest) const LLVM_OVERRIDE;
    virtual bool isTruncateFree(EVT Src, EVT Dest) const LLVM_OVERRIDE;

    virtual bool isZExtFree(Type *Src, Type *Dest) const LLVM_OVERRIDE;
    virtual bool isZExtFree(EVT VT1, EVT VT2) const LLVM_OVERRIDE;

      /// getSetCCResultType - Return the value type to use for ISD::SETCC.
      virtual EVT getSetCCResultType(LLVMContext &Context, EVT VT) const LLVM_OVERRIDE;

      // We want to mark f32/f64 floating point values as
      // legal
      bool
        isFPImmLegal(const APFloat &Imm, EVT VT) const;
      // We don't want to shrink f64/f32 constants because
      // they both take up the same amount of space and
      // we don't want to use a f2d instruction.
      bool ShouldShrinkFPConstant(EVT VT) const;

      /// getFunctionAlignment - Return the Log2 alignment of this
      /// function.
      unsigned int
        getFunctionAlignment(const Function *F) const;

      /// This function returns true if the target allows unaligned memory accesses.
      /// of the specified type. This is used, for example, in situations where an
      /// array copy/move/set is  converted to a sequence of store operations. It's
      /// use helps to ensure that such replacements don't generate code that causes
      /// an alignment error  (trap) on the target machine.
      /// @brief Determine if the target supports unaligned memory accesses.
      bool allowsUnalignedMemoryAccesses(EVT VT) const;

      /// Return true if the load uses larger data types than
      /// the bitcast and false otherwise.
      /// This should disable optimizing:
      /// (char16)((int4*)ptr)[idx] => (char16*)ptr[idx]
      /// but not disable:
      /// (int4)((char16*)ptr)[idx] => (int4*)ptr[idx]
      bool isLoadBitCastBeneficial(EVT load, EVT bitcast) const LLVM_OVERRIDE;

    private:
      CCAssignFn* CCAssignFnForNode(unsigned int CC, bool isReturn,
          bool atCallSite) const;

      SDValue LowerCallResult(SDValue Chain,
          SDValue InFlag,
          CallingConv::ID CallConv,
          bool IsVarArg,
          const SmallVectorImpl<ISD::InputArg> &Ins,
          DebugLoc DL,
          SelectionDAG &DAG,
          SmallVectorImpl<SDValue> &InVals) const;

      SDValue LowerMemArgument(SDValue Chain,
          CallingConv::ID CallConv,
          const SmallVectorImpl<ISD::InputArg> &ArgInfo,
          DebugLoc dl, SelectionDAG &DAG,
          const CCValAssign &VA,  MachineFrameInfo *MFI,
          unsigned i) const;

      SDValue LowerMemOpCallTo(SDValue Chain, SDValue StackPtr,
          SDValue Arg,
          DebugLoc dl, SelectionDAG &DAG,
          const CCValAssign &VA,
          ISD::ArgFlagsTy Flags) const;

      virtual SDValue
        LowerFormalArguments(SDValue Chain,
            CallingConv::ID CallConv, bool isVarArg,
            const SmallVectorImpl<ISD::InputArg> &Ins,
            DebugLoc dl, SelectionDAG &DAG,
            SmallVectorImpl<SDValue> &InVals) const LLVM_OVERRIDE;

      virtual SDValue
      LowerCall(CallLoweringInfo &CLI,
                SmallVectorImpl<SDValue> &InVals) const LLVM_OVERRIDE;

      virtual SDValue
        LowerReturn(SDValue Chain,
            CallingConv::ID CallConv, bool isVarArg,
            const SmallVectorImpl<ISD::OutputArg> &Outs,
            const SmallVectorImpl<SDValue> &OutVals,
            DebugLoc dl, SelectionDAG &DAG) const LLVM_OVERRIDE;

      //+++--- Function dealing with conversions between floating point and
      //integer types ---+++//
      SDValue
        genCLZu64(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        genCLZuN(SDValue Op, SelectionDAG &DAG, uint32_t bits) const;
      SDValue
        genCLZu32(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        genf64toi32(SDValue Op, SelectionDAG &DAG, bool includeSign) const;
      SDValue
        genf64toi64(SDValue Op, SelectionDAG &DAG, bool includeSign) const;
      SDValue
        genf32toi64(SDValue Op, SelectionDAG &DAG, bool includeSign) const;
      SDValue
        geni64tof32(SDValue Op, SelectionDAG &DAG, bool includeSign) const;

      SDValue
        genu32tof64(SDValue Op, EVT dblvt, SelectionDAG &DAG) const;


      SDValue
        genu64tof64(SDValue Op, EVT dblvt, SelectionDAG &DAG) const;

      SDValue
        LowerFP_TO_SINT(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerFP_TO_UINT(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerSINT_TO_FP(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerUINT_TO_FP(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerINTRINSIC_WO_CHAIN(SDValue Op, SelectionDAG& DAG) const;

      SDValue
        LowerINTRINSIC_W_CHAIN(SDValue Op, SelectionDAG& DAG) const;

      SDValue
        LowerINTRINSIC_VOID(SDValue Op, SelectionDAG& DAG) const;

      SDValue
        LowerJumpTable(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerConstantPool(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerExternalSymbol(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerADD(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerSUB(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerSREM(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerSREM8(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerSREM16(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerSREM32(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerSREM64(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerUREM(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerUREM8(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerUREM16(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerUREM32(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerUREM64(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerSDIV(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerSDIV24(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerSDIV32(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerSDIV64(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerUDIV(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerUDIV24(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerUDIV32(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerUDIV64(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerFDIV(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerFDIV32(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerFDIV64(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerMUL(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerOR(SDValue Op, SelectionDAG &DAG) const;

      SDValue LowerSRL_SRA(SDValue Op, SelectionDAG &DAG, bool Signed) const;
      SDValue LowerSRL(SDValue Op, SelectionDAG &DAG) const;
      SDValue LowerSRA(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerBUILD_VECTOR(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerINSERT_VECTOR_ELT(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerEXTRACT_VECTOR_ELT(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerEXTRACT_SUBVECTOR(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerSCALAR_TO_VECTOR(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerCONCAT_VECTORS(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerSETCC(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerSIGN_EXTEND_INREG(SDValue Op, SelectionDAG &DAG) const;

      EVT
        genIntType(uint32_t size = 32, uint32_t numEle = 1) const;

      SDValue
        LowerBITCAST(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerDYNAMIC_STACKALLOC(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerBRCOND(SDValue Op, SelectionDAG &DAG) const;

      SDValue
        LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
      SDValue
        LowerFP_ROUND(SDValue Op, SelectionDAG &DAG) const;
      uint32_t
        addExtensionInstructions(
          uint32_t reg, bool signedShift,
          unsigned int simpleVT) const;
      SDValue
        BuildKernelArgVal(ArgListEntry &Arg, unsigned NumParts,
          unsigned PartIdx, SDValue OutVal, EVT VT, SDValue &Chain,
          unsigned &CBIdx, SelectionDAG &DAG) const;
  }; // AMDILTargetLowering
} // end namespace llvm

#endif    // AMDIL_ISELLOWERING_H_
