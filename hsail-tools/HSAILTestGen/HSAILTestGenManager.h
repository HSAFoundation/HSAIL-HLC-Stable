//===-- HSAILTestGenManager.h - HSAIL Test Generator Manager ------------===//
//
//===----------------------------------------------------------------------===//
//
// (C) 2013 AMD Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDED_HSAIL_TESTGEN_MANAGER_H
#define INCLUDED_HSAIL_TESTGEN_MANAGER_H

#include "HSAILTestGenOptions.h"
#include "HSAILTestGenContext.h"
#include "HSAILTestGenPropDesc.h"
#include "HSAILTestGenInstDesc.h"
#include "HSAILTestGenTestDesc.h"
#include "HSAILTestGenProvider.h"
#include "HSAILTestGenBackend.h"

#include <cassert>

namespace TESTGEN {

//==============================================================================
//==============================================================================
//==============================================================================
// This class manages test generation and interacts with backend 

class TestGenManager
{
private:
    const bool      isPositive;     // test type: positive or negative
    TestGenBackend* backend;        // optional backend 
    Context*        context;        // test context which includes brig container, symbols, etc
    unsigned        testIdx;        // total number of generated tests

    //==========================================================================

public:
    TestGenManager(bool testType) : isPositive(testType), testIdx(0)
    {
        backend = TestGenBackend::get(extension);
    }

    virtual ~TestGenManager()
    {
        TestGenBackend::dispose();
    }

    bool     isPositiveTest()   const { return isPositive; }
    unsigned getGlobalTestIdx() const { return testIdx; }

    //==========================================================================

public:
    virtual bool generate()
    {
        start();

        unsigned num;
        const unsigned* opcodes = PropDesc::getOpcodes(num);
        for (unsigned i = 0; i < num; ++i)
        {
            unsigned opcode = opcodes[i];

            // filter out opcodes which should not be tested
            if (!isOpcodeEnabled(opcode)) continue;

            // skip generation of tests for special opcodes
            if (HSAIL_ASM::isCallInst(opcode)) continue;    //F: generalize
            if (opcode == Brig::BRIG_OPCODE_SBR) continue; //F

            if (InstDesc::isStdOpcode(opcode)   && !instSubset.isSet(SUBSET_STD))   continue;
            if (InstDesc::isGcnOpcode(opcode)   && !instSubset.isSet(SUBSET_GCN))   continue;
            if (InstDesc::isImageOpcode(opcode) && !instSubset.isSet(SUBSET_IMAGE)) continue;

            // Regular tests generation. For instructions which may be encoded 
            // using InstBasic and InstMod formats, only InstMod version is generated.
            if (instVariants & VARIANT_MOD)
            {
                std::unique_ptr<TestGen> desc(TestGen::create(opcode));
                generateTests(*desc);
            }

            // Optional generation of InstBasic version for instructions encoded in InstMod format
            if (InstDesc::getFormat(opcode) == Brig::BRIG_KIND_INST_MOD && (instVariants & VARIANT_BASIC))
            {
                std::unique_ptr<TestGen> basicDesc(TestGen::create(opcode, true));
                generateTests(*basicDesc); // for InstBasic format
            }
        }

        finish();
        return true;
    }

protected:
    virtual bool isOpcodeEnabled(unsigned opcode) = 0;
    virtual bool startTest(Inst inst) = 0;
    virtual void testComplete(TestDesc& testDesc) = 0;
    virtual string getTestName() = 0;

    //==========================================================================
private:
    void start()
    {
        if (testPackage == PACKAGE_SINGLE)
        {
            context = new Context();
            context->defineTestKernel();
        }
    }

    void finish()
    {
        if (testPackage == PACKAGE_SINGLE) 
        {
            context->finalizeTestKernel();
            registerTest(context);
            delete context;
        }
    }

    //==========================================================================
private:

    void generateTests(TestGen& desc)
    {
        if (isPositive)
        {
            genPositiveTests(desc);
        }
        else
        {
            genNegativeTests(desc);
        }
    }

    void genPositiveTests(TestGen& test)
    {
        for (bool start = true; test.nextPrimarySet(start); start = false)
        {
            finalizePositiveSample(test, true);

            for (; test.nextSecondarySet(); )
            {
                finalizePositiveSample(test, false);
            }
        }
    }

    // NB: 'nextSecondarySet' is not called for negative tests to avoid 
    // generation of large number of identical tests
    void genNegativeTests(TestGen& test)
    {
        for (bool start = true; test.nextPrimarySet(start); start = false)
        {
            // Provide a reference to original valid sample (for inspection purpose)
            if (testPackage == PACKAGE_SINGLE && enableComments) createPositiveTest(test, true);

            unsigned id;
            unsigned val;
            for (test.resetNegativeSet(); test.nextNegativeSet(&id, &val); )
            {
                finalizeNegativeSample(test, id, val);
            }
        }
    }

    //==========================================================================
private:

    void finalizePositiveSample(TestGen& test, bool start)
    {
        const Sample positiveSample = test.getPositiveSample();
        Inst inst = positiveSample.getInst();
        
        assert(PropDesc::isValidInst(inst));

        if (testPackage == PACKAGE_SINGLE)
        {
            if (startTest(inst))
            {
                createPositiveTest(test, start);
                ++testIdx;
            }
        }
        else if (testPackage == PACKAGE_INTERNAL)
        {
            if (startTest(inst)) 
            {
                Context* ctx = new Context(positiveSample, true);
                ctx->defineTestKernel();
                Sample res = ctx->cloneSample(positiveSample);
                ctx->finalizeTestKernel();
                registerTest(ctx, res.getInst());
                delete ctx;
                ++testIdx;
            }
        }
        else // testPackage == PACKAGE_SEPARATE
        {
            assert(testPackage == PACKAGE_SEPARATE);

            if (!backend->startTestGroup(inst)) return;

            if (!startTest(inst)) return;

            for (;;)
            {
                Context* ctx = new Context(positiveSample, true);
                if (backend->startTest(ctx, getTestName()))
                {
                    ctx->defineTestKernel();
                    backend->defKernelArgs();

                    ctx->registerTestKernelArgs();
                    backend->startKernelBody();

                    Sample res = ctx->cloneSample(positiveSample);
                    backend->makeTest(res.getInst());
                    backend->endKernelBody();
            
                    ctx->finalizeTestKernel();

                    registerTest(ctx, res.getInst());
                    ++testIdx;
                }
                
                delete ctx;
                backend->endTest();
                if (!backend->genNextTestData()) break;
            }
            backend->endTestGroup();
        }
    }

    //==========================================================================
private:

    void finalizeNegativeSample(TestGen& test, unsigned id, unsigned val)
    {
        assert(PropDesc::isValidInst(test.getPositiveSample().getInst()));
        assert(!PropDesc::isValidInst(test.getNegativeSample().getInst()));

        Sample negativeSample = test.getNegativeSample();

        if (startTest(negativeSample.getInst()))
        {
            if (testPackage == PACKAGE_SINGLE)
            {
                createNegativeTest(test, id, val);
            }
            else if (testPackage == PACKAGE_INTERNAL)
            {
                Context* ctx = new Context(negativeSample, false);
                ctx->defineTestKernel();
                Sample res = ctx->cloneSample(negativeSample, id, val);
                assert(!PropDesc::isValidInst(res.getInst()));
                ctx->finalizeTestKernel();
                registerTest(ctx, res.getInst());
                delete ctx;
            }
            else // testPackage == PACKAGE_SEPARATE
            {
                // NB: PACKAGE_SEPARATE is not supported for negative tests
                assert(testPackage == PACKAGE_SEPARATE);
                assert(false);
            }

            ++testIdx;
        }
    }

    //==========================================================================

    void createPositiveTest(TestGen& test, bool start)
    {
        assert(testPackage == PACKAGE_SINGLE);

        if (start)
        {
            string note;
            if (instVariants & VARIANT_BASIC) {
                if (test.getFormat() == Brig::BRIG_KIND_INST_MOD)
                    note = " (InstMod format)";
                if (test.getFormat() == Brig::BRIG_KIND_INST_BASIC && test.isBasicVariant())
                    note = " (InstBasic format)";
            }
        
            context->emitComment("// ");
            context->emitComment((isPositive? "// Next sample" : "// Next valid sample") + note);
            context->emitComment("// ");

            context->cloneSample(test.getPositiveSample());

            context->emitComment("// ");
        }
        else
        {
            context->cloneSample(test.getPositiveSample());
        }
    }

    void createNegativeTest(TestGen& test, unsigned id, unsigned val)
    {
        assert(testPackage == PACKAGE_SINGLE);

        Sample negativeSample = test.getNegativeSample();
        Sample invalid = context->cloneSample(negativeSample, id, val);
        assert(!PropDesc::isValidInst(invalid.getInst()));
    }

    //==========================================================================
private:

    void registerTest(Context* ctx, Inst inst = Inst())
    {
        assert(ctx);

        TestDesc testDesc;

        backend->registerTest(testDesc);
        testDesc.setContainer(&ctx->getContainer());
        testDesc.setInst(inst);

        testComplete(testDesc);
    }

    //==========================================================================
};

//==============================================================================
//==============================================================================
//==============================================================================

}; // namespace TESTGEN 

#endif // INCLUDED_HSAIL_TESTGEN_MANAGER_H