; Optimize AddrSpaceCast from pipe_t to i32
; http://ocltc.amd.com/bugs/show_bug.cgi?id=9673
;
; RUN: llc -march=hsail -filetype=asm -o - %s | FileCheck %s

declare spir_func i32 @__hsail_get_global_id(i32) nounwind readnone
%opencl.pipe_t = type opaque

; This is a compile time assert bug, but we still want to check optimization
; is performed to generate ld_global.
;
; CHECK: ld_global_align
define spir_kernel void @__OpenCL_test_pipe_convenience_read_int_kernel(%opencl.pipe_t addrspace(1)* nocapture %in_pipe, i32 addrspace(1)* nocapture %dst) nounwind {
entry:
  %0 = tail call spir_func i32 @__hsail_get_global_id(i32 0) nounwind readnone
  %1 = bitcast %opencl.pipe_t addrspace(1)* %in_pipe to i32 addrspace(4)*
  %add.ptr = getelementptr inbounds i32 addrspace(4)* %1, i32 2
  %2 = load i32 addrspace(4)* %add.ptr, align 4
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %dst, i32 %0
  store i32 %2, i32 addrspace(1)* %arrayidx, align 4
  ret void
}