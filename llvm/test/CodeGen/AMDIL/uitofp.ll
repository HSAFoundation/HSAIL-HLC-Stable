; RUN: llc -march=amdil < %s | FileCheck %s
; RUN: llc -march=amdil -mattr=+64bitptr < %s | FileCheck %s

define void @ulong_to_float(i64 %l, i64 %ul, float addrspace(1)* nocapture %result) nounwind {
; CHECK-LABEL: @ulong_to_float
; CHECK: ffb_hi
; CHECK: ilt
; CHECK: cmov_logical
; CHECK: iadd
; CHECK: ffb_hi
; CHECK: ilt
; CHECK: cmov_logical
; CHECK: ieq
; CHECK: cmov_logical
; CHECK: i64shl
; CHECK: iand
; CHECK: u64shr
; CHECK: inegate
; CHECK: iadd
; CHECK: i64ne
; CHECK: cmov_logical
; CHECK: ishl
; CHECK: ior
; CHECK: iand
; CHECK: iand
; CHECK: i64eq
; CHECK: cmov_logical
; CHECK: u64lt
; CHECK: cmov_logical
; CHECK: iadd
entry:
  %conv = uitofp i64 %l to float
  store float %conv, float addrspace(1)* %result, align 16
  ret void
}

; CHECK-LABEL: @vector_ulong_to_float
define void @vector_ulong_to_float(<4 x i64> %l, <4 x i64> %ul, <4 x float> addrspace(1)* nocapture %result) nounwind {
entry:
  %conv = uitofp <4 x i64> %l to <4 x float>
  store <4 x float> %conv, <4 x float> addrspace(1)* %result, align 16
  ret void
}

; Some of the 64-bit setccs produced by the uitofp implementation will
; try to be truncated to 32-bit.
; CHECK-LABEL: @reduce_compare_width_setcc64
define float @reduce_compare_width_setcc64(i32 %x) nounwind {
entry:
  %tmp2 = zext i32 %x to i64
  %conv = uitofp i64 %tmp2 to float
  ret float %conv
}
