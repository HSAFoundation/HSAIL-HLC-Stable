 ; RUN: llc < %s -march=amdil | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"


define void @long_to_float(i64 %l, i64 %ul, float addrspace(1)* nocapture %result) nounwind {
; CHECK: i64shr
; CHECK: i64add
; CHECK: ixor
; CHECK: ffb_hi
; CHECK: ilt
; CHECK: cmov_logical
; CHECK: iadd
; CHECK: ffb_hi
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
entry:
  %conv = sitofp i64 %l to float
  store float %conv, float addrspace(1)* %result, align 16
  ret void
}

define void @vector_long_to_float(<4 x i64> %l, <4 x i64> %ul, <4 x float> addrspace(1)* nocapture %result) nounwind {
entry:
  %conv = sitofp <4 x i64> %l to <4 x float>
  store <4 x float> %conv, <4 x float> addrspace(1)* %result, align 16
  ret void
}
