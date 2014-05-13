; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s

; CHECK: func
; CHECK: dcl_literal [[ZERO:l[0-9]+]], 0x00000000
; CHECK: dcl_literal [[OFFSET:l[0-9]+]], 0x00001000
; CHECK: if_logicalnz
; CHECK: mov [[REG:r[0-9]+]].x___, [[ZERO]]
; CHECK: else
; CHECK: mov [[REG]].x___, [[OFFSET]]
; CHECK: iadd [[REG]].x___, [[REG]].x, l{{[0-9]+}}
; CHECK: endif
; CHECK: lds_store_id(2) [[REG]].x, r{{[0-9]+}}.x

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-amdopencl"

@.str = internal addrspace(2) constant [4 x i8] c"out\00"
@.str1 = internal addrspace(2) constant [5 x i8] c"int*\00"
@.str2 = internal addrspace(2) constant [2 x i8] c"i\00"
@.str3 = internal addrspace(2) constant [4 x i8] c"int\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_test_kernel = global [2 x i8*] [i8* bitcast ([4 x i8] addrspace(2)* @.str to i8*), i8* bitcast ([2 x i8] addrspace(2)* @.str2 to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_test_kernel = global [2 x i8*] [i8* bitcast ([5 x i8] addrspace(2)* @.str1 to i8*), i8* bitcast ([4 x i8] addrspace(2)* @.str3 to i8*)], section "llvm.metadata"
@test_cllocal_buffer = internal addrspace(3) global [1024 x i32] zeroinitializer, align 4
@test_cllocal_buffer2 = internal addrspace(3) global [1024 x i32] zeroinitializer, align 4
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [2 x i8*] [i8* bitcast ([1024 x i32] addrspace(3)* @test_cllocal_buffer to i8*), i8* bitcast ([1024 x i32] addrspace(3)* @test_cllocal_buffer2 to i8*)]
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (i32 addrspace(1)*, i32)* @__OpenCL_test_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([2 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define  void @__OpenCL_test_kernel(i32 addrspace(1)* nocapture %out, i32 %i) nounwind {
entry:
  %cmp = icmp sgt i32 %i, 0
  br i1 %cmp, label %if.then, label %if.else

if.end:                                           ; preds = %if.else, %if.then
  %ptr.0 = phi i32 addrspace(3)* [ getelementptr inbounds ([1024 x i32] addrspace(3)* @test_cllocal_buffer, i32 0, i32 0), %if.then ], [ getelementptr inbounds ([1024 x i32] addrspace(3)* @test_cllocal_buffer2, i32 0, i32 1), %if.else ]
  store i32 1, i32 addrspace(3)* %ptr.0, align 4
  %tmp16 = load i32 addrspace(3)* getelementptr inbounds ([1024 x i32] addrspace(3)* @test_cllocal_buffer, i32 0, i32 0), align 4
  store i32 %tmp16, i32 addrspace(1)* %out, align 4
  ret void

if.then:                                          ; preds = %entry
  %arrayidx = getelementptr i32 addrspace(1)* %out, i32 1
  store i32 1, i32 addrspace(1)* %arrayidx, align 4
  br label %if.end

if.else:                                          ; preds = %entry
  %arrayidx9 = getelementptr i32 addrspace(1)* %out, i32 2
  store i32 2, i32 addrspace(1)* %arrayidx9, align 4
  br label %if.end
}
