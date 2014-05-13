//===- AMDILInstrPatterns.td - AMDIL Target ------------===//
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
//===--------------------------------------------------------------------===//
// This file holds all the custom patterns that are used by the amdil backend
//
//===--------------------------------------------------------------------===//
//===--------------------------------------------------------------------===//
// Custom patterns for conversion operations
//===--------------------------------------------------------------------===////

// integer subtraction
// a - b ==> a + (-b)
def SUBi8rr : Pat<(sub GPRI8:$src0, GPRI8:$src1),
    (ADDi8rr GPRI8:$src0, (NEGi8r GPRI8:$src1))>;
def SUBv2i8rr : Pat<(sub GPRV2I8:$src0, GPRV2I8:$src1),
    (ADDv2i8rr GPRV2I8:$src0, (NEGv2i8r GPRV2I8:$src1))>;
def SUBv4i8rr : Pat<(sub GPRV4I8:$src0, GPRV4I8:$src1),
    (ADDv4i8rr GPRV4I8:$src0, (NEGv4i8r GPRV4I8:$src1))>;
def SUBi16rr : Pat<(sub GPRI16:$src0, GPRI16:$src1),
    (ADDi16rr GPRI16:$src0, (NEGi16r GPRI16:$src1))>;
def SUBv2i16rr : Pat<(sub GPRV2I16:$src0, GPRV2I16:$src1),
    (ADDv2i16rr GPRV2I16:$src0, (NEGv2i16r GPRV2I16:$src1))>;
def SUBv4i16rr : Pat<(sub GPRV4I16:$src0, GPRV4I16:$src1),
    (ADDv4i16rr GPRV4I16:$src0, (NEGv4i16r GPRV4I16:$src1))>;
def SUBi32rr : Pat<(sub GPR_32:$src0, GPR_32:$src1),
    (ADDi32rr GPR_32:$src0, (NEGi32r GPR_32:$src1))>;
def SUBv2i32rr : Pat<(sub GPRV2I32:$src0, GPRV2I32:$src1),
    (ADDv2i32rr GPRV2I32:$src0, (NEGv2i32r GPRV2I32:$src1))>;
def SUBv4i32rr : Pat<(sub GPRV4I32:$src0, GPRV4I32:$src1),
    (ADDv4i32rr GPRV4I32:$src0, (NEGv4i32r GPRV4I32:$src1))>;

// LLVM isn't lowering this correctly, so writing a pattern that
// matches it isntead.
def : Pat<(build_vector (f32 fpimm:$src)),
    (v4f32 (VCREATEv4f32r (LOADCONSTf32 fpimm:$src)))>;
def : Pat<(build_vector (i32 imm:$src)),
    (v4i32 (VCREATEv4i32r (LOADCONSTi32 imm:$src)))>;
def : Pat<(build_vector (i16 imm:$src)),
    (VCREATEv4i16r (LOADCONSTi16 imm:$src))>;
def : Pat<(build_vector (i8 imm:$src)),
    (VCREATEv4i8r (LOADCONSTi8 imm:$src))>;
def : Pat<(build_vector (f64 fpimm:$src)),
    (v2f64 (VCREATEv2f64r (LOADCONSTf64 fpimm:$src)))>;
def : Pat<(build_vector (f32 fpimm:$src)),
    (v2f32 (VCREATEv2f32r (LOADCONSTf32 fpimm:$src)))>;
def : Pat<(build_vector (i64 imm:$src)),
    (v2i64 (VCREATEv2i64r (LOADCONSTi64 imm:$src)))>;
def : Pat<(build_vector (i32 imm:$src)),
    (v2i32 (VCREATEv2i32r (LOADCONSTi32 imm:$src)))>;
def : Pat<(build_vector (i16 imm:$src)),
    (VCREATEv2i16r (LOADCONSTi16 imm:$src))>;
def : Pat<(build_vector (i8 imm:$src)),
    (VCREATEv2i8r (LOADCONSTi8 imm:$src))>;

// Calls:
def : Pat<(IL_call tglobaladdr:$dst),
    (CALL tglobaladdr:$dst)>;
def : Pat<(IL_call texternalsym:$dst),
    (CALL texternalsym:$dst)>;
def : Pat<(IL_call tconstpool:$dst),
  (CALL tconstpool:$dst)>;

include "AMDILConversions.td"

/// Bitfield Insert pattern fragments
def isLoadConstantAllOnes : PatLeaf<(timm),
    [{
      return N->isAllOnesValue();
    }]>;

/// Pattern 1: (lhs & bitpat) | (rhs & ~bitpat)
def bfi_pat1 : PatFrag<(ops node:$bitpat, node:$lhs, node:$rhs),
    (or
     (and node:$lhs, node:$bitpat),
     (and node:$rhs, (not node:$bitpat)))>;

/// Pattern 2: (lhs & bitpat) | (rhs & (bitpat ^ -1))
def bfi_pat2 : PatFrag<(ops node:$bitpat, node:$lhs, node:$rhs),
    (or
     (and node:$lhs, node:$bitpat),
     (and node:$rhs,
      (xor node:$bitpat, isLoadConstantAllOnes) ))>;

/// Pattern 3: (rhs ^ ((rhs ^ lhs) & bitpat))
def bfi_pat3 : PatFrag<(ops node:$bitpat, node:$lhs, node:$rhs),
    (xor node:$rhs,
     (and (xor node:$rhs, node:$lhs),
       node:$bitpat))>;

def bitmask_5bits : PatFrag<(ops node:$mask),
  (and node:$mask, (i32 0x1f))>;

/// Bitfield mask instruction patterns.
/// Pattern 1: ((1 << (width & 0x1F)) + 0xFFFFFFFF) << (offset & 0x1F)
def bfm_pat1 : PatFrag<(ops node:$width, node:$offset),
    (shl (add (shl (i32 1), (bitmask_5bits node:$width)), (i32 0xFFFFFFFF)),
     (bitmask_5bits node:$offset))>;

let Predicates = [HasHWBitFieldInst] in {
defm BFI_PAT1A : TernaryPatReg<IL_OP_BFI, bfi_pat1, GPR_32, i32>;
defm BFI_PAT2A : TernaryPatReg<IL_OP_BFI, bfi_pat2, GPR_32, i32>;
defm BFI_PAT3  : TernaryPatReg<IL_OP_BFI, bfi_pat3, GPR_32, i32>;
defm BFM_PAT1  : BinaryPatMCi32Scalar<IL_OP_BFM, bfm_pat1>;
}

//
// bitalign
// dst = (src0 << (32 - src2[4:0])) | (src1 >> src2[4:0])

// A.  src2 is constant
def bitalign_1 : PatFrag<(ops node:$src0, node:$src1, node:$src2, node:$src3),
    (or (shl  node:$src0, node:$src3), (srl  node:$src1, node:$src2)),
    [{
      SDNode *N_or1 = N->getOperand(1).getNode();
      SDNode *N_src2 = N_or1->getOperand(1).getNode();
      ConstantSDNode* CN_src2 = dyn_cast<ConstantSDNode>(N_src2);
      if (!CN_src2) {
        return false;
      }

      SDNode *N_or0 = N->getOperand(0).getNode();
      SDNode *N_src3 = N_or0->getOperand(1).getNode();
      ConstantSDNode* CN_src3 = dyn_cast<ConstantSDNode>(N_src3);
      if (!CN_src3) {
        return false;
      }

      uint32_t csrc2 = CN_src2->getZExtValue();
      uint32_t csrc3 = CN_src3->getZExtValue();
      return (csrc3 == (32 - csrc2));
    }]>;

def bitalign_2 : PatFrag<(ops node:$src0, node:$src1, node:$src2, node:$src3),
    (or (srl  node:$src1, node:$src2), (shl  node:$src0, node:$src3)),
    [{
      SDNode *N_or0 = N->getOperand(0).getNode();
      SDNode *N_src2 = N_or0->getOperand(1).getNode();
      ConstantSDNode* CN_src2 = dyn_cast<ConstantSDNode>(N_src2);
      if (!CN_src2) {
        return false;
      }

      SDNode *N_or1 = N->getOperand(1).getNode();
      SDNode *N_src3 = N_or1->getOperand(1).getNode();
      ConstantSDNode* CN_src3 = dyn_cast<ConstantSDNode>(N_src3);
      if (!CN_src3) {
        return false;
      }

      uint32_t csrc2 = CN_src2->getZExtValue();
      uint32_t csrc3 = CN_src3->getZExtValue();
      return (csrc3 == (32 - csrc2));
    }]>;

// B.  src2 is a variable

def bitalign_3 : PatFrag<(ops node:$src0, node:$src1, node:$src2),
    (or (shl  node:$src0,
                (bitmask_5bits (sub (i32 0), node:$src2))),
        (srl  node:$src1, (bitmask_5bits node:$src2)))>;

// Do bitalign pattern recognization if device is EG or later.
let Predicates = [IsEGOrLaterDevice] in {
defm BITALIGN_PAT_1  : BitAlignPatFragCI32<IL_OP_BIT_ALIGN, bitalign_1>;
defm BITALIGN_PAT_2  : BitAlignPatFragCI32<IL_OP_BIT_ALIGN, bitalign_2>;
defm BITALIGN_PAT_3  : TernaryPatMCi32Scalar<IL_OP_BIT_ALIGN, bitalign_3>;
}

// unpack[0-3] dst, src

def unpack0 : PatFrag<(ops node:$src),
    (uint_to_fp (and node:$src, (i32 0xFF)))>;
def unpack0_1 : PatFrag<(ops node:$src),
    (uint_to_fp (i32 (IL_ubit_extract (i32 8), (i32 0), node:$src)))>;
def unpack1 : PatFrag<(ops node:$src),
    (uint_to_fp (and (srl node:$src, (i32 8)), (i32 0xFF)))>;
def unpack1_1 : PatFrag<(ops node:$src),
    (uint_to_fp (i32 (IL_ubit_extract (i32 8), (i32 8), node:$src)))>;
def unpack2 : PatFrag<(ops node:$src),
    (uint_to_fp (and (srl node:$src, (i32 16)), (i32 0xFF)))>;
def unpack2_1 : PatFrag<(ops node:$src),
    (uint_to_fp (i32 (IL_ubit_extract (i32 8), (i32 16), node:$src)))>;
def unpack3 : PatFrag<(ops node:$src), (uint_to_fp (srl node:$src, (i32 24)))>;
def unpack3_1 : PatFrag<(ops node:$src),
    (uint_to_fp (i32 (IL_ubit_extract (i32 8), (i32 24), node:$src)))>;

let Predicates = [IsEGOrLaterDevice] in {
defm UNPACK_PAT0   : UnpackPatFrag<IL_OP_UNPACK_0, unpack0>;
defm UNPACK_PAT0_1 : UnpackPatFrag<IL_OP_UNPACK_0, unpack0_1>;
defm UNPACK_PAT1   : UnpackPatFrag<IL_OP_UNPACK_1, unpack1>;
defm UNPACK_PAT1_1 : UnpackPatFrag<IL_OP_UNPACK_1, unpack1_1>;
defm UNPACK_PAT2   : UnpackPatFrag<IL_OP_UNPACK_2, unpack2>;
defm UNPACK_PAT2_1 : UnpackPatFrag<IL_OP_UNPACK_2, unpack2_1>;
defm UNPACK_PAT3   : UnpackPatFrag<IL_OP_UNPACK_3, unpack3>;
defm UNPACK_PAT3_1 : UnpackPatFrag<IL_OP_UNPACK_3, unpack3_1>;
}

