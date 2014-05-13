; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @select_cc_fcmp_oeq_f32_00
; CHECK: cmp_relop(eq)_cmpval(0.0)
define float @select_cc_fcmp_oeq_f32_00(float %a, float %b, float %c) {
  %cmp = fcmp oeq float %c, 0.0
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_oge_f32_00
; CHECK: cmp_relop(ge)_cmpval(0.0)
define float @select_cc_fcmp_oge_f32_00(float %a, float %b, float %c) {
  %cmp = fcmp oge float %c, 0.0
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_olt_f32_00
; CHECK: cmp_relop(lt)_cmpval(0.0)
define float @select_cc_fcmp_olt_f32_00(float %a, float %b, float %c) {
  %cmp = fcmp olt float %c, 0.0
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_oeq_i32_00
; CHECK: cmp_relop(eq)_cmpval(0.0)
define i32 @select_cc_fcmp_oeq_i32_00(i32 %a, i32 %b, float %c) {
  %cmp = fcmp oeq float %c, 0.0
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}

; CHECK-LABEL: @select_cc_fcmp_oge_i32_00
; CHECK: cmp_relop(ge)_cmpval(0.0)
define i32 @select_cc_fcmp_oge_i32_00(i32 %a, i32 %b, float %c) {
  %cmp = fcmp oge float %c, 0.0
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}

; CHECK-LABEL: @select_cc_fcmp_olt_i32_00
; CHECK: cmp_relop(lt)_cmpval(0.0)
define i32 @select_cc_fcmp_olt_i32_00(i32 %a, i32 %b, float %c) {
  %cmp = fcmp olt float %c, 0.0
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}

; CHECK-LABEL: @select_cc_fcmp_oeq_f32_10
; CHECK: cmp_relop(eq)_cmpval(1.0)
define float @select_cc_fcmp_oeq_f32_10(float %a, float %b, float %c) {
  %cmp = fcmp oeq float %c, 1.0
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_oge_f32_10
; CHECK: cmp_relop(ge)_cmpval(1.0)
define float @select_cc_fcmp_oge_f32_10(float %a, float %b, float %c) {
  %cmp = fcmp oge float %c, 1.0
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_olt_f32_10
; CHECK: cmp_relop(lt)_cmpval(1.0)
define float @select_cc_fcmp_olt_f32_10(float %a, float %b, float %c) {
  %cmp = fcmp olt float %c, 1.0
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_oeq_i32_10
; CHECK: cmp_relop(eq)_cmpval(1.0)
define i32 @select_cc_fcmp_oeq_i32_10(i32 %a, i32 %b, float %c) {
  %cmp = fcmp oeq float %c, 1.0
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}

; CHECK-LABEL: @select_cc_fcmp_oge_i32_10
; CHECK: cmp_relop(ge)_cmpval(1.0)
define i32 @select_cc_fcmp_oge_i32_10(i32 %a, i32 %b, float %c) {
  %cmp = fcmp oge float %c, 1.0
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}

; CHECK-LABEL: @select_cc_fcmp_olt_i32_10
; CHECK: cmp_relop(lt)_cmpval(1.0)
define i32 @select_cc_fcmp_olt_i32_10(i32 %a, i32 %b, float %c) {
  %cmp = fcmp olt float %c, 1.0
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}

; CHECK-LABEL: @select_cc_fcmp_oeq_f32_n10
; CHECK: cmp_relop(eq)_cmpval(-1.0)
define float @select_cc_fcmp_oeq_f32_n10(float %a, float %b, float %c) {
  %cmp = fcmp oeq float %c, -1.0
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_oge_f32_n10
; CHECK: cmp_relop(ge)_cmpval(-1.0)
define float @select_cc_fcmp_oge_f32_n10(float %a, float %b, float %c) {
  %cmp = fcmp oge float %c, -1.0
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_olt_f32_n10
; CHECK: cmp_relop(lt)_cmpval(-1.0)
define float @select_cc_fcmp_olt_f32_n10(float %a, float %b, float %c) {
  %cmp = fcmp olt float %c, -1.0
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_oeq_i32_n10
; CHECK: cmp_relop(eq)_cmpval(-1.0)
define i32 @select_cc_fcmp_oeq_i32_n10(i32 %a, i32 %b, float %c) {
  %cmp = fcmp oeq float %c, -1.0
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}

; CHECK-LABEL: @select_cc_fcmp_oge_i32_n10
; CHECK: cmp_relop(ge)_cmpval(-1.0)
define i32 @select_cc_fcmp_oge_i32_n10(i32 %a, i32 %b, float %c) {
  %cmp = fcmp oge float %c, -1.0
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}

; CHECK-LABEL: @select_cc_fcmp_olt_i32_n10
; CHECK: cmp_relop(lt)_cmpval(-1.0)
define i32 @select_cc_fcmp_olt_i32_n10(i32 %a, i32 %b, float %c) {
  %cmp = fcmp olt float %c, -1.0
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}

; CHECK-LABEL: @select_cc_fcmp_oeq_f32_05
; CHECK: cmp_relop(eq)_cmpval(0.5)
define float @select_cc_fcmp_oeq_f32_05(float %a, float %b, float %c) {
  %cmp = fcmp oeq float %c, 0.5
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_oge_f32_05
; CHECK: cmp_relop(ge)_cmpval(0.5)
define float @select_cc_fcmp_oge_f32_05(float %a, float %b, float %c) {
  %cmp = fcmp oge float %c, 0.5
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_olt_f32_05
; CHECK: cmp_relop(lt)_cmpval(0.5)
define float @select_cc_fcmp_olt_f32_05(float %a, float %b, float %c) {
  %cmp = fcmp olt float %c, 0.5
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_oeq_i32_05
; CHECK: cmp_relop(eq)_cmpval(0.5)
define i32 @select_cc_fcmp_oeq_i32_05(i32 %a, i32 %b, float %c) {
  %cmp = fcmp oeq float %c, 0.5
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}

; CHECK-LABEL: @select_cc_fcmp_oge_i32_05
; CHECK: cmp_relop(ge)_cmpval(0.5)
define i32 @select_cc_fcmp_oge_i32_05(i32 %a, i32 %b, float %c) {
  %cmp = fcmp oge float %c, 0.5
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}

; CHECK-LABEL: @select_cc_fcmp_olt_i32_05
; CHECK: cmp_relop(lt)_cmpval(0.5)
define i32 @select_cc_fcmp_olt_i32_05(i32 %a, i32 %b, float %c) {
  %cmp = fcmp olt float %c, 0.5
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}

; CHECK-LABEL: @select_cc_fcmp_oeq_f32_n05
; CHECK: cmp_relop(eq)_cmpval(-0.5)
define float @select_cc_fcmp_oeq_f32_n05(float %a, float %b, float %c) {
  %cmp = fcmp oeq float %c, -0.5
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_oge_f32_n05
; CHECK: cmp_relop(ge)_cmpval(-0.5)
define float @select_cc_fcmp_oge_f32_n05(float %a, float %b, float %c) {
  %cmp = fcmp oge float %c, -0.5
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_olt_f32_n05
; CHECK: cmp_relop(lt)_cmpval(-0.5)
define float @select_cc_fcmp_olt_f32_n05(float %a, float %b, float %c) {
  %cmp = fcmp olt float %c, -0.5
  %select = select i1 %cmp, float %a, float %b
  ret float %select
}

; CHECK-LABEL: @select_cc_fcmp_oeq_i32_n05
; CHECK: cmp_relop(eq)_cmpval(-0.5)
define i32 @select_cc_fcmp_oeq_i32_n05(i32 %a, i32 %b, float %c) {
  %cmp = fcmp oeq float %c, -0.5
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}

; CHECK-LABEL: @select_cc_fcmp_oge_i32_n05
; CHECK: cmp_relop(ge)_cmpval(-0.5)
define i32 @select_cc_fcmp_oge_i32_n05(i32 %a, i32 %b, float %c) {
  %cmp = fcmp oge float %c, -0.5
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}

; CHECK-LABEL: @select_cc_fcmp_olt_i32_n05
; CHECK: cmp_relop(lt)_cmpval(-0.5)
define i32 @select_cc_fcmp_olt_i32_n05(i32 %a, i32 %b, float %c) {
  %cmp = fcmp olt float %c, -0.5
  %select = select i1 %cmp, i32 %a, i32 %b
  ret i32 %select
}
