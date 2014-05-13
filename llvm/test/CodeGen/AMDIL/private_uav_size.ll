; RUN: llc -march=amdil -mcpu=tahiti -mattr=+macro-call <%s |FileCheck %s
; Verifies that private uav size is 400 bytes.

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-unknown-amdopencl"

@.str = internal addrspace(2) constant [4 x i8] c"out\00"
@.str1 = internal addrspace(2) constant [5 x i8] c"int*\00"
@.str2 = internal addrspace(2) constant [2 x i8] c"x\00"
@.str3 = internal addrspace(2) constant [4 x i8] c"int\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_foo_kernel = global [2 x i8*] [i8* bitcast ([4 x i8] addrspace(2)* @.str to i8*), i8* bitcast ([2 x i8] addrspace(2)* @.str2 to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_foo_kernel = global [2 x i8*] [i8* bitcast ([5 x i8] addrspace(2)* @.str1 to i8*), i8* bitcast ([4 x i8] addrspace(2)* @.str3 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (i32 addrspace(1)*, i32)* @__OpenCL_foo_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define void @__OpenCL_foo_kernel(i32 addrspace(1)* nocapture %out, i32 %x) nounwind {
entry:
; CHECK: dcl_typeless_uav_id(8)_stride(4)_length(400)_access(private)
; CHECK: ;memory:uavprivate:400
  %a = alloca [100 x i32], align 4
  %cmp1 = icmp sgt i32 %x, 0
  br i1 %cmp1, label %for.body, label %for.exit

for.exit:                                         ; preds = %for.body, %entry
  %b.0.lcssa = phi i32 [ 0, %entry ], [ %tmp5, %for.body ]
  store i32 %b.0.lcssa, i32 addrspace(1)* %out, align 4
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.03 = phi i32 [ %tmp7, %for.body ], [ 0, %entry ]
  %b.02 = phi i32 [ %tmp5, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr [100 x i32]* %a, i32 0, i32 %i.03
  %tmp4 = load i32* %arrayidx, align 4
  %tmp5 = add nsw i32 %tmp4, %b.02
  %tmp7 = add nsw i32 %i.03, 1
  %cmp = icmp slt i32 %tmp7, %x
  br i1 %cmp, label %for.body, label %for.exit
}
