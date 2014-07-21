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
$enums = {
           'BrigRegisterKind' => {
                                   'bits_default' => 'return (unsigned)-1',
                                   'bits' => sub { "DUMMY" },
                                   'bits_switch' => 'true',
                                   'name' => 'BrigRegisterKind',
                                   'entries' => [
                                                  {
                                                    'mnemo' => '$c',
                                                    'val' => '0',
                                                    'bits' => '1',
                                                    'name' => 'BRIG_REGISTER_CONTROL'
                                                  },
                                                  {
                                                    'val' => '1',
                                                    'mnemo' => '$s',
                                                    'name' => 'BRIG_REGISTER_SINGLE',
                                                    'bits' => '32'
                                                  },
                                                  {
                                                    'bits' => '64',
                                                    'name' => 'BRIG_REGISTER_DOUBLE',
                                                    'mnemo' => '$d',
                                                    'val' => '2'
                                                  },
                                                  {
                                                    'name' => 'BRIG_REGISTER_QUAD',
                                                    'bits' => '128',
                                                    'val' => '3     ',
                                                    'mnemo' => '$q'
                                                  }
                                                ],
                                   'mnemo#deps' => [],
                                   'bits#deps' => [],
                                   'mnemo' => sub { "DUMMY" },
                                   'mnemo#calcState' => 'done',
                                   'bits#calcState' => 'done',
                                   'bits_proto' => 'unsigned getRegBits(Brig::BrigRegisterKind16_t arg)'
                                 },
           'BrigMemoryFenceSegments' => {
                                          'mnemo#calcState' => 'done',
                                          'name' => 'BrigMemoryFenceSegments',
                                          'mnemo' => sub { "DUMMY" },
                                          'mnemo_context' => 'EInstModifierInstFenceContext',
                                          'mnemo_token' => '_EMMemoryFenceSegments',
                                          'mnemo#deps' => [],
                                          'entries' => [
                                                         {
                                                           'name' => 'BRIG_MEMORY_FENCE_SEGMENT_GLOBAL',
                                                           'mnemo' => 'global',
                                                           'val' => '0'
                                                         },
                                                         {
                                                           'name' => 'BRIG_MEMORY_FENCE_SEGMENT_GROUP',
                                                           'val' => '1',
                                                           'mnemo' => 'group'
                                                         },
                                                         {
                                                           'name' => 'BRIG_MEMORY_FENCE_SEGMENT_IMAGE',
                                                           'val' => '2',
                                                           'mnemo' => 'image'
                                                         },
                                                         {
                                                           'skip' => 'true',
                                                           'name' => 'BRIG_MEMORY_FENCE_SEGMENT_LAST',
                                                           'mnemo' => 'last',
                                                           'val' => '3 '
                                                         }
                                                       ]
                                        },
           'BrigRound' => {
                            'name' => 'BrigRound',
                            'mnemo' => 'true',
                            'mnemo_token' => '_EMRound',
                            'mnemo_fn' => 'round2str',
                            'entries' => [
                                           {
                                             'val' => '0',
                                             'name' => 'BRIG_ROUND_NONE'
                                           },
                                           {
                                             'name' => 'BRIG_ROUND_FLOAT_NEAR_EVEN',
                                             'mnemo' => 'near',
                                             'val' => '1'
                                           },
                                           {
                                             'name' => 'BRIG_ROUND_FLOAT_ZERO',
                                             'mnemo' => 'zero',
                                             'val' => '2'
                                           },
                                           {
                                             'name' => 'BRIG_ROUND_FLOAT_PLUS_INFINITY',
                                             'mnemo' => 'up',
                                             'val' => '3'
                                           },
                                           {
                                             'name' => 'BRIG_ROUND_FLOAT_MINUS_INFINITY',
                                             'mnemo' => 'down',
                                             'val' => '4'
                                           },
                                           {
                                             'name' => 'BRIG_ROUND_INTEGER_NEAR_EVEN',
                                             'mnemo' => 'neari',
                                             'val' => '5'
                                           },
                                           {
                                             'val' => '6',
                                             'mnemo' => 'zeroi',
                                             'name' => 'BRIG_ROUND_INTEGER_ZERO'
                                           },
                                           {
                                             'mnemo' => 'upi',
                                             'val' => '7',
                                             'name' => 'BRIG_ROUND_INTEGER_PLUS_INFINITY'
                                           },
                                           {
                                             'name' => 'BRIG_ROUND_INTEGER_MINUS_INFINITY',
                                             'mnemo' => 'downi',
                                             'val' => '8'
                                           },
                                           {
                                             'name' => 'BRIG_ROUND_INTEGER_NEAR_EVEN_SAT',
                                             'val' => '9',
                                             'mnemo' => 'neari_sat'
                                           },
                                           {
                                             'val' => '10',
                                             'mnemo' => 'zeroi_sat',
                                             'name' => 'BRIG_ROUND_INTEGER_ZERO_SAT'
                                           },
                                           {
                                             'name' => 'BRIG_ROUND_INTEGER_PLUS_INFINITY_SAT',
                                             'mnemo' => 'upi_sat',
                                             'val' => '11'
                                           },
                                           {
                                             'name' => 'BRIG_ROUND_INTEGER_MINUS_INFINITY_SAT',
                                             'mnemo' => 'downi_sat',
                                             'val' => '12'
                                           },
                                           {
                                             'name' => 'BRIG_ROUND_INTEGER_SIGNALLING_NEAR_EVEN',
                                             'val' => '13',
                                             'mnemo' => 'sneari'
                                           },
                                           {
                                             'name' => 'BRIG_ROUND_INTEGER_SIGNALLING_ZERO',
                                             'val' => '14',
                                             'mnemo' => 'szeroi'
                                           },
                                           {
                                             'name' => 'BRIG_ROUND_INTEGER_SIGNALLING_PLUS_INFINITY',
                                             'mnemo' => 'supi',
                                             'val' => '15'
                                           },
                                           {
                                             'val' => '16',
                                             'mnemo' => 'sdowni',
                                             'name' => 'BRIG_ROUND_INTEGER_SIGNALLING_MINUS_INFINITY'
                                           },
                                           {
                                             'val' => '17',
                                             'mnemo' => 'sneari_sat',
                                             'name' => 'BRIG_ROUND_INTEGER_SIGNALLING_NEAR_EVEN_SAT'
                                           },
                                           {
                                             'name' => 'BRIG_ROUND_INTEGER_SIGNALLING_ZERO_SAT',
                                             'mnemo' => 'szeroi_sat',
                                             'val' => '18'
                                           },
                                           {
                                             'val' => '19',
                                             'mnemo' => 'supi_sat',
                                             'name' => 'BRIG_ROUND_INTEGER_SIGNALLING_PLUS_INFINITY_SAT'
                                           },
                                           {
                                             'val' => '20 ',
                                             'mnemo' => 'sdowni_sat',
                                             'name' => 'BRIG_ROUND_INTEGER_SIGNALLING_MINUS_INFINITY_SAT'
                                           }
                                         ]
                          },
           'BrigSegCvtModifierMask' => {
                                         'entries' => [
                                                        {
                                                          'name' => 'BRIG_SEG_CVT_NONULL',
                                                          'mnemo' => 'nonull',
                                                          'val' => '1 '
                                                        }
                                                      ],
                                         'name' => 'BrigSegCvtModifierMask'
                                       },
           'BrigKinds' => {
                            'isToplevelOnly_proto' => 'bool isToplevelOnly(Directive d)',
                            'wname#deps' => [],
                            'mnemo' => sub { "DUMMY" },
                            'sizeof#calcState' => 'done',
                            'sizeof' => sub { "DUMMY" },
                            'isToplevelOnly' => sub { "DUMMY" },
                            'isToplevelOnly#deps' => [],
                            'name' => 'BrigKinds',
                            'isBodyOnly_proto' => 'bool isBodyOnly(Directive d)',
                            'sizeof_switch' => 'true',
                            'isBodyOnly_switch' => 'true',
                            'wname' => sub { "DUMMY" },
                            'wname#calcState' => 'done',
                            'isBodyOnly#calcState' => 'done',
                            'mnemo#calcState' => 'done',
                            'sizeof_proto' => 'int size_of_brig_record(unsigned arg)',
                            'isBodyOnly#deps' => [],
                            'sizeof#deps' => [
                                               'wname'
                                             ],
                            'isToplevelOnly_arg' => 'd.brig()->kind',
                            'isToplevelOnly#calcState' => 'done',
                            'isToplevelOnly_default' => 'assert(false); return false',
                            'isBodyOnly_arg' => 'd.brig()->kind',
                            'sizeof_default' => 'return -1',
                            'isToplevelOnly_switch' => 'true',
                            'entries' => [
                                           {
                                             'wname' => 'None',
                                             'isBodyOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigNone)',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_NONE',
                                             'skip' => 'true',
                                             'val' => '0x0000',
                                             'mnemo' => 'None'
                                           },
                                           {
                                             'wname' => 'DirectiveBegin',
                                             'isBodyOnly' => 'false',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_DIRECTIVE_BEGIN',
                                             'sizeof' => 'sizeof(BrigDirectiveBegin)',
                                             'skip' => 'true',
                                             'val' => '0x1000',
                                             'mnemo' => 'DirectiveBegin'
                                           },
                                           {
                                             'isBodyOnly' => 'true',
                                             'wname' => 'DirectiveArgBlockEnd',
                                             'sizeof' => 'sizeof(BrigDirectiveArgBlockEnd)',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_DIRECTIVE_ARG_BLOCK_END',
                                             'mnemo' => 'DirectiveArgBlockEnd',
                                             'val' => '0x1000'
                                           },
                                           {
                                             'wname' => 'DirectiveArgBlockStart',
                                             'isBodyOnly' => 'true',
                                             'val' => '0x1001',
                                             'mnemo' => 'DirectiveArgBlockStart',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_DIRECTIVE_ARG_BLOCK_START',
                                             'sizeof' => 'sizeof(BrigDirectiveArgBlockStart)'
                                           },
                                           {
                                             'name' => 'BRIG_KIND_DIRECTIVE_COMMENT',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigDirectiveComment)',
                                             'val' => '0x1002',
                                             'mnemo' => 'DirectiveComment',
                                             'wname' => 'DirectiveComment',
                                             'isBodyOnly' => 'false'
                                           },
                                           {
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigDirectiveControl)',
                                             'name' => 'BRIG_KIND_DIRECTIVE_CONTROL',
                                             'val' => '0x1003',
                                             'mnemo' => 'DirectiveControl',
                                             'wname' => 'DirectiveControl',
                                             'isBodyOnly' => 'true'
                                           },
                                           {
                                             'val' => '0x1004',
                                             'mnemo' => 'DirectiveExtension',
                                             'sizeof' => 'sizeof(BrigDirectiveExtension)',
                                             'name' => 'BRIG_KIND_DIRECTIVE_EXTENSION',
                                             'isToplevelOnly' => 'true',
                                             'wname' => 'DirectiveExtension',
                                             'isBodyOnly' => 'false'
                                           },
                                           {
                                             'name' => 'BRIG_KIND_DIRECTIVE_FBARRIER',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigDirectiveFbarrier)',
                                             'val' => '0x1005',
                                             'mnemo' => 'DirectiveFbarrier',
                                             'wname' => 'DirectiveFbarrier',
                                             'isBodyOnly' => 'false'
                                           },
                                           {
                                             'val' => '0x1006',
                                             'mnemo' => 'DirectiveFunction',
                                             'name' => 'BRIG_KIND_DIRECTIVE_FUNCTION',
                                             'sizeof' => 'sizeof(BrigDirectiveFunction)',
                                             'isToplevelOnly' => 'true',
                                             'wname' => 'DirectiveFunction',
                                             'isBodyOnly' => 'false'
                                           },
                                           {
                                             'wname' => 'DirectiveIndirectFunction',
                                             'isBodyOnly' => 'false',
                                             'val' => '0x1007',
                                             'mnemo' => 'DirectiveIndirectFunction',
                                             'name' => 'BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION',
                                             'sizeof' => 'sizeof(BrigDirectiveIndirectFunction)',
                                             'isToplevelOnly' => 'true'
                                           },
                                           {
                                             'name' => 'BRIG_KIND_DIRECTIVE_KERNEL',
                                             'sizeof' => 'sizeof(BrigDirectiveKernel)',
                                             'isToplevelOnly' => 'true',
                                             'mnemo' => 'DirectiveKernel',
                                             'val' => '0x1008',
                                             'isBodyOnly' => 'false',
                                             'wname' => 'DirectiveKernel'
                                           },
                                           {
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_DIRECTIVE_LABEL',
                                             'sizeof' => 'sizeof(BrigDirectiveLabel)',
                                             'mnemo' => 'DirectiveLabel',
                                             'val' => '0x1009',
                                             'isBodyOnly' => 'true',
                                             'wname' => 'DirectiveLabel'
                                           },
                                           {
                                             'mnemo' => 'DirectiveLoc',
                                             'val' => '0x100a',
                                             'sizeof' => 'sizeof(BrigDirectiveLoc)',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_DIRECTIVE_LOC',
                                             'isBodyOnly' => 'false',
                                             'wname' => 'DirectiveLoc'
                                           },
                                           {
                                             'mnemo' => 'DirectivePragma',
                                             'val' => '0x100b',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigDirectivePragma)',
                                             'name' => 'BRIG_KIND_DIRECTIVE_PRAGMA',
                                             'isBodyOnly' => 'false',
                                             'wname' => 'DirectivePragma'
                                           },
                                           {
                                             'mnemo' => 'DirectiveSignature',
                                             'val' => '0x100c',
                                             'isToplevelOnly' => 'true',
                                             'sizeof' => 'sizeof(BrigDirectiveSignature)',
                                             'name' => 'BRIG_KIND_DIRECTIVE_SIGNATURE',
                                             'isBodyOnly' => 'false',
                                             'wname' => 'DirectiveSignature'
                                           },
                                           {
                                             'isBodyOnly' => 'false',
                                             'wname' => 'DirectiveVariable',
                                             'mnemo' => 'DirectiveVariable',
                                             'val' => '0x100d',
                                             'name' => 'BRIG_KIND_DIRECTIVE_VARIABLE',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigDirectiveVariable)'
                                           },
                                           {
                                             'wname' => 'DirectiveVersion',
                                             'isBodyOnly' => 'false',
                                             'val' => '0x100e',
                                             'mnemo' => 'DirectiveVersion',
                                             'sizeof' => 'sizeof(BrigDirectiveVersion)',
                                             'name' => 'BRIG_KIND_DIRECTIVE_VERSION',
                                             'isToplevelOnly' => 'true'
                                           },
                                           {
                                             'skip' => 'true',
                                             'mnemo' => 'DirectiveEnd',
                                             'val' => '0x100f',
                                             'isBodyOnly' => 'false',
                                             'wname' => 'DirectiveEnd',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigDirectiveEnd)',
                                             'name' => 'BRIG_KIND_DIRECTIVE_END'
                                           },
                                           {
                                             'isBodyOnly' => 'false',
                                             'wname' => 'InstBegin',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigInstBegin)',
                                             'name' => 'BRIG_KIND_INST_BEGIN',
                                             'mnemo' => 'InstBegin',
                                             'val' => '0x2000',
                                             'skip' => 'true'
                                           },
                                           {
                                             'isBodyOnly' => 'false',
                                             'wname' => 'InstAddr',
                                             'mnemo' => 'InstAddr',
                                             'val' => '0x2000',
                                             'name' => 'BRIG_KIND_INST_ADDR',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigInstAddr)'
                                           },
                                           {
                                             'val' => '0x2001',
                                             'mnemo' => 'InstAtomic',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_INST_ATOMIC',
                                             'sizeof' => 'sizeof(BrigInstAtomic)',
                                             'wname' => 'InstAtomic',
                                             'isBodyOnly' => 'false'
                                           },
                                           {
                                             'isBodyOnly' => 'false',
                                             'wname' => 'InstBasic',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigInstBasic)',
                                             'name' => 'BRIG_KIND_INST_BASIC',
                                             'mnemo' => 'InstBasic',
                                             'val' => '0x2002'
                                           },
                                           {
                                             'isBodyOnly' => 'false',
                                             'wname' => 'InstBr',
                                             'sizeof' => 'sizeof(BrigInstBr)',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_INST_BR',
                                             'mnemo' => 'InstBr',
                                             'val' => '0x2003'
                                           },
                                           {
                                             'val' => '0x2004',
                                             'mnemo' => 'InstCmp',
                                             'name' => 'BRIG_KIND_INST_CMP',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigInstCmp)',
                                             'wname' => 'InstCmp',
                                             'isBodyOnly' => 'false'
                                           },
                                           {
                                             'name' => 'BRIG_KIND_INST_CVT',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigInstCvt)',
                                             'val' => '0x2005',
                                             'mnemo' => 'InstCvt',
                                             'wname' => 'InstCvt',
                                             'isBodyOnly' => 'false'
                                           },
                                           {
                                             'wname' => 'InstImage',
                                             'isBodyOnly' => 'false',
                                             'val' => '0x2006',
                                             'mnemo' => 'InstImage',
                                             'name' => 'BRIG_KIND_INST_IMAGE',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigInstImage)'
                                           },
                                           {
                                             'name' => 'BRIG_KIND_INST_LANE',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigInstLane)',
                                             'val' => '0x2007',
                                             'mnemo' => 'InstLane',
                                             'wname' => 'InstLane',
                                             'isBodyOnly' => 'false'
                                           },
                                           {
                                             'val' => '0x2008',
                                             'mnemo' => 'InstMem',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_INST_MEM',
                                             'sizeof' => 'sizeof(BrigInstMem)',
                                             'wname' => 'InstMem',
                                             'isBodyOnly' => 'false'
                                           },
                                           {
                                             'isBodyOnly' => 'false',
                                             'wname' => 'InstMemFence',
                                             'mnemo' => 'InstMemFence',
                                             'val' => '0x2009',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigInstMemFence)',
                                             'name' => 'BRIG_KIND_INST_MEM_FENCE'
                                           },
                                           {
                                             'wname' => 'InstMod',
                                             'isBodyOnly' => 'false',
                                             'name' => 'BRIG_KIND_INST_MOD',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigInstMod)',
                                             'val' => '0x200a',
                                             'mnemo' => 'InstMod'
                                           },
                                           {
                                             'val' => '0x200b',
                                             'mnemo' => 'InstQueryImage',
                                             'sizeof' => 'sizeof(BrigInstQueryImage)',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_INST_QUERY_IMAGE',
                                             'wname' => 'InstQueryImage',
                                             'isBodyOnly' => 'false'
                                           },
                                           {
                                             'isBodyOnly' => 'false',
                                             'wname' => 'InstQuerySampler',
                                             'mnemo' => 'InstQuerySampler',
                                             'val' => '0x200c',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigInstQuerySampler)',
                                             'name' => 'BRIG_KIND_INST_QUERY_SAMPLER'
                                           },
                                           {
                                             'val' => '0x200d',
                                             'mnemo' => 'InstQueue',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_INST_QUEUE',
                                             'sizeof' => 'sizeof(BrigInstQueue)',
                                             'wname' => 'InstQueue',
                                             'isBodyOnly' => 'false'
                                           },
                                           {
                                             'isBodyOnly' => 'false',
                                             'wname' => 'InstSeg',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigInstSeg)',
                                             'name' => 'BRIG_KIND_INST_SEG',
                                             'mnemo' => 'InstSeg',
                                             'val' => '0x200e'
                                           },
                                           {
                                             'mnemo' => 'InstSegCvt',
                                             'val' => '0x200f',
                                             'name' => 'BRIG_KIND_INST_SEG_CVT',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigInstSegCvt)',
                                             'isBodyOnly' => 'false',
                                             'wname' => 'InstSegCvt'
                                           },
                                           {
                                             'name' => 'BRIG_KIND_INST_SIGNAL',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigInstSignal)',
                                             'mnemo' => 'InstSignal',
                                             'val' => '0x2010',
                                             'isBodyOnly' => 'false',
                                             'wname' => 'InstSignal'
                                           },
                                           {
                                             'mnemo' => 'InstSourceType',
                                             'val' => '0x2011',
                                             'sizeof' => 'sizeof(BrigInstSourceType)',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_INST_SOURCE_TYPE',
                                             'isBodyOnly' => 'false',
                                             'wname' => 'InstSourceType'
                                           },
                                           {
                                             'wname' => 'InstEnd',
                                             'isBodyOnly' => 'false',
                                             'name' => 'BRIG_KIND_INST_END',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigInstEnd)',
                                             'val' => '0x2012',
                                             'mnemo' => 'InstEnd',
                                             'skip' => 'true'
                                           },
                                           {
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigOperandBegin)',
                                             'name' => 'BRIG_KIND_OPERAND_BEGIN',
                                             'wname' => 'OperandBegin',
                                             'isBodyOnly' => 'false',
                                             'val' => '0x3000',
                                             'mnemo' => 'OperandBegin',
                                             'skip' => 'true'
                                           },
                                           {
                                             'wname' => 'OperandAddress',
                                             'isBodyOnly' => 'false',
                                             'val' => '0x3000',
                                             'mnemo' => 'OperandAddress',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigOperandAddress)',
                                             'name' => 'BRIG_KIND_OPERAND_ADDRESS'
                                           },
                                           {
                                             'isBodyOnly' => 'false',
                                             'wname' => 'OperandData',
                                             'mnemo' => 'OperandData',
                                             'val' => '0x3001',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigOperandData)',
                                             'name' => 'BRIG_KIND_OPERAND_DATA'
                                           },
                                           {
                                             'isBodyOnly' => 'false',
                                             'wname' => 'OperandCodeList',
                                             'mnemo' => 'OperandCodeList',
                                             'val' => '0x3002',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_OPERAND_CODE_LIST',
                                             'sizeof' => 'sizeof(BrigOperandCodeList)'
                                           },
                                           {
                                             'wname' => 'OperandCodeRef',
                                             'isBodyOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigOperandCodeRef)',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_OPERAND_CODE_REF',
                                             'val' => '0x3003',
                                             'mnemo' => 'OperandCodeRef'
                                           },
                                           {
                                             'mnemo' => 'OperandImageProperties',
                                             'val' => '0x3004',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_OPERAND_IMAGE_PROPERTIES',
                                             'sizeof' => 'sizeof(BrigOperandImageProperties)',
                                             'isBodyOnly' => 'false',
                                             'wname' => 'OperandImageProperties'
                                           },
                                           {
                                             'isBodyOnly' => 'false',
                                             'wname' => 'OperandOperandList',
                                             'mnemo' => 'OperandOperandList',
                                             'val' => '0x3005',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_OPERAND_OPERAND_LIST',
                                             'sizeof' => 'sizeof(BrigOperandOperandList)'
                                           },
                                           {
                                             'isBodyOnly' => 'false',
                                             'wname' => 'OperandReg',
                                             'name' => 'BRIG_KIND_OPERAND_REG',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigOperandReg)',
                                             'mnemo' => 'OperandReg',
                                             'val' => '0x3006'
                                           },
                                           {
                                             'sizeof' => 'sizeof(BrigOperandSamplerProperties)',
                                             'isToplevelOnly' => 'false',
                                             'name' => 'BRIG_KIND_OPERAND_SAMPLER_PROPERTIES',
                                             'mnemo' => 'OperandSamplerProperties',
                                             'val' => '0x3007',
                                             'isBodyOnly' => 'false',
                                             'wname' => 'OperandSamplerProperties'
                                           },
                                           {
                                             'mnemo' => 'OperandString',
                                             'val' => '0x3008',
                                             'name' => 'BRIG_KIND_OPERAND_STRING',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigOperandString)',
                                             'isBodyOnly' => 'false',
                                             'wname' => 'OperandString'
                                           },
                                           {
                                             'wname' => 'OperandWavesize',
                                             'isBodyOnly' => 'false',
                                             'val' => '0x3009',
                                             'mnemo' => 'OperandWavesize',
                                             'name' => 'BRIG_KIND_OPERAND_WAVESIZE',
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigOperandWavesize)'
                                           },
                                           {
                                             'isToplevelOnly' => 'false',
                                             'sizeof' => 'sizeof(BrigOperandEnd)',
                                             'name' => 'BRIG_KIND_OPERAND_END',
                                             'isBodyOnly' => 'false',
                                             'wname' => 'OperandEnd',
                                             'mnemo' => 'OperandEnd',
                                             'val' => '0x300a ',
                                             'skip' => 'true'
                                           }
                                         ],
                            'mnemo#deps' => [
                                              'wname'
                                            ],
                            'isBodyOnly_default' => 'assert(false); return false',
                            'isBodyOnly' => sub { "DUMMY" }
                          },
           'BrigSamplerFilter' => {
                                    'mnemo#deps' => [],
                                    'entries' => [
                                                   {
                                                     'name' => 'BRIG_FILTER_NEAREST',
                                                     'val' => '0',
                                                     'mnemo' => 'nearest'
                                                   },
                                                   {
                                                     'name' => 'BRIG_FILTER_LINEAR',
                                                     'mnemo' => 'linear',
                                                     'val' => '1'
                                                   }
                                                 ],
                                    'mnemo' => sub { "DUMMY" },
                                    'name' => 'BrigSamplerFilter',
                                    'mnemo#calcState' => 'done'
                                  },
           'BrigProfile' => {
                              'mnemo#deps' => [],
                              'entries' => [
                                             {
                                               'name' => 'BRIG_PROFILE_BASE',
                                               'val' => '0',
                                               'mnemo' => '$base'
                                             },
                                             {
                                               'val' => '1',
                                               'mnemo' => '$full',
                                               'name' => 'BRIG_PROFILE_FULL'
                                             },
                                             {
                                               'skip' => 'true',
                                               'name' => 'BRIG_PROFILE_UNDEF',
                                               'mnemo' => '$undef',
                                               'val' => '2 '
                                             }
                                           ],
                              'mnemo_token' => 'ETargetProfile',
                              'mnemo' => sub { "DUMMY" },
                              'name' => 'BrigProfile',
                              'mnemo#calcState' => 'done'
                            },
           'BrigAtomicOperation' => {
                                      'name' => 'BrigAtomicOperation',
                                      'mnemo_context' => 'EInstModifierInstAtomicContext',
                                      'entries' => [
                                                     {
                                                       'val' => '0',
                                                       'mnemo' => 'add',
                                                       'name' => 'BRIG_ATOMIC_ADD'
                                                     },
                                                     {
                                                       'val' => '1',
                                                       'mnemo' => 'and',
                                                       'name' => 'BRIG_ATOMIC_AND'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_CAS',
                                                       'mnemo' => 'cas',
                                                       'val' => '2'
                                                     },
                                                     {
                                                       'mnemo' => 'exch',
                                                       'val' => '3',
                                                       'name' => 'BRIG_ATOMIC_EXCH'
                                                     },
                                                     {
                                                       'mnemo' => 'ld',
                                                       'val' => '4',
                                                       'name' => 'BRIG_ATOMIC_LD'
                                                     },
                                                     {
                                                       'mnemo' => 'max',
                                                       'val' => '5',
                                                       'name' => 'BRIG_ATOMIC_MAX'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_MIN',
                                                       'val' => '6',
                                                       'mnemo' => 'min'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_OR',
                                                       'mnemo' => 'or',
                                                       'val' => '7'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_ST',
                                                       'val' => '8',
                                                       'mnemo' => 'st'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_SUB',
                                                       'val' => '9',
                                                       'mnemo' => 'sub'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_WRAPDEC',
                                                       'mnemo' => 'wrapdec',
                                                       'val' => '10'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_WRAPINC',
                                                       'val' => '11',
                                                       'mnemo' => 'wrapinc'
                                                     },
                                                     {
                                                       'mnemo' => 'xor',
                                                       'val' => '12',
                                                       'name' => 'BRIG_ATOMIC_XOR'
                                                     },
                                                     {
                                                       'val' => '13',
                                                       'mnemo' => 'wait_eq',
                                                       'name' => 'BRIG_ATOMIC_WAIT_EQ'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_WAIT_NE',
                                                       'val' => '14',
                                                       'mnemo' => 'wait_ne'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_WAIT_LT',
                                                       'val' => '15',
                                                       'mnemo' => 'wait_lt'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_WAIT_GTE',
                                                       'mnemo' => 'wait_gte',
                                                       'val' => '16'
                                                     },
                                                     {
                                                       'val' => '17',
                                                       'mnemo' => 'waittimeout_eq',
                                                       'name' => 'BRIG_ATOMIC_WAITTIMEOUT_EQ'
                                                     },
                                                     {
                                                       'mnemo' => 'waittimeout_ne',
                                                       'val' => '18',
                                                       'name' => 'BRIG_ATOMIC_WAITTIMEOUT_NE'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_WAITTIMEOUT_LT',
                                                       'mnemo' => 'waittimeout_lt',
                                                       'val' => '19'
                                                     },
                                                     {
                                                       'val' => '20',
                                                       'mnemo' => 'waittimeout_gte',
                                                       'name' => 'BRIG_ATOMIC_WAITTIMEOUT_GTE'
                                                     }
                                                   ],
                                      'mnemo#deps' => [],
                                      'mnemo#calcState' => 'done',
                                      'tdcaption' => 'Atomic Operations',
                                      'mnemo' => sub { "DUMMY" },
                                      'mnemo_token' => '_EMAtomicOp'
                                    },
           'BrigSamplerAddressing' => {
                                        'mnemo#calcState' => 'done',
                                        'name' => 'BrigSamplerAddressing',
                                        'mnemo' => sub { "DUMMY" },
                                        'entries' => [
                                                       {
                                                         'mnemo' => 'undefined',
                                                         'val' => '0',
                                                         'name' => 'BRIG_ADDRESSING_UNDEFINED'
                                                       },
                                                       {
                                                         'mnemo' => 'clamp_to_edge',
                                                         'val' => '1',
                                                         'name' => 'BRIG_ADDRESSING_CLAMP_TO_EDGE'
                                                       },
                                                       {
                                                         'name' => 'BRIG_ADDRESSING_CLAMP_TO_BORDER',
                                                         'mnemo' => 'clamp_to_border',
                                                         'val' => '2'
                                                       },
                                                       {
                                                         'mnemo' => 'repeat',
                                                         'val' => '3',
                                                         'name' => 'BRIG_ADDRESSING_REPEAT'
                                                       },
                                                       {
                                                         'mnemo' => 'mirrored_repeat',
                                                         'val' => '4',
                                                         'name' => 'BRIG_ADDRESSING_MIRRORED_REPEAT'
                                                       }
                                                     ],
                                        'mnemo#deps' => [],
                                        'mnemo_token' => 'ESamplerAddressingMode'
                                      },
           'BrigVersion' => {
                              'nodump' => 'true',
                              'nowrap' => 'true',
                              'entries' => [
                                             {
                                               'name' => 'BRIG_VERSION_HSAIL_MAJOR',
                                               'val' => '0'
                                             },
                                             {
                                               'val' => '20140528',
                                               'name' => 'BRIG_VERSION_HSAIL_MINOR'
                                             },
                                             {
                                               'val' => '0',
                                               'name' => 'BRIG_VERSION_BRIG_MAJOR'
                                             },
                                             {
                                               'val' => '20140528',
                                               'name' => 'BRIG_VERSION_BRIG_MINOR'
                                             }
                                           ],
                              'name' => 'BrigVersion'
                            },
           'BrigSamplerCoordNormalization' => {
                                                'mnemo#calcState' => 'done',
                                                'name' => 'BrigSamplerCoordNormalization',
                                                'mnemo' => sub { "DUMMY" },
                                                'mnemo_token' => 'ESamplerCoord',
                                                'mnemo#deps' => [],
                                                'entries' => [
                                                               {
                                                                 'mnemo' => 'unnormalized',
                                                                 'val' => '0',
                                                                 'name' => 'BRIG_COORD_UNNORMALIZED'
                                                               },
                                                               {
                                                                 'mnemo' => 'normalized',
                                                                 'val' => '1',
                                                                 'name' => 'BRIG_COORD_NORMALIZED'
                                                               }
                                                             ]
                                              },
           'BrigImageGeometry' => {
                                    'mnemo#calcState' => 'done',
                                    'name' => 'BrigImageGeometry',
                                    'tdcaption' => 'Geometry',
                                    'mnemo' => sub { "DUMMY" },
                                    'mnemo_token' => 'EImageGeometry',
                                    'entries' => [
                                                   {
                                                     'name' => 'BRIG_GEOMETRY_1D',
                                                     'val' => '0',
                                                     'mnemo' => '1d'
                                                   },
                                                   {
                                                     'name' => 'BRIG_GEOMETRY_2D',
                                                     'mnemo' => '2d',
                                                     'val' => '1'
                                                   },
                                                   {
                                                     'mnemo' => '3d',
                                                     'val' => '2',
                                                     'name' => 'BRIG_GEOMETRY_3D'
                                                   },
                                                   {
                                                     'mnemo' => '1da',
                                                     'val' => '3',
                                                     'name' => 'BRIG_GEOMETRY_1DA'
                                                   },
                                                   {
                                                     'name' => 'BRIG_GEOMETRY_2DA',
                                                     'mnemo' => '2da',
                                                     'val' => '4'
                                                   },
                                                   {
                                                     'val' => '5',
                                                     'mnemo' => '1db',
                                                     'name' => 'BRIG_GEOMETRY_1DB'
                                                   },
                                                   {
                                                     'mnemo' => '2ddepth',
                                                     'val' => '6',
                                                     'name' => 'BRIG_GEOMETRY_2DDEPTH'
                                                   },
                                                   {
                                                     'name' => 'BRIG_GEOMETRY_2DADEPTH',
                                                     'val' => '7',
                                                     'mnemo' => '2dadepth'
                                                   },
                                                   {
                                                     'mnemo' => '',
                                                     'name' => 'BRIG_GEOMETRY_UNKNOWN'
                                                   }
                                                 ],
                                    'mnemo#deps' => []
                                  },
           'BrigLinkage' => {
                              'mnemo#calcState' => 'done',
                              'name' => 'BrigLinkage',
                              'mnemo' => sub { "DUMMY" },
                              'mnemo#deps' => [],
                              'entries' => [
                                             {
                                               'val' => '0',
                                               'mnemo' => '',
                                               'name' => 'BRIG_LINKAGE_NONE'
                                             },
                                             {
                                               'name' => 'BRIG_LINKAGE_PROGRAM',
                                               'val' => '1',
                                               'mnemo' => 'program'
                                             },
                                             {
                                               'name' => 'BRIG_LINKAGE_MODULE',
                                               'mnemo' => 'module',
                                               'val' => '2'
                                             },
                                             {
                                               'name' => 'BRIG_LINKAGE_FUNCTION',
                                               'val' => '3',
                                               'mnemo' => 'function'
                                             },
                                             {
                                               'name' => 'BRIG_LINKAGE_ARG',
                                               'val' => '4',
                                               'mnemo' => 'arg'
                                             }
                                           ]
                            },
           'BrigMemoryScope' => {
                                  'mnemo#calcState' => 'done',
                                  'name' => 'BrigMemoryScope',
                                  'mnemo' => sub { "DUMMY" },
                                  'mnemo_token' => '_EMMemoryScope',
                                  'entries' => [
                                                 {
                                                   'val' => '0',
                                                   'mnemo' => '',
                                                   'name' => 'BRIG_MEMORY_SCOPE_NONE'
                                                 },
                                                 {
                                                   'name' => 'BRIG_MEMORY_SCOPE_WORKITEM',
                                                   'val' => '1',
                                                   'mnemo' => 'wi'
                                                 },
                                                 {
                                                   'mnemo' => 'wv',
                                                   'val' => '2',
                                                   'name' => 'BRIG_MEMORY_SCOPE_WAVEFRONT'
                                                 },
                                                 {
                                                   'name' => 'BRIG_MEMORY_SCOPE_WORKGROUP',
                                                   'mnemo' => 'wg',
                                                   'val' => '3'
                                                 },
                                                 {
                                                   'mnemo' => 'cmp',
                                                   'val' => '4',
                                                   'name' => 'BRIG_MEMORY_SCOPE_COMPONENT'
                                                 },
                                                 {
                                                   'name' => 'BRIG_MEMORY_SCOPE_SYSTEM',
                                                   'val' => '5 ',
                                                   'mnemo' => 'sys'
                                                 }
                                               ],
                                  'mnemo#deps' => []
                                },
           'BrigWidth' => {
                            'name' => 'BrigWidth',
                            'entries' => [
                                           {
                                             'val' => '0',
                                             'name' => 'BRIG_WIDTH_NONE'
                                           },
                                           {
                                             'val' => '1',
                                             'name' => 'BRIG_WIDTH_1'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_2',
                                             'val' => '2'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_4',
                                             'val' => '3'
                                           },
                                           {
                                             'val' => '4',
                                             'name' => 'BRIG_WIDTH_8'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_16',
                                             'val' => '5'
                                           },
                                           {
                                             'val' => '6',
                                             'name' => 'BRIG_WIDTH_32'
                                           },
                                           {
                                             'val' => '7',
                                             'name' => 'BRIG_WIDTH_64'
                                           },
                                           {
                                             'val' => '8',
                                             'name' => 'BRIG_WIDTH_128'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_256',
                                             'val' => '9'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_512',
                                             'val' => '10'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_1024',
                                             'val' => '11'
                                           },
                                           {
                                             'val' => '12',
                                             'name' => 'BRIG_WIDTH_2048'
                                           },
                                           {
                                             'val' => '13',
                                             'name' => 'BRIG_WIDTH_4096'
                                           },
                                           {
                                             'val' => '14',
                                             'name' => 'BRIG_WIDTH_8192'
                                           },
                                           {
                                             'val' => '15',
                                             'name' => 'BRIG_WIDTH_16384'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_32768',
                                             'val' => '16'
                                           },
                                           {
                                             'val' => '17',
                                             'name' => 'BRIG_WIDTH_65536'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_131072',
                                             'val' => '18'
                                           },
                                           {
                                             'val' => '19',
                                             'name' => 'BRIG_WIDTH_262144'
                                           },
                                           {
                                             'val' => '20',
                                             'name' => 'BRIG_WIDTH_524288'
                                           },
                                           {
                                             'val' => '21',
                                             'name' => 'BRIG_WIDTH_1048576'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_2097152',
                                             'val' => '22'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_4194304',
                                             'val' => '23'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_8388608',
                                             'val' => '24'
                                           },
                                           {
                                             'val' => '25',
                                             'name' => 'BRIG_WIDTH_16777216'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_33554432',
                                             'val' => '26'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_67108864',
                                             'val' => '27'
                                           },
                                           {
                                             'val' => '28',
                                             'name' => 'BRIG_WIDTH_134217728'
                                           },
                                           {
                                             'val' => '29',
                                             'name' => 'BRIG_WIDTH_268435456'
                                           },
                                           {
                                             'val' => '30',
                                             'name' => 'BRIG_WIDTH_536870912'
                                           },
                                           {
                                             'val' => '31',
                                             'name' => 'BRIG_WIDTH_1073741824'
                                           },
                                           {
                                             'val' => '32',
                                             'name' => 'BRIG_WIDTH_2147483648'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_WAVESIZE',
                                             'val' => '33'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_ALL',
                                             'val' => '34'
                                           }
                                         ]
                          },
           'BrigSamplerQuery' => {
                                   'entries' => [
                                                  {
                                                    'name' => 'BRIG_SAMPLER_QUERY_ADDRESSING',
                                                    'mnemo' => 'addressing',
                                                    'val' => '0'
                                                  },
                                                  {
                                                    'name' => 'BRIG_SAMPLER_QUERY_COORD',
                                                    'val' => '1',
                                                    'mnemo' => 'coord'
                                                  },
                                                  {
                                                    'mnemo' => 'filter',
                                                    'val' => '2',
                                                    'name' => 'BRIG_SAMPLER_QUERY_FILTER'
                                                  }
                                                ],
                                   'mnemo#deps' => [],
                                   'mnemo_token' => '_EMSamplerQuery',
                                   'mnemo' => sub { "DUMMY" },
                                   'name' => 'BrigSamplerQuery',
                                   'mnemo#calcState' => 'done'
                                 },
           'BrigMachineModel' => {
                                   'mnemo#calcState' => 'done',
                                   'name' => 'BrigMachineModel',
                                   'mnemo' => sub { "DUMMY" },
                                   'mnemo_token' => 'ETargetMachine',
                                   'mnemo#deps' => [],
                                   'entries' => [
                                                  {
                                                    'name' => 'BRIG_MACHINE_SMALL',
                                                    'val' => '0',
                                                    'mnemo' => '$small'
                                                  },
                                                  {
                                                    'mnemo' => '$large',
                                                    'val' => '1',
                                                    'name' => 'BRIG_MACHINE_LARGE'
                                                  },
                                                  {
                                                    'mnemo' => '$undef',
                                                    'val' => '2 ',
                                                    'skip' => 'true',
                                                    'name' => 'BRIG_MACHINE_UNDEF'
                                                  }
                                                ]
                                 },
           'BrigAlignment' => {
                                'rbytes_switch' => 'true',
                                'bytes_switch' => 'true',
                                'rbytes_default' => 'return BRIG_ALIGNMENT_LAST',
                                'mnemo#deps' => [],
                                'entries' => [
                                               {
                                                 'name' => 'BRIG_ALIGNMENT_NONE',
                                                 'val' => '0',
                                                 'mnemo' => 'none',
                                                 'no_mnemo' => 'true'
                                               },
                                               {
                                                 'bytes' => '1',
                                                 'rbytes' => '1',
                                                 'val' => '1',
                                                 'mnemo' => '',
                                                 'name' => 'BRIG_ALIGNMENT_1'
                                               },
                                               {
                                                 'bytes' => '2',
                                                 'rbytes' => '2',
                                                 'name' => 'BRIG_ALIGNMENT_2',
                                                 'val' => '2',
                                                 'mnemo' => '2'
                                               },
                                               {
                                                 'val' => '3',
                                                 'mnemo' => '4',
                                                 'name' => 'BRIG_ALIGNMENT_4',
                                                 'bytes' => '4',
                                                 'rbytes' => '4'
                                               },
                                               {
                                                 'name' => 'BRIG_ALIGNMENT_8',
                                                 'val' => '4',
                                                 'mnemo' => '8',
                                                 'bytes' => '8',
                                                 'rbytes' => '8'
                                               },
                                               {
                                                 'rbytes' => '16',
                                                 'bytes' => '16',
                                                 'name' => 'BRIG_ALIGNMENT_16',
                                                 'mnemo' => '16',
                                                 'val' => '5'
                                               },
                                               {
                                                 'rbytes' => '32',
                                                 'bytes' => '32',
                                                 'mnemo' => '32',
                                                 'val' => '6',
                                                 'name' => 'BRIG_ALIGNMENT_32'
                                               },
                                               {
                                                 'rbytes' => '64',
                                                 'bytes' => '64',
                                                 'mnemo' => '64',
                                                 'val' => '7',
                                                 'name' => 'BRIG_ALIGNMENT_64'
                                               },
                                               {
                                                 'name' => 'BRIG_ALIGNMENT_128',
                                                 'mnemo' => '128',
                                                 'val' => '8',
                                                 'rbytes' => '128',
                                                 'bytes' => '128'
                                               },
                                               {
                                                 'rbytes' => '256',
                                                 'bytes' => '256',
                                                 'mnemo' => '256',
                                                 'val' => '9',
                                                 'name' => 'BRIG_ALIGNMENT_256'
                                               },
                                               {
                                                 'mnemo' => 'last',
                                                 'skip' => 'true',
                                                 'name' => 'BRIG_ALIGNMENT_LAST'
                                               },
                                               {
                                                 'skip' => 'true',
                                                 'name' => 'BRIG_ALIGNMENT_MAX',
                                                 'mnemo' => 'max',
                                                 'val' => 'BRIG_ALIGNMENT_LAST - 1 '
                                               }
                                             ],
                                'mnemo#calcState' => 'done',
                                'rbytes_reverse' => 'true',
                                'rbytes#deps' => [
                                                   'bytes'
                                                 ],
                                'mnemo_proto' => 'const char* align2str(unsigned arg)',
                                'bytes_default' => 'assert(false); return -1',
                                'name' => 'BrigAlignment',
                                'rbytes#calcState' => 'done',
                                'bytes' => sub { "DUMMY" },
                                'rbytes' => sub { "DUMMY" },
                                'bytes_proto' => 'unsigned align2num(unsigned arg)',
                                'bytes#deps' => [],
                                'bytes#calcState' => 'done',
                                'mnemo' => sub { "DUMMY" },
                                'rbytes_proto' => 'Brig::BrigAlignment num2align(uint64_t arg)'
                              },
           'BrigSectionIndex' => {
                                   'mnemo' => sub { "DUMMY" },
                                   'name' => 'BrigSectionIndex',
                                   'mnemo#calcState' => 'done',
                                   'mnemo#deps' => [],
                                   'entries' => [
                                                  {
                                                    'name' => 'BRIG_SECTION_INDEX_DATA',
                                                    'val' => '0',
                                                    'mnemo' => 'hsa_data'
                                                  },
                                                  {
                                                    'mnemo' => 'hsa_code',
                                                    'val' => '1',
                                                    'name' => 'BRIG_SECTION_INDEX_CODE'
                                                  },
                                                  {
                                                    'mnemo' => 'hsa_operand',
                                                    'val' => '2',
                                                    'name' => 'BRIG_SECTION_INDEX_OPERAND'
                                                  },
                                                  {
                                                    'val' => '3',
                                                    'mnemo' => 'hsa_begin_implementation_defined',
                                                    'name' => 'BRIG_SECTION_INDEX_BEGIN_IMPLEMENTATION_DEFINED'
                                                  },
                                                  {
                                                    'mnemo' => 'hsa_implementation_defined',
                                                    'val' => 'BRIG_SECTION_INDEX_BEGIN_IMPLEMENTATION_DEFINED ',
                                                    'skip' => 'true',
                                                    'name' => 'BRIG_SECTION_INDEX_IMPLEMENTATION_DEFINED'
                                                  }
                                                ]
                                 },
           'BrigCompareOperation' => {
                                       'tdcaption' => 'Comparison Operators',
                                       'mnemo' => sub { "DUMMY" },
                                       'name' => 'BrigCompareOperation',
                                       'mnemo#calcState' => 'done',
                                       'mnemo_token' => '_EMCompare',
                                       'mnemo#deps' => [],
                                       'entries' => [
                                                      {
                                                        'name' => 'BRIG_COMPARE_EQ',
                                                        'mnemo' => 'eq',
                                                        'val' => '0'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_NE',
                                                        'mnemo' => 'ne',
                                                        'val' => '1'
                                                      },
                                                      {
                                                        'mnemo' => 'lt',
                                                        'val' => '2',
                                                        'name' => 'BRIG_COMPARE_LT'
                                                      },
                                                      {
                                                        'val' => '3',
                                                        'mnemo' => 'le',
                                                        'name' => 'BRIG_COMPARE_LE'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_GT',
                                                        'mnemo' => 'gt',
                                                        'val' => '4'
                                                      },
                                                      {
                                                        'val' => '5',
                                                        'mnemo' => 'ge',
                                                        'name' => 'BRIG_COMPARE_GE'
                                                      },
                                                      {
                                                        'val' => '6',
                                                        'mnemo' => 'equ',
                                                        'name' => 'BRIG_COMPARE_EQU'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_NEU',
                                                        'val' => '7',
                                                        'mnemo' => 'neu'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_LTU',
                                                        'mnemo' => 'ltu',
                                                        'val' => '8'
                                                      },
                                                      {
                                                        'val' => '9',
                                                        'mnemo' => 'leu',
                                                        'name' => 'BRIG_COMPARE_LEU'
                                                      },
                                                      {
                                                        'val' => '10',
                                                        'mnemo' => 'gtu',
                                                        'name' => 'BRIG_COMPARE_GTU'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_GEU',
                                                        'val' => '11',
                                                        'mnemo' => 'geu'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_NUM',
                                                        'val' => '12',
                                                        'mnemo' => 'num'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_NAN',
                                                        'mnemo' => 'nan',
                                                        'val' => '13'
                                                      },
                                                      {
                                                        'mnemo' => 'seq',
                                                        'val' => '14',
                                                        'name' => 'BRIG_COMPARE_SEQ'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_SNE',
                                                        'mnemo' => 'sne',
                                                        'val' => '15'
                                                      },
                                                      {
                                                        'mnemo' => 'slt',
                                                        'val' => '16',
                                                        'name' => 'BRIG_COMPARE_SLT'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_SLE',
                                                        'val' => '17',
                                                        'mnemo' => 'sle'
                                                      },
                                                      {
                                                        'mnemo' => 'sgt',
                                                        'val' => '18',
                                                        'name' => 'BRIG_COMPARE_SGT'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_SGE',
                                                        'val' => '19',
                                                        'mnemo' => 'sge'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_SGEU',
                                                        'mnemo' => 'sgeu',
                                                        'val' => '20'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_SEQU',
                                                        'mnemo' => 'sequ',
                                                        'val' => '21'
                                                      },
                                                      {
                                                        'mnemo' => 'sneu',
                                                        'val' => '22',
                                                        'name' => 'BRIG_COMPARE_SNEU'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_SLTU',
                                                        'mnemo' => 'sltu',
                                                        'val' => '23'
                                                      },
                                                      {
                                                        'val' => '24',
                                                        'mnemo' => 'sleu',
                                                        'name' => 'BRIG_COMPARE_SLEU'
                                                      },
                                                      {
                                                        'mnemo' => 'snum',
                                                        'val' => '25',
                                                        'name' => 'BRIG_COMPARE_SNUM'
                                                      },
                                                      {
                                                        'val' => '26',
                                                        'mnemo' => 'snan',
                                                        'name' => 'BRIG_COMPARE_SNAN'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_SGTU',
                                                        'mnemo' => 'sgtu',
                                                        'val' => '27'
                                                      }
                                                    ]
                                     },
           'BrigImageChannelOrder' => {
                                        'mnemo' => sub { "DUMMY" },
                                        'name' => 'BrigImageChannelOrder',
                                        'mnemo#calcState' => 'done',
                                        'mnemo_token' => 'EImageOrder',
                                        'mnemo#deps' => [],
                                        'entries' => [
                                                       {
                                                         'mnemo' => 'a',
                                                         'val' => '0',
                                                         'name' => 'BRIG_CHANNEL_ORDER_A'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_R',
                                                         'mnemo' => 'r',
                                                         'val' => '1'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_RX',
                                                         'val' => '2',
                                                         'mnemo' => 'rx'
                                                       },
                                                       {
                                                         'val' => '3',
                                                         'mnemo' => 'rg',
                                                         'name' => 'BRIG_CHANNEL_ORDER_RG'
                                                       },
                                                       {
                                                         'mnemo' => 'rgx',
                                                         'val' => '4',
                                                         'name' => 'BRIG_CHANNEL_ORDER_RGX'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_RA',
                                                         'val' => '5',
                                                         'mnemo' => 'ra'
                                                       },
                                                       {
                                                         'mnemo' => 'rgb',
                                                         'val' => '6',
                                                         'name' => 'BRIG_CHANNEL_ORDER_RGB'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_RGBX',
                                                         'mnemo' => 'rgbx',
                                                         'val' => '7'
                                                       },
                                                       {
                                                         'mnemo' => 'rgba',
                                                         'val' => '8',
                                                         'name' => 'BRIG_CHANNEL_ORDER_RGBA'
                                                       },
                                                       {
                                                         'mnemo' => 'bgra',
                                                         'val' => '9',
                                                         'name' => 'BRIG_CHANNEL_ORDER_BGRA'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_ARGB',
                                                         'val' => '10',
                                                         'mnemo' => 'argb'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_ABGR',
                                                         'val' => '11',
                                                         'mnemo' => 'abgr'
                                                       },
                                                       {
                                                         'val' => '12',
                                                         'mnemo' => 'srgb',
                                                         'name' => 'BRIG_CHANNEL_ORDER_SRGB'
                                                       },
                                                       {
                                                         'mnemo' => 'srgbx',
                                                         'val' => '13',
                                                         'name' => 'BRIG_CHANNEL_ORDER_SRGBX'
                                                       },
                                                       {
                                                         'mnemo' => 'srgba',
                                                         'val' => '14',
                                                         'name' => 'BRIG_CHANNEL_ORDER_SRGBA'
                                                       },
                                                       {
                                                         'val' => '15',
                                                         'mnemo' => 'sbgra',
                                                         'name' => 'BRIG_CHANNEL_ORDER_SBGRA'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_INTENSITY',
                                                         'mnemo' => 'intensity',
                                                         'val' => '16'
                                                       },
                                                       {
                                                         'val' => '17',
                                                         'mnemo' => 'luminance',
                                                         'name' => 'BRIG_CHANNEL_ORDER_LUMINANCE'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_DEPTH',
                                                         'mnemo' => 'depth',
                                                         'val' => '18'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_DEPTH_STENCIL',
                                                         'val' => '19',
                                                         'mnemo' => 'depth_stencil'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_UNKNOWN',
                                                         'mnemo' => ''
                                                       }
                                                     ],
                                        'mnemo_context' => 'EImageOrderContext'
                                      },
           'BrigAllocation' => {
                                 'mnemo_token' => 'EAllocKind',
                                 'entries' => [
                                                {
                                                  'val' => '0',
                                                  'mnemo' => '',
                                                  'name' => 'BRIG_ALLOCATION_NONE'
                                                },
                                                {
                                                  'name' => 'BRIG_ALLOCATION_PROGRAM',
                                                  'val' => '1',
                                                  'mnemo' => 'program'
                                                },
                                                {
                                                  'mnemo' => 'agent',
                                                  'val' => '2',
                                                  'name' => 'BRIG_ALLOCATION_AGENT'
                                                },
                                                {
                                                  'name' => 'BRIG_ALLOCATION_AUTOMATIC',
                                                  'val' => '3',
                                                  'mnemo' => 'automatic'
                                                }
                                              ],
                                 'mnemo#deps' => [],
                                 'mnemo#calcState' => 'done',
                                 'name' => 'BrigAllocation',
                                 'mnemo' => sub { "DUMMY" }
                               },
           'BrigExecutableModifierMask' => {
                                             'name' => 'BrigExecutableModifierMask',
                                             'entries' => [
                                                            {
                                                              'name' => 'BRIG_EXECUTABLE_DEFINITION',
                                                              'val' => '1'
                                                            }
                                                          ],
                                             'nodump' => 'true'
                                           },
           'BrigControlDirective' => {
                                       'mnemo' => sub { "DUMMY" },
                                       'name' => 'BrigControlDirective',
                                       'mnemo#calcState' => 'done',
                                       'mnemo_token' => 'EControl',
                                       'entries' => [
                                                      {
                                                        'val' => '0',
                                                        'mnemo' => 'none',
                                                        'name' => 'BRIG_CONTROL_NONE',
                                                        'skip' => 'true'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CONTROL_ENABLEBREAKEXCEPTIONS',
                                                        'val' => '1',
                                                        'mnemo' => 'enablebreakexceptions'
                                                      },
                                                      {
                                                        'mnemo' => 'enabledetectexceptions',
                                                        'val' => '2',
                                                        'name' => 'BRIG_CONTROL_ENABLEDETECTEXCEPTIONS'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CONTROL_MAXDYNAMICGROUPSIZE',
                                                        'mnemo' => 'maxdynamicgroupsize',
                                                        'val' => '3'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CONTROL_MAXFLATGRIDSIZE',
                                                        'val' => '4',
                                                        'mnemo' => 'maxflatgridsize'
                                                      },
                                                      {
                                                        'mnemo' => 'maxflatworkgroupsize',
                                                        'val' => '5',
                                                        'name' => 'BRIG_CONTROL_MAXFLATWORKGROUPSIZE'
                                                      },
                                                      {
                                                        'mnemo' => 'requestedworkgroupspercu',
                                                        'val' => '6',
                                                        'name' => 'BRIG_CONTROL_REQUESTEDWORKGROUPSPERCU'
                                                      },
                                                      {
                                                        'mnemo' => 'requireddim',
                                                        'val' => '7',
                                                        'name' => 'BRIG_CONTROL_REQUIREDDIM'
                                                      },
                                                      {
                                                        'mnemo' => 'requiredgridsize',
                                                        'val' => '8',
                                                        'name' => 'BRIG_CONTROL_REQUIREDGRIDSIZE'
                                                      },
                                                      {
                                                        'mnemo' => 'requiredworkgroupsize',
                                                        'val' => '9',
                                                        'name' => 'BRIG_CONTROL_REQUIREDWORKGROUPSIZE'
                                                      },
                                                      {
                                                        'mnemo' => 'requirenopartialworkgroups',
                                                        'val' => '10',
                                                        'name' => 'BRIG_CONTROL_REQUIRENOPARTIALWORKGROUPS'
                                                      }
                                                    ],
                                       'mnemo#deps' => []
                                     },
           'BrigPack' => {
                           'mnemo#calcState' => 'done',
                           'name' => 'BrigPack',
                           'mnemo' => sub { "DUMMY" },
                           'tdcaption' => 'Packing',
                           'mnemo_token' => '_EMPacking',
                           'entries' => [
                                          {
                                            'mnemo' => '',
                                            'val' => '0',
                                            'name' => 'BRIG_PACK_NONE'
                                          },
                                          {
                                            'mnemo' => 'pp',
                                            'val' => '1',
                                            'name' => 'BRIG_PACK_PP'
                                          },
                                          {
                                            'name' => 'BRIG_PACK_PS',
                                            'mnemo' => 'ps',
                                            'val' => '2'
                                          },
                                          {
                                            'name' => 'BRIG_PACK_SP',
                                            'val' => '3',
                                            'mnemo' => 'sp'
                                          },
                                          {
                                            'mnemo' => 'ss',
                                            'val' => '4',
                                            'name' => 'BRIG_PACK_SS'
                                          },
                                          {
                                            'mnemo' => 's',
                                            'val' => '5',
                                            'name' => 'BRIG_PACK_S'
                                          },
                                          {
                                            'name' => 'BRIG_PACK_P',
                                            'val' => '6',
                                            'mnemo' => 'p'
                                          },
                                          {
                                            'name' => 'BRIG_PACK_PPSAT',
                                            'mnemo' => 'pp_sat',
                                            'val' => '7'
                                          },
                                          {
                                            'val' => '8',
                                            'mnemo' => 'ps_sat',
                                            'name' => 'BRIG_PACK_PSSAT'
                                          },
                                          {
                                            'name' => 'BRIG_PACK_SPSAT',
                                            'val' => '9',
                                            'mnemo' => 'sp_sat'
                                          },
                                          {
                                            'name' => 'BRIG_PACK_SSSAT',
                                            'mnemo' => 'ss_sat',
                                            'val' => '10'
                                          },
                                          {
                                            'name' => 'BRIG_PACK_SSAT',
                                            'val' => '11',
                                            'mnemo' => 's_sat'
                                          },
                                          {
                                            'mnemo' => 'p_sat',
                                            'val' => '12',
                                            'name' => 'BRIG_PACK_PSAT'
                                          }
                                        ],
                           'mnemo#deps' => []
                         },
           'BrigPackedTypeBits' => {
                                     'name' => 'BrigPackedTypeBits',
                                     'nodump' => 'true',
                                     'entries' => [
                                                    {
                                                      'name' => 'BRIG_TYPE_PACK_SHIFT',
                                                      'val' => '5'
                                                    },
                                                    {
                                                      'val' => '(1 << BRIG_TYPE_PACK_SHIFT) - 1',
                                                      'name' => 'BRIG_TYPE_BASE_MASK'
                                                    },
                                                    {
                                                      'val' => '3 << BRIG_TYPE_PACK_SHIFT',
                                                      'name' => 'BRIG_TYPE_PACK_MASK'
                                                    },
                                                    {
                                                      'name' => 'BRIG_TYPE_PACK_NONE',
                                                      'val' => '0 << BRIG_TYPE_PACK_SHIFT'
                                                    },
                                                    {
                                                      'val' => '1 << BRIG_TYPE_PACK_SHIFT',
                                                      'name' => 'BRIG_TYPE_PACK_32'
                                                    },
                                                    {
                                                      'name' => 'BRIG_TYPE_PACK_64',
                                                      'val' => '2 << BRIG_TYPE_PACK_SHIFT'
                                                    },
                                                    {
                                                      'name' => 'BRIG_TYPE_PACK_128',
                                                      'val' => '3 << BRIG_TYPE_PACK_SHIFT'
                                                    }
                                                  ]
                                   },
           'BrigAluModifierMask' => {
                                      'entries' => [
                                                     {
                                                       'name' => 'BRIG_ALU_ROUND',
                                                       'val' => '31'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ALU_FTZ',
                                                       'val' => '32'
                                                     }
                                                   ],
                                      'name' => 'BrigAluModifierMask'
                                    },
           'BrigMemoryOrder' => {
                                  'mnemo#deps' => [],
                                  'entries' => [
                                                 {
                                                   'name' => 'BRIG_MEMORY_ORDER_NONE',
                                                   'mnemo' => '',
                                                   'val' => '0'
                                                 },
                                                 {
                                                   'mnemo' => 'rlx',
                                                   'val' => '1',
                                                   'name' => 'BRIG_MEMORY_ORDER_RELAXED'
                                                 },
                                                 {
                                                   'mnemo' => 'scacq',
                                                   'val' => '2',
                                                   'name' => 'BRIG_MEMORY_ORDER_SC_ACQUIRE'
                                                 },
                                                 {
                                                   'mnemo' => 'screl',
                                                   'val' => '3',
                                                   'name' => 'BRIG_MEMORY_ORDER_SC_RELEASE'
                                                 },
                                                 {
                                                   'name' => 'BRIG_MEMORY_ORDER_SC_ACQUIRE_RELEASE',
                                                   'mnemo' => 'scar',
                                                   'val' => '4 '
                                                 }
                                               ],
                                  'mnemo_token' => '_EMMemoryOrder',
                                  'mnemo' => sub { "DUMMY" },
                                  'name' => 'BrigMemoryOrder',
                                  'mnemo#calcState' => 'done'
                                },
           'BrigMemoryScope2' => {
                                   'mnemo_token' => 'EMemoryScope',
                                   'mnemo#calcState' => 'done',
                                   'mnemo' => sub { "DUMMY" },
                                   'mnemo_context' => 'EMemoryScopeContext',
                                   'entries' => [
                                                  {
                                                    'mnemo' => '',
                                                    'val' => '0',
                                                    'name' => 'BRIG_MEMORY_SCOPE2_NONE'
                                                  },
                                                  {
                                                    'name' => 'BRIG_MEMORY_SCOPE2_WORKITEM',
                                                    'val' => '1',
                                                    'mnemo' => 'wi'
                                                  },
                                                  {
                                                    'name' => 'BRIG_MEMORY_SCOPE2_WAVEFRONT',
                                                    'val' => '2',
                                                    'mnemo' => 'wv'
                                                  },
                                                  {
                                                    'name' => 'BRIG_MEMORY_SCOPE2_WORKGROUP',
                                                    'mnemo' => 'wg',
                                                    'val' => '3'
                                                  },
                                                  {
                                                    'name' => 'BRIG_MEMORY_SCOPE2_COMPONENT',
                                                    'val' => '4',
                                                    'mnemo' => 'cmp'
                                                  },
                                                  {
                                                    'name' => 'BRIG_MEMORY_SCOPE2_SYSTEM',
                                                    'mnemo' => 'sys',
                                                    'val' => '5 '
                                                  }
                                                ],
                                   'mnemo#deps' => [],
                                   'mnemo_scanner' => 'Instructions',
                                   'name' => 'BrigMemoryScope2'
                                 },
           'BrigImageQuery' => {
                                 'mnemo' => sub { "DUMMY" },
                                 'name' => 'BrigImageQuery',
                                 'mnemo#calcState' => 'done',
                                 'mnemo#deps' => [],
                                 'entries' => [
                                                {
                                                  'val' => '0',
                                                  'mnemo' => 'width',
                                                  'name' => 'BRIG_IMAGE_QUERY_WIDTH'
                                                },
                                                {
                                                  'name' => 'BRIG_IMAGE_QUERY_HEIGHT',
                                                  'mnemo' => 'height',
                                                  'val' => '1'
                                                },
                                                {
                                                  'name' => 'BRIG_IMAGE_QUERY_DEPTH',
                                                  'val' => '2',
                                                  'mnemo' => 'depth'
                                                },
                                                {
                                                  'name' => 'BRIG_IMAGE_QUERY_ARRAY',
                                                  'val' => '3',
                                                  'mnemo' => 'array'
                                                },
                                                {
                                                  'name' => 'BRIG_IMAGE_QUERY_CHANNELORDER',
                                                  'val' => '4',
                                                  'mnemo' => 'channelorder'
                                                },
                                                {
                                                  'val' => '5',
                                                  'mnemo' => 'channeltype',
                                                  'name' => 'BRIG_IMAGE_QUERY_CHANNELTYPE'
                                                }
                                              ]
                               },
           'BrigTypeX' => {
                            'name' => 'BrigTypeX',
                            'dispatch_proto' => 'template<typename RetType, typename Visitor>
RetType dispatchByType_gen(unsigned type, Visitor& v)',
                            'dispatch_incfile' => 'TemplateUtilities',
                            'numBits_switch' => 'true',
                            'numBytes_default' => 'assert(0); return 0',
                            'dispatch#deps' => [],
                            'numBits#calcState' => 'done',
                            'numBits' => sub { "DUMMY" },
                            'mnemo' => sub { "DUMMY" },
                            'dispatch' => sub { "DUMMY" },
                            'numBytes#calcState' => 'done',
                            'mnemo_token' => '_EMType',
                            'numBits_proto' => 'unsigned getBrigTypeNumBits(unsigned arg)',
                            'numBytes_switch' => 'true',
                            'numBytes' => sub { "DUMMY" },
                            'numBits_default' => 'assert(0); return 0',
                            'numBits#deps' => [],
                            'numBytes#deps' => [
                                                 'numBits'
                                               ],
                            'dispatch#calcState' => 'done',
                            'mnemo#deps' => [],
                            'entries' => [
                                           {
                                             'mnemo' => '',
                                             'val' => '0',
                                             'dispatch' => 'v.visitNone(type)',
                                             'name' => 'BRIG_TYPE_NONE'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_U8',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U8> >()',
                                             'val' => '1',
                                             'numBytes' => '1',
                                             'ctype' => 'uint8_t',
                                             'mnemo' => 'u8',
                                             'numBits' => '8'
                                           },
                                           {
                                             'numBits' => '16',
                                             'numBytes' => '2',
                                             'mnemo' => 'u16',
                                             'ctype' => 'uint16_t',
                                             'val' => '2',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U16> >()',
                                             'name' => 'BRIG_TYPE_U16'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_U32',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U32> >()',
                                             'val' => '3',
                                             'numBytes' => '4',
                                             'mnemo' => 'u32',
                                             'numBits' => '32',
                                             'ctype' => 'uint32_t'
                                           },
                                           {
                                             'val' => '4',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U64> >()',
                                             'numBytes' => '8',
                                             'numBits' => '64',
                                             'mnemo' => 'u64',
                                             'ctype' => 'uint64_t',
                                             'name' => 'BRIG_TYPE_U64'
                                           },
                                           {
                                             'numBytes' => '1',
                                             'mnemo' => 's8',
                                             'ctype' => 'int8_t',
                                             'numBits' => '8',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S8> >()',
                                             'val' => '5',
                                             'name' => 'BRIG_TYPE_S8'
                                           },
                                           {
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S16> >()',
                                             'val' => '6',
                                             'numBits' => '16',
                                             'numBytes' => '2',
                                             'mnemo' => 's16',
                                             'ctype' => 'int16_t',
                                             'name' => 'BRIG_TYPE_S16'
                                           },
                                           {
                                             'val' => '7',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S32> >()',
                                             'mnemo' => 's32',
                                             'numBytes' => '4',
                                             'numBits' => '32',
                                             'ctype' => 'int32_t',
                                             'name' => 'BRIG_TYPE_S32'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_S64',
                                             'val' => '8',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S64> >()',
                                             'numBytes' => '8',
                                             'numBits' => '64',
                                             'mnemo' => 's64',
                                             'ctype' => 'int64_t'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_F16',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F16> >()',
                                             'val' => '9',
                                             'numBytes' => '2',
                                             'mnemo' => 'f16',
                                             'numBits' => '16',
                                             'ctype' => 'f16_t'
                                           },
                                           {
                                             'val' => '10',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F32> >()',
                                             'mnemo' => 'f32',
                                             'numBytes' => '4',
                                             'numBits' => '32',
                                             'ctype' => 'float',
                                             'name' => 'BRIG_TYPE_F32'
                                           },
                                           {
                                             'mnemo' => 'f64',
                                             'numBytes' => '8',
                                             'numBits' => '64',
                                             'ctype' => 'double',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F64> >()',
                                             'val' => '11',
                                             'name' => 'BRIG_TYPE_F64'
                                           },
                                           {
                                             'mnemo' => 'b1',
                                             'ctype' => 'bool',
                                             'numBits' => '1',
                                             'numBytes' => '1',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_B1> >()',
                                             'val' => '12',
                                             'name' => 'BRIG_TYPE_B1'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_B8',
                                             'val' => '13',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_B8> >()',
                                             'numBytes' => '1',
                                             'mnemo' => 'b8',
                                             'numBits' => '8',
                                             'ctype' => 'uint8_t'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_B16',
                                             'val' => '14',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_B16> >()',
                                             'mnemo' => 'b16',
                                             'numBytes' => '2',
                                             'ctype' => 'uint16_t',
                                             'numBits' => '16'
                                           },
                                           {
                                             'mnemo' => 'b32',
                                             'numBytes' => '4',
                                             'numBits' => '32',
                                             'ctype' => 'uint32_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_B32> >()',
                                             'val' => '15',
                                             'name' => 'BRIG_TYPE_B32'
                                           },
                                           {
                                             'numBytes' => '8',
                                             'mnemo' => 'b64',
                                             'ctype' => 'uint64_t',
                                             'numBits' => '64',
                                             'val' => '16',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_B64> >()',
                                             'name' => 'BRIG_TYPE_B64'
                                           },
                                           {
                                             'numBytes' => '16',
                                             'numBits' => '128',
                                             'mnemo' => 'b128',
                                             'ctype' => 'b128_t',
                                             'val' => '17',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_B128> >()',
                                             'name' => 'BRIG_TYPE_B128'
                                           },
                                           {
                                             'numBytes' => '8',
                                             'mnemo' => 'samp',
                                             'numBits' => '64',
                                             'val' => '18',
                                             'dispatch' => 'v.visitNone(type)',
                                             'name' => 'BRIG_TYPE_SAMP'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_ROIMG',
                                             'numBits' => '64',
                                             'numBytes' => '8',
                                             'mnemo' => 'roimg',
                                             'val' => '19',
                                             'dispatch' => 'v.visitNone(type)'
                                           },
                                           {
                                             'numBits' => '64',
                                             'numBytes' => '8',
                                             'mnemo' => 'woimg',
                                             'dispatch' => 'v.visitNone(type)',
                                             'val' => '20',
                                             'name' => 'BRIG_TYPE_WOIMG'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_RWIMG',
                                             'dispatch' => 'v.visitNone(type)',
                                             'val' => '21',
                                             'numBits' => '64',
                                             'numBytes' => '8',
                                             'mnemo' => 'rwimg'
                                           },
                                           {
                                             'val' => '22',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_SIG32> >()',
                                             'mnemo' => 'sig32',
                                             'numBytes' => '8',
                                             'numBits' => '64',
                                             'name' => 'BRIG_TYPE_SIG32'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_SIG64',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_SIG64> >()',
                                             'val' => '23',
                                             'numBytes' => '8',
                                             'mnemo' => 'sig64',
                                             'numBits' => '64'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_U8X4',
                                             'mnemo' => 'u8x4',
                                             'numBytes' => '4',
                                             'ctype' => 'uint8_t',
                                             'numBits' => 32,
                                             'val' => 'BRIG_TYPE_U8 | BRIG_TYPE_PACK_32',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U8X4> >()'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_U8X8',
                                             'numBits' => 64,
                                             'numBytes' => '8',
                                             'mnemo' => 'u8x8',
                                             'ctype' => 'uint8_t',
                                             'val' => 'BRIG_TYPE_U8 | BRIG_TYPE_PACK_64',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U8X8> >()'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_U8X16',
                                             'val' => 'BRIG_TYPE_U8 | BRIG_TYPE_PACK_128',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U8X16> >()',
                                             'numBytes' => '16',
                                             'mnemo' => 'u8x16',
                                             'numBits' => 128,
                                             'ctype' => 'uint8_t'
                                           },
                                           {
                                             'ctype' => 'uint16_t',
                                             'numBytes' => '4',
                                             'mnemo' => 'u16x2',
                                             'numBits' => 32,
                                             'val' => 'BRIG_TYPE_U16 | BRIG_TYPE_PACK_32',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U16X2> >()',
                                             'name' => 'BRIG_TYPE_U16X2'
                                           },
                                           {
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U16X4> >()',
                                             'val' => 'BRIG_TYPE_U16 | BRIG_TYPE_PACK_64',
                                             'numBytes' => '8',
                                             'mnemo' => 'u16x4',
                                             'ctype' => 'uint16_t',
                                             'numBits' => 64,
                                             'name' => 'BRIG_TYPE_U16X4'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_U16X8',
                                             'numBits' => 128,
                                             'numBytes' => '16',
                                             'mnemo' => 'u16x8',
                                             'ctype' => 'uint16_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U16X8> >()',
                                             'val' => 'BRIG_TYPE_U16 | BRIG_TYPE_PACK_128'
                                           },
                                           {
                                             'numBytes' => '8',
                                             'mnemo' => 'u32x2',
                                             'ctype' => 'uint32_t',
                                             'numBits' => 64,
                                             'val' => 'BRIG_TYPE_U32 | BRIG_TYPE_PACK_64',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U32X2> >()',
                                             'name' => 'BRIG_TYPE_U32X2'
                                           },
                                           {
                                             'mnemo' => 'u32x4',
                                             'numBytes' => '16',
                                             'numBits' => 128,
                                             'ctype' => 'uint32_t',
                                             'val' => 'BRIG_TYPE_U32 | BRIG_TYPE_PACK_128',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U32X4> >()',
                                             'name' => 'BRIG_TYPE_U32X4'
                                           },
                                           {
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U64X2> >()',
                                             'val' => 'BRIG_TYPE_U64 | BRIG_TYPE_PACK_128',
                                             'numBytes' => '16',
                                             'mnemo' => 'u64x2',
                                             'numBits' => 128,
                                             'ctype' => 'uint64_t',
                                             'name' => 'BRIG_TYPE_U64X2'
                                           },
                                           {
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S8X4> >()',
                                             'val' => 'BRIG_TYPE_S8 | BRIG_TYPE_PACK_32',
                                             'numBytes' => '4',
                                             'ctype' => 'int8_t',
                                             'mnemo' => 's8x4',
                                             'numBits' => 32,
                                             'name' => 'BRIG_TYPE_S8X4'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_S8X8',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S8X8> >()',
                                             'val' => 'BRIG_TYPE_S8 | BRIG_TYPE_PACK_64',
                                             'numBytes' => '8',
                                             'mnemo' => 's8x8',
                                             'ctype' => 'int8_t',
                                             'numBits' => 64
                                           },
                                           {
                                             'numBytes' => '16',
                                             'mnemo' => 's8x16',
                                             'numBits' => 128,
                                             'ctype' => 'int8_t',
                                             'val' => 'BRIG_TYPE_S8 | BRIG_TYPE_PACK_128',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S8X16> >()',
                                             'name' => 'BRIG_TYPE_S8X16'
                                           },
                                           {
                                             'numBytes' => '4',
                                             'mnemo' => 's16x2',
                                             'ctype' => 'int16_t',
                                             'numBits' => 32,
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S16X2> >()',
                                             'val' => 'BRIG_TYPE_S16 | BRIG_TYPE_PACK_32',
                                             'name' => 'BRIG_TYPE_S16X2'
                                           },
                                           {
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S16X4> >()',
                                             'val' => 'BRIG_TYPE_S16 | BRIG_TYPE_PACK_64',
                                             'numBytes' => '8',
                                             'mnemo' => 's16x4',
                                             'ctype' => 'int16_t',
                                             'numBits' => 64,
                                             'name' => 'BRIG_TYPE_S16X4'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_S16X8',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S16X8> >()',
                                             'val' => 'BRIG_TYPE_S16 | BRIG_TYPE_PACK_128',
                                             'numBytes' => '16',
                                             'mnemo' => 's16x8',
                                             'ctype' => 'int16_t',
                                             'numBits' => 128
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_S32X2',
                                             'mnemo' => 's32x2',
                                             'numBytes' => '8',
                                             'ctype' => 'int32_t',
                                             'numBits' => 64,
                                             'val' => 'BRIG_TYPE_S32 | BRIG_TYPE_PACK_64',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S32X2> >()'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_S32X4',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S32X4> >()',
                                             'val' => 'BRIG_TYPE_S32 | BRIG_TYPE_PACK_128',
                                             'numBits' => 128,
                                             'numBytes' => '16',
                                             'mnemo' => 's32x4',
                                             'ctype' => 'int32_t'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_S64X2',
                                             'numBytes' => '16',
                                             'mnemo' => 's64x2',
                                             'numBits' => 128,
                                             'ctype' => 'int64_t',
                                             'val' => 'BRIG_TYPE_S64 | BRIG_TYPE_PACK_128',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S64X2> >()'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_F16X2',
                                             'val' => 'BRIG_TYPE_F16 | BRIG_TYPE_PACK_32',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F16X2> >()',
                                             'ctype' => 'f16_t',
                                             'numBytes' => '4',
                                             'mnemo' => 'f16x2',
                                             'numBits' => 32
                                           },
                                           {
                                             'ctype' => 'f16_t',
                                             'numBytes' => '8',
                                             'mnemo' => 'f16x4',
                                             'numBits' => 64,
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F16X4> >()',
                                             'val' => 'BRIG_TYPE_F16 | BRIG_TYPE_PACK_64',
                                             'name' => 'BRIG_TYPE_F16X4'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_F16X8',
                                             'val' => 'BRIG_TYPE_F16 | BRIG_TYPE_PACK_128',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F16X8> >()',
                                             'numBytes' => '16',
                                             'mnemo' => 'f16x8',
                                             'numBits' => 128,
                                             'ctype' => 'f16_t'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_F32X2',
                                             'numBytes' => '8',
                                             'mnemo' => 'f32x2',
                                             'ctype' => 'float',
                                             'numBits' => 64,
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F32X2> >()',
                                             'val' => 'BRIG_TYPE_F32 | BRIG_TYPE_PACK_64'
                                           },
                                           {
                                             'val' => 'BRIG_TYPE_F32 | BRIG_TYPE_PACK_128',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F32X4> >()',
                                             'numBytes' => '16',
                                             'ctype' => 'float',
                                             'mnemo' => 'f32x4',
                                             'numBits' => 128,
                                             'name' => 'BRIG_TYPE_F32X4'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_F64X2',
                                             'val' => 'BRIG_TYPE_F64 | BRIG_TYPE_PACK_128',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F64X2> >()',
                                             'mnemo' => 'f64x2',
                                             'numBytes' => '16',
                                             'ctype' => 'double',
                                             'numBits' => 128
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_INVALID',
                                             'skip' => 'true',
                                             'dispatch' => 'v.visitNone(type)',
                                             'val' => '-1 ',
                                             'mnemo' => 'invalid'
                                           }
                                         ],
                            'numBytes_proto' => 'unsigned getBrigTypeNumBytes(unsigned arg)',
                            'mnemo#calcState' => 'done',
                            'dispatch_default' => 'return v.visitNone(type)',
                            'dispatch_switch' => 'true',
                            'dispatch_arg' => 'type'
                          },
           'BrigOpcode' => {
                             'mnemo_scanner' => 'Instructions',
                             'numdst#calcState' => 'done',
                             'numdst' => sub { "DUMMY" },
                             'opcodevis' => sub { "DUMMY" },
                             'mnemo_token' => 'EInstruction',
                             'opndparser_incfile' => 'ParserUtilities',
                             'opndparser_default' => 'return &Parser::parseOperands',
                             'hasType_proto' => 'bool instHasType(Brig::BrigOpcode16_t arg)',
                             'opndparser_switch' => 'true',
                             'psopnd' => sub { "DUMMY" },
                             'opcodevis#deps' => [
                                                   'pscode'
                                                 ],
                             'hasType#deps' => [
                                                 'k'
                                               ],
                             'psopnd#deps' => [],
                             'hasType' => sub { "DUMMY" },
                             'opcodeparser_proto' => 'OpcodeParser getOpcodeParser(Brig::BrigOpcode16_t arg)',
                             'k#calcState' => 'done',
                             'opcodeparser_default' => 'return parseMnemoBasic',
                             'opcodevis_default' => 'return RetType()',
                             'k#deps' => [],
                             'opcodeparser' => sub { "DUMMY" },
                             'opndparser_proto' => 'Parser::OperandParser Parser::getOperandParser(Brig::BrigOpcode16_t arg)',
                             'vecOpndIndex_incfile' => 'ParserUtilities',
                             'ftz_switch' => 'true',
                             'ftz_incfile' => 'ItemUtils',
                             'mnemo' => sub { "DUMMY" },
                             'semsupport' => sub { "DUMMY" },
                             'opcodeparser_incfile' => 'ParserUtilities',
                             'pscode#deps' => [
                                                'k'
                                              ],
                             'mnemo#deps' => [],
                             'ftz#calcState' => 'done',
                             'hasType_default' => 'return true',
                             'hasType#calcState' => 'done',
                             'has_memory_order' => sub { "DUMMY" },
                             'mnemo#calcState' => 'done',
                             'opcodevis_arg' => 'inst.opcode()',
                             'semsupport#deps' => [
                                                    'has_memory_order'
                                                  ],
                             'mnemo_context' => 'EDefaultContext',
                             'ftz_proto' => 'inline bool instSupportsFtz(Brig::BrigOpcode16_t arg)',
                             'opcodevis#calcState' => 'done',
                             'ftz' => sub { "DUMMY" },
                             'k' => sub { "DUMMY" },
                             'name' => 'BrigOpcode',
                             'psopnd#calcState' => 'done',
                             'opcodevis_proto' => 'template <typename RetType, typename Visitor> RetType visitOpcode_gen(HSAIL_ASM::Inst inst, Visitor& vis)',
                             'opndparser' => sub { "DUMMY" },
                             'pscode#calcState' => 'done',
                             'numdst#deps' => [],
                             'numdst_switch' => 'true',
                             'opndparser#deps' => [
                                                    'psopnd'
                                                  ],
                             'opcodevis_switch' => 'true',
                             'opcodeparser_switch' => 'true',
                             'has_memory_order#deps' => [],
                             'ftz_default' => 'return false',
                             'vecOpndIndex_default' => 'return -1',
                             'hasType_switch' => 'true',
                             'ftz#deps' => [
                                             'k'
                                           ],
                             'opcodevis_incfile' => 'ItemUtils',
                             'opcodeparser#deps' => [
                                                      'pscode'
                                                    ],
                             'has_memory_order#calcState' => 'done',
                             'semsupport#calcState' => 'done',
                             'opndparser#calcState' => 'done',
                             'tdcaption' => 'Instruction Opcodes',
                             'vecOpndIndex_proto' => 'int vecOpndIndex(Brig::BrigOpcode16_t arg)',
                             'numdst_default' => 'return 1',
                             'entries' => [
                                            {
                                              'mnemo' => 'nop',
                                              'val' => '0',
                                              'opcodeparser' => 'parseMnemoNop',
                                              'opcodevis' => 'vis.visitOpcode_NOP                            (inst)',
                                              'hasType' => 'false',
                                              'k' => 'NOP',
                                              'name' => 'BRIG_OPCODE_NOP',
                                              'pscode' => 'Nop'
                                            },
                                            {
                                              'pscode' => 'BasicOrMod',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD',
                                              'name' => 'BRIG_OPCODE_ABS',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'opcodevis' => 'vis.visitOpcode_ABS                            (inst)',
                                              'val' => '1',
                                              'mnemo' => 'abs'
                                            },
                                            {
                                              'val' => '2',
                                              'mnemo' => 'add',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'opcodevis' => 'vis.visitOpcode_ADD                            (inst)',
                                              'name' => 'BRIG_OPCODE_ADD',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD',
                                              'pscode' => 'BasicOrMod'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_BORROW                         (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_BORROW',
                                              'k' => 'BASIC',
                                              'val' => '3',
                                              'mnemo' => 'borrow'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_CARRY',
                                              'k' => 'BASIC',
                                              'val' => '4',
                                              'mnemo' => 'carry',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_CARRY                          (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'pscode' => 'BasicOrMod',
                                              'name' => 'BRIG_OPCODE_CEIL',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD',
                                              'opcodevis' => 'vis.visitOpcode_CEIL                           (inst)',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'val' => '5',
                                              'mnemo' => 'ceil'
                                            },
                                            {
                                              'pscode' => 'BasicOrMod',
                                              'name' => 'BRIG_OPCODE_COPYSIGN',
                                              'k' => 'BASIC_OR_MOD',
                                              'ftz' => 'true',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'opcodevis' => 'vis.visitOpcode_COPYSIGN                       (inst)',
                                              'val' => '6',
                                              'mnemo' => 'copysign'
                                            },
                                            {
                                              'pscode' => 'BasicOrMod',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD',
                                              'name' => 'BRIG_OPCODE_DIV',
                                              'opcodevis' => 'vis.visitOpcode_DIV                            (inst)',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'val' => '7',
                                              'mnemo' => 'div'
                                            },
                                            {
                                              'opcodevis' => 'vis.visitOpcode_FLOOR                          (inst)',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'val' => '8',
                                              'mnemo' => 'floor',
                                              'pscode' => 'BasicOrMod',
                                              'name' => 'BRIG_OPCODE_FLOOR',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD'
                                            },
                                            {
                                              'pscode' => 'BasicOrMod',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD',
                                              'name' => 'BRIG_OPCODE_FMA',
                                              'opcodevis' => 'vis.visitOpcode_FMA                            (inst)',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'val' => '9',
                                              'mnemo' => 'fma'
                                            },
                                            {
                                              'mnemo' => 'fract',
                                              'val' => '10',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'opcodevis' => 'vis.visitOpcode_FRACT                          (inst)',
                                              'name' => 'BRIG_OPCODE_FRACT',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD',
                                              'pscode' => 'BasicOrMod'
                                            },
                                            {
                                              'opcodevis' => 'vis.visitOpcode_MAD                            (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'mad',
                                              'val' => '11',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_MAD'
                                            },
                                            {
                                              'opcodevis' => 'vis.visitOpcode_MAX                            (inst)',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'mnemo' => 'max',
                                              'val' => '12',
                                              'pscode' => 'BasicOrMod',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD',
                                              'name' => 'BRIG_OPCODE_MAX'
                                            },
                                            {
                                              'pscode' => 'BasicOrMod',
                                              'name' => 'BRIG_OPCODE_MIN',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD',
                                              'opcodevis' => 'vis.visitOpcode_MIN                            (inst)',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'mnemo' => 'min',
                                              'val' => '13'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'opcodevis' => 'vis.visitOpcode_MUL                            (inst)',
                                              'mnemo' => 'mul',
                                              'val' => '14',
                                              'pscode' => 'BasicOrMod',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD',
                                              'name' => 'BRIG_OPCODE_MUL'
                                            },
                                            {
                                              'val' => '15',
                                              'mnemo' => 'mulhi',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'opcodevis' => 'vis.visitOpcode_MULHI                          (inst)',
                                              'k' => 'BASIC_OR_MOD',
                                              'ftz' => 'true',
                                              'name' => 'BRIG_OPCODE_MULHI',
                                              'pscode' => 'BasicOrMod'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'opcodevis' => 'vis.visitOpcode_NEG                            (inst)',
                                              'val' => '16',
                                              'mnemo' => 'neg',
                                              'pscode' => 'BasicOrMod',
                                              'k' => 'BASIC_OR_MOD',
                                              'ftz' => 'true',
                                              'name' => 'BRIG_OPCODE_NEG'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_REM',
                                              'mnemo' => 'rem',
                                              'k' => 'BASIC',
                                              'val' => '17',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_REM                            (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'mnemo' => 'rint',
                                              'val' => '18',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'opcodevis' => 'vis.visitOpcode_RINT                           (inst)',
                                              'name' => 'BRIG_OPCODE_RINT',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD',
                                              'pscode' => 'BasicOrMod'
                                            },
                                            {
                                              'pscode' => 'BasicOrMod',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD',
                                              'name' => 'BRIG_OPCODE_SQRT',
                                              'opcodevis' => 'vis.visitOpcode_SQRT                           (inst)',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'mnemo' => 'sqrt',
                                              'val' => '19'
                                            },
                                            {
                                              'pscode' => 'BasicOrMod',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD',
                                              'name' => 'BRIG_OPCODE_SUB',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'opcodevis' => 'vis.visitOpcode_SUB                            (inst)',
                                              'val' => '20',
                                              'mnemo' => 'sub'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'opcodevis' => 'vis.visitOpcode_TRUNC                          (inst)',
                                              'val' => '21',
                                              'mnemo' => 'trunc',
                                              'pscode' => 'BasicOrMod',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD',
                                              'name' => 'BRIG_OPCODE_TRUNC'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_MAD24',
                                              'val' => '22',
                                              'k' => 'BASIC',
                                              'mnemo' => 'mad24',
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_MAD24                          (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_MAD24HI                        (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'name' => 'BRIG_OPCODE_MAD24HI',
                                              'val' => '23',
                                              'k' => 'BASIC',
                                              'mnemo' => 'mad24hi'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'val' => '24',
                                              'mnemo' => 'mul24',
                                              'name' => 'BRIG_OPCODE_MUL24',
                                              'opcodevis' => 'vis.visitOpcode_MUL24                          (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_MUL24HI',
                                              'mnemo' => 'mul24hi',
                                              'val' => '25',
                                              'k' => 'BASIC',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_MUL24HI                        (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'val' => '26',
                                              'mnemo' => 'shl',
                                              'name' => 'BRIG_OPCODE_SHL',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_SHL                            (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'mnemo' => 'shr',
                                              'k' => 'BASIC',
                                              'val' => '27',
                                              'name' => 'BRIG_OPCODE_SHR',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_SHR                            (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'opcodevis' => 'vis.visitOpcode_AND                            (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'and',
                                              'k' => 'BASIC',
                                              'val' => '28',
                                              'name' => 'BRIG_OPCODE_AND'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_NOT',
                                              'val' => '29',
                                              'k' => 'BASIC',
                                              'mnemo' => 'not',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_NOT                            (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'mnemo' => 'or',
                                              'k' => 'BASIC',
                                              'val' => '30',
                                              'name' => 'BRIG_OPCODE_OR',
                                              'opcodevis' => 'vis.visitOpcode_OR                             (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'pscode' => 'SourceType',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'opcodevis' => 'vis.visitOpcode_POPCOUNT                       (HSAIL_ASM::InstSourceType(inst))',
                                              'name' => 'BRIG_OPCODE_POPCOUNT',
                                              'val' => '31',
                                              'k' => 'SOURCE_TYPE',
                                              'mnemo' => 'popcount'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_XOR                            (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'name' => 'BRIG_OPCODE_XOR',
                                              'k' => 'BASIC',
                                              'val' => '32',
                                              'mnemo' => 'xor'
                                            },
                                            {
                                              'val' => '33',
                                              'k' => 'BASIC',
                                              'mnemo' => 'bitextract',
                                              'name' => 'BRIG_OPCODE_BITEXTRACT',
                                              'opcodevis' => 'vis.visitOpcode_BITEXTRACT                     (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_BITINSERT                      (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'name' => 'BRIG_OPCODE_BITINSERT',
                                              'k' => 'BASIC',
                                              'val' => '34',
                                              'mnemo' => 'bitinsert'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_BITMASK',
                                              'mnemo' => 'bitmask',
                                              'val' => '35',
                                              'k' => 'BASIC',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_BITMASK                        (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'val' => '36',
                                              'k' => 'BASIC',
                                              'mnemo' => 'bitrev',
                                              'name' => 'BRIG_OPCODE_BITREV',
                                              'opcodevis' => 'vis.visitOpcode_BITREV                         (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_BITSELECT                      (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_BITSELECT',
                                              'mnemo' => 'bitselect',
                                              'val' => '37',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'mnemo' => 'firstbit',
                                              'val' => '38',
                                              'k' => 'SOURCE_TYPE',
                                              'name' => 'BRIG_OPCODE_FIRSTBIT',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'opcodevis' => 'vis.visitOpcode_FIRSTBIT                       (HSAIL_ASM::InstSourceType(inst))',
                                              'pscode' => 'SourceType'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_LASTBIT',
                                              'mnemo' => 'lastbit',
                                              'k' => 'SOURCE_TYPE',
                                              'val' => '39',
                                              'pscode' => 'SourceType',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'opcodevis' => 'vis.visitOpcode_LASTBIT                        (HSAIL_ASM::InstSourceType(inst))'
                                            },
                                            {
                                              'k' => 'SOURCE_TYPE',
                                              'vecOpndIndex' => '1',
                                              'name' => 'BRIG_OPCODE_COMBINE',
                                              'pscode' => 'SourceType',
                                              'val' => '40',
                                              'mnemo' => 'combine',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'opcodevis' => 'vis.visitOpcode_COMBINE                        (HSAIL_ASM::InstSourceType(inst))'
                                            },
                                            {
                                              'opcodevis' => 'vis.visitOpcode_EXPAND                         (HSAIL_ASM::InstSourceType(inst))',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'mnemo' => 'expand',
                                              'val' => '41',
                                              'pscode' => 'SourceType',
                                              'name' => 'BRIG_OPCODE_EXPAND',
                                              'vecOpndIndex' => '0',
                                              'k' => 'SOURCE_TYPE'
                                            },
                                            {
                                              'opcodevis' => 'vis.visitOpcode_LDA                            (HSAIL_ASM::InstAddr(inst))',
                                              'opcodeparser' => 'parseMnemoAddr',
                                              'pscode' => 'Addr',
                                              'val' => '42',
                                              'k' => 'ADDR',
                                              'mnemo' => 'lda',
                                              'name' => 'BRIG_OPCODE_LDA'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_MOV                            (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'mov',
                                              'k' => 'BASIC',
                                              'val' => '43',
                                              'name' => 'BRIG_OPCODE_MOV'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_SHUFFLE',
                                              'mnemo' => 'shuffle',
                                              'k' => 'BASIC',
                                              'val' => '44',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_SHUFFLE                        (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_UNPACKHI',
                                              'val' => '45',
                                              'k' => 'BASIC',
                                              'mnemo' => 'unpackhi',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_UNPACKHI                       (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_UNPACKLO                       (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_UNPACKLO',
                                              'k' => 'BASIC',
                                              'val' => '46',
                                              'mnemo' => 'unpacklo'
                                            },
                                            {
                                              'pscode' => 'SourceType',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'opcodevis' => 'vis.visitOpcode_PACK                           (HSAIL_ASM::InstSourceType(inst))',
                                              'name' => 'BRIG_OPCODE_PACK',
                                              'k' => 'SOURCE_TYPE',
                                              'val' => '47',
                                              'mnemo' => 'pack'
                                            },
                                            {
                                              'k' => 'SOURCE_TYPE',
                                              'val' => '48',
                                              'mnemo' => 'unpack',
                                              'name' => 'BRIG_OPCODE_UNPACK',
                                              'opcodevis' => 'vis.visitOpcode_UNPACK                         (HSAIL_ASM::InstSourceType(inst))',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'pscode' => 'SourceType'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_CMOV                           (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'name' => 'BRIG_OPCODE_CMOV',
                                              'mnemo' => 'cmov',
                                              'val' => '49',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'val' => '50',
                                              'k' => 'SOURCE_TYPE',
                                              'mnemo' => 'class',
                                              'name' => 'BRIG_OPCODE_CLASS',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'opcodevis' => 'vis.visitOpcode_CLASS                          (HSAIL_ASM::InstSourceType(inst))',
                                              'pscode' => 'SourceType'
                                            },
                                            {
                                              'mnemo' => 'ncos',
                                              'val' => '51',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_NCOS',
                                              'opcodevis' => 'vis.visitOpcode_NCOS                           (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_NEXP2                          (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'name' => 'BRIG_OPCODE_NEXP2',
                                              'val' => '52',
                                              'k' => 'BASIC',
                                              'mnemo' => 'nexp2'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_NFMA',
                                              'val' => '53',
                                              'k' => 'BASIC',
                                              'mnemo' => 'nfma',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_NFMA                           (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_NLOG2                          (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'name' => 'BRIG_OPCODE_NLOG2',
                                              'mnemo' => 'nlog2',
                                              'val' => '54',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_NRCP                           (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'name' => 'BRIG_OPCODE_NRCP',
                                              'mnemo' => 'nrcp',
                                              'val' => '55',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_NRSQRT',
                                              'val' => '56',
                                              'k' => 'BASIC',
                                              'mnemo' => 'nrsqrt',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_NRSQRT                         (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'opcodevis' => 'vis.visitOpcode_NSIN                           (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'nsin',
                                              'k' => 'BASIC',
                                              'val' => '57',
                                              'name' => 'BRIG_OPCODE_NSIN'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_NSQRT                          (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'nsqrt',
                                              'val' => '58',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_NSQRT'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_BITALIGN                       (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_BITALIGN',
                                              'mnemo' => 'bitalign',
                                              'val' => '59',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'mnemo' => 'bytealign',
                                              'k' => 'BASIC',
                                              'val' => '60',
                                              'name' => 'BRIG_OPCODE_BYTEALIGN',
                                              'opcodevis' => 'vis.visitOpcode_BYTEALIGN                      (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'k' => 'SOURCE_TYPE',
                                              'val' => '61',
                                              'mnemo' => 'packcvt',
                                              'name' => 'BRIG_OPCODE_PACKCVT',
                                              'opcodevis' => 'vis.visitOpcode_PACKCVT                        (HSAIL_ASM::InstSourceType(inst))',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'pscode' => 'SourceType'
                                            },
                                            {
                                              'k' => 'SOURCE_TYPE',
                                              'val' => '62',
                                              'mnemo' => 'unpackcvt',
                                              'name' => 'BRIG_OPCODE_UNPACKCVT',
                                              'opcodevis' => 'vis.visitOpcode_UNPACKCVT                      (HSAIL_ASM::InstSourceType(inst))',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'pscode' => 'SourceType'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'val' => '63',
                                              'mnemo' => 'lerp',
                                              'name' => 'BRIG_OPCODE_LERP',
                                              'opcodevis' => 'vis.visitOpcode_LERP                           (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_SAD',
                                              'k' => 'SOURCE_TYPE',
                                              'val' => '64',
                                              'mnemo' => 'sad',
                                              'pscode' => 'SourceType',
                                              'opcodevis' => 'vis.visitOpcode_SAD                            (HSAIL_ASM::InstSourceType(inst))',
                                              'opcodeparser' => 'parseMnemoSourceType'
                                            },
                                            {
                                              'pscode' => 'SourceType',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'opcodevis' => 'vis.visitOpcode_SADHI                          (HSAIL_ASM::InstSourceType(inst))',
                                              'name' => 'BRIG_OPCODE_SADHI',
                                              'mnemo' => 'sadhi',
                                              'k' => 'SOURCE_TYPE',
                                              'val' => '65'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_SEGMENTP',
                                              'mnemo' => 'segmentp',
                                              'k' => 'SEG_CVT',
                                              'val' => '66',
                                              'pscode' => 'SegCvt',
                                              'opcodevis' => 'vis.visitOpcode_SEGMENTP                       (HSAIL_ASM::InstSegCvt(inst))',
                                              'opcodeparser' => 'parseMnemoSegCvt'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_FTOS',
                                              'mnemo' => 'ftos',
                                              'val' => '67',
                                              'k' => 'SEG_CVT',
                                              'pscode' => 'SegCvt',
                                              'opcodevis' => 'vis.visitOpcode_FTOS                           (HSAIL_ASM::InstSegCvt(inst))',
                                              'opcodeparser' => 'parseMnemoSegCvt'
                                            },
                                            {
                                              'val' => '68',
                                              'k' => 'SEG_CVT',
                                              'mnemo' => 'stof',
                                              'name' => 'BRIG_OPCODE_STOF',
                                              'opcodevis' => 'vis.visitOpcode_STOF                           (HSAIL_ASM::InstSegCvt(inst))',
                                              'opcodeparser' => 'parseMnemoSegCvt',
                                              'pscode' => 'SegCvt'
                                            },
                                            {
                                              'val' => '69',
                                              'mnemo' => 'cmp',
                                              'opcodeparser' => 'parseMnemoCmp',
                                              'opcodevis' => 'vis.visitOpcode_CMP                            (HSAIL_ASM::InstCmp(inst))',
                                              'ftz' => 'true',
                                              'k' => 'CMP',
                                              'name' => 'BRIG_OPCODE_CMP',
                                              'pscode' => 'Cmp'
                                            },
                                            {
                                              'opcodevis' => 'vis.visitOpcode_CVT                            (HSAIL_ASM::InstCvt(inst))',
                                              'opcodeparser' => 'parseMnemoCvt',
                                              'val' => '70',
                                              'mnemo' => 'cvt',
                                              'pscode' => 'Cvt',
                                              'name' => 'BRIG_OPCODE_CVT',
                                              'ftz' => 'true',
                                              'k' => 'CVT'
                                            },
                                            {
                                              'vecOpndIndex' => '0',
                                              'k' => 'MEM',
                                              'name' => 'BRIG_OPCODE_LD',
                                              'pscode' => 'Mem',
                                              'semsupport' => 'true',
                                              'mnemo' => 'ld',
                                              'val' => '71',
                                              'has_memory_order' => 'true',
                                              'opcodeparser' => 'parseMnemoMem',
                                              'opcodevis' => 'vis.visitOpcode_LD                             (HSAIL_ASM::InstMem(inst))'
                                            },
                                            {
                                              'semsupport' => 'true',
                                              'mnemo' => 'st',
                                              'val' => '72',
                                              'has_memory_order' => 'true',
                                              'opcodevis' => 'vis.visitOpcode_ST                             (HSAIL_ASM::InstMem(inst))',
                                              'opcodeparser' => 'parseMnemoMem',
                                              'vecOpndIndex' => '0',
                                              'k' => 'MEM',
                                              'name' => 'BRIG_OPCODE_ST',
                                              'numdst' => '0',
                                              'pscode' => 'Mem'
                                            },
                                            {
                                              'k' => 'ATOMIC',
                                              'val' => '73',
                                              'mnemo' => 'atomic',
                                              'name' => 'BRIG_OPCODE_ATOMIC',
                                              'opcodeparser' => 'parseMnemoAtomic',
                                              'opcodevis' => 'vis.visitOpcode_ATOMIC                         (HSAIL_ASM::InstAtomic(inst))',
                                              'pscode' => 'Atomic'
                                            },
                                            {
                                              'mnemo' => 'atomicnoret',
                                              'val' => '74',
                                              'opcodeparser' => 'parseMnemoAtomic',
                                              'opcodevis' => 'vis.visitOpcode_ATOMICNORET                    (HSAIL_ASM::InstAtomic(inst))',
                                              'name' => 'BRIG_OPCODE_ATOMICNORET',
                                              'k' => 'ATOMIC',
                                              'pscode' => 'Atomic',
                                              'numdst' => '0'
                                            },
                                            {
                                              'val' => '75',
                                              'k' => 'SIGNAL',
                                              'mnemo' => 'signal',
                                              'name' => 'BRIG_OPCODE_SIGNAL',
                                              'opcodevis' => 'vis.visitOpcode_SIGNAL                         (HSAIL_ASM::InstSignal(inst))',
                                              'opcodeparser' => 'parseMnemoSignal',
                                              'pscode' => 'Signal'
                                            },
                                            {
                                              'mnemo' => 'signalnoret',
                                              'val' => '76',
                                              'opcodeparser' => 'parseMnemoSignal',
                                              'opcodevis' => 'vis.visitOpcode_SIGNALNORET                    (HSAIL_ASM::InstSignal(inst))',
                                              'k' => 'SIGNAL',
                                              'name' => 'BRIG_OPCODE_SIGNALNORET',
                                              'numdst' => '0',
                                              'pscode' => 'Signal'
                                            },
                                            {
                                              'numdst' => '0',
                                              'pscode' => 'MemFence',
                                              'k' => 'MEM_FENCE',
                                              'name' => 'BRIG_OPCODE_MEMFENCE',
                                              'opcodevis' => 'vis.visitOpcode_MEMFENCE                       (HSAIL_ASM::InstMemFence(inst))',
                                              'opcodeparser' => 'parseMnemoMemFence',
                                              'mnemo' => 'memfence',
                                              'val' => '77'
                                            },
                                            {
                                              'opcodevis' => 'vis.visitOpcode_RDIMAGE                        (HSAIL_ASM::InstImage(inst))',
                                              'opcodeparser' => 'parseMnemoImage',
                                              'val' => '78',
                                              'mnemo' => 'rdimage',
                                              'pscode' => 'Image',
                                              'name' => 'BRIG_OPCODE_RDIMAGE',
                                              'k' => 'IMAGE',
                                              'vecOpndIndex' => '0'
                                            },
                                            {
                                              'mnemo' => 'ldimage',
                                              'val' => '79',
                                              'opcodevis' => 'vis.visitOpcode_LDIMAGE                        (HSAIL_ASM::InstImage(inst))',
                                              'opcodeparser' => 'parseMnemoImage',
                                              'vecOpndIndex' => '0',
                                              'k' => 'IMAGE',
                                              'name' => 'BRIG_OPCODE_LDIMAGE',
                                              'pscode' => 'Image'
                                            },
                                            {
                                              'pscode' => 'Image',
                                              'numdst' => '0',
                                              'name' => 'BRIG_OPCODE_STIMAGE',
                                              'k' => 'IMAGE',
                                              'vecOpndIndex' => '0',
                                              'opcodevis' => 'vis.visitOpcode_STIMAGE                        (HSAIL_ASM::InstImage(inst))',
                                              'opcodeparser' => 'parseMnemoImage',
                                              'val' => '80',
                                              'mnemo' => 'stimage'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoQueryImage',
                                              'opcodevis' => 'vis.visitOpcode_QUERYIMAGE                     (HSAIL_ASM::InstQueryImage(inst))',
                                              'pscode' => 'QueryImage',
                                              'mnemo' => 'queryimage',
                                              'val' => '81',
                                              'k' => 'QUERY_IMAGE',
                                              'name' => 'BRIG_OPCODE_QUERYIMAGE'
                                            },
                                            {
                                              'val' => '82',
                                              'k' => 'QUERY_SAMPLER',
                                              'mnemo' => 'querysampler',
                                              'name' => 'BRIG_OPCODE_QUERYSAMPLER',
                                              'opcodeparser' => 'parseMnemoQuerySampler',
                                              'opcodevis' => 'vis.visitOpcode_QUERYSAMPLER                   (HSAIL_ASM::InstQuerySampler(inst))',
                                              'pscode' => 'QuerySampler'
                                            },
                                            {
                                              'val' => '83',
                                              'mnemo' => 'cbr',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'opcodevis' => 'vis.visitOpcode_CBR                            (HSAIL_ASM::InstBr(inst))',
                                              'name' => 'BRIG_OPCODE_CBR',
                                              'k' => 'BR',
                                              'pscode' => 'Br',
                                              'numdst' => '0'
                                            },
                                            {
                                              'opcodevis' => 'vis.visitOpcode_BR                             (HSAIL_ASM::InstBr(inst))',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'hasType' => 'false',
                                              'mnemo' => 'br',
                                              'val' => '84',
                                              'numdst' => '0',
                                              'pscode' => 'Br',
                                              'k' => 'BR',
                                              'name' => 'BRIG_OPCODE_BR'
                                            },
                                            {
                                              'pscode' => 'Br',
                                              'numdst' => '0',
                                              'opndparser' => '&Parser::parseSbrOperands',
                                              'name' => 'BRIG_OPCODE_SBR',
                                              'k' => 'BR',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'opcodevis' => 'vis.visitOpcode_SBR                            (HSAIL_ASM::InstBr(inst))',
                                              'psopnd' => 'SbrOperands',
                                              'mnemo' => 'sbr',
                                              'val' => '85'
                                            },
                                            {
                                              'hasType' => 'false',
                                              'opcodevis' => 'vis.visitOpcode_BARRIER                        (HSAIL_ASM::InstBr(inst))',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'mnemo' => 'barrier',
                                              'val' => '86',
                                              'pscode' => 'Br',
                                              'numdst' => '0',
                                              'name' => 'BRIG_OPCODE_BARRIER',
                                              'k' => 'BR'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_WAVEBARRIER',
                                              'k' => 'BR',
                                              'pscode' => 'Br',
                                              'numdst' => '0',
                                              'val' => '87',
                                              'mnemo' => 'wavebarrier',
                                              'hasType' => 'false',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'opcodevis' => 'vis.visitOpcode_WAVEBARRIER                    (HSAIL_ASM::InstBr(inst))'
                                            },
                                            {
                                              'pscode' => 'Br',
                                              'numdst' => '0',
                                              'name' => 'BRIG_OPCODE_ARRIVEFBAR',
                                              'k' => 'BR',
                                              'hasType' => 'false',
                                              'opcodevis' => 'vis.visitOpcode_ARRIVEFBAR                     (HSAIL_ASM::InstBr(inst))',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'mnemo' => 'arrivefbar',
                                              'val' => '88'
                                            },
                                            {
                                              'mnemo' => 'initfbar',
                                              'val' => '89',
                                              'opcodevis' => 'vis.visitOpcode_INITFBAR                       (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'hasType' => 'false',
                                              'k' => 'BASIC_NO_TYPE',
                                              'name' => 'BRIG_OPCODE_INITFBAR',
                                              'numdst' => '0',
                                              'pscode' => 'BasicNoType'
                                            },
                                            {
                                              'val' => '90',
                                              'mnemo' => 'joinfbar',
                                              'hasType' => 'false',
                                              'opcodevis' => 'vis.visitOpcode_JOINFBAR                       (HSAIL_ASM::InstBr(inst))',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'name' => 'BRIG_OPCODE_JOINFBAR',
                                              'k' => 'BR',
                                              'pscode' => 'Br',
                                              'numdst' => '0'
                                            },
                                            {
                                              'val' => '91',
                                              'mnemo' => 'leavefbar',
                                              'hasType' => 'false',
                                              'opcodevis' => 'vis.visitOpcode_LEAVEFBAR                      (HSAIL_ASM::InstBr(inst))',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'name' => 'BRIG_OPCODE_LEAVEFBAR',
                                              'k' => 'BR',
                                              'pscode' => 'Br',
                                              'numdst' => '0'
                                            },
                                            {
                                              'k' => 'BASIC_NO_TYPE',
                                              'name' => 'BRIG_OPCODE_RELEASEFBAR',
                                              'numdst' => '0',
                                              'pscode' => 'BasicNoType',
                                              'mnemo' => 'releasefbar',
                                              'val' => '92',
                                              'opcodevis' => 'vis.visitOpcode_RELEASEFBAR                    (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'hasType' => 'false'
                                            },
                                            {
                                              'pscode' => 'Br',
                                              'numdst' => '0',
                                              'name' => 'BRIG_OPCODE_WAITFBAR',
                                              'k' => 'BR',
                                              'hasType' => 'false',
                                              'opcodevis' => 'vis.visitOpcode_WAITFBAR                       (HSAIL_ASM::InstBr(inst))',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'mnemo' => 'waitfbar',
                                              'val' => '93'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'val' => '94',
                                              'mnemo' => 'ldf',
                                              'name' => 'BRIG_OPCODE_LDF',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_LDF                            (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'mnemo' => 'activelanecount',
                                              'k' => 'LANE',
                                              'val' => '95',
                                              'name' => 'BRIG_OPCODE_ACTIVELANECOUNT',
                                              'opcodeparser' => 'parseMnemoLane',
                                              'opcodevis' => 'vis.visitOpcode_ACTIVELANECOUNT                (HSAIL_ASM::InstLane(inst))',
                                              'pscode' => 'Lane'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_ACTIVELANEID',
                                              'k' => 'LANE',
                                              'val' => '96',
                                              'mnemo' => 'activelaneid',
                                              'pscode' => 'Lane',
                                              'opcodeparser' => 'parseMnemoLane',
                                              'opcodevis' => 'vis.visitOpcode_ACTIVELANEID                   (HSAIL_ASM::InstLane(inst))'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_ACTIVELANEMASK',
                                              'k' => 'LANE',
                                              'vecOpndIndex' => '0',
                                              'pscode' => 'Lane',
                                              'val' => '97',
                                              'mnemo' => 'activelanemask',
                                              'opcodeparser' => 'parseMnemoLane',
                                              'opcodevis' => 'vis.visitOpcode_ACTIVELANEMASK                 (HSAIL_ASM::InstLane(inst))'
                                            },
                                            {
                                              'pscode' => 'Lane',
                                              'opcodevis' => 'vis.visitOpcode_ACTIVELANESHUFFLE              (HSAIL_ASM::InstLane(inst))',
                                              'opcodeparser' => 'parseMnemoLane',
                                              'name' => 'BRIG_OPCODE_ACTIVELANESHUFFLE',
                                              'mnemo' => 'activelaneshuffle',
                                              'val' => '98',
                                              'k' => 'LANE'
                                            },
                                            {
                                              'hasType' => 'false',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'opcodevis' => 'vis.visitOpcode_CALL                           (HSAIL_ASM::InstBr(inst))',
                                              'psopnd' => 'CallOperands',
                                              'mnemo' => 'call',
                                              'val' => '99',
                                              'pscode' => 'Br',
                                              'opndparser' => '&Parser::parseCallOperands',
                                              'numdst' => '0',
                                              'name' => 'BRIG_OPCODE_CALL',
                                              'k' => 'BR'
                                            },
                                            {
                                              'k' => 'BR',
                                              'name' => 'BRIG_OPCODE_SCALL',
                                              'numdst' => '0',
                                              'opndparser' => '&Parser::parseCallOperands',
                                              'pscode' => 'Br',
                                              'mnemo' => 'scall',
                                              'val' => '100',
                                              'psopnd' => 'CallOperands',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'opcodevis' => 'vis.visitOpcode_SCALL                          (HSAIL_ASM::InstBr(inst))'
                                            },
                                            {
                                              'val' => '101',
                                              'mnemo' => 'icall',
                                              'psopnd' => 'CallOperands',
                                              'opcodevis' => 'vis.visitOpcode_ICALL                          (HSAIL_ASM::InstBr(inst))',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'k' => 'BR',
                                              'name' => 'BRIG_OPCODE_ICALL',
                                              'numdst' => '0',
                                              'opndparser' => '&Parser::parseCallOperands',
                                              'pscode' => 'Br'
                                            },
                                            {
                                              'mnemo' => 'ldi',
                                              'val' => '102',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_LDI',
                                              'opcodevis' => 'vis.visitOpcode_LDI                            (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'hasType' => 'false',
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'opcodevis' => 'vis.visitOpcode_RET                            (HSAIL_ASM::InstBasic(inst))',
                                              'val' => '103',
                                              'mnemo' => 'ret',
                                              'pscode' => 'BasicNoType',
                                              'name' => 'BRIG_OPCODE_RET',
                                              'k' => 'BASIC_NO_TYPE'
                                            },
                                            {
                                              'mnemo' => 'alloca',
                                              'val' => '104',
                                              'k' => 'MEM',
                                              'name' => 'BRIG_OPCODE_ALLOCA',
                                              'opcodeparser' => 'parseMnemoMem',
                                              'opcodevis' => 'vis.visitOpcode_ALLOCA                         (HSAIL_ASM::InstMem(inst))',
                                              'pscode' => 'Mem'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_CURRENTWORKGROUPSIZE           (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_CURRENTWORKGROUPSIZE',
                                              'k' => 'BASIC',
                                              'val' => '105',
                                              'mnemo' => 'currentworkgroupsize'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_DIM                            (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'dim',
                                              'val' => '106',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_DIM'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'val' => '107',
                                              'mnemo' => 'gridgroups',
                                              'name' => 'BRIG_OPCODE_GRIDGROUPS',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_GRIDGROUPS                     (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_GRIDSIZE                       (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'name' => 'BRIG_OPCODE_GRIDSIZE',
                                              'k' => 'BASIC',
                                              'val' => '108',
                                              'mnemo' => 'gridsize'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_PACKETCOMPLETIONSIG            (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic',
                                              'k' => 'BASIC',
                                              'val' => '109',
                                              'mnemo' => 'packetcompletionsig',
                                              'name' => 'BRIG_OPCODE_PACKETCOMPLETIONSIG'
                                            },
                                            {
                                              'opcodevis' => 'vis.visitOpcode_PACKETID                       (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'packetid',
                                              'k' => 'BASIC',
                                              'val' => '110',
                                              'name' => 'BRIG_OPCODE_PACKETID'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_WORKGROUPID',
                                              'k' => 'BASIC',
                                              'val' => '111',
                                              'mnemo' => 'workgroupid',
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_WORKGROUPID                    (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_WORKGROUPSIZE                  (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'name' => 'BRIG_OPCODE_WORKGROUPSIZE',
                                              'mnemo' => 'workgroupsize',
                                              'k' => 'BASIC',
                                              'val' => '112'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_WORKITEMABSID                  (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'workitemabsid',
                                              'val' => '113',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_WORKITEMABSID'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'val' => '114',
                                              'mnemo' => 'workitemflatabsid',
                                              'name' => 'BRIG_OPCODE_WORKITEMFLATABSID',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_WORKITEMFLATABSID              (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_WORKITEMFLATID',
                                              'val' => '115',
                                              'k' => 'BASIC',
                                              'mnemo' => 'workitemflatid',
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_WORKITEMFLATID                 (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_WORKITEMID                     (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'name' => 'BRIG_OPCODE_WORKITEMID',
                                              'mnemo' => 'workitemid',
                                              'k' => 'BASIC',
                                              'val' => '116'
                                            },
                                            {
                                              'opcodevis' => 'vis.visitOpcode_CLEARDETECTEXCEPT              (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '117',
                                              'mnemo' => 'cleardetectexcept',
                                              'pscode' => 'Basic',
                                              'numdst' => '0',
                                              'name' => 'BRIG_OPCODE_CLEARDETECTEXCEPT',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'val' => '118',
                                              'k' => 'BASIC',
                                              'mnemo' => 'getdetectexcept',
                                              'name' => 'BRIG_OPCODE_GETDETECTEXCEPT',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_GETDETECTEXCEPT                (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_SETDETECTEXCEPT                (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'setdetectexcept',
                                              'val' => '119',
                                              'numdst' => '0',
                                              'pscode' => 'Basic',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_SETDETECTEXCEPT'
                                            },
                                            {
                                              'pscode' => 'Queue',
                                              'opcodeparser' => 'parseMnemoQueue',
                                              'opcodevis' => 'vis.visitOpcode_ADDQUEUEWRITEINDEX             (HSAIL_ASM::InstQueue(inst))',
                                              'name' => 'BRIG_OPCODE_ADDQUEUEWRITEINDEX',
                                              'mnemo' => 'addqueuewriteindex',
                                              'k' => 'QUEUE',
                                              'val' => '120'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_AGENTCOUNT                     (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_AGENTCOUNT',
                                              'k' => 'BASIC',
                                              'val' => '121',
                                              'mnemo' => 'agentcount'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_AGENTID                        (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'name' => 'BRIG_OPCODE_AGENTID',
                                              'mnemo' => 'agentid',
                                              'k' => 'BASIC',
                                              'val' => '122'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_CASQUEUEWRITEINDEX',
                                              'mnemo' => 'casqueuewriteindex',
                                              'k' => 'QUEUE',
                                              'val' => '123',
                                              'pscode' => 'Queue',
                                              'opcodeparser' => 'parseMnemoQueue',
                                              'opcodevis' => 'vis.visitOpcode_CASQUEUEWRITEINDEX             (HSAIL_ASM::InstQueue(inst))'
                                            },
                                            {
                                              'opcodevis' => 'vis.visitOpcode_LDK                            (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '124',
                                              'k' => 'BASIC',
                                              'mnemo' => 'ldk',
                                              'name' => 'BRIG_OPCODE_LDK'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoQueue',
                                              'opcodevis' => 'vis.visitOpcode_LDQUEUEREADINDEX               (HSAIL_ASM::InstQueue(inst))',
                                              'pscode' => 'Queue',
                                              'mnemo' => 'ldqueuereadindex',
                                              'k' => 'QUEUE',
                                              'val' => '125',
                                              'name' => 'BRIG_OPCODE_LDQUEUEREADINDEX'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_LDQUEUEWRITEINDEX',
                                              'mnemo' => 'ldqueuewriteindex',
                                              'k' => 'QUEUE',
                                              'val' => '126',
                                              'pscode' => 'Queue',
                                              'opcodevis' => 'vis.visitOpcode_LDQUEUEWRITEINDEX              (HSAIL_ASM::InstQueue(inst))',
                                              'opcodeparser' => 'parseMnemoQueue'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_QUEUEID                        (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'queueid',
                                              'k' => 'BASIC',
                                              'val' => '127',
                                              'name' => 'BRIG_OPCODE_QUEUEID'
                                            },
                                            {
                                              'mnemo' => 'queueptr',
                                              'k' => 'BASIC',
                                              'val' => '128',
                                              'name' => 'BRIG_OPCODE_QUEUEPTR',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_QUEUEPTR                       (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'k' => 'QUEUE',
                                              'name' => 'BRIG_OPCODE_STQUEUEREADINDEX',
                                              'numdst' => '0',
                                              'pscode' => 'Queue',
                                              'val' => '129',
                                              'mnemo' => 'stqueuereadindex',
                                              'opcodeparser' => 'parseMnemoQueue',
                                              'opcodevis' => 'vis.visitOpcode_STQUEUEREADINDEX               (HSAIL_ASM::InstQueue(inst))'
                                            },
                                            {
                                              'k' => 'QUEUE',
                                              'name' => 'BRIG_OPCODE_STQUEUEWRITEINDEX',
                                              'numdst' => '0',
                                              'pscode' => 'Queue',
                                              'val' => '130',
                                              'mnemo' => 'stqueuewriteindex',
                                              'opcodeparser' => 'parseMnemoQueue',
                                              'opcodevis' => 'vis.visitOpcode_STQUEUEWRITEINDEX              (HSAIL_ASM::InstQueue(inst))'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_CLOCK',
                                              'k' => 'BASIC',
                                              'val' => '131',
                                              'mnemo' => 'clock',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_CLOCK                          (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_CUID',
                                              'val' => '132',
                                              'k' => 'BASIC',
                                              'mnemo' => 'cuid',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_CUID                           (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'numdst' => '0',
                                              'name' => 'BRIG_OPCODE_DEBUGTRAP',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.visitOpcode_DEBUGTRAP                      (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'mnemo' => 'debugtrap',
                                              'val' => '133'
                                            },
                                            {
                                              'val' => '134',
                                              'k' => 'BASIC',
                                              'mnemo' => 'groupbaseptr',
                                              'name' => 'BRIG_OPCODE_GROUPBASEPTR',
                                              'opcodevis' => 'vis.visitOpcode_GROUPBASEPTR                   (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_KERNARGBASEPTR',
                                              'val' => '135',
                                              'k' => 'BASIC',
                                              'mnemo' => 'kernargbaseptr',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_KERNARGBASEPTR                 (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_LANEID                         (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'laneid',
                                              'val' => '136',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_LANEID'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_MAXCUID',
                                              'k' => 'BASIC',
                                              'val' => '137',
                                              'mnemo' => 'maxcuid',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_MAXCUID                        (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_MAXWAVEID',
                                              'mnemo' => 'maxwaveid',
                                              'k' => 'BASIC',
                                              'val' => '138',
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_MAXWAVEID                      (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic'
                                            },
                                            {
                                              'pscode' => 'Seg',
                                              'opcodeparser' => 'parseMnemoSeg',
                                              'opcodevis' => 'vis.visitOpcode_NULLPTR                        (HSAIL_ASM::InstSeg(inst))',
                                              'name' => 'BRIG_OPCODE_NULLPTR',
                                              'val' => '139',
                                              'k' => 'SEG',
                                              'mnemo' => 'nullptr'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_WAVEID                         (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic',
                                              'k' => 'BASIC',
                                              'val' => '140',
                                              'mnemo' => 'waveid',
                                              'name' => 'BRIG_OPCODE_WAVEID'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'opcodevis' => 'vis.visitOpcode_GCNMADU                        (HSAIL_ASM::InstBasic(inst))',
                                              'hasType' => 'false',
                                              'val' => '(1u << 15) | 0',
                                              'mnemo' => 'gcn_madu',
                                              'pscode' => 'BasicNoType',
                                              'k' => 'BASIC_NO_TYPE',
                                              'name' => 'BRIG_OPCODE_GCNMADU'
                                            },
                                            {
                                              'pscode' => 'BasicNoType',
                                              'k' => 'BASIC_NO_TYPE',
                                              'name' => 'BRIG_OPCODE_GCNMADS',
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'opcodevis' => 'vis.visitOpcode_GCNMADS                        (HSAIL_ASM::InstBasic(inst))',
                                              'hasType' => 'false',
                                              'val' => '(1u << 15) | 1',
                                              'mnemo' => 'gcn_mads'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_GCNMAX3',
                                              'val' => '(1u << 15) | 2',
                                              'k' => 'BASIC',
                                              'mnemo' => 'gcn_max3',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_GCNMAX3                        (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_GCNMIN3',
                                              'k' => 'BASIC',
                                              'val' => '(1u << 15) | 3',
                                              'mnemo' => 'gcn_min3',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_GCNMIN3                        (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_GCNMED3                        (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'gcn_med3',
                                              'val' => '(1u << 15) | 4',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_GCNMED3'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_GCNFLDEXP                      (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic',
                                              'val' => '(1u << 15) | 5',
                                              'k' => 'BASIC',
                                              'mnemo' => 'gcn_fldexp',
                                              'name' => 'BRIG_OPCODE_GCNFLDEXP'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_GCNFREXP_EXP',
                                              'k' => 'BASIC',
                                              'val' => '(1u << 15) | 6',
                                              'mnemo' => 'gcn_frexp_exp',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_GCNFREXP_EXP                   (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_GCNFREXP_MANT',
                                              'val' => '(1u << 15) | 7',
                                              'k' => 'BASIC',
                                              'mnemo' => 'gcn_frexp_mant',
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_GCNFREXP_MANT                  (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_GCNTRIG_PREOP                  (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'name' => 'BRIG_OPCODE_GCNTRIG_PREOP',
                                              'k' => 'BASIC',
                                              'val' => '(1u << 15) | 8',
                                              'mnemo' => 'gcn_trig_preop'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_GCNBFM',
                                              'mnemo' => 'gcn_bfm',
                                              'k' => 'BASIC',
                                              'val' => '(1u << 15) | 9',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_GCNBFM                         (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_GCNLD',
                                              'vecOpndIndex' => '0',
                                              'k' => 'MEM',
                                              'pscode' => 'Mem',
                                              'has_memory_order' => 'true',
                                              'mnemo' => 'gcn_ld',
                                              'semsupport' => 'true',
                                              'val' => '(1u << 15) | 10',
                                              'opcodeparser' => 'parseMnemoMem',
                                              'opcodevis' => 'vis.visitOpcode_GCNLD                          (HSAIL_ASM::InstMem(inst))'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_GCNST',
                                              'k' => 'MEM',
                                              'vecOpndIndex' => '0',
                                              'pscode' => 'Mem',
                                              'has_memory_order' => 'true',
                                              'val' => '(1u << 15) | 11',
                                              'mnemo' => 'gcn_st',
                                              'semsupport' => 'true',
                                              'opcodeparser' => 'parseMnemoMem',
                                              'opcodevis' => 'vis.visitOpcode_GCNST                          (HSAIL_ASM::InstMem(inst))'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_GCNATOMIC',
                                              'mnemo' => 'gcn_atomic',
                                              'val' => '(1u << 15) | 12',
                                              'k' => 'ATOMIC',
                                              'pscode' => 'Atomic',
                                              'opcodeparser' => 'parseMnemoAtomic',
                                              'opcodevis' => 'vis.visitOpcode_GCNATOMIC                      (HSAIL_ASM::InstAtomic(inst))'
                                            },
                                            {
                                              'opcodevis' => 'vis.visitOpcode_GCNATOMICNORET                 (HSAIL_ASM::InstAtomic(inst))',
                                              'opcodeparser' => 'parseMnemoAtomic',
                                              'pscode' => 'Atomic',
                                              'mnemo' => 'gcn_atomicNoRet',
                                              'k' => 'ATOMIC',
                                              'val' => '(1u << 15) | 13',
                                              'name' => 'BRIG_OPCODE_GCNATOMICNORET'
                                            },
                                            {
                                              'mnemo' => 'gcn_sleep',
                                              'k' => 'BASIC',
                                              'val' => '(1u << 15) | 14',
                                              'name' => 'BRIG_OPCODE_GCNSLEEP',
                                              'opcodevis' => 'vis.visitOpcode_GCNSLEEP                       (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_GCNPRIORITY',
                                              'mnemo' => 'gcn_priority',
                                              'val' => '(1u << 15) | 15',
                                              'k' => 'BASIC',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_GCNPRIORITY                    (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'hasType' => 'false',
                                              'opcodevis' => 'vis.visitOpcode_GCNREGIONALLOC                 (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'mnemo' => 'gcn_region_alloc',
                                              'val' => '(1u << 15) | 16',
                                              'pscode' => 'BasicNoType',
                                              'name' => 'BRIG_OPCODE_GCNREGIONALLOC',
                                              'k' => 'BASIC_NO_TYPE'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_GCNMSAD                        (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_GCNMSAD',
                                              'k' => 'BASIC',
                                              'val' => '(1u << 15) | 17',
                                              'mnemo' => 'gcn_msad'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'val' => '(1u << 15) | 18',
                                              'mnemo' => 'gcn_qsad',
                                              'name' => 'BRIG_OPCODE_GCNQSAD',
                                              'opcodevis' => 'vis.visitOpcode_GCNQSAD                        (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_GCNMQSAD                       (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_GCNMQSAD',
                                              'mnemo' => 'gcn_mqsad',
                                              'val' => '(1u << 15) | 19',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'k' => 'BASIC_NO_TYPE',
                                              'name' => 'BRIG_OPCODE_GCNMQSAD4',
                                              'pscode' => 'BasicNoType',
                                              'mnemo' => 'gcn_mqsad4',
                                              'val' => '(1u << 15) | 20',
                                              'opcodevis' => 'vis.visitOpcode_GCNMQSAD4                      (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'hasType' => 'false'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_GCNSADW',
                                              'k' => 'BASIC',
                                              'val' => '(1u << 15) | 21',
                                              'mnemo' => 'gcn_sadw',
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_GCNSADW                        (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_GCNSADD',
                                              'val' => '(1u << 15) | 22',
                                              'k' => 'BASIC',
                                              'mnemo' => 'gcn_sadd',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_GCNSADD                        (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'pscode' => 'Addr',
                                              'opcodeparser' => 'parseMnemoAddr',
                                              'opcodevis' => 'vis.visitOpcode_GCNCONSUME                     (HSAIL_ASM::InstAddr(inst))',
                                              'name' => 'BRIG_OPCODE_GCNCONSUME',
                                              'mnemo' => 'gcn_atomic_consume',
                                              'k' => 'ADDR',
                                              'val' => '(1u << 15) | 23'
                                            },
                                            {
                                              'k' => 'ADDR',
                                              'val' => '(1u << 15) | 24',
                                              'mnemo' => 'gcn_atomic_append',
                                              'name' => 'BRIG_OPCODE_GCNAPPEND',
                                              'opcodeparser' => 'parseMnemoAddr',
                                              'opcodevis' => 'vis.visitOpcode_GCNAPPEND                      (HSAIL_ASM::InstAddr(inst))',
                                              'pscode' => 'Addr'
                                            },
                                            {
                                              'mnemo' => 'gcn_b4xchg',
                                              'val' => '(1u << 15) | 25',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_GCNB4XCHG',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_GCNB4XCHG                      (HSAIL_ASM::InstBasic(inst))',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_GCNB32XCHG',
                                              'mnemo' => 'gcn_b32xchg',
                                              'val' => '(1u << 15) | 26',
                                              'k' => 'BASIC',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'opcodevis' => 'vis.visitOpcode_GCNB32XCHG                     (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_GCNMAX',
                                              'k' => 'BASIC',
                                              'val' => '(1u << 15) | 27',
                                              'mnemo' => 'gcn_max',
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_GCNMAX                         (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_GCNMIN',
                                              'mnemo' => 'gcn_min',
                                              'k' => 'BASIC',
                                              'val' => '(1u << 15) | 28',
                                              'pscode' => 'Basic',
                                              'opcodevis' => 'vis.visitOpcode_GCNMIN                         (HSAIL_ASM::InstBasic(inst))',
                                              'opcodeparser' => 'parseMnemoBasic'
                                            }
                                          ],
                             'opcodeparser#calcState' => 'done',
                             'pscode' => sub { "DUMMY" },
                             'vecOpndIndex' => sub { "DUMMY" },
                             'vecOpndIndex_switch' => 'true',
                             'vecOpndIndex#deps' => [],
                             'vecOpndIndex#calcState' => 'done',
                             'numdst_proto' => 'int instNumDstOperands(Brig::BrigOpcode16_t arg)'
                           },
           'BrigMemoryModifierMask' => {
                                         'name' => 'BrigMemoryModifierMask',
                                         'entries' => [
                                                        {
                                                          'name' => 'BRIG_MEMORY_CONST',
                                                          'val' => '1'
                                                        }
                                                      ]
                                       },
           'BrigSegment' => {
                              'mnemo' => sub { "DUMMY" },
                              'name' => 'BrigSegment',
                              'mnemo#calcState' => 'done',
                              'mnemo#deps' => [],
                              'entries' => [
                                             {
                                               'mnemo' => '',
                                               'val' => '0',
                                               'name' => 'BRIG_SEGMENT_NONE'
                                             },
                                             {
                                               'val' => '1',
                                               'mnemo' => '',
                                               'name' => 'BRIG_SEGMENT_FLAT'
                                             },
                                             {
                                               'name' => 'BRIG_SEGMENT_GLOBAL',
                                               'mnemo' => 'global',
                                               'val' => '2'
                                             },
                                             {
                                               'mnemo' => 'readonly',
                                               'val' => '3',
                                               'name' => 'BRIG_SEGMENT_READONLY'
                                             },
                                             {
                                               'name' => 'BRIG_SEGMENT_KERNARG',
                                               'mnemo' => 'kernarg',
                                               'val' => '4'
                                             },
                                             {
                                               'val' => '5',
                                               'mnemo' => 'group',
                                               'name' => 'BRIG_SEGMENT_GROUP'
                                             },
                                             {
                                               'name' => 'BRIG_SEGMENT_PRIVATE',
                                               'val' => '6',
                                               'mnemo' => 'private'
                                             },
                                             {
                                               'val' => '7',
                                               'mnemo' => 'spill',
                                               'name' => 'BRIG_SEGMENT_SPILL'
                                             },
                                             {
                                               'val' => '8',
                                               'mnemo' => 'arg',
                                               'name' => 'BRIG_SEGMENT_ARG'
                                             },
                                             {
                                               'name' => 'BRIG_SEGMENT_EXTSPACE0',
                                               'val' => '9',
                                               'mnemo' => 'region'
                                             }
                                           ],
                              'mnemo_token' => '_EMSegment',
                              'mnemo_context' => 'EInstModifierContext'
                            },
           'BrigImageChannelType' => {
                                       'mnemo#calcState' => 'done',
                                       'name' => 'BrigImageChannelType',
                                       'mnemo' => sub { "DUMMY" },
                                       'entries' => [
                                                      {
                                                        'name' => 'BRIG_CHANNEL_TYPE_SNORM_INT8',
                                                        'val' => '0',
                                                        'mnemo' => 'snorm_int8'
                                                      },
                                                      {
                                                        'mnemo' => 'snorm_int16',
                                                        'val' => '1',
                                                        'name' => 'BRIG_CHANNEL_TYPE_SNORM_INT16'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNORM_INT8',
                                                        'val' => '2',
                                                        'mnemo' => 'unorm_int8'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNORM_INT16',
                                                        'val' => '3',
                                                        'mnemo' => 'unorm_int16'
                                                      },
                                                      {
                                                        'mnemo' => 'unorm_int24',
                                                        'val' => '4',
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNORM_INT24'
                                                      },
                                                      {
                                                        'val' => '5',
                                                        'mnemo' => 'unorm_short_555',
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNORM_SHORT_555'
                                                      },
                                                      {
                                                        'mnemo' => 'unorm_short_565',
                                                        'val' => '6',
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNORM_SHORT_565'
                                                      },
                                                      {
                                                        'mnemo' => 'unorm_int_101010',
                                                        'val' => '7',
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNORM_INT_101010'
                                                      },
                                                      {
                                                        'val' => '8',
                                                        'mnemo' => 'signed_int8',
                                                        'name' => 'BRIG_CHANNEL_TYPE_SIGNED_INT8'
                                                      },
                                                      {
                                                        'mnemo' => 'signed_int16',
                                                        'val' => '9',
                                                        'name' => 'BRIG_CHANNEL_TYPE_SIGNED_INT16'
                                                      },
                                                      {
                                                        'mnemo' => 'signed_int32',
                                                        'val' => '10',
                                                        'name' => 'BRIG_CHANNEL_TYPE_SIGNED_INT32'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNSIGNED_INT8',
                                                        'mnemo' => 'unsigned_int8',
                                                        'val' => '11'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNSIGNED_INT16',
                                                        'mnemo' => 'unsigned_int16',
                                                        'val' => '12'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNSIGNED_INT32',
                                                        'val' => '13',
                                                        'mnemo' => 'unsigned_int32'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CHANNEL_TYPE_HALF_FLOAT',
                                                        'val' => '14',
                                                        'mnemo' => 'half_float'
                                                      },
                                                      {
                                                        'val' => '15',
                                                        'mnemo' => 'float',
                                                        'name' => 'BRIG_CHANNEL_TYPE_FLOAT'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNKNOWN',
                                                        'mnemo' => ''
                                                      }
                                                    ],
                                       'mnemo#deps' => [],
                                       'mnemo_token' => 'EImageFormat'
                                     },
           'BrigVariableModifierMask' => {
                                           'name' => 'BrigVariableModifierMask',
                                           'nodump' => 'true',
                                           'entries' => [
                                                          {
                                                            'name' => 'BRIG_SYMBOL_DEFINITION',
                                                            'val' => '1'
                                                          },
                                                          {
                                                            'val' => '2',
                                                            'name' => 'BRIG_SYMBOL_CONST'
                                                          },
                                                          {
                                                            'val' => '4',
                                                            'name' => 'BRIG_SYMBOL_ARRAY'
                                                          },
                                                          {
                                                            'name' => 'BRIG_SYMBOL_FLEX_ARRAY',
                                                            'val' => '8'
                                                          }
                                                        ]
                                         }
         };
