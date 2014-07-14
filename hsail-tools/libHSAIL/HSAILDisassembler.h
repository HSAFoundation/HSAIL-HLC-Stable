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
//===-- HSAILDisassembler.h  - BRIG Disassembler ----------------------===//

#ifndef INCLUDED_HSAIL_DISASSEMBLER_H
#define INCLUDED_HSAIL_DISASSEMBLER_H

#include "HSAILBrigContainer.h"
#include "HSAILItems.h"
#include "HSAILUtilities.h"
#include "HSAILFloats.h"

#include <iosfwd>
#include <sstream>

namespace HSAIL_ASM {

enum EFloatDisassemblyMode {
    FloatDisassemblyModeRawBits,
    FloatDisassemblyModeC99,
    FloatDisassemblyModeDecimal
};

void printFloatValue(std::ostream& stream, int mode, f64_t val);
void printFloatValue(std::ostream& stream, int mode, f16_t val);
void printFloatValue(std::ostream& stream, int mode, f32_t val);

class Disassembler {
private:
    BrigContainer&        brig;
    std::ostream*         err;

    mutable std::ostream *stream;
    mutable int           indent;
    mutable bool          hasErr;
    mutable unsigned      mModel;
    mutable unsigned      mProfile;
    unsigned              m_options;

    static const int BRIG_OPERANDS_NUM = 5;

    Disassembler(const Disassembler&); // non-copyable
    const Disassembler &operator=(const Disassembler &);  // not assignable

    class ValueListPrinter;

    //-------------------------------------------------------------------------
    // Public Disassembler API
public:
    enum OutputOptions {
        FloatModeMask = 3,
        PrintInstOffset = 4
    };

    Disassembler(BrigContainer& c, EFloatDisassemblyMode fmode=FloatDisassemblyModeRawBits)
        : brig(c), err(0), stream(0), indent(0), hasErr(false), 
          mModel(Brig::BRIG_MACHINE_LARGE), mProfile(Brig::BRIG_PROFILE_FULL),
          m_options(fmode)
    {}

    void setOutputOptions(unsigned mask) { m_options = mask; }

    int run(std::ostream &s) const;       // Disassemble all BRIG container to stream
    int run(const char* path) const;      // Disassemble all BRIG container to file

    std::string get(Directive d, unsigned model, unsigned profile);   // Disassemble one directive as string
    std::string get(Inst i,      unsigned model, unsigned profile);   // Disassemble one instruction as string
    std::string get(Operand i,   unsigned model, unsigned profile);   // Disassemble one operand as string

    void log(std::ostream &s);                 // Request errors logging into stream s
    bool hasError() const { return hasErr; }   // Return error flag
    void clrError()       { hasErr = false; }  // Clear error flag

    //-------------------------------------------------------------------------
    // Directives
private:

    void printDirectiveFmt(Directive d) const;
    void printDirective(Directive d, bool dump = false) const;

    void printDirective(DirectiveVersion d) const;
    void printDirective(DirectiveKernel d) const;
    void printDirective(DirectiveFunction d) const;
    void printDirective(DirectiveLabel d) const;
    void printDirective(DirectiveComment d) const;
    void printDirective(DirectiveControl d) const;
    void printDirective(DirectiveLoc d) const;
    void printDirective(DirectiveExtension d) const;
    void printDirective(DirectivePragma d) const;
    void printDirective(DirectiveSignature d) const;
    void printDirective(DirectiveLabelTargets d) const;
    void printDirective(DirectiveArgScopeStart d) const;
    void printDirective(DirectiveArgScopeEnd d) const;
    void printDirective(BlockStart d) const;
    void printDirective(BlockEnd d) const;
    void printDirective(BlockNumeric d) const;
    void printDirective(BlockString d) const;

    void printDirective(DirectiveVariable d) const;
    void printDirective(DirectiveVariableInit d) const;
    void printDirective(DirectiveLabelInit d) const;
    void printDirective(DirectiveImageInit d, unsigned type) const;
    void printDirective(DirectiveSamplerInit d, unsigned type) const;
    void printDirective(DirectiveImageProperties d, unsigned type) const;
    void printDirective(DirectiveSamplerProperties d, unsigned type) const;
    void printDirective(DirectiveFbarrier d) const;


    Directive printArgs(Directive arg, unsigned paramNum, Directive scoped) const;
    template <typename List>
    void printLabelList(List list) const;

    void printBody(Inst inst, unsigned instNum, Directive start, Directive end, bool isDecl = false) const;
    Directive printContextDir(Offset off, Directive start, Directive end) const;

    void printSymDecl(DirectiveVariable d) const;
    void printArgDecl(Directive d) const;

    void printProtoType(DirectiveSignatureArgument type) const;

    void printValueList(DataItem data, Brig::BrigType16_t type, unsigned maxElements) const;

    void printStringLiteral(SRef s) const;
    void printComment(SRef s) const;

    Directive next(Directive d) const;

    //-------------------------------------------------------------------------
    // Initializers

    template<typename T> void printInitializer(DirectiveVariable s) const;

    //-------------------------------------------------------------------------
    // Instructions

    void printInstFmt(Inst i) const;
    void printInst(Inst i) const;

    void printInst(InstBasic i) const;
    void printInst(InstMod i) const;
    void printInst(InstAddr i) const;
    void printInst(InstBr i) const;
    void printInst(InstMem i) const;
    void printInst(InstCmp i) const;
    void printInst(InstCvt i) const;
    void printInst(InstAtomic i) const;
    void printInst(InstImage i) const;
    void printInst(InstLane i) const;
    void printInst(InstMemFence i) const;
    void printInst(InstQueue i) const;
    void printInst(InstSeg i) const;
    void printInst(InstSegCvt i) const;
    void printInst(InstSourceType i) const;
    void printInst(InstSignal i) const;
    void printInst(InstQueryImage i) const;
    void printInst(InstQuerySampler i) const;
    void printNop() const;

    void printCallArgs(Inst i) const;
    void printInstArgs(Inst i, int firstArg = 0, int lastArg = BRIG_OPERANDS_NUM) const;
    template<class T> void print_width(T inst) const;
    void print_v(Inst i) const;

    template <typename Inst>
    void print_rounding(Inst i) const;

    //-------------------------------------------------------------------------
    // Operands

    void printOperand(Inst i, unsigned operandIdx) const;
    void printOperand(Operand opr, bool dump = false) const;

    void printOperand(OperandReg opr) const;
    void printOperand(OperandVector opr) const;
    void printOperand(OperandWavesize opr) const;
    void printOperand(OperandAddress opr) const;
    void printOperand(OperandLabelRef opr) const;
    void printOperand(OperandLabelTargetsRef opr) const;
    void printOperand(OperandLabelVariableRef opr) const;
    void printOperand(OperandFunctionRef opr) const;
    void printOperand(OperandArgumentList opr) const;
    void printOperand(OperandFunctionList opr) const;
    void printOperand(OperandSignatureRef opr) const;
    void printOperand(OperandFbarrierRef opr) const;

    void printOperandVector(Inst inst, OperandVector opr, unsigned operandIdx) const;
    void printOperandImmed(Inst inst, OperandImmed opr, unsigned operandIdx) const;
    void printOperandImmed(OperandImmed imm, unsigned requiredType) const;

    SRef getSymbolName(Directive d) const;

    //-------------------------------------------------------------------------
    // BRIG properties

    const char* opcode2str(unsigned opcode) const;
    const char* type2str(unsigned t) const;
    const char* pack2str(unsigned t) const;
    const char* seg2str(Brig::BrigSegment8_t  segment) const;
    const char* cmpOp2str(unsigned opcode) const;
    const char* atomicOperation2str(unsigned op) const;
    const char* imageGeometry2str(unsigned g) const;
    const char* samplerCoordNormalization2str(unsigned val) const;
    const char* samplerFilter2str(unsigned val) const;
    const char* samplerAddressing2str(unsigned val) const;
    const char* samplerQuery2str(unsigned g) const;
    const char* imageQuery2str(unsigned g) const;
    const char* machineModel2str(unsigned machineModel) const;
    const char* profile2str(unsigned profile) const;
    const char* ftz2str(unsigned ftz) const;
    const char* round2str(unsigned val) const;
    const char* memoryOrder2str(unsigned memOrder) const;
    const char* memoryFenceSegments2str(unsigned flags) const;
    const char* memoryScope2str(unsigned flags) const;
    const char* class2str(unsigned val) const;
    const char* v2str(Operand opr) const;
    const char* imageChannelType2str(Brig::BrigImageChannelType8_t fmt) const;
    const char* imageChannelOrder2str(Brig::BrigImageChannelOrder8_t order) const;
    const char* width2str(unsigned val) const;
    const char* const2str(bool isConst) const;
    const char* nonull2str(bool isNoNull) const;

    std::string attr2str_(Brig::BrigLinkage8_t attr) const;
    const char* const2str_(bool isConst) const;
    std::string      align2str_(unsigned val, unsigned type) const;
    std::string      align2str(unsigned val) const;
    std::string      equiv2str(unsigned val) const;
    std::string      modifiers2str(AluModifier mod) const;

    bool hasType(Inst i) const;
    bool isCall(Inst i) const;
    bool isBranch(Inst i) const;
    bool isGcnInst(Inst i) const;

    //-------------------------------------------------------------------------
    // Formatting

    template<typename T>
    void printValue(T val) const { *stream << val; }

    void printValue(char arg) const { *stream << (int)arg; }
    void printValue(unsigned char arg) const { *stream << (int)arg; }
    void printValue(signed char arg) const { *stream << (int)arg; }

    void printValue(f16_t val) const { printFloatValue(*stream, m_options & FloatModeMask, val); }
    void printValue(f32_t val) const { printFloatValue(*stream, m_options & FloatModeMask, val); }
    void printValue(f64_t val) const { printFloatValue(*stream, m_options & FloatModeMask, val); }

    void printValue(const b128_t& val) const;

    template<typename T, size_t N>
    void printValue(const MySmallArray<T,N>& v) const {
        printPackedValue(v.arrayType());
    }

    template<typename T, size_t N>
    void printPackedValue(const T (&val)[N]) const {
        *stream << '_' << typeX2str(CType2Brig<T,N>::value) << '(';
        for(int i=N-1; i>0; --i) {
            printValue(val[i]);
            *stream << ',';
        }
        printValue(val[0]);
        *stream << ')';
    }

    class DisassembleOperandImmed;

    template<typename T1>
    void print(T1 val1) const { *stream << val1; }
    template<typename T1, typename T2>
    void print(T1 val1, T2 val2) const { *stream << val1 << val2; }
    template<typename T1, typename T2, typename T3>
    void print(T1 val1, T2 val2, T3 val3) const { *stream << val1 << val2 << val3; }
    template<typename T1, typename T2, typename T3, typename T4>
    void print(T1 val1, T2 val2, T3 val3, T4 val4) const { *stream << val1 << val2 << val3 << val4; }
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void print(T1 val1, T2 val2, T3 val3, T4 val4, T5 val5) const { *stream << val1 << val2 << val3 << val4 << val5; }

    void print_(const char* s) const { assert(s); if (*s) *stream << '_' << s; }
    void print_(std::string s)      const { if (!s.empty()) *stream << '_' << s; }
    template<typename T1>
    void print_(const char* s, T1 val1) const { assert(s); if (*s) { *stream << '_' << s; } *stream << val1; }

    template<typename T1>
    void printq(bool cond, T1 val1) const { if (cond) *stream << val1; }
    template<typename T1, typename T2>
    void printq(bool cond, T1 val1, T2 val2) const { if (cond) *stream << val1 << val2; }
    template<typename T1, typename T2, typename T3>
    void printq(bool cond, T1 val1, T2 val2, T3 val3) const { if (cond) *stream << val1 << val2 << val3; }

    void printIndent()    const { for (int i = indent; i > 0; --i) *stream << "\t"; }
    void printSeparator() const { *stream << '\t'; }
    void printEOL()       const { *stream << '\n'; }

    void add2ValList(std::string &res, const char* valName, const std::string& val) const 
    {
        if (!val.empty()) // to skip unspecified values
        {
            if (!res.empty()) res += ", ";
            res += valName + (" = " + val);
        }
    }

    void add2ValList(std::string &res, const char* valName, unsigned val) const 
    {
        if (val == 0) return;
        std::ostringstream s;
        s << val;
        add2ValList(res, valName, s.str());
    }

    //-------------------------------------------------------------------------
    // Shortcuts

    template<class T>
    std::string getImpl(T d) const {
        std::ostringstream os;
        stream = &os;
        if (d) printBrig(d);
        return os.str();
    }
    void printBrig(Directive d) const { printDirective(d, true); }
    void printBrig(Inst i)      const { printInst(i); }
    void printBrig(Operand opr) const { printOperand(opr, true); }

    bool wantsExtraNewLineBefore(Directive d) const {
        return (    (d.brig()->kind == Brig::BRIG_DIRECTIVE_LABEL)
                 || (d.brig()->kind == Brig::BRIG_DIRECTIVE_KERNEL)
                 || (d.brig()->kind == Brig::BRIG_DIRECTIVE_FUNCTION));
    }

    //-------------------------------------------------------------------------
    // Errors handling

    template<class T>
    void error(T brigObj, const char* msg, unsigned val) const {
        hasErr = true;
        if (err) *err << msg << ' ' << val << " at offset " << brigObj.brigOffset() << '\n';
        print("/* ", msg, ' ', val, " */");
    }

    const char* invalid(const char* type, unsigned val) const {
        hasErr = true;
        if (err) *err << "Invalid Brig::" << type << " value " << val << '\n';
        return "/*INVALID*/";
    }

    //-------------------------------------------------------------------------
};

} // namespace HSAIL_ASM

#endif
