//===-- HSAILTestGenPropDesc.h - HSAIL Test Generator - Description of test ===//
//
//===----------------------------------------------------------------------===//
//
// (C) 2013 AMD Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDED_HSAIL_TESTGEN_TEST_DESC_H
#define INCLUDED_HSAIL_TESTGEN_TEST_DESC_H

#include "HSAILBrigContainer.h"
#include "HSAILTestGenVal.h"
#include "HSAILTestGenUtilities.h"
#include "HSAILItems.h"

using HSAIL_ASM::BrigContainer;
using HSAIL_ASM::Inst;
using std::string;

namespace TESTGEN {

class TestDataBundle;

//==============================================================================
//==============================================================================
//==============================================================================

string dumpInst(Inst inst);
string getOperandKind(Inst inst, unsigned operandIdx);

//==============================================================================
//==============================================================================
//==============================================================================
// Test description

class TestDesc
{
    //==========================================================================
private:
    BrigContainer*  container;
    TestDataBundle* testData;
    Inst            testInst;
    string          script;

    //==========================================================================
public:
    TestDesc() { clean(); }

    //==========================================================================
private:
    TestDesc(const TestDesc&);                    // non-copyable
    const TestDesc &operator=(const TestDesc &);  // not assignable

    //==========================================================================
    // 
public:
    void clean()
    {
        container = 0;
        testData  = 0;
        testInst  = Inst();
        script.clear();
    }

    void setContainer(BrigContainer* c) { container = c;   }
    void setData(TestDataBundle* data)  { testData = data; }
    void setInst(Inst inst)             { testInst = inst; }
    void setScript(string s)            { script = s;      }

    BrigContainer*  getContainer()  const { return container; }
    TestDataBundle* getData()       const { return testData;  }
    Inst            getInst()       const { return testInst;  }
    string          getScript()     const { return script;    }

    unsigned        getOpcode()           { assert(testInst); return testInst.opcode(); }

    //==========================================================================
};

//==============================================================================
//==============================================================================
//==============================================================================
// Container for storing test values

class TestDataBundle
{
    //==========================================================================
private:
    static const unsigned MAX_BNDLS   = 64;                     // Max number of elements in the bundle (LUA limitation)
    static const unsigned ELEM_SIZE   = 7;                      // 5 src values + 1 dst value + 1 mem value
    static const unsigned MAX_SIZE    = MAX_BNDLS * ELEM_SIZE;  // Max number of elements in all bundles (LUA limitation)
    static const unsigned DST_IDX     = 5;                      // Dst index
    static const unsigned MEM_IDX     = 6;                      // Mem index
    static const unsigned MAX_OPR_IDX = 4;                      // Max operand index

    //==========================================================================
private:
    Val data[MAX_SIZE];                                         // Array for storing test values
    unsigned size;                                              // Number of used elements (not values!) in the array (multiple of ELEM_SIZE)
    unsigned pos;                                               // Current position in the bundle

    unsigned firstSrcArgIdx;
    unsigned srcArgsNum;
    unsigned dstArgsNum;
    unsigned memArgsNum;
    double   precision;

    //==========================================================================
public:
    TestDataBundle()      { clear(); }

    //==========================================================================
    // PUBLIC INTERFACE
    // 1. ADDING DATA:
    //      clear() -> expand() -> (addXXX(...), addXXX(...)...) -> expand() -> (addXXX(...), addXXX(...)...) -> ...
    // 2. READING DATA:
    //      resetPos() -> (getXXX(), getXXX(), ...) -> next()  -> (getXXX(), getXXX(), ...) -> ...
    //
public: 
    void resetPos()       { pos = 0; }
    void clear()          { size = 0; resetPos(); }
    bool expand()         { assert(empty() || valid(0)); if (!full()) { pos = size; size += ELEM_SIZE; return true; } return false; }
    bool next()           { assert(empty() || valid(0)); if (pos + ELEM_SIZE < size) { pos += ELEM_SIZE; return true; } return false; }
    bool full()     const { return size == MAX_SIZE; } 
    bool empty()    const { return size == 0; }

    unsigned getSize()                    const { assert(empty() || valid(0)); return size / ELEM_SIZE; }
                                                
    void setSrcVal(unsigned idx, Val val)       { assert(valid(idx)); data[pos + idx]     = val; }
    void setDstVal(Val val)                     { assert(valid(0));   data[pos + DST_IDX] = val; }
    void setMemVal(Val val)                     { assert(valid(0));   data[pos + MEM_IDX] = val; }

    Val getSrcVal(unsigned idx)           const { assert(valid(idx)); return data[pos + idx];     }
    Val getDstVal()                       const { assert(valid(0));   return data[pos + DST_IDX]; }
    Val getMemVal()                       const { assert(valid(0));   return data[pos + MEM_IDX]; }

    unsigned getSrcValType(unsigned idx)  const { assert(valid(idx)); return getType(idx);     } // Type of first element in the bundle
    unsigned getDstValType()              const { assert(valid(0));   return getType(DST_IDX); }
    unsigned getMemValType()              const { assert(valid(0));   return getType(MEM_IDX); }

    unsigned getSrcValDim(unsigned idx)   const { assert(valid(idx)); return getDim(idx);     } // Type of first element in the bundle
    unsigned getDstValDim()               const { assert(valid(0));   return getDim(DST_IDX); }
    unsigned getMemValDim()               const { assert(valid(0));   return getDim(MEM_IDX); }

    //==========================================================================
public:
    void setupTestArgs(unsigned first, unsigned src, unsigned dst, unsigned mem) 
    { 
        assert(first < 5);
        assert(src   < 5);
        assert(dst   < 2);
        assert(mem   < 2);

        firstSrcArgIdx = first;
        srcArgsNum = src;
        dstArgsNum = dst;
        memArgsNum = mem;
    }
    
    unsigned getArgsNum()                const { return srcArgsNum + dstArgsNum + memArgsNum; }
    unsigned getSrcArgsNum()             const { return srcArgsNum; }
    unsigned getDstArgsNum()             const { return dstArgsNum; }
    unsigned getMemArgsNum()             const { return memArgsNum; }
    unsigned getDstArgIdx()              const { return 0; }
    unsigned getFirstSrcArgIdx()         const { return firstSrcArgIdx; }
    unsigned getLastSrcArgIdx()          const { return firstSrcArgIdx + srcArgsNum - 1; }

    unsigned getPrecision()              const { return firstSrcArgIdx + srcArgsNum - 1; }
    void     setPrecision(double val)          { precision = val; }

    //==========================================================================
public:
    unsigned getKernelArgsNum()                const { return getArgsNum(); }
    bool     isSrcKernelArg(unsigned argIdx)   const { return argIdx < srcArgsNum; }
    bool     isDstKernelArg(unsigned argIdx)   const { return srcArgsNum <= argIdx && argIdx < srcArgsNum + dstArgsNum; }
    bool     isMemKernelArg(unsigned argIdx)   const { return srcArgsNum + dstArgsNum <= argIdx && argIdx < srcArgsNum + dstArgsNum + memArgsNum; }
    unsigned getKernelArgType(unsigned argIdx) const { return getKernelArgValue(argIdx, 0).getType(); }
    Val      getKernelArgValue(unsigned argIdx, unsigned valIdx) const 
    { 
        if      (isSrcKernelArg(argIdx)) return data[valIdx * ELEM_SIZE + firstSrcArgIdx + argIdx];
        else if (isDstKernelArg(argIdx)) return data[valIdx * ELEM_SIZE + DST_IDX];
        else if (isMemKernelArg(argIdx)) return data[valIdx * ELEM_SIZE + MEM_IDX];
        assert(false);
        return Val();
    }

    //==========================================================================
private:
    bool     valid(unsigned idx)   const { return pos < size && size <= MAX_SIZE && 0 <= idx && idx <= MAX_OPR_IDX; }
    unsigned getType(unsigned idx) const { return data[idx].isVector()? data[idx].getVecType() : data[idx].getType(); }
    unsigned getDim(unsigned idx)  const { return data[idx].getDim(); }
    //==========================================================================
};

//==============================================================================
//==============================================================================
//==============================================================================

template<class T>
void emitTestDescription(T& comment, string testName, Inst testInst, TestDataBundle& bundle, unsigned maxTestNum = 0xFFFFFFFF)
{
    assert(!bundle.empty());

    bundle.resetPos();

    comment("Test name: " + testName);
    comment("");
    comment("Instruction: " + dumpInst(testInst));

    for (unsigned testIdx = 0; testIdx < maxTestNum; ++testIdx)
    {
        comment("");
        if (maxTestNum == 1) {
            comment("Test arguments:");
        } else {
            comment("Test#" + index2str(testIdx, 2) + "# arguments:");
        }

        for (unsigned i = bundle.getFirstSrcArgIdx(); i <= bundle.getLastSrcArgIdx(); ++i)
        {
            assert(i < (unsigned)testInst.operands().size()); 
            assert(testInst.operand(i));
            comment("    Arg " + index2str(i) + " (" + getOperandKind(testInst, i) + "):           " + bundle.getSrcVal(i).dump());
        }

        if (bundle.getDstArgsNum() == 1)
        {
            Val dstValue = bundle.getDstVal();
            assert(!dstValue.empty());
            assert(testInst.type() == bundle.getDstValType());
            
            comment("Expected result:           " + dstValue.dump());
        }
        
        if (bundle.getMemArgsNum() == 1)
        {
            Val memValue = bundle.getMemVal();
            assert(!memValue.empty());

            comment("Expected result in memory: " + memValue.dump());
        }

        if (!bundle.next()) break;
    }
}

//==============================================================================
//==============================================================================
//==============================================================================
}; // namespace TESTGEN

#endif // INCLUDED_HSAIL_TESTGEN_TEST_DESC_H
