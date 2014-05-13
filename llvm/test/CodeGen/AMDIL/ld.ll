; RUN: llc < %s -march=amdil | FileCheck %s

@array_i16 =  global [10 x i16] zeroinitializer
@array_constant_i16 =  addrspace(1) constant [10 x i16] zeroinitializer
@array_local_i16 =  addrspace(2) global [10 x i16] zeroinitializer
@array_shared_i16 =  addrspace(4) global [10 x i16] zeroinitializer
@array_i32 =  global [10 x i32] zeroinitializer
@array_constant_i32 =  addrspace(1) constant [10 x i32] zeroinitializer
@array_local_i32 =  addrspace(2) global [10 x i32] zeroinitializer
@array_shared_i32 =  addrspace(4) global [10 x i32] zeroinitializer
@array_i64 =  global [10 x i64] zeroinitializer
@array_constant_i64 =  addrspace(1) constant [10 x i64] zeroinitializer
@array_local_i64 =  addrspace(2) global [10 x i64] zeroinitializer
@array_shared_i64 =  addrspace(4) global [10 x i64] zeroinitializer
@array_float =  global [10 x float] zeroinitializer
@array_constant_float =  addrspace(1) constant [10 x float] zeroinitializer
@array_local_float =  addrspace(2) global [10 x float] zeroinitializer
@array_shared_float =  addrspace(4) global [10 x float] zeroinitializer
@array_double =  global [10 x double] zeroinitializer
@array_constant_double =  addrspace(1) constant [10 x double] zeroinitializer
@array_local_double =  addrspace(2) global [10 x double] zeroinitializer
@array_shared_double =  addrspace(4) global [10 x double] zeroinitializer

define i16 @t1_u16(i16* %p) {
  ; CHECK:      t1_u16
  ; CHECK:      mov r1010.x___, r1.x
  ; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
  ; CHECK-NEXT: iand r1008.x___, r1007.x, l13
  ; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
  ; CHECK-NEXT: iadd r1008, r1008.x, l14
  ; CHECK-NEXT: ieq r1008, r1008, l15
  ; CHECK-NEXT: mov r1011, x1[r1007.x]
  ; CHECK-NEXT: cmov_logical r1011.x___, r1008.y, r1011.y, r1011.x
  ; CHECK-NEXT: cmov_logical r1011.x___, r1008.z, r1011.z, r1011.x
  ; CHECK-NEXT: cmov_logical r1011.x___, r1008.w, r1011.w, r1011.x
  ; CHECK-NEXT: ishr r1007.x___, r1010.x, l16
  ; CHECK-NEXT: iand r1008.x___, r1007.x, l16
  ; CHECK-NEXT: ishr r1007.x___, r1011.x, l17
  ; CHECK-NEXT: cmov_logical r1011.x___, r1008.x, r1007.x, r1011.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %x = load i16* %p
  ret i16 %x
}

define i32 @t1_u32(i32* %p) {

  ; CHECK:      t1_u32
	; CHECK:      mov r1010.x___, r1.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: iadd r1008, r1008.x, l14
	; CHECK-NEXT: ieq r1008, r1008, l15
	; CHECK-NEXT: mov r1011, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.y, r1011.y, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.z, r1011.z, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.w, r1011.w, r1011.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %x = load i32* %p
  ret i32 %x
}

define i64 @t1_u64(i64* %p) {

  ; CHECK:      t1_u64
	; CHECK:      mov r1010.x___, r1.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: iadd r1008, r1008.x, l14
	; CHECK-NEXT: ieq r1008, r1008, l15
	; CHECK-NEXT: mov r1011, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1011.xy__, r1008.x, r1011.xyxy, r1011.zwzw
	; CHECK-NEXT: mov r65.xy__, r1011.xyxy
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %x = load i64* %p
  ret i64 %x
}

define float @t1_f32(float* %p) {

  ; CHECK:      t1_f32
	; CHECK:      mov r1010.x___, r1.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: iadd r1008, r1008.x, l14
	; CHECK-NEXT: ieq r1008, r1008, l15
	; CHECK-NEXT: mov r1011, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.y, r1011.y, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.z, r1011.z, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.w, r1011.w, r1011.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %x = load float* %p
  ret float %x
}

define double @t1_f64(double* %p) {


  ; CHECK:      t1_f64
	; CHECK:      mov r1010.x___, r1.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: iadd r1008, r1008.x, l14
	; CHECK-NEXT: ieq r1008, r1008, l15
	; CHECK-NEXT: mov r1011, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1011.xy__, r1008.x, r1011.xyxy, r1011.zwzw
	; CHECK-NEXT: mov r65.xy__, r1011.xyxy
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %x = load double* %p
  ret double %x
}

define i16 @t2_u16(i16* %p) {


  ; CHECK:      t2_u16
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: iadd r1008, r1008.x, l14
	; CHECK-NEXT: ieq r1008, r1008, l15
	; CHECK-NEXT: mov r1011, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.y, r1011.y, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.z, r1011.z, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.w, r1011.w, r1011.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l16
	; CHECK-NEXT: iand r1008.x___, r1007.x, l16
	; CHECK-NEXT: ishr r1007.x___, r1011.x, l17
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.x, r1007.x, r1011.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr i16* %p, i32 1
  %x = load i16* %i
  ret i16 %x
}

define i32 @t2_u32(i32* %p) {


  ; CHECK:      t2_u32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: iadd r1008, r1008.x, l15
	; CHECK-NEXT: ieq r1008, r1008, l16
	; CHECK-NEXT: mov r1011, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.y, r1011.y, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.z, r1011.z, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.w, r1011.w, r1011.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x


  %i = getelementptr i32* %p, i32 1
  %x = load i32* %i
  ret i32 %x
}

define i64 @t2_u64(i64* %p) {


  ; CHECK:      t2_u64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l14
	; CHECK-NEXT: iadd r1008, r1008.x, l15
	; CHECK-NEXT: ieq r1008, r1008, l16
	; CHECK-NEXT: mov r1011, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1011.xy__, r1008.x, r1011.xyxy, r1011.zwzw
	; CHECK-NEXT: mov r65.xy__, r1011.xyxy
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %i = getelementptr i64* %p, i32 1
  %x = load i64* %i
  ret i64 %x
}

define float @t2_f32(float* %p) {


  ; CHECK:      t2_f32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: iadd r1008, r1008.x, l15
	; CHECK-NEXT: ieq r1008, r1008, l16
	; CHECK-NEXT: mov r1011, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.y, r1011.y, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.z, r1011.z, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.w, r1011.w, r1011.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr float* %p, i32 1
  %x = load float* %i
  ret float %x
}

define double @t2_f64(double* %p) {


  ; CHECK:      t2_f64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l14
	; CHECK-NEXT: iadd r1008, r1008.x, l15
	; CHECK-NEXT: ieq r1008, r1008, l16
	; CHECK-NEXT: mov r1011, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1011.xy__, r1008.x, r1011.xyxy, r1011.zwzw
	; CHECK-NEXT: mov r65.xy__, r1011.xyxy
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %i = getelementptr double* %p, i32 1
  %x = load double* %i
  ret double %x
}

define i16 @t3_u16(i16* %p, i32 %q) {


  ; CHECK:      t3_u16
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: ishl r65.x___, r1.y, r65.x
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: iadd r1008, r1008.x, l15
	; CHECK-NEXT: ieq r1008, r1008, l16
	; CHECK-NEXT: mov r1011, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.y, r1011.y, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.z, r1011.z, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.w, r1011.w, r1011.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l12
	; CHECK-NEXT: ishr r1007.x___, r1011.x, l17
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.x, r1007.x, r1011.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr i16* %p, i32 %q
  %x = load i16* %i
  ret i16 %x
}

define i32 @t3_u32(i32* %p, i32 %q) {


  ; CHECK:      t3_u32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: ishl r65.x___, r1.y, r65.x
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: iadd r1008, r1008.x, l14
	; CHECK-NEXT: ieq r1008, r1008, l15
	; CHECK-NEXT: mov r1011, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.y, r1011.y, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.z, r1011.z, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.w, r1011.w, r1011.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr i32* %p, i32 %q
  %x = load i32* %i
  ret i32 %x
}

define i64 @t3_u64(i64* %p, i32 %q) {


  ; CHECK:      t3_u64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: ishl r65.x___, r1.y, r65.x
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: iadd r1008, r1008.x, l14
	; CHECK-NEXT: ieq r1008, r1008, l15
	; CHECK-NEXT: mov r1011, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1011.xy__, r1008.x, r1011.xyxy, r1011.zwzw
	; CHECK-NEXT: mov r65.xy__, r1011.xyxy
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %i = getelementptr i64* %p, i32 %q
  %x = load i64* %i
  ret i64 %x
}

define float @t3_f32(float* %p, i32 %q) {


  ; CHECK:      t3_f32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: ishl r65.x___, r1.y, r65.x
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: iadd r1008, r1008.x, l14
	; CHECK-NEXT: ieq r1008, r1008, l15
	; CHECK-NEXT: mov r1011, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.y, r1011.y, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.z, r1011.z, r1011.x
	; CHECK-NEXT: cmov_logical r1011.x___, r1008.w, r1011.w, r1011.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr float* %p, i32 %q
  %x = load float* %i
  ret float %x
}

define double @t3_f64(double* %p, i32 %q) {


  ; CHECK:      t3_f64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: ishl r65.x___, r1.y, r65.x
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: iadd r1008, r1008.x, l14
	; CHECK-NEXT: ieq r1008, r1008, l15
	; CHECK-NEXT: mov r1011, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1011.xy__, r1008.x, r1011.xyxy, r1011.zwzw
	; CHECK-NEXT: mov r65.xy__, r1011.xyxy
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %i = getelementptr double* %p, i32 %q
  %x = load double* %i
  ret double %x
}

define i16 @t4_global_u16() {


  ; CHECK:      t4_global_u16
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: iand r1008.x___, r1010.x, l13
	; CHECK-NEXT: ishr r1008.x___, r1008.x, l14
	; CHECK-NEXT: iand r1010.x___, r1010.x, l15
	; CHECK-NEXT: cmov_logical r1008.x___, r1008.x, l17, l16
	; CHECK-NEXT: uav_raw_load_id(0) r1011.x___, r1010.x
	; CHECK-NEXT: ishr r1011.x___, r1011.x, r1008.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr [10 x i16]* @array_i16, i32 0, i32 0
  %x = load i16* %i
  ret i16 %x
}

define i32 @t4_global_u32() {


  ; CHECK:      t4_global_u32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.x___, r1010.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr [10 x i32]* @array_i32, i32 0, i32 0
  %x = load i32* %i
  ret i32 %x
}

define i64 @t4_global_u64() {


  ; CHECK:      t4_global_u64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.xy__, r1010.x
	; CHECK-NEXT: mov r65.xy__, r1011.xyxy
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %i = getelementptr [10 x i64]* @array_i64, i32 0, i32 0
  %x = load i64* %i
  ret i64 %x
}

define float @t4_global_f32() {


  ; CHECK:      t4_global_f32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.x___, r1010.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr [10 x float]* @array_float, i32 0, i32 0
  %x = load float* %i
  ret float %x
}

define double @t4_global_f64() {


  ; CHECK:      t4_global_f64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.xy__, r1010.x
	; CHECK-NEXT: mov r65.xy__, r1011.xyxy
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %i = getelementptr [10 x double]* @array_double, i32 0, i32 0
  %x = load double* %i
  ret double %x
}

define i16 @t4_const_u16() {


  ; CHECK:      t4_const_u16
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: iand r1008.x___, r1010.x, l13
	; CHECK-NEXT: ishr r1008.x___, r1008.x, l14
	; CHECK-NEXT: iand r1010.x___, r1010.x, l15
	; CHECK-NEXT: cmov_logical r1008.x___, r1008.x, l17, l16
	; CHECK-NEXT: uav_raw_load_id(0) r1011.x___, r1010.x
	; CHECK-NEXT: ishr r1011.x___, r1011.x, r1008.x
  ; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr [10 x i16] addrspace(1)* @array_constant_i16, i32 0, i32 0
  %x = load i16 addrspace(1)* %i
  ret i16 %x
}

define i32 @t4_const_u32() {


  ; CHECK:      t4_const_u32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.x___, r1010.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr [10 x i32] addrspace(1)* @array_constant_i32, i32 0, i32 0
  %x = load i32 addrspace(1)* %i
  ret i32 %x
}

define i64 @t4_const_u64() {


  ; CHECK:      t4_const_u64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.xy__, r1010.x
	; CHECK-NEXT: mov r65.xy__, r1011.xyxy
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %i = getelementptr [10 x i64] addrspace(1)* @array_constant_i64, i32 0, i32 0
  %x = load i64 addrspace(1)* %i
  ret i64 %x
}

define float @t4_const_f32() {


  ; CHECK:      t4_const_f32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.x___, r1010.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x


  %i = getelementptr [10 x float] addrspace(1)* @array_constant_float, i32 0, i32 0
  %x = load float addrspace(1)* %i
  ret float %x
}

define double @t4_const_f64() {


  ; CHECK:      t4_const_f64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.xy__, r1010.x
	; CHECK-NEXT: mov r65.xy__, r1011.xyxy
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %i = getelementptr [10 x double] addrspace(1)* @array_constant_double, i32 0, i32 0
  %x = load double addrspace(1)* %i
  ret double %x
}

define i16 @t4_local_u16() {


  ; CHECK:      t4_local_u16
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: iand r1008.x___, r1010.x, l13
	; CHECK-NEXT: ishr r1008.x___, r1008.x, l14
	; CHECK-NEXT: iand r1010.x___, r1010.x, l15
	; CHECK-NEXT: cmov_logical r1008.x___, r1008.x, l17, l16
	; CHECK-NEXT: uav_raw_load_id(0) r1011.x___, r1010.x
	; CHECK-NEXT: ishr r1011.x___, r1011.x, r1008.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr [10 x i16] addrspace(2)* @array_local_i16, i32 0, i32 0
  %x = load i16 addrspace(2)* %i
  ret i16 %x
}

define i32 @t4_local_u32() {


  ; CHECK:      t4_local_u32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.x___, r1010.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr [10 x i32] addrspace(2)* @array_local_i32, i32 0, i32 0
  %x = load i32 addrspace(2)* %i
  ret i32 %x
}

define i64 @t4_local_u64() {


  ; CHECK:      t4_local_u64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.xy__, r1010.x
	; CHECK-NEXT: mov r65.xy__, r1011.xyxy
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %i = getelementptr [10 x i64] addrspace(2)* @array_local_i64, i32 0, i32 0
  %x = load i64 addrspace(2)* %i
  ret i64 %x
}

define float @t4_local_f32() {


  ; CHECK:      t4_local_f32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.x___, r1010.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr [10 x float] addrspace(2)* @array_local_float, i32 0, i32 0
  %x = load float addrspace(2)* %i
  ret float %x
}

define double @t4_local_f64() {


  ; CHECK:      t4_local_f64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.xy__, r1010.x
	; CHECK-NEXT: mov r65.xy__, r1011.xyxy
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %i = getelementptr [10 x double] addrspace(2)* @array_local_double, i32 0, i32 0
  %x = load double addrspace(2)* %i
  ret double %x
}

define i16 @t4_shared_u16() {


  ; CHECK:      t4_shared_u16
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr [10 x i16] addrspace(4)* @array_shared_i16, i32 0, i32 0
  %x = load i16 addrspace(4)* %i
  ret i16 %x
}

define i32 @t4_shared_u32() {


  ; CHECK:      t4_shared_u32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr [10 x i32] addrspace(4)* @array_shared_i32, i32 0, i32 0
  %x = load i32 addrspace(4)* %i
  ret i32 %x
}

define i64 @t4_shared_u64() {


  ; CHECK:      t4_shared_u64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %i = getelementptr [10 x i64] addrspace(4)* @array_shared_i64, i32 0, i32 0
  %x = load i64 addrspace(4)* %i
  ret i64 %x
}

define float @t4_shared_f32() {


  ; CHECK:      t4_shared_f32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr [10 x float] addrspace(4)* @array_shared_float, i32 0, i32 0
  %x = load float addrspace(4)* %i
  ret float %x
}

define double @t4_shared_f64() {


  ; CHECK:      t4_shared_f64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %i = getelementptr [10 x double] addrspace(4)* @array_shared_double, i32 0, i32 0
  %x = load double addrspace(4)* %i
  ret double %x
}

define i16 @t5_u16() {


  ; CHECK:      t5_u16
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r66.x___, l13
	; CHECK-NEXT: iadd r65.x___, r66.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: iand r1008.x___, r1010.x, l14
	; CHECK-NEXT: ishr r1008.x___, r1008.x, l15
	; CHECK-NEXT: iand r1010.x___, r1010.x, l16
	; CHECK-NEXT: cmov_logical r1008.x___, r1008.x, l18, l17
	; CHECK-NEXT: uav_raw_load_id(0) r1011.x___, r1010.x
  ; CHECK-NEXT: ishr r1011.x___, r1011.x, r1008.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr [10 x i16]* @array_i16, i32 0, i32 1
  %x = load i16* %i
  ret i16 %x
}

define i32 @t5_u32() {


  ; CHECK:      t5_u32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r66.x___, l13
	; CHECK-NEXT: iadd r65.x___, r66.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.x___, r1010.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr [10 x i32]* @array_i32, i32 0, i32 1
  %x = load i32* %i
  ret i32 %x
}

define i64 @t5_u64() {


  ; CHECK:      t5_u64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r66.x___, l13
	; CHECK-NEXT: iadd r65.x___, r66.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.xy__, r1010.x
	; CHECK-NEXT: mov r65.xy__, r1011.xyxy
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %i = getelementptr [10 x i64]* @array_i64, i32 0, i32 1
  %x = load i64* %i
  ret i64 %x
}

define float @t5_f32() {


  ; CHECK:      t5_f32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r66.x___, l13
	; CHECK-NEXT: iadd r65.x___, r66.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.x___, r1010.x
	; CHECK-NEXT: mov r65.x___, r1011.x
	; CHECK-NEXT: mov r1.x___, r65.x

  %i = getelementptr [10 x float]* @array_float, i32 0, i32 1
  %x = load float* %i
  ret float %x
}

define double @t5_f64() {


  ; CHECK:      t5_f64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r66.x___, l13
	; CHECK-NEXT: iadd r65.x___, r66.x, r65.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: uav_raw_load_id(0) r1011.xy__, r1010.x
	; CHECK-NEXT: mov r65.xy__, r1011.xyxy
	; CHECK-NEXT: mov r1.xy__, r65.xyxy

  %i = getelementptr [10 x double]* @array_double, i32 0, i32 1
  %x = load double* %i
  ret double %x
}

;CHECK: .global@array_i16:416
;CHECK: .global@array_constant_i16:752
;CHECK: .global@array_local_i16:784
;CHECK: .global@array_shared_i16:128
;CHECK: .global@array_i32:368
;CHECK: .global@array_constant_i32:656
;CHECK: .global@array_local_i32:704
;CHECK: .global@array_shared_i32:0
;CHECK: .global@array_i64:528
;CHECK: .global@array_constant_i64:896
;CHECK: .global@array_local_i64:976
;CHECK: .global@array_shared_i64:240
;CHECK: .global@array_float:608
;CHECK: .global@array_constant_float:1056
;CHECK: .global@array_local_float:1104
;CHECK: .global@array_shared_float:320
;CHECK: .global@array_double:816
;CHECK: .global@array_constant_double:48
;CHECK: .global@array_local_double:160
;CHECK: .global@array_shared_double:448
