//===--------------------- HSAILILNodes.td - HSAIL nodes ------------------===//

//===----------------------------------------------------------------------===//
// Call/Return DAG Nodes
//===----------------------------------------------------------------------===//
def IL_callseq_start : SDNode<"ISD::CALLSEQ_START", SDTIL_CallSeqStart,
                       [SDNPHasChain, SDNPOutGlue]>;

def IL_callseq_end : SDNode<"ISD::CALLSEQ_END",   SDTIL_CallSeqEnd,
                     [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def IL_call : SDNode<"HSAILISD::CALL", SDTIL_Call,
              [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def IL_retflag : SDNode<"HSAILISD::RET_FLAG", SDTNone,
                 [SDNPHasChain, SDNPOptInGlue]>;

def IL_lda_flat   : SDNode<"HSAILISD::LDA_FLAT", SDTypeProfile<1, 1, []>, []>;
def IL_lda_global : SDNode<"HSAILISD::LDA_GLOBAL", SDTypeProfile<1, 1, []>, []>;
def IL_lda_group  : SDNode<"HSAILISD::LDA_GROUP", SDTypeProfile<1, 1, []>, []>;
def IL_lda_private : SDNode<"HSAILISD::LDA_PRIVATE", SDTypeProfile<1, 1, []>, []>;
def IL_lda_readonly : SDNode<"HSAILISD::LDA_READONLY", SDTypeProfile<1, 1, []>, []>;

def IL_trunc_b1 : SDNode<"HSAILISD::TRUNC_B1", SDTypeProfile<1, 1, []>, []>;

def STARGNode_U8 : SDNode<"HSAILISD::ST_SCALAR_ARG_U8", SDTypeProfile<0, 3, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPMayStore]>; 

def STARGNode_U16 : SDNode<"HSAILISD::ST_SCALAR_ARG_U16", SDTypeProfile<0, 3, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPMayStore]>; 

def STARGNode : SDNode<"HSAILISD::ST_SCALAR_ARG", SDTypeProfile<0, 3, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPMayStore]>; 

def LDRETNode_U8 : SDNode<"HSAILISD::LD_SCALAR_RET_U8", SDTypeProfile<1, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPMayLoad]>;

def LDRETNode_U16 : SDNode<"HSAILISD::LD_SCALAR_RET_U16", SDTypeProfile<1, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPMayLoad]>;

def LDRETNode : SDNode<"HSAILISD::LD_SCALAR_RET", SDTypeProfile<1, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPMayLoad]>;

def STRETNode_U8  :  SDNode<"HSAILISD::ST_SCALAR_RET_U8", SDTypeProfile<0, 3, []>,
    [SDNPHasChain, SDNPMayStore]>;

def STRETNode_U16 :  SDNode<"HSAILISD::ST_SCALAR_RET_U16", SDTypeProfile<0, 3, []>,
    [SDNPHasChain, SDNPMayStore]>;

def STRETNode :  SDNode<"HSAILISD::ST_SCALAR_RET", SDTypeProfile<0, 3, []>,
    [SDNPHasChain, SDNPMayStore]>;

def LDPARAMNode_U8 : SDNode<"HSAILISD::LD_SCALAR_ARG_U8", SDTypeProfile<1, 2, []>,
    [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPMayLoad]>;

def LDPARAMNode_U16 : SDNode<"HSAILISD::LD_SCALAR_ARG_U16", SDTypeProfile<1, 2, []>,
    [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPMayLoad]>;

def LDPARAMNode : SDNode<"HSAILISD::LD_SCALAR_ARG", SDTypeProfile<1, 2, []>,
    [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPMayLoad]>;

def ARGDeclNode_U8 : SDNode<"HSAILISD::ARG_DECL_U8", SDTypeProfile<0, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPSideEffect]>;

def ARGDeclNode_U16 : SDNode<"HSAILISD::ARG_DECL_U16", SDTypeProfile<0, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPSideEffect]>;

def ARGDeclNode_U32 : SDNode<"HSAILISD::ARG_DECL_U32", SDTypeProfile<0, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPSideEffect]>;

def ARGDeclNode_F32 : SDNode<"HSAILISD::ARG_DECL_F32", SDTypeProfile<0, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPSideEffect]>;

def ARGDeclNode_U64 : SDNode<"HSAILISD::ARG_DECL_U64", SDTypeProfile<0, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPSideEffect]>;

def ARGDeclNode_F64 : SDNode<"HSAILISD::ARG_DECL_F64", SDTypeProfile<0, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPSideEffect]>;

def RETDeclNode_U8 : SDNode<"HSAILISD::RET_DECL_U8", SDTypeProfile<0, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPSideEffect]>;

def RETDeclNode_U16 : SDNode<"HSAILISD::RET_DECL_U16", SDTypeProfile<0, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPSideEffect]>;

def RETDeclNode_U32 : SDNode<"HSAILISD::RET_DECL_U32", SDTypeProfile<0, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPSideEffect]>;

def RETDeclNode_F32 : SDNode<"HSAILISD::RET_DECL_F32", SDTypeProfile<0, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPSideEffect]>;

def RETDeclNode_U64 : SDNode<"HSAILISD::RET_DECL_U64", SDTypeProfile<0, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPSideEffect]>;

def RETDeclNode_F64 : SDNode<"HSAILISD::RET_DECL_F64", SDTypeProfile<0, 2, []>,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPSideEffect]>;
