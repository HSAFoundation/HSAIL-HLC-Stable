; RUN: llc -march=amdil -mcpu=cypress < %s | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

%struct._image2d_array_t = type opaque

@.str = internal addrspace(2) constant [4 x i8] c"src\00"
@.str1 = internal addrspace(2) constant [16 x i8] c"image2d_array_t\00"
@.str2 = internal addrspace(2) constant [7 x i8] c"uint4*\00"
@llvm.image.annotations.__OpenCL_test_constant_sampler = global [1 x <{ i8*, i32 }>] [<{ i8*, i32 }> <{ i8* bitcast ([4 x i8] addrspace(2)* @.str to i8*), i32 1 }>], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_test_constant_sampler = global [2 x i8*] [i8* bitcast ([16 x i8] addrspace(2)* @.str1 to i8*), i8* bitcast ([7 x i8] addrspace(2)* @.str2 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (%struct._image2d_array_t addrspace(1)*, <4 x i32> addrspace(1)*)* @__OpenCL_test_constant_sampler to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define void @__OpenCL_test_constant_sampler(%struct._image2d_array_t addrspace(1)* %src, <4 x i32> addrspace(1)* nocapture %dst) nounwind {
entry:
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %dst, i32 0
; CHECK: sample_id(0)_sampler(0)_coordtype(unnormalized) r{{[0-9]+}}, r{{[0-9]+}}

  %0 = tail call <4 x i32> @__amdil_image2d_array_read_unnorm(%struct._image2d_array_t addrspace(1)* %src, i32 22, <4 x float> <float 0.000000e+00, float 0.000000e+00, float 5.000000e-01, float 0.000000e+00>) nounwind
  store <4 x i32> %0, <4 x i32> addrspace(1)* %arrayidx, align 16
  ret void
}

declare <4 x i32> @__amdil_image2d_array_read_unnorm(%struct._image2d_array_t addrspace(1)*, i32, <4 x float>) nounwind readonly
