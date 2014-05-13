; RUN: llc -march=amdil -mcpu=tahiti -mattr=+64bitptr,+small-global-objects < %s | FileCheck %s

@basic_cllocal_buf = internal addrspace(3) global [1024 x i32] zeroinitializer, align 4

; CHECK-LABEL: @basic_shrink_lds_store_add_const_ptr
; CHECK-NOT: i64add
; CHECK: lds_store_id
define void @basic_shrink_lds_store_add_const_ptr(i32 addrspace(3)* %ptr, i32 %n) nounwind {
  %incptr = getelementptr i32 addrspace(3)* %ptr, i32 4
  store i32 %n, i32 addrspace(3)* %incptr, align 4
  ret void
}

; CHECK-LABEL: @basic_shrink_lds_store_add_var_ptr
; CHECK: i64add
; CHECK: lds_store_id
define void @basic_shrink_lds_store_add_var_ptr(i32 addrspace(3)* %ptr, i32 %n, i32 %idx) nounwind {
  %incptr = getelementptr i32 addrspace(3)* %ptr, i32 %idx
  store i32 %n, i32 addrspace(3)* %incptr, align 4
  ret void
}

; CHECK-LABEL: @basic_shrink_global_store_add_const_ptr
; CHECK-NOT: i64add
; CHECK: uav_raw_store_addr(64)_id
define void @basic_shrink_global_store_add_const_ptr(i32 addrspace(1)* %ptr, i32 %n) nounwind {
  %incptr = getelementptr i32 addrspace(1)* %ptr, i32 4
  store i32 %n, i32 addrspace(1)* %incptr, align 4
  ret void
}

; CHECK-LABEL: @basic_shrink_global_store_add_var_ptr
; CHECK-NOT: i64add
; CHECK: uav_raw_store_addr(64)_id
define void @basic_shrink_global_store_add_var_ptr(i32 addrspace(1)* %ptr, i32 %n, i32 %idx) nounwind {
  %incptr = getelementptr i32 addrspace(1)* %ptr, i32 %idx
  store i32 %n, i32 addrspace(1)* %incptr, align 4
  ret void
}

; CHECK-LABEL: @basic_shrink_lds_store_add_const_global_address
; CHECK-NOT: i64add
; CHECK: lds_store_id
define void @basic_shrink_lds_store_add_const_global_address(i32 %n) nounwind {
  %incptr = getelementptr [1024 x i32] addrspace(3)* @basic_cllocal_buf, i32 0, i32 4
  store i32 %n, i32 addrspace(3)* %incptr, align 4
  ret void
}

; FIXME: This still has an i64add
; CHECK-LABEL: @basic_shrink_lds_store_add_var_global_address
; CHECK: lds_store_id
define void @basic_shrink_lds_store_add_var_global_address(i32 %n, i32 %idx) nounwind {
  %incptr = getelementptr [1024 x i32] addrspace(3)* @basic_cllocal_buf, i32 0, i32 %idx
  store i32 %n, i32 addrspace(3)* %incptr, align 4
  ret void
}
