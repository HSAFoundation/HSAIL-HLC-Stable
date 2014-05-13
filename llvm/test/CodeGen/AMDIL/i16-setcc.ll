; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @icmp_slt_i16_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000010,
; CHECK: ibit_extract {{r[0-9]+}}.x___, [[LIT_WIDTH]], [[LIT_OFFSET]], r1.{{[xy]}}
; CHECK-NEXT: ibit_extract {{r[0-9]+}}.x___, [[LIT_WIDTH]], [[LIT_OFFSET]], r1.{{[xy]}}
; CHECK-NEXT: ilt {{r[0-9]+}}.x___,
define i1 @icmp_slt_i16_arg(i16 %a, i16 %b) nounwind {
  %cmp = icmp slt i16 %a, %b
  ret i1 %cmp
}

; CHECK-LABEL: @icmp_ult_i16_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000010,
; CHECK: ubit_extract {{r[0-9]+}}.x___, [[LIT_WIDTH]], [[LIT_OFFSET]], r1.{{[xy]}}
; CHECK-NEXT: ubit_extract {{r[0-9]+}}.x___, [[LIT_WIDTH]], [[LIT_OFFSET]], r1.{{[xy]}}
; CHECK-NEXT: ult {{r[0-9]+}}.x___,
define i1 @icmp_ult_i16_arg(i16 %a, i16 %b) nounwind {
  %cmp = icmp ult i16 %a, %b
  ret i1 %cmp
}

; CHECK-LABEL: @icmp_slt_v2i16_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000010,
; CHECK: ibit_extract {{r[0-9]+}}.xy__,
; CHECK: ibit_extract {{r[0-9]+}}.xy__,
; CHECK: ilt {{r[0-9]+}}.xy__,
define <2 x i1> @icmp_slt_v2i16_arg(<2 x i16> %a, <2 x i16> %b) nounwind {
  %cmp = icmp slt <2 x i16> %a, %b
  ret <2 x i1> %cmp
}

; CHECK-LABEL: @icmp_ult_v2i16_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000010,
; CHECK: ubit_extract {{r[0-9]+}}.xy__,
; CHECK: ubit_extract {{r[0-9]+}}.xy__,
define <2 x i1> @icmp_ult_v2i16_arg(<2 x i16> %a, <2 x i16> %b) nounwind {
  %cmp = icmp ult <2 x i16> %a, %b
  ret <2 x i1> %cmp
}

; CHECK-LABEL: @icmp_slt_v4i16_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000010,
; CHECK: ibit_extract {{r[0-9]+}},
; CHECK: ibit_extract {{r[0-9]+}},
; CHECK: ilt {{r[0-9]+}},
define <4 x i1> @icmp_slt_v4i16_arg(<4 x i16> %a, <4 x i16> %b) nounwind {
  %cmp = icmp slt <4 x i16> %a, %b
  ret <4 x i1> %cmp
}

; CHECK-LABEL: @icmp_ult_v4i16_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000010,
; CHECK: ubit_extract {{r[0-9]+}},
; CHECK: ubit_extract {{r[0-9]+}},
; CHECK: ult {{r[0-9]+}},
define <4 x i1> @icmp_ult_v4i16_arg(<4 x i16> %a, <4 x i16> %b) nounwind {
  %cmp = icmp ult <4 x i16> %a, %b
  ret <4 x i1> %cmp
}

; CHECK-LABEL: @icmp_ult_i16_arg_zext
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK: ult
define i16 @icmp_ult_i16_arg_zext(i16 %a, i16 %b) nounwind {
  %cmp = icmp ult i16 %a, %b
  %ext = zext i1 %cmp to i16
  ret i16 %ext
}

; CHECK-LABEL: @icmp_ult_v2i16_arg_zext
define <2 x i16> @icmp_ult_v2i16_arg_zext(<2 x i16> %a, <2 x i16> %b) nounwind {
  %cmp = icmp ult <2 x i16> %a, %b
  %ext = zext <2 x i1> %cmp to <2 x i16>
  ret <2 x i16> %ext
}

; CHECK-LABEL: @icmp_ult_i16_arg_sext
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK: ult
define i16 @icmp_ult_i16_arg_sext(i16 %a, i16 %b) nounwind {
  %cmp = icmp ult i16 %a, %b
  %ext = sext i1 %cmp to i16
  ret i16 %ext
}

; CHECK-LABEL: @icmp_ult_v2i16_arg_sext
define <2 x i16> @icmp_ult_v2i16_arg_sext(<2 x i16> %a, <2 x i16> %b) nounwind {
  %cmp = icmp ult <2 x i16> %a, %b
  %ext = sext <2 x i1> %cmp to <2 x i16>
  ret <2 x i16> %ext
}


; CHECK-LABEL: @icmp_ult_v4i16_arg_sext
define <4 x i16> @icmp_ult_v4i16_arg_sext(<4 x i16> %a, <4 x i16> %b) nounwind {
  %cmp = icmp ult <4 x i16> %a, %b
  %ext = sext <4 x i1> %cmp to <4 x i16>
  ret <4 x i16> %ext
}

; CHECK-LABEL: @select_icmp_slt_i16_arg
; CHECK: ibit_extract
; CHECK: ibit_extract
; CHECK: ilt
; CHECK: cmov_logical
define i16 @select_icmp_slt_i16_arg(i16 %a, i16 %b) nounwind {
  %cmp = icmp slt i16 %a, %b
  %select = select i1 %cmp, i16 %a, i16 %b
  ret i16 %select
}

; CHECK-LABEL: @select_icmp_ult_i16_arg
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK: ult
; CHECK: cmov_logical
define i16 @select_icmp_ult_i16_arg(i16 %a, i16 %b) nounwind {
  %cmp = icmp ult i16 %a, %b
  %select = select i1 %cmp, i16 %a, i16 %b
  ret i16 %select
}

; CHECK-LABEL: @select_icmp_slt_v2i16_arg
; CHECK: ibit_extract
; CHECK: ibit_extract
; CHECK: ilt
; CHECK: cmov_logical
define <2 x i16> @select_icmp_slt_v2i16_arg(<2 x i16> %a, <2 x i16> %b, i16 %c, i16 %d) nounwind {
  %cmp = icmp slt <2 x i16> %a, %b
  %select = select <2 x i1> %cmp, <2 x i16> %a, <2 x i16> %b
  ret <2 x i16> %select
}

; CHECK-LABEL: @select_icmp_slt_v4i16_arg
; CHECK: ibit_extract
; CHECK: ibit_extract
; CHECK: ilt
; CHECK: cmov_logical
define <4 x i16> @select_icmp_slt_v4i16_arg(<4 x i16> %a, <4 x i16> %b, i16 %c, i16 %d) nounwind {
  %cmp = icmp slt <4 x i16> %a, %b
  %select = select <4 x i1> %cmp, <4 x i16> %a, <4 x i16> %b
  ret <4 x i16> %select
}

; CHECK-LABEL: @select_icmp_ult_v2i16_arg
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK: ult
; CHECK: cmov_logical
define <2 x i16> @select_icmp_ult_v2i16_arg(<2 x i16> %a, <2 x i16> %b, i16 %c, i16 %d) nounwind {
  %cmp = icmp ult <2 x i16> %a, %b
  %select = select <2 x i1> %cmp, <2 x i16> %a, <2 x i16> %b
  ret <2 x i16> %select
}

; CHECK-LABEL: @select_icmp_ult_v4i16_arg
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK: ult
; CHECK: cmov_logical
define <4 x i16> @select_icmp_ult_v4i16_arg(<4 x i16> %a, <4 x i16> %b, i16 %c, i16 %d) nounwind {
  %cmp = icmp ult <4 x i16> %a, %b
  %select = select <4 x i1> %cmp, <4 x i16> %a, <4 x i16> %b
  ret <4 x i16> %select
}

; CHECK-LABEL: @icmp_eq_select_i16_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000010,
; CHECK: ubit_extract {{r[0-9]+}}.x___, [[LIT_WIDTH]], [[LIT_OFFSET]], r1.{{[xy]}}
; CHECK-NEXT: ieq {{r[0-9]+}}.x___,
define i16 @icmp_eq_select_i16_arg(i16 %x) nounwind {
  %cmp = icmp eq i16 %x, 0
  %result = select i1 %cmp, i16 0, i16 %x
  ret i16 %result
}

; CHECK-LABEL: @icmp_sge_select_i16_arg
; CHECK: dcl_literal [[LIT_OFFSET:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LIT_WIDTH:l[0-9]+]], 0x00000010,
; CHECK: ibit_extract {{r[0-9]+}}.x___, [[LIT_WIDTH]], [[LIT_OFFSET]], r1.{{[xy]}}
; CHECK: ibit_extract {{r[0-9]+}}.x___, [[LIT_WIDTH]], [[LIT_OFFSET]], r1.{{[xy]}}
; CHECK-NEXT: ige {{r[0-9]+}}.x___,
define i16 @icmp_sge_select_i16_arg(i16 %x, i16 %y) nounwind {
  %cmp = icmp sge i16 %x, %y
  %result = select i1 %cmp, i16 0, i16 %x
  ret i16 %result
}
