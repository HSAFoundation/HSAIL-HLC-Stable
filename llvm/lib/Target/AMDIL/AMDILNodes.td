//===- AMDILNodes.td - AMD IL nodes ------------===//
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
// to the U.S. Export Administration Regulations (“EAR”), (15 C.F.R. Sections
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
// Industry and Security’s website at http://www.bis.doc.gov/.
//
//==-----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Conversion DAG Nodes
//===----------------------------------------------------------------------===//
// Double to Single conversion
def IL_d2f : SDNode<"AMDILISD::DP_TO_FP", SDTIL_DPToFPOp>;

def IL_ubit_extract : SDNode<"AMDILISD::UBIT_EXTRACT",
  SDTIL_IntTernaryOp, []>;
def IL_ibit_extract : SDNode<"AMDILISD::IBIT_EXTRACT",
  SDTIL_IntTernaryOp, []>;
def IL_bfi : SDNode<"AMDILISD::BFI",
  SDTIL_IntTernaryOp, []>;

def IL_ubit_insert : SDNode<"AMDILISD::UBIT_INSERT",
  SDTIL_BitInsert, []>;


//===----------------------------------------------------------------------===//
// Flow Control DAG Nodes
//===----------------------------------------------------------------------===//
def IL_brcond      : SDNode<"AMDILISD::BRANCH_COND", SDTIL_BRCond, [SDNPHasChain]>;

//===----------------------------------------------------------------------===//
// Comparison DAG Nodes
//===----------------------------------------------------------------------===//
def IL_cmp       : SDNode<"AMDILISD::CMP", SDTIL_Cmp>;

//===----------------------------------------------------------------------===//
// Call/Return DAG Nodes
//===----------------------------------------------------------------------===//
def IL_callseq_start : SDNode<"ISD::CALLSEQ_START", SDTIL_CallSeqStart,
    [SDNPHasChain, SDNPOutGlue]>;
def IL_callseq_end   : SDNode<"ISD::CALLSEQ_END",   SDTIL_CallSeqEnd,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;
def IL_call      : SDNode<"AMDILISD::CALL", SDTIL_Call,
    [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]>;

def IL_retflag       : SDNode<"AMDILISD::RET_FLAG", SDTNone,
    [SDNPHasChain, SDNPOptInGlue]>;

//===----------------------------------------------------------------------===//
// Arithmetic DAG Nodes
//===----------------------------------------------------------------------===//
// Address modification nodes
def IL_addaddrri : SDNode<"AMDILISD::ADDADDR", SDTIL_AddAddrri,
    [SDNPCommutative, SDNPAssociative]>;
def IL_addaddrir : SDNode<"AMDILISD::ADDADDR", SDTIL_AddAddrir,
    [SDNPCommutative, SDNPAssociative]>;

//===----------------------------------------------------------------------===//
// Integer functions
//===----------------------------------------------------------------------===//
def IL_umul        : SDNode<"AMDILISD::UMUL"    , SDTIntBinOp,
    [SDNPCommutative, SDNPAssociative]>;

//===----------------------------------------------------------------------===//
// Double functions
//===----------------------------------------------------------------------===//
def IL_dcreate     : SDNode<"AMDILISD::DCREATE"   , SDTIL_DCreate>;
def IL_dcomphi     : SDNode<"AMDILISD::DCOMPHI"     , SDTIL_DComp>;
def IL_dcomplo     : SDNode<"AMDILISD::DCOMPLO"     , SDTIL_DComp>;
def IL_dcreate2     : SDNode<"AMDILISD::DCREATE2"   , SDTIL_DCreate2>;
def IL_dcomphi2     : SDNode<"AMDILISD::DCOMPHI2"     , SDTIL_DComp2>;
def IL_dcomplo2     : SDNode<"AMDILISD::DCOMPLO2"     , SDTIL_DComp2>;

//===----------------------------------------------------------------------===//
// Long functions
//===----------------------------------------------------------------------===//
def IL_lcreate     : SDNode<"AMDILISD::LCREATE"   , SDTIL_LCreate>;
def IL_lcreate2    : SDNode<"AMDILISD::LCREATE2"   , SDTIL_LCreate2>;
def IL_lcomphi     : SDNode<"AMDILISD::LCOMPHI"     , SDTIL_LComp>;
def IL_lcomphi2    : SDNode<"AMDILISD::LCOMPHI2"     , SDTIL_LComp2>;
def IL_lcomplo     : SDNode<"AMDILISD::LCOMPLO"     , SDTIL_LComp>;
def IL_lcomplo2    : SDNode<"AMDILISD::LCOMPLO2"     , SDTIL_LComp2>;

//===----------------------------------------------------------------------===//
// Vector functions
//===----------------------------------------------------------------------===//
def IL_vbuild     : SDNode<"AMDILISD::VBUILD", SDTIL_GenVecBuild,
    []>;
def IL_vextract   : SDNode<"AMDILISD::VEXTRACT", SDTIL_GenVecExtract,
    []>;
def IL_vinsert    : SDNode<"AMDILISD::VINSERT", SDTIL_GenVecInsert,
    []>;
def IL_vconcat    : SDNode<"AMDILISD::VCONCAT", SDTIL_GenVecConcat,
    []>;

//===----------------------------------------------------------------------===//
// AMDIL Atomic Custom SDNodes
//===----------------------------------------------------------------------===//
//===-------------- 32 bit global atomics with return values --------------===//
def atom_g_add : SDNode<"AMDILISD::ATOM_G_ADD", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_and : SDNode<"AMDILISD::ATOM_G_AND", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_cmpxchg : SDNode<"AMDILISD::ATOM_G_CMPXCHG", SDTIL_TriAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_dec : SDNode<"AMDILISD::ATOM_G_DEC", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_inc : SDNode<"AMDILISD::ATOM_G_INC", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_max : SDNode<"AMDILISD::ATOM_G_MAX", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_umax : SDNode<"AMDILISD::ATOM_G_UMAX", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_min : SDNode<"AMDILISD::ATOM_G_MIN", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_umin : SDNode<"AMDILISD::ATOM_G_UMIN", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_or : SDNode<"AMDILISD::ATOM_G_OR", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_sub : SDNode<"AMDILISD::ATOM_G_SUB", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_rsub : SDNode<"AMDILISD::ATOM_G_RSUB", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_xchg : SDNode<"AMDILISD::ATOM_G_XCHG", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_xor : SDNode<"AMDILISD::ATOM_G_XOR", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_g_store : SDNode<"AMDILISD::ATOM_G_STORE", SDTIL_AtomStore,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_g_load : SDNode<"AMDILISD::ATOM_G_LOAD",SDTIL_AtomLoad,
    [SDNPHasChain, SDNPMayLoad, SDNPMemOperand]>;

//===------------- 32 bit global atomics without return values ------------===//
def atom_g_add_noret : SDNode<"AMDILISD::ATOM_G_ADD_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_g_and_noret : SDNode<"AMDILISD::ATOM_G_AND_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_g_cmpxchg_noret : SDNode<"AMDILISD::ATOM_G_CMPXCHG_NORET",
    SDTIL_TriAtomNoRet, [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_cmp_noret : SDNode<"AMDILISD::ATOM_G_CMPXCHG_NORET",
    SDTIL_TriAtomNoRet, [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_g_dec_noret : SDNode<"AMDILISD::ATOM_G_DEC_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_g_inc_noret : SDNode<"AMDILISD::ATOM_G_INC_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_g_max_noret : SDNode<"AMDILISD::ATOM_G_MAX_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_g_umax_noret: SDNode<"AMDILISD::ATOM_G_UMAX_NORET",
    SDTIL_BinAtomNoRet, [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_g_min_noret : SDNode<"AMDILISD::ATOM_G_MIN_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_g_umin_noret: SDNode<"AMDILISD::ATOM_G_UMIN_NORET",
    SDTIL_BinAtomNoRet, [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_g_or_noret : SDNode<"AMDILISD::ATOM_G_OR_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_g_sub_noret : SDNode<"AMDILISD::ATOM_G_SUB_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_g_rsub_noret : SDNode<"AMDILISD::ATOM_G_RSUB_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_g_xchg_noret: SDNode<"AMDILISD::ATOM_G_XCHG_NORET",
    SDTIL_BinAtomNoRet, [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_g_xor_noret : SDNode<"AMDILISD::ATOM_G_XOR_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;

//===---------------- 32 bit flat atomics with return values --------------===//
def atom_f_add : SDNode<"AMDILISD::ATOM_F_ADD", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_f_and : SDNode<"AMDILISD::ATOM_F_AND", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_f_cmpxchg : SDNode<"AMDILISD::ATOM_F_CMPXCHG", SDTIL_TriAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_f_dec : SDNode<"AMDILISD::ATOM_F_DEC", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_f_inc : SDNode<"AMDILISD::ATOM_F_INC", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_f_max : SDNode<"AMDILISD::ATOM_F_MAX", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_f_umax : SDNode<"AMDILISD::ATOM_F_UMAX", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_f_min : SDNode<"AMDILISD::ATOM_F_MIN", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_f_umin : SDNode<"AMDILISD::ATOM_F_UMIN", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_f_or : SDNode<"AMDILISD::ATOM_F_OR", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_f_sub : SDNode<"AMDILISD::ATOM_F_SUB", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_f_xchg : SDNode<"AMDILISD::ATOM_F_XCHG", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_f_xor : SDNode<"AMDILISD::ATOM_F_XOR", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;

//===--------------- 32 bit local atomics with return values --------------===//
def atom_l_add : SDNode<"AMDILISD::ATOM_L_ADD", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_and : SDNode<"AMDILISD::ATOM_L_AND", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_cmpxchg : SDNode<"AMDILISD::ATOM_L_CMPXCHG", SDTIL_TriAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_dec : SDNode<"AMDILISD::ATOM_L_DEC", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_inc : SDNode<"AMDILISD::ATOM_L_INC", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_max : SDNode<"AMDILISD::ATOM_L_MAX", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_umax : SDNode<"AMDILISD::ATOM_L_UMAX", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_min : SDNode<"AMDILISD::ATOM_L_MIN", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_umin : SDNode<"AMDILISD::ATOM_L_UMIN", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_or : SDNode<"AMDILISD::ATOM_L_OR", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_mskor : SDNode<"AMDILISD::ATOM_L_MSKOR", SDTIL_TriAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_sub : SDNode<"AMDILISD::ATOM_L_SUB", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_rsub : SDNode<"AMDILISD::ATOM_L_RSUB", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_xchg : SDNode<"AMDILISD::ATOM_L_XCHG", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_xor : SDNode<"AMDILISD::ATOM_L_XOR", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;

//===-------------- 32 bit local atomics without return values ------------===//
def atom_l_add_noret : SDNode<"AMDILISD::ATOM_L_ADD_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_l_and_noret : SDNode<"AMDILISD::ATOM_L_AND_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_l_cmpxchg_noret : SDNode<"AMDILISD::ATOM_L_CMPXCHG_NORET",
    SDTIL_TriAtomNoRet, [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_dec_noret : SDNode<"AMDILISD::ATOM_L_DEC_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_l_inc_noret : SDNode<"AMDILISD::ATOM_L_INC_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_l_max_noret : SDNode<"AMDILISD::ATOM_L_MAX_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_l_umax_noret: SDNode<"AMDILISD::ATOM_L_UMAX_NORET",
    SDTIL_BinAtomNoRet, [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_l_min_noret : SDNode<"AMDILISD::ATOM_L_MIN_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_l_umin_noret: SDNode<"AMDILISD::ATOM_L_UMIN_NORET",
    SDTIL_BinAtomNoRet, [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_l_or_noret : SDNode<"AMDILISD::ATOM_L_OR_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_l_mskor_noret : SDNode<"AMDILISD::ATOM_L_MSKOR_NORET",
    SDTIL_TriAtomNoRet, [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_l_sub_noret : SDNode<"AMDILISD::ATOM_L_SUB_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_l_rsub_noret : SDNode<"AMDILISD::ATOM_L_RSUB_NORET",
    SDTIL_BinAtomNoRet, [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_l_xchg_noret: SDNode<"AMDILISD::ATOM_L_XCHG_NORET",
    SDTIL_BinAtomNoRet, [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_l_xor_noret : SDNode<"AMDILISD::ATOM_L_XOR_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;

//===--------------- 32 bit region atomics with return values --------------===//
def atom_r_add : SDNode<"AMDILISD::ATOM_R_ADD", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_and : SDNode<"AMDILISD::ATOM_R_AND", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_cmpxchg : SDNode<"AMDILISD::ATOM_R_CMPXCHG", SDTIL_TriAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_dec : SDNode<"AMDILISD::ATOM_R_DEC", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_inc : SDNode<"AMDILISD::ATOM_R_INC", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_max : SDNode<"AMDILISD::ATOM_R_MAX", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_umax : SDNode<"AMDILISD::ATOM_R_UMAX", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_min : SDNode<"AMDILISD::ATOM_R_MIN", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_umin : SDNode<"AMDILISD::ATOM_R_UMIN", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_or : SDNode<"AMDILISD::ATOM_R_OR", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_mskor : SDNode<"AMDILISD::ATOM_R_MSKOR", SDTIL_TriAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_sub : SDNode<"AMDILISD::ATOM_R_SUB", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_rsub : SDNode<"AMDILISD::ATOM_R_RSUB", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_xchg : SDNode<"AMDILISD::ATOM_R_XCHG", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_xor : SDNode<"AMDILISD::ATOM_R_XOR", SDTIL_BinAtom,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;

//===-------------- 32 bit region atomics without return values ------------===//
def atom_r_add_noret : SDNode<"AMDILISD::ATOM_R_ADD_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_r_and_noret : SDNode<"AMDILISD::ATOM_R_AND_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_r_cmpxchg_noret : SDNode<"AMDILISD::ATOM_R_CMPXCHG_NORET",
    SDTIL_TriAtomNoRet, [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_dec_noret : SDNode<"AMDILISD::ATOM_R_DEC_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_r_inc_noret : SDNode<"AMDILISD::ATOM_R_INC_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_r_max_noret : SDNode<"AMDILISD::ATOM_R_MAX_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_r_umax_noret: SDNode<"AMDILISD::ATOM_R_UMAX_NORET",
    SDTIL_BinAtomNoRet, [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_r_min_noret : SDNode<"AMDILISD::ATOM_R_MIN_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_r_umin_noret: SDNode<"AMDILISD::ATOM_R_UMIN_NORET",
    SDTIL_BinAtomNoRet, [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_r_or_noret : SDNode<"AMDILISD::ATOM_R_OR_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_r_mskor_noret : SDNode<"AMDILISD::ATOM_R_MSKOR_NORET", SDTIL_TriAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_r_sub_noret : SDNode<"AMDILISD::ATOM_R_SUB_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_r_rsub_noret : SDNode<"AMDILISD::ATOM_R_RSUB_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;
def atom_r_xchg_noret: SDNode<"AMDILISD::ATOM_R_XCHG_NORET",
    SDTIL_BinAtomNoRet, [SDNPHasChain, SDNPMayLoad, SDNPMayStore, SDNPMemOperand]>;
def atom_r_xor_noret : SDNode<"AMDILISD::ATOM_R_XOR_NORET", SDTIL_BinAtomNoRet,
    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;

//===--------------- 32 bit atomic counter instructions -------------------===//
def append_alloc : SDNode<"AMDILISD::APPEND_ALLOC", SDTIL_Append,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore]>;
def append_consume : SDNode<"AMDILISD::APPEND_CONSUME", SDTIL_Append,
    [SDNPHasChain, SDNPMayLoad, SDNPMayStore]>;
