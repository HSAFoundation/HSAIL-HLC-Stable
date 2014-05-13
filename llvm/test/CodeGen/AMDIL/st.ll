; RUN: llc < %s -march=amdil | FileCheck %s

@array_i16 =  global [10 x i16] zeroinitializer
@array_constant_i16 =  addrspace(1) constant [10 x i16] zeroinitializer
@array_local_i16 =  addrspace(2) global [10 x i16] zeroinitializer
@array_shared_i16 =  addrspace(1) global [10 x i16] zeroinitializer
@array_i32 =  global [10 x i32] zeroinitializer
@array_constant_i32 =  addrspace(1) constant [10 x i32] zeroinitializer
@array_local_i32 =  addrspace(2) global [10 x i32] zeroinitializer
@array_shared_i32 =  addrspace(1) global [10 x i32] zeroinitializer
@array_i64 =  global [10 x i64] zeroinitializer
@array_constant_i64 =  addrspace(1) constant [10 x i64] zeroinitializer
@array_local_i64 =  addrspace(2) global [10 x i64] zeroinitializer
@array_shared_i64 =  addrspace(1) global [10 x i64] zeroinitializer
@array_float =  global [10 x float] zeroinitializer
@array_constant_float =  addrspace(1) constant [10 x float] zeroinitializer
@array_local_float =  addrspace(2) global [10 x float] zeroinitializer
@array_shared_float =  addrspace(1) global [10 x float] zeroinitializer
@array_double =  global [10 x double] zeroinitializer
@array_constant_double =  addrspace(1) constant [10 x double] zeroinitializer
@array_local_double =  addrspace(2) global [10 x double] zeroinitializer
@array_shared_double =  addrspace(1) global [10 x double] zeroinitializer

define void @t1_u16(i16* %p, i16 %x) {

  ; CHECK:      t1_u16
	; CHECK:      mov r1011, r1.y
	; CHECK-NEXT: mov r1010.x___, r1.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: iadd r1008, r1008.x, l14
	; CHECK-NEXT: ieq r1008, r1008, l15
	; CHECK-NEXT: mov r1002, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.y, r1002.y, r1002.x
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.z, r1002.z, r1002.x
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.w, r1002.w, r1002.x
	; CHECK-NEXT: ishr r1003.x___, r1010.x, l16
	; CHECK-NEXT: iand r1003.x___, r1003.x, l16
	; CHECK-NEXT: ishr r1001.x___, r1002.x, l17
	; CHECK-NEXT: cmov_logical r1002.x___, r1003.x, r1002.x, r1011.x
	; CHECK-NEXT: cmov_logical r1001.x___, r1003.x, r1011.x, r1001.x
	; CHECK-NEXT: iand r1002.x___, r1002.x, l18
	; CHECK-NEXT: iand r1001.x___, r1001.x, l18
	; CHECK-NEXT: ishl r1001.x___, r1001.x, l17
	; CHECK-NEXT: ior r1011.x___, r1002.x, r1001.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  store i16 %x, i16* %p
  ret void
}

define void @t1_u32(i32* %p, i32 %x) {

  ; CHECK:      t1_u32
	; CHECK:      mov r1011, r1.y
	; CHECK-NEXT: mov r1010.x___, r1.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  store i32 %x, i32* %p
  ret void
}

define void @t1_u64(i64* %p, i64 %x) {

  ; CHECK:      t1_u64
	; CHECK:      mov r1011, r1.zwzw
	; CHECK-NEXT: mov r1010.x___, r1.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].xy__, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x].__zw, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  store i64 %x, i64* %p
  ret void
}

define void @t1_f32(float* %p, float %x) {

  ; CHECK:      t1_f32
	; CHECK:      mov r1011, r1.y
	; CHECK-NEXT: mov r1010.x___, r1.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  store float %x, float* %p
  ret void
}

define void @t1_f64(double* %p, double %x) {

  ; CHECK:      t1_f64
	; CHECK:      mov r1011, r1.zwzw
	; CHECK-NEXT: mov r1010.x___, r1.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].xy__, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x].__zw, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  store double %x, double* %p
  ret void
}

define void @t2_u16(i16* %p, i16 %x) {

  ; CHECK:      t2_u16
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1011, r1.y
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: iadd r1008, r1008.x, l14
	; CHECK-NEXT: ieq r1008, r1008, l15
	; CHECK-NEXT: mov r1002, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.y, r1002.y, r1002.x
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.z, r1002.z, r1002.x
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.w, r1002.w, r1002.x
	; CHECK-NEXT: ishr r1003.x___, r1010.x, l16
	; CHECK-NEXT: iand r1003.x___, r1003.x, l16
	; CHECK-NEXT: ishr r1001.x___, r1002.x, l17
	; CHECK-NEXT: cmov_logical r1002.x___, r1003.x, r1002.x, r1011.x
	; CHECK-NEXT: cmov_logical r1001.x___, r1003.x, r1011.x, r1001.x
	; CHECK-NEXT: iand r1002.x___, r1002.x, l18
	; CHECK-NEXT: iand r1001.x___, r1001.x, l18
	; CHECK-NEXT: ishl r1001.x___, r1001.x, l17
	; CHECK-NEXT: ior r1011.x___, r1002.x, r1001.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr i16* %p, i32 1
  store i16 %x, i16* %i
  ret void
}

define void @t2_u32(i32* %p, i32 %x) {

  ; CHECK:      t2_u32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1011, r1.y
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr i32* %p, i32 1
  store i32 %x, i32* %i
  ret void
}

define void @t2_u64(i64* %p, i64 %x) {

  ; CHECK:      t2_u64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1011, r1.zwzw
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l14
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].xy__, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x].__zw, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr i64* %p, i32 1
  store i64 %x, i64* %i
  ret void
}

define void @t2_f32(float* %p, float %x) {

  ; CHECK:      t2_f32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1011, r1.y
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr float* %p, i32 1
  store float %x, float* %i
  ret void
}

define void @t2_f64(double* %p, double %x) {

  ; CHECK:      t2_f64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1011, r1.zwzw
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l14
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].xy__, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x].__zw, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr double* %p, i32 1
  store double %x, double* %i
  ret void
}

define void @t3_u16(i16* %p, i32 %q, i16 %x) {

  ; CHECK:      t3_u16
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: ishl r65.x___, r1.y, r65.x
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1011, r1.z
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: iadd r1008, r1008.x, l15
	; CHECK-NEXT: ieq r1008, r1008, l16
	; CHECK-NEXT: mov r1002, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.y, r1002.y, r1002.x
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.z, r1002.z, r1002.x
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.w, r1002.w, r1002.x
	; CHECK-NEXT: ishr r1003.x___, r1010.x, l12
	; CHECK-NEXT: iand r1003.x___, r1003.x, l12
	; CHECK-NEXT: ishr r1001.x___, r1002.x, l17
	; CHECK-NEXT: cmov_logical r1002.x___, r1003.x, r1002.x, r1011.x
	; CHECK-NEXT: cmov_logical r1001.x___, r1003.x, r1011.x, r1001.x
	; CHECK-NEXT: iand r1002.x___, r1002.x, l18
	; CHECK-NEXT: iand r1001.x___, r1001.x, l18
	; CHECK-NEXT: ishl r1001.x___, r1001.x, l17
	; CHECK-NEXT: ior r1011.x___, r1002.x, r1001.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr i16* %p, i32 %q
  store i16 %x, i16* %i
  ret void
}

define void @t3_u32(i32* %p, i32 %q, i32 %x) {

  ; CHECK:      t3_u32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: ishl r65.x___, r1.y, r65.x
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1011, r1.z
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr i32* %p, i32 %q
  store i32 %x, i32* %i
  ret void
}

define void @t3_u64(i64* %p, i32 %q, i64 %x) {

  ; CHECK:      t3_u64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: ishl r65.x___, r1.y, r65.x
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1011, r1.zwzw
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].xy__, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x].__zw, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr i64* %p, i32 %q
  store i64 %x, i64* %i
  ret void
}

define void @t3_f32(float* %p, i32 %q, float %x) {

  ; CHECK:      t3_f32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: ishl r65.x___, r1.y, r65.x
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1011, r1.z
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr float* %p, i32 %q
  store float %x, float* %i
  ret void
}

define void @t3_f64(double* %p, i32 %q, double %x) {

  ; CHECK:      t3_f64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: ishl r65.x___, r1.y, r65.x
	; CHECK-NEXT: iadd r65.x___, r1.x, r65.x
	; CHECK-NEXT: mov r1011, r1.zwzw
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l13
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].xy__, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x].__zw, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr double* %p, i32 %q
  store double %x, double* %i
  ret void
}

define void @t4_global_u16(i16 %x) {

  ; CHECK:      t4_global_u16
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1011, r1.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: iadd r1008, r1008.x, l15
	; CHECK-NEXT: ieq r1008, r1008, l16
	; CHECK-NEXT: mov r1002, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.y, r1002.y, r1002.x
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.z, r1002.z, r1002.x
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.w, r1002.w, r1002.x
	; CHECK-NEXT: ishr r1003.x___, r1010.x, l17
	; CHECK-NEXT: iand r1003.x___, r1003.x, l17
	; CHECK-NEXT: ishr r1001.x___, r1002.x, l18
	; CHECK-NEXT: cmov_logical r1002.x___, r1003.x, r1002.x, r1011.x
	; CHECK-NEXT: cmov_logical r1001.x___, r1003.x, r1011.x, r1001.x
	; CHECK-NEXT: iand r1002.x___, r1002.x, l19
	; CHECK-NEXT: iand r1001.x___, r1001.x, l19
	; CHECK-NEXT: ishl r1001.x___, r1001.x, l18
	; CHECK-NEXT: ior r1011.x___, r1002.x, r1001.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x i16]* @array_i16, i16 0, i16 0
  store i16 %x, i16* %i
  ret void
}

define void @t4_global_u32(i32 %x) {

  ; CHECK:      t4_global_u32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1011, r1.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x i32]* @array_i32, i32 0, i32 0
  store i32 %x, i32* %i
  ret void
}

define void @t4_global_u64(i64 %x) {

  ; CHECK:      t4_global_u64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1011, r1.xyxy
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l14
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].xy__, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x].__zw, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x i64]* @array_i64, i32 0, i32 0
  store i64 %x, i64* %i
  ret void
}

define void @t4_global_f32(float %x) {

  ; CHECK:      t4_global_f32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1011, r1.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x float]* @array_float, i32 0, i32 0
  store float %x, float* %i
  ret void
}

define void @t4_global_f64(double %x) {

  ; CHECK:      t4_global_f64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1011, r1.xyxy
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l14
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].xy__, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x].__zw, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x double]* @array_double, i32 0, i32 0
  store double %x, double* %i
  ret void
}

define void @t4_local_u16(i16 %x) {

  ; CHECK:      t4_local_u16
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1011, r1.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: iadd r1008, r1008.x, l15
	; CHECK-NEXT: ieq r1008, r1008, l16
	; CHECK-NEXT: mov r1002, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.y, r1002.y, r1002.x
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.z, r1002.z, r1002.x
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.w, r1002.w, r1002.x
	; CHECK-NEXT: ishr r1003.x___, r1010.x, l17
	; CHECK-NEXT: iand r1003.x___, r1003.x, l17
	; CHECK-NEXT: ishr r1001.x___, r1002.x, l18
	; CHECK-NEXT: cmov_logical r1002.x___, r1003.x, r1002.x, r1011.x
	; CHECK-NEXT: cmov_logical r1001.x___, r1003.x, r1011.x, r1001.x
	; CHECK-NEXT: iand r1002.x___, r1002.x, l19
	; CHECK-NEXT: iand r1001.x___, r1001.x, l19
	; CHECK-NEXT: ishl r1001.x___, r1001.x, l18
	; CHECK-NEXT: ior r1011.x___, r1002.x, r1001.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x i16] addrspace(2)* @array_local_i16, i32 0, i32 0
  store i16 %x, i16 addrspace(2)* %i
  ret void
}

define void @t4_local_u32(i32 %x) {

  ; CHECK:      t4_local_u32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1011, r1.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x i32] addrspace(2)* @array_local_i32, i32 0, i32 0
  store i32 %x, i32 addrspace(2)* %i
  ret void
}

define void @t4_local_u64(i64 %x) {

  ; CHECK:      t4_local_u64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1011, r1.xyxy
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l14
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].xy__, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x].__zw, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x i64] addrspace(2)* @array_local_i64, i32 0, i32 0
  store i64 %x, i64 addrspace(2)* %i
  ret void
}

define void @t4_local_f32(float %x) {

  ; CHECK:      t4_local_f32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1011, r1.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l13
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x float] addrspace(2)* @array_local_float, i32 0, i32 0
  store float %x, float addrspace(2)* %i
  ret void
}

define void @t4_local_f64(double %x) {

  ; CHECK:      t4_local_f64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r1011, r1.xyxy
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l13
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l14
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].xy__, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x].__zw, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x double] addrspace(2)* @array_local_double, i32 0, i32 0
  store double %x, double addrspace(2)* %i
  ret void
}

define void @t5_u16(i16 %x) {

  ; CHECK:      t5_u16
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r66.x___, l13
	; CHECK-NEXT: iadd r65.x___, r66.x, r65.x
	; CHECK-NEXT: mov r1011, r1.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: iadd r1008, r1008.x, l15
	; CHECK-NEXT: ieq r1008, r1008, l16
	; CHECK-NEXT: mov r1002, x1[r1007.x]
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.y, r1002.y, r1002.x
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.z, r1002.z, r1002.x
	; CHECK-NEXT: cmov_logical r1002.x___, r1008.w, r1002.w, r1002.x
	; CHECK-NEXT: ishr r1003.x___, r1010.x, l17
	; CHECK-NEXT: iand r1003.x___, r1003.x, l17
	; CHECK-NEXT: ishr r1001.x___, r1002.x, l18
	; CHECK-NEXT: cmov_logical r1002.x___, r1003.x, r1002.x, r1011.x
	; CHECK-NEXT: cmov_logical r1001.x___, r1003.x, r1011.x, r1001.x
	; CHECK-NEXT: iand r1002.x___, r1002.x, l19
	; CHECK-NEXT: iand r1001.x___, r1001.x, l19
	; CHECK-NEXT: ishl r1001.x___, r1001.x, l18
	; CHECK-NEXT: ior r1011.x___, r1002.x, r1001.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l12
	; CHECK-NEXT: iand r1008.x___, r1007.x, l14
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l12
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x i16]* @array_i16, i32 0, i32 1
  store i16 %x, i16* %i
  ret void
}

define void @t5_u32(i32 %x) {

  ; CHECK:      t5_u32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r66.x___, l13
	; CHECK-NEXT: iadd r65.x___, r66.x, r65.x
	; CHECK-NEXT: mov r1011, r1.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l14
	; CHECK-NEXT: iand r1008.x___, r1007.x, l15
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l14
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x i32]* @array_i32, i32 0, i32 1
  store i32 %x, i32* %i
  ret void
}

define void @t5_u64(i64 %x) {

  ; CHECK:      t5_u64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r66.x___, l13
	; CHECK-NEXT: iadd r65.x___, r66.x, r65.x
	; CHECK-NEXT: mov r1011, r1.xyxy
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l14
	; CHECK-NEXT: iand r1008.x___, r1007.x, l15
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l15
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].xy__, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x].__zw, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x i64]* @array_i64, i32 0, i32 1
  store i64 %x, i64* %i
  ret void
}

define void @t5_f32(float %x) {

  ; CHECK:      t5_f32
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r66.x___, l13
	; CHECK-NEXT: iadd r65.x___, r66.x, r65.x
	; CHECK-NEXT: mov r1011, r1.x
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l14
	; CHECK-NEXT: iand r1008.x___, r1007.x, l15
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l14
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].x___, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x]._y__, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 2
	; CHECK-NEXT: mov x1[r1007.x].__z_, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: case 3
	; CHECK-NEXT: mov x1[r1007.x].___w, r1011.x
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x float]* @array_float, i32 0, i32 1
  store float %x, float* %i
  ret void
}

define void @t5_f64(double %x) {

  ; CHECK:      t5_f64
	; CHECK:      mov r65.x___, l12
	; CHECK-NEXT: mov r66.x___, l13
	; CHECK-NEXT: iadd r65.x___, r66.x, r65.x
	; CHECK-NEXT: mov r1011, r1.xyxy
	; CHECK-NEXT: mov r1010.x___, r65.x
	; CHECK-NEXT: ishr r1007.x___, r1010.x, l14
	; CHECK-NEXT: iand r1008.x___, r1007.x, l15
	; CHECK-NEXT: ishr r1007.x___, r1007.x, l15
	; CHECK-NEXT: switch r1008.x
	; CHECK-NEXT: default
	; CHECK-NEXT: mov x1[r1007.x].xy__, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: case 1
	; CHECK-NEXT: mov x1[r1007.x].__zw, r1011.xyxy
	; CHECK-NEXT: break
	; CHECK-NEXT: endswitch

  %i = getelementptr [10 x double]* @array_double, i32 0, i32 1
  store double %x, double* %i
  ret void
}

; CHECK: .global@array_i16:496
; CHECK: .global@array_constant_i16
; CHECK: .global@array_local_i16:256
; CHECK: .global@array_shared_i16
; CHECK: .global@array_i32:448
; CHECK: .global@array_constant_i32
; CHECK: .global@array_local_i32:208
; CHECK: .global@array_shared_i32
; CHECK: .global@array_i64:0
; CHECK: .global@array_constant_i64
; CHECK: .global@array_local_i64:368
; CHECK: .global@array_shared_i64
; CHECK: .global@array_float:160
; CHECK: .global@array_constant_float
; CHECK: .global@array_local_float:528
; CHECK: .global@array_shared_float
; CHECK: .global@array_double:288
; CHECK: .global@array_constant_double
; CHECK: .global@array_local_double:80
; CHECK: .global@array_shared_double
