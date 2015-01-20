; ModuleID = 'basic_10.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32"
target triple = "amdil-pc-unknown-amdopencl"

@.str = internal addrspace(2) constant [5 x i8] c"srcA\00"
@.str1 = internal addrspace(2) constant [5 x i8] c"int*\00"
@.str2 = internal addrspace(2) constant [5 x i8] c"srcB\00"
@.str3 = internal addrspace(2) constant [5 x i8] c"int*\00"
@.str4 = internal addrspace(2) constant [4 x i8] c"dst\00"
@.str5 = internal addrspace(2) constant [5 x i8] c"int*\00"
@llvm.signedOrSignedpointee.annotations.__OpenCL_test_int_add_kernel = global [3 x i8*] [i8* bitcast ([5 x i8] addrspace(2)* @.str to i8*), i8* bitcast ([5 x i8] addrspace(2)* @.str2 to i8*), i8* bitcast ([4 x i8] addrspace(2)* @.str4 to i8*)], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_test_int_add_kernel = global [3 x i8*] [i8* bitcast ([5 x i8] addrspace(2)* @.str1 to i8*), i8* bitcast ([5 x i8] addrspace(2)* @.str3 to i8*), i8* bitcast ([5 x i8] addrspace(2)* @.str5 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*)* @__OpenCL_test_int_add_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define void @__OpenCL_test_int_add_kernel(i32 addrspace(1)* %srcA, i32 addrspace(1)* %srcB, i32 addrspace(1)* %dst) nounwind {
entry:
  %srcA.addr = alloca i32 addrspace(1)*, align 4
  %srcB.addr = alloca i32 addrspace(1)*, align 4
  %dst.addr = alloca i32 addrspace(1)*, align 4
  %tid = alloca i32, align 4
  store i32 addrspace(1)* %srcA, i32 addrspace(1)** %srcA.addr, align 4
  store i32 addrspace(1)* %srcB, i32 addrspace(1)** %srcB.addr, align 4
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %dst.addr, align 4
  %call = call i32 @get_global_id(i32 0) nounwind
  store i32 %call, i32* %tid, align 4
  %tmp = load i32 addrspace(1)** %dst.addr, align 4
  %tmp1 = load i32* %tid, align 4
  %arrayidx = getelementptr i32 addrspace(1)* %tmp, i32 %tmp1
  %tmp2 = load i32 addrspace(1)** %srcA.addr, align 4
  %tmp3 = load i32* %tid, align 4
  %arrayidx4 = getelementptr i32 addrspace(1)* %tmp2, i32 %tmp3
  %tmp5 = load i32 addrspace(1)* %arrayidx4, align 4
  %tmp6 = load i32 addrspace(1)** %srcB.addr, align 4
  %tmp7 = load i32* %tid, align 4
  %arrayidx8 = getelementptr i32 addrspace(1)* %tmp6, i32 %tmp7
  %tmp9 = load i32 addrspace(1)* %arrayidx8, align 4
  %tmp10 = add nsw i32 %tmp5, %tmp9
  store i32 %tmp10, i32 addrspace(1)* %arrayidx, align 4
  br label %return

return:                                           ; preds = %entry
  ret void
}

declare i32 @get_global_id(i32) nounwind
