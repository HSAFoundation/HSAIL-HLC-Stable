; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s
; RUN: llc -march=amdil -mcpu=barts < %s | FileCheck %s

; CHECK: uav_raw_load_id({{[0-9]+}})_cached r1011.x___, r1.y
; CHECK: iadd r{{[0-9]+}}.xy__, r1011.x000, l{{[0-9]+}}
; CHECK: uav_raw_load_id({{[0-9]+}})_cached r1011.x___, r{{.*}}
; CHECK: iadd r{{[0-9]+}}.__zw, r1011.00x0, l{{[0-9]+}}

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-unknown-amdopencl"

@.str = internal addrspace(2) constant [7 x i8] c"ulong*\00"
@.str1 = internal addrspace(2) constant [6 x i8] c"uint*\00"
@llvm.argtypename.annotations.__OpenCL_sample_test_kernel = global [2 x i8*] [i8* bitcast ([7 x i8] addrspace(2)* @.str to i8*), i8* bitcast ([6 x i8] addrspace(2)* @.str1 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (i64 addrspace(1)*, i32 addrspace(1)*)* @__OpenCL_sample_test_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define void @__OpenCL_sample_test_kernel(i64 addrspace(1)* nocapture %result, i32 addrspace(1)* nocapture %array_sizes) nounwind {
entry:
  %tmp1 = load i32 addrspace(1)* %array_sizes, align 4
  %conv1 = zext i32 %tmp1 to i64
  %addr2 = getelementptr i32 addrspace(1)* %array_sizes, i32 1
  %tmp2 = load i32 addrspace(1)* %addr2, align 4
  %conv2 = zext i32 %tmp2 to i64
  %tmp3 = add i64 %conv1, %conv2
  store i64 %tmp3, i64 addrspace(1)* %result, align 8
  ret void
}
