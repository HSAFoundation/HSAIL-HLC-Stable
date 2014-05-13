; RUN: llc -march=amdil -o - %s | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024--a64:64:64-f80:128:128-n8:16:32"
target triple = "amdil-applecl-darwin11"

define void @insert_nonconst_index_float_element(<4 x float> addrspace(1)* nocapture %out, <4 x float> %vec, i32 %idx) nounwind {
; CHECK: iadd r{{[0-9]+}}, r{{[0-9]+\.?[0xyzw]*}}, l{{[0-9]+}}.x000
; CHECK: iadd r{{[0-9]+}}, r{{[0-9]+\.?[0xyzw]*}}, l{{[0-9]+}}.0x00
; CHECK: iadd r{{[0-9]+}}, r{{[0-9]+\.?[0xyzw]*}}, l{{[0-9]+}}.00x0
; CHECK: iadd r{{[0-9]+}}, r{{[0-9]+\.?[0xyzw]*}}, l{{[0-9]+}}.000x
  %result = insertelement <4 x float> %vec, float 2.0, i32 %idx
  store <4 x float> %result, <4 x float> addrspace(1)* %out, align 16
  ret void
}

define void @insert_const_index0_float_element(<4 x float> addrspace(1)* nocapture %out, <4 x float> %vec) nounwind {
; CHECK: iadd r{{[0-9]+}}, r{{[0-9]+\.?[0xyzw]*}}, l{{[0-9]+}}.x000
  %result = insertelement <4 x float> %vec, float 2.0, i32 0
  store <4 x float> %result, <4 x float> addrspace(1)* %out, align 16
  ret void
}

define void @insert_const_index1_float_element(<4 x float> addrspace(1)* nocapture %out, <4 x float> %vec) nounwind {
; CHECK: iadd r{{[0-9]+}}, r{{[0-9]+\.?[0xyzw]*}}, l{{[0-9]+}}.0x00
  %result = insertelement <4 x float> %vec, float 2.0, i32 1
  store <4 x float> %result, <4 x float> addrspace(1)* %out, align 16
  ret void
}

define void @insert_const_index2_float_element(<4 x float> addrspace(1)* nocapture %out, <4 x float> %vec) nounwind {
; CHECK: iadd r{{[0-9]+}}, r{{[0-9]+\.?[0xyzw]*}}, l{{[0-9]+}}.00x0
  %result = insertelement <4 x float> %vec, float 2.0, i32 2
  store <4 x float> %result, <4 x float> addrspace(1)* %out, align 16
  ret void
}

define void @insert_const_index3_float_element(<4 x float> addrspace(1)* nocapture %out, <4 x float> %vec) nounwind {
; CHECK: iadd r{{[0-9]+}}, r{{[0-9]+\.?[0xyzw]*}}, l{{[0-9]+}}.000x
  %result = insertelement <4 x float> %vec, float 2.0, i32 3
  store <4 x float> %result, <4 x float> addrspace(1)* %out, align 16
  ret void
}

