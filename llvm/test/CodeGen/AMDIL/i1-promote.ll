; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK-LABEL: @test_and_i1
; CHECK: ieq
; CHECK-NEXT: ieq
; CHECK-NEXT: iand
; CHECK-NEXT: uav_raw_store_id
define void @test_and_i1(i32 addrspace(1)* %out, i32 %a, i32 %b) nounwind {
  %cmpa = icmp eq i32 %a, 123
  %cmpb = icmp eq i32 %b, 456
  %and = and i1 %cmpa, %cmpb
  %ext = sext i1 %and to i32
  store i32 %ext, i32 addrspace(1)* %out
  ret void
}

; CHECK-LABEL: @test_or_i1
; CHECK: ieq
; CHECK-NEXT: ieq
; CHECK-NEXT: ior
; CHECK-NEXT: uav_raw_store_id
define void @test_or_i1(i32 addrspace(1)* %out, i32 %a, i32 %b) nounwind {
  %cmpa = icmp eq i32 %a, 123
  %cmpb = icmp eq i32 %b, 456
  %and = or i1 %cmpa, %cmpb
  %ext = sext i1 %and to i32
  store i32 %ext, i32 addrspace(1)* %out
  ret void
}

; CHECK-LABEL: @test_xor_i1
; CHECK: ieq
; CHECK-NEXT: ieq
; CHECK-NEXT: ixor
; CHECK-NEXT: uav_raw_store_id
define void @test_xor_i1(i32 addrspace(1)* %out, i32 %a, i32 %b) nounwind {
  %cmpa = icmp eq i32 %a, 123
  %cmpb = icmp eq i32 %b, 456
  %and = xor i1 %cmpa, %cmpb
  %ext = sext i1 %and to i32
  store i32 %ext, i32 addrspace(1)* %out
  ret void
}

