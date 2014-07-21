#include "HSAILTestGenBrigContext.h"
#include "HSAILTestGenUtilities.h"

#include "HSAILBrigContainer.h"
#include "HSAILItems.h"
#include "Brig.h"

#include <string>
#include <sstream>

using std::string;
using std::ostringstream;

using HSAIL_ASM::Code;

using HSAIL_ASM::DirectiveVersion;
using HSAIL_ASM::DirectiveKernel;
using HSAIL_ASM::DirectiveFunction;
using HSAIL_ASM::DirectiveExecutable;
using HSAIL_ASM::DirectiveVariable;
using HSAIL_ASM::DirectiveLabel;
using HSAIL_ASM::DirectiveComment;
using HSAIL_ASM::DirectiveExtension;
using HSAIL_ASM::DirectiveArgBlockStart;
using HSAIL_ASM::DirectiveArgBlockEnd;
using HSAIL_ASM::DirectiveFbarrier;

using HSAIL_ASM::InstBasic;
using HSAIL_ASM::InstAtomic;
using HSAIL_ASM::InstCmp;
using HSAIL_ASM::InstCvt;
using HSAIL_ASM::InstImage;
using HSAIL_ASM::InstMem;
using HSAIL_ASM::InstMod;
using HSAIL_ASM::InstBr;
using HSAIL_ASM::InstAddr;

using HSAIL_ASM::OperandReg;
using HSAIL_ASM::OperandOperandList;
using HSAIL_ASM::OperandData;
using HSAIL_ASM::OperandWavesize;
using HSAIL_ASM::OperandAddress;
using HSAIL_ASM::OperandCodeRef;
using HSAIL_ASM::OperandCodeList;

using HSAIL_ASM::ItemList;

using HSAIL_ASM::isSignedType;
using HSAIL_ASM::isFloatType;
using HSAIL_ASM::isOpaqueType;
using HSAIL_ASM::ArbitraryData;
using HSAIL_ASM::getBrigTypeNumBits;

namespace TESTGEN {

//=============================================================================
//=============================================================================
//=============================================================================

void BrigContext::emitVersion()
{
    using namespace Brig;

    DirectiveVersion version = container.append<DirectiveVersion>();

    version.hsailMajor()   = BRIG_VERSION_HSAIL_MAJOR;
    version.hsailMinor()   = BRIG_VERSION_HSAIL_MINOR;
    version.brigMajor()    = BRIG_VERSION_BRIG_MAJOR;
    version.brigMinor()    = BRIG_VERSION_BRIG_MINOR;

    version.machineModel() = getModel();
    version.profile()      = getProfile();
}

void BrigContext::emitExtension(const char* name)
{
    DirectiveExtension ext = container.append<DirectiveExtension>();
    ext.name() = name;
}

Operand BrigContext::emitLabelRef(const char* name)
{
    DirectiveLabel lbl = container.append<DirectiveLabel>();
    lbl.name() = name;

    OperandCodeRef operand = container.append<OperandCodeRef>();
    operand.ref() = lbl;

    return operand;
}

void BrigContext::emitAuxLabel()
{
    ostringstream labName;
    labName << "@aux_label_" << labCount++;
    DirectiveLabel lbl = getContainer().append<DirectiveLabel>();
    lbl.name() = labName.str();
}

void BrigContext::emitComment(string s)
{
    if (!disableComments) {
        DirectiveComment cmt = getContainer().append<DirectiveComment>();
        cmt.name() = s.c_str();
    }
}

//=============================================================================
//=============================================================================
//=============================================================================

void BrigContext::emitRet()
{
    Inst inst = container.append<InstBasic>();
    inst.opcode() = Brig::BRIG_OPCODE_RET;
    inst.type() = Brig::BRIG_TYPE_NONE;
    append(inst, Operand());
}

void BrigContext::emitSt(unsigned type, unsigned segment, Operand from, Operand to)
{
    InstMem inst = getContainer().append<InstMem>();

    inst.opcode()     = Brig::BRIG_OPCODE_ST;
    inst.segment()    = segment;
    inst.type()       = conv2LdStType(type);
    inst.align()      = Brig::BRIG_ALIGNMENT_1;
    inst.width()      = Brig::BRIG_WIDTH_NONE;
    inst.equivClass() = 0;
    inst.modifier().isConst() = false;

    append(inst, from, to);
}

void BrigContext::emitLd(unsigned type, unsigned segment, Operand to, Operand from, unsigned width /*=Brig::BRIG_WIDTH_1*/)
{
    InstMem inst = getContainer().append<InstMem>();

    inst.opcode()     = Brig::BRIG_OPCODE_LD;
    inst.segment()    = segment;
    inst.type()       = conv2LdStType(type);
    inst.width()      = width;
    inst.equivClass() = 0;
    inst.align()      = Brig::BRIG_ALIGNMENT_1;
    inst.modifier().isConst() = false;

    append(inst, to, from);
}

void BrigContext::emitShl(unsigned type, Operand res, Operand src, unsigned shift)
{
    InstBasic inst = getContainer().append<InstBasic>();

    inst.opcode()     = Brig::BRIG_OPCODE_SHL;
    inst.type()       = type;

    append(inst, res, src, emitImm(32, shift));
}

void BrigContext::emitMul(unsigned type, Operand res, Operand src, unsigned multiplier)
{
    InstBasic inst = getContainer().append<InstBasic>();

    inst.opcode()     = Brig::BRIG_OPCODE_MUL;
    inst.type()       = type;

    append(inst, res, src, emitImm(getBrigTypeNumBits(type), multiplier));
}

void BrigContext::emitMov(unsigned type, Operand to, Operand from)
{
    InstBasic inst = getContainer().append<InstBasic>();
    inst.opcode()  = Brig::BRIG_OPCODE_MOV;
    inst.type()    = type;

    append(inst, to, from);
}

void BrigContext::emitAdd(unsigned type, Operand res, Operand op1, Operand op2)
{
    InstBasic inst = getContainer().append<InstBasic>();
    inst.opcode()  = Brig::BRIG_OPCODE_ADD;
    inst.type()    = type;

    append(inst, res, op1, op2);
}

void BrigContext::emitSub(unsigned type, Operand res, Operand op1, Operand op2)
{
    InstBasic inst = getContainer().append<InstBasic>();
    inst.opcode()  = Brig::BRIG_OPCODE_SUB;
    inst.type()    = type;

    append(inst, res, op1, op2);
}

void BrigContext::emitGetWorkItemId(Operand res, unsigned dim)
{
    InstBasic inst   = getContainer().append<InstBasic>();
    inst.opcode()   = Brig::BRIG_OPCODE_WORKITEMABSID;
    inst.type()     = Brig::BRIG_TYPE_U32;

    append(inst, res, emitImm(32, dim));
}

void BrigContext::emitCvt(unsigned dstType, unsigned srcType, OperandReg to, OperandReg from)
{
    InstCvt cvt = getContainer().append<InstCvt>();
    cvt.opcode()     = Brig::BRIG_OPCODE_CVT;
    cvt.type()       = dstType;
    cvt.sourceType() = srcType;

    append(cvt, to, from);
}

void BrigContext::emitLda(OperandReg dst, DirectiveVariable var)
{
    assert(dst);
    assert(var);

    InstAddr lda = getContainer().append<InstAddr>();

    lda.opcode()    = Brig::BRIG_OPCODE_LDA;
    lda.type()      = getSegAddrType(var.segment());
    lda.segment()   = var.segment();

    append(lda, dst, emitAddrRef(var));
}

//=============================================================================
//=============================================================================
//=============================================================================

string BrigContext::getRegName(unsigned size, unsigned idx)
{
    ostringstream name;

    switch(size)
    {
    case 1:      name << "$c";  break;
    case 32:     name << "$s";  break;
    case 64:     name << "$d";  break;
    case 128:    name << "$q";  break;
    default: 
        assert(false);       
        name << "ERR"; 
        break;
    }
    name << idx;

    return name.str();
}

Operand BrigContext::emitReg(OperandReg reg)
{
    OperandReg opr = getContainer().append<OperandReg>();
    opr.regKind() = reg.regKind();
    opr.regNum() = reg.regNum();
    return opr;
}

Operand BrigContext::emitReg(unsigned size, unsigned idx)
{
    OperandReg opr = getContainer().append<OperandReg>();

    switch(size) {
    case 1:   opr.regKind() = Brig::BRIG_REGISTER_CONTROL; break;
    case 32:  opr.regKind() = Brig::BRIG_REGISTER_SINGLE;  break;
    case 64:  opr.regKind() = Brig::BRIG_REGISTER_DOUBLE;  break;
    case 128: opr.regKind() = Brig::BRIG_REGISTER_QUAD;    break;
    default:
      assert("invalid register size");
    }

    opr.regNum() = idx;

    return opr;
}

Operand BrigContext::emitVector(unsigned cnt, unsigned size, unsigned idx0)
{
    assert(2 <= cnt && cnt <= 4);

    ItemList opnds;
    for(unsigned i = 0; i < cnt; ++i) opnds.push_back(emitReg(size, idx0 + i));

    OperandOperandList vec = getContainer().append<OperandOperandList>();
    vec.elements() = opnds;

    return vec;
}

Operand BrigContext::emitVector(unsigned cnt, unsigned size, bool isDst /*=true*/, unsigned immCnt /*=0*/)
{
    assert(2 <= cnt && cnt <= 4);
    assert(size == 8 || size == 16 || size == 32 || size == 64);
    assert(immCnt == 0 || !isDst);
    assert(immCnt <= cnt);

    unsigned rsize = (size <= 32)? 32 : 64;
    unsigned wsCnt = (immCnt == cnt)? 1 : 0;

    unsigned i = 0;
    ItemList opnds;
    for(; i <  wsCnt; ++i) opnds.push_back(emitWavesize());
    for(; i < immCnt; ++i) opnds.push_back(emitImm(size, (i == 0)? 0 : -1));
    for(; i < cnt;    ++i) opnds.push_back(emitReg(rsize, isDst? i : 0));

    OperandOperandList vec = getContainer().append<OperandOperandList>();
    vec.elements() = opnds;

    return vec;
}

Operand BrigContext::emitWavesize()
{
    OperandWavesize ws = getContainer().append<OperandWavesize>();
    return ws;
}

Operand BrigContext::emitImm(unsigned size /*=32*/, uint64_t lVal /*=0*/, uint64_t hVal /*=0*/)
{
    OperandData operand = getContainer().append<OperandData>();
    
    ArbitraryData data;
    switch(size)
    {
    case 1:      data.write((uint8_t)(lVal? 1 : 0), 0); break;
    case 8:      data.write((uint8_t)lVal,          0); break;
    case 16:     data.write((uint16_t)lVal,         0); break;
    case 32:     data.write((uint32_t)lVal,         0); break;
    case 64:     data.write((uint64_t)lVal,         0); break;
    case 128:    data.write((uint64_t)lVal,         0); 
                 data.write((uint64_t)hVal, sizeof(uint64_t)); break;
    default:
        assert(false);
    }

    operand.data() = data.toSRef();
    return operand;
}

Operand BrigContext::emitFBarrierRef(DirectiveFbarrier fb)
{
    OperandCodeRef opr = getContainer().append<OperandCodeRef>();
    opr.ref() = fb;

    return opr;
}

Operand BrigContext::emitFuncRef(DirectiveFunction func) //F merge all 'emit*ref' 
{
    assert(func);

    OperandCodeRef ref = getContainer().append<OperandCodeRef>();
    ref.ref() = func;

    return ref;
}

Operand BrigContext::emitIFuncRef(DirectiveIndirectFunction func)
{
    assert(func);

    OperandCodeRef ref = getContainer().append<OperandCodeRef>();
    ref.ref() = func;

    return ref;
}

Operand BrigContext::emitKernelRef(DirectiveKernel k)
{
    assert(k);

    OperandCodeRef ref = getContainer().append<OperandCodeRef>();
    ref.ref() = k;

    return ref;
}

Operand BrigContext::emitSignatureRef(DirectiveSignature sig)
{
    assert(sig);

    OperandCodeRef ref = getContainer().append<OperandCodeRef>();
    ref.ref() = sig;

    return ref;
}

Operand BrigContext::emitAddrRef(DirectiveVariable var, OperandReg reg, unsigned offset /*=0*/)
{
    assert(reg);

    OperandAddress addr = emitAddrRef(var);
    addr.reg()    = reg;
    addr.offset() = offset;
    return addr;
}

Operand BrigContext::emitAddrRef(DirectiveVariable var, uint64_t offset /*=0*/)
{
    OperandAddress addr = getContainer().append<OperandAddress>();

    addr.symbol() = var;
    addr.offset() = offset;
    return addr;
}

Operand BrigContext::emitAddrRef(OperandReg reg, uint64_t offset)
{
    assert(reg);

    OperandAddress addr = getContainer().append<OperandAddress>();
    
    addr.reg()    = reg;
    addr.offset() = offset;
    return addr;
}

Operand BrigContext::emitAddrRef(uint64_t offset)
{
    OperandAddress addr = getContainer().append<OperandAddress>();
    
    addr.offset() = offset;
    return addr;
}

//=============================================================================
//=============================================================================
//=============================================================================

void BrigContext::initSbrDef(DirectiveExecutable sbr, string name)
{
    sbr.name()                     = name;
    sbr.outArgCount()              = 0;
    sbr.inArgCount()               = 0;
    sbr.firstInArg()               = container.code().end(); // no params
    sbr.linkage()                  = DirectiveSignature(sbr)? Brig::BRIG_LINKAGE_NONE : Brig::BRIG_LINKAGE_MODULE;
    sbr.modifier().isDefinition()  = true;
    sbr.firstCodeBlockEntry()      = getContainer().code().end();
    sbr.nextModuleEntry()          = getContainer().code().end();
    sbr.codeBlockEntryCount()      = 0;
}

DirectiveKernel BrigContext::emitKernel(string name)
{
    kernel = getContainer().append<DirectiveKernel>();
    initSbrDef(kernel, name);
    return kernel;
}

void BrigContext::registerSbrArgs(DirectiveExecutable sbr)
{
    assert(sbr);

    if (sbr.next() != getContainer().code().end())
    {
        unsigned params = 0;

        for (Code c = sbr.next(); c != getContainer().code().end(); c = c.next()) 
        {
            // firstInArg has been initialized for case outArgCount=0.
            // The following code is used when outArgCount=1.
            if (++params == sbr.outArgCount()) sbr.firstInArg() = c.next();
        }

        assert(sbr.inArgCount() == 0 || (sbr.inArgCount() + sbr.outArgCount()) == params);

        sbr.inArgCount() = params - sbr.outArgCount();
        sbr.firstCodeBlockEntry() = getContainer().code().end();
        sbr.nextModuleEntry()     = getContainer().code().end();
    }
    else
    {
        assert(sbr.inArgCount() + sbr.outArgCount() == 0);
    }
}

void BrigContext::endSbrBody(DirectiveExecutable sbr)
{
    assert(sbr);

    if (!DirectiveSignature(sbr))
    {
        // This footer is necessary to avoid hanging labels
        // which refer past the end of the code section
        emitAuxLabel();
        emitRet();

        unsigned cnt = 0;
        for (Code i = sbr.firstCodeBlockEntry(); i != container.code().end(); i = i.next()) ++cnt;
        assert(cnt > 0);

        sbr.codeBlockEntryCount() = cnt;
    }

    sbr.nextModuleEntry() = getContainer().code().end();
}

DirectiveVariable BrigContext::emitArg(unsigned type, string name, unsigned segment /*=Brig::BRIG_SEGMENT_ARG*/)
{
    DirectiveVariable arg = emitSymbol(type, name, segment);
    arg.modifier().isDefinition() = true;
    return arg;
}

DirectiveVariable BrigContext::emitSbrParams(unsigned num, bool isInputParam)
{
    DirectiveVariable res;
    for (unsigned i = 0; i < num; ++i)
    {
        ostringstream s;
        s << (isInputParam? "%in_arg" : "%out_arg") << i;
        DirectiveVariable arg = emitArg(Brig::BRIG_TYPE_U32, s.str());
        if (!res) res = arg;
    }
    return res;
}

void BrigContext::emitSbrEnd(DirectiveExecutable sbr)
{
    endSbrBody(sbr);
}

Operand BrigContext::emitArgList(unsigned num, bool isInputArg)
{
    using namespace Brig;

    OperandCodeList opr = getContainer().append<OperandCodeList>();

    ItemList list;
    for (unsigned i = 0; i < num; ++i)
    {
        ostringstream s;
        s << (isInputArg? "%iarg" : "%oarg") << i;
        DirectiveVariable arg = emitArg(BRIG_TYPE_U32, s.str());
        list.push_back(arg);

        if (isInputArg) // Generate initialization code
        {
            emitSt(BRIG_TYPE_U32, BRIG_SEGMENT_ARG, emitImm(), emitAddrRef(arg));
        }
    }

    opr.elements() = list;

    return opr;
}

//void BrigContext::emitCall(DirectiveFunction func, unsigned outArgs, unsigned inArgs)
//{
//    DirectiveArgBlockStart s1 = getContainer().append<DirectiveArgBlockStart>();
//    s1.code() = getContainer().insts().end();
//    {
//        OperandCodeRef target = getContainer().append<OperandCodeRef>();
//        target.fn() = func;
//
//        Operand outList = emitArgList(outArgs, false);
//        Operand inList  = emitArgList(inArgs, true);
//
//        InstBr inst = getContainer().append<InstBr>();
//        inst.opcode() = Brig::BRIG_OPCODE_CALL;
//        inst.type()   = Brig::BRIG_TYPE_NONE;
//
//        inst.operand(0) = outList;
//        inst.operand(1) = target;
//        inst.operand(2) = inList;
//
//        inst.width()    = getDefWidth(inst, getModel(), getProfile()); // Depends on operand 1
//    }
//    DirectiveArgBlockEnd e1 = getContainer().append<DirectiveArgBlockEnd>();
//    e1.code() = getContainer().insts().end();
//}

unsigned BrigContext::conv2LdStType(unsigned type) // Convert to type supported by ld/st
{
    using namespace Brig;

    if (isSignedType(type) || isFloatType(type) || isOpaqueType(type)) return type;

    switch(getBrigTypeNumBits(type)) 
    {
    case 8:      return BRIG_TYPE_U8;
    case 16:     return BRIG_TYPE_U16;
    case 32:     return BRIG_TYPE_U32;
    case 64:     return BRIG_TYPE_U64;
    case 128:    return BRIG_TYPE_B128;

    default: 
        assert(false); 
        return BRIG_TYPE_NONE;
    }
}

//=============================================================================
//=============================================================================
//=============================================================================

} // namespace TESTGEN
