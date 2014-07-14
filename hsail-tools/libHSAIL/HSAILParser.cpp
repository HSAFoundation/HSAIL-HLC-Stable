// University of Illinois/NCSA
// Open Source License
//
// Copyright (c) 2013, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Developed by:
//
//     HSA Team
//
//     Advanced Micro Devices, Inc
//
//     www.amd.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal with
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimers.
//
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimers in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the names of the LLVM Team, University of Illinois at
//       Urbana-Champaign, nor the names of its contributors may be used to
//       endorse or promote products derived from this Software without specific
//       prior written permission.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
// SOFTWARE.
#include "HSAILUtilities.h"
#include "HSAILParser.h"
#include "HSAILFloats.h"
#include "HSAILConvertors.h"
#include <iosfwd>
#include <sstream>

namespace HSAIL_ASM
{

/*
typedef struct HsaBrig {
  void *string_section;
  void *directive_section;
  void *code_section;
  void *operand_section;
  void *debug_section;
  void *loadmap_section;
  void *global_variables_base;
  size_t string_section_byte_size;
  size_t directive_section_byte_size;
  size_t code_section_byte_size;
  size_t operand_section_byte_size;
  size_t debug_section_byte_size;
  size_t loadmap_section_byte_size;
} HsaBrig;


void fx(const HsaBrig* brig, const char* kernelName) {
    BrigContainer c(
        (char*)brig->string_section,    brig->string_section_byte_size,
        (char*)brig->directive_section, brig->directive_section_byte_size,
        (char*)brig->code_section,      brig->code_section_byte_size, 
        (char*)brig->operand_section,   brig->operand_section_byte_size, 
        (char*)brig->debug_section,     brig->debug_section_byte_size);

    DirectiveExecutable found;
    for(Directive d = c.directives().begin(), e = c.directives().end(); d!=e; ) {
        DirectiveExecutable exec = d;
        if (exec) {
            if (exec.name() == kernelName) {
                if (!found) 
                    found = exec;
                else 
                    return; // error
            }
            d = exec.nextTopLevelDirective();
        } else 
            d = d.next;
    }
}
*/

using namespace std;

typedef Optional<unsigned> OptionalU;

struct DeclPrefix
{
    Optional<bool>                 hasConst;
    Optional<Brig::BrigAlignment>  align;
    Optional<Brig::BrigLinkage8_t> linkage;
};

// Parse helpers


template <Brig::BrigTypeX type>
static typename BrigType<type>::CType readInt(Scanner& s) {
    return s.readIntValue<BrigType<type>,StaticCastConvert>();
}

template <Brig::BrigTypeX type>
static typename BrigType<type>::CType readNonNegativeInt(Scanner& s) {
    return s.readIntValue<BrigType<type>,ConvertIfNonNegativeInt>();
}

template <Brig::BrigTypeX type>
static typename BrigType<type>::CType readPositiveInt(Scanner& s) {
    return s.readIntValue<BrigType<type>,ConvertIfPositiveInt>();
}

template <typename BrigElemType, size_t N>
MySmallArray<typename BrigElemType::CType,N> readPackedLiteralInsideParens(Scanner& scanner)
{
    MySmallArray<typename BrigElemType::CType,N> res;
    for(size_t i=N-1; i > 0; --i) {
        res[i] = scanner.readValue<BrigElemType,ConvertImmediate >();
        scanner.eatToken(EComma);
    }
    res[0] = scanner.readValue<BrigElemType,ConvertImmediate >();
    return res;
}


template <typename DstBrigType,
          template<typename, typename> class Convertor>
class ReadPackedLiteral
{
    Scanner& m_scanner;

    template <typename T,size_t N>
    typename DstBrigType::CType visitImpl(BrigTypePacked<T,N>*) const {
        return convert<DstBrigType, BrigTypePacked<T,N> ,Convertor>(
            readPackedLiteralInsideParens<T,N>(m_scanner));
    }
    typename DstBrigType::CType visitImpl(...) const {
        assert(false); // no other than packed types expected
        return typename DstBrigType::CType();
    }

public:
    ReadPackedLiteral(Scanner& scanner) : m_scanner(scanner) {}

    template <typename BrigType>
    typename DstBrigType::CType visit() const {
        return visitImpl(reinterpret_cast<BrigType*>(NULL)); // dispatch
    }
    typename DstBrigType::CType visitNone(unsigned) const {
        assert(false);
        return typename DstBrigType::CType();
    }
};


template <typename DstBrigType,
          template<typename, typename> class Convertor>
typename DstBrigType::CType readPackedLiteral(Scanner& scanner)
{
    scanner.eatToken(EPackedLiteral);
    SrcLoc   const        srcLoc = scanner.token().srcLoc();
    unsigned const typeFromToken = scanner.token().brigId();

    scanner.eatToken(ELParen);
    typename DstBrigType::CType res = typename DstBrigType::CType();
    try {
        res = dispatchByType<typename DstBrigType::CType>(typeFromToken,ReadPackedLiteral<DstBrigType,Convertor>(scanner));
    } catch (const ConversionError& e) {
        scanner.syntaxError(e.what(),srcLoc); // translate it to syntax error
    }
    scanner.eatToken(ERParen);
    return res;
}

template <typename DstBrigType,
          template<typename, typename> class Convertor>
typename DstBrigType::CType  readValueIncludingPacked(Scanner& scanner)
{
    if (scanner.peek().kind()==EPackedLiteral) {
        return readPackedLiteral<DstBrigType,Convertor>(scanner);
    }
    return scanner.readValue<DstBrigType,Convertor>();
}


// Mnemo parsers


Brig::BrigWidth toBrigWidth(uint32_t v)
{
    assert( (v & (v-1))==0 ); // must be a power of two
    switch(v) {
    case 1u          : return Brig::BRIG_WIDTH_1;
    case 2u          : return Brig::BRIG_WIDTH_2;
    case 4u          : return Brig::BRIG_WIDTH_4;
    case 8u          : return Brig::BRIG_WIDTH_8;
    case 16u         : return Brig::BRIG_WIDTH_16;
    case 32u         : return Brig::BRIG_WIDTH_32;
    case 64u         : return Brig::BRIG_WIDTH_64;
    case 128u        : return Brig::BRIG_WIDTH_128;
    case 256u        : return Brig::BRIG_WIDTH_256;
    case 512u        : return Brig::BRIG_WIDTH_512;
    case 1024u       : return Brig::BRIG_WIDTH_1024;
    case 2048u       : return Brig::BRIG_WIDTH_2048;
    case 4096u       : return Brig::BRIG_WIDTH_4096;
    case 8192u       : return Brig::BRIG_WIDTH_8192;
    case 16384u      : return Brig::BRIG_WIDTH_16384;
    case 32768u      : return Brig::BRIG_WIDTH_32768;
    case 65536u      : return Brig::BRIG_WIDTH_65536;
    case 131072u     : return Brig::BRIG_WIDTH_131072;
    case 262144u     : return Brig::BRIG_WIDTH_262144;
    case 524288u     : return Brig::BRIG_WIDTH_524288;
    case 1048576u    : return Brig::BRIG_WIDTH_1048576;
    case 2097152u    : return Brig::BRIG_WIDTH_2097152;
    case 4194304u    : return Brig::BRIG_WIDTH_4194304;
    case 8388608u    : return Brig::BRIG_WIDTH_8388608;
    case 16777216u   : return Brig::BRIG_WIDTH_16777216;
    case 33554432u   : return Brig::BRIG_WIDTH_33554432;
    case 67108864u   : return Brig::BRIG_WIDTH_67108864;
    case 134217728u  : return Brig::BRIG_WIDTH_134217728;
    case 268435456u  : return Brig::BRIG_WIDTH_268435456;
    case 536870912u  : return Brig::BRIG_WIDTH_536870912;
    case 1073741824u : return Brig::BRIG_WIDTH_1073741824;
    case 2147483648u : return Brig::BRIG_WIDTH_2147483648;
    default: assert(false);
    }
    return Brig::BRIG_WIDTH_NONE;
}

OptionalU tryParseWidthModifier(Scanner& scanner) {
    OptionalU res;
    if (scanner.tryEatToken(EMWidth)) {
        scanner.eatToken(ELParen);
        switch(scanner.peek().kind()) {
        case EKWWidthAll:    scanner.scan(); res = Brig::BRIG_WIDTH_ALL;      break;
        case EWaveSizeMacro: scanner.scan(); res = Brig::BRIG_WIDTH_WAVESIZE; break;
        default:
            uint32_t const width = readInt<Brig::BRIG_TYPE_U32>(scanner);
            if (width < 1u || width > 2147483648u) {
                scanner.syntaxError("Invalid width");
            }
            if (width & (width-1)) {
                scanner.syntaxError("Width must be a power of two");
            }
            res = toBrigWidth(width);
            break;
        }
        scanner.eatToken(ERParen);
    }
    return res;
}

OptionalU tryParseAlignModifier(Scanner& scanner) {
    OptionalU res;
    if (scanner.tryEatToken(EMAlign)) {
        scanner.eatToken(ELParen);

        res = num2align(readInt<Brig::BRIG_TYPE_U32>(scanner));
        if (res.value() == Brig::BRIG_ALIGNMENT_NONE) {
            scanner.syntaxError("Invalid alignment");
        }
        scanner.eatToken(ERParen);
    }
    return res;
}

OptionalU tryParseEquiv(Scanner& scanner) {
    OptionalU equivClass;
    if (scanner.tryEatToken(EMEquiv)) {
        scanner.eatToken(ELParen);
        equivClass = readInt<Brig::BRIG_TYPE_U32>(scanner);
        if (equivClass.value() >= 256) {
            scanner.syntaxError("equivalence class should be in the 0..255 range");
        }
        scanner.eatToken(ERParen);
    }
    return equivClass;
}

Inst parseMnemoBasic(Scanner& scanner, Brigantine& bw, bool expectType) {
    unsigned const opCode = scanner.eatToken(EInstruction);

    OptionalU type;
    if (expectType) type = scanner.eatToken(EMType);
    scanner.eatToken(EMNone);
    // parse done

    if (opCode == Brig::BRIG_OPCODE_CODEBLOCKEND) {
        scanner.syntaxError("Instruction codeblockend is not allowed in HSAIL code");
    }
    InstBasic inst = bw.addInst<InstBasic>(opCode);
    inst.type() = type.isInitialized() ? Brig::BrigTypeX(type.value()) : Brig::BRIG_TYPE_NONE;
    return inst;
}

Inst parseMnemoBasic(Scanner& scanner, Brigantine& bw) {
    return parseMnemoBasic(scanner,bw,true);
}

Inst parseMnemoBasicNoType(Scanner& scanner, Brigantine& bw) {
    Inst res = parseMnemoBasic(scanner,bw,false);
    if (isGcnInst(res.opcode())) res.type() = Brig::BRIG_TYPE_B32; // default type for GCN
    return res;
}

Inst parseMnemoBasicOrMod(Scanner& scanner, Brigantine& bw) {
    unsigned  const opCode  = scanner.eatToken(EInstruction);
    OptionalU const ftz     = scanner.tryEatToken(EMFTZ);
    OptionalU const round   = scanner.tryEatToken(EMRound);
    OptionalU const packing = scanner.tryEatToken(EMPacking);
    unsigned  const type    = scanner.eatToken(EMType);
    scanner.eatToken(EMNone);
    // parse done

    if (ftz.isInitialized() || round.isInitialized() || packing.isInitialized()) {
        InstMod inst = bw.addInst<InstMod>(opCode);
        inst.modifier().ftz() = ftz.isInitialized();
        inst.pack() = packing.isInitialized() ? Brig::BrigPack(packing.value()) : Brig::BRIG_PACK_NONE;
        inst.type() = type;

        // NB: getDefRounding must be called after all other fields are initialized
        inst.modifier().round() = round.isInitialized() ? round.value() : getDefRounding(inst, bw.getMachineModel(), bw.getProfile());

        return inst;
    } else {
        InstBasic inst = bw.addInst<InstBasic>(opCode);
        inst.type() = type;
        return inst;
    }
}

Inst parseMnemoSourceType(Scanner& scanner, Brigantine& bw, int* outVector/* out */) {  
    Scanner::CToken& t = scanner.scan();
    assert(t.kind()==EInstruction || t.kind()==EInstruction_Vx);

    unsigned  const opCode = t.brigId();
    OptionalU const vector = scanner.tryEatToken(EMVector);
    unsigned  const dtype  = scanner.eatToken(EMType);
    unsigned  const stype  = scanner.eatToken(EMType);
    scanner.eatToken(EMNone);
    // parse done

    InstSourceType inst = bw.addInst<InstSourceType>(opCode);
    inst.sourceType() = stype;
    inst.type() = dtype;
    if (outVector!=NULL) {
        *outVector = vector.isInitialized() ? vector.value() : 1;
    }
    return inst;
}

Inst parseMnemoSourceType(Scanner& scanner, Brigantine& bw) {
    return parseMnemoSourceType(scanner,bw,NULL);
}

Inst parseMnemoSeg(Scanner& scanner, Brigantine& bw) {
    unsigned  const  opCode = scanner.eatToken(EInstruction);
    OptionalU const segment = scanner.tryEatToken(EMSegment);
    unsigned  const    type = scanner.eatToken(EMType);
    scanner.eatToken(EMNone);
    // parse done

    InstSeg inst = bw.addInst<InstSeg>(opCode);
    inst.segment() = segment.isInitialized()  ? Brig::BrigSegment(segment.value()) : Brig::BRIG_SEGMENT_FLAT;
    inst.type() = type;
    return inst;
}

Inst parseMnemoAddr(Scanner& scanner, Brigantine& bw) {
    unsigned  const  opCode = scanner.eatToken(EInstruction);
    OptionalU const segment = scanner.tryEatToken(EMSegment);
    unsigned  const    type = scanner.eatToken(EMType);
    scanner.eatToken(EMNone);
    // parse done

    InstAddr inst = bw.addInst<InstAddr>(opCode);
    inst.segment() = segment.isInitialized() ?
        segment.value() :
        isGcnInst(inst.opcode())? Brig::BRIG_SEGMENT_EXTSPACE0 : Brig::BRIG_SEGMENT_FLAT;
    inst.type() = type;
    return inst;
}

Inst parseMnemoMem(Scanner& scanner, Brigantine& bw, int* outVector/* out */) {
    Scanner::CToken& t=scanner.scan();
    assert(t.kind()==EInstruction || t.kind()==EInstruction_Vx);

    unsigned  const opCode     = t.brigId();
    OptionalU const vector     = scanner.tryEatToken(EMVector);
    OptionalU const segment    = scanner.tryEatToken(EMSegment);
    OptionalU const align      = tryParseAlignModifier(scanner);
    OptionalU const isConst    = scanner.tryEatToken(EMConst);
    OptionalU const equivClass = tryParseEquiv(scanner);
    if (opCode==Brig::BRIG_OPCODE_ALLOCA && equivClass.isInitialized()) {
        scanner.syntaxError("equiv modifier is not supported");
    }
    OptionalU const width      = tryParseWidthModifier(scanner);
    unsigned  const type       = scanner.eatToken(EMType);
    scanner.eatToken(EMNone);
    // parse done

    InstMem inst = bw.addInst<InstMem>(opCode);
    inst.type()       = type;
    inst.segment()    = segment.isInitialized() ?    Brig::BrigSegment(segment.value())    : Brig::BRIG_SEGMENT_FLAT;
    inst.equivClass() = equivClass.isInitialized() ? equivClass.value() : 0;
    inst.width()      = width.isInitialized() ?      width.value()      : getDefWidth(inst, bw.getMachineModel(), bw.getProfile());
    inst.align()      = align.isInitialized() ?      align.value()      : Brig::BRIG_ALIGNMENT_1;
    inst.modifier().isConst()  = isConst.isInitialized();

    if (outVector!=NULL) {
        *outVector = vector.isInitialized() ? vector.value() : 1;
    }
    return inst;
}

Inst parseMnemoMem(Scanner& scanner, Brigantine& bw) {
    return parseMnemoMem(scanner,bw,NULL);
}

Inst parseMnemoBr(Scanner& scanner, Brigantine& bw) {
    unsigned  const opCode = scanner.eatToken(EInstruction);
    OptionalU const  width = tryParseWidthModifier(scanner);
    if (opCode==Brig::BRIG_OPCODE_WAVEBARRIER && width.isInitialized()) {
        scanner.syntaxError("width modifier is not supported");
    }
    scanner.eatToken(EMNone);
    // parse done

    InstBr inst = bw.addInst<InstBr>(opCode,Brig::BRIG_TYPE_NONE);

    if (width.isInitialized()) {
        inst.width() = Brig::BrigWidth(width.value());
    }
    else if (inst.opcode() == Brig::BRIG_OPCODE_BRN || 
             inst.opcode() == Brig::BRIG_OPCODE_CBR || 
             inst.opcode() == Brig::BRIG_OPCODE_CALL) {
        // should initialize width using getDefWidth, 
        // but it is only possible after parsing branch/call args.
        // This will be fixed later, after parsing operands
        inst.width() = Brig::BRIG_WIDTH_NONE;
    } else {
        inst.width() = getDefWidth(inst, bw.getMachineModel(), bw.getProfile());
    }

    return inst;
}

Inst parseMnemoCmp(Scanner& scanner, Brigantine& bw) {
    unsigned  const opCode  = scanner.eatToken(EInstruction);
    unsigned  const compOp  = scanner.eatToken(EMCompare);
    OptionalU const ftz     = scanner.tryEatToken(EMFTZ);
    OptionalU const packing = scanner.tryEatToken(EMPacking);
    unsigned  const dstType = scanner.eatToken(EMType, "destination type");
    unsigned  const srcType = scanner.eatToken(EMType, "source type");
    scanner.eatToken(EMNone);
    // parse done

    InstCmp inst = bw.addInst<InstCmp>(opCode,dstType);
    inst.compare()        = compOp;
    inst.sourceType()     = srcType;
    inst.modifier().ftz() = ftz.isInitialized();
    inst.pack()           = packing.isInitialized() ? Brig::BrigPack(packing.value()) : Brig::BRIG_PACK_NONE;
    return inst;
}

Inst parseMnemoCvt(Scanner& scanner, Brigantine& bw) {
    unsigned  const opCode  = scanner.eatToken(EInstruction);
    OptionalU const ftz     = scanner.tryEatToken(EMFTZ); // TBD
    OptionalU const round   = scanner.tryEatToken(EMRound); // TBD is this correct?
    unsigned  const dstType = scanner.eatToken(EMType, "destination type");
    unsigned  const srcType = scanner.eatToken(EMType, "source type");
    scanner.eatToken(EMNone);
    // parse done

    InstCvt inst = bw.addInst<InstCvt>(opCode,dstType);
    inst.sourceType()       = srcType;
    inst.modifier().ftz()   = ftz.isInitialized();
    
    // NB: getDefRounding must be called after all other fields are initialized
    inst.modifier().round() = round.isInitialized() ? round.value() : getDefRounding(inst, bw.getMachineModel(), bw.getProfile());

    return inst;
}

Inst parseMnemoAtomic(Scanner& scanner, Brigantine& bw) {
    unsigned  const opCode          = scanner.eatToken(EInstruction);
    unsigned  const atomicOperation = scanner.eatToken(EMAtomicOp);
    OptionalU const segment         = scanner.tryEatToken(EMSegment);
    unsigned  const memoryOrder     = scanner.eatToken(EMMemoryOrder);
    unsigned  const memoryScope     = scanner.eatToken(EMMemoryScope);
    OptionalU const equivClass      = tryParseEquiv(scanner);
    unsigned  const type            = scanner.eatToken(EMType);
    scanner.eatToken(EMNone);
    // parse done

    InstAtomic inst = bw.addInst<InstAtomic>(opCode,type);
    inst.atomicOperation() = atomicOperation;
    inst.segment()         = segment.isInitialized() ? Brig::BrigSegment(segment.value()) : Brig::BRIG_SEGMENT_FLAT;
    inst.equivClass()      = equivClass.isInitialized() ? equivClass.value() : 0;
    inst.memoryOrder()     = memoryOrder;
    inst.memoryScope()     = memoryScope;
    return inst;
}

Inst parseMnemoMemFence(Scanner& scanner, Brigantine& bw) {
    unsigned  const opCode      = scanner.eatToken(EInstruction);
    OptionalU const fence       = scanner.tryEatToken(EMMemoryFenceSegments);
    unsigned  const memoryOrder = scanner.eatToken(EMMemoryOrder);
    unsigned  const memoryScope = scanner.eatToken(EMMemoryScope);
    scanner.eatToken(EMNone);
    // parse done

    InstMemFence inst = bw.addInst<InstMemFence>(opCode,Brig::BRIG_TYPE_NONE);

    inst.segments()    = fence.isInitialized() ? fence.value() : Brig::BRIG_MEMORY_FENCE_BOTH;
    inst.memoryOrder() = memoryOrder;
    inst.memoryScope() = memoryScope;
    return inst;
}

Inst parseMnemoImage(Scanner& scanner, Brigantine& bw, int* outVector/* out */) {
    Scanner::CToken& t=scanner.scan();
    assert(t.kind()==EInstruction || t.kind()==EInstruction_Vx);

    unsigned  const opCode     = scanner.token().brigId();
    OptionalU const vector     = scanner.tryEatToken(EMVector);    
    unsigned  const geom       = scanner.eatToken(EMGeom);
    OptionalU const equivClass = tryParseEquiv(scanner);
    unsigned  const dstType    = scanner.eatToken(EMType);
    unsigned  const imgType    = scanner.eatToken(EMType);
    unsigned  const coordType  = scanner.eatToken(EMType);
    scanner.eatToken(EMNone);
    // parse done

    InstImage inst    = bw.addInst<InstImage>(opCode,dstType);
    inst.equivClass() = equivClass.isInitialized() ? equivClass.value() : 0;
    inst.geometry()   = geom;
    inst.imageType()  = imgType;
    inst.coordType()  = coordType;

    if (outVector!=NULL) {
        *outVector = vector.isInitialized() ? vector.value() : 1;
    }

    return inst;
}

Inst parseMnemoImage(Scanner& scanner, Brigantine& bw) {
    return parseMnemoImage(scanner,bw,NULL);
}

Inst parseMnemoQueryImage(Scanner& scanner, Brigantine& bw) {
    unsigned  const opCode          = scanner.eatToken(EInstruction);
    unsigned  const geom            = scanner.eatToken(EMGeom);
    unsigned  const query           = scanner.eatToken(EMImageQuery);
    unsigned  const dstType         = scanner.eatToken(EMType);
    unsigned  const imgType         = scanner.eatToken(EMType);
    scanner.eatToken(EMNone);
    // parse done

    InstQueryImage inst    = bw.addInst<InstQueryImage>(opCode,dstType);
    inst.geometry()        = geom;
    inst.imageQuery()      = query;
    inst.imageType()       = imgType;
    return inst;
}

Inst parseMnemoQuerySampler(Scanner& scanner, Brigantine& bw) {
    unsigned  const opCode          = scanner.eatToken(EInstruction);
    unsigned  const query           = scanner.eatToken(EMSamplerQuery);
    unsigned  const dstType         = scanner.eatToken(EMType);
    scanner.eatToken(EMNone);
    // parse done

    InstQuerySampler inst  = bw.addInst<InstQuerySampler>(opCode,dstType);
    inst.samplerQuery()    = query;
    return inst;
}

Inst parseMnemoLane(Scanner& scanner, Brigantine& bw, int* outVector/* out */) {
    Scanner::CToken& t=scanner.scan();
    assert(t.kind()==EInstruction || t.kind()==EInstruction_Vx);
    unsigned  const opCode = scanner.token().brigId();
    OptionalU const vector = scanner.tryEatToken(EMVector);    
    OptionalU const width  = tryParseWidthModifier(scanner);
    unsigned  const dtype  = scanner.eatToken(EMType);
    OptionalU const stype  = scanner.tryEatToken(EMType);
    scanner.eatToken(EMNone);
    // parse done

    InstLane inst = bw.addInst<InstLane>(opCode);
    inst.sourceType() = stype.isInitialized() ? stype.value() : Brig::BRIG_TYPE_NONE;
    inst.width() = width.isInitialized() ? width.value() : getDefWidth(inst, bw.getMachineModel(), bw.getProfile());
    inst.type() = dtype;

    if (outVector!=NULL) {
        *outVector = vector.isInitialized() ? vector.value() : 1;
    }
    return inst;
}

Inst parseMnemoLane(Scanner& scanner, Brigantine& bw) {
    return parseMnemoLane(scanner, bw, NULL);
}

Inst parseMnemoNop(Scanner& scanner, Brigantine& bw) {
    scanner.eatToken(EInstruction);
    InstBasic inst = bw.addInst<InstBasic>(Brig::BRIG_OPCODE_NOP);
    inst.type() = Brig::BRIG_TYPE_NONE;
    return inst;
}

Inst parseMnemoQueue(Scanner& scanner, Brigantine& bw) {
    unsigned  const opCode      = scanner.eatToken(EInstruction);
    OptionalU const segment     = scanner.tryEatToken(EMSegment);
    unsigned  const memoryOrder = scanner.eatToken(EMMemoryOrder);
    unsigned  const type        = scanner.eatToken(EMType);
    scanner.eatToken(EMNone);
    // parse done

    InstQueue inst = bw.addInst<InstQueue>(opCode);
    inst.segment() = segment.isInitialized()  ? Brig::BrigSegment(segment.value()) : Brig::BRIG_SEGMENT_FLAT;
    inst.memoryOrder() = memoryOrder;
    inst.type() = type;
    return inst;
}

Inst parseMnemoSignal(Scanner& scanner, Brigantine& bw) {
    unsigned const opCode          = scanner.eatToken(EInstruction);
    unsigned const signalOperation = scanner.eatToken(EMAtomicOp);
    unsigned const memoryOrder     = scanner.eatToken(EMMemoryOrder);
    unsigned const type            = scanner.eatToken(EMType);
    unsigned const signalType      = scanner.eatToken(EMType);
    scanner.eatToken(EMNone);
    // parse done

    InstSignal inst = bw.addInst<InstSignal>(opCode);
    inst.signalOperation() = signalOperation;
    inst.memoryOrder() = memoryOrder;
    inst.type() = type;
    inst.signalType() = signalType;
    return inst;
}

Inst parseMnemoSegCvt(Scanner& scanner, Brigantine& bw) {
    unsigned  const opCode   = scanner.eatToken(EInstruction);
    OptionalU const segment  = scanner.tryEatToken(EMSegment);
    OptionalU const isNoNull = scanner.tryEatToken(EMNoNull);
    unsigned  const dtype    = scanner.eatToken(EMType);
    OptionalU const stype    = scanner.tryEatToken(EMType);
    scanner.eatToken(EMNone);
    // parse done

    InstSegCvt inst = bw.addInst<InstSegCvt>(opCode);
    inst.sourceType() = stype.isInitialized() ? Brig::BrigTypeX(stype.value()) : Brig::BRIG_TYPE_NONE;
    inst.segment() = segment.isInitialized()  ? Brig::BrigSegment(segment.value()) : Brig::BRIG_SEGMENT_FLAT;
    inst.modifier().isNoNull() = isNoNull.isInitialized();
    inst.type() = dtype;
    return inst;
}

typedef Inst (*OpcodeParser)(Scanner&, Brigantine &);
OpcodeParser getOpcodeParser(Brig::BrigOpcode16_t opcode); // generated

Inst parseMnemo(Scanner& scanner, Brigantine& bw) {
    Inst res;
    switch(scanner.peek().kind()) {
    case EInstruction:
    case EInstruction_Vx: {
            OpcodeParser const parser = getOpcodeParser(scanner.peek().brigId());
            if (!parser) {
                 scanner.syntaxError("unknown instruction");
            }
            res = parser(scanner,bw);
        } break;

    default:;
    }
    return res;
}

Inst parseMnemo(const char* str, Brigantine& bw) {
    std::istringstream instr(str);
    Scanner scanner(instr);
    return parseMnemo(scanner,bw);
}

// Parser

/*
struct ParseGuard {
    const char* m_fn;
    ParseGuard(const char* fn) : m_fn(fn) {
        std::cout << "<" << fn << "> ";
    }
    ~ParseGuard() { std::cout << "</" << m_fn << "> "; }
};

#define PDBG ParseGuard pdbg__(__FUNCTION__);
*/
#define PDBG

Parser::Parser(Scanner& scanner, BrigContainer& container)
    : m_scanner(scanner)
    , m_bw(container)
    , m_gcnEnabled(false)
{
}

void Parser::parseSource()
{
    PDBG;
    do {
        parseProgram();
    } while (peek().kind()!=EEndOfSource);
}

void Parser::parseProgram()
{
    PDBG;
    m_bw.startProgram();

    m_gcnEnabled = false;

    parseVersion();
    do {
        parseTopLevelStatement();
    } while (peek().kind()!=EEndOfSource && peek().kind()!=EKWVersion);

    storeComments(m_bw.container().insts().end()); // there might be comments before end of the program
    m_bw.endProgram();
}


void Parser::parseVersion()
{
    PDBG;

    eatToken(EKWVersion);
    SourceInfo const srcInfo = sourceInfo(token());   
    uint32_t const major = readNonNegativeInt<Brig::BRIG_TYPE_U32>(m_scanner);
    eatToken(EColon);
    uint32_t const minor = readNonNegativeInt<Brig::BRIG_TYPE_U32>(m_scanner);
    eatToken(EColon);
    unsigned const profile = eatToken(ETargetProfile);
    eatToken(EColon);
    unsigned const machineModel = eatToken(ETargetMachine);
    eatToken(ESemi);

    m_bw.version(major,minor,machineModel,profile,&srcInfo);
}

inline static bool IsDeclStart(ETokens t)
{
    return (t==EKWConst || t==EKWAlign || t==ESegment || t==EAttribute);
}

void Parser::storeComments(Inst before)
{
    while (m_scanner.hasComments()) {
        DirectiveComment cmt = m_bw.container().append<DirectiveComment>();
        SRef const s = m_scanner.grabComment();
        cmt.name() = std::string("//").append(s.begin, s.end);
        cmt.code() = before;
    }
}



void Parser::parseTopLevelStatement()
{
    PDBG;

    storeComments(m_bw.container().insts().end());

    switch (peek().kind()) {
    case EKWKernel:       parseKernel(); break;
    case EKWFunction:     parseFunction(); break;
    case EKWSignature:    parseSignature(); break;
    case EKWExtension:    parseExtension();break;
    case EKWPragma:       parsePragma();break;
    case EKWBlockStart:   parseBlock();break;
    case EKWFbarrier:     parseFbarrier(false);break;
    case EControl:        parseControl();break;
    case EKWLoc:          parseLocation(); break;
    default:
        if (IsDeclStart(peek().kind())) {
            DeclPrefix const declPfx = parseDeclPrefix();
            switch (peek().kind()) {
            case EKWKernel:   parseKernel(&declPfx); break;
            case EKWFunction: parseFunction(&declPfx); break;
            case ESegment:
                parseDecl(false,false,declPfx);
                eatToken(ESemi);
            break;
            default:
                syntaxError("Unexpected token after declaration prefix", peek());
            }
        } else {
            syntaxError("Unexpected token at top level", peek());
        }
    }
}

Optional<uint16_t> Parser::tryParseFBar()
{
    Optional<uint16_t> res;
    if (tryEatToken(EColon)) {
        eatToken(EKWFBar);
        eatToken(ELParen);
        res = readNonNegativeInt<Brig::BRIG_TYPE_U16>(m_scanner);
        eatToken(ERParen);
    }
    return res;
}

void Parser::parseKernel(const DeclPrefix* declPrefix) // declPrefix is not used by spec but cannot be ignored
{
    if (declPrefix) {
        if (declPrefix->align.isInitialized())    syntaxError("Align is not allowed with kernels");
        if (declPrefix->hasConst.isInitialized()) syntaxError("Const is not allowed with kernels");
        if (declPrefix->linkage.isInitialized())  syntaxError("Extern/static is not allowed with kernels");
    }
    PDBG;
    eatToken(EKWKernel);
    SourceInfo const srcInfo = sourceInfo(token());

    eatToken(EIDStatic);
    SRef const funcName = token().text();

    DirectiveKernel kern = m_bw.declKernel(funcName,&srcInfo);

    eatToken(ELParen);

    if (!tryEatToken(ERParen)) {
        Directive first = parseDecl(true,true);
        if (first) {
            m_bw.addInputParameter(first);
            while(tryEatToken(EComma)) {
                m_bw.addInputParameter(parseDecl(true,true));
            }
        }
        eatToken(ERParen);
    }

    if (!kern.firstInArg()) {
        kern.firstInArg() = m_bw.container().directives().end();
    }

    if (peek().kind()==ELCurl) {
        parseCodeBlock();
    } else {
        kern.modifier().isDeclaration() = true;
        kern.firstScopedDirective() = m_bw.container().directives().end();
    }

    eatToken(ESemi);
}

void Parser::parseFunction(const DeclPrefix* declPrefix)
{
    if (declPrefix) {
        if (declPrefix->align.isInitialized())    syntaxError("Align is not allowed with functions");
        if (declPrefix->hasConst.isInitialized()) syntaxError("Const is not allowed with functions");
    }

    PDBG;
    eatToken(EKWFunction);
    SourceInfo const srcInfo = sourceInfo(token());

    eatToken(EIDStatic);
    SRef const funcName = token().text();

    DirectiveFunction func = m_bw.declFunc(funcName,&srcInfo);
    if (declPrefix && declPrefix->linkage.isInitialized()) {
        func.modifier().linkage() = declPrefix->linkage.value();
    }

    eatToken(ELParen);

    if (!tryEatToken(ERParen)) {
        Directive first = parseDecl(true,true);
        if (first) {
            m_bw.addOutputParameter(first);
            while(tryEatToken(EComma)) {
                m_bw.addOutputParameter(parseDecl(true,true));
            }
        }
        eatToken(ERParen);
    }

    eatToken(ELParen);

    if (!tryEatToken(ERParen)) {
        Directive first = parseDecl(true,true);
        if (first) {
            m_bw.addInputParameter(first);
            while(tryEatToken(EComma)) {
                m_bw.addInputParameter(parseDecl(true,true));
            }
        }
        eatToken(ERParen);
    }

    if (!func.firstInArg()) {
        func.firstInArg() = m_bw.container().directives().end();
    }

    if (peek().kind()==ELCurl) {
        parseCodeBlock();
    } else {
        func.firstScopedDirective() = m_bw.container().directives().end();
        func.modifier().isDeclaration() = true;
    }

    eatToken(ESemi);
}

void Parser::parseSigArgs(Scanner& s,DirectiveSignatureArguments types, DirectiveSignatureArguments::ArgKind argKind)
{
    eatToken(ELParen);
    if (peek().kind()!=ERParen) {
        do {
            DeclPrefix const declPfx = parseDeclPrefix();
            unsigned   const segment = eatToken(ESegment);
            if (segment!=Brig::BRIG_SEGMENT_ARG) {
                syntaxError("only arg segment allowed in signature declaration");
            }
            unsigned   const dType   = eatToken(EMType);

            tryEatToken(EIDLocal); // optional arg name (not used)

            Optional<uint64_t> dim;
            if (tryEatToken(ELBrace)) {
                dim = peek().kind() != ERBrace ? readPositiveInt<Brig::BRIG_TYPE_U64>(m_scanner) : 0;
                eatToken(ERBrace);
            }

            Brig::BrigAlignment align = declPfx.align.isInitialized() ? declPfx.align.value() : getNaturalAlignment(dType);

            types.addArg(argKind,dType,dim,align);
        } while (tryEatToken(EComma));
    }
    eatToken(ERParen);
}

void Parser::parseSignature()
{
    PDBG;
    eatToken(EKWSignature);
    SourceInfo const srcInfo = sourceInfo(token());

    eatToken(EIDStatic);
    SRef const name = token().text();

    DirectiveSignature sig = m_bw.declSignature(name,&srcInfo);

    if (peek().kind()==ELParen) {
        parseSigArgs(m_scanner, sig.args(), DirectiveSignatureArguments::Output);
        if (peek().kind()==ELParen) {
            parseSigArgs(m_scanner, sig.args(), DirectiveSignatureArguments::Input);
        } else {
            // oops, these were actually input parameters
            sig.inCount() = sig.outCount();
            sig.outCount() = 0;
        }
    }

    eatToken(ESemi);
}

int Parser::parseCodeBlock()
{
    PDBG;
    eatToken(ELCurl);

    m_bw.startBody();

    int numInsts = 0;
    while (!tryEatToken(ERCurl)) {
        numInsts += parseBodyStatement();
    }

    m_bw.endBody();
    return numInsts;
}

int Parser::parseBodyStatement()
{
    PDBG;
    int numInsts = 0;
    switch (peek().kind()) {
    case ELabel:           parseLabel(); break;
    case EKWPragma:        parsePragma(); break;
    case EEmbeddedText:    parseEmbeddedText(); break;
    case EKWSection:       parseDebug(); break;
    case EKWLoc:           parseLocation(); break;
    case EKWBlockStart:    parseBlock();break;
    case EControl:         parseControl();break;
    case EKWFbarrier:      parseFbarrier(true);break;
    case EKWLabelTargets:  parseLabelTargets(); break;
    case ELCurl:           numInsts += parseArgScope(); break;
    case EInstruction:
    case EInstruction_Vx: {
        Inst i = parseInst();
        storeComments(i);
        ++numInsts;
    } break;
    default:
        if (IsDeclStart(peek().kind())) {
            parseDecl(false,true);
            eatToken(ESemi);
        } else {
            syntaxError("Unexpected token in body statement", peek());
        }
    }
    return numInsts;
}

int Parser::parseArgScope()
{
    eatToken(ELCurl);
    {
        SourceInfo const srcInfo = sourceInfo(token());
        m_bw.startArgScope(&srcInfo);
    }

    int numInsts = 0;
    while (peek().kind()!=ERCurl) {
        numInsts += parseBodyStatement();
    };
    
    eatToken(ERCurl);
    {
        SourceInfo const srcInfo = sourceInfo(token());
        m_bw.endArgScope(&srcInfo);
    }
    return numInsts;
}

void Parser::parseLabel()
{
    PDBG;
    eatToken(ELabel);
    SourceInfo const srcInfo = sourceInfo(token());
    SRef const name = token().text();
    eatToken(EColon);

    m_bw.addLabel(name,&srcInfo);
}

void Parser::parseLabelTargets() 
{
    PDBG;
    eatToken(EKWLabelTargets);
    SourceInfo const srcInfo = sourceInfo(token());

    eatToken(EIDLocal);
    SRef const name = token().text();

    eatToken(EEqual);
    eatToken(ELCurl);
    DirectiveLabelTargets list = m_bw.createLabelTargets(name,&srcInfo);

    parseLabelList(list.labels(), 0);

    eatToken(ERCurl);
    eatToken(ESemi);
}

template <typename List>
unsigned Parser::parseLabelList(List list, unsigned expectedSize)
{
    unsigned numRead = 0;
    do {
        eatToken(ELabel);
        SourceInfo const srcInfo = sourceInfo(token());
        if (expectedSize && numRead > expectedSize) {
            syntaxError("Too many elements");
        }
        const SRef& name = token().text();
        ++numRead;
        m_bw.appendLabelRef(list,name,&srcInfo);
    } while(tryEatToken(EComma));

    if (expectedSize && numRead < expectedSize) {
        syntaxError("more labels expected");
    }
    return numRead;
}

class ParseValueList
{
    Scanner&       m_scanner;
    ArbitraryData& m_data;
    unsigned       m_expElements;

    ParseValueList& operator=(const ParseValueList&);

public:
    ParseValueList(Scanner& scanner, ArbitraryData& data, unsigned expElements)
        : m_scanner(scanner), m_data(data), m_expElements(expElements) {}

    template <typename BrigType> unsigned visit() const {
        typedef typename BrigType::CType CType;
        size_t const numElementsBefore = m_data.numElements<CType>();
        do {
            if (m_expElements && m_data.numElements<CType>() > m_expElements) {
                m_scanner.syntaxError("elements more than expected", m_scanner.peek().srcLoc());
            }
            m_data.push_back(readValueIncludingPacked<BrigType,ConvertImmediate>(m_scanner));
        } while(m_scanner.tryEatToken(EComma));
        return static_cast<unsigned>(m_data.numElements<CType>() - numElementsBefore);
    }

    unsigned visitNone(unsigned ) const {
        assert(false);
        return 0;
    }
};

unsigned Parser::parseValueList(Brig::BrigType16_t type, ArbitraryData& data, unsigned maxValues) {
    return dispatchByType<unsigned>(type,ParseValueList(m_scanner,data,maxValues));
}

Directive Parser::parseVariableInitializer(Brig::BrigType16_t type, unsigned expectedSize)
{
    Directive res;
    SourceInfo const srcInfo = sourceInfo(peek());
    if (peek().kind()==ELabel) {
        if ( m_bw.localScope()==NULL ) {
            syntaxError("label initializers can be placed only inside func/kernel body definition", peek());
        }
        DirectiveLabelInit init = m_bw.createLabelInit(&srcInfo);
        init.elementCount() = parseLabelList(init.labels(),expectedSize);
        res = init;
    } else {
        DirectiveVariableInit init = m_bw.createVariableInitializer(type,&srcInfo);
        ArbitraryData values;
        init.elementCount() = parseValueList(type,values,expectedSize);
        init.data() = values.toSRef();
        res = init;
    }
    return res;
}

DirectiveImageInit Parser::parseImageInitializer(Brig::BrigType16_t type, unsigned expectedSize) 
{
    SourceInfo const srcInfo = sourceInfo(peek());
    ItemRange<Directive> dirRange;  
    do {
        if      (type == Brig::BRIG_TYPE_ROIMG) eatToken(EKWROImg);
        else if (type == Brig::BRIG_TYPE_WOIMG) eatToken(EKWWOImg);
        else if (type == Brig::BRIG_TYPE_RWIMG) eatToken(EKWRWImg);

        if (expectedSize > 0 && dirRange.size() > expectedSize) {
            syntaxError("element count exceeds specified");
        }
        DirectiveImageProperties props = parseImageProperties();
        dirRange.push_back(props);
    } while (tryEatToken(EComma));

    DirectiveImageInit init = m_bw.createImageInitializer(&srcInfo);
    copy(init.images(), dirRange);
    return init;
}

DirectiveSamplerInit Parser::parseSamplerInitializer(Brig::BrigType16_t type, unsigned expectedSize) 
{
    SourceInfo const srcInfo = sourceInfo(peek());
    ItemRange<Directive> dirRange;  
    do {
        eatToken(EKWSamp);
        if (expectedSize > 0 && dirRange.size() > expectedSize) {
            syntaxError("element count exceeds specified");
        }
        DirectiveSamplerProperties props = parseSamplerProperties();
        dirRange.push_back(props);
    } while (tryEatToken(EComma));

    DirectiveSamplerInit init = m_bw.createSamplerInitializer(&srcInfo);
    copy(init.samplers(), dirRange);
    return init;
}

DirectiveImageProperties Parser::parseImageProperties()
{
    eatToken(ELParen);
    SourceInfo const srcInfo = sourceInfo(token());
    DirectiveImageProperties props = m_bw.createImageProperties(&srcInfo);
    do {
        ETokens const t = scan().kind();
        SourceInfo const srcInfo = sourceInfo(token());
        eatToken(EEqual);
        switch(t) {
        case EKWImageWidth:
            if (0 != props.width()) {
                syntaxError("Width already set");
            }
            props.width() = readPositiveInt<Brig::BRIG_TYPE_U32>(m_scanner);
            break;
        case EKWImageHeight:
            if (0 != props.height()) {
                syntaxError("Height already set");
            }
            props.height() = readPositiveInt<Brig::BRIG_TYPE_U32>(m_scanner);
            break;
        case EKWImageDepth:
            if (0 != props.depth()) {
                syntaxError("Depth already set");
            }
            props.depth() = readPositiveInt<Brig::BRIG_TYPE_U32>(m_scanner);
            break;
        case EKWImageArray:
            if (0 != props.array()) {
                syntaxError("Array already set");
            }
            props.array() = readPositiveInt<Brig::BRIG_TYPE_U32>(m_scanner);
            break;
        case EKWImageChannelType:
            if (props.channelType() != Brig::BRIG_CHANNEL_TYPE_UNKNOWN) {
                syntaxError("Channel type already set");
            }
            props.channelType() = eatToken(EImageFormat);
            break;
        case EKWImageChannelOrder:
            if (props.channelOrder() != Brig::BRIG_CHANNEL_ORDER_UNKNOWN) {
                syntaxError("Channel order already set");
            }
            props.channelOrder() = eatToken(EImageOrder);
            break;
        case EKWImageGeometry:
            if (props.geometry() != Brig::BRIG_GEOMETRY_UNKNOWN) {
                syntaxError("Geometry already set");
            }
            props.geometry() = eatToken(EImageGeometry);
            break;
        default:
            syntaxError("Invalid image property name",&srcInfo);
        }
    } while (tryEatToken(EComma));
    eatToken(ERParen);

    if (props.geometry()     == Brig::BRIG_GEOMETRY_UNKNOWN)      syntaxError("Missing image geometry",      &srcInfo);
    if (props.channelOrder() == Brig::BRIG_CHANNEL_ORDER_UNKNOWN) syntaxError("Missing image channel order", &srcInfo);
    if (props.channelType()  == Brig::BRIG_CHANNEL_TYPE_UNKNOWN)  syntaxError("Missing image channel type",  &srcInfo);
    if (props.width()        == 0)                                syntaxError("Missing image width",         &srcInfo);

    unsigned geom = props.geometry();

    if (geom == Brig::BRIG_GEOMETRY_2D      || 
        geom == Brig::BRIG_GEOMETRY_3D      || 
        geom == Brig::BRIG_GEOMETRY_2DA     ||
        geom == Brig::BRIG_GEOMETRY_2DDEPTH ||
        geom == Brig::BRIG_GEOMETRY_2DADEPTH) {
        if (props.height() == 0) syntaxError("Missing image height", &srcInfo);
    } else {
        if (props.height() > 0) syntaxError("Image height cannot be specified for this image geometry", &srcInfo);
    }

    if (geom == Brig::BRIG_GEOMETRY_3D) {
        if (props.depth() == 0) syntaxError("Missing image depth", &srcInfo);
    } else {
        if (props.depth() > 0) syntaxError("Image depth cannot be specified for this image geometry", &srcInfo);
    }

    if (geom == Brig::BRIG_GEOMETRY_1DA     || 
        geom == Brig::BRIG_GEOMETRY_2DA     ||
        geom == Brig::BRIG_GEOMETRY_2DADEPTH) {
        if (props.array() == 0) syntaxError("Missing image array", &srcInfo);
    } else {
        if (props.array() > 0) syntaxError("Image array cannot be specified for this image geometry", &srcInfo);
    }

    return props;
}

DirectiveSamplerProperties Parser::parseSamplerProperties()
{
    eatToken(ELParen);
    SourceInfo const srcInfo = sourceInfo(token());
    DirectiveSamplerProperties props = m_bw.createSamplerProperties(&srcInfo);
    unsigned propMask = 0;
    do {
        ETokens const t = scan().kind();
        SourceInfo const srcInfo = sourceInfo(token());
        eatToken(EEqual);
        if (t >= ESamplerFirstProp && t <= ESamplerLastProp) {
            unsigned bit = (1 << (t - ESamplerFirstProp));
            if (propMask & bit) {
                syntaxError("Duplicate sampler property");
            }
            propMask |= bit;
        }
        switch(t) {
        case EKWSamplerAddressing:
            props.addressing() = eatToken(ESamplerAddressingMode);
            break;
        case EKWSamplerCoord:
            props.coord() = eatToken(ESamplerCoord);
            break;
        case EKWSamplerFilter:
            props.filter() = eatToken(ESamplerFilter);
            break;
        default:
            syntaxError("Invalid sampler object property name",&srcInfo);
        }
    } while (tryEatToken(EComma));
    eatToken(ERParen);
    // TBD simplify
    for(unsigned prop = ESamplerFirstProp; prop <= ESamplerLastProp; ++prop) {
        if (propMask & (1 << (prop - ESamplerFirstProp))) {
            continue;
        }
        switch(prop) {
        case EKWSamplerAddressing:
            syntaxError("addressing value missing");
            break;
        case EKWSamplerCoord:
            syntaxError("coord value missing");
            break;
        case EKWSamplerFilter:
            syntaxError("filter value missing");
            break;
        }
    }
    return props;
}

DeclPrefix Parser::parseDeclPrefix()
{
    DeclPrefix res;

    bool exitDo = false;
    do {
        switch(peek().kind()) {
        case EKWConst:
            eatToken(EKWConst);
            if (res.hasConst.isInitialized()) {
                syntaxError("only one const modificator is allowed");
            }
            res.hasConst = true;
            break;
        case EKWAlign:
            eatToken(EKWAlign);
            if (res.align.isInitialized()) {
                syntaxError("only one align modificator is allowed");
            }           
            res.align = num2align(readPositiveInt<Brig::BRIG_TYPE_U16>(m_scanner));
            if (res.align.value()==Brig::BRIG_ALIGNMENT_NONE) {
                syntaxError("invalid alignment value");
            }
            break;
        case EAttribute:
            eatToken(EAttribute);
            if (res.linkage.isInitialized()) {
                syntaxError("only one attribute is allowed");
            }
            res.linkage = token().brigId();
            break;
        default:
            exitDo = true;
        }
    } while(!exitDo);
    return res;
}

DirectiveVariable Parser::parseDecl(bool isArg, bool isLocal)
{
    PDBG;
    DeclPrefix const declPfx = parseDeclPrefix();
    return parseDecl(isArg,isLocal,declPfx);
}

DirectiveVariable Parser::parseDecl(bool isArg, bool isLocal,const DeclPrefix& declPfx)
{
    unsigned const segment = eatToken(ESegment);
    SourceInfo const srcInfo = sourceInfo(token());

    unsigned const dType = eatToken(EMType, "type");

    eatToken(isLocal ? EIDLocal : EIDStatic);
    SRef const name = token().text();

    DirectiveVariable sym;
    switch(dType) {
    case Brig::BRIG_TYPE_ROIMG:
    case Brig::BRIG_TYPE_RWIMG:
    case Brig::BRIG_TYPE_WOIMG:
        sym = m_bw.addImage(name,segment,&srcInfo);
        sym.type() = dType;
        break;
    case Brig::BRIG_TYPE_SAMP:
        sym = m_bw.addSampler(name,segment,&srcInfo);
        break;
    default:
        sym = m_bw.addVariable(name,segment,dType,&srcInfo);
    }

    sym.align() = declPfx.align.isInitialized() ? declPfx.align.value() : (unsigned) getNaturalAlignment(dType);

    if (declPfx.hasConst.isInitialized())
        sym.modifier().isConst() = declPfx.hasConst.value();

    if (declPfx.linkage.isInitialized())
        sym.modifier().linkage() = declPfx.linkage.value();

    if (tryEatToken(ELBrace)) {
        sym.modifier().isArray() = true;
        if (peek().kind() != ERBrace) {
            sym.dim() = readPositiveInt<Brig::BRIG_TYPE_U32>(m_scanner);
        } else {
            sym.modifier().isFlexArray() =
                sym.modifier().linkage() != Brig::BRIG_LINKAGE_EXTERN;
        }
        eatToken(ERBrace);
    }

    if (tryEatToken(EEqual)) {
        const bool isArray = sym.modifier().isArray();
        if (isArray) {
            eatToken(ELCurl);
        }
        switch(dType) {
        case Brig::BRIG_TYPE_ROIMG:
        case Brig::BRIG_TYPE_WOIMG:
        case Brig::BRIG_TYPE_RWIMG: {
            DirectiveImageInit init = parseImageInitializer(dType, static_cast<unsigned>(sym.dim()));
            sym.init() = init;
            if (isArray && sym.dim() == 0) {
                sym.dim() = init.elementCount();
            }
        } break;
        case Brig::BRIG_TYPE_SAMP: { 
            DirectiveSamplerInit init = parseSamplerInitializer(dType, static_cast<unsigned>(sym.dim()));
            sym.init() = init;
            if (isArray && sym.dim() == 0) {
                sym.dim() = init.elementCount();
            }
        } break;
        default:
            sym.init() = parseVariableInitializer(dType, static_cast<unsigned>(sym.dim()));  // TBD095 check static_cast
            if (isArray && sym.dim() == 0) {
                if (DirectiveVariableInit init = sym.init())
                    sym.dim() = init.elementCount();
                else if (DirectiveLabelInit init = sym.init())
                    sym.dim() = init.elementCount();
            }
        }
        if (isArray) {
            eatToken(ERCurl);
        }
        sym.modifier().isFlexArray() = false;
    }
    sym.modifier().isDeclaration() = (isArg || sym.modifier().linkage() == Brig::BRIG_LINKAGE_EXTERN);
    return sym;
}

DirectiveFbarrier Parser::parseFbarrier(bool isLocal) {
    PDBG;
    eatToken(EKWFbarrier);
      
    eatToken(isLocal ? EIDLocal : EIDStatic);
    SourceInfo const srcInfo = sourceInfo(token());
    SRef const name = token().text();

    DirectiveFbarrier fbar = m_bw.addFbarrier(name,&srcInfo);

    eatToken(ESemi);
    return fbar;
}

static std::string parseStringLiteral(Scanner&);

void Parser::parseBlock()
{
    PDBG;
    eatToken(EKWBlockStart);
    SourceInfo const srcInfo = sourceInfo(token());

    SourceInfo const srcInfo2 = sourceInfo(peek());
    std::string const blockName = parseStringLiteral(m_scanner);

    BrigSectionImpl *section = NULL;
    if (blockName=="rti") {
        section = &m_bw.container().directives();
    }  else if (blockName=="debug") {
        section = &m_bw.container().debugChunks();
    } else {
        syntaxError("unsupported block name", &srcInfo2);
    }

    BlockStart bs = section->append<BlockStart>(srcInfo);
    bs.name() = blockName;
    bs.code() = m_bw.container().insts().end();

    bool endFound = false;
    do {
        switch(peek().kind()) {
        case EKWBlockStr: {
            eatToken(EKWBlockStr);
            BlockString str = section->append<BlockString>(sourceInfo(token()));
            str.string() = parseStringLiteral(m_scanner);
        } break;
        case EKWBlockNum: {
            eatToken(EKWBlockNum);
            BlockNumeric num = section->append<BlockNumeric>(sourceInfo(token()));

            num.type() = eatToken(EMType);
            SourceInfo const typeSI = sourceInfo(token());          

            switch(num.type())
            {
            case Brig::BRIG_TYPE_ROIMG:
            case Brig::BRIG_TYPE_RWIMG:
            case Brig::BRIG_TYPE_WOIMG:
            case Brig::BRIG_TYPE_SAMP:
                // this is to avoid parsing values (see below) as opaque types
                syntaxError("invalid blocknumeric type");
            }

            ArbitraryData values;
            num.elementCount() = parseValueList(num.type(), values);
            num.data() = values.toSRef();
        } break;
        case EKWBlockEnd: {
            eatToken(EKWBlockEnd);
            section->append<BlockEnd>(sourceInfo(token()));
            endFound = true;
        } break;
        default:
            syntaxError("unexpected token inside block", peek());
        }
        eatToken(ESemi);
    } while (!endFound);
}

// Instruction parsing

Inst Parser::parseInst()
{
    PDBG;
    Inst res;
    switch(peek().kind()) {
    case EInstruction: {
            SourceInfo const srcInfo = sourceInfo(peek());
            res = parseMnemo(m_scanner, m_bw);

            const char* errMsg = preValidateInst(res, m_bw.getMachineModel(), m_bw.getProfile());
            if (errMsg) syntaxError(errMsg);

            res.annotate(srcInfo);
            {
                if (res.kind()!=Brig::BRIG_INST_NONE) {
                    OperandParser const parser = getOperandParser(res.opcode());
                    assert(parser);
                    (this->*parser)(res);
                }
                eatToken(ESemi);
            }

        } break;
    case EInstruction_Vx: {
            Brig::BrigOpcode16_t const opCode = peek().brigId();
            switch(opCode) {
            case Brig::BRIG_OPCODE_LD:
            case Brig::BRIG_OPCODE_ST:
            case Brig::BRIG_OPCODE_GCNLD:
            case Brig::BRIG_OPCODE_GCNST:
                res = parseInstLdSt();
            break;

            case Brig::BRIG_OPCODE_COMBINE:
                res = parseInstCombineExpand(1);
            break;

            case Brig::BRIG_OPCODE_EXPAND:
                res = parseInstCombineExpand(0);
            break;

            case Brig::BRIG_OPCODE_ACTIVELANEMASK:
                res = parseInstLane();
            break;

            case Brig::BRIG_OPCODE_RDIMAGE:
            case Brig::BRIG_OPCODE_LDIMAGE:
            case Brig::BRIG_OPCODE_STIMAGE:
                res = parseInstImage();
            break;

            default: assert(false);
            };
        } break;
    default:;
    }
    if (!m_gcnEnabled && isGcnInst(res.opcode())) {
        syntaxError("Gcn extension isn't enabled");
    }
    return res;
}

void Parser::checkVxIsValid(int vx, Operand o)
{
    // check whether modifier v2/v3/v4 corresponds to 1st operand
    const SourceInfo* const srcInfo = o.srcInfo();

    switch (vx) {
    case 1: if ( isa<OperandVector>(o)) syntaxError("Unexpected vector operand (or missing _vX suffix)",srcInfo); break;
    case 2: if (!isa<OperandVector>(o) || OperandVector(o).elementCount() != 2) syntaxError("Expected a 2-element vector operand",srcInfo); break;
    case 3: if (!isa<OperandVector>(o) || OperandVector(o).elementCount() != 3) syntaxError("Expected a 3-element vector operand",srcInfo); break;
    case 4: if (!isa<OperandVector>(o) || OperandVector(o).elementCount() != 4) syntaxError("Expected a 4-element vector operand",srcInfo); break;
    default: assert(false); break;
    }
}

Inst Parser::parseInstLdSt()
{
    PDBG;
    SourceInfo const srcInfo = sourceInfo(peek());
    int vector=1;
    Inst inst = parseMnemoMem(m_scanner,m_bw,&vector);

    const char* errMsg = preValidateInst(inst, m_bw.getMachineModel(), m_bw.getProfile());
    if (errMsg) syntaxError(errMsg);

    inst.annotate(srcInfo);

    parseOperands(inst);
    eatToken(ESemi);

    // check whether modifier v2/v3/v4 corresponds to 1st operand
    checkVxIsValid(vector,inst.operand(0));

    return inst;
}

Inst Parser::parseInstLane()
{
    PDBG;
    SourceInfo const srcInfo = sourceInfo(peek());
    int vector=1;
    Inst inst = parseMnemoLane(m_scanner, m_bw, &vector);

    const char* errMsg = preValidateInst(inst, m_bw.getMachineModel(), m_bw.getProfile());
    if (errMsg) syntaxError(errMsg);

    inst.annotate(srcInfo);

    parseOperands(inst);
    eatToken(ESemi);

    // check whether modifier v2/v3/v4 corresponds to 1st operand
    checkVxIsValid(vector,inst.operand(0));

    return inst;
}

Inst Parser::parseInstCombineExpand(unsigned operandIdx)
{
    PDBG;
    SourceInfo const srcInfo = sourceInfo(peek());
    int vector=1;
    Inst inst = parseMnemoSourceType(m_scanner,m_bw,&vector);

    const char* errMsg = preValidateInst(inst, m_bw.getMachineModel(), m_bw.getProfile());
    if (errMsg) syntaxError(errMsg);

    inst.annotate(srcInfo);

    parseOperands(inst);
    eatToken(ESemi);

    if (vector != 2 && vector != 4) syntaxError("Expected v2 or v4 modifier",&srcInfo);

    // check whether modifier v2/v3/v4 corresponds to 1st operand
    checkVxIsValid(vector,inst.operand(operandIdx));

    return inst;
}

Inst Parser::parseInstImage()
{
    PDBG;
    SourceInfo const srcInfo = sourceInfo(peek());
    int vector=1;
    Inst inst = parseMnemoImage(m_scanner,m_bw,&vector);

    const char* errMsg = preValidateInst(inst, m_bw.getMachineModel(), m_bw.getProfile());
    if (errMsg) syntaxError(errMsg);

    inst.annotate(srcInfo);

    parseOperands(inst);
    eatToken(ESemi);

    if (vector == 2 || vector == 3) syntaxError("Modifiers v2 and v3 are not supported",&srcInfo);

    // check whether modifier v2/v3/v4 corresponds to 1st operand
    checkVxIsValid(vector,inst.operand(0));

    return inst;
}

// Operand parsing

inline static bool isIntegerConstant(ETokens t) {
    return (t==EDecimalNumber || t==EOctalNumber || t==EHexNumber);
}

void Parser::parseOperands(Inst inst)
{
    PDBG;
    if (peek().kind()!=ESemi) {
        int i=getOperandsNum(inst);
        if (i==5) return;
        do {
            parseOperandGeneric(inst,i++);
        } while(tryEatToken(EComma) && i < 5);
    }
}

void Parser::parseLdcOperands(Inst inst)
{
    PDBG;
    parseOperandGeneric(inst, 0);
    eatToken(EComma);

    if (peek().kind()==EIDStatic) { // this should be a function ref
        // TBD are you sure only functions are allowed here?
        m_bw.setOperand(inst,1,parseFunctionRef());
    } else {
        eatToken(ELabel);
        m_bw.setOperand(inst,1,parseLabelOperand());
    }
}

Operand Parser::parseActualParamList()
{
    PDBG;
    eatToken(ELParen);
    SourceInfo const srcInfo = sourceInfo(token());

	OperandArgumentList list = m_bw.createArgList(&srcInfo);

	while(!tryEatToken(ERParen)) {
        eatToken(EIDLocal,"unexpected argument");
        SRef const argName = token().text();
        SourceInfo const srcInfo = sourceInfo(token());

        DirectiveVariable arg = m_bw.findInScopes<DirectiveVariable>(argName);
		if (!arg) {
			syntaxError("Symbol not found", &srcInfo);
		}
		list.elements().push_back(arg);

        tryEatToken(EComma);
    }
	return list;
}

void Parser::parseCallOperands(Inst inst)
{
    PDBG;
    Operand target;
    bool isIndirectCall = false;
    switch(peek().kind()) {
    case ERegister:
        isIndirectCall = true;
        target = parseOperandReg();
        break;
    case EIDStatic:
        target = parseFunctionRef();
        break;
    default: syntaxError("invalid call target", peek());
    }

    m_bw.setOperand(inst,1,target);

    Operand outArgs;
    Operand inArgs;

    if (peek().kind()==ELParen) {
        outArgs = parseActualParamList();
        if (peek().kind()==ELParen) {
           inArgs = parseActualParamList();
        } else { // If there is only one argument list - it is input argument list
            inArgs = outArgs;
            outArgs = m_bw.createArgList();
        }
    } else { // No arguments specified
        syntaxError("missing call argument list",peek());
    }

    m_bw.setOperand(inst,0,outArgs);
    m_bw.setOperand(inst,2,inArgs);

    if (isIndirectCall && peek().kind()!=ESemi) {
        if (tryEatToken(ELBrace)) {
            SourceInfo const srcInfo = sourceInfo(token());
            OperandFunctionList list = m_bw.createFuncList(&srcInfo);
            do {
                SRef const fnName = scan().text();
                DirectiveFunction func = m_bw.findInScopes<DirectiveFunction>(fnName);
                if (!func) {
                    syntaxError("function not found");
                }
                if (!list.elements().push_back(func)) {
                    syntaxError("OperandFunctionList overflow");
                }
            } while (tryEatToken(EComma));
            eatToken(ERBrace);
            m_bw.setOperand(inst,3,list);
        } else {
            m_bw.setOperand(inst,3,parseSigRef());
        }
    }
}

void Parser::parseNoOperands(Inst )
{
}

void Parser::parseOperandGeneric(Inst inst, unsigned opndIdx)
{
    unsigned const reqType = getOperandType(inst, opndIdx, m_bw.getMachineModel(), m_bw.getProfile());
    m_bw.setOperand(inst,opndIdx,parseOperandGeneric(reqType));
}

Operand Parser::parseOperandGeneric(unsigned requiredType)
{
    PDBG;
    Operand res;
    switch (peek().kind()) {
    case ELBrace:
        res = parseOperandInBraces();
        break;

    case ELParen: // see mov instruction
        res = parseOperandVector(requiredType);
        break;

    case EPlus:
    case EMinus:
    case EDecimalNumber:
    case EOctalNumber:
    case EHexNumber:
    case EHlfHexNumber:
    case ESglHexNumber:
    case EDblHexNumber:
    case EHlfNumber:
    case ESglNumber:
    case EDblNumber:
    case EHlfC99Number:
    case ESglC99Number:
    case EDblC99Number:
    case EPackedLiteral:
        res = parseConstantGeneric(requiredType);
        break;

    case EWaveSizeMacro:
        {
            scan();
            SourceInfo const srcInfo = sourceInfo(token());
            res = m_bw.createWaveSz(&srcInfo);
        }       
        break;

    case EIDLocal:
    case EIDStatic:
        res = parseOperandRef();
        break;

    case ELabel:
        res = parseLabelOperand();
        break;

    case ERegister:
        res = parseOperandReg();
        break;

    default:
        syntaxError("invalid operand", peek());
    }
    return res;
}

OperandRef Parser::parseOperandRef()
{
    PDBG;
    scan();
    assert(token().kind()==EIDLocal || token().kind()==EIDStatic);
    SourceInfo const srcInfo = sourceInfo(token());
    SRef const name = token().text();
    return m_bw.createDirectiveRef(name,&srcInfo);
}

OperandReg Parser::parseOperandReg()
{
    PDBG;
    eatToken(ERegister);
    SourceInfo const srcInfo = sourceInfo(token());
    SRef const name = token().text();

    return m_bw.createOperandReg(name,&srcInfo);
}

Operand Parser::parseOperandVector(unsigned requiredType)
{
    eatToken(ELParen);
    SourceInfo const srcInfo = sourceInfo(token());

    ItemRange<Operand> opnds;
    while (true) {
        Operand o = parseOperandGeneric(requiredType);
        if (!isa<OperandReg>(o) && !isa<OperandImmed>(o) && !isa<OperandWavesize>(o)) {
            syntaxError("register, wavesize or immediate constant value expected");
        }      
        opnds.push_back(o);

        if (!tryEatToken(EComma)) {
            break;
        } else if (opnds.size()==4) {
            syntaxError("vector cannot contain more than 4 elements");
        }
    }
    eatToken(ERParen);

    OperandVector res = m_bw.createOperandVector(&srcInfo);
    copy(res.operand(), opnds);
    return res;
}

OperandFunctionRef Parser::parseFunctionRef()
{
    eatToken(EIDStatic);
    SRef const fnName = token().text();
    SourceInfo const srcInfo = sourceInfo(token());
    return m_bw.createFuncRef(fnName,&srcInfo);
}

Operand Parser::parseSigRef()
{
    eatToken(EIDStatic);
    SRef const sigName = token().text();
    SourceInfo const srcInfo = sourceInfo(token());
    return m_bw.createSigRef(sigName,&srcInfo);
}

Operand Parser::parseLabelOperand()
{
    eatToken(ELabel);
    SRef const labelName = token().text();
    SourceInfo const srcInfo = sourceInfo(token());

    return m_bw.createLabelRef(labelName,&srcInfo);
}

class ReadAndSetImmediate
{
    Scanner&     m_scanner;
    OperandImmed m_operand;
    ReadAndSetImmediate& operator=(const ReadAndSetImmediate&);
public:
    ReadAndSetImmediate(Scanner& scanner, OperandImmed operand) : m_operand(operand), m_scanner(scanner) {}
    template <typename ReqBrigType> void visit() {
        setImmed(m_operand,readValueIncludingPacked<ReqBrigType,ConvertImmediate>(m_scanner));
    }
    void visitNone(unsigned type) const {
        SrcLoc const srcLoc = m_scanner.peek().srcLoc();
        switch(type) {
        case Brig::BRIG_TYPE_INVALID: m_scanner.syntaxError("malformed instruction", srcLoc); break;
        case Brig::BRIG_TYPE_NONE:    m_scanner.syntaxError("unexpected operand", srcLoc); break;
        default:                      m_scanner.syntaxError("unexpected operand type", srcLoc); break;
        }
    }
};

Operand Parser::parseConstantGeneric(unsigned requiredType)
{
    PDBG;
    SourceInfo const srcInfo = sourceInfo(peek());
    OperandImmed operand = m_bw.createImmed(&srcInfo);
    ReadAndSetImmediate readAndSetImmediate(m_scanner,operand);
    dispatchByType(requiredType,readAndSetImmediate);
    return operand;
}

void Parser::parseAddress(SRef& reg, int64_t& offset)
{
    if (tryEatToken(ERegister)) {
        reg = token().text();
        ETokens const tokenAhead = peek().kind();
        if (tokenAhead==EPlus || tokenAhead==EMinus) {
            scan();
            uint64_t value = readInt<Brig::BRIG_TYPE_U64>(m_scanner);
            if (tokenAhead==EMinus) {
                offset = -(int64_t)value;
            } else {
                offset = (int64_t)value;
            }
        } else {
            offset = 0;
        }
    } else if (isIntegerConstant(peek().kind())) {
        reg = SRef();
        offset = readInt<Brig::BRIG_TYPE_S64>(m_scanner);
    } else {
        syntaxError("Register or offset expected", peek());
    }
}

Operand Parser::parseObjectOperand()
{
    PDBG;
    eatToken(ELBrace);
    SourceInfo const srcInfo = sourceInfo(token());

    scan();
    if (token().kind()!=EIDStatic && token().kind()!=EIDLocal) {
        syntaxError("symbol expected");
    }

    SRef const objName = token().text();
    SRef reg;
    int64_t offset = 0;
    if (tryEatToken(ELAngle)) {
        parseAddress(reg, offset);
        eatToken(ERAngle);
    }
	eatToken(ERBrace);
	return m_bw.createRef(objName,reg,offset,&srcInfo);
}

Operand Parser::parseOperandInBraces()
{
    PDBG;
    eatToken(ELBrace);
    SourceInfo const srcInfo = sourceInfo(token());

    ETokens const tokenAhead = peek().kind();

    if (tokenAhead==ERBrace) syntaxError("Invalid operand", peek()); // This is to avoid incorrect diagnostics

    if (tokenAhead==ELabel) {
        Operand res = parseLabelOperand();
        eatToken(ERBrace);
        return res;
    }

    SRef      name,reg;
    int64_t   offset = 0;
    if (tokenAhead==EIDStatic || tokenAhead==EIDLocal) {
        name = scan().text();
        eatToken(ERBrace);
        if (tryEatToken(ELBrace)) {
            parseAddress(reg, offset);
            eatToken(ERBrace);
        }
    } else {
        parseAddress(reg, offset);
        eatToken(ERBrace);
    }
    return m_bw.createRef(name, reg, offset, &srcInfo);
}



// Stubs


void Parser::parsePragma()
{
    PDBG;
    eatToken(EKWPragma);
    SourceInfo const srcInfo = sourceInfo(token());

    std::string const name = parseStringLiteral(m_scanner);
    eatToken(ESemi);

    DirectivePragma pgm = m_bw.createCodeRefDir<DirectivePragma>(&srcInfo);
    pgm.name() = name;
}

void Parser::parseEmbeddedText()
{
    PDBG;
    eatToken(EEmbeddedText);
    SourceInfo const srcInfo = sourceInfo(token());

    SRef name = token().text();
    // HACK: strip delimeters
    name.begin += 2;
    name.end -= 2;

    DirectivePragma pgm = m_bw.createCodeRefDir<DirectivePragma>(&srcInfo);
    pgm.name() = name;
}

void Parser::parseDebug()
{
    PDBG;
    eatToken(EKWSection);
    eatToken(ESemi);
}

void Parser::parseExtension()
{
    PDBG;
    eatToken(EKWExtension);
    SourceInfo const srcInfo = sourceInfo(token());
    std::string const name = parseStringLiteral(m_scanner);
    eatToken(ESemi);

    DirectiveExtension dir = m_bw.createCodeRefDir<DirectiveExtension>(&srcInfo);
    dir.name() = name;

    if (name=="amd:gcn") {
        m_gcnEnabled = true;
    }
}

void Parser::parseLocation()
{
    PDBG;
    eatToken(EKWLoc);
    SourceInfo const srcInfo = sourceInfo(token());

    unsigned const sourceLine = readPositiveInt<Brig::BRIG_TYPE_U32>(m_scanner);
    unsigned sourceColumn = 1;
    if (isIntegerConstant(peek().kind())) {
        sourceColumn = readPositiveInt<Brig::BRIG_TYPE_U32>(m_scanner);
    }
    if (peek().kind() == EQuot) {
      m_srcFileName = parseStringLiteral(m_scanner);
    }
    eatToken(ESemi);

    DirectiveLoc loc = m_bw.createCodeRefDir<DirectiveLoc>(&srcInfo);
    // \todo: add to Brigantine high-level API for adding DirectiveLoc
    loc.filename() = m_srcFileName;
    loc.line() = sourceLine;
    loc.column() = sourceColumn;
}

void Parser::parseControl()
{
    PDBG;
    unsigned const ctrlId = eatToken(EControl);
    DirectiveControl ctrl = m_bw.container().append<DirectiveControl>(sourceInfo(token()));
    ctrl.code()    = m_bw.container().insts().end();
    ctrl.control() = ctrlId;
    ctrl.type()    = Brig::BRIG_TYPE_U32; // Should actually be NONE for directives wo args

    ControlValues values = ctrl.values();
    if (peek().kind() != ESemi) do { // arguments are optional

        // dp: added WAVESIZE support + disabled negative int values
        SourceInfo const srcInfo = sourceInfo(peek());
        
        Operand res;       
        if (tryEatToken(EWaveSizeMacro)) {
            res = m_bw.createWaveSz(&srcInfo);
        } else {
            res = m_bw.createImmed(&srcInfo);
            uint32_t val = readNonNegativeInt<Brig::BRIG_TYPE_U32>(m_scanner);
            setImmed(res, val, Brig::BRIG_TYPE_U32);
        }
        values.push_back( res );

//      values.push_back( parseConstantGeneric(Brig::BRIG_TYPE_U32) ); // dp
    } while (tryEatToken(EComma));
    eatToken(ESemi);
}

// ----------------------------------------------------------------------------
// Helper code

static std::string parseStringLiteral(Scanner& scanner) {
    std::string ret;
    do {
        scanner.eatToken(EQuot);
        scanner.readSingleStringLiteral(ret);
        scanner.eatToken(EQuot);
    } while(scanner.peek().kind() == EQuot);
    return ret;
}

// Check if expected and actual types of immediate value are compatible.
// Note that actual = b64 for any integer literal.
/*
void Parser::validateImmType(unsigned expected, unsigned actual)
{
    if ((isIntType(expected) && actual == Brig::BRIG_TYPE_B64)
    ||  (isIntType(expected) && actual == Brig::BRIG_TYPE_B32)
    ||  (expected == Brig::BRIG_TYPE_B64 && actual == Brig::BRIG_TYPE_F64)
    ||  (expected == Brig::BRIG_TYPE_B32 && actual == Brig::BRIG_TYPE_F32)
    ||  (isFloatType(expected) && isFloatType(actual))
    ||  (isIntPackedType(expected) && (actual == Brig::BRIG_TYPE_B64 || expected == actual))
    ||  (isFloatPackedType(expected) && expected == actual)) return;

    // There is no information if operand may be an immediate,
    // so error message must be generic
    syntaxError("Invalid operand type");
}
*/


#include "HSAILParserUtilities_gen.hpp"

} // end namespace
