; RUN: llc -asm-verbose=0 -march=amdil < %s | FileCheck %s

declare i32 @__amdil_bfi_i32(i32, i32, i32) nounwind readnone

; CHECK-LABEL: @bfi_intrinsic
; CHECK: func {{.*}}
; CHECK-NEXT: bfi
; CHECK-NEXT: ret_dyn
define i32 @bfi_intrinsic(i32 %src0, i32 %src1, i32 %src2) nounwind {
  %result = call i32 @__amdil_bfi_i32(i32 %src0, i32 %src1, i32 %src2) nounwind readnone
  ret i32 %result
}

; CHECK-LABEL: @good_bfi_0
; CHECK: func {{.*}}
; CHECK-NEXT: bfi
; CHECK-NEXT: ret_dyn
define i32 @good_bfi_0(i32 %src0, i32 %src1, i32 %src2) nounwind {
  %lhs = and i32 %src0, %src1
  %src0_xor_m1 = xor i32 %src0, -1
  %rhs = and i32 %src0_xor_m1, %src2
  %result = or i32 %lhs, %rhs
  ret i32 %result
}

; CHECK-LABEL: @good_bfi_1
; CHECK: func
; CHECK: ior [[REGOR:r[0-9]+]].x___,
; CHECK: ixor [[REGXOR:r[0-9]+]].x___,
; CHECK: bfi {{r[0-9]+}}.x___,  [[REGOR]].x, [[REGXOR]].x,
; CHECK-NEXT: ret_dyn
define i32 @good_bfi_1(i32 %a, i32 %b, i32 %c) nounwind {
  %a_or_b = or i32 %a, %b ; (a | b -> src0)

  %xor_c_m1 = xor i32 %c, -1 ;  ; (c ^ 1) -> src1
  %lhs = and i32 %a_or_b, %xor_c_m1

  %xor_a_or_b = xor i32 %a_or_b, -1 ; c -> src2
  %rhs = and i32 %c, %xor_a_or_b

  %result = or i32 %lhs, %rhs
  ret i32 %result
}

; Have a second xor input
; CHECK-LABEL: @good_bfi_2
; CHECK: func
define i32 @good_bfi_2(i32 %a, i32 %b, i32 %c) nounwind {
  %a_xor_b = xor i32 %a, %b
  %xor_c_m1 = xor i32 %c, -1
  %lhs = and i32 %a_xor_b, %xor_c_m1

  %a_or_b_1 = or i32 %a, %b
  %xor_a_or_b = xor i32 %a_or_b_1, -1
  %rhs = and i32 %c, %xor_a_or_b

  %result = or i32 %lhs, %rhs
  ret i32 %result
}

; CHECK-LABEL: @good_bfi_3
; CHECK: func
; CHECK: ixor [[SRC0:r[0-9]+]].x___,
; CHECK: ixor [[SRC1:r[0-9]+]].x___,
; CHECK: bfi {{r[0-9]+}}.x___, [[SRC0]].x, [[SRC1]].x,
; CHECK: ret_dyn

; src0 -> a xor b
; src1 -> xor c, -1
; src2 -> c
define i32 @good_bfi_3(i32 %a, i32 %b, i32 %c) nounwind {
  %a_xor_b = xor i32 %a, %b
  %xor_c_m1 = xor i32 %c, -1
  %lhs = and i32 %a_xor_b, %xor_c_m1

  %xor_a_xor_b = xor i32 %a_xor_b, -1
  %rhs = and i32 %c, %xor_a_xor_b

  %result = or i32 %lhs, %rhs
  ret i32 %result
}

; bfi instruction is this form:
;  bfi src0, src1, src2 = (src0 & src1) | (~src0 & src2)

; The sequence folded into this was:
; RESULT: or LHS, RHS
;   LHS: i32 = and (and A, B), (xor C, -1) -> says src0 = (and A, B)
;   RHS: i32 = and C, (xor (or A, B), -1) -> says src0 = (or A, B)
;
; The LHS and RHS don't have a consistent value for src0, so this
; doesn't work. There is another required xor.

; CHECK-LABEL: @bad_bfi
; CHECK: func
; CHECK: iand [[SRC2:r[0-9]+]].x___,
; CHECK: ior [[AORB:r[0-9]+]].x___,
; CHECK: ixor [[SRC1:r[0-9]+]].x___, [[AORB:r[0-9]+]].x, l
; CHECK: bfi {{r[0-9]+}}.x___, r1.z, [[SRC1]].x, [[SRC2]].x
; CHECK-NEXT: ret_dyn
; src0 -> c
; src1 -> xor (a | b), -1
; src2 -> a & b
define i32 @bad_bfi(i32 %a, i32 %b, i32 %c) nounwind {
  %a_and_b = and i32 %a, %b
  %xor_c_m1 = xor i32 %c, -1
  %lhs = and i32 %a_and_b, %xor_c_m1

  %a_or_b = or i32 %a, %b
  %xor_a_or_b = xor i32 %a_or_b, -1
  %rhs = and i32 %c, %xor_a_or_b

  %result = or i32 %lhs, %rhs
  ret i32 %result
}

; The comment says this turns into 2 BFIs, but it doesn't. Is that
; correct?
; CHECK-LABEL: @case_bfi_cl823628
define i32 @case_bfi_cl823628(i32 %a, i32 %b, i32 %c, i32 %d) nounwind {
  %a_and_b = and i32 %a, %b
  %c_and_d = and i32 %c, %d
  %result = or i32 %a_and_b, %c_and_d
  ret i32 %result
}
