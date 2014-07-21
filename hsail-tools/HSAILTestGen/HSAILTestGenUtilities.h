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

//=============================================================================
//=============================================================================
//=============================================================================
// AluModifier Wrapper - defined to unify emulation of InstBasic and InstMod instructions

class AluMod
{
private:
    static const unsigned ROUNDING = 0x1F;
    static const unsigned FTZ      = 0x20;

private:
    unsigned bits;

public:
    static const unsigned ROUNDING_NONE       = Brig::BRIG_ROUND_NONE;
    static const unsigned ROUNDING_NEARI      = Brig::BRIG_ROUND_INTEGER_NEAR_EVEN;
    static const unsigned ROUNDING_NEAR       = Brig::BRIG_ROUND_FLOAT_NEAR_EVEN;
    static const unsigned ROUNDING_ZEROI      = Brig::BRIG_ROUND_INTEGER_ZERO;
    static const unsigned ROUNDING_ZERO       = Brig::BRIG_ROUND_FLOAT_ZERO;
    static const unsigned ROUNDING_UPI        = Brig::BRIG_ROUND_INTEGER_PLUS_INFINITY;
    static const unsigned ROUNDING_UP         = Brig::BRIG_ROUND_FLOAT_PLUS_INFINITY;
    static const unsigned ROUNDING_DOWNI      = Brig::BRIG_ROUND_INTEGER_MINUS_INFINITY;
    static const unsigned ROUNDING_DOWN       = Brig::BRIG_ROUND_FLOAT_MINUS_INFINITY;
    static const unsigned ROUNDING_NEARI_SAT  = Brig::BRIG_ROUND_INTEGER_NEAR_EVEN_SAT;
    static const unsigned ROUNDING_ZEROI_SAT  = Brig::BRIG_ROUND_INTEGER_ZERO_SAT;
    static const unsigned ROUNDING_UPI_SAT    = Brig::BRIG_ROUND_INTEGER_PLUS_INFINITY_SAT;
    static const unsigned ROUNDING_DOWNI_SAT  = Brig::BRIG_ROUND_INTEGER_MINUS_INFINITY_SAT;

    static const unsigned ROUNDING_SNEARI     = Brig::BRIG_ROUND_INTEGER_SIGNALLING_NEAR_EVEN;
    static const unsigned ROUNDING_SZEROI     = Brig::BRIG_ROUND_INTEGER_SIGNALLING_ZERO;
    static const unsigned ROUNDING_SUPI       = Brig::BRIG_ROUND_INTEGER_SIGNALLING_PLUS_INFINITY;
    static const unsigned ROUNDING_SDOWNI     = Brig::BRIG_ROUND_INTEGER_SIGNALLING_MINUS_INFINITY;
    static const unsigned ROUNDING_SNEARI_SAT = Brig::BRIG_ROUND_INTEGER_SIGNALLING_NEAR_EVEN_SAT;
    static const unsigned ROUNDING_SZEROI_SAT = Brig::BRIG_ROUND_INTEGER_SIGNALLING_ZERO_SAT;
    static const unsigned ROUNDING_SUPI_SAT   = Brig::BRIG_ROUND_INTEGER_SIGNALLING_PLUS_INFINITY_SAT;
    static const unsigned ROUNDING_SDOWNI_SAT = Brig::BRIG_ROUND_INTEGER_SIGNALLING_MINUS_INFINITY_SAT;

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

    bool isSignaling()           
    {
        return getRounding() == ROUNDING_SNEARI     ||
               getRounding() == ROUNDING_SZEROI     ||
               getRounding() == ROUNDING_SUPI       ||
               getRounding() == ROUNDING_SDOWNI     ||
               getRounding() == ROUNDING_SNEARI_SAT ||
               getRounding() == ROUNDING_SZEROI_SAT ||
               getRounding() == ROUNDING_SUPI_SAT   ||
               getRounding() == ROUNDING_SDOWNI_SAT;
    }
};

//=============================================================================
//=============================================================================
//=============================================================================

void assign(Inst i, int idx, Operand opr);
void append(Inst inst, Operand opr0, Operand opr1 = Operand(), Operand opr2 = Operand());

string index2str(unsigned idx, unsigned width = 0);

} // namespace TESTGEN

// ============================================================================

#endif // INCLUDED_HSAIL_TESTGEN_UTILITIES_H
