//===- AMDILRegisterInfo.td - AMDIL Register defs ----------*- tablegen -*-===//
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
//  Declarations that describe the AMDIL register file
//
//===----------------------------------------------------------------------===//

class AMDILRegWithSubReg<bits<16> num, string n, list<Register> subregs, list<SubRegIndex> subRegIdx>
: RegisterWithSubRegs<n, subregs> {
  field bits<16> Value;
  let Value = num;
  let Namespace = "AMDIL";
  let SubRegIndices = subRegIdx;
}

class AMDILReg<bits<16> num, string n> : Register<n> {
  field bits<16> Value;
  let Value = num;
  let Namespace = "AMDIL";
}

let Namespace = "AMDIL" in {
  // TODO: Use this with LLVM 3.3+
  // def sub_x : SubRegIndex<32, 0>;
  // def sub_y : SubRegIndex<32, 32>;
  // def sub_z : SubRegIndex<32, 64>;
  // def sub_w : SubRegIndex<32, 96>;
  // def sub_xy : SubRegIndex<64, 0>;
  // def sub_zw : SubRegIndex<64, 64>;

  def sub_x : SubRegIndex;
  def sub_y : SubRegIndex;
  def sub_z : SubRegIndex;
  def sub_w : SubRegIndex;

  def sub_xy : SubRegIndex<[sub_x, sub_y]>;
  def sub_zw : SubRegIndex<[sub_z, sub_w]>;
}

foreach Index = {1-288, 1000-1012} in {
  def Rx#Index : AMDILReg<Index, "r"#Index>,
    DwarfRegNum<[Index]>;

  foreach Chan = [ "y", "z", "w" ] in {
    def R#Chan#Index : AMDILReg<Index, "r"#Index>,
      DwarfRegAlias<!cast<Register>("Rx"#Index)>;
  }

  def Rxy#Index : AMDILRegWithSubReg<Index, "r"#Index,
                                     [!cast<Register>("Rx"#Index), !cast<Register>("Ry"#Index)],
                                     [sub_x, sub_y]>,
      DwarfRegAlias<!cast<Register>("Rx"#Index)>;

  def Rzw#Index : AMDILRegWithSubReg<Index, "r"#Index,
                                     [!cast<Register>("Rz"#Index), !cast<Register>("Rw"#Index)],
                                     [sub_z, sub_w]>,
    DwarfRegAlias<!cast<Register>("Rx"#Index)>;

  def R#Index : AMDILRegWithSubReg<Index, "r"#Index,
                                  [!cast<Register>("Rxy"#Index), !cast<Register>("Rzw"#Index)],
                                  [sub_xy, sub_zw]>,
    DwarfRegAlias<!cast<Register>("Rx"#Index)>;
}

foreach Index = {0-255} in {
  def INx#Index : AMDILReg<Index, "in"#Index>,
    DwarfRegNum<[Index]>;

  foreach Chan = [ "y", "z", "w" ] in {
    def IN#Chan#Index : AMDILReg<Index, "in"#Index>,
      DwarfRegAlias<!cast<Register>("INx"#Index)>;
  }

  def INxy#Index : AMDILRegWithSubReg<Index, "in"#Index,
                                     [!cast<Register>("INx"#Index), !cast<Register>("INy"#Index)],
                                     [sub_x, sub_y]>,
      DwarfRegAlias<!cast<Register>("INx"#Index)>;

  def INzw#Index : AMDILRegWithSubReg<Index, "in"#Index,
                                     [!cast<Register>("INz"#Index), !cast<Register>("INw"#Index)],
                                     [sub_z, sub_w]>,
    DwarfRegAlias<!cast<Register>("INx"#Index)>;

  def IN#Index : AMDILRegWithSubReg<Index, "in"#Index,
                                  [!cast<Register>("INxy"#Index), !cast<Register>("INzw"#Index)],
                                  [sub_xy, sub_zw]>,
    DwarfRegAlias<!cast<Register>("INx"#Index)>;
}

foreach Index = {0-7} in {
  // Scalar.
  def OUTx#Index : AMDILReg<Index, "out"#Index>,
    DwarfRegNum<[Index]>;
  def OUTy#Index : AMDILReg<Index, "out"#Index>,
    DwarfRegAlias<!cast<Register>("OUTx"#Index)>;
  def OUTz#Index : AMDILReg<Index, "out"#Index>,
    DwarfRegAlias<!cast<Register>("OUTx"#Index)>;
  def OUTw#Index : AMDILReg<Index, "out"#Index>,
    DwarfRegAlias<!cast<Register>("OUTx"#Index)>;

  // V2.
  def OUTxy#Index : AMDILRegWithSubReg<Index, "out"#Index,
    [!cast<Register>("OUTx"#Index), !cast<Register>("OUTy"#Index)],
    [sub_x, sub_y]>, DwarfRegAlias<!cast<Register>("OUTx"#Index)>;
  def OUTzw#Index : AMDILRegWithSubReg<Index, "out"#Index,
    [!cast<Register>("OUTz"#Index), !cast<Register>("OUTw"#Index)],
    [sub_z, sub_w]>, DwarfRegAlias<!cast<Register>("OUTx"#Index)>;

  // V4.
  def OUT#Index : AMDILRegWithSubReg<Index, "out"#Index,
    [!cast<Register>("OUTxy"#Index), !cast<Register>("OUTzw"#Index)],
    [sub_xy, sub_zw]>, DwarfRegAlias<!cast<Register>("OUTx"#Index)>;
}

foreach Index = {0-255} in {
  // CB1 Scalar.
  def CB1_x#Index : AMDILReg<0, "cb1["#Index#"]">,
    DwarfRegNum<[Index]>;
  def CB1_y#Index : AMDILReg<0, "cb1["#Index#"]">,
    DwarfRegAlias<!cast<Register>("CB1_x"#Index)>;
  def CB1_z#Index : AMDILReg<0, "cb1["#Index#"]">,
    DwarfRegAlias<!cast<Register>("CB1_x"#Index)>;
  def CB1_w#Index : AMDILReg<0, "cb1["#Index#"]">,
    DwarfRegAlias<!cast<Register>("CB1_x"#Index)>;

  // CB1 V2.
  def CB1_xy#Index : AMDILRegWithSubReg<Index, "cb1["#Index#"]",
    [!cast<Register>("CB1_x"#Index), !cast<Register>("CB1_y"#Index)],
    [sub_x, sub_y]>, DwarfRegAlias<!cast<Register>("CB1_x"#Index)>;
  def CB1_zw#Index : AMDILRegWithSubReg<Index, "cb1["#Index#"]",
    [!cast<Register>("CB1_z"#Index), !cast<Register>("CB1_w"#Index)],
    [sub_z, sub_w]>, DwarfRegAlias<!cast<Register>("CB1_x"#Index)>;

  // CB1 V4.
  def CB1_#Index : AMDILRegWithSubReg<Index, "cb1["#Index#"]",
    [!cast<Register>("CB1_x"#Index), !cast<Register>("CB1_z"#Index)],
    [sub_xy, sub_zw]>, DwarfRegAlias<!cast<Register>("CB1_x"#Index)>;
}

// All registers between 1000 and 1024 are reserved and cannot be used
// unless commented in this section
// The reserved registers used for temporaries should be added to the
// register class ReservedTemp.
// The reserved registers which are read only in non-kernel functions
// should be added to register class ReservedReadonly.

// r1021-r1039 are used to dynamically calculate the local/group/thread/region/region_local ID's
// r1020 is used to hold the frame index for local arrays
// r1019 is used to hold the dynamic stack allocation pointer
// r1018 is used as a temporary register for handwritten code
// r1017 is used as a temporary register for handwritten code
// r1016 is used as a temporary register for load/store code
// r1015 is used as a temporary register for data segment offset
// r1014 is used as a temporary register for store code
// r1013 is used as the section data pointer register
// r1012-r1010 and r1001-r1008 are used for temporary I/O registers
// r1028 is used as the frame pointer register
// r1027 is used as mem register
// r1026 is used as the return address register.
// r1030-r1039 are reserved for AMDILCFGStructurizer.
// r1040 is reserved for printf implementation.
// r1130-r1139 are r1030-r1039 renumbered for AMDIL function support
//def R1025 : AMDILReg<1025, "r1025">, DwarfRegNum<[1025]>;
//def R1024 : AMDILReg<1024, "r1024">, DwarfRegNum<[1024]>;
//def R1023 : AMDILReg<1023, "r1023">, DwarfRegNum<[1023]>;
//def R1022 : AMDILReg<1022, "r1022">, DwarfRegNum<[1022]>;
//def R1021 : AMDILReg<1021, "r1021">, DwarfRegNum<[1021]>;
//def R1020 : AMDILReg<1020, "r1020">, DwarfRegNum<[1020]>;

// Registers in ReservedReadOnly and ReservedReadWrite should not use
// temporary registers as they only have function scope
def SP : AMDILReg<4076, "x0[0]">, DwarfRegNum<[1019]>;
def SDP : AMDILReg<4052, "x0[6]">, DwarfRegNum<[1013]>;
def T1 : AMDILReg<4072, "x0[7]">, DwarfRegNum<[1018]>;
def T2 : AMDILReg<4068, "x0[8]">, DwarfRegNum<[1017]>;
def T3 : AMDILReg<4064, "r1016">, DwarfRegNum<[1016]>;
def T4 : AMDILReg<4060, "r1015">, DwarfRegNum<[1015]>;
def T5 : AMDILReg<4056, "r1014">, DwarfRegNum<[1014]>;
def MEMx : AMDILReg<5008,"mem0">, DwarfRegNum<[5008]>;
def MEMxy : AMDILReg<5008, "mem0">, DwarfRegAlias<MEMx>;
def MEMxyz : AMDILReg<5008, "mem0">, DwarfRegAlias<MEMx>;
def MEM   : AMDILReg<5008, "mem0">, DwarfRegAlias<MEMx>;

def RA : AMDILReg<5004, "r1026">, DwarfRegNum<[1026]>;
def FP : AMDILReg<5012, "x0[9]">, DwarfRegNum<[1028]>;
def DFP : AMDILReg<5016, "r1029">, DwarfRegNum<[1029]>;
def CFG1 : AMDILReg<5020, "r1030">, DwarfRegNum<[1030]>;
def CFG2 : AMDILReg<5024, "r1031">, DwarfRegNum<[1031]>;
def CFG3 : AMDILReg<5028, "r1032">, DwarfRegNum<[1032]>;
def CFG4 : AMDILReg<5032, "r1033">, DwarfRegNum<[1033]>;
def CFG5 : AMDILReg<5036, "r1034">, DwarfRegNum<[1034]>;
def CFG6 : AMDILReg<5040, "r1035">, DwarfRegNum<[1035]>;
def CFG7 : AMDILReg<5044, "r1036">, DwarfRegNum<[1036]>;
def CFG8 : AMDILReg<5048, "r1037">, DwarfRegNum<[1037]>;
def CFG9 : AMDILReg<5052, "r1038">, DwarfRegNum<[1038]>;
def CFG10 : AMDILReg<5056, "r1039">, DwarfRegNum<[1039]>;
def PRINTF : AMDILReg<5060, "x0[5]">, DwarfRegNum<[1040]>;
def CFG11 : AMDILReg<5120, "r1130">, DwarfRegNum<[1130]>;
def CFG12 : AMDILReg<5124, "r1131">, DwarfRegNum<[1131]>;
def CFG13 : AMDILReg<5128, "r1132">, DwarfRegNum<[1132]>;
def CFG14 : AMDILReg<5132, "r1133">, DwarfRegNum<[1133]>;
def CFG15 : AMDILReg<5136, "r1134">, DwarfRegNum<[1134]>;
def CFG16 : AMDILReg<5140, "r1135">, DwarfRegNum<[1135]>;
def CFG17 : AMDILReg<5144, "r1136">, DwarfRegNum<[1136]>;
def CFG18 : AMDILReg<5148, "r1137">, DwarfRegNum<[1137]>;
def CFG19 : AMDILReg<5152, "r1138">, DwarfRegNum<[1138]>;
def CFG20 : AMDILReg<5156, "r1139">, DwarfRegNum<[1139]>;


// def GPR_32 : RegisterClass<"AMDIL", [i8, i16, i32, f32], 32,
//   (add (interleave (add (sequence "Rx%u", 1, 288), (sequence "Rx%u", 1000, 1020)),
//                    (add (sequence "Ry%u", 1, 288), (sequence "Ry%u", 1000, 1020)),
//                    (add (sequence "Rz%u", 1, 288), (sequence "Rz%u", 1000, 1020)),
//                    (add (sequence "Rw%u", 1, 288), (sequence "Rw%u", 1000, 1020))),
//         FP, RA, SDP, T5, MEMx)>;

// def GPR_64 : RegisterClass<"AMDIL", [i64, f64, v2i8, v2i16, v2i32, v2f32], 64,
//   (interleave (add (sequence "Rxy%u", 1, 288), (sequence "Rxy%u", 1000, 1020)),
//               (add (sequence "Rzw%u", 1, 288), (sequence "Rzw%u", 1000, 1020)))>;

// def GPR_128 : RegisterClass<"AMDIL", [v4i32, v4f32, v4i8, v4i16, v2i64, v2f64], 128,
//   (add (sequence "R%u", 1, 288), (sequence "R%u", 1000, 1020))>;



def GPR_X_32 : RegisterClass<"AMDIL", [i32, f32], 32,
  (add (sequence "Rx%u", 1, 288), (sequence "Rx%u", 1000, 1012),
       (sequence "INx%u", 0, 255), (sequence "OUTx%u", 0, 7))> {
  let AltOrders = [(add (sequence "Rx%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPR_Y_32 : RegisterClass<"AMDIL", [i32, f32], 32,
  (add (sequence "Ry%u", 1, 288), (sequence "Ry%u", 1000, 1012),
       (sequence "INy%u", 0, 255), (sequence "OUTy%u", 0, 7))> {
  let AltOrders = [(add (sequence "Ry%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPR_Z_32 : RegisterClass<"AMDIL", [i32, f32], 32,
  (add (sequence "Rz%u", 1, 288), (sequence "Rz%u", 1000, 1012),
       (sequence "INz%u", 0, 255), (sequence "OUTz%u", 0, 7))> {
  let AltOrders = [(add (sequence "Rz%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPR_W_32 : RegisterClass<"AMDIL", [i32, f32], 32,
  (add (sequence "Rw%u", 1, 288), (sequence "Rw%u", 1000, 1012),
       (sequence "INw%u", 0, 255), (sequence "OUTw%u", 0, 7))> {
  let AltOrders = [(add (sequence "Rw%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPRI8 : RegisterClass<"AMDIL", [i8], 8,
  (interleave GPR_X_32, GPR_Y_32, GPR_Z_32, GPR_W_32)> {
  let AltOrders = [(add (sequence "Rx%u", 33, 160), (sequence "Ry%u", 33, 160), (sequence "Rz%u", 33, 160), (sequence "Rw%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPRI16 : RegisterClass<"AMDIL", [i16], 16, (add GPRI8)> {
  let AltOrders = [(add (sequence "Rx%u", 33, 160), (sequence "Ry%u", 33, 160), (sequence "Rz%u", 33, 160), (sequence "Rw%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPR_32 : RegisterClass<"AMDIL", [i32, f32], 32,
  (add GPRI8, FP, RA, SDP, T5, MEMx)> {
  let AltOrders = [(add (sequence "Rx%u", 33, 160), (sequence "Ry%u", 33, 160), (sequence "Rz%u", 33, 160), (sequence "Rw%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPR_XY_64 : RegisterClass<"AMDIL", [i64, f64], 64,
  (add (sequence "Rxy%u", 1, 288), (sequence "Rxy%u", 1000, 1012),
       (sequence "INxy%u", 0, 255), (sequence "OUTxy%u", 0, 7))> {
  let AltOrders = [(add (sequence "Rxy%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPR_ZW_64 : RegisterClass<"AMDIL", [i64, f64], 64,
  (add (sequence "Rzw%u", 1, 288), (sequence "Rzw%u", 1000, 1012),
       (sequence "INzw%u", 0, 255), (sequence "OUTzw%u", 0, 7))> {
  let AltOrders = [(add (sequence "Rzw%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPR_64 : RegisterClass<"AMDIL", [i64, f64], 64, (interleave GPR_XY_64, GPR_ZW_64)> {
  let AltOrders = [(add (sequence "Rxy%u", 33, 160), (sequence "Rzw%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPRV2I8 : RegisterClass<"AMDIL", [v2i8], 16, (add GPR_64)> {
  let AltOrders = [(add (sequence "Rxy%u", 33, 160), (sequence "Rzw%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPRV2I16 : RegisterClass<"AMDIL", [v2i16], 32, (add GPR_64)> {
  let AltOrders = [(add (sequence "Rxy%u", 33, 160), (sequence "Rzw%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPRV2I32 : RegisterClass<"AMDIL", [v2i32, v2f32], 64, (add GPR_64, MEMxy)> {
  let AltOrders = [(add (sequence "Rxy%u", 33, 160), (sequence "Rzw%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPRXYV2I32 : RegisterClass<"AMDIL", [v2i32, v2f32], 64, (add GPR_XY_64, MEMxy)> {
  let AltOrders = [(add (sequence "Rxy%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPRV2I64 : RegisterClass<"AMDIL", [v2i64, v2f64], 128,
    (add (sequence "R%u", 1, 288), (sequence "R%u", 1000, 1012),
         (sequence "IN%u", 0, 255), (sequence "OUT%u", 0, 7))> {
  let AltOrders = [(add (sequence "R%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPRV4I8 : RegisterClass<"AMDIL", [v4i8], 32, (add GPRV2I64)> {
  let AltOrders = [(add (sequence "R%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPRV4I16 : RegisterClass<"AMDIL", [v4i16], 64, (add GPRV2I64)> {
  let AltOrders = [(add (sequence "R%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def GPRV4I32 : RegisterClass<"AMDIL", [v4i32, v4f32], 128,
    (add GPRV2I64, SP, T1, T2, T3, T4, MEM)> {
  let AltOrders = [(add (sequence "R%u", 33, 160))];
  let AltOrderSelect = [{
    return 1;
  }];
}

def ReservedTemp : RegisterClass<"AMDIL", [i32], 32,
  (add FP, DFP, RA, PRINTF,
    (sequence "R%u", 1000, 1012),
    (sequence "Rx%u", 1000, 1012),
    (sequence "Ry%u", 1000, 1012),
    (sequence "Rz%u", 1000, 1012),
    (sequence "Rw%u", 1000, 1012),
    (sequence "Rxy%u", 1000, 1012),
    (sequence "Rzw%u", 1000, 1012))>;

def ReservedReadonly : RegisterClass<"AMDIL", [i32], 32,
    (add SDP, T1, T2, T3, T4, T5, MEM, MEMx, MEMxy)>;

def ReservedReadwrite : RegisterClass<"AMDIL", [i32], 32, (add SP)>;
