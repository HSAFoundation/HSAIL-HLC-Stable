; RUN: llc -march=amdil -mcpu=tahiti -mattr=+64bitptr < %s | FileCheck %s

; CHECK: __OpenCL_lds_arg_64_kernel
; CHECK: [[REG:r[0-9]+]].xy__, r1.zw
; CHECK: lds_store_id({{[0-9]}}) [[REG]]

; Filler argument so the local pointer argument will be placed in the
; upper 2 components of the kernel argument register.  The address is
; read from the low components, but the whole register is just used as
; the store address. It needs to be swizzled.
define void @__OpenCL_lds_arg_64_kernel(i32 %arg0, ; r1.x
                                        i32 %arg1, ; r1.y
                                        float addrspace(3)* nocapture %ptr) nounwind { ; r1.zw
  store float 1.000000e+00, float addrspace(3)* %ptr, align 4
  ret void
}

; CHECK: __OpenCL_global_arg_64_kernel
; CHECK: [[REG:r[0-9]+]].xy__, r1.zw
; CHECK: uav_raw_store_addr(64)_id({{[0-9]+}}) mem0.x___, [[REG]]
define void @__OpenCL_global_arg_64_kernel(i32 %arg0, ; r1.x
                                           i32 %arg1, ; r1.y
                                           float addrspace(1)* nocapture %ptr) nounwind { ; r1.zw
  store float 1.000000e+00, float addrspace(1)* %ptr, align 4
  ret void
}
