; RUN: llc -march=amdil -mcpu=tahiti < %s |FileCheck %s
;CHECK: dcl_typeless_uav_id(11)_stride(4)_length(4)_access(read_write)
;CHECK: dcl_typeless_uav_id(10)_stride(4)_length(4)_access(read_only)
;CHECK: dcl_typeless_uav_id(12)_stride(4)_length(4)_access(read_write)
;CHECK: ;ARGSTART
;CHECK: ;pointer:out:i32:1:1:0:uav:12:4:RW:0:0
;CHECK: ;uavid:11
;CHECK: ;cbid:10
;CHECK: ;ARGEND
;CHECK: func
;CHECK: uav_raw_load_id(10)
;CHECK: uav_raw_store_id(12)
;CHECK: uav_raw_load_id(10)
;CHECK: uav_raw_store_id(12)
;CHECK: endfunc

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

@outValues = internal addrspace(2) unnamed_addr constant [16 x i32] [i32 17, i32 1, i32 11, i32 12, i32 1955, i32 11, i32 5, i32 1985, i32 113, i32 1, i32 24, i32 1984, i32 7, i32 23, i32 1979, i32 97], align 4
@outValues2 = internal addrspace(2) unnamed_addr constant [16 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16], align 4
@.str = internal addrspace(2) constant [4 x i8] c"out\00"
@.str1 = internal addrspace(2) constant [5 x i8] c"int*\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_constant_kernel_kernel = global [1 x i8*] [i8* bitcast ([4 x i8] addrspace(2)* @.str to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_constant_kernel_kernel = global [1 x i8*] [i8* bitcast ([5 x i8] addrspace(2)* @.str1 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (i32 addrspace(1)*)* @__OpenCL_constant_kernel_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define  void @__OpenCL_constant_kernel_kernel(i32 addrspace(1)* noalias %out) nounwind {
entry:
  %0 = tail call  <4 x i32> @__amdil_get_global_id_int() nounwind
  %1 = extractelement <4 x i32> %0, i32 0
  %arrayidx = getelementptr i32 addrspace(1)* %out, i32 %1
  %arrayidx3 = getelementptr [16 x i32] addrspace(2)* @outValues, i32 0, i32 %1
  %tmp4 = load i32 addrspace(2)* %arrayidx3, align 4
  store i32 %tmp4, i32 addrspace(1)* %arrayidx, align 4
  %tmp7 = add nsw i32 %1, 1
  %arrayidx8 = getelementptr i32 addrspace(1)* %out, i32 %tmp7
  %arrayidx10 = getelementptr [16 x i32] addrspace(2)* @outValues2, i32 0, i32 %1
  %tmp11 = load i32 addrspace(2)* %arrayidx10, align 4
  store i32 %tmp11, i32 addrspace(1)* %arrayidx8, align 4
  ret void
}

declare  <4 x i32> @__amdil_get_global_id_int() nounwind readonly
