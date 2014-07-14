; XFAIL: *
; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s

; A bit_extract would be redundant with the scalar extended load instructions.
; CHECK-LABEL: @global_sextload_i8_to_i32
; CHECK: uav_byte_load_id
; CHECK-NEXT: uav_raw_store_id
define void @global_sextload_i8_to_i32(i32 addrspace(1)* %out, i8 addrspace(1)* %in) nounwind {
  %val = load i8 addrspace(1)* %in, align 4
  %extval = sext i8 %val to i32
  store i32 %extval, i32 addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: @global_zextload_i8_to_i32
; CHECK: uav_ubyte_load_id
; CHECK-NEXT: uav_raw_store_id
define void @global_zextload_i8_to_i32(i32 addrspace(1)* %out, i8 addrspace(1)* %in) nounwind {
  %val = load i8 addrspace(1)* %in, align 4
  %extval = zext i8 %val to i32
  store i32 %extval, i32 addrspace(1)* %out, align 4
  ret void
}

; CHECK-LABEL: @global_zextload_i8_to_i64
; CHECK: dcl_literal [[LIT31:l[0-9]+]], 0x00000018,
; CHECK: uav_ubyte_load_id
; CHECK-NOT: bit_extract
; CHECK-NOT: ishr
; CHECK-NOT: ushr
; CHECK: uav_raw_store_id
define void @global_zextload_i8_to_i64(i64 addrspace(1)* %out, i8 addrspace(1)* %in) nounwind {
  %val = load i8 addrspace(1)* %in, align 4
  %extval = zext i8 %val to i64
  store i64 %extval, i64 addrspace(1)* %out, align 8
  ret void
}

; CHECK-LABEL: @global_sextload_i8_to_i64
; CHECK: uav_byte_load_id
; CHECK-NOT: bit_extract
; CHECK: uav_raw_store_id
define void @global_sextload_i8_to_i64(i64 addrspace(1)* %out, i8 addrspace(1)* %in) nounwind {
  %val = load i8 addrspace(1)* %in, align 4
  %extval = sext i8 %val to i64
  store i64 %extval, i64 addrspace(1)* %out, align 8
  ret void
}

; CHECK-LABEL: @global_sextload_v2i8_to_v2i32
; CHECK-DAG: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK-DAG: dcl_literal [[LIT8:l[0-9]+]], 0x00000008,
; CHECK: uav_ushort_load_id({{[0-9]+}}) [[LOADREG:r[0-9]+]].x___,
; CHECK: ibit_extract {{r[0-9]+}}.xy__, [[LIT8]], [[LIT0]], [[LOADREG]].x
; CHECK: uav_raw_store_id
define void @global_sextload_v2i8_to_v2i32(<2 x i32> addrspace(1)* %out, <2 x i8> addrspace(1)* %in) nounwind {
  %val = load <2 x i8> addrspace(1)* %in, align 2
  %extval = sext <2 x i8> %val to <2 x i32>
  store <2 x i32> %extval, <2 x i32> addrspace(1)* %out, align 8
  ret void
}

; CHECK-LABEL: @global_sextload_v2i8_to_v2i64
; CHECK-DAG: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK-DAG: dcl_literal [[LIT8:l[0-9]+]], 0x00000008,
; CHECK: uav_ushort_load_id({{[0-9]+}}) [[REG:r[0-9]+]],
; CHECK-NOT: ubit_extract
; CHECK: ibit_extract {{r[0-9]+}}, [[LIT8]], [[LIT8]], [[REG]]
; CHECK: ibit_extract {{r[0-9]+}}, [[LIT8]], [[LIT0]], [[REG]]
; CHECK: uav_raw_store_id
define void @global_sextload_v2i8_to_v2i64(<2 x i64> addrspace(1)* %out, <2 x i8> addrspace(1)* %in) nounwind {
  %val = load <2 x i8> addrspace(1)* %in, align 2
  %extval = sext <2 x i8> %val to <2 x i64>
  store <2 x i64> %extval, <2 x i64> addrspace(1)* %out, align 16
  ret void
}

; CHECK-LABEL: @global_zextload_v2i8_to_v2i32
; CHECK-DAG: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK-DAG: dcl_literal [[LIT8:l[0-9]+]], 0x00000008,
; CHECK: uav_ushort_load_id({{[0-9]+}}) [[REG:r[0-9]+]],
; CHECK: ubit_extract {{r[0-9]+}}, [[LIT8]], [[LIT8]], [[REG]]
; CHECK: ubit_extract {{r[0-9]+}}, [[LIT8]], [[LIT0]], [[REG]]
; CHECK: uav_raw_store_id
define void @global_zextload_v2i8_to_v2i32(<2 x i32> addrspace(1)* %out, <2 x i8> addrspace(1)* %in) nounwind {
  %val = load <2 x i8> addrspace(1)* %in, align 2
  %extval = zext <2 x i8> %val to <2 x i32>
  store <2 x i32> %extval, <2 x i32> addrspace(1)* %out, align 8
  ret void
}

; CHECK-LABEL: @global_sextload_v4i8_to_v4i32
; CHECK-DAG: dcl_literal [[LIT8:l[0-9]+]], 0x00000008,
; CHECK-DAG: dcl_literal [[LIT24:l[0-9]+]], 0x00000018,
; CHECK-DAG: dcl_literal [[LIT16:l[0-9]+]], 0x00000010,
; CHECK-DAG: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK: uav_raw_load_id({{[0-9]+}}) [[REG:r[0-9]+]],
; CHECK: ibit_extract {{r[0-9]+}}, [[LIT8]], [[LIT24]], [[REG]]
; CHECK: ibit_extract {{r[0-9]+}}, [[LIT8]], [[LIT16]], [[REG]]
; CHECK: ibit_extract {{r[0-9]+}}, [[LIT8]], [[LIT8]], [[REG]]
; CHECK: ibit_extract {{r[0-9]+}}, [[LIT8]], [[LIT0]], [[REG]]
; CHECK-NOT: ishr
; CHECK-NOT: ushr
; CHECK: uav_raw_store_id
; CHECK-NEXT: ret_dyn
define void @global_sextload_v4i8_to_v4i32(<4 x i32> addrspace(1)* %out, <4 x i8> addrspace(1)* %in) nounwind {
  %val = load <4 x i8> addrspace(1)* %in, align 4
  %extval = sext <4 x i8> %val to <4 x i32>
  store <4 x i32> %extval, <4 x i32> addrspace(1)* %out, align 16
  ret void
}

; CHECK-LABEL: @global_zextload_v4i8_to_v4i32
; CHECK: uav_raw_load_id({{[0-9]+}}) [[REG:r[0-9]+]],
; CHECK-DAG: dcl_literal [[LIT8:l[0-9]+]], 0x00000008,
; CHECK-DAG: dcl_literal [[LIT24:l[0-9]+]], 0x00000018,
; CHECK-DAG: dcl_literal [[LIT16:l[0-9]+]], 0x00000010,
; CHECK-DAG: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK: ubit_extract {{r[0-9]+}}, [[LIT8]], [[LIT24]], [[REG]]
; CHECK: ubit_extract {{r[0-9]+}}, [[LIT8]], [[LIT16]], [[REG]]
; CHECK: ubit_extract {{r[0-9]+}}, [[LIT8]], [[LIT8]], [[REG]]
; CHECK: ubit_extract {{r[0-9]+}}, [[LIT8]], [[LIT0]], [[REG]]
; CHECK-NOT: iand
; CHECK-NOT: ishr
; CHECK-NOT: ushr
; CHECK: uav_raw_store_id
; CHECK-NEXT: ret_dyn
define void @global_zextload_v4i8_to_v4i32(<4 x i32> addrspace(1)* %out, <4 x i8> addrspace(1)* %in) nounwind {
  %val = load <4 x i8> addrspace(1)* %in, align 4
  %extval = zext <4 x i8> %val to <4 x i32>
  store <4 x i32> %extval, <4 x i32> addrspace(1)* %out, align 16
  ret void
}

; CHECK-LABEL: @global_zextload_v4i8_to_v4i64
; CHECK: uav_raw_load_id({{[0-9]+}}) [[REG:r[0-9]+]],
; CHECK-DAG: dcl_literal [[LIT8:l[0-9]+]], 0x00000008,
; CHECK-DAG: dcl_literal [[LIT24:l[0-9]+]], 0x00000018,
; CHECK-DAG: dcl_literal [[LIT16:l[0-9]+]], 0x00000010,
; CHECK-DAG: dcl_literal [[LIT0:l[0-9]+]], 0x00000000,
; CHECK: ubit_extract {{r[0-9]+}}, [[LIT8]], [[LIT24]], [[REG]]
; CHECK: ubit_extract {{r[0-9]+}}, [[LIT8]], [[LIT16]], [[REG]]
; CHECK: ubit_extract {{r[0-9]+}}, [[LIT8]], [[LIT8]], [[REG]]
; CHECK: ubit_extract {{r[0-9]+}}, [[LIT8]], [[LIT0]], [[REG]]
; CHECK-NOT: iand
; CHECK-NOT: ishr
; CHECK-NOT: ushr
; CHECK: uav_raw_store_id
; CHECK: uav_raw_store_id
; CHECK: uav_raw_store_id
; CHECK: uav_raw_store_id
; CHECK-NEXT: ret_dyn
define void @global_zextload_v4i8_to_v4i64(<4 x i64> addrspace(1)* %out, <4 x i8> addrspace(1)* %in) nounwind {
  %val = load <4 x i8> addrspace(1)* %in, align 4
  %extval = zext <4 x i8> %val to <4 x i64>
  store <4 x i64> %extval, <4 x i64> addrspace(1)* %out, align 32
  ret void
}

; Do not combine for a volatile load.
; CHECK-LABEL: @volatile_global_zextload_v2i8_to_v2i32
; CHECK: uav_ubyte_load_id
; CHECK-NOT: bit_extract
; CHECK-NOT: iand
; CHECK: uav_ubyte_load_id
; CHECK-NOT: bit_extract
; CHECK-NOT: iand
; CHECK: uav_raw_store_id
define void @volatile_global_zextload_v2i8_to_v2i32(<2 x i32> addrspace(1)* %out, <2 x i8> addrspace(1)* %in) nounwind {
  %val = load volatile <2 x i8> addrspace(1)* %in, align 2
  %extval = zext <2 x i8> %val to <2 x i32>
  store <2 x i32> %extval, <2 x i32> addrspace(1)* %out, align 8
  ret void
}

; CHECK-LABEL: @volatile_global_zextload_v2i8_to_v2i64
; CHECK: uav_ubyte_load_id
; CHECK-NOT: bit_extract
; CHECK-NOT: iand
; CHECK: uav_ubyte_load_id
; CHECK-NOT: bit_extract
; CHECK-NOT: iand
; CHECK: uav_raw_store_id
define void @volatile_global_zextload_v2i8_to_v2i64(<2 x i64> addrspace(1)* %out, <2 x i8> addrspace(1)* %in) nounwind {
  %val = load volatile <2 x i8> addrspace(1)* %in, align 2
  %extval = zext <2 x i8> %val to <2 x i64>
  store <2 x i64> %extval, <2 x i64> addrspace(1)* %out, align 16
  ret void
}

; Do not combine for a volatile load.
; CHECK-LABEL: @volatile_global_sextload_v4i8_to_v4i32
; CHECK: uav_byte_load_id
; CHECK-NOT: bit_extract
; CHECK: uav_byte_load_id
; CHECK-NOT: bit_extract
; CHECK: uav_byte_load_id
; CHECK-NOT: bit_extract
; CHECK: uav_byte_load_id
; CHECK-NOT: bit_extract
; CHECK: uav_raw_store_id
define void @volatile_global_sextload_v4i8_to_v4i32(<4 x i32> addrspace(1)* %out, <4 x i8> addrspace(1)* %in) nounwind {
  %val = load volatile <4 x i8> addrspace(1)* %in, align 4
  %extval = sext <4 x i8> %val to <4 x i32>
  store <4 x i32> %extval, <4 x i32> addrspace(1)* %out, align 8
  ret void
}

; Do not combine for a volatile load.
; CHECK-LABEL: @volatile_global_sextload_v4i8_to_v4i64
; CHECK: uav_byte_load_id
; CHECK-NOT: bit_extract
; CHECK: uav_byte_load_id
; CHECK-NOT: bit_extract
; CHECK: uav_raw_store_id
; CHECK: uav_byte_load_id
; CHECK-NOT: bit_extract
; CHECK: uav_byte_load_id
; CHECK-NOT: bit_extract
; CHECK: uav_raw_store_id
define void @volatile_global_sextload_v4i8_to_v4i64(<4 x i64> addrspace(1)* %out, <4 x i8> addrspace(1)* %in) nounwind {
  %val = load volatile <4 x i8> addrspace(1)* %in, align 4
  %extval = sext <4 x i8> %val to <4 x i64>
  store <4 x i64> %extval, <4 x i64> addrspace(1)* %out, align 8
  ret void
}
