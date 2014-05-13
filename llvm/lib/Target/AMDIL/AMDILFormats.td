//==- AMDILInstrFormats.td - AMDIL Instruction Formats ----*- tablegen -*-==//
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
//===--------------------------------------------------------------------===//
include "AMDILTokenDesc.td"

//===--------------------------------------------------------------------===//
// The parent IL instruction class that inherits the Instruction class. This
// class sets the corresponding namespace, the out and input dag lists the
// pattern to match to and the string to print out for the assembly printer.
//===--------------------------------------------------------------------===//
class ILFormat<ILOpCode op, dag outs, dag ins,
               string asmstr, list<dag> pattern> : Instruction {
  let Namespace = "AMDIL";
  dag OutOperandList = outs;
  dag InOperandList = ins;
  ILOpCode operation = op;
  let Pattern = pattern;
  let AsmString = !strconcat(asmstr, "\n");
  bits<2> extLoadType = 0;
  bit load = 0;
  bit store = 0;
  bit truncate = 0;
  bit atomic = 0;
  bit addr64 = 0;
  bit global = 0;
  bit private = 0;
  bit constant = 0;
  bit cpool = 0;
  bit region = 0;
  bit local = 0;
  bit gds = 0;
  bit lds = 0;
  bit cbmem = 0;
  bit scratch = 0;
  bit rawuav = 0;
  bit arenauav = 0;
  bit image = 0;
  bits<2> info = 0;
  bit txld = 0;
  bit sema = 0;
  bit append = 0;
  bit swsextload = 0;
  bit loadconst = 0;
  bit ieee = 0;
  bit zeroop = 0;
  bit flat = 0;
  bits<6> swizzle = 0;
  bit gws = 0;
  bit packed = 0;
  bit sub32bit = 0;
  bits<2> packType = 0;
  bit vector = 0;
  // This has to be kept in sync with (1ULL << AMDIL.h::AMDID) enumeration.
  let TSFlags{2-1} = extLoadType;
  let TSFlags{3} = load;
  let TSFlags{4} = store;
  let TSFlags{5} = truncate;
  let TSFlags{6} = atomic;
  let TSFlags{7} = addr64;
  let TSFlags{8} = global;
  let TSFlags{9} = private;
  let TSFlags{10} = constant;
  let TSFlags{11} = cpool;
  let TSFlags{12} = region;
  let TSFlags{13} = local;
  let TSFlags{14} = gds;
  let TSFlags{15} = lds;
  let TSFlags{16} = cbmem;
  let TSFlags{17} = scratch;
  let TSFlags{18} = rawuav;
  let TSFlags{19} = arenauav;
  let TSFlags{20} = image;
  let TSFlags{22-21} = info;
  let TSFlags{23} = txld;
  let TSFlags{24} = sema;
  let TSFlags{25} = append;
  let TSFlags{26} = swsextload;
  let TSFlags{27} = loadconst;
  let TSFlags{28} = ieee;
  let TSFlags{29} = zeroop;
  let TSFlags{30} = flat;
  let TSFlags{36-31} = swizzle;
  let TSFlags{37} = gws;
  let TSFlags{38} = packed;
  let TSFlags{39} = sub32bit;
  let TSFlags{41-40} = packType;
  let TSFlags{42} = vector;
}

//===-------------------------------------------------------------===//
// The macro class that is an extension of ILFormat but is tailored for
// macros only where all the register types are the same
//===-------------------------------------------------------------===//
class UnaryMacro<RegisterClass Dst,
                 RegisterClass Src0,
                 SDNode OpNode> :
  ILFormat<IL_OP_MACRO, (outs Dst:$dst),
    (ins Src0:$src0),
    "($dst),($src0)",
    [(set Dst:$dst, (OpNode Src0:$src0))]>;

//===-------------------------------------------------------------===//
// The macro class is an extension of ILFormat but is tailored for
// macros only where all the register types are the same
//===-------------------------------------------------------------===//
class BinaryMacro<RegisterClass Dst,
                  RegisterClass Src0,
                  RegisterClass Src1,
                  SDNode OpNode> :
  ILFormat<IL_OP_MACRO, (outs Dst:$dst),
    (ins Src0: $src0, Src1:$src1),
    "($dst),($src0, $src1)",
    [(set Dst:$dst, (OpNode Src0:$src0, Src1:$src1))]>;

//===-------------------------------------------------------------===//
// Classes for dealing with atomic instructions w/ 32bit pointers
//===-------------------------------------------------------------===//
class Append<ILOpCode op, string idType, SDNode intr>:
  ILFormat<op, (outs GPR_32:$dst), (ins MEM32:$id),
    !strconcat(op.Text, !strconcat(idType," $dst")),
    [(set GPR_32:$dst, (intr ADDR:$id))]>;

class UniAtom<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_32:$dst), (ins MEM32:$ptr, i32imm:$id),
  !strconcat(op.Text, !strconcat(idType," $dst, $ptr")),
  [(set GPR_32:$dst, (intr ADDR:$ptr, timm:$id))]>;

class UniAtomNoRet<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM32:$ptr, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $ptr")),
               [(intr ADDR:$ptr, timm:$id)]>;

class BinAtom<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_32:$dst),
               (ins MEM32:$ptr, GPR_32:$src, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, $ptr, $src")),
               [(set GPR_32:$dst, (intr ADDR:$ptr, GPR_32:$src, timm:$id))]>;


class BinAtomNoRet<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM32:$ptr, GPR_32:$src, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $ptr, $src")),
               [(intr ADDR:$ptr, GPR_32:$src, timm:$id)]>;

class TriAtom<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_32:$dst),
               (ins MEM32:$ptr, GPR_32:$src, GPR_32:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, $ptr, $src, $src1")),
               [(set GPR_32:$dst, (intr ADDR:$ptr, GPR_32:$src, GPR_32:$src1, timm:$id))]>;

class CmpXChg<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_32:$dst),
               (ins MEM32:$ptr, GPR_32:$src, GPR_32:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, $ptr, $src1, $src")),
               [(set GPR_32:$dst, (intr ADDR:$ptr, GPR_32:$src, GPR_32:$src1, timm:$id))]>;

class TriAtomNoRet<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM32:$ptr, GPR_32:$src, GPR_32:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $ptr, $src, $src1")),
               [(intr ADDR:$ptr, GPR_32:$src, GPR_32:$src1, timm:$id)]>;

class CmpXChgNoRet<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM32:$ptr, GPR_32:$src, GPR_32:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $ptr, $src1, $src")),
               [(intr ADDR:$ptr, GPR_32:$src, GPR_32:$src1, timm:$id)]>;

class UniAtomI64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_64:$dst),
               (ins MEM32:$ptr, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, ${ptr}0")),
               [(set GPR_64:$dst, (intr ADDR:$ptr, timm:$id))]>;

class UniAtomNoRetI64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM32:$ptr, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," ${ptr}0")),
               [(intr ADDR:$ptr, timm:$id)]>;

class BinAtomI64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_64:$dst),
               (ins MEM32:$ptr, GPR_64:$src, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, ${ptr}0, $src")),
               [(set GPR_64:$dst, (i64 (intr ADDR:$ptr, GPR_64:$src, timm:$id)))]>;


class BinAtomNoRetI64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM32:$ptr, GPR_64:$src, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," ${ptr}0, $src")),
               [(intr ADDR:$ptr, GPR_64:$src, timm:$id)]>;

class TriAtomI64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_64:$dst),
               (ins MEM32:$ptr, GPR_64:$src, GPR_64:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, ${ptr}0, $src, $src1")),
               [(set GPR_64:$dst, (intr ADDR:$ptr, GPR_64:$src, GPR_64:$src1, timm:$id))]>;

class CmpXChgI64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_64:$dst),
               (ins MEM32:$ptr, GPR_64:$src, GPR_64:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, ${ptr}0, $src1, $src")),
               [(set GPR_64:$dst, (intr ADDR:$ptr, GPR_64:$src, GPR_64:$src1, timm:$id))]>;

class TriAtomNoRetI64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM32:$ptr, GPR_64:$src, GPR_64:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," ${ptr}0, $src, $src1")),
               [(intr ADDR:$ptr, GPR_64:$src, GPR_64:$src1, timm:$id)]>;

class CmpXChgNoRetI64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM32:$ptr, GPR_64:$src, GPR_64:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," ${ptr}0, $src1, $src")),
               [(intr ADDR:$ptr, GPR_64:$src, GPR_64:$src1, timm:$id)]>;


//===-------------------------------------------------------------===//
// Classes for dealing with atomic instructions w/ 64bit pointers
//===-------------------------------------------------------------===//
class Append64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_32:$dst),
               (ins MEM64:$id),
               !strconcat(op.Text, !strconcat(idType," $dst")),
               [(set GPR_32:$dst, (intr ADDR64:$id))]>;

class UniAtom64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_32:$dst),
               (ins MEM64:$ptr, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, $ptr")),
               [(set GPR_32:$dst, (intr ADDR64:$ptr, timm:$id))]>;


class UniAtomNoRet64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM64:$ptr, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $ptr")),
               [(intr ADDR64:$ptr, timm:$id)]>;

class BinAtom64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_32:$dst),
               (ins MEM64:$ptr, GPR_32:$src, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, $ptr, $src")),
               [(set GPR_32:$dst, (intr ADDR64:$ptr, GPR_32:$src, timm:$id))]>;


class BinAtomNoRet64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM64:$ptr, GPR_32:$src, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $ptr, $src")),
               [(intr ADDR64:$ptr, GPR_32:$src, timm:$id)]>;

class TriAtom64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_32:$dst),
               (ins MEM64:$ptr, GPR_32:$src, GPR_32:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, $ptr, $src, $src1")),
               [(set GPR_32:$dst, (intr ADDR64:$ptr, GPR_32:$src, GPR_32:$src1, timm:$id))]>;

class CmpXChg64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_32:$dst),
               (ins MEM64:$ptr, GPR_32:$src, GPR_32:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, $ptr, $src1, $src")),
               [(set GPR_32:$dst, (intr ADDR64:$ptr, GPR_32:$src, GPR_32:$src1, timm:$id))]>;

class TriAtomNoRet64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM64:$ptr, GPR_32:$src, GPR_32:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $ptr, $src, $src1")),
               [(intr ADDR64:$ptr, GPR_32:$src, GPR_32:$src1, timm:$id)]>;

class CmpXChgNoRet64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM64:$ptr, GPR_32:$src, GPR_32:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $ptr, $src1, $src")),
               [(intr ADDR64:$ptr, GPR_32:$src, GPR_32:$src1, timm:$id)]>;

class UniAtom64I64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_64:$dst),
               (ins MEM64:$ptr, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, $ptr")),
               [(set GPR_64:$dst, (intr ADDR64:$ptr, timm:$id))]>;


class UniAtomNoRet64I64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM64:$ptr, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $ptr")),
               [(intr ADDR64:$ptr, timm:$id)]>;

class BinAtom64I64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_64:$dst),
               (ins MEM64:$ptr, GPR_64:$src, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, $ptr, $src")),
               [(set GPR_64:$dst, (intr ADDR64:$ptr, GPR_64:$src, timm:$id))]>;

class BinAtomNoRet64I64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM64:$ptr, GPR_64:$src, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $ptr, $src")),
               [(intr ADDR64:$ptr, GPR_64:$src, timm:$id)]>;

class TriAtom64I64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_64:$dst),
               (ins MEM64:$ptr, GPR_64:$src, GPR_64:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, $ptr, $src, $src1")),
               [(set GPR_64:$dst, (intr ADDR64:$ptr, GPR_64:$src, GPR_64:$src1, timm:$id))]>;

class CmpXChg64I64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs GPR_64:$dst),
               (ins MEM64:$ptr, GPR_64:$src, GPR_64:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $dst, $ptr, $src1, $src")),
               [(set GPR_64:$dst, (intr ADDR64:$ptr, GPR_64:$src, GPR_64:$src1, timm:$id))]>;

class TriAtomNoRet64I64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM64:$ptr, GPR_64:$src, GPR_64:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $ptr, $src, $src1")),
               [(intr ADDR64:$ptr, GPR_64:$src, GPR_64:$src1, timm:$id)]>;

class CmpXChgNoRet64I64<ILOpCode op, string idType, SDNode intr> :
  ILFormat<op, (outs),
               (ins MEM64:$ptr, GPR_64:$src, GPR_64:$src1, i32imm:$id),
               !strconcat(op.Text, !strconcat(idType," $ptr, $src1, $src")),
               [(intr ADDR64:$ptr, GPR_64:$src, GPR_64:$src1, timm:$id)]>;

// Atomic global load
class AtomGLoadClass<ILOpCode op, Operand memType, RegisterClass valRegType,
                     ValueType vt, ComplexPattern addrType> :
  ILFormat<op, (outs valRegType:$dst),
               (ins memType:$ptr, GPR_32:$order, i32imm:$id),
               !strconcat(op.Text, "_id($id)_uncached $dst, $ptr"),
               [(set (vt valRegType:$dst),
                 (atom_g_load addrType:$ptr, GPR_32:$order, timm:$id))]>;

// Atomic global store
class AtomGStoreClass<ILOpCode op, Operand memType, RegisterClass valRegType,
                      ValueType vt, ComplexPattern addrType> :
  ILFormat<op, (outs),
               (ins memType:$ptr, valRegType:$src, GPR_32:$order, i32imm:$id),
               !strconcat(op.Text, "_id($id)_uncached $ptr, $src"),
               [(atom_g_store addrType:$ptr, (vt valRegType:$src), GPR_32:$order, timm:$id)]>;
//===-------------------------------------------------------------===//
// Intrinsic classes.
// Generic versions of the above classes but for Target specific
// intrinsics instead of SDNode patterns.
// ===-------------------------------------------------------------===//
let TargetPrefix = "AMDIL", isTarget = 1 in {
  class UnaryIntInt :
    Intrinsic<[llvm_anyint_ty], [LLVMMatchType<0>], [IntrNoMem]>;
  class UnaryIntFloat :
    Intrinsic<[llvm_anyfloat_ty], [LLVMMatchType<0>], [IntrNoMem]>;
  class ConvertIntFTOI :
    Intrinsic<[llvm_anyint_ty], [llvm_anyfloat_ty], [IntrNoMem]>;
  class ConvertIntITOF :
    Intrinsic<[llvm_anyfloat_ty], [llvm_anyint_ty], [IntrNoMem]>;
  class UnaryIntNoRetInt :
    Intrinsic<[], [llvm_anyint_ty], []>;
  class BinaryIntInt :
    Intrinsic<[llvm_anyint_ty], [LLVMMatchType<0>, LLVMMatchType<0>], [IntrNoMem]>;
  class BinaryIntFloat :
    Intrinsic<[llvm_anyfloat_ty], [LLVMMatchType<0>, LLVMMatchType<0>], [IntrNoMem]>;
  class BinaryIntNoRetInt :
    Intrinsic<[], [llvm_anyint_ty, LLVMMatchType<0>], []>;
  class TernaryIntInt :
    Intrinsic<[llvm_anyint_ty], [LLVMMatchType<0>, LLVMMatchType<0>, LLVMMatchType<0>], [IntrNoMem]>;
  class TernaryIntLong :
    Intrinsic<[llvm_i64_ty], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  class TernaryIntFloat :
    Intrinsic<[llvm_anyfloat_ty], [LLVMMatchType<0>, LLVMMatchType<0>, LLVMMatchType<0>], [IntrNoMem]>;
  class UnaryAtomicInt :
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  class BinaryAtomicInt :
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  class TernaryAtomicInt :
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty]>;
  class VoidAtomicIntNoRet :
    Intrinsic<[], [llvm_ptr_ty], [IntrReadWriteArgMem]>;
  class UnaryAtomicIntNoRet :
    Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  class BinaryAtomicIntNoRet :
    Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;
  class TernaryAtomicIntNoRet :
    Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], [IntrReadWriteArgMem]>;

  class UnaryAtomicLong :
    Intrinsic<[llvm_i64_ty], [llvm_ptr_ty, llvm_i64_ty], [IntrReadWriteArgMem]>;
  class BinaryAtomicLong :
    Intrinsic<[llvm_i64_ty], [llvm_ptr_ty, llvm_i64_ty, llvm_i64_ty], [IntrReadWriteArgMem]>;
  class TernaryAtomicLong :
    Intrinsic<[llvm_i64_ty], [llvm_ptr_ty, llvm_i64_ty, llvm_i64_ty, llvm_i64_ty]>;
  class VoidAtomicLongNoRet :
    Intrinsic<[], [llvm_ptr_ty], [IntrReadWriteArgMem]>;
  class UnaryAtomicLongNoRet :
    Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty], [IntrReadWriteArgMem]>;
  class BinaryAtomicLongNoRet :
    Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i64_ty], [IntrReadWriteArgMem]>;
  class TernaryAtomicLongNoRet :
    Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i64_ty, llvm_i64_ty], [IntrReadWriteArgMem]>;
}
