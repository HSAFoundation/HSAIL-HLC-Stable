; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @sext_in_reg_i1_in_i8
define i8 @sext_in_reg_i1_in_i8(i8 %a) nounwind {
  %x = shl i8 %a, 7
  %y = ashr i8 %x, 7
  %z = and i8 %y, %a
  ret i8 %z
}

; CHECK-LABEL: @testcase_2
define i8 @testcase_2(i8 %a) nounwind {
  %and_a_1 = and i8 %a, 1
  %cmp_slt = icmp slt i8 %a, 0
  %sel0 = select i1 %cmp_slt, i8 0, i8 %a
  %x = shl i8 %a, 7
  %y = ashr i8 %x, 7
  %z = and i8 %y, %a
  %xor = xor i8 %sel0, %z
  ret i8 %xor
}

; CHECK-LABEL: @testcase_3
define i8 @testcase_3(i8 %a) nounwind {
  %and_a_1 = and i8 %a, 1
  %cmp_eq = icmp eq i8 %and_a_1, 0
  %cmp_slt = icmp slt i8 %a, 0
  %sel0 = select i1 %cmp_slt, i8 0, i8 %a
  %sel1 = select i1 %cmp_eq, i8 0, i8 %a
  %xor = xor i8 %sel0, %sel1
  ret i8 %xor
}

; CHECK-LABEL: @sext_in_reg_i8_to_i32
; CHECK: ibit_extract
; CHECK-NEXT: ret_dyn
define i32 @sext_in_reg_i8_to_i32(i32 %a) nounwind {
  %shl = shl i32 %a, 24
  %ashr = ashr i32 %shl, 24
  ret i32 %ashr
}

; CHECK-LABEL: @sext_in_reg_i16_to_i32
; CHECK: ibit_extract
; CHECK-NEXT: ret_dyn
define i32 @sext_in_reg_i16_to_i32(i32 %a) nounwind {
  %shl = shl i32 %a, 16
  %ashr = ashr i32 %shl, 16
  ret i32 %ashr
}

; CHECK-LABEL: @sext_in_reg_i8_to_i16
; CHECK: ibit_extract
; CHECK: iand
; CHECK: ishl
; CHECK: ishr
define i16 @sext_in_reg_i8_to_i16(i16 %a) nounwind {
  %shl = shl i16 %a, 8
  %ashr = ashr i16 %shl, 8
  ret i16 %ashr
}

define i8 @testcase(i8 %a) nounwind {
  %and_a_1 = and i8 %a, 1
  %cmp_eq = icmp eq i8 %and_a_1, 0
  %cmp_slt = icmp slt i8 %a, 0
  %sel0 = select i1 %cmp_slt, i8 0, i8 %a
  %sel1 = select i1 %cmp_eq, i8 0, i8 %a
  %xor = xor i8 %sel0, %sel1
  ret i8 %xor
}

; CHECK-LABEL: @sext_in_reg_i8_to_v1i32
; CHECK: ibit_extract
; CHECK-NEXT: ret_dyn
define <1 x i32> @sext_in_reg_i8_to_v1i32(<1 x i32> %a) nounwind {
  %shl = shl <1 x i32> %a, <i32 24>
  %ashr = ashr <1 x i32> %shl, <i32 24>
  ret <1 x i32> %ashr
}

; CHECK-LABEL: @sext_in_reg_i8_to_v1i64
; CHECK: ibit_extract
; CHECK: ishr
; CHECK: ret_dyn
define <1 x i64> @sext_in_reg_i8_to_v1i64(<1 x i64> %a) nounwind {
  %shl = shl <1 x i64> %a, <i64 56>
  %ashr = ashr <1 x i64> %shl, <i64 56>
  ret <1 x i64> %ashr
}

; CHECK-LABEL: @sext_in_reg_i1_in_i8_other_amount
; CHECK: ishl
; CHECK: ibit_extract
; CHECK: ishr
define i8 @sext_in_reg_i1_in_i8_other_amount(i8 %a) nounwind {
  %x = shl i8 %a, 6
  %y = ashr i8 %x, 7
  ret i8 %y
}

; CHECK-LABEL: @sext_in_reg_v2i1_in_v2i8_other_amount
; CHECK: ishl
; CHECK: ibit_extract
; CHECK: ishr
define <2 x i8> @sext_in_reg_v2i1_in_v2i8_other_amount(<2 x i8> %a) nounwind {
  %x = shl <2 x i8> %a, <i8 6, i8 6>
  %y = ashr <2 x i8> %x, <i8 7, i8 7>
  ret <2 x i8> %y
}

