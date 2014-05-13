; There shall be no cbr immediately followed by a brn in this test
;
; RUN: llc < %s -march=hsail -filetype=asm | not FileCheck %s
; CHECK: barrier;
; CHECK-NEXT: {{ +cbr +.*$}}
; CHECK-NEXT: {{ +brn +}}

define void @test(i32 addrspace(1)* nocapture %x, i32 %y, i32 %z) nounwind {
entry:
  %cmp = icmp eq i32 %z, 0
  %arrayidx = getelementptr i32 addrspace(1)* %x, i32 1
  %cmp3 = icmp eq i32 %y, 1
  %cmp13 = icmp slt i32 %y, 3
  br label %while.body

return:                                           ; preds = %if.then25, %if.end18, %if.then9, %if.then5
  ret void

while.body:                                       ; preds = %if.else, %entry
  br i1 %cmp, label %if.then, label %if.end

if.end:                                           ; preds = %if.then, %while.body
  br i1 %cmp3, label %if.then5, label %if.else

if.then:                                          ; preds = %while.body
  store i32 4, i32 addrspace(1)* %arrayidx, align 4
  br label %if.end

if.then5:                                         ; preds = %if.end
  br i1 %cmp, label %if.then9, label %return

if.else:                                          ; preds = %if.end
  br i1 %cmp13, label %if.then15, label %while.body

if.then9:                                         ; preds = %if.then5
  %arrayidx11 = getelementptr i32 addrspace(1)* %x, i32 2
  store i32 3, i32 addrspace(1)* %arrayidx11, align 4
  br label %return

if.then15:                                        ; preds = %if.else
  %cmp17 = icmp slt i32 %z, 4
  br i1 %cmp17, label %if.then19, label %if.end18

if.end18:                                         ; preds = %if.then19, %if.then15
  br i1 %cmp, label %if.then25, label %return

if.then19:                                        ; preds = %if.then15
  %arrayidx21 = getelementptr i32 addrspace(1)* %x, i32 3
  store i32 2, i32 addrspace(1)* %arrayidx21, align 4
  tail call void @__hsail_barrier() nounwind
  br label %if.end18

if.then25:                                        ; preds = %if.end18
  %arrayidx27 = getelementptr i32 addrspace(1)* %x, i32 4
  store i32 1, i32 addrspace(1)* %arrayidx27, align 4
  br label %return
}

declare void @__hsail_barrier() nounwind
