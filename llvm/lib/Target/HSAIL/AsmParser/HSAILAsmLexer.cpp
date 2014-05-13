//===-- HSAILAsmLexer.cpp - Tokenize HSAIL assembly to AsmTokens ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Target/TargetAsmLexer.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "HSAIL.h"

using namespace llvm;

namespace {

class HSAILAsmLexer : public TargetAsmLexer {
  const MCAsmInfo &AsmInfo;

  bool tentativeIsValid;
  AsmToken tentativeToken;

  const AsmToken&
  lexTentative()
  {
    tentativeToken = getLexer()->Lex();
    tentativeIsValid = true;
    return tentativeToken;
  }

  const AsmToken&
  lexDefinite()
  {
    if (tentativeIsValid) {
      tentativeIsValid = false;
      return tentativeToken;
    }
    return getLexer()->Lex();
  }
protected:
  AsmToken
  LexToken();
public:
  HSAILAsmLexer(const Target &T, const MCAsmInfo &MAI)
    : TargetAsmLexer(T), AsmInfo(MAI), tentativeIsValid(false) {}
};

} // end anonymous namespace

#define GET_REGISTER_MATCHER
#include "HSAILGenAsmMatcher.inc"

AsmToken
HSAILAsmLexer::LexToken()
{
  AsmToken lexedToken = lexDefinite();

  switch (lexedToken.getKind()) {
  default:
    return lexedToken;
  case AsmToken::Error:
    SetError(Lexer->getErrLoc(), Lexer->getErr());
    return lexedToken;

  case AsmToken::Percent: {
    const AsmToken &nextToken = lexTentative();
    if (nextToken.getKind() != AsmToken::Identifier)
      return lexedToken;

    if (unsigned regID = MatchRegisterName(nextToken.getString())) {
      lexDefinite();

      // FIXME: This is completely wrong when there is a space or other
      // punctuation between the % and the register name.
      StringRef regStr(lexedToken.getString().data(),
                       lexedToken.getString().size() +
                       nextToken.getString().size());

      return AsmToken(AsmToken::Register, regStr,
                      static_cast<int64_t>(regID));
    }
  }
  }

  return AsmToken();
}

extern "C" void LLVMInitializeHSAILAsmLexer()
{
  RegisterAsmLexer<HSAILAsmLexer> X(TheHSAIL_32Target);
  RegisterAsmLexer<HSAILAsmLexer> Y(TheHSAIL_64Target);
}
