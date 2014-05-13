//===- AMDIL.td - AMDIL Target Machine -------------*- tablegen -*-===//
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
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces which we are implementing
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// AMDIL Subtarget features.
//===----------------------------------------------------------------------===//
def FeatureFP64 : SubtargetFeature<"fp64",
        "CapsOverride[AMDIL::Caps::DoubleOps]",
        "true",
        "Enable 64bit double precision operations">;
def FeatureByteAddress : SubtargetFeature<"byte_addressable_store",
        "CapsOverride[AMDIL::Caps::ByteStores]",
        "true",
        "Enable byte addressable stores">;
def FeatureBarrierDetect : SubtargetFeature<"barrier_detect",
        "CapsOverride[AMDIL::Caps::BarrierDetect]",
        "true",
        "Enable duplicate barrier detection(HD5XXX or later).">;
def FeatureImages : SubtargetFeature<"images",
        "CapsOverride[AMDIL::Caps::Images]",
        "true",
        "Enable image functions">;
def FeatureMultiUAV : SubtargetFeature<"multi_uav",
        "CapsOverride[AMDIL::Caps::MultiUAV]",
        "true",
        "Generate multiple UAV code(HD5XXX family or later)">;
def FeatureStackUAV : SubtargetFeature<"stack-uav",
        "CapsOverride[AMDIL::Caps::StackUAV]",
        "true",
        "Use stack UAV for stack variables">;
def FeatureUseMacroForCall : SubtargetFeature<"macro-call",
        "CapsOverride[AMDIL::Caps::UseMacroForCall]",
        "true",
        "Use outline macro for function call">;
def FeatureMacroDB : SubtargetFeature<"macrodb",
        "CapsOverride[AMDIL::Caps::MacroDB]",
        "true",
        "Use internal macrodb, instead of macrodb in driver">;
def FeatureNoAlias : SubtargetFeature<"noalias",
        "CapsOverride[AMDIL::Caps::NoAlias]",
        "true",
        "assert that all kernel argument pointers are not aliased">;
def FeatureNoInline : SubtargetFeature<"no-inline",
        "CapsOverride[AMDIL::Caps::NoInline]",
        "true",
        "specify whether to not inline functions">;

def Feature64BitPtr : SubtargetFeature<"64bitptr",
        "mIs64bit",
        "true",
        "Specify if 64bit addressing should be used.">;

def FeatureSmallGlobalObjects : SubtargetFeature<"small-global-objects",
        "mSmallGlobalObjects",
        "true",
        "Specify that when using 64-bit pointers, no object will be large enough or span 32-bit boundary to require 64-bit addressing calculations">;
def FeatureDebug : SubtargetFeature<"debug",
        "CapsOverride[AMDIL::Caps::Debug]",
        "true",
        "Debug mode is enabled, so disable hardware accelerated address spaces.">;

def MWGS_3_256_1_1 : SubtargetFeature<"mwgs-3-256-1-1",
        "CapsOverride[AMDIL::Caps::MWGS_3_256_1_1]",
        "true",
        "Maximum workgroup size of 256">;

def MWGS_3_128_1_1 : SubtargetFeature<"mwgs-3-128-1-1",
        "CapsOverride[AMDIL::Caps::MWGS_3_128_1_1]",
        "true",
        "Maximum workgroup size of 128">;

def MWGS_3_64_1_1 : SubtargetFeature<"mwgs-3-64-1-1",
        "CapsOverride[AMDIL::Caps::MWGS_3_64_1_1]",
        "true",
        "Maximum workgroup size of 64">;

def MWGS_3_32_1_1 : SubtargetFeature<"mwgs-3-32-1-1",
        "CapsOverride[AMDIL::Caps::MWGS_3_32_1_1]",
        "true",
        "Maximum workgroup size of 32">;

def MWGS_3_16_1_1 : SubtargetFeature<"mwgs-3-16-1-1",
        "CapsOverride[AMDIL::Caps::MWGS_3_16_1_1]",
        "true",
        "Maximum workgroup size of 16">;

def FeatureMetadata30 : SubtargetFeature<"metadata30",
        "mMetadata30",
        "true",
        "Enable generation of version 3.0 of the metadata spec">;

def FeatureFlatAS : SubtargetFeature<"flatas", "mFlatAddress", "true",
    "Enable flat address space override for global/local/scratch/constant memory.">;

def FeatureShortOps : SubtargetFeature<"short-ops", "mShortOps", "true",
    "Supports operations on i16.">;
def FeatureHalfOps : SubtargetFeature<"half-ops", "mHalfOps", "true",
    "Supports operations on f16.">;

//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//


include "AMDILRegisterInfo.td"
include "AMDILCallingConv.td"
include "AMDILInstrInfo.td"
def AMDILInstrInfo : InstrInfo {}

//===----------------------------------------------------------------------===//
// AMDIL processors supported.
//===----------------------------------------------------------------------===//
include "Processors.td"

//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//
def AMDILAsmWriter : AsmWriter {
  string AsmWriterClassName = "AsmPrinter";
  int Variant = 0;
}

def AMDILAsmParser : AsmParser {
  string AsmParserClassName = "AsmParser";
  int Variant = 0;
  string CommentDelimiter = ";";
  string RegisterPrefix = "r";
}


def AMDIL : Target {
  // Pull in Instruction Info:
  let InstructionSet = AMDILInstrInfo;
  let AssemblyWriters = [AMDILAsmWriter];
  let AssemblyParsers = [AMDILAsmParser];
}

