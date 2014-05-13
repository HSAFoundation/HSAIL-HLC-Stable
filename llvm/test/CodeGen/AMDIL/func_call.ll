;RUN: llc < %s -march=amdil | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

@.str = internal addrspace(2) constant [7 x i8] c"float*\00"
@.str1 = internal addrspace(2) constant [7 x i8] c"float*\00"
@llvm.argtypename.annotations.__OpenCL_helloworld_kernel = global [2 x i8*] [i8* bitcast ([7 x i8] addrspace(2)* @.str to i8*), i8* bitcast ([7 x i8] addrspace(2)* @.str1 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (float addrspace(1)*, float addrspace(1)*)* @__OpenCL_helloworld_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define  float @square(float %x) nounwind noinline {
; CHECK: mul_ieee r33.x___, r1.x, r1.x
; CHECK: uav_raw_store_id(8) mem0.x___, r1029, r33.x
; CHECK: uav_raw_load_id(8) r33.x___, r1029
entry:
  %retval = alloca float, align 4
  %x.addr = alloca float, align 4
  store float %x, float* %x.addr, align 4
  %tmp = load float* %x.addr, align 4
  %tmp1 = load float* %x.addr, align 4
  %tmp2 = fmul float %tmp, %tmp1
  store float %tmp2, float* %retval, align 4
  br label %return

return:                                           ; preds = %entry
  %tmp3 = load float* %retval, align 4
  ret float %tmp3
}

define  void @__OpenCL_helloworld_kernel(float addrspace(1)* %in, float addrspace(1)* %out) nounwind {
entry:
  %in.addr = alloca float addrspace(1)*, align 4
  %out.addr = alloca float addrspace(1)*, align 4
  %num = alloca i32, align 4
  store float addrspace(1)* %in, float addrspace(1)** %in.addr, align 4
  store float addrspace(1)* %out, float addrspace(1)** %out.addr, align 4
  %call = call  i32 @get_global_id(i32 0) nounwind
  store i32 %call, i32* %num, align 4
  %tmp = load float addrspace(1)** %out.addr, align 4
  %tmp1 = load i32* %num, align 4
  %arrayidx = getelementptr float addrspace(1)* %tmp, i32 %tmp1
  %tmp2 = load float addrspace(1)** %in.addr, align 4
  %tmp3 = load i32* %num, align 4
  %arrayidx4 = getelementptr float addrspace(1)* %tmp2, i32 %tmp3
  %tmp5 = load float addrspace(1)* %arrayidx4, align 4
  %call6 = call  float @square(float %tmp5) nounwind
  store float %call6, float addrspace(1)* %arrayidx, align 4
  br label %return

return:                                           ; preds = %entry
  ret void
}

declare  i32 @get_global_id(i32) nounwind
