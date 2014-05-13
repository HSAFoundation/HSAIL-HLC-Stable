; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @lshr_i16_arg
; CHECK: ubit_extract
; CHECK: ushr
define i16 @lshr_i16_arg(i16 %a, i16 %b) nounwind {
  %lshr = lshr i16 %a, %b
  ret i16 %lshr
}

; CHECK-LABEL: @lshr_v2i16_arg
; CHECK: ubit_extract
; CHECK: ushr
define <2 x i16> @lshr_v2i16_arg(<2 x i16> %a, <2 x i16> %b) nounwind {
  %lshr = lshr <2 x i16> %a, %b
  ret <2 x i16> %lshr
}

; CHECK-LABEL: @lshr_v4i16_arg
; CHECK: ubit_extract
; CHECK: ushr
define <4 x i16> @lshr_v4i16_arg(<4 x i16> %a, <4 x i16> %b) nounwind {
  %lshr = lshr <4 x i16> %a, %b
  ret <4 x i16> %lshr
}

; CHECK-LABEL: @lshr_i16_const_arg
; CHECK: ubit_extract
; CHECK: ushr
define i16 @lshr_i16_const_arg(i16 %b) nounwind {
  %lshr = lshr i16 %b, 3
  ret i16 %lshr
}

; CHECK-LABEL: @lshr_v2i16_const_arg
; CHECK: ubit_extract
; CHECK: ushr
define <2 x i16> @lshr_v2i16_const_arg(<2 x i16> %b) nounwind {
  %lshr = lshr <2 x i16> %b, <i16 3, i16 3>
  ret <2 x i16> %lshr
}

; CHECK-LABEL: @lshr_v4i16_const_arg
; CHECK: ubit_extract
; CHECK: ushr
define <4 x i16> @lshr_v4i16_const_arg(<4 x i16> %b) nounwind {
  %lshr = lshr <4 x i16> %b, <i16 3, i16 3, i16 3, i16 3>
  ret <4 x i16> %lshr
}

; CHECK-LABEL: @lshr_v2i16_const_bitcast_arg
define <2 x i16> @lshr_v2i16_const_bitcast_arg(i32 %b) nounwind {
  %b_bc = bitcast i32 %b to <2 x i16>
  %lshr = lshr <2 x i16> %b_bc, <i16 3, i16 3>
  ret <2 x i16> %lshr
}

; CHECK-LABEL: @ashr_i16_arg
; CHECK: ibit_extract
; CHECK: ishr
define i16 @ashr_i16_arg(i16 %a, i16 %b) nounwind {
  %ashr = ashr i16 %a, %b
  ret i16 %ashr
}

; CHECK-LABEL: @ashr_v2i16_arg
; CHECK: ibit_extract
; CHECK: ishr
define <2 x i16> @ashr_v2i16_arg(<2 x i16> %a, <2 x i16> %b) nounwind {
  %ashr = ashr <2 x i16> %a, %b
  ret <2 x i16> %ashr
}

; CHECK-LABEL: @ashr_v4i16_arg
; CHECK: ibit_extract
; CHECK: ishr
define <4 x i16> @ashr_v4i16_arg(<4 x i16> %a, <4 x i16> %b) nounwind {
  %ashr = ashr <4 x i16> %a, %b
  ret <4 x i16> %ashr
}

; CHECK-LABEL: @ashr_i16_const_arg
; CHECK: ibit_extract
; CHECK: ishr
define i16 @ashr_i16_const_arg(i16 %b) nounwind {
  %ashr = ashr i16 %b, 3
  ret i16 %ashr
}

; CHECK-LABEL: @ashr_v2i16_const_arg
; CHECK: ibit_extract
; CHECK: ishr
define <2 x i16> @ashr_v2i16_const_arg(<2 x i16> %b) nounwind {
  %ashr = ashr <2 x i16> %b, <i16 3, i16 3>
  ret <2 x i16> %ashr
}

; CHECK-LABEL: @ashr_v4i16_const_arg
; CHECK: ibit_extract
; CHECK: ishr
define <4 x i16> @ashr_v4i16_const_arg(<4 x i16> %b) nounwind {
  %ashr = ashr <4 x i16> %b, <i16 3, i16 3, i16 3, i16 3>
  ret <4 x i16> %ashr
}

; CHECK-LABEL: @shl_i16_arg
; CHECK: ishl
define i16 @shl_i16_arg(i16 %a, i16 %b) nounwind {
  %shl = shl i16 %a, %b
  ret i16 %shl
}

; CHECK-LABEL: @shl_v2i16_arg
; CHECK: ishl
define <2 x i16> @shl_v2i16_arg(<2 x i16> %a, <2 x i16> %b) nounwind {
  %shl = shl <2 x i16> %a, %b
  ret <2 x i16> %shl
}

; CHECK-LABEL: @shl_v4i16_arg
; CHECK: ishl
define <4 x i16> @shl_v4i16_arg(<4 x i16> %a, <4 x i16> %b) nounwind {
  %shl = shl <4 x i16> %a, %b
  ret <4 x i16> %shl
}

; CHECK-LABEL: @shl_i16_const_arg
; CHECK: ishl
define i16 @shl_i16_const_arg(i16 %a) nounwind {
  %shl = shl i16 %a, 3
  ret i16 %shl
}

; CHECK-LABEL: @shl_v2i16_const_arg
; CHECK: ishl
define <2 x i16> @shl_v2i16_const_arg(<2 x i16> %b) nounwind {
  %shl = shl <2 x i16> %b, <i16 3, i16 3>
  ret <2 x i16> %shl
}

; CHECK-LABEL: @shl_v4i16_const_arg
; CHECK: ishl
define <4 x i16> @shl_v4i16_const_arg(<4 x i16> %b) nounwind {
  %shl = shl <4 x i16> %b, <i16 3, i16 3, i16 3, i16 3>
  ret <4 x i16> %shl
}

; CHECK-LABEL: @shl_v2i16_const_bitcast_arg
; CHECK: ishl
define <2 x i16> @shl_v2i16_const_bitcast_arg(i32 %b) nounwind {
  %b_bc = bitcast i32 %b to <2 x i16>
  %shl = shl <2 x i16> %b_bc, <i16 3, i16 3>
  ret <2 x i16> %shl
}

