; RUN: llc -asm-verbose=0 -march=amdil < %s | FileCheck %s

; CHECK-LABEL: ubit_opt_case_0
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_0(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 16
  %tmp7 = and i32 %tmp6, 16711680
  %tmp9 = extractelement <4 x i32> %tmp2, i32 0
  %tmp10 = shl i32 %tmp9, 8
  %tmp11 = and i32 %tmp10, 65280
  %tmp12 = or i32 %tmp7, %tmp11
  %arrayidx15 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp12, i32 addrspace(1)* %arrayidx15, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_1
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_1(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = and i32 %tmp4, 16711680
  %tmp8 = extractelement <4 x i32> %tmp2, i32 0
  %tmp9 = shl i32 %tmp8, 8
  %tmp10 = and i32 %tmp9, 65280
  %tmp11 = or i32 %tmp10, %tmp6
  %arrayidx14 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp11, i32 addrspace(1)* %arrayidx14, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_2
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_2(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 17
  %tmp7 = and i32 %tmp6, 33423360
  %tmp9 = extractelement <4 x i32> %tmp2, i32 0
  %tmp10 = and i32 %tmp9, 65280
  %tmp11 = or i32 %tmp7, %tmp10
  %arrayidx14 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp11, i32 addrspace(1)* %arrayidx14, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_3
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_3(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 17
  %tmp7 = and i32 %tmp6, 33423360
  %tmp9 = extractelement <4 x i32> %tmp2, i32 0
  %tmp10 = shl i32 %tmp9, 25
  %tmp11 = or i32 %tmp7, %tmp10
  %arrayidx14 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp11, i32 addrspace(1)* %arrayidx14, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_4
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_4(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 17
  %tmp8 = extractelement <4 x i32> %tmp2, i32 0
  %tmp9 = shl i32 %tmp8, 5
  %tmp10 = and i32 %tmp9, 130560
  %tmp11 = or i32 %tmp10, %tmp6
  %arrayidx14 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp11, i32 addrspace(1)* %arrayidx14, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_5
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_5(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = and i32 %tmp4, 65280
  %tmp8 = extractelement <4 x i32> %tmp2, i32 0
  %tmp9 = and i32 %tmp8, 255
  %tmp10 = or i32 %tmp6, %tmp9
  %arrayidx13 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp10, i32 addrspace(1)* %arrayidx13, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_6
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_6(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = and i32 %tmp4, 65280
  %tmp8 = extractelement <4 x i32> %tmp2, i32 0
  %tmp9 = shl i32 %tmp8, 16
  %tmp10 = or i32 %tmp6, %tmp9
  %arrayidx13 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp10, i32 addrspace(1)* %arrayidx13, align 4
  ret void
}

; CHECK: ubit_opt_case_7
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_7(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 9
  %tmp8 = extractelement <4 x i32> %tmp2, i32 0
  %tmp9 = and i32 %tmp8, 255
  %tmp10 = or i32 %tmp6, %tmp9
  %arrayidx13 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp10, i32 addrspace(1)* %arrayidx13, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_8
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_8(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 8
  %tmp7 = and i32 %tmp6, 65280
  %tmp9 = extractelement <4 x i32> %tmp2, i32 0
  %tmp10 = and i32 %tmp9, -65281
  %tmp11 = or i32 %tmp7, %tmp10
  %arrayidx14 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp11, i32 addrspace(1)* %arrayidx14, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_10
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_10(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 10
  %tmp8 = extractelement <4 x i32> %tmp2, i32 0
  %tmp9 = and i32 %tmp8, 63
  %tmp10 = or i32 %tmp6, %tmp9
  %arrayidx13 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp10, i32 addrspace(1)* %arrayidx13, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_11
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_11(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 4
  %tmp7 = and i32 %tmp6, 14336
  %tmp9 = extractelement <4 x i32> %tmp2, i32 0
  %tmp10 = and i32 %tmp9, 7
  %tmp11 = or i32 %tmp7, %tmp10
  %arrayidx14 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp11, i32 addrspace(1)* %arrayidx14, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_12
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_12(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 7
  %tmp7 = and i32 %tmp6, 896
  %tmp9 = extractelement <4 x i32> %tmp2, i32 0
  %tmp10 = and i32 %tmp9, 56
  %tmp11 = or i32 %tmp7, %tmp10
  %arrayidx14 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp11, i32 addrspace(1)* %arrayidx14, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_13
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_13(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 1
  %tmp7 = and i32 %tmp6, 126
  %tmp9 = extractelement <4 x i32> %tmp2, i32 0
  %tmp10 = shl i32 %tmp9, 10
  %tmp11 = or i32 %tmp7, %tmp10
  %arrayidx14 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp11, i32 addrspace(1)* %arrayidx14, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_14
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_14(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %arrayidx5 = getelementptr i32 addrspace(1)* %a, i32 1
  %tmp7 = extractelement <4 x i32> %tmp2, i32 0
  %tmp9 = extractelement <4 x i32> %tmp2, i32 3
  %tmp10 = and i32 %tmp7, %tmp9
  %tmp12 = extractelement <4 x i32> %tmp2, i32 1
  %tmp15 = xor i32 %tmp9, -1
  %tmp16 = and i32 %tmp12, %tmp15
  %tmp17 = or i32 %tmp10, %tmp16
  store i32 %tmp17, i32 addrspace(1)* %arrayidx5, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_15
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_15(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %arrayidx5 = getelementptr i32 addrspace(1)* %a, i32 1
  %tmp7 = extractelement <4 x i32> %tmp2, i32 0
  %tmp9 = extractelement <4 x i32> %tmp2, i32 3
  %tmp10 = and i32 %tmp7, %tmp9
  %tmp12 = extractelement <4 x i32> %tmp2, i32 1
  %tmp15 = xor i32 %tmp9, -1
  %tmp16 = and i32 %tmp12, %tmp15
  %tmp17 = or i32 %tmp10, %tmp16
  store i32 %tmp17, i32 addrspace(1)* %arrayidx5, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_16
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_16(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %arrayidx5 = getelementptr i32 addrspace(1)* %a, i32 1
  %tmp7 = extractelement <4 x i32> %tmp2, i32 3
  %tmp9 = extractelement <4 x i32> %tmp2, i32 0
  %tmp11 = extractelement <4 x i32> %tmp2, i32 1
  %tmp14 = xor i32 %tmp11, %tmp7
  %tmp15 = and i32 %tmp14, %tmp9
  %tmp16 = xor i32 %tmp15, %tmp7
  store i32 %tmp16, i32 addrspace(1)* %arrayidx5, align 4
  ret void
}

; CHECK-LABEL: ubit_opt_case_17
; CHECK: bfi
; CHECK: ret_dyn
define void @ubit_opt_case_17(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %arrayidx5 = getelementptr i32 addrspace(1)* %a, i32 1
  %tmp7 = extractelement <4 x i32> %tmp2, i32 0
  %tmp11 = extractelement <4 x i32> %tmp2, i32 1
  %tmp12 = xor i32 %tmp7, %tmp11
  %tmp14 = extractelement <4 x i32> %tmp2, i32 2
  %tmp15 = and i32 %tmp12, %tmp14
  %tmp16 = xor i32 %tmp15, %tmp7
  store i32 %tmp16, i32 addrspace(1)* %arrayidx5, align 4
  ret void
}

; CHECK-LABEL: ubit_noopt_case_0
; CHECK-NOT: bfi
; CHECK: ret_dyn
define void @ubit_noopt_case_0(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 16
  %tmp7 = and i32 %tmp6, 16711680
  %tmp9 = extractelement <4 x i32> %tmp2, i32 0
  %tmp10 = shl i32 %tmp9, 9
  %tmp11 = and i32 %tmp10, 130560
  %tmp12 = or i32 %tmp7, %tmp11
  %arrayidx15 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp12, i32 addrspace(1)* %arrayidx15, align 4
  ret void
}

; CHECK-LABEL: ubit_noopt_case_1
; CHECK-NOT: bfi
; CHECK: ret_dyn
define void @ubit_noopt_case_1(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = and i32 %tmp4, 16744448
  %tmp8 = extractelement <4 x i32> %tmp2, i32 0
  %tmp9 = shl i32 %tmp8, 8
  %tmp10 = and i32 %tmp9, 65280
  %tmp11 = or i32 %tmp10, %tmp6
  %arrayidx14 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp11, i32 addrspace(1)* %arrayidx14, align 4
  ret void
}

; CHECK-LABEL: ubit_noopt_case_2
; CHECK-NOT: bfi
; CHECK: ret_dyn
define void @ubit_noopt_case_2(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 15
  %tmp7 = and i32 %tmp6, 8355840
  %tmp9 = extractelement <4 x i32> %tmp2, i32 0
  %tmp10 = and i32 %tmp9, 65280
  %tmp11 = or i32 %tmp7, %tmp10
  %arrayidx14 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp11, i32 addrspace(1)* %arrayidx14, align 4
  ret void
}

; CHECK: ubit_noopt_case_3
; CHECK-NOT: bfi
; CHECK: ret_dyn
define void @ubit_noopt_case_3(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 17
  %tmp7 = and i32 %tmp6, 33423360
  %tmp9 = extractelement <4 x i32> %tmp2, i32 0
  %tmp10 = shl i32 %tmp9, 22
  %tmp11 = or i32 %tmp7, %tmp10
  %arrayidx14 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp11, i32 addrspace(1)* %arrayidx14, align 4
  ret void
}

; CHECK-LABEL: ubit_noopt_case_4
; CHECK-NOT: bfi
; CHECK: ret_dyn
define void @ubit_noopt_case_4(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 15
  %tmp8 = extractelement <4 x i32> %tmp2, i32 0
  %tmp9 = shl i32 %tmp8, 5
  %tmp10 = and i32 %tmp9, 130560
  %tmp11 = or i32 %tmp10, %tmp6
  %arrayidx14 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp11, i32 addrspace(1)* %arrayidx14, align 4
  ret void
}

; CHECK-LABEL: ubit_noopt_case_6
; CHECK-NOT: bfi
; CHECK: ret_dyn
define void @ubit_noopt_case_6(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = and i32 %tmp4, 65280
  %tmp8 = extractelement <4 x i32> %tmp2, i32 0
  %tmp9 = shl i32 %tmp8, 12
  %tmp10 = or i32 %tmp6, %tmp9
  %arrayidx13 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp10, i32 addrspace(1)* %arrayidx13, align 4
  ret void
}

; CHECK-LABEL: ubit_noopt_case_8
; CHECK-NOT: bfi
; CHECK: ret_dyn
define void @ubit_noopt_case_8(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %tmp4 = extractelement <4 x i32> %tmp2, i32 1
  %tmp6 = shl i32 %tmp4, 9
  %tmp7 = and i32 %tmp6, 22369280
  %tmp9 = extractelement <4 x i32> %tmp2, i32 0
  %tmp10 = shl i32 %tmp9, 9
  %tmp11 = and i32 %tmp10, 6710784
  %tmp12 = or i32 %tmp7, %tmp11
  %arrayidx15 = getelementptr i32 addrspace(1)* %a, i32 1
  store i32 %tmp12, i32 addrspace(1)* %arrayidx15, align 4
  ret void
}

; CHECK-LABEL: ubit_two_and_const_mask
; CHECK: dcl_literal [[MASKLITERAL_A:l[0-9]+]], 0x00000F0F
; CHECK: dcl_literal [[MASKLITERAL_B:l[0-9]+]], 0x0000F0F0
; CHECK: iand [[SRC2:r[0-9]+]].x___, r1.y, [[MASKLITERAL_A]]
; CHECK: bfi r1.x___, [[MASKLITERAL_B]], r1.x, [[SRC2]].x
; CHECK: ret_dyn
define i32 @ubit_two_and_const_mask(i32 %a, i32 %b) nounwind {
  %mask_a = and i32 %a, 61680 ; 0xF0F0
  %nmask_b = and i32 %b, 3855 ; 0x0F0F
  %result = or i32 %mask_a, %nmask_b
  ret i32 %result
}

; CHECK-LABEL: ubit_two_and_const_mask_inv
; CHECK: dcl_literal [[MASKLITERAL_A:l[0-9]+]], 0x00000F0F
; CHECK: dcl_literal [[MASKLITERAL_B:l[0-9]+]], 0x0000F0F0
; CHECK: iand [[SRC2:r[0-9]+]].x___, r1.x, [[MASKLITERAL_B]]
; CHECK: bfi r1.x___, [[MASKLITERAL_A]], r1.y, [[SRC2]].x
; CHECK: ret_dyn
define i32 @ubit_two_and_const_mask_inv(i32 %a, i32 %b) nounwind {
  %mask_a = and i32 %a, 61680 ; 0xF0F0
  %nmask_b = and i32 %b, 3855 ; 0x0F0F
  %result = or i32 %nmask_b, %mask_a
  ret i32 %result
}

; CHECK-LABEL @inv_const_mask
; CHECK: func
; CHECK: dcl_literal [[LIT:l[0-9]+]], 0x0000DEAD
; CHECK-NEXT: bfi r1.x___, [[LIT]], r1.x, r1.y
; CHECK-NEXT: ret_dyn
define i32 @inv_const_mask(i32 %a, i32 %b) nounwind {
  %mask_a = and i32 %a, 57005 ; 0xdead
  %nmask_b = and i32 %b, 4294910290 ; 0xffff2152 (~0xdead)
  %result = or i32 %mask_a, %nmask_b
  ret i32 %result
}

; CHECK-LABEL: @ubit_opt_shl_and
; CHECK: dcl_literal [[SHIFTLITERAL:l[0-9]+]], 0x00000009,
; CHECK: dcl_literal [[MASKLITERAL:l[0-9]+]], 0x000000FF,
; CHECK: ishl [[SHL:r[0-9]+]].x___, r1.x, [[SHIFTLITERAL]]
; CHECK: bfi r1.x___, [[MASKLITERAL]], r1.y, [[SHL]].x
define i32 @ubit_opt_shl_and(i32 %a, i32 %b) nounwind {
  %shla = shl i32 %a, 9
  %andb = and i32 %b, 255
  %result = or i32 %shla, %andb
  ret i32 %result
}

; CHECK-LABEL: @ubit_opt_shl_and_inv
; CHECK: dcl_literal [[SHIFTLITERAL:l[0-9]+]], 0x00000009,
; CHECK: dcl_literal [[MASKLITERAL:l[0-9]+]], 0x000000FF,
; CHECK: ishl [[SHL:r[0-9]+]].x___, r1.x, [[SHIFTLITERAL]]
; CHECK: bfi r1.x___, [[MASKLITERAL]], r1.y, [[SHL]]
define i32 @ubit_opt_shl_and_inv(i32 %a, i32 %b) nounwind {
  %andb = and i32 %b, 255
  %shla = shl i32 %a, 9
  %result = or i32 %andb, %shla
  ret i32 %result
}

; CHECK-LABEL: bfm_opt_case_0
; CHECK: func {{.*}}
; CHECK: bfm
; CHECK: ret_dyn
define void @bfm_opt_case_0(i32 addrspace(1)* %a, <4 x i32> addrspace(1)* %b) nounwind {
  %arrayidx = getelementptr <4 x i32> addrspace(1)* %b, i32 1
  %tmp2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %arrayidx5 = getelementptr i32 addrspace(1)* %a, i32 1
  %tmp7 = extractelement <4 x i32> %tmp2, i32 0
  %tmp9 = and i32 %tmp7, 31
  %tmp10 = shl i32 1, %tmp9
  %tmp11 = add nsw i32 %tmp10, -1
  %tmp13 = extractelement <4 x i32> %tmp2, i32 1
  %tmp15 = and i32 %tmp13, 31
  %tmp16 = shl i32 %tmp11, %tmp15
  store i32 %tmp16, i32 addrspace(1)* %arrayidx5, align 4
  ret void
}


