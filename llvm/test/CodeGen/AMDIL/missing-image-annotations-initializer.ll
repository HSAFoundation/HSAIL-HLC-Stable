; RUN: llc -march=amdil -o /dev/null %s

%struct._image2d_array_t.0.1.2.3 = type opaque

@llvm.image.annotations.__OpenCL_foo_kernel = external global [1 x <{ i8*, i32 }>], section "llvm.metadata"

define void @__OpenCL_foo_kernel(%struct._image2d_array_t.0.1.2.3 addrspace(1)* %dst) nounwind {
entry:
  unreachable
}
