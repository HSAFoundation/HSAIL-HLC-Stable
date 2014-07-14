; RUN: llc -march=amdil -o /dev/null %s

declare float @foo(float) nounwind readnone

; OpenCL bug 9412

; CHECK-LABEL: @loop_land_info_assert
define void @loop_land_info_assert() nounwind {
entry:
  %cmp = icmp sgt i32 undef, 0
  br label %while.cond.outer

while.cond.outer:
  %tmp = load float* undef
  br label %while.cond

while.cond:
  %cmp1 = icmp slt i32 undef, 4
  br i1 %cmp1, label %convex.exit, label %for.cond

convex.exit:
  %or = or i1 undef, undef
  br i1 %or, label %return, label %if.end

if.end:
  %tmp3 = call float @foo(float %tmp) nounwind
  %cmp2 = fcmp olt float %tmp3, 0x3E80000000000000
  br i1 %cmp2, label %if.else, label %while.cond.outer

if.else:
  store i32 3, i32* undef, align 16
  br label %while.cond

for.cond:
  %cmp3 = icmp slt i32 undef, 1000
  br i1 %cmp3, label %for.body, label %return

for.body:
  br i1 %cmp3, label %self.loop, label %if.end.2

if.end.2:
  %or.cond2 = or i1 undef, undef
  br i1 %or.cond2, label %return, label %for.cond

self.loop:
  br label %self.loop

return:
  ret void
}
