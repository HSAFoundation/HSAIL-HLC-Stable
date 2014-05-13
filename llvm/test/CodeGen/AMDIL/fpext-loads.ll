; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s

declare float @__amdil_half_to_float_f32(i32) nounwind readnone
declare <2 x float> @__amdil_half_to_float_v2f32(<2 x i32>) nounwind readnone


; CHECK-LABEL: @fpext_load_double
; CHECK: uav_raw_load_id({{[0-9]+}}) [[REG:r[0-9]+]].x___,
; CHECK-NEXT: f2d {{r[0-9]+}}.xy__, [[REG]].x
define void @fpext_load_double(float addrspace(1)* noalias nocapture %src,
                               double addrspace(1)* noalias nocapture %dest,
                               i32 %idx) nounwind {
  %arrayidx = getelementptr double addrspace(1)* %dest, i32 %idx
  %arrayidx4 = getelementptr float addrspace(1)* %src, i32 %idx
  %tmp5 = load float addrspace(1)* %arrayidx4, align 4
  %conv = fpext float %tmp5 to double
  store double %conv, double addrspace(1)* %arrayidx, align 8
  ret void
}

; CHECK-LABEL: @fpext_load_float2_to_double2
; CHECK: f2d
; CHECK: f2d
define void @fpext_load_float2_to_double2(<2 x double> addrspace(1)* %out, <2 x float> %in) nounwind {
  %todouble = fpext <2 x float> %in to <2 x double>
  store <2 x double> %todouble, <2 x double> addrspace(1)* %out, align 16
  ret void
}

; CHECK-LABEL: @fpext_load_float_amdil
; CHECK: uav_short_load_id({{[0-9]+}}) [[REG:r[0-9]+]].x___,
; CHECK-NEXT: f162f {{r[0-9]+}}.x___, [[REG]].x
define void @fpext_load_float_amdil(half addrspace(1)* noalias nocapture %src,
                                    float addrspace(1)* noalias nocapture %dest,
                                    i32 %idx) nounwind {
  %arrayidx = getelementptr float addrspace(1)* %dest, i32 %idx
  %arrayidx4 = getelementptr half addrspace(1)* %src, i32 %idx
  %bc = bitcast half addrspace(1)* %arrayidx4 to i16 addrspace(1)*
  %ld = load i16 addrspace(1)* %bc, align 2
  %extld = zext i16 %ld to i32
  %conv = call float @__amdil_half_to_float_f32(i32 %extld) nounwind readnone
  store float %conv, float addrspace(1)* %arrayidx, align 8
  ret void
}

; FAIL-CHECK-LABEL: @fpext_load_float
; FAIL-CHECK: uav_short_load_id({{[0-9]+}}) [[REG:r[0-9]+]].x
; FAIL-CHECK-NEXT: f162f {{r[0-9]+}}.xy__, [[REG]].x
define void @fpext_load_float(half addrspace(1)* noalias nocapture %src,
                              float addrspace(1)* noalias nocapture %dest,
                              i32 %idx) nounwind {
  %arrayidx = getelementptr float addrspace(1)* %dest, i32 %idx
  %arrayidx4 = getelementptr half addrspace(1)* %src, i32 %idx
  %tmp5 = load half addrspace(1)* %arrayidx4, align 4
  %conv = fpext half %tmp5 to float
  store float %conv, float addrspace(1)* %arrayidx, align 8
  ret void
}

; FAIL-CHECK-LABEL: @fpext_load_double_half
; FAIL-CHECK: uav_short_load_id({{[0-9]+}}) [[REG:r[0-9]+]].x
; FAIL-CHECK-NEXT: f162f {{r[0-9]+}}.xy__, [[REG]].x
define void @fpext_load_double_half(half addrspace(1)* noalias nocapture %src,
                                    double addrspace(1)* noalias nocapture %dest,
                                    i32 %idx) nounwind {
  %arrayidx = getelementptr double addrspace(1)* %dest, i32 %idx
  %arrayidx4 = getelementptr half addrspace(1)* %src, i32 %idx
  %tmp5 = load half addrspace(1)* %arrayidx4, align 4
  %conv = fpext half %tmp5 to double
  store double %conv, double addrspace(1)* %arrayidx, align 8
  ret void
}

; CHECK-LABEL: @fpext_load_half2_to_float2_amdil
; CHECK: f162f r{{[0-9]+}}.xy__,
define void @fpext_load_half2_to_float2_amdil(<2 x float> addrspace(1)* %out, <2 x half> %in) nounwind {
  %bcin = bitcast <2 x half> %in to <2 x i16>
  %ext = zext <2 x i16> %bcin to <2 x i32>
  %tofloat = tail call <2 x float> @__amdil_half_to_float_v2f32(<2 x i32> %ext) nounwind
  store <2 x float> %tofloat, <2 x float> addrspace(1)* %out, align 16
  ret void
}

; FAIL-CHECK-LABEL: @fpext_load_half2_to_float2
; FAIL-CHECK: f162f r{{[0-9]+}}.xy__,
define void @fpext_load_half2_to_float2(<2 x float> addrspace(1)* %out, <2 x half> %in) nounwind {
  %tofloat = fpext <2 x half> %in to <2 x float>
  store <2 x float> %tofloat, <2 x float> addrspace(1)* %out, align 16
  ret void
}

; CHECK-LABEL: @fpext_load_half2_to_double2_amdil
; CHECK: f162f r{{[0-9]+}}.xy__,
; CHECK: f2d
; CHECK: f2d
define void @fpext_load_half2_to_double2_amdil(<2 x double> addrspace(1)* %out, <2 x half> %in) nounwind {
  %bcin = bitcast <2 x half> %in to <2 x i16>
  %ext = zext <2 x i16> %bcin to <2 x i32>
  %tofloat = tail call <2 x float> @__amdil_half_to_float_v2f32(<2 x i32> %ext) nounwind
  %todouble = fpext <2 x float> %tofloat to <2 x double>
  store <2 x double> %todouble, <2 x double> addrspace(1)* %out, align 16
  ret void
}

; FAIL-CHECK-LABEL: @fpext_load_half2_to_double2_amdil
; FAIL-CHECK: f162f r{{[0-9]+}}.xy__,
; FAIL-CHECK: f2d
; FAIL-CHECK: f2d
define void @fpext_load_half2_to_double2(<2 x double> addrspace(1)* %out, <2 x half> %in) nounwind {
  %todouble = fpext <2 x half> %in to <2 x double>
  store <2 x double> %todouble, <2 x double> addrspace(1)* %out, align 16
  ret void
}


