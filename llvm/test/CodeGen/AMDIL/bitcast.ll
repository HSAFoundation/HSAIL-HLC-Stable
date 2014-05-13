; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @bc_f32_to_i32
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define i32 @bc_f32_to_i32(float %in) {
  %out = bitcast float %in to i32
  ret i32 %out
}

; CHECK-LABEL: @bc_i32_to_f32
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define float @bc_i32_to_f32(i32 %in) {
  %out = bitcast i32 %in to float
  ret float %out
}

; CHECK-LABEL: @bc_f64_to_i64
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define i64 @bc_f64_to_i64(double %in) {
  %out = bitcast double %in to i64
  ret i64 %out
}

; CHECK-LABEL: @bc_i64_to_f64
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define double @bc_i64_to_f64(i64 %in) {
  %out = bitcast i64 %in to double
  ret double %out
}

; CHECK-LABEL: @bc_v2i32_to_i64
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define i64 @bc_v2i32_to_i64(<2 x i32> %in) {
  %out = bitcast <2 x i32> %in to i64
  ret i64 %out
}

; CHECK-LABEL: @bc_i64_to_v2i32
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define <2 x i32> @bc_i64_to_v2i32(i64 %in) {
  %out = bitcast i64 %in to <2 x i32>
  ret <2 x i32> %out
}

; CHECK-LABEL: @bc_v2f32_to_i64
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define i64 @bc_v2f32_to_i64(<2 x float> %in) {
  %out = bitcast <2 x float> %in to i64
  ret i64 %out
}

; CHECK-LABEL: @bc_i64_to_v2f32
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define <2 x float> @bc_i64_to_v2f32(i64 %in) {
  %out = bitcast i64 %in to <2 x float>
  ret <2 x float> %out
}

; CHECK-LABEL: @bc_v2i32_to_f64
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define double @bc_v2i32_to_double(<2 x i32> %in) {
  %out = bitcast <2 x i32> %in to double
  ret double %out
}

; CHECK-LABEL: @bc_double_to_v2i32
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define <2 x i32> @bc_double_to_v2i32(double %in) {
  %out = bitcast double %in to <2 x i32>
  ret <2 x i32> %out
}

; CHECK-LABEL: @bc_v2i32_to_v4i16
define <4 x i16> @bc_v2i32_to_v4i16(<2 x i32> %in) {
  %out = bitcast <2 x i32> %in to <4 x i16>
  ret <4 x i16> %out
}

; CHECK-LABEL: @bc_v2i32_to_v8i8
define <8 x i8> @bc_v2i32_to_v8i8(<2 x i32> %in) {
  %out = bitcast <2 x i32> %in to <8 x i8>
  ret <8 x i8> %out
}

; CHECK-LABEL: @bc_v8i8_to_v2i32
define <2 x i32> @bc_v8i8_to_v2i32(<8 x i8> %in) {
  %out = bitcast <8 x i8> %in to <2 x i32>
  ret <2 x i32> %out
}

; CHECK-LABEL: @bc_v4i16_to_v2i32
define <2 x i32> @bc_v4i16_to_v2i32(<4 x i16> %in) {
  %out = bitcast <4 x i16> %in to <2 x i32>
  ret <2 x i32> %out
}

; CHECK-LABEL: @bc_v4i8_to_i32
define i32 @bc_v4i8_to_i32(<4 x i8> %in) {
  %out = bitcast <4 x i8> %in to i32
  ret i32 %out
}

; CHECK-LABEL: @bc_i32_to_v4i8
define <4 x i8> @bc_i32_to_v4i8(i32 %in) {
  %out = bitcast i32 %in to <4 x i8>
  ret <4 x i8> %out
}

; CHECK-LABEL: @bc_v4i16_to_i64
define i64 @bc_v4i16_to_i64(<4 x i16> %in) {
  %out = bitcast <4 x i16> %in to i64
  ret i64 %out
}

; CHECK-LABEL: @bc_i64_to_v4i16
define <4 x i16> @bc_i64_to_v4i16(i64 %in) {
  %out = bitcast i64 %in to <4 x i16>
  ret <4 x i16> %out
}

; CHECK-LABEL: @bc_v2i8_to_i16
define i16 @bc_v2i8_to_i16(<2 x i8> %in) {
  %out = bitcast <2 x i8> %in to i16
  ret i16 %out
}

; CHECK-LABEL: @bc_i16_to_v2i8
define <2 x i8> @bc_i16_to_v2i8(i16 %in) {
  %out = bitcast i16 %in to <2 x i8>
  ret <2 x i8> %out
}

; CHECK-LABEL: @bc_v2i64_to_v4i32
define <2 x i64> @bc_v2i64_to_v4i32(<4 x i32> %in) {
  %out = bitcast <4 x i32> %in to <2 x i64>
  ret <2 x i64> %out
}

; CHECK-LABEL: @bc_v4i32_to_v2i64
define <4 x i32> @bc_v4i32_to_v2i64(<2 x i64> %in) {
  %out = bitcast <2 x i64> %in to <4 x i32>
  ret <4 x i32> %out
}

; CHECK-LABEL: @bc_v8i8_to_i64
define i64 @bc_v8i8_to_i64(<8 x i8> %in) {
  %out = bitcast <8 x i8> %in to i64
  ret i64 %out
}

; CHECK-LABEL: @bc_i64_to_v8i8
define <8 x i8> @bc_i64_to_v8i8(i64 %in) {
  %out = bitcast i64 %in to <8 x i8>
  ret <8 x i8> %out
}

; CHECK-LABEL: @bc_v8i16_to_v2i64
define <2 x i64> @bc_v8i16_to_v2i64(<8 x i16> %in) {
  %out = bitcast <8 x i16> %in to <2 x i64>
  ret <2 x i64> %out
}

; CHECK-LABEL: @bc_v2i64_to_v8i16
define <8 x i16> @bc_v2i64_to_v8i16(<2 x i64> %in) {
  %out = bitcast <2 x i64> %in to <8 x i16>
  ret <8 x i16> %out
}

; CHECK-LABEL: @bc_v16i8_to_v2i64
define <2 x i64> @bc_v16i8_to_v2i64(<16 x i8> %in) {
  %out = bitcast <16 x i8> %in to <2 x i64>
  ret <2 x i64> %out
}

; CHECK-LABEL: @bc_v2i64_to_v16i8
define <16 x i8> @bc_v2i64_to_v16i8(<2 x i64> %in) {
  %out = bitcast <2 x i64> %in to <16 x i8>
  ret <16 x i8> %out
}

; CHECK-LABEL: @bc_v16i8_to_v4i32
define <4 x i32> @bc_v16i8_to_v16i8(<16 x i8> %in) {
  %out = bitcast <16 x i8> %in to <4 x i32>
  ret <4 x i32> %out
}

; CHECK-LABEL: @bc_v4i32_to_v16i8
define <16 x i8> @bc_v4i32_to_v16i8(<4 x i32> %in) {
  %out = bitcast <4 x i32> %in to <16 x i8>
  ret <16 x i8> %out
}

; CHECK-LABEL: @bc_v8i16_to_v4i32
define <4 x i32> @bc_v8i16_to_v4i32(<8 x i16> %in) {
  %out = bitcast <8 x i16> %in to <4 x i32>
  ret <4 x i32> %out
}

; CHECK-LABEL: @bc_v4i32_to_v8i16
define <8 x i16> @bc_v4i32_to_v8i16(<4 x i32> %in) {
  %out = bitcast <4 x i32> %in to <8 x i16>
  ret <8 x i16> %out
}

; CHECK-LABEL: @bc_v16i8_to_v8i16
define <8 x i16> @bc_v16i8_to_v8i16(<16 x i8> %in) {
  %out = bitcast <16 x i8> %in to <8 x i16>
  ret <8 x i16> %out
}

; CHECK-LABEL: @bc_v8i16_to_v16i8
define <16 x i8> @bc_v8i16_to_v16i8(<8 x i16> %in) {
  %out = bitcast <8 x i16> %in to <16 x i8>
  ret <16 x i8> %out
}

; CHECK-LABEL: @bc_v2f64_to_v4i32
define <2 x double> @bc_v2f64_to_v4i32(<4 x i32> %in) {
  %out = bitcast <4 x i32> %in to <2 x double>
  ret <2 x double> %out
}

; CHECK-LABEL: @bc_v4i32_to_v2f64
define <4 x i32> @bc_v4i32_to_v2f64(<2 x double> %in) {
  %out = bitcast <2 x double> %in to <4 x i32>
  ret <4 x i32> %out
}

; CHECK-LABEL: @bc_f64_to_v4i16
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define <4 x i16> @bc_f64_to_v4i16(double %in) {
  %out = bitcast double %in to <4 x i16>
  ret <4 x i16> %out
}

; CHECK-LABEL: @bc_v4i16_to_f64
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define double @bc_v4i16_to_f64(<4 x i16> %in) {
  %out = bitcast <4 x i16> %in to double
  ret double %out
}

; CHECK-LABEL: @bc_f64_to_v8i16
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define <8 x i16> @bc_f64_to_v8i16(<2 x double> %in) {
  %out = bitcast <2 x double> %in to <8 x i16>
  ret <8 x i16> %out
}

; CHECK-LABEL: @bc_v8i16_to_f64
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define <2 x double> @bc_v8i16_to_f64(<8 x i16> %in) {
  %out = bitcast <8 x i16> %in to <2 x double>
  ret <2 x double> %out
}

; CHECK-LABEL: @bc_f64_to_v16i8
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define <16 x i8> @bc_f64_to_v16i8(<2 x double> %in) {
  %out = bitcast <2 x double> %in to <16 x i8>
  ret <16 x i8> %out
}

; CHECK-LABEL: @bc_v16i8_to_f64
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define <2 x double> @bc_v16i8_to_f64(<16 x i8> %in) {
  %out = bitcast <16 x i8> %in to <2 x double>
  ret <2 x double> %out
}

; CHECK-LABEL: @bc_v2i16_to_i32
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define i32 @bc_v2i16_to_i32(<2 x i16> %in) {
  %out = bitcast <2 x i16> %in to i32
  ret i32 %out
}

; CHECK-LABEL: @bc_i32_to_v2i16
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define <2 x i16> @bc_i32_to_v2i16(i32 %in) {
  %out = bitcast i32 %in to <2 x i16>
  ret <2 x i16> %out
}

; CHECK-LABEL: @bc_v2i16_to_f32
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define float @bc_v2i16_to_f32(<2 x i16> %in) {
  %out = bitcast <2 x i16> %in to float
  ret float %out
}

; CHECK-LABEL: @bc_f32_to_v2i16
; CHECK: func {{[0-9]+}}
; CHECK-NEXT: ret_dyn
define <2 x i16> @bc_f32_to_v2i16(float %in) {
  %out = bitcast float %in to <2 x i16>
  ret <2 x i16> %out
}

; CHECK-LABEL: @bc_v4i8_to_f32
define float @bc_v4i8_to_f32(<4 x i8> %in) {
  %out = bitcast <4 x i8> %in to float
  ret float %out
}

; CHECK-LABEL: @bc_f32_to_v4i8
define <4 x i8> @bc_f32_to_v4i8(float %in) {
  %out = bitcast float %in to <4 x i8>
  ret <4 x i8> %out
}

; CHECK-LABEL: @bc_v8i8_to_f64
define double @bc_v8i8_to_f64(<8 x i8> %in) {
  %out = bitcast <8 x i8> %in to double
  ret double %out
}

; CHECK-LABEL: @bc_f64_to_v8i8
define <8 x i8> @bc_f64_to_v8i8(double %in) {
  %out = bitcast double %in to <8 x i8>
  ret <8 x i8> %out
}

; CHECK-LABEL: @bc_v4i8_to_v2i16
define <2 x i16> @bc_v4i8_to_v2i16(<4 x i8> %in) {
  %out = bitcast <4 x i8> %in to <2 x i16>
  ret <2 x i16> %out
}

; CHECK-LABEL: @bc_v2i16_to_v4i8
define <4 x i8> @bc_v2i16_to_v4i8(<2 x i16> %in) {
  %out = bitcast <2 x i16> %in to <4 x i8>
  ret <4 x i8> %out
}


