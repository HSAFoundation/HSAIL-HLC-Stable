
#include "HSAILTestGenProp.h"
#include "HSAILTestGenPropDesc.h"
#include "HSAILValidatorBase.h"
#include "HSAILTestGenOptions.h"

#include <algorithm>

using namespace HSAIL_ASM;

namespace TESTGEN {

//=============================================================================
//=============================================================================
//=============================================================================
// Mappings of abstract HDL values of extended properties to actual Brig values

// FIXME: ADD "HDL" PREFIX FOR HDL-GENERATED VALUES (THINK OVER UNIFICATION OF PREFIXES FOR HDL AND TGEN)
static const unsigned valMapDesc[] = // from HDL to TestGen
{
// FIXME: note that keys in this map may be used as indices to an array that hold expanded values (??? use array instead of map???)

    OPERAND_VAL_NULL,       O_NULL, 0,

    OPERAND_VAL_REG,        O_CREG, O_SREG, O_DREG, O_QREG, 0,

    OPERAND_VAL_VEC_2,      O_VEC2_R32_SRC, O_VEC2_R64_SRC,
                            O_VEC2_I8_SRC,  O_VEC2_I16_SRC, O_VEC2_I32_SRC, O_VEC2_I64_SRC,
                            O_VEC2_M8_SRC,  O_VEC2_M16_SRC, O_VEC2_M32_SRC, O_VEC2_M64_SRC,
                            O_VEC2_R32_DST, O_VEC2_R64_DST,
                            0,

    OPERAND_VAL_VEC_3,      O_VEC3_R32_SRC, O_VEC3_R64_SRC,
                            O_VEC3_I8_SRC,  O_VEC3_I16_SRC, O_VEC3_I32_SRC, O_VEC3_I64_SRC,
                            O_VEC3_M8_SRC,  O_VEC3_M16_SRC, O_VEC3_M32_SRC, O_VEC3_M64_SRC,
                            O_VEC3_R32_DST, O_VEC3_R64_DST,
                            0,

    OPERAND_VAL_VEC_4,      O_VEC4_R32_SRC, O_VEC4_R64_SRC,
                            O_VEC4_I8_SRC,  O_VEC4_I16_SRC, O_VEC4_I32_SRC, O_VEC4_I64_SRC,
                            O_VEC4_M8_SRC,  O_VEC4_M16_SRC, O_VEC4_M32_SRC, O_VEC4_M64_SRC,
                            O_VEC4_R32_DST, O_VEC4_R64_DST,
                            0,

    OPERAND_VAL_IMM,        O_IMM1_X, O_IMM8_X, O_IMM16_X, O_IMM32_X, O_IMM64_X, O_IMM128_X, O_WAVESIZE, 0,

    OPERAND_VAL_LAB,        O_LABELREF, 0,

    OPERAND_VAL_ADDR,       O_ADDRESS_FLAT_REG, O_ADDRESS_FLAT_OFF, 
                            O_ADDRESS_GLOBAL_VAR, O_ADDRESS_READONLY_VAR, O_ADDRESS_GROUP_VAR, O_ADDRESS_PRIVATE_VAR,
                            O_ADDRESS_GLOBAL_ROIMG, O_ADDRESS_GLOBAL_WOIMG, O_ADDRESS_GLOBAL_RWIMG, O_ADDRESS_GLOBAL_SAMP, O_ADDRESS_GLOBAL_SIG32, O_ADDRESS_GLOBAL_SIG64,
                            O_ADDRESS_READONLY_ROIMG, O_ADDRESS_READONLY_RWIMG, O_ADDRESS_READONLY_SAMP, O_ADDRESS_READONLY_SIG32, O_ADDRESS_READONLY_SIG64, 0,

    OPERAND_VAL_FUNC,       O_FUNCTIONREF, 0,

    OPERAND_VAL_ARGLIST,    0,
    OPERAND_VAL_JUMPTAB,    0,
    OPERAND_VAL_CALLTAB,    0,
    OPERAND_VAL_FBARRIER,   O_FBARRIERREF, 0,

    OPERAND_VAL_ROIMAGE,    O_ADDRESS_GLOBAL_ROIMG, O_ADDRESS_READONLY_ROIMG, 0, 
    OPERAND_VAL_WOIMAGE,    O_ADDRESS_GLOBAL_WOIMG, 0, 
    OPERAND_VAL_RWIMAGE,    O_ADDRESS_GLOBAL_RWIMG, O_ADDRESS_READONLY_RWIMG, 0, 
    OPERAND_VAL_SAMPLER,    O_ADDRESS_GLOBAL_SAMP,  O_ADDRESS_READONLY_SAMP,  0,

    OPERAND_VAL_IMM0T2,     O_IMM32_0, O_IMM32_1, O_IMM32_2, 0, 
    OPERAND_VAL_IMM0T3,     O_IMM32_0, O_IMM32_1, O_IMM32_2, O_IMM32_3, 0,

    OPERAND_VAL_INVALID,    0,
    
    EQCLASS_VAL_0,          EQCLASS_0, 0,
    EQCLASS_VAL_ANY,        EQCLASS_0, EQCLASS_1, EQCLASS_2, EQCLASS_255, 0,
    EQCLASS_VAL_INVALID,    0,
};

const unsigned* getValMapDesc(unsigned* size) { *size = sizeof(valMapDesc) / sizeof(unsigned); return valMapDesc; };

//=============================================================================
//=============================================================================
//=============================================================================

unsigned operandId2SymId(unsigned operandId)
{
    switch(operandId)
    {
    case O_ADDRESS_GLOBAL_VAR:      return SYM_GLOBAL_VAR;
    case O_ADDRESS_READONLY_VAR:    return SYM_READONLY_VAR;
    case O_ADDRESS_GROUP_VAR:       return SYM_GROUP_VAR;
    case O_ADDRESS_PRIVATE_VAR:     return SYM_PRIVATE_VAR;

    case O_ADDRESS_GLOBAL_ROIMG:    return SYM_GLOBAL_ROIMG;
    case O_ADDRESS_READONLY_ROIMG:  return SYM_READONLY_ROIMG;
    case O_ADDRESS_GLOBAL_RWIMG:    return SYM_GLOBAL_RWIMG;
    case O_ADDRESS_READONLY_RWIMG:  return SYM_READONLY_RWIMG;
    case O_ADDRESS_GLOBAL_WOIMG:    return SYM_GLOBAL_WOIMG;

    case O_ADDRESS_GLOBAL_SAMP:     return SYM_GLOBAL_SAMP;
    case O_ADDRESS_READONLY_SAMP:   return SYM_READONLY_SAMP;

    case O_ADDRESS_GLOBAL_SIG32:    return SYM_GLOBAL_SIG32;
    case O_ADDRESS_READONLY_SIG32:  return SYM_READONLY_SIG32;
    case O_ADDRESS_GLOBAL_SIG64:    return SYM_GLOBAL_SIG64;
    case O_ADDRESS_READONLY_SIG64:  return SYM_READONLY_SIG64;

    case O_FBARRIERREF:             return SYM_FBARRIER;
    case O_FUNCTIONREF:             return SYM_FUNC;
    case O_LABELREF:                return SYM_LABEL;

    default:                        return SYM_NONE;
    }
}

bool isSupportedOperand(unsigned oprId)
{
    unsigned symId = operandId2SymId(oprId);
    return symId == SYM_NONE || isSupportedSym(symId);
}

const SymDesc symDescTab[SYM_MAXID] =
{
    {},

    {SYM_FUNC,           "&TestFunc",      Brig::BRIG_TYPE_NONE,  Brig::BRIG_SEGMENT_NONE},
    {SYM_GLOBAL_VAR,     "&GlobalVar",     Brig::BRIG_TYPE_S32,   Brig::BRIG_SEGMENT_GLOBAL},
    {SYM_GROUP_VAR,      "&GroupVar",      Brig::BRIG_TYPE_S32,   Brig::BRIG_SEGMENT_GROUP},
    {SYM_PRIVATE_VAR,    "&PrivateVar",    Brig::BRIG_TYPE_S32,   Brig::BRIG_SEGMENT_PRIVATE},
    {SYM_READONLY_VAR,   "&ReadonlyVar",   Brig::BRIG_TYPE_S32,   Brig::BRIG_SEGMENT_READONLY},
    {SYM_GLOBAL_ROIMG,   "&GlobalROImg",   Brig::BRIG_TYPE_ROIMG, Brig::BRIG_SEGMENT_GLOBAL},
    {SYM_GLOBAL_WOIMG,   "&GlobalWOImg",   Brig::BRIG_TYPE_WOIMG, Brig::BRIG_SEGMENT_GLOBAL},
    {SYM_GLOBAL_RWIMG,   "&GlobalRWImg",   Brig::BRIG_TYPE_RWIMG, Brig::BRIG_SEGMENT_GLOBAL},
    {SYM_READONLY_ROIMG, "&ReadonlyROImg", Brig::BRIG_TYPE_ROIMG, Brig::BRIG_SEGMENT_READONLY},
    {SYM_READONLY_RWIMG, "&ReadonlyRWImg", Brig::BRIG_TYPE_RWIMG, Brig::BRIG_SEGMENT_READONLY},
    {SYM_GLOBAL_SAMP,    "&GlobalSamp",    Brig::BRIG_TYPE_SAMP,  Brig::BRIG_SEGMENT_GLOBAL},
    {SYM_READONLY_SAMP,  "&ReadonlySamp",  Brig::BRIG_TYPE_SAMP,  Brig::BRIG_SEGMENT_READONLY},
    {SYM_GLOBAL_SIG32,   "&GlobalSig32",   Brig::BRIG_TYPE_SIG32, Brig::BRIG_SEGMENT_GLOBAL},
    {SYM_READONLY_SIG32, "&ReadonlySig32", Brig::BRIG_TYPE_SIG32, Brig::BRIG_SEGMENT_READONLY},
    {SYM_GLOBAL_SIG64,   "&GlobalSig64",   Brig::BRIG_TYPE_SIG64, Brig::BRIG_SEGMENT_GLOBAL},
    {SYM_READONLY_SIG64, "&ReadonlySig64", Brig::BRIG_TYPE_SIG64, Brig::BRIG_SEGMENT_READONLY},
    {SYM_FBARRIER,       "&Fbarrier",      Brig::BRIG_TYPE_NONE,  Brig::BRIG_SEGMENT_NONE},
    {SYM_LABEL,          "@TestLabel",     Brig::BRIG_TYPE_NONE,  Brig::BRIG_SEGMENT_NONE},

};

const char* getSymName(unsigned symId)    { assert(SYM_MINID < symId && symId < SYM_MAXID && symDescTab[symId].id == symId); return symDescTab[symId].name;    }
unsigned    getSymType(unsigned symId)    { assert(SYM_MINID < symId && symId < SYM_MAXID && symDescTab[symId].id == symId); return symDescTab[symId].type;    }
unsigned    getSymSegment(unsigned symId) { assert(SYM_MINID < symId && symId < SYM_MAXID && symDescTab[symId].id == symId); return symDescTab[symId].segment; }

bool isSupportedSym(unsigned symId)
{
    assert(SYM_MINID < symId && symId < SYM_MAXID);
    return validateProp(PROP_TYPE, getSymType(symId), machineModel, profile, instSubset.isSet(SUBSET_IMAGE)) == 0;
}

//=============================================================================
//=============================================================================
//=============================================================================

string prop2str(unsigned id)
{
    return PropValidator::prop2str(id);
}

string operand2str(unsigned operandId)
{
    switch(operandId)
    {
    case O_NULL:          return "none";
    
    case O_CREG:          return "$c0";
    case O_SREG:          return "$s0";
    case O_DREG:          return "$d0";
    case O_QREG:          return "$q0";

    case O_VEC2_R32_SRC:  return "($s0, $s0)";
    case O_VEC3_R32_SRC:  return "($s0, $s0, $s0)";
    case O_VEC4_R32_SRC:  return "($s0, $s0, $s0, $s0)";
    case O_VEC2_R64_SRC:  return "($d0, $d0)";
    case O_VEC3_R64_SRC:  return "($d0, $d0, $d0)";
    case O_VEC4_R64_SRC:  return "($d0, $d0, $d0, $d0)";
    case O_VEC2_I8_SRC:   return "(WS, IMM8)";
    case O_VEC3_I8_SRC:   return "(WS, IMM8, IMM8)";
    case O_VEC4_I8_SRC:   return "(WS, IMM8, IMM8, IMM8)";
    case O_VEC2_M8_SRC:   return "(IMM8, $s0)";
    case O_VEC3_M8_SRC:   return "(IMM8, IMM8, $s0)";
    case O_VEC4_M8_SRC:   return "(IMM8, IMM8, $s0, $s0)";
    case O_VEC2_I16_SRC:  return "(WS, IMM16)";
    case O_VEC3_I16_SRC:  return "(WS, IMM16, IMM16)";
    case O_VEC4_I16_SRC:  return "(WS, IMM16, IMM16, IMM16)";
    case O_VEC2_M16_SRC:  return "(IMM16, $s0)";
    case O_VEC3_M16_SRC:  return "(IMM16, IMM16, $s0)";
    case O_VEC4_M16_SRC:  return "(IMM16, IMM16, $s0, $s0)";
    case O_VEC2_I32_SRC:  return "(WS, IMM32)";
    case O_VEC3_I32_SRC:  return "(WS, IMM32, IMM32)";
    case O_VEC4_I32_SRC:  return "(WS, IMM32, IMM32, IMM32)";
    case O_VEC2_M32_SRC:  return "(IMM32, $s0)";
    case O_VEC3_M32_SRC:  return "(IMM32, IMM32, $s0)";
    case O_VEC4_M32_SRC:  return "(IMM32, IMM32, $s0, $s0)";
    case O_VEC2_I64_SRC:  return "(WS, IMM64)";
    case O_VEC3_I64_SRC:  return "(WS, IMM64, IMM64)";
    case O_VEC4_I64_SRC:  return "(WS, IMM64, IMM64, IMM64)";
    case O_VEC2_M64_SRC:  return "(IMM64, $d0)";
    case O_VEC3_M64_SRC:  return "(IMM64, IMM64, $d0)";
    case O_VEC4_M64_SRC:  return "(IMM64, IMM64, $d0, $d0)";
    case O_VEC2_R32_DST:  return "($s0, $s1)";
    case O_VEC3_R32_DST:  return "($s0, $s1, $s2)";
    case O_VEC4_R32_DST:  return "($s0, $s1, $s2, $s3)";
    case O_VEC3_R64_DST:  return "($d0, $d1, $d2)";
    case O_VEC2_R64_DST:  return "($d0, $d1)";
    case O_VEC4_R64_DST:  return "($d0, $d1, $d2, $d3)";
    
    case O_IMM1_X:        return "IMM#b1";
    case O_IMM8_X:        return "IMM#b8";
    case O_IMM16_X:       return "IMM#b16";
    case O_IMM32_X:       return "IMM#b32";
    case O_IMM64_X:       return "IMM#b64";
    case O_IMM128_X:      return "IMM#b128";

    case O_IMM32_0:       return "0";
    case O_IMM32_1:       return "1";
    case O_IMM32_2:       return "2";
    case O_IMM32_3:       return "3";

    case O_WAVESIZE:      return "WAVESIZE";

    case O_LABELREF:      
    case O_FUNCTIONREF:   
    case O_FBARRIERREF:   return getSymName(operandId2SymId(operandId));


    case O_ADDRESS_FLAT_REG:            return (machineModel == BRIG_MACHINE_LARGE)? "[$d0]" : "[$s0]";
    case O_ADDRESS_FLAT_OFF:            return "[0]";

    case O_ADDRESS_GLOBAL_VAR:          
    case O_ADDRESS_READONLY_VAR:        
    case O_ADDRESS_GROUP_VAR:           
    case O_ADDRESS_PRIVATE_VAR:        
    case O_ADDRESS_GLOBAL_ROIMG:        
    case O_ADDRESS_GLOBAL_RWIMG:        
    case O_ADDRESS_GLOBAL_WOIMG:        
    case O_ADDRESS_GLOBAL_SAMP:         
    case O_ADDRESS_GLOBAL_SIG32:        
    case O_ADDRESS_GLOBAL_SIG64:        
    case O_ADDRESS_READONLY_ROIMG:      
    case O_ADDRESS_READONLY_RWIMG:      
    case O_ADDRESS_READONLY_SAMP:       
    case O_ADDRESS_READONLY_SIG32:      
    case O_ADDRESS_READONLY_SIG64:      return string("[") + getSymName(operandId2SymId(operandId)) + "]";
    
    case O_JUMPTAB:                     return "[Jumptab]"; // currently unused
    case O_CALLTAB:                     return "[Calltab]"; // currently unused

    default: assert(false); return "";
    }
}

string eqclass2str(unsigned id)
{
    switch(id)
    {
    case EQCLASS_0:         return "0";
    case EQCLASS_1:         return "1";
    case EQCLASS_2:         return "2";
    case EQCLASS_255:       return "255";

    default: assert(false); return "";
    }
}

string val2str(unsigned id, unsigned val)
{
    if (PROP_D0 <= id && id <= PROP_S4) // TestGen-specific
    {
        return operand2str(val);
    }
    else if (id == PROP_EQUIVCLASS)        // TestGen-specific
    {
        return eqclass2str(val);
    }
    else
    {
        return PropValidator::val2str(id, val);
    }
}

//=============================================================================
//=============================================================================
//=============================================================================

struct ImmOperandDetector { bool operator()(unsigned val) { return isImmOperandId(val); }};
    
// This is not a generic soultion but rather a hack.
// It is because removal of imm operands may cause TestGen fail finding valid combinations of opernads.
void Prop::tryRemoveImmOperands()
{
    vector<unsigned> copy = pValues;

    pValues.erase(std::remove_if(pValues.begin(), pValues.end(), ImmOperandDetector()), pValues.end());
    
    // There are instructions which accept imm operands only - for these keep operand list unchanged.
    if (pValues.empty()) pValues = copy;
}

void Prop::init(const unsigned* pVals, unsigned pValsNum, const unsigned* nVals, unsigned nValsNum)
{
    for (unsigned i = 0; i < pValsNum; ++i) appendPositive(pVals[i]);
    for (unsigned i = 0; i < nValsNum; ++i) appendNegative(nVals[i]); // NB: positive values may be excluded for neutral props 

    if (!enableImmOperands && isOperandProp(propId)) tryRemoveImmOperands();

    // This is to minimize deps from HDL-generated code
    std::sort(pValues.begin(), pValues.end());
    std::sort(nValues.begin(), nValues.end());
}

Prop* Prop::create(unsigned propId, const unsigned* pVals, unsigned pValsNum, const unsigned* nVals, unsigned nValsNum)
{
    Prop* prop;
    if (PropDesc::isBrigProp(propId))
    {
        prop = new Prop(propId);
    }
    else 
    {
        prop = new ExtProp(propId);
    }
    prop->init(pVals, pValsNum, nVals, nValsNum);
    return prop;
}

map<unsigned, const unsigned*> ExtProp::valMap;

//=============================================================================
//=============================================================================
//=============================================================================

}; // namespace TESTGEN
