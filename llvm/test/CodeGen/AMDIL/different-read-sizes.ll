; RUN: llc -march=amdil -mattr=+images -mcpu=tahiti -enable-tail-merge | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

%struct._image2d_array_t = type opaque

; Make sure that reads of different sizes don't get recognized as the same
define void @different_read_sizes(i8 addrspace(1)* nocapture %src, %struct._image2d_array_t addrspace(1)* %dst, i32 %format) nounwind {
;CHECK: uav_ushort_load_id
;CHECK: uav_ubyte_load_id
entry:
  switch i32 %format, label %dummylabel [
    i32 1, label %switch.case
    i32 2, label %switch.case8
  ]

switch.case:                                      ; preds = %entry
  %tmp4 = load i8 addrspace(1)* %src, align 1
  %conv5 = zext i8 %tmp4 to i32
  %tmp7 = insertelement <4 x i32> <i32 undef, i32 0, i32 0, i32 0>, i32 %conv5, i32 0
  br label %dummylabel

dummylabel:                                       ; preds = %switch.case8, %switch.case, %entry
  %pixel.0 = phi <4 x i32> [ zeroinitializer, %entry ], [ %tmp14, %switch.case8 ], [ %tmp7, %switch.case ]
  tail call void @__amdil_image2d_array_write(%struct._image2d_array_t addrspace(1)* %dst, <4 x i32> zeroinitializer, <4 x i32> %pixel.0) nounwind
  ret void

switch.case8:                                     ; preds = %entry
  %conv = bitcast i8 addrspace(1)* %src to i16 addrspace(1)*
  %tmp11 = load i16 addrspace(1)* %conv, align 2
  %conv12 = zext i16 %tmp11 to i32
  %tmp14 = insertelement <4 x i32> <i32 undef, i32 0, i32 0, i32 0>, i32 %conv12, i32 0
  br label %dummylabel
}

declare void @__amdil_image2d_array_write(%struct._image2d_array_t addrspace(1)*, <4 x i32>, <4 x i32>) nounwind
