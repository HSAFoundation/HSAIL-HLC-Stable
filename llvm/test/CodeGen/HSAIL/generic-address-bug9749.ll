; Test case from generic_address generic_variable_volatile (Bug 9749).
; RUN: llc -march=hsail-64 -filetype=asm -o - %s | FileCheck %s

target triple = "hsail64-pc-unknown-amdopencl"

@testKernel.val = internal addrspace(1) global float 0.000000e+00, align 4

declare spir_func i32 @_Z9get_fencePU3AS4v(i8 addrspace(4)*)

; Should generate stof_global and flat load
;
; CHECK: stof_global_u64_u64
; CHECK: ld_align(4)_f32
define spir_kernel void @__OpenCL_testKernel_kernel(i32 addrspace(1)* nocapture %results) nounwind {
entry:
  %ptr = alloca float addrspace(4)*, align 8
  %0 = call spir_func i32 @__hsail_get_global_id(i32 0) nounwind readnone
  %1 = zext i32 %0 to i64
  store float 0x3FB99999A0000000, float addrspace(1)* @testKernel.val, align 4
  store volatile float addrspace(4)* bitcast (float addrspace(1)* @testKernel.val to float addrspace(4)*), float addrspace(4)** %ptr, align 8
  %2 = load volatile float addrspace(4)** %ptr, align 8
  %3 = load float addrspace(1)* @testKernel.val, align 4
  %4 = bitcast float addrspace(4)* %2 to i8 addrspace(4)*
  %call.i = call spir_func i32 @_Z9get_fencePU3AS4v(i8 addrspace(4)* %4) nounwind
  %switch.i.i = icmp ult i32 %call.i, 4
  br i1 %switch.i.i, label %if.end.i, label %helperFunction.exit

if.end.i:                                         ; preds = %entry
  %5 = load float addrspace(4)* %2, align 4
  %not.cmp.i = fcmp oeq float %5, %3
  %phitmp = zext i1 %not.cmp.i to i32
  br label %helperFunction.exit

helperFunction.exit:                              ; preds = %if.end.i, %entry
  %retval.0.i = phi i32 [ 0, %entry ], [ %phitmp, %if.end.i ]
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %results, i64 %1
  store i32 %retval.0.i, i32 addrspace(1)* %arrayidx, align 4
  ret void
}

declare spir_func i32 @__hsail_get_global_id(i32) nounwind readnone