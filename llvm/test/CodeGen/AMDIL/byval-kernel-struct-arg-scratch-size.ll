; RUN: llc -O0 -march=amdil -mcpu=barts < %s | FileCheck -check-prefix=BARTS-DEBUG %s
; RUN: llc -march=amdil -mcpu=barts < %s | FileCheck -check-prefix=BARTS %s

%struct.__CircularWSDequeSO = type { i32, i32, %struct.__CircularArraySO } ; size = 28
%struct.__CircularArraySO = type { i32, %struct.__Task } ; size = 20
%struct.__Task = type { i32, %union.__TaskParamsSO, i32 } ; size = 16
%union.__TaskParamsSO = type { %struct.__IntPairSO } ; size = 8
%struct.__IntPairSO = type { i32, i32 } ; size = 8

@sgv399 = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv400 = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv401 = internal constant [0 x i8*] zeroinitializer
@rvgv402 = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (%struct.__CircularWSDequeSO*, %struct.__CircularArraySO addrspace(1)*)* @__OpenCL_CircularWSDequeSOCopyElement_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv399 to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv400 to i8*), i8* bitcast ([0 x i8*]* @lvgv401 to i8*), i8* bitcast ([0 x i8*]* @rvgv402 to i8*), i32 0 }]

declare <4 x i32> @__amdil_get_global_id_int() nounwind readnone

; BARTS-DEBUG: dcl_indexed_temp_array x1[6]
; BARTS-DEBUG-NEXT: dcl_literal l{{[0-9]+}}, 96, 32, 16, 64

; BARTS: dcl_indexed_temp_array x1[2]
; BARTS-NEXT: dcl_literal l{{[0-9]+}}, 32, 32, 16, 0

define void @__OpenCL_CircularWSDequeSOCopyElement_kernel(%struct.__CircularWSDequeSO* byval %a, %struct.__CircularArraySO addrspace(1)* %out) nounwind {
entry:
  %0 = call <4 x i32> @__amdil_get_global_id_int() nounwind
  %1 = extractelement <4 x i32> %0, i32 0
  %arrayidx = getelementptr %struct.__CircularArraySO addrspace(1)* %out, i32 %1
  %structele = getelementptr inbounds %struct.__CircularWSDequeSO* %a, i32 0, i32 2
  %tmp2 = load volatile %struct.__CircularArraySO* %structele, align 4
  store %struct.__CircularArraySO %tmp2, %struct.__CircularArraySO addrspace(1)* %arrayidx, align 4
  br label %return

return:
  ret void
}
