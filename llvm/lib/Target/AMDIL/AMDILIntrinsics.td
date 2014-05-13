 //===--- AMDILIntrinsics.td - Defines AMDIL Intrinscs -*- tablegen -*------===//
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
// This file defines all of the amdil-specific intrinsics
//
//===---------------------------------------------------------------===//

let TargetPrefix = "AMDIL", isTarget = 1 in {
//------------- Synchronization Functions - OpenCL 6.11.9 --------------------//
  def int_AMDIL_gws : GCCBuiltin<"__amdil_gws">,
    UnaryIntNoRetInt;
  def int_AMDIL_gws_global : GCCBuiltin<"__amdil_gws_global">,
    UnaryIntNoRetInt;
  def int_AMDIL_gws_local : GCCBuiltin<"__amdil_gws_local">,
    UnaryIntNoRetInt;
  def int_AMDIL_gws_region : GCCBuiltin<"__amdil_gws_region">,
    UnaryIntNoRetInt;
  def int_AMDIL_gws_global_local : GCCBuiltin<"__amdil_gws_global_local">,
    UnaryIntNoRetInt;
  def int_AMDIL_gws_global_region : GCCBuiltin<"__amdil_gws_global_region">,
    UnaryIntNoRetInt;
  def int_AMDIL_gws_local_region : GCCBuiltin<"__amdil_gws_local_region">,
    UnaryIntNoRetInt;
  def int_AMDIL_fence : GCCBuiltin<"__amdil_mem_fence">,
    UnaryIntNoRetInt;
  def int_AMDIL_fence_global : GCCBuiltin<"__amdil_mem_fence_global">,
    UnaryIntNoRetInt;
  def int_AMDIL_fence_local : GCCBuiltin<"__amdil_mem_fence_local">,
    UnaryIntNoRetInt;
  def int_AMDIL_fence_region : GCCBuiltin<"__amdil_mem_fence_region">,
    UnaryIntNoRetInt;
  def int_AMDIL_fence_global_local : GCCBuiltin<"__amdil_mem_fence_global_local">,
    UnaryIntNoRetInt;
  def int_AMDIL_fence_region_global : GCCBuiltin<"__amdil_mem_fence_region_global">,
    UnaryIntNoRetInt;
  def int_AMDIL_fence_region_local : GCCBuiltin<"__amdil_mem_fence_region_local">,
    UnaryIntNoRetInt;

  def int_AMDIL_read_fence : GCCBuiltin<"__amdil_read_mem_fence">,
    UnaryIntNoRetInt;
  def int_AMDIL_read_fence_global : GCCBuiltin<"__amdil_read_mem_fence_global">,
    UnaryIntNoRetInt;
  def int_AMDIL_read_fence_local : GCCBuiltin<"__amdil_read_mem_fence_local">,
    UnaryIntNoRetInt;
  def int_AMDIL_read_fence_region : GCCBuiltin<"__amdil_read_mem_fence_region">,
    UnaryIntNoRetInt;
  def int_AMDIL_read_fence_global_local : GCCBuiltin<"__amdil_read_mem_fence_global_local">,
    UnaryIntNoRetInt;
  def int_AMDIL_read_fence_region_global : GCCBuiltin<"__amdil_read_mem_fence_region_global">,
    UnaryIntNoRetInt;
  def int_AMDIL_read_fence_region_local : GCCBuiltin<"__amdil_read_mem_fence_region_local">,
    UnaryIntNoRetInt;

  def int_AMDIL_write_fence : GCCBuiltin<"__amdil_write_mem_fence">,
    UnaryIntNoRetInt;
  def int_AMDIL_write_fence_global : GCCBuiltin<"__amdil_write_mem_fence_global">,
    UnaryIntNoRetInt;
  def int_AMDIL_write_fence_local : GCCBuiltin<"__amdil_write_mem_fence_local">,
    UnaryIntNoRetInt;
  def int_AMDIL_write_fence_region : GCCBuiltin<"__amdil_write_mem_fence_region">,
    UnaryIntNoRetInt;
  def int_AMDIL_write_fence_global_local : GCCBuiltin<"__amdil_write_mem_fence_global_local">,
    UnaryIntNoRetInt;
  def int_AMDIL_write_fence_region_global : GCCBuiltin<"__amdil_write_mem_fence_region_global">,
    UnaryIntNoRetInt;
  def int_AMDIL_write_fence_region_local : GCCBuiltin<"__amdil_write_mem_fence_region_local">,
    UnaryIntNoRetInt;

  let Properties = [IntrReadMem] in {
    def int_AMDIL_early_exit : GCCBuiltin<"__amdil_early_exit">,
      UnaryIntNoRetInt;
  }

  def int_AMDIL_cmov_logical : GCCBuiltin<"__amdil_cmov_logical">,
    TernaryIntInt;
  def int_AMDIL_fabs : GCCBuiltin<"__amdil_fabs">, UnaryIntFloat;
  def int_AMDIL_abs : GCCBuiltin<"__amdil_abs">, UnaryIntInt;

  def int_AMDIL_ibit_extract : GCCBuiltin<"__amdil_ibit_extract">,
    TernaryIntInt;
  def int_AMDIL_ubit_extract : GCCBuiltin<"__amdil_ubit_extract">,
    TernaryIntInt;
  def int_AMDIL_bit_reverse_u32 : GCCBuiltin<"__amdil_ubit_reverse">,
    UnaryIntInt;
  def int_AMDIL_bit_count_i32 : GCCBuiltin<"__amdil_count_bits">,
    UnaryIntInt;
  def int_AMDIL_bit_find_first_lo : GCCBuiltin<"__amdil_ffb_lo">,
    UnaryIntInt;
  def int_AMDIL_bit_find_first_hi : GCCBuiltin<"__amdil_ffb_hi">,
    UnaryIntInt;
  def int_AMDIL_bit_find_first_sgn : GCCBuiltin<"__amdil_ffb_signed">,
    UnaryIntInt;
  def int_AMDIL_media_bitalign : GCCBuiltin<"__amdil_bitalign">,
    TernaryIntInt;
  def int_AMDIL_media_bytealign : GCCBuiltin<"__amdil_bytealign">,
    TernaryIntInt;
  def int_AMDIL_ubit_insert : GCCBuiltin<"__amdil_ubit_insert">,
    Intrinsic<[llvm_anyint_ty],
              [LLVMMatchType<0>, LLVMMatchType<0>, LLVMMatchType<0>, LLVMMatchType<0>],
              [IntrNoMem]>;

  def int_AMDIL_bfi : GCCBuiltin<"__amdil_bfi">,
    TernaryIntInt;
  def int_AMDIL_bfm : GCCBuiltin<"__amdil_bfm">,
    BinaryIntInt;
  def int_AMDIL_mad_i32 : GCCBuiltin<"__amdil_imad">,
    TernaryIntInt;
  def int_AMDIL_mad_u32 : GCCBuiltin<"__amdil_umad">,
    TernaryIntInt;
  def int_AMDIL_mad : GCCBuiltin<"__amdil_mad">,
    TernaryIntFloat;

  let Properties = [Commutative, IntrNoMem] in {
    def int_AMDIL_mulhi_i32 : GCCBuiltin<"__amdil_imul_high">,
      BinaryIntInt;
    def int_AMDIL_mulhi_u32 : GCCBuiltin<"__amdil_umul_high">,
      BinaryIntInt;
    def int_AMDIL_mul24_i32 : GCCBuiltin<"__amdil_imul24">,
      BinaryIntInt;
    def int_AMDIL_mul24_u32 : GCCBuiltin<"__amdil_umul24">,
      BinaryIntInt;
    def int_AMDIL_mulhi24_i32 : GCCBuiltin<"__amdil_imul24_high">,
      BinaryIntInt;
    def int_AMDIL_mulhi24_u32 : GCCBuiltin<"__amdil_umul24_high">,
      BinaryIntInt;
  }
  def int_AMDIL_mad24_i32 : GCCBuiltin<"__amdil_imad24">,
    TernaryIntInt;
  def int_AMDIL_mad24_u32 : GCCBuiltin<"__amdil_umad24">,
    TernaryIntInt;
  def int_AMDIL_carry_i32 : GCCBuiltin<"__amdil_carry">,
    BinaryIntInt;
  def int_AMDIL_borrow_i32 : GCCBuiltin<"__amdil_borrow">,
    BinaryIntInt;

  let Properties = [Commutative, IntrNoMem] in {
    def int_AMDIL_min_i32 : GCCBuiltin<"__amdil_imin">,
      BinaryIntInt;
    def int_AMDIL_min_u32 : GCCBuiltin<"__amdil_umin">,
      BinaryIntInt;
    def int_AMDIL_min : GCCBuiltin<"__amdil_min">,
      BinaryIntFloat;
    def int_AMDIL_max_i32 : GCCBuiltin<"__amdil_imax">,
      BinaryIntInt;
    def int_AMDIL_max_u32 : GCCBuiltin<"__amdil_umax">,
      BinaryIntInt;
    def int_AMDIL_max : GCCBuiltin<"__amdil_max">,
      BinaryIntFloat;
  }
  def int_AMDIL_media_lerp_u4 : GCCBuiltin<"__amdil_u4lerp">,
    TernaryIntInt;
  def int_AMDIL_media_sad : GCCBuiltin<"__amdil_sad">,
    TernaryIntInt;
  def int_AMDIL_media_sad_hi : GCCBuiltin<"__amdil_sadhi">,
    TernaryIntInt;
  def int_AMDIL_fraction : GCCBuiltin<"__amdil_fraction">,
    UnaryIntFloat;
  def int_AMDIL_clamp : GCCBuiltin<"__amdil_clamp">,
    TernaryIntFloat;
  def int_AMDIL_pireduce : GCCBuiltin<"__amdil_pireduce">,
    UnaryIntFloat;
  def int_AMDIL_round_nearest : GCCBuiltin<"__amdil_round_nearest">,
    Intrinsic<[llvm_anyfloat_ty], [LLVMMatchType<0>], [IntrNoMem]>;
  def int_AMDIL_round_neginf : GCCBuiltin<"__amdil_round_neginf">,
    Intrinsic<[llvm_anyfloat_ty], [LLVMMatchType<0>], [IntrNoMem]>;
  def int_AMDIL_round_posinf : GCCBuiltin<"__amdil_round_posinf">,
    Intrinsic<[llvm_anyfloat_ty], [LLVMMatchType<0>], [IntrNoMem]>;
  def int_AMDIL_round_zero : GCCBuiltin<"__amdil_round_zero">,
    UnaryIntFloat;
  def int_AMDIL_acos : GCCBuiltin<"__amdil_acos">,
    UnaryIntFloat;
  def int_AMDIL_atan : GCCBuiltin<"__amdil_atan">,
    UnaryIntFloat;
  def int_AMDIL_asin : GCCBuiltin<"__amdil_asin">,
    UnaryIntFloat;
  def int_AMDIL_cos : GCCBuiltin<"__amdil_cos">,
    UnaryIntFloat;
  def int_AMDIL_cos_vec : GCCBuiltin<"__amdil_cos_vec">,
    UnaryIntFloat;
  def int_AMDIL_tan : GCCBuiltin<"__amdil_tan">,
    UnaryIntFloat;
  def int_AMDIL_sin : GCCBuiltin<"__amdil_sin">,
    UnaryIntFloat;
  def int_AMDIL_sin_vec : GCCBuiltin<"__amdil_sin_vec">,
    UnaryIntFloat;
  def int_AMDIL_ftz : GCCBuiltin<"__amdil_ftz">,
    UnaryIntFloat;
  def int_AMDIL_pow : GCCBuiltin<"__amdil_pow">, BinaryIntFloat;
  def int_AMDIL_div : GCCBuiltin<"__amdil_div">, BinaryIntFloat;
  def int_AMDIL_div_precise : GCCBuiltin<"__amdil_div_precise">, BinaryIntFloat;
  def int_AMDIL_udiv : GCCBuiltin<"__amdil_udiv">, BinaryIntInt;
  def int_AMDIL_sqrt : GCCBuiltin<"__amdil_sqrt">,
    UnaryIntFloat;
  def int_AMDIL_sqrt_vec : GCCBuiltin<"__amdil_sqrt_vec">,
    UnaryIntFloat;
  def int_AMDIL_exp : GCCBuiltin<"__amdil_exp">,
    UnaryIntFloat;
  def int_AMDIL_exp_vec : GCCBuiltin<"__amdil_exp_vec">,
    UnaryIntFloat;
  def int_AMDIL_exn : GCCBuiltin<"__amdil_exn">,
    UnaryIntFloat;
  def int_AMDIL_log : GCCBuiltin<"__amdil_log">,
    UnaryIntFloat;
  def int_AMDIL_log_vec : GCCBuiltin<"__amdil_log_vec">,
    UnaryIntFloat;
  def int_AMDIL_ln : GCCBuiltin<"__amdil_ln">,
    UnaryIntFloat;
  def int_AMDIL_sign : GCCBuiltin<"__amdil_sign">,
    UnaryIntFloat;
  def int_AMDIL_fma : GCCBuiltin<"__amdil_fma">,
    TernaryIntFloat;
  def int_AMDIL_rsq : GCCBuiltin<"__amdil_rsq">,
    UnaryIntFloat;
  def int_AMDIL_rsq_vec : GCCBuiltin<"__amdil_rsq_vec">,
    UnaryIntFloat;
  def int_AMDIL_length : GCCBuiltin<"__amdil_length">,
    UnaryIntFloat;
  def int_AMDIL_lerp : GCCBuiltin<"__amdil_lerp">,
    TernaryIntFloat;
  def int_AMDIL_media_sad4 : GCCBuiltin<"__amdil_sad4">,
    Intrinsic<[llvm_i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_i32_ty], [IntrNoMem]>;

  def int_AMDIL_frexp_f64 : GCCBuiltin<"__amdil_frexp">,
    Intrinsic<[llvm_v2i64_ty], [llvm_double_ty], [IntrNoMem]>;
  def int_AMDIL_ldexp : GCCBuiltin<"__amdil_ldexp">,
    Intrinsic<[llvm_anyfloat_ty], [llvm_anyfloat_ty, llvm_anyint_ty], [IntrNoMem]>;
  def int_AMDIL_drcp : GCCBuiltin<"__amdil_rcp">,
    Intrinsic<[llvm_double_ty], [llvm_double_ty], [IntrNoMem]>;
  def int_AMDIL_convert_f16_f32 : GCCBuiltin<"__amdil_half_to_float">,
    ConvertIntITOF;
  def int_AMDIL_convert_f32_f16 : GCCBuiltin<"__amdil_float_to_half">,
    ConvertIntFTOI;
  def int_AMDIL_convert_f32_i32_rpi : GCCBuiltin<"__amdil_float_to_int_rpi">,
    ConvertIntFTOI;
  def int_AMDIL_convert_f32_i32_flr : GCCBuiltin<"__amdil_float_to_int_flr">,
    ConvertIntFTOI;
  def int_AMDIL_convert_f32_f16_near : GCCBuiltin<"__amdil_float_to_half_near">,
    ConvertIntFTOI;
  def int_AMDIL_convert_f32_f16_neg_inf : GCCBuiltin<"__amdil_float_to_half_neg_inf">,
    ConvertIntFTOI;
  def int_AMDIL_convert_f32_f16_plus_inf : GCCBuiltin<"__amdil_float_to_half_plus_inf">,
    ConvertIntFTOI;
  def int_AMDIL_media_convert_f2v4u8 : GCCBuiltin<"__amdil_f_2_u4">,
    Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty], [IntrNoMem]>;
  def int_AMDIL_media_unpack_byte_0 : GCCBuiltin<"__amdil_unpack_0">,
    ConvertIntITOF;
  def int_AMDIL_media_unpack_byte_1 : GCCBuiltin<"__amdil_unpack_1">,
    ConvertIntITOF;
  def int_AMDIL_media_unpack_byte_2 : GCCBuiltin<"__amdil_unpack_2">,
    ConvertIntITOF;
  def int_AMDIL_media_unpack_byte_3 : GCCBuiltin<"__amdil_unpack_3">,
    ConvertIntITOF;
  def int_AMDIL_dp2_add : GCCBuiltin<"__amdil_dp2_add">,
    Intrinsic<[llvm_float_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_float_ty], [IntrNoMem]>;
  def int_AMDIL_dp2 : GCCBuiltin<"__amdil_dp2">,
    Intrinsic<[llvm_float_ty], [llvm_v2f32_ty, llvm_v2f32_ty], [IntrNoMem]>;
  def int_AMDIL_dp3 : GCCBuiltin<"__amdil_dp3">,
    Intrinsic<[llvm_float_ty], [llvm_v4f32_ty, llvm_v4f32_ty], [IntrNoMem]>;
  def int_AMDIL_dp4 : GCCBuiltin<"__amdil_dp4">,
        Intrinsic<[llvm_float_ty], [llvm_v4f32_ty, llvm_v4f32_ty], [IntrNoMem]>;
//===---------------------- Image functions begin ------------------------===//
  def int_AMDIL_image1d_write : GCCBuiltin<"__amdil_image1d_write">,
    Intrinsic<[], [llvm_ptr_ty, llvm_v2i32_ty, llvm_v4i32_ty], [IntrReadWriteArgMem]>;

  def int_AMDIL_image1d_load : GCCBuiltin<"__amdil_image1d_load">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_v4i32_ty], [IntrReadWriteArgMem]>;

  def int_AMDIL_image1d_read_norm : GCCBuiltin<"__amdil_image1d_read_norm">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_v4f32_ty], [IntrReadArgMem]>;

  def int_AMDIL_image1d_read_unnorm : GCCBuiltin<"__amdil_image1d_read_unnorm">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_v4f32_ty], [IntrReadArgMem]>;

  def int_AMDIL_image1d_info0 : GCCBuiltin<"__amdil_image1d_info0">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty], [IntrReadMem]>;

  def int_AMDIL_image1d_info1 : GCCBuiltin<"__amdil_image1d_info1">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty], [IntrReadMem]>;

  def int_AMDIL_image1d_array_write : GCCBuiltin<"__amdil_image1d_array_write">,
    Intrinsic<[], [llvm_ptr_ty, llvm_v2i32_ty, llvm_v4i32_ty], [IntrReadWriteArgMem]>;

  def int_AMDIL_image1d_array_load : GCCBuiltin<"__amdil_image1d_array_load">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_v4i32_ty], [IntrReadWriteArgMem]>;

  def int_AMDIL_image1d_array_read_norm : GCCBuiltin<"__amdil_image1d_array_read_norm">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_v4f32_ty], [IntrReadArgMem]>;

  def int_AMDIL_image1d_array_read_unnorm : GCCBuiltin<"__amdil_image1d_array_read_unnorm">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_v4f32_ty], [IntrReadArgMem]>;

  def int_AMDIL_image1d_array_info0 : GCCBuiltin<"__amdil_image1d_array_info0">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty], [IntrReadMem]>;

  def int_AMDIL_image1d_array_info1 : GCCBuiltin<"__amdil_image1d_array_info1">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty], [IntrReadMem]>;

  def int_AMDIL_image1d_buffer_write : GCCBuiltin<"__amdil_image1d_buffer_write">,
    Intrinsic<[], [llvm_ptr_ty, llvm_v2i32_ty, llvm_v4i32_ty], [IntrReadWriteArgMem]>;

  def int_AMDIL_image1d_buffer_load : GCCBuiltin<"__amdil_image1d_buffer_load">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty], [IntrReadArgMem]>;

  def int_AMDIL_image1d_buffer_read_norm : GCCBuiltin<"__amdil_image1d_buffer_read_norm">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_v4f32_ty], [IntrReadArgMem]>;

  def int_AMDIL_image1d_buffer_read_unnorm : GCCBuiltin<"__amdil_image1d_buffer_read_unnorm">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_v4f32_ty], [IntrReadArgMem]>;

  def int_AMDIL_image1d_buffer_info0 : GCCBuiltin<"__amdil_image1d_buffer_info0">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty], [IntrReadMem]>;

  def int_AMDIL_image1d_buffer_info1 : GCCBuiltin<"__amdil_image1d_buffer_info1">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty], [IntrReadMem]>;

  def int_AMDIL_image2d_write : GCCBuiltin<"__amdil_image2d_write">,
    Intrinsic<[], [llvm_ptr_ty, llvm_v2i32_ty, llvm_v4i32_ty], [IntrReadWriteArgMem]>;

  def int_AMDIL_image2d_load : GCCBuiltin<"__amdil_image2d_load">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_v4i32_ty], [IntrReadWriteArgMem]>;

  def int_AMDIL_image2d_read_norm : GCCBuiltin<"__amdil_image2d_read_norm">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_v4f32_ty], [IntrReadArgMem]>;

  def int_AMDIL_image2d_read_unnorm : GCCBuiltin<"__amdil_image2d_read_unnorm">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_v4f32_ty], [IntrReadArgMem]>;

  def int_AMDIL_image2d_info0 : GCCBuiltin<"__amdil_image2d_info0">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty], [IntrReadMem]>;

  def int_AMDIL_image2d_info1 : GCCBuiltin<"__amdil_image2d_info1">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty], [IntrReadMem]>;

  def int_AMDIL_image2d_array_write : GCCBuiltin<"__amdil_image2d_array_write">,
    Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty, llvm_v4i32_ty], [IntrReadWriteArgMem]>;

  def int_AMDIL_image2d_array_load : GCCBuiltin<"__amdil_image2d_array_load">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_v4i32_ty], [IntrReadWriteArgMem]>;

  def int_AMDIL_image2d_array_read_norm : GCCBuiltin<"__amdil_image2d_array_read_norm">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_v4f32_ty], [IntrReadArgMem]>;

  def int_AMDIL_image2d_array_read_unnorm : GCCBuiltin<"__amdil_image2d_array_read_unnorm">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_v4f32_ty], [IntrReadArgMem]>;

  def int_AMDIL_image2d_array_info0 : GCCBuiltin<"__amdil_image2d_array_info0">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty], [IntrReadMem]>;

  def int_AMDIL_image2d_array_info1 : GCCBuiltin<"__amdil_image2d_array_info1">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty], [IntrReadMem]>;

  def int_AMDIL_image3d_write : GCCBuiltin<"__amdil_image3d_write">,
    Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty, llvm_v4i32_ty], [IntrReadWriteArgMem]>;

  def int_AMDIL_image3d_load : GCCBuiltin<"__amdil_image3d_load">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_v4i32_ty], [IntrReadWriteArgMem]>;

  def int_AMDIL_image3d_read_norm : GCCBuiltin<"__amdil_image3d_read_norm">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_v4f32_ty], [IntrReadArgMem]>;

  def int_AMDIL_image3d_read_unnorm : GCCBuiltin<"__amdil_image3d_read_unnorm">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_v4f32_ty], [IntrReadArgMem]>;

  def int_AMDIL_image3d_info0 : GCCBuiltin<"__amdil_image3d_info0">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty], [IntrReadMem]>;

  def int_AMDIL_image3d_info1 : GCCBuiltin<"__amdil_image3d_info1">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty], [IntrReadMem]>;

//===---------------------- Image functions end --------------------------===//

  def int_AMDIL_append_alloc_i32 : GCCBuiltin<"__amdil_append_alloc">,
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty], [IntrReadWriteArgMem]>;
  def int_AMDIL_append_consume_i32 : GCCBuiltin<"__amdil_append_consume">,
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty], [IntrReadWriteArgMem]>;

  // The workitem functions may access the constant buffers, but that
  // is effectively not not accessing memory as far as anything would
  // be concerned.
  def int_AMDIL_get_global_id : GCCBuiltin<"__amdil_get_global_id_int">,
    Intrinsic<[llvm_v4i32_ty], [], [IntrNoMem]>;
  def int_AMDIL_get_local_id : GCCBuiltin<"__amdil_get_local_id_int">,
    Intrinsic<[llvm_v4i32_ty], [], [IntrNoMem]>;
  def int_AMDIL_get_group_id : GCCBuiltin<"__amdil_get_group_id_int">,
    Intrinsic<[llvm_v4i32_ty], [], [IntrNoMem]>;
  def int_AMDIL_get_num_groups : GCCBuiltin<"__amdil_get_num_groups_int">,
    Intrinsic<[llvm_v4i32_ty], [], [IntrNoMem]>;
  def int_AMDIL_get_local_size : GCCBuiltin<"__amdil_get_local_size_int">,
    Intrinsic<[llvm_v4i32_ty], [], [IntrNoMem]>;
  def int_AMDIL_get_global_size : GCCBuiltin<"__amdil_get_global_size_int">,
    Intrinsic<[llvm_v4i32_ty], [], [IntrNoMem]>;
  def int_AMDIL_get_global_offset : GCCBuiltin<"__amdil_get_global_offset_int">,
    Intrinsic<[llvm_v4i32_ty], [], [IntrNoMem]>;
  def int_AMDIL_get_work_dim : GCCBuiltin<"__amdil_get_work_dim_int">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem]>;
  def int_AMDIL_get_printf_offset : GCCBuiltin<"__amdil_get_printf_offset">,
    Intrinsic<[llvm_anyint_ty], [], [IntrReadMem]>;
  def int_AMDIL_get_printf_size : GCCBuiltin<"__amdil_get_printf_size">,
    Intrinsic<[llvm_anyint_ty], [], [IntrNoMem]>;
  def int_AMDIL_get_printf_item_offset : GCCBuiltin<"__amdil_get_printf_item_offset">,
    Intrinsic<[llvm_anyint_ty], [], [IntrReadMem]>;
  def int_AMDIL_inc_printf_item_offset : GCCBuiltin<"__amdil_inc_printf_item_offset">,
    Intrinsic<[], [llvm_anyint_ty], [IntrReadWriteArgMem]>;
  def int_AMDIL_get_region_num_groups : GCCBuiltin<"__amdil_get_region_num_groups_int">,
    Intrinsic<[llvm_v4i32_ty], [], [IntrNoMem]>;
  def int_AMDIL_get_sw_gws_offset : GCCBuiltin<"__amdil_get_sw_gws_offset">,
    Intrinsic<[llvm_anyint_ty], [], [IntrNoMem]>;
  /// Intrinsics for atomic instructions with no return value
  /// Signed 32 bit integer atomics for global address space
  def int_AMDIL_atomic_add_gi32_noret : GCCBuiltin<"__atomic_add_gi32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_sub_gi32_noret : GCCBuiltin<"__atomic_sub_gi32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_rsub_gi32_noret : GCCBuiltin<"__atomic_rsub_gi32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_xchg_gi32_noret : GCCBuiltin<"__atomic_xchg_gi32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_inc_gi32_noret : GCCBuiltin<"__atomic_inc_gi32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_dec_gi32_noret : GCCBuiltin<"__atomic_dec_gi32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_cmpxchg_gi32_noret : GCCBuiltin<"__atomic_cmpxchg_gi32_noret">,
    TernaryAtomicIntNoRet;
  def int_AMDIL_atomic_min_gi32_noret : GCCBuiltin<"__atomic_min_gi32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_max_gi32_noret : GCCBuiltin<"__atomic_max_gi32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_and_gi32_noret : GCCBuiltin<"__atomic_and_gi32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_or_gi32_noret : GCCBuiltin<"__atomic_or_gi32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_xor_gi32_noret : GCCBuiltin<"__atomic_xor_gi32_noret">,
    BinaryAtomicIntNoRet;

  // Atomic stores.
  def int_AMDIL_atomic_store_gv4i32 : GCCBuiltin<"__atomic_store_explicit_gv4i32">,
    Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  def int_AMDIL_atomic_store_gv4u32 : GCCBuiltin<"__atomic_store_explicit_gv4u32">,
    Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  def int_AMDIL_atomic_store_gv2i32 : GCCBuiltin<"__atomic_store_explicit_gv2i32">,
    Intrinsic<[], [llvm_ptr_ty, llvm_v2i32_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  def int_AMDIL_atomic_store_gv2u32 : GCCBuiltin<"__atomic_store_explicit_gv2u32">,
    Intrinsic<[], [llvm_ptr_ty, llvm_v2i32_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  def int_AMDIL_atomic_store_gi64 : GCCBuiltin<"__atomic_store_explicit_gi64">,
    Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  def int_AMDIL_atomic_store_gu64 : GCCBuiltin<"__atomic_store_explicit_gu64">,
    Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  def int_AMDIL_atomic_store_gi32 : GCCBuiltin<"__atomic_store_explicit_gi32">,
    Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  def int_AMDIL_atomic_store_gu32 : GCCBuiltin<"__atomic_store_explicit_gu32">,
    Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  def int_AMDIL_atomic_store_gi16 : GCCBuiltin<"__atomic_store_explicit_gi16">,
    Intrinsic<[], [llvm_ptr_ty, llvm_i16_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  def int_AMDIL_atomic_store_gu16 : GCCBuiltin<"__atomic_store_explicit_gu16">,
    Intrinsic<[], [llvm_ptr_ty, llvm_i16_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  def int_AMDIL_atomic_store_gi8 : GCCBuiltin<"__atomic_store_explicit_gi8">,
    Intrinsic<[], [llvm_ptr_ty, llvm_i8_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  def int_AMDIL_atomic_store_gu8 : GCCBuiltin<"__atomic_store_explicit_gu8">,
    Intrinsic<[], [llvm_ptr_ty, llvm_i8_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;

  // Atomic loads.
  def int_AMDIL_atomic_load_gv4i32 : GCCBuiltin<"__atomic_load_explicit_gv4i32">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty], [IntrReadArgMem]>;
  def int_AMDIL_atomic_load_gv4u32 : GCCBuiltin<"__atomic_load_explicit_gv4u32">,
    Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i32_ty], [IntrReadArgMem]>;
  def int_AMDIL_atomic_load_gv2i32 : GCCBuiltin<"__atomic_load_explicit_gv2i32">,
    Intrinsic<[llvm_v2i32_ty], [llvm_ptr_ty, llvm_i32_ty], [IntrReadArgMem]>;
  def int_AMDIL_atomic_load_gv2u32 : GCCBuiltin<"__atomic_load_explicit_gv2u32">,
    Intrinsic<[llvm_v2i32_ty], [llvm_ptr_ty, llvm_i32_ty], [IntrReadArgMem]>;
  def int_AMDIL_atomic_load_gi64 : GCCBuiltin<"__atomic_load_explicit_gi64">,
    Intrinsic<[llvm_i64_ty], [llvm_ptr_ty, llvm_i32_ty], [IntrReadArgMem]>;
  def int_AMDIL_atomic_load_gu64 : GCCBuiltin<"__atomic_load_explicit_gu64">,
    Intrinsic<[llvm_i64_ty], [llvm_ptr_ty, llvm_i32_ty], [IntrReadArgMem]>;
  def int_AMDIL_atomic_load_gi32 : GCCBuiltin<"__atomic_load_explicit_gi32">,
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty, llvm_i32_ty], [IntrReadArgMem]>;
  def int_AMDIL_atomic_load_gu32 : GCCBuiltin<"__atomic_load_explicit_gu32">,
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty, llvm_i32_ty], [IntrReadArgMem]>;
  def int_AMDIL_atomic_load_gi16 : GCCBuiltin<"__atomic_load_explicit_gi16">,
    Intrinsic<[llvm_i16_ty], [llvm_ptr_ty, llvm_i32_ty], [IntrReadArgMem]>;
  def int_AMDIL_atomic_load_gu16 : GCCBuiltin<"__atomic_load_explicit_gu16">,
    Intrinsic<[llvm_i16_ty], [llvm_ptr_ty, llvm_i32_ty], [IntrReadArgMem]>;
  def int_AMDIL_atomic_load_gi8 : GCCBuiltin<"__atomic_load_explicit_gi8">,
    Intrinsic<[llvm_i8_ty], [llvm_ptr_ty, llvm_i32_ty], [IntrReadArgMem]>;
  def int_AMDIL_atomic_load_gu8 : GCCBuiltin<"__atomic_load_explicit_gu8">,
    Intrinsic<[llvm_i8_ty], [llvm_ptr_ty, llvm_i32_ty], [IntrReadArgMem]>;

  /// Unsigned 32 bit integer atomics for global address space
  def int_AMDIL_atomic_add_gu32_noret : GCCBuiltin<"__atomic_add_gu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_sub_gu32_noret : GCCBuiltin<"__atomic_sub_gu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_rsub_gu32_noret : GCCBuiltin<"__atomic_rsub_gu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_xchg_gu32_noret : GCCBuiltin<"__atomic_xchg_gu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_inc_gu32_noret : GCCBuiltin<"__atomic_inc_gu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_dec_gu32_noret : GCCBuiltin<"__atomic_dec_gu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_cmpxchg_gu32_noret : GCCBuiltin<"__atomic_cmpxchg_gu32_noret">,
    TernaryAtomicIntNoRet;
  def int_AMDIL_atomic_min_gu32_noret : GCCBuiltin<"__atomic_min_gu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_max_gu32_noret : GCCBuiltin<"__atomic_max_gu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_and_gu32_noret : GCCBuiltin<"__atomic_and_gu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_or_gu32_noret : GCCBuiltin<"__atomic_or_gu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_xor_gu32_noret : GCCBuiltin<"__atomic_xor_gu32_noret">,
    BinaryAtomicIntNoRet;


  /// Intrinsics for atomic instructions with a return value
  /// Signed 32 bit integer atomics for global address space
  def int_AMDIL_atomic_add_gi32 : GCCBuiltin<"__atomic_add_gi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_sub_gi32 : GCCBuiltin<"__atomic_sub_gi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_rsub_gi32 : GCCBuiltin<"__atomic_rsub_gi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_gi32 : GCCBuiltin<"__atomic_xchg_gi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_inc_gi32 : GCCBuiltin<"__atomic_inc_gi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_dec_gi32 : GCCBuiltin<"__atomic_dec_gi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_cmpxchg_gi32 : GCCBuiltin<"__atomic_cmpxchg_gi32">,
    TernaryAtomicInt;
  def int_AMDIL_atomic_min_gi32 : GCCBuiltin<"__atomic_min_gi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_max_gi32 : GCCBuiltin<"__atomic_max_gi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_and_gi32 : GCCBuiltin<"__atomic_and_gi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_or_gi32 : GCCBuiltin<"__atomic_or_gi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xor_gi32 : GCCBuiltin<"__atomic_xor_gi32">,
    BinaryAtomicInt;

  /// 32 bit float atomics required by OpenCL
  def int_AMDIL_atomic_xchg_gf32 : GCCBuiltin<"__atomic_xchg_gf32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_gf32_noret : GCCBuiltin<"__atomic_xchg_gf32_noret">,
    BinaryAtomicIntNoRet;

  /// Unsigned 32 bit integer atomics for global address space
  def int_AMDIL_atomic_add_gu32 : GCCBuiltin<"__atomic_add_gu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_sub_gu32 : GCCBuiltin<"__atomic_sub_gu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_rsub_gu32 : GCCBuiltin<"__atomic_rsub_gu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_gu32 : GCCBuiltin<"__atomic_xchg_gu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_inc_gu32 : GCCBuiltin<"__atomic_inc_gu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_dec_gu32 : GCCBuiltin<"__atomic_dec_gu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_cmpxchg_gu32 : GCCBuiltin<"__atomic_cmpxchg_gu32">,
    TernaryAtomicInt;
  def int_AMDIL_atomic_min_gu32 : GCCBuiltin<"__atomic_min_gu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_max_gu32 : GCCBuiltin<"__atomic_max_gu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_and_gu32 : GCCBuiltin<"__atomic_and_gu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_or_gu32 : GCCBuiltin<"__atomic_or_gu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xor_gu32 : GCCBuiltin<"__atomic_xor_gu32">,
    BinaryAtomicInt;


  /// Intrinsics for atomic instructions with no return value
  /// Signed 32 bit integer atomics for local address space
  def int_AMDIL_atomic_add_li32_noret : GCCBuiltin<"__atomic_add_li32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_sub_li32_noret : GCCBuiltin<"__atomic_sub_li32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_rsub_li32_noret : GCCBuiltin<"__atomic_rsub_li32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_xchg_li32_noret : GCCBuiltin<"__atomic_xchg_li32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_inc_li32_noret : GCCBuiltin<"__atomic_inc_li32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_dec_li32_noret : GCCBuiltin<"__atomic_dec_li32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_cmpxchg_li32_noret : GCCBuiltin<"__atomic_cmpxchg_li32_noret">,
    TernaryAtomicIntNoRet;
  def int_AMDIL_atomic_min_li32_noret : GCCBuiltin<"__atomic_min_li32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_max_li32_noret : GCCBuiltin<"__atomic_max_li32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_and_li32_noret : GCCBuiltin<"__atomic_and_li32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_or_li32_noret : GCCBuiltin<"__atomic_or_li32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_mskor_li32_noret : GCCBuiltin<"__atomic_mskor_li32_noret">,
    TernaryAtomicIntNoRet;
  def int_AMDIL_atomic_xor_li32_noret : GCCBuiltin<"__atomic_xor_li32_noret">,
    BinaryAtomicIntNoRet;

  /// Signed 32 bit integer atomics for region address space
  def int_AMDIL_atomic_add_ri32_noret : GCCBuiltin<"__atomic_add_ri32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_sub_ri32_noret : GCCBuiltin<"__atomic_sub_ri32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_rsub_ri32_noret : GCCBuiltin<"__atomic_rsub_ri32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_xchg_ri32_noret : GCCBuiltin<"__atomic_xchg_ri32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_inc_ri32_noret : GCCBuiltin<"__atomic_inc_ri32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_dec_ri32_noret : GCCBuiltin<"__atomic_dec_ri32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_cmpxchg_ri32_noret : GCCBuiltin<"__atomic_cmpxchg_ri32_noret">,
    TernaryAtomicIntNoRet;
  def int_AMDIL_atomic_min_ri32_noret : GCCBuiltin<"__atomic_min_ri32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_max_ri32_noret : GCCBuiltin<"__atomic_max_ri32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_and_ri32_noret : GCCBuiltin<"__atomic_and_ri32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_or_ri32_noret : GCCBuiltin<"__atomic_or_ri32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_mskor_ri32_noret : GCCBuiltin<"__atomic_mskor_ri32_noret">,
    TernaryAtomicIntNoRet;
  def int_AMDIL_atomic_xor_ri32_noret : GCCBuiltin<"__atomic_xor_ri32_noret">,
    BinaryAtomicIntNoRet;


  /// Unsigned 32 bit integer atomics for local address space
  def int_AMDIL_atomic_add_lu32_noret : GCCBuiltin<"__atomic_add_lu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_sub_lu32_noret : GCCBuiltin<"__atomic_sub_lu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_rsub_lu32_noret : GCCBuiltin<"__atomic_rsub_lu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_xchg_lu32_noret : GCCBuiltin<"__atomic_xchg_lu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_inc_lu32_noret : GCCBuiltin<"__atomic_inc_lu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_dec_lu32_noret : GCCBuiltin<"__atomic_dec_lu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_cmpxchg_lu32_noret : GCCBuiltin<"__atomic_cmpxchg_lu32_noret">,
    TernaryAtomicIntNoRet;
  def int_AMDIL_atomic_min_lu32_noret : GCCBuiltin<"__atomic_min_lu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_max_lu32_noret : GCCBuiltin<"__atomic_max_lu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_and_lu32_noret : GCCBuiltin<"__atomic_and_lu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_or_lu32_noret : GCCBuiltin<"__atomic_or_lu32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_mskor_lu32_noret : GCCBuiltin<"__atomic_mskor_lu32_noret">,
    TernaryAtomicIntNoRet;
  def int_AMDIL_atomic_xor_lu32_noret : GCCBuiltin<"__atomic_xor_lu32_noret">,
    BinaryAtomicIntNoRet;

  /// Unsigned 32 bit integer atomics for region address space
  def int_AMDIL_atomic_add_ru32_noret : GCCBuiltin<"__atomic_add_ru32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_sub_ru32_noret : GCCBuiltin<"__atomic_sub_ru32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_rsub_ru32_noret : GCCBuiltin<"__atomic_rsub_ru32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_xchg_ru32_noret : GCCBuiltin<"__atomic_xchg_ru32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_inc_ru32_noret : GCCBuiltin<"__atomic_inc_ru32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_dec_ru32_noret : GCCBuiltin<"__atomic_dec_ru32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_cmpxchg_ru32_noret : GCCBuiltin<"__atomic_cmpxchg_ru32_noret">,
    TernaryAtomicIntNoRet;
  def int_AMDIL_atomic_min_ru32_noret : GCCBuiltin<"__atomic_min_ru32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_max_ru32_noret : GCCBuiltin<"__atomic_max_ru32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_and_ru32_noret : GCCBuiltin<"__atomic_and_ru32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_or_ru32_noret : GCCBuiltin<"__atomic_or_ru32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_mskor_ru32_noret : GCCBuiltin<"__atomic_mskor_ru32_noret">,
    TernaryAtomicIntNoRet;
  def int_AMDIL_atomic_xor_ru32_noret : GCCBuiltin<"__atomic_xor_ru32_noret">,
    BinaryAtomicIntNoRet;

  // TODO: Maybe it would be best to assume get_cycle_count has side
  // effects to fully prevent it from being reordered?
  def int_AMDIL_get_cycle_count : GCCBuiltin<"__amdil_get_cycle_count">,
    Intrinsic<[llvm_i64_ty], [], [IntrReadMem]>;
  def int_AMDIL_compute_unit_id : GCCBuiltin<"__amdil_compute_unit_id">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem]>;
  def int_AMDIL_wavefront_id : GCCBuiltin<"__amdil_wavefront_id">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem]>;

  // Poison the output register.
  def int_AMDIL_poisonv : GCCBuiltin<"__amdil_poisonv">,
    Intrinsic<[llvm_anyint_ty], [LLVMMatchType<0>], [IntrReadMem]>;

  /// Intrinsics for atomic instructions with a return value.
  /// Signed 32 bit integer atomics for local address space.
  def int_AMDIL_atomic_add_li32 : GCCBuiltin<"__atomic_add_li32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_sub_li32 : GCCBuiltin<"__atomic_sub_li32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_rsub_li32 : GCCBuiltin<"__atomic_rsub_li32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_li32 : GCCBuiltin<"__atomic_xchg_li32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_inc_li32 : GCCBuiltin<"__atomic_inc_li32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_dec_li32 : GCCBuiltin<"__atomic_dec_li32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_cmpxchg_li32 : GCCBuiltin<"__atomic_cmpxchg_li32">,
    TernaryAtomicInt;
  def int_AMDIL_atomic_min_li32 : GCCBuiltin<"__atomic_min_li32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_max_li32 : GCCBuiltin<"__atomic_max_li32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_and_li32 : GCCBuiltin<"__atomic_and_li32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_or_li32 : GCCBuiltin<"__atomic_or_li32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_mskor_li32 : GCCBuiltin<"__atomic_mskor_li32">,
    TernaryAtomicInt;
  def int_AMDIL_atomic_xor_li32 : GCCBuiltin<"__atomic_xor_li32">,
    BinaryAtomicInt;

  /// Signed 32 bit integer atomics for region address space.
  def int_AMDIL_atomic_add_ri32 : GCCBuiltin<"__atomic_add_ri32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_sub_ri32 : GCCBuiltin<"__atomic_sub_ri32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_rsub_ri32 : GCCBuiltin<"__atomic_rsub_ri32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_ri32 : GCCBuiltin<"__atomic_xchg_ri32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_inc_ri32 : GCCBuiltin<"__atomic_inc_ri32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_dec_ri32 : GCCBuiltin<"__atomic_dec_ri32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_cmpxchg_ri32 : GCCBuiltin<"__atomic_cmpxchg_ri32">,
    TernaryAtomicInt;
  def int_AMDIL_atomic_min_ri32 : GCCBuiltin<"__atomic_min_ri32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_max_ri32 : GCCBuiltin<"__atomic_max_ri32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_and_ri32 : GCCBuiltin<"__atomic_and_ri32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_or_ri32 : GCCBuiltin<"__atomic_or_ri32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_mskor_ri32 : GCCBuiltin<"__atomic_mskor_ri32">,
    TernaryAtomicInt;
  def int_AMDIL_atomic_xor_ri32 : GCCBuiltin<"__atomic_xor_ri32">,
    BinaryAtomicInt;

  /// 32 bit float atomics required by OpenCL
  def int_AMDIL_atomic_xchg_lf32 : GCCBuiltin<"__atomic_xchg_lf32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_lf32_noret : GCCBuiltin<"__atomic_xchg_lf32_noret">,
    BinaryAtomicIntNoRet;
  def int_AMDIL_atomic_xchg_rf32 : GCCBuiltin<"__atomic_xchg_rf32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_rf32_noret : GCCBuiltin<"__atomic_xchg_rf32_noret">,
    BinaryAtomicIntNoRet;

  /// Unsigned 32 bit integer atomics for local address space
  def int_AMDIL_atomic_add_lu32 : GCCBuiltin<"__atomic_add_lu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_sub_lu32 : GCCBuiltin<"__atomic_sub_lu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_rsub_lu32 : GCCBuiltin<"__atomic_rsub_lu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_lu32 : GCCBuiltin<"__atomic_xchg_lu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_inc_lu32 : GCCBuiltin<"__atomic_inc_lu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_dec_lu32 : GCCBuiltin<"__atomic_dec_lu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_cmpxchg_lu32 : GCCBuiltin<"__atomic_cmpxchg_lu32">,
    TernaryAtomicInt;
  def int_AMDIL_atomic_min_lu32 : GCCBuiltin<"__atomic_min_lu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_max_lu32 : GCCBuiltin<"__atomic_max_lu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_and_lu32 : GCCBuiltin<"__atomic_and_lu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_or_lu32 : GCCBuiltin<"__atomic_or_lu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_mskor_lu32 : GCCBuiltin<"__atomic_mskor_lu32">,
    TernaryAtomicInt;
  def int_AMDIL_atomic_xor_lu32 : GCCBuiltin<"__atomic_xor_lu32">,
    BinaryAtomicInt;

  /// Unsigned 32 bit integer atomics for region address space.
  def int_AMDIL_atomic_add_ru32 : GCCBuiltin<"__atomic_add_ru32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_sub_ru32 : GCCBuiltin<"__atomic_sub_ru32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_rsub_ru32 : GCCBuiltin<"__atomic_rsub_ru32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_ru32 : GCCBuiltin<"__atomic_xchg_ru32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_inc_ru32 : GCCBuiltin<"__atomic_inc_ru32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_dec_ru32 : GCCBuiltin<"__atomic_dec_ru32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_cmpxchg_ru32 : GCCBuiltin<"__atomic_cmpxchg_ru32">,
    TernaryAtomicInt;
  def int_AMDIL_atomic_min_ru32 : GCCBuiltin<"__atomic_min_ru32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_max_ru32 : GCCBuiltin<"__atomic_max_ru32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_and_ru32 : GCCBuiltin<"__atomic_and_ru32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_or_ru32 : GCCBuiltin<"__atomic_or_ru32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_mskor_ru32 : GCCBuiltin<"__atomic_mskor_ru32">,
    TernaryAtomicInt;
  def int_AMDIL_atomic_xor_ru32 : GCCBuiltin<"__atomic_xor_ru32">,
    BinaryAtomicInt;

  /// Semaphore signal/wait.
  def int_AMDIL_semaphore_wait : GCCBuiltin<"__amdil_semaphore_wait">,
    VoidAtomicIntNoRet;
  def int_AMDIL_semaphore_signal : GCCBuiltin<"__amdil_semaphore_signal">,
    VoidAtomicIntNoRet;
  def int_AMDIL_max_semaphore_size : GCCBuiltin<"__amdil_max_semaphore_size">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem]>;

  def int_AMDIL_semaphore_wait_imm : GCCBuiltin<"__amdil_semaphore_wait_imm">,
    UnaryIntNoRetInt;
  def int_AMDIL_semaphore_signal_imm : GCCBuiltin<"__amdil_semaphore_signal_imm">,
    UnaryIntNoRetInt;

  /// Intrinsics for atomic instructions with no return value.
  /// Signed 64 bit integer atomics for global address space.
  def int_AMDIL_atomic_add_gi64_noret : GCCBuiltin<"__atomic_add_gi64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_sub_gi64_noret : GCCBuiltin<"__atomic_sub_gi64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_rsub_gi64_noret : GCCBuiltin<"__atomic_rsub_gi64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_xchg_gi64_noret : GCCBuiltin<"__atomic_xchg_gi64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_inc_gi64_noret : GCCBuiltin<"__atomic_inc_gi64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_dec_gi64_noret : GCCBuiltin<"__atomic_dec_gi64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_cmpxchg_gi64_noret : GCCBuiltin<"__atomic_cmpxchg_gi64_noret">,
    TernaryAtomicLongNoRet;
  def int_AMDIL_atomic_min_gi64_noret : GCCBuiltin<"__atomic_min_gi64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_max_gi64_noret : GCCBuiltin<"__atomic_max_gi64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_and_gi64_noret : GCCBuiltin<"__atomic_and_gi64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_or_gi64_noret : GCCBuiltin<"__atomic_or_gi64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_xor_gi64_noret : GCCBuiltin<"__atomic_xor_gi64_noret">,
    BinaryAtomicLongNoRet;


  /// Unsigned 64 bit integer atomics for global address space.
  def int_AMDIL_atomic_add_gu64_noret : GCCBuiltin<"__atomic_add_gu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_sub_gu64_noret : GCCBuiltin<"__atomic_sub_gu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_rsub_gu64_noret : GCCBuiltin<"__atomic_rsub_gu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_xchg_gu64_noret : GCCBuiltin<"__atomic_xchg_gu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_inc_gu64_noret : GCCBuiltin<"__atomic_inc_gu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_dec_gu64_noret : GCCBuiltin<"__atomic_dec_gu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_cmpxchg_gu64_noret : GCCBuiltin<"__atomic_cmpxchg_gu64_noret">,
    TernaryAtomicLongNoRet;
  def int_AMDIL_atomic_min_gu64_noret : GCCBuiltin<"__atomic_min_gu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_max_gu64_noret : GCCBuiltin<"__atomic_max_gu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_and_gu64_noret : GCCBuiltin<"__atomic_and_gu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_or_gu64_noret : GCCBuiltin<"__atomic_or_gu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_xor_gu64_noret : GCCBuiltin<"__atomic_xor_gu64_noret">,
    BinaryAtomicLongNoRet;


  /// Intrinsics for atomic instructions with a return value.
  /// Signed 64 bit integer atomics for global address space.
  def int_AMDIL_atomic_add_gi64 : GCCBuiltin<"__atomic_add_gi64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_sub_gi64 : GCCBuiltin<"__atomic_sub_gi64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_rsub_gi64 : GCCBuiltin<"__atomic_rsub_gi64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_xchg_gi64 : GCCBuiltin<"__atomic_xchg_gi64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_inc_gi64 : GCCBuiltin<"__atomic_inc_gi64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_dec_gi64 : GCCBuiltin<"__atomic_dec_gi64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_cmpxchg_gi64 : GCCBuiltin<"__atomic_cmpxchg_gi64">,
    TernaryAtomicLong;
  def int_AMDIL_atomic_min_gi64 : GCCBuiltin<"__atomic_min_gi64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_max_gi64 : GCCBuiltin<"__atomic_max_gi64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_and_gi64 : GCCBuiltin<"__atomic_and_gi64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_or_gi64 : GCCBuiltin<"__atomic_or_gi64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_xor_gi64 : GCCBuiltin<"__atomic_xor_gi64">,
    BinaryAtomicLong;

  /// Unsigned 64 bit integer atomics for global address space.
  def int_AMDIL_atomic_add_gu64 : GCCBuiltin<"__atomic_add_gu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_sub_gu64 : GCCBuiltin<"__atomic_sub_gu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_rsub_gu64 : GCCBuiltin<"__atomic_rsub_gu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_xchg_gu64 : GCCBuiltin<"__atomic_xchg_gu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_inc_gu64 : GCCBuiltin<"__atomic_inc_gu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_dec_gu64 : GCCBuiltin<"__atomic_dec_gu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_cmpxchg_gu64 : GCCBuiltin<"__atomic_cmpxchg_gu64">,
    TernaryAtomicLong;
  def int_AMDIL_atomic_min_gu64 : GCCBuiltin<"__atomic_min_gu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_max_gu64 : GCCBuiltin<"__atomic_max_gu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_and_gu64 : GCCBuiltin<"__atomic_and_gu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_or_gu64 : GCCBuiltin<"__atomic_or_gu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_xor_gu64 : GCCBuiltin<"__atomic_xor_gu64">,
    BinaryAtomicLong;

  /// Intrinsics for atomic instructions with no return value.
  /// Signed 64 bit integer atomics for local address space.
  def int_AMDIL_atomic_add_li64_noret : GCCBuiltin<"__atomic_add_li64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_sub_li64_noret : GCCBuiltin<"__atomic_sub_li64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_rsub_li64_noret : GCCBuiltin<"__atomic_rsub_li64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_xchg_li64_noret : GCCBuiltin<"__atomic_xchg_li64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_inc_li64_noret : GCCBuiltin<"__atomic_inc_li64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_dec_li64_noret : GCCBuiltin<"__atomic_dec_li64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_cmpxchg_li64_noret : GCCBuiltin<"__atomic_cmpxchg_li64_noret">,
    TernaryAtomicLongNoRet;
  def int_AMDIL_atomic_min_li64_noret : GCCBuiltin<"__atomic_min_li64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_max_li64_noret : GCCBuiltin<"__atomic_max_li64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_and_li64_noret : GCCBuiltin<"__atomic_and_li64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_or_li64_noret : GCCBuiltin<"__atomic_or_li64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_mskor_li64_noret : GCCBuiltin<"__atomic_mskor_li64_noret">,
    TernaryAtomicLongNoRet;
  def int_AMDIL_atomic_xor_li64_noret : GCCBuiltin<"__atomic_xor_li64_noret">,
    BinaryAtomicLongNoRet;

  /// Signed 64 bit integer atomics for region address space.
  def int_AMDIL_atomic_add_ri64_noret : GCCBuiltin<"__atomic_add_ri64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_sub_ri64_noret : GCCBuiltin<"__atomic_sub_ri64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_rsub_ri64_noret : GCCBuiltin<"__atomic_rsub_ri64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_xchg_ri64_noret : GCCBuiltin<"__atomic_xchg_ri64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_inc_ri64_noret : GCCBuiltin<"__atomic_inc_ri64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_dec_ri64_noret : GCCBuiltin<"__atomic_dec_ri64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_cmpxchg_ri64_noret : GCCBuiltin<"__atomic_cmpxchg_ri64_noret">,
    TernaryAtomicLongNoRet;
  def int_AMDIL_atomic_min_ri64_noret : GCCBuiltin<"__atomic_min_ri64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_max_ri64_noret : GCCBuiltin<"__atomic_max_ri64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_and_ri64_noret : GCCBuiltin<"__atomic_and_ri64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_or_ri64_noret : GCCBuiltin<"__atomic_or_ri64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_mskor_ri64_noret : GCCBuiltin<"__atomic_mskor_ri64_noret">,
    TernaryAtomicLongNoRet;
  def int_AMDIL_atomic_xor_ri64_noret : GCCBuiltin<"__atomic_xor_ri64_noret">,
    BinaryAtomicLongNoRet;

  /// Unsigned 64 bit integer atomics for local address space.
  def int_AMDIL_atomic_add_lu64_noret : GCCBuiltin<"__atomic_add_lu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_sub_lu64_noret : GCCBuiltin<"__atomic_sub_lu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_rsub_lu64_noret : GCCBuiltin<"__atomic_rsub_lu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_xchg_lu64_noret : GCCBuiltin<"__atomic_xchg_lu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_inc_lu64_noret : GCCBuiltin<"__atomic_inc_lu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_dec_lu64_noret : GCCBuiltin<"__atomic_dec_lu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_cmpxchg_lu64_noret : GCCBuiltin<"__atomic_cmpxchg_lu64_noret">,
    TernaryAtomicLongNoRet;
  def int_AMDIL_atomic_min_lu64_noret : GCCBuiltin<"__atomic_min_lu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_max_lu64_noret : GCCBuiltin<"__atomic_max_lu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_and_lu64_noret : GCCBuiltin<"__atomic_and_lu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_or_lu64_noret : GCCBuiltin<"__atomic_or_lu64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_mskor_lu64_noret : GCCBuiltin<"__atomic_mskor_lu64_noret">,
    TernaryAtomicLongNoRet;
  def int_AMDIL_atomic_xor_lu64_noret : GCCBuiltin<"__atomic_xor_lu64_noret">,
    BinaryAtomicLongNoRet;

  /// Unsigned 64 bit integer atomics for region address space.
  def int_AMDIL_atomic_add_ru64_noret : GCCBuiltin<"__atomic_add_ru64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_sub_ru64_noret : GCCBuiltin<"__atomic_sub_ru64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_rsub_ru64_noret : GCCBuiltin<"__atomic_rsub_ru64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_xchg_ru64_noret : GCCBuiltin<"__atomic_xchg_ru64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_inc_ru64_noret : GCCBuiltin<"__atomic_inc_ru64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_dec_ru64_noret : GCCBuiltin<"__atomic_dec_ru64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_cmpxchg_ru64_noret : GCCBuiltin<"__atomic_cmpxchg_ru64_noret">,
    TernaryAtomicLongNoRet;
  def int_AMDIL_atomic_min_ru64_noret : GCCBuiltin<"__atomic_min_ru64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_max_ru64_noret : GCCBuiltin<"__atomic_max_ru64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_and_ru64_noret : GCCBuiltin<"__atomic_and_ru64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_or_ru64_noret : GCCBuiltin<"__atomic_or_ru64_noret">,
    BinaryAtomicLongNoRet;
  def int_AMDIL_atomic_mskor_ru64_noret : GCCBuiltin<"__atomic_mskor_ru64_noret">,
    TernaryAtomicLongNoRet;
  def int_AMDIL_atomic_xor_ru64_noret : GCCBuiltin<"__atomic_xor_ru64_noret">,
    BinaryAtomicLongNoRet;

  /// Intrinsics for atomic instructions with a return value
  /// Signed 64 bit integer atomics for local address space
  def int_AMDIL_atomic_add_li64 : GCCBuiltin<"__atomic_add_li64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_sub_li64 : GCCBuiltin<"__atomic_sub_li64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_rsub_li64 : GCCBuiltin<"__atomic_rsub_li64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_xchg_li64 : GCCBuiltin<"__atomic_xchg_li64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_inc_li64 : GCCBuiltin<"__atomic_inc_li64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_dec_li64 : GCCBuiltin<"__atomic_dec_li64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_cmpxchg_li64 : GCCBuiltin<"__atomic_cmpxchg_li64">,
    TernaryAtomicLong;
  def int_AMDIL_atomic_min_li64 : GCCBuiltin<"__atomic_min_li64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_max_li64 : GCCBuiltin<"__atomic_max_li64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_and_li64 : GCCBuiltin<"__atomic_and_li64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_or_li64 : GCCBuiltin<"__atomic_or_li64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_mskor_li64 : GCCBuiltin<"__atomic_mskor_li64">,
    TernaryAtomicLong;
  def int_AMDIL_atomic_xor_li64 : GCCBuiltin<"__atomic_xor_li64">,
    BinaryAtomicLong;

  /// Signed 64 bit integer atomics for region address space
  def int_AMDIL_atomic_add_ri64 : GCCBuiltin<"__atomic_add_ri64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_sub_ri64 : GCCBuiltin<"__atomic_sub_ri64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_rsub_ri64 : GCCBuiltin<"__atomic_rsub_ri64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_xchg_ri64 : GCCBuiltin<"__atomic_xchg_ri64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_inc_ri64 : GCCBuiltin<"__atomic_inc_ri64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_dec_ri64 : GCCBuiltin<"__atomic_dec_ri64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_cmpxchg_ri64 : GCCBuiltin<"__atomic_cmpxchg_ri64">,
    TernaryAtomicLong;
  def int_AMDIL_atomic_min_ri64 : GCCBuiltin<"__atomic_min_ri64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_max_ri64 : GCCBuiltin<"__atomic_max_ri64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_and_ri64 : GCCBuiltin<"__atomic_and_ri64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_or_ri64 : GCCBuiltin<"__atomic_or_ri64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_mskor_ri64 : GCCBuiltin<"__atomic_mskor_ri64">,
    TernaryAtomicLong;
  def int_AMDIL_atomic_xor_ri64 : GCCBuiltin<"__atomic_xor_ri64">,
    BinaryAtomicLong;

  /// Unsigned 64 bit integer atomics for local address space.
  def int_AMDIL_atomic_add_lu64 : GCCBuiltin<"__atomic_add_lu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_sub_lu64 : GCCBuiltin<"__atomic_sub_lu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_rsub_lu64 : GCCBuiltin<"__atomic_rsub_lu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_xchg_lu64 : GCCBuiltin<"__atomic_xchg_lu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_inc_lu64 : GCCBuiltin<"__atomic_inc_lu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_dec_lu64 : GCCBuiltin<"__atomic_dec_lu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_cmpxchg_lu64 : GCCBuiltin<"__atomic_cmpxchg_lu64">,
    TernaryAtomicLong;
  def int_AMDIL_atomic_min_lu64 : GCCBuiltin<"__atomic_min_lu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_max_lu64 : GCCBuiltin<"__atomic_max_lu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_and_lu64 : GCCBuiltin<"__atomic_and_lu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_or_lu64 : GCCBuiltin<"__atomic_or_lu64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_mskor_lu64 : GCCBuiltin<"__atomic_mskor_lu64">,
    TernaryAtomicLong;
  def int_AMDIL_atomic_xor_lu64 : GCCBuiltin<"__atomic_xor_lu64">,
    BinaryAtomicLong;

  /// Unsigned 64 bit integer atomics for region address space
  def int_AMDIL_atomic_add_ru64 : GCCBuiltin<"__atomic_add_ru64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_sub_ru64 : GCCBuiltin<"__atomic_sub_ru64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_rsub_ru64 : GCCBuiltin<"__atomic_rsub_ru64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_xchg_ru64 : GCCBuiltin<"__atomic_xchg_ru64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_inc_ru64 : GCCBuiltin<"__atomic_inc_ru64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_dec_ru64 : GCCBuiltin<"__atomic_dec_ru64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_cmpxchg_ru64 : GCCBuiltin<"__atomic_cmpxchg_ru64">,
    TernaryAtomicLong;
  def int_AMDIL_atomic_min_ru64 : GCCBuiltin<"__atomic_min_ru64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_max_ru64 : GCCBuiltin<"__atomic_max_ru64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_and_ru64 : GCCBuiltin<"__atomic_and_ru64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_or_ru64 : GCCBuiltin<"__atomic_or_ru64">,
    BinaryAtomicLong;
  def int_AMDIL_atomic_mskor_ru64 : GCCBuiltin<"__atomic_mskor_ru64">,
    TernaryAtomicLong;
  def int_AMDIL_atomic_xor_ru64 : GCCBuiltin<"__atomic_xor_ru64">,
    BinaryAtomicLong;

  def int_AMDIL_min3_i32 : GCCBuiltin<"__amdil_imin3">,
    TernaryIntInt;
  def int_AMDIL_min3_u32 : GCCBuiltin<"__amdil_umin3">,
    TernaryIntInt;
  def int_AMDIL_min3 : GCCBuiltin<"__amdil_min3">,
    TernaryIntFloat;
  def int_AMDIL_max3_i32 : GCCBuiltin<"__amdil_imax3">,
    TernaryIntInt;
  def int_AMDIL_max3_u32 : GCCBuiltin<"__amdil_umax3">,
    TernaryIntInt;
  def int_AMDIL_max3 : GCCBuiltin<"__amdil_max3">,
    TernaryIntFloat;
  def int_AMDIL_med3_i32 : GCCBuiltin<"__amdil_imed3">,
    TernaryIntInt;
  def int_AMDIL_med3_u32 : GCCBuiltin<"__amdil_umed3">,
    TernaryIntInt;
  def int_AMDIL_med3 : GCCBuiltin<"__amdil_med3">,
    TernaryIntFloat;
  def int_AMDIL_class : GCCBuiltin<"__amdil_class">,
    Intrinsic<[llvm_anyint_ty], [llvm_anyfloat_ty, llvm_anyint_ty], [IntrNoMem]>;
  def int_AMDIL_trig_preop_f64 : GCCBuiltin<"__amdil_trig_preop">,
      Intrinsic<[llvm_double_ty], [llvm_double_ty, llvm_float_ty], [IntrNoMem]>;

  def int_AMDIL_frexp_exp : GCCBuiltin<"__amdil_frexp_exp">,
    ConvertIntFTOI;
  def int_AMDIL_frexp_mant : GCCBuiltin<"__amdil_frexp_mant">,
    UnaryIntFloat;
  def int_AMDIL_media_sad16 : GCCBuiltin<"__amdil_sad16">,
    TernaryIntInt;
  def int_AMDIL_media_sad32 : GCCBuiltin<"__amdil_sad32">,
    TernaryIntInt;

  def int_AMDIL_media_msad : GCCBuiltin<"__amdil_msad">,
    TernaryIntInt;
  def int_AMDIL_media_qsad : GCCBuiltin<"__amdil_qsad">,
    Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i32_ty, llvm_i64_ty], [IntrNoMem]>;
  def int_AMDIL_media_mqsad : GCCBuiltin<"__amdil_mqsad">,
    Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i32_ty, llvm_i64_ty], [IntrNoMem]>;

  /// Rounding and flushing control intrinsics.
  /// Floating point addition - round to even.
  let Properties = [Commutative, IntrNoMem] in {
    def int_AMDIL_add_rte : GCCBuiltin<"__amdil_add_rte">,
      BinaryIntFloat;
    /// Floating point addition - round to positive infinity
    def int_AMDIL_add_rtp : GCCBuiltin<"__amdil_add_rtp">,
      BinaryIntFloat;
    /// Floating point addition - round to negative infinity.
    def int_AMDIL_add_rtn : GCCBuiltin<"__amdil_add_rtn">,
      BinaryIntFloat;
    /// Floating point addition - round to zero
    def int_AMDIL_add_rtz : GCCBuiltin<"__amdil_add_rtz">,
     BinaryIntFloat;
    /// Floating point multiplication - round to even
    def int_AMDIL_mul_rte : GCCBuiltin<"__amdil_mul_rte">,
      BinaryIntFloat;
    /// Floating point multiplication - round to positive infinity
    def int_AMDIL_mul_rtp : GCCBuiltin<"__amdil_mul_rtp">,
      BinaryIntFloat;
    /// Floating point multiplication - round to negative infinity
    def int_AMDIL_mul_rtn : GCCBuiltin<"__amdil_mul_rtn">,
      BinaryIntFloat;
    /// Floating point multiplication - round to zero
    def int_AMDIL_mul_rtz : GCCBuiltin<"__amdil_mul_rtz">,
      BinaryIntFloat;
  }

  /// Floating point subtraction - round to even
  def int_AMDIL_sub_rte : GCCBuiltin<"__amdil_sub_rte">,
    BinaryIntFloat;
  /// Floating point subtraction - round to positive infinity
  def int_AMDIL_sub_rtp : GCCBuiltin<"__amdil_sub_rtp">,
    BinaryIntFloat;
  /// Floating point subtraction - round to negative infinity
  def int_AMDIL_sub_rtn : GCCBuiltin<"__amdil_sub_rtn">,
    BinaryIntFloat;
  /// Floating point subtraction - round to zero
  def int_AMDIL_sub_rtz : GCCBuiltin<"__amdil_sub_rtz">,
    BinaryIntFloat;
  /// Floating point mad - round to even
  def int_AMDIL_mad_rte : GCCBuiltin<"__amdil_mad_rte">,
    TernaryIntFloat;
  /// Floating point mad - round to positive infinity
  def int_AMDIL_mad_rtp : GCCBuiltin<"__amdil_mad_rtp">,
    TernaryIntFloat;
  /// Floating point mad - round to negative infinity
  def int_AMDIL_mad_rtn : GCCBuiltin<"__amdil_mad_rtn">,
    TernaryIntFloat;
  /// Floating point mad - round to zero
  def int_AMDIL_mad_rtz : GCCBuiltin<"__amdil_mad_rtz">,
    TernaryIntFloat;
  /// Floating point fused multiply-fma - round to even
  def int_AMDIL_fma_rte : GCCBuiltin<"__amdil_fma_rte">,
    TernaryIntFloat;
  /// Floating point fused multiply-fma - round to positive infinity
  def int_AMDIL_fma_rtp : GCCBuiltin<"__amdil_fma_rtp">,
    TernaryIntFloat;
  /// Floating point fused multiply-fma - round to negative infinity
  def int_AMDIL_fma_rtn : GCCBuiltin<"__amdil_fma_rtn">,
    TernaryIntFloat;
  /// Floating point fused multiply-fma - round to zero
  def int_AMDIL_fma_rtz : GCCBuiltin<"__amdil_fma_rtz">,
    TernaryIntFloat;


  def int_AMDIL_mad_u64_u32 : GCCBuiltin<"__amdil_umadu64u32">,
    TernaryIntLong;
  def int_AMDIL_mad_i64_i32 : GCCBuiltin<"__amdil_imadi64i32">,
    TernaryIntLong;

  def int_AMDIL_media_mqsad_u32_u8 : GCCBuiltin<"__amdil_mqsadu32u8">,
    Intrinsic<[llvm_v4i32_ty], [llvm_v2i32_ty, llvm_i32_ty, llvm_v4i8_ty], [IntrNoMem]>;

    // @TODO, add atomics and real machine instructions.
  def int_AMDIL_atomic_add_fi32 : GCCBuiltin<"__atomic_add_fi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_sub_fi32 : GCCBuiltin<"__atomic_sub_fi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_fi32 : GCCBuiltin<"__atomic_xchg_fi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_inc_fi32 : GCCBuiltin<"__atomic_inc_fi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_dec_fi32 : GCCBuiltin<"__atomic_dec_fi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_cmpxchg_fi32 : GCCBuiltin<"__atomic_cmpxchg_fi32">,
    TernaryAtomicInt;
  def int_AMDIL_atomic_min_fi32 : GCCBuiltin<"__atomic_min_fi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_max_fi32 : GCCBuiltin<"__atomic_max_fi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_and_fi32 : GCCBuiltin<"__atomic_and_fi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_or_fi32 : GCCBuiltin<"__atomic_or_fi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xor_fi32 : GCCBuiltin<"__atomic_xor_fi32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_add_fu32 : GCCBuiltin<"__atomic_add_fu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_sub_fu32 : GCCBuiltin<"__atomic_sub_fu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_fu32 : GCCBuiltin<"__atomic_xchg_fu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_inc_fu32 : GCCBuiltin<"__atomic_inc_fu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_dec_fu32 : GCCBuiltin<"__atomic_dec_fu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_cmpxchg_fu32 : GCCBuiltin<"__atomic_cmpxchg_fu32">,
    TernaryAtomicInt;
  def int_AMDIL_atomic_min_fu32 : GCCBuiltin<"__atomic_min_fu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_max_fu32 : GCCBuiltin<"__atomic_max_fu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_and_fu32 : GCCBuiltin<"__atomic_and_fu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_or_fu32 : GCCBuiltin<"__atomic_or_fu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xor_fu32 : GCCBuiltin<"__atomic_xor_fu32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_ff32 : GCCBuiltin<"__atomic_xchg_ff32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_cmpxchg_ff32 : GCCBuiltin<"__atomic_cmpxchg_ff32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_min_ff32 : GCCBuiltin<"__atomic_min_ff32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_max_ff32 : GCCBuiltin<"__atomic_max_ff32">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_add_fi64 : GCCBuiltin<"__atomic_add_fi64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_sub_fi64 : GCCBuiltin<"__atomic_sub_fi64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_fi64 : GCCBuiltin<"__atomic_xchg_fi64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_inc_fi64 : GCCBuiltin<"__atomic_inc_fi64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_dec_fi64 : GCCBuiltin<"__atomic_dec_fi64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_cmpxchg_fi64 : GCCBuiltin<"__atomic_cmpxchg_fi64">,
    TernaryAtomicInt;
  def int_AMDIL_atomic_min_fi64 : GCCBuiltin<"__atomic_min_fi64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_max_fi64 : GCCBuiltin<"__atomic_max_fi64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_and_fi64 : GCCBuiltin<"__atomic_and_fi64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_or_fi64 : GCCBuiltin<"__atomic_or_fi64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xor_fi64 : GCCBuiltin<"__atomic_xor_fi64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_add_fu64 : GCCBuiltin<"__atomic_add_fu64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_sub_fu64 : GCCBuiltin<"__atomic_sub_fu64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_fu64 : GCCBuiltin<"__atomic_xchg_fu64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_inc_fu64 : GCCBuiltin<"__atomic_inc_fu64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_dec_fu64 : GCCBuiltin<"__atomic_dec_fu64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_cmpxchg_fu64 : GCCBuiltin<"__atomic_cmpxchg_fu64">,
    TernaryAtomicInt;
  def int_AMDIL_atomic_min_fu64 : GCCBuiltin<"__atomic_min_fu64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_max_fu64 : GCCBuiltin<"__atomic_max_fu64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_and_fu64 : GCCBuiltin<"__atomic_and_fu64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_or_fu64 : GCCBuiltin<"__atomic_or_fu64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xor_fu64 : GCCBuiltin<"__atomic_xor_fu64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_xchg_ff64 : GCCBuiltin<"__atomic_xchg_ff64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_cmpxchg_ff64 : GCCBuiltin<"__atomic_cmpxchg_ff64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_min_ff64 : GCCBuiltin<"__atomic_min_ff64">,
    BinaryAtomicInt;
  def int_AMDIL_atomic_max_ff64 : GCCBuiltin<"__atomic_max_ff64">,
    BinaryAtomicInt;

    // Cross-thread Operations
 def int_AMDIL_ballot : GCCBuiltin<"__amdil_ballot">,
    Intrinsic<[llvm_anyint_ty], [LLVMMatchType<0>], [IntrReadMem]>;
 def int_AMDIL_readfirstlane: GCCBuiltin<"__amdil_readfirstlane">,
    Intrinsic<[llvm_anyint_ty], [LLVMMatchType<0>], [IntrReadMem]>;
 def int_AMDIL_lane_id: GCCBuiltin<"__amdil_lane_id">,
    Intrinsic<[llvm_anyint_ty], [], [IntrReadMem]>;
 def int_AMDIL_receivelane: GCCBuiltin<"__amdil_receivelane">,
    Intrinsic<[llvm_anyint_ty],
              [llvm_anyint_ty, llvm_anyint_ty, llvm_anyint_ty, llvm_anyint_ty],
              [IntrReadMem]>;
}
