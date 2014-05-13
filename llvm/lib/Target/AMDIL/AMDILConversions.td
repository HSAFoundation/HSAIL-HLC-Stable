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
//==-----------------------------------------------------------------------===//
def actosi16rr:Pat < (i16 (anyext GPRI8:$src)),
  (UBIT_EXTRACTi16iir
    (i16 8),
    (i16 0),
    (COPY_TO_REGCLASS GPRI8:$src, GPRI16))>;

def uctosi16rr:Pat < (i16 (zext GPRI8:$src)),
  (UBIT_EXTRACTi16iir
    (i16 8),
    (i16 0),
    (COPY_TO_REGCLASS GPRI8:$src, GPRI16))>;

def sctosi16rr:Pat < (i16 (sext GPRI8:$src)),
  (IBIT_EXTRACTi16iir
    (i16 8),
    (i16 0),
    (COPY_TO_REGCLASS GPRI8:$src, GPRI16))>;

def actoii32rr:Pat < (i32 (anyext GPRI8:$src)),
  (UBIT_EXTRACTi32iir
    (i32 8),
    (i32 0),
    (COPY_TO_REGCLASS GPRI8:$src, GPR_32))>;

def uctoii32rr:Pat < (i32 (zext GPRI8:$src)),
  (UBIT_EXTRACTi32iir
    (i32 8),
    (i32 0),
    (COPY_TO_REGCLASS GPRI8:$src, GPR_32))>;

def sctoii32rr:Pat < (i32 (sext GPRI8:$src)),
  (IBIT_EXTRACTi32iir
    (i32 8),
    (i32 0),
    (COPY_TO_REGCLASS GPRI8:$src, GPR_32))>;

def actoli64rr:Pat < (i64 (anyext GPRI8:$src)),
(LCREATEi64rr
 (USHRi32i32rr
  (SHLi32i32rr
(COPY_TO_REGCLASS GPRI8:$src, GPR_32),
   (LOADCONSTi32 24)),
  (LOADCONSTi32 24)),
 (LOADCONSTi32 0)) >;


def uctoli64rr:Pat < (i64 (zext GPRI8:$src)),
(LCREATEi64rr
 (USHRi32i32rr
  (SHLi32i32rr
(COPY_TO_REGCLASS GPRI8:$src, GPR_32),
   (LOADCONSTi32 24)),
  (LOADCONSTi32 24)),
 (LOADCONSTi32 0)) >;


def sctoli64rr:Pat < (i64 (sext GPRI8:$src)),
(LCREATEi64rr
 (SHRi32i32rr
  (SHLi32i32rr
(COPY_TO_REGCLASS GPRI8:$src, GPR_32),
   (LOADCONSTi32 24)),
  (LOADCONSTi32 24)),
 (SHRi32i32rr
  (SHLi32i32rr
(COPY_TO_REGCLASS GPRI8:$src, GPR_32),
   (LOADCONSTi32 24)),
  (LOADCONSTi32 31))) >;


def astoii32rr:Pat < (i32 (anyext GPRI16:$src)),
  (UBIT_EXTRACTi32rrr
    (i32 16),
    (i32 0),
    (COPY_TO_REGCLASS GPRI16:$src, GPR_32))>;

def ustoii32rr:Pat < (i32 (zext GPRI16:$src)),
  (UBIT_EXTRACTi32rrr
    (i32 16),
    (i32 0),
    (COPY_TO_REGCLASS GPRI16:$src, GPR_32))>;

def sstoii32rr:Pat < (i32 (sext GPRI16:$src)),
  (IBIT_EXTRACTi32rrr
    (i32 16),
    (i32 0),
    (COPY_TO_REGCLASS GPRI16:$src, GPR_32))>;

def astoli64rr:Pat < (i64 (anyext GPRI16:$src)),
(LCREATEi64rr
 (USHRi32i32rr
  (SHLi32i32rr
(COPY_TO_REGCLASS GPRI16:$src, GPR_32),
   (LOADCONSTi32 16)),
  (LOADCONSTi32 16)),
 (LOADCONSTi32 0)) >;


def ustoli64rr:Pat < (i64 (zext GPRI16:$src)),
(LCREATEi64rr
 (USHRi32i32rr
  (SHLi32i32rr
(COPY_TO_REGCLASS GPRI16:$src, GPR_32),
   (LOADCONSTi32 16)),
  (LOADCONSTi32 16)),
 (LOADCONSTi32 0)) >;


def sstoli64rr:Pat < (i64 (sext GPRI16:$src)),
(LCREATEi64rr
 (SHRi32i32rr
  (SHLi32i32rr
(COPY_TO_REGCLASS GPRI16:$src, GPR_32),
   (LOADCONSTi32 16)),
  (LOADCONSTi32 16)),
 (SHRi32i32rr
  (SHLi32i32rr
(COPY_TO_REGCLASS GPRI16:$src, GPR_32),
   (LOADCONSTi32 16)),
  (LOADCONSTi32 31))) >;


def aitoli64rr:Pat < (i64 (anyext GPR_32:$src)),
(LCREATEi64rr
(COPY_TO_REGCLASS GPR_32:$src, GPR_32),
 (LOADCONSTi32 0)) >;


def uitoli64rr:Pat < (i64 (zext GPR_32:$src)),
(LCREATEi64rr
(COPY_TO_REGCLASS GPR_32:$src, GPR_32),
 (LOADCONSTi32 0)) >;

def sitoli64rr:Pat < (i64 (sext GPR_32:$src)),
  (LCREATEi64rr GPR_32:$src, (SHRi32i32ri GPR_32:$src, (i32 31)))
>;

def sctoff32rr:Pat < (f32 (sint_to_fp GPRI8:$src)),
(f32
 (ITOFf32r
  (SHRi32i32rr
   (SHLi32i32rr
(COPY_TO_REGCLASS GPRI8:$src, GPR_32),
    (LOADCONSTi32 24)),
   (LOADCONSTi32 24)))) >;


def uctoff32rr:Pat < (f32 (uint_to_fp GPRI8:$src)),
(f32
 (UTOFf32r
  (USHRi32i32rr
   (SHLi32i32rr
(COPY_TO_REGCLASS GPRI8:$src, GPR_32),
    (LOADCONSTi32 24)),
   (LOADCONSTi32 24)))) >;


def ftosci8rr:Pat < (i8 (fp_to_sint GPR_32:$src)),
(i8
 (COPY_TO_REGCLASS
  (ANDi32rr
(FTOIi32r GPR_32:$src),
   (LOADCONSTi32 0x000000FF)), GPRI8)) >;


def ftouci8rr:Pat < (i8 (fp_to_uint GPR_32:$src)),
(i8
 (COPY_TO_REGCLASS
  (ANDi32rr
(FTOUi32r GPR_32:$src),
   (LOADCONSTi32 0x000000FF)), GPRI8)) >;


def sctodf64rr:Pat < (f64 (sint_to_fp GPRI8:$src)),
(f64 (FTODr
      (ITOFf32r
       (SHRi32i32rr
  (SHLi32i32rr
(COPY_TO_REGCLASS GPRI8:$src, GPR_32),
   (LOADCONSTi32 24)),
  (LOADCONSTi32 24))))) >;


def uctodf64rr:Pat < (f64 (uint_to_fp GPRI8:$src)),
(f64 (FTODr
      (UTOFf32r
       (USHRi32i32rr
  (SHLi32i32rr
(COPY_TO_REGCLASS GPRI8:$src, GPR_32),
   (LOADCONSTi32 24)),
  (LOADCONSTi32 24))))) >;


def dtosci8rr:Pat < (i8 (fp_to_sint GPR_64:$src)),
(i8
 (COPY_TO_REGCLASS
  (ANDi32rr
(FTOIi32r (DTOFr GPR_64:$src)),
   (LOADCONSTi32 0x000000FF)), GPRI8)) >;


def dtouci8rr:Pat < (i8 (fp_to_uint GPR_64:$src)),
(i8
 (COPY_TO_REGCLASS
  (ANDi32rr
(FTOUi32r (DTOFr GPR_64:$src)),
   (LOADCONSTi32 0x000000FF)), GPRI8)) >;


def sstoff32rr:Pat < (f32 (sint_to_fp GPRI16:$src)),
(f32
 (ITOFf32r
  (SHRi32i32rr
   (SHLi32i32rr
(COPY_TO_REGCLASS GPRI16:$src, GPR_32),
    (LOADCONSTi32 16)),
   (LOADCONSTi32 16)))) >;


def ustoff32rr:Pat < (f32 (uint_to_fp GPRI16:$src)),
(f32
 (UTOFf32r
  (USHRi32i32rr
   (SHLi32i32rr
(COPY_TO_REGCLASS GPRI16:$src, GPR_32),
    (LOADCONSTi32 16)),
   (LOADCONSTi32 16)))) >;


def ftossi16rr:Pat < (i16 (fp_to_sint GPR_32:$src)),
(i16
 (COPY_TO_REGCLASS
  (ANDi32rr
(FTOIi32r GPR_32:$src),
   (LOADCONSTi32 0x0000FFFF)), GPRI16)) >;


def ftousi16rr:Pat < (i16 (fp_to_uint GPR_32:$src)),
(i16
 (COPY_TO_REGCLASS
  (ANDi32rr
(FTOUi32r GPR_32:$src),
   (LOADCONSTi32 0x0000FFFF)), GPRI16)) >;


def sstodf64rr:Pat < (f64 (sint_to_fp GPRI16:$src)),
(f64 (FTODr
      (ITOFf32r
       (SHRi32i32rr
  (SHLi32i32rr
(COPY_TO_REGCLASS GPRI16:$src, GPR_32),
   (LOADCONSTi32 16)),
  (LOADCONSTi32 16))))) >;


def ustodf64rr:Pat < (f64 (uint_to_fp GPRI16:$src)),
(f64 (FTODr
      (UTOFf32r
       (USHRi32i32rr
  (SHLi32i32rr
(COPY_TO_REGCLASS GPRI16:$src, GPR_32),
   (LOADCONSTi32 16)),
  (LOADCONSTi32 16))))) >;


def dtossi16rr:Pat < (i16 (fp_to_sint GPR_64:$src)),
(i16
 (COPY_TO_REGCLASS
  (ANDi32rr
(FTOIi32r (DTOFr GPR_64:$src)),
   (LOADCONSTi32 0x0000FFFF)), GPRI16)) >;


def dtousi16rr:Pat < (i16 (fp_to_uint GPR_64:$src)),
(i16
 (COPY_TO_REGCLASS
  (ANDi32rr
(FTOUi32r (DTOFr GPR_64:$src)),
   (LOADCONSTi32 0x0000FFFF)), GPRI16)) >;





def stoci8rr:Pat < (i8 (trunc GPRI16:$src)),
(COPY_TO_REGCLASS
 (SHRi32i32rr
 (SHLi32i32rr
   (COPY_TO_REGCLASS (ANDi16rr GPRI16:$src,
                     (LOADCONSTi16 0x000000FF)), GPR_32),
                     (LOADCONSTi32 16)),
                     (LOADCONSTi32 16)), GPRI8)>;


def itoci8rr:Pat < (i8 (trunc GPR_32:$src)),
(COPY_TO_REGCLASS
 (SHRi32i32rr
 (SHLi32i32rr
   (COPY_TO_REGCLASS
     (ANDi32rr GPR_32:$src,
       (LOADCONSTi32 0x000000FF)), GPR_32),
       (LOADCONSTi32 24)),
 (LOADCONSTi32 24)), GPRI8) >;


def itosi16rr:Pat < (i16 (trunc GPR_32:$src)),
(COPY_TO_REGCLASS
 (SHRi32i32rr
 (SHLi32i32rr
   (COPY_TO_REGCLASS (ANDi32rr GPR_32:$src,
                       (LOADCONSTi32 0x0000FFFF)), GPR_32),
   (LOADCONSTi32 16)),
   (LOADCONSTi32 16)), GPRI16)>;


def ltoci8rr:Pat < (i8 (trunc GPR_64:$src)),
(COPY_TO_REGCLASS
 (SHRi32i32rr
 (SHLi32i32rr
   (ANDi32rr
(LLOi64r GPR_64:$src),
    (LOADCONSTi32 0x000000FF))
 , (LOADCONSTi32 24)),
 (LOADCONSTi32 24))
  , GPRI8) >;


def ltosi16rr:Pat < (i16 (trunc GPR_64:$src)),
(COPY_TO_REGCLASS
 (SHRi32i32rr
 (SHLi32i32rr
   (ANDi32rr
(LLOi64r GPR_64:$src),
    (LOADCONSTi32 0x0000FFFF))
 , (LOADCONSTi32 16)),
 (LOADCONSTi32 16))
  , GPRI16) >;


def ltoii32rr:Pat < (i32 (trunc GPR_64:$src)), (LLOi64r GPR_64:$src) >;

def actosv2i16rr:Pat < (v2i16 (anyext GPRV2I8:$src)),
  (UBIT_EXTRACTv2i16rrr
    (VCREATEv2i16r (LOADCONSTi16 8)),
    (VCREATEv2i16r (LOADCONSTi16 0)),
    (COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I16))>;

def uctosv2i16rr:Pat < (v2i16 (zext GPRV2I8:$src)),
  (UBIT_EXTRACTv2i16rrr
    (VCREATEv2i16r (LOADCONSTi16 8)),
    (VCREATEv2i16r (LOADCONSTi16 0)),
    (COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I16))>;

def sctosv2i16rr:Pat < (v2i16 (sext GPRV2I8:$src)),
  (IBIT_EXTRACTv2i16rrr
    (VCREATEv2i16r (LOADCONSTi16 8)),
    (VCREATEv2i16r (LOADCONSTi16 0)),
    (COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I16))>;

def actoiv2i32rr:Pat < (v2i32 (anyext GPRV2I8:$src)),
  (UBIT_EXTRACTv2i32rrr
    (VCREATEv2i32r (LOADCONSTi32 8)),
    (VCREATEv2i32r (LOADCONSTi32 0)),
    (COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I32))>;

def uctoiv2i32rr:Pat <(v2i32 (zext GPRV2I8:$src)),
  (UBIT_EXTRACTv2i32rrr
    (VCREATEv2i32r (LOADCONSTi32 8)),
    (VCREATEv2i32r (LOADCONSTi32 0)),
    (COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I32))>;

def sctoiv2i32rr:Pat < (v2i32 (sext GPRV2I8:$src)),
  (IBIT_EXTRACTv2i32rrr
    (VCREATEv2i32r (LOADCONSTi32 8)),
    (VCREATEv2i32r (LOADCONSTi32 0)),
    (COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I32))>;

def actolv2i64rr:Pat < (v2i64 (anyext GPRV2I8:$src)),
(LCREATEv2i64rr
 (USHRv2i32i32rr
  (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I32),
   (VCREATEv2i32r (LOADCONSTi32 24))),
  (VCREATEv2i32r (LOADCONSTi32 24))),
 (VCREATEv2i32r (LOADCONSTi32 0))) >;


def uctolv2i64rr:Pat < (v2i64 (zext GPRV2I8:$src)),
(LCREATEv2i64rr
 (USHRv2i32i32rr
  (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I32),
   (VCREATEv2i32r (LOADCONSTi32 24))),
  (VCREATEv2i32r (LOADCONSTi32 24))),
 (VCREATEv2i32r (LOADCONSTi32 0))) >;


def sctolv2i64rr:Pat < (v2i64 (sext GPRV2I8:$src)),
(LCREATEv2i64rr
 (SHRv2i32i32rr
  (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I32),
   (VCREATEv2i32r (LOADCONSTi32 24))),
  (VCREATEv2i32r (LOADCONSTi32 24))),
 (SHRv2i32i32rr
  (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I32),
   (VCREATEv2i32r (LOADCONSTi32 24))),
  (VCREATEv2i32r (LOADCONSTi32 31)))) >;


def astoiv2i32rr:Pat < (v2i32 (anyext GPRV2I16:$src)),
  (UBIT_EXTRACTv2i32rrr
    (VCREATEv2i32r (LOADCONSTi32 16)),
    (VCREATEv2i32r (LOADCONSTi32 0)),
    (COPY_TO_REGCLASS GPRV2I16:$src, GPRV2I32))>;

def ustoiv2i32rr:Pat < (v2i32 (zext GPRV2I16:$src)),
  (UBIT_EXTRACTv2i32rrr
    (VCREATEv2i32r (LOADCONSTi32 16)),
    (VCREATEv2i32r (LOADCONSTi32 0)),
    (COPY_TO_REGCLASS GPRV2I16:$src, GPRV2I32))>;

// FIXME: BUILD_VECTOR isn't lowered for constants
// def ustoiv2i32rr:Pat < (v2i32 (zext GPRV2I16:$src)),
//   (UBIT_EXTRACTv2i32iir
//     (v2i32 16),
//     (v2i32 0),
//     (COPY_TO_REGCLASS GPRV2I16:$src, GPRV2I32))>;

def sstoiv2i32rr:Pat < (v2i32 (sext GPRV2I16:$src)),
  (IBIT_EXTRACTv2i32rrr
    (VCREATEv2i32r (LOADCONSTi32 16)),
    (VCREATEv2i32r (LOADCONSTi32 0)),
    (COPY_TO_REGCLASS GPRV2I16:$src, GPRV2I32))>;

def astolv2i64rr:Pat < (v2i64 (anyext GPRV2I16:$src)),
(LCREATEv2i64rr
  (USHRv2i32i32rr
    (SHLv2i32i32rr
      (COPY_TO_REGCLASS GPRV2I16:$src, GPRV2I32),
      (VCREATEv2i32r (LOADCONSTi32 16))),
      (VCREATEv2i32r (LOADCONSTi32 16))),
    (VCREATEv2i32r (LOADCONSTi32 0)))>;


def ustolv2i64rr:Pat < (v2i64 (zext GPRV2I16:$src)),
(LCREATEv2i64rr
  (USHRv2i32i32rr
    (SHLv2i32i32rr
      (COPY_TO_REGCLASS GPRV2I16:$src, GPRV2I32),
      (VCREATEv2i32r (LOADCONSTi32 16))),
      (VCREATEv2i32r (LOADCONSTi32 16))),
    (VCREATEv2i32r (LOADCONSTi32 0))) >;


def sstolv2i64rr:Pat < (v2i64 (sext GPRV2I16:$src)),
(LCREATEv2i64rr
 (SHRv2i32i32rr
  (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I16:$src, GPRV2I32),
   (VCREATEv2i32r (LOADCONSTi32 16))),
  (VCREATEv2i32r (LOADCONSTi32 16))),
 (SHRv2i32i32rr
  (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I16:$src, GPRV2I32),
   (VCREATEv2i32r (LOADCONSTi32 16))),
  (VCREATEv2i32r (LOADCONSTi32 31)))) >;


def aitolv2i64rr:Pat < (v2i64 (anyext GPRV2I32:$src)),
(LCREATEv2i64rr GPRV2I32:$src, (VCREATEv2i32r (LOADCONSTi32 0))) >;

def uitolv2i64rr:Pat < (v2i64 (zext GPRV2I32:$src)),
(LCREATEv2i64rr GPRV2I32:$src,
 (VCREATEv2i32r (LOADCONSTi32 0))) >;


def sitolv2i64rr:Pat < (v2i64 (sext GPRV2I32:$src)),
(LCREATEv2i64rr
  GPRV2I32:$src,
  (SHRv2i32i32rr
    (SHLv2i32i32rr
      GPRV2I32:$src,
      (VCREATEv2i32r (LOADCONSTi32 0))),
  (VCREATEv2i32r (LOADCONSTi32 31))))>;



def sctofv2f32rr:Pat < (v2f32 (sint_to_fp GPRV2I8:$src)),
(v2f32
 (ITOFv2f32r
  (SHRv2i32i32rr
   (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I32),
    (VCREATEv2i32r (LOADCONSTi32 24))),
   (VCREATEv2i32r (LOADCONSTi32 24))))) >;


def uctofv2f32rr:Pat < (v2f32 (uint_to_fp GPRV2I8:$src)),
(v2f32
 (UTOFv2f32r
  (USHRv2i32i32rr
   (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I32),
    (VCREATEv2i32r (LOADCONSTi32 24))),
   (VCREATEv2i32r (LOADCONSTi32 24))))) >;


def ftoscv2i8rr:Pat < (v2i8 (fp_to_sint GPRV2I32:$src)),
(v2i8
 (COPY_TO_REGCLASS
  (ANDv2i32rr
(FTOIv2i32r GPRV2I32:$src),
   (VCREATEv2i32r (LOADCONSTi32 0x000000FF))), GPRV2I8)) >;


def ftoucv2i8rr:Pat < (v2i8 (fp_to_uint GPRV2I32:$src)),
(v2i8
 (COPY_TO_REGCLASS
  (ANDv2i32rr
(FTOUv2i32r GPRV2I32:$src),
   (VCREATEv2i32r (LOADCONSTi32 0x000000FF))), GPRV2I8)) >;

def sctodv2f64rr:Pat < (v2f64 (sint_to_fp GPRV2I8:$src)),
(v2f64
 (VINSERTv2f64rr
 (VCREATEv2f64r
 (FTODr
  (VEXTRACTv2f32r
  (ITOFv2f32r
   (SHRv2i32i32rr
    (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I32),
     (VCREATEv2i32r (LOADCONSTi32 24))),
    (VCREATEv2i32r (LOADCONSTi32 24)))),
  1)
 )),
 (FTODr
  (VEXTRACTv2f32r
  (ITOFv2f32r
   (SHRv2i32i32rr
    (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I32),
     (VCREATEv2i32r (LOADCONSTi32 24))),
    (VCREATEv2i32r (LOADCONSTi32 24)))),
  2)
  ), 1, 256)
 ) >;

def uctodv2f64rr:Pat < (v2f64 (uint_to_fp GPRV2I8:$src)),
(v2f64
 (VINSERTv2f64rr
 (VCREATEv2f64r
 (FTODr
  (VEXTRACTv2f32r
  (UTOFv2f32r
   (USHRv2i32i32rr
    (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I32),
     (VCREATEv2i32r (LOADCONSTi32 24))),
    (VCREATEv2i32r (LOADCONSTi32 24)))),
  1)
 )),
 (FTODr
  (VEXTRACTv2f32r
  (UTOFv2f32r
   (USHRv2i32i32rr
    (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I8:$src, GPRV2I32),
     (VCREATEv2i32r (LOADCONSTi32 24))),
    (VCREATEv2i32r (LOADCONSTi32 24)))),
  2)
  ), 1, 256)
 ) >;


def dtoscv2i8rr:Pat < (v2i8 (fp_to_sint GPRV2I64:$src)),
(v2i8
 (COPY_TO_REGCLASS
  (ANDv2i32rr
(FTOIv2i32r (VINSERTv2f32rr
             (VCREATEv2f32r
              (DTOFr (VEXTRACTv2f64r GPRV2I64:$src, 1))),
             (DTOFr (VEXTRACTv2f64r GPRV2I64:$src, 2)), 1, 256)),
   (VCREATEv2i32r (LOADCONSTi32 0x000000FF))), GPRV2I8)) >;


def dtoucv2i8rr:Pat < (v2i8 (fp_to_uint GPRV2I64:$src)),
(v2i8
 (COPY_TO_REGCLASS
  (ANDv2i32rr
(FTOUv2i32r (VINSERTv2f32rr
             (VCREATEv2f32r
              (DTOFr (VEXTRACTv2f64r GPRV2I64:$src, 1))),
             (DTOFr (VEXTRACTv2f64r GPRV2I64:$src, 2)), 1, 256)),
   (VCREATEv2i32r (LOADCONSTi32 0x000000FF))), GPRV2I8)) >;


def sstofv2f32rr:Pat < (v2f32 (sint_to_fp GPRV2I16:$src)),
(v2f32
 (ITOFv2f32r
  (SHRv2i32i32rr
   (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I16:$src, GPRV2I32),
    (VCREATEv2i32r (LOADCONSTi32 16))),
   (VCREATEv2i32r (LOADCONSTi32 16))))) >;


def ustofv2f32rr:Pat < (v2f32 (uint_to_fp GPRV2I16:$src)),
(v2f32
 (UTOFv2f32r
  (USHRv2i32i32rr
   (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I16:$src, GPRV2I32),
    (VCREATEv2i32r (LOADCONSTi32 16))),
   (VCREATEv2i32r (LOADCONSTi32 16))))) >;


def ftossv2i16rr:Pat < (v2i16 (fp_to_sint GPRV2I32:$src)),
(v2i16
 (COPY_TO_REGCLASS
  (ANDv2i32rr
(FTOIv2i32r GPRV2I32:$src),
   (VCREATEv2i32r (LOADCONSTi32 0x0000FFFF))), GPRV2I16)) >;


def ftousv2i16rr:Pat < (v2i16 (fp_to_uint GPRV2I32:$src)),
(v2i16
 (COPY_TO_REGCLASS
  (ANDv2i32rr
(FTOUv2i32r GPRV2I32:$src),
   (VCREATEv2i32r (LOADCONSTi32 0x0000FFFF))), GPRV2I16)) >;


def sstodv2f64rr:Pat < (v2f64 (sint_to_fp GPRV2I16:$src)),
(v2f64
 (VINSERTv2f64rr
 (VCREATEv2f64r
 (FTODr
  (VEXTRACTv2f32r
  (ITOFv2f32r
   (SHRv2i32i32rr
    (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I16:$src, GPRV2I32),
     (VCREATEv2i32r (LOADCONSTi32 16))),
    (VCREATEv2i32r (LOADCONSTi32 16)))),
  1)
 )),
 (FTODr
  (VEXTRACTv2f32r
  (ITOFv2f32r
   (SHRv2i32i32rr
    (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I16:$src, GPRV2I32),
     (VCREATEv2i32r (LOADCONSTi32 16))),
    (VCREATEv2i32r (LOADCONSTi32 16)))),
  2)
  ), 1, 256)
 ) >;

def ustodv2f64rr:Pat < (v2f64 (uint_to_fp GPRV2I16:$src)),
(v2f64
 (VINSERTv2f64rr
 (VCREATEv2f64r
 (FTODr
  (VEXTRACTv2f32r
  (UTOFv2f32r
   (USHRv2i32i32rr
    (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I16:$src, GPRV2I32),
     (VCREATEv2i32r (LOADCONSTi32 16))),
    (VCREATEv2i32r (LOADCONSTi32 16)))),
  1)
 )),
 (FTODr
  (VEXTRACTv2f32r
  (UTOFv2f32r
   (USHRv2i32i32rr
    (SHLv2i32i32rr
(COPY_TO_REGCLASS GPRV2I16:$src, GPRV2I32),
     (VCREATEv2i32r (LOADCONSTi32 16))),
    (VCREATEv2i32r (LOADCONSTi32 16)))),
  2)
  ), 1, 256)
 ) >;


def dtossv2i16rr:Pat < (v2i16 (fp_to_sint GPRV2I64:$src)),
(v2i16
 (COPY_TO_REGCLASS
  (ANDv2i32rr
(FTOIv2i32r (VINSERTv2f32rr
             (VCREATEv2f32r
              (DTOFr (VEXTRACTv2f64r GPRV2I64:$src, 1))),
             (DTOFr (VEXTRACTv2f64r GPRV2I64:$src, 2)), 1, 256)),
   (VCREATEv2i32r (LOADCONSTi32 0x0000FFFF))), GPRV2I16)) >;


def dtousv2i16rr:Pat < (v2i16 (fp_to_uint GPRV2I64:$src)),
(v2i16
 (COPY_TO_REGCLASS
  (ANDv2i32rr
(FTOUv2i32r (VINSERTv2f32rr
             (VCREATEv2f32r
              (DTOFr (VEXTRACTv2f64r GPRV2I64:$src, 1))),
             (DTOFr (VEXTRACTv2f64r GPRV2I64:$src, 2)), 1, 256)),
   (VCREATEv2i32r (LOADCONSTi32 0x0000FFFF))), GPRV2I16)) >;

def stocv2i8rr:Pat < (v2i8 (trunc GPRV2I16:$src)),
(COPY_TO_REGCLASS
 (SHRv2i32i32rr
 (SHLv2i32i32rr
   (COPY_TO_REGCLASS
(ANDv2i16rr GPRV2I16:$src,
     (VCREATEv2i16r (LOADCONSTi16 0x000000FF))), GPRV2I32)
   , (VCREATEv2i32r (LOADCONSTi32 24))),
 (VCREATEv2i32r (LOADCONSTi32 24)))
  , GPRV2I8) >;


def itocv2i8rr:Pat < (v2i8 (trunc GPRV2I32:$src)),
(COPY_TO_REGCLASS
 (SHRv2i32i32rr
 (SHLv2i32i32rr
   (COPY_TO_REGCLASS
(ANDv2i32rr GPRV2I32:$src,
     (VCREATEv2i32r (LOADCONSTi32 0x000000FF))), GPRV2I32)
   , (VCREATEv2i32r (LOADCONSTi32 24))),
 (VCREATEv2i32r (LOADCONSTi32 24)))
  , GPRV2I8) >;


def itosv2i16rr:Pat < (v2i16 (trunc GPRV2I32:$src)),
(COPY_TO_REGCLASS
 (SHRv2i32i32rr
 (SHLv2i32i32rr
   (COPY_TO_REGCLASS
(ANDv2i32rr GPRV2I32:$src,
     (VCREATEv2i32r (LOADCONSTi32 0x0000FFFF))), GPRV2I32)
   , (VCREATEv2i32r (LOADCONSTi32 16))),
 (VCREATEv2i32r (LOADCONSTi32 16)))
  , GPRV2I16) >;


def ltocv2i8rr:Pat < (v2i8 (trunc GPRV2I64:$src)),
(COPY_TO_REGCLASS
 (SHRv2i32i32rr
 (SHLv2i32i32rr
   (ANDv2i32rr
(LLOv2i64r GPRV2I64:$src),
    (VCREATEv2i32r (LOADCONSTi32 0x000000FF)))
   , (VCREATEv2i32r (LOADCONSTi32 24))),
 (VCREATEv2i32r (LOADCONSTi32 24)))
  , GPRV2I8) >;


def ltosv2i16rr:Pat < (v2i16 (trunc GPRV2I64:$src)),
(COPY_TO_REGCLASS
 (SHRv2i32i32rr
 (SHLv2i32i32rr
   (ANDv2i32rr
(LLOv2i64r GPRV2I64:$src),
    (VCREATEv2i32r (LOADCONSTi32 0x0000FFFF)))
   , (VCREATEv2i32r (LOADCONSTi32 16))),
 (VCREATEv2i32r (LOADCONSTi32 16)))
  , GPRV2I16) >;


def ltoiv2i32rr:Pat < (v2i32 (trunc GPRV2I64:$src)), (LLOv2i64r GPRV2I64:$src)>;


def actosv4i16rr:Pat < (v4i16 (anyext GPRV4I8:$src)),
  (UBIT_EXTRACTv4i16rrr
    (VCREATEv4i16r (LOADCONSTi16 8)),
    (VCREATEv4i16r (LOADCONSTi16 0)),
    (COPY_TO_REGCLASS GPRV4I8:$src, GPRV4I16))>;

def uctosv4i16rr:Pat < (v4i16 (zext GPRV4I8:$src)),
  (UBIT_EXTRACTv4i16rrr
    (VCREATEv4i16r (LOADCONSTi16 8)),
    (VCREATEv4i16r (LOADCONSTi16 0)),
    (COPY_TO_REGCLASS GPRV4I8:$src, GPRV4I16))>;

def sctosv4i16rr:Pat < (v4i16 (sext GPRV4I8:$src)),
  (IBIT_EXTRACTv4i16rrr
    (VCREATEv4i16r (LOADCONSTi16 8)),
    (VCREATEv4i16r (LOADCONSTi16 0)),
    (COPY_TO_REGCLASS GPRV4I8:$src, GPRV4I16))>;

def actoiv4i32rr:Pat < (v4i32 (anyext GPRV4I8:$src)),
  (UBIT_EXTRACTv4i32rrr
    (VCREATEv4i32r (LOADCONSTi32 8)),
    (VCREATEv4i32r (LOADCONSTi32 0)),
    (COPY_TO_REGCLASS GPRV4I8:$src, GPRV4I32))>;

def uctoiv4i32rr:Pat < (v4i32 (zext GPRV4I8:$src)),
  (UBIT_EXTRACTv4i32rrr
    (VCREATEv4i32r (LOADCONSTi32 8)),
    (VCREATEv4i32r (LOADCONSTi32 0)),
    (COPY_TO_REGCLASS GPRV4I8:$src, GPRV4I32))>;

def sctoiv4i32rr:Pat < (v4i32 (sext GPRV4I8:$src)),
  (IBIT_EXTRACTv4i32rrr
    (VCREATEv4i32r (LOADCONSTi32 8)),
    (VCREATEv4i32r (LOADCONSTi32 0)),
    (COPY_TO_REGCLASS GPRV4I8:$src, GPRV4I32))>;

def astoiv4i32rr:Pat < (v4i32 (anyext GPRV4I16:$src)),
  (UBIT_EXTRACTv4i32rrr
    (VCREATEv4i32r (LOADCONSTi32 16)),
    (VCREATEv4i32r (LOADCONSTi32 0)),
    (COPY_TO_REGCLASS GPRV4I16:$src, GPRV4I32))>;

def ustoiv4i32rr:Pat < (v4i32 (zext GPRV4I16:$src)),
  (UBIT_EXTRACTv4i32rrr
    (VCREATEv4i32r (LOADCONSTi32 16)),
    (VCREATEv4i32r (LOADCONSTi32 0)),
    (COPY_TO_REGCLASS GPRV4I16:$src, GPRV4I32))>;

def sstoiv4i32rr:Pat < (v4i32 (sext GPRV4I16:$src)),
  (IBIT_EXTRACTv4i32rrr
    (VCREATEv4i32r (LOADCONSTi32 16)),
    (VCREATEv4i32r (LOADCONSTi32 0)),
    (COPY_TO_REGCLASS GPRV4I16:$src, GPRV4I32))>;


def sctofv4f32rr:Pat < (v4f32 (sint_to_fp GPRV4I8:$src)),
(v4f32
 (ITOFv4f32r
  (SHRv4i32i32rr
   (SHLv4i32i32rr
(COPY_TO_REGCLASS GPRV4I8:$src, GPRV4I32),
    (VCREATEv4i32r (LOADCONSTi32 24))),
   (VCREATEv4i32r (LOADCONSTi32 24))))) >;


def uctofv4f32rr:Pat < (v4f32 (uint_to_fp GPRV4I8:$src)),
(v4f32
 (UTOFv4f32r
  (USHRv4i32i32rr
   (SHLv4i32i32rr
(COPY_TO_REGCLASS GPRV4I8:$src, GPRV4I32),
    (VCREATEv4i32r (LOADCONSTi32 24))),
   (VCREATEv4i32r (LOADCONSTi32 24))))) >;


def ftoscv4i8rr:Pat < (v4i8 (fp_to_sint GPRV4I32:$src)),
(v4i8
 (COPY_TO_REGCLASS
  (ANDv4i32rr
(FTOIv4i32r GPRV4I32:$src),
   (VCREATEv4i32r (LOADCONSTi32 0x000000FF))), GPRV4I8)) >;


def ftoucv4i8rr:Pat < (v4i8 (fp_to_uint GPRV4I32:$src)),
(v4i8
 (COPY_TO_REGCLASS
  (ANDv4i32rr
(FTOUv4i32r GPRV4I32:$src),
   (VCREATEv4i32r (LOADCONSTi32 0x000000FF))), GPRV4I8)) >;


def sstofv4f32rr:Pat < (v4f32 (sint_to_fp GPRV4I16:$src)),
(v4f32
 (ITOFv4f32r
  (SHRv4i32i32rr
   (SHLv4i32i32rr
(COPY_TO_REGCLASS GPRV4I16:$src, GPRV4I32),
    (VCREATEv4i32r (LOADCONSTi32 16))),
   (VCREATEv4i32r (LOADCONSTi32 16))))) >;


def ustofv4f32rr:Pat < (v4f32 (uint_to_fp GPRV4I16:$src)),
(v4f32
 (UTOFv4f32r
  (USHRv4i32i32rr
   (SHLv4i32i32rr
(COPY_TO_REGCLASS GPRV4I16:$src, GPRV4I32),
    (VCREATEv4i32r (LOADCONSTi32 16))),
   (VCREATEv4i32r (LOADCONSTi32 16))))) >;


def ftossv4i16rr:Pat < (v4i16 (fp_to_sint GPRV4I32:$src)),
(v4i16
 (COPY_TO_REGCLASS
  (ANDv4i32rr
(FTOIv4i32r GPRV4I32:$src),
   (VCREATEv4i32r (LOADCONSTi32 0x0000FFFF))), GPRV4I16)) >;


def ftousv4i16rr:Pat < (v4i16 (fp_to_uint GPRV4I32:$src)),
(v4i16
 (COPY_TO_REGCLASS
  (ANDv4i32rr
(FTOUv4i32r GPRV4I32:$src),
   (VCREATEv4i32r (LOADCONSTi32 0x0000FFFF))), GPRV4I16)) >;





def stocv4i8rr:Pat < (v4i8 (trunc GPRV4I16:$src)),
(COPY_TO_REGCLASS
 (SHRv4i32i32rr
 (SHLv4i32i32rr
   (COPY_TO_REGCLASS
(ANDv4i16rr GPRV4I16:$src,
     (VCREATEv4i16r (LOADCONSTi16 0x000000FF))), GPRV4I32)
   , (VCREATEv4i32r (LOADCONSTi32 24))),
 (VCREATEv4i32r (LOADCONSTi32 24)))
  , GPRV4I8) >;


def itocv4i8rr:Pat < (v4i8 (trunc GPRV4I32:$src)),
(COPY_TO_REGCLASS
 (SHRv4i32i32rr
 (SHLv4i32i32rr
   (COPY_TO_REGCLASS
(ANDv4i32rr GPRV4I32:$src,
     (VCREATEv4i32r (LOADCONSTi32 0x000000FF))), GPRV4I32)
   , (VCREATEv4i32r (LOADCONSTi32 24))),
 (VCREATEv4i32r (LOADCONSTi32 24)))
  , GPRV4I8) >;


def itosv4i16rr:Pat < (v4i16 (trunc GPRV4I32:$src)),
(COPY_TO_REGCLASS
 (SHRv4i32i32rr
 (SHLv4i32i32rr
   (COPY_TO_REGCLASS
(ANDv4i32rr GPRV4I32:$src,
     (VCREATEv4i32r (LOADCONSTi32 0x0000FFFF))), GPRV4I32)
   , (VCREATEv4i32r (LOADCONSTi32 16))),
 (VCREATEv4i32r (LOADCONSTi32 16)))
  , GPRV4I16) >;

