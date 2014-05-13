; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @lshr_i8_arg
; CHECK: ubit_extract
; CHECK: ushr
define i8 @lshr_i8_arg(i8 %a, i8 %b) nounwind {
  %lshr = lshr i8 %a, %b
  ret i8 %lshr
}

; CHECK-LABEL: @lshr_v2i8_arg
; CHECK: ubit_extract
; CHECK: ushr
define <2 x i8> @lshr_v2i8_arg(<2 x i8> %a, <2 x i8> %b) nounwind {
  %lshr = lshr <2 x i8> %a, %b
  ret <2 x i8> %lshr
}

; CHECK-LABEL: @lshr_v4i8_arg
; CHECK: ubit_extract
; CHECK: ushr
define <4 x i8> @lshr_v4i8_arg(<4 x i8> %a, <4 x i8> %b) nounwind {
  %lshr = lshr <4 x i8> %a, %b
  ret <4 x i8> %lshr
}

; CHECK-LABEL: @lshr_i8_const_arg
; CHECK: ubit_extract
; CHECK: ushr
define i8 @lshr_i8_const_arg(i8 %b) nounwind {
  %lshr = lshr i8 %b, 3
  ret i8 %lshr
}

; CHECK-LABEL: @lshr_v2i8_const_arg
; CHECK: ubit_extract
; CHECK: ushr
define <2 x i8> @lshr_v2i8_const_arg(<2 x i8> %b) nounwind {
  %lshr = lshr <2 x i8> %b, <i8 3, i8 3>
  ret <2 x i8> %lshr
}

; CHECK-LABEL: @lshr_v4i8_const_arg
; CHECK: ubit_extract
; CHECK: ushr
define <4 x i8> @lshr_v4i8_const_arg(<4 x i8> %b) nounwind {
  %lshr = lshr <4 x i8> %b, <i8 3, i8 3, i8 3, i8 3>
  ret <4 x i8> %lshr
}

; CHECK-LABEL: @lshr_v2i8_const_bitcast_arg
define <2 x i8> @lshr_v2i8_const_bitcast_arg(i16 %b) nounwind {
  %b_bc = bitcast i16 %b to <2 x i8>
  %lshr = lshr <2 x i8> %b_bc, <i8 3, i8 3>
  ret <2 x i8> %lshr
}

; CHECK-LABEL: @ashr_i8_arg
; CHECK: ibit_extract
; CHECK: ishr
define i8 @ashr_i8_arg(i8 %a, i8 %b) nounwind {
  %ashr = ashr i8 %a, %b
  ret i8 %ashr
}

; CHECK-LABEL: @ashr_v2i8_arg
; CHECK: ibit_extract
; CHECK: ishr
define <2 x i8> @ashr_v2i8_arg(<2 x i8> %a, <2 x i8> %b) nounwind {
  %ashr = ashr <2 x i8> %a, %b
  ret <2 x i8> %ashr
}

; CHECK-LABEL @ashr_v4i8_arg
; CHECK: ibit_extract
; CHECK: ishr
define <4 x i8> @ashr_v4i8_arg(<4 x i8> %a, <4 x i8> %b) nounwind {
  %ashr = ashr <4 x i8> %a, %b
  ret <4 x i8> %ashr
}

; CHECK-LABEL: @ashr_i8_const_arg
; CHECK: ibit_extract
; CHECK: ishr
define i8 @ashr_i8_const_arg(i8 %b) nounwind {
  %ashr = ashr i8 %b, 3
  ret i8 %ashr
}

; CHECK-LABEL: @ashr_v2i8_const_arg
; CHECK: ibit_extract
; CHECK: ishr
define <2 x i8> @ashr_v2i8_const_arg(<2 x i8> %b) nounwind {
  %ashr = ashr <2 x i8> %b, <i8 3, i8 3>
  ret <2 x i8> %ashr
}

; CHECK: ibit_extract
; CHECK: ishr
define <4 x i8> @ashr_v4i8_const_arg(<4 x i8> %b) nounwind {
  %ashr = ashr <4 x i8> %b, <i8 3, i8 3, i8 3, i8 3>
  ret <4 x i8> %ashr
}

; CHECK-LABEL: @shl_i8_arg
; CHECK: ishl
define i8 @shl_i8_arg(i8 %a, i8 %b) nounwind {
  %shl = shl i8 %a, %b
  ret i8 %shl
}

; CHECK-LABEL: @shl_v2i8_arg
; CHECK: ishl
define <2 x i8> @shl_v2i8_arg(<2 x i8> %a, <2 x i8> %b) nounwind {
  %shl = shl <2 x i8> %a, %b
  ret <2 x i8> %shl
}

; CHECK-LABEL: @shl_v4i8_arg
; CHECK: ishl
define <4 x i8> @shl_v4i8_arg(<4 x i8> %a, <4 x i8> %b) nounwind {
  %shl = shl <4 x i8> %a, %b
  ret <4 x i8> %shl
}

; CHECK-LABEL: @shl_i8_const_arg
; CHECK: ishl
define i8 @shl_i8_const_arg(i8 %a) nounwind {
  %shl = shl i8 %a, 3
  ret i8 %shl
}

; CHECK-LABEL: @shl_v2i8_const_arg
; CHECK: ishl
define <2 x i8> @shl_v2i8_const_arg(<2 x i8> %b) nounwind {
  %shl = shl <2 x i8> %b, <i8 3, i8 3>
  ret <2 x i8> %shl
}

; CHECK-LABEL: @shl_v4i8_const_arg
; CHECK: ishl
define <4 x i8> @shl_v4i8_const_arg(<4 x i8> %b) nounwind {
  %shl = shl <4 x i8> %b, <i8 3, i8 3, i8 3, i8 3>
  ret <4 x i8> %shl
}

; CHECK-LABEL: @shl_v2i8_const_bitcast_arg
; CHECK: ishl
define <2 x i8> @shl_v2i8_const_bitcast_arg(i16 %b) nounwind {
  %b_bc = bitcast i16 %b to <2 x i8>
  %shl = shl <2 x i8> %b_bc, <i8 3, i8 3>
  ret <2 x i8> %shl
}

