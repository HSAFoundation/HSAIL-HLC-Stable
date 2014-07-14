; There shall be vec3 load and store here
;
; RUN: llc < %s -march=hsail -filetype=asm | FileCheck %s
; CHECK: ld_v3
; CHECK: st_v3

define void @test(<3 x i32> addrspace(1)* nocapture %in, <3 x i32> addrspace(1)* nocapture %out) nounwind {
entry:
  %arrayidx = getelementptr <3 x i32> addrspace(1)* %out, i32 0
  %arrayidx4 = getelementptr <3 x i32> addrspace(1)* %in, i32 0
  %tmp5 = load <3 x i32> addrspace(1)* %arrayidx4, align 16
  store <3 x i32> %tmp5, <3 x i32> addrspace(1)* %arrayidx, align 16
  ret void
}
