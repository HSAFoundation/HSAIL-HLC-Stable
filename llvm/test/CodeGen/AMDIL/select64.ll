; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; Don't need any 64-bit ops to get to a 64-bit setcc type.
; CHECK-LABEL: @select_i64_value_i32_cmp
; CHECK: dcl_literal [[OFFSETLIT:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[WIDTHLIT:l[0-9]+]], 0x00000001,
; CHECK: dcl_literal [[SHIFTLIT:l[0-9]+]], 0x0000001F,
; CHECK-NEXT: ine [[CMPREG:r[0-9]+]].x___,
; CHECK-NEXT: ibit_extract [[EXT32REG:r[0-9]+]].x___, [[WIDTHLIT]], [[OFFSETLIT]], [[CMPREG]].x
; CHECK-NEXT: ishr [[SHIFTREG:r[0-9]+]].x___, [[EXT32REG]].x, [[SHIFTLIT]]
; CHECK-NEXT: iadd [[EXT64REG:r[0-9]+]].xy__, [[EXT32REG]].x000, [[SHIFTREG]].0x00
; CHECK-NEXT: cmov_logical r1.xy__, [[EXT64REG]].x,
define i64 @select_i64_value_i32_cmp(i32 %a, i32 %b, i64 %c, i64 %d) nounwind {
  %cmp = icmp ne i32 %a, %b
  %y = select i1 %cmp, i64 %c, i64 %d
  ret i64 %y
}

; CHECK-LABEL: @select_v2i64_value_v2i32_cmp
; CHECK: ine [[CMPREG:r[0-9]+]].xy__,
; CHECK: ret_dyn
define <2 x i64> @select_v2i64_value_v2i32_cmp(<2 x i32> %a, <2 x i32> %b, <2 x i64> %c, <2 x i64> %d) nounwind {
  %cmp = icmp ne <2 x i32> %a, %b
  %y = select <2 x i1> %cmp, <2 x i64> %c, <2 x i64> %d
  ret <2 x i64> %y
}

