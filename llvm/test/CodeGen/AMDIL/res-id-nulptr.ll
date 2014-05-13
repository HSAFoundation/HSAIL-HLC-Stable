; RUN: llc -march=amdil -mcpu=tahiti -mattr=+macro-call <%s |FileCheck %s
; Verifies no assertion and correct resource id is declared

@.str = internal addrspace(2) constant [2 x i8] c"a\00"
@.str1 = internal addrspace(2) constant [5 x i8] c"int*\00"
@.str2 = internal addrspace(2) constant [2 x i8] c"b\00"
@.str3 = internal addrspace(2) constant [5 x i8] c"int*\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_foo_kernel = global [2 x i8*] [i8* bitcast ([2 x i8] addrspace(2)* @.str to i8*), i8* bitcast ([2 x i8] addrspace(2)* @.str2 to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_foo_kernel = global [2 x i8*] [i8* bitcast ([5 x i8] addrspace(2)* @.str1 to i8*), i8* bitcast ([5 x i8] addrspace(2)* @.str3 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*)* @__OpenCL_foo_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

; CHECK: mdef({{[0-9]+}})_out(0)_in(1)_outline ; bar
; CHECK: dcl_typeless_uav_id([[RESID:[0-9]+]])_stride(4)_length(4)_access(read_write)
define internal fastcc void @bar(i32 addrspace(1)* %b, i32 addrspace(1)* nocapture %c) nounwind noinline {
entry:
  %cmp = icmp eq i32 addrspace(1)* %b, null
  %c.b = select i1 %cmp, i32 addrspace(1)* %c, i32 addrspace(1)* %b
  store i32 1, i32 addrspace(1)* %c.b, align 4
  ret void
}

; CHECK: mdef({{[0-9]+}})_out(0)_in(1)_outline ; foo
; CHECK: dcl_typeless_uav_id([[RESID]])_stride(4)_length(4)_access(read_write)
define void @__OpenCL_foo_kernel(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* %b) nounwind {
entry:
  tail call fastcc void @bar(i32 addrspace(1)* null, i32 addrspace(1)* %a) nounwind
  tail call fastcc void @bar(i32 addrspace(1)* %b, i32 addrspace(1)* %a) nounwind
  ret void
}
