; RUN: llc < %s -march=amdil | FileCheck %s

define float @t1_f32(float %x, float %y) {
; CHECK:	mul_ieee r65.x___, r1.x, r1.y
; CHECK:	mov r1.x___, r65.x
; CHECK:	ret_dyn
; CHECK:  ret

  %z = fmul float %x, %y
  ret float %z
}

define float @t2_f32(float %x) {
; CHECK:	mov r65.x___, l12
; CHECK:	mul_ieee r65.x___, r1.x, r65.x
; CHECK:	mov r1.x___, r65.x
; CHECK:	ret_dyn
; CHECK:  ret

  %z = fmul float %x, 5.0
  ret float %z
}
