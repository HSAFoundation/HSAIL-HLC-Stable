; RUN llc < %s -march=amdil | FileCheck %s

define i64 @t1(i64 %x, i64 %y) {
; CHECK: i64shl r{{[0-9]+}}.xy__, r{{[0-9]+}}.xyxy, r{{[0-9]+}}.x
	%z = shl i64 %x, %y
; CHECK: ret_dyn
	ret i64 %z
}

define i64 @t2(i64 %x) {
; CHECK: i64shl r{{[0-9]+}}.xy__, r{{[0-9]+}}.xyxy, l{{[0-9]+}}
	%z = shl i64 %x, 3
; CHECK: ret_dyn
	ret i64 %z
}

define i64 @t3(i64 %x) {
; CHECK: mov [[REGISTER:r[0-9]+]].xy__, l{{[0-9]+}}
; CHECK: i64shl r{{[0-9]+}}.xy__, [[REGISTER]].xyxy, r{{[0-9]+}}.x
	%z = shl i64 3, %x
; CHECK: ret_dyn
	ret i64 %z
}
