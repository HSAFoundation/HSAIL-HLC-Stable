; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

@.str = internal addrspace(2) constant [4 x i8] c"out\00"
@.str1 = internal addrspace(2) constant [5 x i8] c"int*\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_read_poison_i32_kernel = global [1 x i8*] [i8* bitcast ([4 x i8] addrspace(2)* @.str to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_read_poison_i32_kernel = global [1 x i8*] [i8* bitcast ([5 x i8] addrspace(2)* @.str1 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@.str2 = internal addrspace(2) constant [4 x i8] c"out\00"
@.str3 = internal addrspace(2) constant [6 x i8] c"int2*\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_read_poison_v2i32_kernel = global [1 x i8*] [i8* bitcast ([4 x i8] addrspace(2)* @.str2 to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_read_poison_v2i32_kernel = global [1 x i8*] [i8* bitcast ([6 x i8] addrspace(2)* @.str3 to i8*)], section "llvm.metadata"
@sgv4 = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv5 = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv6 = internal constant [0 x i8*] zeroinitializer
@rvgv7 = internal constant [0 x i8*] zeroinitializer
@.str8 = internal addrspace(2) constant [4 x i8] c"out\00"
@.str9 = internal addrspace(2) constant [6 x i8] c"int4*\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_read_poison_v4i32_kernel = global [1 x i8*] [i8* bitcast ([4 x i8] addrspace(2)* @.str8 to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_read_poison_v4i32_kernel = global [1 x i8*] [i8* bitcast ([6 x i8] addrspace(2)* @.str9 to i8*)], section "llvm.metadata"
@sgv10 = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv11 = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv12 = internal constant [0 x i8*] zeroinitializer
@rvgv13 = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [3 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (i32 addrspace(1)*)* @__OpenCL_read_poison_i32_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }, { i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (<2 x i32> addrspace(1)*)* @__OpenCL_read_poison_v2i32_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv4 to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv5 to i8*), i8* bitcast ([0 x i8*]* @lvgv6 to i8*), i8* bitcast ([0 x i8*]* @rvgv7 to i8*), i32 0 }, { i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (<4 x i32> addrspace(1)*)* @__OpenCL_read_poison_v4i32_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv10 to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv11 to i8*), i8* bitcast ([0 x i8*]* @lvgv12 to i8*), i8* bitcast ([0 x i8*]* @rvgv13 to i8*), i32 0 }], section "llvm.metadata"

define  void @__OpenCL_read_poison_i32_kernel(i32 addrspace(1)* nocapture %out) nounwind {
; CHECK: ; @__OpenCL_read_poison_i32_kernel
; CHECK: poisonv [[REG:r[0-9]+]].x___,
entry:
  %call = tail call  i32 @__amdil_poisonv_i32(i32 0) nounwind
  store i32 %call, i32 addrspace(1)* %out, align 4
  ret void
}

declare  i32 @__amdil_poisonv_i32(i32) nounwind

define  void @__OpenCL_read_poison_v2i32_kernel(<2 x i32> addrspace(1)* nocapture %out) nounwind {
; CHECK: ; @__OpenCL_read_poison_v2i32_kernel
; CHECK: poisonv [[REG:r[0-9]+]].xy__,
entry:
  %call = tail call  <2 x i32> @__amdil_poisonv_v2i32(<2 x i32> zeroinitializer) nounwind
  store <2 x i32> %call, <2 x i32> addrspace(1)* %out, align 8
  ret void
}

declare  <2 x i32> @__amdil_poisonv_v2i32(<2 x i32>) nounwind

define  void @__OpenCL_read_poison_v4i32_kernel(<4 x i32> addrspace(1)* nocapture %out) nounwind {
; CHECK: ; @__OpenCL_read_poison_v4i32_kernel
; CHECK: poisonv [[REG:r[0-9]+]],
entry:
  %call = tail call  <4 x i32> @__amdil_poisonv_v4i32(<4 x i32> zeroinitializer) nounwind
  store <4 x i32> %call, <4 x i32> addrspace(1)* %out, align 16
  ret void
}

declare  <4 x i32> @__amdil_poisonv_v4i32(<4 x i32>) nounwind
