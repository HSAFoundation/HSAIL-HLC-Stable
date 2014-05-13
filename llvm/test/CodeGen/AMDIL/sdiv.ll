; RUN: llc -march=amdil < %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

define <4 x i32> @v4i32_sdiv(<4 x i32> %a, <4 x i32> %b) nounwind readnone {
; CHECK: @v4i32_sdiv
; CHECK: ilt {{r[0-9]+}}, r
; CHECK: iadd
; CHECK: ixor
; CHECK: udiv {{r[0-9]+}},
; CHECK: ixor
; CHECK: iadd
; CHECK: ixor
; CHECK: ret_dyn
  %result = sdiv <4 x i32> %a, %b
  ret <4 x i32> %result
}

define <2 x i32> @v2i32_sdiv(<2 x i32> %a, <2 x i32> %b) nounwind readnone {
; CHECK: @v2i32_sdiv
; CHECK: ilt {{r[0-9]+}}.xy__, r
; CHECK: iadd
; CHECK: ixor
; CHECK: udiv {{r[0-9]+}}.xy__,
; CHECK: ixor
; CHECK: iadd
; CHECK: ixor
; CHECK: ret_dyn
  %result = sdiv <2 x i32> %a, %b
  ret <2 x i32> %result
}

define i32 @i32_sdiv(i32 %a, i32 %b) nounwind readnone {
; CHECK: @i32_sdiv
; CHECK: ilt {{r[0-9]+}}.x___, r
; CHECK: iadd
; CHECK: ixor
; CHECK: udiv {{r[0-9]+}}.x___,
; CHECK: ixor
; CHECK: iadd
; CHECK: ixor
; CHECK: ret_dyn
  %result = sdiv i32 %a, %b
  ret i32 %result
}
