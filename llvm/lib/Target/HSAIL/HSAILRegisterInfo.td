//==- HSAILRegisterInfo.td - Main HSAIL Register Definition -*- tablegen -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the HSAIL register file, defining the registers themselves,
// aliases between the registers, and the register classes built out of the
// registers.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Register definitions...
//
class HSAILReg<bits<16> num, string n> : Register<n> {
  field bits<16> Value;
  let Value = num;
  let Namespace = "HSAIL";
}


//32-bit registers for signed, unsigned integers or float values
def S0 : HSAILReg<0, "$s0">, DwarfRegNum<[0]>;
def S1 : HSAILReg<1, "$s1">, DwarfRegNum<[1]>;
def S2 : HSAILReg<2, "$s2">, DwarfRegNum<[2]>;
def S3 : HSAILReg<3, "$s3">, DwarfRegNum<[3]>;
def S4 : HSAILReg<4, "$s4">, DwarfRegNum<[4]>;
def S5 : HSAILReg<5, "$s5">, DwarfRegNum<[5]>;
def S6 : HSAILReg<6, "$s6">, DwarfRegNum<[6]>;
def S7 : HSAILReg<7, "$s7">, DwarfRegNum<[7]>;
def S8 : HSAILReg<8, "$s8">, DwarfRegNum<[8]>;
def S9 : HSAILReg<9, "$s9">, DwarfRegNum<[9]>;
def S10 : HSAILReg<10, "$s10">, DwarfRegNum<[10]>;
def S11 : HSAILReg<11, "$s11">, DwarfRegNum<[11]>;
def S12 : HSAILReg<12, "$s12">, DwarfRegNum<[12]>;
def S13 : HSAILReg<13, "$s13">, DwarfRegNum<[13]>;
def S14 : HSAILReg<14, "$s14">, DwarfRegNum<[14]>;
def S15 : HSAILReg<15, "$s15">, DwarfRegNum<[15]>;
def S16 : HSAILReg<16, "$s16">, DwarfRegNum<[16]>;
def S17 : HSAILReg<17, "$s17">, DwarfRegNum<[17]>;
def S18 : HSAILReg<18, "$s18">, DwarfRegNum<[18]>;
def S19 : HSAILReg<19, "$s19">, DwarfRegNum<[19]>;
def S20 : HSAILReg<20, "$s20">, DwarfRegNum<[20]>;
def S21 : HSAILReg<21, "$s21">, DwarfRegNum<[21]>;
def S22 : HSAILReg<22, "$s22">, DwarfRegNum<[22]>;
def S23 : HSAILReg<23, "$s23">, DwarfRegNum<[23]>;
def S24 : HSAILReg<24, "$s24">, DwarfRegNum<[24]>;
def S25 : HSAILReg<25, "$s25">, DwarfRegNum<[25]>;
def S26 : HSAILReg<26, "$s26">, DwarfRegNum<[26]>;
def S27 : HSAILReg<27, "$s27">, DwarfRegNum<[27]>;
def S28 : HSAILReg<28, "$s28">, DwarfRegNum<[28]>;
def S29 : HSAILReg<29, "$s29">, DwarfRegNum<[29]>;
def S30 : HSAILReg<30, "$s30">, DwarfRegNum<[30]>;
def S31 : HSAILReg<31, "$s31">, DwarfRegNum<[31]>;
def S32 : HSAILReg<32, "$s32">, DwarfRegNum<[32]>;
def S33 : HSAILReg<33, "$s33">, DwarfRegNum<[33]>;
def S34 : HSAILReg<34, "$s34">, DwarfRegNum<[34]>;
def S35 : HSAILReg<35, "$s35">, DwarfRegNum<[35]>;
def S36 : HSAILReg<36, "$s36">, DwarfRegNum<[36]>;
def S37 : HSAILReg<37, "$s37">, DwarfRegNum<[37]>;
def S38 : HSAILReg<38, "$s38">, DwarfRegNum<[38]>;
def S39 : HSAILReg<39, "$s39">, DwarfRegNum<[39]>;
def S40 : HSAILReg<40, "$s40">, DwarfRegNum<[40]>;
def S41 : HSAILReg<41, "$s41">, DwarfRegNum<[41]>;
def S42 : HSAILReg<42, "$s42">, DwarfRegNum<[42]>;
def S43 : HSAILReg<43, "$s43">, DwarfRegNum<[43]>;
def S44 : HSAILReg<44, "$s44">, DwarfRegNum<[44]>;
def S45 : HSAILReg<45, "$s45">, DwarfRegNum<[45]>;
def S46 : HSAILReg<46, "$s46">, DwarfRegNum<[46]>;
def S47 : HSAILReg<47, "$s47">, DwarfRegNum<[47]>;
def S48 : HSAILReg<48, "$s48">, DwarfRegNum<[48]>;
def S49 : HSAILReg<49, "$s49">, DwarfRegNum<[49]>;
def S50 : HSAILReg<50, "$s50">, DwarfRegNum<[50]>;
def S51 : HSAILReg<51, "$s51">, DwarfRegNum<[51]>;
def S52 : HSAILReg<52, "$s52">, DwarfRegNum<[52]>;
def S53 : HSAILReg<53, "$s53">, DwarfRegNum<[53]>;
def S54 : HSAILReg<54, "$s54">, DwarfRegNum<[54]>;
def S55 : HSAILReg<55, "$s55">, DwarfRegNum<[55]>;
def S56 : HSAILReg<56, "$s56">, DwarfRegNum<[56]>;
def S57 : HSAILReg<57, "$s57">, DwarfRegNum<[57]>;
def S58 : HSAILReg<58, "$s58">, DwarfRegNum<[58]>;
def S59 : HSAILReg<59, "$s59">, DwarfRegNum<[59]>;
def S60 : HSAILReg<60, "$s60">, DwarfRegNum<[60]>;
def S61 : HSAILReg<61, "$s61">, DwarfRegNum<[61]>;
def S62 : HSAILReg<62, "$s62">, DwarfRegNum<[62]>;
def S63 : HSAILReg<63, "$s63">, DwarfRegNum<[63]>;
def S64 : HSAILReg<64, "$s64">, DwarfRegNum<[64]>;
def S65 : HSAILReg<65, "$s65">, DwarfRegNum<[65]>;
def S66 : HSAILReg<66, "$s66">, DwarfRegNum<[66]>;
def S67 : HSAILReg<67, "$s67">, DwarfRegNum<[67]>;
def S68 : HSAILReg<68, "$s68">, DwarfRegNum<[68]>;
def S69 : HSAILReg<69, "$s69">, DwarfRegNum<[69]>;
def S70 : HSAILReg<70, "$s70">, DwarfRegNum<[70]>;
def S71 : HSAILReg<71, "$s71">, DwarfRegNum<[71]>;
def S72 : HSAILReg<72, "$s72">, DwarfRegNum<[72]>;
def S73 : HSAILReg<73, "$s73">, DwarfRegNum<[73]>;
def S74 : HSAILReg<74, "$s74">, DwarfRegNum<[74]>;
def S75 : HSAILReg<75, "$s75">, DwarfRegNum<[75]>;
def S76 : HSAILReg<76, "$s76">, DwarfRegNum<[76]>;
def S77 : HSAILReg<77, "$s77">, DwarfRegNum<[77]>;
def S78 : HSAILReg<78, "$s78">, DwarfRegNum<[78]>;
def S79 : HSAILReg<79, "$s79">, DwarfRegNum<[79]>;
def S80 : HSAILReg<80, "$s80">, DwarfRegNum<[80]>;
def S81 : HSAILReg<81, "$s81">, DwarfRegNum<[81]>;
def S82 : HSAILReg<82, "$s82">, DwarfRegNum<[82]>;
def S83 : HSAILReg<83, "$s83">, DwarfRegNum<[83]>;
def S84 : HSAILReg<84, "$s84">, DwarfRegNum<[84]>;
def S85 : HSAILReg<85, "$s85">, DwarfRegNum<[85]>;
def S86 : HSAILReg<86, "$s86">, DwarfRegNum<[86]>;
def S87 : HSAILReg<87, "$s87">, DwarfRegNum<[87]>;
def S88 : HSAILReg<88, "$s88">, DwarfRegNum<[88]>;
def S89 : HSAILReg<89, "$s89">, DwarfRegNum<[89]>;
def S90 : HSAILReg<90, "$s90">, DwarfRegNum<[90]>;
def S91 : HSAILReg<91, "$s91">, DwarfRegNum<[91]>;
def S92 : HSAILReg<92, "$s92">, DwarfRegNum<[92]>;
def S93 : HSAILReg<93, "$s93">, DwarfRegNum<[93]>;
def S94 : HSAILReg<94, "$s94">, DwarfRegNum<[94]>;
def S95 : HSAILReg<95, "$s95">, DwarfRegNum<[95]>;
def S96 : HSAILReg<96, "$s96">, DwarfRegNum<[96]>;
def S97 : HSAILReg<97, "$s97">, DwarfRegNum<[97]>;
def S98 : HSAILReg<98, "$s98">, DwarfRegNum<[98]>;
def S99 : HSAILReg<99, "$s99">, DwarfRegNum<[99]>;
def S100 : HSAILReg<100, "$s100">, DwarfRegNum<[100]>;
def S101 : HSAILReg<101, "$s101">, DwarfRegNum<[101]>;
def S102 : HSAILReg<102, "$s102">, DwarfRegNum<[102]>;
def S103 : HSAILReg<103, "$s103">, DwarfRegNum<[103]>;
def S104 : HSAILReg<104, "$s104">, DwarfRegNum<[104]>;
def S105 : HSAILReg<105, "$s105">, DwarfRegNum<[105]>;
def S106 : HSAILReg<106, "$s106">, DwarfRegNum<[106]>;
def S107 : HSAILReg<107, "$s107">, DwarfRegNum<[107]>;
def S108 : HSAILReg<108, "$s108">, DwarfRegNum<[108]>;
def S109 : HSAILReg<109, "$s109">, DwarfRegNum<[109]>;
def S110 : HSAILReg<110, "$s110">, DwarfRegNum<[110]>;
def S111 : HSAILReg<111, "$s111">, DwarfRegNum<[111]>;
def S112 : HSAILReg<112, "$s112">, DwarfRegNum<[112]>;
def S113 : HSAILReg<113, "$s113">, DwarfRegNum<[113]>;
def S114 : HSAILReg<114, "$s114">, DwarfRegNum<[114]>;
def S115 : HSAILReg<115, "$s115">, DwarfRegNum<[115]>;
def S116 : HSAILReg<116, "$s116">, DwarfRegNum<[116]>;
def S117 : HSAILReg<117, "$s117">, DwarfRegNum<[117]>;
def S118 : HSAILReg<118, "$s118">, DwarfRegNum<[118]>;
def S119 : HSAILReg<119, "$s119">, DwarfRegNum<[119]>;
def S120 : HSAILReg<120, "$s120">, DwarfRegNum<[120]>;
def S121 : HSAILReg<121, "$s121">, DwarfRegNum<[121]>;
def S122 : HSAILReg<122, "$s122">, DwarfRegNum<[122]>;
def S123 : HSAILReg<123, "$s123">, DwarfRegNum<[123]>;
def S124 : HSAILReg<124, "$s124">, DwarfRegNum<[124]>;
def S125 : HSAILReg<125, "$s125">, DwarfRegNum<[125]>;
def S126 : HSAILReg<126, "$s126">, DwarfRegNum<[126]>;
def S127 : HSAILReg<127, "$s127">, DwarfRegNum<[127]>;


//64-bit registers for signed, unsigned long integers or double float values
def D0 : HSAILReg<0, "$d0">, DwarfRegNum<[128]>;
def D1 : HSAILReg<1, "$d1">, DwarfRegNum<[129]>;
def D2 : HSAILReg<2, "$d2">, DwarfRegNum<[130]>;
def D3 : HSAILReg<3, "$d3">, DwarfRegNum<[131]>;
def D4 : HSAILReg<4, "$d4">, DwarfRegNum<[132]>;
def D5 : HSAILReg<5, "$d5">, DwarfRegNum<[133]>;
def D6 : HSAILReg<6, "$d6">, DwarfRegNum<[134]>;
def D7 : HSAILReg<7, "$d7">, DwarfRegNum<[135]>;
def D8 : HSAILReg<8, "$d8">, DwarfRegNum<[136]>;
def D9 : HSAILReg<9, "$d9">, DwarfRegNum<[137]>;
def D10 : HSAILReg<10, "$d10">, DwarfRegNum<[138]>;
def D11 : HSAILReg<11, "$d11">, DwarfRegNum<[139]>;
def D12 : HSAILReg<12, "$d12">, DwarfRegNum<[140]>;
def D13 : HSAILReg<13, "$d13">, DwarfRegNum<[141]>;
def D14 : HSAILReg<14, "$d14">, DwarfRegNum<[142]>;
def D15 : HSAILReg<15, "$d15">, DwarfRegNum<[143]>;
def D16 : HSAILReg<16, "$d16">, DwarfRegNum<[144]>;
def D17 : HSAILReg<17, "$d17">, DwarfRegNum<[145]>;
def D18 : HSAILReg<18, "$d18">, DwarfRegNum<[146]>;
def D19 : HSAILReg<19, "$d19">, DwarfRegNum<[147]>;
def D20 : HSAILReg<20, "$d20">, DwarfRegNum<[148]>;
def D21 : HSAILReg<21, "$d21">, DwarfRegNum<[149]>;
def D22 : HSAILReg<22, "$d22">, DwarfRegNum<[150]>;
def D23 : HSAILReg<23, "$d23">, DwarfRegNum<[151]>;
def D24 : HSAILReg<24, "$d24">, DwarfRegNum<[152]>;
def D25 : HSAILReg<25, "$d25">, DwarfRegNum<[153]>;
def D26 : HSAILReg<26, "$d26">, DwarfRegNum<[154]>;
def D27 : HSAILReg<27, "$d27">, DwarfRegNum<[155]>;
def D28 : HSAILReg<28, "$d28">, DwarfRegNum<[156]>;
def D29 : HSAILReg<29, "$d29">, DwarfRegNum<[157]>;
def D30 : HSAILReg<30, "$d30">, DwarfRegNum<[158]>;
def D31 : HSAILReg<31, "$d31">, DwarfRegNum<[159]>;
def D32 : HSAILReg<32, "$d32">, DwarfRegNum<[160]>;
def D33 : HSAILReg<33, "$d33">, DwarfRegNum<[161]>;
def D34 : HSAILReg<34, "$d34">, DwarfRegNum<[162]>;
def D35 : HSAILReg<35, "$d35">, DwarfRegNum<[163]>;
def D36 : HSAILReg<36, "$d36">, DwarfRegNum<[164]>;
def D37 : HSAILReg<37, "$d37">, DwarfRegNum<[165]>;
def D38 : HSAILReg<38, "$d38">, DwarfRegNum<[166]>;
def D39 : HSAILReg<39, "$d39">, DwarfRegNum<[167]>;
def D40 : HSAILReg<40, "$d40">, DwarfRegNum<[168]>;
def D41 : HSAILReg<41, "$d41">, DwarfRegNum<[169]>;
def D42 : HSAILReg<42, "$d42">, DwarfRegNum<[170]>;
def D43 : HSAILReg<43, "$d43">, DwarfRegNum<[171]>;
def D44 : HSAILReg<44, "$d44">, DwarfRegNum<[172]>;
def D45 : HSAILReg<45, "$d45">, DwarfRegNum<[173]>;
def D46 : HSAILReg<46, "$d46">, DwarfRegNum<[174]>;
def D47 : HSAILReg<47, "$d47">, DwarfRegNum<[175]>;
def D48 : HSAILReg<48, "$d48">, DwarfRegNum<[176]>;
def D49 : HSAILReg<49, "$d49">, DwarfRegNum<[177]>;
def D50 : HSAILReg<50, "$d50">, DwarfRegNum<[178]>;
def D51 : HSAILReg<51, "$d51">, DwarfRegNum<[179]>;
def D52 : HSAILReg<52, "$d52">, DwarfRegNum<[180]>;
def D53 : HSAILReg<53, "$d53">, DwarfRegNum<[181]>;
def D54 : HSAILReg<54, "$d54">, DwarfRegNum<[182]>;
def D55 : HSAILReg<55, "$d55">, DwarfRegNum<[183]>;
def D56 : HSAILReg<56, "$d56">, DwarfRegNum<[184]>;
def D57 : HSAILReg<57, "$d57">, DwarfRegNum<[185]>;
def D58 : HSAILReg<58, "$d58">, DwarfRegNum<[186]>;
def D59 : HSAILReg<59, "$d59">, DwarfRegNum<[187]>;
def D60 : HSAILReg<60, "$d60">, DwarfRegNum<[188]>;
def D61 : HSAILReg<61, "$d61">, DwarfRegNum<[189]>;
def D62 : HSAILReg<62, "$d62">, DwarfRegNum<[190]>;
def D63 : HSAILReg<63, "$d63">, DwarfRegNum<[191]>;


//1-bit control registers
def C0 : HSAILReg<0, "$c0">, DwarfRegNum<[192]>;
def C1 : HSAILReg<1, "$c1">, DwarfRegNum<[193]>;
def C2 : HSAILReg<2, "$c2">, DwarfRegNum<[194]>;
def C3 : HSAILReg<3, "$c3">, DwarfRegNum<[195]>;
def C4 : HSAILReg<4, "$c4">, DwarfRegNum<[196]>;
def C5 : HSAILReg<5, "$c5">, DwarfRegNum<[197]>;
def C6 : HSAILReg<6, "$c6">, DwarfRegNum<[198]>;
def C7 : HSAILReg<7, "$c7">, DwarfRegNum<[199]>;


// These registers are used for returning values from a function
// HSA-TODO: have only 1 return return register, and 
// pass the rest as Stack to support vector return
def R0 : HSAILReg<0, "r0">;
def R1 : HSAILReg<1, "r1">;
def R2 : HSAILReg<2, "r2">;
def R3 : HSAILReg<3, "r3">;
def R4 : HSAILReg<4, "r4">;
def R5 : HSAILReg<5, "r5">;
def R6 : HSAILReg<6, "r6">;
def R7 : HSAILReg<7, "r7">;
def R8 : HSAILReg<8, "r8">;
def R9 : HSAILReg<9, "r9">;
def R10 : HSAILReg<10, "r10">;
def R11 : HSAILReg<11, "r11">;
def R12 : HSAILReg<12, "r12">;
def R13 : HSAILReg<13, "r13">;
def R14 : HSAILReg<14, "r14">;
def R15 : HSAILReg<15, "r15">;


// These registers are used for passing agruments to a function
def P0 : HSAILReg<0, "p0">;
def P1 : HSAILReg<1, "p1">;
def P2 : HSAILReg<2, "p2">;
def P3 : HSAILReg<3, "p3">;
def P4 : HSAILReg<4, "p4">;
def P5 : HSAILReg<5, "p5">;
def P6 : HSAILReg<6, "p6">;
def P7 : HSAILReg<7, "p7">;
def P8 : HSAILReg<8, "p8">;
def P9 : HSAILReg<9, "p9">;
def P10 : HSAILReg<10, "p10">;
def P11 : HSAILReg<11, "p11">;
def P12 : HSAILReg<12, "p12">;
def P13 : HSAILReg<13, "p13">;
def P14 : HSAILReg<14, "p14">;
def P15 : HSAILReg<15, "p15">;
def P16 : HSAILReg<16, "p16">;
def P17 : HSAILReg<17, "p17">;
def P18 : HSAILReg<18, "p18">;
def P19 : HSAILReg<19, "p19">;
def P20 : HSAILReg<20, "p20">;
def P21 : HSAILReg<21, "p21">;
def P22 : HSAILReg<22, "p22">;
def P23 : HSAILReg<23, "p23">;
def P24 : HSAILReg<24, "p24">;
def P25 : HSAILReg<25, "p25">;
def P26 : HSAILReg<26, "p26">;
def P27 : HSAILReg<27, "p27">;
def P28 : HSAILReg<28, "p28">;
def P29 : HSAILReg<29, "p29">;
def P30 : HSAILReg<30, "p30">;
def P31 : HSAILReg<31, "p31">;
def P32 : HSAILReg<32, "p32">;



// HSA_TODO: increase the number of registers to handle more parameters and
// and return types. 
def PARAMREG : RegisterClass<"HSAIL", [i1, i32, f32, i64, f64], 64, 
    	       	        (add include "HSAILPARAMREG.td")> {
  let AltOrders = [(add (sequence "P%u", 0, 31))];
  let isAllocatable = 1;
  let AltOrderSelect = [{
          return 0;
        }];
}
			
def RETREG : RegisterClass<"HSAIL", [i1, i32, f32, i64, f64], 64,
                        (add include "HSAILRETREG.td")> {
  let AltOrders = [(add (sequence "R%u", 0, 15))];
  let isAllocatable = 1;
  let AltOrderSelect = [{
          return 0;
        }];
}



//===----------------------------------------------------------------------===//
// HSAILReg Class Definitions... now that we have all of the pieces, define the
// top-level register classes.  The order specified in the register list is
// implicitly defined to be the register allocation order.
//
def GPR32 : RegisterClass<"HSAIL", [i32, f32], 32,
                         (add include "HSAILGPR32.td")> {
  // Reserve a register for spill/local base register and select the alternate 
  // allocation set based on the flat address instruction selection. 
  // Note: Update HSAILRegisterInfo.cpp:getReservedLocalBaseReg if the
  // reserved register is changed.
  let AltOrders = [(add (sequence "S%u", 1, 127))]; // S0 excluded
  let AltOrderSelect = [{
    const TargetMachine &TM = MF.getTarget();
    const HSAILSubtarget &Subtarget = TM.getSubtarget<HSAILSubtarget>();
    return (Subtarget.usesFlatAddr());
  }];
}

def GPR64 : RegisterClass<"HSAIL", [i64, f64], 64,
                         (add include "HSAILGPR64.td")> {
  // Reserve a register for spill/local base register and select the alternate 
  // allocation set based on the flat address instruction selection. 
  // Note: Update HSAILRegisterInfo.cpp:getReservedLocalBaseReg if the
  // reserved register is changed.
  let AltOrders = [(add (sequence "D%u", 1, 63))]; // D0 excluded
  let AltOrderSelect = [{
    const TargetMachine &TM = MF.getTarget();
    const HSAILSubtarget &Subtarget = TM.getSubtarget<HSAILSubtarget>();
    return (Subtarget.usesFlatAddr());
  }];

}

def CR : RegisterClass<"HSAIL", [i1], 32,
                         ( add include "HSAILCR.td")> {
  let Size = 32;
  let AltOrders = [(add (sequence "C%u", 0, 7))];
  let AltOrderSelect = [{
          return 0;
        }];

}
