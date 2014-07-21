
#include "HSAILTestGenSample.h"
#include "HSAILTestGenContext.h"
#include "HSAILTestGenUtilities.h"

using HSAIL_ASM::ItemList;
using HSAIL_PROPS::getOperandIdx;

namespace TESTGEN {

//==============================================================================
//==============================================================================
//==============================================================================

unsigned Sample::get(unsigned propId) const
{
    assert(PROP_MINID < propId && propId < PROP_MAXID);
    assert(!isEmpty());

    if (isOperandProp(propId))
    {
        int idx = getOperandIdx(propId);
        assert(0 <= idx && idx <= 4);
        assert(idx < inst.operands().size());
        return getContext()->operand2id(inst.operand(idx));
    }
    else 
    {
        unsigned val = getBrigProp(inst, propId);
        return (propId == PROP_EQUIVCLASS)? getContext()->eqclass2id(val) : val;
    }
}

void Sample::set(unsigned propId, unsigned val)
{
    assert(PROP_MINID < propId && propId < PROP_MAXID);
    assert(!isEmpty());

    using namespace Brig;

    if (isOperandProp(propId))
    {
        int idx = getOperandIdx(propId);
        assert(0 <= idx && idx <= 4);
        assert(idx < inst.operands().size());

        assign(inst, idx, getContext()->id2operand(val));
    }
    else 
    {
        if (propId == PROP_EQUIVCLASS) val = getContext()->id2eqclass(val);
        setBrigProp(inst, propId, val);
    }
}

void Sample::copyFrom(const Sample s, bool compactOperands)
{
    assert(!s.isEmpty());
    assert(inst.brig()->kind == s.inst.brig()->kind);

    memcpy(inst.brig(), s.inst.brig(), s.inst.brig()->byteCount);

    ItemList list;
    for (int i = 0; i < s.inst.operands().size(); ++i) // Get rid of unused operands
    {
        if (!s.inst.operand(i) && compactOperands) break;
        list.push_back(Operand());
    }
    inst.operands() = list;
}

//==============================================================================
//==============================================================================
//==============================================================================

} // namespace TESTGEN
