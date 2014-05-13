; RUN: llc -march=amdil < %s | FileCheck %s

; CHECK-LABEL: @shl_r_r
; CHECK: ishl r1.x___, r1.x, r1.y
; CHECK-NEXT: ret_dyn
define i32 @shl_r_r(i32 %x, i32 %y) {
  %z = shl i32 %x, %y
  ret i32 %z
}

; CHECK-LABEL: @shl_r_i
; CHECK: dcl_literal [[LIT3:l[0-9]+]], 0x00000003,
; CHECK: ishl r1.x___, r1.x, [[LIT3]]
; CHECK: ret_dyn
define i32 @shl_r_i(i32 %x) {
  %z = shl i32 %x, 3
  ret i32 %z
}

; CHECK-LABEL: @shl_i_r
; CHECK: dcl_literal [[LIT3:l[0-9]+]], 0x00000003,
; CHECK: mov [[LITREG:r[0-9]+]].x___, [[LIT3]]
; CHECK: ishl r1.x___, [[LITREG]].x, r1.x
; CHECK: ret_dyn
define i32 @shl_i_r(i32 %x) {
  %z = shl i32 3, %x
  ret i32 %z
}
