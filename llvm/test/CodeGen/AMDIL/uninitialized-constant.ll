; RUN: llc -march=amdil < %s | FileCheck %s

@__tbl_M32_LOGE = external addrspace(2) unnamed_addr constant [129 x <2 x float>], align 8

; CHECK: .global@__tbl_M32_LOGE:0
; CHECK: ;#float:0:258 <uninitalized constant>

define void @__OpenCL_foo_kernel() {
entry:
  br label %for.body.i

for.exit.i:
  %0 = getelementptr [129 x <2 x float>] addrspace(2)* @__tbl_M32_LOGE, i32 0, i32 undef
  unreachable

for.body.i:
  br i1 undef, label %switch.case89.i, label %dummylabel.i

dummylabel.i:
  br i1 undef, label %for.body.i, label %for.exit.i

switch.case89.i:
  br label %dummylabel.i
}
