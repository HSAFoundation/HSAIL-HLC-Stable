; XFAIL: *

; xfail for now since the current checks rely on a later patch to
; remove select on bool values

; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @icmp_false_i32_const
; CHECK: dcl_literal [[ZERO:l[0-9]+]], 0x00000000
; CHECK-NEXT: mov r1.x___, [[ZERO]]
; CHECK-NEXT: ret_dyn
define i32 @icmp_false_i32_const(i32 %x) nounwind {
  %cmp = icmp false i32 %x, 1
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_eq_i32_const
; CHECK: dcl_literal [[ONE:l[0-9]+]], 0x00000001
; CHECK-NEXT: ieq r1.x___, r1.x, [[ONE]]
; CHECK-NEXT: ret_dyn
define i32 @icmp_eq_i32_const(i32 %x) nounwind {
  %cmp = icmp eq i32 %x, 1
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_sgt_i32_const
; CHECK: ilt [[REG:r[0-9]+]].x___,
; CHECK: dcl_literal [[LITZERO:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LITONE:l[0-9]+]], 0x00000001,
; CHECK: cmov_logical r{{[0-9]+}}.x___, [[REG]].x, [[LITONE]], [[ZERO]]
define i32 @icmp_sgt_i32_const(i32 %x) nounwind {
  %cmp = icmp sgt i32 %x, 1
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_sge_i32_const
; CHECK: ige
define i32 @icmp_sge_i32_const(i32 %x) nounwind {
  %cmp = icmp sge i32 %x, 1
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_slt_i32_const
; CHECK: ilt
define i32 @icmp_slt_i32_const(i32 %x) nounwind {
  %cmp = icmp slt i32 %x, 1
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_sle_i32_const
; CHECK: ilt [[REG:r[0-9]+]].x___,
; CHECK: dcl_literal [[LITZERO:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LITONE:l[0-9]+]], 0x00000001,
; CHECK: cmov_logical r{{[0-9]+}}.x___, [[REG]].x, [[LITONE]], [[ZERO]]
define i32 @icmp_sle_i32_const(i32 %x) nounwind {
  %cmp = icmp sle i32 %x, 1
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_ugt_i32_const
; CHECK: ult [[REG:r[0-9]+]].x___,
; CHECK: dcl_literal [[LITZERO:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LITONE:l[0-9]+]], 0x00000001,
; CHECK: cmov_logical r{{[0-9]+}}.x___, [[REG]].x, [[LITONE]], [[ZERO]]
define i32 @icmp_ugt_i32_const(i32 %x) nounwind {
  %cmp = icmp ugt i32 %x, 1
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_ult_i32_const
; CHECK: ieq
define i32 @icmp_ult_i32_const(i32 %x) nounwind {
  %cmp = icmp ult i32 %x, 1
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_ule_i32_const
; CHECK: uge
; CHECK: cmov_logical
define i32 @icmp_ule_i32_const(i32 %x) nounwind {
  %cmp = icmp ule i32 %x, 1
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_true_i32_const
; CHECK: dcl_literal [[NEGONE:l[0-9]+]], 0xFFFFFFFF,
; CHECK-NEXT: mov r1.x___, [[NEGONE]]
; CHECK-NEXT: ret_dyn
define i32 @icmp_true_i32_const(i32 %x) nounwind {
  %cmp = icmp true i32 %x, 1
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_false_i32
; CHECK: dcl_literal [[ZERO:l[0-9]+]], 0x00000000
; CHECK-NEXT: mov r1.x___, [[ZERO]]
; CHECK-NEXT: ret_dyn
define i32 @icmp_false_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp false i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_eq_i32
; CHECK: ieq
define i32 @icmp_eq_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp eq i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_sgt_i32
; CHECK: ilt
; CHECK: cmov_logical
define i32 @icmp_sgt_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp sgt i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_sge_i32
; CHECK: ige
define i32 @icmp_sge_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp sge i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_slt_i32
; CHECK: ilt
define i32 @icmp_slt_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp slt i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_sle_i32
; CHECK: ige
; CHECK: cmov_logical
define i32 @icmp_sle_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp sle i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_one_i32
define i32 @icmp_one_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp one i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_ord_i32
define i32 @icmp_ord_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp ord i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_ueq_i32
define i32 @icmp_ueq_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp ueq i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_ugt_i32
define i32 @icmp_ugt_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp ugt i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_uge_i32
define i32 @icmp_uge_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp uge i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_ult_i32
define i32 @icmp_ult_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp ult i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_ule_i32
define i32 @icmp_ule_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp ule i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_une_i32
define i32 @icmp_une_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp une i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_uno_i32
define i32 @icmp_uno_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp uno i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

; CHECK-LABEL: @icmp_true_i32
; CHECK: dcl_literal [[NEGONE:l[0-9]+]], 0xFFFFFFFF,
; CHECK-NEXT: mov r1.x___, [[NEGONE]]
; CHECK-NEXT: ret_dyn
define i32 @icmp_true_i32(i32 %x, i32 %y) nounwind {
  %cmp = icmp true i32 %x, %y
  %sextcmp = sext i1 %cmp to i32
  ret i32 %sextcmp
}

