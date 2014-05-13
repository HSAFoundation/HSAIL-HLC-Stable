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
#pragma once
#ifndef INCLUDED_HSAIL_SCANNER_H
#define INCLUDED_HSAIL_SCANNER_H

/*
struct Guard {
    const char **m_beg;
    const char **m_pos;
    Guard(const char **beg, const char **pos)
    : m_beg(beg), m_pos(pos) { }
    ~Guard() {
            std::cout << "\"";
            for(const char *p = *m_beg; p != *m_pos; ++p) {
                    std::cout << *p;
            }
            std::cout << "\" ";
    }
};
*/


#include "Brig.h"
#include "HSAILTypeUtilities.h"
#include "HSAILUtilities.h"
#include "HSAILSRef.h"
#include "HSAILFloats.h"
#include "HSAILConvertors.h"

#include <memory>
#include <vector>
#include <string>
#include <list>
#include <iosfwd>
#include <cassert>

struct SrcLoc
{
    int line;
    int column;
};

void printError(std::ostream& os, std::istream& is, const SrcLoc& errLoc, const char* message);

class SyntaxError
{
    std::string m_errorMessage;
    SrcLoc      m_srcLoc;
public:
    SyntaxError() {
        m_srcLoc.line = 0;
        m_srcLoc.column = 0;
    }
    SyntaxError(const std::string& errorMessage)
        : m_errorMessage(errorMessage)
    {
        m_srcLoc.line = 0;
        m_srcLoc.column = 0;
    }
    SyntaxError(const std::string& errorMessage,const SrcLoc& srcLoc)
        : m_errorMessage(errorMessage)
        , m_srcLoc(srcLoc)
    {
    }
    const std::string& what() const { return m_errorMessage; }
    const SrcLoc& where() const { return m_srcLoc; }
    void print(std::ostream& os, std::istream& is) const {
        printError(os,is,m_srcLoc,m_errorMessage.c_str());
    }
};

class StreamScannerBase
{
    StreamScannerBase& operator=(const StreamScannerBase&);
public:
    typedef std::vector<char>         BufferContainer;

protected:
    const char *m_end;

    std::istream&   m_is;
    BufferContainer m_buffer;

    void readChars(int n);
    void readBuffer();
    std::streamoff  streamPosAt(const char *i) const;

public:
    StreamScannerBase(std::istream& is);
};

namespace HSAIL_ASM
{

enum ETokens
{
    EEmpty,
    EEndOfSource,

    // delimiters
    EQuot,
    ELCurl,
    ERCurl,
    ELParen,
    ERParen,
    ELBrace,
    ERBrace,
    ELAngle,
    ERAngle,
    EDot,
    EComma,
    ESemi,
    EColon,
    EPlus,
    EMinus,
    EEqual,

    // Keywords
    EKWVersion,
    EKWKernel,
    EKWFunction,
    EKWPragma,
    EKWSection,
    EKWRTI,
    EKWLoc,
    EKWConst,
    EKWAlign,
    EKWExtension,

    EKWImageWidth,
    EKWImageHeight,
    EKWImageDepth,
    EKWImageFormat,
    EKWImageOrder,
    EKWImageGeometry,
    EKWImageArray,

    ESamplerFirstProp,
    EKWSamplerBoundaryU = ESamplerFirstProp,
    EKWSamplerBoundaryV,
    EKWSamplerBoundaryW,
    EKWSamplerCoord,
    EKWSamplerFilter,
    ESamplerLastProp = EKWSamplerFilter,

    ESamplerBoundaryMode,
    ESamplerCoord,
    ESamplerFilter,

    EKWBlockStart,
    EKWBlockNum,
    EKWBlockStr,
    EKWBlockEnd,

    EKWFBar,
    EKWLabelTargets,
    EKWSignature,
    EKWWidthAll,
    EKWFbarrier,
    EKWRWImg,
    EKWROImg,
    EKWWOImg,
    EKWSamp,

    // constants
    EDecimalNumber,
    EOctalNumber,
    EHexNumber,
    EHlfHexNumber,
    ESglHexNumber,
    EDblHexNumber,
    EHlfNumber,
    ESglNumber,
    EDblNumber,
    EHlfC99Number,
    ESglC99Number,
    EDblC99Number,
    EPackedLiteral,
    EStringLiteral,
    EEmbeddedText,

    // user names
    ELabel,
    EIDStatic,
    EIDLocal,
    ERegister,

    EAttribute,
    ESegment,

    // other
    EId,
    EWaveSizeMacro,
    EControl,
    ETargetMachine,
    ETargetProfile,
    ETargetSftz,
    EImageFormat,
    EImageOrder,
    EImageGeometry,

    // insts
    EInstNoType,
    EInstBase,
    EInstLdSt,
    EInstAtomic,
    EInstCvt,
    EInstCmp,
    EInstLdc,
    EInstAddrSpace,
    EInstBranch,
    EInstBarrierSync,
    EInstCall,
    EInstModAlu,
    EInstReadImage,
    EInstLdStImage,
    EInstAtomicImage,
    EInstQueryImage,

    EInstSkip, // TBD remove

    EInstruction,
    EInstruction_Vx,

    // modifiers
    EModifiers,
    EMType = EModifiers,
    EMPacking,
    EMMemoryOrder,
    EMMemoryScope,
    EMAtomicOp,
    EMSegment,
    EMNoNull,
    EMWidth,
    EMAlign,
    EMVector,
    EMEquiv,
    EMRound,
    EMFTZ,
    EMHi,
    EMCompare,
    EMGeom,
    EMImageModifier,
    EMFBar,
    EMConst,
    EMMemoryFenceSegments,
    EMSkip, // TBD remove
    EMNone
};

enum EScanContext {
    EDefaultContext,
    EImageOrderContext,
    EInstModifierContext,
    EInstModifierInstAtomicContext,   
};

class Scanner : public StreamScannerBase
{
public:
    explicit Scanner(std::istream& is, bool disableComments=true);

    typedef std::list<SRef> CommentList;

    class Token {
        friend class Scanner;
        Scanner       *m_scanner;
        std::streamoff m_lineStart;
        int            m_lineNum;
        SRef           m_text;
        int            m_brigId;
        ETokens        m_kind;

        CommentList    m_comments;

        void    clear();
        void    appendComment(const char *begin, const char *end);

    public:
        SRef    text()   const { return m_text; } 
        int     brigId() const { return m_brigId; } 
        ETokens kind()   const { return m_kind; } 
        SrcLoc  srcLoc() const;
    };

    typedef const Token CToken;

    CToken& token() const { return *m_curToken; }

    SrcLoc srcLoc(const CToken& t) const { 
      std::streamoff const posOfs = streamPosAt(t.m_text.begin);
      assert(posOfs >= t.m_lineStart);
      SrcLoc const res = { t.m_lineNum, static_cast<int>(posOfs - t.m_lineStart) };
      return res;
    }

    CToken& peek(EScanContext ctx=EDefaultContext);
    CToken& scan(EScanContext ctx=EDefaultContext);

    void readSingleStringLiteral(std::string& outString);

    template <typename DstBrigType,
              template<typename, typename> class Convertor>
    typename DstBrigType::CType readIntValue();

    template <typename DstBrigType,
              template<typename, typename> class Convertor>
    typename DstBrigType::CType readValue();

    bool hasComments() const { return m_comments.get()!=NULL && !m_comments->empty(); }
    SRef grabComment();

    void syntaxError(const std::string& message, const SrcLoc& srcLoc) const {
        throw SyntaxError(message, srcLoc);
    }

    void syntaxError(const std::string& message, CToken* t) const {
        syntaxError(message, t->srcLoc());
    }

    void syntaxError(const std::string& message) const {
        syntaxError(message, m_curToken ? m_curToken->srcLoc() : SrcLoc());
    }

    void throwTokenExpected(ETokens token, const char* message, const SrcLoc& loc);

    static EScanContext getTokenContext(ETokens token);

    unsigned eatToken(ETokens token, const char* message=NULL) {
        CToken& t = scan(getTokenContext(token));
        if (t.kind()!=token) {
            throwTokenExpected(token, message, t.srcLoc());
        }
        return t.brigId();
    }

    Optional<unsigned> tryEatToken(ETokens token) {
        EScanContext const ctx = getTokenContext(token);
        Optional<unsigned> res;
        if (peek(ctx).kind()==token) {
            res = scan(ctx).brigId();
        }
        return res;
    }

private:
    Scanner& operator=(const Scanner&);

    Token   m_pool[2]; // circular pool - one for current, one for peek
    Token*  m_curToken;
    Token*  m_peekToken;     

    int                        m_lineNum;
    std::streamoff             m_lineStart;
    std::auto_ptr<CommentList> m_comments;

    class istringstreamalert;
    class Variant;

    Token&       newToken();
    Token&       scanNext(EScanContext ctx);

    void         readSingleStringLiteral(Token &t, std::string& outString);
    ETokens      scanDefault(EScanContext ctx, Token &t);
    ETokens      scanModifier(EScanContext ctx, Token &t);
    Variant      readValueVariant();
    void         newComment(const char *init=NULL);
    void         appendComment(const char *begin, const char *end);
    void         nextLine(const char *atPos);
    void         skipWhitespaces(Token& t);
    const char*  skipOneLinearComment(const char* from, Token& t);
    const char*  skipMultilineComment(const char* from, Token& t);
    void         scanEmbeddedText(Token &t);

    SrcLoc       srcLoc(const char* pos) const;

    void syntaxError(const char* pos, const std::string& message) const {
        syntaxError(message, srcLoc(pos));
    }
};

inline SrcLoc Scanner::Token::srcLoc() const { 
    return m_scanner->srcLoc(*this);
}

class Scanner::Variant
{
    union {
        int64_t  m_int64;
        uint64_t m_uint64;
        uint16_t m_f16x;
        uint32_t m_f32x;
        uint64_t m_f64x;
    };
    enum EKind {
        EInvalid,
        EInt64,
        EUInt64,
        EF16,
        EF32,
        EF64
    } m_kind;

public:
    Variant() : m_kind(EInvalid) {}
    explicit Variant(int64_t  i64)  : m_int64(i64), m_kind(EInt64) {}
    explicit Variant(uint64_t u64)  : m_uint64(u64), m_kind(EUInt64) {}
    explicit Variant(f16_t    f16)  : m_f16x(f16.rawBits()), m_kind(EF16) {}
    explicit Variant(f32_t    f32)  : m_f32x(f32.rawBits()), m_kind(EF32) {}
    explicit Variant(f64_t    f64)  : m_f64x(f64.rawBits()), m_kind(EF64) {}

    bool isInteger() const {
        return m_kind == EInt64 || m_kind == EUInt64;
    }

    template <typename DstBrigType,
              template<typename, typename> class Convertor>
    typename DstBrigType::CType convertInt() const {
        switch(m_kind) {
        case EInt64:  return ::HSAIL_ASM::convert<DstBrigType, BrigType<Brig::BRIG_TYPE_S64>, Convertor>(m_int64);
        case EUInt64: return ::HSAIL_ASM::convert<DstBrigType, BrigType<Brig::BRIG_TYPE_U64>, Convertor>(m_uint64);
        default: assert(false);
        }
        return typename DstBrigType::CType();
    }

    template <typename DstBrigType,
              template<typename, typename> class Convertor>
    typename DstBrigType::CType convert() const {
        switch(m_kind) {
        case EInt64:
        case EUInt64: return convertInt<DstBrigType,Convertor>();
        case EF16:    return ::HSAIL_ASM::convert<DstBrigType, BrigType<Brig::BRIG_TYPE_F16>, Convertor>(f16_t::fromRawBits(m_f16x));
        case EF32:    return ::HSAIL_ASM::convert<DstBrigType, BrigType<Brig::BRIG_TYPE_F32>, Convertor>(f32_t::fromRawBits(m_f32x));
        case EF64:    return ::HSAIL_ASM::convert<DstBrigType, BrigType<Brig::BRIG_TYPE_F64>, Convertor>(f64_t::fromRawBits(m_f64x));
        default: assert(false);
        }
        return typename DstBrigType::CType();;
    }
};

template <typename DstBrigType,
          template<typename, typename> class Convertor>
typename DstBrigType::CType Scanner::readIntValue() {
    try {
        Variant const v = readValueVariant();
        if (!v.isInteger()) {
            syntaxError("integer constant expected");
        }
        return v.convertInt<DstBrigType,Convertor>();
    } catch (const ConversionError& e) {
        syntaxError(e.what()); // translate it to syntax error
    }
    return typename DstBrigType::CType();
}

// more generic version
template <typename DstBrigType,
          template<typename, typename> class Convertor>
typename DstBrigType::CType Scanner::readValue() {
    try {
        Variant const v = readValueVariant();
        return v.convert<DstBrigType,Convertor>();
    } catch (const ConversionError& e) {
        syntaxError(e.what()); // translate it to syntax error
    }
    return typename DstBrigType::CType();
}

} // end namespace

#endif // INCLUDED_HSAIL_SCANNER_H
