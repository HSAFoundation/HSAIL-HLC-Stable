; RUN: llc -march=amdil -mcpu=barts < %s

declare void @llvm.lifetime.start(i64, i8* nocapture) nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) nounwind

define void @foo() {
entry:
  %temp.i = alloca [8 x <4 x i32>], align 16
  br i1 undef, label %for.body.i.lr.ph, label %for.exit

for.exit:                                         ; preds = %for.exit.i, %entry
  %tmp152 = phi <4 x float> [ undef, %entry ], [ %tmp114, %for.exit.i ]
  %tmp159 = fdiv <4 x float> %tmp152, undef
  %tmp5.i = fsub <4 x float> %tmp159, undef
  br i1 undef, label %if.then73.i, label %if.end72.i

if.end72.i:                                       ; preds = %if.then73.i, %for.exit
  %tmp89.i = phi <4 x float> [ %tmp79.i, %if.then73.i ], [ undef, %for.exit ]
  br i1 false, label %if.then265, label %infloop

if.then73.i:                                      ; preds = %for.exit
  %tmp79.i = shufflevector <4 x float> undef, <4 x float> %tmp5.i, <4 x i32> <i32 0, i32 1, i32 6, i32 3>
  br label %if.end72.i

for.body.i.lr.ph:                                 ; preds = %entry
  %0 = bitcast [8 x <4 x i32>]* %temp.i to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %0)
  br label %dummylabel.i

for.exit.i:                                       ; preds = %dummylabel.i
  %1 = load <2 x float> addrspace(2)* undef, align 8
  %2 = extractelement <2 x float> %1, i32 0
  %3 = insertelement <4 x float> undef, float %2, i32 0
  %4 = insertelement <4 x float> %3, float undef, i32 1
  %5 = insertelement <4 x float> %4, float undef, i32 2
  %6 = insertelement <4 x float> %5, float undef, i32 3
  %7 = fadd <4 x float> undef, %6
  %8 = fadd <4 x float> %7, undef
  %9 = bitcast <4 x float> %8 to <4 x i32>
  %ret.i34 = select <4 x i1> undef, <4 x i32> undef, <4 x i32> %9
  %ret.i32 = select <4 x i1> undef, <4 x i32> undef, <4 x i32> %ret.i34
  %ret.i30 = select <4 x i1> undef, <4 x i32> <i32 2143289344, i32 2143289344, i32 2143289344, i32 2143289344>, <4 x i32> %ret.i32
  %ret.i28 = select <4 x i1> undef, <4 x i32> <i32 -8388608, i32 -8388608, i32 -8388608, i32 -8388608>, <4 x i32> %ret.i30
  %10 = bitcast <4 x i32> %ret.i28 to <4 x float>
  %tmp261.i = fmul <4 x float> %10, <float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00>
  %11 = bitcast [8 x <4 x i32>]* %temp.i to i8*
  %tmp281.i = fmul <4 x float> %tmp261.i, undef
  call void @llvm.lifetime.end(i64 -1, i8* %11)
  %tmp90 = fmul <4 x float> undef, %tmp281.i
  %tmp91 = fadd <4 x float> undef, %tmp90
  %12 = bitcast <4 x float> %tmp91 to <4 x i32>
  %ret.i3 = select <4 x i1> undef, <4 x i32> %12, <4 x i32> undef
  %13 = bitcast <4 x i32> %ret.i3 to <4 x float>
  %tmp108 = fmul <4 x float> undef, %13
  %tmp114 = fadd <4 x float> %tmp108, undef
  br label %for.exit

dummylabel.i:                                     ; preds = %dummylabel.i, %for.body.i.lr.ph
  br i1 undef, label %dummylabel.i, label %for.exit.i

if.then265:                                       ; preds = %if.end72.i
  ret void

infloop:                                          ; preds = %infloop, %if.end72.i
  br label %infloop
}
