; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s
; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @cmp_false_f64
define i32 @cmp_false_f64_const(double %x) nounwind {
  %cmp = fcmp false double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_oeq_f64
define i32 @cmp_oeq_f64_const(double %x) nounwind {
  %cmp = fcmp oeq double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ogt_f64
define i32 @cmp_ogt_f64_const(double %x) nounwind {
  %cmp = fcmp ogt double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_oge_f64
define i32 @cmp_oge_f64_const(double %x) nounwind {
  %cmp = fcmp oge double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_olt_f64
define i32 @cmp_olt_f64_const(double %x) nounwind {
  %cmp = fcmp olt double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ole_f64
define i32 @cmp_ole_f64_const(double %x) nounwind {
  %cmp = fcmp ole double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_one_f64
define i32 @cmp_one_f64_const(double %x) nounwind {
  %cmp = fcmp one double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ord_f64
define i32 @cmp_ord_f64_const(double %x) nounwind {
  %cmp = fcmp ord double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ueq_f64
define i32 @cmp_ueq_f64_const(double %x) nounwind {
  %cmp = fcmp ueq double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ugt_f64
define i32 @cmp_ugt_f64_const(double %x) nounwind {
  %cmp = fcmp ugt double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_uge_f64
define i32 @cmp_uge_f64_const(double %x) nounwind {
  %cmp = fcmp uge double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ult_f64
define i32 @cmp_ult_f64_const(double %x) nounwind {
  %cmp = fcmp ult double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ule_f64
define i32 @cmp_ule_f64_const(double %x) nounwind {
  %cmp = fcmp ule double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_une_f64
define i32 @cmp_une_f64_const(double %x) nounwind {
  %cmp = fcmp une double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_uno_f64
define i32 @cmp_uno_f64_const(double %x) nounwind {
  %cmp = fcmp uno double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_true_f64
; CHECK: dcl_literal [[NEGONE:l[0-9]+]], 0xFFFFFFFF,
; CHECK-NEXT: mov r1.x___, [[NEGONE]]
; CHECK-NEXT: ret_dyn
define i32 @cmp_true_f64_const(double %x) nounwind {
  %cmp = fcmp true double %x, 1.000000e+00
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_false_f64
; CHECK: dcl_literal [[ZERO:l[0-9]+]], 0x00000000
; CHECK-NEXT: mov r1.x___, [[ZERO]]
; CHECK-NEXT: ret_dyn
define i32 @cmp_false_f64(double %x, double %y) nounwind {
  %cmp = fcmp false double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_oeq_f64
define i32 @cmp_oeq_f64(double %x, double %y) nounwind {
  %cmp = fcmp oeq double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ogt_f64
define i32 @cmp_ogt_f64(double %x, double %y) nounwind {
  %cmp = fcmp ogt double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_oge_f64
define i32 @cmp_oge_f64(double %x, double %y) nounwind {
  %cmp = fcmp oge double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_olt_f64
define i32 @cmp_olt_f64(double %x, double %y) nounwind {
  %cmp = fcmp olt double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ole_f64
define i32 @cmp_ole_f64(double %x, double %y) nounwind {
  %cmp = fcmp ole double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_one_f64
define i32 @cmp_one_f64(double %x, double %y) nounwind {
  %cmp = fcmp one double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ord_f64
define i32 @cmp_ord_f64(double %x, double %y) nounwind {
  %cmp = fcmp ord double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ueq_f64
define i32 @cmp_ueq_f64(double %x, double %y) nounwind {
  %cmp = fcmp ueq double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ugt_f64
define i32 @cmp_ugt_f64(double %x, double %y) nounwind {
  %cmp = fcmp ugt double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_uge_f64
define i32 @cmp_uge_f64(double %x, double %y) nounwind {
  %cmp = fcmp uge double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ult_f64
define i32 @cmp_ult_f64(double %x, double %y) nounwind {
  %cmp = fcmp ult double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_ule_f64
define i32 @cmp_ule_f64(double %x, double %y) nounwind {
  %cmp = fcmp ule double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_une_f64
define i32 @cmp_une_f64(double %x, double %y) nounwind {
  %cmp = fcmp une double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_uno_f64
define i32 @cmp_uno_f64(double %x, double %y) nounwind {
  %cmp = fcmp uno double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @cmp_true_f64
; CHECK: dcl_literal [[NEGONE:l[0-9]+]], 0xFFFFFFFF,
; CHECK-NEXT: mov r1.x___, [[NEGONE]]
; CHECK-NEXT: ret_dyn
define i32 @cmp_true_f64(double %x, double %y) nounwind {
  %cmp = fcmp true double %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

