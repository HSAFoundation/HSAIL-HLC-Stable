//===- AMDILCallingConv.td - Calling Conventions AMDIL -----*- tablegen -*-===//
// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// If you use the software (in whole or in part), you shall adhere to all
// applicable U.S., European, and other export laws, including but not limited
// to the U.S. Export Administration Regulations (“EAR”), (15 C.F.R. Sections
// 730 through 774), and E.U. Council Regulation (EC) No 1334/2000 of 22 June
// 2000.  Further, pursuant to Section 740.6 of the EAR, you hereby certify
// that, except pursuant to a license granted by the United States Department
// of Commerce Bureau of Industry and Security or as otherwise permitted
// pursuant to a License Exception under the U.S. Export Administration
// Regulations ("EAR"), you will not (1) export, re-export or release to a
// national of a country in Country Groups D:1, E:1 or E:2 any restricted
// technology, software, or source code you receive hereunder, or (2) export to
// Country Groups D:1, E:1 or E:2 the direct product of such technology or
// software, if such foreign produced direct product is subject to national
// security controls as identified on the Commerce Control List (currently
// found in Supplement 1 to Part 774 of EAR).  For the most current Country
// Group listings, or for additional information about the EAR or your
// obligations under those regulations, please refer to the U.S. Bureau of
// Industry and Security’s website at http://www.bis.doc.gov/.
//
//==--------------------------------------------------------------===//
//
// This describes the calling conventions for the AMDIL architectures.
//
//===-------------------------------------------------------------===//

//===-------------------------------------------------------------===//
// Return Value Calling Conventions
//===-------------------------------------------------------------===//

// AMDIL 32-bit C return-value convention.
def RetCC_AMDIL32 : CallingConv<[
  // Since IL has no return values, all values can be emulated on the
  // stack The stack can then be mapped to a number of sequential
  // virtual registers in IL
  CCIfType<[f32], CCBitConvertToType<i32>>,
  CCIfType<[i64, f64, v2f32], CCBitConvertToType<v2i32>>,
  CCIfType<[v2i64, v4f32, v2f64], CCBitConvertToType<v4i32>>,

  CCIfType<[i1, i8, i16, i32], CCAssignToReg<
    [OUTx0, OUTy0, OUTz0, OUTw0, OUTx1, OUTy1, OUTz1, OUTw1,
     OUTx2, OUTy2, OUTz2, OUTw2, OUTx3, OUTy3, OUTz3, OUTw3,
     OUTx4, OUTy4, OUTz4, OUTw4, OUTx5, OUTy5, OUTz5, OUTw5,
     OUTx6, OUTy6, OUTz6, OUTw6, OUTx7, OUTy7, OUTz7, OUTw7]>>,

  CCIfType<[i1, i8, i16, i32], CCAssignToStack<4, 4>>,

  CCIfType<[v2i1, v2i8, v2i16, v2i32], CCAssignToReg<
    [OUTxy0, OUTzw0, OUTxy1, OUTzw1, OUTxy2, OUTzw2, OUTxy3, OUTzw3,
     OUTxy4, OUTzw4, OUTxy5, OUTzw5, OUTxy6, OUTzw6, OUTxy7, OUTzw7]>>,

  CCIfType<[v2i1, v2i8, v2i16, v2i32], CCAssignToStack<8, 4>>,
  CCIfType<[v8i8], CCPromoteToType<v8i32>>,
  CCIfType<[v4i1, v4i8, v4i16, v4i32],
    CCAssignToReg<[OUT0, OUT1, OUT2, OUT3, OUT4, OUT5, OUT6, OUT7]>>,

  CCAssignToStack<16, 4>
]>;

// AMDIL 32-bit C Calling convention.
def CC_AMDIL32 : CallingConv<[
 // CCIfByVal<CCPassByVal<4, 4>>,
 // Since IL has parameter values, all values can be emulated on the
 // stack The stack can then be mapped to a number of sequential
 // virtual registers in IL.
  CCIfType<[f32], CCBitConvertToType<i32>>,
  CCIfType<[i64, f64, v2f32], CCBitConvertToType<v2i32>>,
  CCIfType<[v2i64, v4f32, v2f64], CCBitConvertToType<v4i32>>,
//  CCIfType<[f16], CCBitConvertToType<i16>>,

  CCIfType<[i1, i8, i16, i32], CCAssignToReg<
    [ include "AMDILRegisterUsesINScalar.td" ]>>,

  CCIfType<[i32], CCAssignToStack<4, 4>>,

  CCIfType<[v2i1, v2i8, v2i16, v2i32], CCAssignToReg<
    [ include "AMDILRegisterUsesINV2.td" ]>>,

  CCIfType<[v2i32], CCAssignToStack<8, 4>>,
  CCIfType<[v8i8], CCPromoteToType<v8i32>>,

  CCIfType<[v4i1, v4i8, v4i16, v4i32], CCAssignToReg<
    [ include "AMDILRegisterUsesINV4.td" ]>>,

  CCAssignToStack<16, 4>
]>;

// AMDIL 32-bit C Calling convention.
// Register assignment for actual arguments.
def Call_CC_AMDIL32 : CallingConv<[
  CCIfType<[f32], CCBitConvertToType<i32>>,
  CCIfType<[i64, f64, v2f32], CCBitConvertToType<v2i32>>,
  CCIfType<[v2i64, v4f32, v2f64],
    CCBitConvertToType<v4i32>>,

  CCIfType<[v8i8], CCPromoteToType<v8i32>>,

//CCIfType<[f16], CCBitConvertToType<i16>>,

  CCIfType<[i1, i8, i16, i32], CCAssignToReg<
    [ include "AMDILRegisterUsesScalar.td" ]> >,

 CCIfType<[v2i1, v2i8, v2i16, v2i32], CCAssignToReg<
   [ include "AMDILRegisterUsesV2.td" ]> >,

 CCIfType<[v4i1, v4i8, v4i16, v4i32], CCAssignToReg<
   [ include "AMDILRegisterUsesV4.td" ]> >,

  CCAssignToStack<16, 16>
]>;

// AMDIL 32-bit C return-value convention.
// Register assignment for actual return values.
def Call_RetCC_AMDIL32 : CallingConv<[
  CCIfType<[f32], CCBitConvertToType<i32>>,
  CCIfType<[i64, f64, v2f32], CCBitConvertToType<v2i32>>,
  CCIfType<[v2i64, v4f32, v2f64],
    CCBitConvertToType<v4i32>>,
//  CCIfType<[f16], CCBitConvertToType<i16>>,
  CCIfType<[v8i8], CCPromoteToType<v8i32>>,
 CCIfType<[i1, i8, i16, i32], CCAssignToReg<
 [ include "AMDILRegisterUsesScalar.td" ]> >,

 CCIfType<[v2i1, v2i8, v2i16, v2i32], CCAssignToReg<
 [ include "AMDILRegisterUsesV2.td" ]> >,

 CCIfType<[v4i1, v4i8, v4i16, v4i32], CCAssignToReg<
 [ include "AMDILRegisterUsesV4.td" ]> >,

 CCAssignToStack<16, 16>
]>;

//===----------------------------------------------------------------------===//
// Calling Conventions for Small I/O RegSet for Stress Testing
// Do not have small regset for return values as we do not support passing
// return values on stack yet.
//===----------------------------------------------------------------------===//

// AMDIL 32-bit C Calling convention.
def Small_CC_AMDIL32 : CallingConv<[
 CCIfType<[i1, i8, i16, i32, f32], CCAssignToReg<
 [INx0, INy0, INz0, INw0]>>,

 CCIfType<[i1, i8, i16, i32, f32], CCAssignToStack<4, 4>>,

 CCIfType<[v2i32, v2f32, v2i8, v2i16, f64, i64], CCAssignToReg<
 [INxy0]>>,

 CCIfType<[v2i32, v2f32, v2i8, v2i16, f64, i64], CCAssignToStack<8, 4>>,

 CCIfType<[v4i32, v4f32, v4i8, v4i16, v2f64, v2i64], CCAssignToReg<
 [IN0]>>,

 CCIfType<[v8i8], CCPromoteToType<v8i32>>,
 CCAssignToStack<16, 4>
]>;

// AMDIL 32-bit C Calling convention.
// Register assignment for actual arguments
def Small_Call_CC_AMDIL32 : CallingConv<[
 CCIfType<[i1, i8, i16, i32, f32], CCAssignToReg<
 [Rx1, Ry1, Rz1, Rw1]>>,

 CCIfType<[i1, i8, i16, i32, f32], CCAssignToStack<4, 4>>,

 CCIfType<[v2i32, v2f32, v2i8, v2i16, f64, i64], CCAssignToReg<
 [Rxy1, Rzw1]>>,

 CCIfType<[v2i32, v2f32, v2i8, v2i16, f64, i64], CCAssignToStack<8, 4>>,

 CCIfType<[v4i32, v4f32, v4i8, v4i16, v2f64, v2i64], CCAssignToReg<
 [R1]>>,
 CCIfType<[v8i8], CCPromoteToType<v8i32>>,
 CCAssignToStack<16, 4>
]>;

//===----------------------------------------------------------------------===//
// Callee-saved register lists.
//===----------------------------------------------------------------------===//

def CSR_NoRegs : CalleeSavedRegs<(add)>;
// the registers that actually need to be saved by callees
def CSR_CSR : CalleeSavedRegs<(add FP)>;
// all registers except the ones used for return values are preserved in callee
def CSR_CC : CalleeSavedRegs<(add (sequence "R%u", 9, 288), (sequence "R%u", 1000, 1012), FP)>;
