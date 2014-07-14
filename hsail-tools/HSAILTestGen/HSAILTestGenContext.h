//===-- HSAILTestGenContext.h - HSAIL Test Generator Context --------------===//
//
//===----------------------------------------------------------------------===//
//
// (C) 2013 AMD Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDED_HSAIL_TESTGEN_CONTEXT_H
#define INCLUDED_HSAIL_TESTGEN_CONTEXT_H

#include "HSAILTestGenOptions.h"
#include "HSAILTestGenProp.h"
#include "HSAILTestGenSample.h"
#include "HSAILTestGenUtilities.h"

#include "HSAILValidator.h"

#include "HSAILBrigContainer.h"
#include "HSAILBrigObjectFile.h"
#include "HSAILItems.h"
#include "Brig.h"

#include <cassert>
#include <sstream>

using std::ostringstream;

namespace TESTGEN {

//==============================================================================
//==============================================================================
//==============================================================================
// Brig context for test generation.

class Context : public BrigContext
{
    friend class Sample;

private:
    const string fileName;          // name of final brig file with test(s)
    DirectiveKernel testKernel;     // kernel for which test code is generated
    Operand operandTab[O_MAXID];    // operands used for testing. Created at first access

private: 
    Directive symTab[SYM_MAXID];       // Symbols used for testing

    //==========================================================================

private:
    Context(const Context&); // non-copyable
    const Context &operator=(const Context &);  // not assignable

    //==========================================================================

public:

    // Used for two different purposes.
    // 1. If 'file' is not empty, create context for a set of instructions (added separately).
    // 2. If 'file' is empty, create special 'playground' context for generation of temporary samples.
    //    This context and all generated code is never saved (i.e. thrown away when TestTGen exits).
    Context(string file = "") : BrigContext(machineModel, profile, !enableComments), fileName(file)
    {
        emitVersion();
        if (instSubset.isSet(SUBSET_GCN))   emitExtension("amd:gcn");
        if (instSubset.isSet(SUBSET_IMAGE)) emitExtension("IMAGE");
        genSymbols();
    }

    // Used to create context for tests which include just one instruction specified by sample
    Context(string file, const Sample s, bool isPositive) : BrigContext(machineModel, profile, !enableComments), fileName(file)
    {
        assert(!file.empty());

        emitVersion();

        if (HSAIL_ASM::isGcnInst(s.getOpcode()))
        {
            assert(instSubset.isSet(SUBSET_GCN));
            emitExtension("amd:gcn");
        }
        else if (HSAIL_ASM::hasImageExtProps(s.getInst()))
        {
            // positive tests must not include image-specific props unless "-image" option is specified
            // negative tests may include image-specific types even if "-image" option is not specified
            assert(instSubset.isSet(SUBSET_IMAGE) || !isPositive); 
            emitExtension("IMAGE");
        }

        genSymbol(operandId2SymId(s.get(PROP_S0)));
        genSymbol(operandId2SymId(s.get(PROP_S1)));
        genSymbol(operandId2SymId(s.get(PROP_S2)));
        genSymbol(operandId2SymId(s.get(PROP_S3)));
        genSymbol(operandId2SymId(s.get(PROP_S4)));
    }

    //==========================================================================

public:

    void defineTestKernel()        { testKernel = emitKernel("&Test"); }
    void registerTestKernelArgs()  { registerSbrArgs(testKernel);      }

    void finalizeTestKernel()
    {
        assert(testKernel && testKernel.container() == &getContainer());

        endSbrBody(testKernel);
    }

    void save(bool dumpContainer = false)
    {
        if (dumpContainer) dump();

        assert(!fileName.empty());

        if (HSAIL_ASM::BrigStreamer::save(getContainer(), fileName.c_str()))
        {
            ostringstream msg;
            msg << "Failed to save " << fileName.c_str();
            throw TestGenError(msg.str());
        }
    }

    bool validate()
    {
        HSAIL_ASM::Validator vld(getContainer());
        return vld.validate(HSAIL_ASM::Validator::VM_BrigNotLinked);
    }

    //==========================================================================

public:
    // Used to create positive tests
    Sample cloneSample(const Sample s)
    {
        assert(!s.isEmpty());

        Sample copy = createSample(s.getFormat(), s.getOpcode());
        copy.copyFrom(s);

        copy.set(PROP_S0, s.get(PROP_S0));
        copy.set(PROP_S1, s.get(PROP_S1));
        copy.set(PROP_S2, s.get(PROP_S2));
        copy.set(PROP_S3, s.get(PROP_S3));
        copy.set(PROP_S4, s.get(PROP_S4));

        finalizeSample(copy);

        return copy;
    }

    // used to create negative tests
    Sample cloneSample(const Sample s, unsigned id, unsigned val)
    {
        assert(!s.isEmpty());

        emitComment("// ");
        emitComment("// ");
        ostringstream text;
        text << "// Invalid value of " << prop2str(id) << " = " << val2str(id, val);
        emitComment(text.str());
        emitComment("// ");

        return cloneSample(s);
    }

    Sample createSample(unsigned format, unsigned opcode)
    {
        return Sample(this, appendInst(getContainer(), format), opcode);
    }

    //==========================================================================

private:

    void finalizeSample(Sample sample)
    {
        using namespace Brig;
        unsigned opcode = sample.getOpcode();

        if (opcode == BRIG_OPCODE_BRN || opcode == BRIG_OPCODE_RET)
        {
            emitAuxLabel(); // Generate aux label to avoid "unreachable code" error
        }
    }

    //==========================================================================
    // Mapping of operand ids to brig operands and back
private:

    unsigned operand2id(Operand opr) const 
    {
        if (!opr) return O_NULL;

        // NB: This code is inefficient and should be optimized.
        // But it amounts to about 1% of execution time. This is because 'operand2id' is
        // used only for accepted samples; generation and validation of all samples
        // takes up most of execution time.
        for (unsigned i = O_MINID + 1; i < O_MAXID; ++i)
        {
            if (i == O_NULL) continue;
            // NB: only used operands are created
            if (readOperand(i).brigOffset() == opr.brigOffset()) return i;
        }

        assert(false);
        return (unsigned)-1;
    }

    Operand id2operand(unsigned oprId)
    {
        assert(O_MINID < oprId && oprId < O_MAXID);

        return getOperand(oprId);
    }

    //==========================================================================
    // Mapping of eqclass ids to values and back
private:

    unsigned eqclass2id(unsigned equiv) const 
    {
        if (equiv == 0)   return EQCLASS_0;
        if (equiv == 1)   return EQCLASS_1;
        if (equiv == 2)   return EQCLASS_2;
        if (equiv == 255) return EQCLASS_255;

        assert(false);
        return (unsigned)-1;
    }

    unsigned id2eqclass(unsigned eqClassId)
    {
        assert(EQCLASS_MINID < eqClassId && eqClassId < EQCLASS_MAXID);

        if (eqClassId == EQCLASS_0)   return 0;
        if (eqClassId == EQCLASS_1)   return 1;
        if (eqClassId == EQCLASS_2)   return 2;
        if (eqClassId == EQCLASS_255) return 255;

        assert(false);
        return 0xFF;
    }

    //==========================================================================

private:

    bool    isOperandCreated(unsigned oprId) const { assert(O_MINID < oprId && oprId < O_MAXID); return oprId == O_NULL || operandTab[oprId]; }
    Operand readOperand(unsigned oprId)      const { assert(O_MINID < oprId && oprId < O_MAXID); return operandTab[oprId]; }

    Operand getOperand(unsigned oprId);                 // create if not created yet

    void genSymbols();                                  // Create all symbols
    void genSymbol(unsigned symId);                     // Create only symbol required for operandId
    Operand emitOperandRef(unsigned symId);

    Directive emitSymbol(unsigned symId);

    //==========================================================================

private:

    void dump() //F
    {
        for(Directive d = getContainer().directives().begin();
            d != getContainer().directives().end();
            d = d.next())
        {
            //d.dump(std::cout);
        }
        for(Inst i = getContainer().insts().begin();
            i != getContainer().insts().end();
            i = i.next())
        {
            //i.dump(std::cout);
        }
        for(Operand o = getContainer().operands().begin();
            o != getContainer().operands().end();
            o = o.next())
        {
            //o.dump(std::cout);
        }
    }

};

//==============================================================================
//==============================================================================
//==============================================================================

}; // namespace TESTGEN

#endif // INCLUDED_HSAIL_TESTGEN_CONTEXT_H