; RUN: llc -march=amdil -mcpu=tahiti <%s |FileCheck %s
; Verifies that the copy from r1.x after the first call is not removed by
; machine copy propagation

; CHECK: func {{.*}} ; test
; CHECK: mov r1.x___, r{{[0-9]+}}.x
; CHECK: mov r1._y__, r{{[0-9]+}}.x
; CHECK: call {{.*}} ; __pow_i32
; CHECK: mov [[RES1_REG:r[0-9]+]].x___, r1.x
; CHECK: mov r1.x___, r{{[0-9]+}}.x
; CHECK: mov r1._y__, r{{[0-9]+}}.x
; CHECK: call {{.*}} ; __pow_i32
; CHECK: imul r{{[0-9]+}}.x___, [[RES1_REG]].x, r1.x
; CHECK: endfunc ; test

define  void @__OpenCL_test_kernel(i32 %x, i32 addrspace(1)* nocapture %out) nounwind {
entry:
  %call = tail call i32 @__pow_i32(i32 2, i32 3) nounwind
  %call1 = tail call i32 @__pow_i32(i32 3, i32 4) nounwind
  %tmp2 = mul i32 %call, %call1
  store i32 %tmp2, i32 addrspace(1)* %out, align 4
  ret void
}

define internal fastcc i32 @__pow_i32(i32 %x, i32 %y) nounwind readonly noinline {
  %tmp1 = mul i32 %x, %y
  ret i32 %tmp1
}
