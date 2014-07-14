#include "HSAILTestGenContext.h"

namespace TESTGEN {

//=============================================================================
//=============================================================================
//=============================================================================

#define IMM_DEFAULT

#ifdef IMM_DEFAULT
static const uint64_t imm1_x   = 0xFFFFFFFFFFFFFFFFULL;
static const uint64_t imm8_x   = 0xFFFFFFFFFFFFFFFFULL;
static const uint64_t imm16_x  = 0xFFFFFFFFFFFFFFFFULL;
static const uint64_t imm32_x  = 0xFFFFFFFFFFFFFFFFULL;
static const uint64_t imm64_x  = 0xFFFFFFFFFFFFFFFFULL;
static const uint64_t imm128_h = 0x0;
static const uint64_t imm128_l = 7777777777777777777ULL;
#else
static const uint64_t imm1_x   = 0x1U;
static const uint64_t imm8_x   = 0x7FU;
static const uint64_t imm16_x  = 0x7FFFU;
static const uint64_t imm32_x  = 0x7FFFFFFFU;
static const uint64_t imm64_x  = 0x7FFFFFFFFFFFFFFFULL;
static const uint64_t imm128_h = 0x7FFFFFFFFFFFFFFFULL;
static const uint64_t imm128_l = 0xFFFFFFFFFFFFFFFFULL;
#endif

//=============================================================================
//=============================================================================
//=============================================================================

Operand Context::getOperand(unsigned oprId)
{
    assert(O_MINID < oprId && oprId < O_MAXID);

    using namespace Brig;

    if (isOperandCreated(oprId)) return operandTab[oprId];

    Operand opr = Operand();
    unsigned machineSize = (machineModel == BRIG_MACHINE_LARGE)? 64 : 32;

    switch(oprId)
    {
    case O_NULL:          opr = Operand(&getContainer(), 0);       break; // FIXME: revise using and creating O_NULL

    case O_CREG:          opr = emitReg(1);               break;
    case O_SREG:          opr = emitReg(32);              break;
    case O_DREG:          opr = emitReg(64);              break;
    case O_QREG:          opr = emitReg(128);             break;
                                                         
    case O_IMM1_X:        opr = emitImm(1,   imm1_x );    break;
    case O_IMM8_X:        opr = emitImm(8,   imm8_x );    break;
    case O_IMM16_X:       opr = emitImm(16,  imm16_x);    break;
    case O_IMM32_X:       opr = emitImm(32,  imm32_x);    break;
    case O_IMM64_X:       opr = emitImm(64,  imm64_x);    break;
    case O_IMM128_X:      opr = emitImm(128, imm128_l, imm128_h); break;

    case O_IMM32_0:       opr = emitImm(32,  0);          break;
    case O_IMM32_1:       opr = emitImm(32,  1);          break;
    case O_IMM32_2:       opr = emitImm(32,  2);          break;
    case O_IMM32_3:       opr = emitImm(32,  3);          break;

    case O_VEC2_R32_SRC:  opr = emitVector(2, 32, false);    break;
    case O_VEC3_R32_SRC:  opr = emitVector(3, 32, false);    break;
    case O_VEC4_R32_SRC:  opr = emitVector(4, 32, false);    break;
    case O_VEC2_R64_SRC:  opr = emitVector(2, 64, false);    break;                        
    case O_VEC3_R64_SRC:  opr = emitVector(3, 64, false);    break;                        
    case O_VEC4_R64_SRC:  opr = emitVector(4, 64, false);    break;   

    case O_VEC2_I8_SRC:   opr = emitVector(2, 8,  false, 2); break;
    case O_VEC3_I8_SRC:   opr = emitVector(3, 8,  false, 3); break;
    case O_VEC4_I8_SRC:   opr = emitVector(4, 8,  false, 4); break;
    case O_VEC2_M8_SRC:   opr = emitVector(2, 8,  false, 1); break;
    case O_VEC3_M8_SRC:   opr = emitVector(3, 8,  false, 2); break;
    case O_VEC4_M8_SRC:   opr = emitVector(4, 8,  false, 2); break;

    case O_VEC2_I16_SRC:  opr = emitVector(2, 16, false, 2); break;
    case O_VEC3_I16_SRC:  opr = emitVector(3, 16, false, 3); break;
    case O_VEC4_I16_SRC:  opr = emitVector(4, 16, false, 4); break;
    case O_VEC2_M16_SRC:  opr = emitVector(2, 16, false, 1); break;
    case O_VEC3_M16_SRC:  opr = emitVector(3, 16, false, 2); break;
    case O_VEC4_M16_SRC:  opr = emitVector(4, 16, false, 2); break;

    case O_VEC2_I32_SRC:  opr = emitVector(2, 32, false, 2); break;
    case O_VEC3_I32_SRC:  opr = emitVector(3, 32, false, 3); break;
    case O_VEC4_I32_SRC:  opr = emitVector(4, 32, false, 4); break;
    case O_VEC2_M32_SRC:  opr = emitVector(2, 32, false, 1); break;
    case O_VEC3_M32_SRC:  opr = emitVector(3, 32, false, 2); break;
    case O_VEC4_M32_SRC:  opr = emitVector(4, 32, false, 2); break;

    case O_VEC2_I64_SRC:  opr = emitVector(2, 64, false, 2); break;                        
    case O_VEC3_I64_SRC:  opr = emitVector(3, 64, false, 3); break;                        
    case O_VEC4_I64_SRC:  opr = emitVector(4, 64, false, 4); break;
    case O_VEC2_M64_SRC:  opr = emitVector(2, 64, false, 1); break;                        
    case O_VEC3_M64_SRC:  opr = emitVector(3, 64, false, 2); break;                        
    case O_VEC4_M64_SRC:  opr = emitVector(4, 64, false, 2); break;                         

    case O_VEC2_R32_DST:  opr = emitVector(2, 32);           break;
    case O_VEC3_R32_DST:  opr = emitVector(3, 32);           break;
    case O_VEC4_R32_DST:  opr = emitVector(4, 32);           break;
    case O_VEC2_R64_DST:  opr = emitVector(2, 64);           break;
    case O_VEC3_R64_DST:  opr = emitVector(3, 64);           break;
    case O_VEC4_R64_DST:  opr = emitVector(4, 64);           break;
                         
    case O_WAVESIZE:      opr = emitWavesize();              break;

    case O_ADDRESS_FLAT_REG:       opr = emitAddrRef(Directive(), emitReg(machineSize)); break;
    case O_ADDRESS_FLAT_OFF:       opr = emitAddrRef(Directive());        break;

    case O_ADDRESS_GLOBAL_VAR:     
    case O_ADDRESS_READONLY_VAR:                                                                       
    case O_ADDRESS_GROUP_VAR:      
    case O_ADDRESS_PRIVATE_VAR:                                                                        
    case O_ADDRESS_GLOBAL_ROIMG:   
    case O_ADDRESS_GLOBAL_WOIMG:   
    case O_ADDRESS_GLOBAL_RWIMG:   
    case O_ADDRESS_GLOBAL_SAMP:    
    case O_ADDRESS_GLOBAL_SIG32:   
    case O_ADDRESS_GLOBAL_SIG64:                                  
    case O_ADDRESS_READONLY_ROIMG: 
    case O_ADDRESS_READONLY_RWIMG: 
    case O_ADDRESS_READONLY_SAMP:  
    case O_ADDRESS_READONLY_SIG32: 
    case O_ADDRESS_READONLY_SIG64:                                 
    case O_FUNCTIONREF:            
    case O_FBARRIERREF:            
    case O_LABELREF:               opr = emitOperandRef(operandId2SymId(oprId)); break;

    case O_JUMPTAB: assert(false); break; // Currently not used
    case O_CALLTAB: assert(false); break; // Currently not used

    }

    operandTab[oprId] = opr;
    return opr;
}

void Context::genSymbols()
{
    for (unsigned i = SYM_MINID + 1; i < SYM_MAXID; ++i) genSymbol(i);
}

Directive Context::emitSymbol(unsigned symId)
{
    assert(SYM_MINID < symId && symId < SYM_MAXID);

    const char* name = getSymName(symId);

    if (symId == SYM_FBARRIER)
    {
        return emitFBarrier(name);
    }
    else if (symId == SYM_FUNC)
    {
        Directive fn = emitFuncStart(name, 0, 0); 
        emitFuncEnd(fn); 
        return fn;
    }
    else
    {
        return BrigContext::emitSymbol(getSymType(symId), name, getSymSegment(symId));
    }
}

//==============================================================================
//==============================================================================
//==============================================================================

void Context::genSymbol(unsigned symId)
{
    assert((SYM_MINID < symId && symId < SYM_MAXID) || symId == SYM_NONE);

    if (symId == SYM_NONE || symId == SYM_LABEL || !isSupportedSym(symId)) return;
    if (!symTab[symId]) symTab[symId] = emitSymbol(symId);
}

Operand Context::emitOperandRef(unsigned symId)
{
    assert(SYM_MINID < symId && symId < SYM_MAXID);
    assert(isSupportedSym(symId));

    switch(symId)
    {
    case SYM_LABEL:                           return emitLabelRef(getSymName(symId));
    case SYM_FUNC:     assert(symTab[symId]); return emitFuncRef(symTab[symId]);
    case SYM_FBARRIER: assert(symTab[symId]); return emitFBarrierRef(symTab[symId]);
    default:           assert(symTab[symId]); return emitAddrRef(symTab[symId]);
    }
}

//==============================================================================
//==============================================================================
//==============================================================================

} // namespace TESTGEN