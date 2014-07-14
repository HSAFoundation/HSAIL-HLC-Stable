; Loads shall be combined into two ld_v4_.*_u32
; This requires that ScheduleDAGSDNodes::ClusterNeighboringLoads() restarts load
; scan from a different offset after shouldScheduleLoadsNear() has returned false.
;
; Stores shall be combined into four st_v4_.*_u16
; This needs clustering restart and clustering of stores in addition to loads.
;
; RUN: llc < %s -march=hsail -filetype=asm | FileCheck %s
; CHECK: ld_v4
; CHECK: ld_v4
; CHECK: st_v4
; CHECK: st_v4
; CHECK: st_v4
; CHECK: st_v4

define void @__OpenCL_test_kernel(<16 x i16> addrspace(1)* nocapture %x) nounwind {
entry:
  %arrayidx = getelementptr <16 x i16> addrspace(1)* %x, i32 1
  %tmp3 = load <16 x i16> addrspace(1)* %x, align 32
  %tmp4 = add nsw <16 x i16> %tmp3, <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  store <16 x i16> %tmp4, <16 x i16> addrspace(1)* %arrayidx, align 32
  ret void
}
