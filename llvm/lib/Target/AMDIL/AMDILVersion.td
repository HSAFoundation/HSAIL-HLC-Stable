//===-- AMDILVersion.td ---------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Intrinsic operation support.
//
//===----------------------------------------------------------------------===//

let TargetPrefix = "AMDIL", isTarget = 1 in {
  def int_AMDIL_barrier : GCCBuiltin<"__amdil_barrier">,
    Intrinsic<[], [], [IntrReadMem]>;
  def int_AMDIL_barrier_global : GCCBuiltin<"__amdil_barrier_global">,
    Intrinsic<[], [], [IntrReadMem]>;
  def int_AMDIL_barrier_local : GCCBuiltin<"__amdil_barrier_local">,
    Intrinsic<[], [], [IntrReadMem]>;
  def int_AMDIL_barrier_global_local : GCCBuiltin<"__amdil_barrier_global_local">,
    Intrinsic<[], [], [IntrReadMem]>;
  def int_AMDIL_barrier_region : GCCBuiltin<"__amdil_barrier_region">,
    Intrinsic<[], [], [IntrReadMem]>;
  def int_AMDIL_barrier_region_local : GCCBuiltin<"__amdil_barrier_region_local">,
    Intrinsic<[], [], [IntrReadMem]>;
  def int_AMDIL_barrier_global_region : GCCBuiltin<"__amdil_barrier_global_region">,
    Intrinsic<[], [], [IntrReadMem]>;

  def int_AMDIL_get_region_id : GCCBuiltin<"__amdil_get_region_id_int">,
    Intrinsic<[llvm_v4i32_ty], [], []>;
  def int_AMDIL_get_region_local_id : GCCBuiltin<"__amdil_get_region_local_id_int">,
    Intrinsic<[llvm_v4i32_ty], [], []>;
  def int_AMDIL_get_num_regions : GCCBuiltin<"__amdil_get_num_regions_int">,
    Intrinsic<[llvm_v4i32_ty], [], []>;
  def int_AMDIL_get_region_size : GCCBuiltin<"__amdil_get_region_size_int">,
    Intrinsic<[llvm_v4i32_ty], [], []>;
}

let Predicates = [hasRegionAS] in {
  let isCall = 1, isNotDuplicable = 1 in {
    def BARRIER : ILFormat<IL_OP_BARRIER, (outs),
      (ins),
      "fence_threads_memory_gds_lds", // FIXME: gws causes hang on SI
      [(int_AMDIL_barrier)]>;
    def BARRIER_REGION : ILFormat<IL_OP_BARRIER_REGION, (outs),
      (ins),
      "fence_threads_gds",
      [(int_AMDIL_barrier_region)]>;

    def BARRIER_REGION_LOCAL : ILFormat<IL_OP_BARRIER_REGION_LOCAL, (outs),
      (ins),
      "fence_threads_gds_lds",
      [(int_AMDIL_barrier_region_local)]>;

    def BARRIER_GLOBAL_REGION : ILFormat<IL_OP_BARRIER_GLOBAL_REGION, (outs),
      (ins),
      "fence_threads_memory_gds",
      [(int_AMDIL_barrier_global_region)]>;
  }

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

// Ones without region
let isCall = 1, isNotDuplicable = 1 in {
  def BARRIER_GLOBAL : ILFormat<IL_OP_BARRIER_GLOBAL, (outs),
    (ins),
    "fence_threads_memory",
    [(int_AMDIL_barrier_global)]>;

  def BARRIER_LOCAL : ILFormat<IL_OP_BARRIER_LOCAL, (outs),
    (ins),
    "fence_threads_lds",
    [(int_AMDIL_barrier_local)]>;

  def BARRIER_GLOBAL_LOCAL : ILFormat<IL_OP_BARRIER_GLOBAL_LOCAL, (outs),
    (ins),
    "fence_threads_memory_lds",
    [(int_AMDIL_barrier_global_local)]>;
}
