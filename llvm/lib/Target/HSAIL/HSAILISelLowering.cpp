//===-- HSAILISelLowering.cpp - HSAIL DAG Lowering Implementation ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that HSAIL uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "hsail-isel"
#include "HSAIL.h"
#include "HSAILInstrInfo.h"
#include "HSAILCOFFObjectFile.h"
#include "HSAILELFTargetObjectFile.h"
#include "HSAILISelLowering.h"
#include "HSAILMachoTargetObjectFile.h"
#include "HSAILMachineFunctionInfo.h"
#include "HSAILSubtarget.h"
#include "HSAILTargetMachine.h"
#include "HSAILUtilityFunctions.h"
#include "HSAILOpaqueTypes.h"
#include "llvm/CallingConv.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/GlobalAlias.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Intrinsics.h"
#include "llvm/LLVMContext.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "libHSAIL/Brig.h"
#include <sstream>
#include "HSAILGenInstrInfo.inc"

using namespace llvm;
using namespace dwarf;

namespace llvm {
  extern bool EnableExperimentalFeatures;
  extern enum OptimizeForTargetArch OptimizeFor;
}

static cl::opt<bool>
Flag_ampPtrFtos("mamp_ptr_ftos",
            cl::desc("Convert AMP incoming pointers to segment address"),
            cl::init(false));

static TargetLoweringObjectFile *createTLOF(HSAILTargetMachine &TM) {
  const HSAILSubtarget *Subtarget = &TM.getSubtarget<HSAILSubtarget>();
  bool is64Bit = Subtarget->is64Bit();

  if (Subtarget->isTargetEnvMacho()) {
    if (is64Bit)
      return new HSAIL64_MachoTargetObjectFile();
    return new TargetLoweringObjectFileMachO();
  }

  if (Subtarget->isTargetELF()) {
    if (is64Bit)
      return new HSAIL64_DwarfTargetObjectFile(TM);
    return new HSAIL32_DwarfTargetObjectFile(TM);
  }
  if (Subtarget->isTargetCOFF() && !Subtarget->isTargetEnvMacho())
    return new TargetLoweringObjectFileCOFF();
  llvm_unreachable("unknown subtarget type");
}


HSAILTargetLowering::HSAILTargetLowering(HSAILTargetMachine &TM)
  : TargetLowering(TM, createTLOF(TM))
{
  // HSAIL uses a -1 to store a Boolean value as an int. For example,
  // see the return values of the cmp instructions. This also requires
  // that we never use a cvt instruction for converting a Boolean to a
  // larger integer, because HSAIL cvt uses a zext when the source is
  // b1. Due to the setting below, LLVM will ensure that all such
  // conversions are done with the sext instruction.
  setBooleanContents(ZeroOrNegativeOneBooleanContent);

  Subtarget = &TM.getSubtarget<HSAILSubtarget>();

  RegInfo = TM.getRegisterInfo();
  DL = getDataLayout();

  // Set up the register classes.
  addRegisterClass(MVT::i32, &HSAIL::GPR32RegClass);
  addRegisterClass(MVT::i64, &HSAIL::GPR64RegClass);
  addRegisterClass(MVT::f32, &HSAIL::GPR32RegClass);
  addRegisterClass(MVT::f64, &HSAIL::GPR64RegClass);
  addRegisterClass(MVT::i1,  &HSAIL::CRRegClass);

  setOperationAction(ISD::ATOMIC_LOAD, MVT::i32, Custom);
  setOperationAction(ISD::ATOMIC_LOAD, MVT::i64, Custom);
  setOperationAction(ISD::ATOMIC_LOAD, MVT::f32, Custom);
  setOperationAction(ISD::ATOMIC_LOAD, MVT::f64, Custom);
  setOperationAction(ISD::ATOMIC_STORE, MVT::i32, Custom);
  setOperationAction(ISD::ATOMIC_STORE, MVT::i64, Custom);
  setOperationAction(ISD::ATOMIC_STORE, MVT::f32, Custom);
  setOperationAction(ISD::ATOMIC_STORE, MVT::f64, Custom);
  setOperationAction(ISD::BSWAP, MVT::i16, Expand);
  setOperationAction(ISD::BSWAP, MVT::i32, Custom);
  setOperationAction(ISD::BSWAP, MVT::i64, Expand);
  setOperationAction(ISD::ADD, MVT::i1, Custom);
  setOperationAction(ISD::ROTL, MVT::i1, Expand);
  setOperationAction(ISD::ROTL, MVT::i8, Expand);
  setOperationAction(ISD::ROTL, MVT::i16, Expand);
  setOperationAction(ISD::ROTL, MVT::i32, Custom);
  setOperationAction(ISD::ROTL, MVT::i64, Expand);
  setOperationAction(ISD::ROTR, MVT::i1, Expand);
  setOperationAction(ISD::ROTR, MVT::i8, Expand);
  setOperationAction(ISD::ROTR, MVT::i16, Expand);
  setOperationAction(ISD::ROTR, MVT::i32, Custom);
  setOperationAction(ISD::ROTR, MVT::i64, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::Other, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i32, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i64, Expand);

  setOperationAction(ISD::TRUNCATE, MVT::i1, Custom);

  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::BR_CC, MVT::Other, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::Other, Expand);

  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
  setOperationAction(ISD::GlobalAddress, MVT::i64, Custom);

  setOperationAction(ISD::ConstantFP, MVT::f64, Legal);
  setOperationAction(ISD::ConstantFP, MVT::f32, Legal);
  setOperationAction(ISD::Constant, MVT::i32, Legal);
  setOperationAction(ISD::Constant, MVT::i64, Legal);

  setOperationAction(ISD::INTRINSIC_W_CHAIN, MVT::Other, Custom);

  setLoadExtAction(ISD::EXTLOAD, MVT::f32, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v2f32, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v4f32, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v8f32, Expand);

  setLoadExtAction(ISD::EXTLOAD, MVT::i32, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v1i32, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v2i32, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v4i32, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v8i32, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v16i32, Expand);

  setLoadExtAction(ISD::EXTLOAD, MVT::i16, Custom);
  setLoadExtAction(ISD::EXTLOAD, MVT::v1i16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v2i16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v4i16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v8i16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v16i16, Expand);

  setLoadExtAction(ISD::EXTLOAD, MVT::i8, Custom);
  setLoadExtAction(ISD::EXTLOAD, MVT::v2i8, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v4i8, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v8i8, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::v16i8, Expand);

  setLoadExtAction(ISD::ZEXTLOAD, MVT::i16, Custom);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v1i16, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v2i16, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v4i16, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v8i16, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v16i16, Expand);

  setLoadExtAction(ISD::ZEXTLOAD, MVT::i8, Custom);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v2i8, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v4i8, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v8i8, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v16i8, Expand);

  setLoadExtAction(ISD::ZEXTLOAD, MVT::i32, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v1i32, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v2i32, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v4i32, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v8i32, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v16i32, Expand);

  setLoadExtAction(ISD::SEXTLOAD, MVT::i32, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v1i32, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v2i32, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v4i32, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v8i32, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v16i32, Expand);

  setLoadExtAction(ISD::SEXTLOAD, MVT::i16, Custom);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v1i16, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v2i16, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v4i16, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v8i16, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v16i16, Expand);

  setLoadExtAction(ISD::SEXTLOAD, MVT::i8, Custom);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v2i8, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v4i8, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v8i8, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v16i8, Expand);

  setTruncStoreAction(MVT::f64, MVT::f32, Expand);
  setTruncStoreAction(MVT::v2f64, MVT::v2f32, Expand);
  setTruncStoreAction(MVT::v4f64, MVT::v4f32, Expand);
  setTruncStoreAction(MVT::i64, MVT::i32, Expand);
  setTruncStoreAction(MVT::i64, MVT::v1i32, Expand);
  setTruncStoreAction(MVT::i64, MVT::v2i32, Expand);
  setTruncStoreAction(MVT::i64, MVT::v4i32, Expand);
  setTruncStoreAction(MVT::i64, MVT::v8i32, Expand);
  setTruncStoreAction(MVT::i64, MVT::v16i32, Expand);
  setTruncStoreAction(MVT::i64, MVT::i16, Expand);
  setTruncStoreAction(MVT::i64, MVT::v1i16, Expand);
  setTruncStoreAction(MVT::i64, MVT::v2i16, Expand);
  setTruncStoreAction(MVT::i64, MVT::v4i16, Expand);
  setTruncStoreAction(MVT::i64, MVT::v8i16, Expand);
  setTruncStoreAction(MVT::i64, MVT::v16i16, Expand);
  setTruncStoreAction(MVT::i64, MVT::i8, Expand);
  setTruncStoreAction(MVT::i64, MVT::v2i8, Expand);
  setTruncStoreAction(MVT::i64, MVT::v4i8, Expand);
  setTruncStoreAction(MVT::i64, MVT::v8i8, Expand);
  setTruncStoreAction(MVT::i64, MVT::v16i8, Expand);

  setTargetDAGCombine(ISD::INTRINSIC_WO_CHAIN);

  setJumpIsExpensive(true);
  setSelectIsExpensive(true);
  setPow2DivIsCheap(false);
  setPrefLoopAlignment(0);
  setSchedulingPreference(Sched::ILP);
  setSupportJumpTables(false);
#ifdef _DEBUG
  const char* pScheduler = std::getenv("AMD_DEBUG_HSAIL_PRE_RA_SCHEDULER");
  if (pScheduler) {
    if (strcmp(pScheduler, "ilp") == 0) {
      printf("Overriding pre-RA scheduler with %s\n", pScheduler);
      setSchedulingPreference(Sched::ILP);
    } else if (strcmp(pScheduler, "regpressure") == 0) {
      printf("Overriding pre-RA scheduler with %s\n", pScheduler);
      setSchedulingPreference(Sched::RegPressure);
    } else if (strcmp(pScheduler, "hybrid") == 0) {
      printf("Overriding pre-RA scheduler with %s\n", pScheduler);
      setSchedulingPreference(Sched::Hybrid);
    }
  }
#endif

  computeRegisterProperties();
  maxStoresPerMemcpy  = 4096;
  maxStoresPerMemmove = 4096;
  maxStoresPerMemset  = 4096;
}

HSAILTargetLowering::~HSAILTargetLowering() {}

/// getSetCCResultType - Return the ValueType of the result of SETCC
/// operations.  Also used to obtain the target's preferred type for
/// the condition operand of SELECT and BRCOND nodes.  In the case of
/// BRCOND the argument passed is MVT::Other since there are no other
/// operands to get a type hint from.
EVT
HSAILTargetLowering::getSetCCResultType(LLVMContext &Context, EVT VT) const
{
  return MVT::i1;
}

/// getCmpLibcallReturnType - Return the ValueType for comparison
/// libcalls. Comparions libcalls include floating point comparion calls,
/// and Ordered/Unordered check calls on floating point numbers.
MVT::SimpleValueType
HSAILTargetLowering::getCmpLibcallReturnType() const
{
  assert(!"When do we hit this?");
  return MVT::SimpleValueType();
}
/// getSchedulingPreference - Some scheduler, e.g. hybrid, can switch to
/// different scheduling heuristics for different nodes. This function returns
/// the preference (or none) for the given node.
Sched::Preference
HSAILTargetLowering::getSchedulingPreference(SDNode *N) const
{
  return TargetLowering::getSchedulingPreference(N);
}
/// getRegClassFor - Return the register class that should be used for the
/// specified value type.
const TargetRegisterClass*
HSAILTargetLowering::getRegClassFor(EVT VT) const
{
  return TargetLowering::getRegClassFor(VT);
}
/// getRepRegClassFor - Return the 'representative' register class for the
/// specified value type. The 'representative' register class is the largest
/// legal super-reg register class for the register class of the value type.
/// For example, on i386 the rep register class for i8, i16, and i32 are GR32;
/// while the rep register class is GR64 on x86_64.
const TargetRegisterClass*
HSAILTargetLowering::getRepRegClassFor(EVT VT) const
{
  switch (VT.getSimpleVT().SimpleTy) {
    case MVT::i64:
    case MVT::f64:
      return &HSAIL::GPR64RegClass;
    case MVT::i8:
    case MVT::i16:
    case MVT::i32:
    case MVT::f32:
      return &HSAIL::GPR32RegClass;
    case MVT::i1:
      return &HSAIL::CRRegClass;
    default:
      assert(!"When do we hit this?");
      break;
  }
  return NULL;
}

/// getRepRegClassCostFor - Return the cost of the 'representative' register
/// class for the specified value type.
uint8_t
HSAILTargetLowering::getRepRegClassCostFor(EVT VT) const
{
  // Micah: Is this true that the reg class cost for everything is 1 in HSAIL?
  return 1;
}

/// getTgtMemIntrinsic: Given an intrinsic, checks if on the target the
/// intrinsic will need to map to a MemIntrinsicNode (touches memory). If
/// this is the case, it returns true and store the intrinsic
/// information into the IntrinsicInfo that was passed to the function.
bool
HSAILTargetLowering::getTgtMemIntrinsic(IntrinsicInfo &Info,
                                        const CallInst &I,
                                        unsigned Intrinsic) const
{
  return false;
}

/// isFPImmLegal - Returns true if the target can instruction select the
/// specified FP immediate natively. If false, the legalizer will materialize
/// the FP immediate as a load from a constant pool.
bool
HSAILTargetLowering::isFPImmLegal(const APFloat &Imm,
                                  EVT VT) const
{
  // All floating point types are legal for 32bit and 64bit types.
  return (VT == EVT(MVT::f32) || VT == EVT(MVT::f64));
}

/// isShuffleMaskLegal - Targets can use this to indicate that they only
/// support *some* VECTOR_SHUFFLE operations, those with specific masks.
/// By default, if a target supports the VECTOR_SHUFFLE node, all mask values
/// are assumed to be legal.
bool
HSAILTargetLowering::isShuffleMaskLegal(const SmallVectorImpl<int> &Mask,
                                        EVT VT) const
{
  assert(!"When do we hit this?");
  return false;
}

/// canOpTrap - Returns true if the operation can trap for the value type.
/// VT must be a legal type. By default, we optimistically assume most
/// operations don't trap except for divide and remainder.
bool
HSAILTargetLowering::canOpTrap(unsigned Op, EVT VT) const
{
  assert(!"When do we hit this?");
  return false;
}

/// isVectorClearMaskLegal - Similar to isShuffleMaskLegal. This is
/// used by Targets can use this to indicate if there is a suitable
/// VECTOR_SHUFFLE that can be used to replace a VAND with a constant
/// pool entry.
bool
HSAILTargetLowering::isVectorClearMaskLegal(const SmallVectorImpl<int> &Mask,
                                            EVT VT) const
{
  assert(!"When do we hit this?");
  return false;
}

/// getByValTypeAlignment - Return the desired alignment for ByVal aggregate
/// function arguments in the caller parameter area.  This is the actual
/// alignment, not its logarithm.
unsigned
HSAILTargetLowering::getByValTypeAlignment(Type *Ty) const
{
  return TargetLowering::getByValTypeAlignment(Ty);
}

/// ShouldShrinkFPConstant - If true, then instruction selection should
/// seek to shrink the FP constant of the specified type to a smaller type
/// in order to save space and / or reduce runtime.
bool
HSAILTargetLowering::ShouldShrinkFPConstant(EVT VT) const
{
  assert(!"When do we hit this?");
  return false;
}

/// This function returns true if the target allows unaligned memory accesses.
/// of the specified type. This is used, for example, in situations where an
/// array copy/move/set is  converted to a sequence of store operations. It's
/// use helps to ensure that such replacements don't generate code that causes
/// an alignment error  (trap) on the target machine.
/// @brief Determine if the target supports unaligned memory accesses.
bool
HSAILTargetLowering::allowsUnalignedMemoryAccesses(EVT VT) const
{
  return true;
}

/// getOptimalMemOpType - Returns the target specific optimal type for load
/// and store operations as a result of memset, memcpy, and memmove
/// lowering. If DstAlign is zero that means it's safe to destination
/// alignment can satisfy any constraint. Similarly if SrcAlign is zero it
/// means there isn't a need to check it against alignment requirement,
/// probably because the source does not need to be loaded. If
/// 'NonScalarIntSafe' is true, that means it's safe to return a
/// non-scalar-integer type, e.g. empty string source, constant, or loaded
/// from memory. 'MemcpyStrSrc' indicates whether the memcpy source is
/// constant so it does not need to be loaded.
/// It returns EVT::Other if the type should be determined using generic
/// target-independent logic.
EVT
HSAILTargetLowering::getOptimalMemOpType(uint64_t Size,
                                         unsigned DstAlign,
                                         unsigned SrcAlign,
                                         bool NonScalarIntSafe,
                                         bool MemcpyStrSrc,
                                         MachineFunction &MF) const
{
  return TargetLowering::getOptimalMemOpType(Size, DstAlign, SrcAlign,
      NonScalarIntSafe, MemcpyStrSrc, MF);
}

/// getPreIndexedAddressParts - returns true by value, base pointer and
/// offset pointer and addressing mode by reference if the node's address
/// can be legally represented as pre-indexed load / store address.
bool
HSAILTargetLowering::getPreIndexedAddressParts(SDNode *N,
                                               SDValue &Base,
                                               SDValue &Offset,
                                               ISD::MemIndexedMode &AM,
                                               SelectionDAG &DAG) const
{
  assert(!"When do we hit this?");
  return false;
}

/// getPostIndexedAddressParts - returns true by value, base pointer and
/// offset pointer and addressing mode by reference if this node can be
/// combined with a load / store to form a post-indexed load / store.
bool
HSAILTargetLowering::getPostIndexedAddressParts(SDNode *N,
                                                SDNode *Op,
                                                SDValue &Base,
                                                SDValue &Offset,
                                                ISD::MemIndexedMode &AM,
                                                SelectionDAG &DAG) const
{
  assert(!"When do we hit this?");
  return false;
}

/// getJumpTableEncoding - Return the entry encoding for a jump table in the
/// current function.  The returned value is a member of the
/// MachineJumpTableInfo::JTEntryKind enum.
unsigned
HSAILTargetLowering::getJumpTableEncoding() const
{
    return MachineJumpTableInfo::EK_BlockAddress;
}

const MCExpr*
HSAILTargetLowering::LowerCustomJumpTableEntry(const MachineJumpTableInfo *MJTI,
                                               const MachineBasicBlock *MBB,
                                               unsigned uid,
                                               MCContext &Ctx) const
{
  assert(!"When do we hit this?");
  return NULL;
}

/// getPICJumpTableRelocaBase - Returns relocation base for the given PIC
/// jumptable.
SDValue
HSAILTargetLowering::getPICJumpTableRelocBase(SDValue Table,
                                              SelectionDAG &DAG) const
{
  assert(!"When do we hit this?");
  return SDValue();
}

/// getPICJumpTableRelocBaseExpr - This returns the relocation base for the
/// given PIC jumptable, the same as getPICJumpTableRelocBase, but as an
/// MCExpr.
const MCExpr*
HSAILTargetLowering::getPICJumpTableRelocBaseExpr(const MachineFunction *MF,
                                                  unsigned JTI,
                                                  MCContext &Ctx) const
{
  assert(!"When do we hit this?");
  return NULL;
}

/// isOffsetFoldingLegal - Return true if folding a constant offset
/// with the given GlobalAddress is legal.  It is frequently not legal in
/// PIC relocation models.
bool
HSAILTargetLowering::isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const
{
  // Micah: Since HSAIL does not support PIC now, we can always set this to true.
  return true;
}

/// getFunctionAlignment - Return the Log2 alignment of this function.
unsigned
HSAILTargetLowering::getFunctionAlignment(const Function *) const
{
  return 0;
}

/// getStackCookieLocation - Return true if the target stores stack
/// protector cookies at a fixed offset in some non-standard address
/// space, and populates the address space and offset as
/// appropriate.
bool
HSAILTargetLowering::getStackCookieLocation(unsigned &AddressSpace,
                                            unsigned &Offset) const
{
  assert(!"When do we hit this?");
  return false;
}

/// getMaximalGlobalOffset - Returns the maximal possible offset which can be
/// used for loads / stores from the global.
unsigned
HSAILTargetLowering::getMaximalGlobalOffset() const
{
  assert(!"When do we hit this?");
  return 0;
}

/// computeMaskedBitsForTargetNode - Determine which of the bits specified in
/// Mask are known to be either zero or one and return them in the
/// KnownZero/KnownOne bitsets.
void
HSAILTargetLowering::computeMaskedBitsForTargetNode(const SDValue Op,
                                                    const APInt &Mask,
                                                    APInt &KnownZero,
                                                    APInt &KnownOne,
                                                    const SelectionDAG &DAG,
                                                    unsigned Depth) const
{
  return TargetLowering::computeMaskedBitsForTargetNode(Op,
       KnownZero, KnownOne, DAG, Depth);
}

/// ComputeNumSignBitsForTargetNode - This method can be implemented by
/// targets that want to expose additional information about sign bits to the
/// DAG Combiner.
unsigned
HSAILTargetLowering::ComputeNumSignBitsForTargetNode(SDValue Op,
                                                     unsigned Depth) const
{
  return 1;
}

/// isGAPlusOffset - Returns true (and the GlobalValue and the offset) if the
/// node is a GlobalAddress + offset.
bool
HSAILTargetLowering::isGAPlusOffset(SDNode *N,
                                    const GlobalValue* &GA,
                                    int64_t &Offset) const
{
  bool res = TargetLowering::isGAPlusOffset(N, GA, Offset);
  return res;
}

static SDValue
PerformBitalignCombine(SDNode *N, TargetLowering::DAGCombinerInfo &DCI, unsigned IID)
{
  assert(IID == HSAILIntrinsic::HSAIL_bitalign_b32 ||
         IID == HSAILIntrinsic::HSAIL_bytealign_b32);
  SDValue Opr0 = N->getOperand(1);
  SDValue Opr1 = N->getOperand(2);
  SDValue Opr2 = N->getOperand(3);
  ConstantSDNode *SHR = dyn_cast<ConstantSDNode>(Opr2);
  SelectionDAG &DAG = DCI.DAG;
  DebugLoc dl = N->getDebugLoc();
  EVT VT = N->getValueType(0);
  // fold bitalign_b32(x & c1, x & c1, c2) -> bitalign_b32(x, x, c2) & rotr(c1, c2)
  if (SHR && (Opr0 == Opr1) && (Opr0.getOpcode() == ISD::AND)) {
    if (ConstantSDNode *AndMask = dyn_cast<ConstantSDNode>(Opr0.getOperand(1))) {
      uint64_t and_mask = AndMask->getZExtValue();
      uint64_t shr_val = SHR->getZExtValue() & 31U;
      if (IID == HSAILIntrinsic::HSAIL_bytealign_b32) 
        shr_val = (shr_val & 3U) << 3U;
      and_mask = ((and_mask >> shr_val) | (and_mask << (32U - shr_val))) & 0xffffffffu;
      Opr0 = Opr0->getOperand(0);
      return DAG.getNode(ISD::AND, Opr1.getDebugLoc(), Opr1.getValueType(),
        DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, VT,
          DAG.getConstant(IID, MVT::i32), Opr0, Opr0, Opr2),
        DAG.getConstant(and_mask, MVT::i32));
    }
  }
  // fold bitalign_b32(x, y, c) -> bytealign_b32(x, y, c/8) if c & 7 == 0
  if (SHR && (IID == HSAILIntrinsic::HSAIL_bitalign_b32)) {
      uint64_t shr_val = SHR->getZExtValue() & 31U;
      if ((shr_val & 7U) == 0)
        return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, VT,
          DAG.getConstant(HSAILIntrinsic::HSAIL_bytealign_b32, MVT::i32),
          Opr0, Opr1, DAG.getConstant(shr_val >> 3U, MVT::i32));
  }
  return SDValue();
}

static SDValue
PerformIntrinsic_Wo_ChainCombine(SDNode *N, TargetLowering::DAGCombinerInfo &DCI)
{
  assert(N->getOpcode() == ISD::INTRINSIC_WO_CHAIN);
  unsigned IID = cast<ConstantSDNode>(N->getOperand(0))->getZExtValue();
  switch (IID) {
  case HSAILIntrinsic::HSAIL_bitalign_b32: // fall-through
  case HSAILIntrinsic::HSAIL_bytealign_b32:
    return PerformBitalignCombine(N, DCI, IID);
  }
  return SDValue();
}

/// PerformDAGCombine - This method will be invoked for all target nodes and
/// for any target-independent nodes that the target has registered with
/// invoke it for.
///
/// The semantics are as follows:
/// Return Value:
///   SDValue.Val == 0   - No change was made
///   SDValue.Val == N   - N was replaced, is dead, and is already handled.
///   otherwise          - N should be replaced by the returned Operand.
///
/// In addition, methods provided by DAGCombinerInfo may be used to perform
/// more complex transformations.
SDValue
HSAILTargetLowering::PerformDAGCombine(SDNode *N,
                                       DAGCombinerInfo &DCI) const
{
  switch (N->getOpcode()) {
  case ISD::INTRINSIC_WO_CHAIN: return PerformIntrinsic_Wo_ChainCombine(N, DCI);
  default: break;
  }

  return SDValue();
}

/// isTypeDesirableForOp - Return true if the target has native support for
/// the specified value type and it is 'desirable' to use the type for the
/// given node type. e.g. On x86 i16 is legal, but undesirable since i16
/// instruction encodings are longer and some i16 instructions are slow.
bool
HSAILTargetLowering::isTypeDesirableForOp(unsigned Opc, EVT VT) const
{
  return TargetLowering::isTypeDesirableForOp(Opc, VT);
}

/// isDesirableToPromoteOp - Return true if it is profitable for dag combiner
/// to transform a floating point op of specified opcode to a equivalent op of
/// an integer type. e.g. f32 load -> i32 load can be profitable on ARM.
bool
HSAILTargetLowering::isDesirableToTransformToIntegerOp(unsigned Opc,
                                                       EVT VT) const
{
    return (Opc == ISD::LOAD || Opc == ISD::STORE)
        && (VT.getSimpleVT() == MVT::f32
        || VT.getSimpleVT() == MVT::f64);
}

/// IsDesirableToPromoteOp - This method query the target whether it is
/// beneficial for dag combiner to promote the specified node. If true, it
/// should return the desired promotion type by reference.
bool
HSAILTargetLowering::IsDesirableToPromoteOp(SDValue Op, EVT &PVT) const
{
  return TargetLowering::IsDesirableToPromoteOp(Op, PVT);
}

/// findRepresentativeClass - Return the largest legal super-reg register class
/// of the register class for the specified type and its associated "cost".
std::pair<const TargetRegisterClass*, uint8_t>
HSAILTargetLowering::findRepresentativeClass(EVT VT) const
{
  return TargetLowering::findRepresentativeClass(VT);
}

//===--------------------------------------------------------------------===//
// Lowering methods - These methods must be implemented by targets so that
// the SelectionDAGLowering code knows how to lower these.
//

//===----------------------------------------------------------------------===//
//               Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "HSAILGenCallingConv.inc"

/*

/// CanLowerReturn - This hook should be implemented to check whether the
/// return values described by the Outs array can fit into the return
/// registers.  If false is returned, an sret-demotion is performed.

bool
HSAILTargetLowering::CanLowerReturn(CallingConv::ID CallConv,
                                    bool isVarArg,
                                    const SmallVectorImpl<ISD::OutputArg> &Outs,
                                    LLVMContext &Context) const
{
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, getTargetMachine(),
                 RVLocs, Context);
  return CCInfo.CheckReturn(Outs, RetCC_HSAIL);
}

*/

/// LowerReturn - This hook must be implemented to lower outgoing
/// return values, described by the Outs array, into the specified
/// DAG. The implementation should return the resulting token chain
/// value.
SDValue
HSAILTargetLowering::LowerReturn(SDValue Chain,
                                 CallingConv::ID CallConv,
                                 bool isVarArg,
                                 const SmallVectorImpl<ISD::OutputArg> &Outs,
                                 const SmallVectorImpl<SDValue> &OutVals,
                                 DebugLoc dl,
                                 SelectionDAG &DAG) const
{
  MachineFunction &MF = DAG.getMachineFunction();
  HSAILMachineFunctionInfo *FuncInfo = MF.getInfo<HSAILMachineFunctionInfo>();
  const FunctionType *funcType = MF.getFunction()->getFunctionType();
  bool isKernel = isKernelFunc(MF.getFunction());

  SDValue Flag;

  SmallVector<SDValue, 6> RetOps;
  RetOps.push_back(Chain); // Operand #0 = Chain (updated below)
  // Operand #1 = Bytes To Pop
  RetOps.push_back(DAG.getTargetConstant(FuncInfo->getBytesToPopOnReturn(),
                                         MVT::i16));

  const Type * type  = funcType->getReturnType();
  if(type->getTypeID() != Type::VoidTyID) {
    EVT VT = Outs[0].VT;
    SDValue retVariable =  DAG.getRegister(HSAIL::R0, VT);
    MF.addLiveIn(HSAIL::R0, &HSAIL::RETREGRegClass);
    
    // Copy the result values into the output registers.
    
    const Type* sType = type->getScalarType();
    unsigned RetOpcode = HSAILISD::ST_SCALAR_RET;
    if (sType->isIntegerTy(8)) {
      RetOpcode = HSAILISD::ST_SCALAR_RET_U8;
    } else if (sType->isIntegerTy(16)) {
      RetOpcode = HSAILISD::ST_SCALAR_RET_U16;
    }
    unsigned offset = sType->getPrimitiveSizeInBits() / 8;
    const VectorType *VecVT = dyn_cast<VectorType>(type);
    unsigned num_elems = VecVT ? VecVT->getNumElements() : 1;
    for (unsigned i = 0; i < num_elems; i++) {
      SDValue ValToCopy = OutVals[i];
      SDValue Ops[] = { Chain, ValToCopy, retVariable,  DAG.getConstant(i * offset, MVT::i32)};
      Chain = DAG.getNode(RetOpcode, dl, MVT::Other, Ops, 4);
    }
  }

  RetOps[0] = Chain;  // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(HSAILISD::RET_FLAG, dl,
                     MVT::Other, &RetOps[0], RetOps.size());
}

/// isUsedByReturnOnly - Return true if result of the specified node is used
/// by a return node only. This is used to determine whether it is possible
/// to codegen a libcall as tail call at legalization time.
bool
HSAILTargetLowering::isUsedByReturnOnly(SDNode *N) const
{
  assert(!"When do we hit this?");
  return false;
}

//===----------------------------------------------------------------------===//
//                C & StdCall & Fast Calling Convention implementation
//===----------------------------------------------------------------------===//

SDValue
HSAILTargetLowering::LowerMemArgument(SDValue Chain,
                                      CallingConv::ID CallConv,
                                      const SmallVectorImpl<ISD::InputArg> &Ins,
                                      DebugLoc dl, SelectionDAG &DAG,
                                      const CCValAssign &VA,
                                      MachineFrameInfo *MFI,
                                      unsigned i) const
{
  // Create the nodes corresponding to a load from this parameter slot.
  ISD::ArgFlagsTy Flags = Ins[i].Flags;

  bool AlwaysUseMutable = (CallConv==CallingConv::Fast) && getTargetMachine().Options.GuaranteedTailCallOpt;
  bool isImmutable = !AlwaysUseMutable && !Flags.isByVal();

  // In case of tail call optimization mark all arguments mutable. Since they
  // could be overwritten by lowering of arguments in case of a tail call.
  int FI = MFI->CreateFixedObject(VA.getValVT().getSizeInBits()/8,
                                  VA.getLocMemOffset(),
                                  isImmutable);
  SDValue FIN = DAG.getFrameIndex(FI, getPointerTy());

  if (Flags.isByVal())
    return FIN;

  return DAG.getLoad(VA.getValVT(), dl, Chain, FIN,
                     MachinePointerInfo::getFixedStack(FI),
                     false, false,false, 0);
}

static const TargetRegisterClass* getRegClassFromType(unsigned int type) {
  switch (type) {
  default:
    assert(0 && "Passed in type does not match any register classes.");
  case MVT::i1:
    return &HSAIL::CRRegClass;
  case MVT::i32:
  case MVT::f32:
    return &HSAIL::GPR32RegClass;
  case MVT::i64:
  case MVT::f64:
    return &HSAIL::GPR64RegClass;
  }
}


EVT HSAILTargetLowering::getValueType(Type *Ty, bool AllowUnknown ) const
{
    EVT VT = EVT::getEVT(Ty, AllowUnknown);

    if (!EnableExperimentalFeatures)
      return TargetLowering::getValueType(Ty, AllowUnknown);
    
    if (VT != MVT::iPTR)
      return TargetLowering::getValueType(Ty, AllowUnknown);

    OpaqueType OT = GetOpaqueType(Ty);
    if (IsImage(OT) || OT == Sampler)
      return MVT::i64;
    
    return getPointerTy();
}


/// LowerFormalArguments - This hook must be implemented to lower the
/// incoming (formal) arguments, described by the Ins array, into the
/// specified DAG. The implementation should fill in the InVals array
/// with legal-type argument values, and return the resulting token
/// chain value.
///
SDValue
HSAILTargetLowering::LowerFormalArguments(SDValue Chain,
                                          CallingConv::ID CallConv,
                                          bool isVarArg,
                                          const SmallVectorImpl<ISD::InputArg> &Ins,
                                          DebugLoc dl,
                                          SelectionDAG &DAG,
                                          SmallVectorImpl<SDValue> &InVals) const
{
  MachineFunction &MF = DAG.getMachineFunction();
  HSAILMachineFunctionInfo *FuncInfo = MF.getInfo<HSAILMachineFunctionInfo>();
  HSAILParamManager &PM = FuncInfo->getParamManager();
  const FunctionType *funcType = MF.getFunction()->getFunctionType();
  bool isKernel = isKernelFunc(MF.getFunction());
  unsigned int j = 0;
  SDValue InFlag;

  // Map function param types to Ins.
  SmallVector<const Type*, 16> paramTypes;
  unsigned int k = 0;
  Function::const_arg_iterator AI = MF.getFunction()->arg_begin();
  Function::const_arg_iterator AE = MF.getFunction()->arg_end();
  for(; AI != AE; ++AI) {
      const Type* type = AI->getType();

      unsigned ParamSize = Ins[j].VT.getStoreSizeInBits();

      unsigned Param = PM.addArgumentParam(ParamSize);
      const char* ParamName = PM.getParamName(Param);
      std::string md = (AI->getName() + ":" + ParamName + " ").str();
      SDValue ParamValue = DAG.getTargetExternalSymbol(ParamName, MVT::Other);
      PM.addParamType(type, Param);
      SDValue ArgValue;
      unsigned ldParamOp;

      // TODO_HSA: This assumes that char and short vector elements
      // are unpacked in Ins.
      const VectorType *VecVT = dyn_cast<VectorType>(type);
      if (VecVT != NULL) {
        type = VecVT->getElementType();
        EVT VT = Ins[j].VT;
        if(!isKernel)
          ParamValue = DAG.getRegister(HSAIL::PARAMREGRegClass.getRegister(k), VT);

        int offset = type->getPrimitiveSizeInBits() / 8;
        for (unsigned x = 0, y = VecVT->getNumElements(); x < y; ++x) {
          if(isKernel && x>0) {
            ParamSize = Ins[j].VT.getStoreSizeInBits();
            Param = PM.addArgumentParam(ParamSize);
            ParamName = PM.getParamName(Param);
            md += ":" + std::string(ParamName);
            ParamValue = DAG.getTargetExternalSymbol(ParamName, MVT::Other);
            PM.addParamType(type, Param);
          }

          if(isKernel) {
            if (type->isIntegerTy(8)) {
              ldParamOp = HSAILISD::LOAD_PARAM_U8;
            } else if (type->isIntegerTy(16))  {
              ldParamOp = HSAILISD::LOAD_PARAM_U16;
            } else {
              ldParamOp = HSAILISD::LOAD_PARAM;
            }
            ArgValue = DAG.getNode(ldParamOp, dl, Ins[j].VT, Chain, ParamValue);
          } else {
            if (type->isIntegerTy(8)) {
              ldParamOp = HSAILISD::LD_SCALAR_ARG_U8;
            } else if (type->isIntegerTy(16))  {
              ldParamOp = HSAILISD::LD_SCALAR_ARG_U16;
            } else {
              ldParamOp = HSAILISD::LD_SCALAR_ARG;
            }
            SDVTList VTs = DAG.getVTList(Ins[j].VT, MVT::Other, MVT::Glue);
            SDValue Ops[] = { Chain, ParamValue, DAG.getConstant(x * offset, MVT::i32), InFlag };
            ArgValue = DAG.getNode(ldParamOp, dl, VTs, Ops, InFlag.getNode() ? 4 : 3);
            Chain = ArgValue.getValue(1);
            InFlag = ArgValue.getValue(2);
          }
          InVals.push_back(ArgValue);
          j++;
        }
        continue; // is necessary to avoid duplicate increment of 'j'
      } else if (const StructType *StructVT = dyn_cast<StructType>(type)) {
        assert(!"When do we hit this?");
      } else if (type->isPointerTy()) {
        if ( EnableExperimentalFeatures ) {
          OpaqueType OT = GetOpaqueType(type);
          if (IsImage(OT)) {
            ldParamOp = HSAILISD::LOAD_PARAM_IMAGE;
            ArgValue = DAG.getNode(ldParamOp, dl, Ins[j].VT, Chain, ParamValue);
            InVals.push_back(ArgValue);
          } else if (OT == Sampler) {
            ldParamOp = HSAILISD::LOAD_PARAM_SAMP;
            ArgValue = DAG.getNode(ldParamOp, dl, Ins[j].VT, Chain, ParamValue);
            InVals.push_back(ArgValue);
          } else {
            const PointerType *PT = dyn_cast<PointerType>(type);
            Type *CT = PT->getElementType();
            const StructType *ST = dyn_cast<StructType>(CT);
            unsigned addrSpace = PT->getAddressSpace();
            if (ST) {
              if (addrSpace == 0) {
                ldParamOp = HSAILISD::LOAD_PARAM_PTR_STRUCT_BY_VAL;
                ArgValue = DAG.getNode(ldParamOp, dl, Ins[j].VT, Chain, ParamValue);
                FuncInfo->setHasStructByVal(true);
                InVals.push_back(ArgValue);
              } else
              {
                ldParamOp = HSAILISD::LOAD_PARAM_PTR;
                ArgValue = DAG.getNode(ldParamOp, dl, Ins[j].VT, Chain, ParamValue);
                InVals.push_back(ArgValue);
              } 
            } else {
              ldParamOp = HSAILISD::LOAD_PARAM_PTR;
              ArgValue = DAG.getNode(ldParamOp, dl, Ins[j].VT, Chain, ParamValue);
              InVals.push_back(ArgValue);
            }
          }
        // ELSE IF NOT EnableExperimentalFeatures
        } else {
          const PointerType *PT = dyn_cast<PointerType>(type);
          Type *CT = PT->getElementType();
          const StructType *ST = dyn_cast<StructType>(CT);
          unsigned addrSpace = PT->getAddressSpace();
          if (ST) {
            OpaqueType OT = GetOpaqueType(ST);
            if (IsImage(OT) || OT == Sampler) {
              // Lower image and sampler kernel arg to image arg handle index. 
              // We bias the values of image_t and sampler_t arg indices so that 
              // we know that index values >= IMAGE_ARG_BIAS represent kernel args. 
              // Note that if either the order of processing for kernel args  
              // or the biasing of index values is changed here, these changes must be 
              // reflected in HSAILPropagateImageOperands and 
              // HSAILAsmPrinter::printImageMemOperand().
              unsigned index = 
                Subtarget->getImageHandles()->findOrCreateImageHandle(ParamName);
              index += IMAGE_ARG_BIAS;
              ArgValue = DAG.getConstant((index), MVT::i32);
              InVals.push_back(ArgValue);
            }
            else if (addrSpace == 0) {
              ldParamOp = HSAILISD::LOAD_PARAM_PTR_STRUCT_BY_VAL;
              ArgValue = DAG.getNode(ldParamOp, dl, Ins[j].VT, Chain, ParamValue);
              FuncInfo->setHasStructByVal(true);
              InVals.push_back(ArgValue);
            }
	    else {
              ldParamOp = HSAILISD::LOAD_PARAM_PTR;
              ArgValue = DAG.getNode(ldParamOp, dl, Ins[j].VT, Chain, ParamValue);
              InVals.push_back(ArgValue);
            } 
          } else {
            ldParamOp = HSAILISD::LOAD_PARAM_PTR;
            ArgValue = DAG.getNode(ldParamOp, dl, Ins[j].VT, Chain, ParamValue);
            InVals.push_back(ArgValue);
          }
        }  // END EnableExperimentalFeatures
      } else if (type->isIntegerTy(8)) {
        ldParamOp = HSAILISD::LOAD_PARAM_U8;
        ArgValue = DAG.getNode(ldParamOp, dl, Ins[j].VT, Chain, ParamValue);
        InVals.push_back(ArgValue);
      } else if (type->isIntegerTy(16))  {
        ldParamOp = HSAILISD::LOAD_PARAM_U16;
        ArgValue = DAG.getNode(ldParamOp, dl, Ins[j].VT, Chain, ParamValue);
        InVals.push_back(ArgValue);
      } else {
        ldParamOp = HSAILISD::LOAD_PARAM;
        ArgValue = DAG.getNode(ldParamOp, dl, Ins[j].VT, Chain, ParamValue);
        InVals.push_back(ArgValue);
      }
      j++;
      FuncInfo->addMetadata("argmap:"+ md, true);
  }

  return Chain;
}

// TODO: Eliminate this forward declaration, which was introduced when
// the function was made a non-member. Moving the function body in the
// same CL would have obscured the changes being made in the body.
static SDValue LowerCallResult(SDValue Chain,
                               SDValue& InFlag,
                               CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const Type *type,
                               DebugLoc dl,
                               SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals);

/// LowerCall - This hook must be implemented to lower calls into the
/// the specified DAG. The outgoing arguments to the call are described
/// by the Outs array, and the values to be returned by the call are
/// described by the Ins array. The implementation should fill in the
/// InVals array with legal-type return values from the call, and return
/// the resulting token chain value.

SDValue HSAILTargetLowering::LowerCall(CallLoweringInfo &CLI,
                               SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG                     = CLI.DAG;
  DebugLoc &dl                          = CLI.DL;
  SmallVector<ISD::OutputArg, 32> &Outs = CLI.Outs;
  SmallVector<SDValue, 32> &OutVals     = CLI.OutVals;
  SmallVector<ISD::InputArg, 32> &Ins   = CLI.Ins;
  SDValue Chain                         = CLI.Chain;
  SDValue Callee                        = CLI.Callee;
  bool &isTailCall                      = CLI.IsTailCall;
  CallingConv::ID CallConv              = CLI.CallConv;
  bool isVarArg                         = CLI.IsVarArg;

  isTailCall = false;
  MachineFunction& MF = DAG.getMachineFunction();
  // FIXME: DO we need to handle fast calling conventions and tail call
  // optimizations?? X86/PPC ISelLowering
  /*bool hasStructRet = (TheCall->getNumArgs())
    ? TheCall->getArgFlags(0).device()->isSRet()
    : false;*/

  MachineFrameInfo *MFI = MF.getFrameInfo();

  unsigned int NumBytes = 0;//CCInfo.getNextStackOffset();
  if (isTailCall) {
    assert(isTailCall && "Tail Call not handled yet!");
    // See X86/PPC ISelLowering
  }

  SDValue CallSeqStart = DAG.getCALLSEQ_START(Chain, DAG.getIntPtrConstant(NumBytes, true));
  Chain = CallSeqStart.getValue(0);

  SmallVector<std::pair<unsigned int, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;
  SDValue StackPtr;

  const FunctionType * funcType = NULL;

  // If the callee is a GlobalAddress/ExternalSymbol node (quite common,
  // every direct call is) turn it into a TargetGlobalAddress/
  // TargetExternalSymbol
  // node so that legalize doesn't hack it.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))  {
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl, getPointerTy());

    const Function * calleeFunc = static_cast<const Function*>(G->getGlobal());
    funcType = calleeFunc->getFunctionType();
  } 
  else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    Callee = DAG.getTargetExternalSymbol(S->getSymbol(), getPointerTy());

    // HSA_TODO: Use `Outs` and `Ins` instead of funcType in the rest of this function
    assert(!"Not implemented");
  }

  assert(funcType != NULL);

  SmallVector<SDValue, 8> Ops;
  SmallVector<SDValue, 8> RegOps;
  SDVTList VTs = DAG.getVTList(MVT::Other, MVT::Glue);

  SDValue InFlag = CallSeqStart.getValue(1);
  const Type * retType = funcType->getReturnType();
  if(retType->getTypeID() != Type::VoidTyID) {
    SDValue RetValue = DAG.getConstant(HSAIL::R0, MVT::i32);

    unsigned DeclOpcode = ISD::UNDEF;
    const Type* sType = retType->getScalarType();
    if (sType->isIntegerTy(8)) {
      DeclOpcode = HSAILISD::RET_DECL_U8;
    } else if (sType->isIntegerTy(16)) {
      DeclOpcode = HSAILISD::RET_DECL_U16;
    } else if (sType->isIntegerTy(32)) {
      DeclOpcode = HSAILISD::RET_DECL_U32;
    } else if (sType->isIntegerTy(64)) {
      DeclOpcode = HSAILISD::RET_DECL_U64;
    } else if (sType->isFloatTy()) {
      DeclOpcode = HSAILISD::RET_DECL_F32;
    } else if (sType->isDoubleTy()) {
      DeclOpcode = HSAILISD::RET_DECL_F64;
    } else if (sType->isPointerTy()) {
      DeclOpcode = Subtarget->is64Bit() ? HSAILISD::RET_DECL_U64 : HSAILISD::RET_DECL_U32;
    }
    assert(DeclOpcode != ISD::UNDEF);

    const VectorType *VecVT = dyn_cast<VectorType>(retType);

    SDValue arrSize =  DAG.getConstant(VecVT ? VecVT->getNumElements() : 1, MVT::i32);
    SDValue ArgDeclOps[] = {  Chain, RetValue, arrSize, InFlag };
    SDValue ArgDecl = DAG.getNode(DeclOpcode, dl, VTs, ArgDeclOps, InFlag.getNode() ? 4 : 3);
    Chain = ArgDecl.getValue(0);
    InFlag = ArgDecl.getValue(1);
  }

  unsigned int j= 0, k=0;
  for(FunctionType::param_iterator pb = funcType->param_begin(),
    pe = funcType->param_end(); pb != pe; ++pb, ++k) {
    const Type * type = *pb;
    EVT VT = Outs[j].VT;

    assert(HSAIL::PARAMREGRegClass.getNumRegs() > k && "Too many parameters! Increase PARAMReg");
    unsigned RegNo = HSAIL::PARAMREGRegClass.getRegister(k);
    SDValue DeclParamValue = DAG.getConstant(RegNo, MVT::i32);
    SDValue StParamValue = DAG.getRegister(RegNo, VT);

    SDValue Arg = OutVals[j];

    const VectorType *VecVT = dyn_cast<VectorType>(type);
    unsigned num_elem = VecVT ? VecVT->getNumElements() : 1;

    unsigned StOpcode = HSAILISD::ST_SCALAR_ARG;
    unsigned DeclOpcode = ISD::UNDEF;
    const Type* sType = type->getScalarType();
    if (sType->isIntegerTy(8)) {
      StOpcode = HSAILISD::ST_SCALAR_ARG_U8;
      DeclOpcode = HSAILISD::ARG_DECL_U8;
    } else if (sType->isIntegerTy(16)) {
      StOpcode = HSAILISD::ST_SCALAR_ARG_U16;
      DeclOpcode = HSAILISD::ARG_DECL_U16;
    } else if (sType->isIntegerTy(32)) {
      DeclOpcode = HSAILISD::ARG_DECL_U32;
    } else if (sType->isIntegerTy(64)) {
      DeclOpcode = HSAILISD::ARG_DECL_U64;
    } else if (sType->isFloatTy()) {
      DeclOpcode = HSAILISD::ARG_DECL_F32;
    } else if (sType->isDoubleTy()) {
      DeclOpcode = HSAILISD::ARG_DECL_F64;
    } else if (sType->isPointerTy()) {
      DeclOpcode = Subtarget->is64Bit() ? HSAILISD::ARG_DECL_U64 : HSAILISD::ARG_DECL_U32;
    }
    assert(DeclOpcode != ISD::UNDEF);

    // START array parameter declaration
    SDValue arrSize =  DAG.getConstant(num_elem, MVT::i32);
    SDValue ArgDeclOps[] = { Chain, DeclParamValue, arrSize, InFlag };
    SDValue ArgDecl = DAG.getNode(DeclOpcode, dl, VTs, ArgDeclOps, InFlag.getNode() ? 4 : 3);
    Chain = ArgDecl.getValue(0);
    InFlag = ArgDecl.getValue(1);
    // END array parameter declaration

    unsigned elem_size = sType->getPrimitiveSizeInBits() / 8;
    for(unsigned int x =0; x < num_elem; ++x) {
      SDValue Arg = OutVals[j];

      SDValue offsetSD = DAG.getConstant(elem_size * x, MVT::i32);
      SDValue OPs[] = { Chain, Arg, StParamValue, offsetSD, InFlag };
      Chain = DAG.getNode(StOpcode, dl, VTs, OPs, InFlag.getNode() ? 5 : 4);
      InFlag = Chain.getValue(1);
      j++;
    }
    RegOps.push_back(StParamValue);
  }

  // If this is a direct call, pass the chain and the callee
  if (Callee.getNode()) {
    Ops.push_back(Chain);
    Ops.push_back(Callee);
  }

  // Add argument registers to the end of the list so that they are known
  // live into the call
  for (unsigned int i = 0, e = RegOps.size(); i != e; ++i) {
    Ops.push_back(RegOps[i]);
  }

  if (InFlag.getNode()) {
    Ops.push_back(InFlag);
  }

  Chain = DAG.getNode(HSAILISD::CALL, dl, VTs, &Ops[0], Ops.size());
  InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that
  // we return
  Chain = LowerCallResult(Chain, InFlag, CallConv, isVarArg, Ins,
                          retType, dl, DAG, InVals);

  // Create the CALLSEQ_END node
  Chain = DAG.getCALLSEQ_END(Chain,
                             DAG.getIntPtrConstant(NumBytes, true),
                             DAG.getIntPtrConstant(0, true),
                             InFlag);
  return Chain;
}

// LowerCallResult - Lower the result values of an ISD::CALL into the
// appropriate copies out of appropriate physical registers.  This assumes that
// Chain/InFlag are the input chain/flag to use, and that TheCall is the call
// being lowered.  The returns a SDNode with the same number of values as the
// ISD::CALL.
static SDValue LowerCallResult(SDValue Chain,
                               SDValue& InFlag,
                               CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const Type *type,
                               DebugLoc dl,
                               SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals)
{
  if(Ins.size() > 0) {
    // if size is more than 1, then it must be a vector
    SDValue retVariable = DAG.getRegister(HSAIL::R0, Ins[0].VT);
    const Type* sType = type->getScalarType();
    unsigned offset = sType->getPrimitiveSizeInBits() / 8;
    unsigned RetOpcode = HSAILISD::LD_SCALAR_RET;
    if (sType->isIntegerTy(8)) {
      RetOpcode = HSAILISD::LD_SCALAR_RET_U8;
    } else if (sType->isIntegerTy(16)) {
      RetOpcode = HSAILISD::LD_SCALAR_RET_U16;
    }
    for(unsigned i=0; i != Ins.size(); ++i) {
      EVT VT = Ins[i].VT;
      SDValue offsetVal = DAG.getConstant(i * offset, MVT::i32);
      SDValue Ops[] = { Chain, retVariable, offsetVal, InFlag };
      SDVTList VTs = DAG.getVTList(VT, MVT::Other, MVT::Glue);

      SDValue ArgValue = DAG.getNode(RetOpcode, dl, VTs, Ops, InFlag.getNode() ? 4 : 3);

      Chain = ArgValue.getValue(1);
      // we need a glue here to stich together all the vector component loads
      InFlag = ArgValue.getValue(2);
      InVals.push_back(ArgValue);
    }
  }

  return Chain;
}

/// LowerOperationWrapper - This callback is invoked by the type legalizer
/// to legalize nodes with an illegal operand type but legal result types.
/// It replaces the LowerOperation callback in the type Legalizer.
/// The reason we can not do away with LowerOperation entirely is that
/// LegalizeDAG isn't yet ready to use this callback.
/// TODO: Consider merging with ReplaceNodeResults.

/// The target places new result values for the node in Results (their number
/// and types must exactly match those of the original return values of
/// the node), or leaves Results empty, which indicates that the node is not
/// to be custom lowered after all.
/// The default implementation calls LowerOperation.
void
HSAILTargetLowering::LowerOperationWrapper(SDNode *N,
                                           SmallVectorImpl<SDValue> &Results,
                                           SelectionDAG &DAG) const
{
  return TargetLowering::LowerOperationWrapper(N, Results, DAG);
}

#define LOWER(A) \
  case ISD:: A: \
  return Lower##A(Op, DAG)

/// LowerOperation - This callback is invoked for operations that are
/// unsupported by the target, which are registered to use 'custom' lowering,
/// and whose defined values are all legal.
/// If the target has no operations that require custom lowering, it need not
/// implement this.  The default implementation of this aborts.
SDValue
HSAILTargetLowering::LowerOperation(SDValue Op,
                                    SelectionDAG &DAG) const
{
  switch (Op.getOpcode()) {
    LOWER(GlobalAddress);
    LOWER(TRUNCATE);
    LOWER(INTRINSIC_W_CHAIN);
    LOWER(ROTL);
    LOWER(ROTR);
    LOWER(BSWAP);
    LOWER(ADD);
    LOWER(LOAD);
    LOWER(ATOMIC_LOAD);
    LOWER(ATOMIC_STORE);
    break;
  default:
    Op.getNode()->dump();
    assert(0 && "Custom lowering code for this"
           "instruction is not implemented yet!");
    break;
  }
  return Op;
}

/// ReplaceNodeResults - This callback is invoked when a node result type is
/// illegal for the target, and the operation was registered to use 'custom'
/// lowering for that result type.  The target places new result values for
/// the node in Results (their number and types must exactly match those of
/// the original return values of the node), or leaves Results empty, which
/// indicates that the node is not to be custom lowered after all.
///
/// If the target has no operations that require custom lowering, it need not
/// implement this.  The default implementation aborts.
void
HSAILTargetLowering::ReplaceNodeResults(SDNode *N,
                                        SmallVectorImpl<SDValue> &Results,
                                        SelectionDAG &DAG) const
{
  return TargetLowering::ReplaceNodeResults(N, Results, DAG);
}

/// getTargetNodeName() - This method returns the name of a target specific
/// DAG node.
const char*
HSAILTargetLowering::getTargetNodeName(unsigned Opcode) const
{
  switch (Opcode) {
  default:
    llvm_unreachable("Unknown target-node");
    return NULL;
  case HSAILISD::CALL:
    return "HSAILISD::CALL";
  case HSAILISD::RET_FLAG:
    return "HSAILISD::RET_FLAG";
  case HSAILISD::LOAD_PARAM:
    return "HSAILISD::LOAD_PARAM";
  case HSAILISD::LOAD_PARAM_U8:
    return "HSAILISD::LOAD_PARAM_U8";
  case HSAILISD::LOAD_PARAM_U16:
    return "HSAILISD::LOAD_PARAM_U16";
  case HSAILISD::LOAD_PARAM_PTR:
    return "HSAILISD::LOAD_PARAM_PTR";
  case HSAILISD::LOAD_PARAM_PTR_STRUCT_BY_VAL:
    return "HSAILISD::LOAD_PARAM_PTR_STRUCT_BY_VAL";
  case HSAILISD::LDA_FLAT:
    return "HSAILISD::LDA_FLAT";
  case HSAILISD::LDA_GLOBAL:
    return "HSAILISD::LDA_GLOBAL";
  case HSAILISD::LDA_GROUP:
    return "HSAILISD::LDA_GROUP";
  case HSAILISD::LDA_PRIVATE:
    return "HSAILISD::LDA_PRIVATE";
  case HSAILISD::LDA_READONLY:
    return "HSAILISD::LDA_READONLY";
  case HSAILISD::WRAPPER:
    return "HSAILISD::WRAPPER";
  case HSAILISD::TRUNC_B1:
    return "HSAILISD::TRUNC_B1";
  case HSAILISD::LD_SCALAR_RET_U8:
    return "HSAILISD::LD_SCALAR_RET_U8";
  case HSAILISD::LD_SCALAR_RET_U16:
    return "HSAILISD::LD_SCALAR_RET_U16";
  case HSAILISD::LD_SCALAR_RET:
    return "HSAILISD::LD_SCALAR_RET";
  case HSAILISD::ST_SCALAR_ARG_U8:
    return "HSAILISD::ST_SCALAR_ARG_U8";
  case HSAILISD::ST_SCALAR_ARG_U16:
    return "HSAILISD::ST_SCALAR_ARG_U16";
  case HSAILISD::ST_SCALAR_ARG:
    return "HSAILISD::ST_SCALAR_ARG";
  case HSAILISD::ARG_DECL_U8:
    return "HSAILISD::ARG_DECL_U8";
  case HSAILISD::ARG_DECL_U16:
    return "HSAILISD::ARG_DECL_U16";
  case HSAILISD::ARG_DECL_U32:
    return "HSAILISD::ARG_DECL_U32";
  case HSAILISD::ARG_DECL_U64:
    return "HSAILISD::ARG_DECL_U64";
  case HSAILISD::ARG_DECL_F32:
    return "HSAILISD::ARG_DECL_F32";
  case HSAILISD::ARG_DECL_F64:
    return "HSAILISD::ARG_DECL_F64";
  case HSAILISD::RET_DECL_U8:
    return "HSAILISD::RET_DECL_U8";
  case HSAILISD::RET_DECL_U16:
    return "HSAILISD::RET_DECL_U16";
  case HSAILISD::RET_DECL_U32:
    return "HSAILISD::RET_DECL_U32";
  case HSAILISD::RET_DECL_U64:
    return "HSAILISD::RET_DECL_U64";
  case HSAILISD::RET_DECL_F32:
    return "HSAILISD::RET_DECL_F32";
  case HSAILISD::RET_DECL_F64:
    return "HSAILISD::RET_DECL_F64";
  case HSAILISD::ST_SCALAR_RET_U8:
    return "HSAILISD::ST_SCALAR_RET_U8";
  case HSAILISD::ST_SCALAR_RET_U16:
    return "HSAILISD::ST_SCALAR_RET_U16";
  case HSAILISD::ST_SCALAR_RET:
    return "HSAILISD::ST_SCALAR_RET";
  case HSAILISD::LD_SCALAR_ARG_U8:
    return "HSAILISD::LD_SCALAR_ARG_U8";
  case HSAILISD::LD_SCALAR_ARG_U16:
    return "HSAILISD::LD_SCALAR_ARG_U16";
  case HSAILISD::LD_SCALAR_ARG:
    return "HSAILISD::LD_SCALAR_ARG";
  case HSAILISD::LOAD_PARAM_IMAGE:
    return "HSAILISD::LOAD_PARAM_IMAGE";
  case HSAILISD::LOAD_PARAM_SAMP:
    return "HSAILISD::LOAD_PARAM_SAMP";
  }
}

//===--------------------------------------------------------------------===//
// Custom lowering methods
//

/// LowerGlobalAddress - Lowers a global address ref to a target global address lda.
SDValue
HSAILTargetLowering::LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const {
  EVT PtrVT = getPointerTy();
  DebugLoc dl = Op.getDebugLoc();
  const GlobalAddressSDNode *GSDN = cast<GlobalAddressSDNode>(Op);
  const GlobalValue *GV = GSDN->getGlobal();
  const Type *ptrType = GV->getType();
  unsigned addrSpace = dyn_cast<PointerType>(ptrType)->getAddressSpace();
  unsigned opcode;

  if (addrSpace == Subtarget->getFlatAS()) {
    opcode = HSAILISD::LDA_FLAT;
  } else if (addrSpace == Subtarget->getGlobalAS()) {
    opcode = HSAILISD::LDA_GLOBAL;
  } else if (addrSpace == Subtarget->getGroupAS()) {
    opcode = HSAILISD::LDA_GROUP;
  } else if (addrSpace == Subtarget->getPrivateAS()) {
    opcode = HSAILISD::LDA_PRIVATE;
  } else if (addrSpace == Subtarget->getConstantAS()) {
    opcode = HSAILISD::LDA_READONLY;
  } else {
    assert(!"cannot lower GlobalAddress");
  }
  SDValue targetGlobal = DAG.getTargetGlobalAddress(GV, dl, PtrVT,
                                                    GSDN->getOffset());
  return DAG.getNode(opcode, dl, PtrVT.getSimpleVT(), targetGlobal);
}

SDValue 
HSAILTargetLowering::LowerTRUNCATE(SDValue Op, SelectionDAG &DAG) const {
  DebugLoc dl = Op.getDebugLoc();
  EVT VT = Op.getValueType();

  if (VT != MVT::i1) {
    return Op;
  }
  // Generate a custom truncate operation that clears all but the
  // least-significant bit in the source operand before truncating to i1.
  const SDValue src = Op.getOperand(0);
  EVT srcVT = src.getValueType();
  const SDValue trunc =  DAG.getNode(ISD::AND, dl, srcVT, src, 
                          DAG.getConstant(1, srcVT));
  return DAG.getNode(HSAILISD::TRUNC_B1, dl, VT, trunc);
}
SDValue 
HSAILTargetLowering::LowerADD(SDValue Op, SelectionDAG &DAG) const {
  DebugLoc dl = Op.getDebugLoc();
  EVT VT = Op.getValueType();

  if (VT != MVT::i1) {
    return Op;
  }
  const SDValue src = Op.getOperand(0).getOperand(0);
  EVT srcVT = src.getValueType();
  if(Op.getOperand(0).getOpcode() != ISD::TRUNCATE) return Op;
  const SDValue dest = Op.getOperand(1);
  EVT dstcVT = dest.getValueType();
  SDValue Zext = DAG.getNode(ISD::ZERO_EXTEND, dl, MVT::i32,Op.getOperand(1));
  SDValue Zext1 = DAG.getNode(ISD::ZERO_EXTEND, dl, srcVT,Op.getOperand(0)); 
  SDValue add_p = DAG.getNode(ISD::ADD, dl, srcVT,Zext1,Zext);
  SDValue Zext2 = DAG.getNode(HSAILISD::TRUNC_B1, dl, VT,add_p);
  return Zext2;
  
}
SDValue 
HSAILTargetLowering::LowerINTRINSIC_W_CHAIN(SDValue Op, SelectionDAG &DAG) const {
  unsigned IntNo = cast<ConstantSDNode>(Op->getOperand(1))->getZExtValue();

  switch (IntNo) {
  default: 
    return Op;
  case HSAILIntrinsic::HSAIL_rd_imgf_1d_s32:
  case HSAILIntrinsic::HSAIL_rd_imgf_1d_f32:
  case HSAILIntrinsic::HSAIL_rd_imgi_1d_s32:
  case HSAILIntrinsic::HSAIL_rd_imgi_1d_f32:
  case HSAILIntrinsic::HSAIL_rd_imgui_1d_s32:
  case HSAILIntrinsic::HSAIL_rd_imgui_1d_f32:
    // read image 1d array
  case HSAILIntrinsic::HSAIL_rd_imgf_1da_s32:
  case HSAILIntrinsic::HSAIL_rd_imgf_1da_f32:
  case HSAILIntrinsic::HSAIL_rd_imgi_1da_s32:
  case HSAILIntrinsic::HSAIL_rd_imgi_1da_f32:
  case HSAILIntrinsic::HSAIL_rd_imgui_1da_s32:
  case HSAILIntrinsic::HSAIL_rd_imgui_1da_f32:
    // read image 2d
  case HSAILIntrinsic::HSAIL_rd_imgf_2d_s32:
  case HSAILIntrinsic::HSAIL_rd_imgf_2d_f32:
  case HSAILIntrinsic::HSAIL_rd_imgi_2d_s32:
  case HSAILIntrinsic::HSAIL_rd_imgi_2d_f32:
  case HSAILIntrinsic::HSAIL_rd_imgui_2d_s32:
  case HSAILIntrinsic::HSAIL_rd_imgui_2d_f32:
    // read image 2d array
  case HSAILIntrinsic::HSAIL_rd_imgf_2da_s32:
  case HSAILIntrinsic::HSAIL_rd_imgf_2da_f32:
  case HSAILIntrinsic::HSAIL_rd_imgi_2da_s32:
  case HSAILIntrinsic::HSAIL_rd_imgi_2da_f32:
  case HSAILIntrinsic::HSAIL_rd_imgui_2da_s32:
  case HSAILIntrinsic::HSAIL_rd_imgui_2da_f32:
    // read image 3d
  case HSAILIntrinsic::HSAIL_rd_imgf_3d_s32:
  case HSAILIntrinsic::HSAIL_rd_imgf_3d_f32:
  case HSAILIntrinsic::HSAIL_rd_imgi_3d_s32:
  case HSAILIntrinsic::HSAIL_rd_imgi_3d_f32:
  case HSAILIntrinsic::HSAIL_rd_imgui_3d_s32:
  case HSAILIntrinsic::HSAIL_rd_imgui_3d_f32:
    // read image 2d depth
  case HSAILIntrinsic::HSAIL_rd_imgf_2ddepth_s32:
  case HSAILIntrinsic::HSAIL_rd_imgf_2ddepth_f32:
    // read image 2d array depth
  case HSAILIntrinsic::HSAIL_rd_imgf_2dadepth_s32:
  case HSAILIntrinsic::HSAIL_rd_imgf_2dadepth_f32:
    break;
  }

  // Replace sampler operand with index into image handle table
  SDValue ops[16];
  for (unsigned i = 0; i < Op.getNumOperands(); i++) {
    ops[i] = Op.getOperand(i);
  }

  const unsigned SAMPLER_ARG = 3;

  // Detect if sampler operand is an initializer. We do this by extracting 
  // the constant operand and analyzing it. If value of the sampler operand
  // is constant and < IMAGE_ARG_BIAS, then this is a sampler initializer. 
  // Add it to the set of sampler handlers and displace the sampler operand 
  // with a sampler handler index.
  SDValue sampler = Op.getOperand(SAMPLER_ARG);
  unsigned samplerHandleIndex;
  if (isa<ConstantSDNode>(sampler)) {
    unsigned samplerConstant = cast<ConstantSDNode>(sampler)->getZExtValue();
    if (samplerConstant < IMAGE_ARG_BIAS) {
      // This is a sampler initializer.
      // Find or create sampler handle based on init val.
      samplerHandleIndex = 
        Subtarget->getImageHandles()->findOrCreateSamplerHandle(samplerConstant);

      // Actual pointer info is already lost here, all we have is an i32 constant,
      // so we cannot check address space of original pointer. This is done with the
      // check isa<ConstantSDNode>(sampler) above.
      // Provided that this is simply int const we can assume it is not going to be
      // changed, so we could use readonly segment for the sampler.
      // According to OpenCL spec samplers cannot be modified, so that is safe for
      // OpenCL. If we are going to support modifiable or non-OpenCL samplers most
      // likely the whole support code will need change.
      Subtarget->getImageHandles()->getSamplerHandle(samplerHandleIndex)->setRO();
      ops[SAMPLER_ARG] = DAG.getConstant(samplerHandleIndex, MVT::i32);
    }
  }

  DAG.UpdateNodeOperands(Op.getNode(), ops, Op.getNumOperands()),
    Op.getOperand(0).getResNo();

  return Op;
}

SDValue 
HSAILTargetLowering::LowerROTL(SDValue Op, SelectionDAG &DAG) const {
  DebugLoc dl = Op.getDebugLoc();
  EVT VT = Op.getValueType();

  if (VT != MVT::i32) {
    return Op;
  }
  const SDValue src0 = Op.getOperand(0);
  const SDValue src1 = Op.getOperand(1);
  const ConstantSDNode* shift = dyn_cast<ConstantSDNode>(src1);
  return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, VT,
           DAG.getConstant(HSAILIntrinsic::HSAIL_bitalign_b32, MVT::i32),
           src0, src0,
           shift ?
             DAG.getConstant(32 - (shift->getZExtValue() & 31), MVT::i32) :
             DAG.getNode(ISD::SUB, dl, VT, DAG.getConstant(0, VT), src1));  
}

SDValue 
HSAILTargetLowering::LowerROTR(SDValue Op, SelectionDAG &DAG) const {
  DebugLoc dl = Op.getDebugLoc();
  EVT VT = Op.getValueType();

  if (VT != MVT::i32) {
    return Op;
  }
  const SDValue src0 = Op.getOperand(0);
  const SDValue src1 = Op.getOperand(1);
  return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, VT,
           DAG.getConstant(HSAILIntrinsic::HSAIL_bitalign_b32, MVT::i32),
           src0, src0, src1);  
}

SDValue
HSAILTargetLowering::LowerATOMIC_STORE(SDValue Op, SelectionDAG &DAG) const {
  // HSAIL doesnt support SequentiallyConsistent,
  // lower an atomic store with SequentiallyConsistent memory order
  // to Release atomic store and Acquire memfence
  DebugLoc dl = Op.getDebugLoc();
  SDNode *Node = Op.getNode();
  MemSDNode *Mn = dyn_cast<MemSDNode>(Node);
  unsigned fenceOp;

  if (Mn->getOrdering() != SequentiallyConsistent) return Op;

  switch(Mn->getAddressSpace()) {
    case llvm::HSAILAS::GLOBAL_ADDRESS :
      switch(Mn->getMemoryScope()) {
        case Brig::BRIG_MEMORY_SCOPE_WORKGROUP :
          fenceOp = HSAILIntrinsic::HSAIL_memfence_g_acq_wg;break;
        case Brig::BRIG_MEMORY_SCOPE_COMPONENT :
          fenceOp = HSAILIntrinsic::HSAIL_memfence_g_acq_cmp;break;
        case Brig::BRIG_MEMORY_SCOPE_SYSTEM :
          fenceOp = HSAILIntrinsic::HSAIL_memfence_g_acq_sys;break;
        default : llvm_unreachable("invalid memory scope");
      } break;
    case llvm::HSAILAS::GROUP_ADDRESS :
      switch(Mn->getMemoryScope()) {
        case Brig::BRIG_MEMORY_SCOPE_WORKGROUP :
          fenceOp = HSAILIntrinsic::HSAIL_memfence_l_acq_wg;break;
        case Brig::BRIG_MEMORY_SCOPE_COMPONENT :
          fenceOp = HSAILIntrinsic::HSAIL_memfence_l_acq_cmp;break;
        case Brig::BRIG_MEMORY_SCOPE_SYSTEM :
          fenceOp = HSAILIntrinsic::HSAIL_memfence_l_acq_sys;break;
        default : llvm_unreachable("invalid memory scope");
      } break;
    default : llvm_unreachable("Unhandled memory segment on atomic op");
  }

  SDValue ResNode = DAG.getAtomic(ISD::ATOMIC_STORE, dl,
                                  cast<AtomicSDNode>(Node)->getMemoryVT(),
                                  Node->getOperand(0), Node->getOperand(1),
                                  Node->getOperand(2),
                                  cast<AtomicSDNode>(Node)->getMemOperand(),
                                  Release,
                                  cast<AtomicSDNode>(Node)->getSynchScope(),
                                  cast<AtomicSDNode>(Node)->getMemoryScope());
  SmallVector<SDValue, 2> Ops;
  Ops.push_back(ResNode);
  Ops.push_back(DAG.getTargetConstant(fenceOp, MVT::i64));
  SDValue Chain = DAG.getNode(ISD::INTRINSIC_VOID, Op.getDebugLoc(),
                              DAG.getVTList(MVT::Other), Ops.data(), Ops.size());
  return Chain;
}

SDValue
HSAILTargetLowering::LowerATOMIC_LOAD(SDValue Op, SelectionDAG &DAG) const {
 // HSAIL doesnt support SequentiallyConsistent,
 // lower an atomic load with SequentiallyConsistent memory order
 // to a Release memfence and Acquire atomic load
  DebugLoc dl = Op.getDebugLoc();
  SDNode *Node = Op.getNode();
  MemSDNode *Mn = dyn_cast<MemSDNode>(Node);
  unsigned fenceOp;

  if (Mn->getOrdering() != SequentiallyConsistent) return Op;

  switch(Mn->getAddressSpace()) {
    case llvm::HSAILAS::GLOBAL_ADDRESS :
      switch(Mn->getMemoryScope()) {
        case Brig::BRIG_MEMORY_SCOPE_WORKGROUP :
          fenceOp = HSAILIntrinsic::HSAIL_memfence_g_rel_wg;break;
        case Brig::BRIG_MEMORY_SCOPE_COMPONENT :
          fenceOp = HSAILIntrinsic::HSAIL_memfence_g_rel_cmp;break;
        case Brig::BRIG_MEMORY_SCOPE_SYSTEM :
          fenceOp = HSAILIntrinsic::HSAIL_memfence_g_rel_sys;break;
        default : llvm_unreachable("invalid memory scope");
      } break;
    case llvm::HSAILAS::GROUP_ADDRESS :
      switch(Mn->getMemoryScope()) {
        case Brig::BRIG_MEMORY_SCOPE_WORKGROUP :
          fenceOp = HSAILIntrinsic::HSAIL_memfence_l_rel_wg;break;
        case Brig::BRIG_MEMORY_SCOPE_COMPONENT :
          fenceOp = HSAILIntrinsic::HSAIL_memfence_l_rel_cmp;break;
        case Brig::BRIG_MEMORY_SCOPE_SYSTEM :
          fenceOp = HSAILIntrinsic::HSAIL_memfence_l_rel_sys;break;
        default : llvm_unreachable("invalid memory scope");
      } break;
    default : llvm_unreachable("Unhandled memory segment on atomic op");
  }

  SmallVector<SDValue, 2> Ops;
  Ops.push_back(Op.getOperand(0));
  Ops.push_back(DAG.getTargetConstant(fenceOp, MVT::i64));
  SDValue Chain = DAG.getNode(ISD::INTRINSIC_VOID, Op.getDebugLoc(),
                              DAG.getVTList(MVT::Other), Ops.data(), Ops.size());
  SDValue ResNode = DAG.getAtomic(ISD::ATOMIC_LOAD, dl,
                                  cast<AtomicSDNode>(Node)->getMemoryVT(),
                                  Op.getValueType(), Chain, Node->getOperand(1),
                                  cast<AtomicSDNode>(Node)->getMemOperand(),
                                  Acquire,
                                  cast<AtomicSDNode>(Node)->getSynchScope(),
                                  cast<AtomicSDNode>(Node)->getMemoryScope());
  return ResNode;
}

SDValue
HSAILTargetLowering::LowerBSWAP(SDValue Op, SelectionDAG &DAG) const {
  DebugLoc dl = Op.getDebugLoc();
  EVT VT = Op.getValueType();

  if (VT != MVT::i32) {
    return Op;
  }
  const SDValue src = Op.getOperand(0);
  const SDValue opr0 = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, VT,
           DAG.getConstant(HSAILIntrinsic::HSAIL_bytealign_b32, MVT::i32),
           src, src, DAG.getConstant(3, MVT::i32));  
  const SDValue opr1 = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, VT,
           DAG.getConstant(HSAILIntrinsic::HSAIL_bytealign_b32, MVT::i32),
           src, src, DAG.getConstant(1, MVT::i32));  
  return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, VT,
           DAG.getConstant(HSAILIntrinsic::HSAIL_bitsel_u32, MVT::i32),
           DAG.getConstant(0x00ff00ff, VT), opr0, opr1);  
}

SDValue 
HSAILTargetLowering::LowerLOAD(SDValue Op, SelectionDAG &DAG) const {
  // Custom lowering for extload from sub-dword size to i64. We only
  // do it because LLVM currently does not support Expand for EXTLOAD
  // with illegal types.
  // See "EXTLOAD should always be supported!" assert in LegalizeDAG.cpp.
  EVT VT = Op.getValueType();
  if (VT.getSimpleVT() != MVT::i64) return Op;
  LoadSDNode *LD = cast<LoadSDNode>(Op.getNode());
  DebugLoc dl = Op.getDebugLoc();
  ISD::LoadExtType extType = LD->getExtensionType();
  // Do load into 32-bit register.
  SDValue load = DAG.getLoad(ISD::UNINDEXED, extType, MVT::i32, dl,
    Op.getOperand(0), Op.getOperand(1), Op.getOperand(2),
    LD->getMemoryVT(), LD->getMemOperand());
  // Extend
  SDValue extend;
  switch (extType) {
  case ISD::SEXTLOAD:
   extend = DAG.getSExtOrTrunc(load, dl, MVT::i64); break;
  case ISD::EXTLOAD:
  case ISD::ZEXTLOAD:
    extend = DAG.getZExtOrTrunc(load, dl, MVT::i64); break;
  default:
    llvm_unreachable("Should be processing ext load");
  }
  // Replace chain in all uses.
  DAG.ReplaceAllUsesOfValueWith(Op.getValue(1), load.getValue(1));
  return extend;
}

//===--------------------------------------------------------------------===//
// Inline Asm Support hooks
//

/// ExpandInlineAsm - This hook allows the target to expand an inline asm
/// call to be explicit llvm code if it wants to.  This is useful for
/// turning simple inline asms into LLVM intrinsics, which gives the
/// compiler more information about the behavior of the code.
bool
HSAILTargetLowering::ExpandInlineAsm(CallInst *CI) const
{
  assert(!"When do we hit this?");
  return false;
}

/// ParseConstraints - Split up the constraint string from the inline
/// assembly value into the specific constraints and their prefixes,
/// and also tie in the associated operand values.
/// If this returns an empty vector, and if the constraint string itself
/// isn't empty, there was an error parsing.
TargetLowering::AsmOperandInfoVector
HSAILTargetLowering::ParseConstraints(ImmutableCallSite CS) const
{
  assert(!"When do we hit this?");
  return AsmOperandInfoVector();
}

/// Examine constraint type and operand type and determine a weight value.
/// The operand object must already have been set up with the operand type.
TargetLowering::ConstraintWeight
HSAILTargetLowering::getMultipleConstraintMatchWeight(AsmOperandInfo &info,
                                                      int maIndex) const
{
  assert(!"When do we hit this?");
  return ConstraintWeight();
}

/// Examine constraint string and operand type and determine a weight value.
/// The operand object must already have been set up with the operand type.
TargetLowering::ConstraintWeight
HSAILTargetLowering::getSingleConstraintMatchWeight(AsmOperandInfo &info,
                                                    const char *constraint) const
{
  assert(!"When do we hit this?");
  return ConstraintWeight();
}

/// ComputeConstraintToUse - Determines the constraint code and constraint
/// type to use for the specific AsmOperandInfo, setting
/// OpInfo.ConstraintCode and OpInfo.ConstraintType.  If the actual operand
/// being passed in is available, it can be passed in as Op, otherwise an
/// empty SDValue can be passed.
void
HSAILTargetLowering::ComputeConstraintToUse(AsmOperandInfo &OpInfo,
                                            SDValue Op,
                                            SelectionDAG *DAG) const
{
  assert(!"When do we hit this?");
}

/// getConstraintType - Given a constraint, return the type of constraint it
/// is for this target.
TargetLowering::ConstraintType
HSAILTargetLowering::getConstraintType(const std::string &Constraint) const
{
  assert(!"When do we hit this?");
  return ConstraintType();
}

/*
/// getRegClassForInlineAsmConstraint - Given a constraint letter (e.g. "r"),
/// return a list of registers that can be used to satisfy the constraint.
/// This should only be used for C_RegisterClass constraints.
std::vector<unsigned>
HSAILTargetLowering::getRegClassForInlineAsmConstraint(const std::string &Constraint,
                                                       EVT VT) const
{
  return TargetLowering::getRegClassForInlineAsmConstraint(Constraint, VT);
}
*/

/// getRegForInlineAsmConstraint - Given a physical register constraint (e.g.
/// {edx}), return the register number and the register class for the
/// register.
///
/// Given a register class constraint, like 'r', if this corresponds directly
/// to an LLVM register class, return a register of 0 and the register class
/// pointer.
///
/// This should only be used for C_Register constraints.  On error,
/// this returns a register number of 0 and a null register class pointer..
std::pair<unsigned, const TargetRegisterClass*>
HSAILTargetLowering::getRegForInlineAsmConstraint(const std::string &Constraint,
                                                  EVT VT) const
{
  return TargetLowering::getRegForInlineAsmConstraint(Constraint, VT);
}

/// LowerXConstraint - try to replace an X constraint, which matches anything,
/// with another that has more specific requirements based on the type of the
/// corresponding operand.  This returns null if there is no replacement to
/// make.
const char*
HSAILTargetLowering::LowerXConstraint(EVT ConstraintVT) const
{
  assert(!"When do we hit this?");
  return NULL;
}

/// LowerAsmOperandForConstraint - Lower the specified operand into the Ops
/// vector.  If it is invalid, don't add anything to Ops.
void
HSAILTargetLowering::LowerAsmOperandForConstraint(SDValue Op,
                                                  char ConstraintLetter,
                                                  std::vector<SDValue> &Ops,
                                                  SelectionDAG &DAG) const
{
  assert(!"When do we hit this?");
}

#define HSAIL_IS_LDA_FLAT(I) \
  (I)->getOpcode() == HSAIL::ldas_global_flat_addr32 \
  || (I)->getOpcode() == HSAIL::ldas_group_flat_addr32 \
  || (I)->getOpcode() == HSAIL::ldas_global_flat_addr64 \
  || (I)->getOpcode() == HSAIL::ldas_group_flat_addr64

#define HSAIL_IS_LD_PARAMPTR(I) \
  (I)->getOpcode() == HSAIL::PARAMPTR32LDpi \
  || (I)->getOpcode() == HSAIL::PARAMPTR64LDpi

#define HSAIL_IS_LD_PARAMPTR_FLAT(I) \
  (I)->getOpcode() == HSAIL::PARAMPTR32_flatLDpi \
  || (I)->getOpcode() == HSAIL::PARAMPTR64_flatLDpi

#define HSAIL_IS_LD_PARAM_FLAT(I) \
  (I)->getOpcode() ==  HSAIL::PARAMU8_flatLDpi_Kernel  \
  || (I)->getOpcode() ==  HSAIL::PARAMU16_flatLDpi_Kernel \
  || (I)->getOpcode() ==  HSAIL::PARAMU32_flatLDpi_Kernel \
  || (I)->getOpcode() ==  HSAIL::PARAMF32_flatLDpi_Kernel \
  || (I)->getOpcode() ==  HSAIL::PARAMU64_flatLDpi_Kernel \
  || (I)->getOpcode() ==  HSAIL::PARAMF64_flatLDpi_Kernel

// HandleIncomingArgumentConversion -- Here we handle all the incoming
// kernel arguments in flat address mode. The arguments are located in the
// kernarg memory segment. The arguments themselves may be pointers.
// We first convert the load the address of the kernarg parameter and
// convert it to a flat address. The argument is then loaded. If the
// argument is a segment pointer, it is converted to a flat pointer.
// Note all pointers either are 32 or 64 bit registers. Examples show the
// 32 bit case.
//
// Non-flat code                       Flat code
// ------------------------------      ------------------------------
// Scalar argument case:
// ld_kernarg_u32 $s0,[%arg_val0]      lda_u32 $s0,[%arg_val0]
//                                     stof_private_u32 $s0,$s0
//                                     ld_u32 $s1,[$s0]
//
// Pointer argument case (%arg_val1 is a pointer arg):
// ld_kernarg_u32(u64) $s0,[%arg_val1] lda_u32(u64) $s0,[%arg_val1]
//                                     stof_private_u32 $s0,$s0
//                                     ld_u32 $s1,[$s0]
//                                     stof_<segment of arg>_u32 $s0,$s1
//
// AMP does not required the loaded pointer to be converted so
// the second stof is not required since the pointer is by definition
// of AMP, flat.
//                                     lda_u32(u64) $s0,[%arg_val1]
//                                     stof_private_u32 $s0,$s0
//                                     ld_u32 $s0,[$s0]
//
// Returns true if the conversion was performed, false otherwise.
bool
HSAILTargetLowering::HandleIncomingArgumentConversion(MachineInstr *MI,
    MachineBasicBlock *MBB) const
{
  const TargetInstrInfo *TII = getTargetMachine().getInstrInfo();
  const TargetRegisterInfo *TRI = getTargetMachine().getRegisterInfo();
  MachineFunction *MF = MBB->getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  MachineBasicBlock::iterator MBI, MBIE, MBIinsertPoint;
  DebugLoc  DL = MI->getDebugLoc();

  // Insertion points for new instructions
  for (MBI=MBB->begin(), MBIE=MBB->end(); MBI != MBIE; ++MBI ) {
      if (MI == *&MBI) {
         MBIinsertPoint = MBI;
         break;
      }
  }

  HSAILMachineFunctionInfo *FuncInfo = MF->getInfo<HSAILMachineFunctionInfo>();
  HSAILParamManager &PM = FuncInfo->getParamManager();
  const Type * ParamType = 0;
  const char * ParamName = 0;
  EVT VT = Subtarget->is64Bit() ? MVT::i64 : MVT::i32;
  const TargetRegisterClass *rc = getRegClassFor(VT);
  unsigned Vreg1 = MRI.createVirtualRegister(rc);
  unsigned Vreg2 = MRI.createVirtualRegister(rc);

  // First transform the kernarg offset to a flat address, this is done for all flat
  // parameter load instructions regardless of OCL or AMP.
  // ld_u32 reg,[%arg_val] --> lda vreg1,[%arg_val], stof_private vreg2, vreg1
  if (HSAIL_IS_LD_PARAMPTR_FLAT(MI) || HSAIL_IS_LD_PARAM_FLAT(MI)) {
    HSAILParamManager::param_iterator AI, AE;
    const unsigned *Index = 0;
    // Find the parameter in this instruction
    for (AI = PM.arg_begin(), AE = PM.arg_end(); AI != AE; ++AI) {
      Index = *&AI;
      ParamName = PM.getParamName(*Index);
      if (strcmp(ParamName, MI->getOperand(1).getSymbolName()) == 0) {
        ParamType = PM.getParamType(*Index);
        break;
      }
    }
    assert( Index && 
            Index != *&AE && "Could not find argument value for this instr");

    // Emit the lda vreg,[%param], stof_private vreg,vreg sequence that is
    // always required for flat address mode
    unsigned Opc = Subtarget->is64Bit() ?
        HSAIL::ldas_private_flat_addr64 : HSAIL::ldas_private_flat_addr32;
    // Emit lda vreg,[%param]
    MachineInstr *MIbase = BuildMI(*MBB, MBIinsertPoint, DL, TII->get(Opc))
        .addReg(Vreg1, RegState::Define)
        .addExternalSymbol(ParamName)
        .addReg(0)
        .addImm(0);
    // Emit  stof_private Vreg2,Vreg1
    Opc = Subtarget->is64Bit() ?
        HSAIL::private_stof_u64 : HSAIL::private_stof_u32 ;
    MachineInstr *MI_stof = BuildMI(*MBB, MBIinsertPoint, DL, TII->get(Opc))
        .addReg(Vreg2, RegState::Define)
        .addReg(Vreg1, getKillRegState(true)); // last use of Vreg1
  }
  else {
    return false;
  }

  // At this point we can remove the memory operand from
  // the original incoming arg load instruction and substitute the converted
  // register value (Vreg2). ld orig_reg,[%arg_val] --> ld Vreg3,[vreg2]

  // Change the memory operand to a register
  assert( MI->getNumOperands()==2 );
  MachineOperand &MemOp = MI->getOperand(1);
  MemOp.ChangeToRegister(Vreg2, false/* isDef */, false/*isImp*/, true/* isKill */,
      false/* isDead */, false/*isUndef*/, false/*isDebug*/);

  if (HSAIL_IS_LD_PARAM_FLAT(MI)) {
    // We're done, a non-pointer doesn't need stof.
    // We should have lda Vreg1,[%param], stof Vreg2,Vreg1, ld_u32 orig_reg, [Vreg2]
    return true;
  } else if (HSAIL_IS_LD_PARAMPTR_FLAT(MI)) { // Handle pointer parameter loads
    // Now all the remains is to transform ld_u32 orig_reg,[Vreg2] to
    // ld_u32 Vreg3,[Vreg2] and stof_<segment>_<type> orig_reg,Vreg3.
    // Save the original target register
    unsigned SavedTargetReg = MI->getOperand(0).getReg();
    //Set the destination of the load to a new virtual register */
    unsigned Vreg3 = MRI.createVirtualRegister(MRI.getRegClass(SavedTargetReg));
    // The type of the pointer argument
    PointerType *PT = dyn_cast<PointerType>(const_cast<Type *>(ParamType));
    // Since this is a PARAMPTR instruction, PT must have a value.
    assert (PT && "Cannot get type of loaded argument pointer");
    unsigned addrSpace = PT->getAddressSpace();
    {
      // Insert stof to convert OCL segment pointer to flat
      // ld x,[M] ==> ld y,[M], stof x,y
      bool do_convert=false;
      unsigned Opc;

      if (MI->getOpcode() == HSAIL::PARAMPTR32_flatLDpi) {
        if (addrSpace == Subtarget->getFlatAS()) {
          // This is a flat address, no conversion required
          do_convert=false;
        } else if (addrSpace == Subtarget->getGlobalAS()) {
          Opc = HSAIL::global_stof_u32;
          do_convert = true;
        } else if (addrSpace == Subtarget->getConstantAS()) {
          Opc = HSAIL::constant_stof_u32;
          do_convert = true;
        } else if (addrSpace == Subtarget->getGroupAS()) {
          Opc = HSAIL::group_stof_u32;
          do_convert = true;
        } else if (addrSpace == Subtarget->getPrivateAS()) {
          Opc = HSAIL::private_stof_u32;
          do_convert = true;
        } else {
          assert(!"Unknown AddrSpace qualifier on pointer");
        }
      } else if (MI->getOpcode() == HSAIL::PARAMPTR64_flatLDpi) {
        if (addrSpace == Subtarget->getFlatAS()) {
          // This is a flat address, no conversion required
          do_convert=false;
        } else if (addrSpace == Subtarget->getGlobalAS()) {
          Opc = HSAIL::global_stof_u64;
          do_convert = true;
        } else if (addrSpace == Subtarget->getConstantAS()) {
          Opc = HSAIL::constant_stof_u64;
          do_convert = true;
        } else if (addrSpace == Subtarget->getGroupAS()) {
          Opc = HSAIL::group_stof_u64;
          do_convert = true;
        } else if (addrSpace == Subtarget->getPrivateAS()) {
          Opc = HSAIL::private_stof_u64;
          do_convert = true;
        } else {
          assert(!"Unknown AddrSpace qualifier on pointer");
        }
      } else {
        assert(!"Unknown load parameter opcode");
      }

      if (do_convert) {
        // ld Vreg3,[Vreg2]
        MI->getOperand(0).setReg(Vreg3);
        // stof orig_reg,Vreg3
        MachineInstr *MI_stof = BuildMI(*MBB, ++MBIinsertPoint, DL, TII->get(Opc))
            .addReg(SavedTargetReg, RegState::Define)
            .addReg(Vreg3);
      }
      return true;
    }
  }
  return false;
}

// HandleLDAConversion -- convert pointers loaded with LDA to flat addresses
//
bool
HSAILTargetLowering::HandleLDAConversion(MachineInstr *MI,
    MachineBasicBlock *MBB) const
{
  const TargetInstrInfo *TII = getTargetMachine().getInstrInfo();
  const TargetRegisterInfo *TRI = getTargetMachine().getRegisterInfo();
  MachineFunction *MF = MBB->getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  MachineBasicBlock::iterator MBI, MBIE, MBIinsertPoint;
  DebugLoc  DL = MI->getDebugLoc();

  // Insertion points for new instructions
  for (MBI=MBB->begin(), MBIE=MBB->end(); MBI != MBIE; ++MBI ) {
      if (MI == *&MBI) {
         MBIinsertPoint = MBI;
         break;
      }
  }
  // Handle LDA flat cases, these are by definition pointer loads.
  // ldas x,[&addr] ==> ldas y,[&addr]; stof_<seg>_<type> x,y
  if (HSAIL_IS_LDA_FLAT(MI)) {
    unsigned ldareg = MI->getOperand(0).getReg();
    assert(MI->getOperand(0).isReg() && MI->getOperand(0).isDef());
    const TargetRegisterClass *rc = MRI.getRegClass(ldareg);
    unsigned NewVreg = MRI.createVirtualRegister(rc);
    int Opc;

    switch (MI->getOpcode()) {
    case HSAIL::ldas_global_flat_addr32:
      Opc = HSAIL::global_stof_u32;
      break;
    case HSAIL::ldas_group_flat_addr32:
      Opc = HSAIL::group_stof_u32;
      break;
    case HSAIL::ldas_global_flat_addr64:
      Opc = HSAIL::global_stof_u64;
      break;
    case HSAIL::ldas_group_flat_addr64:
      Opc = HSAIL::group_stof_u64;
      break;
    default:
      assert(!"unknown lda flat instruction");
      break;
    }
    MI->getOperand(0).substVirtReg(NewVreg, 0, *TRI);
    MachineInstr *MI_stof = BuildMI(*MF, DL, TII->get(Opc), ldareg).addReg(NewVreg);
    MBB->insertAfter(MBIinsertPoint, MI_stof);
    return true;
  }
  return false;
}

// EmitInstrWithCustomInserter - This method should be implemented by targets
// that mark instructions with the 'usesCustomInserter' flag.  These
// instructions are special in various ways, which require special support to
// insert.  The specified MachineInstr is created but not inserted into any
// basic blocks, and this method is called to expand it into a sequence of
// instructions, potentially also creating new basic blocks and control flow.
MachineBasicBlock*
HSAILTargetLowering::EmitInstrWithCustomInserter(MachineInstr *MI,
                                                 MachineBasicBlock *MBB) const
{
  const TargetInstrInfo *TII = getTargetMachine().getInstrInfo();
  const TargetRegisterInfo *TRI = getTargetMachine().getRegisterInfo();
  DebugLoc dl = MI->getDebugLoc();
  MachineFunction *MF = MBB->getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  MachineBasicBlock::iterator MBI, MBIE, MBIinsertPoint;

  // Insertion points for new instructions
  for (MBI=MBB->begin(), MBIE=MBB->end(); MBI != MBIE; ++MBI ) {
      if (MI == *&MBI) {
         MBIinsertPoint = MBI;
         break;
      }
  }

// TODO_HSA: CustomInserter should be useful for other cases.
// Need a way to disambiguate how usesCustomInserter is used in
// Table Gen.

  if (HandleIncomingArgumentConversion(MI, MBB))
    return MBB;
  if (HandleLDAConversion(MI,MBB))
    return MBB;

  // Handle load/stores, check if loading/storing pointers
  // Examine the memory operands of the instruction
  for (MachineInstr::mmo_iterator I = MI->memoperands_begin();
      I != MI->memoperands_end(); ++I) {
    MachineMemOperand *MMO = (*I);
    const Value *V = MMO->getValue(); // This is the mem operand pointer info
    // The type the pointer points to...
    PointerType *PT =  dyn_cast<PointerType>(V->getType()->getContainedType(0));

    // Loads, if the loaded value (the pointee) is a pointer,
    // do a segment-flat conversion on it.
    // ld x,[mem] ==> ld y,[mem]; stof x,y
    if (MMO->isLoad()) {
      // Is it a segment pointer? If so, convert it.
      if (PT) {
        unsigned addrSpace = PT->getAddressSpace();
        // New vreg for conversion destination
        unsigned lddestreg = MI->getOperand(0).getReg();
        assert(MI->getOperand(0).isReg() && MI->getOperand(0).isDef());
        const TargetRegisterClass *rc = MRI.getRegClass(lddestreg);
        unsigned NewVreg = MRI.createVirtualRegister(rc);
        int Opc;
        bool do_convert=true;

        if (addrSpace == Subtarget->getFlatAS()) {
          // This is a flat address, no conversion required
          do_convert=false;
        } else if (addrSpace == Subtarget->getGlobalAS()) {
          // Insert conversion after load
          Opc = Subtarget->is64Bit() ?
              HSAIL::global_stof_u64 : HSAIL::global_stof_u32;
        } else if (addrSpace == Subtarget->getConstantAS()) {
          // Insert conversion after load
          Opc = Subtarget->is64Bit() ?
              HSAIL::constant_stof_u64 : HSAIL::constant_stof_u32;
        } else if (addrSpace == Subtarget->getGroupAS()) {
          // Insert conversion after load
          Opc = Subtarget->is64Bit() ?
              HSAIL::group_stof_u64 : HSAIL::group_stof_u32;
        } else if (addrSpace == Subtarget->getPrivateAS()) {
          // Insert conversion after load
          Opc = Subtarget->is64Bit() ?
              HSAIL::private_stof_u64 : HSAIL::private_stof_u32;
        } else {
          assert(!"Unknown AddrSpace qualifier on pointer");
        }
        // ld x,[mem] ==> ld newvreg,[mem]; stof x,newvreg
        if (do_convert) {
          MI->getOperand(0).substVirtReg(NewVreg, 0, *TRI);
          MachineInstr *MI_stof = BuildMI(*MF, dl, TII->get(Opc), lddestreg).addReg(NewVreg);
          MBB->insertAfter(MBIinsertPoint, MI_stof);
        }
      }
    }
    // st x,[mem] ==> ftos y,x; st y,[mem]
    else if (MMO->isStore()) { // For stores, insert ftos before MI if necessary
      if (PT) {
        unsigned addrSpace = PT->getAddressSpace();
        // New vreg for conversion destination
        unsigned streg = MI->getOperand(0).getReg();
        assert(MI->getOperand(0).isReg() && MI->getOperand(0).isUse());
        const TargetRegisterClass *rc = MRI.getRegClass(streg);
        unsigned NewVreg = MRI.createVirtualRegister(rc);
        int Opc;
        bool do_convert=true;
        if (addrSpace == Subtarget->getFlatAS()) {
          // This is a flat address, no conversion required
          do_convert = false;
        } else if (addrSpace == Subtarget->getGlobalAS()) {
          Opc = Subtarget->is64Bit() ?
              HSAIL::global_ftos_u64 : HSAIL::global_ftos_u32;
        } else if (addrSpace == Subtarget->getConstantAS()) {
          Opc = Subtarget->is64Bit() ?
              HSAIL::constant_ftos_u64 : HSAIL::constant_ftos_u32;
        } else if (addrSpace == Subtarget->getGroupAS()) {
          Opc = Subtarget->is64Bit() ?
              HSAIL::group_ftos_u64 : HSAIL::group_ftos_u32;
        } else if (addrSpace == Subtarget->getPrivateAS()) {
          Opc = Subtarget->is64Bit() ?
              HSAIL::private_ftos_u64 : HSAIL::private_ftos_u32;
        } else {
          assert(!"Unknown AddrSpace qualifier on pointer");
        }
        if (do_convert) {
          // st x,[mem] ==> ftos y,x; st y,[mem]
          MachineInstr *MI_ftos = BuildMI(*MF, dl, TII->get(Opc), NewVreg).addReg(streg);
          MBB->insert(MBIinsertPoint, MI_ftos);
          MI->getOperand(0).substVirtReg(NewVreg, 0, *TRI);
        }
      }
    } else {
      assert(!"Flat memory conversion error, must be a load or store");
    }
  } // For each memory operand
  // New block not required
  return MBB;
}
//===--------------------------------------------------------------------===//
// Addressing mode description hooks (used by LSR etc).
//
/// isLegalAddressingMode - Return true if the addressing mode represented by
/// AM is legal for this target, for a load/store of the specified type.
/// The type may be VoidTy, in which case only return true if the addressing
/// mode is legal for a load/store of any legal type.
/// TODO: Handle pre/postinc as well.
bool
HSAILTargetLowering::isLegalAddressingMode(const AddrMode &AM,
                                           Type *Ty) const
{
  if (OptimizeFor == SI)
  {
    // Do not generate negative offsets as they can not be folded
    // into instruction
    if (AM.BaseOffs < 0 ||
        AM.Scale < 0)
    {
      return false;
    }
  }
  
  return TargetLowering::isLegalAddressingMode(AM, Ty);
}
/// isTruncateFree - Return true if it's free to truncate a value of
/// type Ty1 to type Ty2. e.g. On x86 it's free to truncate a i32 value in
/// register EAX to i16 by referencing its sub-register AX.
bool
HSAILTargetLowering::isTruncateFree(Type *Ty1, Type *Ty2) const
{
  return TargetLowering::isTruncateFree(Ty1, Ty2);
}

bool
HSAILTargetLowering::isTruncateFree(EVT VT1, EVT VT2) const
{
  return TargetLowering::isTruncateFree(VT1, VT2);
}
/// isZExtFree - Return true if any actual instruction that defines a
/// value of type Ty1 implicitly zero-extends the value to Ty2 in the result
/// register. This does not necessarily include registers defined in
/// unknown ways, such as incoming arguments, or copies from unknown
/// registers. Also, if isTruncateFree(Ty2, Ty1) is true, this
/// does not necessarily apply to truncate instructions. e.g. on x86-64,
/// all instructions that define 32-bit values implicit zero-extend the
/// result out to 64 bits.
bool
HSAILTargetLowering::isZExtFree(const Type *Ty1, const Type *Ty2) const
{
  return false;
}

bool
HSAILTargetLowering::isZExtFree(EVT VT1, EVT VT2) const
{
  return false;
}

/// isNarrowingProfitable - Return true if it's profitable to narrow
/// operations of type VT1 to VT2. e.g. on x86, it's profitable to narrow
/// from i32 to i8 but not from i32 to i16.
bool
HSAILTargetLowering::isNarrowingProfitable(EVT VT1, EVT VT2) const
{
  // This is only profitable in HSAIL to go from a 64bit type to
  // a 32bit type, but not to a 8 or 16bit type.
  return (VT1 == EVT(MVT::i64) && VT2 == EVT(MVT::i32))
    || (VT1 == EVT(MVT::f64) && VT2 == EVT(MVT::f32));
}

/// isLegalICmpImmediate - Return true if the specified immediate is legal
/// icmp immediate, that is the target has icmp instructions which can compare
/// a register against the immediate without having to materialize the
/// immediate into a register.
bool
HSAILTargetLowering::isLegalICmpImmediate(int64_t Imm) const
{
  // HSAIL doesn't have any restrictions on this.
  return true;
}

MVT
HSAILTargetLowering::getShiftAmountTy(EVT LHSTy) const
{
  // Shift amounts in registers must be in S registers
  // Restrict shift amount to 32-bits.
  return MVT::getIntegerVT(32);
}

void
HSAILTargetLowering::AdjustInstrPostInstrSelection(MachineInstr *MI, SDNode *Node) const
{
  if (hasParametrizedAtomicNoRetVersion(MI, Node))
  {
    if (Node->use_size() <= 1)
    {
      DEBUG(dbgs() << "Replacing atomic ");
      DEBUG(MI->dump());
      DEBUG(dbgs() << " with no return version ");
      const TargetInstrInfo *TII = MI->getParent()->getParent()->getTarget().getInstrInfo();
      // Hack: _noret opcode is always next to corresponding atomic ret opcode in td file
      MI->setDesc(TII->get(MI->getOpcode() + 1));
      MI->RemoveOperand(0);
      DEBUG(MI->dump());
      DEBUG(dbgs() << '\n');
    }
  }
}

bool
HSAILTargetLowering::isLoadBitCastBeneficial(EVT lVT, EVT bVT) const
{
  return !(lVT.getSizeInBits() == bVT.getSizeInBits()
      && lVT.getScalarType().getSizeInBits() > bVT.getScalarType().getSizeInBits()
      && bVT.getScalarType().getSizeInBits() < 32
      && lVT.getScalarType().getSizeInBits() >= 32);
}
