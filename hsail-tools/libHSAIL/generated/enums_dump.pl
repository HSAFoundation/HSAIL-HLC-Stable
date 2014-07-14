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
           'BrigImageGeometry' => {
                                    'entries' => [
                                                   {
                                                     'val' => '0',
                                                     'mnemo' => '1d',
                                                     'name' => 'BRIG_GEOMETRY_1D'
                                                   },
                                                   {
                                                     'val' => '1',
                                                     'mnemo' => '2d',
                                                     'name' => 'BRIG_GEOMETRY_2D'
                                                   },
                                                   {
                                                     'val' => '2',
                                                     'name' => 'BRIG_GEOMETRY_3D',
                                                     'mnemo' => '3d'
                                                   },
                                                   {
                                                     'val' => '3',
                                                     'name' => 'BRIG_GEOMETRY_1DA',
                                                     'mnemo' => '1da'
                                                   },
                                                   {
                                                     'mnemo' => '2da',
                                                     'name' => 'BRIG_GEOMETRY_2DA',
                                                     'val' => '4'
                                                   },
                                                   {
                                                     'name' => 'BRIG_GEOMETRY_1DB',
                                                     'mnemo' => '1db',
                                                     'val' => '5'
                                                   },
                                                   {
                                                     'mnemo' => '2ddepth',
                                                     'name' => 'BRIG_GEOMETRY_2DDEPTH',
                                                     'val' => '6'
                                                   },
                                                   {
                                                     'mnemo' => '2dadepth',
                                                     'name' => 'BRIG_GEOMETRY_2DADEPTH',
                                                     'val' => '7'
                                                   },
                                                   {
                                                     'name' => 'BRIG_GEOMETRY_UNKNOWN',
                                                     'mnemo' => ''
                                                   }
                                                 ],
                                    'tdcaption' => 'Geometry',
                                    'mnemo' => sub { "DUMMY" },
                                    'mnemo#calcState' => 'done',
                                    'name' => 'BrigImageGeometry',
                                    'mnemo_token' => 'EImageGeometry',
                                    'mnemo#deps' => []
                                  },
           'BrigAtomicOperation' => {
                                      'mnemo_context' => 'EInstModifierInstAtomicContext',
                                      'mnemo#deps' => [],
                                      'entries' => [
                                                     {
                                                       'mnemo' => 'add',
                                                       'name' => 'BRIG_ATOMIC_ADD',
                                                       'val' => '0'
                                                     },
                                                     {
                                                       'mnemo' => 'and',
                                                       'name' => 'BRIG_ATOMIC_AND',
                                                       'val' => '1'
                                                     },
                                                     {
                                                       'val' => '2',
                                                       'mnemo' => 'cas',
                                                       'name' => 'BRIG_ATOMIC_CAS'
                                                     },
                                                     {
                                                       'val' => '3',
                                                       'name' => 'BRIG_ATOMIC_DEC',
                                                       'mnemo' => 'dec'
                                                     },
                                                     {
                                                       'val' => '4',
                                                       'name' => 'BRIG_ATOMIC_EXCH',
                                                       'mnemo' => 'exch'
                                                     },
                                                     {
                                                       'mnemo' => 'inc',
                                                       'name' => 'BRIG_ATOMIC_INC',
                                                       'val' => '5'
                                                     },
                                                     {
                                                       'mnemo' => 'ld',
                                                       'name' => 'BRIG_ATOMIC_LD',
                                                       'val' => '6'
                                                     },
                                                     {
                                                       'mnemo' => 'max',
                                                       'name' => 'BRIG_ATOMIC_MAX',
                                                       'val' => '7'
                                                     },
                                                     {
                                                       'val' => '8',
                                                       'mnemo' => 'min',
                                                       'name' => 'BRIG_ATOMIC_MIN'
                                                     },
                                                     {
                                                       'val' => '9',
                                                       'mnemo' => 'or',
                                                       'name' => 'BRIG_ATOMIC_OR'
                                                     },
                                                     {
                                                       'mnemo' => 'st',
                                                       'name' => 'BRIG_ATOMIC_ST',
                                                       'val' => '10'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_SUB',
                                                       'mnemo' => 'sub',
                                                       'val' => '11'
                                                     },
                                                     {
                                                       'mnemo' => 'xor',
                                                       'name' => 'BRIG_ATOMIC_XOR',
                                                       'val' => '12'
                                                     },
                                                     {
                                                       'mnemo' => 'wait_eq',
                                                       'name' => 'BRIG_ATOMIC_WAIT_EQ',
                                                       'val' => '13'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_WAIT_NE',
                                                       'mnemo' => 'wait_ne',
                                                       'val' => '14'
                                                     },
                                                     {
                                                       'mnemo' => 'wait_lt',
                                                       'name' => 'BRIG_ATOMIC_WAIT_LT',
                                                       'val' => '15'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_WAIT_GTE',
                                                       'mnemo' => 'wait_gte',
                                                       'val' => '16'
                                                     },
                                                     {
                                                       'mnemo' => 'waittimeout_eq',
                                                       'name' => 'BRIG_ATOMIC_WAITTIMEOUT_EQ',
                                                       'val' => '17'
                                                     },
                                                     {
                                                       'val' => '18',
                                                       'mnemo' => 'waittimeout_ne',
                                                       'name' => 'BRIG_ATOMIC_WAITTIMEOUT_NE'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_WAITTIMEOUT_LT',
                                                       'mnemo' => 'waittimeout_lt',
                                                       'val' => '19'
                                                     },
                                                     {
                                                       'name' => 'BRIG_ATOMIC_WAITTIMEOUT_GTE',
                                                       'mnemo' => 'waittimeout_gte',
                                                       'val' => '20'
                                                     }
                                                   ],
                                      'name' => 'BrigAtomicOperation',
                                      'mnemo_token' => '_EMAtomicOp',
                                      'tdcaption' => 'Atomic Operations',
                                      'mnemo#calcState' => 'done',
                                      'mnemo' => sub { "DUMMY" }
                                    },
           'BrigDirectiveKinds' => {
                                     'sizeof' => sub { "DUMMY" },
                                     'isBodyOnly#calcState' => 'done',
                                     'wname#calcState' => 'done',
                                     'isBodyOnly_default' => 'assert(false); return false',
                                     'sizeof#deps' => [
                                                        'wname'
                                                      ],
                                     'sizeof_switch' => 'true',
                                     'isBodyOnly' => sub { "DUMMY" },
                                     'isBodyOnly_switch' => 'true',
                                     'isToplevelOnly_proto' => 'bool isToplevelOnly(Directive d)',
                                     'isBodyOnly_proto' => 'bool isBodyOnly(Directive d)',
                                     'isToplevelOnly#deps' => [],
                                     'sizeof#calcState' => 'done',
                                     'isBodyOnly#deps' => [],
                                     'wname#deps' => [],
                                     'name' => 'BrigDirectiveKinds',
                                     'isToplevelOnly_default' => 'assert(false); return false',
                                     'isToplevelOnly_switch' => 'true',
                                     'sizeof_default' => 'return -1',
                                     'isToplevelOnly#calcState' => 'done',
                                     'isToplevelOnly' => sub { "DUMMY" },
                                     'isToplevelOnly_arg' => 'd.brig()->kind',
                                     'isBodyOnly_arg' => 'd.brig()->kind',
                                     'entries' => [
                                                    {
                                                      'wname' => 'DirectiveArgScopeEnd',
                                                      'isBodyOnly' => 'true',
                                                      'name' => 'BRIG_DIRECTIVE_ARG_SCOPE_END',
                                                      'sizeof' => 'sizeof(BrigDirectiveArgScopeEnd)',
                                                      'isToplevelOnly' => 'false',
                                                      'val' => '0'
                                                    },
                                                    {
                                                      'val' => '1',
                                                      'isToplevelOnly' => 'false',
                                                      'sizeof' => 'sizeof(BrigDirectiveArgScopeStart)',
                                                      'name' => 'BRIG_DIRECTIVE_ARG_SCOPE_START',
                                                      'isBodyOnly' => 'true',
                                                      'wname' => 'DirectiveArgScopeStart'
                                                    },
                                                    {
                                                      'wname' => 'BlockEnd',
                                                      'isBodyOnly' => 'false',
                                                      'name' => 'BRIG_DIRECTIVE_BLOCK_END',
                                                      'sizeof' => 'sizeof(BrigBlockEnd)',
                                                      'isToplevelOnly' => 'false',
                                                      'val' => '2'
                                                    },
                                                    {
                                                      'isBodyOnly' => 'false',
                                                      'name' => 'BRIG_DIRECTIVE_BLOCK_NUMERIC',
                                                      'wname' => 'BlockNumeric',
                                                      'isToplevelOnly' => 'false',
                                                      'val' => '3',
                                                      'sizeof' => 'sizeof(BrigBlockNumeric)'
                                                    },
                                                    {
                                                      'name' => 'BRIG_DIRECTIVE_BLOCK_START',
                                                      'isBodyOnly' => 'false',
                                                      'wname' => 'BlockStart',
                                                      'val' => '4',
                                                      'isToplevelOnly' => 'false',
                                                      'sizeof' => 'sizeof(BrigBlockStart)'
                                                    },
                                                    {
                                                      'name' => 'BRIG_DIRECTIVE_BLOCK_STRING',
                                                      'isBodyOnly' => 'false',
                                                      'wname' => 'BlockString',
                                                      'val' => '5',
                                                      'isToplevelOnly' => 'false',
                                                      'sizeof' => 'sizeof(BrigBlockString)'
                                                    },
                                                    {
                                                      'wname' => 'DirectiveComment',
                                                      'isBodyOnly' => 'false',
                                                      'name' => 'BRIG_DIRECTIVE_COMMENT',
                                                      'sizeof' => 'sizeof(BrigDirectiveComment)',
                                                      'isToplevelOnly' => 'false',
                                                      'val' => '6'
                                                    },
                                                    {
                                                      'val' => '7',
                                                      'isToplevelOnly' => 'false',
                                                      'sizeof' => 'sizeof(BrigDirectiveControl)',
                                                      'name' => 'BRIG_DIRECTIVE_CONTROL',
                                                      'isBodyOnly' => 'true',
                                                      'wname' => 'DirectiveControl'
                                                    },
                                                    {
                                                      'sizeof' => 'sizeof(BrigDirectiveExtension)',
                                                      'isToplevelOnly' => 'true',
                                                      'val' => '8',
                                                      'wname' => 'DirectiveExtension',
                                                      'isBodyOnly' => 'false',
                                                      'name' => 'BRIG_DIRECTIVE_EXTENSION'
                                                    },
                                                    {
                                                      'val' => '9',
                                                      'isToplevelOnly' => 'false',
                                                      'sizeof' => 'sizeof(BrigDirectiveFbarrier)',
                                                      'name' => 'BRIG_DIRECTIVE_FBARRIER',
                                                      'isBodyOnly' => 'false',
                                                      'wname' => 'DirectiveFbarrier'
                                                    },
                                                    {
                                                      'wname' => 'DirectiveFunction',
                                                      'isBodyOnly' => 'false',
                                                      'name' => 'BRIG_DIRECTIVE_FUNCTION',
                                                      'sizeof' => 'sizeof(BrigDirectiveFunction)',
                                                      'isToplevelOnly' => 'true',
                                                      'val' => '10'
                                                    },
                                                    {
                                                      'sizeof' => 'sizeof(BrigDirectiveImageInit)',
                                                      'val' => '11',
                                                      'isToplevelOnly' => 'true',
                                                      'wname' => 'DirectiveImageInit',
                                                      'name' => 'BRIG_DIRECTIVE_IMAGE_INIT',
                                                      'isBodyOnly' => 'false'
                                                    },
                                                    {
                                                      'isBodyOnly' => 'false',
                                                      'name' => 'BRIG_DIRECTIVE_KERNEL',
                                                      'wname' => 'DirectiveKernel',
                                                      'isToplevelOnly' => 'true',
                                                      'val' => '12',
                                                      'sizeof' => 'sizeof(BrigDirectiveKernel)'
                                                    },
                                                    {
                                                      'val' => '13',
                                                      'isToplevelOnly' => 'false',
                                                      'sizeof' => 'sizeof(BrigDirectiveLabel)',
                                                      'name' => 'BRIG_DIRECTIVE_LABEL',
                                                      'isBodyOnly' => 'true',
                                                      'wname' => 'DirectiveLabel'
                                                    },
                                                    {
                                                      'sizeof' => 'sizeof(BrigDirectiveLabelInit)',
                                                      'isToplevelOnly' => 'false',
                                                      'val' => '14',
                                                      'wname' => 'DirectiveLabelInit',
                                                      'isBodyOnly' => 'true',
                                                      'name' => 'BRIG_DIRECTIVE_LABEL_INIT'
                                                    },
                                                    {
                                                      'isToplevelOnly' => 'false',
                                                      'val' => '15',
                                                      'sizeof' => 'sizeof(BrigDirectiveLabelTargets)',
                                                      'isBodyOnly' => 'true',
                                                      'name' => 'BRIG_DIRECTIVE_LABEL_TARGETS',
                                                      'wname' => 'DirectiveLabelTargets'
                                                    },
                                                    {
                                                      'wname' => 'DirectiveLoc',
                                                      'name' => 'BRIG_DIRECTIVE_LOC',
                                                      'isBodyOnly' => 'false',
                                                      'sizeof' => 'sizeof(BrigDirectiveLoc)',
                                                      'val' => '16',
                                                      'isToplevelOnly' => 'false'
                                                    },
                                                    {
                                                      'isBodyOnly' => 'false',
                                                      'name' => 'BRIG_DIRECTIVE_PRAGMA',
                                                      'wname' => 'DirectivePragma',
                                                      'isToplevelOnly' => 'false',
                                                      'val' => '17',
                                                      'sizeof' => 'sizeof(BrigDirectivePragma)'
                                                    },
                                                    {
                                                      'isToplevelOnly' => 'true',
                                                      'val' => '18',
                                                      'sizeof' => 'sizeof(BrigDirectiveSamplerInit)',
                                                      'isBodyOnly' => 'false',
                                                      'name' => 'BRIG_DIRECTIVE_SAMPLER_INIT',
                                                      'wname' => 'DirectiveSamplerInit'
                                                    },
                                                    {
                                                      'isToplevelOnly' => 'true',
                                                      'val' => '19',
                                                      'sizeof' => 'sizeof(BrigDirectiveSignature)',
                                                      'isBodyOnly' => 'false',
                                                      'name' => 'BRIG_DIRECTIVE_SIGNATURE',
                                                      'wname' => 'DirectiveSignature'
                                                    },
                                                    {
                                                      'isBodyOnly' => 'false',
                                                      'name' => 'BRIG_DIRECTIVE_VARIABLE',
                                                      'wname' => 'DirectiveVariable',
                                                      'isToplevelOnly' => 'false',
                                                      'val' => '20',
                                                      'sizeof' => 'sizeof(BrigDirectiveVariable)'
                                                    },
                                                    {
                                                      'wname' => 'DirectiveVariableInit',
                                                      'name' => 'BRIG_DIRECTIVE_VARIABLE_INIT',
                                                      'isBodyOnly' => 'false',
                                                      'sizeof' => 'sizeof(BrigDirectiveVariableInit)',
                                                      'val' => '21',
                                                      'isToplevelOnly' => 'false'
                                                    },
                                                    {
                                                      'sizeof' => 'sizeof(BrigDirectiveVersion)',
                                                      'isToplevelOnly' => 'true',
                                                      'val' => '22',
                                                      'wname' => 'DirectiveVersion',
                                                      'isBodyOnly' => 'false',
                                                      'name' => 'BRIG_DIRECTIVE_VERSION'
                                                    },
                                                    {
                                                      'val' => '23',
                                                      'isToplevelOnly' => 'true',
                                                      'sizeof' => 'sizeof(BrigDirectiveImageProperties)',
                                                      'name' => 'BRIG_DIRECTIVE_IMAGE_PROPERTIES',
                                                      'isBodyOnly' => 'false',
                                                      'wname' => 'DirectiveImageProperties'
                                                    },
                                                    {
                                                      'sizeof' => 'sizeof(BrigDirectiveSamplerProperties)',
                                                      'val' => '24 ',
                                                      'isToplevelOnly' => 'true',
                                                      'wname' => 'DirectiveSamplerProperties',
                                                      'name' => 'BRIG_DIRECTIVE_SAMPLER_PROPERTIES',
                                                      'isBodyOnly' => 'false'
                                                    }
                                                  ],
                                     'wname' => sub { "DUMMY" },
                                     'sizeof_proto' => 'int size_of_directive(unsigned arg)'
                                   },
           'BrigExecuteableModifierMask' => {
                                              'entries' => [
                                                             {
                                                               'name' => 'BRIG_EXECUTABLE_LINKAGE',
                                                               'val' => '3'
                                                             },
                                                             {
                                                               'name' => 'BRIG_EXECUTABLE_DECLARATION',
                                                               'val' => '4'
                                                             }
                                                           ],
                                              'name' => 'BrigExecuteableModifierMask',
                                              'nodump' => 'true'
                                            },
           'BrigPackedTypeBits' => {
                                     'nodump' => 'true',
                                     'entries' => [
                                                    {
                                                      'name' => 'BRIG_TYPE_PACK_SHIFT',
                                                      'val' => '5'
                                                    },
                                                    {
                                                      'name' => 'BRIG_TYPE_BASE_MASK',
                                                      'val' => '(1 << BRIG_TYPE_PACK_SHIFT) - 1'
                                                    },
                                                    {
                                                      'val' => '3 << BRIG_TYPE_PACK_SHIFT',
                                                      'name' => 'BRIG_TYPE_PACK_MASK'
                                                    },
                                                    {
                                                      'val' => '0 << BRIG_TYPE_PACK_SHIFT',
                                                      'name' => 'BRIG_TYPE_PACK_NONE'
                                                    },
                                                    {
                                                      'val' => '1 << BRIG_TYPE_PACK_SHIFT',
                                                      'name' => 'BRIG_TYPE_PACK_32'
                                                    },
                                                    {
                                                      'val' => '2 << BRIG_TYPE_PACK_SHIFT',
                                                      'name' => 'BRIG_TYPE_PACK_64'
                                                    },
                                                    {
                                                      'val' => '3 << BRIG_TYPE_PACK_SHIFT',
                                                      'name' => 'BRIG_TYPE_PACK_128'
                                                    }
                                                  ],
                                     'name' => 'BrigPackedTypeBits'
                                   },
           'BrigSegment' => {
                              'mnemo_token' => '_EMSegment',
                              'mnemo#deps' => [],
                              'mnemo_context' => 'EInstModifierContext',
                              'entries' => [
                                             {
                                               'name' => 'BRIG_SEGMENT_NONE',
                                               'mnemo' => '',
                                               'val' => '0'
                                             },
                                             {
                                               'name' => 'BRIG_SEGMENT_FLAT',
                                               'mnemo' => '',
                                               'val' => '1'
                                             },
                                             {
                                               'mnemo' => 'global',
                                               'name' => 'BRIG_SEGMENT_GLOBAL',
                                               'val' => '2'
                                             },
                                             {
                                               'name' => 'BRIG_SEGMENT_READONLY',
                                               'mnemo' => 'readonly',
                                               'val' => '3'
                                             },
                                             {
                                               'val' => '4',
                                               'mnemo' => 'kernarg',
                                               'name' => 'BRIG_SEGMENT_KERNARG'
                                             },
                                             {
                                               'mnemo' => 'group',
                                               'name' => 'BRIG_SEGMENT_GROUP',
                                               'val' => '5'
                                             },
                                             {
                                               'mnemo' => 'private',
                                               'name' => 'BRIG_SEGMENT_PRIVATE',
                                               'val' => '6'
                                             },
                                             {
                                               'name' => 'BRIG_SEGMENT_SPILL',
                                               'mnemo' => 'spill',
                                               'val' => '7'
                                             },
                                             {
                                               'val' => '8',
                                               'mnemo' => 'arg',
                                               'name' => 'BRIG_SEGMENT_ARG'
                                             },
                                             {
                                               'val' => '9',
                                               'mnemo' => 'region',
                                               'name' => 'BRIG_SEGMENT_EXTSPACE0'
                                             }
                                           ],
                              'mnemo' => sub { "DUMMY" },
                              'mnemo#calcState' => 'done',
                              'name' => 'BrigSegment'
                            },
           'BrigMemoryOrder' => {
                                  'mnemo_token' => '_EMMemoryOrder',
                                  'mnemo#deps' => [],
                                  'entries' => [
                                                 {
                                                   'val' => '0',
                                                   'name' => 'BRIG_MEMORY_ORDER_NONE',
                                                   'mnemo' => ''
                                                 },
                                                 {
                                                   'name' => 'BRIG_MEMORY_ORDER_RELAXED',
                                                   'mnemo' => 'rlx',
                                                   'val' => '1'
                                                 },
                                                 {
                                                   'mnemo' => 'acq',
                                                   'name' => 'BRIG_MEMORY_ORDER_ACQUIRE',
                                                   'val' => '2'
                                                 },
                                                 {
                                                   'name' => 'BRIG_MEMORY_ORDER_RELEASE',
                                                   'mnemo' => 'rel',
                                                   'val' => '3'
                                                 },
                                                 {
                                                   'val' => '4         ',
                                                   'name' => 'BRIG_MEMORY_ORDER_ACQUIRE_RELEASE',
                                                   'mnemo' => 'ar'
                                                 }
                                               ],
                                  'name' => 'BrigMemoryOrder',
                                  'mnemo' => sub { "DUMMY" },
                                  'mnemo#calcState' => 'done'
                                },
           'BrigSegCvtModifierMask' => {
                                         'entries' => [
                                                        {
                                                          'val' => '1                         ',
                                                          'name' => 'BRIG_SEG_CVT_NONULL',
                                                          'mnemo' => 'nonull'
                                                        }
                                                      ],
                                         'name' => 'BrigSegCvtModifierMask'
                                       },
           'BrigInstKinds' => {
                                'sizeof#calcState' => 'done',
                                'sizeof_default' => 'return -1',
                                'sizeof_proto' => 'int size_of_inst(unsigned arg)',
                                'name' => 'BrigInstKinds',
                                'wname#deps' => [],
                                'wname' => sub { "DUMMY" },
                                'entries' => [
                                               {
                                                 'wname' => 'InstNone',
                                                 'name' => 'BRIG_INST_NONE',
                                                 'skip' => 'true',
                                                 'sizeof' => 'sizeof(BrigInstNone)',
                                                 'val' => '0'
                                               },
                                               {
                                                 'name' => 'BRIG_INST_BASIC',
                                                 'wname' => 'InstBasic',
                                                 'val' => '1',
                                                 'sizeof' => 'sizeof(BrigInstBasic)'
                                               },
                                               {
                                                 'wname' => 'InstAddr',
                                                 'name' => 'BRIG_INST_ADDR',
                                                 'sizeof' => 'sizeof(BrigInstAddr)',
                                                 'val' => '2'
                                               },
                                               {
                                                 'sizeof' => 'sizeof(BrigInstAtomic)',
                                                 'val' => '3',
                                                 'wname' => 'InstAtomic',
                                                 'name' => 'BRIG_INST_ATOMIC'
                                               },
                                               {
                                                 'wname' => 'InstBr',
                                                 'name' => 'BRIG_INST_BR',
                                                 'sizeof' => 'sizeof(BrigInstBr)',
                                                 'val' => '4'
                                               },
                                               {
                                                 'wname' => 'InstCmp',
                                                 'name' => 'BRIG_INST_CMP',
                                                 'sizeof' => 'sizeof(BrigInstCmp)',
                                                 'val' => '5'
                                               },
                                               {
                                                 'name' => 'BRIG_INST_CVT',
                                                 'wname' => 'InstCvt',
                                                 'val' => '6',
                                                 'sizeof' => 'sizeof(BrigInstCvt)'
                                               },
                                               {
                                                 'val' => '7',
                                                 'sizeof' => 'sizeof(BrigInstImage)',
                                                 'name' => 'BRIG_INST_IMAGE',
                                                 'wname' => 'InstImage'
                                               },
                                               {
                                                 'name' => 'BRIG_INST_LANE',
                                                 'wname' => 'InstLane',
                                                 'val' => '8',
                                                 'sizeof' => 'sizeof(BrigInstLane)'
                                               },
                                               {
                                                 'sizeof' => 'sizeof(BrigInstMem)',
                                                 'val' => '9',
                                                 'wname' => 'InstMem',
                                                 'name' => 'BRIG_INST_MEM'
                                               },
                                               {
                                                 'sizeof' => 'sizeof(BrigInstMemFence)',
                                                 'val' => '10',
                                                 'wname' => 'InstMemFence',
                                                 'name' => 'BRIG_INST_MEM_FENCE'
                                               },
                                               {
                                                 'name' => 'BRIG_INST_MOD',
                                                 'wname' => 'InstMod',
                                                 'val' => '11',
                                                 'sizeof' => 'sizeof(BrigInstMod)'
                                               },
                                               {
                                                 'val' => '12',
                                                 'sizeof' => 'sizeof(BrigInstQueryImage)',
                                                 'name' => 'BRIG_INST_QUERY_IMAGE',
                                                 'wname' => 'InstQueryImage'
                                               },
                                               {
                                                 'name' => 'BRIG_INST_QUERY_SAMPLER',
                                                 'wname' => 'InstQuerySampler',
                                                 'val' => '13',
                                                 'sizeof' => 'sizeof(BrigInstQuerySampler)'
                                               },
                                               {
                                                 'val' => '14',
                                                 'sizeof' => 'sizeof(BrigInstQueue)',
                                                 'name' => 'BRIG_INST_QUEUE',
                                                 'wname' => 'InstQueue'
                                               },
                                               {
                                                 'val' => '15',
                                                 'sizeof' => 'sizeof(BrigInstSeg)',
                                                 'name' => 'BRIG_INST_SEG',
                                                 'wname' => 'InstSeg'
                                               },
                                               {
                                                 'wname' => 'InstSegCvt',
                                                 'name' => 'BRIG_INST_SEG_CVT',
                                                 'sizeof' => 'sizeof(BrigInstSegCvt)',
                                                 'val' => '16'
                                               },
                                               {
                                                 'val' => '17',
                                                 'sizeof' => 'sizeof(BrigInstSignal)',
                                                 'name' => 'BRIG_INST_SIGNAL',
                                                 'wname' => 'InstSignal'
                                               },
                                               {
                                                 'name' => 'BRIG_INST_SOURCE_TYPE',
                                                 'wname' => 'InstSourceType',
                                                 'val' => '18',
                                                 'sizeof' => 'sizeof(BrigInstSourceType)'
                                               }
                                             ],
                                'wname#calcState' => 'done',
                                'sizeof#deps' => [
                                                   'wname'
                                                 ],
                                'sizeof' => sub { "DUMMY" },
                                'sizeof_switch' => 'true'
                              },
           'BrigMemoryFenceSegments' => {
                                          'entries' => [
                                                         {
                                                           'name' => 'BRIG_MEMORY_FENCE_NONE',
                                                           'mnemo' => 'none',
                                                           'val' => '0',
                                                           'skip' => 'true'
                                                         },
                                                         {
                                                           'val' => '1',
                                                           'mnemo' => 'group',
                                                           'name' => 'BRIG_MEMORY_FENCE_GROUP'
                                                         },
                                                         {
                                                           'val' => '2',
                                                           'name' => 'BRIG_MEMORY_FENCE_GLOBAL',
                                                           'mnemo' => 'global'
                                                         },
                                                         {
                                                           'name' => 'BRIG_MEMORY_FENCE_BOTH',
                                                           'mnemo' => '',
                                                           'val' => '3'
                                                         },
                                                         {
                                                           'val' => '4',
                                                           'name' => 'BRIG_MEMORY_FENCE_IMAGE',
                                                           'mnemo' => 'image'
                                                         }
                                                       ],
                                          'name' => 'BrigMemoryFenceSegments',
                                          'mnemo' => sub { "DUMMY" },
                                          'mnemo#calcState' => 'done',
                                          'mnemo_token' => '_EMMemoryFenceSegments',
                                          'mnemo#deps' => [],
                                          'mnemo_context' => 'EInstModifierInstFenceContext'
                                        },
           'BrigImageChannelType' => {
                                       'mnemo_token' => 'EImageFormat',
                                       'mnemo#deps' => [],
                                       'entries' => [
                                                      {
                                                        'name' => 'BRIG_CHANNEL_TYPE_SNORM_INT8',
                                                        'mnemo' => 'snorm_int8',
                                                        'val' => '0'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CHANNEL_TYPE_SNORM_INT16',
                                                        'mnemo' => 'snorm_int16',
                                                        'val' => '1'
                                                      },
                                                      {
                                                        'mnemo' => 'unorm_int8',
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNORM_INT8',
                                                        'val' => '2'
                                                      },
                                                      {
                                                        'val' => '3',
                                                        'mnemo' => 'unorm_int16',
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNORM_INT16'
                                                      },
                                                      {
                                                        'val' => '4',
                                                        'mnemo' => 'unorm_int24',
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNORM_INT24'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNORM_SHORT_555',
                                                        'mnemo' => 'unorm_short_555',
                                                        'val' => '5'
                                                      },
                                                      {
                                                        'mnemo' => 'unorm_short_565',
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNORM_SHORT_565',
                                                        'val' => '6'
                                                      },
                                                      {
                                                        'val' => '7',
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNORM_SHORT_101010',
                                                        'mnemo' => 'unorm_short_101010'
                                                      },
                                                      {
                                                        'val' => '8',
                                                        'name' => 'BRIG_CHANNEL_TYPE_SIGNED_INT8',
                                                        'mnemo' => 'signed_int8'
                                                      },
                                                      {
                                                        'val' => '9',
                                                        'name' => 'BRIG_CHANNEL_TYPE_SIGNED_INT16',
                                                        'mnemo' => 'signed_int16'
                                                      },
                                                      {
                                                        'mnemo' => 'signed_int32',
                                                        'name' => 'BRIG_CHANNEL_TYPE_SIGNED_INT32',
                                                        'val' => '10'
                                                      },
                                                      {
                                                        'val' => '11',
                                                        'mnemo' => 'unsigned_int8',
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNSIGNED_INT8'
                                                      },
                                                      {
                                                        'mnemo' => 'unsigned_int16',
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNSIGNED_INT16',
                                                        'val' => '12'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNSIGNED_INT32',
                                                        'mnemo' => 'unsigned_int32',
                                                        'val' => '13'
                                                      },
                                                      {
                                                        'val' => '14',
                                                        'name' => 'BRIG_CHANNEL_TYPE_HALF_FLOAT',
                                                        'mnemo' => 'half_float'
                                                      },
                                                      {
                                                        'mnemo' => 'float',
                                                        'name' => 'BRIG_CHANNEL_TYPE_FLOAT',
                                                        'val' => '15'
                                                      },
                                                      {
                                                        'mnemo' => '',
                                                        'name' => 'BRIG_CHANNEL_TYPE_UNKNOWN'
                                                      }
                                                    ],
                                       'mnemo' => sub { "DUMMY" },
                                       'mnemo#calcState' => 'done',
                                       'name' => 'BrigImageChannelType'
                                     },
           'BrigSamplerFilter' => {
                                    'mnemo#deps' => [],
                                    'entries' => [
                                                   {
                                                     'name' => 'BRIG_FILTER_NEAREST',
                                                     'mnemo' => 'nearest',
                                                     'val' => '0'
                                                   },
                                                   {
                                                     'val' => '1',
                                                     'name' => 'BRIG_FILTER_LINEAR',
                                                     'mnemo' => 'linear'
                                                   }
                                                 ],
                                    'name' => 'BrigSamplerFilter',
                                    'mnemo' => sub { "DUMMY" },
                                    'mnemo#calcState' => 'done'
                                  },
           'BrigOperandKinds' => {
                                   'sizeof#calcState' => 'done',
                                   'sizeof_default' => 'return -1',
                                   'name' => 'BrigOperandKinds',
                                   'sizeof_proto' => 'int size_of_operand(unsigned arg)',
                                   'entries' => [
                                                  {
                                                    'val' => '0',
                                                    'sizeof' => 'sizeof(BrigOperandImmed)',
                                                    'name' => 'BRIG_OPERAND_IMMED',
                                                    'wname' => 'OperandImmed'
                                                  },
                                                  {
                                                    'val' => '1',
                                                    'sizeof' => 'sizeof(BrigOperandWavesize)',
                                                    'name' => 'BRIG_OPERAND_WAVESIZE',
                                                    'wname' => 'OperandWavesize'
                                                  },
                                                  {
                                                    'sizeof' => 'sizeof(BrigOperandReg)',
                                                    'val' => '2',
                                                    'wname' => 'OperandReg',
                                                    'name' => 'BRIG_OPERAND_REG'
                                                  },
                                                  {
                                                    'sizeof' => 'sizeof(BrigOperandVector)',
                                                    'val' => '3',
                                                    'wname' => 'OperandVector',
                                                    'name' => 'BRIG_OPERAND_VECTOR'
                                                  },
                                                  {
                                                    'sizeof' => 'sizeof(BrigOperandAddress)',
                                                    'val' => '4',
                                                    'wname' => 'OperandAddress',
                                                    'name' => 'BRIG_OPERAND_ADDRESS'
                                                  },
                                                  {
                                                    'sizeof' => 'sizeof(BrigOperandArgumentList)',
                                                    'val' => '5',
                                                    'wname' => 'OperandArgumentList',
                                                    'name' => 'BRIG_OPERAND_ARGUMENT_LIST'
                                                  },
                                                  {
                                                    'wname' => 'OperandFunctionList',
                                                    'name' => 'BRIG_OPERAND_FUNCTION_LIST',
                                                    'sizeof' => 'sizeof(BrigOperandFunctionList)',
                                                    'val' => '6'
                                                  },
                                                  {
                                                    'wname' => 'OperandFbarrierRef',
                                                    'name' => 'BRIG_OPERAND_FBARRIER_REF',
                                                    'sizeof' => 'sizeof(BrigOperandFbarrierRef)',
                                                    'val' => '7'
                                                  },
                                                  {
                                                    'name' => 'BRIG_OPERAND_FUNCTION_REF',
                                                    'wname' => 'OperandFunctionRef',
                                                    'val' => '8',
                                                    'sizeof' => 'sizeof(BrigOperandFunctionRef)'
                                                  },
                                                  {
                                                    'wname' => 'OperandLabelRef',
                                                    'name' => 'BRIG_OPERAND_LABEL_REF',
                                                    'sizeof' => 'sizeof(BrigOperandLabelRef)',
                                                    'val' => '9'
                                                  },
                                                  {
                                                    'sizeof' => 'sizeof(BrigOperandLabelTargetsRef)',
                                                    'val' => '10',
                                                    'wname' => 'OperandLabelTargetsRef',
                                                    'name' => 'BRIG_OPERAND_LABEL_TARGETS_REF'
                                                  },
                                                  {
                                                    'wname' => 'OperandSignatureRef',
                                                    'name' => 'BRIG_OPERAND_SIGNATURE_REF',
                                                    'sizeof' => 'sizeof(BrigOperandSignatureRef)',
                                                    'val' => '11'
                                                  },
                                                  {
                                                    'val' => '12',
                                                    'sizeof' => 'sizeof(BrigOperandLabelVariableRef)',
                                                    'name' => 'BRIG_OPERAND_LABEL_VARIABLE_REF',
                                                    'wname' => 'OperandLabelVariableRef'
                                                  }
                                                ],
                                   'wname#deps' => [],
                                   'wname' => sub { "DUMMY" },
                                   'wname#calcState' => 'done',
                                   'sizeof#deps' => [
                                                      'wname'
                                                    ],
                                   'sizeof' => sub { "DUMMY" },
                                   'sizeof_switch' => 'true'
                                 },
           'BrigControlDirective' => {
                                       'mnemo#deps' => [],
                                       'mnemo_token' => 'EControl',
                                       'mnemo#calcState' => 'done',
                                       'mnemo' => sub { "DUMMY" },
                                       'name' => 'BrigControlDirective',
                                       'entries' => [
                                                      {
                                                        'mnemo' => 'none',
                                                        'name' => 'BRIG_CONTROL_NONE',
                                                        'val' => '0',
                                                        'skip' => 'true'
                                                      },
                                                      {
                                                        'val' => '1',
                                                        'name' => 'BRIG_CONTROL_ENABLEBREAKEXCEPTIONS',
                                                        'mnemo' => 'enablebreakexceptions'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CONTROL_ENABLEDETECTEXCEPTIONS',
                                                        'mnemo' => 'enabledetectexceptions',
                                                        'val' => '2'
                                                      },
                                                      {
                                                        'val' => '3',
                                                        'name' => 'BRIG_CONTROL_MAXDYNAMICGROUPSIZE',
                                                        'mnemo' => 'maxdynamicgroupsize'
                                                      },
                                                      {
                                                        'val' => '4',
                                                        'mnemo' => 'maxflatgridsize',
                                                        'name' => 'BRIG_CONTROL_MAXFLATGRIDSIZE'
                                                      },
                                                      {
                                                        'val' => '5',
                                                        'name' => 'BRIG_CONTROL_MAXFLATWORKGROUPSIZE',
                                                        'mnemo' => 'maxflatworkgroupsize'
                                                      },
                                                      {
                                                        'val' => '6',
                                                        'name' => 'BRIG_CONTROL_REQUESTEDWORKGROUPSPERCU',
                                                        'mnemo' => 'requestedworkgroupspercu'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CONTROL_REQUIREDDIM',
                                                        'mnemo' => 'requireddim',
                                                        'val' => '7'
                                                      },
                                                      {
                                                        'val' => '8',
                                                        'mnemo' => 'requiredgridsize',
                                                        'name' => 'BRIG_CONTROL_REQUIREDGRIDSIZE'
                                                      },
                                                      {
                                                        'val' => '9',
                                                        'mnemo' => 'requiredworkgroupsize',
                                                        'name' => 'BRIG_CONTROL_REQUIREDWORKGROUPSIZE'
                                                      },
                                                      {
                                                        'name' => 'BRIG_CONTROL_REQUIRENOPARTIALWORKGROUPS',
                                                        'mnemo' => 'requirenopartialworkgroups',
                                                        'val' => '10'
                                                      }
                                                    ]
                                     },
           'BrigLinkage' => {
                              'entries' => [
                                             {
                                               'mnemo' => '',
                                               'name' => 'BRIG_LINKAGE_NONE',
                                               'val' => '0'
                                             },
                                             {
                                               'val' => '1',
                                               'name' => 'BRIG_LINKAGE_STATIC',
                                               'mnemo' => 'static'
                                             },
                                             {
                                               'name' => 'BRIG_LINKAGE_EXTERN',
                                               'mnemo' => 'extern',
                                               'val' => '2'
                                             }
                                           ],
                              'name' => 'BrigLinkage',
                              'mnemo' => sub { "DUMMY" },
                              'mnemo#calcState' => 'done',
                              'mnemo#deps' => []
                            },
           'BrigMemoryScope' => {
                                  'mnemo#deps' => [],
                                  'mnemo_token' => '_EMMemoryScope',
                                  'name' => 'BrigMemoryScope',
                                  'mnemo#calcState' => 'done',
                                  'mnemo' => sub { "DUMMY" },
                                  'entries' => [
                                                 {
                                                   'val' => '0',
                                                   'name' => 'BRIG_MEMORY_SCOPE_NONE',
                                                   'mnemo' => ''
                                                 },
                                                 {
                                                   'name' => 'BRIG_MEMORY_SCOPE_WAVEFRONT',
                                                   'mnemo' => 'wv',
                                                   'val' => '1'
                                                 },
                                                 {
                                                   'val' => '2',
                                                   'name' => 'BRIG_MEMORY_SCOPE_WORKGROUP',
                                                   'mnemo' => 'wg'
                                                 },
                                                 {
                                                   'val' => '3',
                                                   'name' => 'BRIG_MEMORY_SCOPE_COMPONENT',
                                                   'mnemo' => 'cmp'
                                                 },
                                                 {
                                                   'name' => 'BRIG_MEMORY_SCOPE_SYSTEM',
                                                   'mnemo' => 'sys',
                                                   'val' => '4'
                                                 },
                                                 {
                                                   'name' => 'BRIG_MEMORY_SCOPE_WORKITEM',
                                                   'mnemo' => 'wi',
                                                   'val' => '5                    '
                                                 }
                                               ]
                                },
           'BrigSamplerQuery' => {
                                   'mnemo#deps' => [],
                                   'mnemo_token' => '_EMSamplerQuery',
                                   'name' => 'BrigSamplerQuery',
                                   'mnemo#calcState' => 'done',
                                   'mnemo' => sub { "DUMMY" },
                                   'entries' => [
                                                  {
                                                    'val' => '0',
                                                    'mnemo' => 'addressing',
                                                    'name' => 'BRIG_SAMPLER_QUERY_ADDRESSING'
                                                  },
                                                  {
                                                    'mnemo' => 'coord',
                                                    'name' => 'BRIG_SAMPLER_QUERY_COORD',
                                                    'val' => '1'
                                                  },
                                                  {
                                                    'name' => 'BRIG_SAMPLER_QUERY_FILTER',
                                                    'mnemo' => 'filter',
                                                    'val' => '2'
                                                  }
                                                ]
                                 },
           'BrigProfile' => {
                              'mnemo_token' => 'ETargetProfile',
                              'mnemo#deps' => [],
                              'entries' => [
                                             {
                                               'mnemo' => '$base',
                                               'name' => 'BRIG_PROFILE_BASE',
                                               'val' => '0'
                                             },
                                             {
                                               'name' => 'BRIG_PROFILE_FULL',
                                               'mnemo' => '$full',
                                               'val' => '1'
                                             },
                                             {
                                               'name' => 'BRIG_PROFILE_UNDEF',
                                               'mnemo' => '$undef',
                                               'skip' => 'true',
                                               'val' => '2  '
                                             }
                                           ],
                              'mnemo' => sub { "DUMMY" },
                              'mnemo#calcState' => 'done',
                              'name' => 'BrigProfile'
                            },
           'BrigMachineModel' => {
                                   'mnemo#deps' => [],
                                   'mnemo_token' => 'ETargetMachine',
                                   'name' => 'BrigMachineModel',
                                   'mnemo#calcState' => 'done',
                                   'mnemo' => sub { "DUMMY" },
                                   'entries' => [
                                                  {
                                                    'mnemo' => '$small',
                                                    'name' => 'BRIG_MACHINE_SMALL',
                                                    'val' => '0'
                                                  },
                                                  {
                                                    'val' => '1',
                                                    'mnemo' => '$large',
                                                    'name' => 'BRIG_MACHINE_LARGE'
                                                  },
                                                  {
                                                    'val' => '2  ',
                                                    'skip' => 'true',
                                                    'mnemo' => '$undef',
                                                    'name' => 'BRIG_MACHINE_UNDEF'
                                                  }
                                                ]
                                 },
           'BrigAluModifierMask' => {
                                      'name' => 'BrigAluModifierMask',
                                      'entries' => [
                                                     {
                                                       'name' => 'BRIG_ALU_ROUND',
                                                       'val' => '15'
                                                     },
                                                     {
                                                       'val' => '16',
                                                       'name' => 'BRIG_ALU_FTZ'
                                                     }
                                                   ]
                                    },
           'BrigRound' => {
                            'mnemo_token' => '_EMRound',
                            'entries' => [
                                           {
                                             'name' => 'BRIG_ROUND_NONE',
                                             'val' => '0'
                                           },
                                           {
                                             'val' => '1',
                                             'name' => 'BRIG_ROUND_FLOAT_NEAR_EVEN',
                                             'mnemo' => 'near'
                                           },
                                           {
                                             'mnemo' => 'zero',
                                             'name' => 'BRIG_ROUND_FLOAT_ZERO',
                                             'val' => '2'
                                           },
                                           {
                                             'val' => '3',
                                             'mnemo' => 'up',
                                             'name' => 'BRIG_ROUND_FLOAT_PLUS_INFINITY'
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
                                             'mnemo' => 'zeroi',
                                             'name' => 'BRIG_ROUND_INTEGER_ZERO',
                                             'val' => '6'
                                           },
                                           {
                                             'val' => '7',
                                             'name' => 'BRIG_ROUND_INTEGER_PLUS_INFINITY',
                                             'mnemo' => 'upi'
                                           },
                                           {
                                             'val' => '8',
                                             'name' => 'BRIG_ROUND_INTEGER_MINUS_INFINITY',
                                             'mnemo' => 'downi'
                                           },
                                           {
                                             'mnemo' => 'neari_sat',
                                             'name' => 'BRIG_ROUND_INTEGER_NEAR_EVEN_SAT',
                                             'val' => '9'
                                           },
                                           {
                                             'val' => '10',
                                             'name' => 'BRIG_ROUND_INTEGER_ZERO_SAT',
                                             'mnemo' => 'zeroi_sat'
                                           },
                                           {
                                             'val' => '11',
                                             'mnemo' => 'upi_sat',
                                             'name' => 'BRIG_ROUND_INTEGER_PLUS_INFINITY_SAT'
                                           },
                                           {
                                             'name' => 'BRIG_ROUND_INTEGER_MINUS_INFINITY_SAT',
                                             'mnemo' => 'downi_sat',
                                             'val' => '12     '
                                           }
                                         ],
                            'mnemo_fn' => 'round2str',
                            'name' => 'BrigRound',
                            'mnemo' => 'true'
                          },
           'BrigTypeX' => {
                            'length_proto' => 'int brigtype_get_length(unsigned arg)',
                            'length#calcState' => 'done',
                            'mnemo' => sub { "DUMMY" },
                            'dispatch_arg' => 'type',
                            'dispatch_default' => 'return v.visitNone(type)',
                            'dispatch#deps' => [],
                            'length_switch' => 'true',
                            'length_default' => 'return 0',
                            'name' => 'BrigTypeX',
                            'length#deps' => [],
                            'dispatch_proto' => 'template<typename RetType, typename Visitor>
RetType dispatchByType_gen(unsigned type, Visitor& v)',
                            'dispatch_incfile' => 'TemplateUtilities',
                            'dispatch' => sub { "DUMMY" },
                            'dispatch_switch' => 'true',
                            'mnemo#calcState' => 'done',
                            'mnemo_token' => '_EMType',
                            'entries' => [
                                           {
                                             'dispatch' => 'v.visitNone(type)',
                                             'name' => 'BRIG_TYPE_NONE',
                                             'mnemo' => '',
                                             'val' => '0',
                                             'length' => 0
                                           },
                                           {
                                             'ctype' => 'uint8_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U8> >()',
                                             'mnemo' => 'u8',
                                             'name' => 'BRIG_TYPE_U8',
                                             'length' => '8',
                                             'val' => '1'
                                           },
                                           {
                                             'length' => '16',
                                             'val' => '2',
                                             'mnemo' => 'u16',
                                             'name' => 'BRIG_TYPE_U16',
                                             'ctype' => 'uint16_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U16> >()'
                                           },
                                           {
                                             'length' => '32',
                                             'val' => '3',
                                             'mnemo' => 'u32',
                                             'name' => 'BRIG_TYPE_U32',
                                             'ctype' => 'uint32_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U32> >()'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_U64',
                                             'mnemo' => 'u64',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U64> >()',
                                             'ctype' => 'uint64_t',
                                             'val' => '4',
                                             'length' => '64'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_S8',
                                             'mnemo' => 's8',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S8> >()',
                                             'ctype' => 'int8_t',
                                             'val' => '5',
                                             'length' => '8'
                                           },
                                           {
                                             'mnemo' => 's16',
                                             'name' => 'BRIG_TYPE_S16',
                                             'ctype' => 'int16_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S16> >()',
                                             'length' => '16',
                                             'val' => '6'
                                           },
                                           {
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S32> >()',
                                             'ctype' => 'int32_t',
                                             'name' => 'BRIG_TYPE_S32',
                                             'mnemo' => 's32',
                                             'val' => '7',
                                             'length' => '32'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_S64',
                                             'mnemo' => 's64',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S64> >()',
                                             'ctype' => 'int64_t',
                                             'val' => '8',
                                             'length' => '64'
                                           },
                                           {
                                             'val' => '9',
                                             'length' => '16',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F16> >()',
                                             'ctype' => 'f16_t',
                                             'name' => 'BRIG_TYPE_F16',
                                             'mnemo' => 'f16'
                                           },
                                           {
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F32> >()',
                                             'ctype' => 'float',
                                             'name' => 'BRIG_TYPE_F32',
                                             'mnemo' => 'f32',
                                             'val' => '10',
                                             'length' => '32'
                                           },
                                           {
                                             'val' => '11',
                                             'length' => '64',
                                             'name' => 'BRIG_TYPE_F64',
                                             'mnemo' => 'f64',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F64> >()',
                                             'ctype' => 'double'
                                           },
                                           {
                                             'length' => '1',
                                             'val' => '12',
                                             'mnemo' => 'b1',
                                             'name' => 'BRIG_TYPE_B1',
                                             'ctype' => 'bool',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_B1> >()'
                                           },
                                           {
                                             'val' => '13',
                                             'length' => '8',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_B8> >()',
                                             'ctype' => 'uint8_t',
                                             'name' => 'BRIG_TYPE_B8',
                                             'mnemo' => 'b8'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_B16',
                                             'mnemo' => 'b16',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_B16> >()',
                                             'ctype' => 'uint16_t',
                                             'val' => '14',
                                             'length' => '16'
                                           },
                                           {
                                             'length' => '32',
                                             'val' => '15',
                                             'mnemo' => 'b32',
                                             'name' => 'BRIG_TYPE_B32',
                                             'ctype' => 'uint32_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_B32> >()'
                                           },
                                           {
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_B64> >()',
                                             'ctype' => 'uint64_t',
                                             'name' => 'BRIG_TYPE_B64',
                                             'mnemo' => 'b64',
                                             'val' => '16',
                                             'length' => '64'
                                           },
                                           {
                                             'length' => '128',
                                             'val' => '17',
                                             'ctype' => 'b128_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_B128> >()',
                                             'mnemo' => 'b128',
                                             'name' => 'BRIG_TYPE_B128'
                                           },
                                           {
                                             'mnemo' => 'samp',
                                             'name' => 'BRIG_TYPE_SAMP',
                                             'dispatch' => 'v.visitNone(type)',
                                             'length' => 0,
                                             'val' => '18'
                                           },
                                           {
                                             'length' => 0,
                                             'val' => '19',
                                             'mnemo' => 'roimg',
                                             'name' => 'BRIG_TYPE_ROIMG',
                                             'dispatch' => 'v.visitNone(type)'
                                           },
                                           {
                                             'mnemo' => 'woimg',
                                             'name' => 'BRIG_TYPE_WOIMG',
                                             'dispatch' => 'v.visitNone(type)',
                                             'length' => 0,
                                             'val' => '20'
                                           },
                                           {
                                             'dispatch' => 'v.visitNone(type)',
                                             'name' => 'BRIG_TYPE_RWIMG',
                                             'mnemo' => 'rwimg',
                                             'val' => '21',
                                             'length' => 0
                                           },
                                           {
                                             'mnemo' => 'sig32',
                                             'name' => 'BRIG_TYPE_SIG32',
                                             'dispatch' => 'v.visitNone(type)',
                                             'length' => '32',
                                             'val' => '22'
                                           },
                                           {
                                             'val' => '23',
                                             'length' => '64',
                                             'name' => 'BRIG_TYPE_SIG64',
                                             'mnemo' => 'sig64',
                                             'dispatch' => 'v.visitNone(type)'
                                           },
                                           {
                                             'val' => 'BRIG_TYPE_U8  | BRIG_TYPE_PACK_32',
                                             'length' => 32,
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U8X4> >()',
                                             'ctype' => 'uint8_t',
                                             'name' => 'BRIG_TYPE_U8X4',
                                             'mnemo' => 'u8x4'
                                           },
                                           {
                                             'name' => 'BRIG_TYPE_U8X8',
                                             'mnemo' => 'u8x8',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U8X8> >()',
                                             'ctype' => 'uint8_t',
                                             'val' => 'BRIG_TYPE_U8  | BRIG_TYPE_PACK_64',
                                             'length' => 64
                                           },
                                           {
                                             'mnemo' => 'u8x16',
                                             'name' => 'BRIG_TYPE_U8X16',
                                             'ctype' => 'uint8_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U8X16> >()',
                                             'length' => 128,
                                             'val' => 'BRIG_TYPE_U8  | BRIG_TYPE_PACK_128'
                                           },
                                           {
                                             'val' => 'BRIG_TYPE_U16 | BRIG_TYPE_PACK_32',
                                             'length' => 32,
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U16X2> >()',
                                             'ctype' => 'uint16_t',
                                             'name' => 'BRIG_TYPE_U16X2',
                                             'mnemo' => 'u16x2'
                                           },
                                           {
                                             'val' => 'BRIG_TYPE_U16 | BRIG_TYPE_PACK_64',
                                             'length' => 64,
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U16X4> >()',
                                             'ctype' => 'uint16_t',
                                             'name' => 'BRIG_TYPE_U16X4',
                                             'mnemo' => 'u16x4'
                                           },
                                           {
                                             'val' => 'BRIG_TYPE_U16 | BRIG_TYPE_PACK_128',
                                             'length' => 128,
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U16X8> >()',
                                             'ctype' => 'uint16_t',
                                             'name' => 'BRIG_TYPE_U16X8',
                                             'mnemo' => 'u16x8'
                                           },
                                           {
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U32X2> >()',
                                             'ctype' => 'uint32_t',
                                             'name' => 'BRIG_TYPE_U32X2',
                                             'mnemo' => 'u32x2',
                                             'val' => 'BRIG_TYPE_U32 | BRIG_TYPE_PACK_64',
                                             'length' => 64
                                           },
                                           {
                                             'mnemo' => 'u32x4',
                                             'name' => 'BRIG_TYPE_U32X4',
                                             'ctype' => 'uint32_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U32X4> >()',
                                             'length' => 128,
                                             'val' => 'BRIG_TYPE_U32 | BRIG_TYPE_PACK_128'
                                           },
                                           {
                                             'val' => 'BRIG_TYPE_U64 | BRIG_TYPE_PACK_128',
                                             'length' => 128,
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_U64X2> >()',
                                             'ctype' => 'uint64_t',
                                             'name' => 'BRIG_TYPE_U64X2',
                                             'mnemo' => 'u64x2'
                                           },
                                           {
                                             'length' => 32,
                                             'val' => 'BRIG_TYPE_S8  | BRIG_TYPE_PACK_32',
                                             'mnemo' => 's8x4',
                                             'name' => 'BRIG_TYPE_S8X4',
                                             'ctype' => 'int8_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S8X4> >()'
                                           },
                                           {
                                             'length' => 64,
                                             'val' => 'BRIG_TYPE_S8  | BRIG_TYPE_PACK_64',
                                             'mnemo' => 's8x8',
                                             'name' => 'BRIG_TYPE_S8X8',
                                             'ctype' => 'int8_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S8X8> >()'
                                           },
                                           {
                                             'length' => 128,
                                             'val' => 'BRIG_TYPE_S8  | BRIG_TYPE_PACK_128',
                                             'ctype' => 'int8_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S8X16> >()',
                                             'mnemo' => 's8x16',
                                             'name' => 'BRIG_TYPE_S8X16'
                                           },
                                           {
                                             'val' => 'BRIG_TYPE_S16 | BRIG_TYPE_PACK_32',
                                             'length' => 32,
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S16X2> >()',
                                             'ctype' => 'int16_t',
                                             'name' => 'BRIG_TYPE_S16X2',
                                             'mnemo' => 's16x2'
                                           },
                                           {
                                             'length' => 64,
                                             'val' => 'BRIG_TYPE_S16 | BRIG_TYPE_PACK_64',
                                             'mnemo' => 's16x4',
                                             'name' => 'BRIG_TYPE_S16X4',
                                             'ctype' => 'int16_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S16X4> >()'
                                           },
                                           {
                                             'mnemo' => 's16x8',
                                             'name' => 'BRIG_TYPE_S16X8',
                                             'ctype' => 'int16_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S16X8> >()',
                                             'length' => 128,
                                             'val' => 'BRIG_TYPE_S16 | BRIG_TYPE_PACK_128'
                                           },
                                           {
                                             'val' => 'BRIG_TYPE_S32 | BRIG_TYPE_PACK_64',
                                             'length' => 64,
                                             'name' => 'BRIG_TYPE_S32X2',
                                             'mnemo' => 's32x2',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S32X2> >()',
                                             'ctype' => 'int32_t'
                                           },
                                           {
                                             'length' => 128,
                                             'val' => 'BRIG_TYPE_S32 | BRIG_TYPE_PACK_128',
                                             'mnemo' => 's32x4',
                                             'name' => 'BRIG_TYPE_S32X4',
                                             'ctype' => 'int32_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S32X4> >()'
                                           },
                                           {
                                             'length' => 128,
                                             'val' => 'BRIG_TYPE_S64 | BRIG_TYPE_PACK_128',
                                             'mnemo' => 's64x2',
                                             'name' => 'BRIG_TYPE_S64X2',
                                             'ctype' => 'int64_t',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_S64X2> >()'
                                           },
                                           {
                                             'val' => 'BRIG_TYPE_F16 | BRIG_TYPE_PACK_32',
                                             'length' => 32,
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F16X2> >()',
                                             'ctype' => 'f16_t',
                                             'name' => 'BRIG_TYPE_F16X2',
                                             'mnemo' => 'f16x2'
                                           },
                                           {
                                             'val' => 'BRIG_TYPE_F16 | BRIG_TYPE_PACK_64',
                                             'length' => 64,
                                             'name' => 'BRIG_TYPE_F16X4',
                                             'mnemo' => 'f16x4',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F16X4> >()',
                                             'ctype' => 'f16_t'
                                           },
                                           {
                                             'val' => 'BRIG_TYPE_F16 | BRIG_TYPE_PACK_128',
                                             'length' => 128,
                                             'name' => 'BRIG_TYPE_F16X8',
                                             'mnemo' => 'f16x8',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F16X8> >()',
                                             'ctype' => 'f16_t'
                                           },
                                           {
                                             'ctype' => 'float',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F32X2> >()',
                                             'mnemo' => 'f32x2',
                                             'name' => 'BRIG_TYPE_F32X2',
                                             'length' => 64,
                                             'val' => 'BRIG_TYPE_F32 | BRIG_TYPE_PACK_64'
                                           },
                                           {
                                             'val' => 'BRIG_TYPE_F32 | BRIG_TYPE_PACK_128',
                                             'length' => 128,
                                             'name' => 'BRIG_TYPE_F32X4',
                                             'mnemo' => 'f32x4',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F32X4> >()',
                                             'ctype' => 'float'
                                           },
                                           {
                                             'length' => 128,
                                             'val' => 'BRIG_TYPE_F64 | BRIG_TYPE_PACK_128',
                                             'ctype' => 'double',
                                             'dispatch' => 'v.template visit< BrigType<BRIG_TYPE_F64X2> >()',
                                             'mnemo' => 'f64x2',
                                             'name' => 'BRIG_TYPE_F64X2'
                                           },
                                           {
                                             'val' => '-1 ',
                                             'length' => 0,
                                             'skip' => 'true',
                                             'name' => 'BRIG_TYPE_INVALID',
                                             'mnemo' => 'invalid',
                                             'dispatch' => 'v.visitNone(type)'
                                           }
                                         ],
                            'dispatch#calcState' => 'done',
                            'mnemo#deps' => [],
                            'length' => sub { "DUMMY" }
                          },
           'BrigVersion' => {
                              'name' => 'BrigVersion',
                              'entries' => [
                                             {
                                               'name' => 'BRIG_VERSION_HSAIL_MAJOR',
                                               'val' => '0'
                                             },
                                             {
                                               'name' => 'BRIG_VERSION_HSAIL_MINOR',
                                               'val' => '20140227'
                                             },
                                             {
                                               'name' => 'BRIG_VERSION_BRIG_MAJOR',
                                               'val' => '0'
                                             },
                                             {
                                               'name' => 'BRIG_VERSION_BRIG_MINOR',
                                               'val' => '20140227'
                                             }
                                           ],
                              'nowrap' => 'true',
                              'nodump' => 'true'
                            },
           'BrigWidth' => {
                            'name' => 'BrigWidth',
                            'entries' => [
                                           {
                                             'name' => 'BRIG_WIDTH_NONE',
                                             'val' => '0'
                                           },
                                           {
                                             'val' => '1',
                                             'name' => 'BRIG_WIDTH_1'
                                           },
                                           {
                                             'val' => '2',
                                             'name' => 'BRIG_WIDTH_2'
                                           },
                                           {
                                             'val' => '3',
                                             'name' => 'BRIG_WIDTH_4'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_8',
                                             'val' => '4'
                                           },
                                           {
                                             'val' => '5',
                                             'name' => 'BRIG_WIDTH_16'
                                           },
                                           {
                                             'val' => '6',
                                             'name' => 'BRIG_WIDTH_32'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_64',
                                             'val' => '7'
                                           },
                                           {
                                             'val' => '8',
                                             'name' => 'BRIG_WIDTH_128'
                                           },
                                           {
                                             'val' => '9',
                                             'name' => 'BRIG_WIDTH_256'
                                           },
                                           {
                                             'val' => '10',
                                             'name' => 'BRIG_WIDTH_512'
                                           },
                                           {
                                             'val' => '11',
                                             'name' => 'BRIG_WIDTH_1024'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_2048',
                                             'val' => '12'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_4096',
                                             'val' => '13'
                                           },
                                           {
                                             'val' => '14',
                                             'name' => 'BRIG_WIDTH_8192'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_16384',
                                             'val' => '15'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_32768',
                                             'val' => '16'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_65536',
                                             'val' => '17'
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
                                             'name' => 'BRIG_WIDTH_524288',
                                             'val' => '20'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_1048576',
                                             'val' => '21'
                                           },
                                           {
                                             'val' => '22',
                                             'name' => 'BRIG_WIDTH_2097152'
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
                                             'name' => 'BRIG_WIDTH_16777216',
                                             'val' => '25'
                                           },
                                           {
                                             'val' => '26',
                                             'name' => 'BRIG_WIDTH_33554432'
                                           },
                                           {
                                             'val' => '27',
                                             'name' => 'BRIG_WIDTH_67108864'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_134217728',
                                             'val' => '28'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_268435456',
                                             'val' => '29'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_536870912',
                                             'val' => '30'
                                           },
                                           {
                                             'name' => 'BRIG_WIDTH_1073741824',
                                             'val' => '31'
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
           'BrigSamplerCoordNormalization' => {
                                                'mnemo#deps' => [],
                                                'mnemo_token' => 'ESamplerCoord',
                                                'mnemo#calcState' => 'done',
                                                'mnemo' => sub { "DUMMY" },
                                                'name' => 'BrigSamplerCoordNormalization',
                                                'entries' => [
                                                               {
                                                                 'val' => '0',
                                                                 'mnemo' => 'unnormalized',
                                                                 'name' => 'BRIG_COORD_UNNORMALIZED'
                                                               },
                                                               {
                                                                 'name' => 'BRIG_COORD_NORMALIZED',
                                                                 'mnemo' => 'normalized',
                                                                 'val' => '1'
                                                               }
                                                             ]
                                              },
           'BrigImageChannelOrder' => {
                                        'mnemo_token' => 'EImageOrder',
                                        'mnemo#deps' => [],
                                        'mnemo_context' => 'EImageOrderContext',
                                        'entries' => [
                                                       {
                                                         'val' => '0',
                                                         'mnemo' => 'a',
                                                         'name' => 'BRIG_CHANNEL_ORDER_A'
                                                       },
                                                       {
                                                         'val' => '1',
                                                         'name' => 'BRIG_CHANNEL_ORDER_R',
                                                         'mnemo' => 'r'
                                                       },
                                                       {
                                                         'mnemo' => 'rx',
                                                         'name' => 'BRIG_CHANNEL_ORDER_RX',
                                                         'val' => '2'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_RG',
                                                         'mnemo' => 'rg',
                                                         'val' => '3'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_RGX',
                                                         'mnemo' => 'rgx',
                                                         'val' => '4'
                                                       },
                                                       {
                                                         'mnemo' => 'ra',
                                                         'name' => 'BRIG_CHANNEL_ORDER_RA',
                                                         'val' => '5'
                                                       },
                                                       {
                                                         'val' => '6',
                                                         'mnemo' => 'rgb',
                                                         'name' => 'BRIG_CHANNEL_ORDER_RGB'
                                                       },
                                                       {
                                                         'val' => '7',
                                                         'mnemo' => 'rgbx',
                                                         'name' => 'BRIG_CHANNEL_ORDER_RGBX'
                                                       },
                                                       {
                                                         'val' => '8',
                                                         'mnemo' => 'rgba',
                                                         'name' => 'BRIG_CHANNEL_ORDER_RGBA'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_BGRA',
                                                         'mnemo' => 'bgra',
                                                         'val' => '9'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_ARGB',
                                                         'mnemo' => 'argb',
                                                         'val' => '10'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_ABGR',
                                                         'mnemo' => 'abgr',
                                                         'val' => '11'
                                                       },
                                                       {
                                                         'val' => '12',
                                                         'mnemo' => 'srgb',
                                                         'name' => 'BRIG_CHANNEL_ORDER_SRGB'
                                                       },
                                                       {
                                                         'val' => '13',
                                                         'mnemo' => 'srgbx',
                                                         'name' => 'BRIG_CHANNEL_ORDER_SRGBX'
                                                       },
                                                       {
                                                         'val' => '14',
                                                         'name' => 'BRIG_CHANNEL_ORDER_SRGBA',
                                                         'mnemo' => 'srgba'
                                                       },
                                                       {
                                                         'mnemo' => 'sbgra',
                                                         'name' => 'BRIG_CHANNEL_ORDER_SBGRA',
                                                         'val' => '15'
                                                       },
                                                       {
                                                         'mnemo' => 'intensity',
                                                         'name' => 'BRIG_CHANNEL_ORDER_INTENSITY',
                                                         'val' => '16'
                                                       },
                                                       {
                                                         'val' => '17',
                                                         'name' => 'BRIG_CHANNEL_ORDER_LUMINANCE',
                                                         'mnemo' => 'luminance'
                                                       },
                                                       {
                                                         'val' => '18',
                                                         'name' => 'BRIG_CHANNEL_ORDER_DEPTH',
                                                         'mnemo' => 'depth'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_DEPTH_STENCIL',
                                                         'mnemo' => 'depth_stencil',
                                                         'val' => '19'
                                                       },
                                                       {
                                                         'name' => 'BRIG_CHANNEL_ORDER_UNKNOWN',
                                                         'mnemo' => ''
                                                       }
                                                     ],
                                        'name' => 'BrigImageChannelOrder',
                                        'mnemo' => sub { "DUMMY" },
                                        'mnemo#calcState' => 'done'
                                      },
           'BrigPack' => {
                           'name' => 'BrigPack',
                           'mnemo#calcState' => 'done',
                           'mnemo' => sub { "DUMMY" },
                           'entries' => [
                                          {
                                            'val' => '0',
                                            'mnemo' => '',
                                            'name' => 'BRIG_PACK_NONE'
                                          },
                                          {
                                            'mnemo' => 'pp',
                                            'name' => 'BRIG_PACK_PP',
                                            'val' => '1'
                                          },
                                          {
                                            'val' => '2',
                                            'mnemo' => 'ps',
                                            'name' => 'BRIG_PACK_PS'
                                          },
                                          {
                                            'name' => 'BRIG_PACK_SP',
                                            'mnemo' => 'sp',
                                            'val' => '3'
                                          },
                                          {
                                            'val' => '4',
                                            'mnemo' => 'ss',
                                            'name' => 'BRIG_PACK_SS'
                                          },
                                          {
                                            'mnemo' => 's',
                                            'name' => 'BRIG_PACK_S',
                                            'val' => '5'
                                          },
                                          {
                                            'mnemo' => 'p',
                                            'name' => 'BRIG_PACK_P',
                                            'val' => '6'
                                          },
                                          {
                                            'name' => 'BRIG_PACK_PPSAT',
                                            'mnemo' => 'pp_sat',
                                            'val' => '7'
                                          },
                                          {
                                            'mnemo' => 'ps_sat',
                                            'name' => 'BRIG_PACK_PSSAT',
                                            'val' => '8'
                                          },
                                          {
                                            'val' => '9',
                                            'mnemo' => 'sp_sat',
                                            'name' => 'BRIG_PACK_SPSAT'
                                          },
                                          {
                                            'val' => '10',
                                            'name' => 'BRIG_PACK_SSSAT',
                                            'mnemo' => 'ss_sat'
                                          },
                                          {
                                            'name' => 'BRIG_PACK_SSAT',
                                            'mnemo' => 's_sat',
                                            'val' => '11'
                                          },
                                          {
                                            'name' => 'BRIG_PACK_PSAT',
                                            'mnemo' => 'p_sat',
                                            'val' => '12'
                                          }
                                        ],
                           'tdcaption' => 'Packing',
                           'mnemo#deps' => [],
                           'mnemo_token' => '_EMPacking'
                         },
           'BrigCompareOperation' => {
                                       'mnemo#calcState' => 'done',
                                       'mnemo' => sub { "DUMMY" },
                                       'name' => 'BrigCompareOperation',
                                       'tdcaption' => 'Comparison Operators',
                                       'entries' => [
                                                      {
                                                        'val' => '0',
                                                        'mnemo' => 'eq',
                                                        'name' => 'BRIG_COMPARE_EQ'
                                                      },
                                                      {
                                                        'val' => '1',
                                                        'name' => 'BRIG_COMPARE_NE',
                                                        'mnemo' => 'ne'
                                                      },
                                                      {
                                                        'mnemo' => 'lt',
                                                        'name' => 'BRIG_COMPARE_LT',
                                                        'val' => '2'
                                                      },
                                                      {
                                                        'val' => '3',
                                                        'name' => 'BRIG_COMPARE_LE',
                                                        'mnemo' => 'le'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_GT',
                                                        'mnemo' => 'gt',
                                                        'val' => '4'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_GE',
                                                        'mnemo' => 'ge',
                                                        'val' => '5'
                                                      },
                                                      {
                                                        'val' => '6',
                                                        'mnemo' => 'equ',
                                                        'name' => 'BRIG_COMPARE_EQU'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_NEU',
                                                        'mnemo' => 'neu',
                                                        'val' => '7'
                                                      },
                                                      {
                                                        'mnemo' => 'ltu',
                                                        'name' => 'BRIG_COMPARE_LTU',
                                                        'val' => '8'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_LEU',
                                                        'mnemo' => 'leu',
                                                        'val' => '9'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_GTU',
                                                        'mnemo' => 'gtu',
                                                        'val' => '10'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_GEU',
                                                        'mnemo' => 'geu',
                                                        'val' => '11'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_NUM',
                                                        'mnemo' => 'num',
                                                        'val' => '12'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_NAN',
                                                        'mnemo' => 'nan',
                                                        'val' => '13'
                                                      },
                                                      {
                                                        'mnemo' => 'seq',
                                                        'name' => 'BRIG_COMPARE_SEQ',
                                                        'val' => '14'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_SNE',
                                                        'mnemo' => 'sne',
                                                        'val' => '15'
                                                      },
                                                      {
                                                        'mnemo' => 'slt',
                                                        'name' => 'BRIG_COMPARE_SLT',
                                                        'val' => '16'
                                                      },
                                                      {
                                                        'val' => '17',
                                                        'mnemo' => 'sle',
                                                        'name' => 'BRIG_COMPARE_SLE'
                                                      },
                                                      {
                                                        'val' => '18',
                                                        'mnemo' => 'sgt',
                                                        'name' => 'BRIG_COMPARE_SGT'
                                                      },
                                                      {
                                                        'val' => '19',
                                                        'mnemo' => 'sge',
                                                        'name' => 'BRIG_COMPARE_SGE'
                                                      },
                                                      {
                                                        'mnemo' => 'sgeu',
                                                        'name' => 'BRIG_COMPARE_SGEU',
                                                        'val' => '20'
                                                      },
                                                      {
                                                        'val' => '21',
                                                        'mnemo' => 'sequ',
                                                        'name' => 'BRIG_COMPARE_SEQU'
                                                      },
                                                      {
                                                        'val' => '22',
                                                        'name' => 'BRIG_COMPARE_SNEU',
                                                        'mnemo' => 'sneu'
                                                      },
                                                      {
                                                        'val' => '23',
                                                        'name' => 'BRIG_COMPARE_SLTU',
                                                        'mnemo' => 'sltu'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_SLEU',
                                                        'mnemo' => 'sleu',
                                                        'val' => '24'
                                                      },
                                                      {
                                                        'mnemo' => 'snum',
                                                        'name' => 'BRIG_COMPARE_SNUM',
                                                        'val' => '25'
                                                      },
                                                      {
                                                        'val' => '26',
                                                        'name' => 'BRIG_COMPARE_SNAN',
                                                        'mnemo' => 'snan'
                                                      },
                                                      {
                                                        'name' => 'BRIG_COMPARE_SGTU',
                                                        'mnemo' => 'sgtu',
                                                        'val' => '27'
                                                      }
                                                    ],
                                       'mnemo#deps' => [],
                                       'mnemo_token' => '_EMCompare'
                                     },
           'BrigImageQuery' => {
                                 'mnemo#deps' => [],
                                 'mnemo#calcState' => 'done',
                                 'mnemo' => sub { "DUMMY" },
                                 'name' => 'BrigImageQuery',
                                 'entries' => [
                                                {
                                                  'val' => '0',
                                                  'mnemo' => 'width',
                                                  'name' => 'BRIG_IMAGE_QUERY_WIDTH'
                                                },
                                                {
                                                  'mnemo' => 'height',
                                                  'name' => 'BRIG_IMAGE_QUERY_HEIGHT',
                                                  'val' => '1'
                                                },
                                                {
                                                  'mnemo' => 'depth',
                                                  'name' => 'BRIG_IMAGE_QUERY_DEPTH',
                                                  'val' => '2'
                                                },
                                                {
                                                  'val' => '3',
                                                  'name' => 'BRIG_IMAGE_QUERY_ARRAY',
                                                  'mnemo' => 'array'
                                                },
                                                {
                                                  'val' => '4',
                                                  'mnemo' => 'channelorder',
                                                  'name' => 'BRIG_IMAGE_QUERY_CHANNELORDER'
                                                },
                                                {
                                                  'val' => '5',
                                                  'name' => 'BRIG_IMAGE_QUERY_CHANNELTYPE',
                                                  'mnemo' => 'channeltype'
                                                }
                                              ]
                               },
           'BrigOpcode' => {
                             'opcodevis#deps' => [
                                                   'pscode'
                                                 ],
                             'numdst#calcState' => 'done',
                             'hasType_proto' => 'bool instHasType(Brig::BrigOpcode16_t arg)',
                             'ftz_incfile' => 'ItemUtils',
                             'numdst#deps' => [],
                             'name' => 'BrigOpcode',
                             'ftz#deps' => [
                                             'k'
                                           ],
                             'opcodevis#calcState' => 'done',
                             'has_memory_order#deps' => [],
                             'numdst_switch' => 'true',
                             'opndparser_switch' => 'true',
                             'pscode#deps' => [
                                                'k'
                                              ],
                             'k#calcState' => 'done',
                             'entries' => [
                                            {
                                              'name' => 'BRIG_OPCODE_NOP',
                                              'opndparser' => '&Parser::parseNoOperands',
                                              'k' => 'NOP',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_NOP>                   (inst)',
                                              'hasType' => 'false',
                                              'mnemo' => 'nop',
                                              'val' => '0',
                                              'pscode' => 'Nop',
                                              'opcodeparser' => 'parseMnemoNop',
                                              'numdst' => '0',
                                              'psopnd' => 'NoOperands'
                                            },
                                            {
                                              'mnemo' => 'codeblockend',
                                              'val' => '1',
                                              'pscode' => 'BasicNoType',
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'psopnd' => 'Operands',
                                              'name' => 'BRIG_OPCODE_CODEBLOCKEND',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC_NO_TYPE',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_CODEBLOCKEND>          (HSAIL_ASM::InstBasic(inst))',
                                              'hasType' => 'false'
                                            },
                                            {
                                              'mnemo' => 'abs',
                                              'psopnd' => 'Operands',
                                              'pscode' => 'BasicOrMod',
                                              'val' => '2',
                                              'ftz' => 'true',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'k' => 'BASIC_OR_MOD',
                                              'name' => 'BRIG_OPCODE_ABS',
                                              'opndparser' => '&Parser::parseOperands',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_ABS>                   (inst)'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_ADD>                   (inst)',
                                              'k' => 'BASIC_OR_MOD',
                                              'name' => 'BRIG_OPCODE_ADD',
                                              'opndparser' => '&Parser::parseOperands',
                                              'psopnd' => 'Operands',
                                              'pscode' => 'BasicOrMod',
                                              'val' => '3',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'ftz' => 'true',
                                              'mnemo' => 'add'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '4',
                                              'mnemo' => 'borrow',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_BORROW>                (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_BORROW'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'val' => '5',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'carry',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_CARRY>                 (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_CARRY',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'mnemo' => 'ceil',
                                              'psopnd' => 'Operands',
                                              'val' => '6',
                                              'pscode' => 'BasicOrMod',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'ftz' => 'true',
                                              'k' => 'BASIC_OR_MOD',
                                              'name' => 'BRIG_OPCODE_CEIL',
                                              'opndparser' => '&Parser::parseOperands',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_CEIL>                  (inst)'
                                            },
                                            {
                                              'mnemo' => 'copysign',
                                              'psopnd' => 'Operands',
                                              'pscode' => 'BasicOrMod',
                                              'val' => '7',
                                              'ftz' => 'true',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'k' => 'BASIC_OR_MOD',
                                              'name' => 'BRIG_OPCODE_COPYSIGN',
                                              'opndparser' => '&Parser::parseOperands',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_COPYSIGN>              (inst)'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'val' => '8',
                                              'pscode' => 'BasicOrMod',
                                              'ftz' => 'true',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'mnemo' => 'div',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_DIV>                   (inst)',
                                              'k' => 'BASIC_OR_MOD',
                                              'name' => 'BRIG_OPCODE_DIV',
                                              'opndparser' => '&Parser::parseOperands'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'pscode' => 'BasicOrMod',
                                              'val' => '9',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'ftz' => 'true',
                                              'mnemo' => 'floor',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_FLOOR>                 (inst)',
                                              'k' => 'BASIC_OR_MOD',
                                              'name' => 'BRIG_OPCODE_FLOOR',
                                              'opndparser' => '&Parser::parseOperands'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_FMA>                   (inst)',
                                              'k' => 'BASIC_OR_MOD',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_FMA',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'ftz' => 'true',
                                              'val' => '10',
                                              'pscode' => 'BasicOrMod',
                                              'mnemo' => 'fma'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'ftz' => 'true',
                                              'val' => '11',
                                              'pscode' => 'BasicOrMod',
                                              'mnemo' => 'fract',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_FRACT>                 (inst)',
                                              'k' => 'BASIC_OR_MOD',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_FRACT'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_MAD>                   (HSAIL_ASM::InstBasic(inst))',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_MAD',
                                              'k' => 'BASIC',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '12',
                                              'pscode' => 'Basic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'mad'
                                            },
                                            {
                                              'val' => '13',
                                              'pscode' => 'BasicOrMod',
                                              'ftz' => 'true',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'max',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_MAX>                   (inst)',
                                              'name' => 'BRIG_OPCODE_MAX',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC_OR_MOD'
                                            },
                                            {
                                              'k' => 'BASIC_OR_MOD',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_MIN',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_MIN>                   (inst)',
                                              'mnemo' => 'min',
                                              'psopnd' => 'Operands',
                                              'ftz' => 'true',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'pscode' => 'BasicOrMod',
                                              'val' => '14'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'ftz' => 'true',
                                              'val' => '15',
                                              'pscode' => 'BasicOrMod',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'mul',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_MUL>                   (inst)',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_MUL',
                                              'k' => 'BASIC_OR_MOD'
                                            },
                                            {
                                              'mnemo' => 'mulhi',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'ftz' => 'true',
                                              'pscode' => 'BasicOrMod',
                                              'val' => '16',
                                              'psopnd' => 'Operands',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_MULHI',
                                              'k' => 'BASIC_OR_MOD',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_MULHI>                 (inst)'
                                            },
                                            {
                                              'pscode' => 'BasicOrMod',
                                              'val' => '17',
                                              'ftz' => 'true',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'neg',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_NEG>                   (inst)',
                                              'name' => 'BRIG_OPCODE_NEG',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC_OR_MOD'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_REM',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_REM>                   (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'rem',
                                              'val' => '18',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'ftz' => 'true',
                                              'pscode' => 'BasicOrMod',
                                              'val' => '19',
                                              'mnemo' => 'rint',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_RINT>                  (inst)',
                                              'k' => 'BASIC_OR_MOD',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_RINT'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_SQRT>                  (inst)',
                                              'k' => 'BASIC_OR_MOD',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_SQRT',
                                              'psopnd' => 'Operands',
                                              'ftz' => 'true',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'val' => '20',
                                              'pscode' => 'BasicOrMod',
                                              'mnemo' => 'sqrt'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_SUB>                   (inst)',
                                              'name' => 'BRIG_OPCODE_SUB',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC_OR_MOD',
                                              'val' => '21',
                                              'pscode' => 'BasicOrMod',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'ftz' => 'true',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'sub'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasicOrMod',
                                              'ftz' => 'true',
                                              'val' => '22',
                                              'pscode' => 'BasicOrMod',
                                              'mnemo' => 'trunc',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_TRUNC>                 (inst)',
                                              'k' => 'BASIC_OR_MOD',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_TRUNC'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_MAD24',
                                              'opndparser' => '&Parser::parseOperands',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_MAD24>                 (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'mad24',
                                              'psopnd' => 'Operands',
                                              'val' => '23',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_MAD24HI>               (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_MAD24HI',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '24',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'mad24hi'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '25',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'mul24',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_MUL24>                 (HSAIL_ASM::InstBasic(inst))',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_MUL24',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'val' => '26',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'mnemo' => 'mul24hi',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_MUL24HI>               (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_MUL24HI',
                                              'opndparser' => '&Parser::parseOperands'
                                            },
                                            {
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_SHL',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_SHL>                   (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'shl',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '27',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'mnemo' => 'shr',
                                              'val' => '28',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'name' => 'BRIG_OPCODE_SHR',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_SHR>                   (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'mnemo' => 'and',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '29',
                                              'pscode' => 'Basic',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_AND',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_AND>                   (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_NOT',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_NOT>                   (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'not',
                                              'pscode' => 'Basic',
                                              'val' => '30',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_OR>                    (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_OR',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'pscode' => 'Basic',
                                              'val' => '31',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'or'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_POPCOUNT>              (HSAIL_ASM::InstSourceType(inst))',
                                              'k' => 'SOURCE_TYPE',
                                              'name' => 'BRIG_OPCODE_POPCOUNT',
                                              'opndparser' => '&Parser::parseOperands',
                                              'psopnd' => 'Operands',
                                              'pscode' => 'SourceType',
                                              'val' => '32',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'mnemo' => 'popcount'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_XOR>                   (HSAIL_ASM::InstBasic(inst))',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_XOR',
                                              'k' => 'BASIC',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '33',
                                              'pscode' => 'Basic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'xor'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_BITEXTRACT',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_BITEXTRACT>            (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'bitextract',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '34',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'val' => '35',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'mnemo' => 'bitinsert',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_BITINSERT>             (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_BITINSERT',
                                              'opndparser' => '&Parser::parseOperands'
                                            },
                                            {
                                              'val' => '36',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'bitmask',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_BITMASK>               (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_BITMASK',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_BITREV',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_BITREV>                (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'bitrev',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '37',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_BITSELECT>             (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_BITSELECT',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'pscode' => 'Basic',
                                              'val' => '38',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'bitselect'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_FIRSTBIT>              (HSAIL_ASM::InstSourceType(inst))',
                                              'k' => 'SOURCE_TYPE',
                                              'name' => 'BRIG_OPCODE_FIRSTBIT',
                                              'opndparser' => '&Parser::parseOperands',
                                              'psopnd' => 'Operands',
                                              'pscode' => 'SourceType',
                                              'val' => '39',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'mnemo' => 'firstbit'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_LASTBIT>               (HSAIL_ASM::InstSourceType(inst))',
                                              'name' => 'BRIG_OPCODE_LASTBIT',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'SOURCE_TYPE',
                                              'pscode' => 'SourceType',
                                              'val' => '40',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'lastbit'
                                            },
                                            {
                                              'val' => '41',
                                              'pscode' => 'SourceType',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'mnemo_token' => 'EInstruction_Vx',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'combine',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_COMBINE>               (HSAIL_ASM::InstSourceType(inst))',
                                              'name' => 'BRIG_OPCODE_COMBINE',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'SOURCE_TYPE'
                                            },
                                            {
                                              'k' => 'SOURCE_TYPE',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_EXPAND',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_EXPAND>                (HSAIL_ASM::InstSourceType(inst))',
                                              'mnemo' => 'expand',
                                              'psopnd' => 'Operands',
                                              'mnemo_token' => 'EInstruction_Vx',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'pscode' => 'SourceType',
                                              'val' => '42'
                                            },
                                            {
                                              'val' => '43',
                                              'pscode' => 'Addr',
                                              'opcodeparser' => 'parseMnemoAddr',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'lda',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_LDA>                   (HSAIL_ASM::InstAddr(inst))',
                                              'name' => 'BRIG_OPCODE_LDA',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'ADDR'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_LDC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_LDC>                   (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'ldc',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '44'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_MOV',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_MOV>                   (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'mov',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '45'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_SHUFFLE',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_SHUFFLE>               (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'shuffle',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '46'
                                            },
                                            {
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_UNPACKHI',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_UNPACKHI>              (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'unpackhi',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '47',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'mnemo' => 'unpacklo',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '48',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_UNPACKLO',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_UNPACKLO>              (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'mnemo' => 'pack',
                                              'psopnd' => 'Operands',
                                              'pscode' => 'SourceType',
                                              'val' => '49',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'k' => 'SOURCE_TYPE',
                                              'name' => 'BRIG_OPCODE_PACK',
                                              'opndparser' => '&Parser::parseOperands',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_PACK>                  (HSAIL_ASM::InstSourceType(inst))'
                                            },
                                            {
                                              'pscode' => 'SourceType',
                                              'val' => '50',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'unpack',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_UNPACK>                (HSAIL_ASM::InstSourceType(inst))',
                                              'name' => 'BRIG_OPCODE_UNPACK',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'SOURCE_TYPE'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '51',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'cmov',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_CMOV>                  (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_CMOV'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_CLASS>                 (HSAIL_ASM::InstSourceType(inst))',
                                              'k' => 'SOURCE_TYPE',
                                              'name' => 'BRIG_OPCODE_CLASS',
                                              'opndparser' => '&Parser::parseOperands',
                                              'psopnd' => 'Operands',
                                              'val' => '52',
                                              'pscode' => 'SourceType',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'mnemo' => 'class'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_NCOS>                  (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_NCOS',
                                              'opndparser' => '&Parser::parseOperands',
                                              'psopnd' => 'Operands',
                                              'pscode' => 'Basic',
                                              'val' => '53',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'mnemo' => 'ncos'
                                            },
                                            {
                                              'mnemo' => 'nexp2',
                                              'pscode' => 'Basic',
                                              'val' => '54',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'name' => 'BRIG_OPCODE_NEXP2',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_NEXP2>                 (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '55',
                                              'mnemo' => 'nfma',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_NFMA>                  (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_NFMA'
                                            },
                                            {
                                              'mnemo' => 'nlog2',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '56',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_NLOG2',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_NLOG2>                 (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'pscode' => 'Basic',
                                              'val' => '57',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'mnemo' => 'nrcp',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_NRCP>                  (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_NRCP',
                                              'opndparser' => '&Parser::parseOperands'
                                            },
                                            {
                                              'mnemo' => 'nrsqrt',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '58',
                                              'pscode' => 'Basic',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_NRSQRT',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_NRSQRT>                (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'pscode' => 'Basic',
                                              'val' => '59',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'mnemo' => 'nsin',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_NSIN>                  (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_NSIN',
                                              'opndparser' => '&Parser::parseOperands'
                                            },
                                            {
                                              'val' => '60',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'nsqrt',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_NSQRT>                 (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_NSQRT',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_BITALIGN',
                                              'opndparser' => '&Parser::parseOperands',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_BITALIGN>              (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'bitalign',
                                              'psopnd' => 'Operands',
                                              'val' => '61',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_BYTEALIGN',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_BYTEALIGN>             (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'bytealign',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '62',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'k' => 'SOURCE_TYPE',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_PACKCVT',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_PACKCVT>               (HSAIL_ASM::InstSourceType(inst))',
                                              'mnemo' => 'packcvt',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'val' => '63',
                                              'pscode' => 'SourceType'
                                            },
                                            {
                                              'mnemo' => 'unpackcvt',
                                              'pscode' => 'SourceType',
                                              'val' => '64',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'psopnd' => 'Operands',
                                              'name' => 'BRIG_OPCODE_UNPACKCVT',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'SOURCE_TYPE',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_UNPACKCVT>             (HSAIL_ASM::InstSourceType(inst))'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '65',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'lerp',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_LERP>                  (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_LERP'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_SAD>                   (HSAIL_ASM::InstSourceType(inst))',
                                              'k' => 'SOURCE_TYPE',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_SAD',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'pscode' => 'SourceType',
                                              'val' => '66',
                                              'mnemo' => 'sad'
                                            },
                                            {
                                              'mnemo' => 'sadhi',
                                              'opcodeparser' => 'parseMnemoSourceType',
                                              'pscode' => 'SourceType',
                                              'val' => '67',
                                              'psopnd' => 'Operands',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_SADHI',
                                              'k' => 'SOURCE_TYPE',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_SADHI>                 (HSAIL_ASM::InstSourceType(inst))'
                                            },
                                            {
                                              'mnemo' => 'segmentp',
                                              'opcodeparser' => 'parseMnemoSegCvt',
                                              'pscode' => 'SegCvt',
                                              'val' => '68',
                                              'psopnd' => 'Operands',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_SEGMENTP',
                                              'k' => 'SEG_CVT',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_SEGMENTP>              (HSAIL_ASM::InstSegCvt(inst))'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'pscode' => 'SegCvt',
                                              'val' => '69',
                                              'opcodeparser' => 'parseMnemoSegCvt',
                                              'mnemo' => 'ftos',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_FTOS>                  (HSAIL_ASM::InstSegCvt(inst))',
                                              'k' => 'SEG_CVT',
                                              'name' => 'BRIG_OPCODE_FTOS',
                                              'opndparser' => '&Parser::parseOperands'
                                            },
                                            {
                                              'pscode' => 'SegCvt',
                                              'val' => '70',
                                              'opcodeparser' => 'parseMnemoSegCvt',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'stof',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_STOF>                  (HSAIL_ASM::InstSegCvt(inst))',
                                              'name' => 'BRIG_OPCODE_STOF',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'SEG_CVT'
                                            },
                                            {
                                              'pscode' => 'Cmp',
                                              'val' => '71',
                                              'ftz' => 'true',
                                              'opcodeparser' => 'parseMnemoCmp',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'cmp',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_CMP>                   (HSAIL_ASM::InstCmp(inst))',
                                              'name' => 'BRIG_OPCODE_CMP',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'CMP'
                                            },
                                            {
                                              'mnemo' => 'cvt',
                                              'opcodeparser' => 'parseMnemoCvt',
                                              'ftz' => 'true',
                                              'val' => '72',
                                              'pscode' => 'Cvt',
                                              'psopnd' => 'Operands',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_CVT',
                                              'k' => 'CVT',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_CVT>                   (HSAIL_ASM::InstCvt(inst))'
                                            },
                                            {
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_LD',
                                              'k' => 'MEM',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_LD>                    (HSAIL_ASM::InstMem(inst))',
                                              'semsupport' => 'true',
                                              'has_memory_order' => 'true',
                                              'mnemo' => 'ld',
                                              'opcodeparser' => 'parseMnemoMem',
                                              'pscode' => 'Mem',
                                              'val' => '73',
                                              'mnemo_token' => 'EInstruction_Vx',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'k' => 'MEM',
                                              'name' => 'BRIG_OPCODE_ST',
                                              'opndparser' => '&Parser::parseOperands',
                                              'has_memory_order' => 'true',
                                              'semsupport' => 'true',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_ST>                    (HSAIL_ASM::InstMem(inst))',
                                              'mnemo' => 'st',
                                              'psopnd' => 'Operands',
                                              'mnemo_token' => 'EInstruction_Vx',
                                              'val' => '74',
                                              'pscode' => 'Mem',
                                              'numdst' => '0',
                                              'opcodeparser' => 'parseMnemoMem'
                                            },
                                            {
                                              'mnemo' => 'atomic',
                                              'opcodeparser' => 'parseMnemoAtomic',
                                              'pscode' => 'Atomic',
                                              'val' => '75',
                                              'psopnd' => 'Operands',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_ATOMIC',
                                              'k' => 'ATOMIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_ATOMIC>                (HSAIL_ASM::InstAtomic(inst))'
                                            },
                                            {
                                              'k' => 'ATOMIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_ATOMICNORET',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_ATOMICNORET>           (HSAIL_ASM::InstAtomic(inst))',
                                              'mnemo' => 'atomicnoret',
                                              'psopnd' => 'Operands',
                                              'numdst' => '0',
                                              'opcodeparser' => 'parseMnemoAtomic',
                                              'val' => '76',
                                              'pscode' => 'Atomic'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_SIGNAL>                (HSAIL_ASM::InstSignal(inst))',
                                              'name' => 'BRIG_OPCODE_SIGNAL',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'SIGNAL',
                                              'val' => '77',
                                              'pscode' => 'Signal',
                                              'opcodeparser' => 'parseMnemoSignal',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'signal'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_SIGNALNORET>           (HSAIL_ASM::InstSignal(inst))',
                                              'name' => 'BRIG_OPCODE_SIGNALNORET',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'SIGNAL',
                                              'pscode' => 'Signal',
                                              'val' => '78',
                                              'opcodeparser' => 'parseMnemoSignal',
                                              'numdst' => '0',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'signalnoret'
                                            },
                                            {
                                              'val' => '79',
                                              'pscode' => 'MemFence',
                                              'opcodeparser' => 'parseMnemoMemFence',
                                              'numdst' => '0',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'memfence',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_MEMFENCE>              (HSAIL_ASM::InstMemFence(inst))',
                                              'name' => 'BRIG_OPCODE_MEMFENCE',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'MEM_FENCE'
                                            },
                                            {
                                              'k' => 'IMAGE',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_RDIMAGE',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_RDIMAGE>               (HSAIL_ASM::InstImage(inst))',
                                              'mnemo' => 'rdimage',
                                              'psopnd' => 'Operands',
                                              'mnemo_token' => 'EInstruction_Vx',
                                              'opcodeparser' => 'parseMnemoImage',
                                              'val' => '80',
                                              'pscode' => 'Image'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoImage',
                                              'pscode' => 'Image',
                                              'val' => '81',
                                              'psopnd' => 'Operands',
                                              'mnemo_token' => 'EInstruction_Vx',
                                              'mnemo' => 'ldimage',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_LDIMAGE>               (HSAIL_ASM::InstImage(inst))',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_LDIMAGE',
                                              'k' => 'IMAGE'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_STIMAGE>               (HSAIL_ASM::InstImage(inst))',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_STIMAGE',
                                              'k' => 'IMAGE',
                                              'numdst' => '0',
                                              'opcodeparser' => 'parseMnemoImage',
                                              'val' => '82',
                                              'pscode' => 'Image',
                                              'mnemo_token' => 'EInstruction_Vx',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'stimage'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'pscode' => 'Br',
                                              'val' => '83',
                                              'numdst' => '0',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'mnemo' => 'cbr',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_CBR>                   (HSAIL_ASM::InstBr(inst))',
                                              'hasType' => 'false',
                                              'k' => 'BR',
                                              'name' => 'BRIG_OPCODE_CBR',
                                              'opndparser' => '&Parser::parseOperands'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'val' => '84',
                                              'pscode' => 'Br',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'numdst' => '0',
                                              'mnemo' => 'brn',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_BRN>                   (HSAIL_ASM::InstBr(inst))',
                                              'hasType' => 'false',
                                              'k' => 'BR',
                                              'name' => 'BRIG_OPCODE_BRN',
                                              'opndparser' => '&Parser::parseOperands'
                                            },
                                            {
                                              'hasType' => 'false',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_BARRIER>               (HSAIL_ASM::InstBr(inst))',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_BARRIER',
                                              'k' => 'BR',
                                              'numdst' => '0',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'val' => '85',
                                              'pscode' => 'Br',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'barrier'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'val' => '86',
                                              'pscode' => 'Br',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'numdst' => '0',
                                              'mnemo' => 'wavebarrier',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_WAVEBARRIER>           (HSAIL_ASM::InstBr(inst))',
                                              'hasType' => 'false',
                                              'k' => 'BR',
                                              'name' => 'BRIG_OPCODE_WAVEBARRIER',
                                              'opndparser' => '&Parser::parseOperands'
                                            },
                                            {
                                              'mnemo' => 'arrivefbar',
                                              'numdst' => '0',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'pscode' => 'Br',
                                              'val' => '87',
                                              'psopnd' => 'Operands',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_ARRIVEFBAR',
                                              'k' => 'BR',
                                              'hasType' => 'false',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_ARRIVEFBAR>            (HSAIL_ASM::InstBr(inst))'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_INITFBAR>              (HSAIL_ASM::InstBasic(inst))',
                                              'hasType' => 'false',
                                              'k' => 'BASIC_NO_TYPE',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_INITFBAR',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'numdst' => '0',
                                              'pscode' => 'BasicNoType',
                                              'val' => '88',
                                              'mnemo' => 'initfbar'
                                            },
                                            {
                                              'mnemo' => 'joinfbar',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'numdst' => '0',
                                              'val' => '89',
                                              'pscode' => 'Br',
                                              'psopnd' => 'Operands',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_JOINFBAR',
                                              'k' => 'BR',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_JOINFBAR>              (HSAIL_ASM::InstBr(inst))',
                                              'hasType' => 'false'
                                            },
                                            {
                                              'mnemo' => 'leavefbar',
                                              'numdst' => '0',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'pscode' => 'Br',
                                              'val' => '90',
                                              'psopnd' => 'Operands',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_LEAVEFBAR',
                                              'k' => 'BR',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_LEAVEFBAR>             (HSAIL_ASM::InstBr(inst))',
                                              'hasType' => 'false'
                                            },
                                            {
                                              'k' => 'BASIC_NO_TYPE',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_RELEASEFBAR',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_RELEASEFBAR>           (HSAIL_ASM::InstBasic(inst))',
                                              'hasType' => 'false',
                                              'mnemo' => 'releasefbar',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'numdst' => '0',
                                              'val' => '91',
                                              'pscode' => 'BasicNoType'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_WAITFBAR',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BR',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_WAITFBAR>              (HSAIL_ASM::InstBr(inst))',
                                              'hasType' => 'false',
                                              'mnemo' => 'waitfbar',
                                              'val' => '92',
                                              'pscode' => 'Br',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'numdst' => '0',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_LDF',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_LDF>                   (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'ldf',
                                              'val' => '93',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_ACTIVELANECOUNT>       (HSAIL_ASM::InstLane(inst))',
                                              'k' => 'LANE',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_ACTIVELANECOUNT',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoLane',
                                              'pscode' => 'Lane',
                                              'val' => '94',
                                              'mnemo' => 'activelanecount'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_ACTIVELANEID>          (HSAIL_ASM::InstLane(inst))',
                                              'name' => 'BRIG_OPCODE_ACTIVELANEID',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'LANE',
                                              'pscode' => 'Lane',
                                              'val' => '95',
                                              'opcodeparser' => 'parseMnemoLane',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'activelaneid'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_ACTIVELANEMASK>        (HSAIL_ASM::InstLane(inst))',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_ACTIVELANEMASK',
                                              'k' => 'LANE',
                                              'opcodeparser' => 'parseMnemoLane',
                                              'pscode' => 'Lane',
                                              'val' => '96',
                                              'mnemo_token' => 'EInstruction_Vx',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'activelanemask'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'pscode' => 'Lane',
                                              'val' => '97',
                                              'opcodeparser' => 'parseMnemoLane',
                                              'mnemo' => 'activelaneshuffle',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_ACTIVELANESHUFFLE>     (HSAIL_ASM::InstLane(inst))',
                                              'k' => 'LANE',
                                              'name' => 'BRIG_OPCODE_ACTIVELANESHUFFLE',
                                              'opndparser' => '&Parser::parseOperands'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_CALL',
                                              'opndparser' => '&Parser::parseCallOperands',
                                              'k' => 'BR',
                                              'hasType' => 'false',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_CALL>                  (HSAIL_ASM::InstBr(inst))',
                                              'mnemo' => 'call',
                                              'val' => '98',
                                              'pscode' => 'Br',
                                              'numdst' => '0',
                                              'opcodeparser' => 'parseMnemoBr',
                                              'psopnd' => 'CallOperands'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_RET>                   (HSAIL_ASM::InstBasic(inst))',
                                              'hasType' => 'false',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_RET',
                                              'k' => 'BASIC_NO_TYPE',
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'val' => '99',
                                              'pscode' => 'BasicNoType',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'ret'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_ALLOCA>                (HSAIL_ASM::InstMem(inst))',
                                              'k' => 'MEM',
                                              'name' => 'BRIG_OPCODE_ALLOCA',
                                              'opndparser' => '&Parser::parseOperands',
                                              'psopnd' => 'Operands',
                                              'val' => '100',
                                              'pscode' => 'Mem',
                                              'opcodeparser' => 'parseMnemoMem',
                                              'mnemo' => 'alloca'
                                            },
                                            {
                                              'mnemo' => 'addqueuewriteindex',
                                              'psopnd' => 'Operands',
                                              'pscode' => 'Queue',
                                              'val' => '101',
                                              'opcodeparser' => 'parseMnemoQueue',
                                              'k' => 'QUEUE',
                                              'name' => 'BRIG_OPCODE_ADDQUEUEWRITEINDEX',
                                              'opndparser' => '&Parser::parseOperands',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_ADDQUEUEWRITEINDEX>    (HSAIL_ASM::InstQueue(inst))'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_CASQUEUEWRITEINDEX',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'QUEUE',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_CASQUEUEWRITEINDEX>    (HSAIL_ASM::InstQueue(inst))',
                                              'mnemo' => 'casqueuewriteindex',
                                              'pscode' => 'Queue',
                                              'val' => '102',
                                              'opcodeparser' => 'parseMnemoQueue',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_CLEARDETECTEXCEPT>     (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_CLEARDETECTEXCEPT',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'numdst' => '0',
                                              'val' => '103',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'cleardetectexcept'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '104',
                                              'pscode' => 'Basic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'clock',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_CLOCK>                 (HSAIL_ASM::InstBasic(inst))',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_CLOCK',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'mnemo' => 'cuid',
                                              'val' => '105',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'name' => 'BRIG_OPCODE_CUID',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_CUID>                  (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_CURRENTWORKGROUPSIZE',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_CURRENTWORKGROUPSIZE>  (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'currentworkgroupsize',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '106',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_DEBUGTRAP',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_DEBUGTRAP>             (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'debugtrap',
                                              'pscode' => 'Basic',
                                              'val' => '107',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'numdst' => '0',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_DIM',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_DIM>                   (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'dim',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '108'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GETDETECTEXCEPT>       (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GETDETECTEXCEPT',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '109',
                                              'mnemo' => 'getdetectexcept'
                                            },
                                            {
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GRIDGROUPS',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GRIDGROUPS>            (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'gridgroups',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '110',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'pscode' => 'Basic',
                                              'val' => '111',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'mnemo' => 'gridsize',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GRIDSIZE>              (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_GRIDSIZE',
                                              'opndparser' => '&Parser::parseOperands'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_LANEID',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_LANEID>                (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'laneid',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '112'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_LDQUEUEREADINDEX>      (HSAIL_ASM::InstQueue(inst))',
                                              'name' => 'BRIG_OPCODE_LDQUEUEREADINDEX',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'QUEUE',
                                              'val' => '113',
                                              'pscode' => 'Queue',
                                              'opcodeparser' => 'parseMnemoQueue',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'ldqueuereadindex'
                                            },
                                            {
                                              'mnemo' => 'ldqueuewriteindex',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoQueue',
                                              'val' => '114',
                                              'pscode' => 'Queue',
                                              'k' => 'QUEUE',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_LDQUEUEWRITEINDEX',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_LDQUEUEWRITEINDEX>     (HSAIL_ASM::InstQueue(inst))'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_MAXCUID>               (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_MAXCUID',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'pscode' => 'Basic',
                                              'val' => '115',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'maxcuid'
                                            },
                                            {
                                              'mnemo' => 'maxwaveid',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '116',
                                              'pscode' => 'Basic',
                                              'psopnd' => 'Operands',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_MAXWAVEID',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_MAXWAVEID>             (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'mnemo' => 'nullptr',
                                              'psopnd' => 'Operands',
                                              'val' => '117',
                                              'pscode' => 'Seg',
                                              'opcodeparser' => 'parseMnemoSeg',
                                              'k' => 'SEG',
                                              'name' => 'BRIG_OPCODE_NULLPTR',
                                              'opndparser' => '&Parser::parseOperands',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_NULLPTR>               (HSAIL_ASM::InstSeg(inst))'
                                            },
                                            {
                                              'pscode' => 'Basic',
                                              'val' => '118',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'packetcompletionsig',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_PACKETCOMPLETIONSIG>   (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_PACKETCOMPLETIONSIG',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '119',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'packetid',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_PACKETID>              (HSAIL_ASM::InstBasic(inst))',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_PACKETID',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'mnemo' => 'queueid',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '120',
                                              'pscode' => 'Basic',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_QUEUEID',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_QUEUEID>               (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'mnemo' => 'queueptr',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoSeg',
                                              'pscode' => 'Seg',
                                              'val' => '121',
                                              'k' => 'SEG',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_QUEUEPTR',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_QUEUEPTR>              (HSAIL_ASM::InstSeg(inst))'
                                            },
                                            {
                                              'val' => '122',
                                              'pscode' => 'Seg',
                                              'opcodeparser' => 'parseMnemoSeg',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'servicequeueptr',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_SERVICEQUEUEPTR>       (HSAIL_ASM::InstSeg(inst))',
                                              'name' => 'BRIG_OPCODE_SERVICEQUEUEPTR',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'SEG'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'numdst' => '0',
                                              'val' => '123',
                                              'pscode' => 'Basic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'setdetectexcept',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_SETDETECTEXCEPT>       (HSAIL_ASM::InstBasic(inst))',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_SETDETECTEXCEPT',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'pscode' => 'Queue',
                                              'val' => '124',
                                              'numdst' => '0',
                                              'opcodeparser' => 'parseMnemoQueue',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'stqueuereadindex',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_STQUEUEREADINDEX>      (HSAIL_ASM::InstQueue(inst))',
                                              'name' => 'BRIG_OPCODE_STQUEUEREADINDEX',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'QUEUE'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_STQUEUEWRITEINDEX>     (HSAIL_ASM::InstQueue(inst))',
                                              'k' => 'QUEUE',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_STQUEUEWRITEINDEX',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoQueue',
                                              'numdst' => '0',
                                              'val' => '125',
                                              'pscode' => 'Queue',
                                              'mnemo' => 'stqueuewriteindex'
                                            },
                                            {
                                              'mnemo' => 'waveid',
                                              'psopnd' => 'Operands',
                                              'val' => '126',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_WAVEID',
                                              'opndparser' => '&Parser::parseOperands',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_WAVEID>                (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'mnemo' => 'workgroupid',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '127',
                                              'pscode' => 'Basic',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_WORKGROUPID',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_WORKGROUPID>           (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'name' => 'BRIG_OPCODE_WORKGROUPSIZE',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_WORKGROUPSIZE>         (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'workgroupsize',
                                              'val' => '128',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'mnemo' => 'workitemabsid',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '129',
                                              'pscode' => 'Basic',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_WORKITEMABSID',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_WORKITEMABSID>         (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_WORKITEMFLATABSID>     (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_WORKITEMFLATABSID',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '130',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'workitemflatabsid'
                                            },
                                            {
                                              'mnemo' => 'workitemflatid',
                                              'psopnd' => 'Operands',
                                              'pscode' => 'Basic',
                                              'val' => '131',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_WORKITEMFLATID',
                                              'opndparser' => '&Parser::parseOperands',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_WORKITEMFLATID>        (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_WORKITEMID>            (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_WORKITEMID',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'pscode' => 'Basic',
                                              'val' => '132',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'workitemid'
                                            },
                                            {
                                              'k' => 'QUERY_IMAGE',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_QUERYIMAGE',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_QUERYIMAGE>            (HSAIL_ASM::InstQueryImage(inst))',
                                              'mnemo' => 'queryimage',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoQueryImage',
                                              'pscode' => 'QueryImage',
                                              'val' => '133'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_QUERYSAMPLER>          (HSAIL_ASM::InstQuerySampler(inst))',
                                              'name' => 'BRIG_OPCODE_QUERYSAMPLER',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'QUERY_SAMPLER',
                                              'pscode' => 'QuerySampler',
                                              'val' => '134',
                                              'opcodeparser' => 'parseMnemoQuerySampler',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'querysampler'
                                            },
                                            {
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNMADU',
                                              'k' => 'BASIC_NO_TYPE',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNMADU>               (HSAIL_ASM::InstBasic(inst))',
                                              'hasType' => 'false',
                                              'mnemo' => 'gcn_madu',
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'val' => '(1u << 15) |  0',
                                              'pscode' => 'BasicNoType',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNMADS>               (HSAIL_ASM::InstBasic(inst))',
                                              'hasType' => 'false',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNMADS',
                                              'k' => 'BASIC_NO_TYPE',
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'pscode' => 'BasicNoType',
                                              'val' => '(1u << 15) |  1',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'gcn_mads'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNMAX3>               (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_GCNMAX3',
                                              'opndparser' => '&Parser::parseOperands',
                                              'psopnd' => 'Operands',
                                              'pscode' => 'Basic',
                                              'val' => '(1u << 15) |  2',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'mnemo' => 'gcn_max3'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNMIN3',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNMIN3>               (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'gcn_min3',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '(1u << 15) |  3',
                                              'pscode' => 'Basic'
                                            },
                                            {
                                              'mnemo' => 'gcn_med3',
                                              'psopnd' => 'Operands',
                                              'pscode' => 'Basic',
                                              'val' => '(1u << 15) |  4',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_GCNMED3',
                                              'opndparser' => '&Parser::parseOperands',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNMED3>               (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '(1u << 15) |  5',
                                              'mnemo' => 'gcn_fldexp',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNFLDEXP>             (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNFLDEXP'
                                            },
                                            {
                                              'mnemo' => 'gcn_frexp_exp',
                                              'pscode' => 'Basic',
                                              'val' => '(1u << 15) |  6',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'name' => 'BRIG_OPCODE_GCNFREXP_EXP',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNFREXP_EXP>          (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '(1u << 15) |  7',
                                              'pscode' => 'Basic',
                                              'mnemo' => 'gcn_frexp_mant',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNFREXP_MANT>         (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNFREXP_MANT'
                                            },
                                            {
                                              'mnemo' => 'gcn_trig_preop',
                                              'pscode' => 'Basic',
                                              'val' => '(1u << 15) |  8',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'name' => 'BRIG_OPCODE_GCNTRIG_PREOP',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNTRIG_PREOP>         (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_GCNBFM',
                                              'opndparser' => '&Parser::parseOperands',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNBFM>                (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'gcn_bfm',
                                              'psopnd' => 'Operands',
                                              'pscode' => 'Basic',
                                              'val' => '(1u << 15) |  9',
                                              'opcodeparser' => 'parseMnemoBasic'
                                            },
                                            {
                                              'k' => 'MEM',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNLD',
                                              'semsupport' => 'true',
                                              'has_memory_order' => 'true',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNLD>                 (HSAIL_ASM::InstMem(inst))',
                                              'mnemo' => 'gcn_ld',
                                              'psopnd' => 'Operands',
                                              'mnemo_token' => 'EInstruction_Vx',
                                              'opcodeparser' => 'parseMnemoMem',
                                              'val' => '(1u << 15) | 10',
                                              'pscode' => 'Mem'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNST>                 (HSAIL_ASM::InstMem(inst))',
                                              'has_memory_order' => 'true',
                                              'semsupport' => 'true',
                                              'name' => 'BRIG_OPCODE_GCNST',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'MEM',
                                              'pscode' => 'Mem',
                                              'val' => '(1u << 15) | 11',
                                              'opcodeparser' => 'parseMnemoMem',
                                              'psopnd' => 'Operands',
                                              'mnemo_token' => 'EInstruction_Vx',
                                              'mnemo' => 'gcn_st'
                                            },
                                            {
                                              'k' => 'ATOMIC',
                                              'name' => 'BRIG_OPCODE_GCNATOMIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNATOMIC>             (HSAIL_ASM::InstAtomic(inst))',
                                              'mnemo' => 'gcn_atomic',
                                              'psopnd' => 'Operands',
                                              'val' => '(1u << 15) | 12',
                                              'pscode' => 'Atomic',
                                              'opcodeparser' => 'parseMnemoAtomic'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNATOMICNORET>        (HSAIL_ASM::InstAtomic(inst))',
                                              'k' => 'ATOMIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNATOMICNORET',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoAtomic',
                                              'pscode' => 'Atomic',
                                              'val' => '(1u << 15) | 13',
                                              'mnemo' => 'gcn_atomicNoRet'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'name' => 'BRIG_OPCODE_GCNSLEEP',
                                              'opndparser' => '&Parser::parseOperands',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNSLEEP>              (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'gcn_sleep',
                                              'psopnd' => 'Operands',
                                              'pscode' => 'Basic',
                                              'val' => '(1u << 15) | 14',
                                              'opcodeparser' => 'parseMnemoBasic'
                                            },
                                            {
                                              'mnemo' => 'gcn_priority',
                                              'pscode' => 'Basic',
                                              'val' => '(1u << 15) | 15',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'name' => 'BRIG_OPCODE_GCNPRIORITY',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNPRIORITY>           (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNREGIONALLOC',
                                              'k' => 'BASIC_NO_TYPE',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNREGIONALLOC>        (HSAIL_ASM::InstBasic(inst))',
                                              'hasType' => 'false',
                                              'mnemo' => 'gcn_region_alloc',
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'val' => '(1u << 15) | 16',
                                              'pscode' => 'BasicNoType',
                                              'psopnd' => 'Operands'
                                            },
                                            {
                                              'mnemo' => 'gcn_msad',
                                              'val' => '(1u << 15) | 17',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'name' => 'BRIG_OPCODE_GCNMSAD',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNMSAD>               (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNQSAD',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNQSAD>               (HSAIL_ASM::InstBasic(inst))',
                                              'mnemo' => 'gcn_qsad',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '(1u << 15) | 18'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNMQSAD>              (HSAIL_ASM::InstBasic(inst))',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNMQSAD',
                                              'k' => 'BASIC',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '(1u << 15) | 19',
                                              'pscode' => 'Basic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'gcn_mqsad'
                                            },
                                            {
                                              'val' => '(1u << 15) | 20',
                                              'pscode' => 'BasicNoType',
                                              'opcodeparser' => 'parseMnemoBasicNoType',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'gcn_mqsad4',
                                              'hasType' => 'false',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNMQSAD4>             (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_GCNMQSAD4',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC_NO_TYPE'
                                            },
                                            {
                                              'mnemo' => 'gcn_sadw',
                                              'val' => '(1u << 15) | 21',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'name' => 'BRIG_OPCODE_GCNSADW',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNSADW>               (HSAIL_ASM::InstBasic(inst))'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '(1u << 15) | 22',
                                              'mnemo' => 'gcn_sadd',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNSADD>               (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNSADD'
                                            },
                                            {
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoAddr',
                                              'pscode' => 'Addr',
                                              'val' => '(1u << 15) | 23',
                                              'mnemo' => 'gcn_atomic_consume',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNCONSUME>            (HSAIL_ASM::InstAddr(inst))',
                                              'k' => 'ADDR',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNCONSUME'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoAddr',
                                              'pscode' => 'Addr',
                                              'val' => '(1u << 15) | 24',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'gcn_atomic_append',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNAPPEND>             (HSAIL_ASM::InstAddr(inst))',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNAPPEND',
                                              'k' => 'ADDR'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNB4XCHG>             (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_GCNB4XCHG',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC',
                                              'pscode' => 'Basic',
                                              'val' => '(1u << 15) | 25',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'gcn_b4xchg'
                                            },
                                            {
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNB32XCHG>            (HSAIL_ASM::InstBasic(inst))',
                                              'k' => 'BASIC',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNB32XCHG',
                                              'psopnd' => 'Operands',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'pscode' => 'Basic',
                                              'val' => '(1u << 15) | 26',
                                              'mnemo' => 'gcn_b32xchg'
                                            },
                                            {
                                              'val' => '(1u << 15) | 27',
                                              'pscode' => 'Basic',
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'gcn_max',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNMAX>                (HSAIL_ASM::InstBasic(inst))',
                                              'name' => 'BRIG_OPCODE_GCNMAX',
                                              'opndparser' => '&Parser::parseOperands',
                                              'k' => 'BASIC'
                                            },
                                            {
                                              'opcodeparser' => 'parseMnemoBasic',
                                              'val' => '(1u << 15) | 28',
                                              'pscode' => 'Basic',
                                              'psopnd' => 'Operands',
                                              'mnemo' => 'gcn_min',
                                              'opcodevis' => 'vis.template visitOpcode<BRIG_OPCODE_GCNMIN>                (HSAIL_ASM::InstBasic(inst))',
                                              'opndparser' => '&Parser::parseOperands',
                                              'name' => 'BRIG_OPCODE_GCNMIN',
                                              'k' => 'BASIC'
                                            }
                                          ],
                             'k' => sub { "DUMMY" },
                             'ftz_proto' => 'inline bool instSupportsFtz(Brig::BrigOpcode16_t arg)',
                             'numdst' => sub { "DUMMY" },
                             'ftz' => sub { "DUMMY" },
                             'mnemo_token' => 'EInstruction',
                             'mnemo#calcState' => 'done',
                             'opcodevis_incfile' => 'ItemUtils',
                             'opcodeparser_switch' => 'true',
                             'opcodeparser_incfile' => 'ParserUtilities',
                             'has_memory_order#calcState' => 'done',
                             'hasType' => sub { "DUMMY" },
                             'numdst_default' => 'return 1',
                             'mnemo' => sub { "DUMMY" },
                             'ftz_switch' => 'true',
                             'mnemo_scanner' => 'Instructions',
                             'opcodeparser' => sub { "DUMMY" },
                             'pscode#calcState' => 'done',
                             'opndparser_proto' => 'Parser::OperandParser Parser::getOperandParser(Brig::BrigOpcode16_t arg)',
                             'tdcaption' => 'Instruction Opcodes',
                             'k#deps' => [],
                             'semsupport#calcState' => 'done',
                             'semsupport' => sub { "DUMMY" },
                             'opcodevis_arg' => 'inst.opcode()',
                             'has_memory_order' => sub { "DUMMY" },
                             'opcodeparser_default' => 'return NULL',
                             'opcodevis_proto' => 'template <typename RetType, typename Visitor> RetType visitOpcode_gen(HSAIL_ASM::Inst inst, Visitor& vis)',
                             'opcodevis_switch' => 'true',
                             'opcodeparser#deps' => [
                                                      'pscode'
                                                    ],
                             'mnemo#deps' => [],
                             'opndparser_incfile' => 'ParserUtilities',
                             'opndparser' => sub { "DUMMY" },
                             'opndparser#deps' => [
                                                    'psopnd'
                                                  ],
                             'opcodevis_default' => 'return RetType()',
                             'opndparser#calcState' => 'done',
                             'numdst_proto' => 'int instNumDstOperands(Brig::BrigOpcode16_t arg)',
                             'opndparser_default' => 'return NULL',
                             'semsupport#deps' => [
                                                    'has_memory_order'
                                                  ],
                             'hasType_default' => 'return true',
                             'hasType#calcState' => 'done',
                             'opcodeparser_proto' => 'OpcodeParser getOpcodeParser(Brig::BrigOpcode16_t arg)',
                             'psopnd#calcState' => 'done',
                             'ftz_default' => 'return false',
                             'pscode' => sub { "DUMMY" },
                             'psopnd' => sub { "DUMMY" },
                             'hasType#deps' => [
                                                 'k'
                                               ],
                             'opcodevis' => sub { "DUMMY" },
                             'hasType_switch' => 'true',
                             'ftz#calcState' => 'done',
                             'psopnd#deps' => [],
                             'opcodeparser#calcState' => 'done'
                           },
           'BrigSymbolModifierMask' => {
                                         'nodump' => 'true',
                                         'entries' => [
                                                        {
                                                          'val' => '3',
                                                          'name' => 'BRIG_SYMBOL_LINKAGE'
                                                        },
                                                        {
                                                          'val' => '4',
                                                          'name' => 'BRIG_SYMBOL_DECLARATION'
                                                        },
                                                        {
                                                          'name' => 'BRIG_SYMBOL_CONST',
                                                          'val' => '8'
                                                        },
                                                        {
                                                          'val' => '16',
                                                          'name' => 'BRIG_SYMBOL_ARRAY'
                                                        },
                                                        {
                                                          'val' => '32',
                                                          'name' => 'BRIG_SYMBOL_FLEX_ARRAY'
                                                        }
                                                      ],
                                         'name' => 'BrigSymbolModifierMask'
                                       },
           'BrigAlignment' => {
                                'entries' => [
                                               {
                                                 'val' => '0',
                                                 'name' => 'BRIG_ALIGNMENT_NONE'
                                               },
                                               {
                                                 'name' => 'BRIG_ALIGNMENT_1',
                                                 'val' => '1'
                                               },
                                               {
                                                 'val' => '2',
                                                 'name' => 'BRIG_ALIGNMENT_2'
                                               },
                                               {
                                                 'val' => '3',
                                                 'name' => 'BRIG_ALIGNMENT_4'
                                               },
                                               {
                                                 'val' => '4',
                                                 'name' => 'BRIG_ALIGNMENT_8'
                                               },
                                               {
                                                 'val' => '5',
                                                 'name' => 'BRIG_ALIGNMENT_16'
                                               },
                                               {
                                                 'name' => 'BRIG_ALIGNMENT_32',
                                                 'val' => '6'
                                               },
                                               {
                                                 'name' => 'BRIG_ALIGNMENT_64',
                                                 'val' => '7'
                                               },
                                               {
                                                 'val' => '8',
                                                 'name' => 'BRIG_ALIGNMENT_128'
                                               },
                                               {
                                                 'val' => '9',
                                                 'name' => 'BRIG_ALIGNMENT_256'
                                               },
                                               {
                                                 'skip' => 'true',
                                                 'name' => 'BRIG_ALIGNMENT_LAST'
                                               },
                                               {
                                                 'name' => 'BRIG_ALIGNMENT_MAX',
                                                 'val' => 'BRIG_ALIGNMENT_LAST - 1 ',
                                                 'skip' => 'true'
                                               }
                                             ],
                                'name' => 'BrigAlignment'
                              },
           'BrigMemoryModifierMask' => {
                                         'name' => 'BrigMemoryModifierMask',
                                         'entries' => [
                                                        {
                                                          'val' => '1',
                                                          'name' => 'BRIG_MEMORY_CONST'
                                                        }
                                                      ]
                                       },
           'BrigSamplerAddressing' => {
                                        'mnemo#deps' => [],
                                        'mnemo_token' => 'ESamplerAddressingMode',
                                        'name' => 'BrigSamplerAddressing',
                                        'mnemo#calcState' => 'done',
                                        'mnemo' => sub { "DUMMY" },
                                        'entries' => [
                                                       {
                                                         'val' => '0',
                                                         'name' => 'BRIG_ADDRESSING_UNDEFINED',
                                                         'mnemo' => 'undefined'
                                                       },
                                                       {
                                                         'val' => '1',
                                                         'name' => 'BRIG_ADDRESSING_CLAMP_TO_EDGE',
                                                         'mnemo' => 'clamp_to_edge'
                                                       },
                                                       {
                                                         'val' => '2',
                                                         'name' => 'BRIG_ADDRESSING_CLAMP_TO_BORDER',
                                                         'mnemo' => 'clamp_to_border'
                                                       },
                                                       {
                                                         'val' => '3',
                                                         'mnemo' => 'repeat',
                                                         'name' => 'BRIG_ADDRESSING_REPEAT'
                                                       },
                                                       {
                                                         'mnemo' => 'mirrored_repeat',
                                                         'name' => 'BRIG_ADDRESSING_MIRRORED_REPEAT',
                                                         'val' => '4'
                                                       }
                                                     ]
                                      }
         };
