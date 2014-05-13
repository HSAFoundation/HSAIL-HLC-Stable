; RUN: llc < %s -march=amdil | FileCheck %s
; CHECK: il_cs_2_0
; CHECK-NEXT: dcl_cb cb0[15] ; Constant buffer that holds ABI data
; CHECK-NEXT: dcl_literal l0, 0x00000004, 0x00000001, 0x00000002, 0x00000003
; CHECK-NEXT: dcl_literal l1, 0x00FFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFFFD
; CHECK-NEXT: dcl_literal l2, 0x0000FFFF, 0xFFFFFFFE, 0x000000FF, 0xFFFFFFFC
; CHECK-NEXT: dcl_literal l3, 0x00000018, 0x00000010, 0x00000008, 0xFFFFFFFF
; CHECK-NEXT: dcl_literal l4, 0xFFFFFF00, 0xFFFF0000, 0xFF00FFFF, 0xFFFF00FF
; CHECK-NEXT: dcl_literal l5, 0x00000000, 0x00000004, 0x00000008, 0x0000000C
; CHECK-NEXT: dcl_literal l6, 0x00000020, 0x00000020, 0x00000020, 0x00000020
; CHECK-NEXT: dcl_literal l7, 0x00000018, 0x0000001F, 0x00000010, 0x0000001F
; CHECK-NEXT: dcl_literal l8, 0x80000000, 0x80000000, 0x80000000, 0x80000000
; CHECK: endmain
; CHECK: .text
; CHECK: .global@__OpenCL_foo_Kernel
; CHECK: func 1024
; CHECK: ret_dyn
; CHECK: ret
; CHECK: endfunc
; CHECK: end

define void @__OpenCL_foo_Kernel() {
  ret void
}

