// Copyright (c) 2012, Advanced Micro Devices, Inc.
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
// Predicate that is set to true if 64bit pointer are used.


let Predicates = [Has64BitPtr], addr64 = 1 in {
  let isCodeGenOnly=1 in {
    //===----------------------------------------------------------------------===//
    // Store Memory Operations
    //===----------------------------------------------------------------------===//
    defm GLOBALTRUNCSTORE64 : GTRUNCSTORE<"!global trunc store", MEM64, ADDR64>;
    let global = 1 in {
      defm GLOBALSTORE64              : STORE<"!global store"         , global_store, MEM64, ADDR64>;
    }
    defm LOCALTRUNCSTORE64  : LTRUNCSTORE<"!local trunc store", MEM64, ADDR64>;
    let local = 1 in {
      defm LOCALSTORE64               : STORE<"!local store"          , local_store, MEM64, ADDR64>;
    }
    defm PRIVATETRUNCSTORE64 : PTRUNCSTORE<"!private trunc store", MEM64, ADDR64>;
    let private = 1 in {
      defm PRIVATESTORE64             : STORE<"!private store"        , private_store, MEM64, ADDR64>;
    }
    defm REGIONTRUNCSTORE64 : RTRUNCSTORE<"!region trunc store", MEM64, ADDR64>;
    let region = 1 in {
      defm REGIONSTORE64              : STORE<"!region hw store"      , region_store, MEM64, ADDR64>;
    }
  }


  //===---------------------------------------------------------------------===//
  // Load Memory Operations
  //===---------------------------------------------------------------------===//
  let global = 1 in {
    defm GLOBALLOAD64       : LOAD<"!global load"            , global_load, MEM64, ADDR64>;
    let extLoadType = 2 in {
      defm GLOBALZEXTLOAD64   : LOAD<"!global zext load"       , global_zext_load, MEM64, ADDR64>;
    }
    let extLoadType = 1 in {
      defm GLOBALSEXTLOAD64   : LOAD<"!global sext load"       , global_sext_load, MEM64, ADDR64>;
    }
    let extLoadType = 3 in {
      defm GLOBALAEXTLOAD64   : LOAD<"!global aext load"       , global_aext_load, MEM64, ADDR64>;
    }
  }
  let private = 1 in {
    defm PRIVATELOAD64      : LOAD<"!private load"           , private_load, MEM64, ADDR64>;
    let extLoadType = 2 in {
      defm PRIVATEZEXTLOAD64  : LOAD<"!private zext load"      , private_zext_load, MEM64, ADDR64>;
    }
    let extLoadType = 1 in {
      defm PRIVATESEXTLOAD64  : LOAD<"!private sext load"      , private_sext_load, MEM64, ADDR64>;
    }
    let extLoadType = 3 in {
      defm PRIVATEAEXTLOAD64  : LOAD<"!private aext load"      , private_aext_load, MEM64, ADDR64>;
    }
  }
  let cpool = 1 in {
    defm CPOOLLOAD64        : LOAD<"!constant pool load"     , cp_load, MEM64, ADDR64>;
    let extLoadType = 2 in {
      defm CPOOLZEXTLOAD64    : LOAD<"!constant pool zext load", cp_zext_load, MEM64, ADDR64>;
    }
    let extLoadType = 1 in {
      defm CPOOLSEXTLOAD64    : LOAD<"!constant pool sext load", cp_sext_load, MEM64, ADDR64>;
    }
    let extLoadType = 3 in {
      defm CPOOLAEXTLOAD64    : LOAD<"!constant aext pool load", cp_aext_load, MEM64, ADDR64>;
    }
  }
  let constant = 1 in {
    defm CONSTANTLOAD64     : LOAD<"!constant load"          , constant_load, MEM64, ADDR64>;
    let extLoadType = 2 in {
      defm CONSTANTZEXTLOAD64 : LOAD<"!constant zext load"     , constant_zext_load, MEM64, ADDR64>;
    }
    let extLoadType = 1 in {
      defm CONSTANTSEXTLOAD64 : LOAD<"!constant sext load"     , constant_sext_load, MEM64, ADDR64>;
    }
    let extLoadType = 3 in {
      defm CONSTANTAEXTLOAD64 : LOAD<"!constant aext load"     , constant_aext_load, MEM64, ADDR64>;
    }
  }
  let local = 1 in {
    defm LOCALLOAD64        : LOAD<"!local load"             , local_load, MEM64, ADDR64>;
    let extLoadType = 2 in {
      defm LOCALZEXTLOAD64    : LOAD<"!local zext load"        , local_zext_load, MEM64, ADDR64>;
    }
    let extLoadType = 1 in {
      defm LOCALSEXTLOAD64    : LOAD<"!local sext load"        , local_sext_load, MEM64, ADDR64>;
    }
    let extLoadType = 3 in {
      defm LOCALAEXTLOAD64    : LOAD<"!local aext load"        , local_aext_load, MEM64, ADDR64>;
    }
  }
  let region = 1 in {
    defm REGIONLOAD64       : LOAD<"!region load"            , region_load, MEM64, ADDR64>;
    let extLoadType = 2 in {
      defm REGIONZEXTLOAD64   : LOAD<"!region zext load"       , region_zext_load, MEM64, ADDR64>;
    }
    let extLoadType = 1 in {
      defm REGIONSEXTLOAD64   : LOAD<"!region sext load"       , region_sext_load, MEM64, ADDR64>;
    }
    let extLoadType = 3 in {
      defm REGIONAEXTLOAD64   : LOAD<"!region aext load"       , region_aext_load, MEM64, ADDR64>;
    }
  }
  //===---------------------------------------------------------------------===//
  // IO Expansion Load/Store Instructions
  //===---------------------------------------------------------------------===//
  let mayLoad = 1, cbmem = 1 in {
    def CB64LOAD : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins GPR_XY_64:$addy, i32imm:$id),
        !strconcat(IL_OP_MOV.Text, " $dst, cb$id[$addy]"), []>;
  }
  let addr64 = 1 in {
    defm UAVRAW64 : UAVRawLSInst<GPR_XY_64, "_addr(64)_id($id)">;
    defm SCRATCH64 : ScratchLSInst<GPR_XY_64>;
    defm GDS64 : GDSLSInst<GPR_X_32>;
    defm LDS64 : LDSLSInst<GPR_X_32>;

    let image = 1, txld = 1, load = 1 in {
      def TXLD641D : ILFormat<IL_OP_LOAD, (outs GPRV4I32:$dst),
          (ins MEM64:$ptr, GPRV4I32:$addy),
          !strconcat(IL_OP_LOAD.Text,
              "_id($ptr) $dst, $addy"),
          [(set GPRV4I32:$dst,
              (int_AMDIL_image1d_load ADDR64:$ptr, GPRV4I32:$addy))]>;
      def TXLD641DA : ILFormat<IL_OP_LOAD, (outs GPRV4I32:$dst),
          (ins MEM64:$ptr, GPRV4I32:$addy),
          !strconcat(IL_OP_LOAD.Text,
              "_id($ptr) $dst, $addy"),
          [(set GPRV4I32:$dst,
              (int_AMDIL_image1d_array_load ADDR64:$ptr, GPRV4I32:$addy))]>;
      def TXLD641DB : ILFormat<IL_OP_LOAD, (outs GPRV4I32:$dst),
          (ins MEM64:$ptr, GPR_32:$addy),
          !strconcat(IL_OP_LOAD.Text,
              "_id($ptr) $dst, $addy"),
          [(set GPRV4I32:$dst,
              (int_AMDIL_image1d_buffer_load ADDR64:$ptr, GPR_32:$addy))]>;
      def TXLD642D : ILFormat<IL_OP_LOAD, (outs GPRV4I32:$dst),
          (ins MEM64:$ptr, GPRV4I32:$addy),
          !strconcat(IL_OP_LOAD.Text,
              "_id($ptr) $dst, $addy"),
          [(set GPRV4I32:$dst,
              (int_AMDIL_image2d_load ADDR64:$ptr, GPRV4I32:$addy))]>;
      def TXLD642DA : ILFormat<IL_OP_LOAD, (outs GPRV4I32:$dst),
          (ins MEM64:$ptr, GPRV4I32:$addy),
          !strconcat(IL_OP_LOAD.Text,
              "_id($ptr) $dst, $addy"),
          [(set GPRV4I32:$dst,
              (int_AMDIL_image2d_array_load ADDR64:$ptr, GPRV4I32:$addy))]>;
      def TXLD643D : ILFormat<IL_OP_LOAD, (outs GPRV4I32:$dst),
          (ins MEM64:$ptr, GPRV4I32:$addy),
          !strconcat(IL_OP_LOAD.Text,
              "_id($ptr) $dst, $addy"),
          [(set GPRV4I32:$dst,
              (int_AMDIL_image3d_load ADDR64:$ptr, GPRV4I32:$addy))]>;
    }

    defm READ64 : IMAGEREAD<MEM64, ADDR64>;
    defm INFO064 : IMAGEINFO0<MEM64, ADDR64>;
    defm INFO164 : IMAGEINFO1<MEM64, ADDR64>;
    defm WRITE64 : IMAGEWRITE<MEM64, ADDR64>;

  }

  // Printf related operations.
  let isCall=1, isAsCheapAsAMove = 1 in {
    def GET_PRINTF_OFFSETi64: ILFormat<IL_OP_MOV, (outs GPR_XY_64:$dst),
        (ins), !strconcat(IL_OP_MOV.Text, " $dst, cb0[8].zw"),
        [(set GPR_XY_64:$dst, (int_AMDIL_get_printf_offset))]>;
    def GET_PRINTF_SIZEi64 : ILFormat<IL_OP_MOV, (outs GPR_XY_64:$dst),
        (ins), !strconcat(IL_OP_MOV.Text, " $dst, cb0[9].x0"),
        [(set GPR_XY_64:$dst, (int_AMDIL_get_printf_size))]>;
  }
  let atomic = 1 in {
    let global = 1 in {
      def ATOM64_G_ADD_NORET     : BinAtomNoRet64<IL_OP_UAV_ADD,
          "_addr(64)_id($id)", atom_g_add_noret>;
      def ATOM64_G_AND_NORET     : BinAtomNoRet64<IL_OP_UAV_AND,
          "_addr(64)_id($id)", atom_g_and_noret>;
      def ATOM64_G_MAX_NORET     : BinAtomNoRet64<IL_OP_UAV_MAX,
          "_addr(64)_id($id)", atom_g_max_noret>;
      def ATOM64_G_MIN_NORET     : BinAtomNoRet64<IL_OP_UAV_MIN,
          "_addr(64)_id($id)", atom_g_min_noret>;
      def ATOM64_G_UMAX_NORET    : BinAtomNoRet64<IL_OP_UAV_UMAX,
          "_addr(64)_id($id)", atom_g_umax_noret>;
      def ATOM64_G_UMIN_NORET    : BinAtomNoRet64<IL_OP_UAV_UMIN,
          "_addr(64)_id($id)", atom_g_umin_noret>;
      def ATOM64_G_OR_NORET      : BinAtomNoRet64<IL_OP_UAV_OR,
          "_addr(64)_id($id)", atom_g_or_noret>;
      def ATOM64_G_RSUB_NORET    : BinAtomNoRet64<IL_OP_UAV_RSUB,
          "_addr(64)_id($id)", atom_g_rsub_noret>;
      def ATOM64_G_SUB_NORET     : BinAtomNoRet64<IL_OP_UAV_SUB,
          "_addr(64)_id($id)", atom_g_sub_noret>;
      def ATOM64_G_XOR_NORET     : BinAtomNoRet64<IL_OP_UAV_XOR,
          "_addr(64)_id($id)", atom_g_xor_noret>;
      def ATOM64_G_INC_NORET     : BinAtomNoRet64<IL_OP_UAV_INC,
          "_addr(64)_id($id)", atom_g_inc_noret>;
      def ATOM64_G_DEC_NORET     : BinAtomNoRet64<IL_OP_UAV_DEC,
          "_addr(64)_id($id)", atom_g_dec_noret>;
      def ATOM64_G_CMPXCHG_NORET    : CmpXChgNoRet64<IL_OP_UAV_CMP,
          "_addr(64)_id($id)", atom_g_cmpxchg_noret>;

      // All of the atomic functions that return
      def ATOM64_G_ADD     : BinAtom64<IL_OP_UAV_READ_ADD,
          "_addr(64)_id($id)", atom_g_add>;
      def ATOM64_G_AND     : BinAtom64<IL_OP_UAV_READ_AND,
          "_addr(64)_id($id)", atom_g_and>;
      def ATOM64_G_MAX     : BinAtom64<IL_OP_UAV_READ_MAX,
          "_addr(64)_id($id)", atom_g_max>;
      def ATOM64_G_MIN     : BinAtom64<IL_OP_UAV_READ_MIN,
          "_addr(64)_id($id)", atom_g_min>;
      def ATOM64_G_UMAX    : BinAtom64<IL_OP_UAV_READ_UMAX,
          "_addr(64)_id($id)", atom_g_umax>;
      def ATOM64_G_UMIN    : BinAtom64<IL_OP_UAV_READ_UMIN,
          "_addr(64)_id($id)", atom_g_umin>;
      def ATOM64_G_OR      : BinAtom64<IL_OP_UAV_READ_OR,
          "_addr(64)_id($id)", atom_g_or>;
      def ATOM64_G_RSUB    : BinAtom64<IL_OP_UAV_READ_RSUB,
          "_addr(64)_id($id)", atom_g_rsub>;
      def ATOM64_G_SUB     : BinAtom64<IL_OP_UAV_READ_SUB,
          "_addr(64)_id($id)", atom_g_sub>;
      def ATOM64_G_XOR     : BinAtom64<IL_OP_UAV_READ_XOR,
          "_addr(64)_id($id)", atom_g_xor>;
      def ATOM64_G_INC     : BinAtom64<IL_OP_UAV_READ_INC,
          "_addr(64)_id($id)", atom_g_inc>;
      def ATOM64_G_DEC     : BinAtom64<IL_OP_UAV_READ_DEC,
          "_addr(64)_id($id)", atom_g_dec>;
      def ATOM64_G_XCHG    : BinAtom64<IL_OP_UAV_READ_XCHG,
          "_addr(64)_id($id)", atom_g_xchg>;
      def ATOM64_G_CMPXCHG : CmpXChg64<IL_OP_UAV_READ_CMPXCHG,
          "_addr(64)_id($id)", atom_g_cmpxchg>;
      def ATOM64_G_ADD_NORET_B64  : BinAtomNoRet64I64<IL_OP_UAV_ADD,
          "_b64_addr(64)_id($id)", atom_g_add_noret>;
      def ATOM64_G_AND_NORET_B64  : BinAtomNoRet64I64<IL_OP_UAV_AND,
          "_b64_addr(64)_id($id)", atom_g_and_noret>;
      def ATOM64_G_MAX_NORET_B64  : BinAtomNoRet64I64<IL_OP_UAV_MAX,
          "_b64_addr(64)_id($id)", atom_g_max_noret>;
      def ATOM64_G_MIN_NORET_B64  : BinAtomNoRet64I64<IL_OP_UAV_MIN,
          "_b64_addr(64)_id($id)", atom_g_min_noret>;
      def ATOM64_G_UMAX_NORET_B64 : BinAtomNoRet64I64<IL_OP_UAV_UMAX,
          "_b64_addr(64)_id($id)", atom_g_umax_noret>;
      def ATOM64_G_UMIN_NORET_B64 : BinAtomNoRet64I64<IL_OP_UAV_UMIN,
          "_b64_addr(64)_id($id)", atom_g_umin_noret>;
      def ATOM64_G_OR_NORET_B64   : BinAtomNoRet64I64<IL_OP_UAV_OR,
          "_b64_addr(64)_id($id)", atom_g_or_noret>;
      def ATOM64_G_RSUB_NORET_B64 : BinAtomNoRet64I64<IL_OP_UAV_RSUB,
          "_b64_addr(64)_id($id)", atom_g_rsub_noret>;
      def ATOM64_G_SUB_NORET_B64  : BinAtomNoRet64I64<IL_OP_UAV_SUB,
          "_b64_addr(64)_id($id)", atom_g_sub_noret>;
      def ATOM64_G_XOR_NORET_B64  : BinAtomNoRet64I64<IL_OP_UAV_XOR,
          "_b64_addr(64)_id($id)", atom_g_xor_noret>;
      def ATOM64_G_INC_NORET_B64  : BinAtomNoRet64I64<IL_OP_UAV_INC,
          "_b64_addr(64)_id($id)", atom_g_inc_noret>;
      def ATOM64_G_DEC_NORET_B64  : BinAtomNoRet64I64<IL_OP_UAV_DEC,
          "_b64_addr(64)_id($id)", atom_g_dec_noret>;
      def ATOM64_G_CMPXCHG_NORET_B64 : CmpXChgNoRet64I64<IL_OP_UAV_CMP,
          "_b64_addr(64)_id($id)", atom_g_cmpxchg_noret>;
      def ATOM64_G_ADD_B64     : BinAtom64I64<IL_OP_UAV_READ_ADD,
          "_b64_addr(64)_id($id)", atom_g_add>;
      def ATOM64_G_AND_B64     : BinAtom64I64<IL_OP_UAV_READ_AND,
          "_b64_addr(64)_id($id)", atom_g_and>;
      def ATOM64_G_MAX_B64     : BinAtom64I64<IL_OP_UAV_READ_MAX,
          "_b64_addr(64)_id($id)", atom_g_max>;
      def ATOM64_G_MIN_B64     : BinAtom64I64<IL_OP_UAV_READ_MIN,
          "_b64_addr(64)_id($id)", atom_g_min>;
      def ATOM64_G_UMAX_B64    : BinAtom64I64<IL_OP_UAV_READ_UMAX,
          "_b64_addr(64)_id($id)", atom_g_umax>;
      def ATOM64_G_UMIN_B64    : BinAtom64I64<IL_OP_UAV_READ_UMIN,
          "_b64_addr(64)_id($id)", atom_g_umin>;
      def ATOM64_G_OR_B64      : BinAtom64I64<IL_OP_UAV_READ_OR,
          "_b64_addr(64)_id($id)", atom_g_or>;
      def ATOM64_G_RSUB_B64    : BinAtom64I64<IL_OP_UAV_READ_RSUB,
          "_b64_addr(64)_id($id)", atom_g_rsub>;
      def ATOM64_G_SUB_B64     : BinAtom64I64<IL_OP_UAV_READ_SUB,
          "_b64_addr(64)_id($id)", atom_g_sub>;
      def ATOM64_G_XOR_B64     : BinAtom64I64<IL_OP_UAV_READ_XOR,
          "_b64_addr(64)_id($id)", atom_g_xor>;
      def ATOM64_G_INC_B64     : BinAtom64I64<IL_OP_UAV_READ_INC,
          "_b64_addr(64)_id($id)", atom_g_inc>;
      def ATOM64_G_DEC_B64     : BinAtom64I64<IL_OP_UAV_READ_DEC,
          "_b64_addr(64)_id($id)", atom_g_dec>;
      def ATOM64_G_XCHG_B64    : BinAtom64I64<IL_OP_UAV_READ_XCHG,
          "_b64_addr(64)_id($id)", atom_g_xchg>;
      def ATOM64_G_CMPXCHG_B64 : CmpXChg64I64<IL_OP_UAV_READ_CMPXCHG,
          "_b64_addr(64)_id($id)", atom_g_cmpxchg>;
    }
    let local = 1 in {
      def ATOM64_L_ADD_NORET     : BinAtomNoRet64<IL_OP_LDS_ADD,
          "_id($id)", atom_l_add_noret>;
      def ATOM64_L_AND_NORET     : BinAtomNoRet64<IL_OP_LDS_AND,
          "_id($id)", atom_l_and_noret>;
      def ATOM64_L_MAX_NORET     : BinAtomNoRet64<IL_OP_LDS_MAX,
          "_id($id)", atom_l_max_noret>;
      def ATOM64_L_MIN_NORET     : BinAtomNoRet64<IL_OP_LDS_MIN,
          "_id($id)", atom_l_min_noret>;
      def ATOM64_L_UMAX_NORET    : BinAtomNoRet64<IL_OP_LDS_UMAX,
          "_id($id)", atom_l_umax_noret>;
      def ATOM64_L_UMIN_NORET    : BinAtomNoRet64<IL_OP_LDS_UMIN,
          "_id($id)", atom_l_umin_noret>;
      def ATOM64_L_MSKOR_NORET   : TriAtomNoRet64<IL_OP_LDS_MSKOR,
          "_id($id)", atom_l_mskor_noret>;
      def ATOM64_L_OR_NORET      : BinAtomNoRet64<IL_OP_LDS_OR,
          "_id($id)", atom_l_or_noret>;
      def ATOM64_L_RSUB_NORET    : BinAtomNoRet64<IL_OP_LDS_RSUB,
          "_id($id)", atom_l_rsub_noret>;
      def ATOM64_L_SUB_NORET     : BinAtomNoRet64<IL_OP_LDS_SUB,
          "_id($id)", atom_l_sub_noret>;
      def ATOM64_L_XOR_NORET     : BinAtomNoRet64<IL_OP_LDS_XOR,
          "_id($id)", atom_l_xor_noret>;
      def ATOM64_L_INC_NORET     : BinAtomNoRet64<IL_OP_LDS_INC,
          "_id($id)", atom_l_inc_noret>;
      def ATOM64_L_DEC_NORET     : BinAtomNoRet64<IL_OP_LDS_DEC,
          "_id($id)", atom_l_dec_noret>;
      def ATOM64_L_CMPXCHG_NORET    : TriAtomNoRet64<IL_OP_LDS_CMP,
          "_id($id)", atom_l_cmpxchg_noret>;
      def ATOM64_L_ADD     : BinAtom64<IL_OP_LDS_READ_ADD,
          "_id($id)", atom_l_add>;
      def ATOM64_L_AND     : BinAtom64<IL_OP_LDS_READ_AND,
          "_id($id)", atom_l_and>;
      def ATOM64_L_MAX     : BinAtom64<IL_OP_LDS_READ_MAX,
          "_id($id)", atom_l_max>;
      def ATOM64_L_MIN     : BinAtom64<IL_OP_LDS_READ_MIN,
          "_id($id)", atom_l_min>;
      def ATOM64_L_UMAX    : BinAtom64<IL_OP_LDS_READ_UMAX,
          "_id($id)", atom_l_umax>;
      def ATOM64_L_UMIN    : BinAtom64<IL_OP_LDS_READ_UMIN,
          "_id($id)", atom_l_umin>;
      def ATOM64_L_OR      : BinAtom64<IL_OP_LDS_READ_OR,
          "_id($id)", atom_l_or>;
      def ATOM64_L_MSKOR   : TriAtom64<IL_OP_LDS_READ_MSKOR,
          "_id($id)", atom_l_mskor>;
      def ATOM64_L_RSUB    : BinAtom64<IL_OP_LDS_READ_RSUB,
          "_id($id)", atom_l_rsub>;
      def ATOM64_L_SUB     : BinAtom64<IL_OP_LDS_READ_SUB,
          "_id($id)", atom_l_sub>;
      def ATOM64_L_XOR     : BinAtom64<IL_OP_LDS_READ_XOR,
          "_id($id)", atom_l_xor>;
      def ATOM64_L_INC     : BinAtom64<IL_OP_LDS_READ_INC,
          "_id($id)", atom_l_inc>;
      def ATOM64_L_DEC     : BinAtom64<IL_OP_LDS_READ_DEC,
          "_id($id)", atom_l_dec>;
      def ATOM64_L_XCHG    : BinAtom64<IL_OP_LDS_READ_XCHG,
          "_id($id)", atom_l_xchg>;
      def ATOM64_L_CMPXCHG : TriAtom64<IL_OP_LDS_READ_CMPXCHG,
          "_id($id)", atom_l_cmpxchg>;
      def ATOM64_L_ADD_NORET_B64  : BinAtomNoRet64I64<IL_OP_LDS_ADD,
          "64_id($id)", atom_l_add_noret>;
      def ATOM64_L_AND_NORET_B64  : BinAtomNoRet64I64<IL_OP_LDS_AND,
          "64_id($id)", atom_l_and_noret>;
      def ATOM64_L_MAX_NORET_B64  : BinAtomNoRet64I64<IL_OP_LDS_MAX,
          "64_id($id)", atom_l_max_noret>;
      def ATOM64_L_MIN_NORET_B64  : BinAtomNoRet64I64<IL_OP_LDS_MIN,
          "64_id($id)", atom_l_min_noret>;
      def ATOM64_L_UMAX_NORET_B64 : BinAtomNoRet64I64<IL_OP_LDS_UMAX,
          "64_id($id)", atom_l_umax_noret>;
      def ATOM64_L_UMIN_NORET_B64 : BinAtomNoRet64I64<IL_OP_LDS_UMIN,
          "64_id($id)", atom_l_umin_noret>;
      def ATOM64_L_MSKOR_NORET_B64: TriAtomNoRet64I64<IL_OP_LDS_MSKOR,
          "64_id($id)", atom_l_mskor_noret>;
      def ATOM64_L_OR_NORET_B64   : BinAtomNoRet64I64<IL_OP_LDS_OR,
          "64_id($id)", atom_l_or_noret>;
      def ATOM64_L_RSUB_NORET_B64 : BinAtomNoRet64I64<IL_OP_LDS_RSUB,
          "64_id($id)", atom_l_rsub_noret>;
      def ATOM64_L_SUB_NORET_B64  : BinAtomNoRet64I64<IL_OP_LDS_SUB,
          "64_id($id)", atom_l_sub_noret>;
      def ATOM64_L_XOR_NORET_B64  : BinAtomNoRet64I64<IL_OP_LDS_XOR,
          "64_id($id)", atom_l_xor_noret>;
      def ATOM64_L_INC_NORET_B64  : BinAtomNoRet64I64<IL_OP_LDS_INC,
          "64_id($id)", atom_l_inc_noret>;
      def ATOM64_L_DEC_NORET_B64  : BinAtomNoRet64I64<IL_OP_LDS_DEC,
          "64_id($id)", atom_l_dec_noret>;
      def ATOM64_L_CMPXCHG_NORET_B64 : TriAtomNoRet64I64<IL_OP_LDS_CMP,
          "64_id($id)", atom_l_cmpxchg_noret>;
      def ATOM64_L_ADD_B64     : BinAtom64I64<IL_OP_LDS_READ_ADD,
          "64_id($id)", atom_l_add>;
      def ATOM64_L_AND_B64     : BinAtom64I64<IL_OP_LDS_READ_AND,
          "64_id($id)", atom_l_and>;
      def ATOM64_L_MAX_B64     : BinAtom64I64<IL_OP_LDS_READ_MAX,
          "64_id($id)", atom_l_max>;
      def ATOM64_L_MIN_B64    : BinAtom64I64<IL_OP_LDS_READ_MIN,
          "64_id($id)", atom_l_min>;
      def ATOM64_L_UMAX_B64    : BinAtom64I64<IL_OP_LDS_READ_UMAX,
          "64_id($id)", atom_l_umax>;
      def ATOM64_L_UMIN_B64    : BinAtom64I64<IL_OP_LDS_READ_UMIN,
          "64_id($id)", atom_l_umin>;
      def ATOM64_L_OR_B64      : BinAtom64I64<IL_OP_LDS_READ_OR,
          "64_id($id)", atom_l_or>;
      def ATOM64_L_MSKOR_B64   : TriAtom64I64<IL_OP_LDS_READ_MSKOR,
          "64_id($id)", atom_l_mskor>;
      def ATOM64_L_RSUB_B64    : BinAtom64I64<IL_OP_LDS_READ_RSUB,
          "64_id($id)", atom_l_rsub>;
      def ATOM64_L_SUB_B64     : BinAtom64I64<IL_OP_LDS_READ_SUB,
          "64_id($id)", atom_l_sub>;
      def ATOM64_L_XOR_B64     : BinAtom64I64<IL_OP_LDS_READ_XOR,
          "64_id($id)", atom_l_xor>;
      def ATOM64_L_INC_B64     : BinAtom64I64<IL_OP_LDS_READ_INC,
          "64_id($id)", atom_l_inc>;
      def ATOM64_L_DEC_B64     : BinAtom64I64<IL_OP_LDS_READ_DEC,
          "64_id($id)", atom_l_dec>;
      def ATOM64_L_XCHG_B64    : BinAtom64I64<IL_OP_LDS_READ_XCHG,
          "64_id($id)", atom_l_xchg>;
      def ATOM64_L_CMPXCHG_B64 : TriAtom64I64<IL_OP_LDS_READ_CMPXCHG,
          "64_id($id)", atom_l_cmpxchg>;
    }
    let region = 1 in {
      def ATOM64_R_ADD_NORET     : BinAtomNoRet64<IL_OP_GDS_ADD,
          "_id($id)", atom_r_add_noret>;
      def ATOM64_R_AND_NORET     : BinAtomNoRet64<IL_OP_GDS_AND,
          "_id($id)", atom_r_and_noret>;
      def ATOM64_R_MAX_NORET     : BinAtomNoRet64<IL_OP_GDS_MAX,
          "_id($id)", atom_r_max_noret>;
      def ATOM64_R_MIN_NORET     : BinAtomNoRet64<IL_OP_GDS_MIN,
          "_id($id)", atom_r_min_noret>;
      def ATOM64_R_UMAX_NORET    : BinAtomNoRet64<IL_OP_GDS_UMAX,
          "_id($id)", atom_r_umax_noret>;
      def ATOM64_R_UMIN_NORET    : BinAtomNoRet64<IL_OP_GDS_UMIN,
          "_id($id)", atom_r_umin_noret>;
      def ATOM64_R_MSKOR_NORET   : TriAtomNoRet64<IL_OP_GDS_MSKOR,
          "_id($id)", atom_r_mskor_noret>;
      def ATOM64_R_OR_NORET      : BinAtomNoRet64<IL_OP_GDS_OR,
          "_id($id)", atom_r_or_noret>;
      def ATOM64_R_RSUB_NORET    : BinAtomNoRet64<IL_OP_GDS_RSUB,
          "_id($id)", atom_r_rsub_noret>;
      def ATOM64_R_SUB_NORET     : BinAtomNoRet64<IL_OP_GDS_SUB,
          "_id($id)", atom_r_sub_noret>;
      def ATOM64_R_XOR_NORET     : BinAtomNoRet64<IL_OP_GDS_XOR,
          "_id($id)", atom_r_xor_noret>;
      def ATOM64_R_INC_NORET     : BinAtomNoRet64<IL_OP_GDS_INC,
          "_id($id)", atom_r_inc_noret>;
      def ATOM64_R_DEC_NORET     : BinAtomNoRet64<IL_OP_GDS_DEC,
          "_id($id)", atom_r_dec_noret>;
      def ATOM64_R_CMPXCHG_NORET : TriAtomNoRet64<IL_OP_GDS_CMP,
          "_id($id)", atom_r_cmpxchg_noret>;
      def ATOM64_R_ADD     : BinAtom64<IL_OP_GDS_READ_ADD,
          "_id($id)", atom_r_add>;
      def ATOM64_R_AND     : BinAtom64<IL_OP_GDS_READ_AND,
          "_id($id)", atom_r_and>;
      def ATOM64_R_MAX     : BinAtom64<IL_OP_GDS_READ_MAX,
          "_id($id)", atom_r_max>;
      def ATOM64_R_MIN     : BinAtom64<IL_OP_GDS_READ_MIN,
          "_id($id)", atom_r_min>;
      def ATOM64_R_UMAX    : BinAtom64<IL_OP_GDS_READ_UMAX,
          "_id($id)", atom_r_umax>;
      def ATOM64_R_UMIN    : BinAtom64<IL_OP_GDS_READ_UMIN,
          "_id($id)", atom_r_umin>;
      def ATOM64_R_OR      : BinAtom64<IL_OP_GDS_READ_OR,
          "_id($id)", atom_r_or>;
      def ATOM64_R_MSKOR   : TriAtom64<IL_OP_GDS_READ_MSKOR,
          "_id($id)", atom_r_mskor>;
      def ATOM64_R_RSUB    : BinAtom64<IL_OP_GDS_READ_RSUB,
          "_id($id)", atom_r_rsub>;
      def ATOM64_R_SUB     : BinAtom64<IL_OP_GDS_READ_SUB,
          "_id($id)", atom_r_sub>;
      def ATOM64_R_XOR     : BinAtom64<IL_OP_GDS_READ_XOR,
          "_id($id)", atom_r_xor>;
      def ATOM64_R_INC     : BinAtom64<IL_OP_GDS_READ_INC,
          "_id($id)", atom_r_inc>;
      def ATOM64_R_DEC     : BinAtom64<IL_OP_GDS_READ_DEC,
          "_id($id)", atom_r_dec>;
      def ATOM64_R_XCHG    : BinAtom64<IL_OP_GDS_READ_XCHG,
          "_id($id)", atom_r_xchg>;
      def ATOM64_R_CMPXCHG : TriAtom64<IL_OP_GDS_READ_CMPXCHG,
          "_id($id)", atom_r_cmpxchg>;
      def ATOM64_R_ADD_NORET_B64  : BinAtomNoRet64I64<IL_OP_GDS_ADD,
          "64_id($id)", atom_r_add_noret>;
      def ATOM64_R_AND_NORET_B64  : BinAtomNoRet64I64<IL_OP_GDS_AND,
          "64_id($id)", atom_r_and_noret>;
      def ATOM64_R_MAX_NORET_B64  : BinAtomNoRet64I64<IL_OP_GDS_MAX,
          "64_id($id)", atom_r_max_noret>;
      def ATOM64_R_MIN_NORET_B64  : BinAtomNoRet64I64<IL_OP_GDS_MIN,
          "64_id($id)", atom_r_min_noret>;
      def ATOM64_R_UMAX_NORET_B64 : BinAtomNoRet64I64<IL_OP_GDS_UMAX,
          "64_id($id)", atom_r_umax_noret>;
      def ATOM64_R_UMIN_NORET_B64 : BinAtomNoRet64I64<IL_OP_GDS_UMIN,
          "64_id($id)", atom_r_umin_noret>;
      def ATOM64_R_MSKOR_NORET_B64: TriAtomNoRet64I64<IL_OP_GDS_MSKOR,
          "64_id($id)", atom_r_mskor_noret>;
      def ATOM64_R_OR_NORET_B64   : BinAtomNoRet64I64<IL_OP_GDS_OR,
          "64_id($id)", atom_r_or_noret>;
      def ATOM64_R_RSUB_NORET_B64 : BinAtomNoRet64I64<IL_OP_GDS_RSUB,
          "64_id($id)", atom_r_rsub_noret>;
      def ATOM64_R_SUB_NORET_B64  : BinAtomNoRet64I64<IL_OP_GDS_SUB,
          "64_id($id)", atom_r_sub_noret>;
      def ATOM64_R_XOR_NORET_B64  : BinAtomNoRet64I64<IL_OP_GDS_XOR,
          "64_id($id)", atom_r_xor_noret>;
      def ATOM64_R_INC_NORET_B64  : BinAtomNoRet64I64<IL_OP_GDS_INC,
          "64_id($id)", atom_r_inc_noret>;
      def ATOM64_R_DEC_NORET_B64  : BinAtomNoRet64I64<IL_OP_GDS_DEC,
          "64_id($id)", atom_r_dec_noret>;
      def ATOM64_R_CMPXCHG_NORET_B64 : TriAtomNoRet64I64<IL_OP_GDS_CMP,
          "64_id($id)", atom_r_cmpxchg_noret>;
      def ATOM64_R_ADD_B64     : BinAtom64I64<IL_OP_GDS_READ_ADD,
          "64_id($id)", atom_r_add>;
      def ATOM64_R_AND_B64     : BinAtom64I64<IL_OP_GDS_READ_AND,
          "64_id($id)", atom_r_and>;
      def ATOM64_R_MAX_B64     : BinAtom64I64<IL_OP_GDS_READ_MAX,
          "64_id($id)", atom_r_max>;
      def ATOM64_R_MIN_B64     : BinAtom64I64<IL_OP_GDS_READ_MIN,
          "64_id($id)", atom_r_min>;
      def ATOM64_R_UMAX_B64    : BinAtom64I64<IL_OP_GDS_READ_UMAX,
          "64_id($id)", atom_r_umax>;
      def ATOM64_R_UMIN_B64    : BinAtom64I64<IL_OP_GDS_READ_UMIN,
          "64_id($id)", atom_r_umin>;
      def ATOM64_R_OR_B64      : BinAtom64I64<IL_OP_GDS_READ_OR,
          "64_id($id)", atom_r_or>;
      def ATOM64_R_MSKOR_B64   : TriAtom64I64<IL_OP_GDS_READ_MSKOR,
          "64_id($id)", atom_r_mskor>;
      def ATOM64_R_RSUB_B64    : BinAtom64I64<IL_OP_GDS_READ_RSUB,
          "64_id($id)", atom_r_rsub>;
      def ATOM64_R_SUB_B64     : BinAtom64I64<IL_OP_GDS_READ_SUB,
          "64_id($id)", atom_r_sub>;
      def ATOM64_R_XOR_B64     : BinAtom64I64<IL_OP_GDS_READ_XOR,
          "64_id($id)", atom_r_xor>;
      def ATOM64_R_INC_B64     : BinAtom64I64<IL_OP_GDS_READ_INC,
          "64_id($id)", atom_r_inc>;
      def ATOM64_R_DEC_B64     : BinAtom64I64<IL_OP_GDS_READ_DEC,
          "64_id($id)", atom_r_dec>;
      def ATOM64_R_XCHG_B64    : BinAtom64I64<IL_OP_GDS_READ_XCHG,
          "64_id($id)", atom_r_xchg>;
      def ATOM64_R_CMPXCHG_B64 : TriAtom64I64<IL_OP_GDS_READ_CMPXCHG,
          "64_id($id)", atom_r_cmpxchg>;
    }
  }

  let append = 1, swizzle = 1 in {
    def APPEND64_ALLOC : Append64<IL_OP_APPEND_BUF_ALLOC,
        "_id($id)", append_alloc>;
    def APPEND64_CONSUME : Append64<IL_OP_APPEND_BUF_CONSUME,
        "_id($id)", append_consume>;
  }
  let sema = 1 in {
    let swizzle = 7 in {
      def SEMAPHORE64_WAIT : ILFormat<IL_OP_SEMAPHORE_WAIT, (outs),
          (ins MEM32:$ptr),
          !strconcat(IL_OP_SEMAPHORE_WAIT.Text, "_id($ptr)"),
          [(int_AMDIL_semaphore_wait ADDR:$ptr)]>;

      def SEMAPHORE64_SIGNAL : ILFormat<IL_OP_SEMAPHORE_SIGNAL, (outs),
          (ins MEM32:$ptr),
          !strconcat(IL_OP_SEMAPHORE_SIGNAL.Text, "_id($ptr)"),
          [(int_AMDIL_semaphore_signal ADDR:$ptr)]>;
    }
  }

  let Predicates = [IsSIOrLaterDevice] in {
    let addr64 = 1 in {
      // atomic global load
      defm ATOM64_G_LOAD : AtomGLoadMC<MEM64, ADDR64>;
      // atomic global store
      defm ATOM64_G_STORE : AtomGStoreMC<MEM64, ADDR64>;
    }
  }

}
