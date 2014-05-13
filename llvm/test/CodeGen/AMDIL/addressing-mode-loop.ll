; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @sum_array
; CHECK: dcl_literal [[DBLSIZE:l[0-9]+]], 0x00000008,
; CHECK: iadd r{{[0-9]+}}.x___, r{{[0-9]+}}.x, [[DBLSIZE]]
; CHECK-NEXT: break_logicalz
; CHECK-NEXT: endloop
define void @sum_array(double addrspace(1)* nocapture %partialSums, double addrspace(1)* nocapture %finalSum, i32 %numPSums) nounwind {
entry:
  %cmp1 = icmp sgt i32 %numPSums, 0
  br i1 %cmp1, label %for.body, label %for.exit

for.exit:                                         ; preds = %for.body, %entry
  %tempSum.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %tmp6, %for.body ]
  store double %tempSum.0.lcssa, double addrspace(1)* %finalSum, align 8
  ret void

for.body:                                         ; preds = %entry, %for.body
  %i.03 = phi i32 [ %tmp8, %for.body ], [ 0, %entry ]
  %tempSum.02 = phi double [ %tmp6, %for.body ], [ 0.000000e+00, %entry ]
  %conv = sext i32 %i.03 to i64
  %arrayidx = getelementptr double addrspace(1)* %partialSums, i64 %conv
  %tmp5 = load double addrspace(1)* %arrayidx, align 8
  %tmp6 = fadd double %tempSum.02, %tmp5
  %tmp8 = add nsw i32 %i.03, 1
  %cmp = icmp slt i32 %tmp8, %numPSums
  br i1 %cmp, label %for.body, label %for.exit
}
