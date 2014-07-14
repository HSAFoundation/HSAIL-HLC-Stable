; Optimizations of to_global/to_local/to_private builtins.
;
; RUN: llc -march=hsail-64 -filetype=asm -o - %s | FileCheck %s

; ModuleID = 'to-segment-optimization'
target triple = "hsail64-pc-unknown-amdopencl"

declare spir_func i8 addrspace(3)* @_Z8to_localPU3AS4v(i8 addrspace(4)*)
declare spir_func i8 addrspace(0)* @_Z10to_privatePU3AS4v(i8 addrspace(4)*)
declare spir_func i8 addrspace(1)* @_Z9to_globalPU3AS4v(i8 addrspace(4)*)
declare spir_func void @__hsail_barrier()
declare spir_func i32 @__hsail_get_global_id(i32) nounwind readnone

; Optimize away to_local.
;
; CHECK-LABEL: @Local_generic_to_local
; CHECK: ld_arg_u64 $d[[NUM0:[0-9]+]], [%arg_p0];
; CHECK-NEXT: mov_b32 $s[[NUMS:[0-9]+]], 1;
; CHECK-NEXT: st_group_align(4)_u32 $s[[NUMS]], [$d[[NUM0]]];
; CHECK-NEXT: ret;
define spir_func void @Local_generic_to_local(i32 addrspace(3)* %output) nounwind {
entry:
  %0 = bitcast i32 addrspace(3)* %output to i8 addrspace(4)*
  %call.i = tail call spir_func i8 addrspace(3)* @_Z8to_localPU3AS4v(i8 addrspace(4)* %0) nounwind
  %1 = bitcast i8 addrspace(3)* %call.i to i32 addrspace(3)*
  store i32 1, i32 addrspace(3)* %1, align 4
  ret void
}

; Optimize away to_private.
;
; CHECK-LABEL: @Private_generic_to_private
; CHECK: ld_arg_u64 $d[[NUM0:[0-9]+]], [%arg_p0];
; CHECK-NEXT: mov_b32 $s[[NUMS:[0-9]+]], 1;
; CHECK-NEXT: st_private_align(4)_u32 $s[[NUMS]], [$d[[NUM0]]];
; CHECK-NEXT: ret;
define spir_func void @Private_generic_to_private(i32 addrspace(0)* %output) nounwind {
entry:
  %0 = bitcast i32 addrspace(0)* %output to i8 addrspace(4)*
  %call.i = tail call spir_func i8 addrspace(0)* @_Z10to_privatePU3AS4v(i8 addrspace(4)* %0) nounwind
  %1 = bitcast i8 addrspace(0)* %call.i to i32 addrspace(0)*
  store i32 1, i32 addrspace(0)* %1, align 4
  ret void
}

; Optimize away to_global.
;
; CHECK-LABEL: @Global_generic_to_global
; CHECK: ld_arg_u64 $d[[NUM0:[0-9]+]], [%arg_p0];
; CHECK-NEXT: mov_b32 $s[[NUMS:[0-9]+]], 1;
; CHECK-NEXT: st_global_align(4)_u32 $s[[NUMS]], [$d[[NUM0]]];
; CHECK-NEXT: ret;
define spir_func void @Global_generic_to_global(i32 addrspace(1)* %output) nounwind {
entry:
  %0 = bitcast i32 addrspace(1)* %output to i8 addrspace(4)*
  %call.i = tail call spir_func i8 addrspace(1)* @_Z9to_globalPU3AS4v(i8 addrspace(4)* %0) nounwind
  %1 = bitcast i8 addrspace(1)* %call.i to i32 addrspace(1)*
  store i32 1, i32 addrspace(1)* %1, align 4
  ret void
}

; Could not optimize away. Expand to_local to ftos_group,
; segmentp_group and cmov.
;
; CHECK-LABEL: Generic_to_local
; CHECK: ftos_group_u64_u64 $d[[NUM1:[0-9]+]], $d[[NUM0:[0-9]+]];
; CHECK-NEXT: segmentp_group_b1_u64  $c[[NUMC:[0-9]+]], $d[[NUM0]];
; CHECK-NEXT: cmov_b64  $d{{[0-9]+}}, $c[[NUMC]], $d[[NUM1]], 0;
define spir_func void @Generic_to_local(i32 addrspace(1)* nocapture %output, i32 addrspace(4)* %input) nounwind {
entry:
  %0 = bitcast i32 addrspace(4)* %input to i8 addrspace(4)*
  %call.i = tail call spir_func i8 addrspace(3)* @_Z8to_localPU3AS4v(i8 addrspace(4)* %0) nounwind
  %not.tobool.i = icmp ne i8 addrspace(3)* %call.i, null
  %..i = zext i1 %not.tobool.i to i32
  store i32 %..i, i32 addrspace(1)* %output, align 4
  ret void
}

; Could not optimize away. Need stof for AddrSpaceCast, and
; expand to_global to ftos_global, segmentp_global and cmov.
;
; Check-LABEL: @__OpenCL_testKernel_kernel
; CHECK stof_global_u64_u64
; CHECK stof_group_u64_u64
; CHECK: ftos_global_u64_u64 $d[[NUM1:[0-9]+]], $d[[NUM0:[0-9]+]];
; CHECK-NEXT: segmentp_global_b1_u64  $c[[NUMC:[0-9]+]], $d[[NUM0]];
; CHECK-NEXT: cmov_b64  $d{{[0-9]+}}, $c[[NUMC]], $d[[NUM1]], 0;
@gint = internal addrspace(1) global i32 1, align 4
@testKernel.lint = internal addrspace(3) global i32 0, align 4
define spir_kernel void @__OpenCL_testKernel_kernel(i32 addrspace(1)* nocapture %results) nounwind {
entry:
  %0 = tail call spir_func i32 @__hsail_get_global_id(i32 0) nounwind readnone
  store i32 2, i32 addrspace(3)* @testKernel.lint, align 4
  %rem = and i32 %0, 1
  %tobool = icmp eq i32 %rem, 0
  tail call spir_func void @__hsail_barrier() nounwind
  %1 = select i1 %tobool, i8 addrspace(4)* bitcast (i32 addrspace(3)* @testKernel.lint to i8 addrspace(4)*), i8 addrspace(4)* bitcast (i32 addrspace(1)* @gint to i8 addrspace(4)*)
  %call1 = tail call spir_func i8 addrspace(1)* @_Z9to_globalPU3AS4v(i8 addrspace(4)* %1) nounwind
  %cmp = icmp ne i8 addrspace(1)* %call1, null
  %conv2 = zext i1 %cmp to i32
  %idxprom = zext i32 %0 to i64
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %results, i64 %idxprom
  store i32 %conv2, i32 addrspace(1)* %arrayidx, align 4
  ret void
}
