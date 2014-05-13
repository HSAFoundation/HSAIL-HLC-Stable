//===--------------------------------------------------------------------===//
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
// Intrinsic operation support
//===--------------------------------------------------------------------===//
let TargetPrefix = "AMDIL", isTarget = 1 in {
def int_AMDIL_barrier   : GCCBuiltin<"__amd_barrier">,
     UnaryIntNoRetInt;
def int_AMDIL_barrier_global   : GCCBuiltin<"barrierGlobal">,
     UnaryIntNoRetInt;
def int_AMDIL_barrier_local   : GCCBuiltin<"barrierLocal">,
      UnaryIntNoRetInt;
def int_AMDIL_barrier_region   : GCCBuiltin<"barrierRegion">,
      UnaryIntNoRetInt;
def int_AMDIL_get_region_id : GCCBuiltin<"__amdil_get_region_id_int">,
    Intrinsic<[llvm_v4i32_ty], [], []>;
def int_AMDIL_get_region_local_id : GCCBuiltin<"__amdil_get_region_local_id_int">,
    Intrinsic<[llvm_v4i32_ty], [], []>;
def int_AMDIL_get_num_regions : GCCBuiltin<"__amdil_get_num_regions_int">,
    Intrinsic<[llvm_v4i32_ty], [], []>;
def int_AMDIL_get_region_size : GCCBuiltin<"__amdil_get_region_size_int">,
    Intrinsic<[llvm_v4i32_ty], [], []>;
}

let isCall=1, isNotDuplicable=1 in {
  let Predicates=[hasRegionAS] in {
def BARRIER_EGNI : ILFormat<IL_OP_BARRIER, (outs),
      (ins GPRI32:$flag),
      "fence_threads_memory_lds_gds_gws",
      [(int_AMDIL_barrier GPRI32:$flag)]>;
}
let Predicates=[noRegionAS] in {
def BARRIER_7XX : ILFormat<IL_OP_BARRIER, (outs),
      (ins GPRI32:$flag, GPRI32:$id),
      "fence_threads_memory_lds",
      [(int_AMDIL_barrier GPRI32:$flag)]>;
}

def BARRIER_LOCAL : ILFormat<IL_OP_BARRIER_LOCAL, (outs),
      (ins GPRI32:$flag),
      "fence_threads_lds",
      [(int_AMDIL_barrier_local GPRI32:$flag)]>;

def BARRIER_GLOBAL : ILFormat<IL_OP_BARRIER_GLOBAL, (outs),
      (ins GPRI32:$flag),
      "fence_threads_memory",
      [(int_AMDIL_barrier_global GPRI32:$flag)]>;

def BARRIER_REGION : ILFormat<IL_OP_BARRIER_REGION, (outs),
    (ins GPRI32:$flag),
    "fence_threads_gds",
    [(int_AMDIL_barrier_region GPRI32:$flag)]>;

def GET_REGION_ID : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
    (ins), !strconcat(IL_OP_MOV.Text, " $dst, r1022.xyz0"),
    [(set GPRV4I32:$dst, (int_AMDIL_get_region_id))]>;

def GET_REGION_LOCAL_ID : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
    (ins), !strconcat(IL_OP_MOV.Text, " $dst, r1022.xyz0"),
    [(set GPRV4I32:$dst, (int_AMDIL_get_region_local_id))]>;

def GET_REGION_SIZE : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
    (ins), !strconcat(IL_OP_MOV.Text, " $dst, cb0[10].xyz0"),
    [(set GPRV4I32:$dst, (int_AMDIL_get_region_size))]>;

def GET_NUM_REGIONS : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
    (ins), !strconcat(IL_OP_MOV.Text, " $dst, cb0[11].xyz0"),
    [(set GPRV4I32:$dst, (int_AMDIL_get_num_regions))]>;

}
