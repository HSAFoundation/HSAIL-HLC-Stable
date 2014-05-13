; RUN: llc < %s -march=amdil | FileCheck %s

declare void @__amdil_barrier() nounwind
declare void @__amdil_barrier_global() nounwind
declare void @__amdil_barrier_global_local() nounwind
declare void @__amdil_barrier_region() nounwind
declare void @__amdil_barrier_region_local() nounwind
declare void @__amdil_barrier_global_region() nounwind
declare void @__amdil_barrier_local() nounwind

define void @test_barrier() {
; CHECK: ; @test_barrier
; CHECK: fence_threads_memory_gds_lds
; CHECK-NEXT: ret_dyn
  tail call void @__amdil_barrier() nounwind
  ret void
}

define void @test_barrier_global() {
; CHECK: ; @test_barrier_global
; CHECK: fence_threads_memory
; CHECK-NEXT: ret_dyn
  tail call void @__amdil_barrier_global() nounwind
  ret void
}

define void @test_barrier_global_local() {
; CHECK: ; @test_barrier_global_local
; CHECK: fence_threads_memory_lds
; CHECK-NEXT: ret_dyn
  tail call void @__amdil_barrier_global_local() nounwind
  ret void
}

define void @test_barrier_region() {
; CHECK: ; @test_barrier_region
; CHECK: fence_threads_gds
; CHECK-NEXT: ret_dyn
  tail call void @__amdil_barrier_region() nounwind
  ret void
}

define void @test_barrier_region_local() {
; CHECK: ; @test_barrier_region_local
; CHECK: fence_threads_gds_lds
; CHECK-NEXT: ret_dyn
  tail call void @__amdil_barrier_region_local() nounwind
  ret void
}

define void @test_barrier_global_region() {
; CHECK: ; @test_barrier_global_region
; CHECK: fence_threads_memory_gds
; CHECK-NEXT: ret_dyn
  tail call void @__amdil_barrier_global_region() nounwind
  ret void
}

define void @test_barrier_local() {
; CHECK: ; @test_barrier_local
; CHECK: fence_threads_lds
; CHECK-NEXT: ret_dyn
  tail call void @__amdil_barrier_local() nounwind
  ret void
}

