; RUN: llc -asm-verbose=0 -march=amdil < %s | FileCheck %s

; CHECK-LABEL: @t1_u16
; CHECK: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK: mov r1.x___, [[LIT0]]
; CHECK-NEXT: ret_dyn
define i16 @t1_u16() {
  ret i16 0
}

; CHECK-LABEL: @t1_u32
; CHECK: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK: mov r1.x___, [[LIT0]]
; CHECK-NEXT: ret_dyn
define i32 @t1_u32() {
  ret i32 0
}

; CHECK-LABEL: @t1_u64
; CHECK: dcl_literal [[LIT0:l[0-9]+]], 0x00000000, 0x00000000,
; CHECK: mov r1.xy__, [[LIT0]]
; CHECK-NEXT: ret_dyn
define i64 @t1_u64() {
  ret i64 0
}

; CHECK-LABEL: @t1_f32
; CHECK: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK: mov r1.x___, [[LIT0]]
; CHECK-NEXT: ret_dyn
define float @t1_f32() {
  ret float 0.0
}

; CHECK-LABEL: @t1_f64
; CHECK: dcl_literal [[LIT0:l[0-9]+]], 0x00000000, 0x00000000,
; CHECK: mov r1.xy__, [[LIT0]]
; CHECK-NEXT: ret_dyn
define double @t1_f64() {
  ret double 0.0
}

; CHECK-LABEL: @t2_u16
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define i16 @t2_u16(i16 %x) {
 ret i16 %x
}

; CHECK-LABEL: @t2_u32
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define i32 @t2_u32(i32 %x) {
  ret i32 %x
}

; CHECK-LABEL: @t2_u64
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define i64 @t2_u64(i64 %x) {
  ret i64 %x
}

; CHECK-LABEL: @t3_f32
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define float @t3_f32(float %x) {
  ret float %x
}

; CHECK-LABEL: @t3_f64
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define double @t3_f64(double %x) {
  ret double %x
}

