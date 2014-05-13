; RUN: llc -march=amdil < %s | FileCheck %s

; FIXME: Need to fix literals so kernel function not required to declare them
; Then we can check the value of it
; FIXME: dcl_literal [[LITERAL:l[0-9]+]], 0x0000FFFF, 0xFFFFFFFE, 0x000000FF, 0xFFFFFFFC


define <3 x i32> @sext_vec3(<3 x float> %foo) nounwind {
; FIXME: iadd [[REG:r[0-9]+]], [[REG]].xy0w, [[LITERAL]].00x0

; CHECK: iadd [[REG:r[0-9]+]], {{r[0-9]+}}.xy0w, [[LITERAL:l[0-9]+]].00x0
; CHECK-NEXT: iadd [[REG]], [[REG]].x0zw, [[LITERAL]].0x00
entry:
  %cmp = fcmp oeq <3 x float> %foo, zeroinitializer
  %ext = sext <3 x i1> %cmp to <3 x i32>
  ret <3 x i32> %ext
}

