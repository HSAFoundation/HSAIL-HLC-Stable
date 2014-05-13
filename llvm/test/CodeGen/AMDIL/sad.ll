; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck -check-prefix=SI %s


declare i32 @__amdil_sad_i32(i32, i32, i32) nounwind readonly
declare <2 x i32> @__amdil_sad_v2i32(<2 x i32>, <2 x i32>, <2 x i32>) nounwind readonly
declare <4 x i32> @__amdil_sad_v4i32(<4 x i32>, <4 x i32>, <4 x i32>) nounwind readonly

; Test that different combinations of register and immediate operands
; work with a target intrinsic.
define void @test_sad_combinations(i32 addrspace(1)* nocapture %out, i32 %x) nounwind {
; SI-LABEL: @test_sad_combinations
; SI: sad r{{.*}}, l{{[0-9]+}}, l{{[0-9]+}}, l{{[0-9]+}}
; SI: sad r{{.*}}, r{{.*}}, l{{[0-9]+}}, l{{[0-9]+}}
; SI: sad r{{.*}}, l{{[0-9]+}}, r{{.*}}, l{{[0-9]+}}
; SI: sad r{{.*}}, l{{[0-9]+}}, l{{[0-9]+}}, r{{.*}}
; SI: sad r{{.*}}, r{{.*}}, r{{.*}}, l{{[0-9]+}}
; SI: sad r{{.*}}, r{{.*}}, r{{.*}}, r{{.*}}
; SI: sad r{{.*}}, l{{[0-9]+}}, r{{.*}}, r{{.*}}
; SI: sad r{{.*}}, r{{.*}}, l{{[0-9]+}}, r{{.*}}
; SI: ret_dyn
entry:
  %call = tail call i32 @__amdil_sad_i32(i32 0, i32 0, i32 0) nounwind
  store i32 %call, i32 addrspace(1)* %out, align 4
  %arrayidx2 = getelementptr i32 addrspace(1)* %out, i32 1
  %call4 = tail call i32 @__amdil_sad_i32(i32 %x, i32 0, i32 0) nounwind
  store i32 %call4, i32 addrspace(1)* %arrayidx2, align 4
  %arrayidx6 = getelementptr i32 addrspace(1)* %out, i32 2
  %call8 = tail call i32 @__amdil_sad_i32(i32 0, i32 %x, i32 0) nounwind
  store i32 %call8, i32 addrspace(1)* %arrayidx6, align 4
  %arrayidx10 = getelementptr i32 addrspace(1)* %out, i32 3
  %call12 = tail call i32 @__amdil_sad_i32(i32 0, i32 0, i32 %x) nounwind
  store i32 %call12, i32 addrspace(1)* %arrayidx10, align 4
  %arrayidx14 = getelementptr i32 addrspace(1)* %out, i32 4
  %call17 = tail call i32 @__amdil_sad_i32(i32 %x, i32 %x, i32 0) nounwind
  store i32 %call17, i32 addrspace(1)* %arrayidx14, align 4
  %arrayidx19 = getelementptr i32 addrspace(1)* %out, i32 5
  %call23 = tail call i32 @__amdil_sad_i32(i32 %x, i32 %x, i32 %x) nounwind
  store i32 %call23, i32 addrspace(1)* %arrayidx19, align 4
  %arrayidx25 = getelementptr i32 addrspace(1)* %out, i32 6
  %call28 = tail call i32 @__amdil_sad_i32(i32 0, i32 %x, i32 %x) nounwind
  store i32 %call28, i32 addrspace(1)* %arrayidx25, align 4
  %arrayidx30 = getelementptr i32 addrspace(1)* %out, i32 7
  %call33 = tail call i32 @__amdil_sad_i32(i32 %x, i32 0, i32 %x) nounwind
  store i32 %call33, i32 addrspace(1)* %arrayidx30, align 4
  ret void
}

; TODO: The vector literals are still moved into registers first
define void @test_sad_combinations_v2(<2 x i32> addrspace(1)* nocapture %out, <2 x i32> %x) nounwind {
; SI-LABEL: @test_sad_combinations_v2
; SI: sad
; SI: sad
; SI: sad
; SI: sad
; SI: sad
; SI: sad
; SI: sad
; SI: sad
; SI: ret_dyn
  %call = tail call <2 x i32> @__amdil_sad_v2i32(<2 x i32> zeroinitializer, <2 x i32> zeroinitializer, <2 x i32> zeroinitializer) nounwind
  store <2 x i32> %call, <2 x i32> addrspace(1)* %out, align 8
  %arrayidx2 = getelementptr <2 x i32> addrspace(1)* %out, i32 1
  %call4 = tail call <2 x i32> @__amdil_sad_v2i32(<2 x i32> %x, <2 x i32> zeroinitializer, <2 x i32> zeroinitializer) nounwind
  store <2 x i32> %call4, <2 x i32> addrspace(1)* %arrayidx2, align 8
  %arrayidx6 = getelementptr <2 x i32> addrspace(1)* %out, i32 2
  %call8 = tail call <2 x i32> @__amdil_sad_v2i32(<2 x i32> zeroinitializer, <2 x i32> %x, <2 x i32> zeroinitializer) nounwind
  store <2 x i32> %call8, <2 x i32> addrspace(1)* %arrayidx6, align 8
  %arrayidx10 = getelementptr <2 x i32> addrspace(1)* %out, i32 3
  %call12 = tail call <2 x i32> @__amdil_sad_v2i32(<2 x i32> zeroinitializer, <2 x i32> zeroinitializer, <2 x i32> %x) nounwind
  store <2 x i32> %call12, <2 x i32> addrspace(1)* %arrayidx10, align 8
  %arrayidx14 = getelementptr <2 x i32> addrspace(1)* %out, i32 4
  %call17 = tail call <2 x i32> @__amdil_sad_v2i32(<2 x i32> %x, <2 x i32> %x, <2 x i32> zeroinitializer) nounwind
  store <2 x i32> %call17, <2 x i32> addrspace(1)* %arrayidx14, align 8
  %arrayidx19 = getelementptr <2 x i32> addrspace(1)* %out, i32 5
  %call23 = tail call <2 x i32> @__amdil_sad_v2i32(<2 x i32> %x, <2 x i32> %x, <2 x i32> %x) nounwind
  store <2 x i32> %call23, <2 x i32> addrspace(1)* %arrayidx19, align 8
  %arrayidx25 = getelementptr <2 x i32> addrspace(1)* %out, i32 6
  %call28 = tail call <2 x i32> @__amdil_sad_v2i32(<2 x i32> zeroinitializer, <2 x i32> %x, <2 x i32> %x) nounwind
  store <2 x i32> %call28, <2 x i32> addrspace(1)* %arrayidx25, align 8
  %arrayidx30 = getelementptr <2 x i32> addrspace(1)* %out, i32 7
  %call33 = tail call <2 x i32> @__amdil_sad_v2i32(<2 x i32> %x, <2 x i32> zeroinitializer, <2 x i32> %x) nounwind
  store <2 x i32> %call33, <2 x i32> addrspace(1)* %arrayidx30, align 8
  ret void
}

define void @test_sad_combinations_v4(<4 x i32> addrspace(1)* nocapture %out, <4 x i32> %x) nounwind {
; SI-LABEL: @test_sad_combinations_v4
; SI: sad
; SI: sad
; SI: sad
; SI: sad
; SI: sad
; SI: sad
; SI: sad
; SI: sad
; SI: ret_dyn
  %call = tail call <4 x i32> @__amdil_sad_v4i32(<4 x i32> zeroinitializer, <4 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  store <4 x i32> %call, <4 x i32> addrspace(1)* %out, align 16
  %arrayidx2 = getelementptr <4 x i32> addrspace(1)* %out, i32 1
  %call4 = tail call <4 x i32> @__amdil_sad_v4i32(<4 x i32> %x, <4 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  store <4 x i32> %call4, <4 x i32> addrspace(1)* %arrayidx2, align 16
  %arrayidx6 = getelementptr <4 x i32> addrspace(1)* %out, i32 2
  %call8 = tail call <4 x i32> @__amdil_sad_v4i32(<4 x i32> zeroinitializer, <4 x i32> %x, <4 x i32> zeroinitializer) nounwind
  store <4 x i32> %call8, <4 x i32> addrspace(1)* %arrayidx6, align 16
  %arrayidx10 = getelementptr <4 x i32> addrspace(1)* %out, i32 3
  %call12 = tail call <4 x i32> @__amdil_sad_v4i32(<4 x i32> zeroinitializer, <4 x i32> zeroinitializer, <4 x i32> %x) nounwind
  store <4 x i32> %call12, <4 x i32> addrspace(1)* %arrayidx10, align 16
  %arrayidx14 = getelementptr <4 x i32> addrspace(1)* %out, i32 4
  %call17 = tail call <4 x i32> @__amdil_sad_v4i32(<4 x i32> %x, <4 x i32> %x, <4 x i32> zeroinitializer) nounwind
  store <4 x i32> %call17, <4 x i32> addrspace(1)* %arrayidx14, align 16
  %arrayidx19 = getelementptr <4 x i32> addrspace(1)* %out, i32 5
  %call23 = tail call <4 x i32> @__amdil_sad_v4i32(<4 x i32> %x, <4 x i32> %x, <4 x i32> %x) nounwind
  store <4 x i32> %call23, <4 x i32> addrspace(1)* %arrayidx19, align 16
  %arrayidx25 = getelementptr <4 x i32> addrspace(1)* %out, i32 6
  %call28 = tail call <4 x i32> @__amdil_sad_v4i32(<4 x i32> zeroinitializer, <4 x i32> %x, <4 x i32> %x) nounwind
  store <4 x i32> %call28, <4 x i32> addrspace(1)* %arrayidx25, align 16
  %arrayidx30 = getelementptr <4 x i32> addrspace(1)* %out, i32 7
  %call33 = tail call <4 x i32> @__amdil_sad_v4i32(<4 x i32> %x, <4 x i32> zeroinitializer, <4 x i32> %x) nounwind
  store <4 x i32> %call33, <4 x i32> addrspace(1)* %arrayidx30, align 16
  ret void
}

