; RUN: llc -march=amdil -mcpu=tahiti <%s |FileCheck %s
;CHECK: dcl_typeless_uav_id(9)_stride(4)_length(4)_access(read_write)
;CHECK: dcl_typeless_uav_id(11)_stride(4)_length(4)_access(read_write)
;CHECK: ;ARGSTART
;CHECK: ;uavid:11
;CHECK: ;printfid:9
;CHECK: ;ARGEND
;CHECK: func
;CHECK: uav_raw_store_id(9)
;CHECK: uav_raw_store_id(9)
;CHECK: uav_raw_store_id(9)
;CHECK: uav_raw_store_id(9)
;CHECK: endfunc

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

@.str = internal addrspace(2) constant [5 x i8] c"%2d\0A\00"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void ()* @__OpenCL_printfKernel_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define  void @__OpenCL_printfKernel_kernel() nounwind {
entry:
  %call = tail call  i32 (i8 addrspace(2)*, ...)* @printf(i8 addrspace(2)* getelementptr inbounds ([5 x i8] addrspace(2)* @.str, i32 0, i32 0), i32 0) nounwind
  ret void
}

declare  i32 @printf(i8 addrspace(2)*, ...) nounwind

declare  <4 x i32> @__amdil_get_local_size_int() nounwind readonly

declare  <4 x i32> @__amdil_get_local_id_int() nounwind readonly

declare  i32 @__amdil_get_printf_item_offset() nounwind

declare  void @__amdil_inc_printf_item_offset(i32) nounwind

define  void @___dumpStringID(i32) nounwind {
  %2 = tail call  i32 @__amdil_get_printf_size() nounwind
  %3 = icmp eq i32 %2, 0
  br i1 %3, label %___dumpBytes_v1b32.exit, label %4

; <label>:4                                       ; preds = %1
  %5 = tail call  i32 addrspace(1)* @__amdil_get_printf_offset() nounwind
  %6 = tail call  i32 @__amdil_get_printf_size() nounwind
  %7 = tail call  <4 x i32> @__amdil_get_local_size_int() nounwind
  %8 = tail call  <4 x i32> @__amdil_get_local_id_int() nounwind
  %9 = extractelement <4 x i32> %8, i32 0
  %10 = extractelement <4 x i32> %7, i32 0
  %11 = extractelement <4 x i32> %8, i32 1
  %12 = extractelement <4 x i32> %7, i32 1
  %13 = extractelement <4 x i32> %8, i32 2
  %14 = mul i32 %13, %12
  %15 = add nsw i32 %14, %11
  %16 = mul i32 %15, %10
  %17 = add nsw i32 %16, %9
  %18 = mul i32 %17, %6
  %19 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  %20 = sub i32 %2, %19
  %21 = icmp ult i32 %20, 2
  br i1 %21, label %___dumpBytes_v1b32.exit, label %22

; <label>:22                                      ; preds = %4
  %23 = getelementptr i32 addrspace(1)* %5, i32 %18
  %24 = add i32 %18, %19
  %25 = getelementptr i32 addrspace(1)* %5, i32 %24
  store i32 %0, i32 addrspace(1)* %25, align 4
  tail call  void @__amdil_inc_printf_item_offset(i32 1) nounwind
  %26 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  store i32 %26, i32 addrspace(1)* %23, align 4
  br label %___dumpBytes_v1b32.exit

___dumpBytes_v1b32.exit:                          ; preds = %22, %4, %1
  ret void
}

define  i32 @___dumpBytes_v1b8(i8 zeroext) nounwind {
  %2 = tail call  i32 @__amdil_get_printf_size() nounwind
  %3 = icmp eq i32 %2, 0
  br i1 %3, label %4, label %6

; <label>:4                                       ; preds = %24, %6, %1
  %5 = phi i32 [ 1, %24 ], [ 1, %1 ], [ 0, %6 ]
  ret i32 %5

; <label>:6                                       ; preds = %1
  %7 = tail call  i32 addrspace(1)* @__amdil_get_printf_offset() nounwind
  %8 = tail call  i32 @__amdil_get_printf_size() nounwind
  %9 = tail call  <4 x i32> @__amdil_get_local_size_int() nounwind
  %10 = tail call  <4 x i32> @__amdil_get_local_id_int() nounwind
  %11 = extractelement <4 x i32> %10, i32 0
  %12 = extractelement <4 x i32> %9, i32 0
  %13 = extractelement <4 x i32> %10, i32 1
  %14 = extractelement <4 x i32> %9, i32 1
  %15 = extractelement <4 x i32> %10, i32 2
  %16 = mul i32 %15, %14
  %17 = add nsw i32 %16, %13
  %18 = mul i32 %17, %12
  %19 = add nsw i32 %18, %11
  %20 = mul i32 %19, %8
  %21 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  %22 = sub i32 %2, %21
  %23 = icmp ult i32 %22, 2
  br i1 %23, label %4, label %24

; <label>:24                                      ; preds = %6
  %25 = getelementptr i32 addrspace(1)* %7, i32 %20
  %26 = add i32 %20, %21
  %27 = getelementptr i32 addrspace(1)* %7, i32 %26
  %28 = zext i8 %0 to i32
  store i32 %28, i32 addrspace(1)* %27, align 4
  tail call  void @__amdil_inc_printf_item_offset(i32 1) nounwind
  %29 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  store i32 %29, i32 addrspace(1)* %25, align 4
  br label %4
}

declare  i32 @__amdil_get_printf_size() nounwind

declare  i32 addrspace(1)* @__amdil_get_printf_offset() nounwind

define  i32 @___dumpBytes_v1b16(i16 zeroext) nounwind {
  %2 = tail call  i32 @__amdil_get_printf_size() nounwind
  %3 = icmp eq i32 %2, 0
  br i1 %3, label %4, label %6

; <label>:4                                       ; preds = %24, %6, %1
  %5 = phi i32 [ 1, %24 ], [ 1, %1 ], [ 0, %6 ]
  ret i32 %5

; <label>:6                                       ; preds = %1
  %7 = tail call  i32 addrspace(1)* @__amdil_get_printf_offset() nounwind
  %8 = tail call  i32 @__amdil_get_printf_size() nounwind
  %9 = tail call  <4 x i32> @__amdil_get_local_size_int() nounwind
  %10 = tail call  <4 x i32> @__amdil_get_local_id_int() nounwind
  %11 = extractelement <4 x i32> %10, i32 0
  %12 = extractelement <4 x i32> %9, i32 0
  %13 = extractelement <4 x i32> %10, i32 1
  %14 = extractelement <4 x i32> %9, i32 1
  %15 = extractelement <4 x i32> %10, i32 2
  %16 = mul i32 %15, %14
  %17 = add nsw i32 %16, %13
  %18 = mul i32 %17, %12
  %19 = add nsw i32 %18, %11
  %20 = mul i32 %19, %8
  %21 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  %22 = sub i32 %2, %21
  %23 = icmp ult i32 %22, 2
  br i1 %23, label %4, label %24

; <label>:24                                      ; preds = %6
  %25 = getelementptr i32 addrspace(1)* %7, i32 %20
  %26 = add i32 %20, %21
  %27 = getelementptr i32 addrspace(1)* %7, i32 %26
  %28 = zext i16 %0 to i32
  store i32 %28, i32 addrspace(1)* %27, align 4
  tail call  void @__amdil_inc_printf_item_offset(i32 1) nounwind
  %29 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  store i32 %29, i32 addrspace(1)* %25, align 4
  br label %4
}

define  i32 @___dumpBytes_v1b32(i32) nounwind {
  %2 = tail call  i32 @__amdil_get_printf_size() nounwind
  %3 = icmp eq i32 %2, 0
  br i1 %3, label %4, label %6

; <label>:4                                       ; preds = %24, %6, %1
  %5 = phi i32 [ 1, %24 ], [ 1, %1 ], [ 0, %6 ]
  ret i32 %5

; <label>:6                                       ; preds = %1
  %7 = tail call  i32 addrspace(1)* @__amdil_get_printf_offset() nounwind
  %8 = tail call  i32 @__amdil_get_printf_size() nounwind
  %9 = tail call  <4 x i32> @__amdil_get_local_size_int() nounwind
  %10 = tail call  <4 x i32> @__amdil_get_local_id_int() nounwind
  %11 = extractelement <4 x i32> %10, i32 0
  %12 = extractelement <4 x i32> %9, i32 0
  %13 = extractelement <4 x i32> %10, i32 1
  %14 = extractelement <4 x i32> %9, i32 1
  %15 = extractelement <4 x i32> %10, i32 2
  %16 = mul i32 %15, %14
  %17 = add nsw i32 %16, %13
  %18 = mul i32 %17, %12
  %19 = add nsw i32 %18, %11
  %20 = mul i32 %19, %8
  %21 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  %22 = sub i32 %2, %21
  %23 = icmp ult i32 %22, 2
  br i1 %23, label %4, label %24

; <label>:24                                      ; preds = %6
  %25 = getelementptr i32 addrspace(1)* %7, i32 %20
  %26 = add i32 %20, %21
  %27 = getelementptr i32 addrspace(1)* %7, i32 %26
  store i32 %0, i32 addrspace(1)* %27, align 4
  tail call  void @__amdil_inc_printf_item_offset(i32 1) nounwind
  %28 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  store i32 %28, i32 addrspace(1)* %25, align 4
  br label %4
}

define  i32 @___dumpBytes_v1b64(i64) nounwind {
  %2 = tail call  i32 @__amdil_get_printf_size() nounwind
  %3 = icmp eq i32 %2, 0
  br i1 %3, label %4, label %6

; <label>:4                                       ; preds = %24, %6, %1
  %5 = phi i32 [ 1, %24 ], [ 1, %1 ], [ 0, %6 ]
  ret i32 %5

; <label>:6                                       ; preds = %1
  %7 = tail call  i32 addrspace(1)* @__amdil_get_printf_offset() nounwind
  %8 = tail call  i32 @__amdil_get_printf_size() nounwind
  %9 = tail call  <4 x i32> @__amdil_get_local_size_int() nounwind
  %10 = tail call  <4 x i32> @__amdil_get_local_id_int() nounwind
  %11 = extractelement <4 x i32> %10, i32 0
  %12 = extractelement <4 x i32> %9, i32 0
  %13 = extractelement <4 x i32> %10, i32 1
  %14 = extractelement <4 x i32> %9, i32 1
  %15 = extractelement <4 x i32> %10, i32 2
  %16 = mul i32 %15, %14
  %17 = add nsw i32 %16, %13
  %18 = mul i32 %17, %12
  %19 = add nsw i32 %18, %11
  %20 = mul i32 %19, %8
  %21 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  %22 = sub i32 %2, %21
  %23 = icmp ult i32 %22, 3
  br i1 %23, label %4, label %24

; <label>:24                                      ; preds = %6
  %25 = getelementptr i32 addrspace(1)* %7, i32 %20
  %26 = add i32 %20, %21
  %27 = getelementptr i32 addrspace(1)* %7, i32 %26
  %28 = bitcast i32 addrspace(1)* %27 to i64 addrspace(1)*
  store i64 %0, i64 addrspace(1)* %28, align 8
  tail call  void @__amdil_inc_printf_item_offset(i32 2) nounwind
  %29 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  store i32 %29, i32 addrspace(1)* %25, align 4
  br label %4
}

define  i32 @___dumpBytes_v1b128(<2 x i64>) nounwind {
  %2 = tail call  i32 @__amdil_get_printf_size() nounwind
  %3 = icmp eq i32 %2, 0
  br i1 %3, label %4, label %6

; <label>:4                                       ; preds = %24, %6, %1
  %5 = phi i32 [ 1, %24 ], [ 1, %1 ], [ 0, %6 ]
  ret i32 %5

; <label>:6                                       ; preds = %1
  %7 = tail call  i32 addrspace(1)* @__amdil_get_printf_offset() nounwind
  %8 = tail call  i32 @__amdil_get_printf_size() nounwind
  %9 = tail call  <4 x i32> @__amdil_get_local_size_int() nounwind
  %10 = tail call  <4 x i32> @__amdil_get_local_id_int() nounwind
  %11 = extractelement <4 x i32> %10, i32 0
  %12 = extractelement <4 x i32> %9, i32 0
  %13 = extractelement <4 x i32> %10, i32 1
  %14 = extractelement <4 x i32> %9, i32 1
  %15 = extractelement <4 x i32> %10, i32 2
  %16 = mul i32 %15, %14
  %17 = add nsw i32 %16, %13
  %18 = mul i32 %17, %12
  %19 = add nsw i32 %18, %11
  %20 = mul i32 %19, %8
  %21 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  %22 = sub i32 %2, %21
  %23 = icmp ult i32 %22, 5
  br i1 %23, label %4, label %24

; <label>:24                                      ; preds = %6
  %25 = getelementptr i32 addrspace(1)* %7, i32 %20
  %26 = add i32 %20, %21
  %27 = getelementptr i32 addrspace(1)* %7, i32 %26
  %28 = bitcast i32 addrspace(1)* %27 to <2 x i64> addrspace(1)*
  store <2 x i64> %0, <2 x i64> addrspace(1)* %28, align 16
  tail call  void @__amdil_inc_printf_item_offset(i32 4) nounwind
  %29 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  store i32 %29, i32 addrspace(1)* %25, align 4
  br label %4
}

define  i32 @___dumpBytes_v1b256(<4 x i64>) nounwind {
  %2 = tail call  i32 @__amdil_get_printf_size() nounwind
  %3 = icmp eq i32 %2, 0
  br i1 %3, label %4, label %6

; <label>:4                                       ; preds = %24, %6, %1
  %5 = phi i32 [ 1, %24 ], [ 1, %1 ], [ 0, %6 ]
  ret i32 %5

; <label>:6                                       ; preds = %1
  %7 = tail call  i32 addrspace(1)* @__amdil_get_printf_offset() nounwind
  %8 = tail call  i32 @__amdil_get_printf_size() nounwind
  %9 = tail call  <4 x i32> @__amdil_get_local_size_int() nounwind
  %10 = tail call  <4 x i32> @__amdil_get_local_id_int() nounwind
  %11 = extractelement <4 x i32> %10, i32 0
  %12 = extractelement <4 x i32> %9, i32 0
  %13 = extractelement <4 x i32> %10, i32 1
  %14 = extractelement <4 x i32> %9, i32 1
  %15 = extractelement <4 x i32> %10, i32 2
  %16 = mul i32 %15, %14
  %17 = add nsw i32 %16, %13
  %18 = mul i32 %17, %12
  %19 = add nsw i32 %18, %11
  %20 = mul i32 %19, %8
  %21 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  %22 = sub i32 %2, %21
  %23 = icmp ult i32 %22, 9
  br i1 %23, label %4, label %24

; <label>:24                                      ; preds = %6
  %25 = getelementptr i32 addrspace(1)* %7, i32 %20
  %26 = add i32 %20, %21
  %27 = getelementptr i32 addrspace(1)* %7, i32 %26
  %28 = bitcast i32 addrspace(1)* %27 to <4 x i64> addrspace(1)*
  store <4 x i64> %0, <4 x i64> addrspace(1)* %28, align 32
  tail call  void @__amdil_inc_printf_item_offset(i32 8) nounwind
  %29 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  store i32 %29, i32 addrspace(1)* %25, align 4
  br label %4
}

define  i32 @___dumpBytes_v1b512(<8 x i64>) nounwind {
  %2 = tail call  i32 @__amdil_get_printf_size() nounwind
  %3 = icmp eq i32 %2, 0
  br i1 %3, label %4, label %6

; <label>:4                                       ; preds = %24, %6, %1
  %5 = phi i32 [ 1, %24 ], [ 1, %1 ], [ 0, %6 ]
  ret i32 %5

; <label>:6                                       ; preds = %1
  %7 = tail call  i32 addrspace(1)* @__amdil_get_printf_offset() nounwind
  %8 = tail call  i32 @__amdil_get_printf_size() nounwind
  %9 = tail call  <4 x i32> @__amdil_get_local_size_int() nounwind
  %10 = tail call  <4 x i32> @__amdil_get_local_id_int() nounwind
  %11 = extractelement <4 x i32> %10, i32 0
  %12 = extractelement <4 x i32> %9, i32 0
  %13 = extractelement <4 x i32> %10, i32 1
  %14 = extractelement <4 x i32> %9, i32 1
  %15 = extractelement <4 x i32> %10, i32 2
  %16 = mul i32 %15, %14
  %17 = add nsw i32 %16, %13
  %18 = mul i32 %17, %12
  %19 = add nsw i32 %18, %11
  %20 = mul i32 %19, %8
  %21 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  %22 = sub i32 %2, %21
  %23 = icmp ult i32 %22, 17
  br i1 %23, label %4, label %24

; <label>:24                                      ; preds = %6
  %25 = getelementptr i32 addrspace(1)* %7, i32 %20
  %26 = add i32 %20, %21
  %27 = getelementptr i32 addrspace(1)* %7, i32 %26
  %28 = bitcast i32 addrspace(1)* %27 to <8 x i64> addrspace(1)*
  store <8 x i64> %0, <8 x i64> addrspace(1)* %28, align 64
  tail call  void @__amdil_inc_printf_item_offset(i32 16) nounwind
  %29 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  store i32 %29, i32 addrspace(1)* %25, align 4
  br label %4
}

define  i32 @___dumpBytes_v1b1024(<16 x i64>) nounwind {
  %2 = tail call  i32 @__amdil_get_printf_size() nounwind
  %3 = icmp eq i32 %2, 0
  br i1 %3, label %4, label %6

; <label>:4                                       ; preds = %24, %6, %1
  %5 = phi i32 [ 1, %24 ], [ 1, %1 ], [ 0, %6 ]
  ret i32 %5

; <label>:6                                       ; preds = %1
  %7 = tail call  i32 addrspace(1)* @__amdil_get_printf_offset() nounwind
  %8 = tail call  i32 @__amdil_get_printf_size() nounwind
  %9 = tail call  <4 x i32> @__amdil_get_local_size_int() nounwind
  %10 = tail call  <4 x i32> @__amdil_get_local_id_int() nounwind
  %11 = extractelement <4 x i32> %10, i32 0
  %12 = extractelement <4 x i32> %9, i32 0
  %13 = extractelement <4 x i32> %10, i32 1
  %14 = extractelement <4 x i32> %9, i32 1
  %15 = extractelement <4 x i32> %10, i32 2
  %16 = mul i32 %15, %14
  %17 = add nsw i32 %16, %13
  %18 = mul i32 %17, %12
  %19 = add nsw i32 %18, %11
  %20 = mul i32 %19, %8
  %21 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  %22 = sub i32 %2, %21
  %23 = icmp ult i32 %22, 33
  br i1 %23, label %4, label %24

; <label>:24                                      ; preds = %6
  %25 = getelementptr i32 addrspace(1)* %7, i32 %20
  %26 = add i32 %20, %21
  %27 = getelementptr i32 addrspace(1)* %7, i32 %26
  %28 = bitcast i32 addrspace(1)* %27 to <16 x i64> addrspace(1)*
  store <16 x i64> %0, <16 x i64> addrspace(1)* %28, align 128
  tail call  void @__amdil_inc_printf_item_offset(i32 32) nounwind
  %29 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  store i32 %29, i32 addrspace(1)* %25, align 4
  br label %4
}

define  i32 @___dumpBytes_v1bs(i8 addrspace(2)*) nounwind {
  %2 = tail call  i32 @__amdil_get_printf_size() nounwind
  %3 = icmp eq i32 %2, 0
  br i1 %3, label %4, label %6

; <label>:4                                       ; preds = %.loopexit, %24, %1
  %5 = phi i32 [ 1, %.loopexit ], [ 1, %1 ], [ 0, %24 ]
  ret i32 %5

; <label>:6                                       ; preds = %1
  %7 = tail call  i32 addrspace(1)* @__amdil_get_printf_offset() nounwind
  %8 = tail call  i32 @__amdil_get_printf_size() nounwind
  %9 = tail call  <4 x i32> @__amdil_get_local_size_int() nounwind
  %10 = tail call  <4 x i32> @__amdil_get_local_id_int() nounwind
  %11 = extractelement <4 x i32> %9, i32 0
  %12 = extractelement <4 x i32> %10, i32 1
  %13 = extractelement <4 x i32> %9, i32 1
  %14 = extractelement <4 x i32> %10, i32 2
  %15 = mul i32 %14, %13
  %16 = add nsw i32 %15, %12
  %17 = mul i32 %16, %11
  %18 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  br label %19

; <label>:19                                      ; preds = %19, %6
  %20 = phi i8 addrspace(2)* [ %0, %6 ], [ %21, %19 ]
  %21 = getelementptr i8 addrspace(2)* %20, i32 1
  %22 = load i8 addrspace(2)* %21, align 1
  %23 = icmp eq i8 %22, 0
  br i1 %23, label %24, label %19

; <label>:24                                      ; preds = %19
  %25 = extractelement <4 x i32> %10, i32 0
  %26 = add nsw i32 %17, %25
  %27 = mul i32 %26, %8
  %28 = getelementptr i32 addrspace(1)* %7, i32 %27
  %29 = ptrtoint i8 addrspace(2)* %21 to i32
  %30 = ptrtoint i8 addrspace(2)* %0 to i32
  %31 = sub i32 %29, %30
  %32 = add i32 %31, 4
  %33 = lshr i32 %32, 2
  %34 = sub i32 %2, %18
  %35 = icmp ult i32 %33, %34
  br i1 %35, label %36, label %4

; <label>:36                                      ; preds = %24
  %37 = icmp ugt i32 %31, 3
  br i1 %37, label %.preheader, label %.loopexit

.loopexit:                                        ; preds = %.preheader, %36
  %38 = phi i8 addrspace(2)* [ %0, %36 ], [ %86, %.preheader ]
  %39 = phi i32 [ %18, %36 ], [ %84, %.preheader ]
  %40 = phi i32 [ %31, %36 ], [ %85, %.preheader ]
  %41 = load i8 addrspace(2)* %38, align 1
  %42 = zext i8 %41 to i32
  %43 = getelementptr i8 addrspace(2)* %38, i32 1
  %44 = load i8 addrspace(2)* %43, align 1
  %45 = zext i8 %44 to i32
  %46 = shl nuw nsw i32 %45, 8
  %47 = getelementptr i8 addrspace(2)* %38, i32 2
  %48 = load i8 addrspace(2)* %47, align 1
  %49 = zext i8 %48 to i32
  %50 = shl nuw nsw i32 %49, 16
  %51 = icmp eq i32 %40, 0
  %52 = select i1 %51, i32 0, i32 %42
  %53 = icmp ugt i32 %40, 1
  %54 = select i1 %53, i32 %46, i32 0
  %55 = icmp ugt i32 %40, 2
  %56 = select i1 %55, i32 %50, i32 0
  %57 = or i32 %54, %52
  %58 = or i32 %57, %56
  %59 = add i32 %39, %27
  %60 = getelementptr i32 addrspace(1)* %7, i32 %59
  store i32 %58, i32 addrspace(1)* %60, align 4
  tail call  void @__amdil_inc_printf_item_offset(i32 1) nounwind
  %61 = tail call  i32 @__amdil_get_printf_item_offset() nounwind
  store i32 %61, i32 addrspace(1)* %28, align 4
  br label %4

.preheader:                                       ; preds = %.preheader, %36
  %62 = phi i8 addrspace(2)* [ %86, %.preheader ], [ %0, %36 ]
  %63 = phi i32 [ %84, %.preheader ], [ %18, %36 ]
  %64 = phi i32 [ %85, %.preheader ], [ %31, %36 ]
  %65 = getelementptr i8 addrspace(2)* %62, i32 3
  %66 = load i8 addrspace(2)* %65, align 1
  %67 = zext i8 %66 to i32
  %68 = shl nuw i32 %67, 24
  %69 = getelementptr i8 addrspace(2)* %62, i32 2
  %70 = load i8 addrspace(2)* %69, align 1
  %71 = zext i8 %70 to i32
  %72 = shl nuw nsw i32 %71, 16
  %73 = getelementptr i8 addrspace(2)* %62, i32 1
  %74 = load i8 addrspace(2)* %73, align 1
  %75 = zext i8 %74 to i32
  %76 = shl nuw nsw i32 %75, 8
  %77 = load i8 addrspace(2)* %62, align 1
  %78 = zext i8 %77 to i32
  %79 = or i32 %72, %68
  %80 = or i32 %79, %78
  %81 = or i32 %80, %76
  %82 = add i32 %63, %27
  %83 = getelementptr i32 addrspace(1)* %7, i32 %82
  store i32 %81, i32 addrspace(1)* %83, align 4
  tail call  void @__amdil_inc_printf_item_offset(i32 1) nounwind
  %84 = add i32 %63, 1
  %85 = add i32 %64, -4
  %86 = getelementptr i8 addrspace(2)* %62, i32 4
  %87 = icmp ugt i32 %85, 3
  br i1 %87, label %.preheader, label %.loopexit
}
