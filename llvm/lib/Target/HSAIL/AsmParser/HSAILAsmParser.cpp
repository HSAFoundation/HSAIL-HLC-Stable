//===-- HSAILAsmParser.cpp - Parse HSAIL assembly to MCInst instructions --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Target/TargetAsmParser.h"
#include "HSAIL.h"
#include "HSAILSubtarget.h"
#include "llvm/Target/TargetRegistry.h"
#include "llvm/Target/TargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
struct HSAILOperand;

class HSAILAsmParser : public TargetAsmParser {
  MCAsmParser &Parser;
  TargetMachine &TM;

protected:
  unsigned Is64Bit : 1;

private:
  MCAsmParser&
  getParser() const { return Parser; }

  MCAsmLexer&
  getLexer() const { return Parser.getLexer(); }

  bool
  Error(SMLoc L, const Twine &Msg) { return Parser.Error(L, Msg); }

  HSAILOperand*
  ParseOperand();

  HSAILOperand*
  ParseMemOperand(unsigned SegReg, SMLoc StartLoc);

  bool
  ParseDirectiveWord(unsigned Size, SMLoc L);

  bool
  MatchAndEmitInstruction(SMLoc IDLoc,
                          SmallVectorImpl<MCParsedAsmOperand*> &Operands,
                          MCStreamer &Out);

  /// @name Auto-generated Matcher Functions
  /// {

#define GET_ASSEMBLER_HEADER
#include "HSAILGenAsmMatcher.inc"

  /// }

public:
  HSAILAsmParser(const Target &T, MCAsmParser &parser, TargetMachine &TM)
    : TargetAsmParser(T), Parser(parser), TM(TM) {

    // Initialize the set of available features.
    setAvailableFeatures(ComputeAvailableFeatures(&TM.getSubtarget<HSAILSubtarget>()));
  }
  virtual bool
  ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc);

  virtual bool
  ParseInstruction(StringRef Name,
                   SMLoc NameLoc,
                   SmallVectorImpl<MCParsedAsmOperand*> &Operands);

  virtual bool
  ParseDirective(AsmToken DirectiveID);
};

class HSAIL_32AsmParser : public HSAILAsmParser {
public:
  HSAIL_32AsmParser(const Target &T, MCAsmParser &Parser, TargetMachine &TM)
    : HSAILAsmParser(T, Parser, TM) { Is64Bit = false; }
};

class HSAIL_64AsmParser : public HSAILAsmParser {
public:
  HSAIL_64AsmParser(const Target &T, MCAsmParser &Parser, TargetMachine &TM)
    : HSAILAsmParser(T, Parser, TM) { Is64Bit = true; }
};

} // end anonymous namespace

/// @name Auto-generated Match Functions
/// {

static unsigned
MatchRegisterName(StringRef Name);

/// }

namespace {

/// HSAILOperand - Instances of this class represent a parsed HSAIL machine
/// instruction.
struct HSAILOperand : public MCParsedAsmOperand {
  enum KindTy {
    Token,
    Register,
    Immediate,
    Memory
  } Kind;

  SMLoc StartLoc, EndLoc;

  union {
    struct {
      const char *Data;
      unsigned Length;
    } Tok;

    struct {
      unsigned RegNo;
    } Reg;

    struct {
      const MCExpr *Val;
    } Imm;

    struct {
      unsigned SegReg;
      const MCExpr *Disp;
      unsigned BaseReg;
      unsigned IndexReg;
      unsigned Scale;
    } Mem;
  };

  HSAILOperand(KindTy K, SMLoc Start, SMLoc End)
    : Kind(K), StartLoc(Start), EndLoc(End) {}

  /// getStartLoc - Get the location of the first token of this operand.
  SMLoc
  getStartLoc() const { return StartLoc; }

  /// getEndLoc - Get the location of the last token of this operand.
  SMLoc
  getEndLoc() const { return EndLoc; }

  virtual void
  dump(raw_ostream &OS) const {}

  StringRef
  getToken() const
  {
    assert(!"When do we hit this?");
    return StringRef();
  }

  void
  setTokenValue(StringRef Value)
  {
    assert(!"When do we hit this?");
  }

  unsigned
  getReg() const
  {
    assert(!"When do we hit this?");
    return 0;
  }

  const MCExpr*
  getImm() const
  {
    assert(!"When do we hit this?");
    return NULL;
  }

  const MCExpr*
  getMemDisp() const
  {
    assert(!"When do we hit this?");
    return NULL;
  }

  unsigned
  getMemSegReg() const
  {
    assert(!"When do we hit this?");
    return 0;
  }

  unsigned
  getMemBaseReg() const
  {
    assert(!"When do we hit this?");
    return 0;
  }

  unsigned
  getMemIndexReg() const
  {
    assert(!"When do we hit this?");
    return 0;
  }

  unsigned
  getMemScale() const
  {
    assert(!"When do we hit this?");
    return 0;
  }

  bool
  isToken() const
  {
    assert(!"When do we hit this?");
    return false;
  }

  bool
  isImm() const
  {
    assert(!"When do we hit this?");
    return false;
  }

  bool
  isImmSExti16i8() const
  {
    assert(!"When do we hit this?");
    return false;
  }

  bool
  isImmSExti32i8() const
  {
    assert(!"When do we hit this?");
    return false;
  }

  bool
  isImmSExti64i8() const
  {
    assert(!"When do we hit this?");
    return false;
  }

  bool
  isImmSExti64i32() const
  {
    assert(!"When do we hit this?");
    return false;
  }

  bool
  isMem() const
  {
    assert(!"When do we hit this?");
    return false;
  }

  bool
  isAbsMem() const
  {
    assert(!"When do we hit this?");
    return false;
  }

  bool
  isReg() const
  {
    assert(!"When do we hit this?");
    return false;
  }

  void
  addExpr(MCInst &Inst, const MCExpr *Expr) const
  {
    assert(!"When do we hit this?");
  }

  void
  addRegOperands(MCInst &Inst, unsigned N) const
  {
    assert(!"When do we hit this?");
  }

  void
  addImmOperands(MCInst &Inst, unsigned N) const
  {
    assert(!"When do we hit this?");
  }

  void
  addMemOperands(MCInst &Inst, unsigned N) const
  {
    assert(!"When do we hit this?");
  }

  void
  addAbsMemOperands(MCInst &Inst, unsigned N) const
  {
    assert(!"When do we hit this?");
  }

};

} // end anonymous namespace.


bool
HSAILAsmParser::ParseRegister(unsigned &RegNo,
                              SMLoc &StartLoc,
                              SMLoc &EndLoc)
{
  assert(!"When do we hit this?");
  const AsmToken &Tok = Parser.getTok();
  RegNo = MatchRegisterName(Tok.getString()); // fix a warning problem
  return false;
}

HSAILOperand*
HSAILAsmParser::ParseOperand()
{
  assert(!"When do we hit this?");
  return NULL;
}

/// ParseMemOperand: segment: disp(basereg, indexreg, scale).  The '%ds:' prefix
/// has already been parsed if present.
HSAILOperand*
HSAILAsmParser::ParseMemOperand(unsigned SegReg, SMLoc MemStart)
{
  assert(!"When do we hit this?");
  return NULL;
}

bool
HSAILAsmParser::ParseInstruction(StringRef Name,
                                 SMLoc NameLoc,
                                 SmallVectorImpl<MCParsedAsmOperand*> &Operands)
{
  assert(!"When do we hit this?");
  return false;
}

bool
HSAILAsmParser::MatchAndEmitInstruction(SMLoc IDLoc,
                                        SmallVectorImpl<MCParsedAsmOperand*> &Operands,
                                        MCStreamer &Out) {
  assert(!"When do we hit this?");
  return false;
}


bool
HSAILAsmParser::ParseDirective(AsmToken DirectiveID) {
  assert(!"When do we hit this?");
  return false;
}

/// ParseDirectiveWord
///  ::= .word [ expression (, expression)* ]
bool
HSAILAsmParser::ParseDirectiveWord(unsigned Size, SMLoc L)
{
  assert(!"When do we hit this?");
  return false;
}

extern "C" void LLVMInitializeHSAILAsmLexer();

// Force static initialization.
extern "C" void LLVMInitializeHSAILAsmParser() {
  RegisterAsmParser<HSAIL_32AsmParser> X(TheHSAIL_32Target);
  RegisterAsmParser<HSAIL_64AsmParser> Y(TheHSAIL_64Target);
  LLVMInitializeHSAILAsmLexer();
}

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "HSAILGenAsmMatcher.inc"
