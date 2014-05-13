; RUN: llc -march=amdil -mcpu=cypress < %s | FileCheck %s

define void @t1(i32* %p, i32 %x) {
; CHECK-LABEL: @t1
; CHECK: dcl_literal [[LIT2:l[0-9]+]], 0x00000002,
; CHECK: dcl_literal [[LIT3:l[0-9]+]], 0x00000003,
; CHECK:      ishr [[TMPREG:r[0-9]+]].x___, r1.x, [[LIT2]]
; CHECK-NEXT: iand [[AND:r[0-9]+]].x___, [[TMPREG]].x, [[LIT3]]
; CHECK-NEXT: ishr [[SHR:r[0-9]+]].x___, [[TMPREG]].x, [[LIT2]]
; CHECK-NEXT: switch [[AND]].x
; CHECK-NEXT: default
; CHECK-NEXT: mov x1{{\[}}[[SHR]].x].x, r1.y
; CHECK-NEXT: break
; CHECK-NEXT: case 1
; CHECK-NEXT: mov x1{{\[}}[[SHR]].x].y, r1.y
; CHECK-NEXT: break
; CHECK-NEXT: case 2
; CHECK-NEXT: mov x1{{\[}}[[SHR]].x].z, r1.y
; CHECK-NEXT: break
; CHECK-NEXT: case 3
; CHECK-NEXT: mov x1{{\[}}[[SHR]].x].w, r1.y
; CHECK-NEXT: break
; CHECK-NEXT: endswitch

  store i32 %x, i32* %p
  ret void
}
