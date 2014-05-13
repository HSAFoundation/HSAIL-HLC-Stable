; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @icmp_slt_i8_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000008,
; CHECK: ibit_extract {{r[0-9]+}}.x___, [[LIT_WIDTH]], [[LIT_OFFSET]], r1.{{[xy]}}
; CHECK-NEXT: ibit_extract {{r[0-9]+}}.x___, [[LIT_WIDTH]], [[LIT_OFFSET]], r1.{{[xy]}}
; CHECK-NEXT: ilt {{r[0-9]+}}.x___,
define i1 @icmp_slt_i8_arg(i8 %a, i8 %b) nounwind {
  %cmp = icmp slt i8 %a, %b
  ret i1 %cmp
}

; CHECK-LABEL: @icmp_ult_i8_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000008,
; CHECK: ubit_extract {{r[0-9]+}}.x___, [[LIT_WIDTH]], [[LIT_OFFSET]], r1.{{[xy]}}
; CHECK-NEXT: ubit_extract {{r[0-9]+}}.x___, [[LIT_WIDTH]], [[LIT_OFFSET]], r1.{{[xy]}}
; CHECK-NEXT: ult {{r[0-9]+}}.x___,
define i1 @icmp_ult_i8_arg(i8 %a, i8 %b) nounwind {
  %cmp = icmp ult i8 %a, %b
  ret i1 %cmp
}

; CHECK-LABEL: @icmp_slt_v2i8_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000008,
; CHECK: ibit_extract {{r[0-9]+}}.xy__,
; CHECK: ibit_extract {{r[0-9]+}}.xy__,
; CHECK: ilt {{r[0-9]+}}.xy__,
define <2 x i1> @icmp_slt_v2i8_arg(<2 x i8> %a, <2 x i8> %b) nounwind {
  %cmp = icmp slt <2 x i8> %a, %b
  ret <2 x i1> %cmp
}

; CHECK-LABEL: @icmp_ult_v2i8_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000008,
; CHECK: ubit_extract {{r[0-9]+}}.xy__,
; CHECK: ubit_extract {{r[0-9]+}}.xy__,
define <2 x i1> @icmp_ult_v2i8_arg(<2 x i8> %a, <2 x i8> %b) nounwind {
  %cmp = icmp ult <2 x i8> %a, %b
  ret <2 x i1> %cmp
}

; CHECK-LABEL: @icmp_slt_v4i8_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000008,
; CHECK: ibit_extract {{r[0-9]+}},
; CHECK: ibit_extract {{r[0-9]+}},
; CHECK: ilt {{r[0-9]+}},
define <4 x i1> @icmp_slt_v4i8_arg(<4 x i8> %a, <4 x i8> %b) nounwind {
  %cmp = icmp slt <4 x i8> %a, %b
  ret <4 x i1> %cmp
}

; CHECK-LABEL: @icmp_ult_v4i8_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000008,
; CHECK: ubit_extract {{r[0-9]+}},
; CHECK: ubit_extract {{r[0-9]+}},
; CHECK: ult {{r[0-9]+}},
define <4 x i1> @icmp_ult_v4i8_arg(<4 x i8> %a, <4 x i8> %b) nounwind {
  %cmp = icmp ult <4 x i8> %a, %b
  ret <4 x i1> %cmp
}

; CHECK-LABEL: @icmp_ult_i8_arg_zext
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK: ult
define i8 @icmp_ult_i8_arg_zext(i8 %a, i8 %b) nounwind {
  %cmp = icmp ult i8 %a, %b
  %ext = zext i1 %cmp to i8
  ret i8 %ext
}

; CHECK-LABEL: @icmp_ult_v2i8_arg_zext
define <2 x i8> @icmp_ult_v2i8_arg_zext(<2 x i8> %a, <2 x i8> %b) nounwind {
  %cmp = icmp ult <2 x i8> %a, %b
  %ext = zext <2 x i1> %cmp to <2 x i8>
  ret <2 x i8> %ext
}

; CHECK-LABEL: @icmp_ult_i8_arg_sext
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK: ult
define i8 @icmp_ult_i8_arg_sext(i8 %a, i8 %b) nounwind {
  %cmp = icmp ult i8 %a, %b
  %ext = sext i1 %cmp to i8
  ret i8 %ext
}

; CHECK-LABEL: @icmp_ult_v2i8_arg_sext
define <2 x i8> @icmp_ult_v2i8_arg_sext(<2 x i8> %a, <2 x i8> %b) nounwind {
  %cmp = icmp ult <2 x i8> %a, %b
  %ext = sext <2 x i1> %cmp to <2 x i8>
  ret <2 x i8> %ext
}


; CHECK-LABEL: @icmp_ult_v4i8_arg_sext
define <4 x i8> @icmp_ult_v4i8_arg_sext(<4 x i8> %a, <4 x i8> %b) nounwind {
  %cmp = icmp ult <4 x i8> %a, %b
  %ext = sext <4 x i1> %cmp to <4 x i8>
  ret <4 x i8> %ext
}

; CHECK-LABEL: @select_icmp_slt_i8_arg
; CHECK: ibit_extract
; CHECK: ibit_extract
; CHECK: ilt
; CHECK: cmov_logical
define i8 @select_icmp_slt_i8_arg(i8 %a, i8 %b) nounwind {
  %cmp = icmp slt i8 %a, %b
  %select = select i1 %cmp, i8 %a, i8 %b
  ret i8 %select
}

; CHECK-LABEL: @select_icmp_ult_i8_arg
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK: ult
; CHECK: cmov_logical
define i8 @select_icmp_ult_i8_arg(i8 %a, i8 %b) nounwind {
  %cmp = icmp ult i8 %a, %b
  %select = select i1 %cmp, i8 %a, i8 %b
  ret i8 %select
}

; CHECK-LABEL: @select_icmp_slt_v2i8_arg
; CHECK: ibit_extract
; CHECK: ibit_extract
; CHECK: ilt
; CHECK: cmov_logical
define <2 x i8> @select_icmp_slt_v2i8_arg(<2 x i8> %a, <2 x i8> %b, i8 %c, i8 %d) nounwind {
  %cmp = icmp slt <2 x i8> %a, %b
  %select = select <2 x i1> %cmp, <2 x i8> %a, <2 x i8> %b
  ret <2 x i8> %select
}

; CHECK-LABEL: @select_icmp_slt_v4i8_arg
; CHECK: ibit_extract
; CHECK: ibit_extract
; CHECK: ilt
; CHECK: cmov_logical
define <4 x i8> @select_icmp_slt_v4i8_arg(<4 x i8> %a, <4 x i8> %b, i8 %c, i8 %d) nounwind {
  %cmp = icmp slt <4 x i8> %a, %b
  %select = select <4 x i1> %cmp, <4 x i8> %a, <4 x i8> %b
  ret <4 x i8> %select
}

; CHECK-LABEL: @select_icmp_ult_v2i8_arg
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK: ult
; CHECK: cmov_logical
define <2 x i8> @select_icmp_ult_v2i8_arg(<2 x i8> %a, <2 x i8> %b, i8 %c, i8 %d) nounwind {
  %cmp = icmp ult <2 x i8> %a, %b
  %select = select <2 x i1> %cmp, <2 x i8> %a, <2 x i8> %b
  ret <2 x i8> %select
}

; CHECK-LABEL: @select_icmp_ult_v4i8_arg
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK: ult
; CHECK: cmov_logical
define <4 x i8> @select_icmp_ult_v4i8_arg(<4 x i8> %a, <4 x i8> %b, i8 %c, i8 %d) nounwind {
  %cmp = icmp ult <4 x i8> %a, %b
  %select = select <4 x i1> %cmp, <4 x i8> %a, <4 x i8> %b
  ret <4 x i8> %select
}

; CHECK-LABEL: @icmp_eq_select_i8_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000008,
; CHECK: ubit_extract {{r[0-9]+}}.x___, [[LIT_WIDTH]], [[LIT_OFFSET]], r1.{{[xy]}}
; CHECK-NEXT: ieq {{r[0-9]+}}.x___,
define i8 @icmp_eq_select_i8_arg(i8 %x) nounwind {
  %cmp = icmp eq i8 %x, 0
  %result = select i1 %cmp, i8 0, i8 %x
  ret i8 %result
}

; CHECK-LABEL: @icmp_sge_select_i8_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000008,
; CHECK: ibit_extract {{r[0-9]+}}.x___, [[LIT_WIDTH]], [[LIT_OFFSET]], r1.{{[xy]}}
; CHECK: ibit_extract {{r[0-9]+}}.x___, [[LIT_WIDTH]], [[LIT_OFFSET]], r1.{{[xy]}}
; CHECK-NEXT: ige {{r[0-9]+}}.x___,
define i8 @icmp_sge_select_i8_arg(i8 %x, i8 %y) nounwind {
  %cmp = icmp sge i8 %x, %y
  %result = select i1 %cmp, i8 0, i8 %x
  ret i8 %result
}
