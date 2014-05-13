; RUN: llc -march=amdil < %s

@llvm.signedOrSignedpointee.annotations.__OpenCL_foo_kernel = external global [2 x i8*], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_foo_kernel = external global [7 x i8*], section "llvm.metadata"
@sgv = external addrspace(2) constant [1 x i8]
@fgv = external addrspace(2) constant [1 x i8]
@lvgv = external constant [0 x i8*]
@rvgv = external constant [0 x i8*]
@llvm.global.annotations = external global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }], section "llvm.metadata"
@__tbl_M32_LOGE = external addrspace(2) unnamed_addr constant [129 x <2 x float>], align 8
@__tbl_M32_LOG_INV = external addrspace(2) unnamed_addr constant [129 x float], align 4

define void @__OpenCL_foo_kernel() {
  ret void
}
