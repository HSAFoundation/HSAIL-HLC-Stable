//===---- AMDILMCCodeEmitter.cpp - Convert AMDIL text to AMDIL binary ----===//
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
//
//===---------------------------------------------------------------------===//

#define DEBUG_TYPE "amdil-emitter"
#include "AMDIL.h"
#include "AMDILInstrInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/raw_ostream.h"


using namespace llvm;
#if 0
namespace {
class AMDILMCCodeEmitter : public MCCodeEmitter {
  AMDILMCCodeEmitter(const AMDILMCCodeEmitter &);// DO NOT IMPLEMENT
  void operator=(const AMDILMCCodeEmitter &); // DO NOT IMPLEMENT
  const TargetMachine &TM;
  const TargetInstrInfo &TII;
  MCContext &Ctx;
  bool Is64BitMode;
public:
  AMDILMCCodeEmitter(TargetMachine &tm, MCContext &ctx, bool is64Bit);
  ~AMDILMCCodeEmitter();
  unsigned getNumFixupKinds() const;
  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const;
  static unsigned GetAMDILRegNum(const MCOperand &MO);
  void EmitByte(unsigned char C, unsigned &CurByte, raw_ostream &OS) const;
  void EmitConstant(uint64_t Val, unsigned Size, unsigned &CurByte,
                    raw_ostream &OS) const;
  void EmitImmediate(const MCOperand &Disp, unsigned ImmSize,
                     MCFixupKind FixupKind, unsigned &CurByte, raw_ostream &os,
                     SmallVectorImpl<MCFixup> &Fixups, int ImmOffset = 0) const;

  void EncodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups) const;

}; // class AMDILMCCodeEmitter
} // anonymous namespace

namespace llvm {
MCCodeEmitter *createAMDILMCCodeEmitter(const Target &,
                                        TargetMachine &TM, MCContext &Ctx) {
  return new AMDILMCCodeEmitter(TM, Ctx, false);
}
}

AMDILMCCodeEmitter::AMDILMCCodeEmitter(TargetMachine &TM_,
                                       MCContext &CTX_,
                                       bool Is64Bit)
  : TM(TM), TII(*TM.getInstrInfo()), Ctx(CTX_) {
  Is64BitMode = Is64Bit;
}

AMDILMCCodeEmitter::~AMDILMCCodeEmitter() {
}

unsigned
AMDILMCCodeEmitter::getNumFixupKinds() const {
  return 0;
}

const MCFixupKindInfo &
AMDILMCCodeEmitter::getFixupKindInfo(MCFixupKind Kind) const {
//  const static MCFixupKindInfo Infos[] = {};
  if (Kind < FirstTargetFixupKind) {
    return MCCodeEmitter::getFixupKindInfo(Kind);
  }

  assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
         "Invalid kind!");
  return MCCodeEmitter::getFixupKindInfo(Kind);
// return Infos[Kind - FirstTargetFixupKind];

}

void
AMDILMCCodeEmitter::EmitByte(unsigned char C, unsigned &CurByte,
                             raw_ostream &OS) const {
  OS << (char) C;
  ++CurByte;
}
void
AMDILMCCodeEmitter::EmitConstant(uint64_t Val,
                                 unsigned Size,
                                 unsigned &CurByte,
                                 raw_ostream &OS) const {
  // Output the constant in little endian byte order
  for (unsigned I = 0; I != Size; ++I) {
    EmitByte(Val & 255, CurByte, OS);
    Val >>= 8;
  }
}
void AMDILMCCodeEmitter::EmitImmediate(const MCOperand &DispOp,
                                       unsigned ImmSize,
                                       MCFixupKind FixupKind,
                                       unsigned &CurByte,
                                       raw_ostream &OS,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       int ImmOffset) const {
  // If this is a simple integer displacement that doesn't require a relocation
  // emit it now.
  if (DispOp.isImm()) {
    EmitConstant(DispOp.getImm() + ImmOffset, ImmSize, CurByte, OS);
  }

  // If we have an immoffset, add it to the expression
  const MCExpr *Expr = DispOp.getExpr();

  if (ImmOffset) {
    Expr = MCBinaryExpr::CreateAdd(Expr,
                                   MCConstantExpr::Create(ImmOffset, Ctx), Ctx);
  }

  // Emit a symbolic constant as a fixup and 4 zeros.
  Fixups.push_back(MCFixup::Create(CurByte, Expr, FixupKind));
  // TODO: Why the 4 zeros?
  EmitConstant(0, ImmSize, CurByte, OS);
}

void
AMDILMCCodeEmitter::EncodeInstruction(const MCInst &MI,
                                      raw_ostream &OS,
                                      SmallVectorImpl<MCFixup> &Fixups) const {
#if 0
  unsigned Opcode = MI.getOpcode();
  const TargetInstrDesc &Desc = TII.get(Opcode);
  unsigned TSFlags = Desc.TSFlags;

  // Keep track of the current byte being emitted.
  unsigned CurByte = 0;

  unsigned NumOps = Desc.getNumOperands();
  unsigned CurOp = 0;

  unsigned char BaseOpcode = 0;
#ifndef NDEBUG

  // FIXME: Verify.
  if (// !Desc.isVariadic() &&
    CurOp != NumOps) {
    errs() << "Cannot encode all operands of: ";
    MI.dump();
    errs() << '\n';
    abort();
  }

#endif
#endif
}
#endif
