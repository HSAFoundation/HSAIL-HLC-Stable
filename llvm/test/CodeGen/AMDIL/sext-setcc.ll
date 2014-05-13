; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s

; EPR 392093
; sext (setcc x, y, cc) -> select (setcc x, y, cc) -1, 0
; was using the wrong type for setcc when doing a 64-bit comparison.

; CHECK-LABEL .global@test_zext_fcmp_f64
; CHECK: dlt [[REG:r[0-9]+]].xy__,
; CHECK: iand {{.*}}, [[REG]].x,
; CHECK: ret_dyn
define i32 @test_zext_fcmp_f64(double %arg) {
  %x = fcmp olt double %arg, 0.000000e+00
  %y = zext i1 %x to i32
  ret i32 %y
}

; CHECK-LABEL .global@test_sext_fcmp_f64
; CHECK: dlt [[REG:r[0-9]+]].xy__,
; CHECK: cmov_logical
; CHECK: ret_dyn
define i32 @test_sext_fcmp_f64(double %arg) {
  %x = fcmp olt double %arg, 0.000000e+00
  %y = sext i1 %x to i32
  ret i32 %y
}

; CHECK-LABEL .global@test_zext_fcmp_f32
; CHECK: lt [[REG:r[0-9]+]].x___,
; CHECK: iand {{.*}}, [[REG]].x,
; CHECK: ret_dyn
define i32 @test_zext_fcmp_f32(float %arg) {
  %x = fcmp olt float %arg, 0.000000e+00
  %y = zext i1 %x to i32
  ret i32 %y
}

; CHECK-LABEL .global@test_sext_fcmp_f32
; CHECK: lt [[REG:r[0-9]+]].x___,
; CHECK: cmov_logical
; CHECK: ret_dyn
define i32 @test_sext_fcmp_f32(float %arg) {
  %x = fcmp olt float %arg, 0.000000e+00
  %y = sext i1 %x to i32
  ret i32 %y
}

; CHECK-LABEL .global@test_zext_icmp_i64
; CHECK: i64eq [[REG:r[0-9]+]].xy__,
; CHECK: iand
; CHECK: ret_dyn
define i32 @test_zext_icmp_i64(i64 %arg) {
  %x = icmp eq i64 %arg, 0
  %y = zext i1 %x to i32
  ret i32 %y
}
; CHECK-LABEL .global@test_sext_icmp_i64
; CHECK: i64eq [[REG:r[0-9]+]].xy__,
; CHECK: cmov_logical
; CHECK: ret_dyn
define i32 @test_sext_icmp_i64(i64 %arg) {
  %x = icmp eq i64 %arg, 0
  %y = sext i1 %x to i32
  ret i32 %y
}

; CHECK-LABEL .global@test_zext_icmp_i32
; CHECK: ieq [[REG:r[0-9]+]].x___,
; CHECK: iand
; CHECK: ret_dyn
define i32 @test_zext_icmp_i32(i32 %arg) {
  %x = icmp eq i32 %arg, 0
  %y = zext i1 %x to i32
  ret i32 %y
}

; CHECK-LABEL .global@test_sext_icmp_i32
; CHECK: ieq [[REG:r[0-9]+]].x___,
; CHECK: cmov_logical
; CHECK: ret_dyn
define i32 @test_sext_icmp_i32(i32 %arg) {
  %x = icmp eq i32 %arg, 0
  %y = sext i1 %x to i32
  ret i32 %y
}

