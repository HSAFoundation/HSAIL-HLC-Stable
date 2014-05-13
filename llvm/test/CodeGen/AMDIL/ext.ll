; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @zext_i8_to_i16
; CHECK: dcl_literal [[LIT:l[0-9]+]], 0x00000018,
; CHECK-NEXT: ishl [[REG:r[0-9]+]].x___, r1.x, [[LIT]]
; CHECK-NEXT: ushr r{{[0-9]+}}.x___, [[REG]].x, [[LIT]]
; CHECK-NEXT: ret_dyn
define i16 @zext_i8_to_i16(i8 %src) nounwind {
  %zext = zext i8 %src to i16
  ret i16 %zext
}

; CHECK-LABEL: @sext_i8_to_i16
; CHECK: dcl_literal [[LIT:l[0-9]+]], 0x00000018,
; CHECK-NEXT: ishl [[REG:r[0-9]+]].x___, r1.x, [[LIT]]
; CHECK-NEXT: ishr r{{[0-9]+}}.x___, [[REG]].x, [[LIT]]
; CHECK-NEXT: ret_dyn
define i16 @sext_i8_to_i16(i8 %src) nounwind {
  %sext = sext i8 %src to i16
  ret i16 %sext
}

; CHECK-LABEL: @zext_i8_to_i32
; CHECK: dcl_literal [[LIT:l[0-9]+]], 0x00000018,
; CHECK-NEXT: ishl [[REG:r[0-9]+]].x___, r1.x, [[LIT]]
; CHECK-NEXT: ushr r{{[0-9]+}}.x___, [[REG]].x, [[LIT]]
; CHECK-NEXT: ret_dyn
define i32 @zext_i8_to_i32(i8 %src) nounwind {
  %zext = zext i8 %src to i32
  ret i32 %zext
}

; CHECK-LABEL: @sext_i8_to_i32
; CHECK: dcl_literal [[LIT:l[0-9]+]], 0x00000018,
; CHECK-NEXT: ishl [[REG:r[0-9]+]].x___, r1.x, [[LIT]]
; CHECK-NEXT: ishr r{{[0-9]+}}.x___, [[REG]].x, [[LIT]]
; CHECK-NEXT: ret_dyn
define i32 @sext_i8_to_i32(i8 %src) nounwind {
  %sext = sext i8 %src to i32
  ret i32 %sext
}


