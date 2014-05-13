; RUN: llc -march=amdil -mcpu=tahiti <%s |FileCheck %s
; test the right register is used

target datalayout = "e-p:32:32:32-S0-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f16:16:16-f32:32:32-f64:64:64-f128:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v96:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-v2048:2048:2048"
target triple = "amdil-pc-unknown-amdopencl"

@.str = internal addrspace(2) constant [5 x i8] c"dest\00"
@.str1 = internal addrspace(2) constant [6 x i8] c"long*\00"
@.str2 = internal addrspace(2) constant [5 x i8] c"src1\00"
@.str3 = internal addrspace(2) constant [6 x i8] c"long*\00"
@.str4 = internal addrspace(2) constant [5 x i8] c"src2\00"
@.str5 = internal addrspace(2) constant [6 x i8] c"long*\00"
@.str6 = internal addrspace(2) constant [4 x i8] c"cmp\00"
@.str7 = internal addrspace(2) constant [6 x i8] c"long*\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_select_long3_long3_kernel = global [4 x i8*] [i8* bitcast ([5 x i8] addrspace(2)* @.str to i8*), i8* bitcast ([5 x i8] addrspace(2)* @.str2 to i8*), i8* bitcast ([5 x i8] addrspace(2)* @.str4 to i8*), i8* bitcast ([4 x i8] addrspace(2)* @.str6 to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_select_long3_long3_kernel = global [4 x i8*] [i8* bitcast ([6 x i8] addrspace(2)* @.str1 to i8*), i8* bitcast ([6 x i8] addrspace(2)* @.str3 to i8*), i8* bitcast ([6 x i8] addrspace(2)* @.str5 to i8*), i8* bitcast ([6 x i8] addrspace(2)* @.str7 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (i64 addrspace(1)*, i64 addrspace(1)*, i64 addrspace(1)*, i64 addrspace(1)*)* @__OpenCL_select_long3_long3_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define void @__OpenCL_select_long3_long3_kernel(i64 addrspace(1)* nocapture %dest, i64 addrspace(1)* nocapture %src1, i64 addrspace(1)* nocapture %src2, i64 addrspace(1)* nocapture %cmp) nounwind {
  %a.sroa.0 = alloca <4 x i64>, align 32
  %c.sroa.0 = alloca <4 x i64>, align 32
  %1 = call i32 @get_global_id(i32 0) nounwind
  %2 = and i32 %1, 1
  %3 = icmp eq i32 %2, 0
  br i1 %3, label %._crit_edge, label %4

._crit_edge:                                      ; preds = %0
  %a.sroa.0.0.cast29.pre = bitcast <4 x i64>* %a.sroa.0 to <3 x i64>*
  br label %7

; <label>:4                                       ; preds = %0
  %5 = zext i32 %1 to i64
  %a.sroa.0.0.cast32 = bitcast <4 x i64>* %a.sroa.0 to <3 x i64>*
  %6 = insertelement <3 x i64> undef, i64 %5, i32 1
  store <3 x i64> %6, <3 x i64>* %a.sroa.0.0.cast32, align 32
  br label %7

; <label>:7                                       ; preds = %4, %._crit_edge
  %a.sroa.0.0.cast29.pre-phi = phi <3 x i64>* [ %a.sroa.0.0.cast29.pre, %._crit_edge ], [ %a.sroa.0.0.cast32, %4 ]
  %a.sroa.0.0.load30 = phi <3 x i64> [ undef, %._crit_edge ], [ %6, %4 ]
;CHECK: ior
;CHECK-NOT: mov
;CHECK: uav_raw_store_id
  %8 = insertelement <3 x i64> %a.sroa.0.0.load30, i64 0, i32 0
  store <3 x i64> %8, <3 x i64>* %a.sroa.0.0.cast29.pre-phi, align 32
  %c.sroa.0.0.cast25 = bitcast <4 x i64>* %c.sroa.0 to <3 x i64>*
  store <3 x i64> <i64 0, i64 undef, i64 undef>, <3 x i64>* %c.sroa.0.0.cast25, align 32
  %a.sroa.0.0.load = load <4 x i64>* %a.sroa.0, align 32
  %9 = shufflevector <4 x i64> %a.sroa.0.0.load, <4 x i64> undef, <3 x i32> <i32 0, i32 undef, i32 undef>
  %c.sroa.0.0.load = load <4 x i64>* %c.sroa.0, align 32
  %10 = shufflevector <4 x i64> %c.sroa.0.0.load, <4 x i64> undef, <3 x i32> <i32 0, i32 undef, i32 undef>
  %11 = and <3 x i64> %9, %10
  %12 = extractelement <3 x i64> %11, i32 0
  %13 = getelementptr inbounds i64 addrspace(1)* %dest, i32 %1
  store i64 %12, i64 addrspace(1)* %13, align 8, !tbaa !6
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}

!0 = metadata !{void (i64 addrspace(1)*, i64 addrspace(1)*, i64 addrspace(1)*, i64 addrspace(1)*)* @__OpenCL_select_long3_long3_kernel, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 1, i32 1, i32 1, i32 1}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"long*", metadata !"long*", metadata !"long*", metadata !"long*"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !"", metadata !"", metadata !""}
!5 = metadata !{metadata !"kernel_arg_name", metadata !"dest", metadata !"src1", metadata !"src2", metadata !"cmp"}
!6 = metadata !{metadata !"long", metadata !7}
!7 = metadata !{metadata !"omnipotent char", metadata !8}
!8 = metadata !{metadata !"Simple C/C++ TBAA"}
