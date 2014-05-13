; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

@.str = internal addrspace(2) constant [6 x i8] c"input\00"
@.str1 = internal addrspace(2) constant [6 x i8] c"uint*\00"
@.str2 = internal addrspace(2) constant [9 x i8] c"outMaxes\00"
@.str3 = internal addrspace(2) constant [6 x i8] c"uint*\00"
@llvm.restrictpointer.annotations.__OpenCL_test_kernel = global [2 x i8*] [i8* bitcast ([6 x i8] addrspace(2)* @.str to i8*), i8* bitcast ([9 x i8] addrspace(2)* @.str2 to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_test_kernel = global [2 x i8*] [i8* bitcast ([6 x i8] addrspace(2)* @.str1 to i8*), i8* bitcast ([6 x i8] addrspace(2)* @.str3 to i8*)], section "llvm.metadata"
@test_cllocal_localStorage = internal addrspace(3) global [256 x i32] zeroinitializer, align 4
@sgv = internal addrspace(2) constant [11 x i8] c"RWG256,1,1\00"
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [1 x i8*] [i8* bitcast ([256 x i32] addrspace(3)* @test_cllocal_localStorage to i8*)]
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*)* @__OpenCL_test_kernel to i8*), i8* bitcast ([11 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([1 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

; TODO: The branch on the get_local_id should be removed
define  void @__OpenCL_test_kernel(i32 addrspace(1)* noalias nocapture %input, i32 addrspace(1)* noalias nocapture %outMaxes) nounwind {
; CHECK: fence_threads_lds
; CHECK: whileloop
; CHECK: break_logicalz
; CHECK: endloop
entry:
  %0 = tail call  <4 x i32> @__amdil_get_global_id_int() nounwind
  %1 = extractelement <4 x i32> %0, i32 0
  %2 = tail call  <4 x i32> @__amdil_get_local_id_int() nounwind
  %3 = extractelement <4 x i32> %2, i32 0
  %arrayidx = getelementptr [256 x i32] addrspace(3)* @test_cllocal_localStorage, i32 0, i32 %3
  %arrayidx5 = getelementptr i32 addrspace(1)* %input, i32 %1
  %tmp6 = load i32 addrspace(1)* %arrayidx5, align 4
  store i32 %tmp6, i32 addrspace(3)* %arrayidx, align 4
  tail call  void @barrier(i32 0, i32 1) nounwind
  %cmp = icmp eq i32 %3, 0
  br i1 %cmp, label %for.cond.preheader, label %if.end

for.cond.preheader:                               ; preds = %entry
  %4 = tail call  <4 x i32> @__amdil_get_local_size_int() nounwind
  %5 = extractelement <4 x i32> %4, i32 0
  %cmp101 = icmp eq i32 %5, 0
  br i1 %cmp101, label %for.exit, label %for.body

if.end:                                           ; preds = %for.exit, %entry
  ret void

for.exit:                                         ; preds = %for.body, %for.cond.preheader
  %groupMax.0.lcssa = phi i32 [ 0, %for.cond.preheader ], [ %6, %for.body ]
  store i32 %groupMax.0.lcssa, i32 addrspace(1)* %outMaxes, align 4
  br label %if.end

for.body:                                         ; preds = %for.body, %for.cond.preheader
  %i.03 = phi i32 [ %tmp19, %for.body ], [ 0, %for.cond.preheader ]
  %groupMax.02 = phi i32 [ %6, %for.body ], [ 0, %for.cond.preheader ]
  %arrayidx14 = getelementptr [256 x i32] addrspace(3)* @test_cllocal_localStorage, i32 0, i32 %i.03
  %tmp15 = load i32 addrspace(3)* %arrayidx14, align 4
  %6 = tail call  i32 @__amdil_umax_u32(i32 %tmp15, i32 %groupMax.02) nounwind
  %tmp19 = add i32 %i.03, 1
  %cmp10 = icmp ult i32 %tmp19, %5
  br i1 %cmp10, label %for.body, label %for.exit
}

declare  void @barrier(i32, i32) nounwind

declare  <4 x i32> @__amdil_get_local_size_int() nounwind readonly

declare  <4 x i32> @__amdil_get_local_id_int() nounwind readonly

declare  i32 @__amdil_umax_u32(i32, i32) nounwind readonly

declare  <4 x i32> @__amdil_get_global_id_int() nounwind readonly
