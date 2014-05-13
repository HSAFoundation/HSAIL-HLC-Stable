; RUN: llc -march=amdil -mcpu=cypress -o - %s | FileCheck %s
; RUN: llc -march=amdil -mcpu=tahiti -o - %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

@.str = internal addrspace(2) constant [5 x i8] c"%4s\0A\00"
@.str1 = internal addrspace(2) constant [4 x i8] c"foo\00"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void ()* @__OpenCL_load_const_phi_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define  void @__OpenCL_load_const_phi_kernel() nounwind {
entry:
  br label %loop

; Make sure we have a valid UAV id after encountering a phi to the
; const load

; CHECK-NOT: uav_raw_load_id(0)
loop:
  %tmp40 = phi i8 addrspace(2)* [ getelementptr inbounds ([4 x i8] addrspace(2)* @.str1, i32 0, i32 0), %entry ], [ %tmp41, %loop ]
  %tmp41 = getelementptr i8 addrspace(2)* %tmp40, i32 1
  %tmp42 = load i8 addrspace(2)* %tmp41, align 1
  %tmp43 = icmp eq i8 %tmp42, 0
  br i1 %tmp43, label %exit, label %loop

exit:
  ret void
}
