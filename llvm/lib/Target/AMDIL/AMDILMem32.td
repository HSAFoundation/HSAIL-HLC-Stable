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

// Predicate that is set to true if the device supports the arena UAV.
def ArenaSupport : Predicate<"Subtarget->isSupported(AMDIL::Caps::ArenaUAV)">;

let Predicates = [Has32BitPtr] in {

  let isCodeGenOnly=1 in {
    //===----------------------------------------------------------------------===//
    // Store Memory Operations
    //===----------------------------------------------------------------------===//
    defm GLOBALTRUNCSTORE   : GTRUNCSTORE<"!global trunc store", MEM32, ADDR>;
    let global = 1 in {
      defm GLOBALSTORE        : STORE<"!global store"         , global_store, MEM32, ADDR>;
    }
    defm LOCALTRUNCSTORE    : LTRUNCSTORE<"!local trunc store", MEM32, ADDR>;
    let local = 1 in {
      defm LOCALSTORE         : STORE<"!local store"          , local_store, MEM32, ADDR>;
    }
    defm PRIVATETRUNCSTORE  : PTRUNCSTORE<"!private trunc store", MEM32, ADDR>;
    let private = 1 in {
      defm PRIVATESTORE       : STORE<"!private store"        , private_store, MEM32, ADDR>;
    }
    defm REGIONTRUNCSTORE   : RTRUNCSTORE<"!region trunc store", MEM32, ADDR>;
    let region = 1 in {
      defm REGIONSTORE        : STORE<"!region hw store"      , region_store, MEM32, ADDR>;
    }


    //===---------------------------------------------------------------------===//
    // Load Memory Operations
    //===---------------------------------------------------------------------===//
    let global = 1 in {
      defm GLOBALLOAD         : LOAD<"!global load"            , global_load, MEM32, ADDR>;
      let extLoadType = 2 in {
        defm GLOBALZEXTLOAD     : LOAD<"!global zext load"       , global_zext_load, MEM32, ADDR>;
      }
      let extLoadType = 1 in {
        defm GLOBALSEXTLOAD     : LOAD<"!global sext load"       , global_sext_load, MEM32, ADDR>;
      }
      let extLoadType = 3 in {
        defm GLOBALAEXTLOAD     : LOAD<"!global aext load"       , global_aext_load, MEM32, ADDR>;
      }
    }
    let private = 1 in {
      defm PRIVATELOAD        : LOAD<"!private load"           , private_load, MEM32, ADDR>;
      let extLoadType = 2 in {
        defm PRIVATEZEXTLOAD    : LOAD<"!private zext load"      , private_zext_load, MEM32, ADDR>;
      }
      let extLoadType = 1 in {
        defm PRIVATESEXTLOAD    : LOAD<"!private sext load"      , private_sext_load, MEM32, ADDR>;
      }
      let extLoadType = 3 in {
        defm PRIVATEAEXTLOAD    : LOAD<"!private aext load"      , private_aext_load, MEM32, ADDR>;
      }
    }
    let cpool = 1  in {
      defm CPOOLLOAD          : LOAD<"!constant pool load"     , cp_load, MEM32, ADDR>;
      let extLoadType = 2 in {
        defm CPOOLZEXTLOAD      : LOAD<"!constant pool zext load", cp_zext_load, MEM32, ADDR>;
      }
      let extLoadType = 1 in {
        defm CPOOLSEXTLOAD      : LOAD<"!constant pool sext load", cp_sext_load, MEM32, ADDR>;
      }
      let extLoadType = 3 in {
        defm CPOOLAEXTLOAD      : LOAD<"!constant aext pool load", cp_aext_load, MEM32, ADDR>;
      }
    }
    let constant = 1 in {
      defm CONSTANTLOAD       : LOAD<"!constant load"          , constant_load, MEM32, ADDR>;
      let extLoadType = 2 in {
        defm CONSTANTZEXTLOAD   : LOAD<"!constant zext load"     , constant_zext_load, MEM32, ADDR>;
      }
      let extLoadType = 1 in {
        defm CONSTANTSEXTLOAD   : LOAD<"!constant sext load"     , constant_sext_load, MEM32, ADDR>;
      }
      let extLoadType = 3 in {
        defm CONSTANTAEXTLOAD   : LOAD<"!constant aext load"     , constant_aext_load, MEM32, ADDR>;
      }
    }
    let local = 1 in {
      defm LOCALLOAD          : LOAD<"!local load"             , local_load, MEM32, ADDR>;
      let extLoadType = 2 in {
        defm LOCALZEXTLOAD      : LOAD<"!local zext load"        , local_zext_load, MEM32, ADDR>;
      }
      let extLoadType = 1 in {
        defm LOCALSEXTLOAD      : LOAD<"!local sext load"        , local_sext_load, MEM32, ADDR>;
      }
      let extLoadType = 3 in {
        defm LOCALAEXTLOAD      : LOAD<"!local aext load"        , local_aext_load, MEM32, ADDR>;
      }
      let region = 1 in {
        defm REGIONLOAD         : LOAD<"!region load"            , region_load, MEM32, ADDR>;
        let extLoadType = 2 in {
          defm REGIONZEXTLOAD     : LOAD<"!region zext load"       , region_zext_load, MEM32, ADDR>;
        }
        let extLoadType = 1 in {
          defm REGIONSEXTLOAD     : LOAD<"!region sext load"       , region_sext_load, MEM32, ADDR>;
        }
        let extLoadType = 3 in {
          defm REGIONAEXTLOAD     : LOAD<"!region aext load"       , region_aext_load, MEM32, ADDR>;
        }
      }
    }
  }


  //===---------------------------------------------------------------------===//
  // IO Expansion Load/Store Instructions
  //===---------------------------------------------------------------------===//
  let mayLoad = 1, load = 1, cbmem = 1 in {
    // Constant Buffer Operations
    def CB32LOAD : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins GPR_X_32:$addy, i32imm:$id),
        !strconcat(IL_OP_MOV.Text, " $dst, cb$id[$addy]"), []>;
  }

  defm UAVRAW32 : UAVRawLSInst<GPR_X_32, "_id($id)">;
  defm SCRATCH32 : ScratchLSInst<GPR_X_32>;
  defm UAVARENA32 : ArenaLSInst<GPR_X_32>;
  defm GDS32 : GDSLSInst<GPR_X_32>;
  defm LDS32 : LDSLSInst<GPR_X_32>;

  let image = 1, txld = 1, load = 1 in {
    def TXLD321D : ILFormat<IL_OP_LOAD, (outs GPRV4I32:$dst),
        (ins MEM32:$ptr, GPRV4I32:$addy),
        !strconcat(IL_OP_LOAD.Text,
            "_id($ptr) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image1d_load ADDR:$ptr, GPRV4I32:$addy))]>;
    def TXLD321DA : ILFormat<IL_OP_LOAD, (outs GPRV4I32:$dst),
        (ins MEM32:$ptr, GPRV4I32:$addy),
        !strconcat(IL_OP_LOAD.Text,
            "_id($ptr) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image1d_array_load ADDR:$ptr, GPRV4I32:$addy))]>;
    def TXLD321DB : ILFormat<IL_OP_LOAD, (outs GPRV4I32:$dst),
        (ins MEM32:$ptr, GPR_32:$addy),
        !strconcat(IL_OP_LOAD.Text,
            "_id($ptr) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image1d_buffer_load ADDR:$ptr, GPR_32:$addy))]>;
    def TXLD322D : ILFormat<IL_OP_LOAD, (outs GPRV4I32:$dst),
        (ins MEM32:$ptr, GPRV4I32:$addy),
        !strconcat(IL_OP_LOAD.Text,
            "_id($ptr) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image2d_load ADDR:$ptr, GPRV4I32:$addy))]>;
    def TXLD322DA : ILFormat<IL_OP_LOAD, (outs GPRV4I32:$dst),
        (ins MEM32:$ptr, GPRV4I32:$addy),
        !strconcat(IL_OP_LOAD.Text,
            "_id($ptr) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image2d_array_load ADDR:$ptr, GPRV4I32:$addy))]>;
    def TXLD323D : ILFormat<IL_OP_LOAD, (outs GPRV4I32:$dst),
        (ins MEM32:$ptr, GPRV4I32:$addy),
        !strconcat(IL_OP_LOAD.Text,
            "_id($ptr) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image3d_load ADDR:$ptr, GPRV4I32:$addy))]>;
  }
  defm READ32 : IMAGEREAD<MEM32, ADDR>;
  defm INFO032 : IMAGEINFO0<MEM32, ADDR>;
  defm INFO132 : IMAGEINFO1<MEM32, ADDR>;
  defm WRITE32 : IMAGEWRITE<MEM32, ADDR>;

  // Printf related operations.
  let isCall=1, isAsCheapAsAMove = 1 in {
    def GET_PRINTF_OFFSETi32 : ILFormat<IL_OP_MOV, (outs GPR_X_32:$dst),
        (ins), !strconcat(IL_OP_MOV.Text, " $dst, cb0[8].y0"),
        [(set GPR_X_32:$dst, (int_AMDIL_get_printf_offset))]>;
    def GET_PRINTF_SIZEi32 : ILFormat<IL_OP_MOV, (outs GPR_X_32:$dst),
        (ins), !strconcat(IL_OP_MOV.Text, " $dst, cb0[8].z0"),
        [(set GPR_X_32:$dst, (int_AMDIL_get_printf_size))]>;
  }
  let atomic = 1 in {
    let global = 1 in {
      // Atomic operations
      def ATOM_G_ADD_NORET     : BinAtomNoRet<IL_OP_UAV_ADD,
          "_id($id)", atom_g_add_noret>;
      def ATOM_G_AND_NORET     : BinAtomNoRet<IL_OP_UAV_AND,
          "_id($id)", atom_g_and_noret>;
      def ATOM_G_MAX_NORET     : BinAtomNoRet<IL_OP_UAV_MAX,
          "_id($id)", atom_g_max_noret>;
      def ATOM_G_MIN_NORET     : BinAtomNoRet<IL_OP_UAV_MIN,
          "_id($id)", atom_g_min_noret>;
      def ATOM_G_UMAX_NORET    : BinAtomNoRet<IL_OP_UAV_UMAX,
          "_id($id)", atom_g_umax_noret>;
      def ATOM_G_UMIN_NORET    : BinAtomNoRet<IL_OP_UAV_UMIN,
          "_id($id)", atom_g_umin_noret>;
      def ATOM_G_OR_NORET      : BinAtomNoRet<IL_OP_UAV_OR,
          "_id($id)", atom_g_or_noret>;
      def ATOM_G_RSUB_NORET    : BinAtomNoRet<IL_OP_UAV_RSUB,
          "_id($id)", atom_g_rsub_noret>;
      def ATOM_G_SUB_NORET     : BinAtomNoRet<IL_OP_UAV_SUB,
          "_id($id)", atom_g_sub_noret>;
      def ATOM_G_XOR_NORET     : BinAtomNoRet<IL_OP_UAV_XOR,
          "_id($id)", atom_g_xor_noret>;
      def ATOM_G_INC_NORET     : BinAtomNoRet<IL_OP_UAV_INC,
          "_id($id)", atom_g_inc_noret>;
      def ATOM_G_DEC_NORET     : BinAtomNoRet<IL_OP_UAV_DEC,
          "_id($id)", atom_g_dec_noret>;
      def ATOM_G_CMPXCHG_NORET    : CmpXChgNoRet<IL_OP_UAV_CMP,
          "_id($id)", atom_g_cmpxchg_noret>;
          // All of the atomic functions that return
      def ATOM_G_ADD     : BinAtom<IL_OP_UAV_READ_ADD,
          "_id($id)", atom_g_add>;
      def ATOM_G_AND     : BinAtom<IL_OP_UAV_READ_AND,
          "_id($id)", atom_g_and>;
      def ATOM_G_MAX     : BinAtom<IL_OP_UAV_READ_MAX,
          "_id($id)", atom_g_max>;
      def ATOM_G_MIN     : BinAtom<IL_OP_UAV_READ_MIN,
          "_id($id)", atom_g_min>;
      def ATOM_G_UMAX    : BinAtom<IL_OP_UAV_READ_UMAX,
          "_id($id)", atom_g_umax>;
      def ATOM_G_UMIN    : BinAtom<IL_OP_UAV_READ_UMIN,
          "_id($id)", atom_g_umin>;
      def ATOM_G_OR      : BinAtom<IL_OP_UAV_READ_OR,
          "_id($id)", atom_g_or>;
      def ATOM_G_RSUB    : BinAtom<IL_OP_UAV_READ_RSUB,
          "_id($id)", atom_g_rsub>;
      def ATOM_G_SUB     : BinAtom<IL_OP_UAV_READ_SUB,
          "_id($id)", atom_g_sub>;
      def ATOM_G_XOR     : BinAtom<IL_OP_UAV_READ_XOR,
          "_id($id)", atom_g_xor>;
      def ATOM_G_INC     : BinAtom<IL_OP_UAV_READ_INC,
          "_id($id)", atom_g_inc>;
      def ATOM_G_DEC     : BinAtom<IL_OP_UAV_READ_DEC,
          "_id($id)", atom_g_dec>;
      def ATOM_G_XCHG    : BinAtom<IL_OP_UAV_READ_XCHG,
          "_id($id)", atom_g_xchg>;
      def ATOM_G_CMPXCHG : CmpXChg<IL_OP_UAV_READ_CMPXCHG,
          "_id($id)", atom_g_cmpxchg>;
      def ATOM_G_ADD_NORET_B64  : BinAtomNoRetI64<IL_OP_UAV_ADD,
          "_b64_id($id)", atom_g_add_noret>;
      def ATOM_G_AND_NORET_B64  : BinAtomNoRetI64<IL_OP_UAV_AND,
          "_b64_id($id)", atom_g_and_noret>;
      def ATOM_G_MAX_NORET_B64  : BinAtomNoRetI64<IL_OP_UAV_MAX,
          "_b64_id($id)", atom_g_max_noret>;
      def ATOM_G_MIN_NORET_B64  : BinAtomNoRetI64<IL_OP_UAV_MIN,
          "_b64_id($id)", atom_g_min_noret>;
      def ATOM_G_UMAX_NORET_B64 : BinAtomNoRetI64<IL_OP_UAV_UMAX,
          "_b64_id($id)", atom_g_umax_noret>;
      def ATOM_G_UMIN_NORET_B64 : BinAtomNoRetI64<IL_OP_UAV_UMIN,
          "_b64_id($id)", atom_g_umin_noret>;
      def ATOM_G_OR_NORET_B64   : BinAtomNoRetI64<IL_OP_UAV_OR,
          "_b64_id($id)", atom_g_or_noret>;
      def ATOM_G_RSUB_NORET_B64 : BinAtomNoRetI64<IL_OP_UAV_RSUB,
          "_b64_id($id)", atom_g_rsub_noret>;
      def ATOM_G_SUB_NORET_B64  : BinAtomNoRetI64<IL_OP_UAV_SUB,
          "_b64_id($id)", atom_g_sub_noret>;
      def ATOM_G_XOR_NORET_B64  : BinAtomNoRetI64<IL_OP_UAV_XOR,
          "_b64_id($id)", atom_g_xor_noret>;
      def ATOM_G_INC_NORET_B64  : BinAtomNoRetI64<IL_OP_UAV_INC,
          "_b64_id($id)", atom_g_inc_noret>;
      def ATOM_G_DEC_NORET_B64  : BinAtomNoRetI64<IL_OP_UAV_DEC,
          "_b64_id($id)", atom_g_dec_noret>;
      def ATOM_G_CMPXCHG_NORET_B64 : CmpXChgNoRetI64<IL_OP_UAV_CMP,
          "_b64_id($id)", atom_g_cmpxchg_noret>;
      def ATOM_G_ADD_B64     : BinAtomI64<IL_OP_UAV_READ_ADD,
          "_b64_id($id)", atom_g_add>;
      def ATOM_G_AND_B64     : BinAtomI64<IL_OP_UAV_READ_AND,
          "_b64_id($id)", atom_g_and>;
      def ATOM_G_MAX_B64     : BinAtomI64<IL_OP_UAV_READ_MAX,
          "_b64_id($id)", atom_g_max>;
      def ATOM_G_MIN_B64     : BinAtomI64<IL_OP_UAV_READ_MIN,
          "_b64_id($id)", atom_g_min>;
      def ATOM_G_UMAX_B64    : BinAtomI64<IL_OP_UAV_READ_UMAX,
          "_b64_id($id)", atom_g_umax>;
      def ATOM_G_UMIN_B64    : BinAtomI64<IL_OP_UAV_READ_UMIN,
          "_b64_id($id)", atom_g_umin>;
      def ATOM_G_OR_B64      : BinAtomI64<IL_OP_UAV_READ_OR,
          "_b64_id($id)", atom_g_or>;
      def ATOM_G_RSUB_B64    : BinAtomI64<IL_OP_UAV_READ_RSUB,
          "_b64_id($id)", atom_g_rsub>;
      def ATOM_G_SUB_B64     : BinAtomI64<IL_OP_UAV_READ_SUB,
          "_b64_id($id)", atom_g_sub>;
      def ATOM_G_XOR_B64     : BinAtomI64<IL_OP_UAV_READ_XOR,
          "_b64_id($id)", atom_g_xor>;
      def ATOM_G_INC_B64     : BinAtomI64<IL_OP_UAV_READ_INC,
          "_b64_id($id)", atom_g_inc>;
      def ATOM_G_DEC_B64     : BinAtomI64<IL_OP_UAV_READ_DEC,
          "_b64_id($id)", atom_g_dec>;
      def ATOM_G_XCHG_B64    : BinAtomI64<IL_OP_UAV_READ_XCHG,
          "_b64_id($id)", atom_g_xchg>;
      def ATOM_G_CMPXCHG_B64 : CmpXChgI64<IL_OP_UAV_READ_CMPXCHG,
          "_b64_id($id)", atom_g_cmpxchg>;
    }
    let Predicates = [ArenaSupport], arenauav = 1 in {
      def ATOM_A_ADD_NORET     : BinAtomNoRet<IL_OP_UAV_ADD,
          "_id($id)_arena", atom_g_add_noret>;
      def ATOM_A_AND_NORET     : BinAtomNoRet<IL_OP_UAV_AND,
          "_id($id)_arena", atom_g_and_noret>;
      def ATOM_A_MAX_NORET     : BinAtomNoRet<IL_OP_UAV_MAX,
          "_id($id)_arena", atom_g_max_noret>;
      def ATOM_A_MIN_NORET     : BinAtomNoRet<IL_OP_UAV_MIN,
          "_id($id)_arena", atom_g_min_noret>;
      def ATOM_A_UMAX_NORET    : BinAtomNoRet<IL_OP_UAV_UMAX,
          "_id($id)_arena", atom_g_umax_noret>;
      def ATOM_A_UMIN_NORET    : BinAtomNoRet<IL_OP_UAV_UMIN,
          "_id($id)_arena", atom_g_umin_noret>;
      def ATOM_A_OR_NORET      : BinAtomNoRet<IL_OP_UAV_OR,
          "_id($id)_arena", atom_g_or_noret>;
      def ATOM_A_RSUB_NORET    : BinAtomNoRet<IL_OP_UAV_RSUB,
          "_id($id)_arena", atom_g_rsub_noret>;
      def ATOM_A_SUB_NORET     : BinAtomNoRet<IL_OP_UAV_SUB,
          "_id($id)_arena", atom_g_sub_noret>;
      def ATOM_A_XOR_NORET     : BinAtomNoRet<IL_OP_UAV_XOR,
          "_id($id)_arena", atom_g_xor_noret>;
      def ATOM_A_INC_NORET     : BinAtomNoRet<IL_OP_UAV_INC,
          "_id($id)_arena", atom_g_inc_noret>;
      def ATOM_A_DEC_NORET     : BinAtomNoRet<IL_OP_UAV_DEC,
          "_id($id)_arena", atom_g_dec_noret>;
      def ATOM_A_CMPXCHG_NORET    : CmpXChgNoRet<IL_OP_UAV_CMP,
          "_id($id)_arena", atom_g_cmpxchg_noret>;
      def ATOM_A_ADD     : BinAtom<IL_OP_UAV_READ_ADD,
          "_id($id)_arena", atom_g_add>;
      def ATOM_A_AND     : BinAtom<IL_OP_UAV_READ_AND,
          "_id($id)_arena", atom_g_and>;
      def ATOM_A_MAX     : BinAtom<IL_OP_UAV_READ_MAX,
          "_id($id)_arena", atom_g_max>;
      def ATOM_A_MIN     : BinAtom<IL_OP_UAV_READ_MIN,
          "_id($id)_arena", atom_g_min>;
      def ATOM_A_UMAX    : BinAtom<IL_OP_UAV_READ_UMAX,
          "_id($id)_arena", atom_g_umax>;
      def ATOM_A_UMIN    : BinAtom<IL_OP_UAV_READ_UMIN,
          "_id($id)_arena", atom_g_umin>;
      def ATOM_A_OR      : BinAtom<IL_OP_UAV_READ_OR,
          "_id($id)_arena", atom_g_or>;
      def ATOM_A_RSUB    : BinAtom<IL_OP_UAV_READ_RSUB,
          "_id($id)_arena", atom_g_rsub>;
      def ATOM_A_SUB     : BinAtom<IL_OP_UAV_READ_SUB,
          "_id($id)_arena", atom_g_sub>;
      def ATOM_A_XOR     : BinAtom<IL_OP_UAV_READ_XOR,
          "_id($id)_arena", atom_g_xor>;
      def ATOM_A_INC     : BinAtom<IL_OP_UAV_READ_INC,
          "_id($id)_arena", atom_g_inc>;
      def ATOM_A_DEC     : BinAtom<IL_OP_UAV_READ_DEC,
          "_id($id)_arena", atom_g_dec>;
      def ATOM_A_XCHG    : BinAtom<IL_OP_UAV_READ_XCHG,
          "_id($id)_arena", atom_g_xchg>;
      def ATOM_A_CMPXCHG : CmpXChg<IL_OP_UAV_READ_CMPXCHG,
          "_id($id)_arena", atom_g_cmpxchg>;
    }
    let local = 1 in {
      def ATOM_L_ADD_NORET     : BinAtomNoRet<IL_OP_LDS_ADD,
          "_id($id)", atom_l_add_noret>;
      def ATOM_L_AND_NORET     : BinAtomNoRet<IL_OP_LDS_AND,
          "_id($id)", atom_l_and_noret>;
      def ATOM_L_MAX_NORET     : BinAtomNoRet<IL_OP_LDS_MAX,
          "_id($id)", atom_l_max_noret>;
      def ATOM_L_MIN_NORET     : BinAtomNoRet<IL_OP_LDS_MIN,
          "_id($id)", atom_l_min_noret>;
      def ATOM_L_UMAX_NORET    : BinAtomNoRet<IL_OP_LDS_UMAX,
          "_id($id)", atom_l_umax_noret>;
      def ATOM_L_UMIN_NORET    : BinAtomNoRet<IL_OP_LDS_UMIN,
          "_id($id)", atom_l_umin_noret>;
      def ATOM_L_MSKOR_NORET   : TriAtomNoRet<IL_OP_LDS_MSKOR,
          "_id($id)", atom_l_mskor_noret>;
      def ATOM_L_OR_NORET      : BinAtomNoRet<IL_OP_LDS_OR,
          "_id($id)", atom_l_or_noret>;
      def ATOM_L_RSUB_NORET    : BinAtomNoRet<IL_OP_LDS_RSUB,
          "_id($id)", atom_l_rsub_noret>;
      def ATOM_L_SUB_NORET     : BinAtomNoRet<IL_OP_LDS_SUB,
          "_id($id)", atom_l_sub_noret>;
      def ATOM_L_XOR_NORET     : BinAtomNoRet<IL_OP_LDS_XOR,
          "_id($id)", atom_l_xor_noret>;
      def ATOM_L_INC_NORET     : BinAtomNoRet<IL_OP_LDS_INC,
          "_id($id)", atom_l_inc_noret>;
      def ATOM_L_DEC_NORET     : BinAtomNoRet<IL_OP_LDS_DEC,
          "_id($id)", atom_l_dec_noret>;
      def ATOM_L_CMPXCHG_NORET    : TriAtomNoRet<IL_OP_LDS_CMP,
          "_id($id)", atom_l_cmpxchg_noret>;
      def ATOM_L_ADD     : BinAtom<IL_OP_LDS_READ_ADD,
          "_id($id)", atom_l_add>;
      def ATOM_L_AND     : BinAtom<IL_OP_LDS_READ_AND,
          "_id($id)", atom_l_and>;
      def ATOM_L_MAX     : BinAtom<IL_OP_LDS_READ_MAX,
          "_id($id)", atom_l_max>;
      def ATOM_L_MIN     : BinAtom<IL_OP_LDS_READ_MIN,
          "_id($id)", atom_l_min>;
      def ATOM_L_UMAX    : BinAtom<IL_OP_LDS_READ_UMAX,
          "_id($id)", atom_l_umax>;
      def ATOM_L_UMIN    : BinAtom<IL_OP_LDS_READ_UMIN,
          "_id($id)", atom_l_umin>;
      def ATOM_L_OR      : BinAtom<IL_OP_LDS_READ_OR,
          "_id($id)", atom_l_or>;
      def ATOM_L_MSKOR   : TriAtom<IL_OP_LDS_READ_MSKOR,
          "_id($id)", atom_l_mskor>;
      def ATOM_L_RSUB    : BinAtom<IL_OP_LDS_READ_RSUB,
          "_id($id)", atom_l_rsub>;
      def ATOM_L_SUB     : BinAtom<IL_OP_LDS_READ_SUB,
          "_id($id)", atom_l_sub>;
      def ATOM_L_XOR     : BinAtom<IL_OP_LDS_READ_XOR,
          "_id($id)", atom_l_xor>;
      def ATOM_L_INC     : BinAtom<IL_OP_LDS_READ_INC,
          "_id($id)", atom_l_inc>;
      def ATOM_L_DEC     : BinAtom<IL_OP_LDS_READ_DEC,
          "_id($id)", atom_l_dec>;
      def ATOM_L_XCHG    : BinAtom<IL_OP_LDS_READ_XCHG,
          "_id($id)", atom_l_xchg>;
      def ATOM_L_CMPXCHG : TriAtom<IL_OP_LDS_READ_CMPXCHG,
          "_id($id)", atom_l_cmpxchg>;
      def ATOM_L_ADD_NORET_B64  : BinAtomNoRetI64<IL_OP_LDS_ADD,
          "64_id($id)", atom_l_add_noret>;
      def ATOM_L_AND_NORET_B64  : BinAtomNoRetI64<IL_OP_LDS_AND,
          "64_id($id)", atom_l_and_noret>;
      def ATOM_L_MAX_NORET_B64  : BinAtomNoRetI64<IL_OP_LDS_MAX,
          "64_id($id)", atom_l_max_noret>;
      def ATOM_L_MIN_NORET_B64  : BinAtomNoRetI64<IL_OP_LDS_MIN,
          "64_id($id)", atom_l_min_noret>;
      def ATOM_L_UMAX_NORET_B64 : BinAtomNoRetI64<IL_OP_LDS_UMAX,
          "64_id($id)", atom_l_umax_noret>;
      def ATOM_L_UMIN_NORET_B64 : BinAtomNoRetI64<IL_OP_LDS_UMIN,
          "64_id($id)", atom_l_umin_noret>;
      def ATOM_L_MSKOR_NORET_B64: TriAtomNoRetI64<IL_OP_LDS_MSKOR,
          "64_id($id)", atom_l_mskor_noret>;
      def ATOM_L_OR_NORET_B64   : BinAtomNoRetI64<IL_OP_LDS_OR,
          "64_id($id)", atom_l_or_noret>;
      def ATOM_L_RSUB_NORET_B64 : BinAtomNoRetI64<IL_OP_LDS_RSUB,
          "64_id($id)", atom_l_rsub_noret>;
      def ATOM_L_SUB_NORET_B64  : BinAtomNoRetI64<IL_OP_LDS_SUB,
          "64_id($id)", atom_l_sub_noret>;
      def ATOM_L_XOR_NORET_B64  : BinAtomNoRetI64<IL_OP_LDS_XOR,
          "64_id($id)", atom_l_xor_noret>;
      def ATOM_L_INC_NORET_B64  : BinAtomNoRetI64<IL_OP_LDS_INC,
          "64_id($id)", atom_l_inc_noret>;
      def ATOM_L_DEC_NORET_B64  : BinAtomNoRetI64<IL_OP_LDS_DEC,
          "64_id($id)", atom_l_dec_noret>;
      def ATOM_L_CMPXCHG_NORET_B64 : TriAtomNoRetI64<IL_OP_LDS_CMP,
          "64_id($id)", atom_l_cmpxchg_noret>;
      def ATOM_L_ADD_B64     : BinAtomI64<IL_OP_LDS_READ_ADD,
          "64_id($id)", atom_l_add>;
      def ATOM_L_AND_B64     : BinAtomI64<IL_OP_LDS_READ_AND,
          "64_id($id)", atom_l_and>;
      def ATOM_L_MAX_B64     : BinAtomI64<IL_OP_LDS_READ_MAX,
          "64_id($id)", atom_l_max>;
      def ATOM_L_MIN_B64    : BinAtomI64<IL_OP_LDS_READ_MIN,
          "64_id($id)", atom_l_min>;
      def ATOM_L_UMAX_B64    : BinAtomI64<IL_OP_LDS_READ_UMAX,
          "64_id($id)", atom_l_umax>;
      def ATOM_L_UMIN_B64    : BinAtomI64<IL_OP_LDS_READ_UMIN,
          "64_id($id)", atom_l_umin>;
      def ATOM_L_OR_B64      : BinAtomI64<IL_OP_LDS_READ_OR,
          "64_id($id)", atom_l_or>;
      def ATOM_L_MSKOR_B64   : TriAtomI64<IL_OP_LDS_READ_MSKOR,
          "64_id($id)", atom_l_mskor>;
      def ATOM_L_RSUB_B64    : BinAtomI64<IL_OP_LDS_READ_RSUB,
          "64_id($id)", atom_l_rsub>;
      def ATOM_L_SUB_B64     : BinAtomI64<IL_OP_LDS_READ_SUB,
          "64_id($id)", atom_l_sub>;
      def ATOM_L_XOR_B64     : BinAtomI64<IL_OP_LDS_READ_XOR,
          "64_id($id)", atom_l_xor>;
      def ATOM_L_INC_B64     : BinAtomI64<IL_OP_LDS_READ_INC,
          "64_id($id)", atom_l_inc>;
      def ATOM_L_DEC_B64     : BinAtomI64<IL_OP_LDS_READ_DEC,
          "64_id($id)", atom_l_dec>;
      def ATOM_L_XCHG_B64    : BinAtomI64<IL_OP_LDS_READ_XCHG,
          "64_id($id)", atom_l_xchg>;
      def ATOM_L_CMPXCHG_B64 : TriAtomI64<IL_OP_LDS_READ_CMPXCHG,
          "64_id($id)", atom_l_cmpxchg>;
    }
    let region = 1 in {
      def ATOM_R_ADD_NORET     : BinAtomNoRet<IL_OP_GDS_ADD,
          "_id($id)", atom_r_add_noret>;
      def ATOM_R_AND_NORET     : BinAtomNoRet<IL_OP_GDS_AND,
          "_id($id)", atom_r_and_noret>;
      def ATOM_R_MAX_NORET     : BinAtomNoRet<IL_OP_GDS_MAX,
          "_id($id)", atom_r_max_noret>;
      def ATOM_R_MIN_NORET     : BinAtomNoRet<IL_OP_GDS_MIN,
          "_id($id)", atom_r_min_noret>;
      def ATOM_R_UMAX_NORET    : BinAtomNoRet<IL_OP_GDS_UMAX,
          "_id($id)", atom_r_umax_noret>;
      def ATOM_R_UMIN_NORET    : BinAtomNoRet<IL_OP_GDS_UMIN,
          "_id($id)", atom_r_umin_noret>;
      def ATOM_R_MSKOR_NORET   : TriAtomNoRet<IL_OP_GDS_MSKOR,
          "_id($id)", atom_r_mskor_noret>;
      def ATOM_R_OR_NORET      : BinAtomNoRet<IL_OP_GDS_OR,
          "_id($id)", atom_r_or_noret>;
      def ATOM_R_RSUB_NORET    : BinAtomNoRet<IL_OP_GDS_RSUB,
          "_id($id)", atom_r_rsub_noret>;
      def ATOM_R_SUB_NORET     : BinAtomNoRet<IL_OP_GDS_SUB,
          "_id($id)", atom_r_sub_noret>;
      def ATOM_R_XOR_NORET     : BinAtomNoRet<IL_OP_GDS_XOR,
          "_id($id)", atom_r_xor_noret>;
      def ATOM_R_INC_NORET     : BinAtomNoRet<IL_OP_GDS_INC,
          "_id($id)", atom_r_inc_noret>;
      def ATOM_R_DEC_NORET     : BinAtomNoRet<IL_OP_GDS_DEC,
          "_id($id)", atom_r_dec_noret>;
      def ATOM_R_CMPXCHG_NORET : TriAtomNoRet<IL_OP_GDS_CMP,
          "_id($id)", atom_r_cmpxchg_noret>;
      def ATOM_R_ADD     : BinAtom<IL_OP_GDS_READ_ADD,
          "_id($id)", atom_r_add>;
      def ATOM_R_AND     : BinAtom<IL_OP_GDS_READ_AND,
          "_id($id)", atom_r_and>;
      def ATOM_R_MAX     : BinAtom<IL_OP_GDS_READ_MAX,
          "_id($id)", atom_r_max>;
      def ATOM_R_MIN     : BinAtom<IL_OP_GDS_READ_MIN,
          "_id($id)", atom_r_min>;
      def ATOM_R_UMAX    : BinAtom<IL_OP_GDS_READ_UMAX,
          "_id($id)", atom_r_umax>;
      def ATOM_R_UMIN    : BinAtom<IL_OP_GDS_READ_UMIN,
          "_id($id)", atom_r_umin>;
      def ATOM_R_OR      : BinAtom<IL_OP_GDS_READ_OR,
          "_id($id)", atom_r_or>;
      def ATOM_R_MSKOR   : TriAtom<IL_OP_GDS_READ_MSKOR,
          "_id($id)", atom_r_mskor>;
      def ATOM_R_RSUB    : BinAtom<IL_OP_GDS_READ_RSUB,
          "_id($id)", atom_r_rsub>;
      def ATOM_R_SUB     : BinAtom<IL_OP_GDS_READ_SUB,
          "_id($id)", atom_r_sub>;
      def ATOM_R_XOR     : BinAtom<IL_OP_GDS_READ_XOR,
          "_id($id)", atom_r_xor>;
      def ATOM_R_INC     : BinAtom<IL_OP_GDS_READ_INC,
          "_id($id)", atom_r_inc>;
      def ATOM_R_DEC     : BinAtom<IL_OP_GDS_READ_DEC,
          "_id($id)", atom_r_dec>;
      def ATOM_R_XCHG    : BinAtom<IL_OP_GDS_READ_XCHG,
          "_id($id)", atom_r_xchg>;
      def ATOM_R_CMPXCHG : TriAtom<IL_OP_GDS_READ_CMPXCHG,
          "_id($id)", atom_r_cmpxchg>;
      def ATOM_R_ADD_NORET_B64  : BinAtomNoRetI64<IL_OP_GDS_ADD,
          "64_id($id)", atom_r_add_noret>;
      def ATOM_R_AND_NORET_B64  : BinAtomNoRetI64<IL_OP_GDS_AND,
          "64_id($id)", atom_r_and_noret>;
      def ATOM_R_MAX_NORET_B64  : BinAtomNoRetI64<IL_OP_GDS_MAX,
          "64_id($id)", atom_r_max_noret>;
      def ATOM_R_MIN_NORET_B64  : BinAtomNoRetI64<IL_OP_GDS_MIN,
          "64_id($id)", atom_r_min_noret>;
      def ATOM_R_UMAX_NORET_B64 : BinAtomNoRetI64<IL_OP_GDS_UMAX,
          "64_id($id)", atom_r_umax_noret>;
      def ATOM_R_UMIN_NORET_B64 : BinAtomNoRetI64<IL_OP_GDS_UMIN,
          "64_id($id)", atom_r_umin_noret>;
      def ATOM_R_MSKOR_NORET_B64: TriAtomNoRetI64<IL_OP_GDS_MSKOR,
          "64_id($id)", atom_r_mskor_noret>;
      def ATOM_R_OR_NORET_B64   : BinAtomNoRetI64<IL_OP_GDS_OR,
          "64_id($id)", atom_r_or_noret>;
      def ATOM_R_RSUB_NORET_B64 : BinAtomNoRetI64<IL_OP_GDS_RSUB,
          "64_id($id)", atom_r_rsub_noret>;
      def ATOM_R_SUB_NORET_B64  : BinAtomNoRetI64<IL_OP_GDS_SUB,
          "64_id($id)", atom_r_sub_noret>;
      def ATOM_R_XOR_NORET_B64  : BinAtomNoRetI64<IL_OP_GDS_XOR,
          "64_id($id)", atom_r_xor_noret>;
      def ATOM_R_INC_NORET_B64  : BinAtomNoRetI64<IL_OP_GDS_INC,
          "64_id($id)", atom_r_inc_noret>;
      def ATOM_R_DEC_NORET_B64  : BinAtomNoRetI64<IL_OP_GDS_DEC,
          "64_id($id)", atom_r_dec_noret>;
      def ATOM_R_CMPXCHG_NORET_B64 : TriAtomNoRetI64<IL_OP_GDS_CMP,
          "64_id($id)", atom_r_cmpxchg_noret>;
      def ATOM_R_ADD_B64     : BinAtomI64<IL_OP_GDS_READ_ADD,
          "64_id($id)", atom_r_add>;
      def ATOM_R_AND_B64     : BinAtomI64<IL_OP_GDS_READ_AND,
          "64_id($id)", atom_r_and>;
      def ATOM_R_MAX_B64     : BinAtomI64<IL_OP_GDS_READ_MAX,
          "64_id($id)", atom_r_max>;
      def ATOM_R_MIN_B64     : BinAtomI64<IL_OP_GDS_READ_MIN,
          "64_id($id)", atom_r_min>;
      def ATOM_R_UMAX_B64    : BinAtomI64<IL_OP_GDS_READ_UMAX,
          "64_id($id)", atom_r_umax>;
      def ATOM_R_UMIN_B64    : BinAtomI64<IL_OP_GDS_READ_UMIN,
          "64_id($id)", atom_r_umin>;
      def ATOM_R_OR_B64      : BinAtomI64<IL_OP_GDS_READ_OR,
          "64_id($id)", atom_r_or>;
      def ATOM_R_MSKOR_B64   : TriAtomI64<IL_OP_GDS_READ_MSKOR,
          "64_id($id)", atom_r_mskor>;
      def ATOM_R_RSUB_B64    : BinAtomI64<IL_OP_GDS_READ_RSUB,
          "64_id($id)", atom_r_rsub>;
      def ATOM_R_SUB_B64     : BinAtomI64<IL_OP_GDS_READ_SUB,
          "64_id($id)", atom_r_sub>;
      def ATOM_R_XOR_B64     : BinAtomI64<IL_OP_GDS_READ_XOR,
          "64_id($id)", atom_r_xor>;
      def ATOM_R_INC_B64     : BinAtomI64<IL_OP_GDS_READ_INC,
          "64_id($id)", atom_r_inc>;
      def ATOM_R_DEC_B64     : BinAtomI64<IL_OP_GDS_READ_DEC,
          "64_id($id)", atom_r_dec>;
      def ATOM_R_XCHG_B64    : BinAtomI64<IL_OP_GDS_READ_XCHG,
          "64_id($id)", atom_r_xchg>;
      def ATOM_R_CMPXCHG_B64 : TriAtomI64<IL_OP_GDS_READ_CMPXCHG,
          "64_id($id)", atom_r_cmpxchg>;
    }
  }

  let append = 1, swizzle = 3 in {
    def APPEND_ALLOC : Append<IL_OP_APPEND_BUF_ALLOC,
        "_id($id)", append_alloc>;
    def APPEND_CONSUME : Append<IL_OP_APPEND_BUF_CONSUME,
        "_id($id)", append_consume>;
  }

  let sema = 1 in {
    let swizzle = 7 in {
      def SEMAPHORE_WAIT : ILFormat<IL_OP_SEMAPHORE_WAIT, (outs),
          (ins MEM32:$ptr),
          !strconcat(IL_OP_SEMAPHORE_WAIT.Text, "_id($ptr)"),
          [(int_AMDIL_semaphore_wait ADDR:$ptr)]>;

      def SEMAPHORE_SIGNAL : ILFormat<IL_OP_SEMAPHORE_SIGNAL, (outs),
          (ins MEM32:$ptr),
          !strconcat(IL_OP_SEMAPHORE_SIGNAL.Text, "_id($ptr)"),
          [(int_AMDIL_semaphore_signal ADDR:$ptr)]>;
    }
  }

  let Predicates = [IsSIOrLaterDevice] in {
    // atomic global load
    defm ATOM_G_LOAD : AtomGLoadMC<MEM32, ADDR>;
    // atomic global store
    defm ATOM_G_STORE : AtomGStoreMC<MEM32, ADDR>;
  }
}
