; XFAIL: *
; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; xfail for now since the current checks rely on a later patch to
; remove select on bool values

; CHECK-LABEL: @icmp_false_v4i32_const
; CHECK: dcl_literal [[ZERO:l[0-9]+]], 0x00000000
; CHECK-NEXT: mov r1., [[ZERO]]
; CHECK-NEXT: ret_dyn
define <4 x i32> @icmp_false_v4i32_const(<4 x i32> %x) nounwind {
  %cmp = icmp false <4 x i32> %x, <i32 1, i32 1, i32 1, i32 1>
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_eq_v4i32_const
; CHECK: dcl_literal [[ONE:l[0-9]+]], 0x00000001
; CHECK-NEXT: ieq r1., r1.x, [[ONE]]
; CHECK-NEXT: ret_dyn
define <4 x i32> @icmp_eq_v4i32_const(<4 x i32> %x) nounwind {
  %cmp = icmp eq <4 x i32> %x, <i32 1, i32 1, i32 1, i32 1>
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_sgt_v4i32_const
; CHECK: ilt [[REG:r[0-9]+]],
; CHECK: dcl_literal [[LITZERO:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LITONE:l[0-9]+]], 0x00000001,
; CHECK: cmov_logical r{{[0-9]+}}, [[REG]], [[LITONE]], [[ZERO]]
define <4 x i32> @icmp_sgt_v4i32_const(<4 x i32> %x) nounwind {
  %cmp = icmp sgt <4 x i32> %x, <i32 1, i32 1, i32 1, i32 1>
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_sge_v4i32_const
; CHECK: ige
define <4 x i32> @icmp_sge_v4i32_const(<4 x i32> %x) nounwind {
  %cmp = icmp sge <4 x i32> %x, <i32 1, i32 1, i32 1, i32 1>
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_slt_v4i32_const
; CHECK: ilt
define <4 x i32> @icmp_slt_v4i32_const(<4 x i32> %x) nounwind {
  %cmp = icmp slt <4 x i32> %x, <i32 1, i32 1, i32 1, i32 1>
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_sle_v4i32_const
; CHECK: ilt [[REG:r[0-9]+]],
; CHECK: dcl_literal [[LITZERO:l[0-9]+]], 0x00000000, 0x00000000, 0x00000000, 0x00000000
; CHECK: dcl_literal [[LITONE:l[0-9]+]], 0x00000001, 0x00000001, 0x00000001, 0x00000001
; CHECK: cmov_logical r{{[0-9]+}}, [[REG]], [[LITONE]], [[ZERO]]
define <4 x i32> @icmp_sle_v4i32_const(<4 x i32> %x) nounwind {
  %cmp = icmp sle <4 x i32> %x, <i32 1, i32 1, i32 1, i32 1>
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_ugt_v4i32_const
; CHECK: ult [[REG:r[0-9]+]],
; CHECK: dcl_literal [[LITZERO:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LITONE:l[0-9]+]], 0x00000001,
; CHECK: cmov_logical r{{[0-9]+}}, [[REG]], [[LITONE]], [[ZERO]]
define <4 x i32> @icmp_ugt_v4i32_const(<4 x i32> %x) nounwind {
  %cmp = icmp ugt <4 x i32> %x, <i32 1, i32 1, i32 1, i32 1>
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_ult_v4i32_const
; CHECK: ieq
define <4 x i32> @icmp_ult_v4i32_const(<4 x i32> %x) nounwind {
  %cmp = icmp ult <4 x i32> %x, <i32 1, i32 1, i32 1, i32 1>
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_ule_v4i32_const
; CHECK: uge
; CHECK: cmov_logical
define <4 x i32> @icmp_ule_v4i32_const(<4 x i32> %x) nounwind {
  %cmp = icmp ule <4 x i32> %x, <i32 1, i32 1, i32 1, i32 1>
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_true_v4i32_const
; CHECK: dcl_literal [[NEGONE:l[0-9]+]], 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
; CHECK-NEXT: mov r1, [[NEGONE]]
; CHECK-NEXT: ret_dyn
define <4 x i32> @icmp_true_v4i32_const(<4 x i32> %x) nounwind {
  %cmp = icmp true <4 x i32> %x, <i32 1, i32 1, i32 1, i32 1>
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_false_v4i32
; CHECK: dcl_literal [[ZERO:l[0-9]+]], 0x00000000, 0x00000000
; CHECK-NEXT: mov r1.xy__, [[ZERO]]
; CHECK-NEXT: ret_dyn
define <4 x i32> @icmp_false_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp false <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_eq_v4i32
; CHECK: ieq
define <4 x i32> @icmp_eq_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp eq <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_sgt_v4i32
; CHECK: ilt
; CHECK: cmov_logical
define <4 x i32> @icmp_sgt_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp sgt <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_sge_v4i32
; CHECK: ige
define <4 x i32> @icmp_sge_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp sge <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_slt_v4i32
; CHECK: ilt
define <4 x i32> @icmp_slt_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp slt <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_sle_v4i32
; CHECK: ige
; CHECK: cmov_logical
define <4 x i32> @icmp_sle_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp sle <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_one_v4i32
define <4 x i32> @icmp_one_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp one <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_ord_v4i32
define <4 x i32> @icmp_ord_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp ord <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_ueq_v4i32
define <4 x i32> @icmp_ueq_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp ueq <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_ugt_v4i32
define <4 x i32> @icmp_ugt_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp ugt <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_uge_v4i32
define <4 x i32> @icmp_uge_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp uge <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_ult_v4i32
define <4 x i32> @icmp_ult_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp ult <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_ule_v4i32
define <4 x i32> @icmp_ule_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp ule <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_une_v4i32
define <4 x i32> @icmp_une_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp une <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_uno_v4i32
define <4 x i32> @icmp_uno_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp uno <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

; CHECK-LABEL: @icmp_true_v4i32
; CHECK: dcl_literal [[NEGONE:l[0-9]+]], 0xFFFFFFFF, 0xFFFFFFFF,
; CHECK-NEXT: mov r1.xy__, [[NEGONE]]
; CHECK-NEXT: ret_dyn
define <4 x i32> @icmp_true_v4i32(<4 x i32> %x, <4 x i32> %y) nounwind {
  %cmp = icmp true <4 x i32> %x, %y
  %sextcmp = sext <4 x i1> %cmp to <4 x i32>
  ret <4 x i32> %sextcmp
}

