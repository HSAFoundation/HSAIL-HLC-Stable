let TargetPrefix="AMDIL", isTarget = 1 in {
/// F16 addition - default
def int_AMDIL_add_f16           : GCCBuiltin<"__amdil_add_f16">,
    BinaryIntInt;
/// F16 addition - round to even
def int_AMDIL_add_rte_f16       : GCCBuiltin<"__amdil_add_rte_f16">,
    BinaryIntInt;
/// F16 addition - round to positive infinity
def int_AMDIL_add_rtp_f16       : GCCBuiltin<"__amdil_add_rtp_f16">,
    BinaryIntInt;
/// F16 addition - round to negative infinity
def int_AMDIL_add_rtn_f16       : GCCBuiltin<"__amdil_add_rtn_f16">,
    BinaryIntInt;
/// F16 addition - round to zero
def int_AMDIL_add_rtz_f16       : GCCBuiltin<"__amdil_add_rtz_f16">,
    BinaryIntInt;
def int_AMDIL_sub_f16           : GCCBuiltin<"__amdil_sub_f16">,
    BinaryIntInt;
def int_AMDIL_sub_rte_f16       : GCCBuiltin<"__amdil_sub_rte_f16">,
    BinaryIntInt;
def int_AMDIL_sub_rtp_f16       : GCCBuiltin<"__amdil_sub_rtp_f16">,
    BinaryIntInt;
def int_AMDIL_sub_rtn_f16       : GCCBuiltin<"__amdil_sub_rtn_f16">,
    BinaryIntInt;
def int_AMDIL_sub_rtz_f16       : GCCBuiltin<"__amdil_sub_rtz_f16">,
    BinaryIntInt;
def int_AMDIL_mul_f16           : GCCBuiltin<"__amdil_mul_f16">,
    BinaryIntInt;
def int_AMDIL_mul_rte_f16       : GCCBuiltin<"__amdil_mul_rte_f16">,
    BinaryIntInt;
def int_AMDIL_mul_rtp_f16       : GCCBuiltin<"__amdil_mul_rtp_f16">,
    BinaryIntInt;
def int_AMDIL_mul_rtn_f16       : GCCBuiltin<"__amdil_mul_rtn_f16">,
    BinaryIntInt;
def int_AMDIL_mul_rtz_f16       : GCCBuiltin<"__amdil_mul_rtz_f16">,
    BinaryIntInt;
def int_AMDIL_max_f16           : GCCBuiltin<"__amdil_max_f16">,
    BinaryIntInt;
def int_AMDIL_min_f16           : GCCBuiltin<"__amdil_min_f16">,
    BinaryIntInt;
def int_AMDIL_rcp_f16           : GCCBuiltin<"__amdil_rcp_f16">,
    UnaryIntInt;
def int_AMDIL_rsq_f16           : GCCBuiltin<"__amdil_rsq_f16">,
    UnaryIntInt;
def int_AMDIL_fma_f16           : GCCBuiltin<"__amdil_fma_f16">,
    TernaryIntInt;
def int_AMDIL_fma_rte_f16       : GCCBuiltin<"__amdil_fma_rte_f16">,
    TernaryIntInt;
def int_AMDIL_fma_rtp_f16       : GCCBuiltin<"__amdil_fma_rtp_f16">,
    TernaryIntInt;
def int_AMDIL_fma_rtn_f16       : GCCBuiltin<"__amdil_fma_rtn_f16">,
    TernaryIntInt;
def int_AMDIL_fma_rtz_f16       : GCCBuiltin<"__amdil_fma_rtz_f16">,
    TernaryIntInt;
def int_AMDIL_mad_f16           : GCCBuiltin<"__amdil_mad_f16">,
    TernaryIntInt;
def int_AMDIL_sin_f16           : GCCBuiltin<"__amdil_sin_f16">,
    UnaryIntInt;
def int_AMDIL_cos_f16           : GCCBuiltin<"__amdil_cos_f16">,
    UnaryIntInt;
def int_AMDIL_exp_f16           : GCCBuiltin<"__amdil_exp_f16">,
    UnaryIntInt;
def int_AMDIL_log_f16           : GCCBuiltin<"__amdil_log_f16">,
    UnaryIntInt;
def int_AMDIL_sqrt_f16          : GCCBuiltin<"__amdil_sqrt_f16">,
    UnaryIntInt;
def int_AMDIL_frc_f16           : GCCBuiltin<"__amdil_frc_f16">,
    UnaryIntInt;
def int_AMDIL_ldexp_f16         : GCCBuiltin<"__amdil_ldexp_f16">,
    BinaryIntInt;
def int_AMDIL_frexp_exp_f16     : GCCBuiltin<"__amdil_frexp_exp_f16">,
    UnaryIntInt;
def int_AMDIL_frexp_mant_f16    : GCCBuiltin<"__amdil_frexp_mant_f16">,
    UnaryIntInt;
def int_AMDIL_round_nearest_f16 : GCCBuiltin<"__amdil_round_nearest_f16">,
    UnaryIntInt;
def int_AMDIL_round_neginf_f16  : GCCBuiltin<"__amdil_round_neginf_f16">,
    UnaryIntInt;
def int_AMDIL_round_posinf_f16  : GCCBuiltin<"__amdil_round_posinf_f16">,
    UnaryIntInt;
def int_AMDIL_round_zero_f16    : GCCBuiltin<"__amdil_round_zero_f16">,
    UnaryIntInt;


def int_AMDIL_convert_f64_f32_rte : GCCBuiltin<"__amdil_double_to_float_nearest">,
    Intrinsic<[llvm_anyfloat_ty], [llvm_anyfloat_ty], [IntrNoMem]>;
def int_AMDIL_convert_f64_f32_rtp : GCCBuiltin<"__amdil_double_to_float_posinf">,
    Intrinsic<[llvm_anyfloat_ty], [llvm_anyfloat_ty], [IntrNoMem]>;
def int_AMDIL_convert_f64_f32_rtn : GCCBuiltin<"__amdil_double_to_float_neginf">,
    Intrinsic<[llvm_anyfloat_ty], [llvm_anyfloat_ty], [IntrNoMem]>;
def int_AMDIL_convert_f64_f32_rtz : GCCBuiltin<"__amdil_double_to_float_zero">,
    Intrinsic<[llvm_anyfloat_ty], [llvm_anyfloat_ty], [IntrNoMem]>;

def int_AMDIL_min_i16 : GCCBuiltin<"__amdil_min_i16">,
        BinaryIntInt;
def int_AMDIL_min_u16 : GCCBuiltin<"__amdil_min_u16">,
        BinaryIntInt;
def int_AMDIL_max_i16 : GCCBuiltin<"__amdil_max_i16">,
        BinaryIntInt;
def int_AMDIL_max_u16 : GCCBuiltin<"__amdil_max_u16">,
        BinaryIntInt;
def int_AMDIL_mad_i16 : GCCBuiltin<"__amdil_mad_i16">,
        TernaryIntInt;
def int_AMDIL_mad_u16 : GCCBuiltin<"__amdil_mad_u16">,
        TernaryIntInt;
}
