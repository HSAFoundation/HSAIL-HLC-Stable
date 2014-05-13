; RUN: llc -march=amdil -mcpu=tahiti <%s
; test case for pointer manager, where args include vec3
; No CHECKs. The test is just checking for crashes in the compiler
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

%struct._image2d_t = type opaque

@.str = internal addrspace(2) constant [7 x i8] c"float3\00"
@.str1 = internal addrspace(2) constant [7 x i8] c"float3\00"
@.str2 = internal addrspace(2) constant [7 x i8] c"float3\00"
@.str3 = internal addrspace(2) constant [7 x i8] c"float3\00"
@.str4 = internal addrspace(2) constant [7 x i8] c"float4\00"
@.str5 = internal addrspace(2) constant [9 x i8] c"dstImage\00"
@.str6 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str7 = internal addrspace(2) constant [6 x i8] c"float\00"
@llvm.image.annotations.__OpenCL_Render_kernel = global [1 x <{ i8*, i32 }>] [<{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str5 to i8*), i32 2 }>], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_Render_kernel = global [7 x i8*] [i8* bitcast ([7 x i8] addrspace(2)* @.str to i8*), i8* bitcast ([7 x i8] addrspace(2)* @.str1 to i8*), i8* bitcast ([7 x i8] addrspace(2)* @.str2 to i8*), i8* bitcast ([7 x i8] addrspace(2)* @.str3 to i8*), i8* bitcast ([7 x i8] addrspace(2)* @.str4 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str6 to i8*), i8* bitcast ([6 x i8] addrspace(2)* @.str7 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (<3 x float>, <3 x float>, <3 x float>, <3 x float>, <4 x float>, %struct._image2d_t addrspace(1)*, float)* @__OpenCL_Render_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define  void @__OpenCL_Render_kernel(<3 x float> %cameraPos, <3 x float> %cameraDir, <3 x float> %cameraUp, <3 x float> %cameraRight, <4 x float> %imageToRay, %struct._image2d_t addrspace(1)* %dstImage, float %time) nounwind {
entry:
  %0 = tail call  <4 x i32> @__amdil_get_global_id_int() nounwind
  %1 = extractelement <4 x i32> %0, i32 0
  %2 = extractelement <4 x i32> %0, i32 1
  %tmp4 = insertelement <2 x i32> undef, i32 %1, i32 0
  %tmp7 = insertelement <2 x i32> %tmp4, i32 %2, i32 1
  tail call  void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstImage, <2 x i32> %tmp7, <4 x i32> <i32 0, i32 1036831949, i32 1045220557, i32 1050253722>) nounwind
  ret void
}

declare  <4 x i32> @__amdil_get_global_id_int() nounwind readonly

declare  void @__amdil_image2d_write(%struct._image2d_t addrspace(1)*, <2 x i32>, <4 x i32>) nounwind
