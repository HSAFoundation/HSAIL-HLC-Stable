; Optimization of load/store with generic address
;
; RUN: llc -march=hsail-64 -filetype=asm -o - %s | FileCheck %s


; ModuleID = 'loadstore-optimization.bc'
target triple = "hsail64-pc-unknown-amdopencl"

; optimized to global load/store.
;
; CHECK-LABEL: @Load_store_global
; CHECK: ld_global_align
; CHECK: st_global_align
define spir_func void @Load_store_global(i32 addrspace(1)* nocapture %input, i32 addrspace(1)* nocapture %output) nounwind {
entry:
  %0 = bitcast i32 addrspace(1)* %input to i32 addrspace(4)*
  %1 = bitcast i32 addrspace(1)* %output to i32 addrspace(4)*
  %.val = load i32 addrspace(4)* %0, align 4
  store i32 %.val, i32 addrspace(4)* %1, align 4
  ret void
}

; Optimized to group load/store.
;
; CHECK-LABEL: @Load_store_group
; CHECK: ld_group_align
; CHECK: st_group_align
define spir_func void @Load_store_group(i32 addrspace(3)* nocapture %input, i32 addrspace(3)* nocapture %output) nounwind {
entry:
  %0 = bitcast i32 addrspace(3)* %input to i32 addrspace(4)*
  %1 = bitcast i32 addrspace(3)* %output to i32 addrspace(4)*
  %.val = load i32 addrspace(4)* %0, align 4
  store i32 %.val, i32 addrspace(4)* %1, align 4
  ret void
}

; Optimized to private load/store.
;
; CHECK-LABEL: @Load_store_private
; CHECK: ld_private_align
; CHECK: st_private_align
define spir_func void @Load_store_private(i32 addrspace(0)* nocapture %input, i32 addrspace(0)* nocapture %output) nounwind {
entry:
  %0 = bitcast i32 addrspace(0)* %input to i32 addrspace(4)*
  %1 = bitcast i32 addrspace(0)* %output to i32 addrspace(4)*
  %.val = load i32 addrspace(4)* %0, align 4
  store i32 %.val, i32 addrspace(4)* %1, align 4
  ret void
}

; No optimization. Flat load/store.
;
; CHECK-LABEL: @Load_store_flat
; CHECK: ld_align
; CHECK: st_align
define spir_func void @Load_store_flat(i32 addrspace(4)* nocapture %input, i32 addrspace(4)* nocapture %output) nounwind {
entry:
  %.val = load i32 addrspace(4)* %input, align 4
  store i32 %.val, i32 addrspace(4)* %output, align 4
  ret void
}