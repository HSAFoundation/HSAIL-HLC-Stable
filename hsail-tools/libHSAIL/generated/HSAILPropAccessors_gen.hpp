// University of Illinois/NCSA
// Open Source License
//
// Copyright (c) 2013, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Developed by:
//
//     HSA Team
//
//     Advanced Micro Devices, Inc
//
//     www.amd.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal with
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimers.
//
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimers in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the names of the LLVM Team, University of Illinois at
//       Urbana-Champaign, nor the names of its contributors may be used to
//       endorse or promote products derived from this Software without specific
//       prior written permission.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
// SOFTWARE.
## tn: BrigDirectiveOffset32_t init
## tn: uint16_t labelCount
## tn: BrigOperandOffset32_t values
## tn: uint16_t outArgCount
## tn: BrigCodeOffset32_t code
## tn: uint8_t bytes
## tn: uint32_t width
## tn: bool isNoNull
## tn: uint8_t equivClass
[0] uint8_t BrigInstAtomic::equivClass
[0] uint8_t BrigInstImage::equivClass
[0] uint8_t BrigInstMem::equivClass
## tn: uint32_t column
## tn: BrigSamplerAddressing8_t addressing
## tn: uint32_t offsetLo
## tn: BrigDirectiveOffset32_t nextTopLevelDirective
## tn: BrigSegCvtModifier8_t allBits
## tn: BrigType16_t signalType
[0] BrigType16_t BrigInstSignal::signalType
## tn: BrigDataOffset32_t data
## tn: uint16_t inArgCount
## tn: uint16_t elementCount
## tn: uint32_t height
## tn: BrigDirectiveOffset32_t label
## tn: BrigStringOffset32_t name
## tn: BrigDirectiveOffset32_t elements
## tn: uint16_t valueCount
## tn: BrigMachineModel8_t machineModel
## tn: BrigDirectiveOffset32_t labels
## tn: uint16_t operandCount
## tn: BrigDirectiveOffset32_t firstInArg
## tn: BrigVersion32_t hsailMajor
## tn: uint32_t offsetHi
## tn: BrigAlignment8_t align
[0] BrigAlignment8_t BrigInstMem::align
## tn: BrigAtomicOperation8_t signalOperation
[0] BrigAtomicOperation8_t BrigInstSignal::signalOperation
## tn: uint64_t dim
## tn: bool isArray
## tn: BrigMemoryScope8_t memoryScope
[0] BrigMemoryScope8_t BrigInstAtomic::memoryScope
[0] BrigMemoryScope8_t BrigInstMemFence::memoryScope
## tn: BrigSamplerCoordNormalization8_t coord
## tn: bool ftz
## tn: uint32_t depth
## tn: uint32_t line
## tn: BrigMemoryModifier modifier
[0] BrigMemoryModifier BrigInstMem::modifier
## tn: bool isDeclaration
## tn: BrigPack8_t pack
[0] BrigPack8_t BrigInstCmp::pack
[0] BrigPack8_t BrigInstMod::pack
## tn: BrigDirectiveOffset32_t samplers
## tn: uint16_t imageCount
## tn: BrigVersion32_t brigMajor
## tn: BrigSamplerFilter8_t filter
## tn: BrigProfile8_t profile
## tn: BrigSegment8_t segment
[0] BrigSegment8_t BrigInstAddr::segment
[0] BrigSegment8_t BrigInstAtomic::segment
[0] BrigSegment8_t BrigInstMem::segment
[0] BrigSegment8_t BrigInstQueue::segment
[0] BrigSegment8_t BrigInstSeg::segment
[0] BrigSegment8_t BrigInstSegCvt::segment
## tn: BrigExecutableModifier modifier
## tn: BrigWidth8_t width
[0] BrigWidth8_t BrigInstBr::width
[0] BrigWidth8_t BrigInstLane::width
[0] BrigWidth8_t BrigInstMem::width
## tn: BrigMemoryModifier8_t allBits
## tn: uint64_t offset
## tn: BrigAtomicOperation8_t atomicOperation
[0] BrigAtomicOperation8_t BrigInstAtomic::atomicOperation
## tn: uint16_t samplerCount
## tn: uint32_t array
## tn: BrigDirectiveSignatureArgument args
## tn: BrigCompareOperation8_t compare
[0] BrigCompareOperation8_t BrigInstCmp::compare
## tn: BrigType16_t imageType
[0] BrigType16_t BrigInstImage::imageType
[0] BrigType16_t BrigInstQueryImage::imageType
## tn: BrigStringOffset32_t reg
## tn: BrigType16_t sourceType
[0] BrigType16_t BrigInstCmp::sourceType
[0] BrigType16_t BrigInstCvt::sourceType
[0] BrigType16_t BrigInstLane::sourceType
[0] BrigType16_t BrigInstSegCvt::sourceType
[0] BrigType16_t BrigInstSourceType::sourceType
## tn: BrigImageGeometry8_t geometry
[0] BrigImageGeometry8_t BrigInstImage::geometry
[0] BrigImageGeometry8_t BrigInstQueryImage::geometry
## tn: bool isFlexArray
## tn: BrigVersion32_t brigMinor
## tn: BrigMemoryFenceSegments8_t segments
[0] BrigMemoryFenceSegments8_t BrigInstMemFence::segments
## tn: BrigControlDirective16_t control
## tn: BrigDirectiveOffset32_t objects
## tn: BrigImageQuery8_t imageQuery
[0] BrigImageQuery8_t BrigInstQueryImage::imageQuery
## tn: BrigSegCvtModifier modifier
[0] BrigSegCvtModifier BrigInstSegCvt::modifier
## tn: uint32_t elementCount
## tn: BrigImageChannelType8_t channelType
## tn: BrigVersion32_t hsailMinor
## tn: BrigDirectiveOffset32_t ref
## tn: BrigSymbolModifier modifier
## tn: BrigImageChannelOrder8_t channelOrder
## tn: BrigDirectiveOffset32_t symbol
## tn: BrigAluModifier modifier
[0] BrigAluModifier BrigInstCmp::modifier
[0] BrigAluModifier BrigInstCvt::modifier
[0] BrigAluModifier BrigInstMod::modifier
## tn: bool isConst
## tn: BrigMemoryOrder8_t memoryOrder
[0] BrigMemoryOrder8_t BrigInstAtomic::memoryOrder
[0] BrigMemoryOrder8_t BrigInstMemFence::memoryOrder
[0] BrigMemoryOrder8_t BrigInstQueue::memoryOrder
[0] BrigMemoryOrder8_t BrigInstSignal::memoryOrder
## tn: BrigDirectiveOffset32_t targets
## tn: BrigDirectiveOffset32_t images
## tn: uint16_t byteCount
## tn: BrigExecutableModifier8_t allBits
## tn: uint32_t instCount
## tn: BrigSamplerQuery8_t samplerQuery
[0] BrigSamplerQuery8_t BrigInstQuerySampler::samplerQuery
## tn: uint32_t dimHi
## tn: BrigStringOffset32_t string
## tn: uint32_t dimLo
## tn: BrigStringOffset32_t filename
## tn: BrigSymbolModifier8_t allBits
## tn: BrigType16_t coordType
[0] BrigType16_t BrigInstImage::coordType
## tn: BrigDirectiveOffset32_t firstScopedDirective
## tn: BrigLinkage linkage
## tn: BrigDataOffset32_t dataAs
## tn: BrigAluModifier16_t allBits
## tn: BrigRound8_t round
