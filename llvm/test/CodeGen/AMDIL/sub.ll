; RUN: llc < %s -march=amdil | FileCheck %s

define i16 @t1_u16(i16 %x, i16 %y) {
; CHECK:	mov r66.x___, r1.y
; CHECK-NEXT:	mov r65.x___, l12
; CHECK-NEXT:	ishl r66.x___, r66.x, r65.x
; CHECK-NEXT:	ishr r65.x___, r66.x, r65.x
; CHECK-NEXT:	mov r65.x___, r65.x
; CHECK-NEXT:	inegate r65.x___, r65.x
; CHECK-NEXT:	iadd r65.x___, r1.x, r65.x
; CHECK-NEXT:	mov r1.x___, r65.x
; CHECK-NEXT:	ret_dyn
; CHECK-NEXT:  ret

	%z = sub i16 %x, %y
	ret i16 %z
}

define i32 @t1_u32(i32 %x, i32 %y) {
; CHECK: inegate r65.x___, r1.y
; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
; CHECK-NEXT: mov r1.x___, r65.x
; CHECK-NEXT: ret_dyn
; CHECK-NEXT: ret

	%z = sub i32 %x, %y
	ret i32 %z
}

define float @t1_f32(float %x, float %y) {
; CHECK: sub r65.x___, r1.x, r1.y
; CHECK-NEXT: mov r1.x___, r65.x
; CHECK-NEXT: ret_dyn
; CHECK-NEXT: ret

  %z = fsub float %x, %y
  ret float %z
}

define i16 @t2_u16(i16 %x) {
; CHECK: mov r65.x___, l12
; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
; CHECK-NEXT: mov r1.x___, r65.x
; CHECK-NEXT: ret_dyn
; CHECK-NEXT: ret

	%z = sub i16 %x, 1
	ret i16 %z
}

define i32 @t2_u32(i32 %x) {
; CHECK:	mov r65.x___, l12
; CHECK-NEXT:	iadd r65.x___, r1.x, r65.x
; CHECK-NEXT:	mov r1.x___, r65.x
; CHECK-NEXT:	ret_dyn
; CHECK-NEXT:  ret

	%z = sub i32 %x, 1
	ret i32 %z
}

define i64 @t2_u64(i64 %x) {
; CHECK: mov r66.xy__, l12
; CHECK-NEXT: mov r65.x___, r66.y000
; CHECK-NEXT: mov r67.x___, r1.y000
; CHECK-NEXT: iadd r65.x___, r67.x, r65.x
; CHECK-NEXT: mov r67.x___, r66.x000
; CHECK-NEXT: mov r66.x___, r1.x000
; CHECK-NEXT: iadd r66.x___, r66.x, r67.x
; CHECK-NEXT: ult r67.x___, r66.x, r67.x
; CHECK-NEXT: inegate r67.x___, r67.x
; CHECK-NEXT: iadd r65.x___, r65.x, r67.x
; CHECK-NEXT: iadd r65.xy__, r66.x000, r65.0x00
; CHECK-NEXT: mov r1.xy__, r65.xyxy
; CHECK-NEXT: ret_dyn
; CHECK-NEXT: ret

	%z = sub i64 %x, 1
	ret i64 %z
}

define float @t2_f32(float %x) {
; CHECK: mov r65.x___, l12
; CHECK-NEXT: add r65.x___, r1.x, r65.x
; CHECK-NEXT: mov r1.x___, r65.x
; CHECK-NEXT: ret_dyn
; CHECK-NEXT: ret

  %z = fsub float %x, 1.0
  ret float %z
}
