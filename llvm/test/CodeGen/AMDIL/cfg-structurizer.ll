; XFAIL: *
; RUN: llc -asm-verbose=0 -march=amdil < %s | FileCheck -check-prefix=CHECK -check-prefix=ALWAYS%s
; RUN: llc -O0 -asm-verbose=0 -march=amdil < %s | FileCheck -check-prefix=DEBUG -check-prefix=ALWAYS %s

declare void @do_something() nounwind
declare void @do_something_else() nounwind
declare void @arst() nounwind
declare void @aoeu() nounwind
declare void @foo() nounwind
declare void @bar() nounwind


; Make sure we don't use end iterators with an empty loop.
; CHECK-LABEL: @self_infinite_loop_empty
; CHECK: whileloop
; CHECK-NEXT: endloop
define void @self_infinite_loop_empty() nounwind {
entry:
  br label %self

self:
  br label %self
}

; CHECK-LABEL: @self_infinite_loop_nonempty
; CHECK: whileloop
; CHECK-NEXT: call
; CHECK-NEXT: endloop
define void @self_infinite_loop_nonempty() nounwind {
entry:
  br label %self

self:
  call void @do_something() nounwind
  br label %self
}

; CHECK-LABEL: @self_break_loop
; CHECK: whileloop
; CHECK: ieq
; CHECK-NEXT: break_logicalnz
; CHECK-NEXT: endloop
; CHECK-NOT: warning
; CHECK-NOT: error
define void @self_break_loop() nounwind {
entry:
  br label %self

exit:
  ret void

self:
  %x = load i32* undef
  %c = icmp eq i32 %x, undef
  br i1 %c, label %exit, label %self
}

; CHECK-LABEL: @self_break_loop_inv
; CHECK: whileloop
; CHECK: ieq
; CHECK-NEXT: break_logicalz
; CHECK-NEXT: endloop
; CHECK-NOT: warning
; CHECK-NOT: error
define void @self_break_loop_inv() nounwind {
entry:
  br label %self

self:
  %x = load i32* undef
  %c = icmp ne i32 %x, undef
  br i1 %c, label %exit, label %self

exit:
  ret void
}

; CHECK-LABEL: @self_break_nested_loop
; CHECK: whileloop
; CHECK-NEXT: whileloop
; CHECK: uav_raw_load_id
; CHECK: uav_raw_store_id
; CHECK: ieq [[CONDREG1:r[0-9]+]].x___,
; CHECK-NEXT: break_logicalnz [[CONDREG1]].x
; CHECK-NEXT: endloop
; CHECK-NEXT: call
; CHECK: uav_raw_load
; CHECK: ieq [[CONDREG2:r[0-9]+]].x___,
; CHECK-NEXT: break_logicalnz [[CONDREG2]].x
; CHECK-NEXT: endloop
define void @self_break_nested_loop() nounwind {
entry:
  br label %self

self:
  %x = load i32* undef
  store i32 123, i32* undef
  %c = icmp eq i32 %x, undef
  br i1 %c, label %exit_self, label %self

exit:
  ret void

exit_self:
  call void @do_something() nounwind
  %y = load i32* undef
  %c1 = icmp eq i32 %y, undef
  br i1 %c1, label %exit, label %self
}

; Make sure merging serial blocks works.
; This should only be a concern for debug builds.
; DEBUG-LABEL: @serial_blocks
; DEBUG-NOT: Pseudo branch instruction
; DEBUG-NOT Pseudo unconditional branch instruction
; DEBUG: ret_dyn
define void @serial_blocks() nounwind {
entry:
  call void @do_something()
  br label %a

a:
  %x = load i32* undef
  %c = icmp eq i32 %x, undef
  %ext = zext i1 %c to i32
  store i32 %ext, i32* undef
  br label %b

b:
  call void @do_something()
  ret void
}

; Make sure we don't leave pseudo branch instructions in debug build
; where there's a conditional branch to the same block.
; DEBUG-LABEL: @cond_branch_same_dest
; DEBUG-NOT: Pseudo branch instruction
; DEBUG-NOT Pseudo unconditional branch instruction
; DEBUG: ret_dyn
define void @cond_branch_same_dest() nounwind {
entry:
  call void @do_something()
  %x = load i32* undef
  %c = icmp eq i32 %x, undef
  %ext = zext i1 %c to i32
  store i32 %ext, i32* undef
  br i1 %c, label %b, label %b

b:
  call void @do_something()
  ret void
}

; CHECK-LABEL: @if_then_else_land
; CHECK: ieq [[COND:r[0-9]+]].x___,
; CHECK-NEXT: if_logicalnz [[COND]].x
; CHECK: call {{[0-9]+}} ; arst
; CHECK-NEXT: else
; CHECK: call {{[0-9]+}} ; do_something_else
; CHECK-NEXT: endif
; CHECK-NEXT: call {{[0-9]+}} ; aoeu
; CHECK: uav_raw_store_id
; CHECK: ret_dyn
; CHECK-NOT: error
; CHECK-NOT: warning

; DEBUG-LABEL: @if_then_else_land
; DEBUG-NOT: error
; DEBUG-NOT: warning
define void @if_then_else_land() nounwind {
entry:
  call void @do_something() nounwind
  %x = load i32* undef
  %c = icmp eq i32 %x, undef
  br i1 %c, label %then, label %else

else:
  call void @do_something_else() nounwind
  br label %land

then:
  call void @arst() nounwind
  br label %land

land:
  call void @aoeu() nounwind
  store i32 999, i32* undef
  ret void
}

; Both branches go to return blocks.
; CHECK-LABEL: @if_then_ret_else_ret
; CHECK: if_logicalnz
; CHECK: ret_dyn
; CHECK-NEXT: endif
; CHECK: ret_dyn
; CHECK-NOT: error
; CHECK-NOT: warning
; DEBUG-LABEL: @if_then_ret_else_ret
; DEBUG-NOT: error
; DEBUG-NOT: warning
define void @if_then_ret_else_ret() nounwind {
entry:
  call void @do_something() nounwind
  %x = load i32* undef
  %c = icmp eq i32 %x, undef
  br i1 %c, label %then, label %else

then:
  call void @do_something_else() nounwind
  ret void

else:
  store i32 12345, i32* undef
  ret void
}

; CHECK-LABEL: @if_not_then
; CHECK: ine
; CHECK-NEXT: if_logicalz
; CHECK-NEXT: call
; CHECK-NEXT: endif
; CHECK: uav_raw_store_id
; CHECK: ret_dyn
; CHECK-NEXT: ret
; CHECK-NOT: error
; CHECK-NOT: warning

; DEBUG-LABEL: @if_not_then
; DEBUG-NOT: error
; DEBUG-NOT: warning
define void @if_not_then() nounwind {
entry:
  call void @do_something() nounwind
  %x = load i32* undef
  %c = icmp eq i32 %x, 9
  br i1 %c, label %then, label %land

then:
  call void @do_something_else() nounwind
  br label %land

land:
  br label %exit

exit:
  store i32 123, i32* undef
  ret void
}

; CHECK-LABEL: @if_then
; CHECK: ieq
; CHECK-NEXT: if_logicalnz
; CHECK-NEXT: call
; CHECK-NEXT: endif
; CHECK: uav_raw_store_id
; CHECK: ret_dyn
; CHECK-NOT: error
; CHECK-NOT: warning

; DEBUG-LABEL: @if_then
; DEBUG: ieq
; DEBUG-NEXT: if_logicalnz
; DEBUG-NEXT: call
; DEBUG-NEXT: endif
; DEBUG: uav_raw_store_id
; DEBUG: ret_dyn
; DEBUG-NOT: error
; DEBUG-NOT: warning
define void @if_then() nounwind {
entry:
  call void @do_something() nounwind
  %x = load i32* undef
  %c = icmp eq i32 %x, undef
  br i1 %c, label %then, label %exit

exit:
  store i32 123, i32* undef
  ret void

then:
  call void @do_something_else() nounwind
  br label %exit
}

; CHECK-LABEL: @if_then_ret
; CHECK: ieq
; CHECK: if_logicalnz
; CHECK: call
; CHECK-NEXT: ret_dyn
; CHECK-NEXT: endif
; CHECK-NEXT: ret_dyn
; CHECK-NOT: error
; CHECK-NOT: warning
define void @if_then_ret() nounwind {
entry:
  call void @do_something() nounwind
  %x = load i32* undef
  %c = icmp eq i32 %x, undef
  br i1 %c, label %then, label %else

else:
  ret void

then:
  call void @do_something_else() nounwind
  ret void
}

; CHECK-LABEL: @if_then_else_if_undominated_exit
; CHECK: ieq [[CONDREG1:r[0-9]+]].x___
; CHECK-NEXT: if_logicalnz [[CONDREG1]].x
; CHECK: ieq [[CONDREG2:r[0-9]+]].x___
; CHECK-NEXT: if_logicalnz [[CONDREG2]].x
; CHECK: uav_raw_store_id
; CHECK: else
; CHECK-NEXT: call {{[0-9]+}} ; foo
; CHECK: endif
; CHECK-NEXT: endif
; CHECK-NEXT: call {{[0-9]+}} ; do_something
; CHECK-NEXT: ret_dyn
; CHECK-NOT: error
; CHECK-NOT: warning
define void @if_then_else_if_undominated_exit(i32 %x, i32 %y) nounwind {
entry:
  %cmp1 = icmp eq i32 %x, 0
  br i1 %cmp1, label %if.pred, label %land

land:
  call void @do_something() nounwind
  ret void

if.pred:
  %cmp2 = icmp eq i32 %y, 0
  br i1 %cmp2, label %if.then, label %if.else

if.else:
  tail call void @foo() nounwind
  br label %land

if.then:
  store i32 999, i32* undef
  br label %land
}

; Simple if then else where the landing block loops back to the
; predecessor.
;
; CHECK-LABEL: @if_then_else_land_loop
; CHECK: whileloop
; CHECK: call {{[0-9]+}} ; do_something
; CHECK: uav_raw_load_id
; CHECK: ieq
; CHECK: if_logicalnz
; CHECK: call {{[0-9]+}} ; do_something_else
; CHECK: else
; CHECK: call {{[0-9]+}} ; arst
; CHECK: endif
; CHECK: call {{[0-9]+}} ; aoeu
; CHECK: uav_raw_store_id
; CHECK: endloop
; CHECK-NOT: error
; CHECK-NOT: warning

; DEBUG-LABEL: @if_then_else_land_loop
; DEBUG-NOT: error
; DEBUG-NOT: warning
define void @if_then_else_land_loop() nounwind {
entry:
  br label %pred

pred:
  call void @do_something() nounwind
  %x = load i32* undef
  %c = icmp eq i32 %x, undef
  br i1 %c, label %then, label %else

else:
  call void @arst() nounwind
  br label %land

then:
  call void @do_something_else() nounwind
  br label %land

land:
  call void @aoeu() nounwind
  store i32 999, i32* undef
  br label %pred
}


; CHECK-LABEL: @two_inner_loops
; CHECK: uav_raw_store_id
; CHECK: whileloop

; CHECK: whileloop
; CHECK: uav_raw_load_id
; CHECK: ieq [[COND1:r[0-9]+]].x___,
; CHECK-NEXT: break_logicalz [[COND1]].x
; CHECK-NEXT: endloop

; CHECK: whileloop
; CHECK: uav_raw_store_id
; CHECK: call
; CHECK: uav_raw_load_id
; CHECK: ieq [[COND2:r[0-9]+]].x___,
; CHECK-NEXT: break_logicalnz [[COND2]].x
; CHECK-NEXT: endloop

; CHECK: endloop
; CHECK-NOT: error
; CHECK-NOT: warning
define void @two_inner_loops() nounwind {
entry:
  store i32 123, i32* undef
  br label %a

a:
  %x = load i32* undef
  %cmp1 = icmp eq i32 %x, undef
  br i1 %cmp1, label %a, label %b

b:
  store i32 9999, i32 addrspace(1)* undef
  call void @do_something() nounwind
  %y = load i32 addrspace(1)* undef
  %cmp2 = icmp eq i32 %y, undef
  br i1 %cmp2, label %a, label %b
}

; CHECK-LABEL: @if_then_else_land_loop_backedge_1
; CHECK: whileloop
; CHECK: call {{[0-9]+}} ; do_something
; CHECK: uav_raw_load_id
; CHECK: ieq
; CHECK-NEXT: if_logicalnz
; CHECK-NEXT: call {{[0-9]+}} ; do_something_else
; CHECK-NEXT: else
; CHECK-NEXT: call {{[0-9]+}} ; arst
; CHECK-NEXT: endif
; CHECK: uav_raw_store_id
; CHECK: call {{[0-9]+}} ; arst
; CHECK-NEXT: endloop
; CHECK-NOT: warning
; CHECK-NOT: error
define void @if_then_else_land_loop_backedge_1() nounwind {
entry:
  br label %pred

pred:
  call void @do_something() nounwind
  %x = load i32* undef
  %c = icmp eq i32 %x, undef
  br i1 %c, label %then, label %else

else:
  call void @arst() nounwind
  br label %loop

then:
  call void @do_something_else() nounwind
  br label %loop

loop:
  store i32 123, i32* undef
  call void @arst() nounwind
  br label %pred
}

; Similar to if_then_else_land_loop_backedge, but only one side of the
; if branches back.
; CHECK-LABEL: @if_then_else_land_backedge_pred_true_side
; CHECK: whileloop
; CHECK: call {{[0-9]+}} ; do_something
; CHECK: uav_raw_load_id
; CHECK: ieq [[CONDREG:r[0-9]+]].x___,
; CHECK: if_logicalz [[CONDREG]].x
; CHECK: call {{[0-9]+}} ; arst
; CHECK: uav_raw_store_id
; CHECK: ret_dyn
; CHECK-NEXT: endif
; CHECK-NEXT: call {{[0-9]+}} ; do_something_else
; CHECK-NEXT: endloop
; CHECK-NOT: error
; CHECK-NOT: warning
define void @if_then_else_land_backedge_pred_true_side() nounwind {
entry:
  br label %pred

pred:
  call void @do_something() nounwind
  %x = load i32* undef
  %c = icmp eq i32 %x, undef
  br i1 %c, label %then, label %else

else:
  call void @arst() nounwind
  store i32 123, i32* undef
  ret void

then:
  call void @do_something_else() nounwind
  br label %pred
}

; CHECK-LABEL: @if_then_else_land_backedge_pred_false_side
; CHECK: call {{[0-9]+}} ; do_something
; CHECK: uav_raw_load_id
; CHECK: ieq [[CONDREG:r[0-9]+]].x___,
; CHECK: if_logicalnz [[CONDREG]]
; CHECK: call {{[0-9]+}} ; do_something_else
; CHECK: uav_raw_store_id
; CHECK: ret_dyn
; CHECK-NEXT: endif
; CHECK-NEXT: call {{[0-9]+}} ; arst
; CHECK-NEXT: endloop
; CHECK-NOT: error
; CHECK-NOT: warning

; DEBUG-LABEL: @if_then_else_land_backedge_pred_false_side
; DEBUG-NOT: error
; DEBUG-NOT: warning
define void @if_then_else_land_backedge_pred_false_side() nounwind {
entry:
  br label %pred

pred:
  call void @do_something() nounwind
  %x = load i32* undef
  %c = icmp eq i32 %x, undef
  br i1 %c, label %then, label %else

else:
  call void @arst() nounwind
  br label %pred

then:
  call void @do_something_else() nounwind
  store i32 123, i32* undef
  ret void
}

; CHECK-LABEL: @break_from_loop_pair
; CHECK: ieq [[CONDREG1:r[0-9]+]].x___,
; CHECK-NEXT: if_logicalnz [[CONDREG1]].x
; CHECK-NEXT: whileloop
; CHECK: call {{[0-9]+}} ; do_something
; CHECK: uav_raw_load_id
; CHECK: ine [[CONDREG2:r[0-9]+]].x___,
; CHECK-NEXT: break_logicalz [[CONDREG2]].x
; CHECK-NEXT: call {{[0-9]+}} ; do_something_else
; CHECK-NEXT: endloop
; CHECK-NEXT: else
; CHECK: lds_store_id
; CHECK-NEXT: endif
; CHECK: call {{[0-9]+}} ; arst
; CHECK: uav_raw_store_id
; CHECK: ret_dyn
; CHECK-NOT: warning
; CHECK-NOT: error
define void @break_from_loop_pair(i32 %p) nounwind {
entry:
  %cmp = icmp eq i32 %p, 0
  br i1 %cmp, label %pred, label %arst

arst:
  store i32 9999, i32 addrspace(3)* undef
  br label %then

pred:
  call void @do_something() nounwind
  %x = load i32* undef
  %c = icmp eq i32 %x, undef
  br i1 %c, label %then, label %else

then:
  call void @arst() nounwind
  store i32 123, i32* undef
  ret void

else:
  call void @do_something_else() nounwind
  br label %pred
}

; if-then-else-endif block lands in the predecessor block.
; CHECK-LABEL: @if_then_else_land_loop_backedge_2
; CHECK: whileloop
; CHECK: call {{[0-9]+}} ; do_something
; CHECK: uav_raw_load
; CHECK: ieq [[CONDREG:r[0-9]+]].x___,
; CHECK-NEXT: if_logicalnz [[CONDREG]].x
; CHECK-NEXT: call {{[0-9]+}} ; do_something_else
; CHECK-NEXT: call {{[0-9]+}} ; arst
; CHECK-NEXT: else
; CHECK-NEXT: call {{[0-9]+}} ; arst
; CHECK-NEXT: call {{[0-9]+}} ; arst
; CHECK-NEXT: endif
; CHECK-NEXT: endloop
; CHECK-NOT: error
; CHECK-NOT: warning
; DEBUG-LABEL: @if_then_else_land_loop_backedge_2
; DEBUG-NOT: error
; DEBUG-NOT: warning
define void @if_then_else_land_loop_backedge_2() nounwind {
entry:
  br label %pred

pred:
  call void @do_something() nounwind
  %x = load i32* undef
  %c = icmp eq i32 %x, undef
  br i1 %c, label %then, label %else

else:
  call void @arst() nounwind
  call void @arst() nounwind
  br label %pred

then:
  call void @do_something_else() nounwind
  call void @arst() nounwind
  br label %pred
}

; CHECK-LABEL: @if_then_else_land_backedge_pred
; CHECK: whileloop
; CHECK: call {{[0-9]+}} ; do_something
; CHECK: uav_raw_load_id
; CHECK: ieq [[CONDREG:r[0-9]+]].x___,
; CHECK-NEXT: if_logicalnz [[CONDREG]].x
; CHECK-NEXT: call {{[0-9]+}} ; do_something_else
; CHECK-NEXT: else
; CHECK-NEXT: call {{[0-9]+}} ; arst
; CHECK-NEXT: endif
; CHECK-NEXT: endloop
; CHECK-NOT: error
; CHECK-NOT: warning
define void @if_then_else_land_backedge_pred() nounwind {
entry:
  br label %pred

pred:
  call void @do_something() nounwind
  %x = load i32* undef
  %c = icmp eq i32 %x, undef
  br i1 %c, label %then, label %else

else:
  call void @arst() nounwind
  br label %pred

then:
  call void @do_something_else() nounwind
  br label %pred
}

; Apparent if other dest branches back to not immediate dominator.
; CHECK-LABEL: @backedge_if_non_idom
; CHECK: ieq [[CONDREG1:r[0-9]+]].x___,
; CHECK-NEXT: whileloop
; CHECK: uav_raw_store_id
; CHECK: if_logicalz [[CONDREG1]].x
; CHECK-NEXT: call {{[0-9]+}} ; do_something
; CHECK-NEXT: endif
; CHECK: uav_raw_load_id
; CHECK: ieq [[CONDREG2:r[0-9]+]].x___,
; CHECK-NEXT: if_logicalz [[CONDREG2]]
; CHECK-NEXT: ret_dyn
; CHECK-NEXT: endif
; CHECK-NEXT: call {{[0-9]+}} ; do_something_else
; CHECK-NEXT: endloop
; CHECK-NOT: warning
; CHECK-NOT: label
define void @backedge_if_non_idom(i32 %q) nounwind {
entry:
  %qcmp = icmp ne i32 %q, undef
  br label %a

arst:
  call void @do_something_else() nounwind
  br label %a

a:
  store i32 222, i32* undef
  br i1 %qcmp, label %b, label %c

b:
  call void @do_something() nounwind
  br label %c

c:
  %x = load i32* undef
  %cmp = icmp eq i32 %x, undef
  br i1 %cmp, label %arst, label %e

e:
  ret void
}

; CHECK-LABEL: @func1
; CHECK: ine [[CONDREG1:r[0-9]+]].x___,
; CHECK: whileloop
; CHECK: if_logicalz [[CONDREG1]].x
; CHECK-NEXT: call
; CHECK-NEXT: endif
; CHECK: uav_raw_load_id
; CHECK: ieq [[CONDREG2:r[0-9]+]].x___,
; CHECK-NEXT: break_logicalz [[CONDREG2]]
; CHECK-NEXT: endloop
; CHECK-NEXT: ret_dyn
; CHECK-NOT: error
; CHECK-NOT: warning
define void @func1(i32 %q) nounwind {
entry:
  %qcmp = icmp eq i32 %q, undef
  br label %a

a:
  store i32 222, i32* undef
  br i1 %qcmp, label %b, label %c

b:
  call void @do_something()
  br label %c

c:
  %x = load i32* undef
  %cmp = icmp eq i32 %x, undef
  br i1 %cmp, label %a, label %e

e:
  ret void
}

; CHECK-LABEL: @func2
; CHECK-NOT: error
; CHECK-NOT: warning
define void @func2(i1 %q, i1 %r) nounwind {
entry:
  br label %a

a:
  store i32 222, i32* undef
  br i1 %q, label %b, label %c

b:
  call void @do_something()
  br label %c

c:
  %x = load i32* undef
  %cmp = icmp eq i32 %x, undef
  br i1 %cmp, label %a, label %e

d:
 call void @do_something_else()
 br i1 %r, label %b, label %e

e:
  ret void
}

; CHECK-LABEL: @if_tree
; CHECK: uav_raw_load_id
; CHECK: ieq [[CONDREG1:r[0-9]+]].x___,
; CHECK-NEXT: if_logicalnz [[CONDREG1]]
; CHECK: uav_raw_load_id
; CHECK: ieq [[CONDREG2:r[0-9]+]].x___,
; CHECK-NEXT: if_logicalnz [[CONDREG2]]
; CHECK-NEXT: call {{[0-9]+}} ; arst
; CHECK-NEXT: ret_dyn
; CHECK-NEXT: endif
; CHECK-NEXT: call {{[0-9]+}} ; aoeu
; CHECK-NEXT: ret_dyn
; CHECK-NEXT: endif
; CHECK-NEXT: uav_raw_load_id
; CHECK: ieq [[CONDREG3:r[0-9]+]].x___,
; CHECK-NEXT: if_logicalnz [[CONDREG3]]
; CHECK-NEXT: call {{[0-9]+}} ; do_something
; CHECK-NEXT: ret_dyn
; CHECK-NEXT: endif
; CHECK-NEXT: call {{[0-9]+}} ; do_something_else
; CHECK-NEXT: ret_dyn
; CHECK-NOT: error
; CHECK-NOT: warning

; DEBUG-LABEL: @if_tree
; DEBUG-NOT: error
; DEBUG-NOT: warning
define void @if_tree() nounwind {
entry:
  %x = load i32* undef
  %cmp1 = icmp eq i32 %x, undef
  br i1 %cmp1, label %a, label %b

b:
  %z = load i32* undef
  %cmp3 = icmp eq i32 %z, undef
  br i1 %cmp3, label %e, label %f

a:
  %y = load i32* undef
  %cmp2 = icmp eq i32 %y, undef
  br i1 %cmp2, label %c, label %d

d:
  call void @aoeu() nounwind
  ret void

c:
  call void @arst() nounwind
  ret void

f:
  call void @do_something_else() nounwind
  ret void

e:
  call void @do_something() nounwind
  ret void
}


; CHECK-LABEL: @if_tree_converge
; CHECK: uav_raw_load_id
; CHECK: ieq [[CONDREG1:r[0-9]+]].x___,
; CHECK-NEXT: if_logicalnz [[CONDREG1]]
; CHECK: uav_raw_load_id
; CHECK: ieq [[CONDREG2:r[0-9]+]].x___,
; CHECK-NEXT: if_logicalnz [[CONDREG2]]
; CHECK-NEXT: call {{[0-9]+}} ; arst
; CHECK-NEXT: else
; CHECK-NEXT: call {{[0-9]+}} ; aoeu
; CHECK-NEXT: endif
; CHECK: lds_store_id
; CHECK: else
; CHECK: uav_raw_load_id
; CHECK: ieq [[CONDREG3:r[0-9]+]].x___,
; CHECK-NEXT: if_logicalnz [[CONDREG2]]
; CHECK-NEXT: call {{[0-9]+}} ; do_something
; CHECK-NEXT: else
; CHECK-NEXT: call {{[0-9]+}} ; do_something_else
; CHECK-NEXT: endif
; CHECK: uav_raw_store_id(11)
; CHECK-NEXT: endif
; CHECK: uav_raw_store_id(8)
; CHECK: ret_dyn
; CHECK-NOT: error
; CHECK-NOT: warning

; DEBUG-LABEL: @if_tree_converge
; DEBUG-NOT: error
; DEBUG-NOT: warning
define void @if_tree_converge() nounwind {
entry:
  %x = load i32* undef
  %cmp1 = icmp eq i32 %x, undef
  br i1 %cmp1, label %a, label %b

b:
  %z = load i32* undef
  %cmp3 = icmp eq i32 %z, undef
  br i1 %cmp3, label %e, label %f

a:
  %y = load i32* undef
  %cmp2 = icmp eq i32 %y, undef
  br i1 %cmp2, label %c, label %d

d:
  call void @aoeu() nounwind
  br label %g

c:
  call void @arst() nounwind
  br label %g

e:
  call void @do_something() nounwind
  br label %h

f:
  call void @do_something_else() nounwind
  br label %h

h:
  store i32 9999, i32 addrspace(1)* undef
  br label %exit

g:
  store i32 1213, i32 addrspace(3)* undef
  br label %exit

exit:
  store i32 123, i32* undef
  ret void
}



; DEBUG-LABEL: @if_then_literal
; DEBUG: dcl_literal [[CONDLIT:l[0-9]+]], 0xFFFFFFFF
; DEBUG: if_logicalnz [[CONDLIT]]
; DEBUG: call
; DEBUG: endif
; DEBUG-NOT: error
; DEBUG-NOT: warning
define void @if_then_literal() nounwind {
entry:
  call void @do_something() nounwind
  %x = load i32* undef
  br i1 true, label %then, label %exit

exit:
  store i32 123, i32* undef
  ret void

then:
  call void @do_something_else() nounwind
  br label %exit
}

; DEBUG-LABEL: @if_then_literal_inv
; DEBUG: dcl_literal [[CONDLIT:l[0-9]+]], 0xFFFFFFFF,
; DEBUG: if_logicalz [[CONDLIT]]
; DEBUG: call
; DEBUG: endif
; DEBUG-NOT: error
; DEBUG-NOT: warning
define void @if_then_literal_inv() nounwind {
entry:
  call void @do_something() nounwind
  %x = load i32* undef
  br i1 false, label %then, label %exit

then:
  call void @do_something_else() nounwind
  br label %exit

exit:
  store i32 123, i32* undef
  ret void
}

; DEBUG-LABEL: @self_break_loop_literal
; DEBUG: dcl_literal [[CONDLIT:l[0-9]+]], 0xFFFFFFFF,
; DEBUG: whileloop
; DEBUG: uav_raw_store_id
; DEBUG: break_logicalnz [[CONDLIT]]
; DEBUG-NEXT: endloop
; DEBUG-NOT: warning
; DEBUG-NOT: error
define void @self_break_loop_literal() nounwind {
entry:
  br label %self

exit:
  ret void

self:
  store i32 999, i32* undef
  br i1 true, label %exit, label %self
}

; DEBUG-LABEL: @self_break_loop_literal_inv
; DEBUG: dcl_literal [[CONDLIT:l[0-9]+]], 0xFFFFFFFF,
; DEBUG: whileloop
; DEBUG: uav_raw_store_id
; DEBUG: break_logicalz [[CONDLIT]]
; DEBUG-NEXT: endloop
; DEBUG-NOT: warning
; DEBUG-NOT: error
define void @self_break_loop_literal_inv() nounwind {
entry:
  br label %self

self:
  store i32 999, i32* undef
  br i1 false, label %exit, label %self

exit:
  ret void
}


; CHECK-LABEL: @if_ret_else_if_ret
; CHECK: ieq [[CONDREG1:r[0-9]+]].x___,
; CHECK: if_logicalnz [[CONDREG1]].x
; CHECK: uav_raw_store_id
; CHECK: ieq [[CONDREG2:r[0-9]+]].x___,
; CHECK: if_logicalnz [[CONDREG2]].x
; CHECK-NEXT: call
; CHECK-NEXT: endif
; CHECK-NEXT: endif
; CHECK-NEXT: ret_dyn

; DEBUG-LABEL: @if_ret_else_if_ret
; DEBUG-NOT: warning
; DEBUG-NOT: error
define void @if_ret_else_if_ret(i32 %x, i32 %y) nounwind {
entry:
  %cmp1 = icmp eq i32 %x, 0
  br i1 %cmp1, label %if.end, label %return

return:
  ret void

if.else:
  tail call void @foo() nounwind
  br label %return

if.end:
  store i32 999, i32* undef
  %cmp2 = icmp eq i32 %y, 0
  br i1 %cmp2, label %if.else, label %return
}

; CHECK-LABEL: @if_ret_else_if_ret_inv
; CHECK: ieq [[CONDREG1:r[0-9]+]].x___,
; CHECK: if_logicalnz [[CONDREG1]].x
; CHECK: uav_raw_store_id
; CHECK: ieq [[CONDREG2:r[0-9]+]].x___,
; CHECK: if_logicalz [[CONDREG2]].x
; CHECK-NEXT: call
; CHECK-NEXT: endif
; CHECK-NEXT: endif
; CHECK-NEXT: ret_dyn

; DEBUG-LABEL: @if_ret_else_if_ret_inv
; DEBUG-NOT: warning
; DEBUG-NOT: error
define void @if_ret_else_if_ret_inv(i32 %x, i32 %y) nounwind {
entry:
  %cmp1 = icmp eq i32 %x, 0
  br i1 %cmp1, label %if.end, label %return

return:
  ret void

if.end:
  store i32 999, i32* undef
  %cmp2 = icmp eq i32 %y, 0
  br i1 %cmp2, label %return, label %if.else

if.else:
  tail call void @foo() nounwind
  br label %return
}

; XXX - Old structurizer inserts a continue instead of nesting
; loops. Would that be better? This looks closer to what actually
; needs to occur.

; Test problem where endloop was treated as a real terminator inst,
; resulting in thinking a block was more confusing than it really is.
; CHECK-LABEL: @real_extra_terminator
; CHECK: whileloop
; CHECK-NEXT: whileloop
; CHECK: call
; CHECK: uav_raw_load_id(8)
; CHECK: ieq [[CONDREG1:r[0-9]+]].x___
; CHECK-NEXT: break_logicalz [[CONDREG1]].x
; CHECK-NEXT: endloop
; CHECK: lds_load_id
; CHECK: ieq [[CONDREG2:r[0-9]+]].x___
; CHECK-NEXT: if_logicalnz [[CONDREG2]].x
; CHECK-NEXT: uav_raw_load_id(11)
; CHECK-NEXT: else
; CHECK-NEXT: uav_raw_load_id(8)
; CHECK-NEXT: endif
; CHECK: call
; CHECK: ine [[CONDREG3:r[0-9]+]].x___
; CHECK-NEXT: break_logicalz [[CONDREG3]].x
; CHECK-NEXT: endloop
; CHECK-NEXT: ret_dyn
; CHECK-NOT: warning
; CHECK-NOT: error

; DEBUG-LABEL: @real_extra_terminator
; DEBUG-NOT: warning
; DEBUG-NOT: error
define void @real_extra_terminator() nounwind {
entry:
  br label %for.body

for.body:
  call void @bar() nounwind
  %x = load i32* undef
  %cmp1 = icmp eq i32 %x, -1
  br i1 %cmp1, label %for.body, label %for.break

for.break:
  %y = load i32 addrspace(3)* undef
  %cmp2 = icmp eq i32 %y, 0
  br i1 %cmp2, label %cond.then, label %cond.else

cond.else:
  %loadprivate = load i32* undef, align 4
  br label %cond.end

cond.then:
  %loadglobal = load i32 addrspace(1)* undef, align 4
  br label %cond.end

cond.end:
  %cond212 = phi i32 [ %loadglobal, %cond.then ], [ %loadprivate, %cond.else ]
  call void @arst() nounwind
  %cmp3 = icmp eq i32 %cond212, 0
  br i1 %cmp3, label %exit, label %for.body

exit:
  ret void
}

; CHECK-LABEL: @two_breaks_from_loop_pair_same_dest
; CHECK: whileloop
; CHECK: call {{[0-9]+}} ; do_something
; CHECK: uav_raw_load_id(8)
; CHECK: ieq [[CONDREG1:r[0-9]+]].x___
; CHECK-NEXT: break_logicalnz [[CONDREG1]]
; CHECK: call {{[0-9]+}} ; arst
; CHECK: uav_raw_store_id(8)
; CHECK: ieq [[CONDREG2:r[0-9]+]].x___
; CHECK-NEXT: break_logicalz [[CONDREG2]]
; CHECK-NEXT: endloop
; CHECK: call {{[0-9]+}} ; foo
; CHECK-NEXT: ret_dyn
; CHECK: endfunc
; CHECK-NOT: error
; CHECK-NOT: warning
define void @two_breaks_from_loop_pair_same_dest(i32* %p) nounwind {
entry:
  br label %pred

pred:
  call void @do_something() nounwind
  %x = load i32* undef
  %cmp1 = icmp eq i32 %x, 0
  br i1 %cmp1, label %break, label %mbb

mbb:
  call void @arst() nounwind
  store i32 123, i32* undef
  %y = load i32* %p
  %cmp2 = icmp eq i32 %y, 0
  br i1 %cmp2, label %pred, label %break

break:
  call void @foo() nounwind
  br label %exit

exit:
  ret void
}

; CHECK-LABEL: @two_breaks_from_loop_pair_same_dest_inv
; CHECK: whileloop
; CHECK: call {{[0-9]+}} ; do_something
; CHECK: uav_raw_load_id(8)
; CHECK: ieq [[CONDREG1:r[0-9]+]].x___
; CHECK-NEXT: break_logicalz [[CONDREG1]]
; CHECK: call {{[0-9]+}} ; arst
; CHECK: uav_raw_store_id(8)
; CHECK: ieq [[CONDREG2:r[0-9]+]].x___
; CHECK-NEXT: break_logicalnz [[CONDREG2]]
; CHECK-NEXT: endloop
; CHECK: call {{[0-9]+}} ; foo
; CHECK-NEXT: ret_dyn
; CHECK: endfunc
; CHECK-NOT: error
; CHECK-NOT: warning
define void @two_breaks_from_loop_pair_same_dest_inv(i32* %p) nounwind {
entry:
  br label %pred

pred:
  call void @do_something() nounwind
  %x = load i32* undef
  %cmp1 = icmp eq i32 %x, 0
  br i1 %cmp1, label %mbb, label %break

break:
  call void @foo() nounwind
  br label %exit

mbb:
  call void @arst() nounwind
  store i32 123, i32* undef
  %y = load i32* %p
  %cmp2 = icmp eq i32 %y, 0
  br i1 %cmp2, label %break, label %pred

exit:
  ret void
}

; CHECK-LABEL: @two_breaks_from_loop_pair
; CHECK: warning
; CHECK: error
define void @two_breaks_from_loop_pair(i32 %p) nounwind {
entry:
  br label %pred

pred:
  call void @do_something() nounwind
  %x = load i32* undef
  %cmp1 = icmp eq i32 %x, undef
  br i1 %cmp1, label %mbb, label %break.a

mbb:
  call void @arst() nounwind
  store i32 123, i32* undef
  %y = load i32* undef
  %cmp2 = icmp eq i32 %x, undef
  br i1 %cmp2, label %pred, label %break.b

break.a:
  call void @do_something_else() nounwind
  br label %exit

break.b:
  call void @arst() nounwind
  br label %exit

exit:
  ret void
}

; CHECK-LABEL: @if_common_then_block
; CHECK: warning
; CHECK: error

; DEBUG-LABEL: @if_common_then_block
; DEBUG: warning
; DEBUG: error
define void @if_common_then_block() nounwind {
entry:
  %x = load i32* undef
  %cmp1 = icmp eq i32 %x, undef
  br i1 %cmp1, label %a, label %b

b:
  %z = load i32* undef
  %cmp3 = icmp eq i32 %z, undef
  br i1 %cmp3, label %c, label %d

a:
  %y = load i32* undef
  %cmp2 = icmp eq i32 %y, undef
  br i1 %cmp2, label %c, label %e

e:
  call void @arst() nounwind
  ret void

c:
  store i32 111, i32* undef
  call void @do_something() nounwind
  br label %d

d:
  call void @aoeu() nounwind
  ret void
}

; CHECK-LABEL: @if_common_then_else_block
; CHECK: warning
; CHECK: error

; DEBUG-LABEL: @if_common_then_else_block
; DEBUG: warning
; DEBUG: error
define void @if_common_then_else_blocks() nounwind {
entry:
  %x = load i32* undef
  %cmp1 = icmp eq i32 %x, undef
  br i1 %cmp1, label %a, label %b

a:
  call void @arst() nounwind
  %y = load i32* undef
  %cmp2 = icmp eq i32 %y, undef
  br i1 %cmp2, label %e, label %c

b:
  call void @aoeu() nounwind
  %z = load i32* undef
  %cmp3 = icmp eq i32 %z, undef
  br i1 %cmp3, label %c, label %e

c:
  store i32 111, i32* undef
  call void @do_something() nounwind
  br label %e

e:
  store i32 999, i32 addrspace(3)* undef
  ret void
}

