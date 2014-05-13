; LSR expected to remove all shifts from the loop
; RUN: llc < %s -march=hsail -filetype=asm -lsr-max-reg-recalc=2 | FileCheck %s
; CHECK-NOT: shl

define void @__OpenCL_test_kernel(i32 addrspace(1)* noalias nocapture %sequenceIn, i32 addrspace(1)* nocapture %sequenceOut) nounwind {
entry:
  %arrayidx = getelementptr i32 addrspace(1)* %sequenceIn, i32 10000
  %tmp1 = load i32 addrspace(1)* %arrayidx, align 4
  br label %for.body

for.exit:                                         ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.02 = phi i32 [ 0, %entry ], [ %tmp6, %for.body ]
  %value.01 = phi i32 [ %tmp1, %entry ], [ %tmp9, %for.body ]
  %tmp6 = add nsw i32 %i.02, 1
  %arrayidx7 = getelementptr i32 addrspace(1)* %sequenceIn, i32 %tmp6
  %tmp8 = load i32 addrspace(1)* %arrayidx7, align 4
  %tmp9 = add i32 %tmp8, %value.01
  %arrayidx13 = getelementptr i32 addrspace(1)* %sequenceOut, i32 %tmp6
  store i32 %tmp9, i32 addrspace(1)* %arrayidx13, align 4
  %cmp = icmp slt i32 %tmp6, 10000
  br i1 %cmp, label %for.body, label %for.exit
}
