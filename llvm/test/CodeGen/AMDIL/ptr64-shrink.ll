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


@cllocal_buffer = internal addrspace(3) global [4096 x i32] zeroinitializer, align 4
@lvgv = internal constant [1 x i8*] [i8* bitcast ([4096 x i32] addrspace(3)* @cllocal_buffer to i8*)]

@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*)* @lds_global_address_truncate to i8*), i8* bitcast ([1 x i8] addrspace(2)* null to i8*), i8* bitcast ([1 x i8] addrspace(2)* null to i8*), i8* bitcast ([1 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* null to i8*), i32 0 }], section "llvm.metadata"

declare <4 x i32> @__amdil_get_local_id_int() nounwind readnone

; CHECK-LABEL: @lds_global_address_truncate
define void @lds_global_address_truncate(i32 addrspace(1)* nocapture %output, i32 addrspace(1)* nocapture %input) nounwind {
entry:
  %lid = tail call <4 x i32> @__amdil_get_local_id_int() nounwind
  %id = extractelement <4 x i32> %lid, i32 0
  %tobool = icmp eq i32 %id, 0
  %conv = sext i32 %id to i64
  %arrayidx = getelementptr [4096 x i32] addrspace(3)* @cllocal_buffer, i64 0, i64 %conv
  store i32 123, i32 addrspace(3)* %arrayidx, align 4
  ret void
}
