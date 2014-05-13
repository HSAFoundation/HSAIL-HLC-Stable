; RUN: llc -O0 -march=amdil -mcpu=tahiti <%s |FileCheck %s
; RUN: llc -march=amdil -mcpu=tahiti <%s |FileCheck %s
; test case for fixing the hang that the peephole optimization that deals
; with null pointer caused
; CHECK: mov
; CHECK: uav_raw_store
target triple = "amdil-pc-amdopencl"

@.str = internal addrspace(2) constant [4 x i8] c"out\00"
@.str1 = internal addrspace(2) constant [5 x i8] c"int*\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_test_kernel = global [1 x i8*] [i8* bitcast ([4 x i8] addrspace(2)* @.str to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_test_kernel = global [1 x i8*] [i8* bitcast ([5 x i8] addrspace(2)* @.str1 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (i32 addrspace(1)*)* @__OpenCL_test_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define  void @__OpenCL_test_kernel(i32 addrspace(1)* %out) nounwind {
entry:
  %arrayidx = getelementptr i32 addrspace(1)* %out, i32 0
  %tmp1 = load i32 addrspace(1)* null, align 4
  store i32 %tmp1, i32 addrspace(1)* %arrayidx, align 4
  br label %return

return:                                           ; preds = %entry
  ret void
}
