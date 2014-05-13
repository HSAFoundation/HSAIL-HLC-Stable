; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @trunc_i64_add_to_i32
; CHECK-NOT: i64add
; CHECK: iadd
define void @trunc_i64_add_to_i32(i32 addrspace(1)* %out, i64 %a, i64 %b) {
  %add = add i64 %b, %a
  %trunc = trunc i64 %add to i32
  store i32 %trunc, i32 addrspace(1)* %out, align 8
  ret void
}

; CHECK-LABEL: @trunc_i64_mul_to_i32
; CHECK-NOT: i64mul
; CHECK: imul
define void @trunc_i64_mul_to_i32(i32 addrspace(1)* %out, i64 %a, i64 %b) {
  %add = mul i64 %b, %a
  %trunc = trunc i64 %add to i32
  store i32 %trunc, i32 addrspace(1)* %out, align 8
  ret void
}

; CHECK-LABEL: @trunc_i64_or_to_i32
; CHECK-NOT: ior r{{[0-9]+}}.xy__,
; CHECK: ior r{{[0-9]+}}.x___,
define void @trunc_i64_or_to_i32(i32 addrspace(1)* %out, i64 %a, i64 %b) {
  %add = or i64 %b, %a
  %trunc = trunc i64 %add to i32
  store i32 %trunc, i32 addrspace(1)* %out, align 8
  ret void
}


