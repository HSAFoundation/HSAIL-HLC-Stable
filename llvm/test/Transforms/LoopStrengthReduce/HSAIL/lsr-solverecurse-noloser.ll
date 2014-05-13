; LSR supposed to remove all shifts from the loop
; RUN: llc < %s -march=hsail -filetype=asm | FileCheck %s
; CHECK-NOT: shl

define void @test(float addrspace(1)* nocapture %matrixA, float addrspace(1)* nocapture %matrixC) nounwind {
entry:
  br label %for.body

for.exit:                                         ; preds = %for.body
  store float %tmp5, float addrspace(1)* %matrixC, align 4
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.02 = phi i32 [ 0, %entry ], [ %tmp7, %for.body ]
  %sum.01 = phi float [ 0.000000e+00, %entry ], [ %tmp5, %for.body ]
  %arrayidx = getelementptr float addrspace(1)* %matrixA, i32 %i.02
  %tmp4 = load float addrspace(1)* %arrayidx, align 4
  %tmp5 = fadd float %sum.01, %tmp4
  %tmp7 = add nsw i32 %i.02, 1
  %exitcond = icmp eq i32 %tmp7, 1000
  br i1 %exitcond, label %for.exit, label %for.body
}
