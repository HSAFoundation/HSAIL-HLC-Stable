; RUN: llc -march=amdil -mcpu=cypress -mattr=+fp64 %s -o - | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

; Test cases where the setcc type differs from the type of the vselect
define <2 x i16> @test_vselect_v2i16(<2 x i16> %a, <2 x i16> %b, <2 x float> %c) nounwind {
; CHECK: eq r{{[0-9]+}}.xy__, r{{[0-9]+}}.xyxy, r{{[0-9]+}}.xyxy

; TODO:
; mov r66.xy__, l13
; iand r65.xy__, r65.xyxy, r66.xyxy
; mov r66.xy__, l14
; ishl r65.xy__, r65.xyxy, r66.xyxy
; ishr r65.xy__, r65.xyxy, r66.xyxy
; mov r66.x___, l13
; mov r66.xy__, r66.x
; iand r65.xy__, r65.xyxy, r66.xyxy
; mov r65.xy__, r65.xyxy
; mov r66.x___, l15
; mov r66.xy__, r66.x
; ishl r65.xy__, r65.xyxy, r66.xyxy
; ishr r65.xy__, r65.xyxy, r66.xyxy
; mov r66.xy__, r65.xyxy
; iand r65.xy__, r1.xyxy, r66.xyxy
; mov r67.xy__, l16
; ixor r66.xy__, r66.xyxy, r67.xyxy
; iand r66.xy__, r1.zwzw, r66.xyxy
; ior r65.xy__, r65.xyxy, r66.xyxy
; mov r1.xy__, r65.xyxy
; ret_dyn

entry:
  %cond = fcmp oeq <2 x float> %c, zeroinitializer
  %result = select <2 x i1> %cond, <2 x i16> %a, <2 x i16> %b
  ret <2 x i16> %result
}

define <4 x i8> @test_vselect_v4i8(<4 x i8> %a, <4 x i8> %b, <4 x float> %c) nounwind {
; TODO:
entry:
  %cond = fcmp oeq <4 x float> %c, zeroinitializer
  %result = select <4 x i1> %cond, <4 x i8> %a, <4 x i8> %b
  ret <4 x i8> %result
}

define <2 x i8> @test_vselect_v2i8(<2 x i8> %a, <2 x i8> %b, <2 x float> %c) nounwind {
; TODO:
entry:
  %cond = fcmp oeq <2 x float> %c, zeroinitializer
  %result = select <2 x i1> %cond, <2 x i8> %a, <2 x i8> %b
  ret <2 x i8> %result
}

define <3 x float> @test_vselect_v3f32(<3 x float> %a, <3 x float> %b, <3 x float> %c) nounwind {
; TODO:
entry:
  %cond = fcmp oeq <3 x float> %c, zeroinitializer
  %result = select <3 x i1> %cond, <3 x float> %a, <3 x float> %b
  ret <3 x float> %result
}

define <2 x double> @test_vselect_v2f64(<2 x double> %a, <2 x double> %b, <2 x double> %c) nounwind {
; TODO:
entry:
  %cond = fcmp oeq <2 x double> %c, zeroinitializer
  %result = select <2 x i1> %cond, <2 x double> %a, <2 x double> %b
  ret <2 x double> %result
}

define <3 x double> @test_vselect_v3f64(<3 x double> %a, <3 x double> %b, <3 x double> %c) nounwind {
; TODO:
entry:
  %cond = fcmp oeq <3 x double> %c, zeroinitializer
  %result = select <3 x i1> %cond, <3 x double> %a, <3 x double> %b
  ret <3 x double> %result
}

define <4 x double> @test_vselect_v4f64(<4 x double> %a, <4 x double> %b, <4 x double> %c) nounwind {
; TODO:
entry:
  %cond = fcmp oeq <4 x double> %c, zeroinitializer
  %result = select <4 x i1> %cond, <4 x double> %a, <4 x double> %b
  ret <4 x double> %result
}
