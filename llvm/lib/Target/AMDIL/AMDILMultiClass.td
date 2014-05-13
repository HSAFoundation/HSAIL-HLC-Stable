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
// Multiclass that handles branch instructions
multiclass Branch<SDNode Op, RegisterClass reg, Operand oper, ValueType type, SDNode imm0> {
  def br: ILFormat<IL_OP_IFC, (outs), (ins brtarget:$target, reg:$src0),
      "; Pseudo branch instruction",
      [(Op bb:$target, (type reg:$src0))]>;
  def bi: ILFormat<IL_OP_IFC, (outs), (ins brtarget:$target, oper:$src0),
      "; Pseudo branch instruction",
      [(Op bb:$target, (type imm0:$src0))]>;
}
multiclass BranchMC<SDNode Op> {
  let isBranch = 1, isPseudo = 1, isCodeGenOnly = 1 in {
    let isBarrier = 1 in {
      def b : ILFormat<IL_PSEUDO_INST, (outs), (ins brtarget:$target),
          "; Pseudo unconditional branch instruction",
          [(br bb:$target)]>;
    }

    defm i8  : Branch<Op, GPRI8,   i8imm,  i8,   imm>;
    defm i16 : Branch<Op, GPRI16, i16imm, i16,   imm>;
    defm i32 : Branch<Op, GPR_32, i32imm, i32,   imm>;
    defm i64 : Branch<Op, GPR_64, i64imm, i64,   imm>;
    defm f32 : Branch<Op, GPR_32, f32imm, f32, fpimm>;
    defm f64 : Branch<Op, GPR_64, f64imm, f64, fpimm>;
  }
}

// Multiclass that handles constant values
multiclass ILConstant<string asm> {
  def i8 : ILFormat<IL_OP_MOV, (outs GPRI8:$dst),
                    (ins i8imm:$val),
                    asm, [(set GPRI8:$dst, imm:$val)]>;

  def v2i8 : ILFormat<IL_OP_MOV, (outs GPRV2I8:$dst),
                      (ins v2i8imm:$val),
                      asm, [(set GPRV2I8:$dst, imm:$val)]>;

  def v4i8 : ILFormat<IL_OP_MOV, (outs GPRV4I8:$dst),
                      (ins v4i8imm:$val),
                      asm, [(set GPRV4I8:$dst, imm:$val)]>;

  def i16 : ILFormat<IL_OP_MOV, (outs GPRI16:$dst),
                     (ins i16imm:$val),
                     asm, [(set GPRI16:$dst, imm:$val)]>;

  def v2i16 : ILFormat<IL_OP_MOV, (outs GPRV2I16:$dst),
                       (ins v2i16imm:$val),
                       asm, [(set GPRV2I16:$dst, imm:$val)]>;

  def v4i16 : ILFormat<IL_OP_MOV, (outs GPRV4I16:$dst),
                       (ins v4i16imm:$val),
                       asm, [(set GPRV4I16:$dst, imm:$val)]>;

  def i32 : ILFormat<IL_OP_MOV, (outs GPR_32:$dst),
                     (ins i32imm:$val),
                     asm, [(set GPR_32:$dst, imm:$val)]>;

  def v2i32 : ILFormat<IL_OP_MOV, (outs GPRV2I32:$dst),
                       (ins v2i32imm:$val),
                        asm, [(set GPRV2I32:$dst, imm:$val)]>;

  def v4i32 : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
                       (ins v4i32imm:$val),
                       asm, [(set GPRV4I32:$dst, imm:$val)]>;

  def i64 : ILFormat<IL_OP_MOV, (outs GPR_64:$dst),
                     (ins i64imm:$val),
                     asm, [(set GPR_64:$dst, imm:$val)]>;

  def v2i64 : ILFormat<IL_OP_MOV, (outs GPRV2I64:$dst),
                       (ins v2i64imm:$val),
                       asm, [(set GPRV2I64:$dst, imm:$val)]>;

  def f32 : ILFormat<IL_OP_MOV, (outs GPR_32:$dst),
                     (ins f32imm:$val),
                     asm, [(set GPR_32:$dst, fpimm:$val)]>;

  def v2f32 : ILFormat<IL_OP_MOV, (outs GPRV2I32:$dst),
                       (ins v2f32imm:$val),
                        asm, [(set GPRV2I32:$dst, fpimm:$val)]>;

  def v4f32 : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
                      (ins v4f32imm:$val),
                       asm, [(set GPRV4I32:$dst, fpimm:$val)]>;

  def f64 : ILFormat<IL_OP_MOV, (outs GPR_64:$dst),
                    (ins f64imm:$val),
                     asm, [(set GPR_64:$dst, fpimm:$val)]>;

  def v2f64 : ILFormat<IL_OP_MOV, (outs GPRV2I64:$dst),
                      (ins v2f64imm:$val),
                       asm, [(set GPRV2I64:$dst, fpimm:$val)]>;
}

multiclass StoreMC<string asm, PatFrag opn,
           RegisterClass reg, Operand oper, ValueType type, SDNode imm0,
           Operand mem, ComplexPattern addr> {
  def r : ILFormat<IL_OP_MOV, (outs), (ins reg:$val, mem:$ptr),
                   !strconcat(asm, " $val $ptr"), [(opn (type reg:$val), addr:$ptr)]>;
  //def i : ILFormat<IL_OP_MOV, (outs), (ins oper:$val, mem:$ptr),
  //!strconcat(asm, " $val $ptr"), [(opn (type imm0:$val), addr:$ptr)]>;
}

multiclass LoadMC<string asm,
                  PatFrag opn,
                  RegisterClass reg,
                  Operand oper,
                  ValueType type,
                  SDNode imm0,
                  Operand mem,
                  ComplexPattern addr> {
  def r : ILFormat<IL_OP_MOV, (outs reg:$dst), (ins mem:$ptr),
                   !strconcat(asm, " $dst $ptr"),
                   [(set (type reg:$dst), (opn addr:$ptr))]>;
}

multiclass GTRUNCSTORE<string asm, Operand mem, ComplexPattern addr> {
  let store = 1, truncate = 1, global = 1 in {
    let sub32bit = 1 in {
      defm i16i8  : StoreMC<asm,  global_i8trunc_store, GPRI16, i16imm, i16, imm, mem, addr>;
      defm i32i8  : StoreMC<asm,  global_i8trunc_store, GPR_32, i32imm, i32, imm, mem, addr>;
      defm i64i8  : StoreMC<asm,  global_i8trunc_store, GPR_64, i64imm, i64, imm, mem, addr>;
      defm i32i16 : StoreMC<asm, global_i16trunc_store, GPR_32, i32imm, i32, imm, mem, addr>;
      defm i64i16 : StoreMC<asm, global_i16trunc_store, GPR_64, i64imm, i64, imm, mem, addr>;
    }
    defm i64i32 : StoreMC<asm, global_i32trunc_store, GPR_64, i64imm, i64,   imm, mem, addr>;
    defm f64f32 : StoreMC<asm, global_f32trunc_store, GPR_64, f64imm, f64, fpimm, mem, addr>;
    let packed = 1 in {
      let sub32bit = 1 in {
        let packType = 0 in {
          defm v2i16i8 : StoreMC<asm, global_v2i8trunc_store, GPRV2I16, v2i16imm, v2i16, imm, mem, addr>;
          defm v2i32i8 : StoreMC<asm, global_v2i8trunc_store, GPRV2I32, v2i32imm, v2i32, imm, mem, addr>;
          defm v2i64i8 : StoreMC<asm, global_v2i8trunc_store, GPRV2I64, v2i64imm, v2i64, imm, mem, addr>;
        }
      }
      let packType = 2 in {
        defm v4i16i8 : StoreMC<asm, global_v4i8trunc_store, GPRV4I16, v4i16imm, v4i16, imm, mem, addr>;
        defm v4i32i8 : StoreMC<asm, global_v4i8trunc_store, GPRV4I32, v4i32imm, v4i32, imm, mem, addr>;
      }
      let packType = 1 in {
        defm v2i32i16 : StoreMC<asm, global_v2i16trunc_store, GPRV2I32, v2i32imm, v2i32, imm, mem, addr>;
        defm v2i64i16 : StoreMC<asm, global_v2i16trunc_store, GPRV2I64, v2i64imm, v2i64, imm, mem, addr>;
      }
      let packType = 3 in {
        defm v4i32i16 : StoreMC<asm, global_v4i16trunc_store, GPRV4I32, v4i32imm, v4i32, imm, mem, addr>;
      }
    }
    defm v2f64f32 : StoreMC<asm, global_v2f32trunc_store, GPRV2I64, v2f64imm, v2f64, fpimm, mem, addr>;
    defm v2i64i32 : StoreMC<asm, global_v2i32trunc_store, GPRV2I64, v2i64imm, v2i64,   imm, mem, addr>;
  }
}

multiclass PTRUNCSTORE<string asm, Operand mem, ComplexPattern addr> {
  let store = 1, truncate = 1, private = 1 in {
    let sub32bit = 1 in {
      defm i16i8  : StoreMC<asm,  private_i8trunc_store, GPRI16, i16imm, i16, imm, mem, addr>;
      defm i32i8  : StoreMC<asm,  private_i8trunc_store, GPR_32, i32imm, i32, imm, mem, addr>;
      defm i64i8  : StoreMC<asm,  private_i8trunc_store, GPR_64, i64imm, i64, imm, mem, addr>;
      defm i32i16 : StoreMC<asm, private_i16trunc_store, GPR_32, i32imm, i32, imm, mem, addr>;
      defm i64i16 : StoreMC<asm, private_i16trunc_store, GPR_64, i64imm, i64, imm, mem, addr>;
    }
    defm i64i32 : StoreMC<asm, private_i32trunc_store, GPR_64, i64imm, i64,   imm, mem, addr>;
    defm f64f32 : StoreMC<asm, private_f32trunc_store, GPR_64, f64imm, f64, fpimm, mem, addr>;
    let packed = 1 in {
      let sub32bit = 1 in {
        let packType = 0 in {
          defm v2i16i8 : StoreMC<asm, private_v2i8trunc_store, GPRV2I16, v2i16imm, v2i16, imm, mem, addr>;
          defm v2i32i8 : StoreMC<asm, private_v2i8trunc_store, GPRV2I32, v2i32imm, v2i32, imm, mem, addr>;
          defm v2i64i8 : StoreMC<asm, private_v2i8trunc_store, GPRV2I64, v2i64imm, v2i64, imm, mem, addr>;
        }
      }
      let packType = 2 in {
        defm v4i16i8 : StoreMC<asm, private_v4i8trunc_store, GPRV4I16, v4i16imm, v4i16, imm, mem, addr>;
        defm v4i32i8 : StoreMC<asm, private_v4i8trunc_store, GPRV4I32, v4i32imm, v4i32, imm, mem, addr>;
      }
      let packType = 1 in {
        defm v2i32i16 : StoreMC<asm, private_v2i16trunc_store, GPRV2I32, v2i32imm, v2i32, imm, mem, addr>;
        defm v2i64i16 : StoreMC<asm, private_v2i16trunc_store, GPRV2I64, v2i64imm, v2i64, imm, mem, addr>;
      }
      let packType = 3 in {
        defm v4i32i16 : StoreMC<asm, private_v4i16trunc_store, GPRV4I32, v4i32imm, v4i32, imm, mem, addr>;
      }
    }
    defm v2f64f32 : StoreMC<asm, private_v2f32trunc_store, GPRV2I64, v2f64imm, v2f64, fpimm, mem, addr>;
    defm v2i64i32 : StoreMC<asm, private_v2i32trunc_store, GPRV2I64, v2i64imm, v2i64,   imm, mem, addr>;
  }
}

multiclass LTRUNCSTORE<string asm, Operand mem, ComplexPattern addr> {
  let store = 1, truncate = 1, local = 1 in {
    let sub32bit = 1 in {
      defm i16i8  : StoreMC<asm,  local_i8trunc_store, GPRI16, i16imm, i16, imm, mem, addr>;
      defm i32i8  : StoreMC<asm,  local_i8trunc_store, GPR_32, i32imm, i32, imm, mem, addr>;
      defm i64i8  : StoreMC<asm,  local_i8trunc_store, GPR_64, i64imm, i64, imm, mem, addr>;
      defm i32i16 : StoreMC<asm, local_i16trunc_store, GPR_32, i32imm, i32, imm, mem, addr>;
      defm i64i16 : StoreMC<asm, local_i16trunc_store, GPR_64, i64imm, i64, imm, mem, addr>;
    }
    defm i64i32 : StoreMC<asm, local_i32trunc_store, GPR_64, i64imm, i64,   imm, mem, addr>;
    defm f64f32 : StoreMC<asm, local_f32trunc_store, GPR_64, f64imm, f64, fpimm, mem, addr>;
    let packed = 1 in {
      let sub32bit = 1 in {
        let packType = 0 in {
          defm v2i16i8 : StoreMC<asm, local_v2i8trunc_store, GPRV2I16, v2i16imm, v2i16, imm, mem, addr>;
          defm v2i32i8 : StoreMC<asm, local_v2i8trunc_store, GPRV2I32, v2i32imm, v2i32, imm, mem, addr>;
          defm v2i64i8 : StoreMC<asm, local_v2i8trunc_store, GPRV2I64, v2i64imm, v2i64, imm, mem, addr>;
        }
      }
      let packType = 2 in {
        defm v4i16i8 : StoreMC<asm, local_v4i8trunc_store, GPRV4I16, v4i16imm, v4i16, imm, mem, addr>;
        defm v4i32i8 : StoreMC<asm, local_v4i8trunc_store, GPRV4I32, v4i32imm, v4i32, imm, mem, addr>;
      }
      let packType = 1 in {
        defm v2i32i16 : StoreMC<asm, local_v2i16trunc_store, GPRV2I32, v2i32imm, v2i32, imm, mem, addr>;
        defm v2i64i16 : StoreMC<asm, local_v2i16trunc_store, GPRV2I64, v2i64imm, v2i64, imm, mem, addr>;
      }
      let packType = 3 in {
        defm v4i32i16 : StoreMC<asm, local_v4i16trunc_store, GPRV4I32, v4i32imm, v4i32, imm, mem, addr>;
      }
    }
    defm v2f64f32 : StoreMC<asm, local_v2f32trunc_store, GPRV2I64, v2f64imm, v2f64, fpimm, mem, addr>;
    defm v2i64i32 : StoreMC<asm, local_v2i32trunc_store, GPRV2I64, v2i64imm, v2i64,   imm, mem, addr>;
  }
}

multiclass RTRUNCSTORE<string asm, Operand mem, ComplexPattern addr> {
  let store = 1, truncate = 1, region = 1 in {
    let sub32bit = 1 in {
      defm i16i8  : StoreMC<asm,  region_i8trunc_store, GPRI16, i16imm, i16, imm, mem, addr>;
      defm i32i8  : StoreMC<asm,  region_i8trunc_store, GPR_32, i32imm, i32, imm, mem, addr>;
      defm i64i8  : StoreMC<asm,  region_i8trunc_store, GPR_64, i64imm, i64, imm, mem, addr>;
      defm i32i16 : StoreMC<asm, region_i16trunc_store, GPR_32, i32imm, i32, imm, mem, addr>;
      defm i64i16 : StoreMC<asm, region_i16trunc_store, GPR_64, i64imm, i64, imm, mem, addr>;
    }
    defm i64i32 : StoreMC<asm, region_i32trunc_store, GPR_64, i64imm, i64,   imm, mem, addr>;
    defm f64f32 : StoreMC<asm, region_f32trunc_store, GPR_64, f64imm, f64, fpimm, mem, addr>;
    let packed = 1 in {
      let sub32bit = 1 in {
        let packType = 0 in {
          defm v2i16i8 : StoreMC<asm, region_v2i8trunc_store, GPRV2I16, v2i16imm, v2i16, imm, mem, addr>;
          defm v2i32i8 : StoreMC<asm, region_v2i8trunc_store, GPRV2I32, v2i32imm, v2i32, imm, mem, addr>;
          defm v2i64i8 : StoreMC<asm, region_v2i8trunc_store, GPRV2I64, v2i64imm, v2i64, imm, mem, addr>;
        }
      }
      let packType = 2 in {
        defm v4i16i8 : StoreMC<asm, region_v4i8trunc_store, GPRV4I16, v4i16imm, v4i16, imm, mem, addr>;
        defm v4i32i8 : StoreMC<asm, region_v4i8trunc_store, GPRV4I32, v4i32imm, v4i32, imm, mem, addr>;
      }
      let packType = 1 in {
        defm v2i32i16 : StoreMC<asm, region_v2i16trunc_store, GPRV2I32, v2i32imm, v2i32, imm, mem, addr>;
        defm v2i64i16 : StoreMC<asm, region_v2i16trunc_store, GPRV2I64, v2i64imm, v2i64, imm, mem, addr>;
      }
      let packType = 3 in {
        defm v4i32i16 : StoreMC<asm, region_v4i16trunc_store, GPRV4I32, v4i32imm, v4i32, imm, mem, addr>;
      }
    }
    defm v2f64f32 : StoreMC<asm, region_v2f32trunc_store, GPRV2I64, v2f64imm, v2f64, fpimm, mem, addr>;
    defm v2i64i32 : StoreMC<asm, region_v2i32trunc_store, GPRV2I64, v2i64imm, v2i64,   imm, mem, addr>;
  }
}

multiclass STORE<string asm, PatFrag opn, Operand mem, ComplexPattern addr> {
  let store = 1 in {
    let sub32bit = 1 in {
      defm i8    : StoreMC<asm, opn,    GPRI8,    i8imm,    i8,   imm, mem, addr>;
      defm i16   : StoreMC<asm, opn,   GPRI16,   i16imm,   i16,   imm, mem, addr>;
    }
    defm i32   : StoreMC<asm, opn,   GPR_32,   i32imm,   i32,   imm, mem, addr>;
    defm v2i32 : StoreMC<asm, opn, GPRV2I32, v2i32imm, v2i32,   imm, mem, addr>;
    defm v4i32 : StoreMC<asm, opn, GPRV4I32, v4i32imm, v4i32,   imm, mem, addr>;
    defm i64   : StoreMC<asm, opn,   GPR_64,   i64imm,   i64,   imm, mem, addr>;
    defm v2i64 : StoreMC<asm, opn, GPRV2I64, v2i64imm, v2i64,   imm, mem, addr>;
    defm f32   : StoreMC<asm, opn,   GPR_32,   f32imm,   f32, fpimm, mem, addr>;
    defm v2f32 : StoreMC<asm, opn, GPRV2I32, v2f32imm, v2f32, fpimm, mem, addr>;
    defm v4f32 : StoreMC<asm, opn, GPRV4I32, v4f32imm, v4f32, fpimm, mem, addr>;
    defm f64   : StoreMC<asm, opn,   GPR_64,   f64imm,   f64, fpimm, mem, addr>;
    defm v2f64 : StoreMC<asm, opn, GPRV2I64, v2f64imm, v2f64, fpimm, mem, addr>;
    let packed = 1 in {
      let sub32bit = 1, packType = 1 in {
        defm v2i16 : StoreMC<asm, opn, GPRV2I16, v2i16imm, v2i16,   imm, mem, addr>;
      }
      let packType = 3 in {
        defm v4i16 : StoreMC<asm, opn, GPRV4I16, v4i16imm, v4i16,   imm, mem, addr>;
      }
      let packType = 0 in {
        defm v2i8  : StoreMC<asm, opn,  GPRV2I8,  v2i8imm,  v2i8,   imm, mem, addr>;
      }
      let packType = 2 in {
        defm v4i8  : StoreMC<asm, opn,  GPRV4I8,  v4i8imm,  v4i8,   imm, mem, addr>;
      }
    }
  }
}

// Multiclass that handles load operations
multiclass LOAD<string asm, PatFrag opn, Operand mem, ComplexPattern addr> {
  let load = 1 in {
    let swsextload = 1 in {
      defm i8  : LoadMC<asm, opn,  GPRI8,  i8imm,  i8, imm, mem, addr>;
      defm i16 : LoadMC<asm, opn, GPRI16, i16imm, i16, imm, mem, addr>;
      let packed = 1 in {
        let packType = 0 in {
          defm v2i8  : LoadMC<asm, opn,  GPRV2I8,  v2i8imm,  v2i8, imm, mem, addr>;
        }
        let packType = 2 in {
          defm v4i8  : LoadMC<asm, opn,  GPRV4I8,  v4i8imm,  v4i8, imm, mem, addr>;
        }
        let packType = 1 in {
          defm v2i16 : LoadMC<asm, opn, GPRV2I16, v2i16imm, v2i16, imm, mem, addr>;
        }
        let packType = 3 in {
          defm v4i16 : LoadMC<asm, opn, GPRV4I16, v4i16imm, v4i16, imm, mem, addr>;
        }
      }
    }
    defm f32   : LoadMC<asm, opn,   GPR_32,   f32imm,   f32, imm, mem, addr>;
    defm i32   : LoadMC<asm, opn,   GPR_32,   i32imm,   i32, imm, mem, addr>;
    defm i64   : LoadMC<asm, opn,   GPR_64,   i64imm,   i64, imm, mem, addr>;
    defm f64   : LoadMC<asm, opn,   GPR_64,   f64imm,   f64, imm, mem, addr>;
    defm v2f32 : LoadMC<asm, opn, GPRV2I32, v2f32imm, v2f32, imm, mem, addr>;
    defm v2i32 : LoadMC<asm, opn, GPRV2I32, v2i32imm, v2i32, imm, mem, addr>;
    defm v2i64 : LoadMC<asm, opn, GPRV2I64, v2i64imm, v2i64, imm, mem, addr>;
    defm v2f64 : LoadMC<asm, opn, GPRV2I64, v2f64imm, v2f64, imm, mem, addr>;
    defm v4f32 : LoadMC<asm, opn, GPRV4I32, v4f32imm, v4f32, imm, mem, addr>;
    defm v4i32 : LoadMC<asm, opn, GPRV4I32, v4i32imm, v4i32, imm, mem, addr>;
  }
}

multiclass UAVRawLSInst<RegisterClass reg, string str> {
  let rawuav = 1 in {
    let mayLoad = 1, load = 1 in {
      def LOADi32 : ILFormat<IL_OP_RAW_UAV_LOAD, (outs GPR_32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_RAW_UAV_LOAD.Text, !strconcat(str, " $dst, $addy")), []>;
      def LOADv2i32 : ILFormat<IL_OP_RAW_UAV_LOAD, (outs GPRV2I32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_RAW_UAV_LOAD.Text, !strconcat(str, " $dst, $addy")), []>;
      def LOADv4i32 : ILFormat<IL_OP_RAW_UAV_LOAD, (outs GPRV4I32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_RAW_UAV_LOAD.Text, !strconcat(str, " $dst, $addy")), []>;
      def LOADCACHEDi32 : ILFormat<IL_OP_RAW_UAV_LOAD, (outs GPR_32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_RAW_UAV_LOAD.Text, !strconcat(str, "_cached $dst, $addy")), []>;
      def LOADCACHEDv2i32 : ILFormat<IL_OP_RAW_UAV_LOAD, (outs GPRV2I32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_RAW_UAV_LOAD.Text, !strconcat(str, "_cached $dst, $addy")), []>;
      def LOADCACHEDv4i32 : ILFormat<IL_OP_RAW_UAV_LOAD, (outs GPRV4I32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_RAW_UAV_LOAD.Text, !strconcat(str, "_cached $dst, $addy")), []>;
      def LOADCACHEDALIGNEDv2i32 : ILFormat<IL_OP_RAW_UAV_LOAD, (outs GPRV2I32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_RAW_UAV_LOAD.Text, !strconcat(str, "_cached_aligned $dst, $addy")), []>;
      def LOADCACHEDALIGNEDv4i32 : ILFormat<IL_OP_RAW_UAV_LOAD, (outs GPRV4I32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_RAW_UAV_LOAD.Text, !strconcat(str, "_cached_aligned $dst, $addy")), []>;
      let Predicates = [IsSIOrLaterDevice] in {
        def LOADi8 : ILFormat<IL_OP_UAV_BYTE_LOAD, (outs GPR_32:$dst),
            (ins reg:$addy, i32imm:$id),
            !strconcat(IL_OP_UAV_BYTE_LOAD.Text, !strconcat(str, " $dst, $addy")), []>;
        def LOADu8 : ILFormat<IL_OP_UAV_UBYTE_LOAD, (outs GPR_32:$dst),
            (ins reg:$addy, i32imm:$id),
            !strconcat(IL_OP_UAV_UBYTE_LOAD.Text, !strconcat(str, " $dst, $addy")), []>;
        def LOADi16 : ILFormat<IL_OP_UAV_SHORT_LOAD, (outs GPR_32:$dst),
            (ins reg:$addy, i32imm:$id),
            !strconcat(IL_OP_UAV_SHORT_LOAD.Text, !strconcat(str, " $dst, $addy")), []>;
        def LOADu16 : ILFormat<IL_OP_UAV_USHORT_LOAD, (outs GPR_32:$dst),
            (ins reg:$addy, i32imm:$id),
            !strconcat(IL_OP_UAV_USHORT_LOAD.Text, !strconcat(str, " $dst, $addy")), []>;
        def LOADCACHEDi8 : ILFormat<IL_OP_UAV_BYTE_LOAD, (outs GPR_32:$dst),
            (ins reg:$addy, i32imm:$id),
            !strconcat(IL_OP_UAV_BYTE_LOAD.Text, !strconcat(str, "_cached $dst, $addy")), []>;
        def LOADCACHEDu8 : ILFormat<IL_OP_UAV_UBYTE_LOAD, (outs GPR_32:$dst),
            (ins reg:$addy, i32imm:$id),
            !strconcat(IL_OP_UAV_UBYTE_LOAD.Text, !strconcat(str, "_cached $dst, $addy")), []>;
        def LOADCACHEDi16 : ILFormat<IL_OP_UAV_SHORT_LOAD, (outs GPR_32:$dst),
            (ins reg:$addy, i32imm:$id),
            !strconcat(IL_OP_UAV_SHORT_LOAD.Text, !strconcat(str, "_cached $dst, $addy")), []>;
        def LOADCACHEDu16 : ILFormat<IL_OP_UAV_USHORT_LOAD, (outs GPR_32:$dst),
            (ins reg:$addy, i32imm:$id),
            !strconcat(IL_OP_UAV_USHORT_LOAD.Text, !strconcat(str, "_cached $dst, $addy")), []>;
      }
    }
    let mayStore = 1, store = 1 in {
      def STOREi32 : ILFormat<IL_OP_RAW_UAV_STORE, (outs GPR_32:$mem),
          (ins reg:$addy, GPR_32:$src, i32imm:$id),
          !strconcat(IL_OP_RAW_UAV_STORE.Text, !strconcat(str, " $mem, $addy, $src")), []>;
      def STOREv2i32 : ILFormat<IL_OP_RAW_UAV_STORE, (outs GPRV2I32:$mem),
          (ins reg:$addy, GPRV2I32:$src, i32imm:$id),
          !strconcat(IL_OP_RAW_UAV_STORE.Text, !strconcat(str, " $mem, $addy, $src")), []>;
      def STOREv4i32 : ILFormat<IL_OP_RAW_UAV_STORE, (outs GPRV4I32:$mem),
          (ins reg:$addy, GPRV4I32:$src, i32imm:$id),
          !strconcat(IL_OP_RAW_UAV_STORE.Text, !strconcat(str, " $mem, $addy, $src")), []>;
      let Predicates = [IsSIOrLaterDevice] in {
        def STOREi8 : ILFormat<IL_OP_UAV_BYTE_STORE, (outs GPR_32:$mem),
            (ins reg:$addy, GPR_32:$src, i32imm:$id),
            !strconcat(IL_OP_UAV_BYTE_STORE.Text, !strconcat(str, " $mem, $addy, $src")), []>;
        def STOREi16 : ILFormat<IL_OP_UAV_SHORT_STORE, (outs GPR_32:$mem),
            (ins reg:$addy, GPR_32:$src, i32imm:$id),
            !strconcat(IL_OP_UAV_SHORT_STORE.Text, !strconcat(str, " $mem, $addy, $src")), []>;

      }
    }
  }
}

multiclass ScratchLSInst<RegisterClass reg> {
  let scratch = 1 in {
    let mayLoad = 1, load = 1 in {
      def LOAD : ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_MOV.Text, " $dst, x$id[$addy]"), []>;
    }
    let mayStore = 1, store = 1 in {
      def STORE : ILFormat<IL_OP_MOV, (outs),
          (ins reg:$addy, GPRV4I32:$data, i32imm:$id),
          !strconcat(IL_OP_MOV.Text, " x$id[$addy, $data"), []>;
    }
  }
}

multiclass ArenaLSInst<RegisterClass reg> {
  let arenauav = 1 in {
    let mayStore = 1, store = 1 in {
      def STOREi8 : ILFormat<IL_OP_ARENA_UAV_STORE, (outs reg:$addy),
          (ins GPRI8:$src, i32imm:$id),
          !strconcat(IL_OP_ARENA_UAV_STORE.Text,
              "_id($id)_size(byte) $addy, $src"), []>;
      def STOREi16 : ILFormat<IL_OP_ARENA_UAV_STORE, (outs reg:$addy),
          (ins GPRI16:$src, i32imm:$id),
          !strconcat(IL_OP_ARENA_UAV_STORE.Text,
              "_id($id)_size(short) $addy, $src"), []>;
      def STOREi32 : ILFormat<IL_OP_ARENA_UAV_STORE, (outs reg:$addy),
          (ins GPR_32:$src, i32imm:$id),
          !strconcat(IL_OP_ARENA_UAV_STORE.Text,
              "_id($id)_size(dword) $addy, $src"), []>;
    }
    let mayLoad = 1, load = 1 in {
      def LOADi8 : ILFormat<IL_OP_ARENA_UAV_LOAD, (outs GPRI8:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_ARENA_UAV_LOAD.Text, "_id($id)_size(byte) $dst, $addy"), []>;
      def LOADi16 : ILFormat<IL_OP_ARENA_UAV_LOAD, (outs GPRI16:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_ARENA_UAV_LOAD.Text, "_id($id)_size(short) $dst, $addy"), []>;
      def LOADi32 : ILFormat<IL_OP_ARENA_UAV_LOAD, (outs GPR_32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_ARENA_UAV_LOAD.Text, "_id($id)_size(dword) $dst, $addy"), []>;
    }
  }
}

multiclass LDSLSInst<RegisterClass reg> {
  let lds = 1 in {
    let mayStore = 1, store = 1 in {
      def STOREi8r : ILFormat<IL_OP_LDS_STORE_BYTE, (outs),
          (ins reg:$addy, GPRI8:$src, i32imm:$id),
          !strconcat(IL_OP_LDS_STORE_BYTE.Text, "_id($id) $addy, $src"), []>;
      def STOREi16r : ILFormat<IL_OP_LDS_STORE_SHORT, (outs),
          (ins reg:$addy, GPRI16:$src, i32imm:$id),
          !strconcat(IL_OP_LDS_STORE_SHORT.Text, "_id($id) $addy, $src"), []>;
      def STOREi32r : ILFormat<IL_OP_LDS_STORE, (outs),
          (ins reg:$addy, GPR_32:$src, i32imm:$id),
          !strconcat(IL_OP_LDS_STORE.Text, "_id($id) $addy, $src"), []>;
      def STOREv2i32r : ILFormat<IL_OP_LDS_STORE_VEC, (outs GPR_32:$mem),
          (ins reg:$addy, GPRV4I32:$src, i32imm:$id),
          !strconcat(IL_OP_LDS_STORE_VEC.Text, "_id($id) $mem, $addy, $src, $src"), []>;
      def STOREv4i32r : ILFormat<IL_OP_LDS_STORE_VEC, (outs GPR_32:$mem),
          (ins reg:$addy, GPRV4I32:$src, i32imm:$id),
          !strconcat(IL_OP_LDS_STORE_VEC.Text, "_id($id) $mem, $addy, $src, $src"), []>;
    }
    let mayLoad = 1, load = 1 in {
      def LOADu8r   : ILFormat<IL_OP_LDS_LOAD_UBYTE, (outs GPRI8:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_LDS_LOAD_UBYTE.Text, "_id($id) $dst, $addy"), []>;
      def LOADi8r   : ILFormat<IL_OP_LDS_LOAD_BYTE, (outs GPRI8:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_LDS_LOAD_BYTE.Text, "_id($id) $dst, $addy"), []>;
      def LOADu16r  : ILFormat<IL_OP_LDS_LOAD_USHORT, (outs GPRI16:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_LDS_LOAD_USHORT.Text, "_id($id) $dst, $addy"), []>;
      def LOADi16r  : ILFormat<IL_OP_LDS_LOAD_SHORT, (outs GPRI16:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_LDS_LOAD_SHORT.Text, "_id($id) $dst, $addy"), []>;
      def LOADi32r  : ILFormat<IL_OP_LDS_LOAD, (outs GPR_32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_LDS_LOAD.Text, "_id($id) $dst, $addy"), []>;
      def LOADv2i32r: ILFormat<IL_OP_LDS_LOAD_VEC, (outs GPRV2I32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_LDS_LOAD_VEC.Text, "_id($id) $dst, $addy, $addy"), []>;
      def LOADv4i32r: ILFormat<IL_OP_LDS_LOAD_VEC, (outs GPRV4I32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_LDS_LOAD_VEC.Text, "_id($id) $dst, $addy, $addy"), []>;
    }
  }
}

multiclass GDSLSInst<RegisterClass reg> {
  let gds = 1 in {
    let mayStore = 1, store = 1 in {
      def STOREi8r  : ILFormat<IL_OP_GDS_STORE_BYTE, (outs),
          (ins reg:$addy, GPRI8:$src,  i32imm:$id),
          !strconcat(IL_OP_GDS_STORE_BYTE.Text, "_id($id) $addy, $src"), []>;
      def STOREi16r : ILFormat<IL_OP_GDS_STORE_SHORT, (outs),
          (ins reg:$addy, GPRI16:$src, i32imm:$id),
          !strconcat(IL_OP_GDS_STORE_SHORT.Text, "_id($id) $addy, $src"), []>;
      def STOREi32r : ILFormat<IL_OP_GDS_STORE, (outs),
          (ins reg:$addy, GPR_32:$src, i32imm:$id),
          !strconcat(IL_OP_GDS_STORE.Text, "_id($id) $addy, $src"), []>;
    }
    let mayLoad = 1, load = 1 in {
      def LOADu8r   : ILFormat<IL_OP_GDS_LOAD_UBYTE, (outs GPRI8:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_GDS_LOAD_UBYTE.Text, "_id($id) $dst, $addy"), []>;
      def LOADi8r   : ILFormat<IL_OP_GDS_LOAD_BYTE, (outs GPRI16:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_GDS_LOAD_BYTE.Text, "_id($id) $dst, $addy"), []>;
      def LOADu16r  : ILFormat<IL_OP_GDS_LOAD_USHORT, (outs GPR_32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_GDS_LOAD_USHORT.Text, "_id($id) $dst, $addy"), []>;
      def LOADi16r  : ILFormat<IL_OP_GDS_LOAD_SHORT, (outs GPR_32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_GDS_LOAD_SHORT.Text, "_id($id) $dst, $addy"), []>;
      def LOADi32r  : ILFormat<IL_OP_GDS_LOAD, (outs GPR_32:$dst),
          (ins reg:$addy, i32imm:$id),
          !strconcat(IL_OP_GDS_LOAD.Text, "_id($id) $dst, $addy"), []>;
    }
  }
}
multiclass IMAGEINFO0<Operand mem, ComplexPattern addr> {
  let image = 1, info = 1 in {
    def I1D: ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins mem:$ptr),
        !strconcat(IL_OP_MOV.Text, " $dst, $ptr"),
        [(set GPRV4I32:$dst, (int_AMDIL_image1d_info0 addr:$ptr))]>;
    def I1DA: ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins mem:$ptr),
        !strconcat(IL_OP_MOV.Text, " $dst, $ptr"),
        [(set GPRV4I32:$dst, (int_AMDIL_image1d_array_info0 addr:$ptr))]>;
    def I1DB: ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins mem:$ptr),
        !strconcat(IL_OP_MOV.Text, " $dst, $ptr"),
        [(set GPRV4I32:$dst, (int_AMDIL_image1d_buffer_info0 addr:$ptr))]>;
    def I2D: ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins mem:$ptr),
        !strconcat(IL_OP_MOV.Text, " $dst, $ptr"),
        [(set GPRV4I32:$dst, (int_AMDIL_image2d_info0 addr:$ptr))]>;
    def I2DA: ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins mem:$ptr),
        !strconcat(IL_OP_MOV.Text, " $dst, $ptr"),
        [(set GPRV4I32:$dst, (int_AMDIL_image2d_array_info0 addr:$ptr))]>;
    def I3D: ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins mem:$ptr),
        !strconcat(IL_OP_MOV.Text, " $dst, $ptr"),
        [(set GPRV4I32:$dst, (int_AMDIL_image3d_info0 addr:$ptr))]>;
  }
}
multiclass IMAGEINFO1<Operand mem, ComplexPattern addr> {
  let image = 1, info = 2 in {
    def I1D: ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins mem:$ptr),
        !strconcat(IL_OP_MOV.Text, " $dst, $ptr"),
        [(set GPRV4I32:$dst, (int_AMDIL_image1d_info1 addr:$ptr))]>;
    def I1DA: ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins mem:$ptr),
        !strconcat(IL_OP_MOV.Text, " $dst, $ptr"),
        [(set GPRV4I32:$dst, (int_AMDIL_image1d_array_info1 addr:$ptr))]>;
    def I1DB: ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins mem:$ptr),
        !strconcat(IL_OP_MOV.Text, " $dst, $ptr"),
        [(set GPRV4I32:$dst, (int_AMDIL_image1d_buffer_info1 addr:$ptr))]>;
    def I2D: ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins mem:$ptr),
        !strconcat(IL_OP_MOV.Text, " $dst, $ptr"),
        [(set GPRV4I32:$dst, (int_AMDIL_image2d_info1 addr:$ptr))]>;
    def I2DA: ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins mem:$ptr),
        !strconcat(IL_OP_MOV.Text, " $dst, $ptr"),
        [(set GPRV4I32:$dst, (int_AMDIL_image2d_array_info1 addr:$ptr))]>;
    def I3D: ILFormat<IL_OP_MOV, (outs GPRV4I32:$dst),
        (ins mem:$ptr),
        !strconcat(IL_OP_MOV.Text, " $dst, $ptr"),
        [(set GPRV4I32:$dst, (int_AMDIL_image3d_info1 addr:$ptr))]>;
  }
}
multiclass IMAGEREAD<Operand mem, ComplexPattern addr> {
  let image = 1, load = 1 in {
    def I1D : ILFormat<IL_OP_SAMPLE, (outs GPRV4I32:$dst),
        (ins mem:$ptr, GPR_32:$sampler, GPRV4I32:$addy),
        !strconcat(IL_OP_SAMPLE.Text,
            "_id($ptr)_sampler($sampler)_coordtype(normalized) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image1d_read_norm addr:$ptr, GPR_32:$sampler, GPRV4I32:$addy))]>;
    def I1DA : ILFormat<IL_OP_SAMPLE, (outs GPRV4I32:$dst),
        (ins mem:$ptr, GPR_32:$sampler, GPRV4I32:$addy),
        !strconcat(IL_OP_SAMPLE.Text,
            "_id($ptr)_sampler($sampler)_coordtype(normalized) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image1d_array_read_norm addr:$ptr, GPR_32:$sampler, GPRV4I32:$addy))]>;
    def I1DB : ILFormat<IL_OP_SAMPLE, (outs GPRV4I32:$dst),
        (ins mem:$ptr, GPR_32:$sampler, GPRV4I32:$addy),
        !strconcat(IL_OP_SAMPLE.Text,
            "_id($ptr)_sampler($sampler)_coordtype(normalized) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image1d_buffer_read_norm addr:$ptr, GPR_32:$sampler, GPRV4I32:$addy))]>;
    def I2D : ILFormat<IL_OP_SAMPLE, (outs GPRV4I32:$dst),
        (ins mem:$ptr, GPR_32:$sampler, GPRV4I32:$addy),
        !strconcat(IL_OP_SAMPLE.Text,
            "_id($ptr)_sampler($sampler)_coordtype(normalized) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image2d_read_norm addr:$ptr, GPR_32:$sampler, GPRV4I32:$addy))]>;
    def I2DA : ILFormat<IL_OP_SAMPLE, (outs GPRV4I32:$dst),
        (ins mem:$ptr, GPR_32:$sampler, GPRV4I32:$addy),
        !strconcat(IL_OP_SAMPLE.Text,
            "_id($ptr)_sampler($sampler)_coordtype(normalized) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image2d_array_read_norm addr:$ptr, GPR_32:$sampler, GPRV4I32:$addy))]>;
    def I3D : ILFormat<IL_OP_SAMPLE, (outs GPRV4I32:$dst),
        (ins mem:$ptr, GPR_32:$sampler, GPRV4I32:$addy),
        !strconcat(IL_OP_SAMPLE.Text,
            "_id($ptr)_sampler($sampler)_coordtype(normalized) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image3d_read_norm addr:$ptr, GPR_32:$sampler, GPRV4I32:$addy))]>;
    def I1D_UNNORM : ILFormat<IL_OP_SAMPLE, (outs GPRV4I32:$dst),
        (ins mem:$ptr, GPR_32:$sampler, GPRV4I32:$addy),
        !strconcat(IL_OP_SAMPLE.Text,
            "_id($ptr)_sampler($sampler)_coordtype(unnormalized) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image1d_read_unnorm addr:$ptr, GPR_32:$sampler, GPRV4I32:$addy))]>;
    def I1DA_UNNORM : ILFormat<IL_OP_SAMPLE, (outs GPRV4I32:$dst),
        (ins mem:$ptr, GPR_32:$sampler, GPRV4I32:$addy),
        !strconcat(IL_OP_SAMPLE.Text,
            "_id($ptr)_sampler($sampler)_coordtype(unnormalized) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image1d_array_read_unnorm addr:$ptr, GPR_32:$sampler, GPRV4I32:$addy))]>;
    def I1DB_UNNORM : ILFormat<IL_OP_SAMPLE, (outs GPRV4I32:$dst),
        (ins mem:$ptr, GPR_32:$sampler, GPRV4I32:$addy),
        !strconcat(IL_OP_SAMPLE.Text,
            "_id($ptr)_sampler($sampler)_coordtype(unnormalized) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image1d_buffer_read_unnorm addr:$ptr, GPR_32:$sampler, GPRV4I32:$addy))]>;
    def I2D_UNNORM : ILFormat<IL_OP_SAMPLE, (outs GPRV4I32:$dst),
        (ins mem:$ptr, GPR_32:$sampler, GPRV4I32:$addy),
        !strconcat(IL_OP_SAMPLE.Text,
            "_id($ptr)_sampler($sampler)_coordtype(unnormalized) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image2d_read_unnorm addr:$ptr, GPR_32:$sampler, GPRV4I32:$addy))]>;
    def I2DA_UNNORM : ILFormat<IL_OP_SAMPLE, (outs GPRV4I32:$dst),
        (ins mem:$ptr, GPR_32:$sampler, GPRV4I32:$addy),
        !strconcat(IL_OP_SAMPLE.Text,
            "_id($ptr)_sampler($sampler)_coordtype(unnormalized) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image2d_array_read_unnorm addr:$ptr, GPR_32:$sampler, GPRV4I32:$addy))]>;
    def I3D_UNNORM : ILFormat<IL_OP_SAMPLE, (outs GPRV4I32:$dst),
        (ins mem:$ptr, GPR_32:$sampler, GPRV4I32:$addy),
        !strconcat(IL_OP_SAMPLE.Text,
            "_id($ptr)_sampler($sampler)_coordtype(unnormalized) $dst, $addy"),
        [(set GPRV4I32:$dst,
            (int_AMDIL_image3d_read_unnorm addr:$ptr, GPR_32:$sampler, GPRV4I32:$addy))]>;
  }
}
multiclass IMAGEWRITE<Operand mem, ComplexPattern addr> {
  let image = 1, store = 1 in {
    def I1D: ILFormat<IL_OP_UAV_STORE, (outs),
        (ins mem:$ptr, GPRV2I32:$addy, GPRV4I32:$data),
        !strconcat(IL_OP_UAV_STORE.Text,
            "_id($ptr) $addy, $data"),
        [(int_AMDIL_image1d_write addr:$ptr, GPRV2I32:$addy, GPRV4I32:$data)]>;
    def I1DA: ILFormat<IL_OP_UAV_STORE, (outs),
        (ins mem:$ptr, GPRV2I32:$addy, GPRV4I32:$data),
        !strconcat(IL_OP_UAV_STORE.Text,
            "_id($ptr) $addy, $data"),
        [(int_AMDIL_image1d_array_write addr:$ptr, GPRV2I32:$addy, GPRV4I32:$data)]>;
    def I1DB: ILFormat<IL_OP_UAV_STORE, (outs),
        (ins mem:$ptr, GPRV2I32:$addy, GPRV4I32:$data),
        !strconcat(IL_OP_UAV_STORE.Text,
            "_id($ptr) $addy, $data"),
        [(int_AMDIL_image1d_buffer_write addr:$ptr, GPRV2I32:$addy, GPRV4I32:$data)]>;
    def I2D: ILFormat<IL_OP_UAV_STORE, (outs),
        (ins mem:$ptr, GPRV2I32:$addy, GPRV4I32:$data),
        !strconcat(IL_OP_UAV_STORE.Text,
            "_id($ptr) $addy, $data"),
        [(int_AMDIL_image2d_write addr:$ptr, GPRV2I32:$addy, GPRV4I32:$data)]>;
    def I2DA: ILFormat<IL_OP_UAV_STORE, (outs),
        (ins mem:$ptr, GPRV4I32:$addy, GPRV4I32:$data),
        !strconcat(IL_OP_UAV_STORE.Text,
            "_id($ptr) $addy, $data"),
        [(int_AMDIL_image2d_array_write addr:$ptr, GPRV4I32:$addy, GPRV4I32:$data)]>;
    def I3D: ILFormat<IL_OP_UAV_STORE, (outs),
        (ins mem:$ptr, GPRV4I32:$addy, GPRV4I32:$data),
        !strconcat(IL_OP_UAV_STORE.Text,
            "_id($ptr) $addy, $data"),
        [(int_AMDIL_image3d_write addr:$ptr, GPRV4I32:$addy, GPRV4I32:$data)]>;
  }
}

// Only scalar types should generate flow control
multiclass BranchInstr<ILOpCode opc> {
  def i8r : ILFormat<opc, (outs), (ins GPRI8:$src0),
      !strconcat(opc.Text, " $src0"), []>;
  def i16r : ILFormat<opc, (outs), (ins GPRI16:$src0),
      !strconcat(opc.Text, " $src0"), []>;
  def i32r : ILFormat<opc, (outs), (ins GPR_32:$src0),
      !strconcat(opc.Text, " $src0"), []>;
  def i32i : ILFormat<opc, (outs), (ins i32imm:$src0),
      !strconcat(opc.Text, " $src0"), []>;
  def f32r : ILFormat<opc, (outs), (ins GPR_32:$src0),
      !strconcat(opc.Text, " $src0"), []>;
  let swizzle = 2 in {
    def i64r : ILFormat<opc, (outs), (ins GPR_64:$src0),
        !strconcat(opc.Text, " $src0"), []>;
    def f64r : ILFormat<opc, (outs), (ins GPR_64:$src0),
        !strconcat(opc.Text, " $src0"), []>;
  }
}
// Only scalar types should generate flow control
multiclass BranchInstr2<ILOpCode opc> {
  def i8rr : ILFormat<opc, (outs),  (ins  GPRI8:$src0,  GPRI8:$src1),
      !strconcat(opc.Text, " $src0, $src1"), []>;
  def i16rr : ILFormat<opc, (outs), (ins GPRI16:$src0, GPRI16:$src1),
      !strconcat(opc.Text, " $src0, $src1"), []>;
  def i32rr : ILFormat<opc, (outs), (ins GPR_32:$src0, GPR_32:$src1),
      !strconcat(opc.Text, " $src0, $src1"), []>;
  def f32rr : ILFormat<opc, (outs), (ins GPR_32:$src0, GPR_32:$src1),
      !strconcat(opc.Text, " $src0, $src1"), []>;
  def i64rr : ILFormat<opc, (outs), (ins GPR_64:$src0, GPR_64:$src1),
      !strconcat(opc.Text, " $src0, $src1"), []>;
  def f64rr : ILFormat<opc, (outs), (ins GPR_64:$src0, GPR_64:$src1),
      !strconcat(opc.Text, " $src0, $src1"), []>;
}
//===--------------------------------------------------------------------===//
// The base class for vector extract instructions. The vector extract
// instructions take as an input value a source register and a 32bit integer
// with the same encoding as specified in InsertVectorClass and produces
// a result with only the swizzled component in the destination register.
//===--------------------------------------------------------------------===//

multiclass ExtractVectorClass<SDNode opn, RegisterClass dreg,
                              ValueType dtype, RegisterClass reg,
                              Operand oper, ValueType type,
                              SDNode imm0> {
  let vector = 1 in {
  def r : ILFormat<IL_OP_MOV, (outs dreg:$dst),
  (ins reg:$src, i32imm:$src1),
  !strconcat(IL_OP_MOV.Text, " $dst, $src"),
      [(set (dtype dreg:$dst), (opn (type reg:$src), timm:$src1))]>;
  def i : ILFormat<IL_OP_MOV, (outs dreg:$dst),
  (ins oper:$src, i32imm:$src1),
  !strconcat(IL_OP_MOV.Text, " $dst, $src"),
      [(set (dtype dreg:$dst), (opn (type imm0:$src), timm:$src1))]>;
  }
}

// Class that handles the various vector extract patterns
multiclass VectorExtract<SDNode opn> {
  defm v2f64 : ExtractVectorClass<opn, GPR_64, f64, GPRV2I64, v2f64imm, v2f64, fpimm>;
  defm v4f32 : ExtractVectorClass<opn, GPR_32, f32, GPRV4I32, v4f32imm, v4f32, fpimm>;
  defm v2f32 : ExtractVectorClass<opn, GPR_32, f32, GPRV2I32, v2f32imm, v2f32, fpimm>;
  defm v2i64 : ExtractVectorClass<opn, GPR_64, i64, GPRV2I64, v2i64imm, v2i64,   imm>;
  defm v4i8  : ExtractVectorClass<opn, GPRI8,   i8,  GPRV4I8,  v4i8imm,  v4i8,   imm>;
  defm v4i16 : ExtractVectorClass<opn, GPRI16, i16, GPRV4I16, v4i16imm, v4i16,   imm>;
  defm v4i32 : ExtractVectorClass<opn, GPR_32, i32, GPRV4I32, v4i32imm, v4i32,   imm>;
  defm v2i8  : ExtractVectorClass<opn, GPRI8,   i8,  GPRV2I8,  v2i8imm,  v2i8,   imm>;
  defm v2i16 : ExtractVectorClass<opn, GPRI16, i16, GPRV2I16, v2i16imm, v2i16,   imm>;
  defm v2i32 : ExtractVectorClass<opn, GPR_32, i32, GPRV2I32, v2i32imm, v2i32,   imm>;
}
//===--------------------------------------------------------------------===//
// The base class for vector concatenation. This class creates either a vec2
// or a vec4 of 32bit data types or a vec2 of 64bit data types. This is done
// by swizzling either the 'x' or 'xy' components of the source operands
// into the destination register.
//===--------------------------------------------------------------------===//

multiclass VectorConcatClass<SDNode opn, RegisterClass dreg,
                             ValueType dtype, RegisterClass sreg,
                             Operand soper, ValueType stype,
                             SDNode simm> {
  let vector = 1 in {
    def rr : ILFormat<IL_OP_I_ADD, (outs dreg:$dst), (ins sreg:$src0, sreg:$src1),
      !strconcat(IL_OP_I_ADD.Text, " $dst, $src0, $src1"),
      [(set (dtype dreg:$dst), (opn (stype sreg:$src0), (stype sreg:$src1)))]>;
    def ir : ILFormat<IL_OP_I_ADD, (outs dreg:$dst), (ins soper:$src0, sreg:$src1),
      !strconcat(IL_OP_I_ADD.Text, " $dst, $src0, $src1"),
      [(set (dtype dreg:$dst), (opn (stype simm:$src0), (stype sreg:$src1)))]>;
    def ri : ILFormat<IL_OP_I_ADD, (outs dreg:$dst), (ins sreg:$src0, soper:$src1),
      !strconcat(IL_OP_I_ADD.Text, " $dst, $src0, $src1"),
      [(set (dtype dreg:$dst), (opn (stype sreg:$src0), (stype simm:$src1)))]>;
    def ii : ILFormat<IL_OP_I_ADD, (outs dreg:$dst), (ins soper:$src0, soper:$src1),
      !strconcat(IL_OP_I_ADD.Text, " $dst, $src0, $src1"),
      [(set (dtype dreg:$dst), (opn (stype simm:$src0), (stype simm:$src1)))]>;
  }
}

multiclass VectorConcat<SDNode opn> {
  defm v2f64 : VectorConcatClass<opn, GPRV2I64, v2f64,   GPR_64,   f64imm,   f64, fpimm>;
  defm v2i64 : VectorConcatClass<opn, GPRV2I64, v2i64,   GPR_64,   i64imm,   i64,   imm>;
  defm v4f32 : VectorConcatClass<opn, GPRV4I32, v4f32, GPRV2I32, v2f32imm, v2f32, fpimm>;
  defm v4i32 : VectorConcatClass<opn, GPRV4I32, v4i32, GPRV2I32, v2i32imm, v2i32,   imm>;
  defm v4i16 : VectorConcatClass<opn, GPRV4I16, v4i16, GPRV2I16, v2i16imm, v2i16,   imm>;
  defm v4i8  : VectorConcatClass<opn, GPRV4I8,   v4i8,  GPRV2I8,  v2i8imm,  v2i8,   imm>;
  defm v2f32 : VectorConcatClass<opn, GPRV2I32, v2f32,   GPR_32,   f32imm,   f32, fpimm>;
  defm v2i32 : VectorConcatClass<opn, GPRV2I32, v2i32,   GPR_32,   i32imm,   i32,   imm>;
  defm v2i16 : VectorConcatClass<opn, GPRV2I16, v2i16,   GPRI16,   i16imm,   i16,   imm>;
  defm v2i8  : VectorConcatClass<opn, GPRV2I8,   v2i8,    GPRI8,    i8imm,    i8,   imm>;
}
//===--------------------------------------------------------------------===//
// The base class for vector insert instructions. It is a single dest, quad
// source instruction where the last two source operands must be 32bit
// immediate values that are encoding the swizzle of the source register
// The src2 and src3 instructions must also be inversion of each other such
// that if src2 is 0x1000300(x0z0), src3 must be 0x20004(0y0w). The values
// are encoded as 32bit integer with each 8 char representing a swizzle value.
// The encoding is as follows for 32bit register types:
// 0x00 -> '_'
// 0x01 -> 'x'
// 0x02 -> 'y'
// 0x03 -> 'z'
// 0x04 -> 'w'
// 0x05 -> 'x'
// 0x06 -> 'y'
// 0x07 -> 'z'
// 0x08 -> 'w'
// 0x09 -> '0'
// The encoding is as follows for 64bit register types:
// 0x00 -> "__"
// 0x01 -> "xy"
// 0x02 -> "zw"
// 0x03 -> "xy"
// 0x04 -> "zw"
// 0x05 -> "00"
//===--------------------------------------------------------------------===//

multiclass InsertVectorClass<SDNode opn,
                             RegisterClass sreg,
                             Operand soper,
                             ValueType stype,
                             SDNode simm,
                             RegisterClass sreg1,
                             Operand soper1,
                             ValueType stype1,
                             SDNode simm1> {
  let vector = 1 in {
    def rr : ILFormat<IL_OP_I_ADD, (outs sreg:$dst), (ins sreg:$src0, sreg1:$src1, i32imm:$src2, i32imm:$src3),
      !strconcat(IL_OP_I_ADD.Text, " $dst, $src0, $src1"),
      [(set (stype sreg:$dst), (opn (stype sreg:$src0), (stype1 sreg1:$src1), timm:$src2, timm:$src3))]>;
    def ir : ILFormat<IL_OP_I_ADD, (outs sreg:$dst), (ins soper:$src0, sreg1:$src1, i32imm:$src2, i32imm:$src3),
      !strconcat(IL_OP_I_ADD.Text, " $dst, $src0, $src1"),
      [(set (stype sreg:$dst), (opn (stype simm:$src0), (stype1 sreg1:$src1), timm:$src2, timm:$src3))]>;
    def ri : ILFormat<IL_OP_I_ADD, (outs sreg:$dst), (ins sreg:$src0, soper1:$src1, i32imm:$src2, i32imm:$src3),
      !strconcat(IL_OP_I_ADD.Text, " $dst, $src0, $src1"),
      [(set (stype sreg:$dst), (opn (stype sreg:$src0), (stype1 simm1:$src1), timm:$src2, timm:$src3))]>;
    def ii : ILFormat<IL_OP_I_ADD, (outs sreg:$dst), (ins soper:$src0, soper1:$src1, i32imm:$src2, i32imm:$src3),
      !strconcat(IL_OP_I_ADD.Text, " $dst, $src0, $src1"),
      [(set (stype sreg:$dst), (opn (stype simm:$src0), (stype1 simm1:$src1), timm:$src2, timm:$src3))]>;
  }
}

// Class that handles the various vector insert patterns
multiclass VectorInsert<SDNode opn> {
  defm v2f64 : InsertVectorClass<opn, GPRV2I64, v2f64imm, v2f64, fpimm,
      GPR_64, f64imm, f64, fpimm>;
  defm v4f32 : InsertVectorClass<opn, GPRV4I32, v4f32imm, v4f32, fpimm,
      GPR_32, f32imm, f32, fpimm>;
  defm v2f32 : InsertVectorClass<opn, GPRV2I32, v2f32imm, v2f32, fpimm,
      GPR_32, f32imm, f32, fpimm>;
  defm v2i64 : InsertVectorClass<opn, GPRV2I64, v2i64imm, v2i64,   imm,
      GPR_64, i64imm, i64,   imm>;
  defm v4i8  : InsertVectorClass<opn,  GPRV4I8,  v4i8imm,  v4i8,   imm,
      GPRI8,   i8imm,  i8,   imm>;
  defm v4i16 : InsertVectorClass<opn, GPRV4I16, v4i16imm, v4i16,   imm,
      GPRI16, i16imm, i16,   imm>;
  defm v4i32 : InsertVectorClass<opn, GPRV4I32, v4i32imm, v4i32,   imm,
      GPR_32, i32imm, i32,   imm>;
  defm v2i8  : InsertVectorClass<opn,  GPRV2I8,  v2i8imm,  v2i8,   imm,
      GPRI8,   i8imm,  i8,   imm>;
  defm v2i16 : InsertVectorClass<opn, GPRV2I16, v2i16imm, v2i16,   imm,
      GPRI16, i16imm, i16,   imm>;
  defm v2i32 : InsertVectorClass<opn, GPRV2I32, v2i32imm, v2i32,   imm,
      GPR_32, i32imm, i32,   imm>;
}

multiclass ConvertOpMC<ILOpCode opcode, SDNode intr,
           ValueType dtype, RegisterClass dreg,
           ValueType stype, RegisterClass sreg,
           Operand soper, SDNode imm0> {
  def r : ILFormat<opcode, (outs dreg:$dst), (ins sreg:$src0),
    !strconcat(opcode.Text, " $dst, $src0"),
    [(set (dtype dreg:$dst), (intr (stype sreg:$src0)))]>;
  def i : ILFormat<opcode, (outs dreg:$dst), (ins soper:$src0),
    !strconcat(opcode.Text, " $dst, $src0"),
    [(set (dtype dreg:$dst), (intr (stype imm0:$src0)))]>;
}

multiclass VectorCreate<ILOpCode opc, SDNode opn> {
  defm v4f32 : ConvertOpMC<opc, opn, v4f32, GPRV4I32, f32, GPR_32, f32imm, fpimm>;
  defm v4i16 : ConvertOpMC<opc, opn, v4i16, GPRV4I16, i16, GPRI16, i16imm, imm>;
  defm v4i8  : ConvertOpMC<opc, opn, v4i8,  GPRV4I8,  i8,  GPRI8,  i8imm, imm>;
  defm v4i32 : ConvertOpMC<opc, opn, v4i32, GPRV4I32, i32, GPR_32, i32imm, imm>;
  defm v2f32 : ConvertOpMC<opc, opn, v2f32, GPRV2I32, f32, GPR_32, f32imm, fpimm>;
  defm v2i16 : ConvertOpMC<opc, opn, v2i16, GPRV2I16, i16, GPRI16, i16imm, imm>;
  defm v2i8  : ConvertOpMC<opc, opn, v2i8,  GPRV2I8,  i8,  GPRI8,  i8imm, imm>;
  defm v2i32 : ConvertOpMC<opc, opn, v2i32, GPRV2I32, i32, GPR_32, i32imm, imm>;
  defm v2f64 : ConvertOpMC<opc, opn, v2f64, GPRV2I64, f64, GPR_64, f64imm, fpimm>;
  defm v2i64 : ConvertOpMC<opc, opn, v2i64, GPRV2I64, i64, GPR_64, i64imm, imm>;
}

multiclass IntToAnyMC<ILOpCode opc, SDNode opn> {
  defm i32    : ConvertOpMC<opc, opn, i32,   GPR_32, i32, GPR_32, i32imm, imm>;
  defm i8     : ConvertOpMC<opc, opn, i8,    GPRI8,  i32, GPR_32, i32imm, imm>;
  defm i16    : ConvertOpMC<opc, opn, i16,   GPRI16, i32, GPR_32, i32imm, imm>;
  defm f32    : ConvertOpMC<opc, opn, f32,   GPR_32, i32, GPR_32, i32imm, imm>;
}

multiclass BitConversion<ILOpCode opc, SDNode opn, ValueType t, RegisterClass r> {
  defm i32    : ConvertOpMC<opc, opn, t, r, i32,   GPR_32,   i32imm, imm>;
  defm i8     : ConvertOpMC<opc, opn, t, r, i8,    GPRI8,    i8imm, imm>;
  defm i16    : ConvertOpMC<opc, opn, t, r, i16,   GPRI16,   i16imm, imm>;
  defm f32    : ConvertOpMC<opc, opn, t, r, f32,   GPR_32,   f32imm, fpimm>;
  defm i64    : ConvertOpMC<opc, opn, t, r, i64,   GPR_64,   i64imm, imm>;
  defm f64    : ConvertOpMC<opc, opn, t, r, f64,   GPR_64,   f64imm, fpimm>;
  defm v2i8   : ConvertOpMC<opc, opn, t, r, v2i8,  GPRV2I8,  v2i8imm, imm>;
  defm v2i16  : ConvertOpMC<opc, opn, t, r, v2i16, GPRV2I16, v2i16imm, imm>;
  defm v2i32  : ConvertOpMC<opc, opn, t, r, v2i32, GPRV2I32, v2i32imm, imm>;
  defm v2i64  : ConvertOpMC<opc, opn, t, r, v2i64, GPRV2I64, v2i64imm, imm>;
  defm v2f32  : ConvertOpMC<opc, opn, t, r, v2f32, GPRV2I32, v2f32imm, fpimm>;
  defm v2f64  : ConvertOpMC<opc, opn, t, r, v2f64, GPRV2I64, v2f64imm, fpimm>;
  defm v4i8   : ConvertOpMC<opc, opn, t, r, v4i8,  GPRV4I8,  v4i8imm, imm>;
  defm v4i16  : ConvertOpMC<opc, opn, t, r, v4i16, GPRV4I16, v4i16imm, imm>;
  defm v4i32  : ConvertOpMC<opc, opn, t, r, v4i32, GPRV4I32, v4i32imm, imm>;
  defm v4f32  : ConvertOpMC<opc, opn, t, r, v4f32, GPRV4I32, v4f32imm, fpimm>;
}


multiclass ConvertIntMC<ILOpCode opcode, Intrinsic intr,
           ValueType dtype, RegisterClass dreg,
           ValueType stype, RegisterClass sreg, Operand oper,
           SDNode imm0> {
  def r : ILFormat<opcode, (outs dreg:$dst), (ins sreg:$src0),
  !strconcat(opcode.Text, " $dst, $src0"),
  [(set (dtype dreg:$dst), (intr (stype sreg:$src0)))]>;
  def i : ILFormat<opcode, (outs dreg:$dst), (ins oper:$src0),
  !strconcat(opcode.Text, " $dst, $src0"),
  [(set (dtype dreg:$dst), (intr (stype imm0:$src0)))]>;
}

multiclass IntrConvertF32TOF16<ILOpCode opcode, Intrinsic intr> {
  defm i16   : ConvertIntMC<opcode, intr, i16,   GPRI16,   f32,   GPR_32, f32imm, fpimm>;
  defm v2i16 : ConvertIntMC<opcode, intr, v2i16, GPRV2I16, v2f32, GPRV2I32, v2f32imm, fpimm>;
  defm v4i16 : ConvertIntMC<opcode, intr, v4i16, GPRV4I16, v4f32, GPRV4I32, v4f32imm, fpimm>;
}

multiclass IntrConvertF32TOI32<ILOpCode opcode, Intrinsic intr> {
  defm i32   : ConvertIntMC<opcode, intr, i32,   GPR_32,   f32,   GPR_32, f32imm, fpimm>;
  defm v2i32 : ConvertIntMC<opcode, intr, v2i32, GPRV2I32, v2f32, GPRV2I32, v2f32imm, fpimm>;
  defm v4i32 : ConvertIntMC<opcode, intr, v4i32, GPRV4I32, v4f32, GPRV4I32, v4f32imm, fpimm>;
}

multiclass IntrConvertF64TOI32<ILOpCode opcode, Intrinsic intr> {
  defm i32   : ConvertIntMC<opcode, intr, i32,   GPR_32,   f64,   GPR_64, f64imm, fpimm>;
  defm v2i32 : ConvertIntMC<opcode, intr, v2i32, GPRV2I32, v2f64, GPRV2I64, v2f64imm, fpimm>;
}

multiclass IntrConvertF16TOF32<ILOpCode opcode, Intrinsic intr> {
  defm f32   : ConvertIntMC<opcode, intr, f32,   GPR_32,   i32,   GPR_32, i32imm, imm>;
  defm v2f32 : ConvertIntMC<opcode, intr, v2f32, GPRV2I32, v2i32, GPRV2I32, v2i32imm, imm>;
  defm v4f32 : ConvertIntMC<opcode, intr, v4f32, GPRV4I32, v4i32, GPRV4I32, v4i32imm, imm>;
}


multiclass IntrConvertI32TOF32<ILOpCode opcode, Intrinsic intr> {
  defm f32   : ConvertIntMC<opcode, intr,   f32,   GPR_32,   i32, GPR_32, i32imm, imm>;
  defm v2f32 : ConvertIntMC<opcode, intr, v2f32, GPRV2I32, v2i32, GPRV2I32, v2i32imm, imm>;
  defm v4f32 : ConvertIntMC<opcode, intr, v4f32, GPRV4I32, v4i32, GPRV4I32, v4i32imm, imm>;
}

multiclass ClassExpandMC<ILOpCode opcode, Intrinsic intr,
                         ValueType stype, RegisterClass sreg,
                         Operand soper, SDNode simm,
                         ValueType dtype, RegisterClass dreg,
                         Operand doper, SDNode dimm> {
  def rr : ILFormat<opcode, (outs dreg:$dst), (ins sreg:$src0, dreg:$src1),
    !strconcat(opcode.Text, " $dst, $src0, $src1"),
    [(set (dtype dreg:$dst), (intr (stype sreg:$src0), (dtype dreg:$src1)))]>;
  def ir : ILFormat<opcode, (outs dreg:$dst), (ins soper:$src0, dreg:$src1),
    !strconcat(opcode.Text, " $dst, $src0, $src1"),
    [(set (dtype dreg:$dst), (intr (stype simm:$src0), (dtype dreg:$src1)))]>;
  def ri : ILFormat<opcode, (outs dreg:$dst), (ins sreg:$src0, doper:$src1),
    !strconcat(opcode.Text, " $dst, $src0, $src1"),
    [(set (dtype dreg:$dst), (intr (stype sreg:$src0), (dtype dimm:$src1)))]>;
  def ii : ILFormat<opcode, (outs dreg:$dst), (ins soper:$src0, doper:$src1),
    !strconcat(opcode.Text, " $dst, $src0, $src1"),
    [(set (dtype dreg:$dst), (intr (stype simm:$src0), (dtype dimm:$src1)))]>;
}


multiclass ClassMC<Intrinsic intr> {
  defm f64   : ClassExpandMC<IL_OP_D_CLASS, intr, f64, GPR_64, f64imm, fpimm, i32, GPR_32, i32imm, imm>;
  defm v2f64 : ClassExpandMC<IL_OP_D_CLASS, intr, v2f64, GPRV2I64, v2f64imm, fpimm, v2i32, GPRV2I32, v2i32imm, imm>;
  defm f32   : ClassExpandMC<IL_OP_CLASS, intr, f32, GPR_32, f32imm, fpimm, i32, GPR_32, i32imm, imm>;
  defm v2f32 : ClassExpandMC<IL_OP_CLASS, intr, v2f32, GPRV2I32, v2f32imm, fpimm, v2i32, GPRV2I32, v2i32imm, imm>;
  defm v4f32 : ClassExpandMC<IL_OP_CLASS, intr, v4f32, GPRV4I32, v4f32imm, fpimm, v4i32, GPRV4I32, v4i32imm, imm>;
}

//----------------------------------------------------------------------------//
// Multiclasses that deal with zero (input) operand instructions.
//----------------------------------------------------------------------------//
multiclass NullaryIntMCFull<ILOpCode opc, Intrinsic opn,
                          ValueType dtype, RegisterClass dreg> {
  def r : ILFormat<opc, (outs dreg:$dst), (ins),
    !strconcat(opc.Text, " $dst"),
    [(set (dtype dreg:$dst), (opn))]>;
}

//----------------------------------------------------------------------------//
// Multiclasses that deal with one operand instructions.
//----------------------------------------------------------------------------//
multiclass UnaryNegReg<ILOpCode opc, SDNode opn,
                       RegisterClass reg, Operand oper,
                       ValueType type, SDNode imm0, string str> {
  def r : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0),
    !strconcat(!strconcat(opc.Text, " $dst, "), str),
    [(set (type reg:$dst), (opn (type reg:$src0)))]>;
  def i : ILFormat<opc,
    (outs reg:$dst),
    (ins oper:$src0),
    !strconcat(!strconcat(opc.Text, " $dst, "), str),
    [(set (type reg:$dst),
        (opn (type imm0:$src0)))]>;
}

multiclass UnaryNegExpand<ILOpCode opc, SDNode opn,
                          RegisterClass reg, Operand oper,
                          ValueType type, string str> :
  UnaryNegReg<opc, opn, reg, oper, type, imm, str>;

multiclass UnaryNegExpandFP<ILOpCode opc, SDNode opn,
                            RegisterClass reg, Operand oper,
                            ValueType type, string str> :
  UnaryNegReg<opc, opn, reg, oper, type, fpimm, str>;

multiclass UnaryNegMCf32<ILOpCode opc, SDNode opn> {
  defm f32   : UnaryNegExpandFP<opc, opn, GPR_32, f32imm, f32, "${src0}_neg(xyzw)">;
  defm v2f32 : UnaryNegExpandFP<opc, opn, GPRV2I32, v2f32imm, v2f32, "${src0}_neg(xyzw)">;
  defm v4f32 : UnaryNegExpandFP<opc, opn, GPRV4I32, v4f32imm, v4f32, "${src0}_neg(xyzw)">;
}

multiclass UnaryNegMCf64<ILOpCode opc, SDNode opn> {
  defm f64   : UnaryNegExpandFP<opc, opn, GPR_64, f64imm, f64, "${src0}_neg(yw)">;
  defm v2f64 : UnaryNegExpandFP<opc, opn, GPRV2I64, v2f64imm, v2f64, "${src0}_neg(yw)">;
}
multiclass UnaryOpMCFull<ILOpCode opc, SDNode opn,
           ValueType dtype, RegisterClass dreg,
           ValueType stype0, RegisterClass sreg0,
           Operand oper0, SDNode imm0> {
  def r : ILFormat<opc, (outs dreg:$dst),
    (ins sreg0:$src0),
    !strconcat(opc.Text, " $dst, $src0"),
    [(set (dtype dreg:$dst), (opn (stype0 sreg0:$src0)))]>;
  def i : ILFormat<opc, (outs dreg:$dst),
    (ins oper0:$src0),
    !strconcat(opc.Text, " $dst, $src0"),
    [(set (dtype dreg:$dst), (opn (stype0 imm0:$src0)))]>;
}




multiclass UnaryOpReg<ILOpCode opc, SDNode opn, RegisterClass reg,
                      Operand oper, ValueType type, SDNode imm0> :
  UnaryOpMCFull<opc, opn, type, reg, type, reg, oper, imm0>;

multiclass UnaryOpExpand<ILOpCode opc, SDNode opn,
                         RegisterClass reg, Operand oper,
                         ValueType type> :
  UnaryOpReg<opc, opn, reg, oper, type, imm>;

multiclass UnaryOpExpandFP<ILOpCode opc, SDNode opn,
                           RegisterClass reg, Operand oper,
                           ValueType type> :
  UnaryOpReg<opc, opn, reg, oper, type, fpimm>;


multiclass UnaryOpMCi8Scalar<ILOpCode opc, SDNode opn> {
  defm i8 : UnaryOpExpand<opc, opn, GPRI8, i8imm, i8>;
}

multiclass UnaryOpMCi8Vector<ILOpCode opc, SDNode opn> {
  defm v2i8 : UnaryOpExpand<opc, opn, GPRV2I8, v2i8imm, v2i8>;
  defm v4i8 : UnaryOpExpand<opc, opn, GPRV4I8, v4i8imm, v4i8>;
}

multiclass UnaryOpMCi16Scalar<ILOpCode opc, SDNode opn> {
  defm i16 : UnaryOpExpand<opc, opn, GPRI16, i16imm, i16>;
}

multiclass UnaryOpMCi16Vector<ILOpCode opc, SDNode opn> {
  defm v2i16 : UnaryOpExpand<opc, opn, GPRV2I16, v2i16imm, v2i16>;
  defm v4i16 : UnaryOpExpand<opc, opn, GPRV4I16, v4i16imm, v4i16>;
}

multiclass UnaryOpMCi32Scalar<ILOpCode opc, SDNode opn> {
  defm i32 : UnaryOpExpand<opc, opn, GPR_32,  i32imm, i32>;
}

multiclass UnaryOpMCi32Vector<ILOpCode opc, SDNode opn> {
  defm v2i32 : UnaryOpExpand<opc, opn, GPRV2I32, v2i32imm, v2i32>;
  defm v4i32 : UnaryOpExpand<opc, opn, GPRV4I32, v4i32imm, v4i32>;
}

multiclass UnaryOpMCi64Scalar<ILOpCode opc, SDNode opn> {
  defm i64 : UnaryOpExpand<opc, opn, GPR_64,  i64imm, i64>;
}

multiclass UnaryOpMCi64Vector<ILOpCode opc, SDNode opn> {
  defm v2i64 : UnaryOpExpand<opc, opn, GPRV2I64, v2i64imm, v2i64>;
}

multiclass UnaryOpMCf32Scalar<ILOpCode opc, SDNode opn> {
  defm f32 : UnaryOpExpandFP<opc, opn, GPR_32, f32imm, f32>;
}

multiclass UnaryOpMCf32Vector<ILOpCode opc, SDNode opn> {
  defm v2f32 : UnaryOpExpandFP<opc, opn, GPRV2I32, v2f32imm, v2f32>;
  defm v4f32 : UnaryOpExpandFP<opc, opn, GPRV4I32, v4f32imm, v4f32>;
}


multiclass UnaryOpMCf64Scalar<ILOpCode opc, SDNode opn> {
  defm f64 : UnaryOpExpandFP<opc, opn, GPR_64, f64imm, f64>;
}

multiclass UnaryOpMCf64Vector<ILOpCode opc, SDNode opn> {
  defm v2f64 : UnaryOpExpandFP<opc, opn, GPRV2I64, v2f64imm, v2f64>;
}

multiclass UnaryOpMCi32<ILOpCode opc, SDNode opn> :
  UnaryOpMCi32Scalar<opc, opn>, UnaryOpMCi32Vector<opc, opn>,
  UnaryOpMCi16Scalar<opc, opn>, UnaryOpMCi16Vector<opc, opn>,
  UnaryOpMCi8Scalar< opc, opn>, UnaryOpMCi8Vector< opc, opn>;

multiclass UnaryOpMCi64<ILOpCode opc, SDNode opn> :
  UnaryOpMCi64Scalar<opc, opn>, UnaryOpMCi64Vector<opc, opn>;

multiclass UnaryOpMCf32<ILOpCode opc, SDNode opn> :
  UnaryOpMCf32Scalar<opc, opn>, UnaryOpMCf32Vector<opc, opn>;

multiclass UnaryOpMCf64<ILOpCode opc, SDNode opn> :
  UnaryOpMCf64Scalar<opc, opn>, UnaryOpMCf64Vector<opc, opn>;

multiclass UnaryOpMCInt<ILOpCode opc, SDNode opn> :
  UnaryOpMCi32<opc, opn> , UnaryOpMCi64<opc, opn>;

multiclass UnaryOpMCFP<ILOpCode opc, SDNode opn> :
  UnaryOpMCf32<opc, opn>, UnaryOpMCf64<opc, opn>;

multiclass UnaryOpMC<ILOpCode opc, SDNode opn> :
  UnaryOpMCInt<opc, opn>, UnaryOpMCFP<opc, opn>;

//-------------------------------------------------------------------//
// Multiclasses that deal with one operand intrinsics.
//-------------------------------------------------------------------//
multiclass UnaryIntMCVoid<ILOpCode opc, Intrinsic opn,
           ValueType stype0, RegisterClass sreg0,
           Operand oper0, SDNode imm0,
           string str> {
  def r : ILFormat<opc, (outs), (ins sreg0:$src0),
    !strconcat(opc.Text, str), [(opn (stype0 sreg0:$src0))]>;
  def i : ILFormat<opc, (outs), (ins oper0:$src0),
    !strconcat(opc.Text, str), [(opn (stype0 imm0:$src0))]>;
}

multiclass Fence<ILOpCode opc, Intrinsic opn> :
  UnaryIntMCVoid<opc, opn, i32, GPR_32, i32imm, imm, "">;

multiclass UnaryIntMCFull<ILOpCode opc, Intrinsic opn,
                          ValueType dtype, RegisterClass dreg,
                          ValueType stype0, RegisterClass sreg0,
                          Operand oper0, SDNode imm0> {
  def r : ILFormat<opc, (outs dreg:$dst),
    (ins sreg0:$src0),
    !strconcat(opc.Text, " $dst, $src0"),
    [(set (dtype dreg:$dst), (opn (stype0 sreg0:$src0)))]>;
  def i : ILFormat<opc, (outs dreg:$dst),
    (ins oper0:$src0),
    !strconcat(opc.Text, " $dst, $src0"),
    [(set (dtype dreg:$dst), (opn (stype0 imm0:$src0)))]>;
}

multiclass FTZMCReg<ILOpCode opc, Intrinsic opn,
           RegisterClass reg, Operand oper,
           ValueType type, SDNode imm0> {
  def r : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0),
    !strconcat(opc.Text, " $dst, $src0, r0.1"),
    [(set (type reg:$dst), (opn (type reg:$src0)))]>;
  def i : ILFormat<opc,
    (outs reg:$dst),
    (ins oper:$src0),
    !strconcat(opc.Text, " $dst, $src0, r0.1"),
    [(set (type reg:$dst), (opn (type imm0:$src0)))]>;
}

multiclass FTZMC<ILOpCode opc, Intrinsic opn> {
  defm f32   : FTZMCReg<opc, opn, GPR_32, f32imm, f32, fpimm>;
  defm v2f32 : FTZMCReg<opc, opn, GPRV2I32, v2f32imm, v2f32, fpimm>;
  defm v4f32 : FTZMCReg<opc, opn, GPRV4I32, v4f32imm, v4f32, fpimm>;
}

multiclass UnaryIntReg<ILOpCode opc, Intrinsic opn,
                       RegisterClass reg,
                       Operand oper, ValueType type, SDNode imm0> {
  def r : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0),
   !strconcat(opc.Text, " $dst, $src0"),
    [(set (type reg:$dst), (opn (type reg:$src0)))]>;
  def i : ILFormat<opc,
    (outs reg:$dst),
    (ins oper:$src0),
    !strconcat(opc.Text, " $dst, $src0"),
    [(set (type reg:$dst), (opn (type imm0:$src0)))]>;
}

multiclass UnaryIntExpand<ILOpCode opc, Intrinsic opn,
                          RegisterClass reg, Operand oper,
                          ValueType type> :
  UnaryIntReg<opc, opn, reg, oper, type, imm>;

multiclass UnaryIntExpandFP<ILOpCode opc, Intrinsic opn,
                            RegisterClass reg,
                            Operand oper, ValueType type> :
  UnaryIntReg<opc, opn, reg, oper, type, fpimm>;


multiclass UnaryIntMCi8Scalar<ILOpCode opc, Intrinsic opn> {
  defm i8 : UnaryIntExpand<opc, opn, GPRI8, i8imm, i8>;
}

multiclass UnaryIntMCi8Vector<ILOpCode opc, Intrinsic opn> {
  defm v2i8 : UnaryIntExpand<opc, opn, GPRV2I8, v2i8imm, v2i8>;
  defm v4i8 : UnaryIntExpand<opc, opn, GPRV4I8, v4i8imm, v4i8>;
}

multiclass UnaryIntMCi16Scalar<ILOpCode opc, Intrinsic opn> {
  defm i16 : UnaryIntExpand<opc, opn, GPRI16, i16imm, i16>;
}

multiclass UnaryIntMCi16Vector<ILOpCode opc, Intrinsic opn> {
  defm v2i16 : UnaryIntExpand<opc, opn, GPRV2I16, v2i16imm, v2i16>;
  defm v4i16 : UnaryIntExpand<opc, opn, GPRV4I16, v4i16imm, v4i16>;
}

multiclass UnaryIntMCi32Scalar<ILOpCode opc, Intrinsic opn> {
  defm i32 : UnaryIntExpand<opc, opn, GPR_32,  i32imm, i32>;
}

multiclass UnaryIntMCi32Vector<ILOpCode opc, Intrinsic opn> {
  defm v2i32 : UnaryIntExpand<opc, opn, GPRV2I32, v2i32imm, v2i32>;
  defm v4i32 : UnaryIntExpand<opc, opn, GPRV4I32, v4i32imm, v4i32>;
}

multiclass UnaryIntMCi64Scalar<ILOpCode opc, Intrinsic opn> {
  defm i64 : UnaryIntExpand<opc, opn, GPR_64, i64imm, i64>;
}

multiclass UnaryIntMCi64Vector<ILOpCode opc, Intrinsic opn> {
  defm v2i64 : UnaryIntExpand<opc, opn, GPRV2I64, v2i64imm, v2i64>;
}

multiclass UnaryIntMCf32Scalar<ILOpCode opc, Intrinsic opn> {
  defm f32 : UnaryIntExpandFP<opc, opn, GPR_32,  f32imm, f32>;
}

multiclass UnaryIntMCf32Vector<ILOpCode opc, Intrinsic opn> {
  defm v2f32 : UnaryIntExpandFP<opc, opn, GPRV2I32, v2f32imm, v2f32>;
  defm v4f32 : UnaryIntExpandFP<opc, opn, GPRV4I32, v4f32imm, v4f32>;
}

multiclass UnaryIntMCf64Scalar<ILOpCode opc, Intrinsic opn> {
  defm f64 : UnaryIntExpandFP<opc, opn, GPR_64, f64imm, f64>;
}

multiclass UnaryIntMCf64Vector<ILOpCode opc, Intrinsic opn> {
  defm v2f64 : UnaryIntExpandFP<opc, opn, GPRV2I64, v2f64imm, v2f64>;
}

multiclass UnaryIntMCi32<ILOpCode opc, Intrinsic opn> :
  UnaryIntMCi32Scalar<opc, opn>,
  UnaryIntMCi32Vector<opc, opn>,
  UnaryIntMCi16Scalar<opc, opn>,
  UnaryIntMCi16Vector<opc, opn>,
  UnaryIntMCi8Scalar< opc, opn>,
  UnaryIntMCi8Vector< opc, opn>;

multiclass UnaryIntMCi64<ILOpCode opc, Intrinsic opn> :
  UnaryIntMCi64Scalar<opc, opn>,
  UnaryIntMCi64Vector<opc, opn>;

multiclass UnaryIntMCf32<ILOpCode opc, Intrinsic opn> :
  UnaryIntMCf32Scalar<opc, opn>, UnaryIntMCf32Vector<opc, opn>;

multiclass UnaryIntMCf64<ILOpCode opc, Intrinsic opn> :
  UnaryIntMCf64Scalar<opc, opn>, UnaryIntMCf64Vector<opc, opn>;

multiclass UnaryIntMCInt<ILOpCode opc, Intrinsic opn> :
  UnaryIntMCi32<opc, opn> , UnaryIntMCi64<opc, opn>;

multiclass UnaryIntMCFP<ILOpCode opc, Intrinsic opn> :
  UnaryIntMCf32<opc, opn>, UnaryIntMCf64<opc, opn>;

multiclass UnaryIntMC<ILOpCode opc, Intrinsic opn> :
  UnaryIntMCInt<opc, opn>, UnaryIntMCFP<opc, opn>;

//-------------------------------------------------------------------//
// Multiclasses that deal with one operand pattern fragments.
//-------------------------------------------------------------------//
multiclass UnaryPatReg<ILOpCode opc, PatFrag opn,
           RegisterClass reg, Operand oper,
           ValueType type, SDNode imm0> {
  def r : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0),
    !strconcat(opc.Text, " $dst, $src0"),
    [(set (type reg:$dst), (opn (type reg:$src0)))]>;
}

multiclass UnaryNCPatReg<ILOpCode opc, PatFrag opn,
           RegisterClass reg, Operand oper,
           ValueType type, SDNode imm0> :
  UnaryPatReg<opc, opn, reg, oper, type, imm0> {
    def i : ILFormat<opc,
      (outs reg:$dst),
      (ins oper:$src0),
      !strconcat(opc.Text, " $dst, $src0"),
      [(set (type reg:$dst), (opn (type imm0:$src0)))]>;
}

multiclass UnaryPatExpand<ILOpCode opc, PatFrag opn,
                          RegisterClass reg,
                          Operand oper, ValueType type> :
  UnaryNCPatReg<opc, opn, reg, oper, type, imm>;

multiclass UnaryPatExpandFP<ILOpCode opc, PatFrag opn,
                            RegisterClass reg, Operand oper,
                            ValueType type> :
  UnaryNCPatReg<opc, opn, reg, oper, type, fpimm>;


multiclass UnaryPatMCi8Scalar<ILOpCode opc, PatFrag opn> {
  defm i8 : UnaryPatExpand<opc, opn, GPRI8, i8imm, i8>;
}

multiclass NotMCReg<ILOpCode opc, PatFrag opn, PatFrag vopn> {
  defm i8    : UnaryPatReg<opc, vopn,    GPRI8,    i8imm,    i8, imm>;
  defm v2i8  : UnaryPatReg<opc, vopn,  GPRV2I8,  v2i8imm,  v2i8, imm>;
  defm v4i8  : UnaryPatReg<opc, vopn,  GPRV4I8,  v4i8imm,  v4i8, imm>;
  defm i16   : UnaryPatReg<opc, vopn,   GPRI16,   i16imm,   i16, imm>;
  defm v2i16 : UnaryPatReg<opc, vopn, GPRV2I16, v2i16imm, v2i16, imm>;
  defm v4i16 : UnaryPatReg<opc, vopn, GPRV4I16, v4i16imm, v4i16, imm>;
  defm i32   : UnaryPatReg<opc, vopn,   GPR_32,   i32imm,   i32, imm>;
  defm v2i32 : UnaryPatReg<opc, vopn, GPRV2I32, v2i32imm, v2i32, imm>;
  defm v4i32 : UnaryPatReg<opc, vopn, GPRV4I32, v4i32imm, v4i32, imm>;
  defm i64   : UnaryPatReg<opc, vopn,   GPR_64,   i64imm,   i64, imm>;
  defm v2i64 : UnaryPatReg<opc, vopn, GPRV2I64, v2i64imm, v2i64, imm>;
}


multiclass UnaryPatMCi8Vector<ILOpCode opc, PatFrag opn> {
  defm v2i8  : UnaryPatExpand<opc, opn, GPRV2I8,  v2i8imm, v2i8>;
  defm v4i8  : UnaryPatExpand<opc, opn, GPRV4I8,  v4i8imm, v4i8>;
}

multiclass UnaryPatMCi16Scalar<ILOpCode opc, PatFrag opn> {
  defm i16 : UnaryPatExpand<opc, opn, GPRI16, i16imm, i16>;
}

multiclass UnaryPatMCi16Vector<ILOpCode opc, PatFrag opn> {
  defm v2i16 : UnaryPatExpand<opc, opn, GPRV2I16, v2i16imm, v2i16>;
  defm v4i16 : UnaryPatExpand<opc, opn, GPRV4I16, v4i16imm, v4i16>;
}

multiclass UnaryPatMCi32Scalar<ILOpCode opc, PatFrag opn> {
  defm i32 : UnaryPatExpand<opc, opn, GPR_32,  i32imm,   i32>;
}

multiclass UnaryPatMCi32Vector<ILOpCode opc, PatFrag opn> {
  defm v2i32 : UnaryPatExpand<opc, opn, GPRV2I32, v2i32imm, v2i32>;
  defm v4i32 : UnaryPatExpand<opc, opn, GPRV4I32, v4i32imm, v4i32>;
}

multiclass UnaryPatMCi64Scalar<ILOpCode opc, PatFrag opn> {
  defm i64 : UnaryPatExpand<opc, opn, GPR_64,  i64imm, i64>;
}

multiclass UnaryPatMCi64Vector<ILOpCode opc, PatFrag opn> {
  defm v2i64 : UnaryPatExpand<opc, opn, GPRV2I64, v2i64imm, v2i64>;
}

multiclass UnaryPatMCf32Scalar<ILOpCode opc, PatFrag opn> {
  defm f32 : UnaryPatExpandFP<opc, opn, GPR_32, f32imm, f32>;
}

multiclass UnaryPatMCf32Vector<ILOpCode opc, PatFrag opn> {
  defm v2f32 : UnaryPatExpandFP<opc, opn, GPRV2I32,  v2f32imm, v2f32>;
  defm v4f32 : UnaryPatExpandFP<opc, opn, GPRV4I32, v4f32imm, v4f32>;
}

multiclass UnaryPatMCf64Scalar<ILOpCode opc, PatFrag opn> {
  defm f64 : UnaryPatExpandFP<opc, opn, GPR_64,  f64imm, f64>;
}

multiclass UnaryPatMCf64Vector<ILOpCode opc, PatFrag opn> {
  defm v2f64 : UnaryPatExpandFP<opc, opn, GPRV2I64, v2f64imm, v2f64>;
}

multiclass UnaryPatMCi32<ILOpCode opc, PatFrag opn> :
  UnaryPatMCi32Scalar<opc, opn>, UnaryPatMCi32Vector<opc, opn>,
  UnaryPatMCi16Scalar<opc, opn>, UnaryPatMCi16Vector<opc, opn>,
  UnaryPatMCi8Scalar< opc, opn>, UnaryPatMCi8Vector< opc, opn>;

multiclass UnaryPatMCi64<ILOpCode opc, PatFrag opn> :
  UnaryPatMCi64Scalar<opc, opn>,
  UnaryPatMCi64Vector<opc, opn>;

multiclass UnaryPatMCf32<ILOpCode opc, PatFrag opn> :
  UnaryPatMCf32Scalar<opc, opn>,
  UnaryPatMCf32Vector<opc, opn>;

multiclass UnaryPatMCf64<ILOpCode opc, PatFrag opn> :
  UnaryPatMCf64Scalar<opc, opn>,
  UnaryPatMCf64Vector<opc, opn>;

multiclass UnaryPatMCInt<ILOpCode opc, PatFrag opn> :
  UnaryPatMCi32<opc, opn>,
  UnaryPatMCi64<opc, opn>;

multiclass UnaryPatMCFP<ILOpCode opc, PatFrag opn> :
  UnaryPatMCf32<opc, opn>,
  UnaryPatMCf64<opc, opn>;

multiclass UnaryPatMC<ILOpCode opc, PatFrag opn> :
  UnaryPatMCInt<opc, opn>,
  UnaryPatMCFP<opc, opn>;

multiclass UnpackPatFrag<ILOpCode opc, PatFrag node> {
  def f32r : ILFormat<opc, (outs GPR_32:$dst),
      (ins GPR_32:$src),
      !strconcat(opc.Text, " $dst, $src"),
      [(set GPR_32:$dst, (node (i32 GPR_32:$src)))]>;
  def f32i : ILFormat<opc, (outs GPR_32:$dst),
      (ins i32imm:$src),
      !strconcat(opc.Text, " $dst, $src"),
      [(set GPR_32:$dst, (node (i32 imm:$src)))]>;
}

//-------------------------------------------------------------------//
// Multiclasses for instructions with two commutable operands.
//-------------------------------------------------------------------//
multiclass BinaryNegReg<ILOpCode opc, SDNode opn,
           RegisterClass reg, Operand oper, ValueType type,
           SDNode imm0, string str> {
  def rr : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1),
    !strconcat(!strconcat(opc.Text, " $dst, $src0, "), str),
    [(set (type reg:$dst),
        (opn (type reg:$src0), (type reg:$src1)))]>;
  def ir : ILFormat<opc,
    (outs reg:$dst),
    (ins oper:$src0, reg:$src1),
    !strconcat(!strconcat(opc.Text, " $dst, $src0, "), str),
    [(set (type reg:$dst),
        (opn (type imm0:$src0), (type reg:$src1)))]>;
  def ri : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, oper:$src1),
    !strconcat(!strconcat(opc.Text, " $dst, $src0, "), str),
    [(set (type reg:$dst),
        (opn (type reg:$src0), (type imm0:$src1)))]>;
  def ii : ILFormat<opc,
    (outs reg:$dst),
    (ins oper:$src0, oper:$src1),
    !strconcat(!strconcat(opc.Text, " $dst, $src0, "), str),
    [(set (type reg:$dst),
        (opn (type imm0:$src0), (type imm0:$src1)))]>;
}

multiclass BinaryNegExpand<ILOpCode opc, SDNode opn,
                           RegisterClass reg,
                           Operand oper, ValueType type, string str> :
  BinaryNegReg<opc, opn, reg, oper, type, imm, str>;

multiclass BinaryNegExpandFP<ILOpCode opc, SDNode opn,
                             RegisterClass reg, Operand oper,
                             ValueType type, string str> :
  BinaryNegReg<opc, opn, reg, oper, type, fpimm, str>;

multiclass BinaryNegMCf32<ILOpCode opc, SDNode opn> {
  defm f32   : BinaryNegExpandFP<opc, opn, GPR_32, f32imm, f32, "${src1}_neg(xyzw)">;
  defm v2f32 : BinaryNegExpandFP<opc, opn, GPRV2I32, v2f32imm, v2f32, "${src1}_neg(xyzw)">;
  defm v4f32 : BinaryNegExpandFP<opc, opn, GPRV4I32, v4f32imm, v4f32, "${src1}_neg(xyzw)">;
}

multiclass BinaryNegMCf64<ILOpCode opc, SDNode opn> {
  defm f64   : BinaryNegExpandFP<opc, opn, GPR_64, f64imm, f64, "${src1}_neg(yw)">;
  defm v2f64 : BinaryNegExpandFP<opc, opn, GPRV2I64, v2f64imm, v2f64, "${src1}_neg(yw)">;
}

multiclass BinaryOpMCFull<ILOpCode opc, SDNode opn,
           ValueType dtype, RegisterClass dreg,
           ValueType stype0, RegisterClass sreg0,
           Operand oper0, SDNode imm0,
           ValueType stype1, RegisterClass sreg1,
           Operand oper1, SDNode imm1> {
  def rr : ILFormat<opc, (outs dreg:$dst), (ins sreg0:$src0, sreg1:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (dtype dreg:$dst),
      (opn (stype0 sreg0:$src0), (stype1 sreg1:$src1)))]>;
  def ri : ILFormat<opc, (outs dreg:$dst), (ins sreg0:$src0, oper1:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (dtype dreg:$dst),
      (opn (stype0 sreg0:$src0), (stype1 imm1:$src1)))]>;
  def ii : ILFormat<opc, (outs dreg:$dst), (ins oper0:$src0, oper1:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (dtype dreg:$dst),
        (opn (stype0 imm0:$src0), (stype1 imm1:$src1)))]>;
}

multiclass BinaryOpReg<ILOpCode opc, SDNode opn,
                       RegisterClass reg, Operand oper,
                       ValueType type, SDNode imm0> :
  BinaryOpMCFull<opc, opn, type, reg, type, reg, oper, imm0, type, reg, oper, imm0>;

multiclass BinaryOpExpand<ILOpCode opc, SDNode opn,
                          RegisterClass reg,
                          Operand oper, ValueType type, SDNode imm0> :
  BinaryOpReg<opc, opn, reg, oper, type, imm0>;

multiclass BinaryOpMCi32Scalar<ILOpCode opc, SDNode opn> {
  defm i8  : BinaryOpExpand<opc, opn, GPRI8, i8imm, i8, imm>;
  defm i16 : BinaryOpExpand<opc, opn, GPRI16, i16imm, i16, imm>;
  defm i32 : BinaryOpExpand<opc, opn, GPR_32, i32imm, i32, imm>;
}

multiclass BinaryOpMCi32Vector<ILOpCode opc, SDNode opn> {
  defm v2i8  : BinaryOpExpand<opc, opn, GPRV2I8,  v2i8imm,  v2i8, imm>;
  defm v2i16 : BinaryOpExpand<opc, opn, GPRV2I16, v2i16imm, v2i16, imm>;
  defm v2i32 : BinaryOpExpand<opc, opn, GPRV2I32, v2i32imm, v2i32, imm>;
  defm v4i8  : BinaryOpExpand<opc, opn, GPRV4I8,  v4i8imm,  v4i8, imm>;
  defm v4i16 : BinaryOpExpand<opc, opn, GPRV4I16, v4i16imm, v4i16, imm>;
  defm v4i32 : BinaryOpExpand<opc, opn, GPRV4I32, v4i32imm, v4i32, imm>;
}

multiclass BinaryOpMCi64Scalar<ILOpCode opc, SDNode opn> {
  defm i64 : BinaryOpExpand<opc, opn, GPR_64, i64imm, i64, imm>;
}

multiclass BinaryOpMCi64Vector<ILOpCode opc, SDNode opn> {
  defm v2i64 : BinaryOpExpand<opc, opn, GPRV2I64, v2i64imm, v2i64, imm>;
}

multiclass BinaryOpMCf32Scalar<ILOpCode opc, SDNode opn> {
  defm f32 : BinaryOpExpand<opc, opn, GPR_32,  f32imm, f32, fpimm>;
}

multiclass BinaryOpMCf32Vector<ILOpCode opc, SDNode opn> {
  defm v2f32 : BinaryOpExpand<opc, opn, GPRV2I32, v2f32imm, v2f32, fpimm>;
  defm v4f32 : BinaryOpExpand<opc, opn, GPRV4I32, v4f32imm, v4f32, fpimm>;
}

multiclass BinaryOpMCf64Scalar<ILOpCode opc, SDNode opn> {
  defm f64 : BinaryOpExpand<opc, opn, GPR_64,  f64imm, f64, fpimm>;
}

multiclass BinaryOpMCf64Vector<ILOpCode opc, SDNode opn> {
  defm v2f64 : BinaryOpExpand<opc, opn, GPRV2I64, v2f64imm, v2f64, fpimm>;
}

multiclass BinaryOpMCi32<ILOpCode opc, SDNode opn> :
  BinaryOpMCi32Scalar<opc, opn>,
  BinaryOpMCi32Vector<opc, opn>;

multiclass BinaryOpMCi64<ILOpCode opc, SDNode opn> :
  BinaryOpMCi64Scalar<opc, opn>,
  BinaryOpMCi64Vector<opc, opn>;

multiclass BinaryOpMCf32<ILOpCode opc, SDNode opn> :
  BinaryOpMCf32Scalar<opc, opn>,
  BinaryOpMCf32Vector<opc, opn>;

multiclass BinaryOpMCf64<ILOpCode opc, SDNode opn> :
  BinaryOpMCf64Scalar<opc, opn>,
  BinaryOpMCf64Vector<opc, opn>;

multiclass BinaryOpMCInt<ILOpCode opc, SDNode opn> :
  BinaryOpMCi32<opc, opn>,
  BinaryOpMCi64<opc, opn>;

multiclass BinaryOpMCFP<ILOpCode opc, SDNode opn> :
  BinaryOpMCf32<opc, opn>,
  BinaryOpMCf64<opc, opn>;

multiclass BinaryOpMC<ILOpCode opc, SDNode opn> :
  BinaryOpMCInt<opc, opn>,
  BinaryOpMCFP<opc, opn>;

//----------------------------------------------------------------------------//
// Multiclasses that deal with two operand instructions that are not commutable.
//----------------------------------------------------------------------------//
multiclass BinaryNCOpExpand<ILOpCode opc, SDNode opn,
                            RegisterClass reg, Operand oper,
                            ValueType type, SDNode imm0> :
  BinaryOpExpand<opc, opn, reg, oper, type, imm0> {
    def ir : ILFormat<opc, (outs reg:$dst), (ins oper:$src0, reg:$src1),
              !strconcat(opc.Text, " $dst, $src0, $src1"),
              [(set (type reg:$dst),
                (opn (type imm0:$src0), (type reg:$src1)))]>;
  }


multiclass BinaryNCOpMCi32Scalar<ILOpCode opc, SDNode opn> {
  defm i8  : BinaryNCOpExpand<opc, opn, GPRI8, i8imm, i8, imm>;
  defm i16 : BinaryNCOpExpand<opc, opn, GPRI16, i16imm, i16, imm>;
  defm i32 : BinaryNCOpExpand<opc, opn, GPR_32, i32imm, i32, imm>;
}

multiclass BinaryNCOpMCi32Vector<ILOpCode opc, SDNode opn> {
  defm v2i8  : BinaryNCOpExpand<opc, opn, GPRV2I8, v2i8imm, v2i8, imm>;
  defm v2i16 : BinaryNCOpExpand<opc, opn, GPRV2I16, v2i16imm, v2i16, imm>;
  defm v2i32 : BinaryNCOpExpand<opc, opn, GPRV2I32, v2i32imm, v2i32, imm>;
  defm v4i8  : BinaryNCOpExpand<opc, opn, GPRV4I8, v4i8imm, v4i8, imm>;
  defm v4i16 : BinaryNCOpExpand<opc, opn, GPRV4I16, v4i16imm, v4i16, imm>;
  defm v4i32 : BinaryNCOpExpand<opc, opn, GPRV4I32, v4i32imm, v4i32, imm>;
}

multiclass BinaryNCOpMCi64Scalar<ILOpCode opc, SDNode opn> {
  defm i64 : BinaryNCOpExpand<opc, opn, GPR_64, i64imm, i64, imm>;
}

multiclass BinaryNCOpMCi64Vector<ILOpCode opc, SDNode opn> {
  defm v2i64 : BinaryNCOpExpand<opc, opn, GPRV2I64, v2i64imm, v2i64, imm>;
}

multiclass BinaryNCOpMCf32Scalar<ILOpCode opc, SDNode opn> {
  defm f32 : BinaryNCOpExpand<opc, opn, GPR_32,  f32imm, f32, fpimm>;
}

multiclass BinaryNCOpMCf32Vector<ILOpCode opc, SDNode opn> {
  defm v2f32 : BinaryNCOpExpand<opc, opn, GPRV2I32, v2f32imm, v2f32, fpimm>;
  defm v4f32 : BinaryNCOpExpand<opc, opn, GPRV4I32, v4f32imm, v4f32, fpimm>;
}


multiclass BinaryNCOpMCf64Scalar<ILOpCode opc, SDNode opn> {
  defm f64 : BinaryNCOpExpand<opc, opn, GPR_64, f64imm, f64, fpimm>;
}

multiclass BinaryNCOpMCf64Vector<ILOpCode opc, SDNode opn> {
  defm v2f64 : BinaryNCOpExpand<opc, opn, GPRV2I64, v2f64imm, v2f64, fpimm>;
}

multiclass BinaryNCOpMCi32<ILOpCode opc, SDNode opn> :
  BinaryNCOpMCi32Scalar<opc, opn>,
  BinaryNCOpMCi32Vector<opc, opn>;

multiclass BinaryNCOpMCi64<ILOpCode opc, SDNode opn> :
  BinaryNCOpMCi64Scalar<opc, opn>,
  BinaryNCOpMCi64Vector<opc, opn>;

multiclass BinaryNCOpShift<ILOpCode opc, SDNode opn,
                           RegisterClass dreg, RegisterClass sreg,
                           Operand oper, ValueType type,
                           ValueType stype> {
  def rr : ILFormat<opc,
    (outs dreg:$dst),
    (ins dreg:$src0, sreg:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (type dreg:$dst),
        (opn (type dreg:$src0), (stype sreg:$src1)))]>;
  def ri : ILFormat<opc,
    (outs dreg:$dst),
    (ins dreg:$src0, oper:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (type dreg:$dst),
    (opn (type dreg:$src0), (stype imm:$src1)))]>;
}

multiclass BinaryNCOpMCi32Shift<ILOpCode opc, SDNode opn> {
  defm i8i8     : BinaryNCOpShift<opc, opn, GPRI8,    GPRI8,    i8imm,    i8,    i8>;
  defm v2i8i8   : BinaryNCOpShift<opc, opn, GPRV2I8,  GPRV2I8,  v2i8imm,  v2i8,  v2i8>;
  defm v4i8i8   : BinaryNCOpShift<opc, opn, GPRV4I8,  GPRV4I8,  v4i8imm,  v4i8,  v4i8>;
  defm i16i16   : BinaryNCOpShift<opc, opn, GPRI16,   GPRI16,   i16imm,   i16,   i16>;
  defm v2i16i16 : BinaryNCOpShift<opc, opn, GPRV2I16, GPRV2I16, v2i16imm, v2i16, v2i16>;
  defm v4i16i16 : BinaryNCOpShift<opc, opn, GPRV4I16, GPRV4I16, v4i16imm, v4i16, v4i16>;
  defm i8i32    : BinaryNCOpShift<opc, opn, GPRI8,    GPR_32,   i32imm,   i8,    i32>;
  defm v2i8i32  : BinaryNCOpShift<opc, opn, GPRV2I8,  GPRV2I32, v2i32imm, v2i8,  v2i32>;
  defm v4i8i32  : BinaryNCOpShift<opc, opn, GPRV4I8,  GPRV4I32, v4i32imm, v4i8,  v4i32>;
  defm i16i32   : BinaryNCOpShift<opc, opn, GPRI16,   GPR_32,   i32imm,   i16,   i32>;
  defm v2i16i32 : BinaryNCOpShift<opc, opn, GPRV2I16, GPRV2I32, v2i32imm, v2i16, v2i32>;
  defm v4i16i32 : BinaryNCOpShift<opc, opn, GPRV4I16, GPRV4I32, v4i32imm, v4i16, v4i32>;
  defm i32i32   : BinaryNCOpShift<opc, opn, GPR_32,   GPR_32,   i32imm,   i32,   i32>;
  defm v2i32i32 : BinaryNCOpShift<opc, opn, GPRV2I32, GPRV2I32, v2i32imm, v2i32, v2i32>;
  defm v4i32i32 : BinaryNCOpShift<opc, opn, GPRV4I32, GPRV4I32, v4i32imm, v4i32, v4i32>;
  defm i8i64    : BinaryNCOpShift<opc, opn, GPRI8,    GPR_64,   i64imm,   i8,    i64>;
  defm v2i8i64  : BinaryNCOpShift<opc, opn, GPRV2I8,  GPRV2I64, v2i64imm, v2i8,  v2i64>;
  defm i16i64   : BinaryNCOpShift<opc, opn, GPRI16,   GPR_64,   i64imm,   i16,   i64>;
  defm v2i16i64 : BinaryNCOpShift<opc, opn, GPRV2I16, GPRV2I64, v2i64imm, v2i16, v2i64>;
  defm i32i64   : BinaryNCOpShift<opc, opn, GPR_32,   GPR_64,   i64imm,   i32,   i64>;
  defm v2i32i64 : BinaryNCOpShift<opc, opn, GPRV2I32, GPRV2I64, v2i64imm, v2i32, v2i64>;
}

multiclass BinaryNCOpMCi64Shift<ILOpCode opc, SDNode opn> {
  defm i64i64   : BinaryNCOpShift<opc, opn, GPR_64,   GPR_64,   i64imm,   i64,   i64>;
  defm v2i64i64 : BinaryNCOpShift<opc, opn, GPRV2I64, GPRV2I64, v2i64imm, v2i64, v2i64>;
  defm i64i32   : BinaryNCOpShift<opc, opn, GPR_64,   GPR_32,   i32imm,   i64,   i32>;
  defm v2i64i32 : BinaryNCOpShift<opc, opn, GPRV2I64, GPRV2I32, v2i32imm, v2i64, v2i32>;
}

multiclass BinaryNCOpMCf32<ILOpCode opc, SDNode opn> :
  BinaryNCOpMCf32Scalar<opc, opn>,
  BinaryNCOpMCf32Vector<opc, opn>;

multiclass BinaryNCOpMCf64<ILOpCode opc, SDNode opn> :
  BinaryNCOpMCf64Scalar<opc, opn>,
  BinaryNCOpMCf64Vector<opc, opn>;

multiclass BinaryNCOpMCInt<ILOpCode opc, SDNode opn> :
  BinaryNCOpMCi32<opc, opn>,
  BinaryNCOpMCi64<opc, opn>;

multiclass BinaryNCOpMCFP<ILOpCode opc, SDNode opn> :
  BinaryNCOpMCf32<opc, opn>,
  BinaryNCOpMCf64<opc, opn>;

multiclass BinaryNCOpMC<ILOpCode opc, SDNode opn> :
  BinaryNCOpMCInt<opc, opn>,
  BinaryNCOpMCFP<opc, opn>;

//-------------------------------------------------------------------//
// Multiclasses that deal with two operand intrinsics.
//-------------------------------------------------------------------//

multiclass BinaryIntMCFull<ILOpCode opc, Intrinsic opn,
           ValueType dtype, RegisterClass dreg,
           ValueType stype0, RegisterClass sreg0, Operand oper0, SDNode imm0,
           ValueType stype1, RegisterClass sreg1, Operand oper1, SDNode imm1> {
  def rr : ILFormat<opc, (outs dreg:$dst),
    (ins sreg0:$src0, sreg1:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (dtype dreg:$dst),
        (opn (stype0 sreg0:$src0), (stype1 sreg1:$src1)))]>;
  def ri : ILFormat<opc, (outs dreg:$dst),
    (ins sreg0:$src0, oper1:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (dtype dreg:$dst),
        (opn (stype0 sreg0:$src0), (stype1 imm1:$src1)))]>;
  def ir : ILFormat<opc, (outs dreg:$dst),
    (ins oper0:$src0, sreg1:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (dtype dreg:$dst),
      (opn (stype0 imm0:$src0), (stype1 sreg1:$src1)))]>;
  def ii : ILFormat<opc, (outs dreg:$dst),
    (ins oper0:$src0, oper1:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (dtype dreg:$dst),
        (opn (stype0 imm0:$src0), (stype1 imm1:$src1)))]>;
}

multiclass BinaryIntMCFFI<ILOpCode opc, Intrinsic opn> {
  defm f32 : BinaryIntMCFull<opc, opn, f32, GPR_32, f32, GPR_32, f32imm, fpimm, i32, GPR_32, i32imm, imm>;
  defm v2f32 : BinaryIntMCFull<opc, opn, v2f32, GPRV2I32, v2f32, GPRV2I32, v2f32imm, fpimm, v2i32, GPRV2I32, v2i32imm, imm>;
  defm v4f32 : BinaryIntMCFull<opc, opn, v4f32, GPRV4I32, v4f32, GPRV4I32, v4f32imm, fpimm, v4i32, GPRV4I32, v4i32imm, imm>;
}


multiclass BinaryIntMCDDI<ILOpCode opc, Intrinsic opn> {
  defm f64 : BinaryIntMCFull<opc, opn, f64, GPR_64, f64, GPR_64, f64imm, fpimm, i32, GPR_32, i32imm, imm>;
  defm v2f64 : BinaryIntMCFull<opc, opn, v2f64, GPRV2I64, v2f64, GPRV2I64, v2f64imm, fpimm, v2i32, GPRV2I32, v2i32imm, imm>;
}

multiclass BinaryIntReg<ILOpCode opc, Intrinsic opn,
           RegisterClass reg, Operand oper, ValueType type,
           SDNode imm0> {
  def rr : ILFormat<opc, (outs reg:$dst), (ins reg:$src0, reg:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (type reg:$dst),
        (opn (type reg:$src0), (type reg:$src1)))]>;
  def ri : ILFormat<opc, (outs reg:$dst), (ins reg:$src0, oper:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (type reg:$dst),
      (opn (type reg:$src0), (type imm0:$src1)))]>;
  def ii : ILFormat<opc, (outs reg:$dst), (ins oper:$src0, oper:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (type reg:$dst),
      (opn (type imm0:$src0), (type imm0:$src1)))]>;
}

multiclass BinaryIntExpand<ILOpCode opc, Intrinsic opn,
                           RegisterClass reg, Operand oper,
                           ValueType type> :
  BinaryIntReg<opc, opn, reg, oper, type, imm>;

multiclass BinaryIntExpandFP<ILOpCode opc, Intrinsic opn,
                             RegisterClass reg, Operand oper,
                             ValueType type> :
  BinaryIntReg<opc, opn, reg, oper, type, fpimm>;

multiclass BinaryIntMCi8Scalar<ILOpCode opc, Intrinsic opn> {
  defm i8 : BinaryIntExpand<opc, opn, GPRI8, i8imm, i8>;
}

multiclass BinaryIntMCi8Vector<ILOpCode opc, Intrinsic opn> {
  defm v2i8 : BinaryIntExpand<opc, opn, GPRV2I8, v2i8imm, v2i8>;
  defm v4i8 : BinaryIntExpand<opc, opn, GPRV4I8, v4i8imm, v4i8>;
}

multiclass BinaryIntMCi16Scalar<ILOpCode opc, Intrinsic opn> {
  defm i16 : BinaryIntExpand<opc, opn, GPRI16, i16imm, i16>;
}

multiclass BinaryIntMCi16Vector<ILOpCode opc, Intrinsic opn> {
  defm v2i16 : BinaryIntExpand<opc, opn, GPRV2I16, v2i16imm, v2i16>;
  defm v4i16 : BinaryIntExpand<opc, opn, GPRV4I16, v4i16imm, v4i16>;
}

multiclass BinaryIntMCi32Scalar<ILOpCode opc, Intrinsic opn> {
  defm i32 : BinaryIntExpand<opc, opn, GPR_32,  i32imm, i32>;
}

multiclass BinaryIntMCi32Vector<ILOpCode opc, Intrinsic opn> {
  defm v2i32 : BinaryIntExpand<opc, opn, GPRV2I32, v2i32imm, v2i32>;
  defm v4i32 : BinaryIntExpand<opc, opn, GPRV4I32, v4i32imm, v4i32>;
}

multiclass BinaryIntMCi64Scalar<ILOpCode opc, Intrinsic opn> {
  defm i64 : BinaryIntExpand<opc, opn, GPR_64,  i64imm, i64>;
}

multiclass BinaryIntMCi64Vector<ILOpCode opc, Intrinsic opn> {
  defm v2i64 : BinaryIntExpand<opc, opn, GPRV2I64, v2i64imm, v2i64>;
}

multiclass BinaryIntMCf32Scalar<ILOpCode opc, Intrinsic opn> {
  defm f32 : BinaryIntExpandFP<opc, opn, GPR_32, f32imm, f32>;
}

multiclass BinaryIntMCf32Vector<ILOpCode opc, Intrinsic opn> {
  defm v2f32 : BinaryIntExpandFP<opc, opn, GPRV2I32, v2f32imm, v2f32>;
  defm v4f32 : BinaryIntExpandFP<opc, opn, GPRV4I32, v4f32imm, v4f32>;
}

multiclass BinaryIntMCf64Scalar<ILOpCode opc, Intrinsic opn> {
  defm f64 : BinaryIntExpandFP<opc, opn, GPR_64,  f64imm, f64>;
}

multiclass BinaryIntMCf64Vector<ILOpCode opc, Intrinsic opn> {
  defm v2f64 : BinaryIntExpandFP<opc, opn, GPRV2I64, v2f64imm, v2f64>;
}

multiclass BinaryIntMCi32Only<ILOpCode opc, Intrinsic opn> :
  BinaryIntMCi32Scalar<opc, opn>,
  BinaryIntMCi32Vector<opc, opn>;

multiclass BinaryIntMCi32<ILOpCode opc, Intrinsic opn> :
  BinaryIntMCi32Scalar<opc, opn>,
  BinaryIntMCi32Vector<opc, opn>,
  BinaryIntMCi16Scalar<opc, opn>,
  BinaryIntMCi16Vector<opc, opn>,
  BinaryIntMCi8Scalar< opc, opn>,
  BinaryIntMCi8Vector< opc, opn>;

multiclass BinaryIntMCi64<ILOpCode opc, Intrinsic opn> :
  BinaryIntMCi64Scalar<opc, opn>,
  BinaryIntMCi64Vector<opc, opn>;

multiclass BinaryIntMCf32<ILOpCode opc, Intrinsic opn> :
  BinaryIntMCf32Scalar<opc, opn>,
  BinaryIntMCf32Vector<opc, opn>;

multiclass BinaryIntMCf64<ILOpCode opc, Intrinsic opn> :
  BinaryIntMCf64Scalar<opc, opn>,
  BinaryIntMCf64Vector<opc, opn>;

multiclass BinaryIntMCInt<ILOpCode opc, Intrinsic opn> :
  BinaryIntMCi32<opc, opn>,
  BinaryIntMCi64<opc, opn>;

multiclass BinaryIntMCFP<ILOpCode opc, Intrinsic opn> :
  BinaryIntMCf32<opc, opn>,
  BinaryIntMCf64<opc, opn>;

multiclass BinaryIntMC<ILOpCode opc, Intrinsic opn> :
  BinaryIntMCInt<opc, opn>,
  BinaryIntMCFP<opc, opn>;

//-------------------------------------------------------------------//
// Multiclasses that deal with two operand Pattern Fragments.
//-------------------------------------------------------------------//
multiclass BinaryPatReg<ILOpCode opc, PatFrag opn,
           RegisterClass reg, Operand oper, ValueType type,
           SDNode imm0> {
  def rr : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (type reg:$dst),
        (opn (type reg:$src0), (type reg:$src1)))]>;
  def ri : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, oper:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (type reg:$dst),
      (opn (type reg:$src0), (type imm0:$src1)))]>;
  def ii : ILFormat<opc,
    (outs reg:$dst),
    (ins oper:$src0, oper:$src1),
    !strconcat(opc.Text, " $dst, $src0, $src1"),
    [(set (type reg:$dst),
      (opn (type imm0:$src0), (type imm0:$src1)))]>;
}

multiclass BinaryPatExpand<ILOpCode opc, PatFrag opn,
                           RegisterClass reg,
                           Operand oper, ValueType type> :
  BinaryPatReg<opc, opn, reg, oper, type, imm>;

multiclass BinaryPatExpandFP<ILOpCode opc, PatFrag opn,
                             RegisterClass reg, Operand oper,
                             ValueType type> :
  BinaryPatReg<opc, opn, reg, oper, type, fpimm>;

multiclass BinaryPatMCi8Scalar<ILOpCode opc, PatFrag opn> {
  defm i8 : BinaryPatExpand<opc, opn, GPRI8, i8imm, i8>;
}

multiclass BinaryPatMCi8Vector<ILOpCode opc, PatFrag opn> {
  defm v2i8 : BinaryPatExpand<opc, opn, GPRV2I8,  v2i8imm, v2i8>;
  defm v4i8 : BinaryPatExpand<opc, opn, GPRV4I8,  v4i8imm, v4i8>;
}

multiclass BinaryPatMCi16Scalar<ILOpCode opc, PatFrag opn> {
  defm i16 : BinaryPatExpand<opc, opn, GPRI16,  i16imm, i16>;
}

multiclass BinaryPatMCi16Vector<ILOpCode opc, PatFrag opn> {
  defm v2i16 : BinaryPatExpand<opc, opn, GPRV2I16, v2i16imm, v2i16>;
  defm v4i16 : BinaryPatExpand<opc, opn, GPRV4I16, v4i16imm, v4i16>;
}

multiclass BinaryPatMCi32Scalar<ILOpCode opc, PatFrag opn> {
  defm i32 : BinaryPatExpand<opc, opn, GPR_32,  i32imm, i32>;
}

multiclass BinaryPatMCi32Vector<ILOpCode opc, PatFrag opn> {
  defm v2i32 : BinaryPatExpand<opc, opn, GPRV2I32, v2i32imm, v2i32>;
  defm v4i32 : BinaryPatExpand<opc, opn, GPRV4I32, v4i32imm, v4i32>;
}

multiclass BinaryPatMCi64Scalar<ILOpCode opc, PatFrag opn> {
  defm i64 : BinaryPatExpand<opc, opn, GPR_64, i64imm, i64>;
}

multiclass BinaryPatMCi64Vector<ILOpCode opc, PatFrag opn> {
  defm v2i64 : BinaryPatExpand<opc, opn, GPRV2I64, v2i64imm, v2i64>;
}

multiclass BinaryPatMCf32Scalar<ILOpCode opc, PatFrag opn> {
  defm f32 : BinaryPatExpandFP<opc, opn, GPR_32, f32imm, f32>;
}

multiclass BinaryPatMCf32Vector<ILOpCode opc, PatFrag opn> {
  defm v2f32 : BinaryPatExpandFP<opc, opn, GPRV2I32,  v2f32imm, v2f32>;
  defm v4f32 : BinaryPatExpandFP<opc, opn, GPRV4I32, v4f32imm, v4f32>;
}


multiclass BinaryPatMCf64Scalar<ILOpCode opc, PatFrag opn> {
  defm f64 : BinaryPatExpandFP<opc, opn, GPR_64,  f64imm, f64>;
}

multiclass BinaryPatMCf64Vector<ILOpCode opc, PatFrag opn> {
  defm v2f64 : BinaryPatExpandFP<opc, opn, GPRV2I64, v2f64imm, v2f64>;
}

multiclass BinaryPatMCi32Only<ILOpCode opc, PatFrag opn> :
  BinaryPatMCi32Scalar<opc, opn>,
  BinaryPatMCi32Vector<opc, opn>;

multiclass BinaryPatMCi32<ILOpCode opc, PatFrag opn> :
  BinaryPatMCi32Scalar<opc, opn>,
  BinaryPatMCi32Vector<opc, opn>,
  BinaryPatMCi16Scalar<opc, opn>,
  BinaryPatMCi16Vector<opc, opn>,
  BinaryPatMCi8Scalar< opc, opn>,
  BinaryPatMCi8Vector< opc, opn>;

multiclass BinaryPatMCi64<ILOpCode opc, PatFrag opn> :
  BinaryPatMCi64Scalar<opc, opn>,
  BinaryPatMCi64Vector<opc, opn>;

multiclass BinaryPatMCf32<ILOpCode opc, PatFrag opn> :
  BinaryPatMCf32Scalar<opc, opn>,
  BinaryPatMCf32Vector<opc, opn>;

multiclass BinaryPatMCf64<ILOpCode opc, PatFrag opn> :
  BinaryPatMCf64Scalar<opc, opn>,
  BinaryPatMCf64Vector<opc, opn>;

multiclass BinaryPatMCInt<ILOpCode opc, PatFrag opn> :
  BinaryPatMCi32<opc, opn>,
  BinaryPatMCi64<opc, opn>;

multiclass BinaryPatMCFP<ILOpCode opc, PatFrag opn> :
  BinaryPatMCf32<opc, opn>,
  BinaryPatMCf64<opc, opn>;

multiclass BinaryPatMC<ILOpCode opc, PatFrag opn> :
  BinaryPatMCInt<opc, opn>,
  BinaryPatMCFP<opc, opn>;

//----------------------------------------------------------------------------//
// Multiclasses that deal with three operand instructions.
//----------------------------------------------------------------------------//
multiclass TernaryOpReg<ILOpCode opc, SDNode opn,
           RegisterClass reg, Operand oper,
           ValueType type, SDNode imm0> {
  def rrr : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (type reg:$src0), (type reg:$src1), (type reg:$src2)))]>;
  def iii : ILFormat<opc,
    (outs reg:$dst),
    (ins oper:$src0, oper:$src1, oper:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (type imm0:$src0), (type imm0:$src1), (type imm0:$src2)))]>;
  def irr : ILFormat<opc,
    (outs reg:$dst),
    (ins oper:$src0, reg:$src1, reg:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (type imm0:$src0), (type reg:$src1), (type reg:$src2)))]>;
  def rir : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, oper:$src1, reg:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (type reg:$src0), (type imm0:$src1), (type reg:$src2)))]>;
  def rri : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, oper:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (type reg:$src0), (type reg:$src1), (type imm0:$src2)))]>;
  def iir : ILFormat<opc,
    (outs reg:$dst),
    (ins oper:$src0, oper:$src1, reg:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (type imm0:$src0), (type imm0:$src1), (type reg:$src2)))]>;
  def iri : ILFormat<opc,
    (outs reg:$dst),
    (ins oper:$src0, reg:$src1, oper:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (type imm0:$src0), (type reg:$src1), (type imm0:$src2)))]>;
  def rii : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, oper:$src1, oper:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (type reg:$src0), (type imm0:$src1), (type imm0:$src2)))]>;
}

multiclass TernaryOpExpand<ILOpCode opc, SDNode opn,
                           RegisterClass reg, Operand oper,
                           ValueType type> :
  TernaryOpReg<opc, opn, reg, oper, type, imm>;

multiclass TernaryOpExpandFP<ILOpCode opc, SDNode opn,
                             RegisterClass reg,
                             Operand oper, ValueType type> :
  TernaryOpReg<opc, opn, reg, oper, type, fpimm>;


multiclass TernaryOpMCi8Scalar<ILOpCode opc, SDNode opn> {
  defm i8 : TernaryOpExpand<opc, opn, GPRI8, i8imm, i8>;
}

multiclass TernaryOpMCi8Vector<ILOpCode opc, SDNode opn> {
  defm v2i8 : TernaryOpExpand<opc, opn, GPRV2I8, v2i8imm, v2i8>;
  defm v4i8 : TernaryOpExpand<opc, opn, GPRV4I8, v4i8imm, v4i8>;
}

multiclass TernaryOpMCi16Scalar<ILOpCode opc, SDNode opn> {
  defm i16 : TernaryOpExpand<opc, opn, GPRI16, i16imm, i16>;
}

multiclass TernaryOpMCi16Vector<ILOpCode opc, SDNode opn> {
  defm v2i16 : TernaryOpExpand<opc, opn, GPRV2I16, v2i16imm, v2i16>;
  defm v4i16 : TernaryOpExpand<opc, opn, GPRV4I16, v4i16imm, v4i16>;
}

multiclass TernaryOpMCi32Scalar<ILOpCode opc, SDNode opn> {
  defm i32 : TernaryOpExpand<opc, opn, GPR_32, i32imm, i32>;
}

multiclass TernaryOpMCi32Vector<ILOpCode opc, SDNode opn> {
  defm v2i32 : TernaryOpExpand<opc, opn, GPRV2I32, v2i32imm, v2i32>;
  defm v4i32 : TernaryOpExpand<opc, opn, GPRV4I32, v4i32imm, v4i32>;
}

multiclass TernaryOpMCi64Scalar<ILOpCode opc, SDNode opn> {
  defm i64 : TernaryOpExpand<opc, opn, GPR_64, i64imm, i64>;
}

multiclass TernaryOpMCi64Vector<ILOpCode opc, SDNode opn> {
  defm v2i64 : TernaryOpExpand<opc, opn, GPRV2I64, v2i64imm, v2i64>;
}

multiclass TernaryOpMCf32Scalar<ILOpCode opc, SDNode opn> {
  defm f32 : TernaryOpExpandFP<opc, opn, GPR_32, f32imm, f32>;
}

multiclass TernaryOpMCf32Vector<ILOpCode opc, SDNode opn> {
  defm v2f32 : TernaryOpExpandFP<opc, opn, GPRV2I32, v2f32imm, v2f32>;
  defm v4f32 : TernaryOpExpandFP<opc, opn, GPRV4I32, v4f32imm, v4f32>;
}

multiclass TernaryOpMCf64Scalar<ILOpCode opc, SDNode opn> {
  defm f64 : TernaryOpExpandFP<opc, opn, GPR_64,  f64imm, f64>;
}

multiclass TernaryOpMCf64Vector<ILOpCode opc, SDNode opn> {
  defm v2f64 : TernaryOpExpandFP<opc, opn, GPRV2I64, v2f64imm, v2f64>;
}

multiclass TernaryOpMCi32<ILOpCode opc, SDNode opn> :
  TernaryOpMCi32Scalar<opc, opn>,
  TernaryOpMCi32Vector<opc, opn>,
  TernaryOpMCi16Scalar<opc, opn>,
  TernaryOpMCi16Vector<opc, opn>,
  TernaryOpMCi8Scalar< opc, opn>,
  TernaryOpMCi8Vector< opc, opn>;

multiclass TernaryOpMCi64<ILOpCode opc, SDNode opn> :
  TernaryOpMCi64Scalar<opc, opn>,
  TernaryOpMCi64Vector<opc, opn>;

multiclass TernaryOpMCf32<ILOpCode opc, SDNode opn> :
  TernaryOpMCf32Scalar<opc, opn>,
  TernaryOpMCf32Vector<opc, opn>;

multiclass TernaryOpMCf64<ILOpCode opc, SDNode opn> :
  TernaryOpMCf64Scalar<opc, opn>,
  TernaryOpMCf64Vector<opc, opn>;

multiclass TernaryOpMCInt<ILOpCode opc, SDNode opn> :
  TernaryOpMCi32<opc, opn>,
  TernaryOpMCi64<opc, opn>;

multiclass TernaryOpMCFP<ILOpCode opc, SDNode opn> :
  TernaryOpMCf32<opc, opn>,
  TernaryOpMCf64<opc, opn>;

multiclass TernaryOpMC<ILOpCode opc, SDNode opn> :
  TernaryOpMCInt<opc, opn>,
  TernaryOpMCFP<opc, opn>;
//-------------------------------------------------------------------//
// Multiclasses that deal with the select instruction.
//-------------------------------------------------------------------//
multiclass SelectReg<ILOpCode opc, SDNode opn,
           RegisterClass creg, Operand coper, ValueType ctype,
           RegisterClass reg, Operand oper, ValueType type,
           SDNode imm0> {
  def rrr : ILFormat<opc,
    (outs reg:$dst),
    (ins creg:$src0, reg:$src1, reg:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (ctype creg:$src0), (type reg:$src1), (type reg:$src2)))]>;
  /*
  def iii : ILFormat<opc,
    (outs reg:$dst),
    (ins coper:$src0, oper:$src1, oper:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (ctype imm:$src0), (type imm0:$src1), (type imm0:$src2)))]>;
  def irr : ILFormat<opc,
    (outs reg:$dst),
    (ins coper:$src0, reg:$src1, reg:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (ctype imm:$src0), (type reg:$src1), (type reg:$src2)))]>;
  def rir : ILFormat<opc,
    (outs reg:$dst),
    (ins creg:$src0, oper:$src1, reg:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (ctype creg:$src0), (type imm0:$src1), (type reg:$src2)))]>;
  def rri : ILFormat<opc,
    (outs reg:$dst),
    (ins creg:$src0, reg:$src1, oper:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (ctype creg:$src0), (type reg:$src1), (type imm0:$src2)))]>;
  def iir : ILFormat<opc,
    (outs reg:$dst),
    (ins coper:$src0, oper:$src1, reg:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (ctype imm:$src0), (type imm0:$src1), (type reg:$src2)))]>;
  def iri : ILFormat<opc,
    (outs reg:$dst),
    (ins coper:$src0, reg:$src1, oper:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (ctype imm:$src0), (type reg:$src1), (type imm0:$src2)))]>;
*/
  def rii : ILFormat<opc,
    (outs reg:$dst),
    (ins creg:$src0, oper:$src1, oper:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
        (opn (ctype creg:$src0), (type imm0:$src1), (type imm0:$src2)))]>;
}

multiclass SelectMC<ILOpCode opc> {
  defm i8 : SelectReg<opc, select, GPR_32, i32imm, i32, GPRI8, i8imm, i8, imm>;
  defm i16 : SelectReg<opc, select, GPR_32, i32imm, i32, GPRI16, i16imm, i16, imm>;
  defm i32 : SelectReg<opc, select, GPR_32, i32imm, i32, GPR_32, i32imm, i32, imm>;
  defm f32 : SelectReg<opc, select, GPR_32, i32imm, i32, GPR_32, f32imm, f32, fpimm>;
  let swizzle = 2 in {
    defm i64 : SelectReg<opc, select, GPR_64, i64imm, i64, GPR_64, i64imm, i64, imm>;
    defm f64 : SelectReg<opc, select, GPR_64, i64imm, i64, GPR_64, f64imm, f64, fpimm>;
  }
  defm v2i8 : SelectReg<opc, vselect, GPRV2I32, v2i32imm, v2i32, GPRV2I8, v2i8imm, v2i8, imm>;
  defm v2i16 : SelectReg<opc, vselect, GPRV2I32, v2i32imm, v2i32, GPRV2I16, v2i16imm, v2i16, imm>;
  defm v2i32 : SelectReg<opc, vselect, GPRV2I32, v2i32imm, v2i32, GPRV2I32, v2i32imm, v2i32, imm>;
  defm v2f32 : SelectReg<opc, vselect, GPRV2I32, v2i32imm, v2i32, GPRV2I32, v2f32imm, v2f32, fpimm>;
  let swizzle = 2 in {
    defm v2i64 : SelectReg<opc, vselect, GPRV2I64, v2i64imm, v2i64, GPRV2I64, v2i64imm, v2i64, imm>;
    defm v2f64 : SelectReg<opc, vselect, GPRV2I64, v2i64imm, v2i64, GPRV2I64, v2f64imm, v2f64, fpimm>;
  }
  defm v4i8 : SelectReg<opc, vselect, GPRV4I32, v4i32imm, v4i32, GPRV4I8, v4i8imm,  v4i8, imm>;
  defm v4i16 : SelectReg<opc, vselect, GPRV4I32, v4i32imm, v4i32, GPRV4I16, v4i16imm, v4i16, imm>;
  defm v4i32 : SelectReg<opc, vselect, GPRV4I32, v4i32imm, v4i32, GPRV4I32, v4i32imm, v4i32, imm>;
  defm v4f32 : SelectReg<opc, vselect, GPRV4I32, v4i32imm, v4i32, GPRV4I32, v4f32imm, v4f32, fpimm>;
}

//-------------------------------------------------------------------//
// Multiclasses that deal with two operand Pattern Fragments.
//-------------------------------------------------------------------//

multiclass BinaryBaseCmp<ILOpCode opc,
                         PatLeaf cond,
                         ValueType vt,
                         ValueType dtype,
                         RegisterClass rc,
                         SDNode immType> {
  let isCompare = 1 in {
    def rr : ILFormat<opc, (outs rc:$dst),
      (ins rc:$src0, rc:$src1),
      !strconcat(opc.Text, " $dst, $src0, $src1"),
      [(set (dtype rc:$dst),
        (dtype (setcc (vt rc:$src0), (vt rc:$src1), cond)))]>;

    def ri : ILFormat<opc, (outs rc:$dst),
      (ins rc:$src0, rc:$src1),
      !strconcat(opc.Text, " $dst, $src0, $src1"),
      [(set (dtype rc:$dst),
         (dtype (setcc (vt rc:$src0), (vt immType:$src1), cond)))]>;

    def ir : ILFormat<opc, (outs rc:$dst),
      (ins rc:$src0, rc:$src1),
      !strconcat(opc.Text, " $dst, $src0, $src1"),
      [(set (dtype rc:$dst),
         (dtype (setcc (vt immType:$src0), (vt rc:$src1), cond)))]>;

    def ii : ILFormat<opc, (outs rc:$dst),
      (ins rc:$src0, rc:$src1),
      !strconcat(opc.Text, " $dst, $src0, $src1"),
      [(set (dtype rc:$dst),
        (dtype (setcc (vt immType:$src0), (vt immType:$src1), cond)))]>;
  }
}

multiclass BinaryCmpf32<ILOpCode opc, PatLeaf cond> {
  defm f32 : BinaryBaseCmp<opc, cond, f32, i32, GPR_32, fpimm>;
  defm v2f32 : BinaryBaseCmp<opc, cond, v2f32, v2i32, GPRV2I32, fpimm>;
  defm v4f32 : BinaryBaseCmp<opc, cond, v4f32, v4i32, GPRV4I32, fpimm>;
}

multiclass BinaryCmpf64<ILOpCode opc, PatLeaf cond> {
  defm f64 : BinaryBaseCmp<opc, cond, f64, i64, GPR_64, fpimm>;
  defm v2f64 : BinaryBaseCmp<opc, cond, v2f64, v2i64, GPRV2I64, fpimm>;
}

multiclass BinaryCmpi32<ILOpCode opc, PatLeaf cond> {
  defm i32 : BinaryBaseCmp<opc, cond, i32, i32, GPR_32, imm>;
  defm v2i32 : BinaryBaseCmp<opc, cond, v2i32, v2i32, GPRV2I32, imm>;
  defm v4i32 : BinaryBaseCmp<opc, cond, v4i32, v4i32, GPRV4I32, imm>;
}

multiclass BinaryCmpi64<ILOpCode opc, PatLeaf cond> {
  defm i64 : BinaryBaseCmp<opc, cond, i64, i64, GPR_64, imm>;
  defm v2i64 : BinaryBaseCmp<opc, cond, v2i64, v2i64, GPRV2I64, imm>;
}

/**/
//----------------------------------------------------------------------------//
// Multiclasses that deal with three operand intrinsics.
//----------------------------------------------------------------------------//

multiclass TernaryIntMCFull<ILOpCode opc, Intrinsic opn,
           ValueType dtype, RegisterClass dreg,
           ValueType stype0, RegisterClass sreg0, Operand oper0, SDNode imm0,
           ValueType stype1, RegisterClass sreg1, Operand oper1, SDNode imm1,
           ValueType stype2, RegisterClass sreg2, Operand oper2, SDNode imm2> {
  def rrr : ILFormat<opc, (outs dreg:$dst),
    (ins sreg0:$src0, sreg1:$src1, sreg2:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (dtype dreg:$dst),
        (opn (stype0 sreg0:$src0), (stype1 sreg1:$src1),
         (stype2 sreg2:$src2)))]>;
  def iii : ILFormat<opc, (outs dreg:$dst),
    (ins oper0:$src0, oper1:$src1, oper2:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (dtype dreg:$dst),
        (opn (stype0 imm0:$src0), (stype1 imm1:$src1),
         (stype2 imm2:$src2)))]>;
  def irr : ILFormat<opc, (outs dreg:$dst),
    (ins oper0:$src0, sreg1:$src1, sreg2:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (dtype dreg:$dst),
      (opn (stype0 imm0:$src0), (stype1 sreg1:$src1),
       (stype2 sreg2:$src2)))]>;
  def rir : ILFormat<opc, (outs dreg:$dst),
    (ins sreg0:$src0, oper1:$src1, sreg2:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (dtype dreg:$dst),
      (opn (stype0 sreg0:$src0), (stype1 imm1:$src1),
       (stype2 sreg2:$src2)))]>;
  def rri : ILFormat<opc, (outs dreg:$dst),
    (ins sreg0:$src0, sreg1:$src1, oper2:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (dtype dreg:$dst),
      (opn (stype0 sreg0:$src0), (stype1 sreg1:$src1),
       (stype2 imm2:$src2)))]>;
  def iir : ILFormat<opc, (outs dreg:$dst),
    (ins oper0:$src0, oper1:$src1, sreg2:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (dtype dreg:$dst),
      (opn (stype0 imm0:$src0), (stype1 imm1:$src1),
       (stype2 sreg2:$src2)))]>;
  def iri : ILFormat<opc, (outs dreg:$dst),
    (ins oper0:$src0, sreg1:$src1, oper2:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (dtype dreg:$dst),
      (opn (stype0 imm0:$src0), (stype1 sreg1:$src1),
       (stype2 imm2:$src2)))]>;
  def rii : ILFormat<opc, (outs dreg:$dst),
    (ins sreg0:$src0, oper1:$src1, oper2:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (dtype dreg:$dst),
      (opn (stype0 sreg0:$src0), (stype1 imm1:$src1),
       (stype2 imm2:$src2)))]>;
}

multiclass TernaryIntLLIL64Scalar<ILOpCode opc, Intrinsic opn> {
  defm i64 : TernaryIntMCFull<opc, opn, i64, GPR_64, i64, GPR_64, i64imm, imm, i32, GPR_32, i32imm, imm, i64, GPR_64, i64imm, imm>;
}


//----------------------------------------------------------------------------//
// Multiclasses that deal with three operand pattern fragments.
//----------------------------------------------------------------------------//
multiclass TernaryPatReg<ILOpCode opc, SDPatternOperator opn,
           RegisterClass reg, ValueType type> {
  def rrr : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set (type reg:$dst),
      (opn (type reg:$src0), (type reg:$src1), (type reg:$src2)))]>;
}

multiclass TernaryNCPatReg<ILOpCode opc, SDPatternOperator opn,
           RegisterClass reg, Operand oper, ValueType type, SDNode imm0> :
  TernaryPatReg<opc, opn, reg, type> {
    def irr : ILFormat<opc,
      (outs reg:$dst),
      (ins oper:$src0, reg:$src1, reg:$src2),
      !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
      [(set (type reg:$dst),
        (opn (type imm0:$src0), (type reg:$src1), (type reg:$src2)))]>;
    def rri : ILFormat<opc,
      (outs reg:$dst),
      (ins reg:$src0, reg:$src1, oper:$src2),
      !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
      [(set (type reg:$dst),
        (opn (type reg:$src0), (type reg:$src1), (type imm0:$src2)))]>;
    def iir : ILFormat<opc,
      (outs reg:$dst),
      (ins oper:$src0, oper:$src1, reg:$src2),
      !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
      [(set (type reg:$dst),
        (opn (type imm0:$src0), (type imm0:$src1), (type reg:$src2)))]>;
    def iii : ILFormat<opc,
      (outs reg:$dst),
      (ins oper:$src0, oper:$src1, oper:$src2),
      !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
      [(set (type reg:$dst),
        (opn (type imm0:$src0), (type imm0:$src1), (type imm0:$src2)))]>;
    def rir : ILFormat<opc,
      (outs reg:$dst),
      (ins reg:$src0, oper:$src1, reg:$src2),
      !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
      [(set (type reg:$dst),
        (opn (type reg:$src0), (type imm0:$src1), (type reg:$src2)))]>;
    def iri : ILFormat<opc,
      (outs reg:$dst),
      (ins oper:$src0, reg:$src1, oper:$src2),
      !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
      [(set (type reg:$dst),
        (opn (type imm0:$src0), (type reg:$src1), (type imm0:$src2)))]>;
    def rii : ILFormat<opc,
      (outs reg:$dst),
      (ins reg:$src0, oper:$src1, oper:$src2),
      !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
      [(set (type reg:$dst),
        (opn (type reg:$src0), (type imm0:$src1), (type imm0:$src2)))]>;
}

multiclass TernaryPatExpand<ILOpCode opc, SDPatternOperator opn,
                            RegisterClass reg,
                            Operand oper, ValueType type> :
  TernaryNCPatReg<opc, opn, reg, oper, type, imm>;

multiclass TernaryPatExpandFP<ILOpCode opc, SDPatternOperator opn,
                              RegisterClass reg,
                              Operand oper, ValueType type> :
  TernaryNCPatReg<opc, opn, reg, oper, type, fpimm>;


multiclass TernaryPatMCi8Scalar<ILOpCode opc, SDPatternOperator opn> {
  defm i8 : TernaryPatExpand<opc, opn, GPRI8, i8imm, i8>;
}

multiclass TernaryPatMCi8Vector<ILOpCode opc, SDPatternOperator opn> {
  defm v2i8 : TernaryPatExpand<opc, opn, GPRV2I8, v2i8imm, v2i8>;
  defm v4i8 : TernaryPatExpand<opc, opn, GPRV4I8, v4i8imm, v4i8>;
}

multiclass TernaryPatMCi16Scalar<ILOpCode opc, SDPatternOperator opn> {
  defm i16 : TernaryPatExpand<opc, opn, GPRI16, i16imm, i16>;
}

multiclass TernaryPatMCi16Vector<ILOpCode opc, SDPatternOperator opn> {
  defm v2i16 : TernaryPatExpand<opc, opn, GPRV2I16, v2i16imm, v2i16>;
  defm v4i16 : TernaryPatExpand<opc, opn, GPRV4I16, v4i16imm, v4i16>;
}

multiclass TernaryPatMCi32Scalar<ILOpCode opc, SDPatternOperator opn> {
  defm i32 : TernaryPatExpand<opc, opn, GPR_32, i32imm, i32>;
}

multiclass TernaryPatMCi32Vector<ILOpCode opc, SDPatternOperator opn> {
  defm v2i32 : TernaryPatExpand<opc, opn, GPRV2I32, v2i32imm, v2i32>;
  defm v4i32 : TernaryPatExpand<opc, opn, GPRV4I32, v4i32imm, v4i32>;
}

multiclass TernaryPatMCi64Scalar<ILOpCode opc, SDPatternOperator opn> {
  defm i64 : TernaryPatExpand<opc, opn, GPR_64,  i64imm, i64>;
}

multiclass TernaryPatMCi64Vector<ILOpCode opc, SDPatternOperator opn> {
  defm v2i64 : TernaryPatExpand<opc, opn, GPRV2I64, v2i64imm, v2i64>;
}

multiclass TernaryPatMCf32Scalar<ILOpCode opc, SDPatternOperator opn> {
  defm f32 : TernaryPatExpandFP<opc, opn, GPR_32, f32imm, f32>;
}

multiclass TernaryPatMCf32Vector<ILOpCode opc, SDPatternOperator opn> {
  defm v2f32 : TernaryPatExpandFP<opc, opn, GPRV2I32,  v2f32imm, v2f32>;
  defm v4f32 : TernaryPatExpandFP<opc, opn, GPRV4I32, v4f32imm, v4f32>;
}

multiclass TernaryPatMCf64Scalar<ILOpCode opc, SDPatternOperator opn> {
  defm f64 : TernaryPatExpandFP<opc, opn, GPR_64, f64imm, f64>;
}

multiclass TernaryPatMCf64Vector<ILOpCode opc, SDPatternOperator opn> {
  defm v2f64 : TernaryPatExpandFP<opc, opn, GPRV2I64, v2f64imm, v2f64>;
}

multiclass TernaryPatMCi32<ILOpCode opc, SDPatternOperator opn> :
  TernaryPatMCi32Scalar<opc, opn>,
  TernaryPatMCi32Vector<opc, opn>,
  TernaryPatMCi16Scalar<opc, opn>,
  TernaryPatMCi16Vector<opc, opn>,
  TernaryPatMCi8Scalar<opc, opn>,
  TernaryPatMCi8Vector<opc, opn>;

multiclass TernaryPatMCi64<ILOpCode opc, SDPatternOperator opn> :
  TernaryPatMCi64Scalar<opc, opn>,
  TernaryPatMCi64Vector<opc, opn>;

multiclass TernaryPatMCf32<ILOpCode opc, SDPatternOperator opn> :
  TernaryPatMCf32Scalar<opc, opn>,
  TernaryPatMCf32Vector<opc, opn>;

multiclass TernaryPatMCf64<ILOpCode opc, SDPatternOperator opn> :
  TernaryPatMCf64Scalar<opc, opn>,
  TernaryPatMCf64Vector<opc, opn>;

multiclass TernaryPatMCInt<ILOpCode opc, SDPatternOperator opn> :
  TernaryPatMCi32<opc, opn>,
  TernaryPatMCi64<opc, opn>;

multiclass TernaryPatMCFP<ILOpCode opc, SDPatternOperator opn> :
  TernaryPatMCf32<opc, opn>,
  TernaryPatMCf64<opc, opn>;

multiclass TernaryPatMC<ILOpCode opc, SDPatternOperator opn> :
  TernaryPatMCInt<opc, opn>,
  TernaryPatMCFP<opc, opn>;

//-------------------------------------------------------------------//
// Multiclasses that deal with four operand intrinsics.
//-------------------------------------------------------------------//
multiclass QuaternaryInst<ILOpCode opc, SDPatternOperator opn,
                          ValueType vt, RegisterClass reg,
                          Operand imm, SDNode immnode> {
  def rrrr : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set (vt reg:$dst),
       (opn (vt reg:$src0), (vt reg:$src1),
            (vt reg:$src2), (vt reg:$src3)))]>;

  def iiii : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set (vt reg:$dst),
       (opn (vt immnode:$src0), (vt immnode:$src1),
            (vt immnode:$src2), (vt immnode:$src3)))]>;

  def irrr : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set reg:$dst,
      (opn (vt immnode:$src0), (vt reg:$src1), (vt reg:$src2), (vt reg:$src3)))]>;
  def rirr : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set reg:$dst,
        (opn (vt reg:$src0), (vt reg:$src1), (vt reg:$src2), (vt reg:$src3)))]>;
  def rrir : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set reg:$dst,
      (opn (vt reg:$src0), (vt reg:$src1),
           (vt reg:$src2), (vt reg:$src3)))]>;
  def rrri : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set reg:$dst,
       (opn (vt reg:$src0), (vt reg:$src1),
            (vt reg:$src2), (vt immnode:$src3)))]>;
  def iirr : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set reg:$dst,
      (opn (vt reg:$src0), (vt reg:$src1),
           (vt reg:$src2), (vt reg:$src3)))]>;
  def rrii : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set reg:$dst,
      (opn (vt reg:$src0), (vt reg:$src1),
           (vt reg:$src2), (vt reg:$src3)))]>;
  def irir : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set reg:$dst,
      (opn (vt reg:$src0), (vt reg:$src1),
           (vt reg:$src2), (vt reg:$src3)))]>;
  def irri : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set reg:$dst,
      (opn (vt reg:$src0), (vt reg:$src1),
           (vt reg:$src2), (vt reg:$src3)))]>;
  def riir : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set reg:$dst,
       (opn (vt reg:$src0), (vt reg:$src1),
            (vt reg:$src2), (vt reg:$src3)))]>;
  def riri : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set reg:$dst,
      (opn (vt reg:$src0), (vt reg:$src1),
           (vt reg:$src2), (vt reg:$src3)))]>;
  def iiir : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set reg:$dst,
       (opn (vt immnode:$src0), (vt immnode:$src1),
            (vt immnode:$src2), (vt reg:$src3)))]>;
  def iiri : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set reg:$dst,
      (opn (vt immnode:$src0), (vt immnode:$src1),
           (vt reg:$src2), (vt immnode:$src3)))]>;
  def irii : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set reg:$dst,
      (opn (vt immnode:$src0), (vt reg:$src1),
           (vt immnode:$src2), (vt immnode:$src3)))]>;
  def riii : ILFormat<opc,
    (outs reg:$dst),
    (ins reg:$src0, reg:$src1, reg:$src2, reg:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2, $src3"),
    [(set reg:$dst,
       (opn (vt reg:$src0), (vt immnode:$src1),
            (vt immnode:$src2), (vt immnode:$src3)))]>;
}

multiclass Quaternaryi32Inst<ILOpCode opc, SDPatternOperator opn> {
  defm i32 : QuaternaryInst<opc, opn, i32, GPR_32, i32imm, imm>;
  defm v2i32 : QuaternaryInst<opc, opn, v2i32, GPRV2I32, v2i32imm, imm>;
  defm v4i32 : QuaternaryInst<opc, opn, v4i32, GPRV4I32, v4i32imm, imm>;
}

multiclass BitAlignPatFragCI32<ILOpCode opc, PatFrag node> {
  def i32rrrr : ILFormat<opc, (outs GPR_32:$dst),
    (ins GPR_32:$src0, GPR_32:$src1, GPR_32:$src2, GPR_32:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node GPR_32:$src0, GPR_32:$src1, GPR_32:$src2, GPR_32:$src3))]>;
  def i32irrr : ILFormat<opc, (outs GPR_32:$dst),
    (ins i32imm:$src0, GPR_32:$src1, GPR_32:$src2, GPR_32:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node(i32 imm:$src0), GPR_32:$src1, GPR_32:$src2, GPR_32:$src3))]>;
  def i32rirr : ILFormat<opc, (outs GPR_32:$dst),
    (ins GPR_32:$src0, i32imm:$src1, GPR_32:$src2, GPR_32:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node GPR_32:$src0,(i32 imm:$src1), GPR_32:$src2, GPR_32:$src3))]>;
  def i32rrir : ILFormat<opc, (outs GPR_32:$dst),
    (ins GPR_32:$src0, GPR_32:$src1, i32imm:$src2, GPR_32:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node GPR_32:$src0, GPR_32:$src1,(i32 imm:$src2), GPR_32:$src3))]>;
  def i32rrri : ILFormat<opc, (outs GPR_32:$dst),
    (ins GPR_32:$src0, GPR_32:$src1, GPR_32:$src2, i32imm:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node GPR_32:$src0, GPR_32:$src1, GPR_32:$src2,(i32 imm:$src3)))]>;
  def i32iirr : ILFormat<opc, (outs GPR_32:$dst),
    (ins i32imm:$src0, i32imm:$src1, GPR_32:$src2, GPR_32:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node(i32 imm:$src0),(i32 imm:$src1), GPR_32:$src2, GPR_32:$src3))]>;
  def i32irir : ILFormat<opc, (outs GPR_32:$dst),
    (ins i32imm:$src0, GPR_32:$src1, i32imm:$src2, GPR_32:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node(i32 imm:$src0), GPR_32:$src1,(i32 imm:$src2), GPR_32:$src3))]>;
  def i32irri : ILFormat<opc, (outs GPR_32:$dst),
    (ins i32imm:$src0, GPR_32:$src1, GPR_32:$src2, i32imm:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node(i32 imm:$src0), GPR_32:$src1, GPR_32:$src2,(i32 imm:$src3)))]>;
  def i32riir : ILFormat<opc, (outs GPR_32:$dst),
    (ins GPR_32:$src0, i32imm:$src1, i32imm:$src2, GPR_32:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node GPR_32:$src0,(i32 imm:$src1),(i32 imm:$src2), GPR_32:$src3))]>;
  def i32riri : ILFormat<opc, (outs GPR_32:$dst),
    (ins GPR_32:$src0, i32imm:$src1, GPR_32:$src2, i32imm:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node GPR_32:$src0,(i32 imm:$src1), GPR_32:$src2,(i32 imm:$src3)))]>;
  def i32rrii : ILFormat<opc, (outs GPR_32:$dst),
    (ins GPR_32:$src0, GPR_32:$src1, i32imm:$src2, i32imm:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node GPR_32:$src0, GPR_32:$src1,(i32 imm:$src2),(i32 imm:$src3)))]>;
  def i32iiir : ILFormat<opc, (outs GPR_32:$dst),
    (ins i32imm:$src0, i32imm:$src1, i32imm:$src2, GPR_32:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node(i32 imm:$src0),(i32 imm:$src1),(i32 imm:$src2), GPR_32:$src3))]>;
  def i32iiri : ILFormat<opc, (outs GPR_32:$dst),
    (ins i32imm:$src0, i32imm:$src1, GPR_32:$src2, i32imm:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node(i32 imm:$src0),(i32 imm:$src1), GPR_32:$src2,(i32 imm:$src3)))]>;
  def i32riii : ILFormat<opc, (outs GPR_32:$dst),
    (ins GPR_32:$src0, i32imm:$src1, i32imm:$src2, i32imm:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node GPR_32:$src0,(i32 imm:$src1),(i32 imm:$src2),(i32 imm:$src3)))]>;
  def i32irii : ILFormat<opc, (outs GPR_32:$dst),
    (ins i32imm:$src0, GPR_32:$src1, i32imm:$src2, i32imm:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node(i32 imm:$src0), GPR_32:$src1,(i32 imm:$src2),(i32 imm:$src3)))]>;
  def i32iiii : ILFormat<opc, (outs GPR_32:$dst),
    (ins i32imm:$src0, i32imm:$src1, i32imm:$src2, i32imm:$src3),
    !strconcat(opc.Text, " $dst, $src0, $src1, $src2"),
    [(set GPR_32:$dst, (node(i32 imm:$src0),(i32 imm:$src1),(i32 imm:$src2),(i32 imm:$src3)))]>;
}

// Atomic global load.
multiclass AtomGLoadMC<Operand memOpType, ComplexPattern addrType> {
  let load = 1, global = 1, atomic = 1, rawuav = 1 in {
    def i8 : AtomGLoadClass<IL_OP_UAV_BYTE_LOAD, memOpType, GPRI8, i8, addrType>;
    def i16 : AtomGLoadClass<IL_OP_UAV_SHORT_LOAD, memOpType, GPRI16, i16, addrType>;
    def i32 : AtomGLoadClass<IL_OP_RAW_UAV_LOAD, memOpType, GPR_32, i32, addrType>;
    def v2i32 : AtomGLoadClass<IL_OP_RAW_UAV_LOAD, memOpType, GPRV2I32, v2i32, addrType>;
    def v4i32 : AtomGLoadClass<IL_OP_RAW_UAV_LOAD, memOpType, GPRV4I32, v4i32, addrType>;
  }
}

// Atomic global store.
multiclass AtomGStoreMC<Operand memOpType, ComplexPattern addrType> {
  let store = 1, global = 1, atomic = 1, rawuav = 1 in {
    def i8 : AtomGStoreClass<IL_OP_UAV_BYTE_STORE, memOpType, GPRI8, i8, addrType>;
    def i16 : AtomGStoreClass<IL_OP_UAV_SHORT_STORE, memOpType, GPRI16, i16, addrType>;
    def i32 : AtomGStoreClass<IL_OP_RAW_UAV_STORE, memOpType, GPR_32, i32, addrType>;
    def v2i32 : AtomGStoreClass<IL_OP_RAW_UAV_STORE, memOpType, GPRV2I32, v2i32, addrType>;
    def v4i32 : AtomGStoreClass<IL_OP_RAW_UAV_STORE, memOpType, GPRV4I32, v4i32, addrType>;
  }
}

multiclass FlatStore<ILOpCode opc, RegisterClass ptr, RegisterClass data> {
  def U : ILFormat<opc, (outs), (ins ptr:$addy, data:$src),
    !strconcat(opc.Text, " $src, $addy"), []>;
  def S : ILFormat<opc, (outs), (ins ptr:$addy, data:$src),
    !strconcat(opc.Text, "_slc $src, $addy"), []>;
  def G : ILFormat<opc, (outs), (ins ptr:$addy, data:$src),
    !strconcat(opc.Text, "_glc $src, $addy"), []>;
  def T : ILFormat<opc, (outs), (ins ptr:$addy, data:$src),
    !strconcat(opc.Text, "_prt $src, $addy"), []>;
  def SG : ILFormat<opc, (outs), (ins ptr:$addy, data:$src),
    !strconcat(opc.Text, "_slc_glc $src, $addy"), []>;
  def ST : ILFormat<opc, (outs), (ins ptr:$addy, data:$src),
    !strconcat(opc.Text, "_slc_prt $src, $addy"), []>;
  def TG : ILFormat<opc, (outs), (ins ptr:$addy, data:$src),
    !strconcat(opc.Text, "_prt_glc $src, $addy"), []>;
  def SGT : ILFormat<opc, (outs), (ins ptr:$addy, data:$src),
    !strconcat(opc.Text, "_slc_glc_prt $src, $addy"), []>;
}

multiclass FlatLoad<ILOpCode opc, RegisterClass ptr, RegisterClass data> {
  def U : ILFormat<opc, (outs data:$dst), (ins ptr:$addy),
    !strconcat(opc.Text, " $dst, $addy"), []>;
  def S : ILFormat<opc, (outs data:$dst), (ins ptr:$addy),
    !strconcat(opc.Text, "_slc $dst, $addy"), []>;
  def G : ILFormat<opc, (outs data:$dst), (ins ptr:$addy),
    !strconcat(opc.Text, "_glc $dst, $addy"), []>;
  def T : ILFormat<opc, (outs data:$dst), (ins ptr:$addy),
    !strconcat(opc.Text, "_prt $dst, $addy"), []>;
  def SG : ILFormat<opc, (outs data:$dst), (ins ptr:$addy),
    !strconcat(opc.Text, "_slc_glc $dst, $addy"), []>;
  def ST : ILFormat<opc, (outs data:$dst), (ins ptr:$addy),
    !strconcat(opc.Text, "_slc_prt $dst, $addy"), []>;
  def TG : ILFormat<opc, (outs data:$dst), (ins ptr:$addy),
    !strconcat(opc.Text, "_prt_glc $dst, $addy"), []>;
  def SGT : ILFormat<opc, (outs data:$dst), (ins ptr:$addy),
    !strconcat(opc.Text, "_slc_glc_prt $dst, $addy"), []>;
}

multiclass FlatLoadMC<ILOpCode opc, RegisterClass data> {
  defm P32 : FlatLoad<opc, GPR_32, data>;
  let Predicates = [Has64BitPtr] in {
    defm P64 : FlatLoad<opc, GPR_64, data>;
  }
}

multiclass FlatStoreMC<ILOpCode opc, RegisterClass data> {
  defm P32 : FlatStore<opc, GPR_32, data>;
  let Predicates = [Has64BitPtr] in {
    defm P64 : FlatStore<opc, GPR_64, data>;
  }
}

multiclass FTRUNCSTORE<string asm, Operand mem, ComplexPattern addr> {
  let store = 1, truncate = 1, flat = 1 in {
    let sub32bit = 1 in {
      defm i16i8  : StoreMC<asm,  flat_i8trunc_store, GPRI16, i16imm, i16, imm, mem, addr>;
      defm i32i8  : StoreMC<asm,  flat_i8trunc_store, GPR_32, i32imm, i32, imm, mem, addr>;
      defm i64i8  : StoreMC<asm,  flat_i8trunc_store, GPR_64, i64imm, i64, imm, mem, addr>;
      defm i32i16 : StoreMC<asm, flat_i16trunc_store, GPR_32, i32imm, i32, imm, mem, addr>;
      defm i64i16 : StoreMC<asm, flat_i16trunc_store, GPR_64, i64imm, i64, imm, mem, addr>;
    }
    defm i64i32 : StoreMC<asm, flat_i32trunc_store, GPR_64, i64imm, i64,   imm, mem, addr>;
    defm f64f32 : StoreMC<asm, flat_f32trunc_store, GPR_64, f64imm, f64, fpimm, mem, addr>;
    let packed = 1 in {
      let sub32bit = 1 in {
        let packType = 0 in {
          defm v2i16i8 : StoreMC<asm, flat_v2i8trunc_store, GPRV2I16, v2i16imm, v2i16, imm, mem, addr>;
          defm v2i32i8 : StoreMC<asm, flat_v2i8trunc_store, GPRV2I32, v2i32imm, v2i32, imm, mem, addr>;
          defm v2i64i8 : StoreMC<asm, flat_v2i8trunc_store, GPRV2I64, v2i64imm, v2i64, imm, mem, addr>;
        }
      }
      let packType = 2 in {
        defm v4i16i8 : StoreMC<asm, flat_v4i8trunc_store, GPRV4I16, v4i16imm, v4i16, imm, mem, addr>;
        defm v4i32i8 : StoreMC<asm, flat_v4i8trunc_store, GPRV4I32, v4i32imm, v4i32, imm, mem, addr>;
      }
      let packType = 1 in {
        defm v2i32i16 : StoreMC<asm, flat_v2i16trunc_store, GPRV2I32, v2i32imm, v2i32, imm, mem, addr>;
        defm v2i64i16 : StoreMC<asm, flat_v2i16trunc_store, GPRV2I64, v2i64imm, v2i64, imm, mem, addr>;
      }
      let packType = 3 in {
        defm v4i32i16 : StoreMC<asm, flat_v4i16trunc_store, GPRV4I32, v4i32imm, v4i32, imm, mem, addr>;
      }
    }
    defm v2f64f32 : StoreMC<asm, flat_v2f32trunc_store, GPRV2I64, v2f64imm, v2f64, fpimm, mem, addr>;
    defm v2i64i32 : StoreMC<asm, flat_v2i32trunc_store, GPRV2I64, v2i64imm, v2i64,   imm, mem, addr>;
  }
}

