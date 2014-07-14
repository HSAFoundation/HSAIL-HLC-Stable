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
              'BrigStringOffset32_t' => {
                                          'name' => 'BrigStringOffset32_t',
                                          'type' => 'uint32_t',
                                          'defValue' => '0',
                                          'wtype' => 'StrRef'
                                        },
              'BrigDirectiveKinds16_t' => {
                                            'name' => 'BrigDirectiveKinds16_t',
                                            'type' => 'uint16_t'
                                          },
              'BrigControlDirective16_t' => {
                                              'name' => 'BrigControlDirective16_t',
                                              'type' => 'uint16_t'
                                            },
              'BrigMemoryScope8_t' => {
                                        'name' => 'BrigMemoryScope8_t',
                                        'defValue' => 'Brig::BRIG_MEMORY_SCOPE_SYSTEM',
                                        'type' => 'uint8_t'
                                      },
              'BrigWidth8_t' => {
                                  'name' => 'BrigWidth8_t',
                                  'type' => 'uint8_t'
                                },
              'BrigImageQuery8_t' => {
                                       'name' => 'BrigImageQuery8_t',
                                       'type' => 'uint8_t'
                                     },
              'BrigMemoryFenceSegments8_t' => {
                                                'type' => 'uint8_t',
                                                'name' => 'BrigMemoryFenceSegments8_t'
                                              },
              'BrigSamplerFilter8_t' => {
                                          'name' => 'BrigSamplerFilter8_t',
                                          'type' => 'uint8_t'
                                        },
              'BrigAluModifier16_t' => {
                                         'name' => 'BrigAluModifier16_t',
                                         'type' => 'uint16_t'
                                       },
              'BrigDataOffset32_t' => {
                                        'name' => 'BrigDataOffset32_t',
                                        'type' => 'uint32_t'
                                      },
              'BrigImageGeometry8_t' => {
                                          'defValue' => 'Brig::BRIG_GEOMETRY_UNKNOWN',
                                          'type' => 'uint8_t',
                                          'name' => 'BrigImageGeometry8_t'
                                        },
              'BrigSamplerAddressing8_t' => {
                                              'name' => 'BrigSamplerAddressing8_t',
                                              'type' => 'uint8_t',
                                              'defValue' => 'Brig::BRIG_ADDRESSING_CLAMP_TO_EDGE'
                                            },
              'BrigLinkage8_t' => {
                                    'name' => 'BrigLinkage8_t',
                                    'defValue' => 'Brig::BRIG_LINKAGE_NONE',
                                    'type' => 'uint8_t'
                                  },
              'BrigExecutableModifier8_t' => {
                                               'type' => 'uint8_t',
                                               'name' => 'BrigExecutableModifier8_t'
                                             },
              'BrigSymbolModifier8_t' => {
                                           'name' => 'BrigSymbolModifier8_t',
                                           'type' => 'uint8_t'
                                         },
              'BrigMemoryModifier8_t' => {
                                           'name' => 'BrigMemoryModifier8_t',
                                           'type' => 'uint8_t'
                                         },
              'BrigAtomicOperation8_t' => {
                                            'name' => 'BrigAtomicOperation8_t',
                                            'type' => 'uint8_t'
                                          },
              'BrigVersion32_t' => {
                                     'type' => 'uint32_t',
                                     'name' => 'BrigVersion32_t'
                                   },
              'BrigOperandOffset32_t' => {
                                           'defValue' => '0',
                                           'type' => 'uint32_t',
                                           'name' => 'BrigOperandOffset32_t',
                                           'wtype' => 'ItemRef<Operand>'
                                         },
              'BrigRound8_t' => {
                                  'type' => 'uint8_t',
                                  'name' => 'BrigRound8_t'
                                },
              'BrigSamplerCoordNormalization8_t' => {
                                                      'name' => 'BrigSamplerCoordNormalization8_t',
                                                      'type' => 'uint8_t'
                                                    },
              'BrigProfile8_t' => {
                                    'name' => 'BrigProfile8_t',
                                    'defValue' => 'Brig::BRIG_PROFILE_FULL',
                                    'type' => 'uint8_t'
                                  },
              'BrigSegCvtModifier8_t' => {
                                           'type' => 'uint8_t',
                                           'name' => 'BrigSegCvtModifier8_t'
                                         },
              'BrigMachineModel8_t' => {
                                         'name' => 'BrigMachineModel8_t',
                                         'type' => 'uint8_t',
                                         'defValue' => 'Brig::BRIG_MACHINE_LARGE'
                                       },
              'BrigType16_t' => {
                                  'type' => 'uint16_t',
                                  'name' => 'BrigType16_t'
                                },
              'BrigImageChannelType8_t' => {
                                             'name' => 'BrigImageChannelType8_t',
                                             'type' => 'uint8_t',
                                             'defValue' => 'Brig::BRIG_CHANNEL_TYPE_UNKNOWN'
                                           },
              'BrigInstKinds16_t' => {
                                       'name' => 'BrigInstKinds16_t',
                                       'type' => 'uint16_t'
                                     },
              'BrigOperandKinds16_t' => {
                                          'name' => 'BrigOperandKinds16_t',
                                          'type' => 'uint16_t'
                                        },
              'BrigSamplerQuery8_t' => {
                                         'name' => 'BrigSamplerQuery8_t',
                                         'type' => 'uint8_t'
                                       },
              'BrigImageChannelOrder8_t' => {
                                              'name' => 'BrigImageChannelOrder8_t',
                                              'type' => 'uint8_t',
                                              'defValue' => 'Brig::BRIG_CHANNEL_ORDER_UNKNOWN'
                                            },
              'BrigCodeOffset32_t' => {
                                        'type' => 'uint32_t',
                                        'defValue' => '0',
                                        'name' => 'BrigCodeOffset32_t',
                                        'wtype' => 'ItemRef<Inst>'
                                      },
              'BrigAlignment8_t' => {
                                      'type' => 'uint8_t',
                                      'name' => 'BrigAlignment8_t'
                                    },
              'BrigOpcode16_t' => {
                                    'type' => 'uint16_t',
                                    'name' => 'BrigOpcode16_t'
                                  },
              'BrigPack8_t' => {
                                 'name' => 'BrigPack8_t',
                                 'defValue' => 'Brig::BRIG_PACK_NONE',
                                 'type' => 'uint8_t'
                               },
              'BrigSegment8_t' => {
                                    'name' => 'BrigSegment8_t',
                                    'defValue' => '0',
                                    'type' => 'uint8_t'
                                  },
              'BrigMemoryOrder8_t' => {
                                        'name' => 'BrigMemoryOrder8_t',
                                        'type' => 'uint8_t',
                                        'defValue' => 'Brig::BRIG_MEMORY_ORDER_RELAXED'
                                      },
              'BrigCompareOperation8_t' => {
                                             'type' => 'uint8_t',
                                             'name' => 'BrigCompareOperation8_t'
                                           },
              'BrigDirectiveOffset32_t' => {
                                             'defValue' => '0',
                                             'type' => 'uint32_t',
                                             'name' => 'BrigDirectiveOffset32_t',
                                             'wtype' => 'ItemRef<Directive>'
                                           }
            };
