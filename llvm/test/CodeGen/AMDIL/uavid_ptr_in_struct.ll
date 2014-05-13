; RUN: llc -march=amdil -mcpu=tahiti -mattr="+noalias" <%s |FileCheck %s

; Test the case where a kernel arg is a pointer to a struct, and the struct
; contains a pointer. So the pointer may alias another pointer kernel arg.
; So conservatively assign all kernel args to the default uav except those
; that have noalias attributes.

; CHECK: dcl_typeless_uav_id(11)_stride(4)_length(4)_access(read_write)
; CHECK: ;pointer:myStruct:struct:1:1:0:uav:11:16:RW:0:0
; CHECK: ;pointer:arrB:i32:1:1:16:uav:11:4:RW:0:0
; CHECK: ;pointer:arrC:i32:1:1:32:uav:12:4:RW:0:0
; CHECK: ;uavid:11
; CHECK: uav_raw_load_id(11)_cached
; CHECK: uav_raw_store_id(11)
; CHECK: uav_raw_store_id(11)
; CHECK: uav_raw_store_id(12)

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

%0 = type { %1, i32 }
%1 = type { i32 addrspace(1)*, i32 }

@.str = internal addrspace(2) constant [12 x i8] c"MyStructTy*\00"
@.str1 = internal addrspace(2) constant [5 x i8] c"arrB\00"
@.str2 = internal addrspace(2) constant [5 x i8] c"int*\00"
@.str3 = internal addrspace(2) constant [5 x i8] c"arrC\00"
@.str4 = internal addrspace(2) constant [5 x i8] c"int*\00"
@.str5 = internal addrspace(2) constant [12 x i8] c"initialSize\00"
@.str6 = internal addrspace(2) constant [4 x i8] c"int\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_initizeWSDeque_kernel = global [3 x i8*] [i8* bitcast ([5 x i8] addrspace(2)* @.str1 to i8*), i8* bitcast ([5 x i8] addrspace(2)* @.str3 to i8*), i8* bitcast ([12 x i8] addrspace(2)* @.str5 to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_initizeWSDeque_kernel = global [4 x i8*] [i8* bitcast ([12 x i8] addrspace(2)* @.str to i8*), i8* bitcast ([5 x i8] addrspace(2)* @.str2 to i8*), i8* bitcast ([5 x i8] addrspace(2)* @.str4 to i8*), i8* bitcast ([4 x i8] addrspace(2)* @.str6 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (%0 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32)* @__OpenCL_initizeWSDeque_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define  void @__OpenCL_initizeWSDeque_kernel(%0 addrspace(1)* %myStruct, i32 addrspace(1)* %arrB, i32 addrspace(1)* noalias %arrC, i32 %initialSize) nounwind {
entry:
  %structele1 = getelementptr inbounds %0 addrspace(1)* %myStruct, i32 0, i32 0, i32 0
  %tmp2 = load i32 addrspace(1)* addrspace(1)* %structele1, align 4
  store i32 1, i32 addrspace(1)* %tmp2, align 4
  store i32 1, i32 addrspace(1)* %arrB, align 4
  store i32 1, i32 addrspace(1)* %arrC, align 4
  ret void
}
