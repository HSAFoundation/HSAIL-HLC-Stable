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

// Operations in this file are generic to all data types
// This opcode has custom swizzle pattern encoded in Swizzle Encoder
let isReMaterializable = 0, isAsCheapAsAMove = 1, loadconst = 1 in {
  defm LOADCONST : ILConstant<"mov $dst, $val">;
}

let isCommutable = 1 in {
  defm OR : BinaryOpMCInt<IL_OP_I_OR, or>;
  defm AND : BinaryOpMCInt<IL_OP_AND, and>;
  defm SMUL : BinaryOpMCi32<IL_OP_I_MUL, mul>;
  defm SMULHI : BinaryOpMCi32<IL_OP_I_MUL_HIGH, mulhs>;
  defm XOR : BinaryOpMCInt<IL_OP_I_XOR, xor>;
}

defm EADD : BinaryOpMCi32<IL_OP_I_ADD, adde>;

let isSelect = 1 in {
  defm CMOV_LOGICAL : SelectMC<IL_OP_CMOV_LOGICAL>;
}


// Integer offsets for addressing
let Uses = [SP] in {
  def LOADFIi32 : ILFormat<IL_OP_I_ADD, (outs GPR_32:$dst), (ins i32imm:$val),
                           !strconcat(IL_OP_I_ADD.Text, " $dst, x0[0], $val"),
                           [(set GPR_32:$dst, frameindex:$val)]>;
}

let isCommutable = 1 in {
  def ADDpr : ILFormat<IL_OP_I_ADD, (outs GPR_32:$dst),
        (ins MEM32:$ptr, GPR_32:$offset),
            !strconcat(IL_OP_I_ADD.Text, " $dst, $ptr, $offset"),
            [(set GPR_32:$dst,
          (IL_addaddrri ADDR:$ptr,
            (i32 GPR_32:$offset)))]>;
  def ADDrp : ILFormat<IL_OP_I_ADD, (outs GPR_32:$dst),
        (ins GPR_32:$offset,  MEM32:$ptr),
            !strconcat(IL_OP_I_ADD.Text, " $dst, $offset, $ptr"),
            [(set GPR_32:$dst,
          (IL_addaddrir
            (i32 GPR_32:$offset), ADDR:$ptr))]>;
  def ADDi64pr : ILFormat<IL_OP_I64_ADD, (outs GPR_64:$dst),
        (ins MEM64:$ptr, GPR_64:$offset),
            !strconcat(IL_OP_I64_ADD.Text, " $dst, $ptr, $offset"),
            [(set GPR_64:$dst,
          (IL_addaddrri ADDR64:$ptr,
            (i64 GPR_64:$offset)))]>;
  def ADDi64rp : ILFormat<IL_OP_I64_ADD, (outs GPR_64:$dst),
        (ins GPR_64:$offset,  MEM64:$ptr),
            !strconcat(IL_OP_I64_ADD.Text, " $dst, $offset, $ptr"),
            [(set GPR_64:$dst,
          (IL_addaddrir
            (i64 GPR_64:$offset), ADDR64:$ptr))]>;
}


defm ABS : UnaryIntMCInt<IL_OP_ABS, int_AMDIL_abs>;
defm BITCOUNT : UnaryIntMCInt<IL_OP_IBIT_COUNT, int_AMDIL_bit_count_i32>;
defm FFB_LO : UnaryIntMCInt<IL_OP_I_FFB_LO, int_AMDIL_bit_find_first_lo>;
defm FFB_HI : UnaryIntMCInt<IL_OP_I_FFB_HI, int_AMDIL_bit_find_first_hi>;
defm FFB_SGN : UnaryIntMCInt<IL_OP_I_FFB_SGN, int_AMDIL_bit_find_first_sgn>;

let isCommutable = 1 in {
  defm IMULHI : BinaryIntMCi32<IL_OP_I_MUL_HIGH, int_AMDIL_mulhi_i32>;
}

let Predicates = [HasHWSign24Bit] in {
  let isCommutable = 1 in {
    defm IMUL24 : BinaryIntMCi32<IL_OP_I_MUL24, int_AMDIL_mul24_i32>;
    defm IMULHI24 : BinaryIntMCi32<IL_OP_I_MULHI24, int_AMDIL_mulhi24_i32>;
  }

  defm IMAD24 : TernaryPatMCInt<IL_OP_I_MAD24, int_AMDIL_mad24_i32>;
}

defm CARRY : BinaryIntMCi32<IL_OP_I_CARRY, int_AMDIL_carry_i32>;
defm BORROW : BinaryIntMCi32<IL_OP_I_BORROW, int_AMDIL_borrow_i32>;

let isCommutable = 1 in {
  defm IMIN : BinaryIntMCi32<IL_OP_I_MIN, int_AMDIL_min_i32>;
  defm IMAX : BinaryIntMCi32<IL_OP_I_MAX, int_AMDIL_max_i32>;
}

// These only match the cmov_logical intrinsics used in the
// library. These should be removed entirely once those are removed.
def : Pat<(int_AMDIL_cmov_logical GPR_32:$src0, GPR_32:$src1, GPR_32:$src2),
  (CMOV_LOGICALi32rrr GPR_32:$src0, GPR_32:$src1, GPR_32:$src2)>;
def : Pat<(int_AMDIL_cmov_logical GPRV2I32:$src0, GPRV2I32:$src1, GPRV2I32:$src2),
  (CMOV_LOGICALv2i32rrr GPRV2I32:$src0, GPRV2I32:$src1, GPRV2I32:$src2)>;
def : Pat<(int_AMDIL_cmov_logical GPRV4I32:$src0, GPRV4I32:$src1, GPRV4I32:$src2),
  (CMOV_LOGICALv4i32rrr GPRV4I32:$src0, GPRV4I32:$src1, GPRV4I32:$src2)>;



defm IMAD : TernaryPatMCInt<IL_OP_I_MAD, int_AMDIL_mad_i32>;
defm SAD : TernaryPatMCInt<IL_OP_SAD, int_AMDIL_media_sad>;
defm SADHI : TernaryPatMCInt<IL_OP_SAD_HI, int_AMDIL_media_sad_hi>;

defm SAD4 : TernaryIntMCFull<IL_OP_SAD4, int_AMDIL_media_sad4,
                             i32, GPR_32, v4i32, GPRV4I32, v4i32imm, imm,
                             v4i32, GPRV4I32, v4i32imm, imm, i32, GPR_32, i32imm, imm>;
defm FTOV4U8 : UnaryIntMCFull<IL_OP_F2U4, int_AMDIL_media_convert_f2v4u8,
                              i32, GPR_32, v4f32, GPRV4I32, v4f32imm, fpimm>;

//===-------------------------------------------------------------===//
// Unsigned 32bit integer math instructions start here
//===-------------------------------------------------------------===//
defm UDIV : BinaryOpMCi32<IL_OP_U_DIV, udiv>;
defm NATIVE_UDIV : BinaryIntMCi32<IL_OP_U_DIV, int_AMDIL_udiv>;

let isCommutable = 1 in {
  defm UMUL : BinaryOpMCi32<IL_OP_U_MUL, IL_umul>;
  defm UMULHI : BinaryOpMCi32<IL_OP_U_MUL_HIGH, mulhu>;
  defm UMULHI_INT : BinaryIntMCi32<IL_OP_U_MUL_HIGH, int_AMDIL_mulhi_u32>;
  defm UMULHI24 : BinaryIntMCi32<IL_OP_U_MULHI24, int_AMDIL_mulhi24_u32>;
  defm UMUL24 : BinaryIntMCi32<IL_OP_U_MUL24, int_AMDIL_mul24_u32>;
  defm UMIN : BinaryIntMCi32<IL_OP_U_MIN, int_AMDIL_min_u32>;
  defm UMAX : BinaryIntMCi32<IL_OP_U_MAX, int_AMDIL_max_u32>;
}

let swizzle = 6 in {
  defm UBIT_INSERT : Quaternaryi32Inst<IL_OP_UBIT_INSERT, IL_ubit_insert>;
}

defm UBIT_REVERSE : UnaryIntMCInt<IL_OP_UBIT_REVERSE, int_AMDIL_bit_reverse_u32>;
defm UBIT_EXTRACT : TernaryPatMCInt<IL_OP_UBIT_EXTRACT, IL_ubit_extract>;
defm IBIT_EXTRACT : TernaryPatMCInt<IL_OP_IBIT_EXTRACT, IL_ibit_extract>;

defm BFI : TernaryPatMCInt<IL_OP_BFI, IL_bfi>;
defm BFM : BinaryIntMCi32<IL_OP_BFM, int_AMDIL_bfm>;
defm UMAD : TernaryPatMCInt<IL_OP_U_MAD, int_AMDIL_mad_u32>;
defm UMAD24 : TernaryPatMCInt<IL_OP_U_MAD24, int_AMDIL_mad24_u32>;
defm U4LERP : TernaryPatMCInt<IL_OP_U4_LERP, int_AMDIL_media_lerp_u4>;
defm BITALIGN : TernaryPatMCInt<IL_OP_BIT_ALIGN, int_AMDIL_media_bitalign>;
defm BYTEALIGN : TernaryPatMCInt<IL_OP_BYTE_ALIGN, int_AMDIL_media_bytealign>;

let Predicates = [HasHW64Bit], isCommutable = 1 in {
  defm ADD    : BinaryOpMCi64<IL_OP_I64_ADD, add>;
  defm IMIN64 : BinaryIntMCi64<IL_OP_I64_MIN, int_AMDIL_min_i32>;
  defm UMIN64 : BinaryIntMCi64<IL_OP_U64_MIN, int_AMDIL_min_u32>;
  defm IMAX64 : BinaryIntMCi64<IL_OP_I64_MAX, int_AMDIL_max_i32>;
  defm UMAX64 : BinaryIntMCi64<IL_OP_U64_MAX, int_AMDIL_max_u32>;
}


defm SHL  : BinaryNCOpMCi32Shift<IL_OP_I_SHL, shl>;
defm SHR  : BinaryNCOpMCi32Shift<IL_OP_I_SHR, sra>;
defm USHR : BinaryNCOpMCi32Shift<IL_OP_U_SHR, srl>;

let Predicates = [HasHW64Bit] in {
  defm SHL  : BinaryNCOpMCi64Shift<IL_OP_I64_SHL, shl>;
  defm SHR  : BinaryNCOpMCi64Shift<IL_OP_I64_SHR, sra>;
  defm USHR : BinaryNCOpMCi64Shift<IL_OP_U64_SHR, srl>;
}

let Predicates = [HasTmrRegister] in {
  def Tmr : ILFormat<IL_OP_MOV, (outs GPR_XY_64:$tmr),
      (ins), !strconcat(IL_OP_MOV.Text, " $tmr, Tmr.xyxy"),
      [(set GPR_XY_64:$tmr, (int_AMDIL_get_cycle_count))]>;
}

let Predicates = [IsEGOrLaterDevice] in {
  def CU_ID : ILFormat<IL_OP_CU_ID, (outs GPR_32:$id), (ins),
    !strconcat(IL_OP_CU_ID.Text, " $id"),
    [(set GPR_32:$id, (int_AMDIL_compute_unit_id))]>;
  def WAVE_ID : ILFormat<IL_OP_WAVE_ID, (outs GPR_32:$id), (ins),
    !strconcat(IL_OP_WAVE_ID.Text, " $id"),
    [(set GPR_32:$id, (int_AMDIL_wavefront_id))]>;
}

let ieee = 1, isCommutable = 1 in {
  defm MUL_IEEE : BinaryOpMCf32<IL_OP_MUL_IEEE, fmul>,
                  BinaryOpMCf64<IL_OP_D_MUL, fmul>;
}

let isCommutable = 1 in {
  defm ADD : BinaryOpMCf32<IL_OP_ADD, fadd>,
             BinaryOpMCi32<IL_OP_I_ADD, add>,
             BinaryOpMCf64<IL_OP_D_ADD, fadd>;
}

//===-------------------------------------------------------------===//
// float math instructions start here
//===-------------------------------------------------------------===//
defm ABS : UnaryIntMCf32<IL_OP_ABS, int_AMDIL_fabs>;
defm FRAC : UnaryIntMCf32<IL_OP_FRC, int_AMDIL_fraction>;
defm PIREDUCE : UnaryIntMCf32<IL_OP_PI_REDUCE, int_AMDIL_pireduce>;
defm ROUND_NEAREST : UnaryIntMCf32<IL_OP_ROUND_NEAR, int_AMDIL_round_nearest>;
defm ROUND_NEGINF : UnaryIntMCf32<IL_OP_ROUND_NEG_INF, int_AMDIL_round_neginf>;
defm ROUND_POSINF : UnaryIntMCf32<IL_OP_ROUND_POS_INF, int_AMDIL_round_posinf>;
defm ROUND_ZERO : UnaryIntMCf32<IL_OP_ROUND_ZERO, int_AMDIL_round_zero>;
defm ACOS : UnaryIntMCf32Scalar<IL_OP_ACOS, int_AMDIL_acos>;
defm ATAN : UnaryIntMCf32Scalar<IL_OP_ATAN, int_AMDIL_atan>;
defm ASIN : UnaryIntMCf32Scalar<IL_OP_ASIN, int_AMDIL_asin>;
defm TAN : UnaryIntMCf32Scalar<IL_OP_TAN, int_AMDIL_tan>;
defm SIN : UnaryIntMCf32Scalar<IL_OP_SIN, int_AMDIL_sin>;
defm COS : UnaryIntMCf32Scalar<IL_OP_COS, int_AMDIL_cos>;
defm SQRT : UnaryIntMCf32Scalar<IL_OP_SQRT, int_AMDIL_sqrt>;
defm EXP : UnaryIntMCf32Scalar<IL_OP_EXP, int_AMDIL_exp>;
defm EXPVEC : UnaryIntMCf32<IL_OP_EXP_VEC, int_AMDIL_exp_vec>;
defm SQRTVEC : UnaryIntMCf32<IL_OP_SQRT_VEC, int_AMDIL_sqrt_vec>;
defm COSVEC : UnaryIntMCf32<IL_OP_COS_VEC, int_AMDIL_cos_vec>;
defm SINVEC : UnaryIntMCf32<IL_OP_SIN_VEC, int_AMDIL_sin_vec>;
defm LOGVEC : UnaryIntMCf32<IL_OP_LOG_VEC, int_AMDIL_log_vec>;
defm RSQVEC : UnaryIntMCf32<IL_OP_RSQ_VEC, int_AMDIL_rsq_vec>;
defm EXN : UnaryIntMCf32Scalar<IL_OP_EXN, int_AMDIL_exn>;
defm SIGN : UnaryIntMCf32<IL_OP_SGN, int_AMDIL_sign>;
defm LENGTH : UnaryIntMCf32<IL_OP_LEN, int_AMDIL_length>;
defm POW : BinaryIntMCf32<IL_OP_POW, int_AMDIL_pow>;


let ieee = 1 in {
  let isCommutable = 1 in {
    defm MIN : BinaryIntMCf32<IL_OP_MIN, int_AMDIL_min>;
    defm MAX : BinaryIntMCf32<IL_OP_MAX, int_AMDIL_max>;
  }

  defm MAD : TernaryPatMCf32<IL_OP_MAD, int_AMDIL_mad>;
  defm MOD : BinaryOpMCf32<IL_OP_MOD, frem>;
}

let zeroop = 1 in {
  defm LN  : UnaryIntMCf32Scalar<IL_OP_LN, int_AMDIL_ln>;
  defm LOG : UnaryIntMCf32Scalar<IL_OP_LOG, int_AMDIL_log>;
  defm RSQ : UnaryIntMCf32Scalar<IL_OP_RSQ, int_AMDIL_rsq>;
  defm DIV_INT : BinaryIntMCf32<IL_OP_DIV, int_AMDIL_div>;
  defm DIV_PRECISE : BinaryIntMCf32<IL_OP_DIV_PRECISE, int_AMDIL_div_precise>;
  defm DIV : BinaryOpMCf32<IL_OP_DIV, fdiv>;
}


defm CLAMP : TernaryPatMCf32<IL_OP_CLAMP, int_AMDIL_clamp>;
defm FMA : TernaryPatMCf32<IL_OP_FMA, int_AMDIL_fma>;
defm LERP : TernaryPatMCf32<IL_OP_LERP, int_AMDIL_lerp>;
defm SUB : BinaryOpMCf32<IL_OP_SUB, fsub>;
defm FABS : UnaryOpMCf32<IL_OP_ABS, fabs>;
defm NEARBY : UnaryOpMCf32<IL_OP_ROUND_NEAR, fnearbyint>;
defm TRUNC : UnaryOpMCf32<IL_OP_ROUND_ZERO, ftrunc>;
defm CEIL : UnaryOpMCf32<IL_OP_ROUND_POS_INF, fceil>;
defm FLOOR : UnaryOpMCf32<IL_OP_ROUND_NEG_INF, ffloor>;

let ieee = 1 in {
  defm DP2ADD : TernaryIntMCFull<IL_OP_DP2_ADD, int_AMDIL_dp2_add,
                                 f32, GPR_32, v2f32, GPRV2I32,
                                 v2f32imm, fpimm, v2f32,
                                 GPRV2I32, v2f32imm, fpimm,
                                 f32, GPR_32, f32imm, fpimm>;
  defm DP2 : BinaryIntMCFull<IL_OP_DP2, int_AMDIL_dp2,
                             f32, GPR_32, v2f32, GPRV2I32, v2f32imm,
                             fpimm, v2f32, GPRV2I32, v2f32imm, fpimm>;
  defm DP3 : BinaryIntMCFull<IL_OP_DP3, int_AMDIL_dp3,
                             f32, GPR_32, v4f32, GPRV4I32,
                             v4f32imm, fpimm, v4f32, GPRV4I32,
                             v4f32imm, fpimm>;
  defm DP4 : BinaryIntMCFull<IL_OP_DP4, int_AMDIL_dp4,
                             f32, GPR_32, v4f32, GPRV4I32, v4f32imm,
                             fpimm, v4f32, GPRV4I32, v4f32imm, fpimm>;
  defm FTZ : FTZMC<IL_OP_MUL_IEEE, int_AMDIL_ftz>;
}


defm UNPACK_B0 : IntrConvertI32TOF32<IL_OP_UNPACK_0, int_AMDIL_media_unpack_byte_0>;
defm UNPACK_B1 : IntrConvertI32TOF32<IL_OP_UNPACK_1, int_AMDIL_media_unpack_byte_1>;
defm UNPACK_B2 : IntrConvertI32TOF32<IL_OP_UNPACK_2, int_AMDIL_media_unpack_byte_2>;
defm UNPACK_B3 : IntrConvertI32TOF32<IL_OP_UNPACK_3, int_AMDIL_media_unpack_byte_3>;
defm FTOI_FLR : IntrConvertF32TOI32<IL_OP_FTOI_FLR, int_AMDIL_convert_f32_i32_flr>;
defm FTOI_RPI : IntrConvertF32TOI32<IL_OP_FTOI_RPI, int_AMDIL_convert_f32_i32_rpi>;
defm HTOF : IntrConvertF16TOF32<IL_OP_F16_TO_F32, int_AMDIL_convert_f16_f32>;
defm FTOH : IntrConvertF32TOF16<IL_OP_F32_TO_F16, int_AMDIL_convert_f32_f16>;
defm FTOH_NEAR : IntrConvertF32TOF16<IL_OP_F32_TO_F16_NEAR, int_AMDIL_convert_f32_f16_near>;
defm FTOH_NEG_INF : IntrConvertF32TOF16<IL_OP_F32_TO_F16_NEG_INF, int_AMDIL_convert_f32_f16_neg_inf>;
defm FTOH_PLUS_INF : IntrConvertF32TOF16<IL_OP_F32_TO_F16_PLUS_INF, int_AMDIL_convert_f32_f16_plus_inf>;
defm NEG : UnaryNegMCf32<IL_OP_MOV, fneg>;
defm SUB : BinaryNegMCf64<IL_OP_D_ADD, fsub>;
defm NEG : UnaryNegMCf64<IL_OP_MOV, fneg>;
defm NOT : NotMCReg<IL_OP_I_NOT, not, vnot>;

def vineg : PatFrag<(ops node:$in), (sub immAllZerosV, node:$in)>;

defm NEG : UnaryPatMCi8Scalar<IL_OP_I_NEGATE, ineg>;
defm NEG : UnaryPatMCi8Vector<IL_OP_I_NEGATE, vineg>;
defm NEG : UnaryPatMCi16Scalar<IL_OP_I_NEGATE, ineg>;
defm NEG : UnaryPatMCi16Vector<IL_OP_I_NEGATE, vineg>;
defm NEG : UnaryPatMCi32Scalar<IL_OP_I_NEGATE, ineg>;
defm NEG : UnaryPatMCi32Vector<IL_OP_I_NEGATE, vineg>;
defm NEG : UnaryPatMCi64Scalar<IL_OP_I64_NEGATE, ineg>;
defm NEG : UnaryPatMCi64Vector<IL_OP_I64_NEGATE, vineg>;



let isCommutable = 1 in {
  defm MIN  : BinaryIntMCf64<IL_OP_D_MIN, int_AMDIL_min>;
  defm MAX  : BinaryIntMCf64<IL_OP_D_MAX, int_AMDIL_max>;
}

defm DIV  : BinaryIntMCf64<IL_OP_D_DIV, int_AMDIL_div>;
defm MAD  : TernaryPatMCf64<IL_OP_D_MAD, int_AMDIL_mad>;
defm DFMA : TernaryPatMCf64<IL_OP_D_MAD, int_AMDIL_fma>;
defm FRAC : UnaryIntMCf64<IL_OP_D_FRC, int_AMDIL_fraction>;
defm SQRT : UnaryIntMCf64<IL_OP_D_SQRT, int_AMDIL_sqrt>;
defm RSQ  : UnaryIntMCf64Scalar<IL_OP_D_RSQ, int_AMDIL_rsq>;
defm RCP  : UnaryIntMCf64Scalar<IL_OP_D_RCP, int_AMDIL_drcp>;

defm DABS : UnaryOpMCf64<IL_OP_D_ABS, fabs>;
defm ABS : UnaryIntMCf64<IL_OP_D_ABS, int_AMDIL_fabs>;

defm FREXPf64 : UnaryIntMCFull<IL_OP_D_FREXP, int_AMDIL_frexp_f64,
                               v2i64, GPRV2I64, f64, GPR_64,
                               f64imm, fpimm>;
defm LDEXP : BinaryIntMCDDI<IL_OP_D_LDEXP, int_AMDIL_ldexp>;

def MACRO__sdiv_i16 : BinaryMacro<GPRI16, GPRI16, GPRI16, sdiv>;
def MACRO__sdiv_i32 : BinaryMacro<GPR_32, GPR_32, GPR_32, sdiv>;
def MACRO__udiv_i8  : BinaryMacro< GPRI8, GPRI8, GPRI8, udiv>;
def MACRO__udiv_i16 : BinaryMacro<GPRI16, GPRI16, GPRI16, udiv>;
def MACRO__udiv_i32 : BinaryMacro<GPR_32, GPR_32, GPR_32, udiv>;
def MACRO__smod_i8  : BinaryMacro< GPRI8, GPRI8, GPRI8, srem>;
def MACRO__smod_i16 : BinaryMacro<GPRI16, GPRI16, GPRI16, srem>;
def MACRO__smod_i32 : BinaryMacro<GPR_32, GPR_32, GPR_32, srem>;
def MACRO__umod_i8  : BinaryMacro< GPRI8, GPRI8, GPRI8, urem>;
def MACRO__umod_i16 : BinaryMacro<GPRI16, GPRI16, GPRI16, urem>;
def MACRO__umod_i32 : BinaryMacro<GPR_32, GPR_32, GPR_32, urem>;

let Predicates = [HasHWDDiv] in {
  def MACRO__ddiv_f64_fma: BinaryMacro<GPR_64, GPR_64, GPR_64, fdiv>;
}

def MACRO__ftol_i64  : UnaryMacro<GPR_64, GPR_32, fp_to_sint>;
def MACRO__ultof_f32 : UnaryMacro<GPR_32, GPR_64, uint_to_fp>;
def MACRO__ltof_f32  : UnaryMacro<GPR_32, GPR_64, sint_to_fp>;

let Predicates = [HasSW64Mul] in {
  def MACRO__mul_i64   : BinaryMacro<GPR_64, GPR_64, GPR_64, mul>;
  def MACRO__mul_v2i64 : BinaryMacro<GPRV2I64, GPRV2I64, GPRV2I64, mul>;
}

let Predicates = [HasSW64DivMod] in {
  def MACRO__sdiv_i64 : BinaryMacro<GPR_64, GPR_64, GPR_64, sdiv>;
  def MACRO__udiv_i64 : BinaryMacro<GPR_64, GPR_64, GPR_64, udiv>;
  def MACRO__smod_i64 : BinaryMacro<GPR_64, GPR_64, GPR_64, srem>;
  def MACRO__umod_i64 : BinaryMacro<GPR_64, GPR_64, GPR_64, urem>;
}

let Predicates = [HasHW64DivMod] in {
  defm SDIV : BinaryOpMCi64<IL_OP_I64_DIV, sdiv>;
  defm UDIV : BinaryOpMCi64<IL_OP_U64_DIV, udiv>;
  defm SMOD : BinaryOpMCi64<IL_OP_I64_MOD, srem>;
  defm UMOD : BinaryOpMCi64<IL_OP_U64_MOD, urem>;
}

let Predicates = [HasHW64Mul] in {
  defm SMUL : BinaryOpMCi64<IL_OP_I64_MUL, mul>;
  defm UMUL : BinaryOpMCi64<IL_OP_U64_MUL, IL_umul>;
}


//===---------------------------------------------------------------------===//
// Comparison Instructions
// ===---------------------------------------------------------------------===//

// Patterns that match ordered and undefined NAN comparisons to
// ordered compares.
def cond_seteq_setoeq : PatLeaf <(cond), [{
  return N->get() == ISD::SETEQ || N->get() == ISD::SETOEQ;
}]>;

def cond_setgt_setogt : PatLeaf <(cond), [{
  return N->get() == ISD::SETGT || N->get() == ISD::SETOGT;
}]>;

def cond_setge_setoge : PatLeaf <(cond), [{
  return N->get() == ISD::SETGE || N->get() == ISD::SETOGE;
}]>;

def cond_setlt_setolt : PatLeaf <(cond), [{
  return N->get() == ISD::SETLT || N->get() == ISD::SETOLT;
}]>;
def cond_setle_setole : PatLeaf <(cond), [{
  return N->get() == ISD::SETLE || N->get() == ISD::SETOLE;
}]>;

def cond_setne_setune : PatLeaf <(cond), [{
  return N->get() == ISD::SETNE ||
         N->get() == ISD::SETUNE;
}]>;

def cond_setune : PatLeaf <(cond), [{
  return N->get() == ISD::SETUNE;
}]>;

defm EQ : BinaryCmpf32<IL_OP_EQ, cond_seteq_setoeq>;
//defm GT : BinaryCmpf32<IL_OP_GT, cond_setgt_setogt>;
defm GE : BinaryCmpf32<IL_OP_GE, cond_setge_setoge>;
defm LT : BinaryCmpf32<IL_OP_LT, cond_setlt_setolt>;
//defm LE : BinaryCmpf32<IL_OP_LE, cond_setle_setole>;
defm NE : BinaryCmpf32<IL_OP_NE, cond_setne_setune>;

defm DEQ : BinaryCmpf64<IL_OP_D_EQ, cond_seteq_setoeq>;
// There is no dgt.
defm DGE : BinaryCmpf64<IL_OP_D_GE, cond_setge_setoge>;
defm DLT : BinaryCmpf64<IL_OP_D_LT, cond_setlt_setolt>;
// There is no dle.
defm DNE : BinaryCmpf64<IL_OP_D_NE, cond_setne_setune>;


// Only cond codes used by integers.
def cond_seteq : PatLeaf <(cond), [{
  return N->get() == ISD::SETEQ;
}]>;

def cond_setne : PatLeaf <(cond), [{
  return N->get() == ISD::SETNE;
}]>;

def cond_setslt : PatLeaf <(cond), [{
  return N->get() == ISD::SETLT;
}]>;

def cond_setsle : PatLeaf <(cond), [{
  return N->get() == ISD::SETLE;
}]>;

def cond_setsgt : PatLeaf <(cond), [{
  return N->get() == ISD::SETGT;
}]>;

def cond_setsge : PatLeaf <(cond), [{
  return N->get() == ISD::SETGE;
}]>;

def cond_setult : PatLeaf <(cond), [{
  return N->get() == ISD::SETULT;
}]>;

def cond_setule : PatLeaf <(cond), [{
  return N->get() == ISD::SETULE;
}]>;

def cond_setugt : PatLeaf <(cond), [{
  return N->get() == ISD::SETUGT;
}]>;

def cond_setuge : PatLeaf <(cond), [{
  return N->get() == ISD::SETUGE;
}]>;


defm IEQ : BinaryCmpi32<IL_OP_I_EQ, cond_seteq>,
           BinaryCmpi64<IL_OP_I64_EQ, cond_seteq>;
defm INE : BinaryCmpi32<IL_OP_I_NE, cond_setne>,
           BinaryCmpi64<IL_OP_I64_NE, cond_setne>;
defm ILT : BinaryCmpi32<IL_OP_I_LT, cond_setslt>,
           BinaryCmpi64<IL_OP_I64_LT, cond_setslt>;
// There is no ile or igt.
defm IGE : BinaryCmpi32<IL_OP_I_GE, cond_setsge>,
           BinaryCmpi64<IL_OP_I64_GE, cond_setsge>;
defm ULT : BinaryCmpi32<IL_OP_U_LT, cond_setult>,
           BinaryCmpi64<IL_OP_U64_LT, cond_setult>;
// There is no ule or ugt.
defm UGE : BinaryCmpi32<IL_OP_U_GE, cond_setuge>,
           BinaryCmpi64<IL_OP_U64_GE, cond_setuge>;

//===-------------------------------------------------------------===//
// Scalar ==> Scalar conversion functions
//===-------------------------------------------------------------===//
defm FTOD      : ConvertOpMC<IL_OP_F_2_D, fextend, f64,  GPR_64, f32, GPR_32, f32imm, fpimm>;
defm DTOF      : ConvertOpMC<IL_OP_D_2_F, IL_d2f, f32, GPR_32, f64, GPR_64, f64imm, fpimm>;
defm FTOIi32   : ConvertOpMC<IL_OP_FTOI, fp_to_sint, i32, GPR_32, f32, GPR_32, f32imm, fpimm>;
defm FTOIv2i32 : ConvertOpMC<IL_OP_FTOI, fp_to_sint, v2i32, GPRV2I32, v2f32, GPRV2I32, v2f32imm, fpimm>;
defm FTOIv4i32 : ConvertOpMC<IL_OP_FTOI, fp_to_sint, v4i32, GPRV4I32, v4f32, GPRV4I32, v4f32imm, fpimm>;
defm ITOFf32   : ConvertOpMC<IL_OP_ITOF, sint_to_fp, f32, GPR_32, i32, GPR_32, i32imm, imm>;
defm ITOFv2f32 : ConvertOpMC<IL_OP_ITOF, sint_to_fp, v2f32, GPRV2I32, v2i32, GPRV2I32, v2i32imm, imm>;
defm ITOFv4f32 : ConvertOpMC<IL_OP_ITOF, sint_to_fp, v4f32, GPRV4I32, v4i32, GPRV4I32, v4i32imm, imm>;
defm FTOUi32   : ConvertOpMC<IL_OP_FTOU, fp_to_uint, i32, GPR_32, f32, GPR_32, f32imm, fpimm>;
defm FTOUv2i32 : ConvertOpMC<IL_OP_FTOU, fp_to_uint, v2i32, GPRV2I32, v2f32, GPRV2I32, v2f32imm, fpimm>;
defm FTOUv4i32 : ConvertOpMC<IL_OP_FTOU, fp_to_uint, v4i32, GPRV4I32, v4f32, GPRV4I32, v4f32imm, fpimm>;
defm UTOFf32   : ConvertOpMC<IL_OP_UTOF, uint_to_fp, f32, GPR_32, i32, GPR_32, i32imm, imm>;
defm UTOFv2f32 : ConvertOpMC<IL_OP_UTOF, uint_to_fp, v2f32, GPRV2I32, v2i32, GPRV2I32, v2i32imm, imm>;
defm UTOFv4f32 : ConvertOpMC<IL_OP_UTOF, uint_to_fp, v4f32, GPRV4I32, v4i32, GPRV4I32, v4i32imm, imm>;
defm DTOI      : ConvertOpMC<IL_OP_DTOI, fp_to_sint, i32, GPR_32, f64, GPR_64, f64imm, fpimm>;
defm ITOD      : ConvertOpMC<IL_OP_ITOD, sint_to_fp, f64, GPR_64, i32, GPR_32, i32imm, imm>;
defm DTOU      : ConvertOpMC<IL_OP_DTOU, fp_to_uint, i32, GPR_32, f64, GPR_64, f64imm, fpimm>;
defm UTOD      : ConvertOpMC<IL_OP_UTOD, uint_to_fp, f64, GPR_64, i32, GPR_32, i32imm, imm>;


let swizzle = 2 in {
  def DHIf64r : ILFormat<IL_OP_MOV,  (outs GPR_32:$dst),
                (ins GPR_64:$src),
                !strconcat(IL_OP_MOV.Text, " $dst, $src"),
                [(set GPR_32:$dst, (IL_dcomphi GPR_64:$src))]>;
  def DHIv2f64r : ILFormat<IL_OP_MOV,  (outs GPRV2I32:$dst),
                (ins GPRV2I64:$src),
                !strconcat(IL_OP_MOV.Text, " $dst, $src"),
                [(set GPRV2I32:$dst, (IL_dcomphi2 GPRV2I64:$src))]>;
  def DLOf64r : ILFormat<IL_OP_MOV,  (outs GPR_32:$dst),
                (ins GPR_64:$src),
                !strconcat(IL_OP_MOV.Text, " $dst, $src"),
                [(set GPR_32:$dst, (IL_dcomplo GPR_64:$src))]>;
  def DLOv2f64r : ILFormat<IL_OP_MOV,  (outs GPRV2I32:$dst),
                (ins GPRV2I64:$src),
                !strconcat(IL_OP_MOV.Text, " $dst, $src"),
                [(set GPRV2I32:$dst, (IL_dcomplo2 GPRV2I64:$src))]>;
}

let swizzle = 6 in {
  def DCREATEf64rr : ILFormat<IL_OP_I_ADD, (outs GPR_64:$dst),
                (ins GPR_32:$src0, GPR_32:$src1),
                !strconcat(IL_OP_I_ADD.Text, " $dst, $src0, $src1"),
                [(set GPR_64:$dst, (IL_dcreate GPR_32:$src0, GPR_32:$src1))]>;
  def DCREATEv2f64rr : ILFormat<IL_OP_I_ADD, (outs GPRV2I64:$dst),
                (ins GPRV2I32:$src0, GPRV2I32:$src1),
                !strconcat(IL_OP_I_ADD.Text, " $dst, $src0, $src1"),
                [(set GPRV2I64:$dst,
                    (IL_dcreate2 GPRV2I32:$src0, GPRV2I32:$src1))]>;
}
let swizzle = 2 in {
  def LHIi64r : ILFormat<IL_OP_MOV,  (outs GPR_32:$dst),
                (ins GPR_64:$src),
                !strconcat(IL_OP_MOV.Text, " $dst, $src"),
                [(set GPR_32:$dst, (IL_lcomphi GPR_64:$src))]>;
  def LHIv2i64r : ILFormat<IL_OP_MOV,  (outs GPRV2I32:$dst),
                (ins GPRV2I64:$src),
                !strconcat(IL_OP_MOV.Text, " $dst, $src"),
                [(set GPRV2I32:$dst, (IL_lcomphi2 GPRV2I64:$src))]>;
  def LLOi64r : ILFormat<IL_OP_MOV,  (outs GPR_32:$dst),
                (ins GPR_64:$src),
                !strconcat(IL_OP_MOV.Text, " $dst, $src"),
                [(set GPR_32:$dst, (IL_lcomplo GPR_64:$src))]>;
  def LLOv2i64r : ILFormat<IL_OP_MOV,  (outs GPRV2I32:$dst),
                (ins GPRV2I64:$src),
                !strconcat(IL_OP_MOV.Text, " $dst, $src"),
                [(set GPRV2I32:$dst, (IL_lcomplo2 GPRV2I64:$src))]>;
}

let swizzle = 6 in {
  let swizzle = 1 in {
    def HILO_BITORv4i16rr : ILFormat<IL_OP_I_OR, (outs GPR_32:$dst),
                (ins GPR_32:$src, GPR_32:$src2),
                !strconcat(IL_OP_I_OR.Text, " $dst, $src, $src2"), []>;
    def HILO_BITORv2i32rr : ILFormat<IL_OP_I_OR, (outs GPR_32:$dst),
                (ins GPR_32:$src, GPR_32:$src2),
                !strconcat(IL_OP_I_OR.Text, " $dst, $src, $src2"), []>;
  }
  def HILO_BITORv2i64rr : ILFormat<IL_OP_I_OR, (outs GPR_64:$dst),
                (ins GPR_64:$src, GPR_64:$src2),
                !strconcat(IL_OP_I_OR.Text, " $dst, $src, $src2"), []>;
}

let swizzle = 6 in {
  def LCREATEi64rr : ILFormat<IL_OP_I_ADD, (outs GPR_64:$dst),
                  (ins GPR_32:$src0, GPR_32:$src1),
                  !strconcat(IL_OP_I_ADD.Text, " $dst, $src0, $src1"),
                  [(set GPR_64:$dst, (IL_lcreate GPR_32:$src0, GPR_32:$src1))]>;
  // This opcode has custom swizzle pattern encoded in Swizzle Encoder
  def LCREATEv2i64rr : ILFormat<IL_OP_I_ADD, (outs GPRV2I64:$dst),
                  (ins GPRV2I32:$src0, GPRV2I32:$src1),
                  !strconcat(IL_OP_I_ADD.Text, " $dst, $src0, $src1"),
                  [(set GPRV2I64:$dst,
                    (IL_lcreate2 GPRV2I32:$src0, GPRV2I32:$src1))]>;
}
//===---------------------------------------------------------------------===//
// Scalar ==> Vector conversion functions
//===---------------------------------------------------------------------===//
// This opcode has custom swizzle pattern encoded in Swizzle Encoder
defm VCREATE : VectorCreate<IL_OP_MOV, IL_vbuild>;


//===---------------------------------------------------------------------===//
// Vector ==> Scalar conversion functions
//===---------------------------------------------------------------------===//

// This opcode has custom swizzle pattern encoded in Swizzle Encoder
defm VEXTRACT : VectorExtract<IL_vextract>;

//===---------------------------------------------------------------------===//
// Vector ==> Vector conversion functions
//===---------------------------------------------------------------------===//
// This opcode has custom swizzle pattern encoded in Swizzle Encoder
defm VINSERT : VectorInsert<IL_vinsert>;


// This opcode has custom swizzle pattern encoded in Swizzle Encoder
defm VCONCAT : VectorConcat<IL_vconcat>;


// TODO: Use COPY_TO_REGCLASS?
class BitConvert<ValueType dt, ValueType st, RegisterClass rc> : Pat <
  (dt (bitconvert (st rc:$src0))),
  (dt rc:$src0)
>;

//def : BitConvert<f16, i16, GPRI16>;
//def : BitConvert<i16, f16, GPRI16>;
def : BitConvert<f32, i32, GPR_32>;
def : BitConvert<i32, f32, GPR_32>;
def : BitConvert<f64, i64, GPR_64>;
def : BitConvert<i64, f64, GPR_64>;
def : BitConvert<v2f64, v2i64, GPRV2I64>;
def : BitConvert<v2i64, v2f64, GPRV2I64>;
def : BitConvert<v2f32, v2i32, GPRV2I32>;
def : BitConvert<v2i32, v2f32, GPRV2I32>;
def : BitConvert<v4f32, v4i32, GPRV4I32>;
def : BitConvert<v4i32, v4f32, GPRV4I32>;

// TODO: Remove this. Use BitConvert when the GPRV2I32 and GPR_64 are
// no longer distinct register classes.
class BitConvertCopy<ValueType dt, ValueType st,
                     RegisterClass destrc, RegisterClass srcrc> : Pat <
  (dt (bitconvert (st srcrc:$src0))),
  (dt (COPY_TO_REGCLASS srcrc:$src0, destrc))
>;

def : BitConvertCopy<v2i32, i64, GPRV2I32, GPR_64>;
def : BitConvertCopy<i64, v2i32, GPR_64, GPRV2I32>;
def : BitConvertCopy<v2i32, f64, GPRV2I32, GPR_64>;
def : BitConvertCopy<f64, v2i32, GPR_64, GPRV2I32>;
def : BitConvertCopy<v2f32, i64, GPRV2I32, GPR_64>;
def : BitConvertCopy<i64, v2f32, GPR_64, GPRV2I32>;
def : BitConvertCopy<v2i64, v4i32, GPRV2I64, GPRV4I32>;
def : BitConvertCopy<v4i32, v2i64, GPRV4I32, GPRV2I64>;
def : BitConvertCopy<v2i64, v4f32, GPRV2I64, GPRV4I32>;
def : BitConvertCopy<v4f32, v2i64, GPRV4I32, GPRV2I64>;
def : BitConvertCopy<v2f64, v4f32, GPRV2I64, GPRV4I32>;
def : BitConvertCopy<v4f32, v2f64, GPRV4I32, GPRV2I64>;
def : BitConvertCopy<v2f64, v4i32, GPRV2I64, GPRV4I32>;
def : BitConvertCopy<v4i32, v2f64, GPRV4I32, GPRV2I64>;
def : BitConvertCopy<f64, v4i16, GPR_64, GPRV4I16>;
def : BitConvertCopy<v4i16, f64, GPRV4I16, GPR_64>;
def : BitConvertCopy<i64, v4i16, GPR_64, GPRV4I16>;
def : BitConvertCopy<v4i16, i64, GPRV4I16, GPR_64>;
def : BitConvertCopy<f32, v4i8, GPR_32, GPRV4I8>;
def : BitConvertCopy<v4i8, f32, GPRV4I8, GPR_32>;
def : BitConvertCopy<i32, v4i8, GPR_32, GPRV4I8>;
def : BitConvertCopy<v4i8, i32, GPRV4I8, GPR_32>;
def : BitConvertCopy<i32, v2i16, GPR_32, GPRV2I16>;
def : BitConvertCopy<v2i16, i32, GPRV2I16, GPR_32>;
def : BitConvertCopy<f32, v2i16, GPR_32, GPRV2I16>;
def : BitConvertCopy<v2i16, f32, GPRV2I16, GPR_32>;

//===-------------------------------------------------------------===//
// Custom Inserter for Branches and returns, this eventually will be a
// seperate pass
//===-------------------------------------------------------------===//
let isTerminator = 1 in {
  defm BRANCH : BranchMC<IL_brcond>;
}

//===-------------------------------------------------------------===//
// return instructions
//===-------------------------------------------------------------===//
let isTerminator = 1, isReturn = 1, isBarrier = 1, hasCtrlDep = 1 in {
  let swizzle = 63 in {
    def RETURN : ILFormat<IL_OP_RET,(outs), (ins variable_ops), IL_OP_RET.Text, []>;
    def RETDYN : ILFormat<IL_OP_RET_DYN, (outs), (ins variable_ops), IL_OP_RET_DYN.Text, [(IL_retflag)]>;
  }
}
//===---------------------------------------------------------------------===//
// Lower and raise the stack x amount
//===---------------------------------------------------------------------===//
let Defs = [SP], Uses = [SP], hasSideEffects = 1 in {
  def ADJCALLSTACKDOWN : ILFormat<IL_OP_I_ADD, (outs), (ins i32imm:$amt),
    !strconcat(IL_OP_I_ADD.Text, " x0[0], x0[0], $amt; begin of call sequence $amt"),
    [(IL_callseq_start timm:$amt)]>;
  def ADJCALLSTACKUP : ILFormat<IL_OP_SUB, (outs), (ins i32imm:$amt1,
    i32imm:$amt2),
    !strconcat(IL_OP_SUB.Text, " x0[0], x0[0], $amt1; end of call sequence $amt1 $amt2"),
    [(IL_callseq_end timm:$amt1, timm:$amt2)]>;
}
//===---------------------------------------------------------------------===//
// Handle a function call
//===---------------------------------------------------------------------===//
let isCall = 1 in {
    let Defs = [SP], Uses = [SP] in {
      def CALL : ILFormat<IL_OP_CALL, (outs),
                          (ins calltarget:$dst, variable_ops),
                          !strconcat(IL_OP_CALL.Text, "($dst) "), []>;
    }
}


//===---------------------------------------------------------------------===//
// Flow and Program control Instructions
//===---------------------------------------------------------------------===//
let isTerminator = 1 in {
  def SWITCH : ILFormat<IL_OP_SWITCH, (outs), (ins GPR_32:$src),
                       !strconcat(IL_OP_SWITCH.Text, " $src"), []>;
  def CASE : ILFormat<IL_OP_CASE, (outs), (ins GPR_32:$src),
      !strconcat(IL_OP_CASE.Text, " $src"), []>;
  def BREAK : ILFormat<IL_OP_BREAK, (outs), (ins),
      IL_OP_BREAK.Text, []>;
  def CONTINUE : ILFormat<IL_OP_CONTINUE, (outs), (ins),
      IL_OP_CONTINUE.Text, []>;
  def DEFAULT : ILFormat<IL_OP_DEFAULT, (outs), (ins),
      IL_OP_DEFAULT.Text, []>;
  def ELSE : ILFormat<IL_OP_ELSE, (outs), (ins),
      IL_OP_ELSE.Text, []>;
  def ENDSWITCH : ILFormat<IL_OP_ENDSWITCH, (outs), (ins),
      IL_OP_ENDSWITCH.Text, []>;
  def ENDMAIN : ILFormat<IL_OP_ENDMAIN, (outs), (ins),
      IL_OP_ENDMAIN.Text, []>;
  def END : ILFormat<IL_OP_END, (outs), (ins),
      IL_OP_END.Text, []>;
  def ENDFUNC : ILFormat<IL_OP_ENDFUNC, (outs), (ins),
      IL_OP_ENDFUNC.Text, []>;
  def ENDIF : ILFormat<IL_OP_ENDIF, (outs), (ins),
      IL_OP_ENDIF.Text, []>;
  def WHILELOOP : ILFormat<IL_OP_WHILE, (outs), (ins),
      IL_OP_WHILE.Text, []>;
  def ENDLOOP : ILFormat<IL_OP_ENDLOOP, (outs), (ins),
      IL_OP_ENDLOOP.Text, []>;
  def FUNC : ILFormat<IL_OP_FUNC, (outs), (ins),
      IL_OP_FUNC.Text, []>;
  // This opcode has custom swizzle pattern encoded in Swizzle Encoder
  defm IF_LOGICALNZ : BranchInstr<IL_OP_IF_LOGICALNZ>;
  // This opcode has custom swizzle pattern encoded in Swizzle Encoder
  defm IF_LOGICALZ : BranchInstr<IL_OP_IF_LOGICALZ>;
  // This opcode has custom swizzle pattern encoded in Swizzle Encoder
  defm BREAK_LOGICALNZ : BranchInstr<IL_OP_BREAK_LOGICALNZ>;
  // This opcode has custom swizzle pattern encoded in Swizzle Encoder
  defm BREAK_LOGICALZ : BranchInstr<IL_OP_BREAK_LOGICALZ>;
  // This opcode has custom swizzle pattern encoded in Swizzle Encoder
  defm CONTINUE_LOGICALNZ : BranchInstr<IL_OP_CONTINUE_LOGICALNZ>;
  // This opcode has custom swizzle pattern encoded in Swizzle Encoder
  defm CONTINUE_LOGICALZ : BranchInstr<IL_OP_CONTINUE_LOGICALZ>;
  defm IFC : BranchInstr2<IL_OP_IFC>;
  defm BREAKC : BranchInstr2<IL_OP_BREAKC>;
  defm CONTINUEC : BranchInstr2<IL_OP_CONTINUEC>;
}

let isTerminator = 1, isBarrier = 1, hasCtrlDep = 1 in {
  def TRAP : ILFormat<IL_OP_NOP, (outs), (ins), IL_OP_NOP.Text, [(trap)]>;
}

//===---------------------------------------------------------------------===//
//----------------- Work Item Functions - OpenCL 6.11.1 ---------------------//
//===---------------------------------------------------------------------===//
let isCall = 0, isAsCheapAsAMove = 1, neverHasSideEffects = 1 in {
  def GET_WORK_DIM : ILFormat<IL_OP_MOV, (outs GPR_X_32:$dst), (ins),
      !strconcat(IL_OP_MOV.Text, " $dst, cb0[0].w"),
      [(set GPR_X_32:$dst, (int_AMDIL_get_work_dim))]>;

  def GET_GLOBAL_ID : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
      (ins), !strconcat(IL_OP_MOV.Text, " $dst, x0[1].xyz0"),
      [(set GPRV4I32:$dst, (int_AMDIL_get_global_id))]>;

  def GET_LOCAL_ID : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
      (ins), !strconcat(IL_OP_MOV.Text, " $dst, vTidInGrp0.xyz"),
      [(set GPRV4I32:$dst, (int_AMDIL_get_local_id))]>;

  def GET_GROUP_ID : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
      (ins), !strconcat(IL_OP_MOV.Text, " $dst, x0[2].xyz0"),
      [(set GPRV4I32:$dst, (int_AMDIL_get_group_id))]>;

  def GET_GLOBAL_SIZE : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
      (ins), !strconcat(IL_OP_MOV.Text, " $dst, cb0[0].xyz0"),
      [(set GPRV4I32:$dst, (int_AMDIL_get_global_size))]>;

  def GET_LOCAL_SIZE : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
      (ins), !strconcat(IL_OP_MOV.Text, " $dst, cb0[1].xyz0"),
      [(set GPRV4I32:$dst, (int_AMDIL_get_local_size))]>;

  def GET_NUM_GROUPS : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
      (ins), !strconcat(IL_OP_MOV.Text, " $dst, cb0[2].xyz0"),
      [(set GPRV4I32:$dst, (int_AMDIL_get_num_groups))]>;

  def GET_REGION_NUM_GROUPS : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
      (ins), !strconcat(IL_OP_MOV.Text, " $dst, cb0[12].xyz0"),
      [(set GPRV4I32:$dst, (int_AMDIL_get_region_num_groups))]>;

  def GET_SW_GWS_OFFSET : ILFormat<IL_OP_MOV, (outs GPR_32:$dst),
      (ins), !strconcat(IL_OP_MOV.Text, " $dst, cb0[12].w"),
      [(set GPR_32:$dst, (int_AMDIL_get_sw_gws_offset))]>;

  let Predicates = [Has32BitPtr] in {
    def GET_GLOBAL_OFFSET : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins), !strconcat(IL_OP_MOV.Text, " $dst, cb0[9].xyz0"),
        [(set GPRV4I32:$dst, (int_AMDIL_get_global_offset))]>;
  }

  let Predicates = [Has64BitPtr] in {
    def GET_GLOBAL_OFFSET64 : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins), !strconcat(IL_OP_MOV.Text, " $dst, cb0[9].yzw0"),
        [(set GPRV4I32:$dst, (int_AMDIL_get_global_offset))]>;
  }

  def GET_MAX_SEMAPHORE_SIZE : ILFormat<IL_OP_MOV, (outs GPR_X_32:$dst),
      (ins), !strconcat(IL_OP_MOV.Text, " $dst, cb0[10].w"),
      [(set GPR_X_32:$dst, (int_AMDIL_max_semaphore_size))]>;
}
//===---------------------------------------------------------------------===//
//------------- Synchronization Functions - OpenCL 6.11.9 -------------------//
//===---------------------------------------------------------------------===//
let isCall = 1, hasSideEffects = 1, isNotDuplicable = 1 in {
  // The 'S' suffix is for the '_gws' flag.
  // The 'L' suffix is for the '_lds' flag.
  // The 'M' suffix is for the '_memory' flag.
  // The 'G' suffix is for the '_gds' flag.
  // The 'RO' suffix is for the '_read_only' flag.
  // The 'WO' suffix is for the '_write_only' flag.
  let gws = 1 in {
    defm FENCE_S : Fence<IL_OP_FENCE_S, int_AMDIL_gws>;
    defm FENCE_LS : Fence<IL_OP_FENCE_LS, int_AMDIL_gws_local>;
    defm FENCE_MS : Fence<IL_OP_FENCE_MS, int_AMDIL_gws_global>;
    defm FENCE_GS : Fence<IL_OP_FENCE_GS, int_AMDIL_gws_region>;
    defm FENCE_LMS : Fence<IL_OP_FENCE_LMS, int_AMDIL_gws_global_local>;
    defm FENCE_MGS : Fence<IL_OP_FENCE_MGS, int_AMDIL_gws_global_region>;
    defm FENCE_LGS : Fence<IL_OP_FENCE_LGS, int_AMDIL_gws_local_region>;
  }

  defm FENCE : Fence<IL_OP_FENCE_LMG, int_AMDIL_fence>;
  defm FENCE_L : Fence<IL_OP_FENCE_L, int_AMDIL_fence_local>;
  defm FENCE_M : Fence<IL_OP_FENCE_M, int_AMDIL_fence_global>;
  defm FENCE_G : Fence<IL_OP_FENCE_G, int_AMDIL_fence_region>;
  defm FENCE_LM : Fence<IL_OP_FENCE_LM, int_AMDIL_fence_global_local>;
  defm FENCE_LG : Fence<IL_OP_FENCE_LG, int_AMDIL_fence_region_local>;
  defm FENCE_MG : Fence<IL_OP_FENCE_MG, int_AMDIL_fence_region_global>;
  defm FENCE_RO : Fence<IL_OP_FENCE_RO_LMG, int_AMDIL_read_fence>;
  defm FENCE_RO_L : Fence<IL_OP_FENCE_RO_L, int_AMDIL_read_fence_local>;
  defm FENCE_RO_M : Fence<IL_OP_FENCE_RO_M, int_AMDIL_read_fence_global>;
  defm FENCE_RO_G : Fence<IL_OP_FENCE_RO_G, int_AMDIL_read_fence_region>;
  defm FENCE_RO_LM : Fence<IL_OP_FENCE_RO_LM, int_AMDIL_read_fence_global_local>;
  defm FENCE_RO_LG : Fence<IL_OP_FENCE_RO_LG, int_AMDIL_read_fence_region_local>;
  defm FENCE_RO_MG : Fence<IL_OP_FENCE_RO_MG, int_AMDIL_read_fence_region_global>;
  defm FENCE_WO : Fence<IL_OP_FENCE_WO_LMG, int_AMDIL_write_fence>;
  defm FENCE_WO_L : Fence<IL_OP_FENCE_WO_L, int_AMDIL_write_fence_local>;
  defm FENCE_WO_M : Fence<IL_OP_FENCE_WO_M, int_AMDIL_write_fence_global>;
  defm FENCE_WO_G : Fence<IL_OP_FENCE_WO_G, int_AMDIL_write_fence_region>;
  defm FENCE_WO_LM : Fence<IL_OP_FENCE_WO_LM, int_AMDIL_write_fence_global_local>;
  defm FENCE_WO_LG : Fence<IL_OP_FENCE_WO_LG, int_AMDIL_write_fence_region_local>;
  defm FENCE_WO_MG : Fence<IL_OP_FENCE_WO_MG, int_AMDIL_write_fence_region_global>;
}

let isReturn = 1 in {
  defm EARLY_EXIT : UnaryIntMCVoid<IL_OP_RET_LOGICALNZ,
                                   int_AMDIL_early_exit,
                                   i32,
                                   GPR_32,
                                   i32imm,
                                   imm,
                                   " $src0">;
}

let neverHasSideEffects = 1 in {
  defm MEDIA_UNPACK_0 : ConvertIntMC<IL_OP_UNPACK_0,
                                     int_AMDIL_media_unpack_byte_0,
                                     v4f32, GPRV4I32, v4i32,
                                     GPRV4I32, v4i32imm, imm>;

  defm MEDIA_UNPACK_1 : ConvertIntMC<IL_OP_UNPACK_1,
                                     int_AMDIL_media_unpack_byte_1,
                                     v4f32, GPRV4I32, v4i32,
                                     GPRV4I32, v4i32imm, imm>;
  defm MEDIA_UNPACK_2 : ConvertIntMC<IL_OP_UNPACK_2,
                                     int_AMDIL_media_unpack_byte_2,
                                     v4f32, GPRV4I32, v4i32,
                                     GPRV4I32, v4i32imm, imm>;
  defm MEDIA_UNPACK_3 : ConvertIntMC<IL_OP_UNPACK_3,
                                     int_AMDIL_media_unpack_byte_3,
                                     v4f32, GPRV4I32, v4i32,
                                     GPRV4I32, v4i32imm, imm>;

  let ieee = 1, isCommutable = 1 in {
    defm MIN3 : TernaryPatMCf32<IL_OP_MIN3, int_AMDIL_min3>;
    defm MED3 : TernaryPatMCf32<IL_OP_MED3, int_AMDIL_med3>;
    defm MAX3 : TernaryPatMCf32<IL_OP_MAX3, int_AMDIL_max3>;
    defm IMIN3 : TernaryPatMCInt<IL_OP_I_MIN3, int_AMDIL_min3_i32>;
    defm IMED3 : TernaryPatMCInt<IL_OP_I_MED3, int_AMDIL_med3_i32>;
    defm IMAX3 : TernaryPatMCInt<IL_OP_I_MAX3, int_AMDIL_max3_i32>;
    defm UMIN3 : TernaryPatMCInt<IL_OP_U_MIN3, int_AMDIL_min3_u32>;
    defm UMED3 : TernaryPatMCInt<IL_OP_U_MED3, int_AMDIL_med3_u32>;
    defm UMAX3 : TernaryPatMCInt<IL_OP_U_MAX3, int_AMDIL_max3_u32>;
  }

  defm CLASS : ClassMC<int_AMDIL_class>;
  defm FREXP_EXP : IntrConvertF32TOI32<IL_OP_FREXP_EXP, int_AMDIL_frexp_exp>;
  defm DFREXP_EXP : IntrConvertF64TOI32<IL_OP_D_FREXP_EXP, int_AMDIL_frexp_exp>;

  defm FREXP_MANT : UnaryIntMCf32<IL_OP_FREXP_MANT, int_AMDIL_frexp_mant>;
  defm SAD16 : TernaryPatMCInt<IL_OP_SAD_U16, int_AMDIL_media_sad16>;
  defm SAD32 : TernaryPatMCInt<IL_OP_SAD_U32, int_AMDIL_media_sad32>;
}

let zeroop = 1 in {
  defm DDIV_INT : BinaryIntMCf64<IL_OP_D_DIV, int_AMDIL_div>;
  defm DDIV : BinaryOpMCf64<IL_OP_D_DIV, fdiv>;
}



defm FREXP_MANT : UnaryIntMCf64<IL_OP_D_FREXP_MANT, int_AMDIL_frexp_mant>;
defm DTRIG_PREOPf64 : BinaryIntMCFull<IL_OP_D_TRIG_PREOP,
                                      int_AMDIL_trig_preop_f64,
                                      f64, GPR_64, f64, GPR_64,
                                      f64imm, fpimm, f32, GPR_32,
                                      f32imm, fpimm>;


defm LDEXP : BinaryIntMCFFI<IL_OP_LDEXP, int_AMDIL_ldexp>;
defm MSAD : TernaryPatMCInt<IL_OP_MSAD, int_AMDIL_media_msad>;
defm QSAD : TernaryIntLLIL64Scalar<IL_OP_QSAD, int_AMDIL_media_qsad>;
defm MQSAD : TernaryIntLLIL64Scalar<IL_OP_MQSAD, int_AMDIL_media_mqsad>;

let isCommutable = 1 in {
  defm ADD_RTE : BinaryIntMCf32<IL_OP_ADD_RTE, int_AMDIL_add_rte>;
  defm ADD_RTP : BinaryIntMCf32<IL_OP_ADD_RTP, int_AMDIL_add_rtp>;
  defm ADD_RTN : BinaryIntMCf32<IL_OP_ADD_RTN, int_AMDIL_add_rtn>;
  defm ADD_RTZ : BinaryIntMCf32<IL_OP_ADD_RTZ, int_AMDIL_add_rtz>;
}
defm SUB_RTE : BinaryIntMCf32<IL_OP_SUB_RTE, int_AMDIL_sub_rte>;
defm SUB_RTP : BinaryIntMCf32<IL_OP_SUB_RTP, int_AMDIL_sub_rtp>;
defm SUB_RTN : BinaryIntMCf32<IL_OP_SUB_RTN, int_AMDIL_sub_rtn>;
defm SUB_RTZ : BinaryIntMCf32<IL_OP_SUB_RTZ, int_AMDIL_sub_rtz>;

let isCommutable = 1 in {
  defm MUL_RTE : BinaryIntMCf32<IL_OP_MUL_RTE, int_AMDIL_mul_rte>;
  defm MUL_RTP : BinaryIntMCf32<IL_OP_MUL_RTP, int_AMDIL_mul_rtp>;
  defm MUL_RTN : BinaryIntMCf32<IL_OP_MUL_RTN, int_AMDIL_mul_rtn>;
  defm MUL_RTZ : BinaryIntMCf32<IL_OP_MUL_RTZ, int_AMDIL_mul_rtz>;
}

defm MAD_RTE : TernaryPatMCf32<IL_OP_MAD_RTE, int_AMDIL_mad_rte>;
defm MAD_RTP : TernaryPatMCf32<IL_OP_MAD_RTP, int_AMDIL_mad_rtp>;
defm MAD_RTN : TernaryPatMCf32<IL_OP_MAD_RTN, int_AMDIL_mad_rtn>;
defm MAD_RTZ : TernaryPatMCf32<IL_OP_MAD_RTZ, int_AMDIL_mad_rtz>;
defm FMA_RTE : TernaryPatMCf32<IL_OP_FMA_RTE, int_AMDIL_fma_rte>;
defm FMA_RTP : TernaryPatMCf32<IL_OP_FMA_RTP, int_AMDIL_fma_rtp>;
defm FMA_RTN : TernaryPatMCf32<IL_OP_FMA_RTN, int_AMDIL_fma_rtn>;
defm FMA_RTZ : TernaryPatMCf32<IL_OP_FMA_RTZ, int_AMDIL_fma_rtz>;

let isCommutable = 1 in {
  defm ADD_RTE : BinaryIntMCf64Scalar<IL_OP_D_ADD_RTE, int_AMDIL_add_rte>;
  defm ADD_RTP : BinaryIntMCf64Scalar<IL_OP_D_ADD_RTP, int_AMDIL_add_rtp>;
  defm ADD_RTN : BinaryIntMCf64Scalar<IL_OP_D_ADD_RTN, int_AMDIL_add_rtn>;
  defm ADD_RTZ : BinaryIntMCf64Scalar<IL_OP_D_ADD_RTZ, int_AMDIL_add_rtz>;
}

defm SUB_RTE : BinaryIntMCf64Scalar<IL_OP_D_SUB_RTE, int_AMDIL_sub_rte>;
defm SUB_RTP : BinaryIntMCf64Scalar<IL_OP_D_SUB_RTP, int_AMDIL_sub_rtp>;
defm SUB_RTN : BinaryIntMCf64Scalar<IL_OP_D_SUB_RTN, int_AMDIL_sub_rtn>;
defm SUB_RTZ : BinaryIntMCf64Scalar<IL_OP_D_SUB_RTZ, int_AMDIL_sub_rtz>;
defm MUL_RTE : BinaryIntMCf64Scalar<IL_OP_D_MUL_RTE, int_AMDIL_mul_rte>;
defm MUL_RTP : BinaryIntMCf64Scalar<IL_OP_D_MUL_RTP, int_AMDIL_mul_rtp>;
defm MUL_RTN : BinaryIntMCf64Scalar<IL_OP_D_MUL_RTN, int_AMDIL_mul_rtn>;
defm MUL_RTZ : BinaryIntMCf64Scalar<IL_OP_D_MUL_RTZ, int_AMDIL_mul_rtz>;
defm MAD_RTE : TernaryPatMCf64Scalar<IL_OP_D_MAD_RTE, int_AMDIL_mad_rte>;
defm MAD_RTP : TernaryPatMCf64Scalar<IL_OP_D_MAD_RTP, int_AMDIL_mad_rtp>;
defm MAD_RTN : TernaryPatMCf64Scalar<IL_OP_D_MAD_RTN, int_AMDIL_mad_rtn>;
defm MAD_RTZ : TernaryPatMCf64Scalar<IL_OP_D_MAD_RTZ, int_AMDIL_mad_rtz>;


// FIXME: mayLoad = 0
let mayLoad = 1, isAsCheapAsAMove = 1, neverHasSideEffects = 1 in {
  def PRINT_GET_ITEM_OFFSET : ILFormat<IL_OP_MOV, (outs GPR_X_32:$dst),
                                       (ins),
                                       !strconcat(IL_OP_MOV.Text, " $dst, x0[5].x"),
                                       [(set GPR_X_32:$dst, (int_AMDIL_get_printf_item_offset))]>;
}
let isCall = 1, hasSideEffects = 1 in {
  def PRINT_INC_ITEM_OFFSETr : ILFormat<IL_OP_I_ADD, (outs),
                                       (ins GPR_32:$src),
                                       !strconcat(IL_OP_I_ADD.Text, " x0[5].x, x0[5].x, $src"),
                                       [(int_AMDIL_inc_printf_item_offset GPR_32:$src)]>;
  def PRINT_INC_ITEM_OFFSETi : ILFormat<IL_OP_I_ADD, (outs),
                                        (ins i32imm:$src),
                                        !strconcat(IL_OP_I_ADD.Text, " x0[5].x, x0[5].x, $src"),
                                        [(int_AMDIL_inc_printf_item_offset (i32 imm:$src))]>;
}

let sema = 1, hasSideEffects = 1 in {
  let swizzle = 7 in {
    def SEMAPHORE_WAITi : ILFormat<IL_OP_SEMAPHORE_WAIT, (outs),
                                  (ins i32imm:$ptr),
                                  !strconcat(IL_OP_SEMAPHORE_WAIT.Text, "_id($ptr)"),
                                  [(int_AMDIL_semaphore_wait_imm (i32 imm:$ptr))]>;

    def SEMAPHORE_SIGNALi : ILFormat<IL_OP_SEMAPHORE_SIGNAL, (outs),
                                    (ins i32imm:$ptr),
                                    !strconcat(IL_OP_SEMAPHORE_SIGNAL.Text, "_id($ptr)"),
                                    [(int_AMDIL_semaphore_signal_imm (i32 imm:$ptr))]>;
  }
}

let isAsCheapAsAMove = 1 in {
  defm POISONV : UnaryIntMCInt<IL_OP_POISONV, int_AMDIL_poisonv>;
}


let Predicates = [HasCISupport] in {
  def DCL_FLT32BIT : ILFormat<IL_DCL_FLAT_32BIT, (outs), (ins),
      !strconcat(IL_DCL_FLAT_32BIT.Text, " flat_address_ptr32"), []>;

  let mayLoad = 0, mayStore = 0 in { // This is OK assuming indexed array never used.
    defm NEARBY : UnaryOpMCf64<IL_OP_DROUND_NEAR, fnearbyint>;
    defm TRUNC  : UnaryOpMCf64<IL_OP_DTRUNC, ftrunc>;
    defm CEIL   : UnaryOpMCf64<IL_OP_DCEIL, fceil>;
    defm FLOOR  : UnaryOpMCf64<IL_OP_DFLOOR, ffloor>;
    defm ROUND_NEAREST : UnaryIntMCf64<IL_OP_DROUND_NEAR, int_AMDIL_round_nearest>;
    defm ROUND_NEGINF : UnaryIntMCf64<IL_OP_DFLOOR, int_AMDIL_round_neginf>;
    defm ROUND_POSINF : UnaryIntMCf64<IL_OP_DCEIL, int_AMDIL_round_posinf>;
    defm ROUND_ZERO : UnaryIntMCf64<IL_OP_DTRUNC, int_AMDIL_round_zero>;

    def UMAD6432 : ILFormat<IL_OP_MAD_U64_U32, (outs GPR_64:$dst),
        (ins GPR_32:$src0, GPR_32:$src1, GPR_32:$src2),
        !strconcat(IL_OP_MAD_U64_U32.Text, " $dst, $src0, $src1, $src2"),
        [(set GPR_64:$dst,
            (int_AMDIL_mad_u64_u32 GPR_32:$src0, GPR_32:$src1, GPR_32:$src2))]>;
    def SMAD6432 : ILFormat<IL_OP_MAD_I64_I32, (outs GPR_64:$dst),
        (ins GPR_32:$src0, GPR_32:$src1, GPR_32:$src2),
        !strconcat(IL_OP_MAD_I64_I32.Text, " $dst, $src0, $src1, $src2"),
        [(set GPR_64:$dst,
            (int_AMDIL_mad_i64_i32 GPR_32:$src0, GPR_32:$src1, GPR_32:$src2))]>;

    def MQSAD_i32i8: ILFormat<IL_OP_MQSAD_U32_U8, (outs GPRV4I32:$dst),
        (ins GPRXYV2I32:$src0, GPR_X_32:$src1, GPRV4I8:$src2),
        !strconcat(IL_OP_MQSAD_U32_U8.Text, " $dst, $src0, $src1, $src2"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_media_mqsad_u32_u8 GPRXYV2I32:$src0, GPR_X_32:$src1, GPRV4I8:$src2))]>;
  }

  let Predicates = [Has64BitPtr], flat = 1, addr64 = 1 in {
    let atomic = 1 in {
      def ATOM64_F_ADD64   : BinAtom64I64<IL_OP_UAV_READ_ADD,
          "_op(addx2)", atom_f_add>;
      def ATOM64_F_AND64   : BinAtom64I64<IL_OP_UAV_READ_AND,
          "_op(andx2)", atom_f_and>;
      def ATOM64_F_MAX64   : BinAtom64I64<IL_OP_UAV_READ_MAX,
          "_op(maxx2)", atom_f_max>;
      def ATOM64_F_MIN64   : BinAtom64I64<IL_OP_UAV_READ_MIN,
          "_op(minx2)", atom_f_min>;
      def ATOM64_F_UMAX64  : BinAtom64I64<IL_OP_UAV_READ_UMAX,
          "_op(umaxx2)", atom_f_umax>;
      def ATOM64_F_UMIN64  : BinAtom64I64<IL_OP_UAV_READ_UMIN,
          "_op(uminx2)", atom_f_umin>;
      def ATOM64_F_OR64    : BinAtom64I64<IL_OP_UAV_READ_OR,
          "_op(orx2)", atom_f_or>;
      def ATOM64_F_SUB64   : BinAtom64I64<IL_OP_UAV_READ_SUB,
          "_op(subx2)", atom_f_sub>;
      def ATOM64_F_XOR64   : BinAtom64I64<IL_OP_UAV_READ_XOR,
          "_op(xorx2)", atom_f_xor>;
      def ATOM64_F_INC64   : BinAtom64I64<IL_OP_UAV_READ_INC,
          "_op(incx2)", atom_f_inc>;
      def ATOM64_F_DEC64   : BinAtom64I64<IL_OP_UAV_READ_DEC,
          "_op(decx2)", atom_f_dec>;
      def ATOM64_F_XCHG64  : BinAtom64I64<IL_OP_UAV_READ_XCHG,
          "_op(swapx2)", atom_f_xchg>;
      def ATOM64_F_CMPXCHG64 : CmpXChg64I64<IL_OP_UAV_READ_CMPXCHG,
          "_op(cmpswapx2)", atom_f_cmpxchg>;
      def ATOM64_F_FCMPXCHG64 : CmpXChg64I64<IL_OP_UAV_READ_CMPXCHG,
          "_op(fcmpswapx2)", atom_f_cmpxchg>;
      def ATOM64_F_FMAX64   : BinAtom64I64<IL_OP_UAV_READ_MAX,
          "_op(fmaxx2)", atom_f_max>;
      def ATOM64_F_FMIN64   : BinAtom64I64<IL_OP_UAV_READ_MIN,
          "_op(fminx2)", atom_f_min>;
      def ATOM64_F_ADD     : BinAtom64<IL_OP_UAV_READ_ADD,
          "_op(add)", atom_f_add>;
      def ATOM64_F_AND     : BinAtom64<IL_OP_UAV_READ_AND,
          "_op(and)", atom_f_and>;
      def ATOM64_F_MAX     : BinAtom64<IL_OP_UAV_READ_MAX,
          "_op(max)", atom_f_max>;
      def ATOM64_F_MIN     : BinAtom64<IL_OP_UAV_READ_MIN,
          "_op(min)", atom_f_min>;
      def ATOM64_F_UMAX    : BinAtom64<IL_OP_UAV_READ_UMAX,
          "_op(umax)", atom_f_umax>;
      def ATOM64_F_UMIN    : BinAtom64<IL_OP_UAV_READ_UMIN,
          "_op(umin)", atom_f_umin>;
      def ATOM64_F_OR      : BinAtom64<IL_OP_UAV_READ_OR,
          "_op(or)", atom_f_or>;
      def ATOM64_F_SUB     : BinAtom64<IL_OP_UAV_READ_SUB,
          "_op(sub)", atom_f_sub>;
      def ATOM64_F_XOR     : BinAtom64<IL_OP_UAV_READ_XOR,
          "_op(xor)", atom_f_xor>;
      def ATOM64_F_INC     : BinAtom64<IL_OP_UAV_READ_INC,
          "_op(inc)", atom_f_inc>;
      def ATOM64_F_DEC     : BinAtom64<IL_OP_UAV_READ_DEC,
          "_op(dec)", atom_f_dec>;
      def ATOM64_F_XCHG    : BinAtom64<IL_OP_UAV_READ_XCHG,
          "_op(swap)", atom_f_xchg>;
      def ATOM64_F_CMPXCHG : CmpXChg64<IL_OP_UAV_READ_CMPXCHG,
          "_op(cmpswap)", atom_f_cmpxchg>;
      def ATOM64_F_FCMPXCHG : CmpXChg64<IL_OP_UAV_READ_CMPXCHG,
          "_op(fcmpswap)", atom_f_cmpxchg>;
      def ATOM64_F_FMAX     : BinAtom64<IL_OP_UAV_READ_MAX,
          "_op(fmax)", atom_f_max>;
      def ATOM64_F_FMIN     : BinAtom64<IL_OP_UAV_READ_MIN,
          "_op(fmin)", atom_f_min>;
    }
    let isCodeGenOnly=1 in {
      let store = 1 in {
        defm FLATSTORE64 : STORE<"!flat store", flat_store, MEM64, ADDR64>;
        let truncate = 1 in {
          defm FLATSTORE64 : FTRUNCSTORE<"!flat store", MEM64, ADDR64>;
        }
      }
      let load = 1 in {
        defm FLATLOAD64 : LOAD<"!flat store", flat_load, MEM64, ADDR64>;
        let extLoadType = 2 in {
          defm FLATZEXTLOAD64 : LOAD<"!flat zext load", flat_zext_load, MEM64, ADDR64>;
        }
        let extLoadType = 1 in {
          defm FLATSEXTLOAD64 : LOAD<"!flat sext load", flat_sext_load, MEM64, ADDR64>;
        }
        let extLoadType = 3 in {
          defm FLATAEXTLOAD64 : LOAD<"!flat aext load", flat_aext_load, MEM64, ADDR64>;
        }
      }
    }
  }
  let Predicates = [Has32BitPtr], flat = 1  in {
    let atomic = 1 in {
      def ATOM_F_ADD64 : BinAtomI64<IL_OP_UAV_READ_ADD,
          "_op(addx2)", atom_f_add>;
      def ATOM_F_AND64 : BinAtomI64<IL_OP_UAV_READ_AND,
          "_op(andx2)", atom_f_and>;
      def ATOM_F_MAX64 : BinAtomI64<IL_OP_UAV_READ_MAX,
          "_op(maxx2)", atom_f_max>;
      def ATOM_F_MIN64 : BinAtomI64<IL_OP_UAV_READ_MIN,
          "_op(minx2)", atom_f_min>;
      def ATOM_F_UMAX64 : BinAtomI64<IL_OP_UAV_READ_UMAX,
          "_op(umaxx2)", atom_f_umax>;
      def ATOM_F_UMIN64 : BinAtomI64<IL_OP_UAV_READ_UMIN,
          "_op(uminx2)", atom_f_umin>;
      def ATOM_F_OR64 : BinAtomI64<IL_OP_UAV_READ_OR,
          "_op(orx2)", atom_f_or>;
      def ATOM_F_SUB64 : BinAtomI64<IL_OP_UAV_READ_SUB,
          "_op(subx2)", atom_f_sub>;
      def ATOM_F_XOR64 : BinAtomI64<IL_OP_UAV_READ_XOR,
          "_op(xorx2)", atom_f_xor>;
      def ATOM_F_INC64 : BinAtomI64<IL_OP_UAV_READ_INC,
          "_op(incx2)", atom_f_inc>;
      def ATOM_F_DEC64 : BinAtomI64<IL_OP_UAV_READ_DEC,
          "_op(decx2)", atom_f_dec>;
      def ATOM_F_XCHG64 : BinAtomI64<IL_OP_UAV_READ_XCHG,
          "_op(swapx2)", atom_f_xchg>;
      def ATOM_F_CMPXCHG64 : CmpXChgI64<IL_OP_UAV_READ_CMPXCHG,
          "_op(cmpswapx2)", atom_f_cmpxchg>;
      def ATOM_F_FCMPXCHG64 : CmpXChgI64<IL_OP_UAV_READ_CMPXCHG,
          "_op(fcmpswapx2)", atom_f_cmpxchg>;
      def ATOM_F_FMAX64 : BinAtomI64<IL_OP_UAV_READ_MAX,
          "_op(fmaxx2)", atom_f_max>;
      def ATOM_F_FMIN64 : BinAtomI64<IL_OP_UAV_READ_MIN,
          "_op(fminx2)", atom_f_min>;
      def ATOM_F_ADD : BinAtom<IL_OP_UAV_READ_ADD,
          "_op(add)", atom_f_add>;
      def ATOM_F_AND : BinAtom<IL_OP_UAV_READ_AND,
          "_op(and)", atom_f_and>;
      def ATOM_F_MAX : BinAtom<IL_OP_UAV_READ_MAX,
          "_op(max)", atom_f_max>;
      def ATOM_F_MIN : BinAtom<IL_OP_UAV_READ_MIN,
          "_op(min)", atom_f_min>;
      def ATOM_F_UMAX : BinAtom<IL_OP_UAV_READ_UMAX,
          "_op(umax)", atom_f_umax>;
      def ATOM_F_UMIN : BinAtom<IL_OP_UAV_READ_UMIN,
          "_op(umin)", atom_f_umin>;
      def ATOM_F_OR : BinAtom<IL_OP_UAV_READ_OR,
          "_op(or)", atom_f_or>;
      def ATOM_F_SUB : BinAtom<IL_OP_UAV_READ_SUB,
          "_op(sub)", atom_f_sub>;
      def ATOM_F_XOR : BinAtom<IL_OP_UAV_READ_XOR,
          "_op(xor)", atom_f_xor>;
      def ATOM_F_INC : BinAtom<IL_OP_UAV_READ_INC,
          "_op(inc)", atom_f_inc>;
      def ATOM_F_DEC : BinAtom<IL_OP_UAV_READ_DEC,
          "_op(dec)", atom_f_dec>;
      def ATOM_F_XCHG : BinAtom<IL_OP_UAV_READ_XCHG,
          "_op(swap)", atom_f_xchg>;
      def ATOM_F_CMPXCHG : CmpXChg<IL_OP_UAV_READ_CMPXCHG,
          "_op(cmpswap)", atom_f_cmpxchg>;
      def ATOM_F_FCMPXCHG : CmpXChg<IL_OP_UAV_READ_CMPXCHG,
          "_op(fcmpswap)", atom_f_cmpxchg>;
      def ATOM_F_FMAX : BinAtom<IL_OP_UAV_READ_MAX,
          "_op(fmax)", atom_f_max>;
      def ATOM_F_FMIN : BinAtom<IL_OP_UAV_READ_MIN,
          "_op(fmin)", atom_f_min>;
    }
    let isCodeGenOnly=1 in {
      let store = 1 in {
        let truncate = 1 in {
          defm FLATSTORE : STORE<"!flat store", flat_store, MEM32, ADDR>;
        }
        defm FLATTRUNCSTORE : FTRUNCSTORE<"!flat trunc store", MEM32, ADDR>;
      }
      let load = 1 in {
        defm FLATLOAD : LOAD<"!flat load", flat_load, MEM32, ADDR>;
        let extLoadType = 2 in {
          defm FLATZEXTLOAD : LOAD<"!flat zext load", flat_zext_load, MEM32, ADDR>;
        }
        let extLoadType = 1 in {
          defm FLATSEXTLOAD : LOAD<"!flat sext load", flat_sext_load, MEM32, ADDR>;
        }
        let extLoadType = 3 in {
          defm FLATAEXTLOAD : LOAD<"!flat aext load", flat_aext_load, MEM32, ADDR>;
        }
      }
    }
  }
  let flat = 1 in {
    let load = 1 in {
      defm FLDi8    : FlatLoadMC<FLAT_LOAD_SBYTE,  GPRI8>;
      defm FLDu8    : FlatLoadMC<FLAT_LOAD_UBYTE,  GPRI8>;
      defm FLDi16   : FlatLoadMC<FLAT_LOAD_SSHORT, GPRI16>;
      defm FLDu16   : FlatLoadMC<FLAT_LOAD_USHORT, GPRI16>;
      defm FLDi32   : FlatLoadMC<FLAT_LOAD_DWORD,  GPR_32>;
      defm FLDv2i32 : FlatLoadMC<FLAT_LOAD_DWORD2, GPRV2I32>;
      defm FLDv3i32 : FlatLoadMC<FLAT_LOAD_DWORD3, GPRV4I32>;
      defm FLDv4i32 : FlatLoadMC<FLAT_LOAD_DWORD4, GPRV4I32>;
    }

    let store = 1 in {
      defm FSTi8    : FlatStoreMC<FLAT_STORE_SBYTE,  GPRI8>;
      defm FSTu8    : FlatStoreMC<FLAT_STORE_UBYTE,  GPRI8>;
      defm FSTi16   : FlatStoreMC<FLAT_STORE_SSHORT, GPRI16>;
      defm FSTu16   : FlatStoreMC<FLAT_STORE_USHORT, GPRI16>;
      defm FSTi32   : FlatStoreMC<FLAT_STORE_DWORD,  GPR_32>;
      defm FSTv2i32 : FlatStoreMC<FLAT_STORE_DWORD2, GPRV2I32>;
      defm FSTv3i32 : FlatStoreMC<FLAT_STORE_DWORD3, GPRV4I32>;
      defm FSTv4i32 : FlatStoreMC<FLAT_STORE_DWORD4, GPRV4I32>;
    }
  }
}

let Predicates = [HasCrossThreadOps] in {
    defm BALLOT : UnaryIntMCFull<IL_OP_BALLOT, int_AMDIL_ballot,
                                 i64, GPR_64, i32, GPR_32, i32imm, imm>;
    defm LANE_ID : NullaryIntMCFull<IL_OP_LANE_ID, int_AMDIL_lane_id,
                                    i32, GPR_32>;
    defm READFIRSTLANE: UnaryIntMCFull<IL_OP_READFIRSTLANE,
                                       int_AMDIL_readfirstlane,
                                       v4i32, GPRV4I32,
                                       v4i32, GPRV4I32, v4i32imm, imm>;
    defm RECEIVELANE : QuaternaryInst<IL_OP_RECEIVELANE,
                                      int_AMDIL_receivelane, i32,
                                      GPR_32, i32imm, imm>;
}

//-------------------------------------------------------------------//
// Multiclasses that deal with the cmp instruction.
//-------------------------------------------------------------------//
// Floating-point zero.
def fpimm0 : PatLeaf<(fpimm), [{ return N->isExactlyValue(+0.0); }]>;
// Floating-point one.
def fpimm1 : PatLeaf<(fpimm), [{ return N->isExactlyValue(+1.0); }]>;
// Floating-point negone.
def fpimmneg1 : PatLeaf<(fpimm), [{ return N->isExactlyValue(-1.0); }]>;
// Floating-point halfone.
def fpimm05 : PatLeaf<(fpimm), [{ return N->isExactlyValue(+0.5); }]>;
// Floating-point neghalfone.
def fpimmneg05 : PatLeaf<(fpimm), [{ return N->isExactlyValue(-0.5); }]>;

multiclass CmpRelop<RegisterClass reg, ValueType type, Operand oper, string conststr,
				    PatLeaf imm_para, PatLeaf cc, string ccstr, SDNode imm0> {
  def rirr : ILFormat<IL_OP_CMP,
        (outs GPR_32:$dst),
        (ins reg:$src0, f32imm:$src1, reg:$src2, reg:$src3),
        !strconcat(IL_OP_CMP.Text, "_relop("#ccstr#")_cmpval("#conststr#") $dst, $src0, $src2, $src3"),
        [(set GPR_32:$dst,
                (select (i32 (setcc (f32 reg:$src0), imm_para:$src1, cc)), (type reg:$src2), (type reg:$src3)))]>;
  def riri : ILFormat<IL_OP_CMP,
        (outs GPR_32:$dst),
        (ins reg:$src0, f32imm:$src1, reg:$src2, oper:$src3),
        !strconcat(IL_OP_CMP.Text, "_relop("#ccstr#")_cmpval("#conststr#") $dst, $src0, $src2, $src3"),
        [(set GPR_32:$dst,
                (select (i32 (setcc (f32 reg:$src0), imm_para:$src1, cc)), (type reg:$src2), (type imm0:$src3)))]>;
  def riir : ILFormat<IL_OP_CMP,
        (outs GPR_32:$dst),
        (ins reg:$src0, f32imm:$src1, oper:$src2, reg:$src3),
        !strconcat(IL_OP_CMP.Text, "_relop("#ccstr#")_cmpval("#conststr#") $dst, $src0, $src2, $src3"),
        [(set GPR_32:$dst,
                (select (i32 (setcc (f32 reg:$src0), imm_para:$src1, cc)), (type imm0:$src2), (type reg:$src3)))]>;
  def riii : ILFormat<IL_OP_CMP,
        (outs GPR_32:$dst),
        (ins reg:$src0, f32imm:$src1, oper:$src2, oper:$src3),
        !strconcat(IL_OP_CMP.Text, "_relop("#ccstr#")_cmpval("#conststr#") $dst, $src0, $src2, $src3"),
        [(set GPR_32:$dst,
                (select (i32 (setcc (f32 reg:$src0), imm_para:$src1, cc)), (type imm0:$src2), (type imm0:$src3)))]>;
}

/*
      Vector types are left behind
      v1i32          =  27,   //  1 x i32
      v2i32          =  28,   //  2 x i32
      v4i32          =  29,   //  4 x i32
      v8i32          =  30,   //  8 x i32
      v16i32         =  31,   // 16 x i32
      v2f32          =  38,   //  2 x f32
      v4f32          =  39,   //  4 x f32
      v8f32          =  40,   //  8 x f32
 */
multiclass CmpRelopExpand<string conststr, PatLeaf fpimm0> {
   defm EQf32 : CmpRelop<GPR_32, f32, f32imm, conststr, fpimm0, cond_seteq_setoeq, "eq", fpimm>;
   defm GEf32 : CmpRelop<GPR_32, f32, f32imm, conststr, fpimm0, cond_setge_setoge, "ge", fpimm>;
   defm LTf32 : CmpRelop<GPR_32, f32, f32imm, conststr, fpimm0, cond_setlt_setolt, "lt", fpimm>;
   defm EQi32 : CmpRelop<GPR_32, i32, i32imm, conststr, fpimm0, cond_seteq_setoeq, "eq", imm>;
   defm GEi32 : CmpRelop<GPR_32, i32, i32imm, conststr, fpimm0, cond_setge_setoge, "ge", imm>;
   defm LTi32 : CmpRelop<GPR_32, i32, i32imm, conststr, fpimm0, cond_setlt_setolt, "lt", imm>;
}

defm ZERO_ : CmpRelopExpand<"0.0", fpimm0>;
defm ONE_ : CmpRelopExpand<"1.0", fpimm1>;
defm NEGONE_ : CmpRelopExpand<"-1.0", fpimmneg1>;
defm HALFONE_ : CmpRelopExpand<"0.5", fpimm05>;
defm NEGHALFONE_ : CmpRelopExpand<"-0.5", fpimmneg05>;
