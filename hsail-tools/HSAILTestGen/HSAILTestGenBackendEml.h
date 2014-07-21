//===-- HSAILTestGenEmlBackend.h - Backend for BRIG emulation ----------------------------===//
//
//===----------------------------------------------------------------------===//
//
// (C) 2013 AMD Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDED_HSAIL_TESTGEN_EML_BACKEND_H
#define INCLUDED_HSAIL_TESTGEN_EML_BACKEND_H

#include "HSAILTestGenDataProvider.h"
#include "HSAILTestGenEmulator.h"
#include "HSAILTestGenBackend.h"
#include "HSAILTestGenOptions.h"
#include "HSAILTestGenUtilities.h"

#include <sstream>
#include <iomanip>

using std::string;
using std::ostringstream;
using std::setw;

using Brig::BRIG_TYPE_NONE;
using Brig::BRIG_TYPE_B1;
using Brig::BRIG_TYPE_B8;
using Brig::BRIG_TYPE_B16;
using Brig::BRIG_TYPE_B32;
using Brig::BRIG_TYPE_B64;
using Brig::BRIG_TYPE_B128;
using Brig::BRIG_TYPE_S8;
using Brig::BRIG_TYPE_S16;
using Brig::BRIG_TYPE_S32;
using Brig::BRIG_TYPE_S64;
using Brig::BRIG_TYPE_U8;
using Brig::BRIG_TYPE_U16;
using Brig::BRIG_TYPE_U32;
using Brig::BRIG_TYPE_U64;
using Brig::BRIG_TYPE_F16;
using Brig::BRIG_TYPE_F32;
using Brig::BRIG_TYPE_F64;

using Brig::BRIG_PACK_NONE;
using Brig::BRIG_SEGMENT_NONE;
using Brig::BRIG_SEGMENT_KERNARG;
using Brig::BRIG_SEGMENT_GLOBAL;
using Brig::BRIG_SEGMENT_GROUP;

using HSAIL_ASM::DirectiveKernel;
using HSAIL_ASM::DirectiveFunction;
using HSAIL_ASM::DirectiveIndirectFunction;
using HSAIL_ASM::DirectiveExecutable;
using HSAIL_ASM::DirectiveVariable;
using HSAIL_ASM::DirectiveLabel;
using HSAIL_ASM::DirectiveFbarrier;

using HSAIL_ASM::InstBasic;
using HSAIL_ASM::InstSourceType;
using HSAIL_ASM::InstAtomic;
using HSAIL_ASM::InstCmp;
using HSAIL_ASM::InstCvt;
using HSAIL_ASM::InstImage;
using HSAIL_ASM::InstMem;
using HSAIL_ASM::InstMod;
using HSAIL_ASM::InstBr;

using HSAIL_ASM::OperandOperandList;
using HSAIL_ASM::OperandCodeList;

using HSAIL_ASM::OperandReg;
using HSAIL_ASM::OperandData;
using HSAIL_ASM::OperandWavesize;
using HSAIL_ASM::OperandAddress;
using HSAIL_ASM::OperandCodeRef;

using HSAIL_ASM::isFloatType;
using HSAIL_ASM::isSignedType;
using HSAIL_ASM::getOperandType;
using HSAIL_ASM::isBitType;
using HSAIL_ASM::getPackedDstDim;
using HSAIL_ASM::getUnsignedType;
using HSAIL_ASM::getBitType;
using HSAIL_ASM::ItemList;

namespace TESTGEN {

//==============================================================================
//==============================================================================
//==============================================================================

// General scheme:
// - up to 64 tests are run together
// - test = kernel, each kernel selects its own data
// - kernel args 
//    - src, dst, mem
//    - types (subword)
// - ws handling
// - 
//


// This function declares kernel arguments.
// These arguments are addresses of src, dst and mem arrays.
// Src arrays shall be initialized with test data.
// The number of src arrays is the same as the number 
// of src arguments of instruction being tested.
// Dst array is used by kernel to save the value in dst register 
// after execution of test instruction. This array is created only 
// if the instruction being tested has destination.
// Mem array is used by kernel to save the value in memory 
// after execution of test instruction. This array is created only 
// if the instruction being tested affects memory.
// Results in dst and/or mem arrays shall be compared 
// with expected values.


// Register map for generated code is as follows:
//
//   --------------------------------------------------------------
//   Registers              Usage
//   --------------------------------------------------------------
//   $c0  $s0  $d0  $q0     0-th argument of test instruction
//   $c1  $s1  $d1  $q1     1-th argument of test instruction
//   $c2  $s2  $d2  $q2     2-th argument of test instruction
//   $c3  $s3  $d3  $q3     3-th argument of test instruction
//   $c4  $s4  $d4  $q4     4-th argument of test instruction
//   --------------------------------------------------------------
//        $s5               Temporary               (REG_IDX_TMP)
//        $s6  $d6          Temporary array address (REG_IDX_ADDR)
//        $s7  $d7          Workitem ID             (REG_IDX_ID)
//        $s8  $d8          First index  = id * X1  (REG_IDX_IDX1)
//        $s9  $d9          Second index = id * X2  (REG_IDX_IDX2)
//   --------------------------------------------------------------
//        $s10 $d10         first  vector register  (REG_IDX_VEC)
//        $s11 $d11         second vector register  (REG_IDX_VEC + 1)
//        $s12 $d12         third  vector register  (REG_IDX_VEC + 2)
//        $s13 $d13         fourth vector register  (REG_IDX_VEC + 3)
//   --------------------------------------------------------------
//
//   Other registers are not used 
//   

//==============================================================================
//==============================================================================
//==============================================================================
class EmlBackend : public TestGenBackend
{
private:
    static const unsigned REG_IDX_TMP   = 5;
    static const unsigned REG_IDX_ADDR  = 6;
    static const unsigned REG_IDX_ID    = 7;
    static const unsigned REG_IDX_IDX1  = 8;
    static const unsigned REG_IDX_IDX2  = 9;
    static const unsigned REG_IDX_VEC   = 10;

    //==========================================================================
private:
    static const unsigned OPERAND_IDX_DST = 0;  // Index of destination operand (if any)

    //==========================================================================
private:
    BrigContext *context;                       // Brig context for code generation
    TestDataProvider *provider;                 // Provider of test data
    DirectiveVariable memTestArray;             // Array allocated for testing memory access

protected:
    Inst testSample;                            // Instruction generated for this test. It MUST NOT be modified!
    TestDataBundle bundle;                      // Group of test data bundled together
    string testName;                            // Test name (without extension)

    //==========================================================================
public:
    EmlBackend()
    {
        context = 0;
        provider = 0;
    }

    //==========================================================================
    // Backend Interface Implementation
public:

    // Called to check if tests shall be generated for the specified instruction.
    // If returned value is true, there is at least one test in this group.
    //
    // inst: the instruction which will be generated for this test.
    //       Backend may inspect it but MUST NOT modify it.
    //
    bool startTestGroup(Inst readOnlyInst)
    {
        testSample = readOnlyInst;

        if (testableInst(readOnlyInst) && testableOperands(readOnlyInst))
        {
            // Create a provider of test data for the current instruction.
            // Providers are selected based on data type of each operand.
            // Supported operand types and their test values must be defined
            // for each instruction in HSAILTestGenTestData.h
            provider = getProvider(readOnlyInst);   
            return provider != 0;
        }
        else
        {
             return false;
        }
    }

    // Called when all test for instruction have been generated.
    // This is a good place for backend cleanup
    void endTestGroup()
    {
        // Cleanup: important for proxies
        testSample = Inst();
        delete provider;
        provider = 0;
    }

    // Called to generate data for the next test.
    // Return true if data for next test were generated; false if there are no more test data
    bool genNextTestData()
    {
        assert(provider);
        return provider->nextBundle();
    }

    // Called after current context is created but before generation of test kernel.
    // This is a convenient place for backend to generate auxiliary variables, functions, etc.
    //
    // context:  BRIG context used for test generation.
    //           backend may save this context internally, however, it cannot be used after
    //           endKernelBody is called.
    // testName: test name used for identification purposes, e.g. "abs_000"
    //
    // Return true on success and false if this test shall be skipped.
    //
    bool startTest(BrigContext* ctx, string name)
    {
        if (!bundleTestData()) return false; // emulation failed

        context  = ctx;
        testName = name;

        createMemTestArray(); // Create an array for testing memory access (if required)
        
        return true;
    }

    // Called just before context destruction
    // This is a good place for backend cleanup
    void endTest()
    {
        // Cleanup: important for proxies
        memTestArray = DirectiveVariable();
        bundle.clear();
        context = 0;
    }

    // This function declares kernel arguments.
    // These arguments are addresses of src, dst and mem arrays.
    // Src arrays shall be initialized with test data.
    // The number of src arrays is the same as the number 
    // of src arguments of instruction being tested.
    // Dst array is used by kernel to save the value in dst register 
    // after execution of test instruction. This array is created only 
    // if the instruction being tested has destination.
    // Mem array is used by kernel to save the value in memory 
    // after execution of test instruction. This array is created only 
    // if the instruction being tested affects memory.
    // Results in dst and/or mem arrays shall be compared 
    // with expected values.
    void defKernelArgs()
    {
        for (int i = provider->getFirstSrcOperandIdx(); i <= provider->getLastOperandIdx(); i++)
        {
            context->emitArg(getModelType(), getSrcArrayName(i, "%"), BRIG_SEGMENT_KERNARG); 
        }
        if (hasDstOperand())    context->emitArg(getModelType(), getDstArrayName("%"), BRIG_SEGMENT_KERNARG); 
        if (hasMemoryOperand()) context->emitArg(getModelType(), getMemArrayName("%"), BRIG_SEGMENT_KERNARG); 
    }

    // Called after test kernel is defined but before generation of first kernel instruction.
    void startKernelBody()
    {
        assert(provider);

        emitCommentSeparator();
        CommentBrig commenter(context);
        emitTestDescription(commenter, testName, testSample, bundle, 1);

        emitLoadId(); // Load workitem id (used as an index to arrays with test data)
        emitInitCode();

        emitCommentHeader("This is the instruction being tested:");
    }

    // Called just before generation of "ret" instruction for test kernel
    void endKernelBody()
    {
        assert(provider);

        saveTestResults();
        emitCommentSeparator();
       //genLuaScript();
    }

    // Called after generation of test instruction.
    // This is the place for backend to create a new test based on the specified instruction.
    // inst: the instruction being tested.
    void makeTest(Inst inst)
    {
        assert(inst);
        assert(provider);
        assert(!bundle.empty());

        // Use first set of test data for HSAIL code.
        // This is not really important as all sets have 
        // the same values for data being embedded into code
        // (imm operands etc).
        bundle.resetPos(); 

        // Generate operands for test instruction based on test values from provider
        for (int i = provider->getFirstOperandIdx(); i <= provider->getLastOperandIdx(); i++)
        {
            assert(0 <= i && i < inst.operands().size()); 

            Operand operand = inst.operand(i);
            assert(operand);
            
            if (OperandReg reg = operand)
            {
                assign(inst, i, getOperandReg(i));
            }
            else if (OperandOperandList reg = operand)
            {
                assign(inst, i, getOperandVector(i));
            }
            else if (OperandData immed = operand)
            {
                assign(inst, i, getOperandImmed(i));
            }
            else if (OperandAddress addr = operand)
            {
                assert(!addr.reg() && addr.offset() == 0);
                assign(inst, i, getMemTestArrayAddr());
            }
            else if (OperandWavesize ws = operand)
            {
                // nothing to do
            }
            else
            {
                assert(false); // currently not supported
            }
        }
    }

    // Update test description with backend-specific data
    void registerTest(TestDesc& desc)  
    {
        desc.setData(&bundle);
    }

    //==========================================================================
    // Kernel code generation
private:

    void emitLoadId()
    {
        emitCommentHeader("Load workitem ID");
        initIdReg();
    }

    // Generate initialization code for all input registers and test variables
    void emitInitCode()
    {
        for (int i = provider->getFirstSrcOperandIdx(); i <= provider->getLastOperandIdx(); ++i)
        {
            assert(0 <= i && i < testSample.operands().size()); 

            Operand operand = testSample.operand(i);
            assert(operand);

            if (OperandReg reg = operand)
            {
                emitCommentHeader("Initialization of input register " + getName(getOperandReg(i)));
                initSrcVal(getOperandReg(i), getSrcArrayIdx(i));
            }
            else if (OperandOperandList vec = operand)
            {
                if (getVectorRegSize(vec) != 0) // Vector has register elements
                {
                    emitCommentHeader("Initialization of input vector " + getName(getOperandVector(i)));
                    initSrcVal(getOperandVector(i), getSrcArrayIdx(i));
                }
            }
            else if (OperandAddress(operand))
            {
                emitCommentHeader("Initialization of memory");
                initMemTestArray(getSrcArrayIdx(i));
            }
        }

        if (hasMemoryOperand())
        {
            emitCommentHeader("Initialization of index register for memory access");
            initMemTestArrayIndexReg();
        }

        // This instruction generates packed value, but affects only one packed element
        if (getPacking(testSample) != BRIG_PACK_NONE && 
            getPackedDstDim(getType(testSample), getPacking(testSample)) == 1) 
        {
            // Some packing controls such as 'ss' and 'ss_sat' result in partial dst modification.
            // To simplify testing in these cases, we clear dst register before test instruction.
            
            emitCommentHeader("Clear dst register because test instruction modifies only part of dst value");
            clearDstVal(getOperandReg(provider->getDstOperandIdx()));
        }
    }

    void saveTestResults()
    {
        if (hasDstOperand())
        {
            assert(OPERAND_IDX_DST < (unsigned)testSample.operands().size()); 

            Operand sampleDst = testSample.operand(OPERAND_IDX_DST);
            if (OperandReg(sampleDst))
            {
                OperandReg dst = getOperandReg(OPERAND_IDX_DST);
                emitCommentHeader("Saving dst register " + getName(dst));
                saveDstVal(dst, getDstArrayIdx());
            }
            else if (OperandOperandList(sampleDst))
            {
                OperandOperandList dst = getOperandVector(OPERAND_IDX_DST);
                emitCommentHeader("Saving dst vector " + getName(dst));
                saveDstVal(dst, getDstArrayIdx());
            }
            else
            {
                assert(false);
            }
        }

        if (hasMemoryOperand())
        {
            emitCommentHeader("Saving mem result");
            saveMemTestArray(getMemArrayIdx());
        }
    }

    //==========================================================================
    // Helpers for bundling tests together
private:

    //F This is not valid for signed types, but we have no conversion rules for WS
    bool isValidWsData(Val v) { return v.getAsB64(0) == wavesize && v.getAsB64(1) == 0; }

    // Reject unsuitable test values
    bool validateSrcData(unsigned operandIdx, Val v)
    {
        assert(operandIdx < (unsigned)testSample.operands().size()); //F

        Operand opr = testSample.operand(operandIdx);

        if (v.empty()) // destination operand, ignore
        {
            return true;
        }
        else if (OperandWavesize(opr))
        {
            return isValidWsData(v);
        }
        else if (OperandOperandList vec = opr)
        {
            unsigned dim = vec.elementCount();

            assert(v.getDim() == dim);

            for (unsigned i = 0; i < dim; ++i)
            {
                if (OperandWavesize(vec.elements(i)) && !isValidWsData(v[i])) return false;
            }
        }

        return true;
    }

    bool bundleTestData()
    {
        Val src[5];
        Val dst;
        Val mem;

        bundle.clear();
        unsigned firstSrcArgsIdx = static_cast<unsigned>(provider->getFirstSrcOperandIdx());
        unsigned srcArgsNum = static_cast<unsigned>(provider->getLastOperandIdx() - provider->getFirstSrcOperandIdx() + 1);
        bundle.setupTestArgs(firstSrcArgsIdx, srcArgsNum, hasDstOperand()? 1 : 0, hasMemoryOperand()? 1 : 0);
        bundle.setPrecision(getPrecision(testSample));

        for(;;)
        {     
            bool valid = true;
            for (unsigned i = 0; i < 5 && valid; ++i)                                       // Read current set of test data
            {
                src[i] = provider->getSrcValue(i);
                valid = validateSrcData(i, src[i]);
            }
            
            if (valid)
            {
                dst = emulateDstVal(testSample, src[0], src[1], src[2], src[3], src[4]);    // Return an empty value if emulation failed or there is no dst value
                mem = emulateMemVal(testSample, src[0], src[1], src[2], src[3], src[4]);    // Return an empty value if emulation failed or there is no mem value

                if ((!dst.empty() == hasDstOperand()) &&                                    // Check that all expected results have been provided by emulator
                    (!mem.empty() == hasMemoryOperand()))                                   // Add to bundle unless emulation failed
                {
                    bundle.expand();
                    for (unsigned i = 0; i < 5; ++i) bundle.setSrcVal(i, src[i]);
                    if (hasDstOperand())             bundle.setDstVal(dst);
                    if (hasMemoryOperand())          bundle.setMemVal(mem);
                }
            }

            // Request next set of test data for this bundle, if any
            if (bundle.full() || !provider->next()) break;
        }

        return !bundle.empty();
    }

    //==========================================================================
    // Access to registers
private:

    OperandReg getTmpReg(unsigned size)  { return context->emitReg(size, REG_IDX_TMP); }
    OperandReg getAddrReg()              { return context->emitReg(getModelSize(), REG_IDX_ADDR); }
    OperandReg getIdReg(unsigned size)   { return context->emitReg(size, REG_IDX_ID); }

    OperandReg getIdxReg(unsigned size, unsigned idx) { return context->emitReg(size == 0? getModelSize() : size, idx); }
    OperandReg getIdxReg1(unsigned size = 0)          { return getIdxReg(size, REG_IDX_IDX1); }
    OperandReg getIdxReg2(unsigned size = 0)          { return getIdxReg(size, REG_IDX_IDX2); }

    OperandData getOperandImmed(unsigned idx) // Create i-th operand of test instruction
    {                                      
        assert(idx < (unsigned)testSample.operands().size()); //F

        OperandData immed = testSample.operand(idx);
        assert(immed);

        return context->emitImm(getImmSize(immed), bundle.getSrcVal(idx).getAsB64(0), bundle.getSrcVal(idx).getAsB64(1));
    }

    OperandReg getOperandReg(unsigned idx) // Create register for i-th operand of test instruction.  
    {                                      
        assert(0 <= idx && idx <= 4);
        assert(idx < (unsigned)testSample.operands().size()); //F

        OperandReg reg = testSample.operand(idx); // NB: this register is read-only!

        assert(reg);
        assert(getRegSize(reg) == 1  ||
               getRegSize(reg) == 32 || 
               getRegSize(reg) == 64 || 
               getRegSize(reg) == 128);

        return context->emitReg(getRegSize(reg), idx); // NB: create register in CURRENT context
    }

    OperandOperandList getOperandVector(unsigned idx)  // Create register vector for i-th operand of test instruction.  
    {                                                   
        assert(0 <= idx && idx <= 4);
        assert(idx < (unsigned)testSample.operands().size()); //F
        assert(OperandOperandList(testSample.operand(idx)));

        OperandOperandList vec = testSample.operand(idx); // NB: this vector is read-only!
        unsigned regSize = getVectorRegSize(vec);
        assert(regSize == 0 || regSize == 32 || regSize == 64);

        unsigned cnt = vec.elementCount();
        assert(2 <= cnt && cnt <= 4);

        Val v = bundle.getSrcVal(idx);
        assert(idx < bundle.getFirstSrcArgIdx() || v.getDim() == cnt);

        ItemList opnds;
        for (unsigned i = 0; i < cnt; ++i) 
        {
            Operand opr;

            if (OperandReg x = vec.elements(i)) 
            {
                opr = context->emitReg(regSize, REG_IDX_VEC + i);
            }
            else if (OperandData x = vec.elements(i)) 
            {
                opr = context->emitImm(getImmSize(x), v[i].getAsB64(0), v[i].getAsB64(1));
            }
            else
            {
                assert(OperandWavesize(vec.elements(i)));
                opr = context->emitWavesize();
            }

            opnds.push_back(opr);
        }

        OperandOperandList res = context->getContainer().append<OperandOperandList>();
        res.elements() = opnds;

        return res;
    }

    static unsigned getVectorRegSize(OperandOperandList vec)
    {
        unsigned dim = vec.elementCount();

        for (unsigned i = 0; i < dim; ++i)
        {
            if (OperandReg x = vec.elements(i)) return getRegSize(x);
        }

        return 0; // vector has no register elements
    }

    static bool isVectorWithImm(Operand opr)
    {
        if (OperandOperandList vec = opr)
        {
            unsigned dim = vec.elementCount();

            for (unsigned i = 0; i < dim; ++i) if (!OperandReg(vec.elements(i))) return true;
        }

        return false;
    }

    //==========================================================================
    // Operations with index registers (used to access array elements)
private:

    void initIdReg()
    {
        // Load workitem id
        context->emitGetWorkItemId(getIdReg(32), 0); // Id for 0th dimension

        if (isLargeModel()) // Convert to U64 if necessary
        {
            context->emitCvt(BRIG_TYPE_U64, BRIG_TYPE_U32, getIdReg(64), getIdReg(32));
        }
    }

    OperandReg loadIndexReg(OperandReg idxReg, unsigned dim, unsigned elemSize)
    {
        unsigned addrSize = getRegSize(idxReg);
        if (elemSize == 1) elemSize = 32; // b1 is a special case, always stored as b32
        context->emitMul(getUnsignedType(addrSize), idxReg, getIdReg(addrSize), dim * elemSize / 8);
        return idxReg;
    }

    //==========================================================================
    // Low-level operations with arrays
private:

    OperandReg loadGlobalArrayAddress(OperandReg addrReg, OperandReg indexReg, unsigned arrayIdx)
    {
        assert(addrReg);
        assert(indexReg);
        assert(getRegSize(addrReg) == getRegSize(indexReg));

        context->emitLd(getModelType(), BRIG_SEGMENT_KERNARG, addrReg, context->emitAddrRef(getArray(arrayIdx)));
        context->emitAdd(getModelType(), addrReg, addrReg, indexReg);
        return addrReg;
    }

    unsigned getSrcArrayIdx(unsigned idx)
    {
        assert(static_cast<unsigned>(provider->getFirstSrcOperandIdx()) <= idx);
        assert(idx <= static_cast<unsigned>(provider->getLastOperandIdx()));

        return idx - provider->getFirstSrcOperandIdx();
    }

    unsigned getDstArrayIdx()
    {
        assert(hasDstOperand());
        return provider->getLastOperandIdx() - provider->getFirstSrcOperandIdx() + 1;
    }

    unsigned getMemArrayIdx()
    {
        assert(hasMemoryOperand());
        return provider->getLastOperandIdx() - provider->getFirstSrcOperandIdx() + (hasDstOperand()? 2 : 1);
    }

    DirectiveVariable getArray(unsigned idx)
    {
        return HSAIL_ASM::getInputArg(context->getLastKernel(), idx);
    }

    //==========================================================================
    // Operations with src/dst arrays
private:

    void initSrcVal(OperandReg reg, unsigned arrayIdx)
    {
        assert(reg);

        OperandReg indexReg = loadIndexReg(getIdxReg1(), 1, getRegSize(reg));
        OperandReg addrReg  = loadGlobalArrayAddress(getAddrReg(), indexReg, arrayIdx);
        OperandAddress addr = context->emitAddrRef(addrReg);
        ldReg(getRegSize(reg), reg, addr);
    }

    void initSrcVal(OperandOperandList vector, unsigned arrayIdx)
    {
        assert(vector);

        unsigned dim     = vector.elementCount();
        unsigned regSize = getVectorRegSize(vector);

        assert(regSize == 32 || regSize == 64);

        OperandReg indexReg = loadIndexReg(getIdxReg1(), dim, regSize);
        OperandReg addrReg  = loadGlobalArrayAddress(getAddrReg(), indexReg, arrayIdx);

        for (unsigned i = 0; i < dim; ++i)
        {
            if (OperandReg reg = vector.elements(i))
            {
                OperandAddress addr = context->emitAddrRef(addrReg, getSlotSize(regSize) / 8 * i);
                ldReg(regSize, context->emitReg(getRegSize(reg), reg.regNum()), addr);
            }
        }
    }

    void clearDstVal(OperandReg reg)
    {
        assert(reg);

        context->emitMov(getBitType(getRegSize(reg)), reg, context->emitImm(getRegSize(reg), 0, 0));
    }

    void saveDstVal(OperandReg reg, unsigned arrayIdx)
    {
        assert(reg);

        OperandReg indexReg = loadIndexReg(getIdxReg1(), 1, getRegSize(reg));
        OperandReg addrReg  = loadGlobalArrayAddress(getAddrReg(), indexReg, arrayIdx);
        OperandAddress addr = context->emitAddrRef(addrReg);
        stReg(getRegSize(reg), reg, addr);
    }

    void saveDstVal(OperandOperandList vector, unsigned arrayIdx)
    {
        assert(vector);

        unsigned dim     = vector.elementCount();
        unsigned regSize = getVectorRegSize(vector);

        assert(regSize == 32 || regSize == 64);

        OperandReg indexReg = loadIndexReg(getIdxReg1(), dim, regSize);
        OperandReg addrReg  = loadGlobalArrayAddress(getAddrReg(), indexReg, arrayIdx);

        for (unsigned i = 0; i < dim; ++i)
        {
            OperandAddress addr = context->emitAddrRef(addrReg, getSlotSize(regSize) / 8 * i);
            OperandReg reg = vector.elements(i);
            assert(reg); // dst vectors cannot include imm elements

            stReg(regSize, context->emitReg(reg), addr);
        }
    }

    void ldReg(unsigned elemSize, OperandReg reg, OperandAddress addr)
    {
        assert(reg);
        assert(addr);

        if (elemSize == 1)
        {
            OperandReg tmpReg = getTmpReg(32);
            context->emitLd(BRIG_TYPE_B32, BRIG_SEGMENT_GLOBAL, tmpReg, addr);
            context->emitCvt(BRIG_TYPE_B1, BRIG_TYPE_U32, reg, tmpReg);
        }
        else
        {
            context->emitLd(getBitType(elemSize), BRIG_SEGMENT_GLOBAL, reg, addr);
        }
    }

    void stReg(unsigned elemSize, OperandReg reg, OperandAddress addr)
    {
        assert(reg);
        assert(addr);
       
        if (elemSize == 1)
        {
            OperandReg tmpReg = getTmpReg(32);
            context->emitCvt(BRIG_TYPE_U32, BRIG_TYPE_B1, tmpReg, reg);
            context->emitSt(BRIG_TYPE_B32, BRIG_SEGMENT_GLOBAL, tmpReg, addr);
        }
        else
        {
            context->emitSt(getBitType(elemSize), BRIG_SEGMENT_GLOBAL, reg, addr);
        }
    }

    //==========================================================================
    // Operations with memory test array (required for instructions which access memory)
private:

    bool hasMemoryOperand()
    {
        for (int i = 0; i < testSample.operands().size(); ++i)
        {
            if (OperandAddress(testSample.operand(i))) return true;
        }
        return false;
    }

    bool hasVectorOperand()
    {
        for (int i = 0; i < testSample.operands().size(); ++i)
        {
            if (OperandOperandList(testSample.operand(i))) return true;
        }
        return false;
    }

    void createMemTestArray()
    {                              
        if (hasMemoryOperand()) 
        {
            assert(HSAIL_ASM::getSegment(testSample) != BRIG_SEGMENT_NONE);
            memTestArray = context->emitSymbol(testSample.type(), getTestArrayName(), HSAIL_ASM::getSegment(testSample), bundle.getSize() * getMaxDim());
            
            // FIXME: this is a temporary hack to ensure proper alignment of vector values in memory
            // FIXME: this is not required by HSAIL spec but will likely do in the future
            if (hasVectorOperand()) memTestArray.align() = Brig::BRIG_ALIGNMENT_8; ///F
        }
    }

    void copyMemTestArray(unsigned arrayIdx, bool isDst) 
    {
        assert(memTestArray);

        unsigned glbAddrSize = getModelSize();
        unsigned memAddrSize = context->getSegAddrSize(memTestArray.segment());

        unsigned elemType    = memTestArray.type();
        unsigned elemSize    = getBrigTypeNumBits(elemType);
        unsigned slotSize    = getSlotSize(elemSize);
        OperandReg reg       = getTmpReg(slotSize);
        unsigned dim         = getMaxDim();

        OperandReg indexReg1 = loadIndexReg(getIdxReg1(glbAddrSize), dim, slotSize);
        OperandReg indexReg2 = indexReg1; // Reuse first index register if possible
        if (glbAddrSize != memAddrSize || slotSize != elemSize) 
        {
            indexReg2 = loadIndexReg(getIdxReg2(memAddrSize), dim, elemSize);
        }

        OperandReg addrReg = loadGlobalArrayAddress(getAddrReg(), indexReg1, arrayIdx);

        for (unsigned i = 0; i < dim; ++i)
        {
            OperandAddress addr = context->emitAddrRef(addrReg, slotSize / 8 * i);
            if (isDst)
            {
                ldReg(slotSize, reg, addr);
                context->emitSt(elemType, memTestArray.segment(), reg, getMemTestArrayAddr(indexReg2, elemSize, i));
            }
            else
            {
                context->emitLd(elemType, memTestArray.segment(), reg, getMemTestArrayAddr(indexReg2, elemSize, i));
                stReg(slotSize, reg, addr);
            }
        }
    }

    void initMemTestArray(unsigned arrayIdx) { copyMemTestArray(arrayIdx, true); }
    void saveMemTestArray(unsigned arrayIdx) { copyMemTestArray(arrayIdx, false); }

    Operand getMemTestArrayAddr(OperandReg idxReg, unsigned elemSize = 0, unsigned elemIdx = 0)
    {
        assert(memTestArray);
        unsigned offset = ((elemSize + 7) / 8) * elemIdx; // account for B1 type (1 byte)
        return context->emitAddrRef(memTestArray, idxReg, offset);
    }

    unsigned getMemTestArrayType() { assert(memTestArray); return memTestArray.type(); }

    //==========================================================================

    void initMemTestArrayIndexReg()
    {
        unsigned memAddrSize = context->getSegAddrSize(memTestArray.segment());
        unsigned elemSize    = getBrigTypeNumBits(memTestArray.type());
        unsigned dim         = getMaxDim();

        loadIndexReg(getIdxReg1(memAddrSize), dim, elemSize);
    }

    Operand getMemTestArrayAddr()
    {
        assert(memTestArray);
        unsigned memAddrSize = context->getSegAddrSize(memTestArray.segment());
        return getMemTestArrayAddr(getIdxReg1(memAddrSize));
    }

    //==========================================================================
    // Comments generation
private:

    struct CommentBrig 
    { 
        BrigContext* context;
        CommentBrig(BrigContext* ctx) : context(ctx) {}
        void operator()(string s) { context->emitComment("// " + s); }
    };

    void emitComment(string text)
    {
        context->emitComment("//" + text);
    }

    void emitCommentHeader(string text)
    {
        emitCommentSeparator();
        emitComment(" " + text);
        emitComment("");
    }

    void emitCommentSeparator()
    {
        emitComment("");
        emitComment("======================================================");
    }

    //==========================================================================
    // Symbol names
protected:

    string getSrcArrayName(unsigned idx, string prefix = "") { return prefix + "src" + index2str(idx); }
    string getDstArrayName(              string prefix = "") { return prefix + "dst"; }
    string getMemArrayName(              string prefix = "") { return prefix + "mem"; }

    const char* getTestArrayName() { return "&var0"; }

    //==========================================================================
    // Helpers
private:

    static bool     isSmallModel() { return machineModel == BRIG_MACHINE_SMALL; }
    static bool     isLargeModel() { return machineModel == BRIG_MACHINE_LARGE; }
    static unsigned getModelType() { return isSmallModel() ? BRIG_TYPE_U32 : BRIG_TYPE_U64; }
    static unsigned getModelSize() { return isSmallModel() ? 32 : 64; }

    static string   getName(OperandReg reg) { return getRegName(reg); }
    static string   getName(OperandOperandList vector) 
    { 
        string res;
        for (unsigned i = 0; i < vector.elementCount(); ++i)
        {
            res += (i > 0? ", " : "");
            if (OperandReg reg = vector.elements(i))
            {
                res += getRegName(reg);
            }
            else if (OperandData imm = vector.elements(i))
            {
                res += "imm";
            }
            else
            {
                assert(OperandWavesize(vector.elements(i)));
                res += "ws";
            }
        }
        return "(" + res + ")";
    }

    static unsigned getSlotSize(unsigned typeSize)
    {
        switch(typeSize)
        {
        case 1:
        case 8:
        case 16:
        case 32:    return 32;
        case 64:    return 64;
        case 128:   return 128;
        default:
            assert(false);
            return 0;
        }
    }

    static unsigned getAtomicSrcNum(InstAtomic inst)
    {
        assert(inst);

        unsigned atmOp = inst.atomicOperation();
        return (atmOp == Brig::BRIG_ATOMIC_CAS) ? 3 : (atmOp == Brig::BRIG_ATOMIC_LD) ? 1 : 2;
    }

    bool hasDstOperand()
    {
        assert(provider);
        return provider->getDstOperandIdx() >= 0;
    }

    unsigned getMaxDim() { return getMaxDim(testSample); }

    static unsigned getMaxDim(Inst inst)
    {
        assert(inst);

        for (int i = 0; i < inst.operands().size(); ++i)
        {
            OperandOperandList vec = inst.operand(i);
            if (vec) return vec.elementCount();
        }
        return 1;
    }

    //==========================================================================
    //==========================================================================
    //==========================================================================
    // This section of code describes limitations on instructions 
    // which could be tested

private:

    // Create a provider of test data for the current instruction.
    // Providers are selected based on data type of each operand.
    // Supported operand types for each instruction must be defined
    // in HSAILTestGenTestData.h
    // If the current instruction is not described or required type
    // is not found, this test will be rejected.
    static TestDataProvider* getProvider(Inst inst)
    {
        assert(inst);

        TestDataProvider* p;

        using namespace Brig;
        switch (inst.kind()) 
        {
        case BRIG_KIND_INST_BASIC:
        case BRIG_KIND_INST_MOD:         p = TestDataProvider::getProvider(inst.opcode(), inst.type(), inst.type());                        break;
        case BRIG_KIND_INST_CVT:         p = TestDataProvider::getProvider(inst.opcode(), inst.type(), InstCvt(inst).sourceType());         break;
        case BRIG_KIND_INST_CMP:         p = TestDataProvider::getProvider(inst.opcode(), inst.type(), InstCmp(inst).sourceType());         break;
        case BRIG_KIND_INST_ATOMIC:      p = TestDataProvider::getProvider(inst.opcode(), inst.type(), inst.type(), getAtomicSrcNum(inst)); break;
        case BRIG_KIND_INST_SOURCE_TYPE: p = TestDataProvider::getProvider(inst.opcode(), inst.type(), InstSourceType(inst).sourceType());  break;
        case BRIG_KIND_INST_MEM:         p = TestDataProvider::getProvider(inst.opcode(), inst.type(), InstMem(inst).type());               break;
        default:                    p = 0; /* other formats are not currently supported */                                             break;
        }

        if (p)
        {
            unsigned maxDim = getMaxDim(inst);

            // By default, tests for source non-immediate operands can be bundled together to speedup testing
            for (int i = p->getFirstSrcOperandIdx(); i <= p->getLastOperandIdx(); ++i) 
            {
                assert(0 <= i && i < inst.operands().size()); 

                Operand opr = inst.operand(i);
                assert(opr);

                // NB: If there are vector operands, memory operands (if any) must be processed in similar way.
                unsigned dim  = (OperandOperandList(opr) || OperandAddress(opr))? maxDim : 1;
                bool     lock = !group || OperandData(opr) || OperandWavesize(opr) || isVectorWithImm(opr);
        
                p->registerOperand(i, dim, lock);
            }
            p->reset();
        }

        return p;
    }

    // Check generic limitations on operands.
    static bool testableOperands(Inst inst)
    {
        assert(inst);

        for (int i = 0; i < inst.operands().size(); ++i)
        {
            Operand operand = inst.operand(i);            
            if (!operand) return true;          // no gaps!

            if (OperandAddress addr = operand)
            {
                if (addr.reg() || addr.offset() != 0) return false;
                if (DirectiveVariable var = addr.symbol()) 
                {
                    if (isOpaqueType(addr.symbol().type())) return false;
                }
            }
            else if (OperandWavesize(operand))
            {
                if (wavesize == 0) return false;
            }
            else if (!OperandOperandList(operand) && !OperandReg(operand) && !OperandData(operand))
            {
                return false;
            }
        }

        return true;
    }

}; // class EmlBackend

//==============================================================================
//==============================================================================
//==============================================================================

}; // namespace TESTGEN

#endif // INCLUDED_HSAIL_TESTGEN_EML_BACKEND_H
