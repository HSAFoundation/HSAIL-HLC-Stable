//===-- HSAILTestGenSample.h - HSAIL Test Generator Utilities ------------===//
//
//===----------------------------------------------------------------------===//
//
// (C) 2013 AMD Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDED_HSAIL_TESTGEN_UTILITIES_H
#define INCLUDED_HSAIL_TESTGEN_UTILITIES_H

#include "HSAILBrigContainer.h"
#include "HSAILItems.h"
#include "HSAILSRef.h"
#include "Brig.h"

#include <string>
#include <sstream>
#include <iomanip>

using std::string;

using HSAIL_ASM::BrigContainer;

using HSAIL_ASM::Directive;
using HSAIL_ASM::DirectiveKernel;
using HSAIL_ASM::DirectiveFunction;
using HSAIL_ASM::DirectiveExecutable;
using HSAIL_ASM::DirectiveVariable;
using HSAIL_ASM::DirectiveFbarrier;
using HSAIL_ASM::Inst;
using HSAIL_ASM::Operand;
using HSAIL_ASM::OperandReg;
using HSAIL_ASM::SRef;

using HSAIL_ASM::getNaturalAlignment;

namespace TESTGEN {

// ============================================================================
// ============================================================================
//============================================================================
// Exceptions

class TestGenError
{
private:
    string msg;

public:
    TestGenError() {}
    TestGenError(string s) : msg(s) {}
    ~TestGenError() {}

    const char *what()   const { return msg.c_str(); };
};

// ============================================================================
// ============================================================================
// ============================================================================
// Context for BRIG generation

class BrigContext
{
private:
    BrigContainer   container;    
    DirectiveKernel kernel;
    const bool      disableComments; // True if comments generation shall be disabled, false otherwise
    const unsigned  mModel;          // True if small model is used, false otherwise
    const unsigned  mProfile;        // True if base profile is used, false otherwise
    unsigned        labCount;        // Number of autogenerated labels

private:
    BrigContext(const BrigContext&); // non-copyable
    const BrigContext &operator=(const BrigContext &);  // not assignable

public:
    BrigContext(unsigned model, unsigned profile, bool noComments)
        : mModel(model), mProfile(profile), disableComments(noComments), labCount(0) {}

public:
    BrigContainer& getContainer() { return container; }

    unsigned getModel()   { return mModel;   }
    unsigned getProfile() { return mProfile; }

public: // Directives
    void emitVersion();
    void emitExtension(const char* name);

    void initSbrDef(DirectiveExecutable sbr, string name);
    void registerSbrArgs(DirectiveExecutable sbr);
    void endSbrBody(DirectiveExecutable sbr);

    DirectiveKernel emitKernel(string name);
    DirectiveKernel getLastKernel() { return kernel; }

    DirectiveFunction emitFuncStart(const char* name, unsigned outParams, unsigned inParams);
    void emitFuncEnd(DirectiveFunction func);
    DirectiveVariable emitFuncParams(unsigned num, bool isInputParams);
    DirectiveVariable emitArg(unsigned type, string name, unsigned segment = Brig::BRIG_SEGMENT_ARG);

    void emitAuxLabel();

    void emitComment(string s);

public: // Instructions
    void emitRet();
    void emitCodeBlockEnd();
    void emitCall(DirectiveFunction func, unsigned outArgs, unsigned inArgs);
    void emitSt(unsigned type, unsigned segment, Operand from, Operand to);
    void emitLd(unsigned type, unsigned segment, Operand to, Operand from, unsigned width = Brig::BRIG_WIDTH_1);
    void emitMov(unsigned type, Operand to, Operand from);
    void emitAdd(unsigned type, Operand res, Operand op1, Operand op2);
    void emitSub(unsigned type, Operand res, Operand op1, Operand op2);
    void emitShl(unsigned type, Operand res, Operand src, unsigned shift);
    void emitMul(unsigned type, Operand res, Operand op1, unsigned multiplier);
    void emitGetWorkItemId(Operand res, unsigned dim);
    void emitCvt(unsigned dstType, unsigned srcType, OperandReg to, OperandReg from);
    void emitLda(OperandReg dst, DirectiveVariable var);

public: // Operands
    Operand emitReg(SRef regName);
    Operand emitReg(unsigned size, unsigned idx = 0);
    Operand emitVector(unsigned cnt, unsigned size, unsigned idx0);
    Operand emitVector(unsigned cnt, unsigned size, bool isDst = true, unsigned immCnt = 0);
    Operand emitImm(unsigned size = 32, uint64_t lVal = 0, uint64_t hVal = 0);
    Operand emitWavesize();
    Operand emitFuncRef(DirectiveFunction func);
    Operand emitAddrRef(Directive var, OperandReg reg, unsigned offset = 0);
    Operand emitAddrRef(Directive var, unsigned offset = 0);
    Operand emitAddrRef(OperandReg reg, uint64_t offset = 0);
    Operand emitAddrRef(uint64_t offset = 0);
    Operand emitFBarrierRef(DirectiveFbarrier fb);
    Operand emitLabelRef(const char* name);
    Operand emitArgList(unsigned num, bool isInputArgs);

    string getRegName(unsigned size, unsigned idx);

    DirectiveVariable emitSymbol(unsigned type, string name, unsigned segment = Brig::BRIG_SEGMENT_GLOBAL, unsigned dim = 0)
    {
        DirectiveVariable sym = getContainer().append<DirectiveVariable>();

        sym.name() = name;
        sym.modifier().linkage() = Brig::BRIG_LINKAGE_NONE;
        sym.modifier().isConst() = false;
        sym.modifier().isArray() = (dim > 0);
        sym.modifier().isFlexArray() = false;
        sym.modifier().isDeclaration() = false;
        sym.code() = getContainer().insts().end();
        sym.segment() = segment;
        sym.init() = Directive();
        sym.type() = type;
        sym.align() = getNaturalAlignment(type);
        sym.dim() = dim;

        return sym;
    }

    DirectiveFbarrier emitFBarrier(const char* name)
    {
        DirectiveFbarrier fb = getContainer().append<DirectiveFbarrier>();
        fb.code() = getContainer().insts().end();
        fb.name() = name;

        return fb;
    }

    unsigned getSegAddrSize(unsigned segment)    { return HSAIL_ASM::getSegAddrSize(segment, mModel == Brig::BRIG_MACHINE_LARGE); }
    unsigned getSegAddrType(unsigned segment)    { return (getSegAddrSize(segment) == 32)? Brig::BRIG_TYPE_U32 : Brig::BRIG_TYPE_U64; }
    unsigned conv2LdStType(unsigned type);
};

//=============================================================================
//=============================================================================
//=============================================================================
// AluModifier Wrapper - defined to unify emulation of InstBasic and InstMod instructions

class AluMod
{
private:
    static const unsigned ROUNDING = 0xF;
    static const unsigned FTZ      = 0x10;

private:
    unsigned bits;

public:
    static const unsigned ROUNDING_NONE      = Brig::BRIG_ROUND_NONE;
    static const unsigned ROUNDING_NEARI     = Brig::BRIG_ROUND_INTEGER_NEAR_EVEN;
    static const unsigned ROUNDING_NEAR      = Brig::BRIG_ROUND_FLOAT_NEAR_EVEN;
    static const unsigned ROUNDING_ZEROI     = Brig::BRIG_ROUND_INTEGER_ZERO;
    static const unsigned ROUNDING_ZERO      = Brig::BRIG_ROUND_FLOAT_ZERO;
    static const unsigned ROUNDING_UPI       = Brig::BRIG_ROUND_INTEGER_PLUS_INFINITY;
    static const unsigned ROUNDING_UP        = Brig::BRIG_ROUND_FLOAT_PLUS_INFINITY;
    static const unsigned ROUNDING_DOWNI     = Brig::BRIG_ROUND_INTEGER_MINUS_INFINITY;
    static const unsigned ROUNDING_DOWN      = Brig::BRIG_ROUND_FLOAT_MINUS_INFINITY;
    static const unsigned ROUNDING_NEARI_SAT = Brig::BRIG_ROUND_INTEGER_NEAR_EVEN_SAT;
    static const unsigned ROUNDING_ZEROI_SAT = Brig::BRIG_ROUND_INTEGER_ZERO_SAT;
    static const unsigned ROUNDING_UPI_SAT   = Brig::BRIG_ROUND_INTEGER_PLUS_INFINITY_SAT;
    static const unsigned ROUNDING_DOWNI_SAT = Brig::BRIG_ROUND_INTEGER_MINUS_INFINITY_SAT;

public:
    AluMod(unsigned val = ROUNDING_NONE) : bits(val) {}

public:
    bool isFtz()           { return (bits & FTZ) != 0; }
    unsigned getRounding() { return bits & ROUNDING; }

    bool isSat()           
    {
        return getRounding() == ROUNDING_NEARI_SAT ||
               getRounding() == ROUNDING_ZEROI_SAT ||
               getRounding() == ROUNDING_UPI_SAT   ||
               getRounding() == ROUNDING_DOWNI_SAT;
    }
};

//=============================================================================
//=============================================================================
//=============================================================================

} // namespace TESTGEN

// ============================================================================

#endif // INCLUDED_HSAIL_TESTGEN_UTILITIES_H
