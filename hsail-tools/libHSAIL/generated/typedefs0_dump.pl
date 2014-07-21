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
$typedefs = {
              'BrigType16_t' => {
                                  'name' => 'BrigType16_t',
                                  'type' => 'uint16_t'
                                },
              'BrigOpcode16_t' => {
                                    'name' => 'BrigOpcode16_t',
                                    'type' => 'uint16_t'
                                  },
              'BrigSectionIndex32_t' => {
                                          'type' => 'uint32_t',
                                          'name' => 'BrigSectionIndex32_t'
                                        },
              'BrigMemoryScope8_t' => {
                                        'type' => 'uint8_t',
                                        'defValue' => 'Brig::BRIG_MEMORY_SCOPE_SYSTEM',
                                        'name' => 'BrigMemoryScope8_t'
                                      },
              'BrigMemoryOrder8_t' => {
                                        'defValue' => 'Brig::BRIG_MEMORY_ORDER_RELAXED',
                                        'type' => 'uint8_t',
                                        'name' => 'BrigMemoryOrder8_t'
                                      },
              'BrigAluModifier16_t' => {
                                         'name' => 'BrigAluModifier16_t',
                                         'type' => 'uint16_t'
                                       },
              'BrigRound8_t' => {
                                  'type' => 'uint8_t',
                                  'name' => 'BrigRound8_t'
                                },
              'BrigVariableModifier8_t' => {
                                             'type' => 'uint8_t',
                                             'name' => 'BrigVariableModifier8_t'
                                           },
              'BrigSamplerQuery8_t' => {
                                         'type' => 'uint8_t',
                                         'name' => 'BrigSamplerQuery8_t'
                                       },
              'BrigRegisterKind16_t' => {
                                          'type' => 'uint16_t',
                                          'name' => 'BrigRegisterKind16_t'
                                        },
              'BrigProfile8_t' => {
                                    'defValue' => 'Brig::BRIG_PROFILE_FULL',
                                    'type' => 'uint8_t',
                                    'name' => 'BrigProfile8_t'
                                  },
              'BrigSamplerAddressing8_t' => {
                                              'type' => 'uint8_t',
                                              'defValue' => 'Brig::BRIG_ADDRESSING_CLAMP_TO_EDGE',
                                              'name' => 'BrigSamplerAddressing8_t'
                                            },
              'BrigWidth8_t' => {
                                  'name' => 'BrigWidth8_t',
                                  'type' => 'uint8_t'
                                },
              'BrigDataOffset32_t' => {
                                        'name' => 'BrigDataOffset32_t',
                                        'type' => 'uint32_t'
                                      },
              'BrigSegment8_t' => {
                                    'name' => 'BrigSegment8_t',
                                    'type' => 'uint8_t',
                                    'defValue' => 'Brig::BRIG_SEGMENT_NONE'
                                  },
              'BrigControlDirective16_t' => {
                                              'name' => 'BrigControlDirective16_t',
                                              'type' => 'uint16_t'
                                            },
              'BrigPack8_t' => {
                                 'defValue' => 'Brig::BRIG_PACK_NONE',
                                 'type' => 'uint8_t',
                                 'name' => 'BrigPack8_t'
                               },
              'BrigSamplerCoordNormalization8_t' => {
                                                      'name' => 'BrigSamplerCoordNormalization8_t',
                                                      'type' => 'uint8_t'
                                                    },
              'BrigDataOffsetString32_t' => {
                                              'defValue' => '0',
                                              'type' => 'BrigDataOffset32_t',
                                              'name' => 'BrigDataOffsetString32_t',
                                              'wtype' => 'StrRef'
                                            },
              'BrigAlignment8_t' => {
                                      'name' => 'BrigAlignment8_t',
                                      'defValue' => 'Brig::BRIG_ALIGNMENT_NONE',
                                      'type' => 'uint8_t'
                                    },
              'BrigMachineModel8_t' => {
                                         'name' => 'BrigMachineModel8_t',
                                         'defValue' => 'Brig::BRIG_MACHINE_LARGE',
                                         'type' => 'uint8_t'
                                       },
              'BrigCompareOperation8_t' => {
                                             'name' => 'BrigCompareOperation8_t',
                                             'type' => 'uint8_t'
                                           },
              'BrigImageQuery8_t' => {
                                       'type' => 'uint8_t',
                                       'name' => 'BrigImageQuery8_t'
                                     },
              'BrigDataOffsetOperandList32_t' => {
                                                   'type' => 'BrigDataOffset32_t',
                                                   'defValue' => '0',
                                                   'wtype' => 'ListRef<Operand>',
                                                   'name' => 'BrigDataOffsetOperandList32_t'
                                                 },
              'BrigExecutableModifier8_t' => {
                                               'type' => 'uint8_t',
                                               'name' => 'BrigExecutableModifier8_t'
                                             },
              'BrigAllocation8_t' => {
                                       'type' => 'uint8_t',
                                       'defValue' => 'Brig::BRIG_ALLOCATION_NONE',
                                       'name' => 'BrigAllocation8_t'
                                     },
              'BrigOperandOffset32_t' => {
                                           'defValue' => '0',
                                           'type' => 'uint32_t',
                                           'name' => 'BrigOperandOffset32_t',
                                           'wtype' => 'ItemRef<Operand>'
                                         },
              'BrigMemoryModifier8_t' => {
                                           'name' => 'BrigMemoryModifier8_t',
                                           'type' => 'uint8_t'
                                         },
              'BrigKinds16_t' => {
                                   'name' => 'BrigKinds16_t',
                                   'type' => 'uint16_t'
                                 },
              'BrigSegCvtModifier8_t' => {
                                           'name' => 'BrigSegCvtModifier8_t',
                                           'type' => 'uint8_t'
                                         },
              'BrigImageChannelType8_t' => {
                                             'type' => 'uint8_t',
                                             'defValue' => 'Brig::BRIG_CHANNEL_TYPE_UNKNOWN',
                                             'name' => 'BrigImageChannelType8_t'
                                           },
              'BrigSamplerFilter8_t' => {
                                          'name' => 'BrigSamplerFilter8_t',
                                          'type' => 'uint8_t'
                                        },
              'BrigLinkage8_t' => {
                                    'type' => 'uint8_t',
                                    'defValue' => 'Brig::BRIG_LINKAGE_NONE',
                                    'name' => 'BrigLinkage8_t'
                                  },
              'BrigImageChannelOrder8_t' => {
                                              'name' => 'BrigImageChannelOrder8_t',
                                              'type' => 'uint8_t',
                                              'defValue' => 'Brig::BRIG_CHANNEL_ORDER_UNKNOWN'
                                            },
              'BrigImageGeometry8_t' => {
                                          'type' => 'uint8_t',
                                          'defValue' => 'Brig::BRIG_GEOMETRY_UNKNOWN',
                                          'name' => 'BrigImageGeometry8_t'
                                        },
              'BrigDataOffsetCodeList32_t' => {
                                                'name' => 'BrigDataOffsetCodeList32_t',
                                                'wtype' => 'ListRef<Code>',
                                                'defValue' => '0',
                                                'type' => 'BrigDataOffset32_t'
                                              },
              'BrigStringOffset32_t' => {
                                          'type' => 'uint32_t',
                                          'defValue' => '0',
                                          'wtype' => 'StrRef',
                                          'name' => 'BrigStringOffset32_t'
                                        },
              'BrigCodeOffset32_t' => {
                                        'type' => 'uint32_t',
                                        'defValue' => '0',
                                        'wtype' => 'ItemRef<Code>',
                                        'name' => 'BrigCodeOffset32_t'
                                      },
              'BrigAtomicOperation8_t' => {
                                            'type' => 'uint8_t',
                                            'name' => 'BrigAtomicOperation8_t'
                                          },
              'BrigVersion32_t' => {
                                     'type' => 'uint32_t',
                                     'name' => 'BrigVersion32_t'
                                   }
            };
