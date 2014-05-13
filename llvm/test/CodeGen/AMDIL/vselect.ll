; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @vselect_v2i32
; CHECK: ieq [[CONDREG:r[0-9]+]].xy__,
; CHECK-NEXT: cmov_logical r1.xy__, [[CONDREG]].xy
; CHECK-NEXT: ret_dyn
define <2 x i32> @vselect_v2i32(<2 x i32> %x, <2 x i32> %y) {
  %cmp = icmp eq <2 x i32> %x, zeroinitializer
  %result = select <2 x i1> %cmp, <2 x i32> %x, <2 x i32> %y
  ret <2 x i32> %result
}

; CHECK-LABEL: @vselect_v4i32
; CHECK: ieq [[CONDREG:r[0-9]+]],
; CHECK-NEXT: cmov_logical r1, [[CONDREG]], r1,
; CHECK-NEXT: ret_dyn
define <4 x i32> @vselect_v4i32(<4 x i32> %x, <4 x i32> %y) {
  %cmp = icmp eq <4 x i32> %x, zeroinitializer
  %result = select <4 x i1> %cmp, <4 x i32> %x, <4 x i32> %y
  ret <4 x i32> %result
}

; CHECK-LABEL: @vselect_v2f32
; CHECK: eq [[CONDREG:r[0-9]+]].xy__,
; CHECK-NEXT: cmov_logical r1.xy__, [[CONDREG]].xy
; CHECK-NEXT: ret_dyn
define <2 x float> @vselect_v2f32(<2 x float> %x, <2 x float> %y) {
  %cmp = fcmp eq <2 x float> %x, zeroinitializer
  %result = select <2 x i1> %cmp, <2 x float> %x, <2 x float> %y
  ret <2 x float> %result
}

; CHECK-LABEL: @vselect_v4f32
; CHECK: eq [[CONDREG:r[0-9]+]],
; CHECK-NEXT: cmov_logical r1, [[CONDREG]], r1,
; CHECK-NEXT: ret_dyn
define <4 x float> @vselect_v4f32(<4 x float> %x, <4 x float> %y) {
  %cmp = fcmp eq <4 x float> %x, zeroinitializer
  %result = select <4 x i1> %cmp, <4 x float> %x, <4 x float> %y
  ret <4 x float> %result
}

; CHECK-LABEL: @vselect_v2i64
; CHECK: i64eq [[CONDREG1:r[0-9]+]].xy__,
; CHECK: i64eq [[CONDREG2:r[0-9]+]].xy__,
; CHECK: iadd [[MERGECOND:r[0-9]+]], [[CONDREG2]].xy00, [[CONDREG1]].00xy
; CHECK: cmov_logical r1, [[MERGECOND]], r1, r2
; CHECK-NEXT: ret_dyn
define <2 x i64> @vselect_v2i64(<2 x i64> %x, <2 x i64> %y) {
  %cmp = icmp eq <2 x i64> %x, zeroinitializer
  %result = select <2 x i1> %cmp, <2 x i64> %x, <2 x i64> %y
  ret <2 x i64> %result
}

; CHECK-LABEL: @vselect_v4i64
; CHECK: i64eq
; CHECK: i64eq
; CHECK: cmov_logical
; CHECK: cmov_logical
define <4 x i64> @vselect_v4i64(<4 x i64> %x, <4 x i64> %y) {
  %cmp = icmp eq <4 x i64> %x, zeroinitializer
  %result = select <4 x i1> %cmp, <4 x i64> %x, <4 x i64> %y
  ret <4 x i64> %result
}

; CHECK-LABEL: @vselect_v2f64
; CHECK: deq [[CONDREG1:r[0-9]+]].xy__,
; CHECK: deq [[CONDREG2:r[0-9]+]].xy__,
; CHECK: iadd [[MERGECOND:r[0-9]+]], [[CONDREG2]].xy00, [[CONDREG1]].00xy
; CHECK: cmov_logical r1, [[MERGECOND]], r1, r2
; CHECK-NEXT: ret_dyn
define <2 x  double> @vselect_v2f64(<2 x  double> %x, <2 x  double> %y) {
  %cmp = fcmp eq <2 x  double> %x, zeroinitializer
  %result = select <2 x i1> %cmp, <2 x  double> %x, <2 x  double> %y
  ret <2 x  double> %result
}

; CHECK-LABEL: @vselect_v4f64
; CHECK: deq
; CHECK: deq
; CHECK: cmov_logical
; CHECK: cmov_logical
define <4 x  double> @vselect_v4f64(<4 x  double> %x, <4 x  double> %y) {
  %cmp = fcmp eq <4 x  double> %x, zeroinitializer
  %result = select <4 x i1> %cmp, <4 x  double> %x, <4 x  double> %y
  ret <4 x  double> %result
}

