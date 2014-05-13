; LSR expected to remove all shifts from the loop
; RUN: llc < %s -march=hsail -filetype=asm | FileCheck %s
; CHECK-NOT: shl

define void @test(i32 addrspace(1)* nocapture %B, i32 %n, i32 %m, i32 addrspace(2)* nocapture %B_const) nounwind {
entry:
  %arrayidx1 = getelementptr i32 addrspace(1)* %B, i32 8
  %tmp32 = load i32 addrspace(1)* %arrayidx1, align 4
  %cmp3 = icmp sgt i32 %tmp32, %n
  br i1 %cmp3, label %for.body, label %for.exit

for.exit:                                         ; preds = %if.end, %entry
  ret void

for.body:                                         ; preds = %if.end, %entry
  %arrayidx5 = phi i32 addrspace(1)* [ %arrayidx, %if.end ], [ %arrayidx1, %entry ]
  %j.04 = phi i32 [ %tmp15, %if.end ], [ 8, %entry ]
  %cmp6 = icmp sgt i32 %j.04, %m
  br i1 %cmp6, label %if.then, label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %tmp15 = add nsw i32 %j.04, 9
  %arrayidx = getelementptr i32 addrspace(1)* %B, i32 %tmp15
  %tmp3 = load i32 addrspace(1)* %arrayidx, align 4
  %cmp = icmp sgt i32 %tmp3, %n
  br i1 %cmp, label %for.body, label %for.exit

if.then:                                          ; preds = %for.body
  %arrayidx12 = getelementptr i32 addrspace(2)* %B_const, i32 %j.04
  %tmp13 = load i32 addrspace(2)* %arrayidx12, align 4
  store i32 %tmp13, i32 addrspace(1)* %arrayidx5, align 4
  br label %if.end
}
