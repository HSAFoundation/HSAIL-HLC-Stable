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
#ifndef INCLUDED_HSAIL_PARSER_H
#define INCLUDED_HSAIL_PARSER_H

#include "HSAILScanner.h"
#include "HSAILItems.h"
#include "HSAILBrigantine.h"

#include <stdexcept>
#include <memory>

namespace HSAIL_ASM
{

Inst parseMnemo(const char* str, Brigantine& bw);

class Parser
{
public:
    Parser(Scanner& scanner, BrigContainer& container);

    void parseSource();

private:
    Scanner&    m_scanner;
    Brigantine  m_bw;
    bool        m_gcnEnabled;   
    std::string m_srcFileName;

    Parser& operator=(const Parser&);
    
    Scanner::CToken& scan()  { return m_scanner.scan(); }
    Scanner::CToken& peek()  { return m_scanner.peek(); }
    Scanner::CToken& token() { return m_scanner.token(); }

    unsigned           eatToken(ETokens token, const char* message = 0) { return m_scanner.eatToken(token, message); }
    Optional<unsigned> tryEatToken(ETokens token)                       { return m_scanner.tryEatToken(token); }

    void syntaxError(const std::string& message, const SourceInfo* srcInfo=NULL);
    void syntaxError(const std::string& message, Scanner::CToken& token);

    //void    scan();
    //void    expect(ETokens token, const char* message = 0);
    //void    throwTokenExpected(ETokens token, const char* message);
    //SourceInfo tokenSourceInfo() const;

    void parseProgram();
    void parseVersion();
    void parseTopLevelStatement();
    Optional<uint16_t> tryParseFBar();
    void parseKernel(const struct DeclPrefix* declPrefix=NULL);
    void parseFunction(const struct DeclPrefix* declPrefix=NULL);
    void parseSigArgs(Scanner& s,DirectiveSignatureArguments types, DirectiveSignatureArguments::ArgKind argKind);
    void parseSignature();
    int  parseCodeBlock(); // returns the number of instructions inside
    int  parseBodyStatement(); // returns the number of instructions inside
    void parseLabel();
    void parseLabelTargets();
    Inst parseInst();

    DirectiveVariable parseDecl(bool isArg, bool isLocal);
    DirectiveVariable parseDecl(bool isArg, bool isLocal,const struct DeclPrefix& declPfx);
    struct DeclPrefix parseDeclPrefix();

    Directive                   parseVariableInitializer(Brig::BrigType16_t type, unsigned expectedSize);
    DirectiveImageInit          parseImageInitializer(Brig::BrigType16_t type, unsigned expectedSize);
    DirectiveSamplerInit        parseSamplerInitializer(Brig::BrigType16_t type, unsigned expectedSize);
    DirectiveImageProperties    parseImageProperties();
    DirectiveSamplerProperties  parseSamplerProperties();

    DirectiveFbarrier parseFbarrier(bool isLocal);

    void parseDebug();
    void parsePragma();
    void parseEmbeddedText();
    void parseRTI();
    void parseLocation();
    void parseExtension();
    void parseFileDecl();
    void parseControl();
   
    unsigned parseValueList(Brig::BrigType16_t type, class ArbitraryData& data, unsigned maxValues=0);

    void parseBlock();
    
    typedef void (Parser::*OperandParser)(Inst);
    static OperandParser getOperandParser(Brig::BrigOpcode16_t opcode);

    Inst parseInstLdSt();
    Inst parseInstLane();
    Inst parseInstCombineExpand(unsigned operandIdx); 
    Inst parseInstImage();

    int  parseArgScope();

    void skipInst();

    void checkVxIsValid(int Vx, Operand o);

    void parseOperands(Inst inst);
    void parseLdcOperands(Inst inst);
    void parseCallOperands(Inst inst);
    void parseRdImageOperands(Inst inst);
    void parseAtomicNoRetImageOperands(Inst inst);
    void parseQueryOperands(Inst inst);
    void parseNoOperands(Inst inst);

    Operand parseOperandGeneric(unsigned requiredType);

    void parseOperandGeneric(Inst inst, unsigned opndIdx);
    Operand parseConstantGeneric(unsigned requiredType);
    void validateImmType(unsigned required, unsigned actual);

    OperandRef parseOperandRef();
    OperandReg parseOperandReg();
    Operand parseOperandVector(unsigned requiredType);

    Operand parseLabelOperand();
    OperandFunctionRef parseFunctionRef();
    Operand parseSigRef();

    Operand parseOperandInBraces();
    Operand parseActualParamList();

    Operand parseObjectOperand();
    void parseAddress(SRef& reg, int64_t& offset);

    template <typename List>
    unsigned parseLabelList(List list, unsigned expectedSize);

    void storeComments(Inst before=Inst());
};

inline void Parser::syntaxError(const std::string& message,const SourceInfo* srcInfo) {
    if (srcInfo) {
        SrcLoc const srcLoc = { srcInfo->line, srcInfo->column };
        m_scanner.syntaxError(message,srcLoc);
    } else {
        m_scanner.syntaxError(message);
    }
}

inline SourceInfo sourceInfo(const Scanner::Token& t) {
    SrcLoc const srcLoc = t.srcLoc();
    return SourceInfo(srcLoc.line,srcLoc.column);
}

inline void Parser::syntaxError(const std::string& message, Scanner::CToken& token) {
    SourceInfo const srcInfo = sourceInfo(token);
    syntaxError(message, &srcInfo);       
}

} // end namespace

#endif

