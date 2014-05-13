; RUN: llc -march=amdil -mcpu=tahiti < %s | FileCheck %s

;
; [ entry ]
;     |
;     V
; [ block.a ]
;    ^   |
;    |   V
; [ block.b ]
;
; Assertion hit because normalizeInfiniteLoopExit assumed that having
; an unconditional branch meant not having a conditonal branch, but
; the loop block can still have multiple successors with a conditonal branch.

; CHECK-LABEL: .global@cfg_structurizer_inf_loop_assert
; CHECK: whileloop
; CHECK: uav_raw_load_id
; CHECK: whileloop
; CHECK: if_logicalnz
; CHECK: break
; CHECK: endif
; CHECK: endloop
; CHECK: endloop
define void @cfg_structurizer_inf_loop_assert() nounwind {
entry:
  br label %block.a

block.a:                                          ; preds = %block.b, %entry
  %stuff = load i32* undef
  br label %block.b

block.b:                                          ; preds = %block.b, %block.a
  %i = phi i32 [ %tmp15.i, %block.b ], [ %stuff, %block.a ]
  %tmp15.i = lshr i32 %i, 1
  %tobool.i = icmp eq i32 %tmp15.i, 5
  br i1 %tobool.i, label %block.a, label %block.b
}

