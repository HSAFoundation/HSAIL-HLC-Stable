; XFAIL: *
; RUN: llc -march=amdil -mcpu=tonga < %s | FileCheck %s

; FIXME: Some 32-bit ittshifts before the 16-bit one?
; FIXME: Some of these aren't being selected yet

; CHECK-LABEL: @add_i16
; CHECK: u16add
; CHECK-NEXT: ret_dyn
define i16 @add_i16(i16 %a, i16 %b) {
  %result = add i16 %a, %b
  ret i16 %result
}

; CHECK-LABEL: @sub_i16
; CHECK: u16sub
; CHECK-NEXT: ret_dyn
define i16 @sub_i16(i16 %a, i16 %b) {
  %result = sub i16 %a, %b
  ret i16 %result
}

; CHECK-LABEL: @mul_i16
; CHECK: u16mul
; CHECK-NEXT: ret_dyn
define i16 @mul_i16(i16 %a, i16 %b) {
  %result = mul i16 %a, %b
  ret i16 %result
}

; CHECK-LABEL: @shl_i16
; CHECK: i16shl
; CHECK-NEXT: ret_dyn
define i16 @shl_i16(i16 %a, i16 %b) {
  %result = shl i16 %a, %b
  ret i16 %result
}

; CHECK-LABEL: @ishr_i16
; CHECK: i16shr
define i16 @ishr_i16(i16 %a, i16 %b) {
  %result = ashr i16 %a, %b
  ret i16 %result
}

; CHECK-LABEL: @ushr_i16
; CHECK: u16shr
; CHECK-NEXT: ret_dyn
define i16 @ushr_i16(i16 %a, i16 %b) {
  %result = lshr i16 %a, %b
  ret i16 %result
}

; CHECK-LABEL: @imin_i16
; CHECK: i16min
define i16 @imin_i16(i16 %a, i16 %b) {
  %cmp = icmp slt i16 %a, %b
  %result = select i1 %cmp, i16 %a, i16 %b
  ret i16 %result
}

; CHECK-LABEL: @umin_i16
; CHECK: u16min
define i16 @umin_i16(i16 %a, i16 %b) {
  %cmp = icmp ult i16 %a, %b
  %result = select i1 %cmp, i16 %a, i16 %b
  ret i16 %result
}

; CHECK-LABEL: @imax_i16
; CHECK: i16max
define i16 @imax_i16(i16 %a, i16 %b) {
  %cmp = icmp sge i16 %a, %b
  %result = select i1 %cmp, i16 %a, i16 %b
  ret i16 %result
}

; CHECK-LABEL: @umax_i16
; CHECK: u16max
define i16 @umax_i16(i16 %a, i16 %b) {
  %cmp = icmp uge i16 %a, %b
  %result = select i1 %cmp, i16 %a, i16 %b
  ret i16 %result
}

; CHECK-LABEL: @imad_i16
; CHECK: i16mad
define i16 @imad_i16(i16 %a, i16 %b, i16 %c) {
  %mul = mul i16 %a, %b
  %result = add i16 %mul, %c
  ret i16 %result
}

; CHECK-LABEL: @umad_i16
; CHECK: u16mad
define i16 @umad_i16(i16 %a, i16 %b, i16 %c) {
  %mul = mul i16 %a, %b
  %result = add i16 %mul, %c
  ret i16 %result
}
