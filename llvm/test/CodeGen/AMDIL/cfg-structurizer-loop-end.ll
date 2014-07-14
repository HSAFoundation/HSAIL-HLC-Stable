; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s
;
; source code
; kernel void foo(global int* a) {
;  for (int i = 0; i < 1024; ++i) {
;    if (a[0] > 0)
;      break;
;    if (a[1] > 0)
;      return;
;    if (a[2] > 0)
;      return;
;  }
;  a[3] = 1;
;}
;
; structurizer translates it to something like
; kernel void foo(global int* a) {
;  for (int i = 0; i < 1024; ++i) {
;    if (a[0] > 0) {
;      reg1 = lit1;
;      break;
;    }
;    if (a[1] > 0) {
;      reg1 = lit2;
;      break;
;     }
;    if (a[2] > 0) {
;       reg1 = lit2;
;      break;
;    }
;  }
;  reg2 = lit1;
;  if (reg1 == reg2) {
;    a[3] = 1;
;  }
;}
;
; CHECK-LABEL: func {{[0-9]+}}
; CHECK: whileloop
; CHECK: break_logicalnz
; CHECK: mov [[reg1:r[0-9]+]].x___, [[lit1:l[0-9]+]]
; CHECK: break_logicalnz
; CHECK: mov [[reg1]].x___, [[lit2:l[0-9]+]]
; CHECK: break_logicalnz
; CHECK: mov [[reg1]].x___, [[lit2]]
; CHECK: break_logicalnz
; CHECK: endloop
; CHECK: mov [[reg2:r[0-9]+]].x___, [[lit1]]
; CHECK: ieq [[reg3:r[0-9]+]].x___, [[reg1]].x, [[reg2]].x
; CHECK: if_logicalnz [[reg3]].x
; CHECK: uav_raw_store
; CHECK: ret_dyn
; CHECK: else
; CHECK: ret_dyn
; CHECK: endif
; CHECK: endfunc
; CHECK-NOT: error
; CHECK-NOT: warning

define void @__OpenCL_foo_kernel(i32 addrspace(1)* %a) nounwind {
entry:
  %a.addr = alloca i32 addrspace(1)*, align 8
  %i = alloca i32, align 4
  store i32 addrspace(1)* %a, i32 addrspace(1)** %a.addr, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

return:                                           ; preds = %if.then15, %if.then9, %dummylabel
  ret void

for.cond:                                         ; preds = %for.inc, %entry
  %tmp = load i32* %i, align 4
  %cmp = icmp slt i32 %tmp, 1024
  br i1 %cmp, label %for.body, label %for.exit

for.exit:                                         ; preds = %for.cond
  br label %dummylabel

for.body:                                         ; preds = %for.cond
  %tmp1 = load i32 addrspace(1)** %a.addr, align 8
  %arrayidx = getelementptr i32 addrspace(1)* %tmp1, i64 0
  %tmp2 = load i32 addrspace(1)* %arrayidx, align 4
  %cmp3 = icmp sgt i32 %tmp2, 0
  br i1 %cmp3, label %if.then, label %if.end

for.inc:                                          ; preds = %if.end14
  %tmp16 = load i32* %i, align 4
  %tmp17 = add nsw i32 %tmp16, 1
  store i32 %tmp17, i32* %i, align 4
  br label %for.cond

if.end:                                           ; preds = %for.body
  %tmp4 = load i32 addrspace(1)** %a.addr, align 8
  %arrayidx5 = getelementptr i32 addrspace(1)* %tmp4, i64 1
  %tmp6 = load i32 addrspace(1)* %arrayidx5, align 4
  %cmp7 = icmp sgt i32 %tmp6, 0
  br i1 %cmp7, label %if.then9, label %if.end8

if.then:                                          ; preds = %for.body
  br label %dummylabel

dummylabel:                                       ; preds = %if.then, %for.exit
  %tmp18 = load i32 addrspace(1)** %a.addr, align 8
  %arrayidx19 = getelementptr i32 addrspace(1)* %tmp18, i64 3
  store i32 1, i32 addrspace(1)* %arrayidx19, align 4
  br label %return

if.end8:                                          ; preds = %if.end
  %tmp10 = load i32 addrspace(1)** %a.addr, align 8
  %arrayidx11 = getelementptr i32 addrspace(1)* %tmp10, i64 2
  %tmp12 = load i32 addrspace(1)* %arrayidx11, align 4
  %cmp13 = icmp sgt i32 %tmp12, 0
  br i1 %cmp13, label %if.then15, label %if.end14

if.then9:                                         ; preds = %if.end
  br label %return

if.end14:                                         ; preds = %if.end8
  br label %for.inc

if.then15:                                        ; preds = %if.end8
  br label %return
}
