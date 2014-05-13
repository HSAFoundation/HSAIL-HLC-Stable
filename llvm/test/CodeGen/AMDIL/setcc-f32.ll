; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @cmp_false_f32_const
; CHECK: dcl_literal [[ZERO:l[0-9]+]], 0x00000000
; CHECK-NEXT: mov r1.x___, [[ZERO]]
; CHECK-NEXT: ret_dyn
define i32 @cmp_false_f32_const(float %x) nounwind {
  %cmp = fcmp false float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_oeq_f32_const
; CHECK: eq
define i32 @cmp_oeq_f32_const(float %x) nounwind {
  %cmp = fcmp oeq float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ogt_f32_const
; CHECK: lt
define i32 @cmp_ogt_f32_const(float %x) nounwind {
  %cmp = fcmp ogt float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_oge_f32_const
; CHECK: ge
define i32 @cmp_oge_f32_const(float %x) nounwind {
  %cmp = fcmp oge float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_olt_f32_const
; CHECK: lt
define i32 @cmp_olt_f32_const(float %x) nounwind {
  %cmp = fcmp olt float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ole_f32_const
; CHECK: ge
define i32 @cmp_ole_f32_const(float %x) nounwind {
  %cmp = fcmp ole float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_one_f32_const
; CHECK: ne
; CHECK: eq r{{[0-9]+}}.x___, r1.x, r1.x
; CHECK: iand
; CHECK: iand
define i32 @cmp_one_f32_const(float %x) nounwind {
  %cmp = fcmp one float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ord_f32_const
; CHECK: eq r{{[0-9]+}}.x___, r1.x, r1.x
define i32 @cmp_ord_f32_const(float %x) nounwind {
  %cmp = fcmp ord float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ueq_f32_const
; CHECK: ne r{{[0-9]+}}.x___, r1.x, r1.x
; CHECK: eq
; CHECK: ior
define i32 @cmp_ueq_f32_const(float %x) nounwind {
  %cmp = fcmp ueq float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ugt_f32_const
define i32 @cmp_ugt_f32_const(float %x) nounwind {
  %cmp = fcmp ugt float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_uge_f32_const
; CHECK: ne r{{[0-9]+}}.x___, r1.x, r1.x
; CHECK: ge
; CHECK: ior
define i32 @cmp_uge_f32_const(float %x) nounwind {
  %cmp = fcmp uge float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ult_f32_const
; CHECK: ne r{{[0-9]+}}.x___, r1.x, r1.x
; CHECK: lt
; CHECK: ior
define i32 @cmp_ult_f32_const(float %x) nounwind {
  %cmp = fcmp ult float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ule_f32_const
; CHECK: ne r{{[0-9]+}}.x___, r1.x, r1.x
; CHECK: ge
; CHECK: ior
define i32 @cmp_ule_f32_const(float %x) nounwind {
  %cmp = fcmp ule float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_une_f32_const
; CHECK: une
define i32 @cmp_une_f32_const(float %x) nounwind {
  %cmp = fcmp une float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_uno_f32_const
; CHECK: ne r{{[0-9]+}}.x___, r1.x, r1.x
define i32 @cmp_uno_f32_const(float %x) nounwind {
  %cmp = fcmp uno float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_true_f32_const
; CHECK: dcl_literal [[NEGONE:l[0-9]+]], 0xFFFFFFFF,
; CHECK-NEXT: mov r1.x___, [[NEGONE]]
; CHECK-NEXT: ret_dyn
define i32 @cmp_true_f32_const(float %x) nounwind {
  %cmp = fcmp true float %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_false_f32
; CHECK: dcl_literal [[ZERO:l[0-9]+]], 0x00000000
; CHECK-NEXT: mov r1.x___, [[ZERO]]
; CHECK-NEXT: ret_dyn
define i32 @cmp_false_f32(float %x, float %y) nounwind {
  %cmp = fcmp false float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_oeq_f32
; CHECK: eq
define i32 @cmp_oeq_f32(float %x, float %y) nounwind {
  %cmp = fcmp oeq float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ogt_f32
; CHECK: lt
define i32 @cmp_ogt_f32(float %x, float %y) nounwind {
  %cmp = fcmp ogt float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_oge_f32
; CHECK: ge
define i32 @cmp_oge_f32(float %x, float %y) nounwind {
  %cmp = fcmp oge float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_olt_f32
; CHECK: lt
define i32 @cmp_olt_f32(float %x, float %y) nounwind {
  %cmp = fcmp olt float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ole_f32
; CHECK: ge
define i32 @cmp_ole_f32(float %x, float %y) nounwind {
  %cmp = fcmp ole float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_one_f32
; CHECK: ne
; CHECK: eq r{{[0-9]+}}.x___, r1.y, r1.y
; CHECK: eq r{{[0-9]+}}.x___, r1.x, r1.x
; CHECK: iand
; CHECK: iand
define i32 @cmp_one_f32(float %x, float %y) nounwind {
  %cmp = fcmp one float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ord_f32
; CHECK: eq r{{[0-9]+}}.x___, r1.y, r1.y
; CHECK: eq r{{[0-9]+}}.x___, r1.x, r1.x
; CHECK: iand
define i32 @cmp_ord_f32(float %x, float %y) nounwind {
  %cmp = fcmp ord float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ueq_f32
; CHECK: eq
; CHECK: ne r{{[0-9]+}}.x___, r1.y, r1.y
; CHECK: ne r{{[0-9]+}}.x___, r1.x, r1.x
; CHECK: ior
; CHECK: ior
define i32 @cmp_ueq_f32(float %x, float %y) nounwind {
  %cmp = fcmp ueq float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ugt_f32
; CHECK: lt
; CHECK: ne r{{[0-9]+}}.x___, r1.y, r1.y
; CHECK: ne r{{[0-9]+}}.x___, r1.x, r1.x
; CHECK: ior
; CHECK: ior
define i32 @cmp_ugt_f32(float %x, float %y) nounwind {
  %cmp = fcmp ugt float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_uge_f32
; CHECK: ge
; CHECK: ne r{{[0-9]+}}.x___, r1.y, r1.y
; CHECK: ne r{{[0-9]+}}.x___, r1.x, r1.x
; CHECK: ior
; CHECK: ior
define i32 @cmp_uge_f32(float %x, float %y) nounwind {
  %cmp = fcmp uge float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ult_f32
; CHECK: lt
; CHECK: ne r{{[0-9]+}}.x___, r1.y, r1.y
; CHECK: ne r{{[0-9]+}}.x___, r1.x, r1.x
; CHECK: ior
; CHECK: ior
define i32 @cmp_ult_f32(float %x, float %y) nounwind {
  %cmp = fcmp ult float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ule_f32
; CHECK: ge
; CHECK: ne r{{[0-9]+}}.x___, r1.y, r1.y
; CHECK: ne r{{[0-9]+}}.x___, r1.x, r1.x
; CHECK: ior
; CHECK: ior
define i32 @cmp_ule_f32(float %x, float %y) nounwind {
  %cmp = fcmp ule float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_une_f32
; CHECK: ne
define i32 @cmp_une_f32(float %x, float %y) nounwind {
  %cmp = fcmp une float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_uno_f32
; CHECK: ne r{{[0-9]+}}.x___, r1.y, r1.y
; CHECK: ne r{{[0-9]+}}.x___, r1.x, r1.x
; CHECK: ior
define i32 @cmp_uno_f32(float %x, float %y) nounwind {
  %cmp = fcmp uno float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_true_f32
; CHECK: dcl_literal [[NEGONE:l[0-9]+]], 0xFFFFFFFF,
; CHECK-NEXT: mov r1.x___, [[NEGONE]]
; CHECK-NEXT: ret_dyn
define i32 @cmp_true_f32(float %x, float %y) nounwind {
  %cmp = fcmp true float %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}
