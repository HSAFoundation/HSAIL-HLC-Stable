; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @test_copy_v4i8
; CHECK: uav_raw_load_id({{[0-9]+}}) [[REG:r[0-9]+]].x___,
; CHECK-NEXT: uav_raw_store_id({{[0-9]+}}) mem{{[0-9]\.[xyzw]___}}, r{{[0-9]+\.[xyzw]}}, [[REG]].x
; CHECK: ret_dyn
define void @test_copy_v4i8(<4 x i8> addrspace(1)* %out, <4 x i8> addrspace(1)* %in) nounwind {
  %val = load <4 x i8> addrspace(1)* %in, align 4
  store <4 x i8> %val, <4 x i8> addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: @test_copy_v4i8_x2
; CHECK: uav_raw_load_id({{[0-9]+}}) [[REG:r[0-9]+]].x___,
; CHECK-NEXT: uav_raw_store_id({{[0-9]+}}) mem{{[0-9]\.[xyzw]___}}, r{{[0-9]+\.[xyzw]}}, [[REG]].x
; CHECK-NEXT: uav_raw_store_id({{[0-9]+}}) mem{{[0-9]\.[xyzw]___}}, r{{[0-9]+\.[xyzw]}}, [[REG]].x
; CHECK: ret_dyn
define void @test_copy_v4i8_x2(<4 x i8> addrspace(1)* %out0, <4 x i8> addrspace(1)* %out1, <4 x i8> addrspace(1)* %in) nounwind {
  %val = load <4 x i8> addrspace(1)* %in, align 4
  store <4 x i8> %val, <4 x i8> addrspace(1)* %out0, align 4
  store <4 x i8> %val, <4 x i8> addrspace(1)* %out1, align 4
  ret void
}

; CHECK-LABEL: @test_copy_v4i8_x3
; CHECK: uav_raw_load_id({{[0-9]+}}) [[REG:r[0-9]+]].x___,
; CHECK-NEXT: uav_raw_store_id({{[0-9]+}}) mem{{[0-9]\.[xyzw]___}}, r{{[0-9]+\.[xyzw]}}, [[REG]].x
; CHECK-NEXT: uav_raw_store_id({{[0-9]+}}) mem{{[0-9]\.[xyzw]___}}, r{{[0-9]+\.[xyzw]}}, [[REG]].x
; CHECK-NEXT: uav_raw_store_id({{[0-9]+}}) mem{{[0-9]\.[xyzw]___}}, r{{[0-9]+\.[xyzw]}}, [[REG]].x
; CHECK: ret_dyn
define void @test_copy_v4i8_x3(<4 x i8> addrspace(1)* %out0, <4 x i8> addrspace(1)* %out1, <4 x i8> addrspace(1)* %out2, <4 x i8> addrspace(1)* %in) nounwind {
  %val = load <4 x i8> addrspace(1)* %in, align 4
  store <4 x i8> %val, <4 x i8> addrspace(1)* %out0, align 4
  store <4 x i8> %val, <4 x i8> addrspace(1)* %out1, align 4
  store <4 x i8> %val, <4 x i8> addrspace(1)* %out2, align 4
  ret void
}

; CHECK-LABEL: @test_copy_v4i8_x4
; CHECK: uav_raw_load_id({{[0-9]+}}) [[REG:r[0-9]+]].x___,
; CHECK-NEXT: uav_raw_store_id({{[0-9]+}}) mem{{[0-9]\.[xyzw]___}}, r{{[0-9]+\.[xyzw]}}, [[REG]].x
; CHECK-NEXT: uav_raw_store_id({{[0-9]+}}) mem{{[0-9]\.[xyzw]___}}, r{{[0-9]+\.[xyzw]}}, [[REG]].x
; CHECK-NEXT: uav_raw_store_id({{[0-9]+}}) mem{{[0-9]\.[xyzw]___}}, r{{[0-9]+\.[xyzw]}}, [[REG]].x
; CHECK: ret_dyn
define void @test_copy_v4i8_x4(<4 x i8> addrspace(1)* %out0, <4 x i8> addrspace(1)* %out1, <4 x i8> addrspace(1)* %out2, <4 x i8> addrspace(1)* %out3, <4 x i8> addrspace(1)* %in) nounwind {
  %val = load <4 x i8> addrspace(1)* %in, align 4
  store <4 x i8> %val, <4 x i8> addrspace(1)* %out0, align 4
  store <4 x i8> %val, <4 x i8> addrspace(1)* %out1, align 4
  store <4 x i8> %val, <4 x i8> addrspace(1)* %out2, align 4
  store <4 x i8> %val, <4 x i8> addrspace(1)* %out3, align 4
  ret void
}

; CHECK-LABEL: @test_copy_v4i8_extra_use
; CHECK: uav_raw_load_id
; CHECK: bit_extract
; CHECK: iadd
; CHECK: uav_raw_store_id
; CHECK: ret_dyn
define void @test_copy_v4i8_extra_use(<4 x i8> addrspace(1)* %out0, <4 x i8> addrspace(1)* %out1, <4 x i8> addrspace(1)* %in) nounwind {
  %val = load <4 x i8> addrspace(1)* %in, align 4
  %add = add <4 x i8> %val, <i8 9, i8 9, i8 9, i8 9>
  store <4 x i8> %val, <4 x i8> addrspace(1)* %out0, align 4
  store <4 x i8> %add, <4 x i8> addrspace(1)* %out1, align 4
  ret void
}

; CHECK-LABEL: @test_copy_v4i8_x2_extra_use
; CHECK: uav_raw_load_id
; CHECK: bit_extract
; CHECK: uav_raw_store_id
; CHECK: iadd
; CHECK: uav_raw_store_id
; CHECK-NEXT: uav_raw_store_id
; CHECK: ret_dyn
define void @test_copy_v4i8_x2_extra_use(<4 x i8> addrspace(1)* %out0, <4 x i8> addrspace(1)* %out1, <4 x i8> addrspace(1)* %out2, <4 x i8> addrspace(1)* %in) nounwind {
  %val = load <4 x i8> addrspace(1)* %in, align 4
  %add = add <4 x i8> %val, <i8 9, i8 9, i8 9, i8 9>
  store <4 x i8> %val, <4 x i8> addrspace(1)* %out0, align 4
  store <4 x i8> %add, <4 x i8> addrspace(1)* %out1, align 4
  store <4 x i8> %val, <4 x i8> addrspace(1)* %out2, align 4
  ret void
}

; CHECK-LABEL: @test_copy_v3i8
; CHECK-NOT: bit_insert
; CHECK-NOT: bit_extract
; CHECK: ret_dyn
define void @test_copy_v3i8(<3 x i8> addrspace(1)* %out, <3 x i8> addrspace(1)* %in) nounwind {
  %val = load <3 x i8> addrspace(1)* %in, align 4
  store <3 x i8> %val, <3 x i8> addrspace(1)* %out, align 4
  ret void
}
