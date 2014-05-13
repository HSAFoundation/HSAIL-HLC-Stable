; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @select_i32
; CHECK: ieq [[CONDREG:r[0-9]+]].x___,
; CHECK-NEXT: cmov_logical r1.x___, [[CONDREG]].x,
define i32 @select_i32(i32 %x, i32 %y, i32 %c) {
  %cmp = icmp eq i32 %c, 0
  %result = select i1 %cmp, i32 %x, i32 %y
  ret i32 %result
}

; CHECK-LABEL: @select_v2i32
; CHECK: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LITM1:l[0-9]+]], 0xFFFFFFFF,
; CHECK: ieq [[CONDREG:r[0-9]+]].x___,
; CHECK-NEXT: cmov_logical [[CONDREG:r[0-9]+]].x___, [[CONDREG]].x, [[LITM1]], [[LIT0]]
; CHECK: iand
; CHECK: ixor
; CHECK: and
; CHECK: ior
define <2 x i32> @select_v2i32(<2 x i32> %x, <2 x i32> %y, i32 %c) {
  %cmp = icmp eq i32 %c, 0
  %result = select i1 %cmp, <2 x i32> %x, <2 x i32> %y
  ret <2 x i32> %result
}

; CHECK-LABEL: @select_v4i32
; CHECK: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LITM1:l[0-9]+]], 0xFFFFFFFF,
; CHECK: ieq [[CONDREG:r[0-9]+]].x___,
; CHECK-NEXT: cmov_logical [[CONDREG:r[0-9]+]].x___, [[CONDREG]].x, [[LITM1]], [[LIT0]]
; CHECK: iand
; CHECK: ixor
; CHECK: and
; CHECK: ior
define <4 x i32> @select_v4i32(<4 x i32> %x, <4 x i32> %y, i32 %c) {
  %cmp = icmp eq i32 %c, 0
  %result = select i1 %cmp, <4 x i32> %x, <4 x i32> %y
  ret <4 x i32> %result
}

; CHECK-LABEL: @select_f32
; CHECK: eq [[CONDREG:r[0-9]+]].x___,
; CHECK-NEXT: cmov_logical r1.x___, [[CONDREG]].x,
define float @select_f32(float %x, float %y, float %c) {
  %cmp = fcmp eq float %c, 0x0
  %result = select i1 %cmp, float %x, float %y
  ret float %result
}

; CHECK-LABEL: @select_v2f32
; CHECK: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LITM1:l[0-9]+]], 0xFFFFFFFF,
; CHECK: eq [[CONDREG:r[0-9]+]].x___,
; CHECK-NEXT: cmov_logical [[CONDREG:r[0-9]+]].x___, [[CONDREG]].x, [[LITM1]], [[LIT0]]
; CHECK: iand
; CHECK: ixor
; CHECK: and
; CHECK: ior
define <2 x float> @select_v2f32(<2 x float> %x, <2 x float> %y, float %c) {
  %cmp = fcmp eq float %c, 0x0
  %result = select i1 %cmp, <2 x float> %x, <2 x float> %y
  ret <2 x float> %result
}

; CHECK-LABEL: @select_v4f32
; CHECK: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LITM1:l[0-9]+]], 0xFFFFFFFF,
; CHECK: eq [[CONDREG:r[0-9]+]].x___,
; CHECK-NEXT: cmov_logical [[CONDREG:r[0-9]+]].x___, [[CONDREG]].x, [[LITM1]], [[LIT0]]
; CHECK: iand
; CHECK: ixor
; CHECK: and
; CHECK: ior
define <4 x float> @select_v4f32(<4 x float> %x, <4 x float> %y, float %c) {
  %cmp = fcmp eq float %c, 0x0
  %result = select i1 %cmp, <4 x float> %x, <4 x float> %y
  ret <4 x float> %result
}

; CHECK-LABEL: @select_i64
; CHECK: i64eq [[CONDREG:r[0-9]+]].xy__,
; CHECK-NEXT: cmov_logical r1.xy__, [[CONDREG]].x,
; CHECK-NEXT: ret_dyn
define i64 @select_i64(i64 %x, i64 %y, i64 %c) {
  %cmp = icmp eq i64 %c, 0
  %result = select i1 %cmp, i64 %x, i64 %y
  ret i64 %result
}

; CHECK-LABEL: @select_v2i64
; CHECK: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LITM1:l[0-9]+]], 0xFFFFFFFF,
; CHECK: i64eq [[CONDREG:r[0-9]+]].xy__,
; CHECK-NEXT: cmov_logical [[CONDREG:r[0-9]+]].xy__, [[CONDREG]].x, [[LITM1]], [[LIT0]]
; CHECK: iand
; CHECK: ixor
; CHECK: iand
; CHECK: ior
; CHECK: ret_dyn
define <2 x i64> @select_v2i64(<2 x i64> %x, <2 x i64> %y, i64 %c) {
  %cmp = icmp eq i64 %c, 0
  %result = select i1 %cmp, <2 x i64> %x, <2 x i64> %y
  ret <2 x i64> %result
}

; CHECK-LABEL: @select_v4i64
; CHECK: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LITM1:l[0-9]+]], 0xFFFFFFFF,
; CHECK: i64eq [[CONDREG:r[0-9]+]].xy__,
; CHECK-NEXT: cmov_logical [[CONDREG:r[0-9]+]].xy__, [[CONDREG]].x, [[LITM1]], [[LIT0]]
; CHECK: iand
; CHECK: ixor
; CHECK: iand
; CHECK: ior
; CHECK: iand
; CHECK: iand
; CHECK: ior
; CHECK: ret_dyn
define <4 x i64> @select_v4i64(<4 x i64> %x, <4 x i64> %y, i64 %c) {
  %cmp = icmp eq i64 %c, 0
  %result = select i1 %cmp, <4 x i64> %x, <4 x i64> %y
  ret <4 x i64> %result
}

; CHECK-LABEL: @select_f64
; CHECK: deq [[CONDREG:r[0-9]+]].xy__,
; CHECK-NEXT: cmov_logical r1.xy__, [[CONDREG]].x,
; CHECK-NEXT: ret_dyn
define double @select_f64(double %x, double %y, double %c) {
  %cmp = fcmp eq double %c, 0x0
  %result = select i1 %cmp, double %x, double %y
  ret double %result
}

; CHECK-LABEL: @select_v2f64
; CHECK: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LITM1:l[0-9]+]], 0xFFFFFFFF,
; CHECK: deq [[CONDREG:r[0-9]+]].xy__,
; CHECK-NEXT: cmov_logical [[CONDREG:r[0-9]+]].xy__, [[CONDREG]].x, [[LITM1]], [[LIT0]]
; CHECK: iand
; CHECK: ixor
; CHECK: iand
; CHECK: ior
; CHECK: ret_dyn
define <2 x double> @select_v2f64(<2 x double> %x, <2 x double> %y, double %c) {
  %cmp = fcmp eq double %c, 0x0
  %result = select i1 %cmp, <2 x double> %x, <2 x double> %y
  ret <2 x double> %result
}

; CHECK-LABEL: @select_v4f64
; CHECK: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK: dcl_literal [[LITM1:l[0-9]+]], 0xFFFFFFFF,
; CHECK: deq [[CONDREG:r[0-9]+]].xy__,
; CHECK-NEXT: cmov_logical [[CONDREG:r[0-9]+]].xy__, [[CONDREG]].x, [[LITM1]], [[LIT0]]
; CHECK: iand
; CHECK: ixor
; CHECK: iand
; CHECK: ior
; CHECK: iand
; CHECK: iand
; CHECK: ior
; CHECK: ret_dyn
define <4 x double> @select_v4f64(<4 x double> %x, <4 x double> %y, double %c) {
  %cmp = fcmp eq double %c, 0x0
  %result = select i1 %cmp, <4 x double> %x, <4 x double> %y
  ret <4 x double> %result
}

