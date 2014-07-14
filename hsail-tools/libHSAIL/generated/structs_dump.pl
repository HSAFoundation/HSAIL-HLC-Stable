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
             'BrigDirectiveBase' => {
                                      'name' => 'BrigDirectiveBase',
                                      'align' => undef,
                                      'fields' => [
                                                    {
                                                      'type' => 'uint16_t',
                                                      'name' => 'size'
                                                    },
                                                    {
                                                      'name' => 'kind',
                                                      'type' => 'BrigDirectiveKinds16_t'
                                                    },
                                                    {
                                                      'type' => 'BrigCodeOffset32_t',
                                                      'name' => 'code'
                                                    }
                                                  ],
                                      'nowrap' => 'true'
                                    },
             'BrigDirectiveExtension' => {
                                           'comments' => [
                                                           '/// @}',
                                                           '/// extension directive.'
                                                         ],
                                           'parent' => 'BrigDirectiveCode',
                                           'enum' => 'BRIG_DIRECTIVE_EXTENSION',
                                           'wname' => 'DirectiveExtension',
                                           'fields' => [
                                                         {
                                                           'wname' => 'size',
                                                           'type' => 'uint16_t',
                                                           'name' => 'size',
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'acc' => 'valRef'
                                                         },
                                                         {
                                                           'enum' => 'BrigDirectiveKinds',
                                                           'wname' => 'kind',
                                                           'type' => 'BrigDirectiveKinds16_t',
                                                           'name' => 'kind',
                                                           'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                           'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                         },
                                                         {
                                                           'acc' => 'itemRef<Inst>',
                                                           'wtype' => 'ItemRef<Inst>',
                                                           'name' => 'code',
                                                           'defValue' => '0',
                                                           'type' => 'BrigCodeOffset32_t',
                                                           'wname' => 'code'
                                                         },
                                                         {
                                                           'wname' => 'name',
                                                           'defValue' => '0',
                                                           'type' => 'BrigStringOffset32_t',
                                                           'name' => 'name',
                                                           'wtype' => 'StrRef',
                                                           'acc' => 'strRef'
                                                         }
                                                       ],
                                           'align' => undef,
                                           'name' => 'BrigDirectiveExtension'
                                         },
             'BrigInstCmp' => {
                                'align' => undef,
                                'name' => 'BrigInstCmp',
                                'enum' => 'BRIG_INST_CMP',
                                'fields' => [
                                              {
                                                'wtype' => 'ValRef<uint16_t>',
                                                'acc' => 'valRef',
                                                'type' => 'uint16_t',
                                                'name' => 'size',
                                                'wname' => 'size'
                                              },
                                              {
                                                'type' => 'BrigInstKinds16_t',
                                                'name' => 'kind',
                                                'enum' => 'BrigInstKinds',
                                                'wname' => 'kind',
                                                'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>',
                                                'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>'
                                              },
                                              {
                                                'type' => 'BrigOpcode16_t',
                                                'name' => 'opcode',
                                                'enum' => 'BrigOpcode',
                                                'wname' => 'opcode',
                                                'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>'
                                              },
                                              {
                                                'type' => 'BrigType16_t',
                                                'name' => 'type',
                                                'wname' => 'type',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'acc' => 'valRef'
                                              },
                                              {
                                                'wname' => 'operand',
                                                'type' => 'BrigOperandOffset32_t',
                                                'defValue' => '0',
                                                'name' => 'operands',
                                                'wtype' => 'ItemRef<Operand>',
                                                'size' => '5',
                                                'acc' => 'itemRef<Operand>'
                                              },
                                              {
                                                'wtype' => 'ValRef<uint16_t>',
                                                'acc' => 'valRef',
                                                'wname' => 'sourceType',
                                                'type' => 'BrigType16_t',
                                                'name' => 'sourceType'
                                              },
                                              {
                                                'wname' => 'modifier',
                                                'name' => 'modifier',
                                                'type' => 'BrigAluModifier',
                                                'acc' => 'subItem<AluModifier>',
                                                'wtype' => 'AluModifier'
                                              },
                                              {
                                                'wtype' => 'EnumValRef<Brig::BrigCompareOperation,uint8_t>',
                                                'acc' => 'enumValRef<Brig::BrigCompareOperation,uint8_t>',
                                                'enum' => 'BrigCompareOperation',
                                                'wname' => 'compare',
                                                'type' => 'BrigCompareOperation8_t',
                                                'name' => 'compare'
                                              },
                                              {
                                                'enum' => 'BrigPack',
                                                'wname' => 'pack',
                                                'defValue' => 'Brig::BRIG_PACK_NONE',
                                                'type' => 'BrigPack8_t',
                                                'name' => 'pack',
                                                'wtype' => 'EnumValRef<Brig::BrigPack,uint8_t>',
                                                'acc' => 'enumValRef<Brig::BrigPack,uint8_t>'
                                              },
                                              {
                                                'wname' => 'reserved',
                                                'type' => 'uint16_t',
                                                'defValue' => '0',
                                                'name' => 'reserved',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'skip' => 1,
                                                'acc' => 'valRef'
                                              }
                                            ],
                                'wname' => 'InstCmp',
                                'parent' => 'BrigInst'
                              },
             'BrigOperandImmed' => {
                                     'parent' => 'BrigOperand',
                                     'align' => undef,
                                     'name' => 'BrigOperandImmed',
                                     'wname' => 'OperandImmed',
                                     'fields' => [
                                                   {
                                                     'name' => 'size',
                                                     'type' => 'uint16_t',
                                                     'wname' => 'size',
                                                     'acc' => 'valRef',
                                                     'wtype' => 'ValRef<uint16_t>'
                                                   },
                                                   {
                                                     'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                     'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                     'enum' => 'BrigOperandKinds',
                                                     'wname' => 'kind',
                                                     'type' => 'BrigOperandKinds16_t',
                                                     'name' => 'kind'
                                                   },
                                                   {
                                                     'type' => 'uint16_t',
                                                     'defValue' => '0',
                                                     'name' => 'reserved',
                                                     'wname' => 'reserved',
                                                     'wtype' => 'ValRef<uint16_t>',
                                                     'acc' => 'valRef',
                                                     'skip' => 1
                                                   },
                                                   {
                                                     'acc' => 'valRef',
                                                     'wtype' => 'ValRef<uint16_t>',
                                                     'name' => 'byteCount',
                                                     'type' => 'uint16_t',
                                                     'wname' => 'byteCount'
                                                   },
                                                   {
                                                     'name' => 'bytes',
                                                     'wname' => 'bytes',
                                                     'acc' => 'valRef',
                                                     'noaligncheck' => 'true',
                                                     'type' => 'uint8_t',
                                                     'novisit' => 'true',
                                                     'size' => 1,
                                                     'wtype' => 'ValRef<uint8_t>'
                                                   }
                                                 ],
                                     'enum' => 'BRIG_OPERAND_IMMED'
                                   },
             'BrigInstCvt' => {
                                'fields' => [
                                              {
                                                'acc' => 'valRef',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'wname' => 'size',
                                                'name' => 'size',
                                                'type' => 'uint16_t'
                                              },
                                              {
                                                'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>',
                                                'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                'enum' => 'BrigInstKinds',
                                                'wname' => 'kind',
                                                'type' => 'BrigInstKinds16_t',
                                                'name' => 'kind'
                                              },
                                              {
                                                'type' => 'BrigOpcode16_t',
                                                'name' => 'opcode',
                                                'enum' => 'BrigOpcode',
                                                'wname' => 'opcode',
                                                'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>'
                                              },
                                              {
                                                'type' => 'BrigType16_t',
                                                'name' => 'type',
                                                'wname' => 'type',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'acc' => 'valRef'
                                              },
                                              {
                                                'wtype' => 'ItemRef<Operand>',
                                                'size' => '5',
                                                'acc' => 'itemRef<Operand>',
                                                'wname' => 'operand',
                                                'defValue' => '0',
                                                'type' => 'BrigOperandOffset32_t',
                                                'name' => 'operands'
                                              },
                                              {
                                                'acc' => 'valRef',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'name' => 'sourceType',
                                                'type' => 'BrigType16_t',
                                                'wname' => 'sourceType'
                                              },
                                              {
                                                'wtype' => 'AluModifier',
                                                'acc' => 'subItem<AluModifier>',
                                                'wname' => 'modifier',
                                                'type' => 'BrigAluModifier',
                                                'name' => 'modifier'
                                              }
                                            ],
                                'wname' => 'InstCvt',
                                'enum' => 'BRIG_INST_CVT',
                                'name' => 'BrigInstCvt',
                                'align' => undef,
                                'parent' => 'BrigInst'
                              },
             'BrigOperandFunctionList' => {
                                            'comments' => [
                                                            '/// list of arguments. (in, out function arguments).'
                                                          ],
                                            'parent' => 'BrigOperandList',
                                            'fields' => [
                                                          {
                                                            'acc' => 'valRef',
                                                            'wtype' => 'ValRef<uint16_t>',
                                                            'name' => 'size',
                                                            'type' => 'uint16_t',
                                                            'wname' => 'size'
                                                          },
                                                          {
                                                            'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                            'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                            'name' => 'kind',
                                                            'type' => 'BrigOperandKinds16_t',
                                                            'wname' => 'kind',
                                                            'enum' => 'BrigOperandKinds'
                                                          },
                                                          {
                                                            'name' => 'reserved',
                                                            'type' => 'uint16_t',
                                                            'defValue' => '0',
                                                            'wname' => 'reserved',
                                                            'skip' => 1,
                                                            'acc' => 'valRef',
                                                            'wtype' => 'ValRef<uint16_t>'
                                                          },
                                                          {
                                                            'wname' => 'elementCount',
                                                            'type' => 'uint16_t',
                                                            'defValue' => '0',
                                                            'name' => 'elementCount',
                                                            'wtype' => 'ValRef<uint16_t>',
                                                            'acc' => 'valRef'
                                                          },
                                                          {
                                                            'wname' => 'elements',
                                                            'name' => 'elements',
                                                            'defValue' => '0',
                                                            'acc' => 'itemRef<Directive>',
                                                            'wspecial' => 'FunctionRefList',
                                                            'type' => 'BrigDirectiveOffset32_t',
                                                            'size' => '1',
                                                            'wtype' => 'ItemRef<Directive>'
                                                          }
                                                        ],
                                            'wname' => 'OperandFunctionList',
                                            'enum' => 'BRIG_OPERAND_FUNCTION_LIST',
                                            'name' => 'BrigOperandFunctionList',
                                            'align' => undef
                                          },
             'BrigBlockNumeric' => {
                                     'parent' => 'BrigDirective',
                                     'comments' => [
                                                     '/// numeric data inside block.'
                                                   ],
                                     'align' => undef,
                                     'name' => 'BrigBlockNumeric',
                                     'wname' => 'BlockNumeric',
                                     'fields' => [
                                                   {
                                                     'acc' => 'valRef',
                                                     'wtype' => 'ValRef<uint16_t>',
                                                     'wname' => 'size',
                                                     'name' => 'size',
                                                     'type' => 'uint16_t'
                                                   },
                                                   {
                                                     'wname' => 'kind',
                                                     'enum' => 'BrigDirectiveKinds',
                                                     'name' => 'kind',
                                                     'type' => 'BrigDirectiveKinds16_t',
                                                     'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                     'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                   },
                                                   {
                                                     'wtype' => 'ValRef<uint16_t>',
                                                     'acc' => 'valRef',
                                                     'wname' => 'type',
                                                     'type' => 'BrigType16_t',
                                                     'name' => 'type'
                                                   },
                                                   {
                                                     'skip' => 1,
                                                     'acc' => 'valRef',
                                                     'wtype' => 'ValRef<uint16_t>',
                                                     'name' => 'reserved',
                                                     'defValue' => '0',
                                                     'type' => 'uint16_t',
                                                     'wname' => 'reserved'
                                                   },
                                                   {
                                                     'name' => 'elementCount',
                                                     'defValue' => '0',
                                                     'type' => 'uint32_t',
                                                     'wname' => 'elementCount',
                                                     'acc' => 'valRef',
                                                     'wtype' => 'ValRef<uint32_t>'
                                                   },
                                                   {
                                                     'acc' => 'dataItemRef',
                                                     'wtype' => 'DataItemRef',
                                                     'novisit' => 'true',
                                                     'wname' => 'data',
                                                     'name' => 'data',
                                                     'type' => 'BrigDataOffset32_t'
                                                   },
                                                   {
                                                     'wtype' => 'ValRef<uint32_t>',
                                                     'novisit' => 'true',
                                                     'wspecialgeneric' => 'true',
                                                     'wspecial' => 'DataItemRefT',
                                                     'phantomof' => $structs->{'BrigBlockNumeric'}{'fields'}[5],
                                                     'type' => 'BrigDataOffset32_t',
                                                     'acc' => 'valRef',
                                                     'wname' => 'dataAs',
                                                     'name' => 'dataAs'
                                                   }
                                                 ],
                                     'enum' => 'BRIG_DIRECTIVE_BLOCK_NUMERIC'
                                   },
             'BrigOperandLabelRef' => {
                                        'name' => 'BrigOperandLabelRef',
                                        'align' => undef,
                                        'wname' => 'OperandLabelRef',
                                        'fields' => [
                                                      {
                                                        'wtype' => 'ValRef<uint16_t>',
                                                        'acc' => 'valRef',
                                                        'wname' => 'size',
                                                        'type' => 'uint16_t',
                                                        'name' => 'size'
                                                      },
                                                      {
                                                        'enum' => 'BrigOperandKinds',
                                                        'wname' => 'kind',
                                                        'type' => 'BrigOperandKinds16_t',
                                                        'name' => 'kind',
                                                        'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                        'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>'
                                                      },
                                                      {
                                                        'name' => 'label',
                                                        'type' => 'BrigDirectiveOffset32_t',
                                                        'defValue' => '0',
                                                        'wname' => 'label',
                                                        'acc' => 'itemRef<DirectiveLabel>',
                                                        'wtype' => 'ItemRef<DirectiveLabel>'
                                                      }
                                                    ],
                                        'enum' => 'BRIG_OPERAND_LABEL_REF',
                                        'parent' => 'BrigOperandRef'
                                      },
             'BrigDirectiveSignatureArgument' => {
                                                   'enum' => 'BRIG_DIRECTIVE_SIGNATURE_ARGUMENT',
                                                   'wname' => 'DirectiveSignatureArgument',
                                                   'standalone' => 'true',
                                                   'name' => 'BrigDirectiveSignatureArgument',
                                                   'comments' => [
                                                                   '/// element describing properties of function signature argument.'
                                                                 ],
                                                   'fields' => [
                                                                 {
                                                                   'wname' => 'type',
                                                                   'name' => 'type',
                                                                   'type' => 'BrigType16_t',
                                                                   'acc' => 'valRef',
                                                                   'wtype' => 'ValRef<uint16_t>'
                                                                 },
                                                                 {
                                                                   'wname' => 'align',
                                                                   'enum' => 'BrigAlignment',
                                                                   'name' => 'align',
                                                                   'type' => 'BrigAlignment8_t',
                                                                   'acc' => 'enumValRef<Brig::BrigAlignment,uint8_t>',
                                                                   'wtype' => 'EnumValRef<Brig::BrigAlignment,uint8_t>'
                                                                 },
                                                                 {
                                                                   'acc' => 'subItem<SymbolModifier>',
                                                                   'wtype' => 'SymbolModifier',
                                                                   'wname' => 'modifier',
                                                                   'name' => 'modifier',
                                                                   'type' => 'BrigSymbolModifier'
                                                                 },
                                                                 {
                                                                   'name' => 'dimLo',
                                                                   'type' => 'uint32_t',
                                                                   'wname' => 'dimLo',
                                                                   'acc' => 'valRef',
                                                                   'wtype' => 'ValRef<uint32_t>'
                                                                 },
                                                                 {
                                                                   'type' => 'uint64_t',
                                                                   'name' => 'dim',
                                                                   'phantomof' => $structs->{'BrigDirectiveSignatureArgument'}{'fields'}[3],
                                                                   'wname' => 'dim',
                                                                   'wtype' => 'ValRef<uint64_t>',
                                                                   'acc' => 'reinterpretValRef<uint64_t>'
                                                                 },
                                                                 {
                                                                   'wtype' => 'ValRef<uint32_t>',
                                                                   'acc' => 'valRef',
                                                                   'wname' => 'dimHi',
                                                                   'type' => 'uint32_t',
                                                                   'name' => 'dimHi'
                                                                 }
                                                               ],
                                                   'isroot' => 'true',
                                                   'align' => undef
                                                 },
             'BrigDirectiveFbarrier' => {
                                          'parent' => 'BrigDirectiveCode',
                                          'enum' => 'BRIG_DIRECTIVE_FBARRIER',
                                          'fields' => [
                                                        {
                                                          'acc' => 'valRef',
                                                          'wtype' => 'ValRef<uint16_t>',
                                                          'wname' => 'size',
                                                          'name' => 'size',
                                                          'type' => 'uint16_t'
                                                        },
                                                        {
                                                          'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                          'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                          'name' => 'kind',
                                                          'type' => 'BrigDirectiveKinds16_t',
                                                          'wname' => 'kind',
                                                          'enum' => 'BrigDirectiveKinds'
                                                        },
                                                        {
                                                          'acc' => 'itemRef<Inst>',
                                                          'wtype' => 'ItemRef<Inst>',
                                                          'name' => 'code',
                                                          'defValue' => '0',
                                                          'type' => 'BrigCodeOffset32_t',
                                                          'wname' => 'code'
                                                        },
                                                        {
                                                          'wname' => 'name',
                                                          'name' => 'name',
                                                          'type' => 'BrigStringOffset32_t',
                                                          'defValue' => '0',
                                                          'acc' => 'strRef',
                                                          'wtype' => 'StrRef'
                                                        }
                                                      ],
                                          'wname' => 'DirectiveFbarrier',
                                          'align' => undef,
                                          'name' => 'BrigDirectiveFbarrier'
                                        },
             'BrigOperandAddress' => {
                                       'name' => 'BrigOperandAddress',
                                       'align' => undef,
                                       'enum' => 'BRIG_OPERAND_ADDRESS',
                                       'fields' => [
                                                     {
                                                       'type' => 'uint16_t',
                                                       'name' => 'size',
                                                       'wname' => 'size',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'acc' => 'valRef'
                                                     },
                                                     {
                                                       'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                       'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                       'name' => 'kind',
                                                       'type' => 'BrigOperandKinds16_t',
                                                       'wname' => 'kind',
                                                       'enum' => 'BrigOperandKinds'
                                                     },
                                                     {
                                                       'type' => 'BrigDirectiveOffset32_t',
                                                       'defValue' => '0',
                                                       'name' => 'symbol',
                                                       'wname' => 'symbol',
                                                       'wtype' => 'ItemRef<DirectiveVariable>',
                                                       'acc' => 'itemRef<DirectiveVariable>'
                                                     },
                                                     {
                                                       'wname' => 'reg',
                                                       'defValue' => '0',
                                                       'type' => 'BrigStringOffset32_t',
                                                       'name' => 'reg',
                                                       'wtype' => 'StrRef',
                                                       'acc' => 'strRef'
                                                     },
                                                     {
                                                       'wname' => 'offsetLo',
                                                       'type' => 'uint32_t',
                                                       'name' => 'offsetLo',
                                                       'wtype' => 'ValRef<uint32_t>',
                                                       'acc' => 'valRef'
                                                     },
                                                     {
                                                       'acc' => 'reinterpretValRef<uint64_t>',
                                                       'wtype' => 'ValRef<uint64_t>',
                                                       'name' => 'offset',
                                                       'type' => 'uint64_t',
                                                       'wname' => 'offset',
                                                       'phantomof' => $structs->{'BrigOperandAddress'}{'fields'}[4]
                                                     },
                                                     {
                                                       'wtype' => 'ValRef<uint32_t>',
                                                       'acc' => 'valRef',
                                                       'wname' => 'offsetHi',
                                                       'type' => 'uint32_t',
                                                       'name' => 'offsetHi'
                                                     }
                                                   ],
                                       'wname' => 'OperandAddress',
                                       'parent' => 'BrigOperand'
                                     },
             'BrigDirective' => {
                                  'comments' => [
                                                  '/// @addtogroup Directives',
                                                  '/// @{',
                                                  '/// base class for all directive items.'
                                                ],
                                  'enum' => 'BRIG_DIRECTIVE',
                                  'wname' => 'Directive',
                                  'children' => [
                                                  'BrigDirectiveSamplerInit',
                                                  'BrigDirectiveLabelInit',
                                                  'BrigDirectiveVariableInit',
                                                  'BrigDirectiveImageInit',
                                                  'BrigDirectiveLoc',
                                                  'BrigDirectiveOpaqueInit',
                                                  'BrigDirectiveSignature',
                                                  'BrigDirectiveExecutable',
                                                  'BrigDirectiveFunction',
                                                  'BrigDirectiveArgScopeStart',
                                                  'BrigDirectiveArgScopeEnd',
                                                  'BrigDirectiveCallableBase',
                                                  'BrigBlockEnd',
                                                  'BrigDirectiveVersion',
                                                  'BrigDirectiveSamplerProperties',
                                                  'BrigDirectiveLabel',
                                                  'BrigBlockString',
                                                  'BrigBlockNumeric',
                                                  'BrigDirectiveLabelTargets',
                                                  'BrigDirectiveControl',
                                                  'BrigDirectiveExtension',
                                                  'BrigDirectiveVariable',
                                                  'BrigDirectiveComment',
                                                  'BrigBlockStart',
                                                  'BrigDirectivePragma',
                                                  'BrigDirectiveFbarrier',
                                                  'BrigDirectiveImageProperties',
                                                  'BrigDirectiveKernel',
                                                  'BrigDirectiveCode'
                                                ],
                                  'name' => 'BrigDirective',
                                  'generic' => 'true',
                                  'fields' => [
                                                {
                                                  'acc' => 'valRef',
                                                  'comments' => [
                                                                  '/// item size.'
                                                                ],
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'name' => 'size',
                                                  'type' => 'uint16_t',
                                                  'wname' => 'size'
                                                },
                                                {
                                                  'enum' => 'BrigDirectiveKinds',
                                                  'wname' => 'kind',
                                                  'type' => 'BrigDirectiveKinds16_t',
                                                  'name' => 'kind',
                                                  'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                  'comments' => [
                                                                  '/// item kind. One of BrigDirectiveKinds enum values.'
                                                                ],
                                                  'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                }
                                              ],
                                  'align' => undef,
                                  'isroot' => 'true'
                                },
             'BrigDirectiveSignature' => {
                                           'wname' => 'DirectiveSignature',
                                           'fields' => [
                                                         {
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'acc' => 'valRef',
                                                           'wname' => 'size',
                                                           'type' => 'uint16_t',
                                                           'name' => 'size'
                                                         },
                                                         {
                                                           'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                           'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                           'enum' => 'BrigDirectiveKinds',
                                                           'wname' => 'kind',
                                                           'type' => 'BrigDirectiveKinds16_t',
                                                           'name' => 'kind'
                                                         },
                                                         {
                                                           'wtype' => 'ItemRef<Inst>',
                                                           'acc' => 'itemRef<Inst>',
                                                           'wname' => 'code',
                                                           'type' => 'BrigCodeOffset32_t',
                                                           'defValue' => '0',
                                                           'name' => 'code'
                                                         },
                                                         {
                                                           'wtype' => 'StrRef',
                                                           'acc' => 'strRef',
                                                           'type' => 'BrigStringOffset32_t',
                                                           'defValue' => '0',
                                                           'name' => 'name',
                                                           'wname' => 'name'
                                                         },
                                                         {
                                                           'wname' => 'inCount',
                                                           'type' => 'uint16_t',
                                                           'defValue' => '0',
                                                           'name' => 'inArgCount',
                                                           'comments' => [
                                                                           '// overridden, was ValRef<uint16_t> inArgCount'
                                                                         ],
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'acc' => 'valRef'
                                                         },
                                                         {
                                                           'wname' => 'outCount',
                                                           'name' => 'outArgCount',
                                                           'type' => 'uint16_t',
                                                           'defValue' => '0',
                                                           'acc' => 'valRef',
                                                           'comments' => [
                                                                           '// overridden, was ValRef<uint16_t> outArgCount'
                                                                         ],
                                                           'wtype' => 'ValRef<uint16_t>'
                                                         },
                                                         {
                                                           'wspecial' => 'DirectiveSignatureArguments',
                                                           'wname' => 'args',
                                                           'type' => 'BrigDirectiveSignatureArgument',
                                                           'name' => 'args',
                                                           'wtype' => 'DirectiveSignatureArgument',
                                                           'acc' => 'subItem<DirectiveSignatureArgument>',
                                                           'size' => '1'
                                                         }
                                                       ],
                                           'enum' => 'BRIG_DIRECTIVE_SIGNATURE',
                                           'align' => undef,
                                           'name' => 'BrigDirectiveSignature',
                                           'comments' => [
                                                           '/// function signature.'
                                                         ],
                                           'parent' => 'BrigDirectiveCallableBase'
                                         },
             'BrigOperandLabelVariableRef' => {
                                                'parent' => 'BrigOperandRef',
                                                'enum' => 'BRIG_OPERAND_LABEL_VARIABLE_REF',
                                                'fields' => [
                                                              {
                                                                'wname' => 'size',
                                                                'name' => 'size',
                                                                'type' => 'uint16_t',
                                                                'acc' => 'valRef',
                                                                'wtype' => 'ValRef<uint16_t>'
                                                              },
                                                              {
                                                                'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                                'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                                'wname' => 'kind',
                                                                'enum' => 'BrigOperandKinds',
                                                                'name' => 'kind',
                                                                'type' => 'BrigOperandKinds16_t'
                                                              },
                                                              {
                                                                'name' => 'symbol',
                                                                'defValue' => '0',
                                                                'type' => 'BrigDirectiveOffset32_t',
                                                                'wname' => 'symbol',
                                                                'acc' => 'itemRef<DirectiveVariable>',
                                                                'wtype' => 'ItemRef<DirectiveVariable>'
                                                              }
                                                            ],
                                                'wname' => 'OperandLabelVariableRef',
                                                'name' => 'BrigOperandLabelVariableRef',
                                                'align' => undef
                                              },
             'BrigDirectiveLabel' => {
                                       'parent' => 'BrigDirectiveCode',
                                       'comments' => [
                                                       '/// label directive'
                                                     ],
                                       'name' => 'BrigDirectiveLabel',
                                       'align' => undef,
                                       'enum' => 'BRIG_DIRECTIVE_LABEL',
                                       'wname' => 'DirectiveLabel',
                                       'fields' => [
                                                     {
                                                       'acc' => 'valRef',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'name' => 'size',
                                                       'type' => 'uint16_t',
                                                       'wname' => 'size'
                                                     },
                                                     {
                                                       'name' => 'kind',
                                                       'type' => 'BrigDirectiveKinds16_t',
                                                       'wname' => 'kind',
                                                       'enum' => 'BrigDirectiveKinds',
                                                       'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                       'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                     },
                                                     {
                                                       'wname' => 'code',
                                                       'name' => 'code',
                                                       'defValue' => '0',
                                                       'type' => 'BrigCodeOffset32_t',
                                                       'acc' => 'itemRef<Inst>',
                                                       'wtype' => 'ItemRef<Inst>'
                                                     },
                                                     {
                                                       'wtype' => 'StrRef',
                                                       'acc' => 'strRef',
                                                       'type' => 'BrigStringOffset32_t',
                                                       'defValue' => '0',
                                                       'name' => 'name',
                                                       'wname' => 'name'
                                                     }
                                                   ]
                                     },
             'BrigBlockEnd' => {
                                 'comments' => [
                                                 '/// end of block.'
                                               ],
                                 'parent' => 'BrigDirective',
                                 'wname' => 'BlockEnd',
                                 'fields' => [
                                               {
                                                 'name' => 'size',
                                                 'type' => 'uint16_t',
                                                 'wname' => 'size',
                                                 'acc' => 'valRef',
                                                 'wtype' => 'ValRef<uint16_t>'
                                               },
                                               {
                                                 'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                 'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                 'type' => 'BrigDirectiveKinds16_t',
                                                 'name' => 'kind',
                                                 'enum' => 'BrigDirectiveKinds',
                                                 'wname' => 'kind'
                                               }
                                             ],
                                 'enum' => 'BRIG_DIRECTIVE_BLOCK_END',
                                 'align' => undef,
                                 'name' => 'BrigBlockEnd'
                               },
             'BrigInstBase' => {
                                 'name' => 'BrigInstBase',
                                 'align' => undef,
                                 'fields' => [
                                               {
                                                 'type' => 'uint16_t',
                                                 'name' => 'size'
                                               },
                                               {
                                                 'type' => 'BrigInstKinds16_t',
                                                 'name' => 'kind'
                                               },
                                               {
                                                 'type' => 'BrigOpcode16_t',
                                                 'name' => 'opcode'
                                               },
                                               {
                                                 'type' => 'BrigType16_t',
                                                 'name' => 'type'
                                               },
                                               {
                                                 'size' => '5',
                                                 'name' => 'operands',
                                                 'type' => 'BrigOperandOffset32_t'
                                               }
                                             ],
                                 'nowrap' => 'true'
                               },
             'BrigDirectiveArgScopeEnd' => {
                                             'enum' => 'BRIG_DIRECTIVE_ARG_SCOPE_END',
                                             'wname' => 'DirectiveArgScopeEnd',
                                             'fields' => [
                                                           {
                                                             'type' => 'uint16_t',
                                                             'name' => 'size',
                                                             'wname' => 'size',
                                                             'wtype' => 'ValRef<uint16_t>',
                                                             'acc' => 'valRef'
                                                           },
                                                           {
                                                             'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                             'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                             'wname' => 'kind',
                                                             'enum' => 'BrigDirectiveKinds',
                                                             'name' => 'kind',
                                                             'type' => 'BrigDirectiveKinds16_t'
                                                           },
                                                           {
                                                             'wtype' => 'ItemRef<Inst>',
                                                             'acc' => 'itemRef<Inst>',
                                                             'defValue' => '0',
                                                             'type' => 'BrigCodeOffset32_t',
                                                             'name' => 'code',
                                                             'wname' => 'code'
                                                           }
                                                         ],
                                             'align' => undef,
                                             'name' => 'BrigDirectiveArgScopeEnd',
                                             'parent' => 'BrigDirectiveCode'
                                           },
             'BrigInstSegCvt' => {
                                   'parent' => 'BrigInst',
                                   'name' => 'BrigInstSegCvt',
                                   'align' => undef,
                                   'enum' => 'BRIG_INST_SEG_CVT',
                                   'fields' => [
                                                 {
                                                   'acc' => 'valRef',
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'name' => 'size',
                                                   'type' => 'uint16_t',
                                                   'wname' => 'size'
                                                 },
                                                 {
                                                   'name' => 'kind',
                                                   'type' => 'BrigInstKinds16_t',
                                                   'wname' => 'kind',
                                                   'enum' => 'BrigInstKinds',
                                                   'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                   'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>'
                                                 },
                                                 {
                                                   'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                   'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                   'type' => 'BrigOpcode16_t',
                                                   'name' => 'opcode',
                                                   'enum' => 'BrigOpcode',
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
                                                   'wtype' => 'ItemRef<Operand>',
                                                   'size' => '5',
                                                   'type' => 'BrigOperandOffset32_t',
                                                   'comments' => [
                                                                   '// overridden, was ItemRef<Operand> operand'
                                                                 ],
                                                   'acc' => 'itemRef<Operand>',
                                                   'wname' => 'operands',
                                                   'defValue' => '0',
                                                   'name' => 'operands'
                                                 },
                                                 {
                                                   'wname' => 'sourceType',
                                                   'name' => 'sourceType',
                                                   'type' => 'BrigType16_t',
                                                   'acc' => 'valRef',
                                                   'wtype' => 'ValRef<uint16_t>'
                                                 },
                                                 {
                                                   'defValue' => '0',
                                                   'type' => 'BrigSegment8_t',
                                                   'name' => 'segment',
                                                   'enum' => 'BrigSegment',
                                                   'wname' => 'segment',
                                                   'wtype' => 'EnumValRef<Brig::BrigSegment,uint8_t>',
                                                   'acc' => 'enumValRef<Brig::BrigSegment,uint8_t>'
                                                 },
                                                 {
                                                   'acc' => 'subItem<SegCvtModifier>',
                                                   'wtype' => 'SegCvtModifier',
                                                   'wname' => 'modifier',
                                                   'name' => 'modifier',
                                                   'type' => 'BrigSegCvtModifier'
                                                 }
                                               ],
                                   'wname' => 'InstSegCvt'
                                 },
             'BrigOperandFbarrierRef' => {
                                           'align' => undef,
                                           'name' => 'BrigOperandFbarrierRef',
                                           'fields' => [
                                                         {
                                                           'wname' => 'size',
                                                           'name' => 'size',
                                                           'type' => 'uint16_t',
                                                           'acc' => 'valRef',
                                                           'wtype' => 'ValRef<uint16_t>'
                                                         },
                                                         {
                                                           'enum' => 'BrigOperandKinds',
                                                           'wname' => 'kind',
                                                           'type' => 'BrigOperandKinds16_t',
                                                           'name' => 'kind',
                                                           'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                           'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>'
                                                         },
                                                         {
                                                           'comments' => [
                                                                           '// overridden, was ItemRef<Directive> ref'
                                                                         ],
                                                           'wtype' => 'ItemRef<DirectiveFbarrier>',
                                                           'acc' => 'itemRef<DirectiveFbarrier>',
                                                           'type' => 'BrigDirectiveOffset32_t',
                                                           'defValue' => '0',
                                                           'name' => 'ref',
                                                           'wname' => 'fbar'
                                                         }
                                                       ],
                                           'wname' => 'OperandFbarrierRef',
                                           'enum' => 'BRIG_OPERAND_FBARRIER_REF',
                                           'parent' => 'BrigOperandRef'
                                         },
             'BrigInstAtomic' => {
                                   'parent' => 'BrigInst',
                                   'wname' => 'InstAtomic',
                                   'fields' => [
                                                 {
                                                   'wname' => 'size',
                                                   'name' => 'size',
                                                   'type' => 'uint16_t',
                                                   'acc' => 'valRef',
                                                   'wtype' => 'ValRef<uint16_t>'
                                                 },
                                                 {
                                                   'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                   'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>',
                                                   'name' => 'kind',
                                                   'type' => 'BrigInstKinds16_t',
                                                   'wname' => 'kind',
                                                   'enum' => 'BrigInstKinds'
                                                 },
                                                 {
                                                   'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                   'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                   'type' => 'BrigOpcode16_t',
                                                   'name' => 'opcode',
                                                   'enum' => 'BrigOpcode',
                                                   'wname' => 'opcode'
                                                 },
                                                 {
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'acc' => 'valRef',
                                                   'wname' => 'type',
                                                   'type' => 'BrigType16_t',
                                                   'name' => 'type'
                                                 },
                                                 {
                                                   'wtype' => 'ItemRef<Operand>',
                                                   'acc' => 'itemRef<Operand>',
                                                   'size' => '5',
                                                   'wname' => 'operand',
                                                   'type' => 'BrigOperandOffset32_t',
                                                   'defValue' => '0',
                                                   'name' => 'operands'
                                                 },
                                                 {
                                                   'name' => 'segment',
                                                   'type' => 'BrigSegment8_t',
                                                   'defValue' => '0',
                                                   'wname' => 'segment',
                                                   'enum' => 'BrigSegment',
                                                   'acc' => 'enumValRef<Brig::BrigSegment,uint8_t>',
                                                   'wtype' => 'EnumValRef<Brig::BrigSegment,uint8_t>'
                                                 },
                                                 {
                                                   'acc' => 'enumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                   'wtype' => 'EnumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                   'name' => 'memoryOrder',
                                                   'type' => 'BrigMemoryOrder8_t',
                                                   'defValue' => 'Brig::BRIG_MEMORY_ORDER_RELAXED',
                                                   'wname' => 'memoryOrder',
                                                   'enum' => 'BrigMemoryOrder'
                                                 },
                                                 {
                                                   'enum' => 'BrigMemoryScope',
                                                   'wname' => 'memoryScope',
                                                   'defValue' => 'Brig::BRIG_MEMORY_SCOPE_SYSTEM',
                                                   'type' => 'BrigMemoryScope8_t',
                                                   'name' => 'memoryScope',
                                                   'wtype' => 'EnumValRef<Brig::BrigMemoryScope,uint8_t>',
                                                   'acc' => 'enumValRef<Brig::BrigMemoryScope,uint8_t>'
                                                 },
                                                 {
                                                   'type' => 'BrigAtomicOperation8_t',
                                                   'name' => 'atomicOperation',
                                                   'enum' => 'BrigAtomicOperation',
                                                   'wname' => 'atomicOperation',
                                                   'wtype' => 'EnumValRef<Brig::BrigAtomicOperation,uint8_t>',
                                                   'acc' => 'enumValRef<Brig::BrigAtomicOperation,uint8_t>'
                                                 },
                                                 {
                                                   'type' => 'uint8_t',
                                                   'name' => 'equivClass',
                                                   'wname' => 'equivClass',
                                                   'wtype' => 'ValRef<uint8_t>',
                                                   'acc' => 'valRef'
                                                 },
                                                 {
                                                   'type' => 'uint8_t',
                                                   'wtype' => 'ValRef<uint8_t>',
                                                   'size' => 3,
                                                   'defValue' => '0',
                                                   'name' => 'reserved',
                                                   'wname' => 'reserved',
                                                   'acc' => 'valRef',
                                                   'skip' => 1
                                                 }
                                               ],
                                   'enum' => 'BRIG_INST_ATOMIC',
                                   'align' => undef,
                                   'name' => 'BrigInstAtomic'
                                 },
             'BrigDirectiveControl' => {
                                         'parent' => 'BrigDirectiveCode',
                                         'comments' => [
                                                         '/// control directive.'
                                                       ],
                                         'align' => undef,
                                         'name' => 'BrigDirectiveControl',
                                         'fields' => [
                                                       {
                                                         'wname' => 'size',
                                                         'name' => 'size',
                                                         'type' => 'uint16_t',
                                                         'acc' => 'valRef',
                                                         'wtype' => 'ValRef<uint16_t>'
                                                       },
                                                       {
                                                         'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                         'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                         'wname' => 'kind',
                                                         'enum' => 'BrigDirectiveKinds',
                                                         'name' => 'kind',
                                                         'type' => 'BrigDirectiveKinds16_t'
                                                       },
                                                       {
                                                         'wname' => 'code',
                                                         'name' => 'code',
                                                         'type' => 'BrigCodeOffset32_t',
                                                         'defValue' => '0',
                                                         'acc' => 'itemRef<Inst>',
                                                         'wtype' => 'ItemRef<Inst>'
                                                       },
                                                       {
                                                         'name' => 'control',
                                                         'type' => 'BrigControlDirective16_t',
                                                         'wname' => 'control',
                                                         'enum' => 'BrigControlDirective',
                                                         'acc' => 'enumValRef<Brig::BrigControlDirective,uint16_t>',
                                                         'wtype' => 'EnumValRef<Brig::BrigControlDirective,uint16_t>'
                                                       },
                                                       {
                                                         'acc' => 'valRef',
                                                         'wtype' => 'ValRef<uint16_t>',
                                                         'wname' => 'type',
                                                         'name' => 'type',
                                                         'type' => 'BrigType16_t'
                                                       },
                                                       {
                                                         'wname' => 'reserved',
                                                         'name' => 'reserved',
                                                         'defValue' => '0',
                                                         'type' => 'uint16_t',
                                                         'acc' => 'valRef',
                                                         'skip' => 1,
                                                         'wtype' => 'ValRef<uint16_t>'
                                                       },
                                                       {
                                                         'acc' => 'valRef',
                                                         'wtype' => 'ValRef<uint16_t>',
                                                         'wname' => 'elementCount',
                                                         'name' => 'valueCount',
                                                         'type' => 'uint16_t',
                                                         'defValue' => '0'
                                                       },
                                                       {
                                                         'wname' => 'values',
                                                         'defValue' => '0',
                                                         'name' => 'values',
                                                         'acc' => 'itemRef<Operand>',
                                                         'wspecial' => 'ControlValues',
                                                         'type' => 'BrigOperandOffset32_t',
                                                         'wtype' => 'ItemRef<Operand>',
                                                         'size' => '1'
                                                       }
                                                     ],
                                         'wname' => 'DirectiveControl',
                                         'enum' => 'BRIG_DIRECTIVE_CONTROL'
                                       },
             'BrigOperandLabelTargetsRef' => {
                                               'align' => undef,
                                               'name' => 'BrigOperandLabelTargetsRef',
                                               'enum' => 'BRIG_OPERAND_LABEL_TARGETS_REF',
                                               'wname' => 'OperandLabelTargetsRef',
                                               'fields' => [
                                                             {
                                                               'name' => 'size',
                                                               'type' => 'uint16_t',
                                                               'wname' => 'size',
                                                               'acc' => 'valRef',
                                                               'wtype' => 'ValRef<uint16_t>'
                                                             },
                                                             {
                                                               'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                               'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                               'enum' => 'BrigOperandKinds',
                                                               'wname' => 'kind',
                                                               'type' => 'BrigOperandKinds16_t',
                                                               'name' => 'kind'
                                                             },
                                                             {
                                                               'defValue' => '0',
                                                               'type' => 'BrigDirectiveOffset32_t',
                                                               'name' => 'targets',
                                                               'wname' => 'targets',
                                                               'wtype' => 'ItemRef<DirectiveLabelTargets>',
                                                               'acc' => 'itemRef<DirectiveLabelTargets>'
                                                             }
                                                           ],
                                               'parent' => 'BrigOperandRef'
                                             },
             'BrigDirectiveCode' => {
                                      'fields' => [
                                                    {
                                                      'wtype' => 'ValRef<uint16_t>',
                                                      'acc' => 'valRef',
                                                      'type' => 'uint16_t',
                                                      'name' => 'size',
                                                      'wname' => 'size'
                                                    },
                                                    {
                                                      'enum' => 'BrigDirectiveKinds',
                                                      'wname' => 'kind',
                                                      'type' => 'BrigDirectiveKinds16_t',
                                                      'name' => 'kind',
                                                      'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                      'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                    },
                                                    {
                                                      'type' => 'BrigCodeOffset32_t',
                                                      'defValue' => '0',
                                                      'name' => 'code',
                                                      'wname' => 'code',
                                                      'comments' => [
                                                                      '/// Location in the instruction stream corresponding to this directive.'
                                                                    ],
                                                      'wtype' => 'ItemRef<Inst>',
                                                      'acc' => 'itemRef<Inst>'
                                                    }
                                                  ],
                                      'align' => undef,
                                      'wname' => 'DirectiveCode',
                                      'enum' => 'BRIG_DIRECTIVE_CODE',
                                      'generic' => 'true',
                                      'name' => 'BrigDirectiveCode',
                                      'children' => [
                                                      'BrigDirectiveLabelTargets',
                                                      'BrigDirectiveControl',
                                                      'BrigDirectiveExtension',
                                                      'BrigDirectiveVariable',
                                                      'BrigDirectiveFbarrier',
                                                      'BrigDirectiveImageProperties',
                                                      'BrigBlockStart',
                                                      'BrigDirectiveComment',
                                                      'BrigDirectivePragma',
                                                      'BrigDirectiveKernel',
                                                      'BrigDirectiveImageInit',
                                                      'BrigDirectiveLoc',
                                                      'BrigDirectiveSamplerInit',
                                                      'BrigDirectiveVariableInit',
                                                      'BrigDirectiveLabelInit',
                                                      'BrigDirectiveFunction',
                                                      'BrigDirectiveArgScopeStart',
                                                      'BrigDirectiveSignature',
                                                      'BrigDirectiveOpaqueInit',
                                                      'BrigDirectiveExecutable',
                                                      'BrigDirectiveVersion',
                                                      'BrigDirectiveArgScopeEnd',
                                                      'BrigDirectiveCallableBase',
                                                      'BrigDirectiveLabel',
                                                      'BrigDirectiveSamplerProperties'
                                                    ],
                                      'comments' => [
                                                      '/// base class for directives that positioned in instruction stream.'
                                                    ],
                                      'parent' => 'BrigDirective'
                                    },
             'BrigOperandFunctionRef' => {
                                           'fields' => [
                                                         {
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'acc' => 'valRef',
                                                           'wname' => 'size',
                                                           'type' => 'uint16_t',
                                                           'name' => 'size'
                                                         },
                                                         {
                                                           'name' => 'kind',
                                                           'type' => 'BrigOperandKinds16_t',
                                                           'wname' => 'kind',
                                                           'enum' => 'BrigOperandKinds',
                                                           'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                           'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>'
                                                         },
                                                         {
                                                           'acc' => 'itemRef<DirectiveFunction>',
                                                           'comments' => [
                                                                           '// overridden, was ItemRef<Directive> ref'
                                                                         ],
                                                           'wtype' => 'ItemRef<DirectiveFunction>',
                                                           'wname' => 'fn',
                                                           'name' => 'ref',
                                                           'type' => 'BrigDirectiveOffset32_t',
                                                           'defValue' => '0'
                                                         }
                                                       ],
                                           'wname' => 'OperandFunctionRef',
                                           'enum' => 'BRIG_OPERAND_FUNCTION_REF',
                                           'align' => undef,
                                           'name' => 'BrigOperandFunctionRef',
                                           'parent' => 'BrigOperandRef'
                                         },
             'BrigOperandReg' => {
                                   'parent' => 'BrigOperand',
                                   'name' => 'BrigOperandReg',
                                   'align' => undef,
                                   'fields' => [
                                                 {
                                                   'acc' => 'valRef',
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'name' => 'size',
                                                   'type' => 'uint16_t',
                                                   'wname' => 'size'
                                                 },
                                                 {
                                                   'enum' => 'BrigOperandKinds',
                                                   'wname' => 'kind',
                                                   'type' => 'BrigOperandKinds16_t',
                                                   'name' => 'kind',
                                                   'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                   'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>'
                                                 },
                                                 {
                                                   'wname' => 'reg',
                                                   'type' => 'BrigStringOffset32_t',
                                                   'defValue' => '0',
                                                   'name' => 'reg',
                                                   'wtype' => 'StrRef',
                                                   'acc' => 'strRef'
                                                 }
                                               ],
                                   'wname' => 'OperandReg',
                                   'enum' => 'BRIG_OPERAND_REG'
                                 },
             'BrigDirectiveImageProperties' => {
                                                 'name' => 'BrigDirectiveImageProperties',
                                                 'align' => undef,
                                                 'enum' => 'BRIG_DIRECTIVE_IMAGE_PROPERTIES',
                                                 'fields' => [
                                                               {
                                                                 'wtype' => 'ValRef<uint16_t>',
                                                                 'acc' => 'valRef',
                                                                 'wname' => 'size',
                                                                 'type' => 'uint16_t',
                                                                 'name' => 'size'
                                                               },
                                                               {
                                                                 'wname' => 'kind',
                                                                 'enum' => 'BrigDirectiveKinds',
                                                                 'name' => 'kind',
                                                                 'type' => 'BrigDirectiveKinds16_t',
                                                                 'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                                 'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                               },
                                                               {
                                                                 'name' => 'code',
                                                                 'type' => 'BrigCodeOffset32_t',
                                                                 'defValue' => '0',
                                                                 'wname' => 'code',
                                                                 'acc' => 'itemRef<Inst>',
                                                                 'wtype' => 'ItemRef<Inst>'
                                                               },
                                                               {
                                                                 'wname' => 'width',
                                                                 'name' => 'width',
                                                                 'type' => 'uint32_t',
                                                                 'defValue' => '0',
                                                                 'acc' => 'valRef',
                                                                 'wtype' => 'ValRef<uint32_t>'
                                                               },
                                                               {
                                                                 'acc' => 'valRef',
                                                                 'wtype' => 'ValRef<uint32_t>',
                                                                 'wname' => 'height',
                                                                 'name' => 'height',
                                                                 'defValue' => '0',
                                                                 'type' => 'uint32_t'
                                                               },
                                                               {
                                                                 'wtype' => 'ValRef<uint32_t>',
                                                                 'acc' => 'valRef',
                                                                 'defValue' => '0',
                                                                 'type' => 'uint32_t',
                                                                 'name' => 'depth',
                                                                 'wname' => 'depth'
                                                               },
                                                               {
                                                                 'wname' => 'array',
                                                                 'defValue' => '0',
                                                                 'type' => 'uint32_t',
                                                                 'name' => 'array',
                                                                 'wtype' => 'ValRef<uint32_t>',
                                                                 'acc' => 'valRef'
                                                               },
                                                               {
                                                                 'name' => 'geometry',
                                                                 'type' => 'BrigImageGeometry8_t',
                                                                 'defValue' => 'Brig::BRIG_GEOMETRY_UNKNOWN',
                                                                 'wname' => 'geometry',
                                                                 'enum' => 'BrigImageGeometry',
                                                                 'acc' => 'enumValRef<Brig::BrigImageGeometry,uint8_t>',
                                                                 'wtype' => 'EnumValRef<Brig::BrigImageGeometry,uint8_t>'
                                                               },
                                                               {
                                                                 'type' => 'BrigImageChannelOrder8_t',
                                                                 'defValue' => 'Brig::BRIG_CHANNEL_ORDER_UNKNOWN',
                                                                 'name' => 'channelOrder',
                                                                 'enum' => 'BrigImageChannelOrder',
                                                                 'wname' => 'channelOrder',
                                                                 'wtype' => 'EnumValRef<Brig::BrigImageChannelOrder,uint8_t>',
                                                                 'acc' => 'enumValRef<Brig::BrigImageChannelOrder,uint8_t>'
                                                               },
                                                               {
                                                                 'wname' => 'channelType',
                                                                 'enum' => 'BrigImageChannelType',
                                                                 'name' => 'channelType',
                                                                 'type' => 'BrigImageChannelType8_t',
                                                                 'defValue' => 'Brig::BRIG_CHANNEL_TYPE_UNKNOWN',
                                                                 'acc' => 'enumValRef<Brig::BrigImageChannelType,uint8_t>',
                                                                 'wtype' => 'EnumValRef<Brig::BrigImageChannelType,uint8_t>'
                                                               },
                                                               {
                                                                 'wtype' => 'ValRef<uint8_t>',
                                                                 'skip' => 1,
                                                                 'acc' => 'valRef',
                                                                 'wname' => 'reserved',
                                                                 'defValue' => '0',
                                                                 'type' => 'uint8_t',
                                                                 'name' => 'reserved'
                                                               }
                                                             ],
                                                 'wname' => 'DirectiveImageProperties',
                                                 'parent' => 'BrigDirectiveCode'
                                               },
             'BrigAluModifier' => {
                                    'enum' => 'BRIG_ALU_MODIFIER',
                                    'wname' => 'AluModifier',
                                    'fields' => [
                                                  {
                                                    'type' => 'BrigAluModifier16_t',
                                                    'defValue' => '0',
                                                    'name' => 'allBits',
                                                    'wname' => 'allBits',
                                                    'wtype' => 'ValRef<uint16_t>',
                                                    'acc' => 'valRef'
                                                  },
                                                  {
                                                    'wtype' => 'BFValRef<Brig::BrigRound8_t,0,4>',
                                                    'acc' => 'bFValRef<Brig::BrigRound8_t,0,4>',
                                                    'type' => 'BrigRound8_t',
                                                    'name' => 'round',
                                                    'phantomof' => $structs->{'BrigAluModifier'}{'fields'}[0],
                                                    'enum' => 'BrigRound',
                                                    'wname' => 'round'
                                                  },
                                                  {
                                                    'type' => 'bool',
                                                    'name' => 'ftz',
                                                    'phantomof' => $structs->{'BrigAluModifier'}{'fields'}[0],
                                                    'wname' => 'ftz',
                                                    'wtype' => 'BitValRef<4>',
                                                    'acc' => 'bitValRef<4>'
                                                  }
                                                ],
                                    'standalone' => 'true',
                                    'isroot' => 'true',
                                    'align' => undef,
                                    'name' => 'BrigAluModifier'
                                  },
             'BrigOperandArgumentList' => {
                                            'enum' => 'BRIG_OPERAND_ARGUMENT_LIST',
                                            'fields' => [
                                                          {
                                                            'name' => 'size',
                                                            'type' => 'uint16_t',
                                                            'wname' => 'size',
                                                            'acc' => 'valRef',
                                                            'wtype' => 'ValRef<uint16_t>'
                                                          },
                                                          {
                                                            'wname' => 'kind',
                                                            'enum' => 'BrigOperandKinds',
                                                            'name' => 'kind',
                                                            'type' => 'BrigOperandKinds16_t',
                                                            'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                            'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>'
                                                          },
                                                          {
                                                            'wtype' => 'ValRef<uint16_t>',
                                                            'skip' => 1,
                                                            'acc' => 'valRef',
                                                            'defValue' => '0',
                                                            'type' => 'uint16_t',
                                                            'name' => 'reserved',
                                                            'wname' => 'reserved'
                                                          },
                                                          {
                                                            'acc' => 'valRef',
                                                            'wtype' => 'ValRef<uint16_t>',
                                                            'wname' => 'elementCount',
                                                            'name' => 'elementCount',
                                                            'type' => 'uint16_t',
                                                            'defValue' => '0'
                                                          },
                                                          {
                                                            'wname' => 'elements',
                                                            'name' => 'elements',
                                                            'defValue' => '0',
                                                            'acc' => 'itemRef<Directive>',
                                                            'wspecial' => 'ArgumentRefList',
                                                            'type' => 'BrigDirectiveOffset32_t',
                                                            'size' => '1',
                                                            'wtype' => 'ItemRef<Directive>'
                                                          }
                                                        ],
                                            'wname' => 'OperandArgumentList',
                                            'name' => 'BrigOperandArgumentList',
                                            'align' => undef,
                                            'parent' => 'BrigOperandList'
                                          },
             'BrigDirectiveComment' => {
                                         'parent' => 'BrigDirectiveCode',
                                         'comments' => [
                                                         '/// comment directive.'
                                                       ],
                                         'align' => undef,
                                         'name' => 'BrigDirectiveComment',
                                         'enum' => 'BRIG_DIRECTIVE_COMMENT',
                                         'wname' => 'DirectiveComment',
                                         'fields' => [
                                                       {
                                                         'type' => 'uint16_t',
                                                         'name' => 'size',
                                                         'wname' => 'size',
                                                         'wtype' => 'ValRef<uint16_t>',
                                                         'acc' => 'valRef'
                                                       },
                                                       {
                                                         'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                         'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                         'enum' => 'BrigDirectiveKinds',
                                                         'wname' => 'kind',
                                                         'type' => 'BrigDirectiveKinds16_t',
                                                         'name' => 'kind'
                                                       },
                                                       {
                                                         'wname' => 'code',
                                                         'defValue' => '0',
                                                         'type' => 'BrigCodeOffset32_t',
                                                         'name' => 'code',
                                                         'wtype' => 'ItemRef<Inst>',
                                                         'acc' => 'itemRef<Inst>'
                                                       },
                                                       {
                                                         'wtype' => 'StrRef',
                                                         'acc' => 'strRef',
                                                         'wname' => 'name',
                                                         'defValue' => '0',
                                                         'type' => 'BrigStringOffset32_t',
                                                         'name' => 'name'
                                                       }
                                                     ]
                                       },
             'BrigDirectiveOpaqueInit' => {
                                            'generic' => 'true',
                                            'name' => 'BrigDirectiveOpaqueInit',
                                            'children' => [
                                                            'BrigDirectiveImageInit',
                                                            'BrigDirectiveSamplerInit'
                                                          ],
                                            'wname' => 'DirectiveOpaqueInit',
                                            'enum' => 'BRIG_DIRECTIVE_OPAQUE_INIT',
                                            'parent' => 'BrigDirectiveCode',
                                            'align' => undef,
                                            'fields' => [
                                                          {
                                                            'acc' => 'valRef',
                                                            'wtype' => 'ValRef<uint16_t>',
                                                            'wname' => 'size',
                                                            'name' => 'size',
                                                            'type' => 'uint16_t'
                                                          },
                                                          {
                                                            'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                            'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                            'enum' => 'BrigDirectiveKinds',
                                                            'wname' => 'kind',
                                                            'type' => 'BrigDirectiveKinds16_t',
                                                            'name' => 'kind'
                                                          },
                                                          {
                                                            'wtype' => 'ItemRef<Inst>',
                                                            'acc' => 'itemRef<Inst>',
                                                            'defValue' => '0',
                                                            'type' => 'BrigCodeOffset32_t',
                                                            'name' => 'code',
                                                            'wname' => 'code'
                                                          },
                                                          {
                                                            'wname' => 'imageCount',
                                                            'name' => 'imageCount',
                                                            'defValue' => '0',
                                                            'type' => 'uint16_t',
                                                            'acc' => 'valRef',
                                                            'wtype' => 'ValRef<uint16_t>'
                                                          },
                                                          {
                                                            'acc' => 'valRef',
                                                            'wtype' => 'ValRef<uint16_t>',
                                                            'wname' => 'elementCount',
                                                            'phantomof' => $structs->{'BrigDirectiveOpaqueInit'}{'fields'}[3],
                                                            'name' => 'elementCount',
                                                            'type' => 'uint16_t'
                                                          },
                                                          {
                                                            'wtype' => 'ValRef<uint16_t>',
                                                            'acc' => 'valRef',
                                                            'skip' => 1,
                                                            'defValue' => '0',
                                                            'type' => 'uint16_t',
                                                            'name' => 'reserved',
                                                            'wname' => 'reserved'
                                                          },
                                                          {
                                                            'size' => '1',
                                                            'wtype' => 'ItemRef<Directive>',
                                                            'type' => 'BrigDirectiveOffset32_t',
                                                            'wspecial' => 'OpaqueInitList',
                                                            'acc' => 'itemRef<Directive>',
                                                            'name' => 'objects',
                                                            'defValue' => '0',
                                                            'wname' => 'objects'
                                                          }
                                                        ]
                                          },
             'BrigMemoryModifier' => {
                                       'isroot' => 'true',
                                       'standalone' => 'true',
                                       'name' => 'BrigMemoryModifier',
                                       'align' => undef,
                                       'enum' => 'BRIG_MEMORY_MODIFIER',
                                       'fields' => [
                                                     {
                                                       'wtype' => 'ValRef<uint8_t>',
                                                       'acc' => 'valRef',
                                                       'wname' => 'allBits',
                                                       'defValue' => '0',
                                                       'type' => 'BrigMemoryModifier8_t',
                                                       'name' => 'allBits'
                                                     },
                                                     {
                                                       'wtype' => 'BitValRef<0>',
                                                       'acc' => 'bitValRef<0>',
                                                       'type' => 'bool',
                                                       'name' => 'isConst',
                                                       'phantomof' => $structs->{'BrigMemoryModifier'}{'fields'}[0],
                                                       'wname' => 'isConst'
                                                     }
                                                   ],
                                       'wname' => 'MemoryModifier'
                                     },
             'BrigSegCvtModifier' => {
                                       'enum' => 'BRIG_SEG_CVT_MODIFIER',
                                       'wname' => 'SegCvtModifier',
                                       'fields' => [
                                                     {
                                                       'wname' => 'allBits',
                                                       'name' => 'allBits',
                                                       'type' => 'BrigSegCvtModifier8_t',
                                                       'defValue' => '0',
                                                       'acc' => 'valRef',
                                                       'wtype' => 'ValRef<uint8_t>'
                                                     },
                                                     {
                                                       'phantomof' => $structs->{'BrigSegCvtModifier'}{'fields'}[0],
                                                       'wname' => 'isNoNull',
                                                       'type' => 'bool',
                                                       'name' => 'isNoNull',
                                                       'wtype' => 'BitValRef<0>',
                                                       'acc' => 'bitValRef<0>'
                                                     }
                                                   ],
                                       'isroot' => 'true',
                                       'standalone' => 'true',
                                       'align' => undef,
                                       'name' => 'BrigSegCvtModifier'
                                     },
             'BrigDirectiveLabelInit' => {
                                           'align' => undef,
                                           'name' => 'BrigDirectiveLabelInit',
                                           'enum' => 'BRIG_DIRECTIVE_LABEL_INIT',
                                           'wname' => 'DirectiveLabelInit',
                                           'fields' => [
                                                         {
                                                           'type' => 'uint16_t',
                                                           'name' => 'size',
                                                           'wname' => 'size',
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'acc' => 'valRef'
                                                         },
                                                         {
                                                           'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                           'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                           'name' => 'kind',
                                                           'type' => 'BrigDirectiveKinds16_t',
                                                           'wname' => 'kind',
                                                           'enum' => 'BrigDirectiveKinds'
                                                         },
                                                         {
                                                           'acc' => 'itemRef<Inst>',
                                                           'wtype' => 'ItemRef<Inst>',
                                                           'name' => 'code',
                                                           'type' => 'BrigCodeOffset32_t',
                                                           'defValue' => '0',
                                                           'wname' => 'code'
                                                         },
                                                         {
                                                           'wname' => 'elementCount',
                                                           'name' => 'labelCount',
                                                           'type' => 'uint16_t',
                                                           'defValue' => '0',
                                                           'acc' => 'valRef',
                                                           'wtype' => 'ValRef<uint16_t>'
                                                         },
                                                         {
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'skip' => 1,
                                                           'acc' => 'valRef',
                                                           'defValue' => '0',
                                                           'type' => 'uint16_t',
                                                           'name' => 'reserved',
                                                           'wname' => 'reserved'
                                                         },
                                                         {
                                                           'name' => 'labels',
                                                           'defValue' => '0',
                                                           'wname' => 'labels',
                                                           'acc' => 'itemRef<DirectiveLabel>',
                                                           'type' => 'BrigDirectiveOffset32_t',
                                                           'wspecial' => 'LabelInitList',
                                                           'size' => '1',
                                                           'wtype' => 'ItemRef<DirectiveLabel>'
                                                         }
                                                       ],
                                           'parent' => 'BrigDirectiveCode'
                                         },
             'BrigInstMod' => {
                                'parent' => 'BrigInst',
                                'name' => 'BrigInstMod',
                                'align' => undef,
                                'fields' => [
                                              {
                                                'acc' => 'valRef',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'name' => 'size',
                                                'type' => 'uint16_t',
                                                'wname' => 'size'
                                              },
                                              {
                                                'name' => 'kind',
                                                'type' => 'BrigInstKinds16_t',
                                                'wname' => 'kind',
                                                'enum' => 'BrigInstKinds',
                                                'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>'
                                              },
                                              {
                                                'type' => 'BrigOpcode16_t',
                                                'name' => 'opcode',
                                                'enum' => 'BrigOpcode',
                                                'wname' => 'opcode',
                                                'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>'
                                              },
                                              {
                                                'wname' => 'type',
                                                'name' => 'type',
                                                'type' => 'BrigType16_t',
                                                'acc' => 'valRef',
                                                'wtype' => 'ValRef<uint16_t>'
                                              },
                                              {
                                                'name' => 'operands',
                                                'type' => 'BrigOperandOffset32_t',
                                                'defValue' => '0',
                                                'wname' => 'operand',
                                                'size' => '5',
                                                'acc' => 'itemRef<Operand>',
                                                'wtype' => 'ItemRef<Operand>'
                                              },
                                              {
                                                'wname' => 'modifier',
                                                'name' => 'modifier',
                                                'type' => 'BrigAluModifier',
                                                'acc' => 'subItem<AluModifier>',
                                                'wtype' => 'AluModifier'
                                              },
                                              {
                                                'wtype' => 'EnumValRef<Brig::BrigPack,uint8_t>',
                                                'acc' => 'enumValRef<Brig::BrigPack,uint8_t>',
                                                'defValue' => 'Brig::BRIG_PACK_NONE',
                                                'type' => 'BrigPack8_t',
                                                'name' => 'pack',
                                                'enum' => 'BrigPack',
                                                'wname' => 'pack'
                                              },
                                              {
                                                'wname' => 'reserved',
                                                'name' => 'reserved',
                                                'type' => 'uint8_t',
                                                'defValue' => '0',
                                                'acc' => 'valRef',
                                                'skip' => 1,
                                                'wtype' => 'ValRef<uint8_t>'
                                              }
                                            ],
                                'wname' => 'InstMod',
                                'enum' => 'BRIG_INST_MOD'
                              },
             'BrigInstQuerySampler' => {
                                         'parent' => 'BrigInst',
                                         'align' => undef,
                                         'name' => 'BrigInstQuerySampler',
                                         'wname' => 'InstQuerySampler',
                                         'fields' => [
                                                       {
                                                         'wtype' => 'ValRef<uint16_t>',
                                                         'acc' => 'valRef',
                                                         'type' => 'uint16_t',
                                                         'name' => 'size',
                                                         'wname' => 'size'
                                                       },
                                                       {
                                                         'name' => 'kind',
                                                         'type' => 'BrigInstKinds16_t',
                                                         'wname' => 'kind',
                                                         'enum' => 'BrigInstKinds',
                                                         'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                         'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>'
                                                       },
                                                       {
                                                         'type' => 'BrigOpcode16_t',
                                                         'name' => 'opcode',
                                                         'enum' => 'BrigOpcode',
                                                         'wname' => 'opcode',
                                                         'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                         'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>'
                                                       },
                                                       {
                                                         'type' => 'BrigType16_t',
                                                         'name' => 'type',
                                                         'wname' => 'type',
                                                         'wtype' => 'ValRef<uint16_t>',
                                                         'acc' => 'valRef'
                                                       },
                                                       {
                                                         'comments' => [
                                                                         '// overridden, was ItemRef<Operand> operand'
                                                                       ],
                                                         'acc' => 'itemRef<Operand>',
                                                         'defValue' => '0',
                                                         'name' => 'operands',
                                                         'wname' => 'operands',
                                                         'wtype' => 'ItemRef<Operand>',
                                                         'size' => '5',
                                                         'type' => 'BrigOperandOffset32_t'
                                                       },
                                                       {
                                                         'acc' => 'enumValRef<Brig::BrigSamplerQuery,uint8_t>',
                                                         'wtype' => 'EnumValRef<Brig::BrigSamplerQuery,uint8_t>',
                                                         'wname' => 'samplerQuery',
                                                         'enum' => 'BrigSamplerQuery',
                                                         'name' => 'samplerQuery',
                                                         'type' => 'BrigSamplerQuery8_t'
                                                       },
                                                       {
                                                         'wname' => 'reserved',
                                                         'name' => 'reserved',
                                                         'defValue' => '0',
                                                         'acc' => 'valRef',
                                                         'skip' => 1,
                                                         'type' => 'uint8_t',
                                                         'size' => 3,
                                                         'wtype' => 'ValRef<uint8_t>'
                                                       }
                                                     ],
                                         'enum' => 'BRIG_INST_QUERY_SAMPLER'
                                       },
             'BrigInstNone' => {
                                 'enum' => 'BRIG_INST_NONE',
                                 'fields' => [
                                               {
                                                 'wtype' => 'ValRef<uint16_t>',
                                                 'acc' => 'valRef',
                                                 'type' => 'uint16_t',
                                                 'name' => 'size',
                                                 'wname' => 'size'
                                               },
                                               {
                                                 'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>',
                                                 'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                 'enum' => 'BrigInstKinds',
                                                 'wname' => 'kind',
                                                 'type' => 'BrigInstKinds16_t',
                                                 'name' => 'kind'
                                               }
                                             ],
                                 'wname' => 'InstNone',
                                 'isroot' => 'true',
                                 'standalone' => 'true',
                                 'align' => undef,
                                 'name' => 'BrigInstNone'
                               },
             'BrigDirectiveVersion' => {
                                         'align' => undef,
                                         'name' => 'BrigDirectiveVersion',
                                         'enum' => 'BRIG_DIRECTIVE_VERSION',
                                         'fields' => [
                                                       {
                                                         'acc' => 'valRef',
                                                         'wtype' => 'ValRef<uint16_t>',
                                                         'wname' => 'size',
                                                         'name' => 'size',
                                                         'type' => 'uint16_t'
                                                       },
                                                       {
                                                         'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                         'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                         'type' => 'BrigDirectiveKinds16_t',
                                                         'name' => 'kind',
                                                         'enum' => 'BrigDirectiveKinds',
                                                         'wname' => 'kind'
                                                       },
                                                       {
                                                         'wtype' => 'ItemRef<Inst>',
                                                         'acc' => 'itemRef<Inst>',
                                                         'defValue' => '0',
                                                         'type' => 'BrigCodeOffset32_t',
                                                         'name' => 'code',
                                                         'wname' => 'code'
                                                       },
                                                       {
                                                         'name' => 'hsailMajor',
                                                         'type' => 'BrigVersion32_t',
                                                         'wname' => 'hsailMajor',
                                                         'enum' => 'BrigVersion',
                                                         'novisit' => 'true',
                                                         'acc' => 'enumValRef<Brig::BrigVersion,uint32_t>',
                                                         'wtype' => 'EnumValRef<Brig::BrigVersion,uint32_t>'
                                                       },
                                                       {
                                                         'acc' => 'enumValRef<Brig::BrigVersion,uint32_t>',
                                                         'wtype' => 'EnumValRef<Brig::BrigVersion,uint32_t>',
                                                         'novisit' => 'true',
                                                         'wname' => 'hsailMinor',
                                                         'enum' => 'BrigVersion',
                                                         'name' => 'hsailMinor',
                                                         'type' => 'BrigVersion32_t'
                                                       },
                                                       {
                                                         'name' => 'brigMajor',
                                                         'type' => 'BrigVersion32_t',
                                                         'wname' => 'brigMajor',
                                                         'enum' => 'BrigVersion',
                                                         'novisit' => 'true',
                                                         'acc' => 'enumValRef<Brig::BrigVersion,uint32_t>',
                                                         'wtype' => 'EnumValRef<Brig::BrigVersion,uint32_t>'
                                                       },
                                                       {
                                                         'type' => 'BrigVersion32_t',
                                                         'name' => 'brigMinor',
                                                         'enum' => 'BrigVersion',
                                                         'wname' => 'brigMinor',
                                                         'novisit' => 'true',
                                                         'wtype' => 'EnumValRef<Brig::BrigVersion,uint32_t>',
                                                         'acc' => 'enumValRef<Brig::BrigVersion,uint32_t>'
                                                       },
                                                       {
                                                         'enum' => 'BrigProfile',
                                                         'wname' => 'profile',
                                                         'type' => 'BrigProfile8_t',
                                                         'defValue' => 'Brig::BRIG_PROFILE_FULL',
                                                         'name' => 'profile',
                                                         'wtype' => 'EnumValRef<Brig::BrigProfile,uint8_t>',
                                                         'acc' => 'enumValRef<Brig::BrigProfile,uint8_t>'
                                                       },
                                                       {
                                                         'name' => 'machineModel',
                                                         'defValue' => 'Brig::BRIG_MACHINE_LARGE',
                                                         'type' => 'BrigMachineModel8_t',
                                                         'wname' => 'machineModel',
                                                         'enum' => 'BrigMachineModel',
                                                         'acc' => 'enumValRef<Brig::BrigMachineModel,uint8_t>',
                                                         'wtype' => 'EnumValRef<Brig::BrigMachineModel,uint8_t>'
                                                       },
                                                       {
                                                         'skip' => 1,
                                                         'acc' => 'valRef',
                                                         'wtype' => 'ValRef<uint16_t>',
                                                         'wname' => 'reserved',
                                                         'name' => 'reserved',
                                                         'defValue' => '0',
                                                         'type' => 'uint16_t'
                                                       }
                                                     ],
                                         'wname' => 'DirectiveVersion',
                                         'parent' => 'BrigDirectiveCode'
                                       },
             'BrigDirectiveCallableBase' => {
                                              'fields' => [
                                                            {
                                                              'wtype' => 'ValRef<uint16_t>',
                                                              'acc' => 'valRef',
                                                              'type' => 'uint16_t',
                                                              'name' => 'size',
                                                              'wname' => 'size'
                                                            },
                                                            {
                                                              'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                              'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                              'type' => 'BrigDirectiveKinds16_t',
                                                              'name' => 'kind',
                                                              'enum' => 'BrigDirectiveKinds',
                                                              'wname' => 'kind'
                                                            },
                                                            {
                                                              'type' => 'BrigCodeOffset32_t',
                                                              'defValue' => '0',
                                                              'name' => 'code',
                                                              'wname' => 'code',
                                                              'wtype' => 'ItemRef<Inst>',
                                                              'acc' => 'itemRef<Inst>'
                                                            },
                                                            {
                                                              'acc' => 'strRef',
                                                              'wtype' => 'StrRef',
                                                              'name' => 'name',
                                                              'defValue' => '0',
                                                              'type' => 'BrigStringOffset32_t',
                                                              'wname' => 'name'
                                                            },
                                                            {
                                                              'type' => 'uint16_t',
                                                              'name' => 'inArgCount',
                                                              'wname' => 'inArgCount',
                                                              'wtype' => 'ValRef<uint16_t>',
                                                              'acc' => 'valRef'
                                                            },
                                                            {
                                                              'acc' => 'valRef',
                                                              'wtype' => 'ValRef<uint16_t>',
                                                              'wname' => 'outArgCount',
                                                              'name' => 'outArgCount',
                                                              'type' => 'uint16_t'
                                                            }
                                                          ],
                                              'align' => undef,
                                              'enum' => 'BRIG_DIRECTIVE_CALLABLE_BASE',
                                              'wname' => 'DirectiveCallableBase',
                                              'children' => [
                                                              'BrigDirectiveFunction',
                                                              'BrigDirectiveExecutable',
                                                              'BrigDirectiveSignature',
                                                              'BrigDirectiveKernel'
                                                            ],
                                              'generic' => 'true',
                                              'name' => 'BrigDirectiveCallableBase',
                                              'parent' => 'BrigDirectiveCode'
                                            },
             'BrigOperandWavesize' => {
                                        'parent' => 'BrigOperand',
                                        'name' => 'BrigOperandWavesize',
                                        'align' => undef,
                                        'enum' => 'BRIG_OPERAND_WAVESIZE',
                                        'fields' => [
                                                      {
                                                        'name' => 'size',
                                                        'type' => 'uint16_t',
                                                        'wname' => 'size',
                                                        'acc' => 'valRef',
                                                        'wtype' => 'ValRef<uint16_t>'
                                                      },
                                                      {
                                                        'enum' => 'BrigOperandKinds',
                                                        'wname' => 'kind',
                                                        'type' => 'BrigOperandKinds16_t',
                                                        'name' => 'kind',
                                                        'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                        'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>'
                                                      }
                                                    ],
                                        'wname' => 'OperandWavesize'
                                      },
             'BrigInstSeg' => {
                                'parent' => 'BrigInst',
                                'fields' => [
                                              {
                                                'wtype' => 'ValRef<uint16_t>',
                                                'acc' => 'valRef',
                                                'wname' => 'size',
                                                'type' => 'uint16_t',
                                                'name' => 'size'
                                              },
                                              {
                                                'name' => 'kind',
                                                'type' => 'BrigInstKinds16_t',
                                                'wname' => 'kind',
                                                'enum' => 'BrigInstKinds',
                                                'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>'
                                              },
                                              {
                                                'type' => 'BrigOpcode16_t',
                                                'name' => 'opcode',
                                                'enum' => 'BrigOpcode',
                                                'wname' => 'opcode',
                                                'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>'
                                              },
                                              {
                                                'name' => 'type',
                                                'type' => 'BrigType16_t',
                                                'wname' => 'type',
                                                'acc' => 'valRef',
                                                'wtype' => 'ValRef<uint16_t>'
                                              },
                                              {
                                                'wname' => 'operand',
                                                'name' => 'operands',
                                                'defValue' => '0',
                                                'type' => 'BrigOperandOffset32_t',
                                                'acc' => 'itemRef<Operand>',
                                                'size' => '5',
                                                'wtype' => 'ItemRef<Operand>'
                                              },
                                              {
                                                'acc' => 'enumValRef<Brig::BrigSegment,uint8_t>',
                                                'wtype' => 'EnumValRef<Brig::BrigSegment,uint8_t>',
                                                'name' => 'segment',
                                                'defValue' => '0',
                                                'type' => 'BrigSegment8_t',
                                                'wname' => 'segment',
                                                'enum' => 'BrigSegment'
                                              },
                                              {
                                                'type' => 'uint8_t',
                                                'wtype' => 'ValRef<uint8_t>',
                                                'size' => 3,
                                                'defValue' => '0',
                                                'name' => 'reserved',
                                                'wname' => 'reserved',
                                                'acc' => 'valRef',
                                                'skip' => 1
                                              }
                                            ],
                                'wname' => 'InstSeg',
                                'enum' => 'BRIG_INST_SEG',
                                'name' => 'BrigInstSeg',
                                'align' => undef
                              },
             'BrigString' => {
                               'fields' => [
                                             {
                                               'name' => 'byteCount',
                                               'type' => 'uint32_t'
                                             },
                                             {
                                               'type' => 'uint8_t',
                                               'name' => 'bytes',
                                               'size' => '1'
                                             }
                                           ],
                               'name' => 'BrigString',
                               'align' => undef,
                               'nowrap' => 'true'
                             },
             'BrigInstMemFence' => {
                                     'parent' => 'BrigInst',
                                     'name' => 'BrigInstMemFence',
                                     'align' => undef,
                                     'enum' => 'BRIG_INST_MEM_FENCE',
                                     'wname' => 'InstMemFence',
                                     'fields' => [
                                                   {
                                                     'type' => 'uint16_t',
                                                     'name' => 'size',
                                                     'wname' => 'size',
                                                     'wtype' => 'ValRef<uint16_t>',
                                                     'acc' => 'valRef'
                                                   },
                                                   {
                                                     'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>',
                                                     'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                     'type' => 'BrigInstKinds16_t',
                                                     'name' => 'kind',
                                                     'enum' => 'BrigInstKinds',
                                                     'wname' => 'kind'
                                                   },
                                                   {
                                                     'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                     'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                     'enum' => 'BrigOpcode',
                                                     'wname' => 'opcode',
                                                     'type' => 'BrigOpcode16_t',
                                                     'name' => 'opcode'
                                                   },
                                                   {
                                                     'wtype' => 'ValRef<uint16_t>',
                                                     'acc' => 'valRef',
                                                     'type' => 'BrigType16_t',
                                                     'defValue' => 'Brig::BRIG_TYPE_NONE',
                                                     'name' => 'type',
                                                     'wname' => 'type'
                                                   },
                                                   {
                                                     'wtype' => 'ItemRef<Operand>',
                                                     'size' => '5',
                                                     'acc' => 'itemRef<Operand>',
                                                     'wname' => 'operand',
                                                     'type' => 'BrigOperandOffset32_t',
                                                     'defValue' => '0',
                                                     'name' => 'operands'
                                                   },
                                                   {
                                                     'name' => 'segments',
                                                     'type' => 'BrigMemoryFenceSegments8_t',
                                                     'wname' => 'segments',
                                                     'enum' => 'BrigMemoryFenceSegments',
                                                     'acc' => 'enumValRef<Brig::BrigMemoryFenceSegments,uint8_t>',
                                                     'wtype' => 'EnumValRef<Brig::BrigMemoryFenceSegments,uint8_t>'
                                                   },
                                                   {
                                                     'acc' => 'enumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                     'wtype' => 'EnumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                     'wname' => 'memoryOrder',
                                                     'enum' => 'BrigMemoryOrder',
                                                     'name' => 'memoryOrder',
                                                     'defValue' => 'Brig::BRIG_MEMORY_ORDER_RELAXED',
                                                     'type' => 'BrigMemoryOrder8_t'
                                                   },
                                                   {
                                                     'wname' => 'memoryScope',
                                                     'enum' => 'BrigMemoryScope',
                                                     'name' => 'memoryScope',
                                                     'defValue' => 'Brig::BRIG_MEMORY_SCOPE_SYSTEM',
                                                     'type' => 'BrigMemoryScope8_t',
                                                     'acc' => 'enumValRef<Brig::BrigMemoryScope,uint8_t>',
                                                     'wtype' => 'EnumValRef<Brig::BrigMemoryScope,uint8_t>'
                                                   },
                                                   {
                                                     'acc' => 'valRef',
                                                     'skip' => 1,
                                                     'wtype' => 'ValRef<uint8_t>',
                                                     'name' => 'reserved',
                                                     'defValue' => '0',
                                                     'type' => 'uint8_t',
                                                     'wname' => 'reserved'
                                                   }
                                                 ]
                                   },
             'BrigOperand' => {
                                'isroot' => 'true',
                                'align' => undef,
                                'fields' => [
                                              {
                                                'wtype' => 'ValRef<uint16_t>',
                                                'acc' => 'valRef',
                                                'wname' => 'size',
                                                'type' => 'uint16_t',
                                                'name' => 'size'
                                              },
                                              {
                                                'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                'type' => 'BrigOperandKinds16_t',
                                                'name' => 'kind',
                                                'enum' => 'BrigOperandKinds',
                                                'wname' => 'kind'
                                              }
                                            ],
                                'children' => [
                                                'BrigOperandVector',
                                                'BrigOperandReg',
                                                'BrigOperandSignatureRef',
                                                'BrigOperandFunctionRef',
                                                'BrigOperandArgumentList',
                                                'BrigOperandAddress',
                                                'BrigOperandWavesize',
                                                'BrigOperandRef',
                                                'BrigOperandImmed',
                                                'BrigOperandFunctionList',
                                                'BrigOperandFbarrierRef',
                                                'BrigOperandLabelRef',
                                                'BrigOperandLabelTargetsRef',
                                                'BrigOperandList',
                                                'BrigOperandLabelVariableRef'
                                              ],
                                'generic' => 'true',
                                'name' => 'BrigOperand',
                                'enum' => 'BRIG_OPERAND',
                                'wname' => 'Operand'
                              },
             'BrigInstAddr' => {
                                 'align' => undef,
                                 'name' => 'BrigInstAddr',
                                 'fields' => [
                                               {
                                                 'type' => 'uint16_t',
                                                 'name' => 'size',
                                                 'wname' => 'size',
                                                 'wtype' => 'ValRef<uint16_t>',
                                                 'acc' => 'valRef'
                                               },
                                               {
                                                 'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>',
                                                 'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                 'enum' => 'BrigInstKinds',
                                                 'wname' => 'kind',
                                                 'type' => 'BrigInstKinds16_t',
                                                 'name' => 'kind'
                                               },
                                               {
                                                 'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                 'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                 'wname' => 'opcode',
                                                 'enum' => 'BrigOpcode',
                                                 'name' => 'opcode',
                                                 'type' => 'BrigOpcode16_t'
                                               },
                                               {
                                                 'wtype' => 'ValRef<uint16_t>',
                                                 'acc' => 'valRef',
                                                 'type' => 'BrigType16_t',
                                                 'name' => 'type',
                                                 'wname' => 'type'
                                               },
                                               {
                                                 'wtype' => 'ItemRef<Operand>',
                                                 'acc' => 'itemRef<Operand>',
                                                 'size' => '5',
                                                 'wname' => 'operand',
                                                 'defValue' => '0',
                                                 'type' => 'BrigOperandOffset32_t',
                                                 'name' => 'operands'
                                               },
                                               {
                                                 'acc' => 'enumValRef<Brig::BrigSegment,uint8_t>',
                                                 'wtype' => 'EnumValRef<Brig::BrigSegment,uint8_t>',
                                                 'wname' => 'segment',
                                                 'enum' => 'BrigSegment',
                                                 'name' => 'segment',
                                                 'defValue' => '0',
                                                 'type' => 'BrigSegment8_t'
                                               },
                                               {
                                                 'wname' => 'reserved',
                                                 'defValue' => '0',
                                                 'name' => 'reserved',
                                                 'acc' => 'valRef',
                                                 'skip' => 1,
                                                 'type' => 'uint8_t',
                                                 'wtype' => 'ValRef<uint8_t>',
                                                 'size' => 3
                                               }
                                             ],
                                 'wname' => 'InstAddr',
                                 'enum' => 'BRIG_INST_ADDR',
                                 'parent' => 'BrigInst'
                               },
             'BrigDirectivePragma' => {
                                        'parent' => 'BrigDirectiveCode',
                                        'align' => undef,
                                        'name' => 'BrigDirectivePragma',
                                        'enum' => 'BRIG_DIRECTIVE_PRAGMA',
                                        'fields' => [
                                                      {
                                                        'acc' => 'valRef',
                                                        'wtype' => 'ValRef<uint16_t>',
                                                        'wname' => 'size',
                                                        'name' => 'size',
                                                        'type' => 'uint16_t'
                                                      },
                                                      {
                                                        'enum' => 'BrigDirectiveKinds',
                                                        'wname' => 'kind',
                                                        'type' => 'BrigDirectiveKinds16_t',
                                                        'name' => 'kind',
                                                        'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                        'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                      },
                                                      {
                                                        'wtype' => 'ItemRef<Inst>',
                                                        'acc' => 'itemRef<Inst>',
                                                        'wname' => 'code',
                                                        'type' => 'BrigCodeOffset32_t',
                                                        'defValue' => '0',
                                                        'name' => 'code'
                                                      },
                                                      {
                                                        'acc' => 'strRef',
                                                        'wtype' => 'StrRef',
                                                        'name' => 'name',
                                                        'type' => 'BrigStringOffset32_t',
                                                        'defValue' => '0',
                                                        'wname' => 'name'
                                                      }
                                                    ],
                                        'wname' => 'DirectivePragma'
                                      },
             'BrigDirectiveFunction' => {
                                          'comments' => [
                                                          '/// function directive.'
                                                        ],
                                          'parent' => 'BrigDirectiveExecutable',
                                          'enum' => 'BRIG_DIRECTIVE_FUNCTION',
                                          'fields' => [
                                                        {
                                                          'acc' => 'valRef',
                                                          'wtype' => 'ValRef<uint16_t>',
                                                          'name' => 'size',
                                                          'type' => 'uint16_t',
                                                          'wname' => 'size'
                                                        },
                                                        {
                                                          'name' => 'kind',
                                                          'type' => 'BrigDirectiveKinds16_t',
                                                          'wname' => 'kind',
                                                          'enum' => 'BrigDirectiveKinds',
                                                          'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                          'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                        },
                                                        {
                                                          'wtype' => 'ItemRef<Inst>',
                                                          'acc' => 'itemRef<Inst>',
                                                          'wname' => 'code',
                                                          'defValue' => '0',
                                                          'type' => 'BrigCodeOffset32_t',
                                                          'name' => 'code'
                                                        },
                                                        {
                                                          'type' => 'BrigStringOffset32_t',
                                                          'defValue' => '0',
                                                          'name' => 'name',
                                                          'wname' => 'name',
                                                          'wtype' => 'StrRef',
                                                          'acc' => 'strRef'
                                                        },
                                                        {
                                                          'type' => 'uint16_t',
                                                          'defValue' => '0',
                                                          'name' => 'inArgCount',
                                                          'wname' => 'inArgCount',
                                                          'wtype' => 'ValRef<uint16_t>',
                                                          'acc' => 'valRef'
                                                        },
                                                        {
                                                          'wtype' => 'ValRef<uint16_t>',
                                                          'acc' => 'valRef',
                                                          'wname' => 'outArgCount',
                                                          'defValue' => '0',
                                                          'type' => 'uint16_t',
                                                          'name' => 'outArgCount'
                                                        },
                                                        {
                                                          'acc' => 'itemRef<Directive>',
                                                          'wtype' => 'ItemRef<Directive>',
                                                          'name' => 'firstInArg',
                                                          'type' => 'BrigDirectiveOffset32_t',
                                                          'defValue' => '0',
                                                          'wname' => 'firstInArg'
                                                        },
                                                        {
                                                          'type' => 'BrigDirectiveOffset32_t',
                                                          'defValue' => '0',
                                                          'name' => 'firstScopedDirective',
                                                          'wname' => 'firstScopedDirective',
                                                          'wtype' => 'ItemRef<Directive>',
                                                          'acc' => 'itemRef<Directive>'
                                                        },
                                                        {
                                                          'wname' => 'nextTopLevelDirective',
                                                          'defValue' => '0',
                                                          'type' => 'BrigDirectiveOffset32_t',
                                                          'name' => 'nextTopLevelDirective',
                                                          'wtype' => 'ItemRef<Directive>',
                                                          'acc' => 'itemRef<Directive>'
                                                        },
                                                        {
                                                          'acc' => 'valRef',
                                                          'wtype' => 'ValRef<uint32_t>',
                                                          'name' => 'instCount',
                                                          'type' => 'uint32_t',
                                                          'defValue' => '0',
                                                          'wname' => 'instCount'
                                                        },
                                                        {
                                                          'wtype' => 'ExecutableModifier',
                                                          'acc' => 'subItem<ExecutableModifier>',
                                                          'type' => 'BrigExecutableModifier',
                                                          'name' => 'modifier',
                                                          'wname' => 'modifier'
                                                        },
                                                        {
                                                          'skip' => 1,
                                                          'acc' => 'valRef',
                                                          'wname' => 'reserved',
                                                          'name' => 'reserved',
                                                          'defValue' => '0',
                                                          'size' => 3,
                                                          'wtype' => 'ValRef<uint8_t>',
                                                          'type' => 'uint8_t'
                                                        }
                                                      ],
                                          'wname' => 'DirectiveFunction',
                                          'name' => 'BrigDirectiveFunction',
                                          'align' => undef
                                        },
             'BrigDirectiveArgScopeStart' => {
                                               'align' => undef,
                                               'name' => 'BrigDirectiveArgScopeStart',
                                               'wname' => 'DirectiveArgScopeStart',
                                               'fields' => [
                                                             {
                                                               'wname' => 'size',
                                                               'type' => 'uint16_t',
                                                               'name' => 'size',
                                                               'wtype' => 'ValRef<uint16_t>',
                                                               'acc' => 'valRef'
                                                             },
                                                             {
                                                               'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                               'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                               'type' => 'BrigDirectiveKinds16_t',
                                                               'name' => 'kind',
                                                               'enum' => 'BrigDirectiveKinds',
                                                               'wname' => 'kind'
                                                             },
                                                             {
                                                               'wname' => 'code',
                                                               'name' => 'code',
                                                               'type' => 'BrigCodeOffset32_t',
                                                               'defValue' => '0',
                                                               'acc' => 'itemRef<Inst>',
                                                               'wtype' => 'ItemRef<Inst>'
                                                             }
                                                           ],
                                               'enum' => 'BRIG_DIRECTIVE_ARG_SCOPE_START',
                                               'parent' => 'BrigDirectiveCode'
                                             },
             'BrigDirectiveImageInit' => {
                                           'parent' => 'BrigDirectiveOpaqueInit',
                                           'enum' => 'BRIG_DIRECTIVE_IMAGE_INIT',
                                           'wname' => 'DirectiveImageInit',
                                           'fields' => [
                                                         {
                                                           'acc' => 'valRef',
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'name' => 'size',
                                                           'type' => 'uint16_t',
                                                           'wname' => 'size'
                                                         },
                                                         {
                                                           'enum' => 'BrigDirectiveKinds',
                                                           'wname' => 'kind',
                                                           'type' => 'BrigDirectiveKinds16_t',
                                                           'name' => 'kind',
                                                           'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                           'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                         },
                                                         {
                                                           'name' => 'code',
                                                           'defValue' => '0',
                                                           'type' => 'BrigCodeOffset32_t',
                                                           'wname' => 'code',
                                                           'acc' => 'itemRef<Inst>',
                                                           'wtype' => 'ItemRef<Inst>'
                                                         },
                                                         {
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'acc' => 'valRef',
                                                           'wname' => 'imageCount',
                                                           'type' => 'uint16_t',
                                                           'defValue' => '0',
                                                           'name' => 'imageCount'
                                                         },
                                                         {
                                                           'name' => 'elementCount',
                                                           'type' => 'uint16_t',
                                                           'wname' => 'elementCount',
                                                           'phantomof' => $structs->{'BrigDirectiveImageInit'}{'fields'}[3],
                                                           'acc' => 'valRef',
                                                           'wtype' => 'ValRef<uint16_t>'
                                                         },
                                                         {
                                                           'type' => 'uint16_t',
                                                           'defValue' => '0',
                                                           'name' => 'reserved',
                                                           'wname' => 'reserved',
                                                           'wtype' => 'ValRef<uint16_t>',
                                                           'skip' => 1,
                                                           'acc' => 'valRef'
                                                         },
                                                         {
                                                           'acc' => 'itemRef<DirectiveImageProperties>',
                                                           'wname' => 'images',
                                                           'name' => 'images',
                                                           'defValue' => '0',
                                                           'size' => '1',
                                                           'wtype' => 'ItemRef<DirectiveImageProperties>',
                                                           'wspecial' => 'ImageInitList',
                                                           'type' => 'BrigDirectiveOffset32_t'
                                                         }
                                                       ],
                                           'name' => 'BrigDirectiveImageInit',
                                           'align' => undef
                                         },
             'BrigDirectiveLoc' => {
                                     'parent' => 'BrigDirectiveCode',
                                     'fields' => [
                                                   {
                                                     'type' => 'uint16_t',
                                                     'name' => 'size',
                                                     'wname' => 'size',
                                                     'wtype' => 'ValRef<uint16_t>',
                                                     'acc' => 'valRef'
                                                   },
                                                   {
                                                     'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                     'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                     'name' => 'kind',
                                                     'type' => 'BrigDirectiveKinds16_t',
                                                     'wname' => 'kind',
                                                     'enum' => 'BrigDirectiveKinds'
                                                   },
                                                   {
                                                     'type' => 'BrigCodeOffset32_t',
                                                     'defValue' => '0',
                                                     'name' => 'code',
                                                     'wname' => 'code',
                                                     'wtype' => 'ItemRef<Inst>',
                                                     'acc' => 'itemRef<Inst>'
                                                   },
                                                   {
                                                     'defValue' => '0',
                                                     'type' => 'BrigStringOffset32_t',
                                                     'name' => 'filename',
                                                     'wname' => 'filename',
                                                     'wtype' => 'StrRef',
                                                     'acc' => 'strRef'
                                                   },
                                                   {
                                                     'acc' => 'valRef',
                                                     'wtype' => 'ValRef<uint32_t>',
                                                     'name' => 'line',
                                                     'type' => 'uint32_t',
                                                     'wname' => 'line'
                                                   },
                                                   {
                                                     'wname' => 'column',
                                                     'name' => 'column',
                                                     'defValue' => '1',
                                                     'type' => 'uint32_t',
                                                     'acc' => 'valRef',
                                                     'wtype' => 'ValRef<uint32_t>'
                                                   }
                                                 ],
                                     'wname' => 'DirectiveLoc',
                                     'enum' => 'BRIG_DIRECTIVE_LOC',
                                     'name' => 'BrigDirectiveLoc',
                                     'align' => undef
                                   },
             'BrigDirectiveSamplerInit' => {
                                             'parent' => 'BrigDirectiveOpaqueInit',
                                             'align' => undef,
                                             'name' => 'BrigDirectiveSamplerInit',
                                             'wname' => 'DirectiveSamplerInit',
                                             'fields' => [
                                                           {
                                                             'name' => 'size',
                                                             'type' => 'uint16_t',
                                                             'wname' => 'size',
                                                             'acc' => 'valRef',
                                                             'wtype' => 'ValRef<uint16_t>'
                                                           },
                                                           {
                                                             'type' => 'BrigDirectiveKinds16_t',
                                                             'name' => 'kind',
                                                             'enum' => 'BrigDirectiveKinds',
                                                             'wname' => 'kind',
                                                             'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                             'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                           },
                                                           {
                                                             'wname' => 'code',
                                                             'name' => 'code',
                                                             'type' => 'BrigCodeOffset32_t',
                                                             'defValue' => '0',
                                                             'acc' => 'itemRef<Inst>',
                                                             'wtype' => 'ItemRef<Inst>'
                                                           },
                                                           {
                                                             'wtype' => 'ValRef<uint16_t>',
                                                             'acc' => 'valRef',
                                                             'defValue' => '0',
                                                             'type' => 'uint16_t',
                                                             'name' => 'samplerCount',
                                                             'wname' => 'samplerCount'
                                                           },
                                                           {
                                                             'wname' => 'elementCount',
                                                             'phantomof' => $structs->{'BrigDirectiveSamplerInit'}{'fields'}[3],
                                                             'name' => 'elementCount',
                                                             'type' => 'uint16_t',
                                                             'acc' => 'valRef',
                                                             'wtype' => 'ValRef<uint16_t>'
                                                           },
                                                           {
                                                             'acc' => 'valRef',
                                                             'skip' => 1,
                                                             'wtype' => 'ValRef<uint16_t>',
                                                             'wname' => 'reserved',
                                                             'name' => 'reserved',
                                                             'defValue' => '0',
                                                             'type' => 'uint16_t'
                                                           },
                                                           {
                                                             'name' => 'samplers',
                                                             'defValue' => '0',
                                                             'wname' => 'samplers',
                                                             'acc' => 'itemRef<DirectiveSamplerProperties>',
                                                             'type' => 'BrigDirectiveOffset32_t',
                                                             'wspecial' => 'SamplerInitList',
                                                             'size' => '1',
                                                             'wtype' => 'ItemRef<DirectiveSamplerProperties>'
                                                           }
                                                         ],
                                             'enum' => 'BRIG_DIRECTIVE_SAMPLER_INIT'
                                           },
             'BrigSymbolModifier' => {
                                       'isroot' => 'true',
                                       'standalone' => 'true',
                                       'name' => 'BrigSymbolModifier',
                                       'align' => undef,
                                       'enum' => 'BRIG_SYMBOL_MODIFIER',
                                       'fields' => [
                                                     {
                                                       'wtype' => 'ValRef<uint8_t>',
                                                       'acc' => 'valRef',
                                                       'wname' => 'allBits',
                                                       'defValue' => '0',
                                                       'type' => 'BrigSymbolModifier8_t',
                                                       'name' => 'allBits'
                                                     },
                                                     {
                                                       'wtype' => 'BFValRef<Brig::BrigLinkage8_t,0,2>',
                                                       'acc' => 'bFValRef<Brig::BrigLinkage8_t,0,2>',
                                                       'phantomof' => $structs->{'BrigSymbolModifier'}{'fields'}[0],
                                                       'wname' => 'linkage',
                                                       'type' => 'BrigLinkage',
                                                       'name' => 'linkage'
                                                     },
                                                     {
                                                       'name' => 'isDeclaration',
                                                       'type' => 'bool',
                                                       'wname' => 'isDeclaration',
                                                       'phantomof' => $structs->{'BrigSymbolModifier'}{'fields'}[0],
                                                       'acc' => 'bitValRef<2>',
                                                       'wtype' => 'BitValRef<2>'
                                                     },
                                                     {
                                                       'acc' => 'bitValRef<3>',
                                                       'wtype' => 'BitValRef<3>',
                                                       'wname' => 'isConst',
                                                       'phantomof' => $structs->{'BrigSymbolModifier'}{'fields'}[0],
                                                       'name' => 'isConst',
                                                       'type' => 'bool'
                                                     },
                                                     {
                                                       'wname' => 'isArray',
                                                       'phantomof' => $structs->{'BrigSymbolModifier'}{'fields'}[0],
                                                       'name' => 'isArray',
                                                       'type' => 'bool',
                                                       'acc' => 'bitValRef<4>',
                                                       'wtype' => 'BitValRef<4>'
                                                     },
                                                     {
                                                       'type' => 'bool',
                                                       'name' => 'isFlexArray',
                                                       'phantomof' => $structs->{'BrigSymbolModifier'}{'fields'}[0],
                                                       'wname' => 'isFlexArray',
                                                       'wtype' => 'BitValRef<5>',
                                                       'acc' => 'bitValRef<5>'
                                                     }
                                                   ],
                                       'wname' => 'SymbolModifier'
                                     },
             'BrigInstImage' => {
                                  'parent' => 'BrigInst',
                                  'wname' => 'InstImage',
                                  'fields' => [
                                                {
                                                  'acc' => 'valRef',
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'wname' => 'size',
                                                  'name' => 'size',
                                                  'type' => 'uint16_t'
                                                },
                                                {
                                                  'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>',
                                                  'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                  'enum' => 'BrigInstKinds',
                                                  'wname' => 'kind',
                                                  'type' => 'BrigInstKinds16_t',
                                                  'name' => 'kind'
                                                },
                                                {
                                                  'name' => 'opcode',
                                                  'type' => 'BrigOpcode16_t',
                                                  'wname' => 'opcode',
                                                  'enum' => 'BrigOpcode',
                                                  'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                  'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>'
                                                },
                                                {
                                                  'type' => 'BrigType16_t',
                                                  'name' => 'type',
                                                  'wname' => 'type',
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'acc' => 'valRef'
                                                },
                                                {
                                                  'acc' => 'itemRef<Operand>',
                                                  'size' => '5',
                                                  'wtype' => 'ItemRef<Operand>',
                                                  'wname' => 'operand',
                                                  'name' => 'operands',
                                                  'defValue' => '0',
                                                  'type' => 'BrigOperandOffset32_t'
                                                },
                                                {
                                                  'wname' => 'imageType',
                                                  'type' => 'BrigType16_t',
                                                  'name' => 'imageType',
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'acc' => 'valRef'
                                                },
                                                {
                                                  'acc' => 'valRef',
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'name' => 'coordType',
                                                  'type' => 'BrigType16_t',
                                                  'wname' => 'coordType'
                                                },
                                                {
                                                  'acc' => 'enumValRef<Brig::BrigImageGeometry,uint8_t>',
                                                  'wtype' => 'EnumValRef<Brig::BrigImageGeometry,uint8_t>',
                                                  'name' => 'geometry',
                                                  'type' => 'BrigImageGeometry8_t',
                                                  'defValue' => 'Brig::BRIG_GEOMETRY_UNKNOWN',
                                                  'wname' => 'geometry',
                                                  'enum' => 'BrigImageGeometry'
                                                },
                                                {
                                                  'name' => 'equivClass',
                                                  'type' => 'uint8_t',
                                                  'wname' => 'equivClass',
                                                  'acc' => 'valRef',
                                                  'wtype' => 'ValRef<uint8_t>'
                                                },
                                                {
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'acc' => 'valRef',
                                                  'skip' => 1,
                                                  'defValue' => '0',
                                                  'type' => 'uint16_t',
                                                  'name' => 'reserved',
                                                  'wname' => 'reserved'
                                                }
                                              ],
                                  'enum' => 'BRIG_INST_IMAGE',
                                  'name' => 'BrigInstImage',
                                  'align' => undef
                                },
             'BrigInstMem' => {
                                'name' => 'BrigInstMem',
                                'align' => undef,
                                'enum' => 'BRIG_INST_MEM',
                                'wname' => 'InstMem',
                                'fields' => [
                                              {
                                                'wtype' => 'ValRef<uint16_t>',
                                                'acc' => 'valRef',
                                                'type' => 'uint16_t',
                                                'name' => 'size',
                                                'wname' => 'size'
                                              },
                                              {
                                                'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>',
                                                'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                'type' => 'BrigInstKinds16_t',
                                                'name' => 'kind',
                                                'enum' => 'BrigInstKinds',
                                                'wname' => 'kind'
                                              },
                                              {
                                                'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                'wname' => 'opcode',
                                                'enum' => 'BrigOpcode',
                                                'name' => 'opcode',
                                                'type' => 'BrigOpcode16_t'
                                              },
                                              {
                                                'acc' => 'valRef',
                                                'wtype' => 'ValRef<uint16_t>',
                                                'name' => 'type',
                                                'type' => 'BrigType16_t',
                                                'wname' => 'type'
                                              },
                                              {
                                                'size' => '5',
                                                'acc' => 'itemRef<Operand>',
                                                'wtype' => 'ItemRef<Operand>',
                                                'wname' => 'operand',
                                                'name' => 'operands',
                                                'defValue' => '0',
                                                'type' => 'BrigOperandOffset32_t'
                                              },
                                              {
                                                'enum' => 'BrigSegment',
                                                'wname' => 'segment',
                                                'type' => 'BrigSegment8_t',
                                                'defValue' => '0',
                                                'name' => 'segment',
                                                'wtype' => 'EnumValRef<Brig::BrigSegment,uint8_t>',
                                                'acc' => 'enumValRef<Brig::BrigSegment,uint8_t>'
                                              },
                                              {
                                                'wtype' => 'EnumValRef<Brig::BrigAlignment,uint8_t>',
                                                'acc' => 'enumValRef<Brig::BrigAlignment,uint8_t>',
                                                'type' => 'BrigAlignment8_t',
                                                'name' => 'align',
                                                'enum' => 'BrigAlignment',
                                                'wname' => 'align'
                                              },
                                              {
                                                'wname' => 'equivClass',
                                                'name' => 'equivClass',
                                                'type' => 'uint8_t',
                                                'acc' => 'valRef',
                                                'wtype' => 'ValRef<uint8_t>'
                                              },
                                              {
                                                'wtype' => 'EnumValRef<Brig::BrigWidth,uint8_t>',
                                                'acc' => 'enumValRef<Brig::BrigWidth,uint8_t>',
                                                'enum' => 'BrigWidth',
                                                'wname' => 'width',
                                                'type' => 'BrigWidth8_t',
                                                'name' => 'width'
                                              },
                                              {
                                                'name' => 'modifier',
                                                'type' => 'BrigMemoryModifier',
                                                'wname' => 'modifier',
                                                'acc' => 'subItem<MemoryModifier>',
                                                'wtype' => 'MemoryModifier'
                                              },
                                              {
                                                'acc' => 'valRef',
                                                'skip' => 1,
                                                'wname' => 'reserved',
                                                'defValue' => '0',
                                                'name' => 'reserved',
                                                'wtype' => 'ValRef<uint8_t>',
                                                'size' => 3,
                                                'type' => 'uint8_t'
                                              }
                                            ],
                                'parent' => 'BrigInst'
                              },
             'BrigOperandSignatureRef' => {
                                            'parent' => 'BrigOperandRef',
                                            'enum' => 'BRIG_OPERAND_SIGNATURE_REF',
                                            'wname' => 'OperandSignatureRef',
                                            'fields' => [
                                                          {
                                                            'name' => 'size',
                                                            'type' => 'uint16_t',
                                                            'wname' => 'size',
                                                            'acc' => 'valRef',
                                                            'wtype' => 'ValRef<uint16_t>'
                                                          },
                                                          {
                                                            'type' => 'BrigOperandKinds16_t',
                                                            'name' => 'kind',
                                                            'enum' => 'BrigOperandKinds',
                                                            'wname' => 'kind',
                                                            'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                            'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>'
                                                          },
                                                          {
                                                            'defValue' => '0',
                                                            'type' => 'BrigDirectiveOffset32_t',
                                                            'name' => 'ref',
                                                            'wname' => 'sig',
                                                            'wtype' => 'ItemRef<DirectiveCallableBase>',
                                                            'comments' => [
                                                                            '// overridden, was ItemRef<Directive> ref'
                                                                          ],
                                                            'acc' => 'itemRef<DirectiveCallableBase>'
                                                          }
                                                        ],
                                            'align' => undef,
                                            'name' => 'BrigOperandSignatureRef'
                                          },
             'BrigInstBasic' => {
                                  'parent' => 'BrigInst',
                                  'name' => 'BrigInstBasic',
                                  'align' => undef,
                                  'wname' => 'InstBasic',
                                  'fields' => [
                                                {
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'acc' => 'valRef',
                                                  'wname' => 'size',
                                                  'type' => 'uint16_t',
                                                  'name' => 'size'
                                                },
                                                {
                                                  'wname' => 'kind',
                                                  'enum' => 'BrigInstKinds',
                                                  'name' => 'kind',
                                                  'type' => 'BrigInstKinds16_t',
                                                  'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                  'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>'
                                                },
                                                {
                                                  'wname' => 'opcode',
                                                  'enum' => 'BrigOpcode',
                                                  'name' => 'opcode',
                                                  'type' => 'BrigOpcode16_t',
                                                  'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                  'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>'
                                                },
                                                {
                                                  'wname' => 'type',
                                                  'name' => 'type',
                                                  'type' => 'BrigType16_t',
                                                  'acc' => 'valRef',
                                                  'wtype' => 'ValRef<uint16_t>'
                                                },
                                                {
                                                  'wtype' => 'ItemRef<Operand>',
                                                  'acc' => 'itemRef<Operand>',
                                                  'size' => '5',
                                                  'wname' => 'operand',
                                                  'type' => 'BrigOperandOffset32_t',
                                                  'defValue' => '0',
                                                  'name' => 'operands'
                                                }
                                              ],
                                  'enum' => 'BRIG_INST_BASIC'
                                },
             'BrigDirectiveVariable' => {
                                          'align' => undef,
                                          'name' => 'BrigDirectiveVariable',
                                          'enum' => 'BRIG_DIRECTIVE_VARIABLE',
                                          'wname' => 'DirectiveVariable',
                                          'fields' => [
                                                        {
                                                          'name' => 'size',
                                                          'type' => 'uint16_t',
                                                          'wname' => 'size',
                                                          'acc' => 'valRef',
                                                          'wtype' => 'ValRef<uint16_t>'
                                                        },
                                                        {
                                                          'name' => 'kind',
                                                          'type' => 'BrigDirectiveKinds16_t',
                                                          'wname' => 'kind',
                                                          'enum' => 'BrigDirectiveKinds',
                                                          'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                          'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                        },
                                                        {
                                                          'wname' => 'code',
                                                          'name' => 'code',
                                                          'defValue' => '0',
                                                          'type' => 'BrigCodeOffset32_t',
                                                          'acc' => 'itemRef<Inst>',
                                                          'wtype' => 'ItemRef<Inst>'
                                                        },
                                                        {
                                                          'wname' => 'name',
                                                          'defValue' => '0',
                                                          'type' => 'BrigStringOffset32_t',
                                                          'name' => 'name',
                                                          'wtype' => 'StrRef',
                                                          'acc' => 'strRef'
                                                        },
                                                        {
                                                          'acc' => 'itemRef<Directive>',
                                                          'wtype' => 'ItemRef<Directive>',
                                                          'name' => 'init',
                                                          'defValue' => '0',
                                                          'type' => 'BrigDirectiveOffset32_t',
                                                          'wname' => 'init'
                                                        },
                                                        {
                                                          'wname' => 'type',
                                                          'type' => 'BrigType16_t',
                                                          'name' => 'type',
                                                          'wtype' => 'ValRef<uint16_t>',
                                                          'acc' => 'valRef'
                                                        },
                                                        {
                                                          'wname' => 'segment',
                                                          'enum' => 'BrigSegment',
                                                          'name' => 'segment',
                                                          'defValue' => '0',
                                                          'type' => 'BrigSegment8_t',
                                                          'acc' => 'enumValRef<Brig::BrigSegment,uint8_t>',
                                                          'wtype' => 'EnumValRef<Brig::BrigSegment,uint8_t>'
                                                        },
                                                        {
                                                          'defValue' => '0',
                                                          'type' => 'BrigAlignment8_t',
                                                          'name' => 'align',
                                                          'enum' => 'BrigAlignment',
                                                          'wname' => 'align',
                                                          'wtype' => 'EnumValRef<Brig::BrigAlignment,uint8_t>',
                                                          'acc' => 'enumValRef<Brig::BrigAlignment,uint8_t>'
                                                        },
                                                        {
                                                          'wname' => 'dimLo',
                                                          'defValue' => '0',
                                                          'type' => 'uint32_t',
                                                          'name' => 'dimLo',
                                                          'wtype' => 'ValRef<uint32_t>',
                                                          'acc' => 'valRef'
                                                        },
                                                        {
                                                          'type' => 'uint64_t',
                                                          'name' => 'dim',
                                                          'phantomof' => $structs->{'BrigDirectiveVariable'}{'fields'}[8],
                                                          'wname' => 'dim',
                                                          'wtype' => 'ValRef<uint64_t>',
                                                          'acc' => 'reinterpretValRef<uint64_t>'
                                                        },
                                                        {
                                                          'name' => 'dimHi',
                                                          'type' => 'uint32_t',
                                                          'defValue' => '0',
                                                          'wname' => 'dimHi',
                                                          'acc' => 'valRef',
                                                          'wtype' => 'ValRef<uint32_t>'
                                                        },
                                                        {
                                                          'type' => 'BrigSymbolModifier',
                                                          'name' => 'modifier',
                                                          'wname' => 'modifier',
                                                          'wtype' => 'SymbolModifier',
                                                          'acc' => 'subItem<SymbolModifier>'
                                                        },
                                                        {
                                                          'type' => 'uint8_t',
                                                          'wtype' => 'ValRef<uint8_t>',
                                                          'size' => 3,
                                                          'wname' => 'reserved',
                                                          'defValue' => '0',
                                                          'name' => 'reserved',
                                                          'skip' => 1,
                                                          'acc' => 'valRef'
                                                        }
                                                      ],
                                          'parent' => 'BrigDirectiveCode'
                                        },
             'BrigDirectiveLabelTargets' => {
                                              'fields' => [
                                                            {
                                                              'acc' => 'valRef',
                                                              'wtype' => 'ValRef<uint16_t>',
                                                              'wname' => 'size',
                                                              'name' => 'size',
                                                              'type' => 'uint16_t'
                                                            },
                                                            {
                                                              'enum' => 'BrigDirectiveKinds',
                                                              'wname' => 'kind',
                                                              'type' => 'BrigDirectiveKinds16_t',
                                                              'name' => 'kind',
                                                              'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                              'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                            },
                                                            {
                                                              'type' => 'BrigCodeOffset32_t',
                                                              'defValue' => '0',
                                                              'name' => 'code',
                                                              'wname' => 'code',
                                                              'wtype' => 'ItemRef<Inst>',
                                                              'acc' => 'itemRef<Inst>'
                                                            },
                                                            {
                                                              'wname' => 'name',
                                                              'name' => 'name',
                                                              'defValue' => '0',
                                                              'type' => 'BrigStringOffset32_t',
                                                              'acc' => 'strRef',
                                                              'wtype' => 'StrRef'
                                                            },
                                                            {
                                                              'wname' => 'elementCount',
                                                              'name' => 'labelCount',
                                                              'defValue' => '0',
                                                              'type' => 'uint16_t',
                                                              'acc' => 'valRef',
                                                              'wtype' => 'ValRef<uint16_t>'
                                                            },
                                                            {
                                                              'wname' => 'reserved',
                                                              'type' => 'uint16_t',
                                                              'defValue' => '0',
                                                              'name' => 'reserved',
                                                              'wtype' => 'ValRef<uint16_t>',
                                                              'acc' => 'valRef',
                                                              'skip' => 1
                                                            },
                                                            {
                                                              'acc' => 'itemRef<DirectiveLabel>',
                                                              'wname' => 'labels',
                                                              'defValue' => '0',
                                                              'name' => 'labels',
                                                              'wtype' => 'ItemRef<DirectiveLabel>',
                                                              'size' => '1',
                                                              'wspecial' => 'LabelTargetsList',
                                                              'type' => 'BrigDirectiveOffset32_t'
                                                            }
                                                          ],
                                              'wname' => 'DirectiveLabelTargets',
                                              'enum' => 'BRIG_DIRECTIVE_LABEL_TARGETS',
                                              'name' => 'BrigDirectiveLabelTargets',
                                              'align' => undef,
                                              'parent' => 'BrigDirectiveCode'
                                            },
             'BrigExecutableModifier' => {
                                           'standalone' => 'true',
                                           'isroot' => 'true',
                                           'align' => undef,
                                           'name' => 'BrigExecutableModifier',
                                           'enum' => 'BRIG_EXECUTABLE_MODIFIER',
                                           'wname' => 'ExecutableModifier',
                                           'fields' => [
                                                         {
                                                           'wtype' => 'ValRef<uint8_t>',
                                                           'acc' => 'valRef',
                                                           'defValue' => '0',
                                                           'type' => 'BrigExecutableModifier8_t',
                                                           'name' => 'allBits',
                                                           'wname' => 'allBits'
                                                         },
                                                         {
                                                           'wtype' => 'BFValRef<Brig::BrigLinkage8_t,0,2>',
                                                           'acc' => 'bFValRef<Brig::BrigLinkage8_t,0,2>',
                                                           'phantomof' => $structs->{'BrigExecutableModifier'}{'fields'}[0],
                                                           'wname' => 'linkage',
                                                           'type' => 'BrigLinkage',
                                                           'name' => 'linkage'
                                                         },
                                                         {
                                                           'wtype' => 'BitValRef<2>',
                                                           'acc' => 'bitValRef<2>',
                                                           'phantomof' => $structs->{'BrigExecutableModifier'}{'fields'}[0],
                                                           'wname' => 'isDeclaration',
                                                           'type' => 'bool',
                                                           'name' => 'isDeclaration'
                                                         }
                                                       ]
                                         },
             'BrigDirectiveKernel' => {
                                        'enum' => 'BRIG_DIRECTIVE_KERNEL',
                                        'wname' => 'DirectiveKernel',
                                        'fields' => [
                                                      {
                                                        'wname' => 'size',
                                                        'name' => 'size',
                                                        'type' => 'uint16_t',
                                                        'acc' => 'valRef',
                                                        'wtype' => 'ValRef<uint16_t>'
                                                      },
                                                      {
                                                        'type' => 'BrigDirectiveKinds16_t',
                                                        'name' => 'kind',
                                                        'enum' => 'BrigDirectiveKinds',
                                                        'wname' => 'kind',
                                                        'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                        'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                      },
                                                      {
                                                        'acc' => 'itemRef<Inst>',
                                                        'wtype' => 'ItemRef<Inst>',
                                                        'name' => 'code',
                                                        'type' => 'BrigCodeOffset32_t',
                                                        'defValue' => '0',
                                                        'wname' => 'code'
                                                      },
                                                      {
                                                        'wtype' => 'StrRef',
                                                        'acc' => 'strRef',
                                                        'defValue' => '0',
                                                        'type' => 'BrigStringOffset32_t',
                                                        'name' => 'name',
                                                        'wname' => 'name'
                                                      },
                                                      {
                                                        'acc' => 'valRef',
                                                        'wtype' => 'ValRef<uint16_t>',
                                                        'name' => 'inArgCount',
                                                        'defValue' => '0',
                                                        'type' => 'uint16_t',
                                                        'wname' => 'inArgCount'
                                                      },
                                                      {
                                                        'wtype' => 'ValRef<uint16_t>',
                                                        'acc' => 'valRef',
                                                        'wname' => 'outArgCount',
                                                        'type' => 'uint16_t',
                                                        'defValue' => '0',
                                                        'name' => 'outArgCount'
                                                      },
                                                      {
                                                        'defValue' => '0',
                                                        'type' => 'BrigDirectiveOffset32_t',
                                                        'name' => 'firstInArg',
                                                        'wname' => 'firstInArg',
                                                        'wtype' => 'ItemRef<Directive>',
                                                        'acc' => 'itemRef<Directive>'
                                                      },
                                                      {
                                                        'wname' => 'firstScopedDirective',
                                                        'type' => 'BrigDirectiveOffset32_t',
                                                        'defValue' => '0',
                                                        'name' => 'firstScopedDirective',
                                                        'wtype' => 'ItemRef<Directive>',
                                                        'acc' => 'itemRef<Directive>'
                                                      },
                                                      {
                                                        'name' => 'nextTopLevelDirective',
                                                        'type' => 'BrigDirectiveOffset32_t',
                                                        'defValue' => '0',
                                                        'wname' => 'nextTopLevelDirective',
                                                        'acc' => 'itemRef<Directive>',
                                                        'wtype' => 'ItemRef<Directive>'
                                                      },
                                                      {
                                                        'name' => 'instCount',
                                                        'type' => 'uint32_t',
                                                        'defValue' => '0',
                                                        'wname' => 'instCount',
                                                        'acc' => 'valRef',
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
                                                        'type' => 'uint8_t',
                                                        'size' => 3,
                                                        'wtype' => 'ValRef<uint8_t>',
                                                        'wname' => 'reserved',
                                                        'name' => 'reserved',
                                                        'defValue' => '0',
                                                        'acc' => 'valRef',
                                                        'skip' => 1
                                                      }
                                                    ],
                                        'align' => undef,
                                        'name' => 'BrigDirectiveKernel',
                                        'comments' => [
                                                        '/// kernel directive.'
                                                      ],
                                        'parent' => 'BrigDirectiveExecutable'
                                      },
             'BrigInstSignal' => {
                                   'name' => 'BrigInstSignal',
                                   'align' => undef,
                                   'wname' => 'InstSignal',
                                   'fields' => [
                                                 {
                                                   'acc' => 'valRef',
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'wname' => 'size',
                                                   'name' => 'size',
                                                   'type' => 'uint16_t'
                                                 },
                                                 {
                                                   'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                   'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>',
                                                   'wname' => 'kind',
                                                   'enum' => 'BrigInstKinds',
                                                   'name' => 'kind',
                                                   'type' => 'BrigInstKinds16_t'
                                                 },
                                                 {
                                                   'name' => 'opcode',
                                                   'type' => 'BrigOpcode16_t',
                                                   'wname' => 'opcode',
                                                   'enum' => 'BrigOpcode',
                                                   'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                   'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>'
                                                 },
                                                 {
                                                   'acc' => 'valRef',
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'name' => 'type',
                                                   'type' => 'BrigType16_t',
                                                   'wname' => 'type'
                                                 },
                                                 {
                                                   'wname' => 'operand',
                                                   'name' => 'operands',
                                                   'type' => 'BrigOperandOffset32_t',
                                                   'defValue' => '0',
                                                   'size' => '5',
                                                   'acc' => 'itemRef<Operand>',
                                                   'wtype' => 'ItemRef<Operand>'
                                                 },
                                                 {
                                                   'name' => 'signalType',
                                                   'type' => 'BrigType16_t',
                                                   'wname' => 'signalType',
                                                   'acc' => 'valRef',
                                                   'wtype' => 'ValRef<uint16_t>'
                                                 },
                                                 {
                                                   'wtype' => 'EnumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                   'acc' => 'enumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                   'type' => 'BrigMemoryOrder8_t',
                                                   'defValue' => 'Brig::BRIG_MEMORY_ORDER_RELAXED',
                                                   'name' => 'memoryOrder',
                                                   'enum' => 'BrigMemoryOrder',
                                                   'wname' => 'memoryOrder'
                                                 },
                                                 {
                                                   'wtype' => 'EnumValRef<Brig::BrigAtomicOperation,uint8_t>',
                                                   'acc' => 'enumValRef<Brig::BrigAtomicOperation,uint8_t>',
                                                   'type' => 'BrigAtomicOperation8_t',
                                                   'name' => 'signalOperation',
                                                   'enum' => 'BrigAtomicOperation',
                                                   'wname' => 'signalOperation'
                                                 }
                                               ],
                                   'enum' => 'BRIG_INST_SIGNAL',
                                   'parent' => 'BrigInst'
                                 },
             'BrigInstBr' => {
                               'parent' => 'BrigInst',
                               'name' => 'BrigInstBr',
                               'align' => undef,
                               'wname' => 'InstBr',
                               'fields' => [
                                             {
                                               'wname' => 'size',
                                               'name' => 'size',
                                               'type' => 'uint16_t',
                                               'acc' => 'valRef',
                                               'wtype' => 'ValRef<uint16_t>'
                                             },
                                             {
                                               'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                               'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>',
                                               'wname' => 'kind',
                                               'enum' => 'BrigInstKinds',
                                               'name' => 'kind',
                                               'type' => 'BrigInstKinds16_t'
                                             },
                                             {
                                               'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                               'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                               'enum' => 'BrigOpcode',
                                               'wname' => 'opcode',
                                               'type' => 'BrigOpcode16_t',
                                               'name' => 'opcode'
                                             },
                                             {
                                               'name' => 'type',
                                               'type' => 'BrigType16_t',
                                               'wname' => 'type',
                                               'acc' => 'valRef',
                                               'wtype' => 'ValRef<uint16_t>'
                                             },
                                             {
                                               'wtype' => 'ItemRef<Operand>',
                                               'size' => '5',
                                               'acc' => 'itemRef<Operand>',
                                               'wname' => 'operand',
                                               'type' => 'BrigOperandOffset32_t',
                                               'defValue' => '0',
                                               'name' => 'operands'
                                             },
                                             {
                                               'acc' => 'enumValRef<Brig::BrigWidth,uint8_t>',
                                               'wtype' => 'EnumValRef<Brig::BrigWidth,uint8_t>',
                                               'name' => 'width',
                                               'type' => 'BrigWidth8_t',
                                               'wname' => 'width',
                                               'enum' => 'BrigWidth'
                                             },
                                             {
                                               'wtype' => 'ValRef<uint8_t>',
                                               'size' => 3,
                                               'type' => 'uint8_t',
                                               'acc' => 'valRef',
                                               'skip' => 1,
                                               'defValue' => '0',
                                               'name' => 'reserved',
                                               'wname' => 'reserved'
                                             }
                                           ],
                               'enum' => 'BRIG_INST_BR'
                             },
             'BrigInstSourceType' => {
                                       'parent' => 'BrigInst',
                                       'name' => 'BrigInstSourceType',
                                       'align' => undef,
                                       'wname' => 'InstSourceType',
                                       'fields' => [
                                                     {
                                                       'wname' => 'size',
                                                       'name' => 'size',
                                                       'type' => 'uint16_t',
                                                       'acc' => 'valRef',
                                                       'wtype' => 'ValRef<uint16_t>'
                                                     },
                                                     {
                                                       'wname' => 'kind',
                                                       'enum' => 'BrigInstKinds',
                                                       'name' => 'kind',
                                                       'type' => 'BrigInstKinds16_t',
                                                       'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                       'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>'
                                                     },
                                                     {
                                                       'wname' => 'opcode',
                                                       'enum' => 'BrigOpcode',
                                                       'name' => 'opcode',
                                                       'type' => 'BrigOpcode16_t',
                                                       'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                       'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>'
                                                     },
                                                     {
                                                       'acc' => 'valRef',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'wname' => 'type',
                                                       'name' => 'type',
                                                       'type' => 'BrigType16_t'
                                                     },
                                                     {
                                                       'type' => 'BrigOperandOffset32_t',
                                                       'size' => '5',
                                                       'wtype' => 'ItemRef<Operand>',
                                                       'wname' => 'operands',
                                                       'name' => 'operands',
                                                       'defValue' => '0',
                                                       'acc' => 'itemRef<Operand>',
                                                       'comments' => [
                                                                       '// overridden, was ItemRef<Operand> operand'
                                                                     ]
                                                     },
                                                     {
                                                       'type' => 'BrigType16_t',
                                                       'name' => 'sourceType',
                                                       'wname' => 'sourceType',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'acc' => 'valRef'
                                                     },
                                                     {
                                                       'type' => 'uint16_t',
                                                       'defValue' => '0',
                                                       'name' => 'reserved',
                                                       'wname' => 'reserved',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'skip' => 1,
                                                       'acc' => 'valRef'
                                                     }
                                                   ],
                                       'enum' => 'BRIG_INST_SOURCE_TYPE'
                                     },
             'BrigBlockStart' => {
                                   'parent' => 'BrigDirectiveCode',
                                   'comments' => [
                                                   '/// start block of data.'
                                                 ],
                                   'align' => undef,
                                   'name' => 'BrigBlockStart',
                                   'fields' => [
                                                 {
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'acc' => 'valRef',
                                                   'wname' => 'size',
                                                   'type' => 'uint16_t',
                                                   'name' => 'size'
                                                 },
                                                 {
                                                   'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                   'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                   'enum' => 'BrigDirectiveKinds',
                                                   'wname' => 'kind',
                                                   'type' => 'BrigDirectiveKinds16_t',
                                                   'name' => 'kind'
                                                 },
                                                 {
                                                   'name' => 'code',
                                                   'type' => 'BrigCodeOffset32_t',
                                                   'defValue' => '0',
                                                   'wname' => 'code',
                                                   'acc' => 'itemRef<Inst>',
                                                   'wtype' => 'ItemRef<Inst>'
                                                 },
                                                 {
                                                   'type' => 'BrigStringOffset32_t',
                                                   'defValue' => '0',
                                                   'name' => 'name',
                                                   'wname' => 'name',
                                                   'wtype' => 'StrRef',
                                                   'acc' => 'strRef'
                                                 }
                                               ],
                                   'wname' => 'BlockStart',
                                   'enum' => 'BRIG_DIRECTIVE_BLOCK_START'
                                 },
             'BrigOperandBase' => {
                                    'nowrap' => 'true',
                                    'align' => undef,
                                    'name' => 'BrigOperandBase',
                                    'fields' => [
                                                  {
                                                    'type' => 'uint16_t',
                                                    'name' => 'size'
                                                  },
                                                  {
                                                    'name' => 'kind',
                                                    'type' => 'BrigOperandKinds16_t'
                                                  }
                                                ]
                                  },
             'BrigOperandRef' => {
                                   'generic' => 'true',
                                   'name' => 'BrigOperandRef',
                                   'children' => [
                                                   'BrigOperandFbarrierRef',
                                                   'BrigOperandLabelVariableRef',
                                                   'BrigOperandLabelTargetsRef',
                                                   'BrigOperandLabelRef',
                                                   'BrigOperandSignatureRef',
                                                   'BrigOperandFunctionRef'
                                                 ],
                                   'wname' => 'OperandRef',
                                   'enum' => 'BRIG_OPERAND_REF',
                                   'parent' => 'BrigOperand',
                                   'align' => undef,
                                   'fields' => [
                                                 {
                                                   'wname' => 'size',
                                                   'type' => 'uint16_t',
                                                   'name' => 'size',
                                                   'wtype' => 'ValRef<uint16_t>',
                                                   'acc' => 'valRef'
                                                 },
                                                 {
                                                   'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                   'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                   'wname' => 'kind',
                                                   'enum' => 'BrigOperandKinds',
                                                   'name' => 'kind',
                                                   'type' => 'BrigOperandKinds16_t'
                                                 },
                                                 {
                                                   'acc' => 'itemRef<Directive>',
                                                   'wtype' => 'ItemRef<Directive>',
                                                   'wname' => 'ref',
                                                   'name' => 'ref',
                                                   'defValue' => '0',
                                                   'type' => 'BrigDirectiveOffset32_t'
                                                 }
                                               ]
                                 },
             'BrigDirectiveExecutable' => {
                                            'comments' => [
                                                            '/// common ancestor class for kernel/function directives.'
                                                          ],
                                            'parent' => 'BrigDirectiveCallableBase',
                                            'enum' => 'BRIG_DIRECTIVE_EXECUTABLE',
                                            'wname' => 'DirectiveExecutable',
                                            'children' => [
                                                            'BrigDirectiveKernel',
                                                            'BrigDirectiveFunction'
                                                          ],
                                            'generic' => 'true',
                                            'name' => 'BrigDirectiveExecutable',
                                            'fields' => [
                                                          {
                                                            'name' => 'size',
                                                            'type' => 'uint16_t',
                                                            'wname' => 'size',
                                                            'acc' => 'valRef',
                                                            'wtype' => 'ValRef<uint16_t>'
                                                          },
                                                          {
                                                            'name' => 'kind',
                                                            'type' => 'BrigDirectiveKinds16_t',
                                                            'wname' => 'kind',
                                                            'enum' => 'BrigDirectiveKinds',
                                                            'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                            'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                          },
                                                          {
                                                            'wname' => 'code',
                                                            'type' => 'BrigCodeOffset32_t',
                                                            'defValue' => '0',
                                                            'name' => 'code',
                                                            'wtype' => 'ItemRef<Inst>',
                                                            'acc' => 'itemRef<Inst>'
                                                          },
                                                          {
                                                            'wname' => 'name',
                                                            'name' => 'name',
                                                            'type' => 'BrigStringOffset32_t',
                                                            'defValue' => '0',
                                                            'acc' => 'strRef',
                                                            'wtype' => 'StrRef'
                                                          },
                                                          {
                                                            'type' => 'uint16_t',
                                                            'name' => 'inArgCount',
                                                            'wname' => 'inArgCount',
                                                            'wtype' => 'ValRef<uint16_t>',
                                                            'acc' => 'valRef'
                                                          },
                                                          {
                                                            'name' => 'outArgCount',
                                                            'type' => 'uint16_t',
                                                            'wname' => 'outArgCount',
                                                            'acc' => 'valRef',
                                                            'wtype' => 'ValRef<uint16_t>'
                                                          },
                                                          {
                                                            'name' => 'firstInArg',
                                                            'defValue' => '0',
                                                            'type' => 'BrigDirectiveOffset32_t',
                                                            'wname' => 'firstInArg',
                                                            'acc' => 'itemRef<Directive>',
                                                            'wtype' => 'ItemRef<Directive>'
                                                          },
                                                          {
                                                            'type' => 'BrigDirectiveOffset32_t',
                                                            'defValue' => '0',
                                                            'name' => 'firstScopedDirective',
                                                            'wname' => 'firstScopedDirective',
                                                            'wtype' => 'ItemRef<Directive>',
                                                            'acc' => 'itemRef<Directive>'
                                                          },
                                                          {
                                                            'acc' => 'itemRef<Directive>',
                                                            'wtype' => 'ItemRef<Directive>',
                                                            'name' => 'nextTopLevelDirective',
                                                            'type' => 'BrigDirectiveOffset32_t',
                                                            'defValue' => '0',
                                                            'wname' => 'nextTopLevelDirective'
                                                          },
                                                          {
                                                            'name' => 'instCount',
                                                            'type' => 'uint32_t',
                                                            'wname' => 'instCount',
                                                            'acc' => 'valRef',
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
                                                            'skip' => 1,
                                                            'acc' => 'valRef',
                                                            'wname' => 'reserved',
                                                            'name' => 'reserved',
                                                            'defValue' => '0',
                                                            'size' => '3',
                                                            'wtype' => 'ValRef<uint8_t>',
                                                            'type' => 'uint8_t'
                                                          }
                                                        ],
                                            'align' => undef
                                          },
             'BrigInstLane' => {
                                 'name' => 'BrigInstLane',
                                 'align' => undef,
                                 'enum' => 'BRIG_INST_LANE',
                                 'wname' => 'InstLane',
                                 'fields' => [
                                               {
                                                 'wtype' => 'ValRef<uint16_t>',
                                                 'acc' => 'valRef',
                                                 'type' => 'uint16_t',
                                                 'name' => 'size',
                                                 'wname' => 'size'
                                               },
                                               {
                                                 'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                 'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>',
                                                 'wname' => 'kind',
                                                 'enum' => 'BrigInstKinds',
                                                 'name' => 'kind',
                                                 'type' => 'BrigInstKinds16_t'
                                               },
                                               {
                                                 'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                 'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                 'wname' => 'opcode',
                                                 'enum' => 'BrigOpcode',
                                                 'name' => 'opcode',
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
                                                 'wtype' => 'ItemRef<Operand>',
                                                 'size' => '5',
                                                 'acc' => 'itemRef<Operand>',
                                                 'wname' => 'operand',
                                                 'defValue' => '0',
                                                 'type' => 'BrigOperandOffset32_t',
                                                 'name' => 'operands'
                                               },
                                               {
                                                 'name' => 'sourceType',
                                                 'type' => 'BrigType16_t',
                                                 'wname' => 'sourceType',
                                                 'acc' => 'valRef',
                                                 'wtype' => 'ValRef<uint16_t>'
                                               },
                                               {
                                                 'type' => 'BrigWidth8_t',
                                                 'name' => 'width',
                                                 'enum' => 'BrigWidth',
                                                 'wname' => 'width',
                                                 'wtype' => 'EnumValRef<Brig::BrigWidth,uint8_t>',
                                                 'acc' => 'enumValRef<Brig::BrigWidth,uint8_t>'
                                               },
                                               {
                                                 'acc' => 'valRef',
                                                 'skip' => 1,
                                                 'wtype' => 'ValRef<uint8_t>',
                                                 'wname' => 'reserved',
                                                 'name' => 'reserved',
                                                 'type' => 'uint8_t',
                                                 'defValue' => '0'
                                               }
                                             ],
                                 'parent' => 'BrigInst'
                               },
             'BrigOperandList' => {
                                    'align' => undef,
                                    'fields' => [
                                                  {
                                                    'name' => 'size',
                                                    'type' => 'uint16_t',
                                                    'wname' => 'size',
                                                    'acc' => 'valRef',
                                                    'wtype' => 'ValRef<uint16_t>'
                                                  },
                                                  {
                                                    'type' => 'BrigOperandKinds16_t',
                                                    'name' => 'kind',
                                                    'enum' => 'BrigOperandKinds',
                                                    'wname' => 'kind',
                                                    'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                    'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>'
                                                  },
                                                  {
                                                    'wtype' => 'ValRef<uint16_t>',
                                                    'acc' => 'valRef',
                                                    'skip' => 1,
                                                    'defValue' => '0',
                                                    'type' => 'uint16_t',
                                                    'name' => 'reserved',
                                                    'wname' => 'reserved'
                                                  },
                                                  {
                                                    'acc' => 'valRef',
                                                    'wtype' => 'ValRef<uint16_t>',
                                                    'name' => 'elementCount',
                                                    'defValue' => '0',
                                                    'type' => 'uint16_t',
                                                    'wname' => 'elementCount'
                                                  },
                                                  {
                                                    'acc' => 'itemRef<Directive>',
                                                    'name' => 'elements',
                                                    'defValue' => '0',
                                                    'wname' => 'elements',
                                                    'size' => '1',
                                                    'wtype' => 'ItemRef<Directive>',
                                                    'type' => 'BrigDirectiveOffset32_t',
                                                    'wspecial' => 'RefList'
                                                  }
                                                ],
                                    'parent' => 'BrigOperand',
                                    'children' => [
                                                    'BrigOperandFunctionList',
                                                    'BrigOperandArgumentList'
                                                  ],
                                    'name' => 'BrigOperandList',
                                    'generic' => 'true',
                                    'enum' => 'BRIG_OPERAND_LIST',
                                    'wname' => 'OperandList'
                                  },
             'BrigInstQueryImage' => {
                                       'parent' => 'BrigInst',
                                       'fields' => [
                                                     {
                                                       'acc' => 'valRef',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'wname' => 'size',
                                                       'name' => 'size',
                                                       'type' => 'uint16_t'
                                                     },
                                                     {
                                                       'enum' => 'BrigInstKinds',
                                                       'wname' => 'kind',
                                                       'type' => 'BrigInstKinds16_t',
                                                       'name' => 'kind',
                                                       'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>',
                                                       'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>'
                                                     },
                                                     {
                                                       'type' => 'BrigOpcode16_t',
                                                       'name' => 'opcode',
                                                       'enum' => 'BrigOpcode',
                                                       'wname' => 'opcode',
                                                       'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                       'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>'
                                                     },
                                                     {
                                                       'acc' => 'valRef',
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'wname' => 'type',
                                                       'name' => 'type',
                                                       'type' => 'BrigType16_t'
                                                     },
                                                     {
                                                       'wtype' => 'ItemRef<Operand>',
                                                       'size' => '5',
                                                       'type' => 'BrigOperandOffset32_t',
                                                       'comments' => [
                                                                       '// overridden, was ItemRef<Operand> operand'
                                                                     ],
                                                       'acc' => 'itemRef<Operand>',
                                                       'wname' => 'operands',
                                                       'defValue' => '0',
                                                       'name' => 'operands'
                                                     },
                                                     {
                                                       'wtype' => 'ValRef<uint16_t>',
                                                       'acc' => 'valRef',
                                                       'wname' => 'imageType',
                                                       'type' => 'BrigType16_t',
                                                       'name' => 'imageType'
                                                     },
                                                     {
                                                       'wtype' => 'EnumValRef<Brig::BrigImageGeometry,uint8_t>',
                                                       'acc' => 'enumValRef<Brig::BrigImageGeometry,uint8_t>',
                                                       'type' => 'BrigImageGeometry8_t',
                                                       'defValue' => 'Brig::BRIG_GEOMETRY_UNKNOWN',
                                                       'name' => 'geometry',
                                                       'enum' => 'BrigImageGeometry',
                                                       'wname' => 'geometry'
                                                     },
                                                     {
                                                       'wtype' => 'EnumValRef<Brig::BrigImageQuery,uint8_t>',
                                                       'acc' => 'enumValRef<Brig::BrigImageQuery,uint8_t>',
                                                       'type' => 'BrigImageQuery8_t',
                                                       'name' => 'imageQuery',
                                                       'enum' => 'BrigImageQuery',
                                                       'wname' => 'imageQuery'
                                                     }
                                                   ],
                                       'wname' => 'InstQueryImage',
                                       'enum' => 'BRIG_INST_QUERY_IMAGE',
                                       'align' => undef,
                                       'name' => 'BrigInstQueryImage'
                                     },
             'BrigDirectiveVariableInit' => {
                                              'enum' => 'BRIG_DIRECTIVE_VARIABLE_INIT',
                                              'wname' => 'DirectiveVariableInit',
                                              'fields' => [
                                                            {
                                                              'wname' => 'size',
                                                              'name' => 'size',
                                                              'type' => 'uint16_t',
                                                              'acc' => 'valRef',
                                                              'wtype' => 'ValRef<uint16_t>'
                                                            },
                                                            {
                                                              'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                              'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                              'type' => 'BrigDirectiveKinds16_t',
                                                              'name' => 'kind',
                                                              'enum' => 'BrigDirectiveKinds',
                                                              'wname' => 'kind'
                                                            },
                                                            {
                                                              'wtype' => 'ItemRef<Inst>',
                                                              'acc' => 'itemRef<Inst>',
                                                              'defValue' => '0',
                                                              'type' => 'BrigCodeOffset32_t',
                                                              'name' => 'code',
                                                              'wname' => 'code'
                                                            },
                                                            {
                                                              'wtype' => 'DataItemRef',
                                                              'acc' => 'dataItemRef',
                                                              'novisit' => 'true',
                                                              'wname' => 'data',
                                                              'type' => 'BrigDataOffset32_t',
                                                              'name' => 'data'
                                                            },
                                                            {
                                                              'wname' => 'dataAs',
                                                              'name' => 'dataAs',
                                                              'acc' => 'valRef',
                                                              'phantomof' => $structs->{'BrigDirectiveVariableInit'}{'fields'}[3],
                                                              'wspecial' => 'DataItemRefT',
                                                              'type' => 'BrigDataOffset32_t',
                                                              'wtype' => 'ValRef<uint32_t>',
                                                              'wspecialgeneric' => 'true',
                                                              'novisit' => 'true'
                                                            },
                                                            {
                                                              'defValue' => '0',
                                                              'type' => 'uint32_t',
                                                              'name' => 'elementCount',
                                                              'wname' => 'elementCount',
                                                              'wtype' => 'ValRef<uint32_t>',
                                                              'acc' => 'valRef'
                                                            },
                                                            {
                                                              'type' => 'BrigType16_t',
                                                              'name' => 'type',
                                                              'wname' => 'type',
                                                              'wtype' => 'ValRef<uint16_t>',
                                                              'acc' => 'valRef'
                                                            },
                                                            {
                                                              'name' => 'reserved',
                                                              'defValue' => '0',
                                                              'type' => 'uint16_t',
                                                              'wname' => 'reserved',
                                                              'acc' => 'valRef',
                                                              'skip' => 1,
                                                              'wtype' => 'ValRef<uint16_t>'
                                                            }
                                                          ],
                                              'align' => undef,
                                              'name' => 'BrigDirectiveVariableInit',
                                              'parent' => 'BrigDirectiveCode'
                                            },
             'BrigInst' => {
                             'align' => undef,
                             'isroot' => 'true',
                             'fields' => [
                                           {
                                             'acc' => 'valRef',
                                             'wtype' => 'ValRef<uint16_t>',
                                             'wname' => 'size',
                                             'name' => 'size',
                                             'type' => 'uint16_t'
                                           },
                                           {
                                             'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                             'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>',
                                             'wname' => 'kind',
                                             'enum' => 'BrigInstKinds',
                                             'name' => 'kind',
                                             'type' => 'BrigInstKinds16_t'
                                           },
                                           {
                                             'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                             'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                             'type' => 'BrigOpcode16_t',
                                             'name' => 'opcode',
                                             'enum' => 'BrigOpcode',
                                             'wname' => 'opcode'
                                           },
                                           {
                                             'wtype' => 'ValRef<uint16_t>',
                                             'acc' => 'valRef',
                                             'type' => 'BrigType16_t',
                                             'name' => 'type',
                                             'wname' => 'type'
                                           },
                                           {
                                             'wtype' => 'ItemRef<Operand>',
                                             'acc' => 'itemRef<Operand>',
                                             'size' => '5',
                                             'type' => 'BrigOperandOffset32_t',
                                             'defValue' => '0',
                                             'name' => 'operands',
                                             'wname' => 'operand'
                                           }
                                         ],
                             'generic' => 'true',
                             'name' => 'BrigInst',
                             'children' => [
                                             'BrigInstImage',
                                             'BrigInstMem',
                                             'BrigInstMod',
                                             'BrigInstQuerySampler',
                                             'BrigInstBasic',
                                             'BrigInstQueue',
                                             'BrigInstQueryImage',
                                             'BrigInstLane',
                                             'BrigInstAddr',
                                             'BrigInstSignal',
                                             'BrigInstSourceType',
                                             'BrigInstBr',
                                             'BrigInstSegCvt',
                                             'BrigInstCvt',
                                             'BrigInstCmp',
                                             'BrigInstSeg',
                                             'BrigInstMemFence',
                                             'BrigInstAtomic'
                                           ],
                             'wname' => 'Inst',
                             'enum' => 'BRIG_INST'
                           },
             'BrigBlockString' => {
                                    'comments' => [
                                                    '/// string inside block.'
                                                  ],
                                    'parent' => 'BrigDirective',
                                    'wname' => 'BlockString',
                                    'fields' => [
                                                  {
                                                    'acc' => 'valRef',
                                                    'wtype' => 'ValRef<uint16_t>',
                                                    'name' => 'size',
                                                    'type' => 'uint16_t',
                                                    'wname' => 'size'
                                                  },
                                                  {
                                                    'type' => 'BrigDirectiveKinds16_t',
                                                    'name' => 'kind',
                                                    'enum' => 'BrigDirectiveKinds',
                                                    'wname' => 'kind',
                                                    'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                    'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                  },
                                                  {
                                                    'wtype' => 'StrRef',
                                                    'acc' => 'strRef',
                                                    'type' => 'BrigStringOffset32_t',
                                                    'defValue' => '0',
                                                    'name' => 'string',
                                                    'wname' => 'string'
                                                  }
                                                ],
                                    'enum' => 'BRIG_DIRECTIVE_BLOCK_STRING',
                                    'name' => 'BrigBlockString',
                                    'align' => undef
                                  },
             'BrigOperandVector' => {
                                      'parent' => 'BrigOperand',
                                      'name' => 'BrigOperandVector',
                                      'align' => undef,
                                      'wname' => 'OperandVector',
                                      'fields' => [
                                                    {
                                                      'acc' => 'valRef',
                                                      'wtype' => 'ValRef<uint16_t>',
                                                      'wname' => 'size',
                                                      'name' => 'size',
                                                      'type' => 'uint16_t'
                                                    },
                                                    {
                                                      'acc' => 'enumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                      'wtype' => 'EnumValRef<Brig::BrigOperandKinds,uint16_t>',
                                                      'wname' => 'kind',
                                                      'enum' => 'BrigOperandKinds',
                                                      'name' => 'kind',
                                                      'type' => 'BrigOperandKinds16_t'
                                                    },
                                                    {
                                                      'name' => 'reserved',
                                                      'type' => 'uint16_t',
                                                      'defValue' => '0',
                                                      'wname' => 'reserved',
                                                      'acc' => 'valRef',
                                                      'skip' => 1,
                                                      'wtype' => 'ValRef<uint16_t>'
                                                    },
                                                    {
                                                      'defValue' => '0',
                                                      'type' => 'uint16_t',
                                                      'name' => 'operandCount',
                                                      'wname' => 'operandCount',
                                                      'wtype' => 'ValRef<uint16_t>',
                                                      'acc' => 'valRef'
                                                    },
                                                    {
                                                      'wtype' => 'ValRef<uint16_t>',
                                                      'acc' => 'valRef',
                                                      'phantomof' => $structs->{'BrigOperandVector'}{'fields'}[3],
                                                      'wname' => 'elementCount',
                                                      'type' => 'uint16_t',
                                                      'name' => 'elementCount'
                                                    },
                                                    {
                                                      'acc' => 'itemRef<Operand>',
                                                      'wname' => 'operand',
                                                      'defValue' => '0',
                                                      'name' => 'operands',
                                                      'wtype' => 'ItemRef<Operand>',
                                                      'size' => '1',
                                                      'wspecial' => 'VectorOperandList',
                                                      'type' => 'BrigOperandOffset32_t'
                                                    }
                                                  ],
                                      'enum' => 'BRIG_OPERAND_VECTOR'
                                    },
             'BrigDirectiveSamplerProperties' => {
                                                   'wname' => 'DirectiveSamplerProperties',
                                                   'fields' => [
                                                                 {
                                                                   'wname' => 'size',
                                                                   'name' => 'size',
                                                                   'type' => 'uint16_t',
                                                                   'acc' => 'valRef',
                                                                   'wtype' => 'ValRef<uint16_t>'
                                                                 },
                                                                 {
                                                                   'type' => 'BrigDirectiveKinds16_t',
                                                                   'name' => 'kind',
                                                                   'enum' => 'BrigDirectiveKinds',
                                                                   'wname' => 'kind',
                                                                   'wtype' => 'EnumValRef<Brig::BrigDirectiveKinds,uint16_t>',
                                                                   'acc' => 'enumValRef<Brig::BrigDirectiveKinds,uint16_t>'
                                                                 },
                                                                 {
                                                                   'type' => 'BrigCodeOffset32_t',
                                                                   'defValue' => '0',
                                                                   'name' => 'code',
                                                                   'wname' => 'code',
                                                                   'wtype' => 'ItemRef<Inst>',
                                                                   'acc' => 'itemRef<Inst>'
                                                                 },
                                                                 {
                                                                   'type' => 'BrigSamplerCoordNormalization8_t',
                                                                   'name' => 'coord',
                                                                   'enum' => 'BrigSamplerCoordNormalization',
                                                                   'wname' => 'coord',
                                                                   'wtype' => 'EnumValRef<Brig::BrigSamplerCoordNormalization,uint8_t>',
                                                                   'acc' => 'enumValRef<Brig::BrigSamplerCoordNormalization,uint8_t>'
                                                                 },
                                                                 {
                                                                   'wtype' => 'EnumValRef<Brig::BrigSamplerFilter,uint8_t>',
                                                                   'acc' => 'enumValRef<Brig::BrigSamplerFilter,uint8_t>',
                                                                   'enum' => 'BrigSamplerFilter',
                                                                   'wname' => 'filter',
                                                                   'type' => 'BrigSamplerFilter8_t',
                                                                   'name' => 'filter'
                                                                 },
                                                                 {
                                                                   'acc' => 'enumValRef<Brig::BrigSamplerAddressing,uint8_t>',
                                                                   'wtype' => 'EnumValRef<Brig::BrigSamplerAddressing,uint8_t>',
                                                                   'wname' => 'addressing',
                                                                   'enum' => 'BrigSamplerAddressing',
                                                                   'name' => 'addressing',
                                                                   'defValue' => 'Brig::BRIG_ADDRESSING_CLAMP_TO_EDGE',
                                                                   'type' => 'BrigSamplerAddressing8_t'
                                                                 },
                                                                 {
                                                                   'wname' => 'reserved',
                                                                   'name' => 'reserved',
                                                                   'defValue' => '0',
                                                                   'type' => 'uint8_t',
                                                                   'acc' => 'valRef',
                                                                   'skip' => 1,
                                                                   'wtype' => 'ValRef<uint8_t>'
                                                                 }
                                                               ],
                                                   'enum' => 'BRIG_DIRECTIVE_SAMPLER_PROPERTIES',
                                                   'name' => 'BrigDirectiveSamplerProperties',
                                                   'align' => undef,
                                                   'parent' => 'BrigDirectiveCode'
                                                 },
             'BrigInstQueue' => {
                                  'align' => undef,
                                  'name' => 'BrigInstQueue',
                                  'wname' => 'InstQueue',
                                  'fields' => [
                                                {
                                                  'acc' => 'valRef',
                                                  'wtype' => 'ValRef<uint16_t>',
                                                  'name' => 'size',
                                                  'type' => 'uint16_t',
                                                  'wname' => 'size'
                                                },
                                                {
                                                  'acc' => 'enumValRef<Brig::BrigInstKinds,uint16_t>',
                                                  'wtype' => 'EnumValRef<Brig::BrigInstKinds,uint16_t>',
                                                  'name' => 'kind',
                                                  'type' => 'BrigInstKinds16_t',
                                                  'wname' => 'kind',
                                                  'enum' => 'BrigInstKinds'
                                                },
                                                {
                                                  'acc' => 'enumValRef<Brig::BrigOpcode,uint16_t>',
                                                  'wtype' => 'EnumValRef<Brig::BrigOpcode,uint16_t>',
                                                  'name' => 'opcode',
                                                  'type' => 'BrigOpcode16_t',
                                                  'wname' => 'opcode',
                                                  'enum' => 'BrigOpcode'
                                                },
                                                {
                                                  'wname' => 'type',
                                                  'name' => 'type',
                                                  'type' => 'BrigType16_t',
                                                  'acc' => 'valRef',
                                                  'wtype' => 'ValRef<uint16_t>'
                                                },
                                                {
                                                  'wname' => 'operands',
                                                  'name' => 'operands',
                                                  'defValue' => '0',
                                                  'acc' => 'itemRef<Operand>',
                                                  'comments' => [
                                                                  '// overridden, was ItemRef<Operand> operand'
                                                                ],
                                                  'type' => 'BrigOperandOffset32_t',
                                                  'size' => '5',
                                                  'wtype' => 'ItemRef<Operand>'
                                                },
                                                {
                                                  'acc' => 'enumValRef<Brig::BrigSegment,uint8_t>',
                                                  'wtype' => 'EnumValRef<Brig::BrigSegment,uint8_t>',
                                                  'wname' => 'segment',
                                                  'enum' => 'BrigSegment',
                                                  'name' => 'segment',
                                                  'defValue' => '0',
                                                  'type' => 'BrigSegment8_t'
                                                },
                                                {
                                                  'wtype' => 'EnumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                  'acc' => 'enumValRef<Brig::BrigMemoryOrder,uint8_t>',
                                                  'type' => 'BrigMemoryOrder8_t',
                                                  'defValue' => 'Brig::BRIG_MEMORY_ORDER_RELAXED',
                                                  'name' => 'memoryOrder',
                                                  'enum' => 'BrigMemoryOrder',
                                                  'wname' => 'memoryOrder'
                                                },
                                                {
                                                  'wname' => 'reserved',
                                                  'name' => 'reserved',
                                                  'defValue' => '0',
                                                  'type' => 'uint16_t',
                                                  'acc' => 'valRef',
                                                  'skip' => 1,
                                                  'wtype' => 'ValRef<uint16_t>'
                                                }
                                              ],
                                  'enum' => 'BRIG_INST_QUEUE',
                                  'parent' => 'BrigInst'
                                }
           };
