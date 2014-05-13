; RUN: opt < %s -basicaa -gvn -S | FileCheck %s

; We don't want to simplify loads if we will hoist them in different empty basic block.
; This handles only the case when we have splitted critical edge at the end of the loop.
; There should not be loads before the last branch.

define void @test(<4 x i32> addrspace(1)* nocapture %input, <4 x i32> addrspace(3)* nocapture %shared) nounwind {
entry:
  br label %for.body

for.body:                                         ; preds = %entry
  %arrayidx30 = getelementptr <4 x i32> addrspace(1)* %input, i32 1
  %tmp31 = load <4 x i32> addrspace(1)* %arrayidx30, align 16
  store <4 x i32> %tmp31, <4 x i32> addrspace(3)* %shared, align 16
  br label %for.body38

for.exit35:                                       ; preds = %for.body38
  ret void

for.body38:                                       ; preds = %for.body, %for.body38
  %k.010 = phi i32 [ 0, %for.body ], [ %tmp358, %for.body38 ]
  %temp.09 = phi <4 x i32> [ zeroinitializer, %for.body ], [ %tmp356, %for.body38 ]
  %tmp86 = extractelement <4 x i32> %temp.09, i32 0
  %arrayidx98 = getelementptr <4 x i32> addrspace(3)* %shared, i32 %k.010
  %tmp99 = load <4 x i32> addrspace(3)* %arrayidx98, align 16
  %tmp100 = extractelement <4 x i32> %tmp99, i32 0
  %tmp356 = insertelement <4 x i32> undef, i32 1, i32 3
  %tmp358 = add nsw i32 %k.010, 1
  %cmp37 = icmp slt i32 %tmp358, 3
  br i1 %cmp37, label %for.body38, label %for.exit35

; CHECK: for.body38.for.body38_crit_edge:{{.*$}}
; CHECK-NEXT: {{br label %for\.body38}}
}
