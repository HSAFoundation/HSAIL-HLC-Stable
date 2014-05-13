; RUN llc < %s -march=amdil | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

define  void @__OpenCL_i64_to_f32_mad(i64 addrspace(1)* noalias nocapture %in, float addrspace(1)* noalias nocapture %out) nounwind {
entry:
  %tmpi64 = load i64 addrspace(1)* %in, align 8
  %tmpf32 = sitofp i64 %tmpi64 to float
  ; This should have converted to float. Just pass the result to something the SDNode expects to be float
  %tmpmad = tail call float @__amdil_mad(float %tmpf32, float %tmpf32, float %tmpf32) nounwind
  store float %tmpmad, float addrspace(1)* %out, align 4
  ret void
}

declare float @__amdil_mad(float, float, float) nounwind readnone
; RUN llc < %s -march=amdil | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

define  void @__OpenCL_i64_to_f32_mad(i64 addrspace(1)* noalias nocapture %in, float addrspace(1)* noalias nocapture %out) nounwind {
entry:
  %tmpi64 = load i64 addrspace(1)* %in, align 8
  %tmpf32 = sitofp i64 %tmpi64 to float
  ; This should have converted to float. Just pass the result to something the SDNode expects to be float
  %tmpmad = tail call float @__amdil_mad(float %tmpf32, float %tmpf32, float %tmpf32) nounwind
  store float %tmpmad, float addrspace(1)* %out, align 4
  ret void
}

declare float @__amdil_mad(float, float, float) nounwind readnone
