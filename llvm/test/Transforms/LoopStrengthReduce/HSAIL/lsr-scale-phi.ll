; LSR should create only a single shl for the loop
; RUN: llc < %s -march=hsail -filetype=asm | FileCheck %s
; CHECK: shl
; CHECK-NOT: shl

define void @test(double addrspace(1)* noalias nocapture %val, double addrspace(1)* noalias nocapture %vec, i32 addrspace(1)* noalias nocapture %cols, double addrspace(1)* noalias nocapture %out) nounwind {
entry:
  br label %for.body

for.exit:                                         ; preds = %for.body
  store double %tmp14, double addrspace(1)* %out, align 8
  ret void

for.body:                                         ; preds = %for.body, %entry
  %j.02 = phi i32 [ 0, %entry ], [ %tmp16, %for.body ]
  %t.01 = phi double [ 0.000000e+00, %entry ], [ %tmp14, %for.body ]
  %arrayidx = getelementptr i32 addrspace(1)* %cols, i32 %j.02
  %tmp3 = load i32 addrspace(1)* %arrayidx, align 4
  %arrayidx7 = getelementptr double addrspace(1)* %val, i32 %j.02
  %tmp8 = load double addrspace(1)* %arrayidx7, align 8
  %arrayidx11 = getelementptr double addrspace(1)* %vec, i32 %tmp3
  %tmp12 = load double addrspace(1)* %arrayidx11, align 8
  %tmp13 = fmul double %tmp8, %tmp12
  %tmp14 = fadd double %t.01, %tmp13
  %tmp16 = add nsw i32 %j.02, 1
  %cmp = icmp slt i32 %tmp16, 10000
  br i1 %cmp, label %for.body, label %for.exit
}
