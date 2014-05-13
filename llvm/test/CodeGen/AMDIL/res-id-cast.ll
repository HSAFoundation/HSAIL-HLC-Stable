; RUN: llc -march=amdil -mcpu=tahiti -mattr=+macro-call <%s |FileCheck %s
; Verifies no assertion and correct resource id is declared

@.str = internal addrspace(2) constant [2 x i8] c"a\00"
@.str1 = internal addrspace(2) constant [5 x i8] c"int*\00"
@.str2 = internal addrspace(2) constant [2 x i8] c"b\00"
@.str3 = internal addrspace(2) constant [5 x i8] c"int*\00"
@.str4 = internal addrspace(2) constant [2 x i8] c"i\00"
@.str5 = internal addrspace(2) constant [4 x i8] c"int\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_foo_kernel = global [3 x i8*] [i8* bitcast ([2 x i8] addrspace(2)* @.str to i8*), i8* bitcast ([2 x i8] addrspace(2)* @.str2 to i8*), i8* bitcast ([2 x i8] addrspace(2)* @.str4 to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_foo_kernel = global [3 x i8*] [i8* bitcast ([5 x i8] addrspace(2)* @.str1 to i8*), i8* bitcast ([5 x i8] addrspace(2)* @.str3 to i8*), i8* bitcast ([4 x i8] addrspace(2)* @.str5 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @__OpenCL_foo_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

; CHECK: mdef({{[0-9]+}})_out(0)_in(1)_outline ; bar
; CHECK: dcl_typeless_uav_id([[RESID:[0-9]+]])_stride(4)_length(4)_access(read_write)
define internal fastcc void @bar(float addrspace(1)* nocapture %b, float addrspace(1)* nocapture %c, i32 %i) nounwind noinline {
entry:
  %arrayidx = getelementptr float addrspace(1)* %b, i32 %i
  store float 0x3FB99999A0000000, float addrspace(1)* %arrayidx, align 4
  %arrayidx4 = getelementptr float addrspace(1)* %c, i32 %i
  store float 0x3FC99999A0000000, float addrspace(1)* %arrayidx4, align 4
  ret void
}

; CHECK: mdef({{[0-9]+}})_out(0)_in(1)_outline ; foo
; CHECK: dcl_typeless_uav_id([[RESID]])_stride(4)_length(4)_access(read_write)
define void @__OpenCL_foo_kernel(i32 addrspace(1)* nocapture %a, i32 addrspace(1)* nocapture %b, i32 %i) nounwind {
entry:
  %conv = bitcast i32 addrspace(1)* %a to float addrspace(1)*
  %conv2 = bitcast i32 addrspace(1)* %b to float addrspace(1)*
  tail call fastcc void @bar(float addrspace(1)* %conv, float addrspace(1)* %conv2, i32 %i) nounwind
  ret void
}
