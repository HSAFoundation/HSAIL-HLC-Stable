//===-- HSAILTestGenUtilities.cpp - HSAIL Test Generator Utilities ------------===//
//
//===----------------------------------------------------------------------===//
//
// (C) 2013 AMD Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#include "HSAILTestGenUtilities.h"

#include <string>
#include <sstream>
#include <iomanip>

using std::string;
using std::ostringstream;
using std::setfill;
using std::setw;

using HSAIL_ASM::ItemList;

namespace TESTGEN {

// ============================================================================
// ============================================================================
//============================================================================

void assign(Inst inst, int idx, Operand opr)
{
    assert(0 <= idx && idx <= 4);
    assert(idx < inst.operands().size());
    inst.operands().writeAccess(idx) = opr;
}

void append(Inst inst, Operand opr0, Operand opr1 /*=Operand()*/, Operand opr2 /*=Operand()*/)
{
    assert(inst);
    assert(!inst.operands() || inst.operands().size() == 0);

    ItemList list;

    if (opr0) list.push_back(opr0);
    if (opr1) list.push_back(opr1);
    if (opr2) list.push_back(opr2);

    inst.operands() = list;
}

string index2str(unsigned idx, unsigned width /*=0*/)
{
    ostringstream s;
    if (width > 0) {
        s << setfill('0');
        s << setw(width);
    }
    s << idx;
    return s.str();
}

//=============================================================================
//=============================================================================
//=============================================================================

} // namespace TESTGEN

