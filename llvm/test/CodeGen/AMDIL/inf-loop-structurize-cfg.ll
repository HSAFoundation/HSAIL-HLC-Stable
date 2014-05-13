; RUN: llc -march=amdil < %s | FileCheck %s

; FIXME: Why the useless break on constant?

; CHECK-LABEL: .global@inf_loop_irreducible_cfg
; CHECK: whileloop
; CHECK-NEXT: mov [[REG:r[0-9]+]], l
; CHECK-NEXT: break_logicalz [[REG]]
; CHECK-NEXT: endloop
; CHECK-NEXT: ret
define void @inf_loop_irreducible_cfg() nounwind {
entry:
  br label %block.a

block.a:
  br label %block.a
}
