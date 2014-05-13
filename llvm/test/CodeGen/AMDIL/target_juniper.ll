; RUN: llc < %s -march=amdil -mcpu=juniper | FileCheck %s
; CHECK: ;device:juniper
; ModuleID = '_temp_0_cypress_optimized.bc'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:32:32-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64"
target triple = "amdil-pc-amdopencl"

%0 = type { i8*, i8*, i8*, i8*, i32 }

@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x %0] [%0 { i8* bitcast (void ()* @__OpenCL_foo_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"

define void @__OpenCL_foo_kernel() nounwind readnone {
entry:
  ret void
}
