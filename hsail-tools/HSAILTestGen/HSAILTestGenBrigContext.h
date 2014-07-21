//===-- HSAILTestGenSample.h - HSAIL Test Generator Utilities ------------===//
//
//===----------------------------------------------------------------------===//
//
// (C) 2013 AMD Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDED_HSAIL_TESTGEN_BRIG_CONTEXT_H
#define INCLUDED_HSAIL_TESTGEN_BRIG_CONTEXT_H

#include "HSAILBrigContainer.h"
#include "HSAILItems.h"
#include "HSAILSRef.h"
#include "Brig.h"

#include <string>
#include <sstream>

using std::string;

using HSAIL_ASM::BrigContainer;

using HSAIL_ASM::Code;
using HSAIL_ASM::Directive;
using HSAIL_ASM::DirectiveKernel;
using HSAIL_ASM::DirectiveFunction;
using HSAIL_ASM::DirectiveIndirectFunction;
using HSAIL_ASM::DirectiveSignature;
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
        : disableComments(noComments), mModel(model), mProfile(profile), labCount(0) {}

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

    template<class T>
    T emitSbrStart(const char* name, unsigned outParams, unsigned inParams)
    {
        T sbr = getContainer().append<T>();
        initSbrDef(sbr, name);

        sbr.inArgCount()  = inParams;
        sbr.outArgCount() = outParams;

        emitSbrParams(outParams, false);
        emitSbrParams(inParams,  true);

        sbr.firstCodeBlockEntry() = getContainer().code().end();
        sbr.nextModuleEntry()     = getContainer().code().end();

        registerSbrArgs(sbr);

        return sbr;
    }

    void emitSbrEnd(DirectiveExecutable exe);
    DirectiveVariable emitSbrParams(unsigned num, bool isInputParams);
    DirectiveVariable emitArg(unsigned type, string name, unsigned segment = Brig::BRIG_SEGMENT_ARG);

    void emitAuxLabel();

    void emitComment(string s);

public: // Instructions
    void emitRet();
    //void emitCall(DirectiveFunction func, unsigned outArgs, unsigned inArgs);
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
    Operand emitReg(OperandReg reg);
    Operand emitReg(unsigned size, unsigned idx);
    Operand emitVector(unsigned cnt, unsigned size, unsigned idx0);
    Operand emitVector(unsigned cnt, unsigned size, bool isDst = true, unsigned immCnt = 0);
    Operand emitImm(unsigned size = 32, uint64_t lVal = 0, uint64_t hVal = 0);
    Operand emitWavesize();
    Operand emitFuncRef(DirectiveFunction func);
    Operand emitIFuncRef(DirectiveIndirectFunction func);
    Operand emitKernelRef(DirectiveKernel k);
    Operand emitSignatureRef(DirectiveSignature k);
    Operand emitAddrRef(DirectiveVariable var, OperandReg reg, unsigned offset = 0);
    Operand emitAddrRef(DirectiveVariable var, uint64_t offset = 0);
    Operand emitAddrRef(OperandReg reg, uint64_t offset = 0);
    Operand emitAddrRef(uint64_t offset = 0);
    Operand emitFBarrierRef(DirectiveFbarrier fb);
    Operand emitLabelRef(const char* name);
    Operand emitArgList(unsigned num, bool isInputArgs);

    string getRegName(unsigned size, unsigned idx);

    DirectiveVariable emitSymbol(unsigned type, string name, unsigned segment = Brig::BRIG_SEGMENT_GLOBAL, unsigned dim = 0)
    {
        assert(name.length() > 0);

        DirectiveVariable sym = getContainer().append<DirectiveVariable>();

        sym.name() = name;
        sym.modifier().isConst() = false;
        sym.modifier().isArray() = (dim > 0);
        sym.modifier().isFlexArray() = false;
        sym.modifier().isDefinition() = true;

        sym.linkage() = (name[0] == '%')? Brig::BRIG_LINKAGE_FUNCTION : Brig::BRIG_LINKAGE_MODULE;

        sym.allocation() = Brig::BRIG_ALLOCATION_AUTOMATIC;
        if      (segment == Brig::BRIG_SEGMENT_GLOBAL)   sym.allocation() = Brig::BRIG_ALLOCATION_PROGRAM;
        else if (segment == Brig::BRIG_SEGMENT_READONLY) sym.allocation() = Brig::BRIG_ALLOCATION_AGENT;

        sym.segment() = segment;
        sym.init() = Directive();
        sym.type() = type;
        sym.align() = getNaturalAlignment(type);
        sym.dim() = dim;

        return sym;
    }

    DirectiveFbarrier emitFBarrier(const char* name)
    {
        assert(strlen(name) > 0);

        DirectiveFbarrier fb = getContainer().append<DirectiveFbarrier>();
        fb.name() = name;
        fb.linkage() = (name[0] == '%')? Brig::BRIG_LINKAGE_FUNCTION : Brig::BRIG_LINKAGE_MODULE;
        fb.modifier().isDefinition() = true;

        return fb;
    }

    unsigned getSegAddrSize(unsigned segment)    { return HSAIL_ASM::getSegAddrSize(segment, mModel == Brig::BRIG_MACHINE_LARGE); }
    unsigned getSegAddrType(unsigned segment)    { return (getSegAddrSize(segment) == 32)? Brig::BRIG_TYPE_U32 : Brig::BRIG_TYPE_U64; }
    unsigned conv2LdStType(unsigned type);
};

//=============================================================================
//=============================================================================
//=============================================================================

} // namespace TESTGEN

// ============================================================================

#endif // INCLUDED_HSAIL_TESTGEN_BRIG_CONTEXT_H
