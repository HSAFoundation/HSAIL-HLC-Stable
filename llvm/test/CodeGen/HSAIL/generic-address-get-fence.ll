; get_fence optimization
;
; RUN: llc -march=hsail-64 -filetype=asm -o - %s | FileCheck %s

; ModuleID = 'get-fence-optimization.bc'
target triple = "hsail64-pc-unknown-amdopencl"

declare spir_func i32 @_Z9get_fencePU3AS4v(i8 addrspace(4)*)

; Local address space ==> CLK_LOCAL_MEM_FENCE (==2)
;
; CHECK-LABEL: @Local_fence_group
; CHECK: mov_b32 $s[[NUM:[0-9]+]], 2;
; CHECK: st_group_align(4)_u32 $s[[NUM]]
define spir_func void @Local_fence_group(i32 addrspace(3)* %output) nounwind {
entry:
  %0 = bitcast i32 addrspace(3)* %output to i32 addrspace(4)*
  %1 = bitcast i32 addrspace(3)* %output to i8 addrspace(4)*
  %call.i = tail call spir_func i32 @_Z9get_fencePU3AS4v(i8 addrspace(4)* %1) nounwind
  store i32 %call.i, i32 addrspace(4)* %0, align 4
  ret void
}

; Private address space ==> CLK_LOCAL_MEM_FENCE (==2)
;
; CHECK-LABEL: @Local_fence_private
; CHECK: mov_b32 $s[[NUM:[0-9]+]], 2;
; CHECK: st_private_align(4)_u32 $s[[NUM]]
define spir_func void @Local_fence_private(i32 addrspace(0)* %output) nounwind {
entry:
  %0 = bitcast i32 addrspace(0)* %output to i32 addrspace(4)*
  %1 = bitcast i32 addrspace(0)* %output to i8 addrspace(4)*
  %call.i = tail call spir_func i32 @_Z9get_fencePU3AS4v(i8 addrspace(4)* %1) nounwind
  store i32 %call.i, i32 addrspace(4)* %0, align 4
  ret void
}

; Global address space ==> CLK_GLOBAL_MEM_FENCE (==1)
;
; CHECK-LABEL: @Global_fence_global
; CHECK: mov_b32 $s[[NUM:[0-9]+]], 1;
; CHECK: st_global_align(4)_u32 $s[[NUM]]
define spir_func void @Global_fence_global(i32 addrspace(1)* %output) nounwind {
entry:
  %0 = bitcast i32 addrspace(1)* %output to i32 addrspace(4)*
  %1 = bitcast i32 addrspace(1)* %output to i8 addrspace(4)*
  %call.i = tail call spir_func i32 @_Z9get_fencePU3AS4v(i8 addrspace(4)* %1) nounwind
  store i32 %call.i, i32 addrspace(4)* %0, align 4
  ret void
}

; Generic address space ==> CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE (==3)
;
; CHECK-LABEL: @Global_local_fence_generic
; CHECK: mov_b32 $s[[NUM:[0-9]+]], 3;
; CHECK: st_align(4)_u32 $s[[NUM]]
define spir_func void @Global_local_fence_generic(i32 addrspace(4)* %output) nounwind {
entry:
  %0 = bitcast i32 addrspace(4)* %output to i8 addrspace(4)*
  %call.i = tail call spir_func i32 @_Z9get_fencePU3AS4v(i8 addrspace(4)* %0) nounwind
  store i32 %call.i, i32 addrspace(4)* %output, align 4
  ret void
}