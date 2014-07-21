//===-- HSAILTestGenNavigator.h - HSAIL Test Generator - Navigator ------------===//
//
//===----------------------------------------------------------------------===//
//
// (C) 2013 AMD Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDED_HSAIL_TESTGEN_NAVIGATOR_H
#define INCLUDED_HSAIL_TESTGEN_NAVIGATOR_H

#include "HSAILItems.h"

#include <vector>
#include <string>

using HSAIL_ASM::Inst;
using std::vector;
using std::string;

namespace TESTGEN {

class TestGenNavigatorImpl;

//==============================================================================
//==============================================================================
//==============================================================================
// Categorization and filtration of tests

class TestGenNavigator
{
private:
    TestGenNavigatorImpl* impl;

public:
    TestGenNavigator();
    ~TestGenNavigator();

public:
    bool isOpcodeEnabled(unsigned opcode) const;
    bool startTest(Inst inst);
    string getTestTags(unsigned testIdx, bool isFullDesc = true);
    string getRelTestPath();
};

//==============================================================================
//==============================================================================
//==============================================================================

} // namespace TESTGEN

#endif // INCLUDED_HSAIL_TESTGEN_NAVIGATOR_H
