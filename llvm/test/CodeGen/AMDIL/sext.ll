; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @sext_i8_to_i32
; CHECK: dcl_literal [[LITOFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LITWIDTH:l[0-9]+]], 0x00000008,
; CHECK: ibit_extract r1.x___, [[LITWIDTH]], [[LITOFFSET]], r1.x
; CHECK-NEXT: ret_dyn
define i32 @sext_i8_to_i32(i8 %x) nounwind {
  %y = sext i8 %x to i32
  ret i32 %y
}

; CHECK-LABEL: @sext_i32_to_i64
; CHECK: dcl_literal [[LIT:l[0-9]+]], 0x0000001F,
; CHECK-NEXT: ishr [[REG:r[0-9]+]].x___, r1.x, [[LIT]]
; CHECK-NEXT: iadd r1.xy__, r1.x000, [[REG]].0x00
; CHECK-NEXT: ret_dyn
define i64 @sext_i32_to_i64(i32 %x) nounwind {
  %y = sext i32 %x to i64
  ret i64 %y
}

; Make sure we don't do a 32 bit shift on a 32-bit type when doing a
; 32 in 64 sext_inregin. This should be the same as a regular sign
; extension since it's really a 32-bit subregister.

; CHECK-LABEL: @sext_after_trunc_i32_to_i64
; CHECK: dcl_literal [[LIT:l[0-9]+]], 0x0000001F,
; CHECK: ishr [[REG:r[0-9]+]].x___, r{{[0-9]+}}.x, [[LIT]]
; CHECK-NEXT: iadd r1.xy__, r{{[0-9]+}}.x000, [[REG]].0x00
; CHECK-NEXT: ret_dyn
define i64 @sext_after_trunc_i32_to_i64(i64 %a) nounwind {
  %b = trunc i64 %a to i32
  %c = sext i32 %b to i64
  ret i64 %c
}
