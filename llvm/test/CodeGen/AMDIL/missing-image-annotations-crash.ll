; RUN: llc -march=amdil < %s | FileCheck %s

; Annotations missing

%struct._image2d_array_t.0.1.2.4 = type opaque

declare void @foo() nounwind

define void @__OpenCL_image_kernel(%struct._image2d_array_t.0.1.2.4 addrspace(1)* %dst) nounwind {
entry:
  br i1 undef, label %if.end, label %return

return:
  ret void

if.end:
  call void @foo() nounwind
  unreachable
}

