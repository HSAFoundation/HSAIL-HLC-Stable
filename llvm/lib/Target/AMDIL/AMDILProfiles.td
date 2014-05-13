//===- AMDILProfiles.td - AMD IL Profiles ------------===//
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
// to the U.S. Export Administration Regulations (EAR), (15 C.F.R. Sections
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
// Industry and Securitys website at http://www.bis.doc.gov/.
//
//==-----------------------------------------------------------------------===//
// These are used for custom selection dag type profiles

//===----------------------------------------------------------------------===//
// Custom Selection DAG Type Profiles
//===----------------------------------------------------------------------===//
// SDTCisDP - The specified operand has double type
// Tablegen needs to be hacked to get this constraint to work
//class SDTCisDP<int OpNum> : SDTypeConstraint<OpNum>;

//===----------------------------------------------------------------------===//
// Generic Profile Types
//===----------------------------------------------------------------------===//

def SDTIL_GenUnaryOp : SDTypeProfile<1, 1, [
    SDTCisSameAs<0, 1>
    ]>;
def SDTIL_GenBinaryOp : SDTypeProfile<1, 2, [
    SDTCisSameAs<0, 1>, SDTCisSameAs<1, 2>
    ]>;
def SDTIL_GenTernaryOp : SDTypeProfile<1, 3, [
    SDTCisSameAs<0, 1>, SDTCisSameAs<1, 2>, SDTCisSameAs<2, 3>
    ]>;
def SDTIL_GenCMovLog : SDTypeProfile<1, 3, [
    SDTCisSameAs<0, 2>, SDTCisSameAs<2, 3>, SDTCisInt<1>
    ]>;
def SDTIL_GenVecBuild : SDTypeProfile<1, 1, [
    SDTCisEltOfVec<1, 0>
    ]>;

def SDTIL_GenVecExtract : SDTypeProfile<1, 2, [
    SDTCisEltOfVec<0, 1>, SDTCisVT<2, i32>
    ]>;

def SDTIL_GenVecInsert : SDTypeProfile<1, 4, [
    SDTCisEltOfVec<2, 1>, SDTCisSameAs<0, 1>,
    SDTCisVT<3, i32>, SDTCisVT<4, i32>
    ]>;

def SDTIL_GenVecShuffle : SDTypeProfile <1, 2, [
    SDTCisSameAs<0, 1>, SDTCisVT<2, i32>
    ]>;

def SDTIL_GenVecConcat : SDTypeProfile <1, 2, [
    SDTCisSameAs<1, 2>
    ]>;
//===----------------------------------------------------------------------===//
// Conversion Profile Types
//===----------------------------------------------------------------------===//

def SDTIL_IntTernaryOp : SDTypeProfile <1, 3,
  [SDTCisInt<0>, SDTCisSameAs<0, 1>, SDTCisSameAs<0, 2>]>;

def SDTIL_BitInsert : SDTypeProfile <1, 4,
  [SDTCisInt<0>, SDTCisSameAs<0, 1>, SDTCisSameAs<0, 2>, SDTCisSameAs<0, 3>]>;

def SDTIL_DPToFPOp : SDTypeProfile<1, 1, [
    SDTCisFP<0>, SDTCisFP<1>, SDTCisOpSmallerThanOp<0, 1>
    ]>; // d2f

def SDTIL_AnyToInt : SDTypeProfile<1, 1, [
    SDTCisInt<0>
    ]>;
def SDTIL_IntToAny : SDTypeProfile<1, 1, [
    SDTCisInt<1>
    ]>;
//===----------------------------------------------------------------------===//
// Scalar Profile Types
//===----------------------------------------------------------------------===//

// Add instruction pattern to handle offsets of memory operationns
def SDTIL_AddAddrri: SDTypeProfile<1, 2, [
    SDTCisInt<0>, SDTCisPtrTy<1>, SDTCisSameAs<0, 2>
    ]>;
def SDTIL_AddAddrir : SDTypeProfile<1, 2, [
    SDTCisInt<0>, SDTCisPtrTy<2>, SDTCisSameAs<0, 1>
    ]>;

def SDTIL_LCreate : SDTypeProfile<1, 2, [
    SDTCisVT<0, i64>, SDTCisVT<1, i32>, SDTCisSameAs<1, 2>
    ]>;
def SDTIL_LCreate2 : SDTypeProfile<1, 2, [
    SDTCisVT<0, v2i64>, SDTCisVT<1, v2i32>, SDTCisSameAs<1, 2>
    ]>;
def SDTIL_LComp : SDTypeProfile<1, 1, [
    SDTCisVT<0, i32>, SDTCisVT<1, i64>
    ]>;
def SDTIL_LComp2 : SDTypeProfile<1, 1, [
    SDTCisVT<0, v2i32>, SDTCisVT<1, v2i64>
    ]>;
def SDTIL_DCreate : SDTypeProfile<1, 2, [
    SDTCisVT<0, f64>, SDTCisVT<1, i32>, SDTCisSameAs<1, 2>
    ]>;
def SDTIL_DComp : SDTypeProfile<1, 1, [
    SDTCisVT<0, i32>, SDTCisVT<1, f64>
    ]>;
def SDTIL_DCreate2 : SDTypeProfile<1, 2, [
    SDTCisVT<0, v2f64>, SDTCisVT<1, v2i32>, SDTCisSameAs<1, 2>
    ]>;
def SDTIL_DComp2 : SDTypeProfile<1, 1, [
    SDTCisVT<0, v2i32>, SDTCisVT<1, v2f64>
    ]>;
//===----------------------------------------------------------------------===//
// Flow Control Profile Types
//===----------------------------------------------------------------------===//
// Profile for Normal Call
def SDTIL_Call : SDTypeProfile<0, 1, [
    SDTCisVT<0, i32>
    ]>;
// Branch instruction where second and third are basic blocks
def SDTIL_BRCond : SDTypeProfile<0, 2, [
    SDTCisVT<0, OtherVT>
    ]>;
// Comparison instruction
def SDTIL_Cmp  : SDTypeProfile<1, 3, [
    SDTCisSameAs<0, 2>, SDTCisSameAs<2,3>, SDTCisVT<1, i32>
    ]>;


//===----------------------------------------------------------------------===//
// Call Sequence Profiles
//===----------------------------------------------------------------------===//
def SDTIL_CallSeqStart  : SDCallSeqStart< [
    SDTCisVT<0, i32>
    ]>;
def SDTIL_CallSeqEnd    : SDCallSeqEnd< [
    SDTCisVT<0, i32>, SDTCisVT<1, i32>
    ]>;

//===----------------------------------------------------------------------===//
// Image Operation Profiles
//===----------------------------------------------------------------------===//
def SDTIL_ImageRead  : SDTypeProfile<1, 3,
    [SDTCisVT<0, v4i32>, SDTCisPtrTy<1>, SDTCisVT<2, i32>, SDTCisVT<3, v4f32>]>;
def SDTIL_ImageWrite : SDTypeProfile<0, 3,
    [SDTCisPtrTy<0>, SDTCisVT<1, v2i32>, SDTCisVT<2, v4i32>]>;
def SDTIL_ImageWrite3D : SDTypeProfile<0, 3,
    [SDTCisPtrTy<0>, SDTCisVT<1, v4i32>, SDTCisVT<2, v4i32>]>;
def SDTIL_ImageInfo  : SDTypeProfile<1, 1,
    [SDTCisVT<0, v4i32>, SDTCisPtrTy<1>]>;
//===----------------------------------------------------------------------===//
// Atomic Operation Profiles
//===----------------------------------------------------------------------===//
def SDTIL_UniAtomNoRet : SDTypeProfile<0, 2, [
    SDTCisPtrTy<0>, SDTCisInt<1>
    ]>;
def SDTIL_BinAtomNoRet : SDTypeProfile<0, 3, [
    SDTCisPtrTy<0>, SDTCisInt<1>, SDTCisVT<2, i32>
    ]>;
def SDTIL_TriAtomNoRet : SDTypeProfile<0, 4, [
    SDTCisPtrTy<0>, SDTCisInt<1>, SDTCisInt<2>, SDTCisVT<3, i32>
    ]>;
def SDTIL_UniAtom : SDTypeProfile<1, 2, [
    SDTCisInt<0>, SDTCisPtrTy<1>, SDTCisVT<2, i32>
    ]>;
def SDTIL_BinAtom : SDTypeProfile<1, 3, [
    SDTCisSameAs<0, 2>, SDTCisPtrTy<1>, SDTCisInt<2>, SDTCisVT<3, i32>
    ]>;
def SDTIL_TriAtom : SDTypeProfile<1, 4, [
    SDTCisSameAs<0, 2>, SDTCisPtrTy<1>, SDTCisInt<2>,
    SDTCisSameAs<3, 2>, SDTCisVT<4, i32>
    ]>;

def SDTIL_BinAtomFloat : SDTypeProfile<1, 3, [
    SDTCisVT<0, i32>, SDTCisPtrTy<1>, SDTCisVT<2, f32>, SDTCisVT<3, f32>
    ]>;
def SDTIL_BinAtomNoRetFloat : SDTypeProfile<0, 3, [
    SDTCisPtrTy<0>, SDTCisVT<1, f32>, SDTCisVT<2, f32>
    ]>;

def SDTIL_Append : SDTypeProfile<1, 1, [
    SDTCisVT<0, i32>, SDTCisPtrTy<1>
    ]>;

//===----------------------------------------------------------------------===//
// Atomic Load/Store Profile Types
//===----------------------------------------------------------------------===//

def SDTIL_AtomLoad : SDTypeProfile<1, 3, [
  SDTCisPtrTy<1>, SDTCisInt<2>, SDTCisVT<3, i32>
]>;

def SDTIL_AtomStore : SDTypeProfile<0, 4, [
  SDTCisPtrTy<0>, SDTCisInt<2>, SDTCisVT<3, i32>
]>;
