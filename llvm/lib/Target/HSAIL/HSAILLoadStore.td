////////////////////////////////////////////////////////////////////////////////
// stof, ftos -- not matched, inserted in MachineInstr lowering 
// 32 bit
def global_stof_u32 : HSAILInst<(outs GPR32:$dst), (ins GPR32:$src),
                     "stof_global_u32\t$dst, $src",
                     []>;

def group_stof_u32 : HSAILInst<(outs GPR32:$dst), (ins GPR32:$src),
                     "stof_group_u32\t$dst, $src",
                     []>;

def constant_stof_u32 : HSAILInst<(outs GPR32:$dst), (ins GPR32:$src),
                     "stof_readonly_u32\t$dst, $src",
                     []>;

def private_stof_u32 : HSAILInst<(outs GPR32:$dst), (ins GPR32:$src),
                     "stof_private_u32\t$dst, $src",
                     []>;

def global_ftos_u32 : HSAILInst<(outs GPR32:$dst), (ins GPR32:$src),
                     "ftos_global_u32\t$dst, $src",
                     []>;

def group_ftos_u32 : HSAILInst<(outs GPR32:$dst), (ins GPR32:$src),
                     "ftos_group_u32\t$dst, $src",
                     []>;

def constant_ftos_u32 : HSAILInst<(outs GPR32:$dst), (ins GPR32:$src),
                     "ftos_readonly_u32\t$dst, $src",
                     []>;

def private_ftos_u32 : HSAILInst<(outs GPR32:$dst), (ins GPR32:$src),
                     "ftos_private_u32\t$dst, $src",
                     []>;
// 64 bit
def global_stof_u64 : HSAILInst<(outs GPR64:$dst), (ins GPR64:$src),
                     "stof_global_u64\t$dst, $src",
                     []>;

def group_stof_u64 : HSAILInst<(outs GPR64:$dst), (ins GPR64:$src),
                     "stof_group_u64\t$dst, $src",
                     []>;

def constant_stof_u64 : HSAILInst<(outs GPR64:$dst), (ins GPR64:$src),
                     "stof_readonly_u64\t$dst, $src",
                     []>;

def private_stof_u64 : HSAILInst<(outs GPR64:$dst), (ins GPR64:$src),
                     "stof_private_u64\t$dst, $src",
                     []>;

def global_ftos_u64 : HSAILInst<(outs GPR64:$dst), (ins GPR64:$src),
                     "ftos_global_u64\t$dst, $src",
                     []>;

def group_ftos_u64 : HSAILInst<(outs GPR64:$dst), (ins GPR64:$src),
                     "ftos_group_u64\t$dst, $src",
                     []>;

def constant_ftos_u64 : HSAILInst<(outs GPR64:$dst), (ins GPR64:$src),
                     "ftos_readonly_u64\t$dst, $src",
                     []>;

def private_ftos_u64 : HSAILInst<(outs GPR64:$dst), (ins GPR64:$src),
                     "ftos_private_u64\t$dst, $src",
                     []>;

multiclass HSAIL_PARAM_ARG_DECL<string prefix> {
     def _u8  : HSAILInst<(outs ), (ins i32imm:$p, i32imm:$size),
                !strconcat("arg_u8 %",  !strconcat(prefix, "$p")),
                [(ARGDeclNode_U8 (i32 imm:$p), (i32 imm:$size))]>;

     def _u16 : HSAILInst<(outs ), (ins i32imm:$p, i32imm:$size),
                !strconcat("arg_u16 %", !strconcat(prefix, "$p")),
                [(ARGDeclNode_U16 (i32 imm:$p), (i32 imm:$size))]>;

     def _f32 : HSAILInst<(outs ), (ins i32imm:$p, i32imm:$size),
                !strconcat("arg_f32 %", !strconcat(prefix, "$p")),
                [(ARGDeclNode_F32 (i32 imm:$p), (i32 imm:$size))]>;

     def _u32 : HSAILInst<(outs ), (ins i32imm:$p, i32imm:$size),
                !strconcat("arg_u32 %", !strconcat(prefix, "$p")),
                [(ARGDeclNode_U32 (i32 imm:$p), (i32 imm:$size))]>;

     def _f64 :  HSAILInst<(outs ), (ins i32imm:$p, i32imm:$size),
                !strconcat("arg_f64 %", !strconcat(prefix, "$p")),
                [(ARGDeclNode_F64 (i32 imm:$p), (i32 imm:$size))]>;

     def _u64 : HSAILInst<(outs ), (ins i32imm:$p, i32imm:$size),
                !strconcat("arg_u64 %", !strconcat(prefix, "$p")),
                [(ARGDeclNode_U64 (i32 imm:$p), (i32 imm:$size))]>;
}

multiclass HSAIL_RET_ARG_DECL<string prefix> {
     def _u8  : HSAILInst<(outs ), (ins i32imm:$p, i32imm:$size),
                !strconcat("arg_u8 %",  !strconcat(prefix, "$p")),
                [(RETDeclNode_U8 (i32 imm:$p), (i32 imm:$size))]>;

     def _u16 : HSAILInst<(outs ), (ins i32imm:$p, i32imm:$size),
                !strconcat("arg_u16 %", !strconcat(prefix, "$p")),
                [(RETDeclNode_U16 (i32 imm:$p), (i32 imm:$size))]>;

     def _f32 : HSAILInst<(outs ), (ins i32imm:$p, i32imm:$size),
                !strconcat("arg_f32 %", !strconcat(prefix, "$p")),
                [(RETDeclNode_F32 (i32 imm:$p), (i32 imm:$size))]>;

     def _u32 : HSAILInst<(outs ), (ins i32imm:$p, i32imm:$size),
                !strconcat("arg_u32 %", !strconcat(prefix, "$p")),
                [(RETDeclNode_U32 (i32 imm:$p), (i32 imm:$size))]>;

     def _f64 :  HSAILInst<(outs ), (ins i32imm:$p, i32imm:$size),
                !strconcat("arg_f64 %", !strconcat(prefix, "$p")),
                [(RETDeclNode_F64 (i32 imm:$p), (i32 imm:$size))]>;

     def _u64 : HSAILInst<(outs ), (ins i32imm:$p, i32imm:$size),
                !strconcat("arg_u64 %", !strconcat(prefix, "$p")),
                [(RETDeclNode_U64 (i32 imm:$p), (i32 imm:$size))]>;
}

multiclass HSAIL_ST_ARG<SDNode stNode, string oper, string prefix, RegisterClass RC, Predicate ArchType> {
  let mayStore = 1 in {
    def _f32 :  HSAILInst<(outs), (ins GPR32:$src, RC:$dst, i32imm:$offset),
            !strconcat(!strconcat(oper,"_f32 $src, [%"), !strconcat(prefix, "$dst][$offset]")),
            [(stNode (f32 GPR32:$src), (f32 RC:$dst), (i32 imm:$offset))]>,
                Requires<[ArchType]>;

    def _u32 :  HSAILInst<(outs), (ins GPR32:$src, RC:$dst, i32imm:$offset),
                !strconcat(!strconcat(oper,"_u32 $src, [%"), !strconcat(prefix, "$dst][$offset]")),
                [(stNode (i32 GPR32:$src), (i32 RC:$dst), (i32 imm:$offset))]>,
                Requires<[ArchType]>;

    def _f64 :  HSAILInst<(outs), (ins GPR64:$src, RC:$dst, i32imm:$offset),
                !strconcat(!strconcat(oper,"_f64 $src, [%"), !strconcat(prefix, "$dst][$offset]")),
                [(stNode (f64 GPR64:$src), (f64 RC:$dst), (i32 imm:$offset))]>,
                Requires<[ArchType]>;

    def _u64 :  HSAILInst<(outs), (ins GPR64:$src, RC:$dst, i32imm:$offset),
                !strconcat(!strconcat(oper,"_u64 $src, [%"), !strconcat(prefix, "$dst][$offset]")),
                [(stNode (i64 GPR64:$src), (i64 RC:$dst), (i32 imm:$offset))]>,
                Requires<[ArchType]>;

    def _b1 :  HSAILInst<(outs), (ins CR:$src, RC:$dst, i32imm:$offset),
            !strconcat(!strconcat(oper,"_b1 $src, [%"), !strconcat(prefix, "$dst][$offset]")),
        [(stNode (i1 CR:$src), (i1 RC:$dst), (i32 imm:$offset))]>,
        Requires<[ArchType]>;
  }
}

multiclass HSAIL_LD_ARG<SDNode ldNode, string oper, string prefix, RegisterClass RC, Predicate ArchType> {
  let mayLoad = 1 in {
    def _f32 :  HSAILInst<(outs GPR32:$dst), (ins RC:$src, i32imm:$offset),
                !strconcat(!strconcat(oper,"_f32 $dst, [%"), !strconcat(prefix, "$src][$offset]")),
                [(set (f32 GPR32:$dst), (ldNode (f32 RC:$src), (i32 imm:$offset)))]>,
                Requires<[ArchType]>;

    def _u32 :  HSAILInst<(outs GPR32:$dst), (ins RC:$src, i32imm:$offset),
                !strconcat(!strconcat(oper,"_u32 $dst, [%"), !strconcat(prefix, "$src][$offset]")),
                [(set (i32 GPR32:$dst), (ldNode (i32 RC:$src), (i32 imm:$offset)))]>,
                Requires<[ArchType]>;

    def _f64 :  HSAILInst<(outs GPR64:$dst), (ins RC:$src, i32imm:$offset),
                !strconcat(!strconcat(oper,"_f64 $dst, [%"), !strconcat(prefix, "$src][$offset]")),
                [(set (f64 GPR64:$dst), (ldNode (f64 RC:$src), (i32 imm:$offset)))]>,
                Requires<[ArchType]>;

    def _u64 :  HSAILInst<(outs GPR64:$dst), (ins RC:$src, i32imm:$offset),
                !strconcat(!strconcat(oper,"_u64 $dst, [%"), !strconcat(prefix, "$src][$offset]")),
                [(set (i64 GPR64:$dst), (ldNode (i64 RC:$src), (i32 imm:$offset)))]>,
                Requires<[ArchType]>;

    def _b1 : HSAILInst<(outs CR:$dst), (ins RC:$src, i32imm:$offset),
            !strconcat(!strconcat(oper,"_b1 $dst, [%"), !strconcat(prefix, "$src][$offset]")),
            [(set (i1 CR:$dst), (ldNode (i1 RC:$src), (i32 imm:$offset)))]>,
            Requires<[ArchType]>;
  }
}

let Defs = [include "HSAILPARAMREG.td"] in {
  let needSpecialProcessing = 1 in {

    defm arg : HSAIL_PARAM_ARG_DECL<"param_p">;

    // Subword args require special SDNodes. Hence they are not
    // instantiated inside the corresponding multiclass.
    let mayStore = 1 in {
    def st_parg_u8 : HSAILInst<(outs), (ins GPR32:$src, PARAMREG:$dst, i32imm:$offset),
                     "st_arg_u8 $src, [%param_$dst][$offset]",
                     [(STARGNode_U8 (i32 GPR32:$src), (i32 PARAMREG:$dst), (i32 imm:$offset))]>,
                     Requires<[HSAILArchGeneric]>;

    def st_parg_u16 : HSAILInst<(outs), (ins GPR32:$src, PARAMREG:$dst, i32imm:$offset),
                     "st_arg_u16 $src, [%param_$dst][$offset]",
                     [(STARGNode_U16 (i32 GPR32:$src), (i32 PARAMREG:$dst), (i32 imm:$offset))]>,
                     Requires<[HSAILArchGeneric]>;

    defm st_parg : HSAIL_ST_ARG<STARGNode, "st_arg", "param_", PARAMREG, HSAILArchGeneric>;
    defm st_parg_flat : HSAIL_ST_ARG<STARGNode, "st", "param_", PARAMREG, HSAILArchUsesFlatAddr>;
    }
  }
}

let Defs = [include "HSAILRETREG.td"] in {
  let needSpecialProcessing = 1 in {

    defm arg_ret : HSAIL_RET_ARG_DECL<"ret_r">;

    // Subword args require special SDNodes. Hence they are not
    // instantiated inside the corresponding multiclass.
    let mayStore = 1 in {
    def st_rarg_u8 : HSAILInst<(outs), (ins GPR32:$src, RETREG:$dst, i32imm:$offset),
                     "st_arg_u8 $src, [%ret_$dst][$offset]",
                     [(STRETNode_U8 (i32 GPR32:$src), (i32 RETREG:$dst), (i32 imm:$offset))]>,
                     Requires<[HSAILArchGeneric]>;

    def st_rarg_u16 : HSAILInst<(outs), (ins GPR32:$src, RETREG:$dst, i32imm:$offset),
                     "st_arg_u16 $src, [%ret_$dst][$offset]",
                     [(STRETNode_U16 (i32 GPR32:$src), (i32 RETREG:$dst), (i32 imm:$offset))]>,
                     Requires<[HSAILArchGeneric]>;

    defm st_rarg : HSAIL_ST_ARG<STRETNode, "st_arg", "ret_", RETREG, HSAILArchGeneric>;
    defm st_rarg_flat : HSAIL_ST_ARG<STRETNode, "st", "ret_", RETREG, HSAILArchUsesFlatAddr>;
    }
  }
}

let Uses = [include "HSAILPARAMREG.td"] in {
  let needSpecialProcessing = 1, mayLoad = 1 in {
    // Subword args require special SDNodes.
    // instantiated inside the corresponding multiclass.
    def ld_parg_u8  : HSAILInst<(outs GPR32:$dst), (ins PARAMREG:$src, i32imm:$offset),
                "ld_arg_u8 $dst, [%param_$src][$offset]",
                [(set (i32 GPR32:$dst), (LDPARAMNode_U8 (i32 PARAMREG:$src), (i32 imm:$offset)))]>,
                Requires<[HSAILArchGeneric]>;

    def ld_parg_u16 : HSAILInst<(outs GPR32:$dst), (ins PARAMREG:$src, i32imm:$offset),
                "ld_arg_u16 $dst, [%param_$src][$offset]",
                [(set (i32 GPR32:$dst), (LDPARAMNode_U16 (i32 PARAMREG:$src), (i32 imm:$offset)))]>,
                Requires<[HSAILArchGeneric]>;

    defm ld_parg : HSAIL_LD_ARG<LDPARAMNode, "ld_arg", "param_", PARAMREG, HSAILArchGeneric>;
    defm ld_parg_flat : HSAIL_LD_ARG<LDPARAMNode, "ld", "param_", PARAMREG, HSAILArchUsesFlatAddr>;
  }
}

let Uses = [include "HSAILRETREG.td"] in {
  let needSpecialProcessing = 1, mayLoad = 1 in {
    // Subword args require special SDNodes.
    // instantiated inside the corresponding multiclass.
    def ld_rarg_u8  : HSAILInst<(outs GPR32:$dst), (ins RETREG:$src, i32imm:$offset),
                "ld_arg_u8 $dst, [%ret_$src][$offset]",
                [(set (i32 GPR32:$dst), (LDRETNode_U8 (i32 RETREG:$src), (i32 imm:$offset)))]>,
                Requires<[HSAILArchGeneric]>;

    def ld_rarg_u16 : HSAILInst<(outs GPR32:$dst), (ins RETREG:$src, i32imm:$offset),
                "ld_arg_u16 $dst, [%ret_$src][$offset]",
                [(set (i32 GPR32:$dst), (LDRETNode_U16 (i32 RETREG:$src), (i32 imm:$offset)))]>,
                Requires<[HSAILArchGeneric]>;

    defm ld_rarg : HSAIL_LD_ARG<LDRETNode, "ld_arg", "ret_", RETREG, HSAILArchGeneric>;
    defm ld_rarg_flat : HSAIL_LD_ARG<LDRETNode, "ld", "ret_", RETREG, HSAILArchUsesFlatAddr>;
  }
}

def HSAILWrapper : SDNode<"HSAILISD::WRAPPER", 
    SDTypeProfile<1, 1, [SDTCisPtrTy<0>]>>;

def wrapper : PatFrag<(ops node:$ptr), (HSAILWrapper node:$ptr), [{
    return dyn_cast<ExternalSymbolSDNode>(N->getOperand(0));
}]>;


// Load kernarg
def HSAILloadparam_u8_
  : SDNode<"HSAILISD::LOAD_PARAM_U8", SDTypeProfile<1, 1, [SDTCisPtrTy<1>]>,
           [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue]>;

def HSAILloadparam_u16_
  : SDNode<"HSAILISD::LOAD_PARAM_U16", SDTypeProfile<1, 1, [SDTCisPtrTy<1>]>,
           [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue]>;

def HSAILloadparam_
  : SDNode<"HSAILISD::LOAD_PARAM", SDTypeProfile<1, 1, [SDTCisPtrTy<1>]>,
           [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue]>;

def HSAILloadparam_rwimg_
  : SDNode<"HSAILISD::LOAD_PARAM_IMAGE", SDTypeProfile<1, 1, [SDTCisPtrTy<1>]>,
           [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue]>;

def HSAILloadparam_samp_
  : SDNode<"HSAILISD::LOAD_PARAM_SAMP", SDTypeProfile<1, 1, [SDTCisPtrTy<1>]>,
           [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue]>;

def HSAILloadparam_u8
  : PatFrag<(ops node:$ptr),
            (HSAILloadparam_u8_ node:$ptr)>;
def HSAILloadparam_u16
  : PatFrag<(ops node:$ptr),
            (HSAILloadparam_u16_ node:$ptr)>;
def HSAILloadparam
  : PatFrag<(ops node:$ptr),
            (HSAILloadparam_ node:$ptr)>;
def HSAILloadparam_rwimg
  : PatFrag<(ops node:$ptr),
            (HSAILloadparam_rwimg_ node:$ptr)>;
def HSAILloadparam_samp
  : PatFrag<(ops node:$ptr),
            (HSAILloadparam_samp_ node:$ptr)>;

multiclass HSAIL_PARAM_LD_ST<PatFrag loadParam, string asm, RegisterClass RC,
                             ValueType vt, Predicate ArchType> {
  let needSpecialProcessing = 1 in {
    def LDpi_Kernel : HSAILInst<(outs RC:$d), (ins i32imm:$a),
                     !strconcat(asm, "\t$d, [$a]"),
                     [(set RC:$d, (vt (loadParam texternalsym:$a)))]>,
                      Requires<[ ArchType, HSAILisKernel ]>;

  }
}

multiclass HSAIL_PARAM_LD_ST_ARG<PatFrag loadParam, string asm, RegisterClass RC,
                                 ValueType vt, Predicate ArchType> {
  let needSpecialProcessing = 1 in {
    def LDpi : HSAILInst<(outs RC:$d), (ins i32imm:$a),
                     !strconcat(asm,  "\t$d, [$a]"),
                     [(set RC:$d, (vt (loadParam texternalsym:$a)))]>,
                      Requires<[ ArchType, HSAILisNotKernel ]>;
  }
}

// Scalar value param loads/stores

let usesCustomInserter = 1 in {
  defm PARAMU8   : HSAIL_PARAM_LD_ST<HSAILloadparam_u8, "ld_kernarg_u8", GPR32, i32, HSAILArchGeneric>;
  defm PARAMU16  : HSAIL_PARAM_LD_ST<HSAILloadparam_u16, "ld_kernarg_u16", GPR32, i32, HSAILArchGeneric>;
  defm PARAMU32  : HSAIL_PARAM_LD_ST<HSAILloadparam, "ld_kernarg_u32", GPR32, i32, HSAILArchGeneric>;
  defm PARAMF32  : HSAIL_PARAM_LD_ST<HSAILloadparam, "ld_kernarg_f32", GPR32, f32, HSAILArchGeneric>;
  defm PARAMU64  : HSAIL_PARAM_LD_ST<HSAILloadparam, "ld_kernarg_u64", GPR64, i64, HSAILArchGeneric>;
  defm PARAMF64  : HSAIL_PARAM_LD_ST<HSAILloadparam, "ld_kernarg_f64", GPR64, f64, HSAILArchGeneric>;

  defm PARAMU8_flat  : HSAIL_PARAM_LD_ST<HSAILloadparam_u8, "ld_u8", GPR32, i32, HSAILArchUsesFlatAddr>;
  defm PARAMU16_flat : HSAIL_PARAM_LD_ST<HSAILloadparam_u16, "ld_u16", GPR32, i32, HSAILArchUsesFlatAddr>;
  defm PARAMU32_flat : HSAIL_PARAM_LD_ST<HSAILloadparam, "ld_u32", GPR32, i32, HSAILArchUsesFlatAddr>;
  defm PARAMF32_flat : HSAIL_PARAM_LD_ST<HSAILloadparam, "ld_f32", GPR32, f32, HSAILArchUsesFlatAddr>;
  defm PARAMU64_flat : HSAIL_PARAM_LD_ST<HSAILloadparam, "ld_u64", GPR64, i64, HSAILArchUsesFlatAddr>;
  defm PARAMF64_flat : HSAIL_PARAM_LD_ST<HSAILloadparam, "ld_f64", GPR64, f64, HSAILArchUsesFlatAddr>;


  defm PARAMU8   : HSAIL_PARAM_LD_ST_ARG<HSAILloadparam_u8, "ld_arg_u8", GPR32, i32, HSAILArchGeneric>;
  defm PARAMU16  : HSAIL_PARAM_LD_ST_ARG<HSAILloadparam_u16, "ld_arg_u16", GPR32, i32, HSAILArchGeneric>;
  defm PARAMU32  : HSAIL_PARAM_LD_ST_ARG<HSAILloadparam, "ld_arg_u32", GPR32, i32, HSAILArchGeneric>;
  defm PARAMF32  : HSAIL_PARAM_LD_ST_ARG<HSAILloadparam, "ld_arg_f32", GPR32, f32, HSAILArchGeneric>;
  defm PARAMU64  : HSAIL_PARAM_LD_ST_ARG<HSAILloadparam, "ld_arg_u64", GPR64, i64, HSAILArchGeneric>;
  defm PARAMF64  : HSAIL_PARAM_LD_ST_ARG<HSAILloadparam, "ld_arg_f64", GPR64, f64, HSAILArchGeneric>;

  defm PARAMU8_flat   : HSAIL_PARAM_LD_ST_ARG<HSAILloadparam_u8, "ld_u8", GPR32, i32, HSAILArchUsesFlatAddr>;
  defm PARAMU16_flat  : HSAIL_PARAM_LD_ST_ARG<HSAILloadparam_u16, "ld_u16", GPR32, i32, HSAILArchUsesFlatAddr>;
  defm PARAMU32_flat  : HSAIL_PARAM_LD_ST_ARG<HSAILloadparam, "ld_u32", GPR32, i32, HSAILArchUsesFlatAddr>;
  defm PARAMF32_flat  : HSAIL_PARAM_LD_ST_ARG<HSAILloadparam, "ld_f32", GPR32, f32, HSAILArchUsesFlatAddr>;
  defm PARAMU64_flat  : HSAIL_PARAM_LD_ST_ARG<HSAILloadparam, "ld_u64", GPR64, i64, HSAILArchUsesFlatAddr>;
  defm PARAMF64_flat  : HSAIL_PARAM_LD_ST_ARG<HSAILloadparam, "ld_f64", GPR64, f64, HSAILArchUsesFlatAddr>;

  defm PARAMRWIMG  : HSAIL_PARAM_LD_ST<HSAILloadparam_rwimg, "ld_kernarg_rwimg", GPR64, i64, HSAILArchGeneric>;
  defm PARAMSAMP  : HSAIL_PARAM_LD_ST<HSAILloadparam_samp, "ld_kernarg_samp", GPR64, i64, HSAILArchGeneric>;
}

def HSAILloadparamptr_
  : SDNode<"HSAILISD::LOAD_PARAM_PTR", SDTypeProfile<1, 1, [SDTCisPtrTy<1>]>,
           [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue]>;

def HSAILloadparamptr_structbyval_
  : SDNode<"HSAILISD::LOAD_PARAM_PTR_STRUCT_BY_VAL", SDTypeProfile<1, 1, [SDTCisPtrTy<1>]>,
           [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue]>;


def HSAILloadparamptr
  : PatFrag<(ops node:$ptr),
            (HSAILloadparamptr_ node:$ptr)>;

def HSAILloadparamptr_structbyval
  : PatFrag<(ops node:$ptr),
            (HSAILloadparamptr_structbyval_ node:$ptr)>;

let  needSpecialProcessing = 1 in {
def global_ld_u32_struct_by_val :  HSAILInst<(outs GPR32:$dst), (ins i32imm:$src),
     				 "ld_kernarg_u32\t$dst, [$src]",
				 [(set (i32 GPR32:$dst), (HSAILloadparamptr_structbyval texternalsym:$src))]>,
                                 Requires<[ HSAILisKernel ]>;
def global_ld_u32_struct_by_val_arg :  HSAILInst<(outs GPR32:$dst), (ins i32imm:$src),
     				 "ld_arg_u32\t$dst, [$src]",
				 [(set (i32 GPR32:$dst), (HSAILloadparamptr_structbyval texternalsym:$src))]>,
                                 Requires<[ HSAILisNotKernel ]>;
}

let  needSpecialProcessing = 1 in {
def global_ld_u64_struct_by_val :  HSAILInst<(outs GPR64:$dst), (ins i32imm:$src),
     				 "ld_kernarg_u64\t$dst, [$src]",
				 [(set (i64 GPR64:$dst), (HSAILloadparamptr_structbyval texternalsym:$src))]>,
                                 Requires<[ HSAILisKernel ]>;
def global_ld_u64_struct_by_val_arg :  HSAILInst<(outs GPR64:$dst), (ins i32imm:$src),
     				 "ld_arg_u64\t$dst, [$src]",
				 [(set (i64 GPR64:$dst), (HSAILloadparamptr_structbyval texternalsym:$src))]>,
				 Requires<[ HSAILisNotKernel ]>;
}

multiclass HSAIL_PARAM_LD_ST_PTR<PatFrag loadParam, string asm, RegisterClass RC,
                                 ValueType vt, Predicate ArchType> {
  let needSpecialProcessing = 1 in {
    def LDpi        : HSAILInst<(outs RC:$d), (ins i32imm:$a),
                      AsmStr<asm, "\t$d, [$a]">.s,
                      [(set RC:$d, (vt (loadParam texternalsym:$a)))]>,
                      Requires<[ ArchType,  HSAILisKernel ]>;
  }
}
multiclass HSAIL_PARAM_LD_ST_PTR_NK<PatFrag loadParam, string asm, RegisterClass RC,
                                    ValueType vt, Predicate ArchType> {
  let needSpecialProcessing = 1 in {
  def LDpi_NK     : HSAILInst<(outs RC:$d), (ins i32imm:$a),
                      AsmStr<asm, "\t$d, [$a]">.s,
                      [(set RC:$d, (vt (loadParam texternalsym:$a)))]>,
                      Requires<[ ArchType,  HSAILisNotKernel ]>;

  }
}

// We'll use the custom inserter for the following decisions:
// if non-flat address mode (HSAILArchGeneric):
//   if (AMP && convert_flat_to_segment (amp_ptr_ftos is true):
//     insert ftos following the pointer load
//   else if ((AMP && !convert_flat_to_segment) || OCL):
//     do nothing: 
//       AMP pointer remains flat, use ld/st_global for access
//       OCL pointer remains a segment pointer.
// else: (flat-address mode: HSAILArchUseFlatAddr)
//   if (AMP): 
//     do nothing, 
//       ignore the amp_ptr_ftos setting
//       flat mode override amp_ptr_ftos, pointer remains flat
//   else if (OCL)
//     convert loaded pointer using stof.
//

let usesCustomInserter = 1, needSpecialProcessing=1, hasWidthOperand=1 in {
  defm PARAMPTR32 : HSAIL_PARAM_LD_ST_PTR<HSAILloadparamptr, "ld_kernarg_u32", GPR32, 
                                          i32, HSAILArchGeneric>;
  defm PARAMPTR64 : HSAIL_PARAM_LD_ST_PTR<HSAILloadparamptr, "ld_kernarg_u64", GPR64, 
                                          i64, HSAILArchGeneric>;

  defm PARAMPTR32_flat :  HSAIL_PARAM_LD_ST_PTR<HSAILloadparamptr, "ld_u32", GPR32, 
                                                i32, HSAILArchUsesFlatAddr>; 
  defm PARAMPTR64_flat :  HSAIL_PARAM_LD_ST_PTR<HSAILloadparamptr, "ld_u64", GPR64, 
                                                i64, HSAILArchUsesFlatAddr>;

  defm PARAMPTR32 : HSAIL_PARAM_LD_ST_PTR_NK<HSAILloadparamptr, "ld_arg_u32", GPR32,
                                          i32, HSAILArchGeneric>;
  defm PARAMPTR64 : HSAIL_PARAM_LD_ST_PTR_NK<HSAILloadparamptr, "ld_arg_u64", GPR64,
                                          i64, HSAILArchGeneric>;

  defm PARAMPTR32_flat :  HSAIL_PARAM_LD_ST_PTR_NK<HSAILloadparamptr, "ld_u32", GPR32,
                                                i32, HSAILArchUsesFlatAddr>;
  defm PARAMPTR64_flat :  HSAIL_PARAM_LD_ST_PTR_NK<HSAILloadparamptr, "ld_u64", GPR64,
                                                i64, HSAILArchUsesFlatAddr>;

}


////////////////////////////////////////////////////////////////////////////////
// ld (integer, singed and unsigned, 32-bit and 64-bit;
//     float, 32-bit and 64-bit; addr, 32bit and 64-bit)
multiclass HSAIL_LD<PatFrag op, RegisterClass RC, ValueType Ty,
                         string asm,
                         Predicate ArchType>
{
  let needSpecialProcessing = 1, hasAddress = 1, isLoad = 1 in {
    let VectorSize = 1, addressOpNum = 1 in {
      def _v1: HSAILInst<(outs RC:$dst), (ins MEMOP:$ptr, WIDTH_OPND:$w, ALIGN_OPND:$alignnment, CONST_OPERAND:$isConst),
                       "ld_"#asm#"\t$dst, $ptr",
                       [(set (Ty RC:$dst), (op ADDR:$ptr))]>,
                     VectorOperation, 
                     Requires<[ ArchType ]>;
    }

    let mayLoad = 1 in {
      let VectorSize = 2, addressOpNum = 2 in {
        def _v2 : HSAILInst<(outs RC:$dst1, RC:$dst2), (ins MEMOP:$ptr, WIDTH_OPND:$w, ALIGN_OPND:$alignnment, CONST_OPERAND:$isConst),
                             "ld_v2_"#asm#"\t($dst1, $dst2 ), $ptr", []>,
                           VectorOperation, Requires<[ ArchType ]>; 
      }
      let VectorSize = 3, addressOpNum = 3 in {
        def _v3 : HSAILInst<(outs RC:$dst1, RC:$dst2, RC:$dst3), (ins MEMOP:$ptr, WIDTH_OPND:$w, ALIGN_OPND:$alignnment, CONST_OPERAND:$isConst),
                             "ld_v3_"#asm#"\t($dst1, $dst2, $dst3 ), $ptr", []>,
                           VectorOperation, Requires<[ ArchType ]>; 
      }
      let VectorSize = 4, addressOpNum = 4 in {
        def _v4 : HSAILInst<(outs RC:$dst1, RC:$dst2, RC:$dst3, RC:$dst4), (ins MEMOP:$ptr, WIDTH_OPND:$w, ALIGN_OPND:$alignnment, CONST_OPERAND:$isConst),
                             "ld_v4_"#asm#"\t($dst1, $dst2, $dst3, $dst4 ), $ptr", []>,
                           VectorOperation, Requires<[ ArchType ]>; 
      }
    }
  }
}

defm flat_ld_u32 : HSAIL_LD<flat_load, GPR32, i32,
                                 "u32",
                                 HSAILArchGeneric>;
defm flat_ld_u64 : HSAIL_LD<flat_load, GPR64, i64,
                                 "u64",
                                 HSAILArchGeneric>;
defm flat_ld_f32 : HSAIL_LD<flat_load, GPR32, f32,
                                 "f32",
                                 HSAILArchGeneric>;
defm flat_ld_f64 : HSAIL_LD<flat_load, GPR64, f64,
                                 "f64",
                                 HSAILArchGeneric>;

defm global_ld_u32 : HSAIL_LD<global_load, GPR32, i32,
                                   "global_u32",
                                   HSAILArchGeneric>;
defm global_ld_u64 : HSAIL_LD<global_load, GPR64, i64,
                                   "global_u64",
                                   HSAILArchGeneric>;
defm global_ld_f32 : HSAIL_LD<global_load, GPR32, f32,
                                   "global_f32",
                                   HSAILArchGeneric>;
defm global_ld_f64 : HSAIL_LD<global_load, GPR64, f64,
                                   "global_f64",
                                   HSAILArchGeneric>;

defm constant_ld_u32 : HSAIL_LD<constant_load, GPR32, i32,
                                "readonly_u32",
                                HSAILArchGeneric>;
defm constant_ld_u64 : HSAIL_LD<constant_load, GPR64, i64,
                                "readonly_u64",
                                HSAILArchGeneric>;
defm constant_ld_f32 : HSAIL_LD<constant_load, GPR32, f32,
                                "readonly_f32",
                                HSAILArchGeneric>;
defm constant_ld_f64 : HSAIL_LD<constant_load, GPR64, f64,
                                "readonly_f64",
                                HSAILArchGeneric>;

defm group_ld_u32 : HSAIL_LD<group_load, GPR32, i32,
                                  "group_u32",
                                  HSAILArchGeneric>;
defm group_ld_u64 : HSAIL_LD<group_load, GPR64, i64,
                                  "group_u64",
                                  HSAILArchGeneric>;
defm group_ld_f32 : HSAIL_LD<group_load, GPR32, f32,
                                  "group_f32",
                                  HSAILArchGeneric>;
defm group_ld_f64 : HSAIL_LD<group_load, GPR64, f64,
                                  "group_f64",
                                  HSAILArchGeneric>;

defm private_ld_u32 : HSAIL_LD<private_load, GPR32, i32,
                                  "private_u32",
                                  HSAILArchGeneric>;
defm private_ld_u64 : HSAIL_LD<private_load, GPR64, i64,
                                  "private_u64",
                                  HSAILArchGeneric>;
defm private_ld_f32 : HSAIL_LD<private_load, GPR32, f32,
                                  "private_f32",
                                  HSAILArchGeneric>;
defm private_ld_f64 : HSAIL_LD<private_load, GPR64, f64,
                                  "private_f64",
                                  HSAILArchGeneric>;

// For architectures supporting a flat memory addressing we use
// the custom inserter  insert segment to flat or
// flat to segment conversion instructions if loaded value is a pointer 
let usesCustomInserter = 1 in {
  defm flat_ld_u32_flat : HSAIL_LD<flat_load, GPR32, i32,
                                 "u32",
                                 HSAILArchUsesFlatAddr>;
  defm flat_ld_u64_flat : HSAIL_LD<flat_load, GPR64, i64,
                                 "u64",
                                 HSAILArchUsesFlatAddr>;
}

  defm flat_ld_f32_flat : HSAIL_LD<flat_load, GPR32, f32,
                                 "f32",
                                 HSAILArchUsesFlatAddr>;
  defm flat_ld_f64_flat : HSAIL_LD<flat_load, GPR64, f64,
                                 "f64",
                                 HSAILArchUsesFlatAddr>;

let usesCustomInserter = 1 in {
  defm global_ld_u32_flat : HSAIL_LD<global_load, GPR32, i32,
                                   "u32",
                                   HSAILArchUsesFlatAddr>;
  defm global_ld_u64_flat : HSAIL_LD<global_load, GPR64, i64,
                                   "u64",
                                   HSAILArchUsesFlatAddr>;
}
  defm global_ld_f32_flat : HSAIL_LD<global_load, GPR32, f32,
                                   "f32",
                                   HSAILArchUsesFlatAddr>;
  defm global_ld_f64_flat : HSAIL_LD<global_load, GPR64, f64,
                                   "f64",
                                   HSAILArchUsesFlatAddr>;

let usesCustomInserter = 1 in {
  defm constant_ld_u32_flat : HSAIL_LD<constant_load, GPR32, i32,
                                "u32",
                                HSAILArchUsesFlatAddr>;
  defm constant_ld_u64_flat : HSAIL_LD<constant_load, GPR64, i64,
                                "u64",
                                HSAILArchUsesFlatAddr>;
}
  defm constant_ld_f32_flat : HSAIL_LD<constant_load, GPR32, f32,
                                "f32",
                                HSAILArchUsesFlatAddr>;
  defm constant_ld_f64_flat : HSAIL_LD<constant_load, GPR64, f64,
                                "f64",
                                HSAILArchUsesFlatAddr>;

let usesCustomInserter = 1 in {
  defm group_ld_u32_flat : HSAIL_LD<group_load, GPR32, i32,
                                  "u32",
                                  HSAILArchUsesFlatAddr>;
  defm group_ld_u64_flat : HSAIL_LD<group_load, GPR64, i64,
                                  "u64",
                                  HSAILArchUsesFlatAddr>;
}

  defm group_ld_f32_flat : HSAIL_LD<group_load, GPR32, f32,
                                  "f32",
                                  HSAILArchUsesFlatAddr>;
  defm group_ld_f64_flat : HSAIL_LD<group_load, GPR64, f64,
                                  "f64",
                                  HSAILArchUsesFlatAddr>;

let usesCustomInserter = 1 in {
  defm private_ld_u32_flat : HSAIL_LD<private_load, GPR32, i32,
                                  "u32",
                                  HSAILArchUsesFlatAddr>;
  defm private_ld_u64_flat : HSAIL_LD<private_load, GPR64, i64,
                                  "u64",
                                  HSAILArchUsesFlatAddr>;
}

  defm private_ld_f32_flat : HSAIL_LD<private_load, GPR32, f32,
                                  "f32",
                                  HSAILArchUsesFlatAddr>;
  defm private_ld_f64_flat : HSAIL_LD<private_load, GPR64, f64,
                                  "f64",
                                  HSAILArchUsesFlatAddr>;


let hasAddress = 1, mayLoad = 1, isPseudo = 1 in {
  def spill_ld_b1 : HSAILInst<(outs CR:$dst), (ins MEMOP:$ptr, WIDTH_OPND:$w,
                              ALIGN_OPND:$alignnment, CONST_OPERAND:$isConst),
                              "ld_spill_b1\t$dst, $ptr", []>,
                              Requires<[HSAILArchGeneric]>;

  def spill_ld_b1_flat : HSAILInst<(outs CR:$dst), (ins MEMOP:$ptr, WIDTH_OPND:$w,
                              ALIGN_OPND:$alignnment, CONST_OPERAND:$isConst),
                              "ld_spill_b1\t$dst, $ptr", []>,
                              Requires<[HSAILArchUsesFlatAddr]>;
}

let needSpecialProcessing = 1 in {
  defm spill_ld_u32 : HSAIL_LD<spill_load, GPR32, i32,
                                   "spill_u32", HSAILArchGeneric>;
  defm spill_ld_u64 : HSAIL_LD<spill_load, GPR64, i64,
                                   "spill_u64", HSAILArchGeneric>;
  defm spill_ld_f32 : HSAIL_LD<spill_load, GPR32, f32,
                                   "spill_f32", HSAILArchGeneric>;
  defm spill_ld_f64 : HSAIL_LD<spill_load, GPR64, f64,
                                   "spill_f64", HSAILArchGeneric>;

  defm spill_ld_u32_flat : HSAIL_LD<spill_load, GPR32, i32,
                                   "u32", HSAILArchUsesFlatAddr>;
  defm spill_ld_u64_flat : HSAIL_LD<spill_load, GPR64, i64,
                                   "u64", HSAILArchUsesFlatAddr>;
  defm spill_ld_f32_flat : HSAIL_LD<spill_load, GPR32, f32,
                                   "f32", HSAILArchUsesFlatAddr>;
  defm spill_ld_f64_flat : HSAIL_LD<spill_load, GPR64, f64,
                                   "f64", HSAILArchUsesFlatAddr>;
}

////////////////////////////////////////////////////////////////////////////////
// Extended loads

// extended 8-bit loads
defm flat_ext_ld_u32_u8       : HSAIL_LD<extloadi8_flat,
                                GPR32, i32, "u8", HSAILArchGeneric>;
defm global_ext_ld_u32_u8     : HSAIL_LD<extloadi8_global,
                                GPR32, i32, "global_u8", HSAILArchGeneric>;
defm constant_ext_ld_u32_u8   : HSAIL_LD<extloadi8_constant,
                                GPR32, i32, "readonly_u8", HSAILArchGeneric>;
defm group_ext_ld_u32_u8      : HSAIL_LD<extloadi8_group,
                                GPR32, i32, "group_u8", HSAILArchGeneric>;
defm private_ext_ld_u32_u8    : HSAIL_LD<extloadi8_private, 
                                GPR32, i32, "private_u8", HSAILArchGeneric>;

defm flat_sext_ld_s32_s8      : HSAIL_LD<sextloadi8_flat, 
                                GPR32, i32, "s8", HSAILArchGeneric>;
defm global_sext_ld_s32_s8    : HSAIL_LD<sextloadi8_global, 
                                GPR32, i32, "global_s8", HSAILArchGeneric>;
defm constant_sext_ld_s32_s8  : HSAIL_LD<sextloadi8_constant, 
                                GPR32, i32, "readonly_s8", HSAILArchGeneric>;
defm group_sext_ld_s32_s8     : HSAIL_LD<sextloadi8_group, 
                                GPR32, i32, "group_s8", HSAILArchGeneric>;
defm private_sext_ld_s32_s8   : HSAIL_LD<sextloadi8_private, 
                                GPR32, i32, "private_s8", HSAILArchGeneric>;



defm flat_zext_ld_u32_u8      : HSAIL_LD<zextloadi8_flat, 
                                GPR32, i32, "u8", HSAILArchGeneric>;
defm global_zext_ld_u32_u8    : HSAIL_LD<zextloadi8_global, 
                                GPR32, i32, "global_u8", HSAILArchGeneric>;
defm constant_zext_ld_u32_u8  : HSAIL_LD<zextloadi8_constant, 
                                GPR32, i32, "readonly_u8", HSAILArchGeneric>;
defm group_zext_ld_u32_u8     : HSAIL_LD<zextloadi8_group, 
                                GPR32, i32, "group_u8", HSAILArchGeneric>;
defm private_zext_ld_u32_u8   : HSAIL_LD<zextloadi8_private, 
                                GPR32, i32, "private_u8", HSAILArchGeneric>;

// flat address, extended 8-bit loads, no custom inserter since no 8 bit pointers
defm flat_ext_ld_u32_u8_flat       : HSAIL_LD<extloadi8_flat,
                                GPR32, i32, "u8", HSAILArchUsesFlatAddr>;
defm global_ext_ld_u32_u8_flat     : HSAIL_LD<extloadi8_global,
                                GPR32, i32, "u8", HSAILArchUsesFlatAddr>;
defm constant_ext_ld_u32_u8_flat   : HSAIL_LD<extloadi8_constant,
                                GPR32, i32, "u8", HSAILArchUsesFlatAddr>;
defm group_ext_ld_u32_u8_flat      : HSAIL_LD<extloadi8_group,
                                GPR32, i32, "u8", HSAILArchUsesFlatAddr>;
defm private_ext_ld_u32_u8_flat    : HSAIL_LD<extloadi8_private, 
                                GPR32, i32, "u8", HSAILArchUsesFlatAddr>;

defm flat_sext_ld_s32_s8_flat      : HSAIL_LD<sextloadi8_flat, 
                                GPR32, i32, "s8", HSAILArchUsesFlatAddr>;
defm global_sext_ld_s32_s8_flat    : HSAIL_LD<sextloadi8_global, 
                                GPR32, i32, "s8", HSAILArchUsesFlatAddr>;
defm constant_sext_ld_s32_s8_flat  : HSAIL_LD<sextloadi8_constant, 
                                GPR32, i32, "s8", HSAILArchUsesFlatAddr>;
defm group_sext_ld_s32_s8_flat     : HSAIL_LD<sextloadi8_group, 
                                GPR32, i32, "s8", HSAILArchUsesFlatAddr>;
defm private_sext_ld_s32_s8_flat   : HSAIL_LD<sextloadi8_private, 
                                GPR32, i32, "s8", HSAILArchUsesFlatAddr>;

defm flat_zext_ld_u32_u8_flat      : HSAIL_LD<zextloadi8_flat, 
                                GPR32, i32, "u8", HSAILArchUsesFlatAddr>;
defm global_zext_ld_u32_u8_flat    : HSAIL_LD<zextloadi8_global, 
                                GPR32, i32, "u8", HSAILArchUsesFlatAddr>;
defm constant_zext_ld_u32_u8_flat  : HSAIL_LD<zextloadi8_constant, 
                                GPR32, i32, "u8", HSAILArchUsesFlatAddr>;
defm group_zext_ld_u32_u8_flat     : HSAIL_LD<zextloadi8_group, 
                                GPR32, i32, "u8", HSAILArchUsesFlatAddr>;
defm private_zext_ld_u32_u8_flat   : HSAIL_LD<zextloadi8_private, 
                                GPR32, i32, "u8", HSAILArchUsesFlatAddr>;

// extended 16-bit loads
defm flat_ext_ld_u32_u16      : HSAIL_LD<extloadi16_flat,
                                GPR32, i32, "u16", HSAILArchGeneric>;
defm global_ext_ld_u32_u16    : HSAIL_LD<extloadi16_global, 
                                GPR32, i32, "global_u16", HSAILArchGeneric>;
defm constant_ext_ld_u32_u16  : HSAIL_LD<extloadi16_constant, 
                                GPR32, i32, "readonly_u16", HSAILArchGeneric>;
defm group_ext_ld_u32_u16     : HSAIL_LD<extloadi16_group, 
                                GPR32, i32, "group_u16", HSAILArchGeneric>;
defm private_ext_ld_u32_u16   : HSAIL_LD<extloadi16_private, 
                                GPR32, i32, "private_u16", HSAILArchGeneric>;

defm flat_sext_ld_s32_s16     : HSAIL_LD<sextloadi16_flat, 
                                GPR32, i32, "s16", HSAILArchGeneric>;
defm global_sext_ld_s32_s16   : HSAIL_LD<sextloadi16_global, 
                                GPR32, i32, "global_s16", HSAILArchGeneric>;
defm constant_sext_ld_s32_s16 : HSAIL_LD<sextloadi16_constant, 
                                GPR32, i32, "readonly_s16", HSAILArchGeneric>;
defm group_sext_ld_s32_s16    : HSAIL_LD<sextloadi16_group, 
                                GPR32, i32, "group_s16", HSAILArchGeneric>;
defm private_sext_ld_s32_s16  : HSAIL_LD<sextloadi16_private, 
                                GPR32, i32, "private_s16", HSAILArchGeneric>;

defm flat_zext_ld_u32_u16     : HSAIL_LD<zextloadi16_flat, 
                                GPR32, i32, "u16", HSAILArchGeneric>;
defm global_zext_ld_u32_u16   : HSAIL_LD<zextloadi16_global, 
                                GPR32, i32, "global_u16", HSAILArchGeneric>;
defm constant_zext_ld_u32_u16 : HSAIL_LD<zextloadi16_constant, 
                                GPR32, i32, "readonly_u16", HSAILArchGeneric>;
defm group_zext_ld_u32_u16    : HSAIL_LD<zextloadi16_group, 
                                GPR32, i32, "group_u16", HSAILArchGeneric>;
defm private_zext_ld_u32_u16  : HSAIL_LD<zextloadi16_private, 
                                GPR32, i32, "private_u16", HSAILArchGeneric>;

// flat address, extended 16-bit loads
defm flat_ext_ld_u32_u16_flat      : HSAIL_LD<extloadi16_flat,
                                GPR32, i32, "u16", HSAILArchUsesFlatAddr>;
defm global_ext_ld_u32_u16_flat    : HSAIL_LD<extloadi16_global, 
                                GPR32, i32, "u16", HSAILArchUsesFlatAddr>;
defm constant_ext_ld_u32_u16_flat  : HSAIL_LD<extloadi16_constant, 
                                GPR32, i32, "u16", HSAILArchUsesFlatAddr>;
defm group_ext_ld_u32_u16_flat     : HSAIL_LD<extloadi16_group, 
                                GPR32, i32, "u16", HSAILArchUsesFlatAddr>;
defm private_ext_ld_u32_u16_flat   : HSAIL_LD<extloadi16_private, 
                                GPR32, i32, "u16", HSAILArchUsesFlatAddr>;

defm flat_sext_ld_s32_s16_flat     : HSAIL_LD<sextloadi16_flat, 
                                GPR32, i32, "s16", HSAILArchUsesFlatAddr>;
defm global_sext_ld_s32_s16_flat   : HSAIL_LD<sextloadi16_global, 
                                GPR32, i32, "s16", HSAILArchUsesFlatAddr>;
defm constant_sext_ld_s32_s16_flat : HSAIL_LD<sextloadi16_constant, 
                                GPR32, i32, "s16", HSAILArchUsesFlatAddr>;
defm group_sext_ld_s32_s16_flat    : HSAIL_LD<sextloadi16_group, 
                                GPR32, i32, "s16", HSAILArchUsesFlatAddr>;
defm private_sext_ld_s32_s16_flat  : HSAIL_LD<sextloadi16_private, 
                                GPR32, i32, "s16", HSAILArchUsesFlatAddr>;

defm flat_zext_ld_u32_u16_flat     : HSAIL_LD<zextloadi16_flat, 
                                GPR32, i32, "u16", HSAILArchUsesFlatAddr>;
defm global_zext_ld_u32_u16_flat   : HSAIL_LD<zextloadi16_global, 
                                GPR32, i32, "u16", HSAILArchUsesFlatAddr>;
defm constant_zext_ld_u32_u16_flat : HSAIL_LD<zextloadi16_constant, 
                                GPR32, i32, "u16", HSAILArchUsesFlatAddr>;
defm group_zext_ld_u32_u16_flat    : HSAIL_LD<zextloadi16_group, 
                                GPR32, i32, "u16", HSAILArchUsesFlatAddr>;
defm private_zext_ld_u32_u16_flat  : HSAIL_LD<zextloadi16_private, 
                                GPR32, i32, "u16", HSAILArchUsesFlatAddr>;


////////////////////////////////////////////////////////////////////////////////
// st (integer, singed and unsigned, 32-bit and 64-bit;
//     float, 32-bit and 64-bit; addr, 32bit and 64-bit)
multiclass HSAIL_ST<PatFrag op, RegisterClass RC, ValueType Ty,
                         string asm, Predicate ArchType>
{
  let needSpecialProcessing = 1, hasAddress = 1, isStore = 1 in {
    let VectorSize = 1, addressOpNum = 1 in {
      def _v1 : HSAILInst<(outs), (ins RC:$val, MEMOP:$ptr, ALIGN_OPND:$alignnment),
                          "st_"#asm#"\t$val, $ptr",
                          [(op (Ty RC:$val), ADDR:$ptr)]>,
                          VectorOperation,
                          Requires<[ArchType]>; 
    }

    let mayStore = 1 in {
      let VectorSize = 2, addressOpNum = 2 in {
        def _v2 : HSAILInst<(outs), 
                            (ins RC:$val1, RC:$val2, MEMOP:$ptr, ALIGN_OPND:$alignnment),
                            "st_v2_"#asm#"\t($val1, $val2 ), $ptr",
                            []>, VectorOperation, Requires<[ArchType]>; 
      }
      let VectorSize = 3, addressOpNum = 3 in {
        def _v3 : HSAILInst<(outs), 
                            (ins RC:$val1, RC:$val2, RC:$val3, MEMOP:$ptr, ALIGN_OPND:$alignnment),
                            "st_v3_"#asm#"\t($val1, $val2, $val3 ), $ptr",
                            []>, VectorOperation, Requires<[ArchType]>; 
      }
      let VectorSize = 4, addressOpNum = 4 in {
        def _v4 : HSAILInst<(outs), 
                            (ins RC:$val1, RC:$val2, 
                                 RC:$val3, RC:$val4, MEMOP:$ptr, ALIGN_OPND:$alignnment),
                            "st_v4_"#asm#"\t($val1, $val2, $val3, $val4 ), $ptr",
                            []>, VectorOperation, Requires<[ArchType]>; 
      }
    }
  }
}

defm flat_st_u32 : HSAIL_ST<flat_store, GPR32, i32, "u32",
                                 HSAILArchGeneric>;
defm flat_st_u64 : HSAIL_ST<flat_store, GPR64, i64, "u64",
                                 HSAILArchGeneric>;
defm flat_st_f32 : HSAIL_ST<flat_store, GPR32, f32, "f32",
                                 HSAILArchGeneric>;
defm flat_st_f64 : HSAIL_ST<flat_store, GPR64, f64, "f64",
                                 HSAILArchGeneric>;

defm global_st_u32 : HSAIL_ST<global_store, GPR32, i32, "global_u32",
                                   HSAILArchGeneric>;
defm global_st_u64 : HSAIL_ST<global_store, GPR64, i64, "global_u64",
                                   HSAILArchGeneric>;
defm global_st_f32 : HSAIL_ST<global_store, GPR32, f32, "global_f32",
                                   HSAILArchGeneric>;
defm global_st_f64 : HSAIL_ST<global_store, GPR64, f64, "global_f64",
                                   HSAILArchGeneric>;

defm group_st_u32 : HSAIL_ST<group_store, GPR32, i32, "group_u32",
                                  HSAILArchGeneric>;
defm group_st_u64 : HSAIL_ST<group_store, GPR64, i64, "group_u64",
                                  HSAILArchGeneric>;
defm group_st_f32 : HSAIL_ST<group_store, GPR32, f32, "group_f32",
                                  HSAILArchGeneric>;
defm group_st_f64 : HSAIL_ST<group_store, GPR64, f64, "group_f64",
                                  HSAILArchGeneric>;

defm private_st_u32 : HSAIL_ST<private_store, GPR32, i32,
                          "private_u32", HSAILArchGeneric>;
defm private_st_u64 : HSAIL_ST<private_store, GPR64, i64,
                          "private_u64", HSAILArchGeneric>;
defm private_st_f32 : HSAIL_ST<private_store, GPR32, f32,
                          "private_f32", HSAILArchGeneric>;
defm private_st_f64 : HSAIL_ST<private_store, GPR64, f64,
                          "private_f64", HSAILArchGeneric>;

// flat address, stores
let usesCustomInserter = 1 in {
defm flat_st_u32_flat : HSAIL_ST<flat_store, GPR32, i32, "u32",
                                 HSAILArchUsesFlatAddr>;
defm flat_st_u64_flat : HSAIL_ST<flat_store, GPR64, i64, "u64",
                                 HSAILArchUsesFlatAddr>;
}
defm flat_st_f32_flat : HSAIL_ST<flat_store, GPR32, f32, "f32",
                                 HSAILArchUsesFlatAddr>;
defm flat_st_f64_flat : HSAIL_ST<flat_store, GPR64, f64, "f64",
                                 HSAILArchUsesFlatAddr>;

let usesCustomInserter = 1 in {
defm global_st_u32_flat : HSAIL_ST<global_store, GPR32, i32, "u32",
                                   HSAILArchUsesFlatAddr>;
defm global_st_u64_flat : HSAIL_ST<global_store, GPR64, i64, "u64",
                                   HSAILArchUsesFlatAddr>;
}
defm global_st_f32_flat : HSAIL_ST<global_store, GPR32, f32, "f32",
                                   HSAILArchUsesFlatAddr>;
defm global_st_f64_flat : HSAIL_ST<global_store, GPR64, f64, "f64",
                                   HSAILArchUsesFlatAddr>;

let usesCustomInserter = 1 in {
defm group_st_u32_flat : HSAIL_ST<group_store, GPR32, i32, "u32",
                                  HSAILArchUsesFlatAddr>;
defm group_st_u64_flat : HSAIL_ST<group_store, GPR64, i64, "u64",
                                  HSAILArchUsesFlatAddr>;
}

defm group_st_f32_flat : HSAIL_ST<group_store, GPR32, f32, "f32",
                                  HSAILArchUsesFlatAddr>;
defm group_st_f64_flat : HSAIL_ST<group_store, GPR64, f64, "f64",
                                  HSAILArchUsesFlatAddr>;

let usesCustomInserter = 1 in {
defm private_st_u32_flat : HSAIL_ST<private_store, GPR32, i32, "u32",
                                  HSAILArchUsesFlatAddr>;
defm private_st_u64_flat : HSAIL_ST<private_store, GPR64, i64, "u64",
                                  HSAILArchUsesFlatAddr>;
}
defm private_st_f32_flat : HSAIL_ST<private_store, GPR32, f32, "f32",
                                  HSAILArchUsesFlatAddr>;
defm private_st_f64_flat : HSAIL_ST<private_store, GPR64, f64, "f64",
                                  HSAILArchUsesFlatAddr>;

// spill stores
let hasAddress = 1, mayStore = 1, isPseudo = 1 in {
  def spill_st_b1 : HSAILInst<(outs), (ins CR:$val, MEMOP:$ptr,
                                       ALIGN_OPND:$alignnment),
                              "st_spill_b1\t$val, $ptr", []>,
                              Requires<[HSAILArchGeneric]>;

  def spill_st_b1_flat : HSAILInst<(outs), (ins CR:$val, MEMOP:$ptr,
                                       ALIGN_OPND:$alignnment),
                              "st_spill_b1\t$val, $ptr", []>,
                              Requires<[HSAILArchUsesFlatAddr]>;
}

defm spill_st_u32 : HSAIL_ST<spill_store, GPR32, i32,
                                 "spill_u32", HSAILArchGeneric>;
defm spill_st_u64 : HSAIL_ST<spill_store, GPR64, i64,
                                 "spill_u64", HSAILArchGeneric>;
defm spill_st_f32 : HSAIL_ST<spill_store, GPR32, f32,
                                 "spill_f32", HSAILArchGeneric>;
defm spill_st_f64 : HSAIL_ST<spill_store, GPR64, f64,
                                 "spill_f64", HSAILArchGeneric>;

defm spill_st_u32_flat : HSAIL_ST<spill_store, GPR32, i32,
                                 "u32", HSAILArchUsesFlatAddr>;
defm spill_st_u64_flat : HSAIL_ST<spill_store, GPR64, i64,
                                 "u64", HSAILArchUsesFlatAddr>;
defm spill_st_f32_flat : HSAIL_ST<spill_store, GPR32, f32,
                                 "f32", HSAILArchUsesFlatAddr>;
defm spill_st_f64_flat : HSAIL_ST<spill_store, GPR64, f64,
                                 "f64", HSAILArchUsesFlatAddr>;

////////////////////////////////////////////////////////////////////////////////
// Truncating stores

// truncating 8-bit stores
defm flat_truncst_u32_u8     : HSAIL_ST<truncstorei8_flat,
                                                GPR32, i32, "u8",
                                                HSAILArchGeneric>;
defm global_truncst_u32_u8   : HSAIL_ST<truncstorei8_global,
                                                GPR32, i32, "global_u8",
                                                HSAILArchGeneric>;
defm group_truncst_u32_u8    : HSAIL_ST<truncstorei8_group,
                                                GPR32, i32, "group_u8",
                                                HSAILArchGeneric>;
defm private_truncst_u32_u8  : HSAIL_ST<truncstorei8_private,
                                           GPR32, i32, "private_u8",
                                           HSAILArchGeneric>;

// flat address, truncating 8-bit stores
defm flat_truncst_u32_u8_flat     : HSAIL_ST<truncstorei8_flat,
                                                GPR32, i32, "u8",
                                                HSAILArchUsesFlatAddr>;
defm global_truncst_u32_u8_flat   : HSAIL_ST<truncstorei8_global,
                                                GPR32, i32, "u8",
                                                HSAILArchUsesFlatAddr>;
defm group_truncst_u32_u8_flat    : HSAIL_ST<truncstorei8_group,
                                                GPR32, i32, "u8",
                                                HSAILArchUsesFlatAddr>;
defm private_truncst_u32_u8_flat  : HSAIL_ST<truncstorei8_private,
                                                GPR32, i32, "u8",
                                                HSAILArchUsesFlatAddr>;

// truncating 16-bit stores
defm flat_truncst_u32_u16    : HSAIL_ST<truncstorei16_flat,
                                                GPR32, i32, "u16",
                                                HSAILArchGeneric>;
defm global_truncst_u32_u16  : HSAIL_ST<truncstorei16_global,
                                                GPR32, i32, "global_u16",
                                                HSAILArchGeneric>;
defm group_truncst_u32_u16   : HSAIL_ST<truncstorei16_group,
                                                GPR32, i32, "group_u16",
                                                HSAILArchGeneric>;
defm private_truncst_u32_u16 : HSAIL_ST<truncstorei16_private,
                                                GPR32, i32, "private_u16",
                                                HSAILArchGeneric>;

// flat address, truncating 16-bit stores
defm flat_truncst_u32_u16_flat    : HSAIL_ST<truncstorei16_flat,
                                                GPR32, i32, "u16",
                                                HSAILArchUsesFlatAddr>;
defm global_truncst_u32_u16_flat  : HSAIL_ST<truncstorei16_global,
                                                GPR32, i32, "u16",
                                                HSAILArchUsesFlatAddr>;
defm group_truncst_u32_u16_flat   : HSAIL_ST<truncstorei16_group,
                                                GPR32, i32, "u16",
                                                HSAILArchUsesFlatAddr>;
defm private_truncst_u32_u16_flat : HSAIL_ST<truncstorei16_private,
                                                GPR32, i32, "u16",
                                                HSAILArchUsesFlatAddr>;


////////////////////////////////////////////////////////////////////////////////
// load memory address

//LDA flat and Group address
multiclass LdaAddr<SDNode op, string asm,  Predicate ArchType>
{
  let hasAddress = 1, addressOpNum = 1 in {
    def _addr32 : HSAILInst<(outs GPR32:$dst), (ins MEMOP:$addr), 
        asm#"_u32"#"\t$dst, $addr",
        [(set (i32 GPR32:$dst), (op ADDR:$addr))]>,
        Requires<[In32BitMode, ArchType]>;

    def _addr64 : HSAILInst<(outs GPR64:$dst), (ins MEMOP:$addr), 
        asm#"_u64"#"\t$dst, $addr",
        [(set (i64 GPR64:$dst), (op ADDR:$addr))]>,
        Requires<[In64BitMode, ArchType]>;
  }
}

let needSpecialProcessing = 1 in {
  // TODO_HSA: is lda_flat legal? See Spec "Additional Information About lda"
  defm lda_flat      : LdaAddr<IL_lda_flat, "lda", HSAILArchGeneric>;
  defm ldas_group    : LdaAddr<IL_lda_group, "lda_group", HSAILArchGeneric>;
  defm ldas_global : LdaAddr<IL_lda_global,"lda_global", HSAILArchGeneric>;
  defm ldas_constant : LdaAddr<IL_lda_readonly, "lda_readonly", HSAILArchGeneric>;
  defm ldas_private  : LdaAddr<IL_lda_private, "lda_private", HSAILArchGeneric>;
}


// flat address lda, requires an stof instruction for flat address architecture.
let usesCustomInserter = 1, needSpecialProcessing = 1 in {
  defm ldas_global_flat   : LdaAddr<IL_lda_global, "lda", HSAILArchUsesFlatAddr>;
  defm ldas_group_flat    : LdaAddr<IL_lda_group, "lda", HSAILArchUsesFlatAddr>;
  defm ldas_private_flat  : LdaAddr<IL_lda_private, "lda", HSAILArchUsesFlatAddr>;
}
