; RUN: llc -march=amdil -mcpu=tahiti < %s
; Crash on printf of a 3 vector

@.str = internal addrspace(2) constant [16 x i8] c"my_float3=%v3f\0A\00"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void ()* @__OpenCL_sample_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"


; CHECK: call {{[0-9]+}} ; ___dumpBytes_v1b128

define void @__OpenCL_sample_kernel() nounwind {
entry:
  %call = tail call i32 (i8 addrspace(2)*, ...)* @printf(i8 addrspace(2)* getelementptr inbounds ([16 x i8] addrspace(2)* @.str, i32 0, i32 0), <3 x float> <float 4.000000e+00, float 3.000000e+00, float 2.000000e+00>) nounwind
  ret void
}

declare i32 @printf(i8 addrspace(2)*, ...) nounwind

declare <4 x i32> @__amdil_get_local_size_int() nounwind readnone

declare <4 x i32> @__amdil_get_local_id_int() nounwind readnone

declare i32 @__amdil_get_printf_item_offset() nounwind readonly

declare void @__amdil_inc_printf_item_offset(i32) nounwind

declare i32 @__amdil_get_printf_size() nounwind readnone

declare i32 addrspace(1)* @__amdil_get_printf_offset() nounwind

declare i32 @___dumpBytes_v1b128(<2 x i64>) nounwind alwaysinline
