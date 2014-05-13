; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s
; CHECK: ;#DATASTART:32
; CHECK: ;#i32:0:8:1:2:3:0:1:2:3:0
; CHECK: ;#DATAEND
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-unknown-amdopencl"

@st = internal addrspace(2) unnamed_addr constant [2 x <3 x i32>] [<3 x i32> <i32 1, i32 2, i32 3>, <3 x i32> <i32 1, i32 2, i32 3>], align 16
@.str = internal addrspace(2) constant [4 x i8] c"out\00"
@.str1 = internal addrspace(2) constant [6 x i8] c"int3*\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_test_kernel = global [1 x i8*] [i8* bitcast ([4 x i8] addrspace(2)* @.str to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_test_kernel = global [1 x i8*] [i8* bitcast ([6 x i8] addrspace(2)* @.str1 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (<3 x i32> addrspace(1)*)* @__OpenCL_test_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define void @__OpenCL_test_kernel(<3 x i32> addrspace(1)* nocapture %out) nounwind {
entry:
  %0 = tail call <4 x i32> @__amdil_get_global_id_int() nounwind
  %1 = extractelement <4 x i32> %0, i32 0
  %arrayidx2 = getelementptr [2 x <3 x i32>] addrspace(2)* @st, i32 0, i32 %1
  %tmp3 = load <3 x i32> addrspace(2)* %arrayidx2, align 16
  store <3 x i32> %tmp3, <3 x i32> addrspace(1)* %out, align 16
  ret void
}

declare <4 x i32> @__amdil_get_global_id_int() nounwind readonly
