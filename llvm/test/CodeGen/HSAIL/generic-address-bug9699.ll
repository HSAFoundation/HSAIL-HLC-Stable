; Avoid optimizing AddrSpaceCast to PtrToIntInst/IntToPtrInst pair
; so that AddrSpaceCast can be recognized and optimized. Global
; load/store should be generated instead of flat ones in the test
; cases  (Bug 9699).
;
; RUN: llc -march=hsail -filetype=asm -o - %s | FileCheck %s


%0 = type { i32, i32, i32, [116 x i8], [1 x i8] }
%opencl.pipe_t = type { i32, i32, i32, [116 x i8], [1 x i8] }

; AddrSpaceCast should be optimized away and segment load (ld_global) should be
; generated for "%22 = load i8 addrspace(4)* %2, align 1".
;
; CHECK-LABEL: @__OpenCL_test_pipe_convenience_write_char_kernel
; CHECK-NOT: stof_global
; CHECK: ld_global_u8
define spir_kernel void @__OpenCL_test_pipe_convenience_write_char_kernel(i8 addrspace(1)* nocapture %src, %opencl.pipe_t addrspace(1)* %out_pipe) nounwind {
entry:
  %0 = alloca i32, align 4
  %1 = call spir_func i32 @__hsail_get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %src, i32 %1
  %2 = bitcast i8 addrspace(1)* %arrayidx to i8 addrspace(4)*
  %3 = bitcast %opencl.pipe_t addrspace(1)* %out_pipe to %0 addrspace(1)*
  %4 = getelementptr inbounds %opencl.pipe_t addrspace(1)* %out_pipe, i32 0, i32 0
  %5 = load atomic volatile i32 addrspace(1)* %4 monotonic, align 4, !mem.scope !16
  %6 = getelementptr inbounds %opencl.pipe_t addrspace(1)* %out_pipe, i32 0, i32 2
  %7 = load i32 addrspace(1)* %6, align 4
  %8 = getelementptr inbounds %opencl.pipe_t addrspace(1)* %out_pipe, i32 0, i32 1
  %9 = add i32 %7, %5
  %10 = bitcast i32* %0 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %10) nounwind
  %11 = load atomic volatile i32 addrspace(1)* %8 acquire, align 4, !mem.scope !16
  store i32 %11, i32* %0, align 4
  br label %12

; <label>:12                                      ; preds = %16, %entry
  %13 = phi i32 [ %18, %16 ], [ %11, %entry ]
  %14 = add i32 %13, 1
  %15 = icmp ugt i32 %14, %9
  br i1 %15, label %__write_pipe_internal_1.exit, label %16

; <label>:16                                      ; preds = %12
  %17 = load volatile i32* %0, align 4
  %18 = cmpxchg i32 addrspace(1)* %8, i32 %17, i32 %14 acq_rel, !mem.scope !16
  store volatile i32 %18, i32* %0, align 4
  %19 = icmp eq i32 %18, %17
  br i1 %19, label %.exit.i, label %12

.exit.i:                                          ; preds = %16
  call void @llvm.lifetime.end(i64 -1, i8* %10) nounwind
  %20 = icmp eq i32 %17, -1
  br i1 %20, label %__write_pipe_internal_1.exit, label %21

; <label>:21                                      ; preds = %.exit.i
  %22 = load i8 addrspace(4)* %2, align 1
  %23 = urem i32 %17, %7
  %24 = getelementptr inbounds %0 addrspace(1)* %3, i32 0, i32 4, i32 %23
  store i8 %22, i8 addrspace(1)* %24, align 1
  br label %__write_pipe_internal_1.exit

__write_pipe_internal_1.exit:                     ; preds = %21, %.exit.i, %12
  ret void
}

; AddrSpaceCast should be optimized away and segment store (st_global) should be
; generated for "store i8 %22, i8 addrspace(4)* %2, align 1".
;
; CHECK-LABEL: @__OpenCL_test_pipe_convenience_read_char_kernel
; CHECK-NOT: stof_global
; CHECK: st_global_u8
define spir_kernel void @__OpenCL_test_pipe_convenience_read_char_kernel(%opencl.pipe_t addrspace(1)* %in_pipe, i8 addrspace(1)* nocapture %dst) nounwind {
entry:
  %0 = alloca i32, align 4
  %1 = call spir_func i32 @__hsail_get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %dst, i32 %1
  %2 = bitcast i8 addrspace(1)* %arrayidx to i8 addrspace(4)*
  %3 = getelementptr inbounds %opencl.pipe_t addrspace(1)* %in_pipe, i32 0, i32 1
  %4 = load atomic volatile i32 addrspace(1)* %3 monotonic, align 4, !mem.scope !16
  %5 = getelementptr inbounds %opencl.pipe_t addrspace(1)* %in_pipe, i32 0, i32 0
  %6 = bitcast i32* %0 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %6) nounwind
  %7 = load atomic volatile i32 addrspace(1)* %5 acquire, align 4, !mem.scope !16
  store i32 %7, i32* %0, align 4
  br label %8

; <label>:8                                       ; preds = %12, %entry
  %9 = phi i32 [ %14, %12 ], [ %7, %entry ]
  %10 = add i32 %9, 1
  %11 = icmp ugt i32 %10, %4
  br i1 %11, label %__read_pipe_internal_1.exit, label %12

; <label>:12                                      ; preds = %8
  %13 = load volatile i32* %0, align 4
  %14 = cmpxchg i32 addrspace(1)* %5, i32 %13, i32 %10 acq_rel, !mem.scope !16
  store volatile i32 %14, i32* %0, align 4
  %15 = icmp eq i32 %14, %13
  br i1 %15, label %.exit.i, label %8

.exit.i:                                          ; preds = %12
  call void @llvm.lifetime.end(i64 -1, i8* %6) nounwind
  %16 = icmp eq i32 %13, -1
  br i1 %16, label %__read_pipe_internal_1.exit, label %17

; <label>:17                                      ; preds = %.exit.i
  %18 = getelementptr inbounds %opencl.pipe_t addrspace(1)* %in_pipe, i32 0, i32 2
  %19 = load i32 addrspace(1)* %18, align 4
  %20 = urem i32 %13, %19
  %21 = getelementptr inbounds %opencl.pipe_t addrspace(1)* %in_pipe, i32 0, i32 4, i32 %20
  %22 = load i8 addrspace(1)* %21, align 1
  store i8 %22, i8 addrspace(4)* %2, align 1
  %23 = add i32 %4, -1
  %24 = icmp eq i32 %13, %23
  br i1 %24, label %25, label %__read_pipe_internal_1.exit

; <label>:25                                      ; preds = %17
  store atomic volatile i32 0, i32 addrspace(1)* %3 release, align 4, !mem.scope !16
  store atomic volatile i32 0, i32 addrspace(1)* %5 monotonic, align 4, !mem.scope !16
  br label %__read_pipe_internal_1.exit

__read_pipe_internal_1.exit:                      ; preds = %25, %17, %.exit.i, %8
  ret void
}

declare spir_func i32 @__hsail_get_global_id(i32) nounwind readnone

declare void @llvm.lifetime.start(i64, i8* nocapture) nounwind

declare void @llvm.lifetime.end(i64, i8* nocapture) nounwind

!opencl.kernels = !{!0, !6}

!0 = metadata !{void (i8 addrspace(1)*, %opencl.pipe_t addrspace(1)*)* @__OpenCL_test_pipe_convenience_write_char_kernel, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 1, i32 1}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"char*", metadata !"pipe __global char *"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !"pipe"}
!5 = metadata !{metadata !"kernel_arg_base_type", metadata !"char*", metadata !"pipe __global char *"}
!6 = metadata !{void (%opencl.pipe_t addrspace(1)*, i8 addrspace(1)*)* @__OpenCL_test_pipe_convenience_read_char_kernel, metadata !1, metadata !2, metadata !7, metadata !8, metadata !9}
!7 = metadata !{metadata !"kernel_arg_type", metadata !"pipe __global char *", metadata !"char*"}
!8 = metadata !{metadata !"kernel_arg_type_qual", metadata !"pipe", metadata !""}
!9 = metadata !{metadata !"kernel_arg_base_type", metadata !"pipe __global char *", metadata !"char*"}
!10 = metadata !{i32 1, i32 2}
!11 = metadata !{i32 2, i32 0}
!12 = metadata !{}
!13 = metadata !{metadata !"cl_khr_depth_images"}
!14 = metadata !{metadata !"cl_doubles"}
!15 = metadata !{metadata !"cl_images"}
!16 = metadata !{i32 3}