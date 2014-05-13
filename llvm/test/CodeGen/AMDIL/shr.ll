; RUN: llc -march=amdil < %s | FileCheck %s

; CHECK-LABEL: @lshr_r_r
; CHECK:      ushr r1.x___, r1.x, r1.y
; CHECK-NEXT: ret_dyn
define i32 @lshr_r_r(i32 %x, i32 %y) {
  %z = lshr i32 %x, %y
  ret i32 %z
}

; CHECK-LABEL: @lshr_r_i
; CHECK: dcl_literal [[LIT3:l[0-9]+]], 0x00000003,
; CHECK: ushr r1.x___, r1.x, [[LIT3]]
; CHECK-NEXT: ret_dyn
define i32 @lshr_r_i(i32 %x) {
  %z = lshr i32 %x, 3
  ret i32 %z
}

; CHECK-LABEL: @lshr_i_r
; CHECK: dcl_literal [[LIT3:l[0-9]+]], 0x00000003,
; CHECK: mov [[LITREG:r[0-9]+]].x___, [[LIT3]]
; CHECK-NEXT: ushr r1.x___, [[LITREG]].x, r1.x
; CHECK-NEXT: ret_dyn
define i32 @lshr_i_r(i32 %x) {
  %z = lshr i32 3, %x
  ret i32 %z
}

; CHECK-LABEL: @ashr_r_i
; CHECK:      ishr r1.x___, r1.x, r1.y
; CHECK-NEXT: ret_dyn
define i32 @ashr_r_i(i32 %x, i32 %y) {
  %z = ashr i32 %x, %y
  ret i32 %z
}

; CHECK-LABEL: @t5
; CHECK: dcl_literal [[LIT3:l[0-9]+]], 0x00000003,
; CHECK: ishr r1.x___, r1.x, [[LIT3]]
; CHECK-NEXT: ret_dyn
define i32 @t5(i32 %x) {
  %z = ashr i32 %x, 3
  ret i32 %z
}

; CHECK-LABEL: @ashr_i_r
; CHECK: dcl_literal [[LIT3:l[0-9]+]], 0x00000003,
; CHECK: mov [[LITREG:r[0-9]+]].x___, [[LIT3]]
; CHECK-NEXT: ushr r1.x___, [[LITREG]].x, r1.x
; CHECK-NEXT: ret_dyn
define i32 @ashr_i_r(i32 %x) {
  %z = ashr i32 3, %x
  ret i32 %z
}
