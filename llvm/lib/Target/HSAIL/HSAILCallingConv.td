//=- HSAILCallingConv.td - Calling Conventions HSAIL 32/64 ---*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the HSAIL-32 and HSAIL-64
// architectures.
//
//===----------------------------------------------------------------------===//

/// CCIfSubtarget - Match if the current subtarget has a feature F.
class CCIfSubtarget<string F, CCAction A>
 : CCIf<!strconcat("State.getTarget().getSubtarget<HSAILSubtarget>().", F), A>;

//===----------------------------------------------------------------------===//
// HSAIL Return Value Calling Conventions
//===----------------------------------------------------------------------===//

// Return-value conventions common to all HSAIL CC's.
def RetCC_HSAIL_Common : CallingConv<[
  CCIfType<[i1], CCAssignToReg<[include "HSAILCR.td"]>>,
  CCIfType<[i32, f32], CCAssignToReg<[include "HSAILRETREG.td"]>>,
  CCIfType<[i64, f64], CCAssignToReg<[include "HSAILRETREG.td"]>>
]>;

// This is the root return-value convention for the HSAIL-32 backend.
def RetCC_HSAIL_32 : CallingConv<[
  CCDelegateTo<RetCC_HSAIL_Common>
]>;

// This is the root return-value convention for the HSAIL-32 backend.
def RetCC_HSAIL_64 : CallingConv<[
  CCDelegateTo<RetCC_HSAIL_Common>
]>;

// This is the return-value convention used for the entire HSAIL backend.
def RetCC_HSAIL : CallingConv<[
  CCIfSubtarget<"is64Bit()", CCDelegateTo<RetCC_HSAIL_64>>,
  CCDelegateTo<RetCC_HSAIL_32>
]>;


//===----------------------------------------------------------------------===//
// HSAIL Calling Convention
//===----------------------------------------------------------------------===//

def CC_HSAIL_Common : CallingConv<[
  // Integer/Float values get stored in stack slots that are 4 bytes in
  // size and 4-byte aligned.
  CCIfType<[i32, f32], CCAssignToReg<[include "HSAILPARAMREG.td"]>>,	

  // Doubles get 8-byte slots that are 8-byte aligned.
  CCIfType<[i64, f64], CCAssignToReg<[include "HSAILPARAMREG.td"]>>

]>;

// This is the root argument convention for the HSAIL-32 backend.
def CC_HSAIL_32 : CallingConv<[
  CCDelegateTo<CC_HSAIL_Common>
]>;

// This is the root argument convention for the HSAIL-64 backend.
def CC_HSAIL_64 : CallingConv<[
  CCDelegateTo<CC_HSAIL_Common>
]>;

// This is the argument convention used for the entire HSAIL backend.
def CC_HSAIL : CallingConv<[
  CCIfSubtarget<"is64Bit()", CCDelegateTo<CC_HSAIL_64>>,
  CCDelegateTo<CC_HSAIL_32>
]>;
