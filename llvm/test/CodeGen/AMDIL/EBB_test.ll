; RUN: llc -march=amdil -mcpu=tahiti <%s |FileCheck %s

; CHECK: func {{[0-9]+}} ; binomial_options {{ *}}; @__OpenCL_binomial_options_kernel
; CHECK: mov r1, r{{[0-9]+}}
; CHECK: call {{[0-9]+}} ; myexp
; CHECK: mov r{{[0-9]+}}, r1
; CHECK: mov r1, r{{[0-9]+}}
; CHECK: call {{[0-9]+}} ; myexp
; CHECK: abs r{{[0-9]+}}, r1

; ModuleID = '_temp_0_Tahiti_optimized.bc'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

@.str = internal addrspace(2) constant [9 x i8] c"numSteps\00"
@.str1 = internal addrspace(2) constant [4 x i8] c"int\00"
@.str2 = internal addrspace(2) constant [8 x i8] c"float4*\00"
@.str3 = internal addrspace(2) constant [10 x i8] c"randArray\00"
@.str4 = internal addrspace(2) constant [8 x i8] c"float4*\00"
@.str5 = internal addrspace(2) constant [8 x i8] c"float4*\00"
@.str6 = internal addrspace(2) constant [8 x i8] c"float4*\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_binomial_options_kernel = global [1 x i8*] [i8* bitcast ([9 x i8] addrspace(2)* @.str to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_binomial_options_kernel = global [5 x i8*] [i8* bitcast ([4 x i8] addrspace(2)* @.str1 to i8*), i8* bitcast ([8 x i8] addrspace(2)* @.str2 to i8*), i8* bitcast ([8 x i8] addrspace(2)* @.str4 to i8*), i8* bitcast ([8 x i8] addrspace(2)* @.str5 to i8*), i8* bitcast ([8 x i8] addrspace(2)* @.str6 to i8*)], section "llvm.metadata"
@llvm.argtypeconst.annotations.__OpenCL_binomial_options_kernel = global [1 x i8*] [i8* bitcast ([10 x i8] addrspace(2)* @.str3 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (i32, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(3)*, <4 x float> addrspace(3)*)* @__OpenCL_binomial_options_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define internal fastcc <4 x float> @myexp(<4 x float> %x) nounwind readnone noinline {
entry:
  %tmp2 = fmul <4 x float> %x, %x
  ret <4 x float> %tmp2
}

define  void @__OpenCL_binomial_options_kernel(i32 %numSteps, <4 x float> addrspace(1)* noalias nocapture %randArray, <4 x float> addrspace(1)* nocapture %output, <4 x float> addrspace(3)* nocapture %callA, <4 x float> addrspace(3)* nocapture %callB) nounwind {
entry:
  %0 = tail call  <4 x i32> @__amdil_get_local_id_int() nounwind
  %1 = extractelement <4 x i32> %0, i32 0
  %2 = tail call  <4 x i32> @__amdil_get_group_id_int() nounwind
  %3 = extractelement <4 x i32> %2, i32 0
  %arrayidx = getelementptr <4 x float> addrspace(1)* %randArray, i32 %3
  %tmp3 = load <4 x float> addrspace(1)* %arrayidx, align 16
  %tmp5 = fsub <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %tmp3
  %tmp18 = fmul <4 x float> %tmp5, <float 2.500000e-01, float 2.500000e-01, float 2.500000e-01, float 2.500000e-01>
  %tmp20 = fmul <4 x float> %tmp3, <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>
  %tmp21 = fadd <4 x float> %tmp18, %tmp20
  %conv = sitofp i32 %numSteps to float
  %tmp24 = fdiv float 1.000000e+00, %conv
  %conv25 = insertelement <4 x float> undef, float %tmp24, i32 0
  %conv26 = insertelement <4 x float> %conv25, float %tmp24, i32 1
  %conv27 = insertelement <4 x float> %conv26, float %tmp24, i32 2
  %conv28 = insertelement <4 x float> %conv27, float %tmp24, i32 3
  %tmp29 = fmul <4 x float> %tmp21, %conv28
  %4 = tail call  <4 x float> @__amdil_sqrt_vec_v4f32(<4 x float> %tmp29) nounwind
  %tmp32 = fmul <4 x float> %4, <float 0x3FD3333340000000, float 0x3FD3333340000000, float 0x3FD3333340000000, float 0x3FD3333340000000>
  %tmp34 = fmul <4 x float> %tmp29, <float 0x3F947AE140000000, float 0x3F947AE140000000, float 0x3F947AE140000000, float 0x3F947AE140000000>
  %call36 = tail call fastcc <4 x float> @myexp(<4 x float> %tmp34) nounwind
  %call40 = tail call fastcc <4 x float> @myexp(<4 x float> %tmp32) nounwind
  %tmp42 = fdiv <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %call40
  %cmp = icmp eq i32 %1, 0
  br i1 %cmp, label %if.then, label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void

if.then:                                          ; preds = %entry
  %tmp48 = fsub <4 x float> %call40, %tmp42
  %tmp45 = fsub <4 x float> %call36, %tmp42
  %tmp49 = fdiv <4 x float> %tmp45, %tmp48
  %tmp38 = fdiv <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %call36
  %tmp54 = fmul <4 x float> %tmp49, %tmp38
  %arrayidx61 = getelementptr <4 x float> addrspace(1)* %output, i32 %3
  store <4 x float> %tmp54, <4 x float> addrspace(1)* %arrayidx61, align 16
  br label %if.end
}

declare  <4 x i32> @__amdil_get_local_id_int() nounwind readonly

declare  <4 x float> @__amdil_sqrt_vec_v4f32(<4 x float>) nounwind readonly

declare  <4 x i32> @__amdil_get_group_id_int() nounwind readonly

