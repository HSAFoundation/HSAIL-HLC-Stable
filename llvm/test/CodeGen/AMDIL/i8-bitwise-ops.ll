; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @and_i8_arg
; CHECK: func
; CHECK-NEXT: iand
; CHECK-NEXT: ret_dyn
define i8 @and_i8_arg(i8 %a, i8 %b) nounwind  {
  %and = and i8 %a, %b
  ret i8 %and
}

; CHECK-LABEL: @and_v2i8_arg
; CHECK: ubit_extract
; CHECK iand
define <2 x i8> @and_v2i8_arg(<2 x i8> %a, <2 x i8> %b) nounwind  {
  %and = and <2 x i8> %a, %b
  ret <2 x i8> %and
}

; CHECK-LABEL: @and_v4i8_arg
; CHECK: ubit_extract
; CHECK iand
define <4 x i8> @and_v4i8_arg(<4 x i8> %a, <4 x i8> %b) nounwind  {
  %and = and <4 x i8> %a, %b
  ret <4 x i8> %and
}

; CHECK-LABEL: @and_i8_const_arg
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK iand
define i8 @and_i8_const_arg(i8 %a) nounwind  {
  %and = and i8 %a, 3
  ret i8 %and
}

; CHECK-LABEL: @and_v2i8_const_arg
; CHECK: ubit_extract
; CHECK: ubit_extract
define <2 x i8> @and_v2i8_const_arg(<2 x i8> %b) nounwind  {
  %and = and <2 x i8> %b, <i8 3, i8 3>
  ret <2 x i8> %and
}

; CHECK-LABEL: @and_v4i8_const_arg
define <4 x i8> @and_v4i8_const_arg(<4 x i8> %b) nounwind  {
  %and = and <4 x i8> %b, <i8 3, i8 3, i8 3, i8 3>
  ret <4 x i8> %and
}

; CHECK-LABEL: @or_i8_arg
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK ior
define i8 @or_i8_arg(i8 %a, i8 %b) nounwind  {
  %and = or i8 %a, %b
  ret i8 %and
}

; CHECK-LABEL: @or_v2i8_arg
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK: ubit_extract
define <2 x i8> @or_v2i8_arg(<2 x i8> %a, <2 x i8> %b) nounwind  {
  %or = or <2 x i8> %a, %b
  ret <2 x i8> %or
}

; CHECK-LABEL: @or_v4i8_arg
define <4 x i8> @or_v4i8_arg(<4 x i8> %a, <4 x i8> %b) nounwind  {
  %or = or <4 x i8> %a, %b
  ret <4 x i8> %or
}

; CHECK-LABEL: @or_i8_const_arg
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK ior
define i8 @or_i8_const_arg(i8 %a) nounwind  {
  %and = or i8 %a, 3
  ret i8 %and
}

; CHECK-LABEL: @or_v2i8_const_arg
define <2 x i8> @or_v2i8_const_arg(<2 x i8> %b) nounwind  {
  %or = or <2 x i8> %b, <i8 3, i8 3>
  ret <2 x i8> %or
}

; CHECK-LABEL: @or_v4i8_const_arg
define <4 x i8> @or_v4i8_const_arg(<4 x i8> %b) nounwind  {
  %or = or <4 x i8> %b, <i8 3, i8 3, i8 3, i8 3>
  ret <4 x i8> %or
}

; CHECK-LABEL: @xor_i8_arg
; CHECK: ubit_extract
; CHECK: ubit_extract
; CHECK ixor
define i8 @xor_i8_arg(i8 %a, i8 %b) nounwind  {
  %and = xor i8 %a, %b
  ret i8 %and
}

; CHECK-LABEL: @xor_v2i8_arg
define <2 x i8> @xor_v2i8_arg(<2 x i8> %a, <2 x i8> %b) nounwind  {
  %xor = xor <2 x i8> %a, %b
  ret <2 x i8> %xor
}

; CHECK-LABEL: @xor_v4i8_arg
define <4 x i8> @xor_v4i8_arg(<4 x i8> %a, <4 x i8> %b) nounwind  {
  %xor = xor <4 x i8> %a, %b
  ret <4 x i8> %xor
}

; CHECK-LABEL: @xor_i8_const_arg
; CHECK: ubit_extract
; CHECK ixor
define i8 @xor_i8_const_arg(i8 %a) nounwind  {
  %and = xor i8 %a, 3
  ret i8 %and
}

; CHECK-LABEL: @xor_v2i8_const_arg
; CHECK: ubit_extract
; CHECK ixor
define <2 x i8> @xor_v2i8_const_arg(<2 x i8> %b) nounwind  {
  %xor = xor <2 x i8> %b, <i8 3, i8 3>
  ret <2 x i8> %xor
}

; CHECK-LABEL: @xor_v4i8_const_arg
; CHECK: ubit_extract
; CHECK ixor
define <4 x i8> @xor_v4i8_const_arg(<4 x i8> %b) nounwind  {
  %xor = xor <4 x i8> %b, <i8 3, i8 3, i8 3, i8 3>
  ret <4 x i8> %xor
}


