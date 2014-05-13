; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; Check that the calling conventions work for various types

; CHECK-LABEL: func {{[0-9]+}} ; i32_mul
; CHECK: imul r1.x___
; CHECK-NEXT: ret_dyn
define internal fastcc i32 @i32_mul(i32 %x, i32 %y) nounwind readonly noinline {
  %tmp1 = mul i32 %x, %y
  ret i32 %tmp1
}

define void @__OpenCL_test_kernel_mul_i32(i32 addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call i32 @i32_mul(i32 2, i32 3) nounwind
  store i32 %call, i32 addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; v2i32_mul
; CHECK: imul r1.xy__
; CHECK-NEXT: ret_dyn
define internal fastcc <2 x i32> @v2i32_mul(<2 x i32> %x, <2 x i32> %y) nounwind readonly noinline {
  %tmp1 = mul <2 x i32> %x, %y
  ret <2 x i32> %tmp1
}

define void @__OpenCL_test_kernel_mul_v2i32(<2 x i32> addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call <2 x i32> @v2i32_mul(<2 x i32> <i32 2, i32 4>, <2 x i32> <i32 3, i32 5>) nounwind
  store <2 x i32> %call, <2 x i32> addrspace(1)* %out, align 4
  ret void
}


; CHECK-LABEL: func {{[0-9]+}} ; v4i32_mul
; CHECK: imul r1,
; CHECK-NEXT: ret_dyn
define internal fastcc <4 x i32> @v4i32_mul(<4 x i32> %x, <4 x i32> %y) nounwind readonly noinline {
  %tmp1 = mul <4 x i32> %x, %y
  ret <4 x i32> %tmp1
}

define void @__OpenCL_test_kernel_mul_v4i32(<4 x i32> addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call <4 x i32> @v4i32_mul(<4 x i32> <i32 2, i32 4, i32 2, i32 1>, <4 x i32> <i32 3, i32 5, i32 1, i32 1>) nounwind
  store <4 x i32> %call, <4 x i32> addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; f32_mul
; CHECK: mul_ieee r1.x___
; CHECK-NEXT: ret_dyn
define internal fastcc float @f32_mul(float %x, float %y) nounwind readonly noinline {
  %tmp1 = fmul float %x, %y
  ret float %tmp1
}

define void @__OpenCL_test_kernel_mul_f32(float addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call float @f32_mul(float 4.0, float 5.0) nounwind
  store float %call, float addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; v2f32_mul
; CHECK: mul_ieee r1.xy__
; CHECK-NEXT: ret_dyn
define internal fastcc <2 x float> @v2f32_mul(<2 x float> %x, <2 x float> %y) nounwind readonly noinline {
  %tmp1 = fmul <2 x float> %x, %y
  ret <2 x float> %tmp1
}

define void @__OpenCL_test_kernel_mul_v2f32(<2 x float> addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call <2 x float> @v2f32_mul(<2 x float> <float 2.0, float 4.0>, <2 x float> <float 3.0, float 5.0>) nounwind
  store <2 x float> %call, <2 x float> addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; v4f32_mul
; CHECK: mul_ieee r1,
; CHECK-NEXT: ret_dyn
define internal fastcc <4 x float> @v4f32_mul(<4 x float> %x, <4 x float> %y) nounwind readonly noinline {
  %tmp1 = fmul <4 x float> %x, %y
  ret <4 x float> %tmp1
}

define void @__OpenCL_test_kernel_mul_v4f32(<4 x float> addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call <4 x float> @v4f32_mul(<4 x float> <float 2.0, float 4.0, float 1.0, float 1.0>, <4 x float> <float 3.0, float 5.0, float 8.0, float 1.0>) nounwind
  store <4 x float> %call, <4 x float> addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; f64_mul
; CHECK: dmul r1.xy__
; CHECK-NEXT: ret_dyn
define internal fastcc double @f64_mul(double %x, double %y) nounwind readonly noinline {
  %tmp1 = fmul double %x, %y
  ret double %tmp1
}

define void @__OpenCL_test_kernel_mul_f64(double addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call double @f64_mul(double 4.0, double 8.0) nounwind
  store double %call, double addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; v2f64_mul
; CHECK: dmul {{r[0-9]+}}.xy__,
; CHECK: dmul {{r[0-9]+}}.xy__,
; CHECK: iadd r1, {{r[0-9]+}}.xy00, {{r[0-9]+}}.00xy
; CHECK-NEXT: ret_dyn
define internal fastcc <2 x double> @v2f64_mul(<2 x double> %x, <2 x double> %y) nounwind readonly noinline {
  %tmp1 = fmul <2 x double> %x, %y
  ret <2 x double> %tmp1
}

define void @__OpenCL_test_kernel_mul_v2f64(<2 x double> addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call <2 x double> @v2f64_mul(<2 x double> <double 2.0, double 4.0>, <2 x double> <double 3.0, double 5.0>) nounwind
  store <2 x double> %call, <2 x double> addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; i64_mul
; CHECK: i64mul r1.xy__
; CHECK-NEXT: ret_dyn
define internal fastcc i64 @i64_mul(i64 %x, i64 %y) nounwind readonly noinline {
  %tmp1 = mul i64 %x, %y
  ret i64 %tmp1
}

define void @__OpenCL_test_kernel_mul_i64(i64 addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call i64 @i64_mul(i64 4, i64 8) nounwind
  store i64 %call, i64 addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; v2i64_mul
; CHECK: i64mul {{r[0-9]+}}.xy__,
; CHECK: i64mul {{r[0-9]+}}.xy__,
; CHECK: iadd r1, {{r[0-9]+}}.xy00, {{r[0-9]+}}.00xy
; CHECK-NEXT: ret_dyn
define internal fastcc <2 x i64> @v2i64_mul(<2 x i64> %x, <2 x i64> %y) nounwind readonly noinline {
  %tmp1 = mul <2 x i64> %x, %y
  ret <2 x i64> %tmp1
}

define void @__OpenCL_test_kernel_mul_v2i64(<2 x i64> addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call <2 x i64> @v2i64_mul(<2 x i64> <i64 2, i64 4>, <2 x i64> <i64 3, i64 5>) nounwind
  store <2 x i64> %call, <2 x i64> addrspace(1)* %out, align 16
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; i1_xor
; CHECK: ixor r1.x___
; CHECK-NEXT: ret_dyn
define internal fastcc i1 @i1_xor(i1 %x) nounwind readonly noinline {
  %y = xor i1 %x, true
  ret i1 %y
}

define void @__OpenCL_test_kernel_xor_i1(i32 addrspace(1)* noalias nocapture %out, i32 %x) nounwind {
  %y = icmp eq i32 %x, 0
  %tmp2 = tail call i1 @i1_xor(i1 %y) nounwind readonly
  %tmp3 = sext i1 %tmp2 to i32
  store i32 %tmp3, i32 addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; v2i1xor
; CHECK: ixor r1.xy__,
; CHECK-NEXT: ret_dyn
define internal fastcc <2 x i1> @v2i1xor(<2 x i1> %x) nounwind readonly noinline {
  %y = xor <2 x i1> %x, <i1 true, i1 false>
  ret <2 x i1> %y
}

define void @__OpenCL_test_kernel_xor_v2i1(<2 x i32> addrspace(1)* noalias nocapture %out, <2 x i32> %x) nounwind {
  %y = icmp eq <2 x i32> %x, zeroinitializer
  %tmp2 = tail call <2 x i1> @v2i1xor(<2 x i1> %y) nounwind readonly
  %tmp3 = sext <2 x i1> %tmp2 to <2 x i32>
  store <2 x i32> %tmp3, <2 x i32> addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; v4i1xor
; CHECK: ixor r1,
; CHECK-NEXT: ret_dyn
define internal fastcc <4 x i1> @v4i1xor(<4 x i1> %x) nounwind readonly noinline {
  %y = xor <4 x i1> %x, <i1 true, i1 false, i1 true, i1 false>
  ret <4 x i1> %y
}

define void @__OpenCL_test_kernel_xor_v4i1(<4 x i32> addrspace(1)* noalias nocapture %out, <4 x i32> %x) nounwind {
  %y = icmp eq <4 x i32> %x, zeroinitializer
  %tmp2 = tail call <4 x i1> @v4i1xor(<4 x i1> %y) nounwind readonly
  %tmp3 = sext <4 x i1> %tmp2 to <4 x i32>
  store <4 x i32> %tmp3, <4 x i32> addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; i16_mul
; CHECK: imul r1.x___
; CHECK-NEXT: ret_dyn
define internal fastcc i16 @i16_mul(i16 %x, i16 %y) nounwind readonly noinline {
  %tmp1 = mul i16 %x, %y
  ret i16 %tmp1
}

define void @__OpenCL_test_kernel_mul_i16(i16 addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call i16 @i16_mul(i16 4, i16 3) nounwind
  store i16 %call, i16 addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; v2i16_mul
; CHECK: imul r1.xy__
; CHECK-NEXT: ret_dyn
define internal fastcc <2 x i16> @v2i16_mul(<2 x i16> %x, <2 x i16> %y) nounwind readonly noinline {
  %tmp1 = mul <2 x i16> %x, %y
  ret <2 x i16> %tmp1
}

define void @__OpenCL_test_kernel_mul_v2i16(<2 x i16> addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call <2 x i16> @v2i16_mul(<2 x i16> <i16 2, i16 4>, <2 x i16> <i16 3, i16 5>) nounwind
  store <2 x i16> %call, <2 x i16> addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; v4i16_mul
; CHECK: imul r1,
; CHECK-NEXT: ret_dyn
define internal fastcc <4 x i16> @v4i16_mul(<4 x i16> %x, <4 x i16> %y) nounwind readonly noinline {
  %tmp1 = mul <4 x i16> %x, %y
  ret <4 x i16> %tmp1
}

define void @__OpenCL_test_kernel_mul_v4i16(<4 x i16> addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call <4 x i16> @v4i16_mul(<4 x i16> <i16 2, i16 4, i16 2, i16 1>, <4 x i16> <i16 3, i16 5, i16 1, i16 1>) nounwind
  store <4 x i16> %call, <4 x i16> addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; v8i16_mul
; CHECK: imul r1,
; CHECK: imul r2,
; CHECK-NEXT: ret_dyn
define internal fastcc <8 x i16> @v8i16_mul(<8 x i16> %x, <8 x i16> %y) nounwind readonly noinline {
  %tmp1 = mul <8 x i16> %x, %y
  ret <8 x i16> %tmp1
}

define void @__OpenCL_test_kernel_mul_v8i16(<8 x i16> addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call <8 x i16> @v8i16_mul(<8 x i16> <i16 2, i16 4, i16 2, i16 1, i16 2, i16 4, i16 2, i16 1>, <8 x i16> <i16 3, i16 5, i16 1, i16 1, i16 2, i16 4, i16 2, i16 1>) nounwind
  store <8 x i16> %call, <8 x i16> addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; i8_mul
; CHECK: ibit_extract
; CHECK: ibit_extract
; CHECK: imul r1.x___
; CHECK-NEXT: ret_dyn
define internal fastcc i8 @i8_mul(i8 %x, i8 %y) nounwind readonly noinline {
  %tmp1 = mul i8 %x, %y
  ret i8 %tmp1
}

define void @__OpenCL_test_kernel_mul_i8(i8 addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call i8 @i8_mul(i8 5, i8 7) nounwind
  store i8 %call, i8 addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: @__OpenCL_test_kernel_mul_load_i8
; CHECK: uav_byte_load_id
; CHECK: uav_byte_load_id
; CHECK: ret_dyn
define void @__OpenCL_test_kernel_mul_load_i8(i8 addrspace(1)* noalias nocapture %out, i8 addrspace(1)* noalias nocapture %aptr, i8 addrspace(1)* noalias nocapture %bptr) nounwind {
  %a = load i8 addrspace(1)* %aptr
  %b = load i8 addrspace(1)* %bptr
  %call = tail call i8 @i8_mul(i8 %a, i8 %b) nounwind
  store i8 %call, i8 addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; v2i8_mul
; CHECK: imul r1.xy__
; CHECK-NEXT: ret_dyn
define internal fastcc <2 x i8> @v2i8_mul(<2 x i8> %x, <2 x i8> %y) nounwind readonly noinline {
  %tmp1 = mul <2 x i8> %x, %y
  ret <2 x i8> %tmp1
}

define void @__OpenCL_test_kernel_mul_v2i8(<2 x i8> addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call <2 x i8> @v2i8_mul(<2 x i8> <i8 2, i8 4>, <2 x i8> <i8 3, i8 5>) nounwind
  store <2 x i8> %call, <2 x i8> addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: func {{[0-9]+}} ; v4i8_mul
; CHECK: imul r1,
; CHECK-NEXT: ret_dyn
define internal fastcc <4 x i8> @v4i8_mul(<4 x i8> %x, <4 x i8> %y) nounwind readonly noinline {
  %tmp1 = mul <4 x i8> %x, %y
  ret <4 x i8> %tmp1
}

define void @__OpenCL_test_kernel_mul_v4i8(<4 x i8> addrspace(1)* noalias nocapture %out) nounwind {
  %call = tail call <4 x i8> @v4i8_mul(<4 x i8> <i8 2, i8 4, i8 2, i8 1>, <4 x i8> <i8 3, i8 5, i8 1, i8 1>) nounwind
  store <4 x i8> %call, <4 x i8> addrspace(1)* %out, align 4
  ret void
}

; XCHECK-LABEL: func {{[0-9]+}} ; half_mul
; XCHECK: imul r1.xy__
; XCHECK-NEXT: ret_dyn
; define internal fastcc half @half_mul(half %x, half %y) nounwind readonly noinline {
;   %tmp1 = fmul half %x, %y
;   ret half %tmp1
; }

; define void @__OpenCL_test_kernel_mul_half(half addrspace(1)* noalias nocapture %out) nounwind {
;   %call = tail call half @half_mul(half 2.0, half 4.0) nounwind
;   store half %call, half addrspace(1)* %out, align 4
;   ret void
; }

