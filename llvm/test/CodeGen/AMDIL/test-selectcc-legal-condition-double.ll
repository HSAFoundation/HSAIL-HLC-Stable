; RUN: llc -march=amdil -mcpu=tahiti -mattr=+fp64 -o - %s | FileCheck %s
; RUN: llc -march=amdil -mcpu=cypress -mattr=+fp64 -o - %s | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

@.str = internal addrspace(2) constant [10 x i8] c"double16*\00"
@.str1 = internal addrspace(2) constant [10 x i8] c"double16*\00"
@llvm.argtypename.annotations.__OpenCL_test_select_cc_legal_condition_double_kernel = global [2 x i8*] [i8* bitcast ([10 x i8] addrspace(2)* @.str to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str1 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (double addrspace(1)*, double)* @__OpenCL_test_select_cc_legal_condition_double_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define void @__OpenCL_test_select_cc_legal_condition_double_kernel(double addrspace(1)* nocapture %out, double %v) nounwind {
; CHECK: dge
; CHECK: iand
; CHECK: itod
; CHECK: uav_raw_store_id
entry:
  %cmp = fcmp oge double %v, 5.000000e-01
  %ext = zext i1 %cmp to i32
  %foo = sitofp i32 %ext to double
  store double %foo, double addrspace(1)* %out, align 4
  ret void
}


define void @__OpenCL_test_select_cc_legal_condition_double_2_kernel(<2 x double> addrspace(1)* nocapture %out, <2 x double> %v) nounwind {
; CHECK: dge
; CHECK: dge
; CHECK: iadd
; CHECK: iand
; CHECK: itod
; CHECK: itod
; CHECK: uav_raw_store_id
entry:
  %cmp = fcmp oge <2 x double> %v, <double 5.000000e-01, double 5.000000e-01>
  %ext = zext <2 x i1> %cmp to <2 x i32>
  %foo = sitofp <2 x i32> %ext to <2 x double>
  store <2 x double> %foo, <2 x double> addrspace(1)* %out, align 4
  ret void
}

