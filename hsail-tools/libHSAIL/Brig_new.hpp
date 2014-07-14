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
enum BrigVersion {
    //.nowrap
    //.nodump
    BRIG_VERSION_HSAIL_MAJOR = 0,
    BRIG_VERSION_HSAIL_MINOR = 20140227,
    BRIG_VERSION_BRIG_MAJOR = 0,
    BRIG_VERSION_BRIG_MINOR = 20140227
};

// new typedefs
typedef uint8_t  BrigRound8_t;
typedef uint8_t  BrigAtomicOperation8_t;
typedef uint8_t  BrigLinkage8_t;              //.defValue=Brig::BRIG_LINKAGE_NONE
typedef uint8_t  BrigCompareOperation8_t;
typedef uint16_t BrigControlDirective16_t;
typedef uint16_t BrigType16_t;
typedef uint16_t BrigDirectiveKinds16_t;
typedef uint8_t  BrigImageGeometry8_t;        //.defValue=Brig::BRIG_GEOMETRY_UNKNOWN
typedef uint8_t  BrigImageChannelType8_t;     //.defValue=Brig::BRIG_CHANNEL_TYPE_UNKNOWN
typedef uint8_t  BrigImageChannelOrder8_t;    //.defValue=Brig::BRIG_CHANNEL_ORDER_UNKNOWN
typedef uint8_t  BrigMachineModel8_t;         //.defValue=Brig::BRIG_MACHINE_LARGE
typedef uint8_t  BrigMemoryOrder8_t;          //.defValue=Brig::BRIG_MEMORY_ORDER_RELAXED
typedef uint8_t  BrigMemoryScope8_t;          //.defValue=Brig::BRIG_MEMORY_SCOPE_SYSTEM
typedef uint16_t BrigOpcode16_t;
typedef uint8_t  BrigPack8_t;                 //.defValue=Brig::BRIG_PACK_NONE
typedef uint8_t  BrigProfile8_t;              //.defValue=Brig::BRIG_PROFILE_FULL
typedef uint8_t  BrigSegment8_t;              //.defValue=0
typedef uint32_t BrigCodeOffset32_t;          //.defValue=0      //.wtype=ItemRef<Inst>
typedef uint32_t BrigDirectiveOffset32_t;     //.defValue=0      //.wtype=ItemRef<Directive>
typedef uint32_t BrigOperandOffset32_t;       //.defValue=0      //.wtype=ItemRef<Operand>
typedef uint32_t BrigStringOffset32_t;        //.defValue=0      //.wtype=StrRef
typedef uint16_t BrigAluModifier16_t;
typedef uint32_t BrigDataOffset32_t;
typedef uint8_t  BrigExecutableModifier8_t;
typedef uint8_t  BrigSymbolModifier8_t;
typedef uint8_t  BrigMemoryModifier8_t;
typedef uint8_t  BrigSamplerAddressing8_t;     //.defValue=Brig::BRIG_ADDRESSING_CLAMP_TO_EDGE
typedef uint8_t  BrigWidth8_t;
typedef uint16_t BrigOperandKinds16_t;
typedef uint16_t BrigInstKinds16_t;
typedef uint32_t BrigVersion32_t;
typedef uint8_t  BrigSamplerFilter8_t;
typedef uint8_t  BrigSamplerCoordNormalization8_t;
typedef uint8_t  BrigAlignment8_t;
typedef uint8_t  BrigMemoryFenceSegments8_t;
typedef uint8_t  BrigSegCvtModifier8_t;
typedef uint8_t  BrigImageQuery8_t;
typedef uint8_t  BrigSamplerQuery8_t;


enum BrigAtomicOperation { //.tdcaption="Atomic Operations"
    //.mnemo={ s/^BRIG_ATOMIC_//;lc }
    //.mnemo_token=_EMAtomicOp
    //.mnemo_context=EInstModifierInstAtomicContext
  BRIG_ATOMIC_ADD = 0,
  BRIG_ATOMIC_AND = 1,
  BRIG_ATOMIC_CAS = 2,
  BRIG_ATOMIC_DEC = 3,
  BRIG_ATOMIC_EXCH = 4,
  BRIG_ATOMIC_INC = 5,
  BRIG_ATOMIC_LD = 6,
  BRIG_ATOMIC_MAX = 7,
  BRIG_ATOMIC_MIN = 8,
  BRIG_ATOMIC_OR = 9,
  BRIG_ATOMIC_ST = 10,
  BRIG_ATOMIC_SUB = 11,
  BRIG_ATOMIC_XOR = 12,
  BRIG_ATOMIC_WAIT_EQ = 13,
  BRIG_ATOMIC_WAIT_NE = 14,
  BRIG_ATOMIC_WAIT_LT = 15,
  BRIG_ATOMIC_WAIT_GTE = 16,
  BRIG_ATOMIC_WAITTIMEOUT_EQ = 17,
  BRIG_ATOMIC_WAITTIMEOUT_NE = 18,
  BRIG_ATOMIC_WAITTIMEOUT_LT = 19,
  BRIG_ATOMIC_WAITTIMEOUT_GTE = 20
};

enum BrigSamplerAddressing {
    //.mnemo={ s/^BRIG_ADDRESSING_//;lc }
    //.mnemo_token=ESamplerAddressingMode
    BRIG_ADDRESSING_UNDEFINED = 0,
    BRIG_ADDRESSING_CLAMP_TO_EDGE = 1,
    BRIG_ADDRESSING_CLAMP_TO_BORDER = 2,
    BRIG_ADDRESSING_REPEAT = 3,
    BRIG_ADDRESSING_MIRRORED_REPEAT = 4
};

//BrigCompareOperation
enum BrigCompareOperation { //.tdcaption="Comparison Operators"
    //.mnemo={ s/^BRIG_COMPARE_//;lc }
    //.mnemo_token=_EMCompare
    BRIG_COMPARE_EQ   = 0,
    BRIG_COMPARE_NE   = 1,
    BRIG_COMPARE_LT   = 2,
    BRIG_COMPARE_LE   = 3,
    BRIG_COMPARE_GT   = 4,
    BRIG_COMPARE_GE   = 5,
    BRIG_COMPARE_EQU  = 6,
    BRIG_COMPARE_NEU  = 7,
    BRIG_COMPARE_LTU  = 8,
    BRIG_COMPARE_LEU  = 9,
    BRIG_COMPARE_GTU  = 10,
    BRIG_COMPARE_GEU  = 11,
    BRIG_COMPARE_NUM  = 12,
    BRIG_COMPARE_NAN  = 13,
    BRIG_COMPARE_SEQ  = 14,
    BRIG_COMPARE_SNE  = 15,
    BRIG_COMPARE_SLT  = 16,
    BRIG_COMPARE_SLE  = 17,
    BRIG_COMPARE_SGT  = 18,
    BRIG_COMPARE_SGE  = 19,
    BRIG_COMPARE_SGEU = 20,
    BRIG_COMPARE_SEQU = 21,
    BRIG_COMPARE_SNEU = 22,
    BRIG_COMPARE_SLTU = 23,
    BRIG_COMPARE_SLEU = 24,
    BRIG_COMPARE_SNUM = 25,
    BRIG_COMPARE_SNAN = 26,
    BRIG_COMPARE_SGTU = 27
};

//BrigControlDirective was BrigControlType
enum BrigControlDirective {
    //.mnemo={ s/^BRIG_CONTROL_//;lc }
    //.mnemo_token=EControl
    BRIG_CONTROL_NONE                       = 0, //.skip
    BRIG_CONTROL_ENABLEBREAKEXCEPTIONS      = 1,
    BRIG_CONTROL_ENABLEDETECTEXCEPTIONS     = 2,
    BRIG_CONTROL_MAXDYNAMICGROUPSIZE        = 3,
    BRIG_CONTROL_MAXFLATGRIDSIZE            = 4,
    BRIG_CONTROL_MAXFLATWORKGROUPSIZE       = 5,
    BRIG_CONTROL_REQUESTEDWORKGROUPSPERCU   = 6,
    BRIG_CONTROL_REQUIREDDIM                = 7,
    BRIG_CONTROL_REQUIREDGRIDSIZE           = 8,
    BRIG_CONTROL_REQUIREDWORKGROUPSIZE      = 9,
    BRIG_CONTROL_REQUIRENOPARTIALWORKGROUPS = 10
};

enum BrigPackedTypeBits {
    //.nodump
    BRIG_TYPE_PACK_SHIFT = 5,
    BRIG_TYPE_BASE_MASK = (1 << BRIG_TYPE_PACK_SHIFT) - 1,
    BRIG_TYPE_PACK_MASK = 3 << BRIG_TYPE_PACK_SHIFT,
    BRIG_TYPE_PACK_NONE = 0 << BRIG_TYPE_PACK_SHIFT,
    BRIG_TYPE_PACK_32   = 1 << BRIG_TYPE_PACK_SHIFT,
    BRIG_TYPE_PACK_64   = 2 << BRIG_TYPE_PACK_SHIFT,
    BRIG_TYPE_PACK_128  = 3 << BRIG_TYPE_PACK_SHIFT
};

//BrigType was BrigDataType
// TBD rename to BrigType and resolve conflicts
enum BrigTypeX {
    //.length={ /([0-9]+)X([0-9]+)/ ? $1*$2 : /([0-9]+)/ ? $1 : 0 }
    //.length_switch //.length_proto="int brigtype_get_length(unsigned arg)" //.length_default="return 0"
    //.mnemo={ s/^BRIG_TYPE_//;lc }
    //.mnemo_token=_EMType
    //.dispatch_switch //.dispatch_incfile=TemplateUtilities
    //.dispatch_proto="template<typename RetType, typename Visitor>\nRetType dispatchByType_gen(unsigned type, Visitor& v)"
    //.dispatch={ /^BRIG_TYPE_[BUSF][0-9]+/ ? "v.template visit< BrigType<$_> >()" : "v.visitNone(type)" }
    //.dispatch_arg="type" //.dispatch_default="return v.visitNone(type)"
    BRIG_TYPE_NONE = 0,      //.mnemo=""
    BRIG_TYPE_U8 = 1,        //.ctype=uint8_t  // unsigned integer 8 bits
    BRIG_TYPE_U16 = 2,       //.ctype=uint16_t // unsigned integer 16 bits
    BRIG_TYPE_U32 = 3,       //.ctype=uint32_t // unsigned integer 32 bits
    BRIG_TYPE_U64 = 4,       //.ctype=uint64_t // unsigned integer 64 bits
    BRIG_TYPE_S8 = 5,        //.ctype=int8_t   // signed integer 8 bits
    BRIG_TYPE_S16 = 6,       //.ctype=int16_t  // signed integer 16 bits
    BRIG_TYPE_S32 = 7,       //.ctype=int32_t  // signed integer 32 bits
    BRIG_TYPE_S64 = 8,       //.ctype=int64_t  // signed integer 64 bits
    BRIG_TYPE_F16 = 9,       //.ctype=f16_t    // floating-point 16 bits
    BRIG_TYPE_F32 = 10,      //.ctype=float    // floating-point 32 bits
    BRIG_TYPE_F64 = 11,      //.ctype=double   // floating-point 64 bits
    BRIG_TYPE_B1 = 12,       //.ctype=bool     // uninterpreted bit string of length 1 bit
    BRIG_TYPE_B8 = 13,       //.ctype=uint8_t  // uninterpreted bit string of length 8 bits
    BRIG_TYPE_B16 = 14,      //.ctype=uint16_t // uninterpreted bit string of length 16 bits
    BRIG_TYPE_B32 = 15,      //.ctype=uint32_t // uninterpreted bit string of length 32 bits
    BRIG_TYPE_B64 = 16,      //.ctype=uint64_t // uninterpreted bit string of length 64 bits
    BRIG_TYPE_B128 = 17,     //.ctype=b128_t   // uninterpreted bit string of length 128 bits
    BRIG_TYPE_SAMP = 18,     //.mnemo=samp  // sampler object
    BRIG_TYPE_ROIMG = 19,    //.mnemo=roimg // read-only image object
    BRIG_TYPE_WOIMG = 20,    //.mnemo=woimg // write-only image object
    BRIG_TYPE_RWIMG = 21,    //.mnemo=rwimg // read/write image object
    BRIG_TYPE_SIG32 = 22,    //.mnemo=sig32 // signal handle to signal object with 32 bit signal value
    BRIG_TYPE_SIG64 = 23,    //.mnemo=sig64 // signal handle to signal object with 64 bit signal value


    BRIG_TYPE_U8X4  = BRIG_TYPE_U8  | BRIG_TYPE_PACK_32,   //.ctype=uint8_t  // four bytes unsigned
    BRIG_TYPE_U8X8  = BRIG_TYPE_U8  | BRIG_TYPE_PACK_64,   //.ctype=uint8_t  // eight bytes unsigned
    BRIG_TYPE_U8X16 = BRIG_TYPE_U8  | BRIG_TYPE_PACK_128,  //.ctype=uint8_t  // 16 bytes unsigned
    BRIG_TYPE_U16X2 = BRIG_TYPE_U16 | BRIG_TYPE_PACK_32,   //.ctype=uint16_t // two short unsigned integers
    BRIG_TYPE_U16X4 = BRIG_TYPE_U16 | BRIG_TYPE_PACK_64,   //.ctype=uint16_t // four short unsigned integers
    BRIG_TYPE_U16X8 = BRIG_TYPE_U16 | BRIG_TYPE_PACK_128,  //.ctype=uint16_t // eight short unsigned integer
    BRIG_TYPE_U32X2 = BRIG_TYPE_U32 | BRIG_TYPE_PACK_64,   //.ctype=uint32_t // two unsigned integers
    BRIG_TYPE_U32X4 = BRIG_TYPE_U32 | BRIG_TYPE_PACK_128,  //.ctype=uint32_t // four unsigned integers
    BRIG_TYPE_U64X2 = BRIG_TYPE_U64 | BRIG_TYPE_PACK_128,  //.ctype=uint64_t // two 64-bit unsigned integers
    BRIG_TYPE_S8X4  = BRIG_TYPE_S8  | BRIG_TYPE_PACK_32,   //.ctype=int8_t   // four bytes signed
    BRIG_TYPE_S8X8  = BRIG_TYPE_S8  | BRIG_TYPE_PACK_64,   //.ctype=int8_t   // eight bytes signed
    BRIG_TYPE_S8X16 = BRIG_TYPE_S8  | BRIG_TYPE_PACK_128,  //.ctype=int8_t   // 16 bytes signed
    BRIG_TYPE_S16X2 = BRIG_TYPE_S16 | BRIG_TYPE_PACK_32,   //.ctype=int16_t  // two short signed integers
    BRIG_TYPE_S16X4 = BRIG_TYPE_S16 | BRIG_TYPE_PACK_64,   //.ctype=int16_t  // four short signed integers
    BRIG_TYPE_S16X8 = BRIG_TYPE_S16 | BRIG_TYPE_PACK_128,  //.ctype=int16_t  // eight short signed integers
    BRIG_TYPE_S32X2 = BRIG_TYPE_S32 | BRIG_TYPE_PACK_64,   //.ctype=int32_t  // two signed integers
    BRIG_TYPE_S32X4 = BRIG_TYPE_S32 | BRIG_TYPE_PACK_128,  //.ctype=int32_t  // four signed integers
    BRIG_TYPE_S64X2 = BRIG_TYPE_S64 | BRIG_TYPE_PACK_128,  //.ctype=int64_t  // two 64-bit signed integers
    BRIG_TYPE_F16X2 = BRIG_TYPE_F16 | BRIG_TYPE_PACK_32,   //.ctype=f16_t    // two half-floats
    BRIG_TYPE_F16X4 = BRIG_TYPE_F16 | BRIG_TYPE_PACK_64,   //.ctype=f16_t    // four half-floats
    BRIG_TYPE_F16X8 = BRIG_TYPE_F16 | BRIG_TYPE_PACK_128,  //.ctype=f16_t    // eight half-floats
    BRIG_TYPE_F32X2 = BRIG_TYPE_F32 | BRIG_TYPE_PACK_64,   //.ctype=float    // two floats
    BRIG_TYPE_F32X4 = BRIG_TYPE_F32 | BRIG_TYPE_PACK_128,  //.ctype=float    // four floats
    BRIG_TYPE_F64X2 = BRIG_TYPE_F64 | BRIG_TYPE_PACK_128,   //.ctype=double   // two doubles

    // Used internally
    BRIG_TYPE_INVALID                       = -1 //.skip
};

enum BrigDirectiveKinds {
    //.wname={ s/^BRIG//; MACRO2Name($_) }
    //.sizeof=$wname{ "sizeof(Brig$wname)" }
    //.sizeof_switch //.sizeof_proto="int size_of_directive(unsigned arg)" //.sizeof_default="return -1"

    //.isBodyOnly={ "false" }
    //.isBodyOnly_switch //.isBodyOnly_proto="bool isBodyOnly(Directive d)" //.isBodyOnly_arg="d.brig()->kind"
    //.isBodyOnly_default="assert(false); return false"
    //.isToplevelOnly={ "false" }
    //.isToplevelOnly_switch //.isToplevelOnly_proto="bool isToplevelOnly(Directive d)" //.isToplevelOnly_arg="d.brig()->kind"
    //.isToplevelOnly_default="assert(false); return false"

    BRIG_DIRECTIVE_ARG_SCOPE_END = 0,    //.isBodyOnly=true
    BRIG_DIRECTIVE_ARG_SCOPE_START = 1,  //.isBodyOnly=true
    BRIG_DIRECTIVE_BLOCK_END = 2,                                    //.wname=BlockEnd
    BRIG_DIRECTIVE_BLOCK_NUMERIC = 3,                                //.wname=BlockNumeric
    BRIG_DIRECTIVE_BLOCK_START = 4,                                  //.wname=BlockStart
    BRIG_DIRECTIVE_BLOCK_STRING = 5,                                 //.wname=BlockString
    BRIG_DIRECTIVE_COMMENT = 6,
    BRIG_DIRECTIVE_CONTROL = 7,          //.isBodyOnly=true
    BRIG_DIRECTIVE_EXTENSION = 8,        //.isToplevelOnly=true
    BRIG_DIRECTIVE_FBARRIER = 9,
    BRIG_DIRECTIVE_FUNCTION = 10,        //.isToplevelOnly=true
    BRIG_DIRECTIVE_IMAGE_INIT = 11,      //.isToplevelOnly=true
    BRIG_DIRECTIVE_KERNEL = 12,          //.isToplevelOnly=true
    BRIG_DIRECTIVE_LABEL = 13,           //.isBodyOnly=true
    BRIG_DIRECTIVE_LABEL_INIT = 14,      //.isBodyOnly=true
    BRIG_DIRECTIVE_LABEL_TARGETS = 15,   //.isBodyOnly=true
    BRIG_DIRECTIVE_LOC = 16,
    BRIG_DIRECTIVE_PRAGMA = 17,
    BRIG_DIRECTIVE_SAMPLER_INIT = 18,    //.isToplevelOnly=true
    BRIG_DIRECTIVE_SIGNATURE = 19,       //.isToplevelOnly=true
    BRIG_DIRECTIVE_VARIABLE = 20,
    BRIG_DIRECTIVE_VARIABLE_INIT = 21,
    BRIG_DIRECTIVE_VERSION = 22,         //.isToplevelOnly=true
    BRIG_DIRECTIVE_IMAGE_PROPERTIES = 23,  //.isToplevelOnly=true
    BRIG_DIRECTIVE_SAMPLER_PROPERTIES = 24 //.isToplevelOnly=true
};

//BrigImageGeometry was BrigGeom
enum BrigImageGeometry { //.tdcaption="Geometry"
    //.mnemo={ s/^BRIG_GEOMETRY_//;lc }
    //.mnemo_token=EImageGeometry
    BRIG_GEOMETRY_1D  = 0,
    BRIG_GEOMETRY_2D  = 1,
    BRIG_GEOMETRY_3D  = 2,
    BRIG_GEOMETRY_1DA = 3,
    BRIG_GEOMETRY_2DA = 4,
    BRIG_GEOMETRY_1DB = 5,
    BRIG_GEOMETRY_2DDEPTH = 6,
    BRIG_GEOMETRY_2DADEPTH = 7,

    // used internally
    BRIG_GEOMETRY_UNKNOWN //.mnemo=""
};

enum BrigImageChannelType {
    //.mnemo={ s/^BRIG_CHANNEL_TYPE_//;lc }
    //.mnemo_token=EImageFormat

    BRIG_CHANNEL_TYPE_SNORM_INT8         = 0,
    BRIG_CHANNEL_TYPE_SNORM_INT16        = 1,
    BRIG_CHANNEL_TYPE_UNORM_INT8         = 2,
    BRIG_CHANNEL_TYPE_UNORM_INT16        = 3,
    BRIG_CHANNEL_TYPE_UNORM_INT24        = 4,
    BRIG_CHANNEL_TYPE_UNORM_SHORT_555    = 5,
    BRIG_CHANNEL_TYPE_UNORM_SHORT_565    = 6,
    BRIG_CHANNEL_TYPE_UNORM_SHORT_101010 = 7,
    BRIG_CHANNEL_TYPE_SIGNED_INT8        = 8,
    BRIG_CHANNEL_TYPE_SIGNED_INT16       = 9,
    BRIG_CHANNEL_TYPE_SIGNED_INT32       = 10,
    BRIG_CHANNEL_TYPE_UNSIGNED_INT8      = 11,
    BRIG_CHANNEL_TYPE_UNSIGNED_INT16     = 12,
    BRIG_CHANNEL_TYPE_UNSIGNED_INT32     = 13,
    BRIG_CHANNEL_TYPE_HALF_FLOAT         = 14,
    BRIG_CHANNEL_TYPE_FLOAT              = 15,

    // used internally
    BRIG_CHANNEL_TYPE_UNKNOWN //.mnemo=""
};

enum BrigImageChannelOrder {
    //.mnemo={ s/^BRIG_CHANNEL_ORDER_?//;lc }
    //.mnemo_token=EImageOrder
    //.mnemo_context=EImageOrderContext
    BRIG_CHANNEL_ORDER_A             = 0,
    BRIG_CHANNEL_ORDER_R             = 1,
    BRIG_CHANNEL_ORDER_RX            = 2,
    BRIG_CHANNEL_ORDER_RG            = 3,
    BRIG_CHANNEL_ORDER_RGX           = 4,
    BRIG_CHANNEL_ORDER_RA            = 5,
    BRIG_CHANNEL_ORDER_RGB           = 6,
    BRIG_CHANNEL_ORDER_RGBX          = 7,
    BRIG_CHANNEL_ORDER_RGBA          = 8,
    BRIG_CHANNEL_ORDER_BGRA          = 9,
    BRIG_CHANNEL_ORDER_ARGB          = 10,
    BRIG_CHANNEL_ORDER_ABGR          = 11,
    BRIG_CHANNEL_ORDER_SRGB          = 12,
    BRIG_CHANNEL_ORDER_SRGBX         = 13,
    BRIG_CHANNEL_ORDER_SRGBA         = 14,
    BRIG_CHANNEL_ORDER_SBGRA         = 15,
    BRIG_CHANNEL_ORDER_INTENSITY     = 16,
    BRIG_CHANNEL_ORDER_LUMINANCE     = 17,
    BRIG_CHANNEL_ORDER_DEPTH         = 18,
    BRIG_CHANNEL_ORDER_DEPTH_STENCIL = 19,

    // used internally
    BRIG_CHANNEL_ORDER_UNKNOWN //.mnemo="" // used when no order is specified
};

enum BrigImageQuery {
    //.mnemo={ s/^BRIG_IMAGE_QUERY_//;lc }
    BRIG_IMAGE_QUERY_WIDTH = 0,
    BRIG_IMAGE_QUERY_HEIGHT = 1,
    BRIG_IMAGE_QUERY_DEPTH = 2,
    BRIG_IMAGE_QUERY_ARRAY = 3,
    BRIG_IMAGE_QUERY_CHANNELORDER = 4,
    BRIG_IMAGE_QUERY_CHANNELTYPE = 5
};

enum BrigSamplerQuery {
    //.mnemo={ s/^BRIG_SAMPLER_QUERY_//;lc }
    //.mnemo_token=_EMSamplerQuery
    BRIG_SAMPLER_QUERY_ADDRESSING = 0,
    BRIG_SAMPLER_QUERY_COORD = 1,
    BRIG_SAMPLER_QUERY_FILTER = 2
};

enum BrigInstKinds {
    //.wname={ s/^BRIG//; MACRO2Name($_) }
    //.sizeof=$wname{ "sizeof(Brig$wname)" }
    //.sizeof_switch //.sizeof_proto="int size_of_inst(unsigned arg)" //.sizeof_default="return -1"
    BRIG_INST_NONE          = 0,   //.skip // TBD remove skip

    BRIG_INST_BASIC         = 1,
    BRIG_INST_ADDR          = 2,
    BRIG_INST_ATOMIC        = 3,
    BRIG_INST_BR            = 4,
    BRIG_INST_CMP           = 5,
    BRIG_INST_CVT           = 6,
    BRIG_INST_IMAGE         = 7,
    BRIG_INST_LANE          = 8,
    BRIG_INST_MEM           = 9,
    BRIG_INST_MEM_FENCE     = 10,
    BRIG_INST_MOD           = 11,
    BRIG_INST_QUERY_IMAGE   = 12,
    BRIG_INST_QUERY_SAMPLER = 13,
    BRIG_INST_QUEUE         = 14,
    BRIG_INST_SEG           = 15,
    BRIG_INST_SEG_CVT       = 16,
    BRIG_INST_SIGNAL        = 17,
    BRIG_INST_SOURCE_TYPE   = 18
};

//BrigMachineModel was BrigMachine
enum BrigMachineModel {
    //.mnemo={ s/^BRIG_MACHINE_//; '$'.lc }
    //.mnemo_token=ETargetMachine
    BRIG_MACHINE_SMALL = 0, // 32-bit model (all addresses are 32 bits;
                            // a pointer fits into an s register)
    BRIG_MACHINE_LARGE = 1, // 64-bit model (all addresses are 64 bits;
                            // a pointer fits into a d register)

    BRIG_MACHINE_UNDEF = 2  //.skip
};

enum BrigMemoryOrder {
    //.mnemo={ s/^BRIG_MEMORY_ORDER_//; lc }
    //.mnemo_token=_EMMemoryOrder
    BRIG_MEMORY_ORDER_NONE = 0,                   //.mnemo=""
    BRIG_MEMORY_ORDER_RELAXED = 1,                //.mnemo=rlx
    BRIG_MEMORY_ORDER_ACQUIRE = 2,                //.mnemo=acq
    BRIG_MEMORY_ORDER_RELEASE = 3,                //.mnemo=rel
    BRIG_MEMORY_ORDER_ACQUIRE_RELEASE = 4         //.mnemo=ar
};


enum BrigMemoryScope {
    //.mnemo={ s/^BRIG_MEMORY_SCOPE_//; lc }
    //.mnemo_token=_EMMemoryScope
  BRIG_MEMORY_SCOPE_NONE = 0,                       //.mnemo=""
  BRIG_MEMORY_SCOPE_WAVEFRONT = 1,                  //.mnemo="wv" 
  BRIG_MEMORY_SCOPE_WORKGROUP = 2,                  //.mnemo="wg"
  BRIG_MEMORY_SCOPE_COMPONENT = 3,                  //.mnemo="cmp"
  BRIG_MEMORY_SCOPE_SYSTEM = 4,                     //.mnemo="sys"
  BRIG_MEMORY_SCOPE_WORKITEM = 5                    //.mnemo="wi"
};

enum BrigOpcode { //.tdcaption="Instruction Opcodes"
    //.k={ "BASIC" }
    //.pscode=$k{ MACRO2Name("_".$k) }
    //.opcodeparser=$pscode{ return $pscode && "parseMnemo$pscode" }
    //.opcodeparser_incfile=ParserUtilities
    //.opcodeparser_switch //.opcodeparser_proto="OpcodeParser getOpcodeParser(Brig::BrigOpcode16_t arg)" //.opcodeparser_default="return NULL"

    //.psopnd={'Operands'}
    //.opndparser=$psopnd{ return $psopnd && "&Parser::parse$psopnd" }
    //.opndparser_incfile=ParserUtilities
    //.opndparser_switch //.opndparser_proto="Parser::OperandParser Parser::getOperandParser(Brig::BrigOpcode16_t arg)" //.opndparser_default="return NULL"

    //.mnemo={ s/^BRIG_OPCODE_//; ; s/GCN([^_])/GCN_$1/; lc }
    //.mnemo_scanner=Instructions //.mnemo_token=EInstruction

    //.has_memory_order={undef}
    //.semsupport=$has_memory_order{ return $has_memory_order && "true" }

    //.hasType=$k{ return ($k and $k eq "BASIC_NO_TYPE" || $k eq "BR") ? "false" : undef; }
    //.hasType_switch  //.hasType_proto="bool instHasType(Brig::BrigOpcode16_t arg)" //.hasType_default="return true"

    //.opcodevis=$pscode{ "vis.template " . sprintf("%-47s(","visitOpcode<$_>") . ($pscode =~m/^(BasicOrMod|Nop)$/? "inst" : "HSAIL_ASM::Inst". ($pscode=~m/BasicNoType/? "Basic":$pscode) ."(inst)").")" }
    //.opcodevis_switch //.opcodevis_proto="template <typename RetType, typename Visitor> RetType visitOpcode_gen(HSAIL_ASM::Inst inst, Visitor& vis)"
    //.opcodevis_arg="inst.opcode()" //.opcodevis_default="return RetType()"
    //.opcodevis_incfile=ItemUtils

    //.ftz=$k{ return ($k eq "BASIC_OR_MOD" or $k eq "CMP" or $k eq "CVT") ? "true" : undef }
    //.ftz_incfile=ItemUtils //.ftz_switch //.ftz_proto="inline bool instSupportsFtz(Brig::BrigOpcode16_t arg)" //.ftz_default="return false"

    //.numdst={undef}
    //.numdst_switch //.numdst_proto="int instNumDstOperands(Brig::BrigOpcode16_t arg)" //.numdst_default="return 1"
    BRIG_OPCODE_NOP                     =    0,  //.k=NOP //.hasType=false //.psopnd=NoOperands //.numdst=0
    BRIG_OPCODE_CODEBLOCKEND            =    1,  //.k=BASIC_NO_TYPE
    BRIG_OPCODE_ABS                     =    2,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_ADD                     =    3,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_BORROW                  =    4,  
    BRIG_OPCODE_CARRY                   =    5,  
    BRIG_OPCODE_CEIL                    =    6,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_COPYSIGN                =    7,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_DIV                     =    8,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_FLOOR                   =    9,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_FMA                     =   10,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_FRACT                   =   11,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_MAD                     =   12,  
    BRIG_OPCODE_MAX                     =   13,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_MIN                     =   14,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_MUL                     =   15,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_MULHI                   =   16,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_NEG                     =   17,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_REM                     =   18,  
    BRIG_OPCODE_RINT                    =   19,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_SQRT                    =   20,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_SUB                     =   21,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_TRUNC                   =   22,  //.k=BASIC_OR_MOD
    BRIG_OPCODE_MAD24                   =   23,  
    BRIG_OPCODE_MAD24HI                 =   24,  
    BRIG_OPCODE_MUL24                   =   25,  
    BRIG_OPCODE_MUL24HI                 =   26,  
    BRIG_OPCODE_SHL                     =   27,  
    BRIG_OPCODE_SHR                     =   28,  
    BRIG_OPCODE_AND                     =   29,  
    BRIG_OPCODE_NOT                     =   30,  
    BRIG_OPCODE_OR                      =   31,  
    BRIG_OPCODE_POPCOUNT                =   32,  //.k=SOURCE_TYPE
    BRIG_OPCODE_XOR                     =   33,  
    BRIG_OPCODE_BITEXTRACT              =   34,  
    BRIG_OPCODE_BITINSERT               =   35,  
    BRIG_OPCODE_BITMASK                 =   36,  
    BRIG_OPCODE_BITREV                  =   37,  
    BRIG_OPCODE_BITSELECT               =   38,  
    BRIG_OPCODE_FIRSTBIT                =   39,  //.k=SOURCE_TYPE
    BRIG_OPCODE_LASTBIT                 =   40,  //.k=SOURCE_TYPE
    BRIG_OPCODE_COMBINE                 =   41,  //.k=SOURCE_TYPE    //.mnemo_token=EInstruction_Vx
    BRIG_OPCODE_EXPAND                  =   42,  //.k=SOURCE_TYPE    //.mnemo_token=EInstruction_Vx
    BRIG_OPCODE_LDA                     =   43,  //.k=ADDR
    BRIG_OPCODE_LDC                     =   44,  
    BRIG_OPCODE_MOV                     =   45,  
    BRIG_OPCODE_SHUFFLE                 =   46,  
    BRIG_OPCODE_UNPACKHI                =   47,  
    BRIG_OPCODE_UNPACKLO                =   48,  
    BRIG_OPCODE_PACK                    =   49,  //.k=SOURCE_TYPE
    BRIG_OPCODE_UNPACK                  =   50,  //.k=SOURCE_TYPE
    BRIG_OPCODE_CMOV                    =   51,  
    BRIG_OPCODE_CLASS                   =   52,  //.k=SOURCE_TYPE
    BRIG_OPCODE_NCOS                    =   53,  
    BRIG_OPCODE_NEXP2                   =   54,  
    BRIG_OPCODE_NFMA                    =   55,  
    BRIG_OPCODE_NLOG2                   =   56,  
    BRIG_OPCODE_NRCP                    =   57,  
    BRIG_OPCODE_NRSQRT                  =   58,  
    BRIG_OPCODE_NSIN                    =   59,  
    BRIG_OPCODE_NSQRT                   =   60,  
    BRIG_OPCODE_BITALIGN                =   61,  
    BRIG_OPCODE_BYTEALIGN               =   62,  
    BRIG_OPCODE_PACKCVT                 =   63,  //.k=SOURCE_TYPE
    BRIG_OPCODE_UNPACKCVT               =   64,  //.k=SOURCE_TYPE
    BRIG_OPCODE_LERP                    =   65,  
    BRIG_OPCODE_SAD                     =   66,  //.k=SOURCE_TYPE
    BRIG_OPCODE_SADHI                   =   67,  //.k=SOURCE_TYPE
    BRIG_OPCODE_SEGMENTP                =   68,  //.k=SEG_CVT
    BRIG_OPCODE_FTOS                    =   69,  //.k=SEG_CVT
    BRIG_OPCODE_STOF                    =   70,  //.k=SEG_CVT
    BRIG_OPCODE_CMP                     =   71,  //.k=CMP
    BRIG_OPCODE_CVT                     =   72,  //.k=CVT
    BRIG_OPCODE_LD                      =   73,  //.k=MEM               //.has_memory_order  //.mnemo_token=EInstruction_Vx
    BRIG_OPCODE_ST                      =   74,  //.k=MEM  //.numdst=0  //.has_memory_order  //.mnemo_token=EInstruction_Vx
    BRIG_OPCODE_ATOMIC                  =   75,  //.k=ATOMIC
    BRIG_OPCODE_ATOMICNORET             =   76,  //.k=ATOMIC    //.numdst=0
    BRIG_OPCODE_SIGNAL                  =   77,  //.k=SIGNAL
    BRIG_OPCODE_SIGNALNORET             =   78,  //.k=SIGNAL    //.numdst=0
    BRIG_OPCODE_MEMFENCE                =   79,  //.k=MEM_FENCE //.numdst=0
    BRIG_OPCODE_RDIMAGE                 =   80,  //.k=IMAGE             //.mnemo_token=EInstruction_Vx
    BRIG_OPCODE_LDIMAGE                 =   81,  //.k=IMAGE             //.mnemo_token=EInstruction_Vx
    BRIG_OPCODE_STIMAGE                 =   82,  //.k=IMAGE //.numdst=0 //.mnemo_token=EInstruction_Vx 
    BRIG_OPCODE_CBR                     =   83,  //.k=BR    //.numdst=0
    BRIG_OPCODE_BRN                     =   84,  //.k=BR    //.numdst=0
    BRIG_OPCODE_BARRIER                 =   85,  //.k=BR    //.numdst=0
    BRIG_OPCODE_WAVEBARRIER             =   86,  //.k=BR    //.numdst=0
    BRIG_OPCODE_ARRIVEFBAR              =   87,  //.k=BR    //.numdst=0
    BRIG_OPCODE_INITFBAR                =   88,  //.k=BASIC_NO_TYPE //.numdst=0
    BRIG_OPCODE_JOINFBAR                =   89,  //.k=BR    //.numdst=0
    BRIG_OPCODE_LEAVEFBAR               =   90,  //.k=BR    //.numdst=0
    BRIG_OPCODE_RELEASEFBAR             =   91,  //.k=BASIC_NO_TYPE //.numdst=0
    BRIG_OPCODE_WAITFBAR                =   92,  //.k=BR    //.numdst=0
    BRIG_OPCODE_LDF                     =   93,
    BRIG_OPCODE_ACTIVELANECOUNT         =   94,  //.k=LANE
    BRIG_OPCODE_ACTIVELANEID            =   95,  //.k=LANE
    BRIG_OPCODE_ACTIVELANEMASK          =   96,  //.k=LANE              //.mnemo_token=EInstruction_Vx
    BRIG_OPCODE_ACTIVELANESHUFFLE       =   97,  //.k=LANE
    BRIG_OPCODE_CALL                    =   98,  //.k=BR //.psopnd=CallOperands //.numdst=0
    BRIG_OPCODE_RET                     =   99,  //.k=BASIC_NO_TYPE
    BRIG_OPCODE_ALLOCA                  =  100,  //.k=MEM
    BRIG_OPCODE_ADDQUEUEWRITEINDEX      =  101,  //.k=QUEUE
    BRIG_OPCODE_CASQUEUEWRITEINDEX      =  102,  //.k=QUEUE
    BRIG_OPCODE_CLEARDETECTEXCEPT       =  103,            //.numdst=0
    BRIG_OPCODE_CLOCK                   =  104,
    BRIG_OPCODE_CUID                    =  105,
    BRIG_OPCODE_CURRENTWORKGROUPSIZE    =  106,
    BRIG_OPCODE_DEBUGTRAP               =  107,            //.numdst=0
    BRIG_OPCODE_DIM                     =  108,
    BRIG_OPCODE_GETDETECTEXCEPT         =  109,
    BRIG_OPCODE_GRIDGROUPS              =  110,
    BRIG_OPCODE_GRIDSIZE                =  111,
    BRIG_OPCODE_LANEID                  =  112,
    BRIG_OPCODE_LDQUEUEREADINDEX        =  113,  //.k=QUEUE
    BRIG_OPCODE_LDQUEUEWRITEINDEX       =  114,  //.k=QUEUE
    BRIG_OPCODE_MAXCUID                 =  115,
    BRIG_OPCODE_MAXWAVEID               =  116,
    BRIG_OPCODE_NULLPTR                 =  117,  //.k=SEG
    BRIG_OPCODE_PACKETCOMPLETIONSIG     =  118,
    BRIG_OPCODE_PACKETID                =  119,  
    BRIG_OPCODE_QUEUEID                 =  120,
    BRIG_OPCODE_QUEUEPTR                =  121,  //.k=SEG
    BRIG_OPCODE_SERVICEQUEUEPTR         =  122,  //.k=SEG
    BRIG_OPCODE_SETDETECTEXCEPT         =  123,              //.numdst=0
    BRIG_OPCODE_STQUEUEREADINDEX        =  124,  //.k=QUEUE  //.numdst=0
    BRIG_OPCODE_STQUEUEWRITEINDEX       =  125,  //.k=QUEUE  //.numdst=0
    BRIG_OPCODE_WAVEID                  =  126,
    BRIG_OPCODE_WORKGROUPID             =  127,
    BRIG_OPCODE_WORKGROUPSIZE           =  128,
    BRIG_OPCODE_WORKITEMABSID           =  129,
    BRIG_OPCODE_WORKITEMFLATABSID       =  130,
    BRIG_OPCODE_WORKITEMFLATID          =  131,
    BRIG_OPCODE_WORKITEMID              =  132,
    BRIG_OPCODE_QUERYIMAGE              =  133,  //.k=QUERY_IMAGE
    BRIG_OPCODE_QUERYSAMPLER            =  134,  //.k=QUERY_SAMPLER

    BRIG_OPCODE_GCNMADU        = (1u << 15) |  0, //.k=BASIC_NO_TYPE
    BRIG_OPCODE_GCNMADS        = (1u << 15) |  1, //.k=BASIC_NO_TYPE
    BRIG_OPCODE_GCNMAX3        = (1u << 15) |  2, 
    BRIG_OPCODE_GCNMIN3        = (1u << 15) |  3, 
    BRIG_OPCODE_GCNMED3        = (1u << 15) |  4, 
    BRIG_OPCODE_GCNFLDEXP      = (1u << 15) |  5, 
    BRIG_OPCODE_GCNFREXP_EXP   = (1u << 15) |  6, 
    BRIG_OPCODE_GCNFREXP_MANT  = (1u << 15) |  7, 
    BRIG_OPCODE_GCNTRIG_PREOP  = (1u << 15) |  8, 
    BRIG_OPCODE_GCNBFM         = (1u << 15) |  9, 
    BRIG_OPCODE_GCNLD          = (1u << 15) | 10, //.k=MEM               //.has_memory_order  //.mnemo_token=EInstruction_Vx
    BRIG_OPCODE_GCNST          = (1u << 15) | 11, //.k=MEM               //.has_memory_order  //.mnemo_token=EInstruction_Vx
    BRIG_OPCODE_GCNATOMIC      = (1u << 15) | 12, //.k=ATOMIC
    BRIG_OPCODE_GCNATOMICNORET = (1u << 15) | 13, //.k=ATOMIC        //.mnemo=gcn_atomicNoRet
    BRIG_OPCODE_GCNSLEEP       = (1u << 15) | 14,          
    BRIG_OPCODE_GCNPRIORITY    = (1u << 15) | 15,          
    BRIG_OPCODE_GCNREGIONALLOC = (1u << 15) | 16, //.k=BASIC_NO_TYPE //.mnemo=gcn_region_alloc
    BRIG_OPCODE_GCNMSAD        = (1u << 15) | 17,          
    BRIG_OPCODE_GCNQSAD        = (1u << 15) | 18,          
    BRIG_OPCODE_GCNMQSAD       = (1u << 15) | 19,          
    BRIG_OPCODE_GCNMQSAD4      = (1u << 15) | 20, //.k=BASIC_NO_TYPE
    BRIG_OPCODE_GCNSADW        = (1u << 15) | 21,          
    BRIG_OPCODE_GCNSADD        = (1u << 15) | 22,          
    BRIG_OPCODE_GCNCONSUME     = (1u << 15) | 23, //.k=ADDR          //.mnemo=gcn_atomic_consume
    BRIG_OPCODE_GCNAPPEND      = (1u << 15) | 24, //.k=ADDR          //.mnemo=gcn_atomic_append
    BRIG_OPCODE_GCNB4XCHG      = (1u << 15) | 25,                    //.mnemo=gcn_b4xchg
    BRIG_OPCODE_GCNB32XCHG     = (1u << 15) | 26,                    //.mnemo=gcn_b32xchg
    BRIG_OPCODE_GCNMAX         = (1u << 15) | 27, 
    BRIG_OPCODE_GCNMIN         = (1u << 15) | 28,
};

enum BrigOperandKinds {
    //.wname={ s/^BRIG//; MACRO2Name($_) }
    //.sizeof=$wname{ "sizeof(Brig$wname)" }
    //.sizeof_switch //.sizeof_proto="int size_of_operand(unsigned arg)" //.sizeof_default="return -1"

    BRIG_OPERAND_IMMED              = 0,
    BRIG_OPERAND_WAVESIZE           = 1,
    BRIG_OPERAND_REG                = 2,
    BRIG_OPERAND_VECTOR             = 3,
    BRIG_OPERAND_ADDRESS            = 4,
    BRIG_OPERAND_ARGUMENT_LIST      = 5,
    BRIG_OPERAND_FUNCTION_LIST      = 6,
    BRIG_OPERAND_FBARRIER_REF       = 7,
    BRIG_OPERAND_FUNCTION_REF       = 8,
    BRIG_OPERAND_LABEL_REF          = 9,
    BRIG_OPERAND_LABEL_TARGETS_REF  = 10,
    BRIG_OPERAND_SIGNATURE_REF      = 11,
    BRIG_OPERAND_LABEL_VARIABLE_REF = 12
};

enum BrigPack { //.tdcaption="Packing"
    //.mnemo={ s/^BRIG_PACK_//;s/SAT$/_sat/;lc }
    //.mnemo_token=_EMPacking
    BRIG_PACK_NONE  = 0, //.mnemo=""
    BRIG_PACK_PP    = 1,
    BRIG_PACK_PS    = 2,
    BRIG_PACK_SP    = 3,
    BRIG_PACK_SS    = 4,
    BRIG_PACK_S     = 5,
    BRIG_PACK_P     = 6,
    BRIG_PACK_PPSAT = 7,
    BRIG_PACK_PSSAT = 8,
    BRIG_PACK_SPSAT = 9,
    BRIG_PACK_SSSAT = 10,
    BRIG_PACK_SSAT  = 11,
    BRIG_PACK_PSAT  = 12
};

enum BrigProfile {
    //.mnemo={ s/^BRIG_PROFILE_//;'$'.lc }
    //.mnemo_token=ETargetProfile
    BRIG_PROFILE_BASE = 0,
    BRIG_PROFILE_FULL = 1,

    BRIG_PROFILE_UNDEF = 2  //.skip
};

enum BrigSegment {
    //.mnemo={ s/^BRIG_SEGMENT_//;lc}
    //.mnemo_token=_EMSegment
    //.mnemo_context=EInstModifierContext
    BRIG_SEGMENT_NONE     = 0, //.mnemo=""
    BRIG_SEGMENT_FLAT     = 1, //.mnemo=""
    BRIG_SEGMENT_GLOBAL   = 2,
    BRIG_SEGMENT_READONLY = 3,
    BRIG_SEGMENT_KERNARG  = 4,
    BRIG_SEGMENT_GROUP    = 5,
    BRIG_SEGMENT_PRIVATE  = 6,
    BRIG_SEGMENT_SPILL    = 7,
    BRIG_SEGMENT_ARG      = 8,
    BRIG_SEGMENT_EXTSPACE0 = 9, //.mnemo="region"
};

enum BrigMemoryFenceSegments {
    //.mnemo={ s/^BRIG_MEMORY_FENCE_//;lc }
    //.mnemo_token=_EMMemoryFenceSegments
    //.mnemo_context=EInstModifierInstFenceContext
    BRIG_MEMORY_FENCE_NONE   = 0, //.skip
    BRIG_MEMORY_FENCE_GROUP  = 1,
    BRIG_MEMORY_FENCE_GLOBAL = 2,
    BRIG_MEMORY_FENCE_BOTH   = 3, //.mnemo=""
    BRIG_MEMORY_FENCE_IMAGE  = 4
};

enum BrigWidth {
    BRIG_WIDTH_NONE = 0,
    BRIG_WIDTH_1 = 1,
    BRIG_WIDTH_2 = 2,
    BRIG_WIDTH_4 = 3,
    BRIG_WIDTH_8 = 4,
    BRIG_WIDTH_16 = 5,
    BRIG_WIDTH_32 = 6,
    BRIG_WIDTH_64 = 7,
    BRIG_WIDTH_128 = 8,
    BRIG_WIDTH_256 = 9,
    BRIG_WIDTH_512 = 10,
    BRIG_WIDTH_1024 = 11,
    BRIG_WIDTH_2048 = 12,
    BRIG_WIDTH_4096 = 13,
    BRIG_WIDTH_8192 = 14,
    BRIG_WIDTH_16384 = 15,
    BRIG_WIDTH_32768 = 16,
    BRIG_WIDTH_65536 = 17,
    BRIG_WIDTH_131072 = 18,
    BRIG_WIDTH_262144 = 19,
    BRIG_WIDTH_524288 = 20,
    BRIG_WIDTH_1048576 = 21,
    BRIG_WIDTH_2097152 = 22,
    BRIG_WIDTH_4194304 = 23,
    BRIG_WIDTH_8388608 = 24,
    BRIG_WIDTH_16777216 = 25,
    BRIG_WIDTH_33554432 = 26,
    BRIG_WIDTH_67108864 = 27,
    BRIG_WIDTH_134217728 = 28,
    BRIG_WIDTH_268435456 = 29,
    BRIG_WIDTH_536870912 = 30,
    BRIG_WIDTH_1073741824 = 31,
    BRIG_WIDTH_2147483648 = 32,
    BRIG_WIDTH_WAVESIZE = 33,
    BRIG_WIDTH_ALL = 34
};

// subitems

enum BrigRound {
    //.mnemo
    //.mnemo_fn=round2str //.mnemo_token=_EMRound
    BRIG_ROUND_NONE = 0,
    BRIG_ROUND_FLOAT_NEAR_EVEN = 1,                //.mnemo=near
    BRIG_ROUND_FLOAT_ZERO = 2,                     //.mnemo=zero
    BRIG_ROUND_FLOAT_PLUS_INFINITY = 3,            //.mnemo=up
    BRIG_ROUND_FLOAT_MINUS_INFINITY = 4,           //.mnemo=down
    BRIG_ROUND_INTEGER_NEAR_EVEN = 5,              //.mnemo=neari
    BRIG_ROUND_INTEGER_ZERO = 6,                   //.mnemo=zeroi
    BRIG_ROUND_INTEGER_PLUS_INFINITY = 7,          //.mnemo=upi
    BRIG_ROUND_INTEGER_MINUS_INFINITY = 8,         //.mnemo=downi
    BRIG_ROUND_INTEGER_NEAR_EVEN_SAT = 9,          //.mnemo=neari_sat
    BRIG_ROUND_INTEGER_ZERO_SAT = 10,              //.mnemo=zeroi_sat
    BRIG_ROUND_INTEGER_PLUS_INFINITY_SAT = 11,     //.mnemo=upi_sat
    BRIG_ROUND_INTEGER_MINUS_INFINITY_SAT = 12     //.mnemo=downi_sat
};

//BrigAluModifierMask was BrigAluModBits
enum BrigAluModifierMask {
    BRIG_ALU_ROUND = 15,
    BRIG_ALU_FTZ = 16
};

struct BrigAluModifier { //.isroot //.standalone
    BrigAluModifier16_t allBits; //.defValue=0
    //^^ BrigRound8_t   round;   //.wtype=BFValRef<Brig::BrigRound8_t,0,4>
    //^^ bool           ftz;     //.wtype=BitValRef<4>
};


//BrigSymbolModifierMask was BrigSymbolModifier
enum BrigSymbolModifierMask {
    //.nodump
    BRIG_SYMBOL_LINKAGE     = 3,
    BRIG_SYMBOL_DECLARATION = 4,
    BRIG_SYMBOL_CONST       = 8,
    BRIG_SYMBOL_ARRAY       = 16,
    BRIG_SYMBOL_FLEX_ARRAY  = 32
};

//BrigLinkage was BrigAttribute
enum BrigLinkage {
    //.mnemo={ s/^BRIG_LINKAGE_//;s/NONE//;lc }
    BRIG_LINKAGE_NONE = 0,
    BRIG_LINKAGE_STATIC = 1,
    BRIG_LINKAGE_EXTERN = 2
};

struct BrigSymbolModifier { //.isroot //.standalone
    BrigSymbolModifier8_t allBits; //.defValue=0
    //^^ BrigLinkage linkage;       //.wtype=BFValRef<Brig::BrigLinkage8_t,0,2>
    //^^ bool        isDeclaration; //.wtype=BitValRef<2>
    //^^ bool        isConst;       //.wtype=BitValRef<3>
    //^^ bool        isArray;       //.wtype=BitValRef<4>
    //^^ bool        isFlexArray;   //.wtype=BitValRef<5>
};

enum BrigExecuteableModifierMask {
    //.nodump
    BRIG_EXECUTABLE_LINKAGE = 3,
    BRIG_EXECUTABLE_DECLARATION = 4
};

struct BrigExecutableModifier { //.isroot //.standalone
    BrigExecutableModifier8_t allBits; //.defValue=0
    //^^ BrigLinkage linkage;       //.wtype=BFValRef<Brig::BrigLinkage8_t,0,2>
    //^^ bool        isDeclaration; //.wtype=BitValRef<2>
};

//BrigSamplerFilter was BrigAddrFilter
enum BrigSamplerFilter {
    //.mnemo={ s/^BRIG_FILTER_//;lc }
    BRIG_FILTER_NEAREST = 0,
    BRIG_FILTER_LINEAR = 1
};

enum BrigMemoryModifierMask {
    BRIG_MEMORY_CONST = 1
};

struct BrigMemoryModifier { //.isroot //.standalone
    BrigMemoryModifier8_t      allBits;   //.defValue=0
    //^^ bool                  isConst;   //.wtype=BitValRef<0>
};

struct BrigSegCvtModifier { //.isroot //.standalone
    BrigSegCvtModifier8_t      allBits;   //.defValue=0
    //^^ bool                  isNoNull;    //.wtype=BitValRef<0>
};

enum BrigSegCvtModifierMask {
  BRIG_SEG_CVT_NONULL = 1                         //.mnemo="nonull"
};

enum BrigAlignment {
    BRIG_ALIGNMENT_NONE = 0,
    BRIG_ALIGNMENT_1 = 1,
    BRIG_ALIGNMENT_2 = 2,
    BRIG_ALIGNMENT_4 = 3,
    BRIG_ALIGNMENT_8 = 4,
    BRIG_ALIGNMENT_16 = 5,
    BRIG_ALIGNMENT_32 = 6,
    BRIG_ALIGNMENT_64 = 7,
    BRIG_ALIGNMENT_128 = 8,
    BRIG_ALIGNMENT_256 = 9,

    BRIG_ALIGNMENT_LAST, //.skip
    BRIG_ALIGNMENT_MAX = BRIG_ALIGNMENT_LAST - 1 //.skip
};

enum BrigSamplerCoordNormalization {
    //.mnemo={ s/^BRIG_COORD_//;lc }
    //.mnemo_token=ESamplerCoord
    BRIG_COORD_UNNORMALIZED = 0,
    BRIG_COORD_NORMALIZED = 1
};

struct BrigString {
    //.nowrap
    uint32_t byteCount;
    uint8_t  bytes[1];
};


// ============================================================================
//  Directives

struct BrigDirectiveBase {
    //.nowrap
    uint16_t               size;
    BrigDirectiveKinds16_t kind;
    BrigCodeOffset32_t     code;
};


/// @addtogroup Directives
/// @{
/// base class for all directive items.
struct BrigDirective { //.generic //.isroot
    /// item size.
    uint16_t               size;
    /// item kind. One of BrigDirectiveKinds enum values.
    BrigDirectiveKinds16_t kind;
};

/// base class for directives that positioned in instruction stream.
struct BrigDirectiveCode { //.generic //.parent=BrigDirective
    uint16_t               size;
    BrigDirectiveKinds16_t kind;
    /// Location in the instruction stream corresponding to this directive.
    BrigCodeOffset32_t     code;
};

struct BrigDirectiveVersion { //.parent=BrigDirectiveCode
    uint16_t               size;
    BrigDirectiveKinds16_t kind;
    BrigCodeOffset32_t     code;
    BrigVersion32_t        hsailMajor;  //.novisit // TBD095 make visitor
    BrigVersion32_t        hsailMinor;  //.novisit // TBD095 make visitor
    BrigVersion32_t        brigMajor;   //.novisit // TBD095 make visitor
    BrigVersion32_t        brigMinor;   //.novisit // TBD095 make visitor
    BrigProfile8_t         profile;
    BrigMachineModel8_t    machineModel;
    uint16_t reserved;                  //.defValue=0
};

/// start block of data.
struct BrigBlockStart { //.parent=BrigDirectiveCode //.enum=BRIG_DIRECTIVE_BLOCK_START
    uint16_t               size;
    BrigDirectiveKinds16_t kind;
    BrigCodeOffset32_t     code;
    BrigStringOffset32_t   name;
};

/// numeric data inside block.
struct BrigBlockNumeric { //.parent=BrigDirective //.enum=BRIG_DIRECTIVE_BLOCK_NUMERIC
    uint16_t               size;
    BrigDirectiveKinds16_t kind;
    BrigType16_t           type;
    uint16_t reserved;                      //.defValue=0
    uint32_t               elementCount;    //.defValue=0
    BrigDataOffset32_t     data;            //.wtype=DataItemRef //.novisit
    //^^ BrigDataOffset32_t dataAs;         //.wspecial=DataItemRefT //.wspecialgeneric //.novisit
};

/// string inside block.
struct BrigBlockString { //.parent=BrigDirective //.enum=BRIG_DIRECTIVE_BLOCK_STRING
    uint16_t               size;
    BrigDirectiveKinds16_t kind;
    BrigStringOffset32_t   string;
};

/// end of block.
struct BrigBlockEnd { //.parent=BrigDirective //.enum=BRIG_DIRECTIVE_BLOCK_END
    uint16_t size;
    BrigDirectiveKinds16_t kind;
};

/// comment directive.
struct BrigDirectiveComment { //.parent=BrigDirectiveCode
    uint16_t               size;
    BrigDirectiveKinds16_t kind;
    BrigCodeOffset32_t     code;
    BrigStringOffset32_t   name;
};

/// control directive.
struct BrigDirectiveControl { //.parent=BrigDirectiveCode
    uint16_t                 size;
    BrigDirectiveKinds16_t   kind;
    BrigCodeOffset32_t       code;
    BrigControlDirective16_t control;
    BrigType16_t             type;
    uint16_t reserved;                   //.defValue=0
    uint16_t                 valueCount; //.wname=elementCount //.defValue=0
    BrigOperandOffset32_t    values[1];  //.wtype=ItemRef<Operand> //.wspecial=ControlValues
};

/// @}
/// extension directive.
struct BrigDirectiveExtension { //.parent=BrigDirectiveCode
    uint16_t               size;
    BrigDirectiveKinds16_t kind;
    BrigCodeOffset32_t     code;
    BrigStringOffset32_t   name;
};

struct BrigDirectiveCallableBase { //.generic //.parent=BrigDirectiveCode
    uint16_t               size;
    BrigDirectiveKinds16_t kind;
    BrigCodeOffset32_t     code;
    BrigStringOffset32_t   name;
    uint16_t               inArgCount;
    uint16_t               outArgCount;
};

/// common ancestor class for kernel/function directives.
struct BrigDirectiveExecutable { //.generic //.parent=BrigDirectiveCallableBase
    uint16_t                  size;
    BrigDirectiveKinds16_t    kind;
    BrigCodeOffset32_t        code;
    BrigStringOffset32_t      name;
    uint16_t                  inArgCount;
    uint16_t                  outArgCount;
    BrigDirectiveOffset32_t   firstInArg;
    BrigDirectiveOffset32_t   firstScopedDirective;
    BrigDirectiveOffset32_t   nextTopLevelDirective;
    uint32_t                  instCount;
    BrigExecutableModifier    modifier; //.acc=subItem<ExecutableModifier> //.wtype=ExecutableModifier
    uint8_t reserved[3];                //.defValue=0
};

/// function directive.
struct BrigDirectiveFunction { //.parent=BrigDirectiveExecutable
    uint16_t                  size;
    BrigDirectiveKinds16_t    kind;
    BrigCodeOffset32_t        code;
    BrigStringOffset32_t      name;
    uint16_t                  inArgCount;            //.defValue=0
    uint16_t                  outArgCount;           //.defValue=0
    BrigDirectiveOffset32_t   firstInArg;
    BrigDirectiveOffset32_t   firstScopedDirective;
    BrigDirectiveOffset32_t   nextTopLevelDirective;
    uint32_t                  instCount;             //.defValue=0
    BrigExecutableModifier    modifier;              //.acc=subItem<ExecutableModifier> //.wtype=ExecutableModifier
    uint8_t reserved[3];                             //.defValue=0
};

/// kernel directive.
struct BrigDirectiveKernel { //.parent=BrigDirectiveExecutable
    uint16_t                  size;
    BrigDirectiveKinds16_t    kind;
    BrigCodeOffset32_t        code;
    BrigStringOffset32_t      name;
    uint16_t                  inArgCount;            //.defValue=0
    uint16_t                  outArgCount;           //.defValue=0
    BrigDirectiveOffset32_t   firstInArg;
    BrigDirectiveOffset32_t   firstScopedDirective;
    BrigDirectiveOffset32_t   nextTopLevelDirective;
    uint32_t                  instCount;             //.defValue=0
    BrigExecutableModifier    modifier;              //.acc=subItem<ExecutableModifier> //.wtype=ExecutableModifier
    uint8_t reserved[3];                             //.defValue=0
};

/// element describing properties of function signature argument.
struct BrigDirectiveSignatureArgument { //.isroot //.standalone
    BrigType16_t          type;
    BrigAlignment8_t      align;
    BrigSymbolModifier    modifier; //.acc=subItem<SymbolModifier> //.wtype=SymbolModifier
    uint32_t              dimLo;
    //^^ uint64_t         dim;      //.wtype=ValRef<uint64_t> //.acc=reinterpretValRef<uint64_t>
    uint32_t              dimHi;
};

/// function signature.
struct BrigDirectiveSignature { //.parent=BrigDirectiveCallableBase
    uint16_t                       size;
    BrigDirectiveKinds16_t         kind;
    BrigCodeOffset32_t             code;
    BrigStringOffset32_t           name;
    uint16_t                       inArgCount;  //.defValue=0 //.wname=inCount
    uint16_t                       outArgCount; //.defValue=0 //.wname=outCount
    BrigDirectiveSignatureArgument args[1]; //.wtype=DirectiveSignatureArgument //.acc=subItem<DirectiveSignatureArgument> //.wspecial=DirectiveSignatureArguments
};

struct BrigDirectiveVariable { //.parent=BrigDirectiveCode
    uint16_t                size;
    BrigDirectiveKinds16_t  kind;
    BrigCodeOffset32_t      code;
    BrigStringOffset32_t    name;
    BrigDirectiveOffset32_t init;     //.wtype=ItemRef<Directive> // it might be DirectiveVariableInit or DirectiveLabelInit
    BrigType16_t            type;
    BrigSegment8_t          segment;
    BrigAlignment8_t        align;    //.defValue=0
    uint32_t                dimLo;    //.defValue=0
    //^^ uint64_t           dim;      //.wtype=ValRef<uint64_t> //.acc=reinterpretValRef<uint64_t>
    uint32_t                dimHi;    //.defValue=0
    BrigSymbolModifier      modifier; //.acc=subItem<SymbolModifier> //.wtype=SymbolModifier
    uint8_t reserved[3];              //.defValue=0
};

struct BrigDirectiveVariableInit { //.parent=BrigDirectiveCode
    uint16_t                size;
    BrigDirectiveKinds16_t  kind;
    BrigCodeOffset32_t      code;
    BrigDataOffset32_t      data;         //.wtype=DataItemRef //.novisit
    //^^ BrigDataOffset32_t dataAs;       //.wspecial=DataItemRefT //.wspecialgeneric //.novisit
    uint32_t                elementCount; //.defValue=0
    BrigType16_t            type;
    uint16_t reserved;                    //.defValue=0
};

struct BrigDirectiveOpaqueInit { //.generic //.parent=BrigDirectiveCode
    uint16_t                size;
    BrigDirectiveKinds16_t  kind;
    BrigCodeOffset32_t      code;
    uint16_t                imageCount; //.defValue=0
    //^^ uint16_t           elementCount;
    uint16_t reserved;                  //.defValue=0
    BrigDirectiveOffset32_t objects[1];  //.wtype=ItemRef<Directive> //.wspecial=OpaqueInitList
};

struct BrigDirectiveImageInit { //.parent=BrigDirectiveOpaqueInit
    uint16_t                size;
    BrigDirectiveKinds16_t  kind;
    BrigCodeOffset32_t      code;
    uint16_t                imageCount; //.defValue=0
    //^^ uint16_t           elementCount;
    uint16_t reserved;                  //.defValue=0
    BrigDirectiveOffset32_t images[1];  //.wtype=ItemRef<DirectiveImageProperties> //.wspecial=ImageInitList
};

struct BrigDirectiveImageProperties { //.parent=BrigDirectiveCode
    uint16_t                 size;
    BrigDirectiveKinds16_t   kind;
    BrigCodeOffset32_t       code;
    uint32_t                 width;  //.defValue=0
    uint32_t                 height; //.defValue=0
    uint32_t                 depth;  //.defValue=0
    uint32_t                 array;  //.defValue=0
    BrigImageGeometry8_t     geometry;
    BrigImageChannelOrder8_t channelOrder;
    BrigImageChannelType8_t  channelType;
    uint8_t reserved;               //.defValue=0
};

struct BrigDirectiveSamplerInit { //.parent=BrigDirectiveOpaqueInit
    uint16_t                   size;
    BrigDirectiveKinds16_t     kind;
    BrigCodeOffset32_t         code;
    uint16_t                   samplerCount; //.defValue=0
    //^^ uint16_t              elementCount;
    uint16_t                   reserved;     //.defValue=0
    BrigDirectiveOffset32_t    samplers[1];  //.wtype=ItemRef<DirectiveSamplerProperties> //.wspecial=SamplerInitList
};

struct BrigDirectiveSamplerProperties { //.parent=BrigDirectiveCode
    uint16_t                         size;
    BrigDirectiveKinds16_t           kind;
    BrigCodeOffset32_t               code;
    BrigSamplerCoordNormalization8_t coord;
    BrigSamplerFilter8_t             filter;
    BrigSamplerAddressing8_t         addressing;
    uint8_t reserved;                               //.defValue=0
};

/// label directive
struct BrigDirectiveLabel { //.parent=BrigDirectiveCode
    uint16_t                size;
    BrigDirectiveKinds16_t  kind;
    BrigCodeOffset32_t      code;
    BrigStringOffset32_t    name;
};

// \todo: this should be renamed BrigDirectiveLabelList?
struct BrigDirectiveLabelTargets { //.parent=BrigDirectiveCode
    uint16_t                size;
    BrigDirectiveKinds16_t  kind;
    BrigCodeOffset32_t      code;
    BrigStringOffset32_t    name;       //.wname=name
    uint16_t                labelCount; //.wname=elementCount //.defValue=0
    uint16_t reserved;                  //.defValue=0
    BrigDirectiveOffset32_t labels[1];  //.wtype=ItemRef<DirectiveLabel> //.wspecial=LabelTargetsList
};

struct BrigDirectiveLabelInit { //.parent=BrigDirectiveCode
    uint16_t                size;
    BrigDirectiveKinds16_t  kind;
    BrigCodeOffset32_t      code;
    uint16_t                labelCount; //.wname=elementCount //.defValue=0
    uint16_t reserved;                  //.defValue=0
    BrigDirectiveOffset32_t labels[1];  //.wtype=ItemRef<DirectiveLabel> //.wspecial=LabelInitList
};

struct BrigDirectiveLoc { //.parent=BrigDirectiveCode
    uint16_t                size;
    BrigDirectiveKinds16_t  kind;
    BrigCodeOffset32_t      code;
    BrigStringOffset32_t    filename;
    uint32_t                line;
    uint32_t                column; //.defValue=1
};

struct BrigDirectivePragma { //.parent=BrigDirectiveCode
    uint16_t                size;
    BrigDirectiveKinds16_t  kind;
    BrigCodeOffset32_t      code;
    BrigStringOffset32_t    name;
};

struct BrigDirectiveArgScopeStart { //.parent=BrigDirectiveCode
    uint16_t               size;
    BrigDirectiveKinds16_t kind;
    BrigCodeOffset32_t     code;
};

struct BrigDirectiveArgScopeEnd { //.parent=BrigDirectiveCode
    uint16_t               size;
    BrigDirectiveKinds16_t kind;
    BrigCodeOffset32_t     code;
};

// new
struct BrigDirectiveFbarrier { //.parent=BrigDirectiveCode
    uint16_t               size;
    BrigDirectiveKinds16_t kind;
    BrigCodeOffset32_t     code;
    BrigStringOffset32_t   name;
};


// ============================================================================
//  Instructions

struct BrigInst { //.generic //.isroot
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;
    BrigOperandOffset32_t operands[5]; //.wname=operand
};

struct BrigInstBase {
    //.nowrap
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;
    BrigOperandOffset32_t operands[5];
};

struct BrigInstAtomic {
    uint16_t               size;
    BrigInstKinds16_t      kind;
    BrigOpcode16_t         opcode;
    BrigType16_t           type;
    BrigOperandOffset32_t  operands[5]; //.wname=operand

    BrigSegment8_t         segment;
    BrigMemoryOrder8_t     memoryOrder;
    BrigMemoryScope8_t     memoryScope;
    BrigAtomicOperation8_t atomicOperation;
    uint8_t                equivClass;
    uint8_t reserved[3];                    //.defValue=0
};

struct BrigInstMemFence {
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;        //.defValue=Brig::BRIG_TYPE_NONE
    BrigOperandOffset32_t operands[5]; //.wname=operand

    BrigMemoryFenceSegments8_t segments;
    BrigMemoryOrder8_t    memoryOrder;
    BrigMemoryScope8_t    memoryScope;
    uint8_t reserved;                    //.defValue=0
};

struct BrigInstBasic {
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;
    BrigOperandOffset32_t operands[5];  //.wname=operand

    //uint8_t reserved[3];
};

struct BrigInstMod {
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;
    BrigOperandOffset32_t operands[5];  //.wname=operand

    BrigAluModifier       modifier;     //.acc=subItem<AluModifier> //.wtype=AluModifier
    BrigPack8_t           pack;

    uint8_t reserved;                   //.defValue=0
};

struct BrigInstCmp {
    uint16_t                size;
    BrigInstKinds16_t       kind;
    BrigOpcode16_t          opcode;
    BrigType16_t            type;
    BrigOperandOffset32_t   operands[5]; //.wname=operand

    BrigType16_t            sourceType;
    BrigAluModifier         modifier;    //.acc=subItem<AluModifier> //.wtype=AluModifier
    BrigCompareOperation8_t compare;
    BrigPack8_t             pack;

    uint16_t reserved;                   //.defValue=0
};

struct BrigInstCvt {
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;
    BrigOperandOffset32_t operands[5]; //.wname=operand

    BrigType16_t          sourceType;
    BrigAluModifier       modifier;    //.acc=subItem<AluModifier> //.wtype=AluModifier
};

struct BrigInstImage {
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;
    BrigOperandOffset32_t operands[5]; //.wname=operand

    BrigType16_t          imageType;
    BrigType16_t          coordType;
    BrigImageGeometry8_t  geometry;
    uint8_t               equivClass;

    uint16_t reserved; //.defValue=0
};

struct BrigInstQueryImage {
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;
    BrigOperandOffset32_t operands[5];
    BrigType16_t          imageType;
    BrigImageGeometry8_t  geometry;
    BrigImageQuery8_t     imageQuery;
};

struct BrigInstQuerySampler {
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;
    BrigOperandOffset32_t operands[5];
    BrigSamplerQuery8_t   samplerQuery;
    uint8_t reserved[3];                 //.defValue=0
};

struct BrigInstMem {
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;
    BrigOperandOffset32_t operands[5];  //.wname=operand
    BrigSegment8_t        segment;
    BrigAlignment8_t      align;
    uint8_t               equivClass;
    BrigWidth8_t          width;
    BrigMemoryModifier    modifier;     //.acc=subItem<MemoryModifier> //.wtype=MemoryModifier
    uint8_t reserved[3];                //.defValue=0
};

struct BrigInstAddr {
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;
    BrigOperandOffset32_t operands[5]; //.wname=operand

    BrigSegment8_t        segment;

    uint8_t reserved[3];               //.defValue=0
};

// new
struct BrigInstSourceType {
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;
    BrigOperandOffset32_t operands[5];

    BrigType16_t          sourceType;

    uint16_t reserved;                  //.defValue=0
};

struct BrigInstSeg {
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;
    BrigOperandOffset32_t operands[5]; //.wname=operand
    BrigSegment8_t        segment;
    uint8_t reserved[3];                  //.defValue=0
};

struct BrigInstBr {
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;
    BrigOperandOffset32_t operands[5]; //.wname=operand

    BrigWidth8_t          width;

    uint8_t reserved[3];                  //.defValue=0
};

struct BrigInstLane {
    uint16_t              size;
    BrigInstKinds16_t     kind;
    BrigOpcode16_t        opcode;
    BrigType16_t          type;
    BrigOperandOffset32_t operands[5]; //.wname=operand

    BrigType16_t          sourceType;
    BrigWidth8_t          width;

    uint8_t reserved;                  //.defValue=0
};

struct BrigInstNone { //.isroot //.standalone
    uint16_t              size;
    BrigInstKinds16_t     kind;
};

struct BrigInstSignal {
    uint16_t               size;
    BrigInstKinds16_t      kind;
    BrigOpcode16_t         opcode;
    BrigType16_t           type;
    BrigOperandOffset32_t  operands[5]; //.wname=operand

    BrigType16_t           signalType;
    BrigMemoryOrder8_t     memoryOrder;
    BrigAtomicOperation8_t signalOperation;
};

struct BrigInstQueue {
    uint16_t               size;
    BrigInstKinds16_t      kind;
    BrigOpcode16_t         opcode;
    BrigType16_t           type;
    BrigOperandOffset32_t  operands[5];
    BrigSegment8_t         segment;
    BrigMemoryOrder8_t     memoryOrder;
    uint16_t reserved;                  //.defValue=0
};

struct BrigInstSegCvt {
    uint16_t               size;
    BrigInstKinds16_t      kind;
    BrigOpcode16_t         opcode;
    BrigType16_t           type;
    BrigOperandOffset32_t  operands[5];

    BrigType16_t           sourceType;
    BrigSegment8_t         segment;
    BrigSegCvtModifier     modifier; //.acc=subItem<SegCvtModifier> //.wtype=SegCvtModifier
};

// ============================================================================
//  Operands

struct BrigOperandBase { //.nowrap
    uint16_t             size;
    BrigOperandKinds16_t kind;
};

struct BrigOperand { //.generic //.isroot
    uint16_t             size;
    BrigOperandKinds16_t kind;
};

struct BrigOperandAddress {
    uint16_t                size;
    BrigOperandKinds16_t    kind;
    BrigDirectiveOffset32_t symbol;      //.wtype=ItemRef<DirectiveVariable>
    BrigStringOffset32_t    reg;
    uint32_t                offsetLo;
    //^^ uint64_t           offset;      //.wtype=ValRef<uint64_t> //.acc=reinterpretValRef<uint64_t>
    uint32_t                offsetHi;
};

struct BrigOperandList { //.generic
    uint16_t                size;
    BrigOperandKinds16_t    kind;
    uint16_t reserved;                    //.defValue=0
    uint16_t                elementCount; //.defValue=0
    BrigDirectiveOffset32_t elements[1];  //.wspecial=RefList
};

struct BrigOperandArgumentList { //.parent=BrigOperandList
    uint16_t                size;
    BrigOperandKinds16_t    kind;
    uint16_t reserved;                    //.defValue=0
    uint16_t                elementCount; //.defValue=0
    BrigDirectiveOffset32_t elements[1];  //.wspecial=ArgumentRefList
};

/// list of arguments. (in, out function arguments).
struct BrigOperandFunctionList { //.parent=BrigOperandList
    uint16_t                size;
    BrigOperandKinds16_t    kind;
    uint16_t reserved;                    //.defValue=0
    uint16_t                elementCount; //.defValue=0
    BrigDirectiveOffset32_t elements[1];  //.wspecial=FunctionRefList
};

struct BrigOperandRef { //.generic
    uint16_t                size;
    BrigOperandKinds16_t    kind;
    BrigDirectiveOffset32_t ref; //.wtype=ItemRef<Directive>
};

struct BrigOperandLabelRef { //.parent=BrigOperandRef
    uint16_t                size;
    BrigOperandKinds16_t    kind;
    BrigDirectiveOffset32_t label; //.wtype=ItemRef<DirectiveLabel>
};

struct BrigOperandLabelTargetsRef {  //.parent=BrigOperandRef
    uint16_t                size;
    BrigOperandKinds16_t    kind;
    BrigDirectiveOffset32_t targets;     //.wtype=ItemRef<DirectiveLabelTargets>
};

struct BrigOperandLabelVariableRef { //.parent=BrigOperandRef
    uint16_t                size;
    BrigOperandKinds16_t    kind;
    BrigDirectiveOffset32_t symbol;  //.wtype=ItemRef<DirectiveVariable>
};

struct BrigOperandFunctionRef { //.parent=BrigOperandRef
    uint16_t                size;
    BrigOperandKinds16_t    kind;
    BrigDirectiveOffset32_t ref; //.wname=fn //.wtype=ItemRef<DirectiveFunction>
};

struct BrigOperandSignatureRef { //.parent=BrigOperandRef
    uint16_t                size;
    BrigOperandKinds16_t    kind;
    BrigDirectiveOffset32_t ref; //.wname=sig //.wtype=ItemRef<DirectiveCallableBase>
    /* must reference a directive with kind
       of BRIG_DIRECTIVE_FUNCTION or BRIG_DIRECTIVE_SIGNATURE. */
};

struct BrigOperandFbarrierRef { //.parent=BrigOperandRef
    uint16_t                size;
    BrigOperandKinds16_t    kind;
    BrigDirectiveOffset32_t ref; //.wname=fbar //.wtype=ItemRef<DirectiveFbarrier>
};

struct BrigOperandImmed {
    uint16_t                size;
    BrigOperandKinds16_t    kind;
    uint16_t                reserved;  //.defValue=0
    uint16_t                byteCount;
    uint8_t                 bytes[1];  //.novisit //.noaligncheck
};

struct BrigOperandReg {
    uint16_t             size;
    BrigOperandKinds16_t kind;
    BrigStringOffset32_t reg;
};

struct BrigOperandVector {
    uint16_t              size;
    BrigOperandKinds16_t  kind;
    uint16_t reserved;                  //.defValue=0   
    uint16_t              operandCount; //.defValue=0
    //^^uint16_t          elementCount; //.wname=elementCount
    BrigOperandOffset32_t operands[1];  //.wname=operand //.wtype=ItemRef<Operand> //.wspecial=VectorOperandList
};

struct BrigOperandWavesize {
    uint16_t             size;
    BrigOperandKinds16_t kind;
};