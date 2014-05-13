; RUN: llc -march=amdil -mcpu=tonga < %s | FileCheck %s
; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @select_short
; CHECK: ibit_extract
; CHECK: ibit_extract
; CHECK: ilt
; CHECK: cmov_logical
; CHECK-NOT: cmov_logical
; CHECK: ret_dyn
define <2 x i16> @select_short_v2i16(<2 x i16> %x, <2 x i16> %y) {
  %x.0 = extractelement <2 x i16> %x, i32 0
  %y.0 = extractelement <2 x i16> %y, i32 0
  %slt = icmp slt i16 %x.0, %y.0
  %result = select i1 %slt, <2 x i16> %x, <2 x i16> %y
  ret <2 x i16> %result
}

