; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @fptrunc_store_double_float
; CHECK: uav_raw_load_id({{[0-9]+}}) [[REG:r[0-9]+]].xy__,
; CHECK-NEXT: d2f {{r[0-9]+}}.x___, [[REG]].xy
define void @fptrunc_store_double_float(double addrspace(1)* noalias nocapture %src,
                                       float addrspace(1)* noalias nocapture %dest,
                                       i32 %idx) nounwind {
  %arrayidx = getelementptr float addrspace(1)* %dest, i32 %idx
  %arrayidx4 = getelementptr double addrspace(1)* %src, i32 %idx
  %tmp5 = load double addrspace(1)* %arrayidx4, align 8
  %conv = fptrunc double %tmp5 to float
  store float %conv, float addrspace(1)* %arrayidx, align 4
  ret void
}

; CHECK-LABEL: @fptrunc_store_double2_float2
; CHECK: uav_raw_load_id({{[0-9]+}}) [[REG:r[0-9]+]],
; CHECK-NEXT: d2f {{r[0-9]+}}.x___, [[REG]].xy
; CHECK-NEXT: d2f {{r[0-9]+}}._y__, [[REG]].zw
; CHECK: uav_raw_store_id
define void @fptrunc_store_double2_float2(<2 x double> addrspace(1)* noalias nocapture %src,
                                          <2 x float> addrspace(1)* noalias nocapture %dest,
                                          i32 %idx) nounwind {
  %arrayidx = getelementptr <2 x float> addrspace(1)* %dest, i32 %idx
  %arrayidx4 = getelementptr <2 x double> addrspace(1)* %src, i32 %idx
  %tmp5 = load <2 x double> addrspace(1)* %arrayidx4, align 8
  %conv = fptrunc <2 x double> %tmp5 to <2 x float>
  store <2 x float> %conv, <2 x float> addrspace(1)* %arrayidx, align 4
  ret void
}


