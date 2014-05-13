//----------------------------------------------------------------------------//
// AMDILEnumeratedTypes Section
//----------------------------------------------------------------------------//
def IL_OP_F16_ADD               : ILOpCode<600, "f16add">;
def IL_OP_F16_ADD_RTE           : ILOpCode<601, "f16add_rte">;
def IL_OP_F16_ADD_RTP           : ILOpCode<602, "f16add_rtp">;
def IL_OP_F16_ADD_RTN           : ILOpCode<603, "f16add_rtn">;
def IL_OP_F16_ADD_RTZ           : ILOpCode<604, "f16add_rtz">;
def IL_OP_F16_SUB               : ILOpCode<605, "f16sub">;
def IL_OP_F16_SUB_RTE           : ILOpCode<606, "f16sub_rte">;
def IL_OP_F16_SUB_RTP           : ILOpCode<607, "f16sub_rtp">;
def IL_OP_F16_SUB_RTN           : ILOpCode<608, "f16sub_rtn">;
def IL_OP_F16_SUB_RTZ           : ILOpCode<609, "f16sub_rtz">;
def IL_OP_F16_MUL               : ILOpCode<605, "f16mul">;
def IL_OP_F16_MUL_RTE           : ILOpCode<606, "f16mul_rte">;
def IL_OP_F16_MUL_RTP           : ILOpCode<607, "f16mul_rtp">;
def IL_OP_F16_MUL_RTN           : ILOpCode<608, "f16mul_rtn">;
def IL_OP_F16_MUL_RTZ           : ILOpCode<609, "f16mul_rtz">;
def IL_OP_F16_MAX               : ILOpCode<610, "f16max">;
def IL_OP_F16_MIN               : ILOpCode<611, "f16min">;
def IL_OP_F16_RCP               : ILOpCode<612, "f16rcp">;
def IL_OP_F16_RSQ               : ILOpCode<613, "f16rsq">;
def IL_OP_F16_FMA               : ILOpCode<614, "f16fma">;
def IL_OP_F16_FMA_RTE           : ILOpCode<615, "f16fma_rte">;
def IL_OP_F16_FMA_RTP           : ILOpCode<616, "f16fma_rtp">;
def IL_OP_F16_FMA_RTN           : ILOpCode<617, "f16fma_rtn">;
def IL_OP_F16_FMA_RTZ           : ILOpCode<618, "f16fma_rtz">;
def IL_OP_F16_MAD               : ILOpCode<619, "f16mad">;
def IL_OP_F16_SIN               : ILOpCode<620, "f16sin">;
def IL_OP_F16_COS               : ILOpCode<621, "f16cos">;
def IL_OP_F16_EXP               : ILOpCode<622, "f16exp">;
def IL_OP_F16_LOG               : ILOpCode<623, "f16log">;
def IL_OP_F16_SQRT              : ILOpCode<624, "f16sqrt">;
def IL_OP_F16_FRC               : ILOpCode<625, "f16frc">;
def IL_OP_F16_LDEXP             : ILOpCode<626, "f16ldexp">;
def IL_OP_F16_FREXP_EXP         : ILOpCode<627, "f16frexp_exp">;
def IL_OP_F16_FREXP_MANT        : ILOpCode<628, "f16frexp_mant">;
def IL_OP_D_2_F_RTE             : ILOpCode<629, "d2f_rte">;
def IL_OP_D_2_F_RTP             : ILOpCode<630, "d2f_rtp">;
def IL_OP_D_2_F_RTN             : ILOpCode<631, "d2f_rtn">;
def IL_OP_D_2_F_RTZ             : ILOpCode<632, "d2f_rtz">;
def IL_OP_F16_ROUND_NEAR        : ILOpCode<633, "f16round_nearest">;
def IL_OP_F16_ROUND_NEGINF      : ILOpCode<634, "f16round_neginf">;
def IL_OP_F16_ROUND_POSINF      : ILOpCode<635, "f16round_plusinf">;
def IL_OP_F16_ROUND_ZERO        : ILOpCode<636, "f16round_z">;
def IL_OP_U16_ADD               : ILOpCode<637, "u16add">;
def IL_OP_U16_SUB               : ILOpCode<638, "u16sub">;
def IL_OP_U16_MAX               : ILOpCode<639, "u16max">;
def IL_OP_I16_MAX               : ILOpCode<640, "i16max">;
def IL_OP_U16_MIN               : ILOpCode<641, "u16min">;
def IL_OP_I16_MIN               : ILOpCode<642, "i16min">;
def IL_OP_U16_MUL               : ILOpCode<643, "u16mul">;
def IL_OP_U16_MAD               : ILOpCode<644, "u16mad">;
def IL_OP_I16_MAD               : ILOpCode<645, "i16mad">;
def IL_OP_I16_SHL               : ILOpCode<646, "i16shl">;
def IL_OP_I16_SHR               : ILOpCode<647, "i16shr">;
def IL_OP_U16_SHR               : ILOpCode<648, "u16shr">;

//----------------------------------------------------------------------------//
// AMDILInstrInfo Section
//----------------------------------------------------------------------------//

// Predicate that is set to true if 16bit operations are supported or not
def HasShortOps : Predicate<"Subtarget->hasShortOps()">;
def HasHalfOps : Predicate<"Subtarget->hasHalfOps()">;

//----------------------------------------------------------------------------//
// AMDILMulticlass Section
//----------------------------------------------------------------------------//

multiclass UnaryIntMCi16<ILOpCode opc, Intrinsic opn>
: UnaryIntMCi16Scalar<opc, opn>, UnaryIntMCi16Vector<opc, opn>;

multiclass BinaryIntMCi16<ILOpCode opc, Intrinsic opn>
: BinaryIntMCi16Scalar<opc, opn>, BinaryIntMCi16Vector<opc, opn>;

multiclass TernaryPatMCi16<ILOpCode opc, Intrinsic opn>
: TernaryPatMCi16Scalar<opc, opn>, TernaryPatMCi16Vector<opc, opn>;

multiclass IntrConvertF64TOF32<ILOpCode opcode, Intrinsic intr>
{
  defm f32   : ConvertIntMC<opcode, intr, f32,   GPR_32,   f64,   GPR_64,
                            f64imm, fpimm>;
  defm v2f32 : ConvertIntMC<opcode, intr, v2f32, GPRV2I32, v2f64, GPRV2I64,
                            v2f64imm, fpimm>;
}

multiclass BinaryOpMCi16Scalar<ILOpCode opc, SDNode opn>
{
  defm i8    : BinaryOpExpand<opc, opn, GPRI8,   i8imm,    i8, imm>;
  defm i16   : BinaryOpExpand<opc, opn, GPRI16,  i16imm,   i16, imm>;
}

multiclass BinaryOpMCi16Vector<ILOpCode opc, SDNode opn>
{
  defm v2i8  : BinaryOpExpand<opc, opn, GPRV2I8,  v2i8imm,  v2i8, imm>;
  defm v2i16 : BinaryOpExpand<opc, opn, GPRV2I16, v2i16imm, v2i16, imm>;
  defm v4i8  : BinaryOpExpand<opc, opn, GPRV4I8,  v4i8imm,  v4i8, imm>;
  defm v4i16 : BinaryOpExpand<opc, opn, GPRV4I16, v4i16imm, v4i16, imm>;
}

multiclass BinaryOpMCi16<ILOpCode opc, SDNode opn>
: BinaryOpMCi16Scalar<opc, opn>, BinaryOpMCi16Vector<opc, opn>;

multiclass BinaryNCOpMCi16Shift<ILOpCode opc, SDNode opn>
{
  defm i8i8     : BinaryNCOpShift<opc, opn, GPRI8,    GPRI8,    i8imm,    i8,    i8>;
  defm v2i8i8   : BinaryNCOpShift<opc, opn, GPRV2I8,  GPRV2I8,  v2i8imm,  v2i8,  v2i8>;
  defm v4i8i8   : BinaryNCOpShift<opc, opn, GPRV4I8,  GPRV4I8,  v4i8imm,  v4i8,  v4i8>;
  defm i16i16   : BinaryNCOpShift<opc, opn, GPRI16,   GPRI16,   i16imm,   i16,   i16>;
  defm v2i16i16 : BinaryNCOpShift<opc, opn, GPRV2I16, GPRV2I16, v2i16imm, v2i16, v2i16>;
  defm v4i16i16 : BinaryNCOpShift<opc, opn, GPRV4I16, GPRV4I16, v4i16imm, v4i16, v4i16>;
  defm i8i32    : BinaryNCOpShift<opc, opn, GPRI8,    GPR_32,   i32imm,   i8,    i32>;
  defm v2i8i32  : BinaryNCOpShift<opc, opn, GPRV2I8,  GPRV2I32, v2i32imm, v2i8,  v2i32>;
  defm v4i8i32  : BinaryNCOpShift<opc, opn, GPRV4I8,  GPRV4I32, v4i32imm, v4i8,  v4i32>;
  defm i16i32   : BinaryNCOpShift<opc, opn, GPRI16,   GPR_32,   i32imm,   i16,   i32>;
  defm v2i16i32 : BinaryNCOpShift<opc, opn, GPRV2I16, GPRV2I32, v2i32imm, v2i16, v2i32>;
  defm v4i16i32 : BinaryNCOpShift<opc, opn, GPRV4I16, GPRV4I32, v4i32imm, v4i16, v4i32>;
  defm i8i64    : BinaryNCOpShift<opc, opn, GPRI8,    GPR_64,   i64imm,   i8,    i64>;
  defm v2i8i64  : BinaryNCOpShift<opc, opn, GPRV2I8,  GPRV2I64, v2i64imm, v2i8,  v2i64>;
  defm i16i64   : BinaryNCOpShift<opc, opn, GPRI16,   GPR_64,   i64imm,   i16,   i64>;
  defm v2i16i64 : BinaryNCOpShift<opc, opn, GPRV2I16, GPRV2I64, v2i64imm, v2i16, v2i64>;
}

//----------------------------------------------------------------------------//
// AMDILInstructions Section
//----------------------------------------------------------------------------//

// mayLoad = 0 and mayStore = 0 is OK assuming indexed array is never
// used.
let Predicates = [HasShortOps], mayLoad = 0, mayStore = 0 in {
  let isCommutable = 1 in {
    defm ADD16 : BinaryOpMCi16<IL_OP_U16_ADD, add>;
    defm MUL16 : BinaryOpMCi16<IL_OP_U16_MUL, mul>;
    defm U16_MAX : BinaryIntMCi16<IL_OP_U16_MAX, int_AMDIL_max_u16>;
    defm I16_MAX : BinaryIntMCi16<IL_OP_I16_MAX, int_AMDIL_max_i16>;
    defm U16_MIN : BinaryIntMCi16<IL_OP_U16_MIN, int_AMDIL_min_u16>;
    defm I16_MIN : BinaryIntMCi16<IL_OP_I16_MIN, int_AMDIL_min_i16>;
  }

  defm SUB16 : BinaryOpMCi16<IL_OP_U16_SUB, sub>;
  defm SHL16 : BinaryNCOpMCi16Shift<IL_OP_I16_SHL, shl>;
  defm SHRI16 : BinaryNCOpMCi16Shift<IL_OP_I16_SHR, sra>;
  defm SHRU16 : BinaryNCOpMCi16Shift<IL_OP_U16_SHR, srl>;

  defm U16_MAD : TernaryPatMCi16<IL_OP_U16_MAD, int_AMDIL_mad_u16>;
  defm I16_MAD : TernaryPatMCi16<IL_OP_I16_MAD, int_AMDIL_mad_i16>;
}

let Predicates = [HasShortOps], mayLoad = 0, mayStore = 0 in {
  let isCommutable = 1 in {
    defm F16_ADD     : BinaryIntMCi16<IL_OP_F16_ADD, int_AMDIL_add_f16>;
    defm F16_ADD_RTE : BinaryIntMCi16<IL_OP_F16_ADD_RTE, int_AMDIL_add_rte_f16>;
    defm F16_ADD_RTP : BinaryIntMCi16<IL_OP_F16_ADD_RTP, int_AMDIL_add_rtp_f16>;
    defm F16_ADD_RTN : BinaryIntMCi16<IL_OP_F16_ADD_RTN, int_AMDIL_add_rtn_f16>;
    defm F16_ADD_RTZ : BinaryIntMCi16<IL_OP_F16_ADD_RTZ, int_AMDIL_add_rtz_f16>;
  }

  defm F16_SUB     : BinaryIntMCi16<IL_OP_F16_SUB, int_AMDIL_sub_f16>;
  defm F16_SUB_RTE : BinaryIntMCi16<IL_OP_F16_SUB_RTE, int_AMDIL_sub_rte_f16>;
  defm F16_SUB_RTP : BinaryIntMCi16<IL_OP_F16_SUB_RTP, int_AMDIL_sub_rtp_f16>;
  defm F16_SUB_RTN : BinaryIntMCi16<IL_OP_F16_SUB_RTN, int_AMDIL_sub_rtn_f16>;
  defm F16_SUB_RTZ : BinaryIntMCi16<IL_OP_F16_SUB_RTZ, int_AMDIL_sub_rtz_f16>;

  let isCommutable = 1 in {
    defm F16_MUL     : BinaryIntMCi16<IL_OP_F16_MUL, int_AMDIL_mul_f16>;
    defm F16_MUL_RTE : BinaryIntMCi16<IL_OP_F16_MUL_RTE, int_AMDIL_mul_rte_f16>;
    defm F16_MUL_RTP : BinaryIntMCi16<IL_OP_F16_MUL_RTP, int_AMDIL_mul_rtp_f16>;
    defm F16_MUL_RTN : BinaryIntMCi16<IL_OP_F16_MUL_RTN, int_AMDIL_mul_rtn_f16>;
    defm F16_MUL_RTZ : BinaryIntMCi16<IL_OP_F16_MUL_RTZ, int_AMDIL_mul_rtz_f16>;
    defm F16_MAX     : BinaryIntMCi16<IL_OP_F16_MAX, int_AMDIL_max_f16>;
    defm F16_MIN     : BinaryIntMCi16<IL_OP_F16_MIN, int_AMDIL_min_f16>;
  }

  defm F16_RCP     : UnaryIntMCi16<IL_OP_F16_RCP, int_AMDIL_rcp_f16>;
  defm F16_RSQ     : UnaryIntMCi16<IL_OP_F16_RSQ, int_AMDIL_rsq_f16>;
  defm F16_FMA     : TernaryPatMCi16<IL_OP_F16_FMA, int_AMDIL_fma_f16>;
  defm F16_FMA_RTE : TernaryPatMCi16<IL_OP_F16_FMA_RTE, int_AMDIL_fma_rte_f16>;
  defm F16_FMA_RTP : TernaryPatMCi16<IL_OP_F16_FMA_RTP, int_AMDIL_fma_rtp_f16>;
  defm F16_FMA_RTN : TernaryPatMCi16<IL_OP_F16_FMA_RTN, int_AMDIL_fma_rtn_f16>;
  defm F16_FMA_RTZ : TernaryPatMCi16<IL_OP_F16_FMA_RTZ, int_AMDIL_fma_rtz_f16>;
  defm F16_MAD     : TernaryPatMCi16<IL_OP_F16_MAD, int_AMDIL_mad_f16>;
  defm F16_SIN     : UnaryIntMCi16<IL_OP_F16_SIN, int_AMDIL_sin_f16>;
  defm F16_COS     : UnaryIntMCi16<IL_OP_F16_COS, int_AMDIL_cos_f16>;
  defm F16_EXP     : UnaryIntMCi16<IL_OP_F16_EXP, int_AMDIL_exp_f16>;
  defm F16_LOG     : UnaryIntMCi16<IL_OP_F16_LOG, int_AMDIL_log_f16>;
  defm F16_SQRT    : UnaryIntMCi16<IL_OP_F16_SQRT, int_AMDIL_sqrt_f16>;
  defm F16_FRC     : UnaryIntMCi16<IL_OP_F16_FRC, int_AMDIL_frc_f16>;
  defm F16_LDEXP   : BinaryIntMCi16<IL_OP_F16_LDEXP, int_AMDIL_ldexp_f16>;
  defm F16_FREXP_EXP :
        UnaryIntMCi16<IL_OP_F16_FREXP_EXP, int_AMDIL_frexp_exp_f16>;
  defm F16_FREXP_MANT :
        UnaryIntMCi16<IL_OP_F16_FREXP_MANT, int_AMDIL_frexp_mant_f16>;
  defm F16_ROUND_NEAR :
        UnaryIntMCi16<IL_OP_F16_ROUND_NEAR, int_AMDIL_round_nearest_f16>;
  defm F16_ROUND_NEGINF :
        UnaryIntMCi16<IL_OP_F16_ROUND_NEGINF, int_AMDIL_round_neginf_f16>;
  defm F16_ROUND_POSINF :
        UnaryIntMCi16<IL_OP_F16_ROUND_POSINF, int_AMDIL_round_posinf_f16>;
  defm F16_ROUND_ZERO :
        UnaryIntMCi16<IL_OP_F16_ROUND_ZERO, int_AMDIL_round_zero_f16>;

  defm DTOF_RTE : IntrConvertF64TOF32<IL_OP_D_2_F_RTE,
                                      int_AMDIL_convert_f64_f32_rte>;
  defm DTOF_RTP : IntrConvertF64TOF32<IL_OP_D_2_F_RTP,
                                      int_AMDIL_convert_f64_f32_rtp>;
  defm DTOF_RTN : IntrConvertF64TOF32<IL_OP_D_2_F_RTN,
                                      int_AMDIL_convert_f64_f32_rtn>;
  defm DTOF_RTZ : IntrConvertF64TOF32<IL_OP_D_2_F_RTZ,
                                    int_AMDIL_convert_f64_f32_rtz>;
}

