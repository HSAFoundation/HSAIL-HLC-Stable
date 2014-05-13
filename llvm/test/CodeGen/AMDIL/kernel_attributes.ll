; RUN: llc -march=amdil < %s | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"
@.str = internal addrspace(2) constant [7 x i8] c"float*\00"
@.str1 = internal addrspace(2) constant [7 x i8] c"float*\00"
@.str2 = internal addrspace(2) constant [4 x i8] c"Res\00"
@.str3 = internal addrspace(2) constant [5 x i8] c"int*\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_test_kernel = global [1 x i8*] [i8* bitcast ([4 x i8] addrspace(2)* @.str2 to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_test_kernel = global [3 x i8*] [i8* bitcast ([7 x i8] addrspace(2)* @.str to i8*), i8* bitcast ([7 x i8] addrspace(2)* @.str1 to i8*), i8* bitcast ([5 x i8] addrspace(2)* @.str3 to i8*)], section "llvm.metadata"
; CHECK: ;wsh:64:0:0
; CHECK: ;vth:float4
@sgv = internal addrspace(2) constant [19 x i8] c"WGH64,0,0VTHfloat4\00"
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, i32 addrspace(1)*)* @__OpenCL_test_kernel to i8*), i8* bitcast ([19 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define  void @__OpenCL_test_kernel(float addrspace(1)* nocapture %F1, float addrspace(1)* nocapture %F2, i32 addrspace(1)* nocapture %Res) nounwind {
entry:
  %call = tail call  i32 @get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float addrspace(1)* %F1, i32 %call
  %0 = load float addrspace(1)* %arrayidx, align 4, !tbaa !11
  %arrayidx1 = getelementptr inbounds float addrspace(1)* %F2, i32 %call
  %1 = load float addrspace(1)* %arrayidx1, align 4, !tbaa !11
  %call2 = tail call  i32 @__islessgreater_f32(float %0, float %1) nounwind readnone
  %arrayidx3 = getelementptr inbounds i32 addrspace(1)* %Res, i32 %call
  store i32 %call2, i32 addrspace(1)* %arrayidx3, align 4, !tbaa !14
  ret void
}

declare  i32 @get_global_id(i32) nounwind readnone

declare  i32 @__islessgreater_f32(float, float) nounwind readnone

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!8}
!opencl.ocl.version = !{!9}
!opencl.used.extensions = !{!10}
!opencl.used.optional.core.features = !{!10}
!opencl.compiler.options = !{!10}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32 addrspace(1)*)* @__OpenCL_test_kernel, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5, metadata !6, metadata !7}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 1, i32 1, i32 1}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"float*", metadata !"float*", metadata !"int*"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !"", metadata !""}
!5 = metadata !{metadata !"kernel_arg_name", metadata !"F1", metadata !"F2", metadata !"Res"}
!6 = metadata !{metadata !"work_group_size_hint", i32 64, i32 0, i32 0}
!7 = metadata !{metadata !"vec_type_hint", <4 x float> undef, i32 0}
!8 = metadata !{i32 1, i32 0}
!9 = metadata !{i32 1, i32 2}
!10 = metadata !{}
!11 = metadata !{metadata !"float", metadata !12}
!12 = metadata !{metadata !"omnipotent char", metadata !13}
!13 = metadata !{metadata !"Simple C/C++ TBAA"}
!14 = metadata !{metadata !"int", metadata !12}
