; RUN: llc -asm-verbose=0 -march=amdil -mcpu=tahiti < %s | FileCheck %s

declare { i64, i1 } @llvm.uadd.with.overflow.i64(i64, i64) nounwind readnone
declare { i64, i1 } @llvm.sadd.with.overflow.i64(i64, i64) nounwind readnone
declare { i64, i1 } @llvm.usub.with.overflow.i64(i64, i64) nounwind readnone
declare { i64, i1 } @llvm.ssub.with.overflow.i64(i64, i64) nounwind readnone

; CHECK-LABEL: @uaddo64
; CHECK: i64add
define i64 @uaddo64(i64 %a, i64 %b) {
  %c = call { i64, i1 } @llvm.uadd.with.overflow.i64(i64 %b, i64 undef) nounwind
  %d = extractvalue { i64, i1 } %c, 1
  %e = zext i1 %d to i64
  %ret = add i64 %e, %a
  ret i64 %ret
}

; CHECK-LABEL: @saddo64
; CHECK: i64add
define i64 @saddo64(i64 %a, i64 %b) {
  %c = call { i64, i1 } @llvm.sadd.with.overflow.i64(i64 %b, i64 %a) nounwind
  %d = extractvalue { i64, i1 } %c, 1
  %e = zext i1 %d to i64
  %ret = add i64 %e, %a
  ret i64 %ret
}

; CHECK-LABEL: @ssubo64
; CHECK: iadd
define i64 @ssubo64(i64 %a, i64 %b) {
  %c = call { i64, i1 } @llvm.ssub.with.overflow.i64(i64 %b, i64 %a)
  %d = extractvalue { i64, i1 } %c, 1
  %e = zext i1 %d to i64
  %ret = sub i64 %e, %a
  ret i64 %ret
}

; CHECK-LABEL: @usubo64
; CHECK: iadd
define i64 @usubo64(i64 %a, i64 %b) {
  %c = call { i64, i1 } @llvm.usub.with.overflow.i64(i64 %b, i64 %a)
  %d = extractvalue { i64, i1 } %c, 1
  %e = zext i1 %d to i64
  %ret = sub i64 %e, %a
  ret i64 %ret
}
