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
$structs = {
             'BrigInstBasic' => {
                                  'fields' => [
                                                {
                                                  'type' => 'uint16_t',
                                                  'wname' => 'byteCount',
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'name' => 'byteCount',
                                                  'acc' => 'valRef'
                                                },
                                                {
                                                  'type' => 'BrigKinds16_t',
                                                  'wname' => 'kind',
                                                  'enum' => 'BrigKinds',
                                                  'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                  'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                  'name' => 'kind'
                                                },
                                                {
                                                  'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                  'enum' => 'BrigOpcode',
                                                  'name' => 'opcode',
                                                  'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                  'wname' => 'opcode',
                                                  'type' => 'BrigOpcode16_t'
                                                },
                                                {
                                                  'wname' => 'type',
                                                  'type' => 'BrigType16_t',
                                                  'acc' => 'valRef',
                                                  'name' => 'type',
                                                  'wtype' => 'ValRef<uint16_t>'
                                                },
                                                {
                                                  'type' => 'BrigDataOffsetOperandList32_t',
                                                  'wname' => 'operands',
                                                  'defValue' => '0',
                                                  'wtype' => 'ListRef<Operand>',
                                                  'name' => 'operands',
                                                  'acc' => 'listRef<Operand>'
                                                }
                                              ],
                                  'enum' => 'BRIG_KIND_INST_BASIC',
                                  'name' => 'BrigInstBasic',
                                  'wname' => 'InstBasic',
                                  'parent' => 'BrigInst',
                                  'align' => undef
                                },
             'BrigInstSourceType' => {
                                       'name' => 'BrigInstSourceType',
                                       'fields' => [
                                                     {
                                                       'type' => 'uint16_t',
                                                       'wname' => 'byteCount',
                                                       'acc' => 'valRef',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'name' => 'byteCount'
                                                     },
                                                     {
                                                       'wname' => 'kind',
                                                       'type' => 'BrigKinds16_t',
                                                       'enum' => 'BrigKinds',
                                                       'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                       'name' => 'kind',
                                                       'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>'
                                                     },
                                                     {
                                                       'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                       'enum' => 'BrigOpcode',
                                                       'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                       'name' => 'opcode',
                                                       'type' => 'BrigOpcode16_t',
                                                       'wname' => 'opcode'
                                                     },
                                                     {
                                                       'wname' => 'type',
                                                       'type' => 'BrigType16_t',
                                                       'name' => 'type',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'acc' => 'valRef'
                                                     },
                                                     {
                                                       'acc' => 'listRef<Operand>',
                                                       'wtype' => 'ListRef<Operand>',
                                                       'name' => 'operands',
                                                       'type' => 'BrigDataOffsetOperandList32_t',
                                                       'wname' => 'operands',
                                                       'defValue' => '0'
                                                     },
                                                     {
                                                       'wname' => 'sourceType',
                                                       'type' => 'BrigType16_t',
                                                       'acc' => 'valRef',
                                                       'name' => 'sourceType',
                                                       'wtype' => 'ValRef<uint16_t>'
                                                     },
                                                     {
                                                       'acc' => 'valRef',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'skip' => 1,
                                                       'name' => 'reserved',
                                                       'type' => 'uint16_t',
                                                       'defValue' => '0',
                                                       'wname' => 'reserved'
                                                     }
                                                   ],
                                       'enum' => 'BRIG_KIND_INST_SOURCE_TYPE',
                                       'align' => undef,
                                       'wname' => 'InstSourceType',
                                       'parent' => 'BrigInst'
                                     },
             'BrigInstMemFence' => {
                                     'align' => undef,
                                     'parent' => 'BrigInst',
                                     'wname' => 'InstMemFence',
                                     'name' => 'BrigInstMemFence',
                                     'enum' => 'BRIG_KIND_INST_MEM_FENCE',
                                     'fields' => [
                                                   {
                                                     'wtype' => 'ValRef<uint16_t>',
                                                     'name' => 'byteCount',
                                                     'acc' => 'valRef',
                                                     'type' => 'uint16_t',
                                                     'wname' => 'byteCount'
                                                   },
                                                   {
                                                     'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                     'enum' => 'BrigKinds',
                                                     'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                     'name' => 'kind',
                                                     'type' => 'BrigKinds16_t',
                                                     'wname' => 'kind'
                                                   },
                                                   {
                                                     'wname' => 'opcode',
                                                     'type' => 'BrigOpcode16_t',
                                                     'name' => 'opcode',
                                                     'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                     'enum' => 'BrigOpcode',
                                                     'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>'
                                                   },
                                                   {
                                                     'wname' => 'type',
                                                     'type' => 'BrigType16_t',
                                                     'acc' => 'valRef',
                                                     'name' => 'type',
                                                     'wtype' => 'ValRef<uint16_t>'
                                                   },
                                                   {
                                                     'type' => 'BrigDataOffsetOperandList32_t',
                                                     'wname' => 'operands',
                                                     'defValue' => '0',
                                                     'wtype' => 'ListRef<Operand>',
                                                     'name' => 'operands',
                                                     'acc' => 'listRef<Operand>'
                                                   },
                                                   {
                                                     'enum' => 'BrigMemoryOrder',
                                                     'acc' => 'enumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                     'name' => 'memoryOrder',
                                                     'wtype' => 'EnumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                     'wname' => 'memoryOrder',
                                                     'defValue' => 'Brig::BRIG_MEMORY_ORDER_RELAXED',
                                                     'type' => 'BrigMemoryOrder8_t'
                                                   },
                                                   {
                                                     'enum' => 'BrigMemoryScope',
                                                     'acc' => 'enumValRef<Brig::BrigMemoryScope,uint8_t>',
                                                     'wtype' => 'EnumValRef<Brig::BrigMemoryScope,uint8_t>',
                                                     'name' => 'globalSegmentMemoryScope',
                                                     'type' => 'BrigMemoryScope8_t',
                                                     'defValue' => 'Brig::BRIG_MEMORY_SCOPE_SYSTEM',
                                                     'wname' => 'globalSegmentMemoryScope'
                                                   },
                                                   {
                                                     'type' => 'BrigMemoryScope8_t',
                                                     'wname' => 'groupSegmentMemoryScope',
                                                     'defValue' => 'Brig::BRIG_MEMORY_SCOPE_SYSTEM',
                                                     'wtype' => 'EnumValRef<Brig::BrigMemoryScope,uint8_t>',
                                                     'name' => 'groupSegmentMemoryScope',
                                                     'enum' => 'BrigMemoryScope',
                                                     'acc' => 'enumValRef<Brig::BrigMemoryScope,uint8_t>'
                                                   },
                                                   {
                                                     'wtype' => 'EnumValRef<Brig::BrigMemoryScope,uint8_t>',
                                                     'name' => 'imageSegmentMemoryScope',
                                                     'acc' => 'enumValRef<Brig::BrigMemoryScope,uint8_t>',
                                                     'enum' => 'BrigMemoryScope',
                                                     'type' => 'BrigMemoryScope8_t',
                                                     'wname' => 'imageSegmentMemoryScope',
                                                     'defValue' => 'Brig::BRIG_MEMORY_SCOPE_SYSTEM'
                                                   }
                                                 ]
                                   },
             'BrigInstMem' => {
                                'align' => undef,
                                'parent' => 'BrigInst',
                                'wname' => 'InstMem',
                                'name' => 'BrigInstMem',
                                'fields' => [
                                              {
                                                'wname' => 'byteCount',
                                                'type' => 'uint16_t',
                                                'acc' => 'valRef',
                                                'name' => 'byteCount',
                                                'wtype' => 'ValRef<uint16_t>'
                                              },
                                              {
                                                'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                'name' => 'kind',
                                                'enum' => 'BrigKinds',
                                                'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                'type' => 'BrigKinds16_t',
                                                'wname' => 'kind'
                                              },
                                              {
                                                'wname' => 'opcode',
                                                'type' => 'BrigOpcode16_t',
                                                'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                'enum' => 'BrigOpcode',
                                                'name' => 'opcode',
                                                'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>'
                                              },
                                              {
                                                'wname' => 'type',
                                                'type' => 'BrigType16_t',
                                                'acc' => 'valRef',
                                                'name' => 'type',
                                                'wtype' => 'ValRef<uint16_t>'
                                              },
                                              {
                                                'type' => 'BrigDataOffsetOperandList32_t',
                                                'wname' => 'operands',
                                                'defValue' => '0',
                                                'acc' => 'listRef<Operand>',
                                                'wtype' => 'ListRef<Operand>',
                                                'name' => 'operands'
                                              },
                                              {
                                                'defValue' => 'Brig::BRIG_SEGMENT_NONE',
                                                'wname' => 'segment',
                                                'type' => 'BrigSegment8_t',
                                                'name' => 'segment',
                                                'wtype' => 'EnumValRef<Brig::BrigSegment,uint8_t>',
                                                'enum' => 'BrigSegment',
                                                'acc' => 'enumValRef<Brig::BrigSegment,uint8_t>'
                                              },
                                              {
                                                'enum' => 'BrigAlignment',
                                                'acc' => 'enumValRef<Brig::BrigAlignment,uint8_t>',
                                                'wtype' => 'EnumValRef<Brig::BrigAlignment,uint8_t>',
                                                'name' => 'align',
                                                'type' => 'BrigAlignment8_t',
                                                'defValue' => 'Brig::BRIG_ALIGNMENT_NONE',
                                                'wname' => 'align'
                                              },
                                              {
                                                'acc' => 'valRef',
                                                'name' => 'equivClass',
                                                'wtype' => 'ValRef<uint8_t>',
                                                'wname' => 'equivClass',
                                                'type' => 'uint8_t'
                                              },
                                              {
                                                'wtype' => 'EnumValRef<Brig::BrigWidth,uint8_t>',
                                                'name' => 'width',
                                                'acc' => 'enumValRef<Brig::BrigWidth,uint8_t>',
                                                'enum' => 'BrigWidth',
                                                'type' => 'BrigWidth8_t',
                                                'wname' => 'width'
                                              },
                                              {
                                                'wname' => 'modifier',
                                                'type' => 'BrigMemoryModifier',
                                                'name' => 'modifier',
                                                'wtype' => 'MemoryModifier',
                                                'acc' => 'subItem<MemoryModifier>'
                                              },
                                              {
                                                'acc' => 'valRef',
                                                'name' => 'reserved',
                                                'wtype' => 'ValRef<uint8_t>',
                                                'wname' => 'reserved',
                                                'defValue' => '0',
                                                'type' => 'uint8_t',
                                                'skip' => 1,
                                                'size' => 3
                                              }
                                            ],
                                'enum' => 'BRIG_KIND_INST_MEM'
                              },
             'BrigInstQueryImage' => {
                                       'name' => 'BrigInstQueryImage',
                                       'fields' => [
                                                     {
                                                       'acc' => 'valRef',
                                                       'name' => 'byteCount',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'wname' => 'byteCount',
                                                       'type' => 'uint16_t'
                                                     },
                                                     {
                                                       'type' => 'BrigKinds16_t',
                                                       'wname' => 'kind',
                                                       'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                       'name' => 'kind',
                                                       'enum' => 'BrigKinds',
                                                       'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>'
                                                     },
                                                     {
                                                       'type' => 'BrigOpcode16_t',
                                                       'wname' => 'opcode',
                                                       'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                       'enum' => 'BrigOpcode',
                                                       'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                       'name' => 'opcode'
                                                     },
                                                     {
                                                       'acc' => 'valRef',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'name' => 'type',
                                                       'type' => 'BrigType16_t',
                                                       'wname' => 'type'
                                                     },
                                                     {
                                                       'name' => 'operands',
                                                       'wtype' => 'ListRef<Operand>',
                                                       'acc' => 'listRef<Operand>',
                                                       'wname' => 'operands',
                                                       'defValue' => '0',
                                                       'type' => 'BrigDataOffsetOperandList32_t'
                                                     },
                                                     {
                                                       'acc' => 'valRef',
                                                       'name' => 'imageType',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'wname' => 'imageType',
                                                       'type' => 'BrigType16_t'
                                                     },
                                                     {
                                                       'acc' => 'enumValRef<Brig::BrigImageGeometry,uint8_t>',
                                                       'enum' => 'BrigImageGeometry',
                                                       'name' => 'geometry',
                                                       'wtype' => 'EnumValRef<Brig::BrigImageGeometry,uint8_t>',
                                                       'defValue' => 'Brig::BRIG_GEOMETRY_UNKNOWN',
                                                       'wname' => 'geometry',
                                                       'type' => 'BrigImageGeometry8_t'
                                                     },
                                                     {
                                                       'type' => 'BrigImageQuery8_t',
                                                       'wname' => 'imageQuery',
                                                       'acc' => 'enumValRef<Brig::BrigImageQuery,uint8_t>',
                                                       'enum' => 'BrigImageQuery',
                                                       'wtype' => 'EnumValRef<Brig::BrigImageQuery,uint8_t>',
                                                       'name' => 'imageQuery'
                                                     }
                                                   ],
                                       'enum' => 'BRIG_KIND_INST_QUERY_IMAGE',
                                       'align' => undef,
                                       'wname' => 'InstQueryImage',
                                       'parent' => 'BrigInst'
                                     },
             'BrigDirectiveControl' => {
                                         'enum' => 'BRIG_KIND_DIRECTIVE_CONTROL',
                                         'fields' => [
                                                       {
                                                         'name' => 'byteCount',
                                                         'wtype' => 'ValRef<uint16_t>',
                                                         'acc' => 'valRef',
                                                         'wname' => 'byteCount',
                                                         'type' => 'uint16_t'
                                                       },
                                                       {
                                                         'type' => 'BrigKinds16_t',
                                                         'wname' => 'kind',
                                                         'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                         'name' => 'kind',
                                                         'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                         'enum' => 'BrigKinds'
                                                       },
                                                       {
                                                         'wname' => 'control',
                                                         'type' => 'BrigControlDirective16_t',
                                                         'name' => 'control',
                                                         'wtype' => 'EnumValRef<Brig::BrigControlDirective,uint16_t>',
                                                         'acc' => 'enumValRef<Brig::BrigControlDirective,uint16_t>',
                                                         'enum' => 'BrigControlDirective'
                                                       },
                                                       {
                                                         'name' => 'reserved',
                                                         'wtype' => 'ValRef<uint16_t>',
                                                         'skip' => 1,
                                                         'acc' => 'valRef',
                                                         'defValue' => '0',
                                                         'wname' => 'reserved',
                                                         'type' => 'uint16_t'
                                                       },
                                                       {
                                                         'defValue' => '0',
                                                         'wname' => 'operands',
                                                         'type' => 'BrigDataOffsetOperandList32_t',
                                                         'acc' => 'listRef<Operand>',
                                                         'name' => 'operands',
                                                         'wtype' => 'ListRef<Operand>'
                                                       }
                                                     ],
                                         'name' => 'BrigDirectiveControl',
                                         'parent' => 'BrigDirective',
                                         'wname' => 'DirectiveControl',
                                         'align' => undef
                                       },
             'BrigInstLane' => {
                                 'fields' => [
                                               {
                                                 'name' => 'byteCount',
                                                 'wtype' => 'ValRef<uint16_t>',
                                                 'acc' => 'valRef',
                                                 'wname' => 'byteCount',
                                                 'type' => 'uint16_t'
                                               },
                                               {
                                                 'wname' => 'kind',
                                                 'type' => 'BrigKinds16_t',
                                                 'name' => 'kind',
                                                 'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                 'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                 'enum' => 'BrigKinds'
                                               },
                                               {
                                                 'wname' => 'opcode',
                                                 'type' => 'BrigOpcode16_t',
                                                 'name' => 'opcode',
                                                 'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                 'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                 'enum' => 'BrigOpcode'
                                               },
                                               {
                                                 'wname' => 'type',
                                                 'type' => 'BrigType16_t',
                                                 'name' => 'type',
                                                 'wtype' => 'ValRef<uint16_t>',
                                                 'acc' => 'valRef'
                                               },
                                               {
                                                 'type' => 'BrigDataOffsetOperandList32_t',
                                                 'wname' => 'operands',
                                                 'defValue' => '0',
                                                 'wtype' => 'ListRef<Operand>',
                                                 'name' => 'operands',
                                                 'acc' => 'listRef<Operand>'
                                               },
                                               {
                                                 'name' => 'sourceType',
                                                 'wtype' => 'ValRef<uint16_t>',
                                                 'acc' => 'valRef',
                                                 'wname' => 'sourceType',
                                                 'type' => 'BrigType16_t'
                                               },
                                               {
                                                 'type' => 'BrigWidth8_t',
                                                 'wname' => 'width',
                                                 'acc' => 'enumValRef<Brig::BrigWidth,uint8_t>',
                                                 'enum' => 'BrigWidth',
                                                 'wtype' => 'EnumValRef<Brig::BrigWidth,uint8_t>',
                                                 'name' => 'width'
                                               },
                                               {
                                                 'type' => 'uint8_t',
                                                 'wname' => 'reserved',
                                                 'defValue' => '0',
                                                 'wtype' => 'ValRef<uint8_t>',
                                                 'skip' => 1,
                                                 'name' => 'reserved',
                                                 'acc' => 'valRef'
                                               }
                                             ],
                                 'enum' => 'BRIG_KIND_INST_LANE',
                                 'name' => 'BrigInstLane',
                                 'wname' => 'InstLane',
                                 'parent' => 'BrigInst',
                                 'align' => undef
                               },
             'BrigDirective' => {
                                  'children' => [
                                                  'BrigDirectivePragma',
                                                  'BrigDirectiveKernel',
                                                  'BrigDirectiveLoc',
                                                  'BrigDirectiveExecutable',
                                                  'BrigDirectiveArgBlockEnd',
                                                  'BrigDirectiveArgBlockStart',
                                                  'BrigDirectiveComment',
                                                  'BrigDirectiveExtension',
                                                  'BrigDirectiveVariable',
                                                  'BrigDirectiveIndirectFunction',
                                                  'BrigDirectiveFunction',
                                                  'BrigDirectiveFbarrier',
                                                  'BrigDirectiveSignature',
                                                  'BrigDirectiveControl',
                                                  'BrigDirectiveNone',
                                                  'BrigDirectiveVersion',
                                                  'BrigDirectiveLabel'
                                                ],
                                  'wname' => 'Directive',
                                  'name' => 'BrigDirective',
                                  'align' => undef,
                                  'parent' => 'BrigCode',
                                  'generic' => 'true',
                                  'enum' => 'BRIG_KIND_DIRECTIVE',
                                  'fields' => [
                                                {
                                                  'type' => 'uint16_t',
                                                  'wname' => 'byteCount',
                                                  'acc' => 'valRef',
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'name' => 'byteCount'
                                                },
                                                {
                                                  'name' => 'kind',
                                                  'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                  'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                  'enum' => 'BrigKinds',
                                                  'wname' => 'kind',
                                                  'type' => 'BrigKinds16_t'
                                                }
                                              ]
                                },
             'BrigDirectiveFbarrier' => {
                                          'align' => undef,
                                          'wname' => 'DirectiveFbarrier',
                                          'parent' => 'BrigDirective',
                                          'name' => 'BrigDirectiveFbarrier',
                                          'enum' => 'BRIG_KIND_DIRECTIVE_FBARRIER',
                                          'fields' => [
                                                        {
                                                          'wname' => 'byteCount',
                                                          'type' => 'uint16_t',
                                                          'name' => 'byteCount',
                                                          'wtype' => 'ValRef<uint16_t>',
                                                          'acc' => 'valRef'
                                                        },
                                                        {
                                                          'type' => 'BrigKinds16_t',
                                                          'wname' => 'kind',
                                                          'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                          'enum' => 'BrigKinds',
                                                          'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                          'name' => 'kind'
                                                        },
                                                        {
                                                          'wtype' => 'StrRef',
                                                          'name' => 'name',
                                                          'acc' => 'strRef',
                                                          'type' => 'BrigDataOffsetString32_t',
                                                          'wname' => 'name',
                                                          'defValue' => '0'
                                                        },
                                                        {
                                                          'type' => 'BrigExecutableModifier',
                                                          'wname' => 'modifier',
                                                          'wtype' => 'ExecutableModifier',
                                                          'name' => 'modifier',
                                                          'acc' => 'subItem<ExecutableModifier>'
                                                        },
                                                        {
                                                          'type' => 'BrigLinkage8_t',
                                                          'defValue' => 'Brig::BRIG_LINKAGE_NONE',
                                                          'wname' => 'linkage',
                                                          'acc' => 'enumValRef<Brig::BrigLinkage,uint8_t>',
                                                          'enum' => 'BrigLinkage',
                                                          'wtype' => 'EnumValRef<Brig::BrigLinkage,uint8_t>',
                                                          'name' => 'linkage'
                                                        },
                                                        {
                                                          'defValue' => '0',
                                                          'wname' => 'reserved',
                                                          'type' => 'uint16_t',
                                                          'name' => 'reserved',
                                                          'skip' => 1,
                                                          'wtype' => 'ValRef<uint16_t>',
                                                          'acc' => 'valRef'
                                                        }
                                                      ]
                                        },
             'BrigOperandCodeList' => {
                                        'align' => undef,
                                        'wname' => 'OperandCodeList',
                                        'parent' => 'BrigOperand',
                                        'name' => 'BrigOperandCodeList',
                                        'fields' => [
                                                      {
                                                        'wname' => 'byteCount',
                                                        'type' => 'uint16_t',
                                                        'name' => 'byteCount',
                                                        'wtype' => 'ValRef<uint16_t>',
                                                        'acc' => 'valRef'
                                                      },
                                                      {
                                                        'enum' => 'BrigKinds',
                                                        'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                        'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                        'name' => 'kind',
                                                        'type' => 'BrigKinds16_t',
                                                        'wname' => 'kind'
                                                      },
                                                      {
                                                        'defValue' => '0',
                                                        'wname' => 'elements',
                                                        'type' => 'BrigDataOffsetCodeList32_t',
                                                        'hcode' => [
                                                                     'unsigned elementCount();',
                                                                     'Code elements(int index);'
                                                                   ],
                                                        'name' => 'elements',
                                                        'wtype' => 'ListRef<Code>',
                                                        'acc' => 'listRef<Code>',
                                                        'implcode' => [
                                                                        'inline unsigned KLASS::elementCount() { return elements().size(); }',
                                                                        'inline Code KLASS::elements(int index) { return elements()[index]; }'
                                                                      ]
                                                      }
                                                    ],
                                        'enum' => 'BRIG_KIND_OPERAND_CODE_LIST'
                                      },
             'BrigDirectiveVariable' => {
                                          'fields' => [
                                                        {
                                                          'wname' => 'byteCount',
                                                          'type' => 'uint16_t',
                                                          'name' => 'byteCount',
                                                          'wtype' => 'ValRef<uint16_t>',
                                                          'acc' => 'valRef'
                                                        },
                                                        {
                                                          'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                          'enum' => 'BrigKinds',
                                                          'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                          'name' => 'kind',
                                                          'type' => 'BrigKinds16_t',
                                                          'wname' => 'kind'
                                                        },
                                                        {
                                                          'wname' => 'name',
                                                          'defValue' => '0',
                                                          'type' => 'BrigDataOffsetString32_t',
                                                          'name' => 'name',
                                                          'wtype' => 'StrRef',
                                                          'acc' => 'strRef'
                                                        },
                                                        {
                                                          'name' => 'init',
                                                          'wtype' => 'ItemRef<Operand>',
                                                          'acc' => 'itemRef<Operand>',
                                                          'wname' => 'init',
                                                          'defValue' => '0',
                                                          'type' => 'BrigOperandOffset32_t'
                                                        },
                                                        {
                                                          'acc' => 'valRef',
                                                          'name' => 'type',
                                                          'wtype' => 'ValRef<uint16_t>',
                                                          'wname' => 'type',
                                                          'type' => 'BrigType16_t'
                                                        },
                                                        {
                                                          'type' => 'BrigSegment8_t',
                                                          'defValue' => 'Brig::BRIG_SEGMENT_NONE',
                                                          'wname' => 'segment',
                                                          'acc' => 'enumValRef<Brig::BrigSegment,uint8_t>',
                                                          'enum' => 'BrigSegment',
                                                          'wtype' => 'EnumValRef<Brig::BrigSegment,uint8_t>',
                                                          'name' => 'segment'
                                                        },
                                                        {
                                                          'type' => 'BrigAlignment8_t',
                                                          'wname' => 'align',
                                                          'defValue' => 'Brig::BRIG_ALIGNMENT_NONE',
                                                          'acc' => 'enumValRef<Brig::BrigAlignment,uint8_t>',
                                                          'enum' => 'BrigAlignment',
                                                          'wtype' => 'EnumValRef<Brig::BrigAlignment,uint8_t>',
                                                          'name' => 'align'
                                                        },
                                                        {
                                                          'wname' => 'dim',
                                                          'type' => 'BrigUInt64',
                                                          'acc' => 'subItem<UInt64>',
                                                          'name' => 'dim',
                                                          'wtype' => 'UInt64'
                                                        },
                                                        {
                                                          'wname' => 'modifier',
                                                          'type' => 'BrigVariableModifier',
                                                          'name' => 'modifier',
                                                          'wtype' => 'VariableModifier',
                                                          'acc' => 'subItem<VariableModifier>'
                                                        },
                                                        {
                                                          'wtype' => 'EnumValRef<Brig::BrigLinkage,uint8_t>',
                                                          'name' => 'linkage',
                                                          'enum' => 'BrigLinkage',
                                                          'acc' => 'enumValRef<Brig::BrigLinkage,uint8_t>',
                                                          'type' => 'BrigLinkage8_t',
                                                          'wname' => 'linkage',
                                                          'defValue' => 'Brig::BRIG_LINKAGE_NONE'
                                                        },
                                                        {
                                                          'defValue' => 'Brig::BRIG_ALLOCATION_NONE',
                                                          'wname' => 'allocation',
                                                          'type' => 'BrigAllocation8_t',
                                                          'name' => 'allocation',
                                                          'wtype' => 'EnumValRef<Brig::BrigAllocation,uint8_t>',
                                                          'acc' => 'enumValRef<Brig::BrigAllocation,uint8_t>',
                                                          'enum' => 'BrigAllocation'
                                                        },
                                                        {
                                                          'name' => 'reserved',
                                                          'wtype' => 'ValRef<uint8_t>',
                                                          'skip' => 1,
                                                          'acc' => 'valRef',
                                                          'defValue' => '0',
                                                          'wname' => 'reserved',
                                                          'type' => 'uint8_t'
                                                        }
                                                      ],
                                          'enum' => 'BRIG_KIND_DIRECTIVE_VARIABLE',
                                          'name' => 'BrigDirectiveVariable',
                                          'parent' => 'BrigDirective',
                                          'wname' => 'DirectiveVariable',
                                          'align' => undef
                                        },
             'BrigInstSeg' => {
                                'align' => undef,
                                'wname' => 'InstSeg',
                                'parent' => 'BrigInst',
                                'name' => 'BrigInstSeg',
                                'enum' => 'BRIG_KIND_INST_SEG',
                                'fields' => [
                                              {
                                                'wtype' => 'ValRef<uint16_t>',
                                                'name' => 'byteCount',
                                                'acc' => 'valRef',
                                                'type' => 'uint16_t',
                                                'wname' => 'byteCount'
                                              },
                                              {
                                                'type' => 'BrigKinds16_t',
                                                'wname' => 'kind',
                                                'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                'enum' => 'BrigKinds',
                                                'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                'name' => 'kind'
                                              },
                                              {
                                                'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                'enum' => 'BrigOpcode',
                                                'name' => 'opcode',
                                                'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                'wname' => 'opcode',
                                                'type' => 'BrigOpcode16_t'
                                              },
                                              {
                                                'type' => 'BrigType16_t',
                                                'wname' => 'type',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'name' => 'type',
                                                'acc' => 'valRef'
                                              },
                                              {
                                                'name' => 'operands',
                                                'wtype' => 'ListRef<Operand>',
                                                'acc' => 'listRef<Operand>',
                                                'defValue' => '0',
                                                'wname' => 'operands',
                                                'type' => 'BrigDataOffsetOperandList32_t'
                                              },
                                              {
                                                'enum' => 'BrigSegment',
                                                'acc' => 'enumValRef<Brig::BrigSegment,uint8_t>',
                                                'wtype' => 'EnumValRef<Brig::BrigSegment,uint8_t>',
                                                'name' => 'segment',
                                                'type' => 'BrigSegment8_t',
                                                'defValue' => 'Brig::BRIG_SEGMENT_NONE',
                                                'wname' => 'segment'
                                              },
                                              {
                                                'skip' => 1,
                                                'size' => 3,
                                                'acc' => 'valRef',
                                                'wtype' => 'ValRef<uint8_t>',
                                                'name' => 'reserved',
                                                'type' => 'uint8_t',
                                                'defValue' => '0',
                                                'wname' => 'reserved'
                                              }
                                            ]
                              },
             'BrigDirectiveExtension' => {
                                           'fields' => [
                                                         {
                                                           'acc' => 'valRef',
                                                           'name' => 'byteCount',
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'wname' => 'byteCount',
                                                           'type' => 'uint16_t'
                                                         },
                                                         {
                                                           'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                           'enum' => 'BrigKinds',
                                                           'name' => 'kind',
                                                           'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                           'wname' => 'kind',
                                                           'type' => 'BrigKinds16_t'
                                                         },
                                                         {
                                                           'acc' => 'strRef',
                                                           'name' => 'name',
                                                           'wtype' => 'StrRef',
                                                           'defValue' => '0',
                                                           'wname' => 'name',
                                                           'type' => 'BrigDataOffsetString32_t'
                                                         }
                                                       ],
                                           'enum' => 'BRIG_KIND_DIRECTIVE_EXTENSION',
                                           'name' => 'BrigDirectiveExtension',
                                           'wname' => 'DirectiveExtension',
                                           'parent' => 'BrigDirective',
                                           'align' => undef
                                         },
             'BrigDirectiveComment' => {
                                         'parent' => 'BrigDirective',
                                         'wname' => 'DirectiveComment',
                                         'align' => undef,
                                         'fields' => [
                                                       {
                                                         'acc' => 'valRef',
                                                         'name' => 'byteCount',
                                                         'wtype' => 'ValRef<uint16_t>',
                                                         'wname' => 'byteCount',
                                                         'type' => 'uint16_t'
                                                       },
                                                       {
                                                         'type' => 'BrigKinds16_t',
                                                         'wname' => 'kind',
                                                         'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                         'name' => 'kind',
                                                         'enum' => 'BrigKinds',
                                                         'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>'
                                                       },
                                                       {
                                                         'wtype' => 'StrRef',
                                                         'name' => 'name',
                                                         'acc' => 'strRef',
                                                         'type' => 'BrigDataOffsetString32_t',
                                                         'wname' => 'name',
                                                         'defValue' => '0'
                                                       }
                                                     ],
                                         'enum' => 'BRIG_KIND_DIRECTIVE_COMMENT',
                                         'name' => 'BrigDirectiveComment'
                                       },
             'BrigInstSegCvt' => {
                                   'name' => 'BrigInstSegCvt',
                                   'enum' => 'BRIG_KIND_INST_SEG_CVT',
                                   'fields' => [
                                                 {
                                                   'acc' => 'valRef',
                                                   'name' => 'byteCount',
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'wname' => 'byteCount',
                                                   'type' => 'uint16_t'
                                                 },
                                                 {
                                                   'wname' => 'kind',
                                                   'type' => 'BrigKinds16_t',
                                                   'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                   'enum' => 'BrigKinds',
                                                   'name' => 'kind',
                                                   'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>'
                                                 },
                                                 {
                                                   'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                   'enum' => 'BrigOpcode',
                                                   'name' => 'opcode',
                                                   'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                   'wname' => 'opcode',
                                                   'type' => 'BrigOpcode16_t'
                                                 },
                                                 {
                                                   'wname' => 'type',
                                                   'type' => 'BrigType16_t',
                                                   'name' => 'type',
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'acc' => 'valRef'
                                                 },
                                                 {
                                                   'acc' => 'listRef<Operand>',
                                                   'name' => 'operands',
                                                   'wtype' => 'ListRef<Operand>',
                                                   'defValue' => '0',
                                                   'wname' => 'operands',
                                                   'type' => 'BrigDataOffsetOperandList32_t'
                                                 },
                                                 {
                                                   'type' => 'BrigType16_t',
                                                   'wname' => 'sourceType',
                                                   'acc' => 'valRef',
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'name' => 'sourceType'
                                                 },
                                                 {
                                                   'enum' => 'BrigSegment',
                                                   'acc' => 'enumValRef<Brig::BrigSegment,uint8_t>',
                                                   'name' => 'segment',
                                                   'wtype' => 'EnumValRef<Brig::BrigSegment,uint8_t>',
                                                   'wname' => 'segment',
                                                   'defValue' => 'Brig::BRIG_SEGMENT_NONE',
                                                   'type' => 'BrigSegment8_t'
                                                 },
                                                 {
                                                   'acc' => 'subItem<SegCvtModifier>',
                                                   'name' => 'modifier',
                                                   'wtype' => 'SegCvtModifier',
                                                   'wname' => 'modifier',
                                                   'type' => 'BrigSegCvtModifier'
                                                 }
                                               ],
                                   'align' => undef,
                                   'wname' => 'InstSegCvt',
                                   'parent' => 'BrigInst'
                                 },
             'BrigVariableModifier' => {
                                         'wname' => 'VariableModifier',
                                         'standalone' => 'true',
                                         'align' => undef,
                                         'isroot' => 'true',
                                         'enum' => 'BRIG_KIND_VARIABLE_MODIFIER',
                                         'fields' => [
                                                       {
                                                         'wtype' => 'ValRef<uint8_t>',
                                                         'name' => 'allBits',
                                                         'acc' => 'valRef',
                                                         'type' => 'BrigVariableModifier8_t',
                                                         'wname' => 'allBits',
                                                         'defValue' => '0'
                                                       },
                                                       {
                                                         'acc' => 'bitValRef<0>',
                                                         'name' => 'isDefinition',
                                                         'wtype' => 'BitValRef<0>',
                                                         'wname' => 'isDefinition',
                                                         'type' => 'bool',
                                                         'phantomof' => $structs->{'BrigVariableModifier'}{'fields'}[0]
                                                       },
                                                       {
                                                         'wname' => 'isConst',
                                                         'type' => 'bool',
                                                         'phantomof' => $structs->{'BrigVariableModifier'}{'fields'}[0],
                                                         'acc' => 'bitValRef<1>',
                                                         'name' => 'isConst',
                                                         'wtype' => 'BitValRef<1>'
                                                       },
                                                       {
                                                         'wtype' => 'BitValRef<2>',
                                                         'name' => 'isArray',
                                                         'acc' => 'bitValRef<2>',
                                                         'phantomof' => $structs->{'BrigVariableModifier'}{'fields'}[0],
                                                         'type' => 'bool',
                                                         'wname' => 'isArray'
                                                       },
                                                       {
                                                         'wtype' => 'BitValRef<3>',
                                                         'name' => 'isFlexArray',
                                                         'acc' => 'bitValRef<3>',
                                                         'phantomof' => $structs->{'BrigVariableModifier'}{'fields'}[0],
                                                         'type' => 'bool',
                                                         'wname' => 'isFlexArray'
                                                       }
                                                     ],
                                         'name' => 'BrigVariableModifier'
                                       },
             'BrigOperandImageProperties' => {
                                               'name' => 'BrigOperandImageProperties',
                                               'fields' => [
                                                             {
                                                               'type' => 'uint16_t',
                                                               'wname' => 'byteCount',
                                                               'wtype' => 'ValRef<uint16_t>',
                                                               'name' => 'byteCount',
                                                               'acc' => 'valRef'
                                                             },
                                                             {
                                                               'wname' => 'kind',
                                                               'type' => 'BrigKinds16_t',
                                                               'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                               'enum' => 'BrigKinds',
                                                               'name' => 'kind',
                                                               'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>'
                                                             },
                                                             {
                                                               'type' => 'BrigUInt64',
                                                               'wname' => 'width',
                                                               'acc' => 'subItem<UInt64>',
                                                               'wtype' => 'UInt64',
                                                               'name' => 'width'
                                                             },
                                                             {
                                                               'wtype' => 'UInt64',
                                                               'name' => 'height',
                                                               'acc' => 'subItem<UInt64>',
                                                               'type' => 'BrigUInt64',
                                                               'wname' => 'height'
                                                             },
                                                             {
                                                               'acc' => 'subItem<UInt64>',
                                                               'name' => 'depth',
                                                               'wtype' => 'UInt64',
                                                               'wname' => 'depth',
                                                               'type' => 'BrigUInt64'
                                                             },
                                                             {
                                                               'wname' => 'array',
                                                               'type' => 'BrigUInt64',
                                                               'acc' => 'subItem<UInt64>',
                                                               'name' => 'array',
                                                               'wtype' => 'UInt64'
                                                             },
                                                             {
                                                               'type' => 'BrigImageGeometry8_t',
                                                               'wname' => 'geometry',
                                                               'defValue' => 'Brig::BRIG_GEOMETRY_UNKNOWN',
                                                               'wtype' => 'EnumValRef<Brig::BrigImageGeometry,uint8_t>',
                                                               'name' => 'geometry',
                                                               'acc' => 'enumValRef<Brig::BrigImageGeometry,uint8_t>',
                                                               'enum' => 'BrigImageGeometry'
                                                             },
                                                             {
                                                               'acc' => 'enumValRef<Brig::BrigImageChannelOrder,uint8_t>',
                                                               'enum' => 'BrigImageChannelOrder',
                                                               'wtype' => 'EnumValRef<Brig::BrigImageChannelOrder,uint8_t>',
                                                               'name' => 'channelOrder',
                                                               'type' => 'BrigImageChannelOrder8_t',
                                                               'wname' => 'channelOrder',
                                                               'defValue' => 'Brig::BRIG_CHANNEL_ORDER_UNKNOWN'
                                                             },
                                                             {
                                                               'type' => 'BrigImageChannelType8_t',
                                                               'wname' => 'channelType',
                                                               'defValue' => 'Brig::BRIG_CHANNEL_TYPE_UNKNOWN',
                                                               'wtype' => 'EnumValRef<Brig::BrigImageChannelType,uint8_t>',
                                                               'name' => 'channelType',
                                                               'acc' => 'enumValRef<Brig::BrigImageChannelType,uint8_t>',
                                                               'enum' => 'BrigImageChannelType'
                                                             },
                                                             {
                                                               'name' => 'reserved',
                                                               'skip' => 1,
                                                               'wtype' => 'ValRef<uint8_t>',
                                                               'acc' => 'valRef',
                                                               'defValue' => '0',
                                                               'wname' => 'reserved',
                                                               'type' => 'uint8_t'
                                                             }
                                                           ],
                                               'enum' => 'BRIG_KIND_OPERAND_IMAGE_PROPERTIES',
                                               'align' => undef,
                                               'parent' => 'BrigOperand',
                                               'wname' => 'OperandImageProperties'
                                             },
             'BrigDirectiveArgBlockEnd' => {
                                             'name' => 'BrigDirectiveArgBlockEnd',
                                             'enum' => 'BRIG_KIND_DIRECTIVE_ARG_BLOCK_END',
                                             'fields' => [
                                                           {
                                                             'type' => 'uint16_t',
                                                             'wname' => 'byteCount',
                                                             'acc' => 'valRef',
                                                             'wtype' => 'ValRef<uint16_t>',
                                                             'name' => 'byteCount'
                                                           },
                                                           {
                                                             'name' => 'kind',
                                                             'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                             'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                             'enum' => 'BrigKinds',
                                                             'wname' => 'kind',
                                                             'type' => 'BrigKinds16_t'
                                                           }
                                                         ],
                                             'align' => undef,
                                             'wname' => 'DirectiveArgBlockEnd',
                                             'parent' => 'BrigDirective'
                                           },
             'BrigOperandReg' => {
                                   'enum' => 'BRIG_KIND_OPERAND_REG',
                                   'fields' => [
                                                 {
                                                   'name' => 'byteCount',
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'acc' => 'valRef',
                                                   'wname' => 'byteCount',
                                                   'type' => 'uint16_t'
                                                 },
                                                 {
                                                   'type' => 'BrigKinds16_t',
                                                   'wname' => 'kind',
                                                   'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                   'enum' => 'BrigKinds',
                                                   'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                   'name' => 'kind'
                                                 },
                                                 {
                                                   'wname' => 'regKind',
                                                   'type' => 'BrigRegisterKind16_t',
                                                   'name' => 'regKind',
                                                   'wtype' => 'EnumValRef<Brig::BrigRegisterKind,uint16_t>',
                                                   'acc' => 'enumValRef<Brig::BrigRegisterKind,uint16_t>',
                                                   'enum' => 'BrigRegisterKind'
                                                 },
                                                 {
                                                   'acc' => 'valRef',
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'name' => 'regNum',
                                                   'type' => 'uint16_t',
                                                   'wname' => 'regNum'
                                                 }
                                               ],
                                   'name' => 'BrigOperandReg',
                                   'parent' => 'BrigOperand',
                                   'wname' => 'OperandReg',
                                   'align' => undef
                                 },
             'BrigDirectiveExecutable' => {
                                            'children' => [
                                                            'BrigDirectiveKernel',
                                                            'BrigDirectiveIndirectFunction',
                                                            'BrigDirectiveSignature',
                                                            'BrigDirectiveFunction'
                                                          ],
                                            'wname' => 'DirectiveExecutable',
                                            'name' => 'BrigDirectiveExecutable',
                                            'align' => undef,
                                            'parent' => 'BrigDirective',
                                            'generic' => 'true',
                                            'enum' => 'BRIG_KIND_DIRECTIVE_EXECUTABLE',
                                            'fields' => [
                                                          {
                                                            'type' => 'uint16_t',
                                                            'wname' => 'byteCount',
                                                            'wtype' => 'ValRef<uint16_t>',
                                                            'name' => 'byteCount',
                                                            'acc' => 'valRef'
                                                          },
                                                          {
                                                            'name' => 'kind',
                                                            'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                            'enum' => 'BrigKinds',
                                                            'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                            'wname' => 'kind',
                                                            'type' => 'BrigKinds16_t'
                                                          },
                                                          {
                                                            'type' => 'BrigDataOffsetString32_t',
                                                            'defValue' => '0',
                                                            'wname' => 'name',
                                                            'acc' => 'strRef',
                                                            'wtype' => 'StrRef',
                                                            'name' => 'name'
                                                          },
                                                          {
                                                            'name' => 'outArgCount',
                                                            'wtype' => 'ValRef<uint16_t>',
                                                            'acc' => 'valRef',
                                                            'wname' => 'outArgCount',
                                                            'defValue' => '0',
                                                            'type' => 'uint16_t'
                                                          },
                                                          {
                                                            'acc' => 'valRef',
                                                            'name' => 'inArgCount',
                                                            'wtype' => 'ValRef<uint16_t>',
                                                            'wname' => 'inArgCount',
                                                            'defValue' => '0',
                                                            'type' => 'uint16_t'
                                                          },
                                                          {
                                                            'acc' => 'itemRef<Code>',
                                                            'wtype' => 'ItemRef<Code>',
                                                            'name' => 'firstInArg',
                                                            'type' => 'BrigCodeOffset32_t',
                                                            'wname' => 'firstInArg',
                                                            'defValue' => '0'
                                                          },
                                                          {
                                                            'acc' => 'itemRef<Code>',
                                                            'wtype' => 'ItemRef<Code>',
                                                            'name' => 'firstCodeBlockEntry',
                                                            'type' => 'BrigCodeOffset32_t',
                                                            'wname' => 'firstCodeBlockEntry',
                                                            'defValue' => '0'
                                                          },
                                                          {
                                                            'type' => 'BrigCodeOffset32_t',
                                                            'wname' => 'nextModuleEntry',
                                                            'defValue' => '0',
                                                            'acc' => 'itemRef<Code>',
                                                            'wtype' => 'ItemRef<Code>',
                                                            'name' => 'nextModuleEntry'
                                                          },
                                                          {
                                                            'acc' => 'valRef',
                                                            'wtype' => 'ValRef<uint32_t>',
                                                            'name' => 'codeBlockEntryCount',
                                                            'type' => 'uint32_t',
                                                            'defValue' => '0',
                                                            'wname' => 'codeBlockEntryCount'
                                                          },
                                                          {
                                                            'acc' => 'subItem<ExecutableModifier>',
                                                            'wtype' => 'ExecutableModifier',
                                                            'name' => 'modifier',
                                                            'type' => 'BrigExecutableModifier',
                                                            'wname' => 'modifier'
                                                          },
                                                          {
                                                            'defValue' => 'Brig::BRIG_LINKAGE_NONE',
                                                            'wname' => 'linkage',
                                                            'type' => 'BrigLinkage8_t',
                                                            'name' => 'linkage',
                                                            'wtype' => 'EnumValRef<Brig::BrigLinkage,uint8_t>',
                                                            'acc' => 'enumValRef<Brig::BrigLinkage,uint8_t>',
                                                            'enum' => 'BrigLinkage'
                                                          },
                                                          {
                                                            'defValue' => '0',
                                                            'wname' => 'reserved',
                                                            'type' => 'uint16_t',
                                                            'acc' => 'valRef',
                                                            'name' => 'reserved',
                                                            'wtype' => 'ValRef<uint16_t>',
                                                            'skip' => 1
                                                          }
                                                        ]
                                          },
             'BrigInstCmp' => {
                                'name' => 'BrigInstCmp',
                                'enum' => 'BRIG_KIND_INST_CMP',
                                'fields' => [
                                              {
                                                'wname' => 'byteCount',
                                                'type' => 'uint16_t',
                                                'name' => 'byteCount',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'acc' => 'valRef'
                                              },
                                              {
                                                'type' => 'BrigKinds16_t',
                                                'wname' => 'kind',
                                                'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                'name' => 'kind',
                                                'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                'enum' => 'BrigKinds'
                                              },
                                              {
                                                'wname' => 'opcode',
                                                'type' => 'BrigOpcode16_t',
                                                'name' => 'opcode',
                                                'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                'enum' => 'BrigOpcode',
                                                'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>'
                                              },
                                              {
                                                'wname' => 'type',
                                                'type' => 'BrigType16_t',
                                                'acc' => 'valRef',
                                                'name' => 'type',
                                                'wtype' => 'ValRef<uint16_t>'
                                              },
                                              {
                                                'acc' => 'listRef<Operand>',
                                                'wtype' => 'ListRef<Operand>',
                                                'name' => 'operands',
                                                'type' => 'BrigDataOffsetOperandList32_t',
                                                'wname' => 'operands',
                                                'defValue' => '0'
                                              },
                                              {
                                                'wname' => 'sourceType',
                                                'type' => 'BrigType16_t',
                                                'acc' => 'valRef',
                                                'name' => 'sourceType',
                                                'wtype' => 'ValRef<uint16_t>'
                                              },
                                              {
                                                'wtype' => 'AluModifier',
                                                'name' => 'modifier',
                                                'acc' => 'subItem<AluModifier>',
                                                'type' => 'BrigAluModifier',
                                                'wname' => 'modifier'
                                              },
                                              {
                                                'name' => 'compare',
                                                'wtype' => 'EnumValRef<Brig::BrigCompareOperation,uint8_t>',
                                                'acc' => 'enumValRef<Brig::BrigCompareOperation,uint8_t>',
                                                'enum' => 'BrigCompareOperation',
                                                'wname' => 'compare',
                                                'type' => 'BrigCompareOperation8_t'
                                              },
                                              {
                                                'name' => 'pack',
                                                'wtype' => 'EnumValRef<Brig::BrigPack,uint8_t>',
                                                'enum' => 'BrigPack',
                                                'acc' => 'enumValRef<Brig::BrigPack,uint8_t>',
                                                'wname' => 'pack',
                                                'defValue' => 'Brig::BRIG_PACK_NONE',
                                                'type' => 'BrigPack8_t'
                                              },
                                              {
                                                'acc' => 'valRef',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'skip' => 1,
                                                'name' => 'reserved',
                                                'type' => 'uint16_t',
                                                'defValue' => '0',
                                                'wname' => 'reserved'
                                              }
                                            ],
                                'align' => undef,
                                'parent' => 'BrigInst',
                                'wname' => 'InstCmp'
                              },
             'BrigInstAtomic' => {
                                   'fields' => [
                                                 {
                                                   'name' => 'byteCount',
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'acc' => 'valRef',
                                                   'wname' => 'byteCount',
                                                   'type' => 'uint16_t'
                                                 },
                                                 {
                                                   'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                   'name' => 'kind',
                                                   'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                   'enum' => 'BrigKinds',
                                                   'type' => 'BrigKinds16_t',
                                                   'wname' => 'kind'
                                                 },
                                                 {
                                                   'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                   'enum' => 'BrigOpcode',
                                                   'name' => 'opcode',
                                                   'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                   'wname' => 'opcode',
                                                   'type' => 'BrigOpcode16_t'
                                                 },
                                                 {
                                                   'wname' => 'type',
                                                   'type' => 'BrigType16_t',
                                                   'acc' => 'valRef',
                                                   'name' => 'type',
                                                   'wtype' => 'ValRef<uint16_t>'
                                                 },
                                                 {
                                                   'defValue' => '0',
                                                   'wname' => 'operands',
                                                   'type' => 'BrigDataOffsetOperandList32_t',
                                                   'acc' => 'listRef<Operand>',
                                                   'name' => 'operands',
                                                   'wtype' => 'ListRef<Operand>'
                                                 },
                                                 {
                                                   'type' => 'BrigSegment8_t',
                                                   'defValue' => 'Brig::BRIG_SEGMENT_NONE',
                                                   'wname' => 'segment',
                                                   'enum' => 'BrigSegment',
                                                   'acc' => 'enumValRef<Brig::BrigSegment,uint8_t>',
                                                   'wtype' => 'EnumValRef<Brig::BrigSegment,uint8_t>',
                                                   'name' => 'segment'
                                                 },
                                                 {
                                                   'name' => 'memoryOrder',
                                                   'wtype' => 'EnumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                   'enum' => 'BrigMemoryOrder',
                                                   'acc' => 'enumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                   'wname' => 'memoryOrder',
                                                   'defValue' => 'Brig::BRIG_MEMORY_ORDER_RELAXED',
                                                   'type' => 'BrigMemoryOrder8_t'
                                                 },
                                                 {
                                                   'wname' => 'memoryScope',
                                                   'defValue' => 'Brig::BRIG_MEMORY_SCOPE_SYSTEM',
                                                   'type' => 'BrigMemoryScope8_t',
                                                   'enum' => 'BrigMemoryScope',
                                                   'acc' => 'enumValRef<Brig::BrigMemoryScope,uint8_t>',
                                                   'name' => 'memoryScope',
                                                   'wtype' => 'EnumValRef<Brig::BrigMemoryScope,uint8_t>'
                                                 },
                                                 {
                                                   'wtype' => 'EnumValRef<Brig::BrigAtomicOperation,uint8_t>',
                                                   'name' => 'atomicOperation',
                                                   'acc' => 'enumValRef<Brig::BrigAtomicOperation,uint8_t>',
                                                   'enum' => 'BrigAtomicOperation',
                                                   'type' => 'BrigAtomicOperation8_t',
                                                   'wname' => 'atomicOperation'
                                                 },
                                                 {
                                                   'wname' => 'equivClass',
                                                   'type' => 'uint8_t',
                                                   'acc' => 'valRef',
                                                   'name' => 'equivClass',
                                                   'wtype' => 'ValRef<uint8_t>'
                                                 },
                                                 {
                                                   'size' => 3,
                                                   'skip' => 1,
                                                   'type' => 'uint8_t',
                                                   'defValue' => '0',
                                                   'wname' => 'reserved',
                                                   'wtype' => 'ValRef<uint8_t>',
                                                   'name' => 'reserved',
                                                   'acc' => 'valRef'
                                                 }
                                               ],
                                   'enum' => 'BRIG_KIND_INST_ATOMIC',
                                   'name' => 'BrigInstAtomic',
                                   'parent' => 'BrigInst',
                                   'wname' => 'InstAtomic',
                                   'align' => undef
                                 },
             'BrigInstMod' => {
                                'name' => 'BrigInstMod',
                                'enum' => 'BRIG_KIND_INST_MOD',
                                'fields' => [
                                              {
                                                'acc' => 'valRef',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'name' => 'byteCount',
                                                'type' => 'uint16_t',
                                                'wname' => 'byteCount'
                                              },
                                              {
                                                'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                'enum' => 'BrigKinds',
                                                'name' => 'kind',
                                                'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                'wname' => 'kind',
                                                'type' => 'BrigKinds16_t'
                                              },
                                              {
                                                'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                'name' => 'opcode',
                                                'enum' => 'BrigOpcode',
                                                'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                'type' => 'BrigOpcode16_t',
                                                'wname' => 'opcode'
                                              },
                                              {
                                                'acc' => 'valRef',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'name' => 'type',
                                                'type' => 'BrigType16_t',
                                                'wname' => 'type'
                                              },
                                              {
                                                'acc' => 'listRef<Operand>',
                                                'wtype' => 'ListRef<Operand>',
                                                'name' => 'operands',
                                                'type' => 'BrigDataOffsetOperandList32_t',
                                                'wname' => 'operands',
                                                'defValue' => '0'
                                              },
                                              {
                                                'wtype' => 'AluModifier',
                                                'name' => 'modifier',
                                                'acc' => 'subItem<AluModifier>',
                                                'type' => 'BrigAluModifier',
                                                'wname' => 'modifier'
                                              },
                                              {
                                                'type' => 'BrigPack8_t',
                                                'wname' => 'pack',
                                                'defValue' => 'Brig::BRIG_PACK_NONE',
                                                'enum' => 'BrigPack',
                                                'acc' => 'enumValRef<Brig::BrigPack,uint8_t>',
                                                'wtype' => 'EnumValRef<Brig::BrigPack,uint8_t>',
                                                'name' => 'pack'
                                              },
                                              {
                                                'type' => 'uint8_t',
                                                'wname' => 'reserved',
                                                'defValue' => '0',
                                                'acc' => 'valRef',
                                                'skip' => 1,
                                                'wtype' => 'ValRef<uint8_t>',
                                                'name' => 'reserved'
                                              }
                                            ],
                                'align' => undef,
                                'wname' => 'InstMod',
                                'parent' => 'BrigInst'
                              },
             'BrigCode' => {
                             'fields' => [
                                           {
                                             'type' => 'uint16_t',
                                             'wname' => 'byteCount',
                                             'wtype' => 'ValRef<uint16_t>',
                                             'name' => 'byteCount',
                                             'acc' => 'valRef'
                                           },
                                           {
                                             'type' => 'BrigKinds16_t',
                                             'wname' => 'kind',
                                             'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                             'name' => 'kind',
                                             'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                             'enum' => 'BrigKinds'
                                           }
                                         ],
                             'enum' => 'BRIG_KIND_CODE',
                             'isroot' => 'true',
                             'section' => 'BRIG_SECTION_INDEX_CODE',
                             'align' => undef,
                             'generic' => 'true',
                             'name' => 'BrigCode',
                             'children' => [
                                             'BrigDirectiveLabel',
                                             'BrigInstSignal',
                                             'BrigDirectiveVersion',
                                             'BrigInstImage',
                                             'BrigDirectiveNone',
                                             'BrigInstQueue',
                                             'BrigInstQuerySampler',
                                             'BrigDirectiveSignature',
                                             'BrigDirectiveFunction',
                                             'BrigDirectiveIndirectFunction',
                                             'BrigInstBr',
                                             'BrigDirectiveArgBlockStart',
                                             'BrigInstCvt',
                                             'BrigDirectiveLoc',
                                             'BrigDirectiveKernel',
                                             'BrigInst',
                                             'BrigInstAddr',
                                             'BrigDirectivePragma',
                                             'BrigInstBasic',
                                             'BrigInstSourceType',
                                             'BrigInstMemFence',
                                             'BrigInstQueryImage',
                                             'BrigInstMem',
                                             'BrigDirectiveControl',
                                             'BrigDirective',
                                             'BrigInstLane',
                                             'BrigDirectiveFbarrier',
                                             'BrigInstSeg',
                                             'BrigDirectiveVariable',
                                             'BrigDirectiveExtension',
                                             'BrigDirectiveComment',
                                             'BrigInstSegCvt',
                                             'BrigDirectiveExecutable',
                                             'BrigDirectiveArgBlockEnd',
                                             'BrigInstCmp',
                                             'BrigInstMod',
                                             'BrigInstAtomic'
                                           ],
                             'wname' => 'Code'
                           },
             'BrigOperandData' => {
                                    'fields' => [
                                                  {
                                                    'acc' => 'valRef',
                                                    'wtype' => 'ValRef<uint16_t>',
                                                    'name' => 'byteCount',
                                                    'type' => 'uint16_t',
                                                    'wname' => 'byteCount'
                                                  },
                                                  {
                                                    'wname' => 'kind',
                                                    'type' => 'BrigKinds16_t',
                                                    'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                    'enum' => 'BrigKinds',
                                                    'name' => 'kind',
                                                    'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>'
                                                  },
                                                  {
                                                    'defValue' => '0',
                                                    'wname' => 'data',
                                                    'type' => 'BrigDataOffsetString32_t',
                                                    'acc' => 'strRef',
                                                    'name' => 'data',
                                                    'wtype' => 'StrRef'
                                                  }
                                                ],
                                    'enum' => 'BRIG_KIND_OPERAND_DATA',
                                    'name' => 'BrigOperandData',
                                    'parent' => 'BrigOperand',
                                    'wname' => 'OperandData',
                                    'align' => undef
                                  },
             'BrigOperandAddress' => {
                                       'name' => 'BrigOperandAddress',
                                       'enum' => 'BRIG_KIND_OPERAND_ADDRESS',
                                       'fields' => [
                                                     {
                                                       'acc' => 'valRef',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'name' => 'byteCount',
                                                       'type' => 'uint16_t',
                                                       'wname' => 'byteCount'
                                                     },
                                                     {
                                                       'wname' => 'kind',
                                                       'type' => 'BrigKinds16_t',
                                                       'enum' => 'BrigKinds',
                                                       'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                       'name' => 'kind',
                                                       'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>'
                                                     },
                                                     {
                                                       'name' => 'symbol',
                                                       'wtype' => 'ItemRef<DirectiveVariable>',
                                                       'acc' => 'itemRef<DirectiveVariable>',
                                                       'defValue' => '0',
                                                       'wname' => 'symbol',
                                                       'type' => 'BrigCodeOffset32_t'
                                                     },
                                                     {
                                                       'name' => 'reg',
                                                       'wtype' => 'ItemRef<OperandReg>',
                                                       'acc' => 'itemRef<OperandReg>',
                                                       'defValue' => '0',
                                                       'wname' => 'reg',
                                                       'type' => 'BrigOperandOffset32_t'
                                                     },
                                                     {
                                                       'acc' => 'subItem<UInt64>',
                                                       'wtype' => 'UInt64',
                                                       'name' => 'offset',
                                                       'type' => 'BrigUInt64',
                                                       'wname' => 'offset'
                                                     }
                                                   ],
                                       'align' => undef,
                                       'parent' => 'BrigOperand',
                                       'wname' => 'OperandAddress'
                                     },
             'BrigDirectiveLabel' => {
                                       'name' => 'BrigDirectiveLabel',
                                       'enum' => 'BRIG_KIND_DIRECTIVE_LABEL',
                                       'fields' => [
                                                     {
                                                       'type' => 'uint16_t',
                                                       'wname' => 'byteCount',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'name' => 'byteCount',
                                                       'acc' => 'valRef'
                                                     },
                                                     {
                                                       'wname' => 'kind',
                                                       'type' => 'BrigKinds16_t',
                                                       'enum' => 'BrigKinds',
                                                       'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                       'name' => 'kind',
                                                       'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>'
                                                     },
                                                     {
                                                       'type' => 'BrigDataOffsetString32_t',
                                                       'defValue' => '0',
                                                       'wname' => 'name',
                                                       'wtype' => 'StrRef',
                                                       'name' => 'name',
                                                       'acc' => 'strRef'
                                                     }
                                                   ],
                                       'align' => undef,
                                       'wname' => 'DirectiveLabel',
                                       'parent' => 'BrigDirective'
                                     },
             'BrigInstSignal' => {
                                   'fields' => [
                                                 {
                                                   'type' => 'uint16_t',
                                                   'wname' => 'byteCount',
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'name' => 'byteCount',
                                                   'acc' => 'valRef'
                                                 },
                                                 {
                                                   'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                   'enum' => 'BrigKinds',
                                                   'name' => 'kind',
                                                   'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                   'wname' => 'kind',
                                                   'type' => 'BrigKinds16_t'
                                                 },
                                                 {
                                                   'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                   'enum' => 'BrigOpcode',
                                                   'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                   'name' => 'opcode',
                                                   'type' => 'BrigOpcode16_t',
                                                   'wname' => 'opcode'
                                                 },
                                                 {
                                                   'type' => 'BrigType16_t',
                                                   'wname' => 'type',
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'name' => 'type',
                                                   'acc' => 'valRef'
                                                 },
                                                 {
                                                   'wtype' => 'ListRef<Operand>',
                                                   'name' => 'operands',
                                                   'acc' => 'listRef<Operand>',
                                                   'type' => 'BrigDataOffsetOperandList32_t',
                                                   'defValue' => '0',
                                                   'wname' => 'operands'
                                                 },
                                                 {
                                                   'wname' => 'signalType',
                                                   'type' => 'BrigType16_t',
                                                   'acc' => 'valRef',
                                                   'name' => 'signalType',
                                                   'wtype' => 'ValRef<uint16_t>'
                                                 },
                                                 {
                                                   'wtype' => 'EnumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                   'name' => 'memoryOrder',
                                                   'enum' => 'BrigMemoryOrder',
                                                   'acc' => 'enumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                   'type' => 'BrigMemoryOrder8_t',
                                                   'wname' => 'memoryOrder',
                                                   'defValue' => 'Brig::BRIG_MEMORY_ORDER_RELAXED'
                                                 },
                                                 {
                                                   'enum' => 'BrigAtomicOperation',
                                                   'acc' => 'enumValRef<Brig::BrigAtomicOperation,uint8_t>',
                                                   'wtype' => 'EnumValRef<Brig::BrigAtomicOperation,uint8_t>',
                                                   'name' => 'signalOperation',
                                                   'type' => 'BrigAtomicOperation8_t',
                                                   'wname' => 'signalOperation'
                                                 }
                                               ],
                                   'enum' => 'BRIG_KIND_INST_SIGNAL',
                                   'name' => 'BrigInstSignal',
                                   'wname' => 'InstSignal',
                                   'parent' => 'BrigInst',
                                   'align' => undef
                                 },
             'BrigOperandString' => {
                                      'align' => undef,
                                      'parent' => 'BrigOperand',
                                      'wname' => 'OperandString',
                                      'name' => 'BrigOperandString',
                                      'enum' => 'BRIG_KIND_OPERAND_STRING',
                                      'fields' => [
                                                    {
                                                      'wname' => 'byteCount',
                                                      'type' => 'uint16_t',
                                                      'acc' => 'valRef',
                                                      'name' => 'byteCount',
                                                      'wtype' => 'ValRef<uint16_t>'
                                                    },
                                                    {
                                                      'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                      'name' => 'kind',
                                                      'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                      'enum' => 'BrigKinds',
                                                      'type' => 'BrigKinds16_t',
                                                      'wname' => 'kind'
                                                    },
                                                    {
                                                      'name' => 'string',
                                                      'wtype' => 'StrRef',
                                                      'acc' => 'strRef',
                                                      'defValue' => '0',
                                                      'wname' => 'string',
                                                      'type' => 'BrigDataOffsetString32_t'
                                                    }
                                                  ]
                                    },
             'BrigDirectiveVersion' => {
                                         'parent' => 'BrigDirective',
                                         'wname' => 'DirectiveVersion',
                                         'align' => undef,
                                         'fields' => [
                                                       {
                                                         'wname' => 'byteCount',
                                                         'type' => 'uint16_t',
                                                         'acc' => 'valRef',
                                                         'name' => 'byteCount',
                                                         'wtype' => 'ValRef<uint16_t>'
                                                       },
                                                       {
                                                         'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                         'name' => 'kind',
                                                         'enum' => 'BrigKinds',
                                                         'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                         'type' => 'BrigKinds16_t',
                                                         'wname' => 'kind'
                                                       },
                                                       {
                                                         'wname' => 'hsailMajor',
                                                         'type' => 'BrigVersion32_t',
                                                         'enum' => 'BrigVersion',
                                                         'acc' => 'valRef',
                                                         'name' => 'hsailMajor',
                                                         'wtype' => 'ValRef<uint32_t>'
                                                       },
                                                       {
                                                         'acc' => 'valRef',
                                                         'enum' => 'BrigVersion',
                                                         'wtype' => 'ValRef<uint32_t>',
                                                         'name' => 'hsailMinor',
                                                         'type' => 'BrigVersion32_t',
                                                         'wname' => 'hsailMinor'
                                                       },
                                                       {
                                                         'wname' => 'brigMajor',
                                                         'type' => 'BrigVersion32_t',
                                                         'acc' => 'valRef',
                                                         'enum' => 'BrigVersion',
                                                         'name' => 'brigMajor',
                                                         'wtype' => 'ValRef<uint32_t>'
                                                       },
                                                       {
                                                         'wname' => 'brigMinor',
                                                         'type' => 'BrigVersion32_t',
                                                         'name' => 'brigMinor',
                                                         'wtype' => 'ValRef<uint32_t>',
                                                         'enum' => 'BrigVersion',
                                                         'acc' => 'valRef'
                                                       },
                                                       {
                                                         'acc' => 'enumValRef<Brig::BrigProfile,uint8_t>',
                                                         'enum' => 'BrigProfile',
                                                         'wtype' => 'EnumValRef<Brig::BrigProfile,uint8_t>',
                                                         'name' => 'profile',
                                                         'type' => 'BrigProfile8_t',
                                                         'wname' => 'profile',
                                                         'defValue' => 'Brig::BRIG_PROFILE_FULL'
                                                       },
                                                       {
                                                         'defValue' => 'Brig::BRIG_MACHINE_LARGE',
                                                         'wname' => 'machineModel',
                                                         'type' => 'BrigMachineModel8_t',
                                                         'acc' => 'enumValRef<Brig::BrigMachineModel,uint8_t>',
                                                         'enum' => 'BrigMachineModel',
                                                         'name' => 'machineModel',
                                                         'wtype' => 'EnumValRef<Brig::BrigMachineModel,uint8_t>'
                                                       },
                                                       {
                                                         'wname' => 'reserved',
                                                         'defValue' => '0',
                                                         'type' => 'uint16_t',
                                                         'name' => 'reserved',
                                                         'wtype' => 'ValRef<uint16_t>',
                                                         'skip' => 1,
                                                         'acc' => 'valRef'
                                                       }
                                                     ],
                                         'enum' => 'BRIG_KIND_DIRECTIVE_VERSION',
                                         'name' => 'BrigDirectiveVersion'
                                       },
             'BrigDirectiveNone' => {
                                      'fields' => [
                                                    {
                                                      'wname' => 'byteCount',
                                                      'type' => 'uint16_t',
                                                      'acc' => 'valRef',
                                                      'name' => 'byteCount',
                                                      'wtype' => 'ValRef<uint16_t>'
                                                    },
                                                    {
                                                      'wname' => 'kind',
                                                      'type' => 'BrigKinds16_t',
                                                      'enum' => 'BrigKinds',
                                                      'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                      'name' => 'kind',
                                                      'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>'
                                                    }
                                                  ],
                                      'enum' => 'BRIG_KIND_NONE',
                                      'name' => 'BrigDirectiveNone',
                                      'parent' => 'BrigDirective',
                                      'wname' => 'DirectiveNone',
                                      'align' => undef
                                    },
             'BrigInstImage' => {
                                  'enum' => 'BRIG_KIND_INST_IMAGE',
                                  'fields' => [
                                                {
                                                  'acc' => 'valRef',
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'name' => 'byteCount',
                                                  'type' => 'uint16_t',
                                                  'wname' => 'byteCount'
                                                },
                                                {
                                                  'name' => 'kind',
                                                  'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                  'enum' => 'BrigKinds',
                                                  'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                  'wname' => 'kind',
                                                  'type' => 'BrigKinds16_t'
                                                },
                                                {
                                                  'name' => 'opcode',
                                                  'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                  'enum' => 'BrigOpcode',
                                                  'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                  'wname' => 'opcode',
                                                  'type' => 'BrigOpcode16_t'
                                                },
                                                {
                                                  'name' => 'type',
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'acc' => 'valRef',
                                                  'wname' => 'type',
                                                  'type' => 'BrigType16_t'
                                                },
                                                {
                                                  'type' => 'BrigDataOffsetOperandList32_t',
                                                  'wname' => 'operands',
                                                  'defValue' => '0',
                                                  'acc' => 'listRef<Operand>',
                                                  'wtype' => 'ListRef<Operand>',
                                                  'name' => 'operands'
                                                },
                                                {
                                                  'wname' => 'imageType',
                                                  'type' => 'BrigType16_t',
                                                  'acc' => 'valRef',
                                                  'name' => 'imageType',
                                                  'wtype' => 'ValRef<uint16_t>'
                                                },
                                                {
                                                  'wname' => 'coordType',
                                                  'type' => 'BrigType16_t',
                                                  'acc' => 'valRef',
                                                  'name' => 'coordType',
                                                  'wtype' => 'ValRef<uint16_t>'
                                                },
                                                {
                                                  'defValue' => 'Brig::BRIG_GEOMETRY_UNKNOWN',
                                                  'wname' => 'geometry',
                                                  'type' => 'BrigImageGeometry8_t',
                                                  'acc' => 'enumValRef<Brig::BrigImageGeometry,uint8_t>',
                                                  'enum' => 'BrigImageGeometry',
                                                  'name' => 'geometry',
                                                  'wtype' => 'EnumValRef<Brig::BrigImageGeometry,uint8_t>'
                                                },
                                                {
                                                  'acc' => 'valRef',
                                                  'name' => 'equivClass',
                                                  'wtype' => 'ValRef<uint8_t>',
                                                  'wname' => 'equivClass',
                                                  'type' => 'uint8_t'
                                                },
                                                {
                                                  'type' => 'uint16_t',
                                                  'wname' => 'reserved',
                                                  'defValue' => '0',
                                                  'acc' => 'valRef',
                                                  'skip' => 1,
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'name' => 'reserved'
                                                }
                                              ],
                                  'name' => 'BrigInstImage',
                                  'wname' => 'InstImage',
                                  'parent' => 'BrigInst',
                                  'align' => undef
                                },
             'BrigInstQueue' => {
                                  'name' => 'BrigInstQueue',
                                  'enum' => 'BRIG_KIND_INST_QUEUE',
                                  'fields' => [
                                                {
                                                  'acc' => 'valRef',
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'name' => 'byteCount',
                                                  'type' => 'uint16_t',
                                                  'wname' => 'byteCount'
                                                },
                                                {
                                                  'enum' => 'BrigKinds',
                                                  'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                  'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                  'name' => 'kind',
                                                  'type' => 'BrigKinds16_t',
                                                  'wname' => 'kind'
                                                },
                                                {
                                                  'name' => 'opcode',
                                                  'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                  'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                  'enum' => 'BrigOpcode',
                                                  'wname' => 'opcode',
                                                  'type' => 'BrigOpcode16_t'
                                                },
                                                {
                                                  'type' => 'BrigType16_t',
                                                  'wname' => 'type',
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'name' => 'type',
                                                  'acc' => 'valRef'
                                                },
                                                {
                                                  'wtype' => 'ListRef<Operand>',
                                                  'name' => 'operands',
                                                  'acc' => 'listRef<Operand>',
                                                  'type' => 'BrigDataOffsetOperandList32_t',
                                                  'wname' => 'operands',
                                                  'defValue' => '0'
                                                },
                                                {
                                                  'type' => 'BrigSegment8_t',
                                                  'defValue' => 'Brig::BRIG_SEGMENT_NONE',
                                                  'wname' => 'segment',
                                                  'wtype' => 'EnumValRef<Brig::BrigSegment,uint8_t>',
                                                  'name' => 'segment',
                                                  'acc' => 'enumValRef<Brig::BrigSegment,uint8_t>',
                                                  'enum' => 'BrigSegment'
                                                },
                                                {
                                                  'defValue' => 'Brig::BRIG_MEMORY_ORDER_RELAXED',
                                                  'wname' => 'memoryOrder',
                                                  'type' => 'BrigMemoryOrder8_t',
                                                  'acc' => 'enumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                  'enum' => 'BrigMemoryOrder',
                                                  'name' => 'memoryOrder',
                                                  'wtype' => 'EnumValRef<Brig::BrigMemoryOrder,uint8_t>'
                                                },
                                                {
                                                  'acc' => 'valRef',
                                                  'name' => 'reserved',
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'skip' => 1,
                                                  'wname' => 'reserved',
                                                  'defValue' => '0',
                                                  'type' => 'uint16_t'
                                                }
                                              ],
                                  'align' => undef,
                                  'parent' => 'BrigInst',
                                  'wname' => 'InstQueue'
                                },
             'BrigOperandOperandList' => {
                                           'parent' => 'BrigOperand',
                                           'wname' => 'OperandOperandList',
                                           'align' => undef,
                                           'fields' => [
                                                         {
                                                           'type' => 'uint16_t',
                                                           'wname' => 'byteCount',
                                                           'acc' => 'valRef',
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'name' => 'byteCount'
                                                         },
                                                         {
                                                           'type' => 'BrigKinds16_t',
                                                           'wname' => 'kind',
                                                           'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                           'name' => 'kind',
                                                           'enum' => 'BrigKinds',
                                                           'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>'
                                                         },
                                                         {
                                                           'wtype' => 'ListRef<Operand>',
                                                           'hcode' => [
                                                                        'unsigned elementCount();',
                                                                        'Operand elements(int index);'
                                                                      ],
                                                           'name' => 'elements',
                                                           'acc' => 'listRef<Operand>',
                                                           'implcode' => [
                                                                           'inline unsigned KLASS::elementCount() { return elements().size(); }',
                                                                           'inline Operand KLASS::elements(int index) { return elements()[index]; }'
                                                                         ],
                                                           'type' => 'BrigDataOffsetOperandList32_t',
                                                           'defValue' => '0',
                                                           'wname' => 'elements'
                                                         }
                                                       ],
                                           'enum' => 'BRIG_KIND_OPERAND_OPERAND_LIST',
                                           'name' => 'BrigOperandOperandList'
                                         },
             'BrigSegCvtModifier' => {
                                       'fields' => [
                                                     {
                                                       'wname' => 'allBits',
                                                       'defValue' => '0',
                                                       'type' => 'BrigSegCvtModifier8_t',
                                                       'acc' => 'valRef',
                                                       'name' => 'allBits',
                                                       'wtype' => 'ValRef<uint8_t>'
                                                     },
                                                     {
                                                       'type' => 'bool',
                                                       'wname' => 'isNoNull',
                                                       'phantomof' => $structs->{'BrigSegCvtModifier'}{'fields'}[0],
                                                       'acc' => 'bitValRef<0>',
                                                       'wtype' => 'BitValRef<0>',
                                                       'name' => 'isNoNull'
                                                     }
                                                   ],
                                       'enum' => 'BRIG_KIND_SEG_CVT_MODIFIER',
                                       'isroot' => 'true',
                                       'name' => 'BrigSegCvtModifier',
                                       'wname' => 'SegCvtModifier',
                                       'standalone' => 'true',
                                       'align' => undef
                                     },
             'BrigData' => {
                             'align' => undef,
                             'nowrap' => 'true',
                             'name' => 'BrigData',
                             'fields' => [
                                           {
                                             'type' => 'uint32_t',
                                             'name' => 'byteCount'
                                           },
                                           {
                                             'name' => 'bytes',
                                             'size' => '1',
                                             'type' => 'uint8_t'
                                           }
                                         ]
                           },
             'BrigInstQuerySampler' => {
                                         'enum' => 'BRIG_KIND_INST_QUERY_SAMPLER',
                                         'fields' => [
                                                       {
                                                         'wtype' => 'ValRef<uint16_t>',
                                                         'name' => 'byteCount',
                                                         'acc' => 'valRef',
                                                         'type' => 'uint16_t',
                                                         'wname' => 'byteCount'
                                                       },
                                                       {
                                                         'name' => 'kind',
                                                         'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                         'enum' => 'BrigKinds',
                                                         'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                         'wname' => 'kind',
                                                         'type' => 'BrigKinds16_t'
                                                       },
                                                       {
                                                         'enum' => 'BrigOpcode',
                                                         'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                         'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                         'name' => 'opcode',
                                                         'type' => 'BrigOpcode16_t',
                                                         'wname' => 'opcode'
                                                       },
                                                       {
                                                         'wname' => 'type',
                                                         'type' => 'BrigType16_t',
                                                         'name' => 'type',
                                                         'wtype' => 'ValRef<uint16_t>',
                                                         'acc' => 'valRef'
                                                       },
                                                       {
                                                         'type' => 'BrigDataOffsetOperandList32_t',
                                                         'defValue' => '0',
                                                         'wname' => 'operands',
                                                         'acc' => 'listRef<Operand>',
                                                         'wtype' => 'ListRef<Operand>',
                                                         'name' => 'operands'
                                                       },
                                                       {
                                                         'wname' => 'samplerQuery',
                                                         'type' => 'BrigSamplerQuery8_t',
                                                         'name' => 'samplerQuery',
                                                         'wtype' => 'EnumValRef<Brig::BrigSamplerQuery,uint8_t>',
                                                         'enum' => 'BrigSamplerQuery',
                                                         'acc' => 'enumValRef<Brig::BrigSamplerQuery,uint8_t>'
                                                       },
                                                       {
                                                         'acc' => 'valRef',
                                                         'name' => 'reserved',
                                                         'wtype' => 'ValRef<uint8_t>',
                                                         'wname' => 'reserved',
                                                         'defValue' => '0',
                                                         'type' => 'uint8_t',
                                                         'skip' => 1,
                                                         'size' => 3
                                                       }
                                                     ],
                                         'name' => 'BrigInstQuerySampler',
                                         'wname' => 'InstQuerySampler',
                                         'parent' => 'BrigInst',
                                         'align' => undef
                                       },
             'BrigDirectiveSignature' => {
                                           'fields' => [
                                                         {
                                                           'name' => 'byteCount',
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'acc' => 'valRef',
                                                           'wname' => 'byteCount',
                                                           'type' => 'uint16_t'
                                                         },
                                                         {
                                                           'name' => 'kind',
                                                           'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                           'enum' => 'BrigKinds',
                                                           'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                           'wname' => 'kind',
                                                           'type' => 'BrigKinds16_t'
                                                         },
                                                         {
                                                           'name' => 'name',
                                                           'wtype' => 'StrRef',
                                                           'acc' => 'strRef',
                                                           'defValue' => '0',
                                                           'wname' => 'name',
                                                           'type' => 'BrigDataOffsetString32_t'
                                                         },
                                                         {
                                                           'defValue' => '0',
                                                           'wname' => 'outArgCount',
                                                           'type' => 'uint16_t',
                                                           'name' => 'outArgCount',
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'acc' => 'valRef'
                                                         },
                                                         {
                                                           'type' => 'uint16_t',
                                                           'wname' => 'inArgCount',
                                                           'defValue' => '0',
                                                           'acc' => 'valRef',
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'name' => 'inArgCount'
                                                         },
                                                         {
                                                           'wtype' => 'ItemRef<Code>',
                                                           'name' => 'firstInArg',
                                                           'acc' => 'itemRef<Code>',
                                                           'type' => 'BrigCodeOffset32_t',
                                                           'defValue' => '0',
                                                           'wname' => 'firstInArg'
                                                         },
                                                         {
                                                           'acc' => 'itemRef<Code>',
                                                           'wtype' => 'ItemRef<Code>',
                                                           'name' => 'firstCodeBlockEntry',
                                                           'type' => 'BrigCodeOffset32_t',
                                                           'defValue' => '0',
                                                           'wname' => 'firstCodeBlockEntry'
                                                         },
                                                         {
                                                           'type' => 'BrigCodeOffset32_t',
                                                           'wname' => 'nextModuleEntry',
                                                           'defValue' => '0',
                                                           'wtype' => 'ItemRef<Code>',
                                                           'name' => 'nextModuleEntry',
                                                           'acc' => 'itemRef<Code>'
                                                         },
                                                         {
                                                           'acc' => 'valRef',
                                                           'name' => 'codeBlockEntryCount',
                                                           'wtype' => 'ValRef<uint32_t>',
                                                           'defValue' => '0',
                                                           'wname' => 'codeBlockEntryCount',
                                                           'type' => 'uint32_t'
                                                         },
                                                         {
                                                           'wtype' => 'ExecutableModifier',
                                                           'name' => 'modifier',
                                                           'acc' => 'subItem<ExecutableModifier>',
                                                           'type' => 'BrigExecutableModifier',
                                                           'wname' => 'modifier'
                                                         },
                                                         {
                                                           'name' => 'linkage',
                                                           'wtype' => 'EnumValRef<Brig::BrigLinkage,uint8_t>',
                                                           'acc' => 'enumValRef<Brig::BrigLinkage,uint8_t>',
                                                           'enum' => 'BrigLinkage',
                                                           'wname' => 'linkage',
                                                           'defValue' => 'Brig::BRIG_LINKAGE_NONE',
                                                           'type' => 'BrigLinkage8_t'
                                                         },
                                                         {
                                                           'skip' => 1,
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'name' => 'reserved',
                                                           'acc' => 'valRef',
                                                           'type' => 'uint16_t',
                                                           'wname' => 'reserved',
                                                           'defValue' => '0'
                                                         }
                                                       ],
                                           'enum' => 'BRIG_KIND_DIRECTIVE_SIGNATURE',
                                           'name' => 'BrigDirectiveSignature',
                                           'wname' => 'DirectiveSignature',
                                           'parent' => 'BrigDirectiveExecutable',
                                           'align' => undef
                                         },
             'BrigOperand' => {
                                'wname' => 'Operand',
                                'children' => [
                                                'BrigOperandAddress',
                                                'BrigOperandString',
                                                'BrigOperandImageProperties',
                                                'BrigOperandReg',
                                                'BrigOperandOperandList',
                                                'BrigOperandSamplerProperties',
                                                'BrigOperandCodeList',
                                                'BrigOperandWavesize',
                                                'BrigOperandData',
                                                'BrigOperandCodeRef'
                                              ],
                                'name' => 'BrigOperand',
                                'generic' => 'true',
                                'align' => undef,
                                'isroot' => 'true',
                                'enum' => 'BRIG_KIND_OPERAND',
                                'fields' => [
                                              {
                                                'type' => 'uint16_t',
                                                'wname' => 'byteCount',
                                                'acc' => 'valRef',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'name' => 'byteCount'
                                              },
                                              {
                                                'wname' => 'kind',
                                                'type' => 'BrigKinds16_t',
                                                'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                'enum' => 'BrigKinds',
                                                'name' => 'kind',
                                                'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>'
                                              }
                                            ],
                                'section' => 'BRIG_SECTION_INDEX_OPERAND'
                              },
             'BrigOperandSamplerProperties' => {
                                                 'fields' => [
                                                               {
                                                                 'acc' => 'valRef',
                                                                 'name' => 'byteCount',
                                                                 'wtype' => 'ValRef<uint16_t>',
                                                                 'wname' => 'byteCount',
                                                                 'type' => 'uint16_t'
                                                               },
                                                               {
                                                                 'type' => 'BrigKinds16_t',
                                                                 'wname' => 'kind',
                                                                 'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                                 'name' => 'kind',
                                                                 'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                                 'enum' => 'BrigKinds'
                                                               },
                                                               {
                                                                 'wname' => 'coord',
                                                                 'type' => 'BrigSamplerCoordNormalization8_t',
                                                                 'name' => 'coord',
                                                                 'wtype' => 'EnumValRef<Brig::BrigSamplerCoordNormalization,uint8_t>',
                                                                 'acc' => 'enumValRef<Brig::BrigSamplerCoordNormalization,uint8_t>',
                                                                 'enum' => 'BrigSamplerCoordNormalization'
                                                               },
                                                               {
                                                                 'wname' => 'filter',
                                                                 'type' => 'BrigSamplerFilter8_t',
                                                                 'name' => 'filter',
                                                                 'wtype' => 'EnumValRef<Brig::BrigSamplerFilter,uint8_t>',
                                                                 'enum' => 'BrigSamplerFilter',
                                                                 'acc' => 'enumValRef<Brig::BrigSamplerFilter,uint8_t>'
                                                               },
                                                               {
                                                                 'wtype' => 'EnumValRef<Brig::BrigSamplerAddressing,uint8_t>',
                                                                 'name' => 'addressing',
                                                                 'enum' => 'BrigSamplerAddressing',
                                                                 'acc' => 'enumValRef<Brig::BrigSamplerAddressing,uint8_t>',
                                                                 'type' => 'BrigSamplerAddressing8_t',
                                                                 'defValue' => 'Brig::BRIG_ADDRESSING_CLAMP_TO_EDGE',
                                                                 'wname' => 'addressing'
                                                               },
                                                               {
                                                                 'wname' => 'reserved',
                                                                 'defValue' => '0',
                                                                 'type' => 'uint8_t',
                                                                 'acc' => 'valRef',
                                                                 'name' => 'reserved',
                                                                 'wtype' => 'ValRef<uint8_t>',
                                                                 'skip' => 1
                                                               }
                                                             ],
                                                 'enum' => 'BRIG_KIND_OPERAND_SAMPLER_PROPERTIES',
                                                 'name' => 'BrigOperandSamplerProperties',
                                                 'parent' => 'BrigOperand',
                                                 'wname' => 'OperandSamplerProperties',
                                                 'align' => undef
                                               },
             'BrigExecutableModifier' => {
                                           'standalone' => 'true',
                                           'wname' => 'ExecutableModifier',
                                           'align' => undef,
                                           'enum' => 'BRIG_KIND_EXECUTABLE_MODIFIER',
                                           'isroot' => 'true',
                                           'fields' => [
                                                         {
                                                           'wname' => 'allBits',
                                                           'defValue' => '0',
                                                           'type' => 'BrigExecutableModifier8_t',
                                                           'acc' => 'valRef',
                                                           'name' => 'allBits',
                                                           'wtype' => 'ValRef<uint8_t>'
                                                         },
                                                         {
                                                           'type' => 'bool',
                                                           'wname' => 'isDefinition',
                                                           'phantomof' => $structs->{'BrigExecutableModifier'}{'fields'}[0],
                                                           'acc' => 'bitValRef<0>',
                                                           'wtype' => 'BitValRef<0>',
                                                           'name' => 'isDefinition'
                                                         }
                                                       ],
                                           'name' => 'BrigExecutableModifier'
                                         },
             'BrigDirectiveFunction' => {
                                          'name' => 'BrigDirectiveFunction',
                                          'enum' => 'BRIG_KIND_DIRECTIVE_FUNCTION',
                                          'fields' => [
                                                        {
                                                          'acc' => 'valRef',
                                                          'wtype' => 'ValRef<uint16_t>',
                                                          'name' => 'byteCount',
                                                          'type' => 'uint16_t',
                                                          'wname' => 'byteCount'
                                                        },
                                                        {
                                                          'type' => 'BrigKinds16_t',
                                                          'wname' => 'kind',
                                                          'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                          'name' => 'kind',
                                                          'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                          'enum' => 'BrigKinds'
                                                        },
                                                        {
                                                          'wname' => 'name',
                                                          'defValue' => '0',
                                                          'type' => 'BrigDataOffsetString32_t',
                                                          'acc' => 'strRef',
                                                          'name' => 'name',
                                                          'wtype' => 'StrRef'
                                                        },
                                                        {
                                                          'type' => 'uint16_t',
                                                          'defValue' => '0',
                                                          'wname' => 'outArgCount',
                                                          'acc' => 'valRef',
                                                          'wtype' => 'ValRef<uint16_t>',
                                                          'name' => 'outArgCount'
                                                        },
                                                        {
                                                          'defValue' => '0',
                                                          'wname' => 'inArgCount',
                                                          'type' => 'uint16_t',
                                                          'name' => 'inArgCount',
                                                          'wtype' => 'ValRef<uint16_t>',
                                                          'acc' => 'valRef'
                                                        },
                                                        {
                                                          'acc' => 'itemRef<Code>',
                                                          'name' => 'firstInArg',
                                                          'wtype' => 'ItemRef<Code>',
                                                          'defValue' => '0',
                                                          'wname' => 'firstInArg',
                                                          'type' => 'BrigCodeOffset32_t'
                                                        },
                                                        {
                                                          'acc' => 'itemRef<Code>',
                                                          'name' => 'firstCodeBlockEntry',
                                                          'wtype' => 'ItemRef<Code>',
                                                          'defValue' => '0',
                                                          'wname' => 'firstCodeBlockEntry',
                                                          'type' => 'BrigCodeOffset32_t'
                                                        },
                                                        {
                                                          'defValue' => '0',
                                                          'wname' => 'nextModuleEntry',
                                                          'type' => 'BrigCodeOffset32_t',
                                                          'name' => 'nextModuleEntry',
                                                          'wtype' => 'ItemRef<Code>',
                                                          'acc' => 'itemRef<Code>'
                                                        },
                                                        {
                                                          'type' => 'uint32_t',
                                                          'defValue' => '0',
                                                          'wname' => 'codeBlockEntryCount',
                                                          'acc' => 'valRef',
                                                          'wtype' => 'ValRef<uint32_t>',
                                                          'name' => 'codeBlockEntryCount'
                                                        },
                                                        {
                                                          'wtype' => 'ExecutableModifier',
                                                          'name' => 'modifier',
                                                          'acc' => 'subItem<ExecutableModifier>',
                                                          'type' => 'BrigExecutableModifier',
                                                          'wname' => 'modifier'
                                                        },
                                                        {
                                                          'type' => 'BrigLinkage8_t',
                                                          'defValue' => 'Brig::BRIG_LINKAGE_NONE',
                                                          'wname' => 'linkage',
                                                          'wtype' => 'EnumValRef<Brig::BrigLinkage,uint8_t>',
                                                          'name' => 'linkage',
                                                          'enum' => 'BrigLinkage',
                                                          'acc' => 'enumValRef<Brig::BrigLinkage,uint8_t>'
                                                        },
                                                        {
                                                          'wname' => 'reserved',
                                                          'defValue' => '0',
                                                          'type' => 'uint16_t',
                                                          'acc' => 'valRef',
                                                          'name' => 'reserved',
                                                          'wtype' => 'ValRef<uint16_t>',
                                                          'skip' => 1
                                                        }
                                                      ],
                                          'align' => undef,
                                          'parent' => 'BrigDirectiveExecutable',
                                          'wname' => 'DirectiveFunction'
                                        },
             'BrigDirectiveIndirectFunction' => {
                                                  'align' => undef,
                                                  'parent' => 'BrigDirectiveExecutable',
                                                  'wname' => 'DirectiveIndirectFunction',
                                                  'name' => 'BrigDirectiveIndirectFunction',
                                                  'fields' => [
                                                                {
                                                                  'acc' => 'valRef',
                                                                  'wtype' => 'ValRef<uint16_t>',
                                                                  'name' => 'byteCount',
                                                                  'type' => 'uint16_t',
                                                                  'wname' => 'byteCount'
                                                                },
                                                                {
                                                                  'wname' => 'kind',
                                                                  'type' => 'BrigKinds16_t',
                                                                  'name' => 'kind',
                                                                  'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                                  'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                                  'enum' => 'BrigKinds'
                                                                },
                                                                {
                                                                  'wname' => 'name',
                                                                  'defValue' => '0',
                                                                  'type' => 'BrigDataOffsetString32_t',
                                                                  'acc' => 'strRef',
                                                                  'name' => 'name',
                                                                  'wtype' => 'StrRef'
                                                                },
                                                                {
                                                                  'type' => 'uint16_t',
                                                                  'defValue' => '0',
                                                                  'wname' => 'outArgCount',
                                                                  'acc' => 'valRef',
                                                                  'wtype' => 'ValRef<uint16_t>',
                                                                  'name' => 'outArgCount'
                                                                },
                                                                {
                                                                  'type' => 'uint16_t',
                                                                  'wname' => 'inArgCount',
                                                                  'defValue' => '0',
                                                                  'acc' => 'valRef',
                                                                  'wtype' => 'ValRef<uint16_t>',
                                                                  'name' => 'inArgCount'
                                                                },
                                                                {
                                                                  'type' => 'BrigCodeOffset32_t',
                                                                  'defValue' => '0',
                                                                  'wname' => 'firstInArg',
                                                                  'acc' => 'itemRef<Code>',
                                                                  'wtype' => 'ItemRef<Code>',
                                                                  'name' => 'firstInArg'
                                                                },
                                                                {
                                                                  'type' => 'BrigCodeOffset32_t',
                                                                  'wname' => 'firstCodeBlockEntry',
                                                                  'defValue' => '0',
                                                                  'wtype' => 'ItemRef<Code>',
                                                                  'name' => 'firstCodeBlockEntry',
                                                                  'acc' => 'itemRef<Code>'
                                                                },
                                                                {
                                                                  'type' => 'BrigCodeOffset32_t',
                                                                  'wname' => 'nextModuleEntry',
                                                                  'defValue' => '0',
                                                                  'acc' => 'itemRef<Code>',
                                                                  'wtype' => 'ItemRef<Code>',
                                                                  'name' => 'nextModuleEntry'
                                                                },
                                                                {
                                                                  'defValue' => '0',
                                                                  'wname' => 'codeBlockEntryCount',
                                                                  'type' => 'uint32_t',
                                                                  'acc' => 'valRef',
                                                                  'name' => 'codeBlockEntryCount',
                                                                  'wtype' => 'ValRef<uint32_t>'
                                                                },
                                                                {
                                                                  'acc' => 'subItem<ExecutableModifier>',
                                                                  'wtype' => 'ExecutableModifier',
                                                                  'name' => 'modifier',
                                                                  'type' => 'BrigExecutableModifier',
                                                                  'wname' => 'modifier'
                                                                },
                                                                {
                                                                  'type' => 'BrigLinkage8_t',
                                                                  'defValue' => 'Brig::BRIG_LINKAGE_NONE',
                                                                  'wname' => 'linkage',
                                                                  'wtype' => 'EnumValRef<Brig::BrigLinkage,uint8_t>',
                                                                  'name' => 'linkage',
                                                                  'acc' => 'enumValRef<Brig::BrigLinkage,uint8_t>',
                                                                  'enum' => 'BrigLinkage'
                                                                },
                                                                {
                                                                  'wname' => 'reserved',
                                                                  'defValue' => '0',
                                                                  'type' => 'uint16_t',
                                                                  'acc' => 'valRef',
                                                                  'name' => 'reserved',
                                                                  'wtype' => 'ValRef<uint16_t>',
                                                                  'skip' => 1
                                                                }
                                                              ],
                                                  'enum' => 'BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION'
                                                },
             'BrigOperandCodeRef' => {
                                       'align' => undef,
                                       'wname' => 'OperandCodeRef',
                                       'parent' => 'BrigOperand',
                                       'name' => 'BrigOperandCodeRef',
                                       'fields' => [
                                                     {
                                                       'wname' => 'byteCount',
                                                       'type' => 'uint16_t',
                                                       'name' => 'byteCount',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'acc' => 'valRef'
                                                     },
                                                     {
                                                       'name' => 'kind',
                                                       'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                       'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                       'enum' => 'BrigKinds',
                                                       'wname' => 'kind',
                                                       'type' => 'BrigKinds16_t'
                                                     },
                                                     {
                                                       'type' => 'BrigCodeOffset32_t',
                                                       'wname' => 'ref',
                                                       'defValue' => '0',
                                                       'wtype' => 'ItemRef<Code>',
                                                       'name' => 'ref',
                                                       'acc' => 'itemRef<Code>'
                                                     }
                                                   ],
                                       'enum' => 'BRIG_KIND_OPERAND_CODE_REF'
                                     },
             'BrigInstBr' => {
                               'fields' => [
                                             {
                                               'acc' => 'valRef',
                                               'wtype' => 'ValRef<uint16_t>',
                                               'name' => 'byteCount',
                                               'type' => 'uint16_t',
                                               'wname' => 'byteCount'
                                             },
                                             {
                                               'name' => 'kind',
                                               'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                               'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                               'enum' => 'BrigKinds',
                                               'wname' => 'kind',
                                               'type' => 'BrigKinds16_t'
                                             },
                                             {
                                               'name' => 'opcode',
                                               'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                               'enum' => 'BrigOpcode',
                                               'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                               'wname' => 'opcode',
                                               'type' => 'BrigOpcode16_t'
                                             },
                                             {
                                               'wtype' => 'ValRef<uint16_t>',
                                               'name' => 'type',
                                               'acc' => 'valRef',
                                               'type' => 'BrigType16_t',
                                               'wname' => 'type'
                                             },
                                             {
                                               'acc' => 'listRef<Operand>',
                                               'wtype' => 'ListRef<Operand>',
                                               'name' => 'operands',
                                               'type' => 'BrigDataOffsetOperandList32_t',
                                               'defValue' => '0',
                                               'wname' => 'operands'
                                             },
                                             {
                                               'wtype' => 'EnumValRef<Brig::BrigWidth,uint8_t>',
                                               'name' => 'width',
                                               'enum' => 'BrigWidth',
                                               'acc' => 'enumValRef<Brig::BrigWidth,uint8_t>',
                                               'type' => 'BrigWidth8_t',
                                               'wname' => 'width'
                                             },
                                             {
                                               'size' => 3,
                                               'skip' => 1,
                                               'type' => 'uint8_t',
                                               'defValue' => '0',
                                               'wname' => 'reserved',
                                               'acc' => 'valRef',
                                               'wtype' => 'ValRef<uint8_t>',
                                               'name' => 'reserved'
                                             }
                                           ],
                               'enum' => 'BRIG_KIND_INST_BR',
                               'name' => 'BrigInstBr',
                               'wname' => 'InstBr',
                               'parent' => 'BrigInst',
                               'align' => undef
                             },
             'BrigDirectiveArgBlockStart' => {
                                               'enum' => 'BRIG_KIND_DIRECTIVE_ARG_BLOCK_START',
                                               'fields' => [
                                                             {
                                                               'acc' => 'valRef',
                                                               'name' => 'byteCount',
                                                               'wtype' => 'ValRef<uint16_t>',
                                                               'wname' => 'byteCount',
                                                               'type' => 'uint16_t'
                                                             },
                                                             {
                                                               'name' => 'kind',
                                                               'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                               'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                               'enum' => 'BrigKinds',
                                                               'wname' => 'kind',
                                                               'type' => 'BrigKinds16_t'
                                                             }
                                                           ],
                                               'name' => 'BrigDirectiveArgBlockStart',
                                               'parent' => 'BrigDirective',
                                               'wname' => 'DirectiveArgBlockStart',
                                               'align' => undef
                                             },
             'BrigUInt64' => {
                               'isroot' => 'true',
                               'enum' => 'BRIG_KIND_U_INT64',
                               'fields' => [
                                             {
                                               'name' => 'lo',
                                               'wtype' => 'ValRef<uint32_t>',
                                               'acc' => 'valRef',
                                               'wname' => 'lo',
                                               'defValue' => '0',
                                               'type' => 'uint32_t'
                                             },
                                             {
                                               'wtype' => 'ValRef<uint32_t>',
                                               'hcode' => [
                                                            'KLASS& operator=(uint64_t rhs);',
                                                            'operator uint64_t();'
                                                          ],
                                               'name' => 'hi',
                                               'acc' => 'valRef',
                                               'implcode' => [
                                                               'inline KLASS& KLASS::operator=(uint64_t rhs) { lo() = (uint32_t)rhs; hi() = (uint32_t)(rhs >> 32); return *this; }',
                                                               'inline KLASS::operator uint64_t() { return ((uint64_t)hi()) << 32 | lo(); }'
                                                             ],
                                               'type' => 'uint32_t',
                                               'wname' => 'hi',
                                               'defValue' => '0'
                                             }
                                           ],
                               'name' => 'BrigUInt64',
                               'wname' => 'UInt64',
                               'standalone' => 'true',
                               'align' => undef
                             },
             'BrigInstCvt' => {
                                'enum' => 'BRIG_KIND_INST_CVT',
                                'fields' => [
                                              {
                                                'type' => 'uint16_t',
                                                'wname' => 'byteCount',
                                                'acc' => 'valRef',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'name' => 'byteCount'
                                              },
                                              {
                                                'enum' => 'BrigKinds',
                                                'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                'name' => 'kind',
                                                'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                'wname' => 'kind',
                                                'type' => 'BrigKinds16_t'
                                              },
                                              {
                                                'enum' => 'BrigOpcode',
                                                'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                'name' => 'opcode',
                                                'type' => 'BrigOpcode16_t',
                                                'wname' => 'opcode'
                                              },
                                              {
                                                'wname' => 'type',
                                                'type' => 'BrigType16_t',
                                                'name' => 'type',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'acc' => 'valRef'
                                              },
                                              {
                                                'acc' => 'listRef<Operand>',
                                                'name' => 'operands',
                                                'wtype' => 'ListRef<Operand>',
                                                'defValue' => '0',
                                                'wname' => 'operands',
                                                'type' => 'BrigDataOffsetOperandList32_t'
                                              },
                                              {
                                                'wtype' => 'ValRef<uint16_t>',
                                                'name' => 'sourceType',
                                                'acc' => 'valRef',
                                                'type' => 'BrigType16_t',
                                                'wname' => 'sourceType'
                                              },
                                              {
                                                'acc' => 'subItem<AluModifier>',
                                                'name' => 'modifier',
                                                'wtype' => 'AluModifier',
                                                'wname' => 'modifier',
                                                'type' => 'BrigAluModifier'
                                              }
                                            ],
                                'name' => 'BrigInstCvt',
                                'parent' => 'BrigInst',
                                'wname' => 'InstCvt',
                                'align' => undef
                              },
             'BrigDirectiveLoc' => {
                                     'align' => undef,
                                     'wname' => 'DirectiveLoc',
                                     'parent' => 'BrigDirective',
                                     'name' => 'BrigDirectiveLoc',
                                     'enum' => 'BRIG_KIND_DIRECTIVE_LOC',
                                     'fields' => [
                                                   {
                                                     'acc' => 'valRef',
                                                     'name' => 'byteCount',
                                                     'wtype' => 'ValRef<uint16_t>',
                                                     'wname' => 'byteCount',
                                                     'type' => 'uint16_t'
                                                   },
                                                   {
                                                     'name' => 'kind',
                                                     'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                     'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                     'enum' => 'BrigKinds',
                                                     'wname' => 'kind',
                                                     'type' => 'BrigKinds16_t'
                                                   },
                                                   {
                                                     'wname' => 'filename',
                                                     'defValue' => '0',
                                                     'type' => 'BrigDataOffsetString32_t',
                                                     'acc' => 'strRef',
                                                     'name' => 'filename',
                                                     'wtype' => 'StrRef'
                                                   },
                                                   {
                                                     'acc' => 'valRef',
                                                     'wtype' => 'ValRef<uint32_t>',
                                                     'name' => 'line',
                                                     'type' => 'uint32_t',
                                                     'wname' => 'line'
                                                   },
                                                   {
                                                     'wtype' => 'ValRef<uint32_t>',
                                                     'name' => 'column',
                                                     'acc' => 'valRef',
                                                     'type' => 'uint32_t',
                                                     'defValue' => '1',
                                                     'wname' => 'column'
                                                   }
                                                 ]
                                   },
             'BrigDirectiveKernel' => {
                                        'align' => undef,
                                        'wname' => 'DirectiveKernel',
                                        'parent' => 'BrigDirectiveExecutable',
                                        'name' => 'BrigDirectiveKernel',
                                        'enum' => 'BRIG_KIND_DIRECTIVE_KERNEL',
                                        'fields' => [
                                                      {
                                                        'acc' => 'valRef',
                                                        'name' => 'byteCount',
                                                        'wtype' => 'ValRef<uint16_t>',
                                                        'wname' => 'byteCount',
                                                        'type' => 'uint16_t'
                                                      },
                                                      {
                                                        'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                        'enum' => 'BrigKinds',
                                                        'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                        'name' => 'kind',
                                                        'type' => 'BrigKinds16_t',
                                                        'wname' => 'kind'
                                                      },
                                                      {
                                                        'defValue' => '0',
                                                        'wname' => 'name',
                                                        'type' => 'BrigDataOffsetString32_t',
                                                        'acc' => 'strRef',
                                                        'name' => 'name',
                                                        'wtype' => 'StrRef'
                                                      },
                                                      {
                                                        'type' => 'uint16_t',
                                                        'wname' => 'outArgCount',
                                                        'defValue' => '0',
                                                        'wtype' => 'ValRef<uint16_t>',
                                                        'name' => 'outArgCount',
                                                        'acc' => 'valRef'
                                                      },
                                                      {
                                                        'acc' => 'valRef',
                                                        'name' => 'inArgCount',
                                                        'wtype' => 'ValRef<uint16_t>',
                                                        'defValue' => '0',
                                                        'wname' => 'inArgCount',
                                                        'type' => 'uint16_t'
                                                      },
                                                      {
                                                        'type' => 'BrigCodeOffset32_t',
                                                        'defValue' => '0',
                                                        'wname' => 'firstInArg',
                                                        'acc' => 'itemRef<Code>',
                                                        'wtype' => 'ItemRef<Code>',
                                                        'name' => 'firstInArg'
                                                      },
                                                      {
                                                        'acc' => 'itemRef<Code>',
                                                        'wtype' => 'ItemRef<Code>',
                                                        'name' => 'firstCodeBlockEntry',
                                                        'type' => 'BrigCodeOffset32_t',
                                                        'defValue' => '0',
                                                        'wname' => 'firstCodeBlockEntry'
                                                      },
                                                      {
                                                        'wtype' => 'ItemRef<Code>',
                                                        'name' => 'nextModuleEntry',
                                                        'acc' => 'itemRef<Code>',
                                                        'type' => 'BrigCodeOffset32_t',
                                                        'wname' => 'nextModuleEntry',
                                                        'defValue' => '0'
                                                      },
                                                      {
                                                        'wname' => 'codeBlockEntryCount',
                                                        'defValue' => '0',
                                                        'type' => 'uint32_t',
                                                        'name' => 'codeBlockEntryCount',
                                                        'wtype' => 'ValRef<uint32_t>',
                                                        'acc' => 'valRef'
                                                      },
                                                      {
                                                        'acc' => 'subItem<ExecutableModifier>',
                                                        'name' => 'modifier',
                                                        'wtype' => 'ExecutableModifier',
                                                        'wname' => 'modifier',
                                                        'type' => 'BrigExecutableModifier'
                                                      },
                                                      {
                                                        'wname' => 'linkage',
                                                        'defValue' => 'Brig::BRIG_LINKAGE_NONE',
                                                        'type' => 'BrigLinkage8_t',
                                                        'name' => 'linkage',
                                                        'wtype' => 'EnumValRef<Brig::BrigLinkage,uint8_t>',
                                                        'enum' => 'BrigLinkage',
                                                        'acc' => 'enumValRef<Brig::BrigLinkage,uint8_t>'
                                                      },
                                                      {
                                                        'type' => 'uint16_t',
                                                        'defValue' => '0',
                                                        'wname' => 'reserved',
                                                        'acc' => 'valRef',
                                                        'wtype' => 'ValRef<uint16_t>',
                                                        'skip' => 1,
                                                        'name' => 'reserved'
                                                      }
                                                    ]
                                      },
             'BrigInst' => {
                             'fields' => [
                                           {
                                             'type' => 'uint16_t',
                                             'wname' => 'byteCount',
                                             'acc' => 'valRef',
                                             'wtype' => 'ValRef<uint16_t>',
                                             'name' => 'byteCount'
                                           },
                                           {
                                             'wname' => 'kind',
                                             'type' => 'BrigKinds16_t',
                                             'name' => 'kind',
                                             'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                             'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                             'enum' => 'BrigKinds'
                                           },
                                           {
                                             'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                             'enum' => 'BrigOpcode',
                                             'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                             'name' => 'opcode',
                                             'type' => 'BrigOpcode16_t',
                                             'wname' => 'opcode'
                                           },
                                           {
                                             'acc' => 'valRef',
                                             'name' => 'type',
                                             'wtype' => 'ValRef<uint16_t>',
                                             'wname' => 'type',
                                             'type' => 'BrigType16_t'
                                           },
                                           {
                                             'defValue' => '0',
                                             'wname' => 'operands',
                                             'type' => 'BrigDataOffsetOperandList32_t',
                                             'hcode' => [
                                                          'Operand operand(int index);'
                                                        ],
                                             'name' => 'operands',
                                             'wtype' => 'ListRef<Operand>',
                                             'acc' => 'listRef<Operand>',
                                             'implcode' => [
                                                             'inline Operand KLASS::operand(int index) { return operands()[index]; }'
                                                           ]
                                           }
                                         ],
                             'enum' => 'BRIG_KIND_INST',
                             'align' => undef,
                             'parent' => 'BrigCode',
                             'generic' => 'true',
                             'name' => 'BrigInst',
                             'children' => [
                                             'BrigInstQueue',
                                             'BrigInstImage',
                                             'BrigInstMem',
                                             'BrigInstQueryImage',
                                             'BrigInstLane',
                                             'BrigInstQuerySampler',
                                             'BrigInstBasic',
                                             'BrigInstSourceType',
                                             'BrigInstSignal',
                                             'BrigInstMemFence',
                                             'BrigInstSeg',
                                             'BrigInstBr',
                                             'BrigInstCvt',
                                             'BrigInstCmp',
                                             'BrigInstSegCvt',
                                             'BrigInstAddr',
                                             'BrigInstMod',
                                             'BrigInstAtomic'
                                           ],
                             'wname' => 'Inst'
                           },
             'BrigMemoryModifier' => {
                                       'align' => undef,
                                       'wname' => 'MemoryModifier',
                                       'standalone' => 'true',
                                       'name' => 'BrigMemoryModifier',
                                       'isroot' => 'true',
                                       'enum' => 'BRIG_KIND_MEMORY_MODIFIER',
                                       'fields' => [
                                                     {
                                                       'defValue' => '0',
                                                       'wname' => 'allBits',
                                                       'type' => 'BrigMemoryModifier8_t',
                                                       'acc' => 'valRef',
                                                       'name' => 'allBits',
                                                       'wtype' => 'ValRef<uint8_t>'
                                                     },
                                                     {
                                                       'phantomof' => $structs->{'BrigMemoryModifier'}{'fields'}[0],
                                                       'type' => 'bool',
                                                       'wname' => 'isConst',
                                                       'wtype' => 'BitValRef<0>',
                                                       'name' => 'isConst',
                                                       'acc' => 'bitValRef<0>'
                                                     }
                                                   ]
                                     },
             'BrigInstAddr' => {
                                 'fields' => [
                                               {
                                                 'name' => 'byteCount',
                                                 'wtype' => 'ValRef<uint16_t>',
                                                 'acc' => 'valRef',
                                                 'wname' => 'byteCount',
                                                 'type' => 'uint16_t'
                                               },
                                               {
                                                 'wname' => 'kind',
                                                 'type' => 'BrigKinds16_t',
                                                 'name' => 'kind',
                                                 'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                 'enum' => 'BrigKinds',
                                                 'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>'
                                               },
                                               {
                                                 'type' => 'BrigOpcode16_t',
                                                 'wname' => 'opcode',
                                                 'enum' => 'BrigOpcode',
                                                 'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                 'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                 'name' => 'opcode'
                                               },
                                               {
                                                 'wname' => 'type',
                                                 'type' => 'BrigType16_t',
                                                 'name' => 'type',
                                                 'wtype' => 'ValRef<uint16_t>',
                                                 'acc' => 'valRef'
                                               },
                                               {
                                                 'defValue' => '0',
                                                 'wname' => 'operands',
                                                 'type' => 'BrigDataOffsetOperandList32_t',
                                                 'acc' => 'listRef<Operand>',
                                                 'name' => 'operands',
                                                 'wtype' => 'ListRef<Operand>'
                                               },
                                               {
                                                 'enum' => 'BrigSegment',
                                                 'acc' => 'enumValRef<Brig::BrigSegment,uint8_t>',
                                                 'wtype' => 'EnumValRef<Brig::BrigSegment,uint8_t>',
                                                 'name' => 'segment',
                                                 'type' => 'BrigSegment8_t',
                                                 'defValue' => 'Brig::BRIG_SEGMENT_NONE',
                                                 'wname' => 'segment'
                                               },
                                               {
                                                 'size' => 3,
                                                 'skip' => 1,
                                                 'type' => 'uint8_t',
                                                 'defValue' => '0',
                                                 'wname' => 'reserved',
                                                 'acc' => 'valRef',
                                                 'wtype' => 'ValRef<uint8_t>',
                                                 'name' => 'reserved'
                                               }
                                             ],
                                 'enum' => 'BRIG_KIND_INST_ADDR',
                                 'name' => 'BrigInstAddr',
                                 'wname' => 'InstAddr',
                                 'parent' => 'BrigInst',
                                 'align' => undef
                               },
             'BrigOperandWavesize' => {
                                        'align' => undef,
                                        'wname' => 'OperandWavesize',
                                        'parent' => 'BrigOperand',
                                        'name' => 'BrigOperandWavesize',
                                        'fields' => [
                                                      {
                                                        'type' => 'uint16_t',
                                                        'wname' => 'byteCount',
                                                        'acc' => 'valRef',
                                                        'wtype' => 'ValRef<uint16_t>',
                                                        'name' => 'byteCount'
                                                      },
                                                      {
                                                        'name' => 'kind',
                                                        'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                        'enum' => 'BrigKinds',
                                                        'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                        'wname' => 'kind',
                                                        'type' => 'BrigKinds16_t'
                                                      }
                                                    ],
                                        'enum' => 'BRIG_KIND_OPERAND_WAVESIZE'
                                      },
             'BrigAluModifier' => {
                                    'wname' => 'AluModifier',
                                    'standalone' => 'true',
                                    'align' => undef,
                                    'fields' => [
                                                  {
                                                    'wtype' => 'ValRef<uint16_t>',
                                                    'name' => 'allBits',
                                                    'acc' => 'valRef',
                                                    'type' => 'BrigAluModifier16_t',
                                                    'wname' => 'allBits',
                                                    'defValue' => '0'
                                                  },
                                                  {
                                                    'acc' => 'bFValRef<Brig::BrigRound8_t,0,5>',
                                                    'enum' => 'BrigRound',
                                                    'name' => 'round',
                                                    'wtype' => 'BFValRef<Brig::BrigRound8_t,0,5>',
                                                    'wname' => 'round',
                                                    'type' => 'BrigRound8_t',
                                                    'phantomof' => $structs->{'BrigAluModifier'}{'fields'}[0]
                                                  },
                                                  {
                                                    'type' => 'bool',
                                                    'wname' => 'ftz',
                                                    'phantomof' => $structs->{'BrigAluModifier'}{'fields'}[0],
                                                    'acc' => 'bitValRef<5>',
                                                    'wtype' => 'BitValRef<5>',
                                                    'name' => 'ftz'
                                                  }
                                                ],
                                    'enum' => 'BRIG_KIND_ALU_MODIFIER',
                                    'isroot' => 'true',
                                    'name' => 'BrigAluModifier'
                                  },
             'BrigDirectivePragma' => {
                                        'align' => undef,
                                        'parent' => 'BrigDirective',
                                        'wname' => 'DirectivePragma',
                                        'name' => 'BrigDirectivePragma',
                                        'enum' => 'BRIG_KIND_DIRECTIVE_PRAGMA',
                                        'fields' => [
                                                      {
                                                        'acc' => 'valRef',
                                                        'wtype' => 'ValRef<uint16_t>',
                                                        'name' => 'byteCount',
                                                        'type' => 'uint16_t',
                                                        'wname' => 'byteCount'
                                                      },
                                                      {
                                                        'type' => 'BrigKinds16_t',
                                                        'wname' => 'kind',
                                                        'acc' => 'enumValRef<Brig::BrigKinds,uint16_t>',
                                                        'enum' => 'BrigKinds',
                                                        'wtype' => 'EnumValRef<Brig::BrigKinds,uint16_t>',
                                                        'name' => 'kind'
                                                      },
                                                      {
                                                        'wtype' => 'ListRef<Operand>',
                                                        'name' => 'operands',
                                                        'acc' => 'listRef<Operand>',
                                                        'type' => 'BrigDataOffsetOperandList32_t',
                                                        'defValue' => '0',
                                                        'wname' => 'operands'
                                                      }
                                                    ]
                                      },
             'BrigBase' => {
                             'name' => 'BrigBase',
                             'fields' => [
                                           {
                                             'name' => 'byteCount',
                                             'type' => 'uint16_t'
                                           },
                                           {
                                             'name' => 'kind',
                                             'type' => 'BrigKinds16_t'
                                           }
                                         ],
                             'nowrap' => 'true',
                             'align' => undef
                           }
           };
