; Generic address 32-bit address support (Bug 9683).
;
; RUN: llc -march=hsail -filetype=asm -o - %s | FileCheck %s


%0 = type { i32, i32, i32, [116 x i8], [1 x i8] }
%struct.TestStruct = type { i8, i32 }
%opencl.pipe_t = type { i32, i32, i32, [116 x i8], [1 x i8] }

; Check 32-bit stof/flat_ld and stof/flat_st pairs.
;
; CHECK-LABEL: @__OpenCL_test_pipe_write_struct_kernel
; CHECK: stof_global_u32_u32  $s[[LDNUM:[0-9]+]], $s{{[0-9]+}};
; CHECK: stof_global_u32_u32  $s[[STNUM:[0-9]+]], $s{{[0-9]+}};
; CHECK: ld_align(4)_u32 $s{{[0-9]+}}, [$s[[LDNUM]]];
; CHECK: st_align(4)_u32 $s{{[0-9]+}}, [$s[[STNUM]]];
define spir_kernel void @__OpenCL_test_pipe_write_struct_kernel(%struct.TestStruct addrspace(1)* %src, %opencl.pipe_t addrspace(1)* %out_pipe) nounwind {
entry:
  %0 = alloca i32, align 4
  %1 = getelementptr inbounds %opencl.pipe_t addrspace(1)* %out_pipe, i32 0, i32 0
  %2 = load atomic volatile i32 addrspace(1)* %1 monotonic, align 4, !mem.scope !16
  %3 = getelementptr inbounds %opencl.pipe_t addrspace(1)* %out_pipe, i32 0, i32 2
  %4 = load i32 addrspace(1)* %3, align 4
  %5 = getelementptr inbounds %opencl.pipe_t addrspace(1)* %out_pipe, i32 0, i32 1
  %6 = add i32 %4, %2
  %7 = bitcast i32* %0 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %7) nounwind
  %8 = load atomic volatile i32 addrspace(1)* %5 acquire, align 4, !mem.scope !16
  store i32 %8, i32* %0, align 4
  br label %9

; <label>:9                                       ; preds = %13, %entry
  %10 = phi i32 [ %15, %13 ], [ %8, %entry ]
  %11 = add i32 %10, 1
  %12 = icmp ugt i32 %11, %6
  br i1 %12, label %if.end, label %13

; <label>:13                                      ; preds = %9
  %14 = load volatile i32* %0, align 4
  %15 = cmpxchg i32 addrspace(1)* %5, i32 %14, i32 %11 acq_rel, !mem.scope !16
  store volatile i32 %15, i32* %0, align 4
  %16 = icmp eq i32 %15, %14
  br i1 %16, label %__reserve_write_pipe_internal_user.exit, label %9

__reserve_write_pipe_internal_user.exit:          ; preds = %13
  call void @llvm.lifetime.end(i64 -1, i8* %7) nounwind
  %17 = icmp eq i32 %14, -1
  br i1 %17, label %if.end, label %if.then

if.then:                                          ; preds = %__reserve_write_pipe_internal_user.exit
  %18 = call spir_func i32 @__hsail_get_global_id(i32 0) nounwind readnone
  %19 = getelementptr inbounds %struct.TestStruct addrspace(1)* %src, i32 %18, i32 0
  %20 = bitcast %opencl.pipe_t addrspace(1)* %out_pipe to %0 addrspace(1)*
  %21 = load i32 addrspace(1)* %3, align 4
  %22 = urem i32 %14, %21
  %23 = shl i32 %22, 3
  %24 = getelementptr inbounds %0 addrspace(1)* %20, i32 0, i32 4, i32 %23
  %25 = bitcast i8 addrspace(1)* %19 to i32 addrspace(4)*
  %26 = getelementptr inbounds i32 addrspace(4)* %25, i32 2
  %27 = bitcast i8 addrspace(1)* %24 to i32 addrspace(4)*
  br label %28

; <label>:28                                      ; preds = %28, %if.then
  %29 = phi i32 addrspace(4)* [ %27, %if.then ], [ %33, %28 ]
  %30 = phi i32 addrspace(4)* [ %25, %if.then ], [ %31, %28 ]
  %31 = getelementptr inbounds i32 addrspace(4)* %30, i32 1
  %32 = load i32 addrspace(4)* %30, align 4
  %33 = getelementptr inbounds i32 addrspace(4)* %29, i32 1
  store i32 %32, i32 addrspace(4)* %29, align 4
  %34 = icmp ult i32 addrspace(4)* %31, %26
  br i1 %34, label %28, label %if.end

if.end:                                           ; preds = %28, %__reserve_write_pipe_internal_user.exit, %9
  ret void
}

; Check 32-bit stof/flat_st and stof/flat_ld pairs.
;
; CHECK-LABEL: @__OpenCL_test_pipe_read_struct_kernel
; CHECK: stof_global_u32_u32  $s[[STNUM:[0-9]+]], $s{{[0-9]+}};
; CHECK: stof_global_u32_u32  $s[[LDNUM:[0-9]+]], $s{{[0-9]+}};
; CHECK: ld_align(4)_u32 $s{{[0-9]+}}, [$s[[LDNUM]]];
; CHECK: st_align(4)_u32 $s{{[0-9]+}}, [$s[[STNUM]]];
define spir_kernel void @__OpenCL_test_pipe_read_struct_kernel(%opencl.pipe_t addrspace(1)* %in_pipe, %struct.TestStruct addrspace(1)* nocapture %dst) nounwind {
entry:
  %0 = alloca i32, align 4
  %1 = getelementptr inbounds %opencl.pipe_t addrspace(1)* %in_pipe, i32 0, i32 1
  %2 = load atomic volatile i32 addrspace(1)* %1 monotonic, align 4, !mem.scope !16
  %3 = getelementptr inbounds %opencl.pipe_t addrspace(1)* %in_pipe, i32 0, i32 0
  %4 = bitcast i32* %0 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %4) nounwind
  %5 = load atomic volatile i32 addrspace(1)* %3 acquire, align 4, !mem.scope !16
  store i32 %5, i32* %0, align 4
  br label %6

; <label>:6                                       ; preds = %10, %entry
  %7 = phi i32 [ %12, %10 ], [ %5, %entry ]
  %8 = add i32 %7, 1
  %9 = icmp ugt i32 %8, %2
  br i1 %9, label %.exit.i, label %10

; <label>:10                                      ; preds = %6
  %11 = load volatile i32* %0, align 4
  %12 = cmpxchg i32 addrspace(1)* %3, i32 %11, i32 %8 acq_rel, !mem.scope !16
  store volatile i32 %12, i32* %0, align 4
  %13 = icmp eq i32 %12, %11
  br i1 %13, label %.exit.i, label %6

.exit.i:                                          ; preds = %10, %6
  %14 = phi i32 [ -1, %6 ], [ %11, %10 ]
  call void @llvm.lifetime.end(i64 -1, i8* %4) nounwind
  %15 = add i32 %14, 1
  %16 = icmp eq i32 %15, %2
  br i1 %16, label %17, label %__reserve_read_pipe_internal_user.exit

; <label>:17                                      ; preds = %.exit.i
  store atomic volatile i32 0, i32 addrspace(1)* %1 release, align 4, !mem.scope !16
  store atomic volatile i32 0, i32 addrspace(1)* %3 monotonic, align 4, !mem.scope !16
  br label %__reserve_read_pipe_internal_user.exit

__reserve_read_pipe_internal_user.exit:           ; preds = %17, %.exit.i
  %18 = icmp eq i32 %14, -1
  br i1 %18, label %if.end, label %if.then

if.then:                                          ; preds = %__reserve_read_pipe_internal_user.exit
  %19 = call spir_func i32 @__hsail_get_global_id(i32 0) nounwind readnone
  %20 = getelementptr inbounds %struct.TestStruct addrspace(1)* %dst, i32 %19, i32 0
  %21 = getelementptr inbounds %opencl.pipe_t addrspace(1)* %in_pipe, i32 0, i32 2
  %22 = load i32 addrspace(1)* %21, align 4
  %23 = urem i32 %14, %22
  %24 = shl i32 %23, 3
  %25 = getelementptr inbounds %opencl.pipe_t addrspace(1)* %in_pipe, i32 0, i32 4, i32 %24
  %26 = bitcast i8 addrspace(1)* %25 to i32 addrspace(4)*
  %27 = getelementptr inbounds i32 addrspace(4)* %26, i32 2
  %28 = bitcast i8 addrspace(1)* %20 to i32 addrspace(4)*
  br label %29

; <label>:29                                      ; preds = %29, %if.then
  %30 = phi i32 addrspace(4)* [ %28, %if.then ], [ %34, %29 ]
  %31 = phi i32 addrspace(4)* [ %26, %if.then ], [ %32, %29 ]
  %32 = getelementptr inbounds i32 addrspace(4)* %31, i32 1
  %33 = load i32 addrspace(4)* %31, align 4
  %34 = getelementptr inbounds i32 addrspace(4)* %30, i32 1
  store i32 %33, i32 addrspace(4)* %30, align 4
  %35 = icmp ult i32 addrspace(4)* %32, %27
  br i1 %35, label %29, label %if.end

if.end:                                           ; preds = %29, %__reserve_read_pipe_internal_user.exit
  ret void
}


declare spir_func i32 @__hsail_get_global_id(i32) nounwind readnone

declare void @llvm.lifetime.start(i64, i8* nocapture) nounwind

declare void @llvm.lifetime.end(i64, i8* nocapture) nounwind

!0 = metadata !{void (%struct.TestStruct addrspace(1)*, %opencl.pipe_t addrspace(1)*)* @__OpenCL_test_pipe_write_struct_kernel, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 1, i32 1}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"TestStruct*", metadata !"pipe __global TestStruct *"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !"pipe"}
!5 = metadata !{metadata !"kernel_arg_base_type", metadata !"TestStruct*", metadata !"pipe __global TestStruct *"}
!6 = metadata !{void (%opencl.pipe_t addrspace(1)*, %struct.TestStruct addrspace(1)*)* @__OpenCL_test_pipe_read_struct_kernel, metadata !1, metadata !2, metadata !7, metadata !8, metadata !9}
!7 = metadata !{metadata !"kernel_arg_type", metadata !"pipe __global TestStruct *", metadata !"TestStruct*"}
!8 = metadata !{metadata !"kernel_arg_type_qual", metadata !"pipe", metadata !""}
!9 = metadata !{metadata !"kernel_arg_base_type", metadata !"pipe __global TestStruct *", metadata !"TestStruct*"}
!10 = metadata !{i32 1, i32 2}
!11 = metadata !{i32 2, i32 0}
!12 = metadata !{}
!13 = metadata !{metadata !"cl_khr_depth_images"}
!14 = metadata !{metadata !"cl_doubles"}
!15 = metadata !{metadata !"cl_images"}
!16 = metadata !{i32 3}
