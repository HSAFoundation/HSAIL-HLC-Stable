//===--------------------------------------------------------------------===//
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
// ILTokenDesc.td - The IL Token Descriptions - Micah Villmow - 9-3-2008
//===--------------------------------------------------------------------===//
include "AMDILEnumeratedTypes.td"
//===--------------------------------------------------------------------===//
//
//
//
//===--------------------------------------------------------------------===//
// Each token is 32 bits as specified in section 2.1 of the IL spec
class ILToken <bits<32> n> {
    field bits<32> _bits = n;
}

// Section 2.2.1 - IL Language Token
class ILLang<bits<8> client_type> : ILToken<0> {
    let _bits{0-7} = client_type;
}

// Section 2.2.2 - IL Version Token
class ILVersion<bits<8> minor_version, bits<8> major_version, ILShader shader_type>  : ILToken<0> {
    let _bits{0-7} = minor_version;
    let _bits{8-15} = major_version;
    let _bits{16-23} = shader_type.Value;
}

// Section 2.2.3 - IL Opcode Token
class ILOpcode<ILOpCode opcode, bits<14> control, bit sec_mod_pre, bit pri_mod_pre> : ILToken<0> {
    let _bits{0-15} = opcode.Value;
    let _bits{16-29} = control;
    let _bits{30} = sec_mod_pre;
    let _bits{31} = pri_mod_pre;
}

// Section 2.2.4 - IL Destination Token
class ILDst<AMDILReg register_num, ILRegType register_type, bit mod_pre, bits<2> relative_address, bit dimension, bit immediate_pre, bit extended> : ILToken<0> {
    let _bits{0-15} = register_num.Value;
    let _bits{16-21} = register_type.Value;
    let _bits{22} = mod_pre;
    let _bits{23-24} = relative_address;
    let _bits{25} = dimension;
    let _bits{26} = immediate_pre;
    let _bits{31} = extended;
}

// Section 2.2.5 - IL Destination Modifier Token
class ILDstMod<ILModDstComp x, ILModDstComp y, ILModDstComp z, ILModDstComp w, bit clamp, ILShiftScale shift_scale> : ILToken<0> {
    let _bits{0-1} = x.Value;
    let _bits{2-3} = y.Value;
    let _bits{4-5} = z.Value;
    let _bits{6-7} = w.Value;
    let _bits{8} = clamp;
    //let _bits{9-12} = shift_scale;
}

// Section 2.2.6 - IL Source Token
class ILSrc<AMDILReg register_num, ILRegType register_type, bit mod_pre, bits<2> relative_address, bit dimension, bit immediate_pre, bit extended> : ILToken<0> {
    let _bits{0-15} = register_num.Value;
    let _bits{16-21} = register_type.Value;
    let _bits{22} = mod_pre;
    let _bits{23-24} = relative_address;
    let _bits{25} = dimension;
    let _bits{26} = immediate_pre;
    let _bits{31} = extended;
}

// Section 2.2.7 - IL Source Modifier Token
class ILSrcMod<ILComponentSelect swizzle_x, bit negate_x, ILComponentSelect swizzle_y, bit negate_y,
               ILComponentSelect swizzle_z, bit negate_z, ILComponentSelect swizzle_w, bit negate_w,
               bit invert, bit bias, bit x2, bit sign, bit abs, ILDivComp divComp,
               bits<8> clamp> : ILToken<0> {
    let _bits{0-2} = swizzle_x.Value;
    let _bits{3} = negate_x;
    let _bits{4-6} = swizzle_y.Value;
    let _bits{7} = negate_y;
    let _bits{8-10} = swizzle_z.Value;
    let _bits{11} = negate_z;
    let _bits{12-14} = swizzle_w.Value;
    let _bits{15} = negate_w;
    let _bits{16} = invert;
    let _bits{17} = bias;
    let _bits{18} = x2;
    let _bits{19} = sign;
    let _bits{20} = abs;
    let _bits{21-23} = divComp.Value;
    let _bits{24-31} = clamp;
}

// Section 2.2.8 - IL Relative Address Token
class ILRelAddr<AMDILReg address_register, bit loop_relative, ILAddressing component> : ILToken<0> {
    let _bits{0-15} = address_register.Value;
    let _bits{16} = loop_relative;
    let _bits{17-19} = component.Value;
}

// IL Literal Token
class ILLiteral<bits<32> val> : ILToken<0> {
    let _bits = val;
}

// All tokens required for a destination register
class ILDstReg<ILDst Reg, ILDstMod Mod, ILRelAddr Rel, ILSrc Reg_Rel, ILSrcMod Reg_Rel_Mod> {
    ILDst       reg = Reg;
    ILDstMod    mod = Mod;
    ILRelAddr   rel = Rel;
    ILSrc       reg_rel = Reg_Rel;
    ILSrcMod    reg_rel_mod = Reg_Rel_Mod;
}

// All tokens required for a source register
class ILSrcReg<ILSrc Reg, ILSrcMod Mod, ILRelAddr Rel, ILSrc Reg_Rel, ILSrcMod Reg_Rel_Mod> {
    ILSrc       reg = Reg;
    ILSrcMod    mod = Mod;
    ILRelAddr   rel = Rel;
    ILSrc       reg_rel = Reg_Rel;
    ILSrcMod    reg_rel_mod = Reg_Rel_Mod;
}

