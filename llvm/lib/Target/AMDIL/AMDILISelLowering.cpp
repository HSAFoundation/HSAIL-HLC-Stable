//===-- AMDILISelLowering.cpp - AMDIL DAG Lowering Implementation ---------===//
// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// If you use the software (in whole or in part), you shall adhere to all
// applicable U.S., European, and other export laws, including but not limited
// to the U.S. Export Administration Regulations (EAR), (15 C.F.R. Sections
// 730 through 774), and E.U. Council Regulation (EC) No 1334/2000 of 22 June
// 2000.  Further, pursuant to Section 740.6 of the EAR, you hereby certify
// that, except pursuant to a license granted by the United States Department
// of Commerce Bureau of Industry and Security or as otherwise permitted
// pursuant to a License Exception under the U.S. Export Administration
// Regulations ("EAR"), you will not (1) export, re-export or release to a
// national of a country in Country Groups D:1, E:1 or E:2 any restricted
// technology, software, or source code you receive hereunder, or (2) export to
// Country Groups D:1, E:1 or E:2 the direct product of such technology or
// software, if such foreign produced direct product is subject to national
// security controls as identified on the Commerce Control List (currently
// found in Supplement 1 to Part 774 of EAR).  For the most current Country
// Group listings, or for additional information about the EAR or your
// obligations under those regulations, please refer to the U.S. Bureau of
// Industry and Securitys website at http://www.bis.doc.gov/.
//
//==-----------------------------------------------------------------------===//
//
// This file implements the interfaces that AMDIL uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "AMDILISelLowering.h"
#include "AMDIL.h"
#include "AMDILIntrinsicInfo.h"
#ifdef USE_APPLE
#include "AMDILLLVMApple.h"
#else
#include "AMDILLLVMPC.h"
#endif
#include "AMDILMachineFunctionInfo.h"
#include "AMDILModuleInfo.h"
#include "AMDILSubtarget.h"
#include "AMDILTargetMachine.h"
#include "AMDILUtilityFunctions.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/CallingConv.h"
#include "llvm/DerivedTypes.h"
#include "llvm/GlobalValue.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Intrinsics.h"
#include "llvm/Instructions.h"
#include "llvm/CodeGen/Analysis.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Target/TargetOptions.h"
#ifndef USE_APPLE
#include "../../CodeGen/SelectionDAG/SDNodeDbgValue.h"
#endif
#include "../../Transforms/IPO/AMDSymbolName.h"
using namespace llvm;

//===----------------------------------------------------------------------===//
// Calling Convention Implementation
//===----------------------------------------------------------------------===//
#include "AMDILGenCallingConv.inc"

//===----------------------------------------------------------------------===//
// TargetLowering Implementation Help Functions Begin
//===----------------------------------------------------------------------===//

static EVT getIntegerType(LLVMContext &Context, EVT VT) {
  if (VT.isVector())
    return VT.changeVectorElementTypeToInteger();
  return EVT::getIntegerVT(Context, VT.getSizeInBits());
}

static MVT getInt32VT(MVT VT) {
  return VT.isVector()
    ? MVT::getVectorVT(MVT::i32, VT.getVectorNumElements())
    : MVT::i32;
}

// Find a larger type to do a load / store of a vector with.
static EVT getEquivalentMemType(LLVMContext &Ctx, EVT VT) {
  unsigned StoreSize = VT.getStoreSizeInBits();
  if (StoreSize <= 32)
    return EVT::getIntegerVT(Ctx, StoreSize);

  assert(StoreSize % 32 == 0 && "Store size not a multiple of 32");

  return EVT::getVectorVT(Ctx, MVT::i32, StoreSize / 32);
}

// Type for a vector that will be loaded to.
static EVT getEquivalentLoadRegType(LLVMContext &Ctx, EVT VT) {
  unsigned StoreSize = VT.getStoreSizeInBits();
  if (StoreSize <= 32)
    return MVT::getIntegerVT(32);

  return MVT::getVectorVT(MVT::i32, StoreSize / 32);
}

SDValue
AMDILTargetLowering::LowerMemArgument(
    SDValue Chain,
    CallingConv::ID CallConv,
    const SmallVectorImpl<ISD::InputArg> &Ins,
    DebugLoc dl, SelectionDAG &DAG,
    const CCValAssign &VA,
    MachineFrameInfo *MFI,
    unsigned i) const
{
  // Create the nodes corresponding to a load from this parameter slot.
  ISD::ArgFlagsTy Flags = Ins[i].Flags;

  bool AlwaysUseMutable = (CallConv==CallingConv::Fast) &&
    getTargetMachine().Options.GuaranteedTailCallOpt;
  bool isImmutable = !AlwaysUseMutable && !Flags.isByVal();

  // FIXME: For now, all byval parameter objects are marked mutable. This can
  // be changed with more analysis.
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
      false, false, false, 0);
}
//===----------------------------------------------------------------------===//
// TargetLowering Implementation Help Functions End
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// TargetLowering Class Implementation Begins
//===----------------------------------------------------------------------===//

void AMDILTargetLowering::setFloatCondCodeActions(MVT::SimpleValueType VT) {
  // Ordered comparions are legal. Match undefined NAN compares to the ordered,
  // and expand unordered. GE and LE are missing for some reason.
  setCondCodeAction(ISD::SETOEQ, VT, Legal);
  setCondCodeAction(ISD::SETOGT, VT, Expand);
  setCondCodeAction(ISD::SETOGE, VT, Legal);
  setCondCodeAction(ISD::SETOLT, VT, Legal);
  setCondCodeAction(ISD::SETOLE, VT, Expand);

  // FIXME: I think ne / dne are really supposed to be ordered from the
  // description and it would only be consistent, but seems to be getting true
  // for one nan operand instead of false, so for now match setune to ne / dne
  // since that's what was passing before. Possibly an SC bug.
  setCondCodeAction(ISD::SETONE, VT, Expand);

  setCondCodeAction(ISD::SETEQ, VT, Legal);
  setCondCodeAction(ISD::SETGT, VT, Expand);
  setCondCodeAction(ISD::SETGE, VT, Legal);
  setCondCodeAction(ISD::SETLT, VT, Legal);
  setCondCodeAction(ISD::SETLE, VT, Expand);
  setCondCodeAction(ISD::SETNE, VT, Legal);

  setCondCodeAction(ISD::SETO, VT, Expand);
  setCondCodeAction(ISD::SETUO, VT, Expand);
  setCondCodeAction(ISD::SETUEQ, VT, Expand);
  setCondCodeAction(ISD::SETUGT, VT, Expand);
  setCondCodeAction(ISD::SETUGE, VT, Expand);
  setCondCodeAction(ISD::SETULT, VT, Expand);
  setCondCodeAction(ISD::SETULE, VT, Expand);
  setCondCodeAction(ISD::SETUNE, VT, Legal);
}

void AMDILTargetLowering::setIntCondCodeActions(MVT::SimpleValueType VT) {
  setCondCodeAction(ISD::SETEQ, VT, Legal);
  setCondCodeAction(ISD::SETNE, VT, Legal);
  setCondCodeAction(ISD::SETLT, VT, Legal);
  setCondCodeAction(ISD::SETLE, VT, Expand);
  setCondCodeAction(ISD::SETGT, VT, Expand);
  setCondCodeAction(ISD::SETGE, VT, Legal);

  setCondCodeAction(ISD::SETULT, VT, Legal);
  setCondCodeAction(ISD::SETULE, VT, Expand);
  setCondCodeAction(ISD::SETUGT, VT, Expand);
  setCondCodeAction(ISD::SETUGE, VT, Legal);
}

AMDILTargetLowering::AMDILTargetLowering(TargetMachine &TM)
  : TargetLowering(TM, new TargetLoweringObjectFileELF()),
    Subtarget(TM.getSubtarget<AMDILSubtarget>()) {
  setBooleanVectorContents(ZeroOrNegativeOneBooleanContent);
  setBooleanContents(ZeroOrNegativeOneBooleanContent);

  MVT::SimpleValueType Types[] = {
    MVT::i32,
    MVT::f32,
    MVT::f64,
    MVT::i64,
    MVT::v4f32,
    MVT::v4i32,
    MVT::v2f32,
    MVT::v2i32,
    MVT::v2f64,
    MVT::v2i64
  };

  MVT::SimpleValueType IntTypes[] = {
    MVT::i32,
    MVT::i64
  };

  MVT::SimpleValueType FloatTypes[] = {
    MVT::f32,
    MVT::f64
  };

  MVT::SimpleValueType VectorTypes[] = {
    MVT::v4f32,
    MVT::v4i32,
    MVT::v2f32,
    MVT::v2i32,
    MVT::v2f64,
    MVT::v2i64
  };

  const AMDILSubtarget *stm = &getTargetMachine().getSubtarget<AMDILSubtarget>();

  // These are the current register classes that are
  // supported.
  addRegisterClass(MVT::i32, &AMDIL::GPR_32RegClass);
  addRegisterClass(MVT::f32, &AMDIL::GPR_32RegClass);

  if (stm->isSupported(AMDIL::Caps::DoubleOps)) {
    addRegisterClass(MVT::f64, &AMDIL::GPR_64RegClass);
    addRegisterClass(MVT::v2f64, &AMDIL::GPRV2I64RegClass);
  }

  addRegisterClass(MVT::v2f32, &AMDIL::GPRV2I32RegClass);
  addRegisterClass(MVT::v4f32, &AMDIL::GPRV4I32RegClass);
  addRegisterClass(MVT::v2i32, &AMDIL::GPRV2I32RegClass);
  addRegisterClass(MVT::v4i32, &AMDIL::GPRV4I32RegClass);

  if (stm->isSupported(AMDIL::Caps::LongOps)) {
    addRegisterClass(MVT::i64, &AMDIL::GPR_64RegClass);
    addRegisterClass(MVT::v2i64, &AMDIL::GPRV2I64RegClass);
  }

  // Make some ops legal since the "generic" target lowering made them expand
  // (See lib/CodeGen/SelectionDag/TargetLowering.cpp)
  setOperationAction(ISD::FTRUNC, MVT::f32, Legal);
  setOperationAction(ISD::FNEARBYINT, MVT::f32, Legal);
  setOperationAction(ISD::FCEIL,  MVT::f32, Legal);
  setOperationAction(ISD::FLOG ,  MVT::f32, Legal);
  // Set explicitly to expand in case default changes
  setOperationAction(ISD::FRINT,  MVT::f32, Expand);
  setOperationAction(ISD::OR,   MVT::i32, Custom);
  setOperationAction(ISD::OR, MVT::v2i32, Custom);
  setOperationAction(ISD::OR, MVT::v4i32, Custom);

  // Workaround documented in DAGCombiner::visitSELECT
  // Avoid turning the (select setcc ..) back into select_cc
  setOperationAction(ISD::SELECT_CC, MVT::Other, Expand);

  /*
  setOperationAction(ISD::BITCAST, MVT::i16, Custom);
  setOperationAction(ISD::BITCAST, MVT::v2i16, Custom);
  setOperationAction(ISD::BITCAST, MVT::v4i16, Custom);
  setOperationAction(ISD::BITCAST, MVT::v2i8, Custom);
  setOperationAction(ISD::BITCAST, MVT::v4i8, Custom);
  */

  for (unsigned int i = 0; i < array_lengthof(Types); ++i) {
    MVT VT(Types[i]);

    setOperationAction(ISD::EXTRACT_SUBVECTOR, VT, Custom);
    setOperationAction(ISD::FP_ROUND, VT, Expand);
    setOperationAction(ISD::SUBE, VT, Expand);
    setOperationAction(ISD::SUBC, VT, Expand);
    setOperationAction(ISD::ADD, VT, Custom);
    setOperationAction(ISD::ADDE, VT, Expand);
    setOperationAction(ISD::ADDC, VT, Expand);
    setOperationAction(ISD::BR_JT, VT, Expand);
    // TODO: This should only be for integer/f64 types,
    // f32 types can you if_relop instruction.
    setOperationAction(ISD::BR_CC, VT, Expand);
    setOperationAction(ISD::BRIND, VT, Expand);

    // TODO: SELECT_CC should be a legal cmp_relop.
    setOperationAction(ISD::SELECT_CC, VT, Custom);
    // TODO: Implement custom UREM/SREM routines
    setOperationAction(ISD::UREM, VT, Expand);
    setOperationAction(ISD::SREM, VT, Expand);

    if (VT != MVT::i64 && VT != MVT::v2i64) {
      setOperationAction(ISD::SDIV, VT, Custom);
      setOperationAction(ISD::UDIV, VT, Custom);
    }

    setOperationAction(ISD::SINT_TO_FP, VT, Custom);
    setOperationAction(ISD::UINT_TO_FP, VT, Custom);
    setOperationAction(ISD::FP_TO_SINT, VT, Custom);
    setOperationAction(ISD::FP_TO_UINT, VT, Custom);

    // TODO: Can possibly get rid of custom lowering of bitcasts after removing
    // the fake legal integer types.
    //setOperationAction(ISD::BITCAST, VT, Custom);
    setOperationAction(ISD::GlobalAddress, VT, Custom);
    setOperationAction(ISD::JumpTable, VT, Custom);
    setOperationAction(ISD::ConstantPool, VT, Custom);
    setOperationAction(ISD::SMUL_LOHI, VT, Expand);
    setOperationAction(ISD::UMUL_LOHI, VT, Expand);
    /*

    for (unsigned y = 0; y < numTypes; ++y) {
      MVT::SimpleValueType DVT = (MVT::SimpleValueType)types[y];
      setTruncStoreAction(VT, DVT, Expand);
    }
    setLoadExtAction(ISD::SEXTLOAD, VT, Expand);
    setLoadExtAction(ISD::ZEXTLOAD, VT, Expand);
    setLoadExtAction(ISD::EXTLOAD, VT, Expand);
    */

    setOperationAction(ISD::INSERT_VECTOR_ELT, VT, Custom);
    setOperationAction(ISD::EXTRACT_VECTOR_ELT, VT, Custom);
  }

  setLoadExtAction(ISD::EXTLOAD, MVT::f64, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::f32, Expand);

  // FIXME: This hits an assertion in SelectionDAG. It seems to not handle the
  // case where the loaded type isn't legal.
  //setLoadExtAction(ISD::EXTLOAD, MVT::f16, Expand);

  for (int I = MVT::FIRST_FP_VECTOR_VALUETYPE;
       I <= MVT::LAST_FP_VECTOR_VALUETYPE; ++I) {
    MVT VT(static_cast<MVT::SimpleValueType>(I));
    setLoadExtAction(ISD::EXTLOAD, VT, Expand);
  }

  // Handling of i1 vectors seem to be broken in lots of places, so expand them.
  // FIXME: They still seem to be broken even with expand. What is the correct
  // behavior? Currently it seems to be packing the bits inside an i8, but
  // should it really be promoted to v2i8?

  setOperationAction(ISD::LOAD, MVT::v2i1, Expand);
  setOperationAction(ISD::LOAD, MVT::v4i1, Expand);

  setLoadExtAction(ISD::EXTLOAD, MVT::v2i1, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v2i1, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v2i1, Expand);

  setLoadExtAction(ISD::EXTLOAD, MVT::v4i1, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v4i1, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v4i1, Expand);

  setLoadExtAction(ISD::EXTLOAD, MVT::v2i8, Custom);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v2i8, Custom);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v2i8, Custom);

  setLoadExtAction(ISD::EXTLOAD, MVT::v4i8, Custom);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v4i8, Custom);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v4i8, Custom);

  setLoadExtAction(ISD::EXTLOAD, MVT::v2i16, Custom);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v2i16, Custom);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v2i16, Custom);

  setLoadExtAction(ISD::EXTLOAD, MVT::v4i16, Custom);
  setLoadExtAction(ISD::SEXTLOAD, MVT::v4i16, Custom);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v4i16, Custom);

  setTruncStoreAction(MVT::f64, MVT::f32, Expand);
  setTruncStoreAction(MVT::f64, MVT::f16, Expand);
  setTruncStoreAction(MVT::f32, MVT::f16, Expand);


  setTruncStoreAction(MVT::v2i32, MVT::v2i1, Expand);
  setTruncStoreAction(MVT::v4i32, MVT::v4i1, Expand);

  setTruncStoreAction(MVT::v2i32, MVT::v2i16, Custom);
  setTruncStoreAction(MVT::v2i32, MVT::v2i8, Custom);
  setTruncStoreAction(MVT::v4i32, MVT::v4i8, Custom);
  setTruncStoreAction(MVT::v4i32, MVT::v4i16, Custom);

  // TODO: Half op instructions provide these.
  //setOperationAction(ISD::FP_ROUND_INREG, MVT::f16, Promote);
  //setOperationAction(ISD::FP_ROUND, MVT::f16, Promote);
  //AddPromotedToType(ISD::FP_ROUND, MVT::f16, MVT::f32);

  setOperationAction(ISD::FP_EXTEND, MVT::v2f16, Expand);
  setOperationAction(ISD::FP_EXTEND, MVT::v2f32, Expand);

  // We need to handle these cases for constant buffers / kernel arguments.
  setOperationAction(ISD::LOAD, MVT::f32, Promote);
  AddPromotedToType(ISD::LOAD, MVT::f32, MVT::i32);

  setOperationAction(ISD::LOAD, MVT::v2f32, Promote);
  AddPromotedToType(ISD::LOAD, MVT::v2f32, MVT::v2i32);

  setOperationAction(ISD::LOAD, MVT::v4f32, Promote);
  AddPromotedToType(ISD::LOAD, MVT::v4f32, MVT::v4i32);

  setOperationAction(ISD::LOAD, MVT::v4f64, Custom);
  setOperationAction(ISD::STORE, MVT::v4f64, Custom);
  setOperationAction(ISD::LOAD, MVT::v4i64, Custom);
  setOperationAction(ISD::STORE, MVT::v4i64, Custom);

  setOperationAction(ISD::LOAD, MVT::f64, Promote);
  AddPromotedToType(ISD::LOAD, MVT::f64, MVT::i64);

  setOperationAction(ISD::LOAD, MVT::v2f64, Promote);
  AddPromotedToType(ISD::LOAD, MVT::v2f64, MVT::v2i64);

  setOperationAction(ISD::LOAD, MVT::i32, Custom);
  setOperationAction(ISD::LOAD, MVT::v2i32, Custom);
  setOperationAction(ISD::LOAD, MVT::v4i32, Custom);
  setOperationAction(ISD::LOAD, MVT::v8i32, Custom);
  setOperationAction(ISD::LOAD, MVT::v16i32, Custom);

  setOperationAction(ISD::LOAD, MVT::i64, Custom);
  setOperationAction(ISD::LOAD, MVT::v2i64, Custom);
  setOperationAction(ISD::LOAD, MVT::v4i64, Custom);
  setOperationAction(ISD::LOAD, MVT::v8i64, Custom);
  setOperationAction(ISD::LOAD, MVT::v16i64, Custom);

  setLoadExtAction(ISD::SEXTLOAD, MVT::i8, Custom);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i8, Custom);
  setLoadExtAction(ISD::EXTLOAD, MVT::i8, Custom);

  setLoadExtAction(ISD::SEXTLOAD, MVT::i16, Custom);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i16, Custom);
  setLoadExtAction(ISD::EXTLOAD, MVT::i16, Custom);


  setLoadExtAction(ISD::SEXTLOAD, MVT::v8i16, Custom);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::v8i16, Custom);
  setLoadExtAction(ISD::EXTLOAD, MVT::v8i16, Custom);

  setLoadExtAction(ISD::SEXTLOAD, MVT::i32, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i32, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::i32, Expand);

  for (int I = MVT::FIRST_INTEGER_VECTOR_VALUETYPE;
       I <= MVT::LAST_INTEGER_VECTOR_VALUETYPE; ++I) {
    MVT::SimpleValueType VT = static_cast<MVT::SimpleValueType>(I);
    setIntCondCodeActions(VT);
  }

  for (int I = MVT::FIRST_INTEGER_VALUETYPE;
       I <= MVT::LAST_INTEGER_VALUETYPE; ++I) {
    MVT::SimpleValueType VT = static_cast<MVT::SimpleValueType>(I);
    setIntCondCodeActions(VT);
  }

  for (unsigned I = 0; I < array_lengthof(FloatTypes); ++I) {
    MVT VT(FloatTypes[I]);
    // IL does not have these operations for floating point types
    setOperationAction(ISD::FP_ROUND_INREG, VT, Expand);
    setOperationAction(ISD::FP_ROUND, VT, Custom);
  }

  for (int I = MVT::FIRST_FP_VALUETYPE;
       I <= MVT::LAST_FP_VALUETYPE; ++I) {
    setFloatCondCodeActions(static_cast<MVT::SimpleValueType>(I));
  }

  for (int I = MVT::FIRST_FP_VECTOR_VALUETYPE;
       I <= MVT::LAST_FP_VECTOR_VALUETYPE; ++I) {
    setFloatCondCodeActions(static_cast<MVT::SimpleValueType>(I));
  }

  for (unsigned int i = 0; i < array_lengthof(IntTypes); ++i) {
    MVT VT(IntTypes[i]);

    // GPU also does not have divrem function for signed or unsigned
    setOperationAction(ISD::SDIVREM, VT, Expand);
    setOperationAction(ISD::UDIVREM, VT, Expand);
    setOperationAction(ISD::FP_ROUND, VT, Expand);

    // GPU does not have [S|U]MUL_LOHI functions as a single instruction
    setOperationAction(ISD::SMUL_LOHI, VT, Expand);
    setOperationAction(ISD::UMUL_LOHI, VT, Expand);

    // GPU doesn't have a rotl, rotr, or byteswap instruction
    setOperationAction(ISD::ROTR, VT, Expand);
    setOperationAction(ISD::ROTL, VT, Expand);
    setOperationAction(ISD::BSWAP, VT, Expand);

    // GPU doesn't have any counting operators
    setOperationAction(ISD::CTPOP, VT, Expand);
    setOperationAction(ISD::CTTZ, VT, Expand);
    setOperationAction(ISD::CTLZ, VT, Expand);
  }

  for (unsigned int i = 0; i < array_lengthof(VectorTypes); ++i) {
    MVT VT(VectorTypes[i]);

    setOperationAction(ISD::BUILD_VECTOR, VT, Custom);
    setOperationAction(ISD::EXTRACT_SUBVECTOR, VT, Custom);
    setOperationAction(ISD::SCALAR_TO_VECTOR, VT, Custom);
    setOperationAction(ISD::VECTOR_SHUFFLE, VT, Expand);
    setOperationAction(ISD::CONCAT_VECTORS, VT, Custom);
    setOperationAction(ISD::FP_ROUND, VT, Expand);
    setOperationAction(ISD::SDIVREM, VT, Expand);
    setOperationAction(ISD::UDIVREM, VT, Expand);
    setOperationAction(ISD::SMUL_LOHI, VT, Expand);
    setOperationAction(ISD::SELECT, VT, Expand);
  }

  setOperationAction(ISD::VSELECT, MVT::i32, Legal);
  setOperationAction(ISD::VSELECT, MVT::v2i32, Legal);
  setOperationAction(ISD::VSELECT, MVT::v4i32, Legal);

  setOperationAction(ISD::VSELECT, MVT::f32, Legal);
  setOperationAction(ISD::VSELECT, MVT::v2f32, Legal);
  setOperationAction(ISD::VSELECT, MVT::v4f32, Legal);

  setOperationAction(ISD::VSELECT, MVT::v2i64, Legal);
  setOperationAction(ISD::VSELECT, MVT::v2f64, Legal);

  setOperationAction(ISD::SELECT, MVT::i32, Legal);
  setOperationAction(ISD::SELECT, MVT::f32, Legal);

  // FIXME: Scalar select on vectors could be handled better. Currently you get
  // a cmov_logical cond, -1, 0, and then do some bitwise operations with the
  // resulting mask on the vector. We could instead broadcast the compare
  // result, and still use a cmov_logical to select all components.
  // setOperationAction(ISD::SELECT, MVT::v2i32, Legal);
  // setOperationAction(ISD::SELECT, MVT::v4i32, Legal);
  // setOperationAction(ISD::SELECT, MVT::v2f32, Legal);
  // setOperationAction(ISD::SELECT, MVT::v4f32, Legal);
  // setOperationAction(ISD::SELECT, MVT::v2i64, Legal);
  // setOperationAction(ISD::SELECT, MVT::v2f64, Legal);

  setOperationAction(ISD::FP_ROUND, MVT::Other, Expand);
  if (stm->isSupported(AMDIL::Caps::LongOps)) {
    setOperationAction(ISD::SRL, MVT::v2i64, Expand);
    setOperationAction(ISD::SRA, MVT::v2i64, Expand);
    setOperationAction(ISD::SHL, MVT::v2i64, Expand);
    setOperationAction(ISD::SUB, MVT::i64, Custom);
    setOperationAction(ISD::ADD, MVT::i64, Custom);
    setOperationAction(ISD::MULHU, MVT::i64, Expand);
    setOperationAction(ISD::MULHU, MVT::v2i64, Expand);
    setOperationAction(ISD::MULHS, MVT::i64, Expand);
    setOperationAction(ISD::MULHS, MVT::v2i64, Expand);
    setOperationAction(ISD::MUL, MVT::v2i64, Expand);
    setOperationAction(ISD::SUB, MVT::v2i64, Expand);
    setOperationAction(ISD::SRL, MVT::v2i64, Expand);
    setOperationAction(ISD::SRA, MVT::v2i64, Expand);
    setOperationAction(ISD::SHL, MVT::v2i64, Expand);
    setOperationAction(ISD::ADD, MVT::v2i64, Expand);
    setOperationAction(ISD::SREM, MVT::v2i64, Expand);
    setOperationAction(ISD::Constant          , MVT::i64  , Legal);
    setOperationAction(ISD::UDIV, MVT::v2i64, Expand);
    setOperationAction(ISD::SDIV, MVT::v2i64, Expand);
    setOperationAction(ISD::FP_TO_SINT, MVT::v2i64, Expand);
    setOperationAction(ISD::TRUNCATE, MVT::v2i64, Expand);
    setOperationAction(ISD::SIGN_EXTEND, MVT::v2i64, Expand);
    setOperationAction(ISD::ZERO_EXTEND, MVT::v2i64, Expand);
    setOperationAction(ISD::ANY_EXTEND, MVT::v2i64, Expand);
    setOperationAction(ISD::SETCC, MVT::v2i64, Custom);
  }

  if (stm->isSupported(AMDIL::Caps::DoubleOps)) {
    // we support loading/storing v2f64 but not operations on the type
    setOperationAction(ISD::FADD, MVT::v2f64, Expand);
    setOperationAction(ISD::FSUB, MVT::v2f64, Expand);
    setOperationAction(ISD::FMUL, MVT::v2f64, Expand);
    setOperationAction(ISD::FP_ROUND, MVT::v2f64, Expand);
    setOperationAction(ISD::FP_ROUND_INREG, MVT::v2f64, Expand);
    setOperationAction(ISD::FP_EXTEND, MVT::v2f64, Expand);
    setOperationAction(ISD::ConstantFP        , MVT::f64  , Legal);
    setOperationAction(ISD::FDIV, MVT::v2f64, Expand);
    // We want to expand vector conversions into their scalar
    // counterparts.
    setOperationAction(ISD::SINT_TO_FP, MVT::v2f64, Expand);
    setOperationAction(ISD::UINT_TO_FP, MVT::v2f64, Expand);
    setOperationAction(ISD::FP_TO_SINT, MVT::v2f64, Expand);
    setOperationAction(ISD::FP_TO_UINT, MVT::v2f64, Expand);
    setOperationAction(ISD::TRUNCATE, MVT::v2f64, Expand);
    setOperationAction(ISD::SIGN_EXTEND, MVT::v2f64, Expand);
    setOperationAction(ISD::ZERO_EXTEND, MVT::v2f64, Expand);
    setOperationAction(ISD::ANY_EXTEND, MVT::v2f64, Expand);
    setOperationAction(ISD::FABS, MVT::f64, Legal);
    setOperationAction(ISD::FABS, MVT::v2f64, Expand);
    setOperationAction(ISD::SETCC, MVT::v2f64, Custom);
  }

  if (stm->hasShortOps()) {
    // TODO: There aren't really i16 registers, so this won't work. These should
    // be selected by patterns checking that only the low 16-bits are demanded.10
    setOperationAction(ISD::ADD, MVT::i16, Legal);
    setOperationAction(ISD::SUB, MVT::i16, Legal);
    setOperationAction(ISD::MUL, MVT::i16, Legal);
    setOperationAction(ISD::SHL, MVT::i16, Legal);
    setOperationAction(ISD::SRA, MVT::i16, Legal);
    setOperationAction(ISD::SRL, MVT::i16, Legal);
  }

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v2i1, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v4i1, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v2i8, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v4i8, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v2i16, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::v4i16, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i32, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::Other, Custom);

  setOperationAction(ISD::SUBC, MVT::Other, Expand);
  setOperationAction(ISD::ADDE, MVT::Other, Expand);
  setOperationAction(ISD::ADDC, MVT::Other, Expand);
  setOperationAction(ISD::BRCOND, MVT::Other, Custom);
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::BRIND, MVT::Other, Expand);
  setOperationAction(ISD::BR_CC, MVT::Other, Expand);
  setOperationAction(ISD::FDIV, MVT::f32, Custom);
  setOperationAction(ISD::FDIV, MVT::v2f32, Custom);
  setOperationAction(ISD::FDIV, MVT::v4f32, Custom);

  setOperationAction(ISD::BUILD_VECTOR, MVT::Other, Custom);
  // Use the default implementation.
  setOperationAction(ISD::VAARG             , MVT::Other, Expand);
  setOperationAction(ISD::VACOPY            , MVT::Other, Expand);
  setOperationAction(ISD::VAEND             , MVT::Other, Expand);
  setOperationAction(ISD::STACKSAVE         , MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE      , MVT::Other, Expand);
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32  , Custom);
  setOperationAction(ISD::ConstantFP        , MVT::f32    , Legal);
  setOperationAction(ISD::Constant          , MVT::i32    , Legal);
  setOperationAction(ISD::TRAP              , MVT::Other  , Legal);


  setOperationAction(ISD::INTRINSIC_WO_CHAIN, MVT::Other, Custom);

  // Target-specific dag combine patterns
  setTargetDAGCombine(ISD::FADD);
  setTargetDAGCombine(ISD::FSUB);
  setTargetDAGCombine(ISD::LOAD);
  setTargetDAGCombine(ISD::STORE);

  setStackPointerRegisterToSaveRestore(AMDIL::SP);
  setSchedulingPreference(Sched::RegPressure);
  setPow2DivIsCheap(false);
  setPrefLoopAlignment(16);
#ifndef USE_APPLE
  setSelectIsExpensive(true);
  setJumpIsExpensive(true);
#endif
  computeRegisterProperties();

  // The hardware has no true divide instruction.
  setIntDivIsCheap(false);

  maxStoresPerMemcpy  = 4096;
  maxStoresPerMemmove = 4096;
  maxStoresPerMemset  = 4096;
}

// TODO: Use one in SelectionDAG
static void ExtractVectorElements(SDValue Op, SelectionDAG &DAG,
                                  SmallVectorImpl<SDValue> &Args,
                                  unsigned Start,
                                  unsigned Count) {
  EVT VT = Op.getValueType();
  EVT EltTy = VT.getVectorElementType();
  DebugLoc DL = Op.getDebugLoc();
  for (unsigned i = Start, e = Start + Count; i != e; ++i) {
    Args.push_back(DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL,
                               EltTy,
                               Op, DAG.getConstant(i, MVT::i32)));
  }
}

// This only works for region/local/global address spaces on EG/NI as
// the other address spaces required 128 bit alignement of loads/stores.
// However, there is no way to disable for those address spaces
// and only for specific types.
// TODO: Modify this API call to pass in the address space/instruction
bool
AMDILTargetLowering::allowsUnalignedMemoryAccesses(EVT VT) const
{
  return (VT == MVT::v4f32 || VT == MVT::v4i32
      || VT == MVT::v2f32 || VT == MVT::v2i32
      || VT == MVT::f64   || VT == MVT::i64
      || VT == MVT::v2f64 || VT == MVT::v2i64);
}

const char *
AMDILTargetLowering::getTargetNodeName(unsigned Opcode) const
{
  switch (Opcode) {
    default: return 0;
    case AMDILISD::UBIT_EXTRACT: return "AMDILISD::UBIT_EXTRACT";
    case AMDILISD::IBIT_EXTRACT: return "AMDILISD::IBIT_EXTRACT";
    case AMDILISD::UBIT_INSERT: return "AMDILISD::UBIT_INSERT";
    case AMDILISD::BFI: return "AMDILISD::BFI";
    case AMDILISD::DP_TO_FP:  return "AMDILISD::DP_TO_FP";
    case AMDILISD::FP_TO_DP:  return "AMDILISD::FP_TO_DP";
    case AMDILISD::CALL:  return "AMDILISD::CALL";
    case AMDILISD::RET:   return "AMDILISD::RET";
    case AMDILISD::UMUL: return "AMDILISD::UMUL";
    case AMDILISD::VBUILD: return "AMDILISD::VBUILD";
    case AMDILISD::VEXTRACT: return "AMDILISD::VEXTRACT";
    case AMDILISD::VINSERT: return "AMDILISD::VINSERT";
    case AMDILISD::VCONCAT: return "AMDILISD::VCONCAT";
    case AMDILISD::LCREATE: return "AMDILISD::LCREATE";
    case AMDILISD::LCOMPHI: return "AMDILISD::LCOMPHI";
    case AMDILISD::LCOMPLO: return "AMDILISD::LCOMPLO";
    case AMDILISD::DCREATE: return "AMDILISD::DCREATE";
    case AMDILISD::DCOMPHI: return "AMDILISD::DCOMPHI";
    case AMDILISD::DCOMPLO: return "AMDILISD::DCOMPLO";
    case AMDILISD::LCREATE2: return "AMDILISD::LCREATE2";
    case AMDILISD::LCOMPHI2: return "AMDILISD::LCOMPHI2";
    case AMDILISD::LCOMPLO2: return "AMDILISD::LCOMPLO2";
    case AMDILISD::DCREATE2: return "AMDILISD::DCREATE2";
    case AMDILISD::DCOMPHI2: return "AMDILISD::DCOMPHI2";
    case AMDILISD::DCOMPLO2: return "AMDILISD::DCOMPLO2";
    case AMDILISD::RET_FLAG: return "AMDILISD::RET_FLAG";
    case AMDILISD::BRANCH_COND: return "AMDILISD::BRANCH_COND";
    case AMDILISD::ADDADDR: return "AMDILISD::ADDADDR";
    case AMDILISD::CONST_ADDRESS: return "AMDILISD::CONST_ADDRESS";
    case AMDILISD::ATOM_F_ADD: return "AMDILISD::ATOM_F_ADD";
    case AMDILISD::ATOM_F_AND: return "AMDILISD::ATOM_F_AND";
    case AMDILISD::ATOM_F_CMPXCHG: return "AMDILISD::ATOM_F_CMPXCHG";
    case AMDILISD::ATOM_F_DEC: return "AMDILISD::ATOM_F_DEC";
    case AMDILISD::ATOM_F_INC: return "AMDILISD::ATOM_F_INC";
    case AMDILISD::ATOM_F_MAX: return "AMDILISD::ATOM_F_MAX";
    case AMDILISD::ATOM_F_UMAX: return "AMDILISD::ATOM_F_UMAX";
    case AMDILISD::ATOM_F_MIN: return "AMDILISD::ATOM_F_MIN";
    case AMDILISD::ATOM_F_UMIN: return "AMDILISD::ATOM_F_UMIN";
    case AMDILISD::ATOM_F_OR: return "AMDILISD::ATOM_F_OR";
    case AMDILISD::ATOM_F_SUB: return "AMDILISD::ATOM_F_SUB";
    case AMDILISD::ATOM_F_XCHG: return "AMDILISD::ATOM_F_XCHG";
    case AMDILISD::ATOM_F_XOR: return "AMDILISD::ATOM_F_XOR";
    case AMDILISD::ATOM_G_ADD: return "AMDILISD::ATOM_G_ADD";
    case AMDILISD::ATOM_G_AND: return "AMDILISD::ATOM_G_AND";
    case AMDILISD::ATOM_G_CMPXCHG: return "AMDILISD::ATOM_G_CMPXCHG";
    case AMDILISD::ATOM_G_DEC: return "AMDILISD::ATOM_G_DEC";
    case AMDILISD::ATOM_G_INC: return "AMDILISD::ATOM_G_INC";
    case AMDILISD::ATOM_G_MAX: return "AMDILISD::ATOM_G_MAX";
    case AMDILISD::ATOM_G_UMAX: return "AMDILISD::ATOM_G_UMAX";
    case AMDILISD::ATOM_G_MIN: return "AMDILISD::ATOM_G_MIN";
    case AMDILISD::ATOM_G_UMIN: return "AMDILISD::ATOM_G_UMIN";
    case AMDILISD::ATOM_G_OR: return "AMDILISD::ATOM_G_OR";
    case AMDILISD::ATOM_G_SUB: return "AMDILISD::ATOM_G_SUB";
    case AMDILISD::ATOM_G_RSUB: return "AMDILISD::ATOM_G_RSUB";
    case AMDILISD::ATOM_G_XCHG: return "AMDILISD::ATOM_G_XCHG";
    case AMDILISD::ATOM_G_XOR: return "AMDILISD::ATOM_G_XOR";
    case AMDILISD::ATOM_G_STORE: return "AMDILISD::ATOM_G_STORE";
    case AMDILISD::ATOM_G_LOAD: return "AMDILISD::ATOM_G_LOAD";
    case AMDILISD::ATOM_G_ADD_NORET: return "AMDILISD::ATOM_G_ADD_NORET";
    case AMDILISD::ATOM_G_AND_NORET: return "AMDILISD::ATOM_G_AND_NORET";
    case AMDILISD::ATOM_G_CMPXCHG_NORET: return "AMDILISD::ATOM_G_CMPXCHG_NORET";
    case AMDILISD::ATOM_G_DEC_NORET: return "AMDILISD::ATOM_G_DEC_NORET";
    case AMDILISD::ATOM_G_INC_NORET: return "AMDILISD::ATOM_G_INC_NORET";
    case AMDILISD::ATOM_G_MAX_NORET: return "AMDILISD::ATOM_G_MAX_NORET";
    case AMDILISD::ATOM_G_UMAX_NORET: return "AMDILISD::ATOM_G_UMAX_NORET";
    case AMDILISD::ATOM_G_MIN_NORET: return "AMDILISD::ATOM_G_MIN_NORET";
    case AMDILISD::ATOM_G_UMIN_NORET: return "AMDILISD::ATOM_G_UMIN_NORET";
    case AMDILISD::ATOM_G_OR_NORET: return "AMDILISD::ATOM_G_OR_NORET";
    case AMDILISD::ATOM_G_SUB_NORET: return "AMDILISD::ATOM_G_SUB_NORET";
    case AMDILISD::ATOM_G_RSUB_NORET: return "AMDILISD::ATOM_G_RSUB_NORET";
    case AMDILISD::ATOM_G_XCHG_NORET: return "AMDILISD::ATOM_G_XCHG_NORET";
    case AMDILISD::ATOM_G_XOR_NORET: return "AMDILISD::ATOM_G_XOR_NORET";
    case AMDILISD::ATOM_L_ADD: return "AMDILISD::ATOM_L_ADD";
    case AMDILISD::ATOM_L_AND: return "AMDILISD::ATOM_L_AND";
    case AMDILISD::ATOM_L_CMPXCHG: return "AMDILISD::ATOM_L_CMPXCHG";
    case AMDILISD::ATOM_L_DEC: return "AMDILISD::ATOM_L_DEC";
    case AMDILISD::ATOM_L_INC: return "AMDILISD::ATOM_L_INC";
    case AMDILISD::ATOM_L_MAX: return "AMDILISD::ATOM_L_MAX";
    case AMDILISD::ATOM_L_UMAX: return "AMDILISD::ATOM_L_UMAX";
    case AMDILISD::ATOM_L_MIN: return "AMDILISD::ATOM_L_MIN";
    case AMDILISD::ATOM_L_UMIN: return "AMDILISD::ATOM_L_UMIN";
    case AMDILISD::ATOM_L_OR: return "AMDILISD::ATOM_L_OR";
    case AMDILISD::ATOM_L_SUB: return "AMDILISD::ATOM_L_SUB";
    case AMDILISD::ATOM_L_RSUB: return "AMDILISD::ATOM_L_RSUB";
    case AMDILISD::ATOM_L_XCHG: return "AMDILISD::ATOM_L_XCHG";
    case AMDILISD::ATOM_L_XOR: return "AMDILISD::ATOM_L_XOR";
    case AMDILISD::ATOM_L_ADD_NORET: return "AMDILISD::ATOM_L_ADD_NORET";
    case AMDILISD::ATOM_L_AND_NORET: return "AMDILISD::ATOM_L_AND_NORET";
    case AMDILISD::ATOM_L_CMPXCHG_NORET: return "AMDILISD::ATOM_L_CMPXCHG_NORET";
    case AMDILISD::ATOM_L_DEC_NORET: return "AMDILISD::ATOM_L_DEC_NORET";
    case AMDILISD::ATOM_L_INC_NORET: return "AMDILISD::ATOM_L_INC_NORET";
    case AMDILISD::ATOM_L_MAX_NORET: return "AMDILISD::ATOM_L_MAX_NORET";
    case AMDILISD::ATOM_L_UMAX_NORET: return "AMDILISD::ATOM_L_UMAX_NORET";
    case AMDILISD::ATOM_L_MIN_NORET: return "AMDILISD::ATOM_L_MIN_NORET";
    case AMDILISD::ATOM_L_UMIN_NORET: return "AMDILISD::ATOM_L_UMIN_NORET";
    case AMDILISD::ATOM_L_OR_NORET: return "AMDILISD::ATOM_L_OR_NORET";
    case AMDILISD::ATOM_L_SUB_NORET: return "AMDILISD::ATOM_L_SUB_NORET";
    case AMDILISD::ATOM_L_RSUB_NORET: return "AMDILISD::ATOM_L_RSUB_NORET";
    case AMDILISD::ATOM_L_XCHG_NORET: return "AMDILISD::ATOM_L_XCHG_NORET";
    case AMDILISD::ATOM_R_ADD: return "AMDILISD::ATOM_R_ADD";
    case AMDILISD::ATOM_R_AND: return "AMDILISD::ATOM_R_AND";
    case AMDILISD::ATOM_R_CMPXCHG: return "AMDILISD::ATOM_R_CMPXCHG";
    case AMDILISD::ATOM_R_DEC: return "AMDILISD::ATOM_R_DEC";
    case AMDILISD::ATOM_R_INC: return "AMDILISD::ATOM_R_INC";
    case AMDILISD::ATOM_R_MAX: return "AMDILISD::ATOM_R_MAX";
    case AMDILISD::ATOM_R_UMAX: return "AMDILISD::ATOM_R_UMAX";
    case AMDILISD::ATOM_R_MIN: return "AMDILISD::ATOM_R_MIN";
    case AMDILISD::ATOM_R_UMIN: return "AMDILISD::ATOM_R_UMIN";
    case AMDILISD::ATOM_R_OR: return "AMDILISD::ATOM_R_OR";
    case AMDILISD::ATOM_R_MSKOR: return "AMDILISD::ATOM_R_MSKOR";
    case AMDILISD::ATOM_R_SUB: return "AMDILISD::ATOM_R_SUB";
    case AMDILISD::ATOM_R_RSUB: return "AMDILISD::ATOM_R_RSUB";
    case AMDILISD::ATOM_R_XCHG: return "AMDILISD::ATOM_R_XCHG";
    case AMDILISD::ATOM_R_XOR: return "AMDILISD::ATOM_R_XOR";
    case AMDILISD::ATOM_R_ADD_NORET: return "AMDILISD::ATOM_R_ADD_NORET";
    case AMDILISD::ATOM_R_AND_NORET: return "AMDILISD::ATOM_R_AND_NORET";
    case AMDILISD::ATOM_R_CMPXCHG_NORET: return "AMDILISD::ATOM_R_CMPXCHG_NORET";
    case AMDILISD::ATOM_R_DEC_NORET: return "AMDILISD::ATOM_R_DEC_NORET";
    case AMDILISD::ATOM_R_INC_NORET: return "AMDILISD::ATOM_R_INC_NORET";
    case AMDILISD::ATOM_R_MAX_NORET: return "AMDILISD::ATOM_R_MAX_NORET";
    case AMDILISD::ATOM_R_UMAX_NORET: return "AMDILISD::ATOM_R_UMAX_NORET";
    case AMDILISD::ATOM_R_MIN_NORET: return "AMDILISD::ATOM_R_MIN_NORET";
    case AMDILISD::ATOM_R_UMIN_NORET: return "AMDILISD::ATOM_R_UMIN_NORET";
    case AMDILISD::ATOM_R_OR_NORET: return "AMDILISD::ATOM_R_OR_NORET";
    case AMDILISD::ATOM_R_MSKOR_NORET: return "AMDILISD::ATOM_R_MSKOR_NORET";
    case AMDILISD::ATOM_R_SUB_NORET: return "AMDILISD::ATOM_R_SUB_NORET";
    case AMDILISD::ATOM_R_RSUB_NORET: return "AMDILISD::ATOM_R_RSUB_NORET";
    case AMDILISD::ATOM_R_XCHG_NORET: return "AMDILISD::ATOM_R_XCHG_NORET";
    case AMDILISD::ATOM_R_XOR_NORET: return "AMDILISD::ATOM_R_XOR_NORET";
    case AMDILISD::APPEND_ALLOC: return "AMDILISD::APPEND_ALLOC";
    case AMDILISD::APPEND_CONSUME: return "AMDILISD::APPEND_CONSUME";
  };
}

bool AMDILTargetLowering::isLegalAddressingMode(const AddrMode &AM,
                                                Type *Ty) const {
  if (AM.BaseGV) {
    if (AM.BaseOffs || AM.HasBaseReg || AM.Scale)
      return false;
    return true;
  }

  // Technically AMDIL does have some more available addressing modes, but we
  // don't use any of them. For the actual hardware, the available addressing
  // modes are different for different address spaces. They also change in
  // 64-bit mode, but for global and local, register + small immediate should
  // always work.

  switch (AM.Scale) {
  case 0: // "r", "r+i" or "i" is allowed
    break;
  case 1:
    if (AM.HasBaseReg) // "r+r+i" or "r+r" is not allowed.
      return false;
    // Otherwise we have r+i.
    break;
  default:
    // No scale > 1 is allowed
    return false;
  }

  return true;
}

bool AMDILTargetLowering::isTruncateFree(Type *Src, Type *Dest) const {
  const DataLayout *DL = getDataLayout();
  unsigned SrcSize = DL->getTypeSizeInBits(Src);
  unsigned DestSize = DL->getTypeSizeInBits(Dest);
  return (DestSize < SrcSize && DestSize % 32 == 0);
}

bool AMDILTargetLowering::isTruncateFree(EVT Src, EVT Dest) const {
  // Truncate is just accessing a subregister for any multiple of the register
  // size.
  return (Dest.bitsLT(Src) && Dest.getSizeInBits() % 32 == 0);
}

bool AMDILTargetLowering::isZExtFree(Type *Src, Type *Dest) const {
  const DataLayout *DL = getDataLayout();
  unsigned SrcSize = DL->getTypeSizeInBits(Src->getScalarType());
  unsigned DestSize = DL->getTypeSizeInBits(Dest->getScalarType());

  return SrcSize == 32 && DestSize == 64;
}

bool AMDILTargetLowering::isZExtFree(EVT Src, EVT Dest) const {
  return Src.getScalarType().getSizeInBits() == 32 &&
    Dest.getScalarType().getSizeInBits() == 64;
}

/// getSetCCResultType - Return the value type to use for ISD::SETCC.
EVT AMDILTargetLowering::getSetCCResultType(LLVMContext &Context, EVT VT) const
{
  if (VT == MVT::Other)
    return MVT::i32;

  if (!VT.isVector())
    return VT.getSizeInBits() <= 32 ? MVT::i32 : MVT::i64;

  MVT EleTy = (VT.getScalarType().getSizeInBits() == 64) ? MVT::i64 : MVT::i32;
  return EVT::getVectorVT(Context, EleTy, VT.getVectorNumElements());
}


bool
AMDILTargetLowering::getTgtMemIntrinsic(IntrinsicInfo &Info,
    const CallInst &I, unsigned Intrinsic) const
{
  if (Intrinsic <= AMDILIntrinsic::last_non_AMDIL_intrinsic
      || Intrinsic > AMDILIntrinsic::num_AMDIL_intrinsics) {
    return false;
  }
  bool bitCastToInt = false;
  unsigned IntNo;
  bool isStore = true;
  bool isRet = true;
  switch (Intrinsic) {
    default: return false; // Don't custom lower most intrinsics.
    case AMDILIntrinsic::AMDIL_atomic_add_gi32:
    case AMDILIntrinsic::AMDIL_atomic_add_gu32:
    case AMDILIntrinsic::AMDIL_atomic_add_gi64:
    case AMDILIntrinsic::AMDIL_atomic_add_gu64:
             IntNo = AMDILISD::ATOM_G_ADD; break;
    case AMDILIntrinsic::AMDIL_atomic_add_gi32_noret:
    case AMDILIntrinsic::AMDIL_atomic_add_gu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_add_gi64_noret:
    case AMDILIntrinsic::AMDIL_atomic_add_gu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_G_ADD_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_add_lu32:
    case AMDILIntrinsic::AMDIL_atomic_add_li32:
    case AMDILIntrinsic::AMDIL_atomic_add_lu64:
    case AMDILIntrinsic::AMDIL_atomic_add_li64:
             IntNo = AMDILISD::ATOM_L_ADD; break;
    case AMDILIntrinsic::AMDIL_atomic_add_li32_noret:
    case AMDILIntrinsic::AMDIL_atomic_add_lu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_add_li64_noret:
    case AMDILIntrinsic::AMDIL_atomic_add_lu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_L_ADD_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_add_ru32:
    case AMDILIntrinsic::AMDIL_atomic_add_ri32:
    case AMDILIntrinsic::AMDIL_atomic_add_ru64:
    case AMDILIntrinsic::AMDIL_atomic_add_ri64:
             IntNo = AMDILISD::ATOM_R_ADD; break;
    case AMDILIntrinsic::AMDIL_atomic_add_ri32_noret:
    case AMDILIntrinsic::AMDIL_atomic_add_ru32_noret:
    case AMDILIntrinsic::AMDIL_atomic_add_ri64_noret:
    case AMDILIntrinsic::AMDIL_atomic_add_ru64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_R_ADD_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_and_gi32:
    case AMDILIntrinsic::AMDIL_atomic_and_gu32:
    case AMDILIntrinsic::AMDIL_atomic_and_gi64:
    case AMDILIntrinsic::AMDIL_atomic_and_gu64:
             IntNo = AMDILISD::ATOM_G_AND; break;
    case AMDILIntrinsic::AMDIL_atomic_and_gi32_noret:
    case AMDILIntrinsic::AMDIL_atomic_and_gu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_and_gi64_noret:
    case AMDILIntrinsic::AMDIL_atomic_and_gu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_G_AND_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_and_li32:
    case AMDILIntrinsic::AMDIL_atomic_and_lu32:
    case AMDILIntrinsic::AMDIL_atomic_and_li64:
    case AMDILIntrinsic::AMDIL_atomic_and_lu64:
             IntNo = AMDILISD::ATOM_L_AND; break;
    case AMDILIntrinsic::AMDIL_atomic_and_li32_noret:
    case AMDILIntrinsic::AMDIL_atomic_and_lu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_and_li64_noret:
    case AMDILIntrinsic::AMDIL_atomic_and_lu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_L_AND_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_and_ri32:
    case AMDILIntrinsic::AMDIL_atomic_and_ru32:
    case AMDILIntrinsic::AMDIL_atomic_and_ri64:
    case AMDILIntrinsic::AMDIL_atomic_and_ru64:
             IntNo = AMDILISD::ATOM_R_AND; break;
    case AMDILIntrinsic::AMDIL_atomic_and_ri32_noret:
    case AMDILIntrinsic::AMDIL_atomic_and_ru32_noret:
    case AMDILIntrinsic::AMDIL_atomic_and_ri64_noret:
    case AMDILIntrinsic::AMDIL_atomic_and_ru64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_R_AND_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_gi32:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_gu32:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_gi64:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_gu64:
             IntNo = AMDILISD::ATOM_G_CMPXCHG; break;
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_gi32_noret:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_gu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_gi64_noret:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_gu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_G_CMPXCHG_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_li32:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_lu32:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_li64:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_lu64:
             IntNo = AMDILISD::ATOM_L_CMPXCHG; break;
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_li32_noret:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_lu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_li64_noret:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_lu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_L_CMPXCHG_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_ri32:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_ru32:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_ri64:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_ru64:
             IntNo = AMDILISD::ATOM_R_CMPXCHG; break;
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_ri32_noret:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_ru32_noret:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_ri64_noret:
    case AMDILIntrinsic::AMDIL_atomic_cmpxchg_ru64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_R_CMPXCHG_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_dec_gi32:
    case AMDILIntrinsic::AMDIL_atomic_dec_gu32:
    case AMDILIntrinsic::AMDIL_atomic_dec_gi64:
    case AMDILIntrinsic::AMDIL_atomic_dec_gu64:
               IntNo = AMDILISD::ATOM_G_DEC;
             break;
    case AMDILIntrinsic::AMDIL_atomic_dec_gi32_noret:
    case AMDILIntrinsic::AMDIL_atomic_dec_gu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_dec_gi64_noret:
    case AMDILIntrinsic::AMDIL_atomic_dec_gu64_noret:
             isRet = false;
               IntNo = AMDILISD::ATOM_G_DEC_NORET;
             break;
    case AMDILIntrinsic::AMDIL_atomic_dec_li32:
    case AMDILIntrinsic::AMDIL_atomic_dec_lu32:
    case AMDILIntrinsic::AMDIL_atomic_dec_li64:
    case AMDILIntrinsic::AMDIL_atomic_dec_lu64:
               IntNo = AMDILISD::ATOM_L_DEC;
             break;
    case AMDILIntrinsic::AMDIL_atomic_dec_li32_noret:
    case AMDILIntrinsic::AMDIL_atomic_dec_lu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_dec_li64_noret:
    case AMDILIntrinsic::AMDIL_atomic_dec_lu64_noret:
             isRet = false;
               IntNo = AMDILISD::ATOM_L_DEC_NORET;
             break;
    case AMDILIntrinsic::AMDIL_atomic_dec_ri32:
    case AMDILIntrinsic::AMDIL_atomic_dec_ru32:
    case AMDILIntrinsic::AMDIL_atomic_dec_ri64:
    case AMDILIntrinsic::AMDIL_atomic_dec_ru64:
               IntNo = AMDILISD::ATOM_R_DEC;
             break;
    case AMDILIntrinsic::AMDIL_atomic_dec_ri32_noret:
    case AMDILIntrinsic::AMDIL_atomic_dec_ru32_noret:
    case AMDILIntrinsic::AMDIL_atomic_dec_ri64_noret:
    case AMDILIntrinsic::AMDIL_atomic_dec_ru64_noret:
             isRet = false;
               IntNo = AMDILISD::ATOM_R_DEC_NORET;
             break;
    case AMDILIntrinsic::AMDIL_atomic_inc_gi32:
    case AMDILIntrinsic::AMDIL_atomic_inc_gu32:
    case AMDILIntrinsic::AMDIL_atomic_inc_gi64:
    case AMDILIntrinsic::AMDIL_atomic_inc_gu64:
               IntNo = AMDILISD::ATOM_G_INC;
             break;
    case AMDILIntrinsic::AMDIL_atomic_inc_gi32_noret:
    case AMDILIntrinsic::AMDIL_atomic_inc_gu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_inc_gi64_noret:
    case AMDILIntrinsic::AMDIL_atomic_inc_gu64_noret:
             isRet = false;
               IntNo = AMDILISD::ATOM_G_INC_NORET;
             break;
    case AMDILIntrinsic::AMDIL_atomic_store_gv4u32:
    case AMDILIntrinsic::AMDIL_atomic_store_gv4i32:
    case AMDILIntrinsic::AMDIL_atomic_store_gv2u32:
    case AMDILIntrinsic::AMDIL_atomic_store_gv2i32:
    case AMDILIntrinsic::AMDIL_atomic_store_gu64:
    case AMDILIntrinsic::AMDIL_atomic_store_gi64:
    case AMDILIntrinsic::AMDIL_atomic_store_gu32:
    case AMDILIntrinsic::AMDIL_atomic_store_gi32:
    case AMDILIntrinsic::AMDIL_atomic_store_gu16:
    case AMDILIntrinsic::AMDIL_atomic_store_gi16:
    case AMDILIntrinsic::AMDIL_atomic_store_gu8:
    case AMDILIntrinsic::AMDIL_atomic_store_gi8:
             isRet = false;
             IntNo = AMDILISD::ATOM_G_STORE;
             break;
    case AMDILIntrinsic::AMDIL_atomic_load_gv4u32:
    case AMDILIntrinsic::AMDIL_atomic_load_gv4i32:
    case AMDILIntrinsic::AMDIL_atomic_load_gv2u32:
    case AMDILIntrinsic::AMDIL_atomic_load_gv2i32:
    case AMDILIntrinsic::AMDIL_atomic_load_gu64:
    case AMDILIntrinsic::AMDIL_atomic_load_gi64:
    case AMDILIntrinsic::AMDIL_atomic_load_gu32:
    case AMDILIntrinsic::AMDIL_atomic_load_gi32:
    case AMDILIntrinsic::AMDIL_atomic_load_gu16:
    case AMDILIntrinsic::AMDIL_atomic_load_gi16:
    case AMDILIntrinsic::AMDIL_atomic_load_gu8:
    case AMDILIntrinsic::AMDIL_atomic_load_gi8:
             IntNo = AMDILISD::ATOM_G_LOAD;
             isStore = false;
             break;
    case AMDILIntrinsic::AMDIL_atomic_inc_li32:
    case AMDILIntrinsic::AMDIL_atomic_inc_lu32:
    case AMDILIntrinsic::AMDIL_atomic_inc_li64:
    case AMDILIntrinsic::AMDIL_atomic_inc_lu64:
               IntNo = AMDILISD::ATOM_L_INC;
             break;
    case AMDILIntrinsic::AMDIL_atomic_inc_li32_noret:
    case AMDILIntrinsic::AMDIL_atomic_inc_lu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_inc_li64_noret:
    case AMDILIntrinsic::AMDIL_atomic_inc_lu64_noret:
             isRet = false;
               IntNo = AMDILISD::ATOM_L_INC_NORET;
             break;
    case AMDILIntrinsic::AMDIL_atomic_inc_ri32:
    case AMDILIntrinsic::AMDIL_atomic_inc_ru32:
    case AMDILIntrinsic::AMDIL_atomic_inc_ri64:
    case AMDILIntrinsic::AMDIL_atomic_inc_ru64:
               IntNo = AMDILISD::ATOM_R_INC;
             break;
    case AMDILIntrinsic::AMDIL_atomic_inc_ri32_noret:
    case AMDILIntrinsic::AMDIL_atomic_inc_ru32_noret:
    case AMDILIntrinsic::AMDIL_atomic_inc_ri64_noret:
    case AMDILIntrinsic::AMDIL_atomic_inc_ru64_noret:
             isRet = false;
               IntNo = AMDILISD::ATOM_R_INC_NORET;
             break;
    case AMDILIntrinsic::AMDIL_atomic_max_gi32:
    case AMDILIntrinsic::AMDIL_atomic_max_gi64:
             IntNo = AMDILISD::ATOM_G_MAX; break;
    case AMDILIntrinsic::AMDIL_atomic_max_gu32:
    case AMDILIntrinsic::AMDIL_atomic_max_gu64:
             IntNo = AMDILISD::ATOM_G_UMAX; break;
    case AMDILIntrinsic::AMDIL_atomic_max_gi32_noret:
    case AMDILIntrinsic::AMDIL_atomic_max_gi64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_G_MAX_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_max_gu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_max_gu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_G_UMAX_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_max_li32:
    case AMDILIntrinsic::AMDIL_atomic_max_li64:
             IntNo = AMDILISD::ATOM_L_MAX; break;
    case AMDILIntrinsic::AMDIL_atomic_max_lu32:
    case AMDILIntrinsic::AMDIL_atomic_max_lu64:
             IntNo = AMDILISD::ATOM_L_UMAX; break;
    case AMDILIntrinsic::AMDIL_atomic_max_li32_noret:
    case AMDILIntrinsic::AMDIL_atomic_max_li64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_L_MAX_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_max_lu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_max_lu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_L_UMAX_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_max_ri32:
    case AMDILIntrinsic::AMDIL_atomic_max_ri64:
             IntNo = AMDILISD::ATOM_R_MAX; break;
    case AMDILIntrinsic::AMDIL_atomic_max_ru32:
    case AMDILIntrinsic::AMDIL_atomic_max_ru64:
             IntNo = AMDILISD::ATOM_R_UMAX; break;
    case AMDILIntrinsic::AMDIL_atomic_max_ri32_noret:
    case AMDILIntrinsic::AMDIL_atomic_max_ri64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_R_MAX_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_max_ru32_noret:
    case AMDILIntrinsic::AMDIL_atomic_max_ru64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_R_UMAX_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_min_gi32:
    case AMDILIntrinsic::AMDIL_atomic_min_gi64:
             IntNo = AMDILISD::ATOM_G_MIN; break;
    case AMDILIntrinsic::AMDIL_atomic_min_gu32:
    case AMDILIntrinsic::AMDIL_atomic_min_gu64:
             IntNo = AMDILISD::ATOM_G_UMIN; break;
    case AMDILIntrinsic::AMDIL_atomic_min_gi32_noret:
    case AMDILIntrinsic::AMDIL_atomic_min_gi64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_G_MIN_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_min_gu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_min_gu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_G_UMIN_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_min_li32:
    case AMDILIntrinsic::AMDIL_atomic_min_li64:
             IntNo = AMDILISD::ATOM_L_MIN; break;
    case AMDILIntrinsic::AMDIL_atomic_min_lu32:
    case AMDILIntrinsic::AMDIL_atomic_min_lu64:
             IntNo = AMDILISD::ATOM_L_UMIN; break;
    case AMDILIntrinsic::AMDIL_atomic_min_li32_noret:
    case AMDILIntrinsic::AMDIL_atomic_min_li64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_L_MIN_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_min_lu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_min_lu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_L_UMIN_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_min_ri32:
    case AMDILIntrinsic::AMDIL_atomic_min_ri64:
             IntNo = AMDILISD::ATOM_R_MIN; break;
    case AMDILIntrinsic::AMDIL_atomic_min_ru32:
    case AMDILIntrinsic::AMDIL_atomic_min_ru64:
             IntNo = AMDILISD::ATOM_R_UMIN; break;
    case AMDILIntrinsic::AMDIL_atomic_min_ri32_noret:
    case AMDILIntrinsic::AMDIL_atomic_min_ri64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_R_MIN_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_min_ru32_noret:
    case AMDILIntrinsic::AMDIL_atomic_min_ru64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_R_UMIN_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_or_gi32:
    case AMDILIntrinsic::AMDIL_atomic_or_gu32:
    case AMDILIntrinsic::AMDIL_atomic_or_gi64:
    case AMDILIntrinsic::AMDIL_atomic_or_gu64:
             IntNo = AMDILISD::ATOM_G_OR; break;
    case AMDILIntrinsic::AMDIL_atomic_or_gi32_noret:
    case AMDILIntrinsic::AMDIL_atomic_or_gu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_or_gi64_noret:
    case AMDILIntrinsic::AMDIL_atomic_or_gu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_G_OR_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_or_li32:
    case AMDILIntrinsic::AMDIL_atomic_or_lu32:
    case AMDILIntrinsic::AMDIL_atomic_or_li64:
    case AMDILIntrinsic::AMDIL_atomic_or_lu64:
             IntNo = AMDILISD::ATOM_L_OR; break;
    case AMDILIntrinsic::AMDIL_atomic_or_li32_noret:
    case AMDILIntrinsic::AMDIL_atomic_or_lu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_or_li64_noret:
    case AMDILIntrinsic::AMDIL_atomic_or_lu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_L_OR_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_or_ri32:
    case AMDILIntrinsic::AMDIL_atomic_or_ru32:
    case AMDILIntrinsic::AMDIL_atomic_or_ri64:
    case AMDILIntrinsic::AMDIL_atomic_or_ru64:
             IntNo = AMDILISD::ATOM_R_OR; break;
    case AMDILIntrinsic::AMDIL_atomic_or_ri32_noret:
    case AMDILIntrinsic::AMDIL_atomic_or_ru32_noret:
    case AMDILIntrinsic::AMDIL_atomic_or_ri64_noret:
    case AMDILIntrinsic::AMDIL_atomic_or_ru64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_R_OR_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_sub_gi32:
    case AMDILIntrinsic::AMDIL_atomic_sub_gu32:
    case AMDILIntrinsic::AMDIL_atomic_sub_gi64:
    case AMDILIntrinsic::AMDIL_atomic_sub_gu64:
             IntNo = AMDILISD::ATOM_G_SUB; break;
    case AMDILIntrinsic::AMDIL_atomic_sub_gi32_noret:
    case AMDILIntrinsic::AMDIL_atomic_sub_gu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_sub_gi64_noret:
    case AMDILIntrinsic::AMDIL_atomic_sub_gu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_G_SUB_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_sub_li32:
    case AMDILIntrinsic::AMDIL_atomic_sub_lu32:
    case AMDILIntrinsic::AMDIL_atomic_sub_li64:
    case AMDILIntrinsic::AMDIL_atomic_sub_lu64:
             IntNo = AMDILISD::ATOM_L_SUB; break;
    case AMDILIntrinsic::AMDIL_atomic_sub_li32_noret:
    case AMDILIntrinsic::AMDIL_atomic_sub_lu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_sub_li64_noret:
    case AMDILIntrinsic::AMDIL_atomic_sub_lu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_L_SUB_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_sub_ri32:
    case AMDILIntrinsic::AMDIL_atomic_sub_ru32:
    case AMDILIntrinsic::AMDIL_atomic_sub_ri64:
    case AMDILIntrinsic::AMDIL_atomic_sub_ru64:
             IntNo = AMDILISD::ATOM_R_SUB; break;
    case AMDILIntrinsic::AMDIL_atomic_sub_ri32_noret:
    case AMDILIntrinsic::AMDIL_atomic_sub_ru32_noret:
    case AMDILIntrinsic::AMDIL_atomic_sub_ri64_noret:
    case AMDILIntrinsic::AMDIL_atomic_sub_ru64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_R_SUB_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_rsub_gi32:
    case AMDILIntrinsic::AMDIL_atomic_rsub_gu32:
    case AMDILIntrinsic::AMDIL_atomic_rsub_gi64:
    case AMDILIntrinsic::AMDIL_atomic_rsub_gu64:
             IntNo = AMDILISD::ATOM_G_RSUB; break;
    case AMDILIntrinsic::AMDIL_atomic_rsub_gi32_noret:
    case AMDILIntrinsic::AMDIL_atomic_rsub_gu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_rsub_gi64_noret:
    case AMDILIntrinsic::AMDIL_atomic_rsub_gu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_G_RSUB_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_rsub_li32:
    case AMDILIntrinsic::AMDIL_atomic_rsub_lu32:
    case AMDILIntrinsic::AMDIL_atomic_rsub_li64:
    case AMDILIntrinsic::AMDIL_atomic_rsub_lu64:
             IntNo = AMDILISD::ATOM_L_RSUB; break;
    case AMDILIntrinsic::AMDIL_atomic_rsub_li32_noret:
    case AMDILIntrinsic::AMDIL_atomic_rsub_lu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_rsub_li64_noret:
    case AMDILIntrinsic::AMDIL_atomic_rsub_lu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_L_RSUB_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_rsub_ri32:
    case AMDILIntrinsic::AMDIL_atomic_rsub_ru32:
    case AMDILIntrinsic::AMDIL_atomic_rsub_ri64:
    case AMDILIntrinsic::AMDIL_atomic_rsub_ru64:
             IntNo = AMDILISD::ATOM_R_RSUB; break;
    case AMDILIntrinsic::AMDIL_atomic_rsub_ri32_noret:
    case AMDILIntrinsic::AMDIL_atomic_rsub_ru32_noret:
    case AMDILIntrinsic::AMDIL_atomic_rsub_ri64_noret:
    case AMDILIntrinsic::AMDIL_atomic_rsub_ru64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_R_RSUB_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_xchg_gf32:
             bitCastToInt = true;
    case AMDILIntrinsic::AMDIL_atomic_xchg_gi32:
    case AMDILIntrinsic::AMDIL_atomic_xchg_gu32:
    case AMDILIntrinsic::AMDIL_atomic_xchg_gi64:
    case AMDILIntrinsic::AMDIL_atomic_xchg_gu64:
             IntNo = AMDILISD::ATOM_G_XCHG; break;
    case AMDILIntrinsic::AMDIL_atomic_xchg_gf32_noret:
             bitCastToInt = true;
    case AMDILIntrinsic::AMDIL_atomic_xchg_gi32_noret:
    case AMDILIntrinsic::AMDIL_atomic_xchg_gu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_xchg_gi64_noret:
    case AMDILIntrinsic::AMDIL_atomic_xchg_gu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_G_XCHG_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_xchg_lf32:
             bitCastToInt = true;
    case AMDILIntrinsic::AMDIL_atomic_xchg_li32:
    case AMDILIntrinsic::AMDIL_atomic_xchg_lu32:
    case AMDILIntrinsic::AMDIL_atomic_xchg_li64:
    case AMDILIntrinsic::AMDIL_atomic_xchg_lu64:
             IntNo = AMDILISD::ATOM_L_XCHG; break;
    case AMDILIntrinsic::AMDIL_atomic_xchg_lf32_noret:
             bitCastToInt = true;
    case AMDILIntrinsic::AMDIL_atomic_xchg_li32_noret:
    case AMDILIntrinsic::AMDIL_atomic_xchg_lu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_xchg_li64_noret:
    case AMDILIntrinsic::AMDIL_atomic_xchg_lu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_L_XCHG_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_xchg_rf32:
             bitCastToInt = true;
    case AMDILIntrinsic::AMDIL_atomic_xchg_ri32:
    case AMDILIntrinsic::AMDIL_atomic_xchg_ru32:
    case AMDILIntrinsic::AMDIL_atomic_xchg_ri64:
    case AMDILIntrinsic::AMDIL_atomic_xchg_ru64:
             IntNo = AMDILISD::ATOM_R_XCHG; break;
    case AMDILIntrinsic::AMDIL_atomic_xchg_rf32_noret:
             bitCastToInt = true;
    case AMDILIntrinsic::AMDIL_atomic_xchg_ri32_noret:
    case AMDILIntrinsic::AMDIL_atomic_xchg_ru32_noret:
    case AMDILIntrinsic::AMDIL_atomic_xchg_ri64_noret:
    case AMDILIntrinsic::AMDIL_atomic_xchg_ru64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_R_XCHG_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_xor_gi32:
    case AMDILIntrinsic::AMDIL_atomic_xor_gu32:
    case AMDILIntrinsic::AMDIL_atomic_xor_gi64:
    case AMDILIntrinsic::AMDIL_atomic_xor_gu64:
             IntNo = AMDILISD::ATOM_G_XOR; break;
    case AMDILIntrinsic::AMDIL_atomic_xor_gi32_noret:
    case AMDILIntrinsic::AMDIL_atomic_xor_gu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_xor_gi64_noret:
    case AMDILIntrinsic::AMDIL_atomic_xor_gu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_G_XOR_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_xor_li32:
    case AMDILIntrinsic::AMDIL_atomic_xor_lu32:
    case AMDILIntrinsic::AMDIL_atomic_xor_li64:
    case AMDILIntrinsic::AMDIL_atomic_xor_lu64:
             IntNo = AMDILISD::ATOM_L_XOR; break;
    case AMDILIntrinsic::AMDIL_atomic_xor_li32_noret:
    case AMDILIntrinsic::AMDIL_atomic_xor_lu32_noret:
    case AMDILIntrinsic::AMDIL_atomic_xor_li64_noret:
    case AMDILIntrinsic::AMDIL_atomic_xor_lu64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_L_XOR_NORET; break;
    case AMDILIntrinsic::AMDIL_atomic_xor_ri32:
    case AMDILIntrinsic::AMDIL_atomic_xor_ru32:
    case AMDILIntrinsic::AMDIL_atomic_xor_ri64:
    case AMDILIntrinsic::AMDIL_atomic_xor_ru64:
             IntNo = AMDILISD::ATOM_R_XOR; break;
    case AMDILIntrinsic::AMDIL_atomic_xor_ri32_noret:
    case AMDILIntrinsic::AMDIL_atomic_xor_ru32_noret:
    case AMDILIntrinsic::AMDIL_atomic_xor_ri64_noret:
    case AMDILIntrinsic::AMDIL_atomic_xor_ru64_noret:
             isRet = false;
             IntNo = AMDILISD::ATOM_R_XOR_NORET; break;
    case AMDILIntrinsic::AMDIL_append_alloc_i32:
             IntNo = AMDILISD::APPEND_ALLOC; break;
    case AMDILIntrinsic::AMDIL_append_consume_i32:
             IntNo = AMDILISD::APPEND_CONSUME; break;
  };
  Info.opc = IntNo;
  Info.memVT = (bitCastToInt) ? MVT::f32 : MVT::i32;
  Info.ptrVal = I.getOperand(0);
  Info.offset = 0;
  Info.align = 4;
  Info.vol = true;
  Info.readMem = isRet;
  Info.writeMem = isStore;
  return true;
}
// The backend supports 32 and 64 bit floating point immediates
bool
AMDILTargetLowering::isFPImmLegal(const APFloat &Imm, EVT VT) const
{
  if (VT.getScalarType().getSimpleVT().SimpleTy == MVT::f32
      || VT.getScalarType().getSimpleVT().SimpleTy == MVT::f64) {
    return true;
  } else {
    return false;
  }
}

bool
AMDILTargetLowering::ShouldShrinkFPConstant(EVT VT) const
{
  if (VT.getScalarType().getSimpleVT().SimpleTy == MVT::f32
      || VT.getScalarType().getSimpleVT().SimpleTy == MVT::f64) {
    return false;
  } else {
    return true;
  }
}


// isMaskedValueZeroForTargetNode - Return true if 'Op & Mask' is known to
// be zero. Op is expected to be a target specific node. Used by DAG
// combiner.

void
AMDILTargetLowering::computeMaskedBitsForTargetNode(
    const SDValue Op,
    APInt &KnownZero,
    APInt &KnownOne,
    const SelectionDAG &DAG,
    unsigned Depth) const
{
  APInt KnownZero2;
  APInt KnownOne2;
  unsigned BitWidth = KnownZero.getBitWidth();
  KnownZero = KnownOne = APInt(BitWidth, 0); // Don't know anything

  unsigned Opc = Op.getOpcode();
  switch (Opc) {
    default: break;
    case ISD::SELECT_CC:
    case AMDILISD::SELECT_CC:
             DAG.ComputeMaskedBits(
                 Op.getOperand(1),
                 KnownZero,
                 KnownOne,
                 Depth + 1
                 );
             DAG.ComputeMaskedBits(
                 Op.getOperand(0),
                 KnownZero2,
                 KnownOne2
                 );
             assert((KnownZero & KnownOne) == 0
                 && "Bits known to be one AND zero?");
             assert((KnownZero2 & KnownOne2) == 0
                 && "Bits known to be one AND zero?");
             // Only known if known in both the LHS and RHS
             KnownOne &= KnownOne2;
             KnownZero &= KnownZero2;
             break;
  case AMDILISD::IBIT_EXTRACT:
  case AMDILISD::UBIT_EXTRACT: {
    // TODO: Handle vectors.
    ConstantSDNode *Width = dyn_cast<ConstantSDNode>(Op.getOperand(0));
    if (!Width)
      break;


    uint32_t Val = Width->getAPIntValue().getZExtValue();
    if (Opc == AMDILISD::IBIT_EXTRACT)
      KnownOne = APInt::getHighBitsSet(32, 32 - Val);
    else
      KnownZero = APInt::getHighBitsSet(32, 32 - Val);

    break;
  }
  case AMDILISD::LCREATE2: {
    EVT HalfVT = Op.getOperand(0).getValueType();
    unsigned HalfBitWidth = HalfVT.getSizeInBits();
    APInt KnownZeroLo, KnownOneLo;
    APInt KnownZeroHi, KnownOneHi;
    DAG.ComputeMaskedBits(Op.getOperand(0), KnownZeroLo, KnownOneLo, Depth + 1);
    DAG.ComputeMaskedBits(Op.getOperand(1), KnownZeroHi, KnownOneHi, Depth + 1);

    KnownZero = KnownZeroLo.zext(BitWidth) |
                KnownZeroHi.zext(BitWidth).shl(HalfBitWidth);

    KnownOne = KnownOneLo.zext(BitWidth) |
               KnownOneHi.zext(BitWidth).shl(HalfBitWidth);
    break;
  }
  case AMDILISD::LCOMPLO: {
    DAG.ComputeMaskedBits(Op.getOperand(0), KnownZero, KnownOne, Depth + 1);
    KnownZero = KnownZero.trunc(32);
    KnownOne = KnownOne.trunc(32);
    break;
  }
  case AMDILISD::LCOMPHI: {
    DAG.ComputeMaskedBits(Op.getOperand(0), KnownZero, KnownOne, Depth + 1);
    KnownZero = KnownZero.lshr(32).trunc(32);
    KnownOne = KnownOne.lshr(32).trunc(32);
    break;
  }
  }
}

// This is the function that determines which calling convention should
// be used. Currently there is only one calling convention
CCAssignFn *AMDILTargetLowering::CCAssignFnForNode(CallingConv::ID CC,
                                                   bool isReturn,
                                                   bool atCallSite,
                                                   bool isKernel) const {
#ifndef NDEBUG
  // stress testing passing args and returns on the stack
  if (getenv("AMD_OCL_STRESS_CALLING_CONV") && !isReturn) {
    if (atCallSite) {
      return Small_Call_CC_AMDIL32;
    }
    return Small_CC_AMDIL32;
  }
#endif

  const AMDILSubtarget &STM = getTargetMachine().getSubtarget<AMDILSubtarget>();
  // if (!STM.isSupported(AMDIL::Caps::UseMacroForCall) || atCallSite) {
  if ((!STM.isSupported(AMDIL::Caps::UseMacroForCall) && !isKernel) ||
      atCallSite) {
    if (isReturn)
      return Call_RetCC_AMDIL32;

    return Call_CC_AMDIL32;
  }

  if (isReturn)
    return RetCC_AMDIL32;

  return CC_AMDIL32;
}

// LowerCallResult - Lower the result values of an ISD::CALL into the
// appropriate copies out of appropriate physical registers.  This assumes that
// Chain/InFlag are the input chain/flag to use, and that TheCall is the call
// being lowered.  The returns a SDNode with the same number of values as the
// ISD::CALL.
SDValue AMDILTargetLowering::LowerCallResult(
  SDValue Chain,
  SDValue InFlag,
  CallingConv::ID CallConv,
  bool IsVarArg,
  const SmallVectorImpl<ISD::InputArg> &Ins,
  DebugLoc DL,
  SelectionDAG &DAG,
  SmallVectorImpl<SDValue> &InVals) const {
  // Assign locations to each value returned by this call
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), RVLocs, *DAG.getContext());
  CCInfo.AnalyzeCallResult(Ins, CCAssignFnForNode(CallConv, true, true));

  const MachineRegisterInfo &MRI = DAG.getMachineFunction().getRegInfo();
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    EVT CopyVT = RVLocs[i].getValVT();
    if (RVLocs[i].isRegLoc()) {
      unsigned Reg = RVLocs[i].getLocReg();
      Chain = DAG.getCopyFromReg(Chain, DL, Reg, CopyVT, InFlag).getValue(1);
      SDValue Val = Chain.getValue(0);
      InFlag = Chain.getValue(2);
      InVals.push_back(Val);
    }
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//                           Other Lowering Hooks
//===----------------------------------------------------------------------===//

// Recursively assign SDNodeOrdering to any unordered nodes
// This is necessary to maintain source ordering of instructions
// under -O0 to avoid odd-looking "skipping around" issues.
  static const SDValue
Ordered( SelectionDAG &DAG, unsigned order, const SDValue New )
{
  if (order != 0 && DAG.GetOrdering( New.getNode() ) == 0) {
    DAG.AssignOrdering( New.getNode(), order );
    for (unsigned i = 0, e = New.getNumOperands(); i < e; ++i)
      Ordered( DAG, order, New.getOperand(i) );
  }
  return New;
}

#define LOWER(A) \
  case ISD:: A: \
return Ordered( DAG, DAG.GetOrdering( Op.getNode() ), Lower##A(Op, DAG) )

SDValue
AMDILTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const
{
  switch (Op.getOpcode()) {
    default:
      Op.getNode()->dump();
      llvm_unreachable("Custom lowering code for this"
                       "instruction is not implemented yet!");
      break;
      LOWER(OR);
      LOWER(GlobalAddress);
      LOWER(INTRINSIC_WO_CHAIN);
      LOWER(JumpTable);
      LOWER(ConstantPool);
      LOWER(ExternalSymbol);
  case ISD::LOAD: {
    SDValue Result = LowerLOAD(Op, DAG);
    assert((!Result.getNode() ||
            Result.getNode()->getNumValues() == 2) &&
           "Load should return a value and a chain");
    return Result;
  }

  case ISD::STORE:
    return LowerSTORE(Op, DAG);

      LOWER(FP_TO_SINT);
      LOWER(FP_TO_UINT);
      LOWER(SINT_TO_FP);
      LOWER(UINT_TO_FP);
      LOWER(ADD);
      LOWER(MUL);
      LOWER(SUB);
      LOWER(FDIV);
      LOWER(SDIV);
      LOWER(SREM);
      LOWER(UDIV);
      LOWER(UREM);
      LOWER(SRL);
      LOWER(SRA);
      LOWER(BUILD_VECTOR);
      LOWER(INSERT_VECTOR_ELT);
      LOWER(EXTRACT_VECTOR_ELT);
      LOWER(EXTRACT_SUBVECTOR);
      LOWER(SCALAR_TO_VECTOR);
      LOWER(CONCAT_VECTORS);
      LOWER(SETCC);
      LOWER(SELECT_CC);
      LOWER(SIGN_EXTEND_INREG);
      LOWER(BITCAST);
      LOWER(DYNAMIC_STACKALLOC);
      LOWER(BRCOND);
      LOWER(BR_CC);
      LOWER(FP_ROUND);
  }
  return Op;
}

// FIXME: Move custom handling of illegal vector types here if necessary.
void AMDILTargetLowering::ReplaceNodeResults(SDNode *N,
                                             SmallVectorImpl<SDValue> &Results,
                                             SelectionDAG &DAG) const {
  switch (N->getOpcode()) {
  case ISD::SIGN_EXTEND_INREG:
    // Different parts of legalization seem to interpret which type of
    // sign_extend_inreg is the one to check for custom lowering. The extended
    // from type is what really matters, but some places check for custom
    // lowering of the result type. This results in trying to use
    // ReplaceNodeResults to sext_in_reg to an illegal type, so we'll just do
    // nothing here and let the illegal result integer be handled normally.
    return;

  case ISD::LOAD: {
    SDValue Result = LowerLOAD(SDValue(N, 0), DAG);
    Results.push_back(Result.getValue(0));
    Results.push_back(Result.getValue(1));

    // XXX: LLVM seems not to replace Chain Value inside CustomWidenLowerNode
    // function
    DAG.ReplaceAllUsesOfValueWith(SDValue(N, 1), Result.getValue(1));
    break;
  }

  case ISD::BITCAST:
    Results.push_back(LowerBITCAST(SDValue(N, 0), DAG));
    break;

#if 0
  case ISD::EXTRACT_VECTOR_ELT: {
    MVT VT = N->getValueType();
    MVT ScalarVT = VT.getScalarType();

    unsigned ScalarBitSize = ScalarVT.getSizeInBits();
    ConstVal[0] = ScalarBitSize;
    ConstVal[1] = 0;

    // FIXME: No need to go through intrinsic.
    // AMDIL_bit_extract_i32
    // AMDIL_bit_extract_u32
    unsigned Opc = IsSigned ? AMDILIntrinsic::AMDIL_bit_extract_i32
                            : AMDILIntrinsic::AMDIL_bit_extract_u32;
    return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, OVT,
                       DAG.getTargetConstant(Opc, MVT::i32),
                       DAG.getTargetConstant(ScalarBitSize),
                       DAG.getTargetConstant(0),
                       SDValue(N, 0));
  }
#endif
  default:
    N->dump();
    llvm_unreachable("ReplaceNodeResults not implemented for this instruction");
  }
}

int
AMDILTargetLowering::getVarArgsFrameOffset() const
{
  return VarArgsFrameOffset;
}
#undef LOWER

SDValue
AMDILTargetLowering::LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const
{
  SDValue DST = Op;
  const GlobalAddressSDNode *GADN = cast<GlobalAddressSDNode>(Op);
  const GlobalValue *G = GADN->getGlobal();
  DebugLoc DL = Op.getDebugLoc();
  MachineFunction &MF = DAG.getMachineFunction();
  AMDILModuleInfo* AMI = &(MF.getMMI().getObjFileInfo<AMDILModuleInfo>());
  unsigned AS = G->getType()->getAddressSpace();
  EVT PtrVT = getPointerTy(AS);
  EVT LocalPtrVT = MVT::i32;
  assert((AS == AMDILAS::LOCAL_ADDRESS || AS == AMDILAS::CONSTANT_ADDRESS) &&
         "Unexpected address space for global variable");

  const AMDILSubtarget *stm = &getTargetMachine().getSubtarget<AMDILSubtarget>();
  if (stm->is64bit()) {
    assert((AS != AMDILAS::LOCAL_ADDRESS || PtrVT == MVT::i64) &&
           "Remove this in LLVM 3.4+");
  }

  int64_t base_offset = GADN->getOffset();
  int32_t arrayoffset = AMI->getArrayOffset(G->getName().str());
  int32_t constoffset = AMI->getConstOffset(G->getName().str());
  if (arrayoffset != -1) {
    // We will do per-pointer local buffer allocation.
    // Here we temporarily use an addri node to represent the address
    // of the local array. It will be replaced in AMDILPointerManager
    // when we figure out which local pointer is allocated in its own buffer.
    if (stm->usesHardware(AMDIL::Caps::LocalMem) &&
        AS == AMDILAS::LOCAL_ADDRESS) {
      SDValue addr = DAG.getTargetGlobalAddress(G, DL, PtrVT);
      DST = DAG.getConstant(base_offset, MVT::i32);
      DST = DAG.getNode(AMDILISD::ADDADDR, DL, LocalPtrVT,
                        DAG.getNode(ISD::TRUNCATE, DL, LocalPtrVT, addr),
                        DST);
      DST = DAG.getNode(ISD::ZERO_EXTEND, DL, PtrVT, DST);
    } else {
      DST = DAG.getConstant(arrayoffset, PtrVT);
      DST = DAG.getNode(ISD::ADD, DL, PtrVT,
          DST, DAG.getConstant(base_offset, PtrVT));
    }
  } else if (constoffset != -1) {
    SDValue addr = DAG.getTargetGlobalAddress(G, DL, PtrVT);
    SDValue base_offset_const = DAG.getConstant(base_offset, PtrVT);
    if (AMI->getConstHWBit(G->getName().str())) {
      // Retain the GlobalAddress in the instruction stream so that
      // pointer manager can trace the pointer usage back to the GlobalAddress.
      // The GlobalAddress will be replaced by constoffset at the end of
      // pointer manager
      DST = DAG.getNode(AMDILISD::ADDADDR, DL, PtrVT, addr, base_offset_const);
    } else {
      SDValue DPReg = DAG.getRegister(AMDIL::SDP, PtrVT);
      DPReg = DAG.getNode(ISD::ADD, DL, PtrVT, DPReg, base_offset_const);
      DST = DAG.getNode(AMDILISD::ADDADDR, DL, PtrVT, addr, DPReg);
    }
  } else {
    const GlobalVariable *GV = dyn_cast<GlobalVariable>(G);
    if (!GV) {
      DST = DAG.getTargetGlobalAddress(GV, DL, PtrVT);
    } else {
      if (GV->hasInitializer()) {
        const Constant *C = dyn_cast<Constant>(GV->getInitializer());
        if (const ConstantInt *CI = dyn_cast<ConstantInt>(C)) {
          DST = DAG.getConstant(CI->getValue(), Op.getValueType());

        } else if (const ConstantFP *CF = dyn_cast<ConstantFP>(C)) {
          DST = DAG.getConstantFP(CF->getValueAPF(),
              Op.getValueType());
        } else if (dyn_cast<ConstantAggregateZero>(C)) {
          EVT VT = Op.getValueType();
          if (VT.isInteger()) {
            DST = DAG.getConstant(0, VT);
          } else {
            DST = DAG.getConstantFP(0, VT);
          }
        } else {
          assert(!"lowering this type of Global Address "
              "not implemented yet!");
          C->dump();
          DST = DAG.getTargetGlobalAddress(GV, DL, PtrVT);
        }
      } else {
        DST = DAG.getTargetGlobalAddress(GV, DL, PtrVT);
      }
    }
  }
  return DST;
}

SDValue AMDILTargetLowering::LowerINTRINSIC_WO_CHAIN(SDValue Op,
                                                     SelectionDAG &DAG) const {
  unsigned IntrinsicID = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();
  DebugLoc DL = Op.getDebugLoc();
  EVT VT = Op.getValueType();

  switch (IntrinsicID) {
  case AMDILIntrinsic::AMDIL_bfi:
    return DAG.getNode(AMDILISD::BFI, DL, VT,
                       Op.getOperand(1),
                       Op.getOperand(2),
                       Op.getOperand(3));

  case AMDILIntrinsic::AMDIL_ubit_extract:
    return DAG.getNode(AMDILISD::UBIT_EXTRACT, DL, VT,
                       Op.getOperand(1),
                       Op.getOperand(2),
                       Op.getOperand(3));

  case AMDILIntrinsic::AMDIL_ibit_extract:
    return DAG.getNode(AMDILISD::IBIT_EXTRACT, DL, VT,
                       Op.getOperand(1),
                       Op.getOperand(2),
                       Op.getOperand(3));

  case AMDILIntrinsic::AMDIL_ubit_insert:
    return DAG.getNode(AMDILISD::UBIT_INSERT, DL, VT,
                       Op.getOperand(1),
                       Op.getOperand(2),
                       Op.getOperand(3),
                       Op.getOperand(4));

  default:
    return Op;
  }
}

SDValue
AMDILTargetLowering::LowerJumpTable(SDValue Op, SelectionDAG &DAG) const
{
  JumpTableSDNode *JT = cast<JumpTableSDNode>(Op);
  SDValue Result = DAG.getTargetJumpTable(JT->getIndex(), getPointerTy());
  return Result;
}
SDValue
AMDILTargetLowering::LowerConstantPool(SDValue Op, SelectionDAG &DAG) const
{
  ConstantPoolSDNode *CP = cast<ConstantPoolSDNode>(Op);
  EVT PtrVT = Op.getValueType();
  SDValue Result;
  if (CP->isMachineConstantPoolEntry()) {
    Result = DAG.getTargetConstantPool(CP->getMachineCPVal(), PtrVT,
        CP->getAlignment(), CP->getOffset(), CP->getTargetFlags());
  } else {
    Result = DAG.getTargetConstantPool(CP->getConstVal(), PtrVT,
        CP->getAlignment(), CP->getOffset(), CP->getTargetFlags());
  }
  return Result;
}

SDValue
AMDILTargetLowering::LowerExternalSymbol(SDValue Op, SelectionDAG &DAG) const
{
  const char *Sym = cast<ExternalSymbolSDNode>(Op)->getSymbol();
  SDValue Result = DAG.getTargetExternalSymbol(Sym, getPointerTy());
  return Result;
}


SDValue AMDILTargetLowering::ScalarizeVectorLoad(SDValue Op,
                                                 SelectionDAG &DAG) const {
  LoadSDNode *Load = cast<LoadSDNode>(Op);
  EVT MemVT = Load->getMemoryVT();
  EVT MemEltVT = MemVT.getVectorElementType();

  EVT LoadVT = Op.getValueType();
  EVT EltVT = LoadVT.getVectorElementType();

  EVT PtrVT = Load->getBasePtr().getValueType();

  unsigned NumElts = Load->getMemoryVT().getVectorNumElements();
  SmallVector<SDValue, 8> Loads;
  SmallVector<SDValue, 8> Chains;
  DebugLoc SL = Op.getDebugLoc();

  unsigned MemEltSize = MemEltVT.getStoreSize();
  MachinePointerInfo SrcValue(Load->getMemOperand()->getValue());

  for (unsigned i = 0; i < NumElts; ++i) {
    SDValue Ptr = DAG.getNode(ISD::ADD, SL, PtrVT, Load->getBasePtr(),
                              DAG.getConstant(i * MemEltSize, PtrVT));
    SDValue NewLoad
      = DAG.getExtLoad(Load->getExtensionType(), SL, EltVT,
                       Load->getChain(), Ptr,
                       SrcValue.getWithOffset(i * MemEltSize),
                       MemEltVT, Load->isVolatile(), Load->isNonTemporal(),
                       Load->getAlignment());
    Loads.push_back(NewLoad.getValue(0));
    Chains.push_back(NewLoad.getValue(1));
  }

  SDValue Ops[2] = {
    DAG.getNode(ISD::BUILD_VECTOR, SL, LoadVT, Loads.data(), Loads.size()),
    DAG.getNode(ISD::TokenFactor, SL, MVT::Other, Chains.data(), Chains.size())
  };

  return DAG.getMergeValues(Ops, 2, SL);
}

SDValue AMDILTargetLowering::SplitVectorLoad(SDValue Op,
                                             SelectionDAG &DAG) const {
  EVT VT = Op.getValueType();

  // If this is a 2 element vector, we really want to scalarize and not create
  // weird 1 element vectors.
  if (VT.getVectorNumElements() == 2)
    return ScalarizeVectorLoad(Op, DAG);

  LoadSDNode *Load = cast<LoadSDNode>(Op);
  SDValue Chain = Load->getChain();
  SDValue BasePtr = Load->getBasePtr();
  EVT PtrVT = BasePtr.getValueType();
  EVT MemVT = Load->getMemoryVT();
  DebugLoc SL = Op.getDebugLoc();

  EVT LoVT, HiVT;
  EVT LoMemVT, HiMemVT;
  SDValue Lo, Hi;

  llvm::tie(LoVT, HiVT) = DAG.GetSplitDestVTs(VT);
  llvm::tie(LoMemVT, HiMemVT) = DAG.GetSplitDestVTs(MemVT);
  llvm::tie(Lo, Hi) = DAG.SplitVector(Op, SL, LoVT, HiVT);

  MachinePointerInfo SrcValue(Load->getMemOperand()->getValue());

  SDValue LoLoad
    = DAG.getExtLoad(Load->getExtensionType(), SL, LoVT,
                     Chain, BasePtr,
                     SrcValue,
                     LoMemVT, Load->isVolatile(), Load->isNonTemporal(),
                     Load->getAlignment());

  SDValue HiPtr = DAG.getNode(ISD::ADD, SL, PtrVT, BasePtr,
                              DAG.getConstant(LoMemVT.getStoreSize(), PtrVT));
  SDValue HiLoad
    = DAG.getExtLoad(Load->getExtensionType(), SL, HiVT,
                     Chain, HiPtr,
                     SrcValue.getWithOffset(LoMemVT.getStoreSize()),
                     HiMemVT, Load->isVolatile(), Load->isNonTemporal(),
                     Load->getAlignment());

  SDValue Ops[2] = {
    DAG.getNode(ISD::CONCAT_VECTORS, SL, VT, LoLoad, HiLoad),
    DAG.getNode(ISD::TokenFactor, SL, MVT::Other,
                LoLoad.getValue(1), HiLoad.getValue(1))
  };

  return DAG.getMergeValues(Ops, 2, SL);
}

// Extract the small type VT packed in a 32-bit integer, into a 32-bit vector,
// and cast to a desired type.
// (SrcVT = v4i8) -> v4i32 (0, 8, 16, 24)
// (SrcVT = v2i16) -> v2i32 (0, 16)
SDValue AMDILTargetLowering::unpackSmallVector(SelectionDAG &DAG,
                                               SDValue Op,
                                               DebugLoc DL,
                                               EVT FinalVT,
                                               EVT SrcVT,
                                               bool Signed) const {
  unsigned SrcBitSize = SrcVT.getScalarSizeInBits();

  EVT LoadVT = Op.getValueType();

  assert(LoadVT.getScalarType() == MVT::i32 &&
         "All loads should be to an i32 vector");

  unsigned LoadNElts = LoadVT.isVector() ? LoadVT.getVectorNumElements() : 1;
  unsigned NElts = SrcVT.isVector() ? SrcVT.getVectorNumElements() : 1;

  assert(NElts % LoadNElts == 0);

  EVT FinalScalarVT = FinalVT.getScalarType();

  SDValue Width = DAG.getConstant(SrcBitSize, MVT::i32);

  unsigned NEltsPerLoadElt = NElts / LoadNElts;

  // The small vector element has NElts packed into LoadNElts i32 components.
  // TODO: Don't scalarize this. We can do a vector bit_extract.
  SmallVector<SDValue, 8> Ops;
  for (unsigned I = 0; I < LoadNElts; ++I) {
    SDValue LoadElt = LoadNElts == 1 ?
      Op :
      DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, MVT::i32, Op,
                  DAG.getConstant(I, MVT::i32));

    for (unsigned J = 0; J < NEltsPerLoadElt; ++J) {
      SDValue Offset = DAG.getConstant(SrcBitSize * J, MVT::i32);
      SDValue BFE
        = DAG.getNode(Signed ? AMDILISD::IBIT_EXTRACT : AMDILISD::UBIT_EXTRACT,
                      DL, MVT::i32, Width, Offset, LoadElt);

      // Convert to the original type.
      SDValue ExtOrig = DAG.getNode(Signed ? ISD::SIGN_EXTEND : ISD::ZERO_EXTEND,
                                    DL, FinalScalarVT, BFE);
      Ops.push_back(ExtOrig);
    }
  }

  return DAG.getNode(ISD::BUILD_VECTOR, DL, FinalVT, Ops.data(), Ops.size());
}

SDValue AMDILTargetLowering::packSmallVector(SelectionDAG &DAG,
                                             ArrayRef<SDValue> Elts,
                                             DebugLoc DL,
                                             EVT PackedVT) const {
  unsigned BitWidth = PackedVT.getScalarSizeInBits();
  SDValue BitWidthVal = DAG.getTargetConstant(BitWidth, MVT::i32);
  unsigned NElts = Elts.size();

  EVT StoreVT = getEquivalentMemType(*DAG.getContext(), PackedVT);

  assert(NElts > 0);

  int NPackedElts = StoreVT.isVector() ? StoreVT.getVectorNumElements() : 1;

  // bit_insert(int width, int offset, int src2, int src3):
  //   if (width != 0) {
  //     int bitmask = (((1 << width) - 1) << offset) & 0xFFFFFFFF
  //     dst = ((src2 << offset) & bitmask) | (src3 & ~bitmask)
  //   }

  assert(NElts % NPackedElts == 0);

  SmallVector<SDValue, 4> PackedElts;
  for (int I = 0; I < NPackedElts; ++I) {
    SDValue Packed = DAG.getConstant(0, MVT::i32); // Could really be undef.
    unsigned NEltsPerPack = NElts / NPackedElts;
    for (unsigned J = 0; J < NEltsPerPack; ++J) {
      uint32_t Offset = J * BitWidth;

      SDValue Elt = Elts[NEltsPerPack * I + J];
      Packed = DAG.getNode(AMDILISD::UBIT_INSERT,
                           DL,
                           MVT::i32,
                           BitWidthVal,
                           DAG.getTargetConstant(Offset, MVT::i32),
                           Elt, // Replace bits from.
                           Packed); // Replace bits in.
    }

    PackedElts.push_back(Packed);
  }

  if (NPackedElts == 1)
    return PackedElts.front();

  return DAG.getNode(ISD::BUILD_VECTOR, DL, StoreVT,
                     PackedElts.data(), PackedElts.size());
}

static bool isConstantAddressBlock(unsigned AS) {
  return AS >= AMDILAS::CONSTANT_BUFFER_0 && AS <= AMDILAS::CONSTANT_BUFFER_14;
}

// TODO: Struct loads are not handled, and end up being split into per-element
// loads.
SDValue AMDILTargetLowering::LowerLOAD(SDValue Op, SelectionDAG &DAG) const {
  const LoadSDNode *Load = cast<LoadSDNode>(Op);
  EVT MemEVT = Load->getMemoryVT();

  if (!MemEVT.isSimple())
    return SDValue();

  DebugLoc DL = Op.getDebugLoc();
  ISD::LoadExtType ExtType = Load->getExtensionType();
  EVT VT = Op.getValueType();
  MVT MemVT = MemEVT.getSimpleVT();
  unsigned AS = Load->getAddressSpace();

  unsigned StoreSize = MemVT.getStoreSizeInBits();
  if (StoreSize > Subtarget.getMaxLoadSizeInBits(AS) && VT.isVector() &&
      VT.isPow2VectorType() && (StoreSize % 32 == 0)) {
    // Split in half.
    return SplitVectorLoad(Op, DAG);
  }

  // Don't combine volatile loads, and instead scalarize them.
  // XXX - Should this really not be OK for a volatile load?
  if (MemVT.isVector() && Load->isVolatile())
    return ScalarizeVectorLoad(Op, DAG);
  // Custom unpack small vectors. We can load them in a single word, and then
  // unpack with BFE instructions.
  // e.g.
  // v4i8 (load ...) ->
  //   x = i32 load
  //   trunc (build_vector (bfe x, 0, 8),
  //                       (bfe x, 8, 8),
  //                       (bfe x, 16, 8),
  //                       (bfe x, 24, 8))
  //
  if (ExtType != ISD::NON_EXTLOAD &&
      Subtarget.hasBFE() &&
      MemVT.isVector() &&
      MemVT.getScalarSizeInBits() < 32) {
    assert(!VT.isFloatingPoint() && "FP ext loads not yet handled");

    // Figure out what type to load as.
    EVT LoadVT = getEquivalentMemType(*DAG.getContext(), MemVT);
    EVT DestLoadVT = getEquivalentLoadRegType(*DAG.getContext(), MemVT);

    // If we're loading a v2i8 as an i16, we need to do an extload of the i16 to
    // i32.
    ISD::LoadExtType NewLoadExt
      = (LoadVT.getSizeInBits() < 32) ? ISD::ZEXTLOAD : ISD::NON_EXTLOAD;

    // FIXME: We are ending up with an extra and x, 255 on the unpacked and
    // extended vector, even though we know the upper bits have already been
    // zeroed by the BFE.
    SDValue NewLoad = DAG.getLoad(Load->getAddressingMode(),
                                  NewLoadExt,
                                  DestLoadVT,
                                  DL,
                                  Load->getChain(),
                                  Load->getBasePtr(),
                                  Load->getOffset(),
                                  LoadVT,
                                  Load->getMemOperand());

    SDValue Ops[2] = {
      unpackSmallVector(DAG, NewLoad, DL, VT, MemVT, ExtType == ISD::SEXTLOAD),
      NewLoad.getValue(1)
    };

    return DAG.getMergeValues(Ops, 2, DL);
  }

  if (ExtType != ISD::NON_EXTLOAD && !VT.isVector() && VT.getSizeInBits() > 32) {
    // We can do the extload to 32-bits, and then need to separately extend to
    // 64-bits.

    SDValue ExtLoad32 = DAG.getLoad(Load->getAddressingMode(),
                                    ExtType, MVT::i32, DL,
                                    Load->getChain(),
                                    Load->getBasePtr(),
                                    Load->getOffset(),
                                    MemVT,
                                    Load->getMemOperand());
    SDValue Result = DAG.getNode(ISD::getExtForLoadExtType(ExtType),
                                 DL, VT, ExtLoad32);

    SDValue Ops[2] = {
      Result,
      ExtLoad32.getValue(1)
    };

    return DAG.getMergeValues(Ops, 2, DL);
  }

  if (ExtType == ISD::NON_EXTLOAD && VT.isVector() &&
      MemVT.getScalarSizeInBits() == 64) {
    EVT LoadVT = getEquivalentMemType(*DAG.getContext(), MemVT);
    SDValue NewLoad = DAG.getLoad(Load->getAddressingMode(),
                                  ISD::NON_EXTLOAD,
                                  LoadVT,
                                  DL,
                                  Load->getChain(),
                                  Load->getBasePtr(),
                                  Load->getOffset(),
                                  LoadVT,
                                  Load->getMemOperand());

    SDValue Ops[2] = {
      DAG.getNode(ISD::BITCAST, DL, VT, NewLoad),
      NewLoad.getValue(1)
    };

    return DAG.getMergeValues(Ops, 2, DL);
  }

  if (isConstantAddressBlock(AS)) {
    SDValue Chain = Load->getChain();
    SDValue Ptr = Load->getBasePtr();
    int64_t Offset = Load->getSrcValueOffset();
    unsigned CBId = AS - AMDILAS::CONSTANT_BUFFER_0;
    MVT PtrVT = getPointerTy();

    EVT LoadVT = getEquivalentLoadRegType(*DAG.getContext(), MemVT);

    assert(MemVT.getStoreSizeInBits() <= 128);
    assert((!VT.isVector() || ExtType == ISD::NON_EXTLOAD) &&
           "Unexpected vector extload from constant buffer");
    assert((Offset % 16 == 0) && "Unexpected src value offset");


    // Each kernel parameter is 128-bit aligned.
    // Convert to from bytes to 16-byte index.
    SDValue NewPtr = DAG.getNode(ISD::SRL, DL, PtrVT,
                                 Ptr, DAG.getConstant(4, PtrVT));
    SDValue Result = DAG.getNode(AMDILISD::CONST_ADDRESS, DL, MVT::v4i32,
                                 NewPtr, DAG.getTargetConstant(CBId, MVT::i32));

    // Copy the entire 16-byte CB index, and then extract the components we
    // need.

    // FIXME: This is a hack to avoid needing to fix source swizzles, and only
    // handles the cases we need to care about for kernel arguments.
    if (LoadVT.isVector()) {
      unsigned NElts = LoadVT.getVectorNumElements();
      assert(NElts <= 4);

      if (NElts < 4) {
        SmallVector<SDValue, 8> NewElts;
        EVT VecVT = EVT::getVectorVT(*DAG.getContext(), MVT::i32, NElts);
        for (unsigned I = 0; I < NElts; ++I) {
          SDValue Elt = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, MVT::i32,
                                    Result, DAG.getConstant(I, MVT::i32));
          NewElts.push_back(Elt);
        }

        Result = DAG.getNode(ISD::BUILD_VECTOR, DL, VecVT,
                             NewElts.data(), NewElts.size());
      }
    } else {
      Result = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, MVT::i32,
                            Result, DAG.getConstant(0, MVT::i32));
    }

    if (ExtType == ISD::ZEXTLOAD) {
      Result = DAG.getZeroExtendInReg(Result, DL, MemVT.getScalarType());
    } else if (ExtType == ISD::SEXTLOAD) {
      Result = DAG.getNode(ISD::SIGN_EXTEND_INREG, DL, LoadVT,
                           Result,
                           DAG.getValueType(MemVT.getScalarType()));
    }

    SDValue Ops[2] = {
      Result,
      Load->getChain()
    };

    return DAG.getMergeValues(Ops, 2, DL);
  }

  return SDValue();
}

SDValue AMDILTargetLowering::MergeVectorStore(SDValue Op,
                                              SelectionDAG &DAG) const {
  StoreSDNode *Store = cast<StoreSDNode>(Op);
  EVT MemVT = Store->getMemoryVT();

  if (!MemVT.isSimple())
    return SDValue();

  // Byte stores are really expensive, so if possible, try to pack 32-bit vector
  // truncating store into an i32 store.
  assert(MemVT.isVector() && "This only handles vector stores");

  DebugLoc DL = Op.getDebugLoc();
  SDValue Value = Store->getValue();
  EVT VT = Value.getValueType();
  SDValue Ptr = Store->getBasePtr();
  unsigned MemBits = MemVT.getStoreSizeInBits();
  EVT MemEltVT = MemVT.getVectorElementType();
  unsigned MemEltBits = MemEltVT.getSizeInBits();
  unsigned MemNumElements = MemVT.getVectorNumElements();
  EVT PackedVT = getEquivalentMemType(*DAG.getContext(), MemVT);

  if (!Store->isTruncatingStore()) {
    if (MemEltBits != 64)
      return SDValue();

    EVT StoreVT = getEquivalentMemType(*DAG.getContext(), VT);
    SDValue Cast = DAG.getNode(ISD::BITCAST, DL, StoreVT, Value);

    return DAG.getStore(Store->getChain(), DL, Cast,
                        Ptr, Store->getMemOperand());
  }

  SmallVector<SDValue, 8> Elts;
  ExtractVectorElements(Value, DAG, Elts, 0, MemNumElements);
  SDValue Packed = packSmallVector(DAG, Elts, DL, MemVT);

  if (MemBits < 32) {
    // Doing a byte or short store.
    EVT StoreVT = getEquivalentMemType(*DAG.getContext(), PackedVT);
    assert(Packed.getValueType() == MVT::i32);

    return DAG.getTruncStore(Store->getChain(),
                             DL,
                             Packed,
                             Ptr,
                             StoreVT,
                             Store->getMemOperand());
  }

  return DAG.getStore(Store->getChain(), DL, Packed,
                      Ptr, Store->getMemOperand());
}

SDValue AMDILTargetLowering::ScalarizeVectorStore(SDValue Op,
                                                  SelectionDAG &DAG) const {
  StoreSDNode *Store = cast<StoreSDNode>(Op);
  EVT MemEltVT = Store->getMemoryVT().getVectorElementType();
  EVT EltVT = Store->getValue().getValueType().getVectorElementType();
  EVT PtrVT = Store->getBasePtr().getValueType();
  unsigned NumElts = Store->getMemoryVT().getVectorNumElements();
  DebugLoc SL = Op.getDebugLoc();

  unsigned EltSize = EltVT.getStoreSize();
  MachinePointerInfo SrcValue(Store->getMemOperand()->getValue());

  SmallVector<SDValue, 8> Chains;
  for (unsigned i = 0, e = NumElts; i != e; ++i) {
    SDValue Val = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, SL, EltVT,
                              Store->getValue(),
                              DAG.getConstant(i, MVT::i32));
    SDValue Ptr = DAG.getNode(ISD::ADD, SL, PtrVT,
                              Store->getBasePtr(),
                              DAG.getConstant(i * MemEltVT.getStoreSize(),
                                              PtrVT));
    SDValue NewStore =
      DAG.getTruncStore(Store->getChain(), SL, Val, Ptr,
                        SrcValue.getWithOffset(i * EltSize),
                        MemEltVT, Store->isNonTemporal(), Store->isVolatile(),
                        Store->getAlignment());
    Chains.push_back(NewStore);
  }

  return DAG.getNode(ISD::TokenFactor, SL, MVT::Other,
                     Chains.data(), Chains.size());
}

SDValue AMDILTargetLowering::SplitVectorStore(SDValue Op,
                                              SelectionDAG &DAG) const {
  StoreSDNode *Store = cast<StoreSDNode>(Op);
  EVT MemVT = Store->getMemoryVT();
  SDValue Val = Store->getValue();
  EVT VT = Val.getValueType();

  SDValue Chain = Store->getChain();
  SDValue BasePtr = Store->getBasePtr();
  DebugLoc SL = Op.getDebugLoc();

  EVT LoVT, HiVT;
  EVT LoMemVT, HiMemVT;
  SDValue Lo, Hi;

  llvm::tie(LoVT, HiVT) = DAG.GetSplitDestVTs(VT);
  llvm::tie(LoMemVT, HiMemVT) = DAG.GetSplitDestVTs(MemVT);
  llvm::tie(Lo, Hi) = DAG.SplitVector(Val, SL, LoVT, HiVT);

  EVT PtrVT = BasePtr.getValueType();
  SDValue HiPtr = DAG.getNode(ISD::ADD, SL, PtrVT, BasePtr,
                              DAG.getConstant(LoMemVT.getStoreSize(), PtrVT));

  MachinePointerInfo SrcValue(Store->getMemOperand()->getValue());
  SDValue LoStore
    = DAG.getTruncStore(Chain, SL, Lo,
                        BasePtr,
                        SrcValue,
                        LoMemVT,
                        Store->isNonTemporal(),
                        Store->isVolatile(),
                        Store->getAlignment());
  SDValue HiStore
    = DAG.getTruncStore(Chain, SL, Hi,
                        HiPtr,
                        SrcValue.getWithOffset(LoMemVT.getStoreSize()),
                        HiMemVT,
                        Store->isNonTemporal(),
                        Store->isVolatile(),
                        Store->getAlignment());

  return DAG.getNode(ISD::TokenFactor, SL, MVT::Other, LoStore, HiStore);
}

SDValue AMDILTargetLowering::LowerSTORE(SDValue Op, SelectionDAG &DAG) const {
  StoreSDNode *Store = cast<StoreSDNode>(Op);
  EVT MemVT = Store->getMemoryVT();

  if (!MemVT.isVector())
    return SDValue();

  if (Store->isVolatile())
    return ScalarizeVectorStore(Op, DAG);

  unsigned AS = Store->getAddressSpace();
  if (MemVT.getStoreSizeInBits() > Subtarget.getMaxStoreSizeInBits(AS)) {
    // Store instructions are all for 128 bits or less, so split in half.
    return SplitVectorStore(Op, DAG);
  }

  return AMDILTargetLowering::MergeVectorStore(Op, DAG);
}

/// LowerFORMAL_ARGUMENTS - transform physical registers into
/// virtual registers and generate load operations for
/// arguments places on the stack.
/// TODO: isVarArg, hasStructRet, isMemReg
  SDValue
AMDILTargetLowering::LowerFormalArguments(SDValue Chain,
    CallingConv::ID CallConv,
    bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins,
    DebugLoc dl,
    SelectionDAG &DAG,
    SmallVectorImpl<SDValue> &InVals) const{
  MachineFunction &MF = DAG.getMachineFunction();
  AMDILMachineFunctionInfo *FuncInfo = MF.getInfo<AMDILMachineFunctionInfo>();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  //const Function *Fn = MF.getFunction();
  //MachineRegisterInfo &RegInfo = MF.getRegInfo();

  SmallVector<CCValAssign, 16> ArgLocs;
  CallingConv::ID CC = MF.getFunction()->getCallingConv();
  //bool hasStructRet = MF.getFunction()->hasStructRetAttr();

  CCState CCInfo(CC, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());

  bool IsKernel = FuncInfo->isKernel();
  // When more calling conventions are added, they need to be chosen here
  CCInfo.AnalyzeFormalArguments(Ins, CCAssignFnForNode(CallConv, false,
                                                       false, IsKernel));

  // first and last vector register used for input/output at a call site
  unsigned FirstReg = AMDIL::IN0;
  unsigned LastReg = AMDIL::IN255;
  const AMDILSubtarget &STM = getTargetMachine().getSubtarget<AMDILSubtarget>();
  if (!IsKernel && !STM.isSupported(AMDIL::Caps::UseMacroForCall)) {
    FirstReg = AMDIL::R1;
    LastReg = AMDIL::R1012;
  }

  // Keep track of which vector registers (e.g. in0) are used for the arg list.
  SmallSet<unsigned, 4> VectorRegs;

  const TargetRegisterInfo *TRI = getTargetMachine().getRegisterInfo();
  for (unsigned int i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc()) {
      EVT RegVT = VA.getLocVT();
      EVT ValVT = VA.getValVT();
      const TargetRegisterClass *RC
        = AMDILRegisterInfo::getRegClassFromType(RegVT.getSimpleVT());

      unsigned Reg = VA.getLocReg();
      FuncInfo->addArgReg(Reg);
      unsigned VectorReg = getVectorReg(Reg, TRI);
      assert(VectorReg >= FirstReg && VectorReg <= LastReg &&
             "unexpected register assigned for call argument");
      VectorRegs.insert(VectorReg);

      unsigned VReg = MF.addLiveIn(Reg, RC);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, VReg, RegVT);

      // If this is an 8 or 16-bit value, it is really passed
      // promoted to 32 bits.  Insert an assert[sz]ext to capture
      // this, then truncate to the right size.
      if (VA.getLocInfo() == CCValAssign::SExt) {
        ArgValue = DAG.getNode(
            ISD::AssertSext,
            dl,
            RegVT,
            ArgValue,
            DAG.getValueType(ValVT.getScalarType()));
      } else if (VA.getLocInfo() == CCValAssign::ZExt ||
                 VA.getLocInfo() == CCValAssign::AExt) {
        ArgValue = DAG.getNode(
            ISD::AssertZext,
            dl,
            RegVT,
            ArgValue,
            DAG.getValueType(ValVT.getScalarType()));
      } else if (VA.getLocInfo() == CCValAssign::BCvt) {
        ArgValue = DAG.getNode(ISD::BITCAST, dl, ValVT, ArgValue);
      }

      if (VA.getLocInfo() != CCValAssign::Full &&
          VA.getLocInfo() != CCValAssign::BCvt) {
        ArgValue = DAG.getNode(
            ISD::TRUNCATE,
            dl,
            ValVT,
            ArgValue);
      }
      // Add the value to the list of arguments
      // to be passed in registers
      InVals.push_back(ArgValue);
      if (isVarArg) {
        llvm_unreachable("Variable arguments are not yet supported");
        // See MipsISelLowering.cpp for ideas on how to implement
      }
    } else if(VA.isMemLoc()) {
      InVals.push_back(LowerMemArgument(Chain, CallConv, Ins,
            dl, DAG, VA, MFI, i));
    } else {
      llvm_unreachable("found a Value Assign that is "
                       "neither a register or a memory location");
    }
  }
  FuncInfo->setArgNumVecRegs(VectorRegs.size());
  /*if (hasStructRet) {
    llvm_unreachable("Has struct return is not yet implemented");
  // See MipsISelLowering.cpp for ideas on how to implement
  }*/

  unsigned int StackSize = CCInfo.getNextStackOffset();
  if (isVarArg) {
    llvm_unreachable("Variable arguments are not yet supported");
    // See X86/PPC/CellSPU ISelLowering.cpp for ideas on how to implement
  }
  // This needs to be changed to non-zero if the return function needs
  // to pop bytes
  FuncInfo->setBytesToPopOnReturn(StackSize);
  return Chain;
}
/// CreateCopyOfByValArgument - Make a copy of an aggregate at address specified
/// by "Src" to address "Dst" with size and alignment information specified by
/// the specific parameter attribute. The copy will be passed as a byval
/// function parameter.
static SDValue CreateCopyOfByValArgument(SDValue Src,
                                         SDValue Dst,
                                         SDValue Chain,
                                         ISD::ArgFlagsTy Flags,
                                         SelectionDAG &DAG) {
  llvm_unreachable("MemCopy does not exist yet");
  SDValue SizeNode = DAG.getConstant(Flags.getByValSize(), MVT::i32);

  return DAG.getMemcpy(Chain,
      Src.getDebugLoc(),
                       Dst,
                       Src,
                       SizeNode,
                       Flags.getByValAlign(),
                       /*IsVol=*/false,
                       /*AlwaysInline=*/true,
                       MachinePointerInfo(),
                       MachinePointerInfo());
}

SDValue
AMDILTargetLowering::LowerMemOpCallTo(SDValue Chain,
    SDValue StackPtr, SDValue Arg,
    DebugLoc dl, SelectionDAG &DAG,
    const CCValAssign &VA,
    ISD::ArgFlagsTy Flags) const
{
  unsigned int LocMemOffset = VA.getLocMemOffset();
  SDValue PtrOff = DAG.getIntPtrConstant(LocMemOffset);
  PtrOff = DAG.getNode(ISD::ADD,
      dl,
      getPointerTy(), StackPtr, PtrOff);
  if (Flags.isByVal()) {
    PtrOff = CreateCopyOfByValArgument(Arg, PtrOff, Chain, Flags, DAG);
  } else {
    PtrOff = DAG.getStore(Chain, dl, Arg, PtrOff,
        MachinePointerInfo::getStack(LocMemOffset),
        false, false, 0);
  }
  return PtrOff;
}

// Promote the value if needed.
static SDValue promoteCCVal(SelectionDAG &DAG,
                            DebugLoc DL,
                            CCValAssign::LocInfo LI,
                            EVT LocVT,
                            SDValue Op) {
  switch (LI) {
  default:
    llvm_unreachable("Unknown loc info!");
  case CCValAssign::Full:
  case CCValAssign::BCvt:
    return Op;
  case CCValAssign::SExt:
    return DAG.getNode(ISD::SIGN_EXTEND, DL, LocVT, Op);
  case CCValAssign::ZExt:
    return DAG.getNode(ISD::ZERO_EXTEND, DL, LocVT, Op);
  case CCValAssign::AExt:
    return DAG.getNode(ISD::ANY_EXTEND, DL, LocVT, Op);
  }
}

// Temporary hack to fix 3-vector arguments by manually widening to the
// 4-vector physical register type. This avoids creating creating an
// unpromotable Register node with an illegal v3 type. This should not be
// necessary when kernel args are treated correct as loads.
static SDValue loadVec3ArgAsVec4(SelectionDAG &DAG,
                                 DebugLoc dl,
                                 SDValue ArgLoad,
                                 EVT ExtVecVT) {
  SmallVector<SDValue, 4> Elts;
  EVT EltVT = ExtVecVT.getVectorElementType();
  EVT WideVecVT = EVT::getVectorVT(*DAG.getContext(), EltVT, 4);

  ExtractVectorElements(ArgLoad, DAG, Elts, 0, 3);
  Elts.push_back(DAG.getUNDEF(EltVT));

  return DAG.getNode(ISD::BUILD_VECTOR, dl, WideVecVT,
                     Elts.data(), Elts.size());
}

// FIXME: This is a hack. Kernel arguments are not passed in registers, and
// should not be modeled as such. They are loads, and this should all be
// handled in LowerFormalArguments. A kernel calling convention should specify
// kernel arguments are passed in memory, and set up the argument offsets.
//
// What happens now is hard to follow. First, as things worked before (and
// still do with macro calls disabled), the AsmPrinter directly prints out
// argument setup, calls into the "body" of the kernel function and then those
// registers are assumed to hold the kernel arguments. This doesn't have the
// correct information about the used registers from the generated calling
// convention and types as is available here in lowering, and makes various
// broken assumptions about the number and type of used registers, as well as
// being a hack which will not work in future versions of LLVM which are
// moving to prevent directly printing text instructions.
//
// With macro calls, this setup still occurs, but is then just ignored. The
// assumption about kernel arguments being passed as registers is moved here
// and handled somewhat differently differently, but still wrong. This
// pretends that there is a call to the kernel that has a special argument
// setup. Here sets up loads from the CBs (which are modelled as special
// registers instead of memory), and then sets up a call to a "normal" macro
// function.
//
// This all needs to be fixed by creating a kernel calling convention that
// sets up load from the CBs in LowerFormalArguments. The prelude emitted in
// the AsmPrinter needs to be removed, and the call to the true kernel body
// should not involve a call, particularly a macro call.

// Setup and return the source of kernel arguments.
// See the runtime ABI doc for the ABI of kernel arg passing from runtime
// to compiled code.
void AMDILTargetLowering::BuildKernelArgVal(
  SmallVectorImpl<SDValue> &Result,
  ArgListEntry &Arg,
  unsigned NumParts,
  SDValue OutVal,
  EVT VT,
  EVT ArgVT,
  SDValue &Chain,
  unsigned &CBIdx,
  SelectionDAG &DAG) const {
  assert(VT.isSimple() && "unexpected kernel arg type");
  assert(CBIdx < 256 && "too many kernel args for CB1 registers");
  DebugLoc dl;
  MVT PtrVT = getPointerTy();

  // Handle the case that the kernel arg is an image pointer
  if (const PointerType *PTy = dyn_cast<PointerType>(Arg.Ty)) {
    const Type *ETy = PTy->getElementType();
    if (isImageType(ETy)) {
      SDValue Val = DAG.getConstant(CBIdx, MVT::i32);
      CBIdx += NUM_EXTRA_SLOTS_PER_IMAGE + 1;
      Result.push_back(Val);
      return;
    }

    if (PTy->getAddressSpace() == AMDILAS::CONSTANT_ADDRESS &&
        Subtarget.usesHardware(AMDIL::Caps::ConstantMem)) {
      ++CBIdx; // This still reserves an argument index.
      Result.push_back(DAG.getConstant(0, MVT::i32));
      return;
    }
  }

  // Byval struct type kernel arg has been passed through pointer to stack
  // mem. Copy the struct value from cb1 to stack mem.
  // TODO: In principle we shouldn't have to do this if the struct is never
  // modified which is the common case.
  if (isa<FrameIndexSDNode>(OutVal.getNode())) {
    PointerType *PTy = dyn_cast<PointerType>(Arg.Ty);
    assert(PTy && PTy->getAddressSpace() == AMDILAS::PRIVATE_ADDRESS);
    StructType *STy = cast<StructType>(PTy->getElementType());
    unsigned OrigSize = getDataLayout()->getTypeStoreSize(STy);
    unsigned Size = DataLayout::RoundUpAlignment(OrigSize, 16);

    PointerType *SrcPtrTy = PointerType::get(STy, AMDILAS::CONSTANT_BUFFER_1);
    MachinePointerInfo SrcPtrInfo(UndefValue::get(SrcPtrTy));

    SmallVector<SDValue, 8> Chains;

    for (unsigned Offset = 0; Offset < Size; Offset += 16, ++CBIdx) {
      assert(CBIdx < 256 && "Too many kernel args for CB1 registers.");

      // Convert to bytes.
      SDValue SrcPtr = DAG.getConstant(16 * CBIdx, PtrVT);

      SDValue PartLoad = DAG.getLoad(MVT::v4i32,
                                     dl,
                                     Chain,
                                     SrcPtr,
                                     SrcPtrInfo.getWithOffset(Offset),
                                     false, // isVolatile
                                     true, // isNonTemporal
                                     true, // isInvariant
                                     16);
      SDValue DestPtr = OutVal;
      if (Offset != 0) {
        // Generate FI + Offset for store address.
        DestPtr = DAG.getNode(ISD::ADD, dl, DestPtr.getValueType(),
                              DestPtr,
                              DAG.getConstant(Offset, DestPtr.getValueType()));
      }

      MachinePointerInfo DestPtrInfo(NULL, Offset);
      SDValue Tmp = DAG.getStore(Chain, dl, PartLoad, DestPtr,
                                 DestPtrInfo,
                                 false /* isVolatile */,
                                 true /* isNonTemporal */,
                                 16 /* Alignment */);
      Chains.push_back(Tmp);
    }

    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other,
                        Chains.data(), Chains.size());

    Result.push_back(OutVal);
    return;
  }

  unsigned ArgSize = ArgVT.getStoreSize();
  if (ArgVT.isVector()) {
    EVT ExtVecVT = EVT::getVectorVT(*DAG.getContext(),
                                    VT.getScalarType(),
                                    ArgVT.getVectorNumElements());
    PointerType *PtrTy
      = PointerType::get(ArgVT.getTypeForEVT(*DAG.getContext()),
                         AMDILAS::CONSTANT_BUFFER_1);

    ISD::LoadExtType Ext = ISD::NON_EXTLOAD;
    if (ArgVT.getScalarSizeInBits() < 32)
      Ext = Arg.isSExt ? ISD::SEXTLOAD : ISD::ZEXTLOAD;

    if (ArgVT.getScalarSizeInBits() < 32) {
      // The ABI for kernel arguments is really stupid. It neither packs
      // efficiently, nor avoids extend operations. 8 and 16-bit vectors have 4
      // components packed into the low 32-bit component of the 128-bit CB
      // index. The next 4 are then 128-bit aligned in the next index.

      unsigned ArgNElts = ArgVT.getVectorNumElements();

      // FIXME
      unsigned LoadElts = ArgNElts == 3 ? 4 : ArgNElts;

      // We're really loading 4 elements at a time with a gap in between.
      EVT RealMemVT = EVT::getVectorVT(*DAG.getContext(),
                                       ArgVT.getScalarType(),
                                       std::min(LoadElts, 4u));

      EVT RealLoadVT = EVT::getVectorVT(*DAG.getContext(),
                                        MVT::i32,
                                        std::min(LoadElts, 4u));

      unsigned MemSplit = (ArgVT.getVectorNumElements() + 4 - 1) / 4;

      bool Scalarize = !VT.isVector();

      MVT CBPtrVT = getPointerTy();
      assert((Scalarize || NumParts == 1) &&
             "Split vector not expected for this type");
      for (unsigned I = 0; I < MemSplit; ++I, ++CBIdx) {
        SDValue ArgLoad = DAG.getExtLoad(
          Ext, dl, RealLoadVT, Chain,
          DAG.getConstant(16 * CBIdx, CBPtrVT), // Convert to bytes.
          MachinePointerInfo(UndefValue::get(PtrTy)), // Only uses the CB index.
          RealMemVT, false, true, 16);

        // Append.
        if (Scalarize)
          ExtractVectorElements(ArgLoad, DAG, Result, 0, std::min(ArgNElts, 4u));
        else
          Result.push_back(ArgLoad);
      }

      return;
    }

    // We load whatever size we need to here. When the load is lowered, it will
    // be appropriately split into separate 128-bit loads.
    SDValue ArgLoad = DAG.getLoad(
      ISD::UNINDEXED,
      Ext,
      ExtVecVT,
      dl,
      Chain,
      DAG.getConstant(16 * CBIdx, PtrVT), // Convert to bytes.
      DAG.getUNDEF(MVT::i32),
      MachinePointerInfo(UndefValue::get(PtrTy)), // Only uses the CB index.
      ArgVT, false, true, true, 16);

    // XXX - What?
    Chain = SDValue(ArgLoad.getNode(), 1);

    // FIXME: Remove this.
    if (ExtVecVT.getVectorNumElements() == 3)
      ArgLoad = loadVec3ArgAsVec4(DAG, dl, ArgLoad, ExtVecVT);

    if (!VT.isVector()) {
      // Argument was scalarized.
      ExtractVectorElements(ArgLoad, DAG, Result, 0, NumParts);
      CBIdx += (ArgSize + 16 - 1) / 16; // FIXME should get offset from CCValAssign
      return;
    }

    unsigned NElts = ArgVT.getVectorNumElements();
    if (NumParts > 1) {
      // Vector was split.
      // Extract and recombine into correct vector types.

      assert(VT.getVectorElementType() == ArgVT.getVectorElementType());

      unsigned EltsPerSplit = NElts / NumParts;
      assert(NElts % NumParts == 0);


      for (unsigned I = 0; I < NumParts; ++I) {
        SmallVector<SDValue, 8> Elts;
        ExtractVectorElements(ArgLoad, DAG, Elts,
                              I * EltsPerSplit, EltsPerSplit);

        SDValue NewV = DAG.getNode(ISD::BUILD_VECTOR, dl, VT,
                                   Elts.data(), Elts.size());
        Result.push_back(NewV);
      }

      CBIdx += (ArgSize + 16 - 1) / 16; // FIXME should get offset from CCValAssign
      return;
    }

    // Vector was simply promoted.
    Result.push_back(ArgLoad);
    CBIdx += (ArgSize + 16 - 1) / 16; // FIXME should get offset from CCValAssign
    return;
  }

  if (ArgVT.getScalarSizeInBits() < 32) {
    PointerType *PtrTy
      = PointerType::get(ArgVT.getTypeForEVT(*DAG.getContext()),
                         AMDILAS::CONSTANT_BUFFER_1);

    ISD::LoadExtType Ext = Arg.isSExt ? ISD::SEXTLOAD : ISD::ZEXTLOAD;
    SDValue ArgLoad = DAG.getExtLoad(
      Ext, dl, VT, Chain,
      DAG.getConstant(16 * CBIdx, PtrVT), // Convert to bytes.
      MachinePointerInfo(UndefValue::get(PtrTy)), // Only uses the CB index.
      ArgVT, false, true, 16);

    // XXX - What?
    Chain = SDValue(ArgLoad.getNode(), 1);

    Result.push_back(ArgLoad);
    CBIdx += (ArgSize + 16 - 1) / 16; // FIXME should get offset from CCValAssign
    return;
  }

  assert(!Arg.isSExt && !Arg.isZExt);
  // Handle the basic case when the arg is a scalar, or vector of elem >=
  // 32-bit.

  PointerType *PtrTy
    = PointerType::get(ArgVT.getTypeForEVT(*DAG.getContext()),
                       AMDILAS::CONSTANT_BUFFER_1);

  SDValue NewLoad = DAG.getLoad(VT,
                                dl,
                                Chain,
                                DAG.getConstant(16 * CBIdx, PtrVT),
                                MachinePointerInfo(UndefValue::get(PtrTy)),
                                false,
                                true,
                                false,
                                16);
  // XXX - What?
  Chain = SDValue(NewLoad.getNode(), 1);

  Result.push_back(NewLoad);
  CBIdx += (ArgSize + 16 - 1) / 16; // FIXME should get offset from CCValAssign
}

/// LowerCALL - functions arguments are copied from virtual
/// regs to (physical regs)/(stack frame), CALLSEQ_START and
/// CALLSEQ_END are emitted.
/// TODO: isVarArg, isTailCall, hasStructRet
SDValue AMDILTargetLowering::LowerCall(CallLoweringInfo &CLI,
                                       SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG                     = CLI.DAG;
  DebugLoc &DL                          = CLI.DL;
  SmallVector<ISD::OutputArg, 32> &Outs = CLI.Outs;
  SmallVector<SDValue, 32> &OutVals     = CLI.OutVals;
  SmallVector<ISD::InputArg, 32> &Ins   = CLI.Ins;
  SDValue Chain                         = CLI.Chain;
  SDValue Callee                        = CLI.Callee;
  ArgListTy &Args                       = CLI.Args;
  bool &IsTailCall                      = CLI.IsTailCall;
  CallingConv::ID CallConv              = CLI.CallConv;
  bool IsVarArg                         = CLI.IsVarArg;

  MachineFunction& MF = DAG.getMachineFunction();
  bool IsKernelStub
    = AMDSymbolNames::isStubFunctionName(MF.getFunction()->getName());
  bool IsStubToKernelCall = false;
  if (IsKernelStub) {
    const GlobalAddressSDNode *CalleeNode
      = cast<GlobalAddressSDNode>(Callee.getNode());
    const Function *CalleeFunc = cast<Function>(CalleeNode->getGlobal());
    IsStubToKernelCall
      = AMDSymbolNames::isKernelFunctionName(CalleeFunc->getName());
  }
  // FIXME: DO we need to handle fast calling conventions and tail call
  // optimizations?? X86/PPC ISelLowering
  /*bool hasStructRet = (TheCall->getNumArgs())
    ? TheCall->getArgFlags(0).device()->isSRet()
    : false;*/

  MachineFrameInfo *MFI = MF.getFrameInfo();

  // TODO: Handle tail calls
  IsTailCall = false;

  // Analyze operands of the call, assigning locations to each operand
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());
  // Analyize the calling operands, but need to change
  // if we have more than one calling convetion
  CCInfo.AnalyzeCallOperands(Outs, CCAssignFnForNode(CallConv, false, true /*, IsStubToKernelCall*/)); // XXX

  unsigned int NumBytes = CCInfo.getNextStackOffset();

    // See X86/PPC ISelLowering
  assert(!IsTailCall && "Tail Call not handled yet!");

  Chain = DAG.getCALLSEQ_START(Chain, DAG.getIntPtrConstant(NumBytes, true));

  SmallVector<std::pair<unsigned int, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;
  SDValue StackPtr = DAG.getCopyFromReg(Chain, DL, AMDIL::SP, getPointerTy());

  //unsigned int FirstStacArgLoc = 0;
  //int LastArgStackLoc = 0;

  // calculate how many parts each argument is split into
  SmallVector<unsigned, 4> ArgNumParts;
  if (IsStubToKernelCall) {
    for (unsigned i = 0, n = Args.size(); i < n; ++i) {
      unsigned NumParts = 0;
      SmallVector<EVT, 4> ValueVTs;
      ComputeValueVTs(*this, Args[i].Ty, ValueVTs);
      for (unsigned Value = 0, NumValues = ValueVTs.size();
        Value != NumValues; ++Value) {
        EVT VT = ValueVTs[Value];
        NumParts += getNumRegisters(CLI.RetTy->getContext(), VT);
      }
      ArgNumParts.push_back(NumParts);
    }
  }

  // Walk the register/memloc assignments, insert copies/loads.
  unsigned CBIdx = 0;
  for (unsigned i = 0, ArgIdx = 0, e = ArgLocs.size(); i != e; ++ArgIdx) {
    CCValAssign &VA = ArgLocs[i];
    SDValue OutVal = OutVals[i];
    const ISD::OutputArg &Arg = Outs[i];

    if (IsStubToKernelCall) {
      SmallVector<SDValue, 8> BuildArg;

      // FIXME: This should be handled in LowerFormalArguments with a kernel
      // calling convention. Handling this here at the "call site" for the
      // kernel is backwards.
      unsigned NumParts = ArgNumParts[ArgIdx];

      // If this is a call from a kernel stub to a kernel function, setup the
      // value to be passed into the kernel arg.
      BuildKernelArgVal(BuildArg, Args[ArgIdx], NumParts, OutVal,
                        VA.getValVT(), Arg.ArgVT, Chain, CBIdx, DAG);
      if (BuildArg.size() != 1) {
        // If the argument was scalarized or split, we have to copy each
        // component.
        for (unsigned K = 0; K != NumParts; ++K) {
          assert((VA.getLocInfo() == CCValAssign::Full ||
                  VA.getLocInfo() == CCValAssign::BCvt) && "Unimplemented");
          CCValAssign &VA = ArgLocs[i + K];
          assert(VA.isRegLoc());
          SDValue PromoteVal = promoteCCVal(DAG, DL, VA.getLocInfo(),
                                            VA.getLocVT(), BuildArg[K]);
          RegsToPass.push_back(std::make_pair(VA.getLocReg(), PromoteVal));
        }

        // We've processed all parts for this argument.
        i += BuildArg.size();
        continue;
      }

      assert(BuildArg.size() == 1);
      OutVal = BuildArg.front();
    }

    ++i;
    OutVal = promoteCCVal(DAG, DL, VA.getLocInfo(), VA.getLocVT(), OutVal);

    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), OutVal));
    } else if (VA.isMemLoc()) {
      // SP has been updated to top of stack at function prolog. Since our
      // stack grows upwards, start of callee's frame is at SP-NumBytes.
      unsigned LocMemOffset = VA.getLocMemOffset();
      SDValue PtrOff = DAG.getIntPtrConstant(LocMemOffset - NumBytes);
      PtrOff = DAG.getNode(ISD::ADD, DL, getPointerTy(), StackPtr, PtrOff);
      SDValue StoreOpToMem
        = DAG.getStore(Chain, DL, OutVal, PtrOff,
                       MachinePointerInfo::getStack(LocMemOffset),
                       false, false, 0);
      MemOpChains.push_back(StoreOpToMem);
    } else {
      llvm_unreachable("Not a Reg/Mem Loc, major error!");
    }
  }
  if (!MemOpChains.empty()) {
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other,
                        MemOpChains.data(), MemOpChains.size());
  }

  // Build a sequence of copy-to-reg nodes chained together with token chain
    // and flag operands which copy the outgoing args into the appropriate regs.
  SDValue InFlag;
  if (!IsTailCall) {
    for (unsigned int i = 0, e = RegsToPass.size(); i != e; ++i) {
      Chain = DAG.getCopyToReg(Chain, DL, RegsToPass[i].first,
        RegsToPass[i].second, InFlag);
      InFlag = Chain.getValue(1);
    }
  }
  // If the callee is a GlobalAddress/ExternalSymbol node (quite common,
  // every direct call is) turn it into a TargetGlobalAddress/
  // TargetExternalSymbol
  // node so that legalize doesn't hack it.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))  {
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), DL, getPointerTy());
  } else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    Callee = DAG.getTargetExternalSymbol(S->getSymbol(), getPointerTy());
  } else if (IsTailCall) {
    llvm_unreachable("Tail calls are not handled yet");
    // see X86 ISelLowering for ideas on implementation: 1708
  }

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;

  if (IsTailCall) {
    llvm_unreachable("Tail calls are not handled yet");
    // see X86 ISelLowering for ideas on implementation: 1721
  }
  // If this is a direct call, pass the chain and the callee
  if (Callee.getNode()) {
    Ops.push_back(Chain);
    Ops.push_back(Callee);
  }

  if (IsTailCall) {
    llvm_unreachable("Tail calls are not handled yet");
    // see X86 ISelLowering for ideas on implementation: 1739
  }

  // Add argument registers to the end of the list so that they are known
  // live into the call
  for (unsigned int i = 0, e = RegsToPass.size(); i != e; ++i) {
    Ops.push_back(DAG.getRegister(
          RegsToPass[i].first,
          RegsToPass[i].second.getValueType()));
  }

  // Add a register mask operand representing the call-preserved registers.
  const TargetRegisterInfo *TRI = getTargetMachine().getRegisterInfo();
  const uint32_t *Mask = TRI->getCallPreservedMask(CallConv);
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (InFlag.getNode()) {
    Ops.push_back(InFlag);
  }

  // Emit Tail Call
  if (IsTailCall) {
    llvm_unreachable("Tail calls are not handled yet");
    // see X86 ISelLowering for ideas on implementation: 1762
  }

  Chain = DAG.getNode(AMDILISD::CALL,
                      DL,
                      NodeTys,
                      &Ops[0],
                      Ops.size());
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node
  Chain = DAG.getCALLSEQ_END(
      Chain,
      DAG.getIntPtrConstant(NumBytes, true),
      DAG.getIntPtrConstant(0, true),
      InFlag);
  InFlag = Chain.getValue(1);
  // Handle result values, copying them out of physregs into vregs that
  // we return
  return LowerCallResult(Chain,
                         InFlag,
                         CallConv,
                         IsVarArg,
                         Ins,
                         DL,
                         DAG,
      InVals);
}

static uint32_t getMaskForBFI(SDValue &Op) {
  uint64_t lmask = 0;
  ConstantSDNode *RC = NULL;
  if (Op.getOpcode() == ISD::AND) {
    RC = dyn_cast<ConstantSDNode>(Op->getOperand(1));
    if (RC) lmask = RC->getZExtValue();
  } else if (Op.getOpcode() == ISD::SHL) {
    RC = dyn_cast<ConstantSDNode>(Op->getOperand(1));
    if (RC) lmask = (~0U << RC->getZExtValue());
  } else if (Op.getOpcode() == ISD::SRL) {
    RC = dyn_cast<ConstantSDNode>(Op->getOperand(1));
    if (RC) lmask = (~0U >> RC->getZExtValue());
  }

  return static_cast<uint32_t>(lmask);
}

SDValue AMDILTargetLowering::LowerOR(SDValue Op, SelectionDAG &DAG) const {
  EVT OVT = Op.getValueType();

  if (OVT.getScalarType() != MVT::i32 ||
      (getTargetMachine().getOptLevel() == CodeGenOpt::None)) {
    return SDValue(Op.getNode(), 0);
  }

  if (!getTargetMachine().Options.EnableBFO)
    return SDValue(Op.getNode(), 0);

  SDValue LOP = Op.getOperand(0);
  SDValue ROP = Op.getOperand(1);
  uint32_t lmask = getMaskForBFI(LOP);
  uint32_t rmask = getMaskForBFI(ROP);

  // Handles the (A & B) | (C & D) => BFI B, A, C | BFI D, C, A
  // Check for a constant mask.
  if (lmask && rmask && !(lmask & rmask)) {
    SDValue LHS, RHS, MASK;

    if (LOP.getOpcode() == ISD::AND &&
        ROP.getOpcode() == ISD::AND &&
        lmask == ~rmask) {
      // If the RHS is also an and with the inverse mask, we don't need to mask
      // it again.
      // e.g. (A & constant) | (B & ~constant) -> bfi constant, A, B
      LHS = LOP.getOperand(0);
      RHS = ROP.getOperand(0);
      MASK = LOP.getOperand(1); // Should be able to use either mask.
    } else if (LOP.getOpcode() == ISD::AND) {
      LHS = LOP.getOperand(0);
      RHS = ROP;
      MASK = LOP.getOperand(1);
    } else if (ROP.getOpcode() == ISD::AND) {
      LHS = ROP.getOperand(0);
      RHS = LOP;
      MASK = ROP.getOperand(1);
    } else {
      return SDValue(Op.getNode(), 0);
    }

    DebugLoc DL = Op.getDebugLoc();

    // bfi src0, src1, src2 = (src0 & src1) | (~src0 & src2)
    return DAG.getNode(AMDILISD::BFI, DL, OVT, MASK, LHS, RHS);

    // (A & B) | (C & (B ^ -1)) => BFI B, A, C is handled by instruction
    // patterns.
  }

  return SDValue(Op.getNode(), 0);
}

SDValue
AMDILTargetLowering::LowerADD(SDValue Op, SelectionDAG &DAG) const
{
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  DebugLoc DL = Op.getDebugLoc();
  EVT OVT = Op.getValueType();
  SDValue DST;
  const AMDILSubtarget *stm = &this->getTargetMachine()
    .getSubtarget<AMDILSubtarget>();
  bool isVec = OVT.isVector();
  if (OVT.getScalarType() == MVT::i64) {
    MVT INTTY = MVT::i32;
    if (OVT == MVT::v2i64) {
      INTTY = MVT::v2i32;
    }
    if (stm->usesHardware(AMDIL::Caps::LongOps)
        && INTTY == MVT::i32) {
      DST = SDValue(Op.getNode(), 0);
    } else {
      SDValue LHSLO, LHSHI, RHSLO, RHSHI, INTLO, INTHI;
      // TODO: need to turn this into a bitcast of i64/v2i64 to v2i32/v4i32
      LHSLO = DAG.getNode((isVec) ? AMDILISD::LCOMPLO2 : AMDILISD::LCOMPLO, DL, INTTY, LHS);
      RHSLO = DAG.getNode((isVec) ? AMDILISD::LCOMPLO2 : AMDILISD::LCOMPLO, DL, INTTY, RHS);
      LHSHI = DAG.getNode((isVec) ? AMDILISD::LCOMPHI2 : AMDILISD::LCOMPHI, DL, INTTY, LHS);
      RHSHI = DAG.getNode((isVec) ? AMDILISD::LCOMPHI2 : AMDILISD::LCOMPHI, DL, INTTY, RHS);
      INTLO = DAG.getNode(ISD::ADD, DL, INTTY, LHSLO, RHSLO);
      INTHI = DAG.getNode(ISD::ADD, DL, INTTY, LHSHI, RHSHI);
      SDValue cmp;
      cmp = DAG.getSetCC(DL, INTTY, INTLO, RHSLO, ISD::SETULT);
      INTHI = DAG.getNode(ISD::SUB, DL, INTTY, INTHI, cmp);
      DST = DAG.getNode((isVec) ? AMDILISD::LCREATE2 : AMDILISD::LCREATE, DL, OVT,
          INTLO, INTHI);
    }
  } else {
    if (LHS.getOpcode() == ISD::FrameIndex ||
        RHS.getOpcode() == ISD::FrameIndex) {
      DST = DAG.getNode(AMDILISD::ADDADDR,
          DL,
          OVT,
          LHS, RHS);
    } else {
      DST = SDValue(Op.getNode(), 0);
    }
  }
  return DST;
}
SDValue
AMDILTargetLowering::genCLZuN(SDValue Op, SelectionDAG &DAG,
    uint32_t bits) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT INTTY = Op.getValueType();
  EVT FPTY;
  if (INTTY.isVector()) {
    FPTY = EVT(MVT::getVectorVT(MVT::f32,
          INTTY.getVectorNumElements()));
  } else {
    FPTY = EVT(MVT::f32);
  }
  /* static inline uint
     __clz_Nbit(uint x)
     {
     int xor = 0x3f800000U | x;
     float tp = as_float(xor);
     float t = tp + -1.0f;
     uint tint = as_uint(t);
     int cmp = (x != 0);
     uint tsrc = tint >> 23;
     uint tmask = tsrc & 0xffU;
     uint cst = (103 + N)U - tmask;
     return cmp ? cst : N;
     }
     */
  assert(INTTY.getScalarType().getSimpleVT().SimpleTy == MVT::i32
      && "genCLZu16 only works on 32bit types");
  // uint x = Op
  SDValue x = Op;
  // xornode = 0x3f800000 | x
  SDValue xornode = DAG.getNode(ISD::OR, DL, INTTY,
      DAG.getConstant(0x3f800000, INTTY), x);
  // float tp = as_float(xornode)
  SDValue tp = DAG.getNode(ISD::BITCAST, DL, FPTY, xornode);
  // float t = tp + -1.0f
  SDValue t = DAG.getNode(ISD::FADD, DL, FPTY, tp,
      DAG.getConstantFP(-1.0f, FPTY));
  // uint tint = as_uint(t)
  SDValue tint = DAG.getNode(ISD::BITCAST, DL, INTTY, t);
  // int cmp = (x != 0)
  SDValue cmp = DAG.getSetCC(DL, INTTY, x, DAG.getConstant(0, INTTY), ISD::SETNE);
  // uint tsrc = tint >> 23
  SDValue tsrc = DAG.getNode(ISD::SRL, DL, INTTY, tint,
      DAG.getConstant(23, INTTY));
  // uint tmask = tsrc & 0xFF
  SDValue tmask = DAG.getNode(ISD::AND, DL, INTTY, tsrc,
      DAG.getConstant(0xFFU, INTTY));
  // uint cst = (103 + bits) - tmask
  SDValue cst = DAG.getNode(ISD::SUB, DL, INTTY,
      DAG.getConstant((103U + bits), INTTY), tmask);
  // return cmp ? cst : N
  cst = DAG.getSelect(DL, INTTY, cmp, cst,
      DAG.getConstant(bits, INTTY));
  return cst;
}

SDValue
AMDILTargetLowering::genCLZu32(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT INTTY = Op.getValueType();
    //__clz_32bit(uint u)
    //{
    // int z = __amdil_ffb_hi(u) ;
    // return z < 0 ? 32 : z;
    // }
    // uint u = op
    SDValue u = Op;
    // int z = __amdil_ffb_hi(u)
    SDValue z = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, INTTY,
        DAG.getConstant(AMDILIntrinsic::AMDIL_bit_find_first_hi, MVT::i32),
        u);
    // int cmp = z < 0
    SDValue cmp = DAG.getSetCC(DL, INTTY, z, DAG.getConstant(0, INTTY), ISD::SETLT);
    // return cmp ? 32 : z
  return DAG.getSelect(DL, INTTY, cmp,
        DAG.getConstant(32, INTTY), z);
  }
SDValue
AMDILTargetLowering::genCLZu64(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT INTTY;
  EVT LONGTY = Op.getValueType();
  bool isVec = LONGTY.isVector();
  if (isVec) {
    INTTY = EVT(MVT::getVectorVT(MVT::i32, Op.getValueType()
          .getVectorNumElements()));
  } else {
    INTTY = EVT(MVT::i32);
  }

    // Evergreen:
    // static inline uint
    // __clz_u64(ulong x)
    // {
    //uint zhi = __clz_32bit((uint)(x >> 32));
    //uint zlo = __clz_32bit((uint)(x & 0xffffffffUL));
    //return zhi == 32U ? 32U + zlo : zhi;
    //}
    //ulong x = op
    SDValue x = Op;
    // uint xhi = x >> 32
    SDValue xlo = DAG.getNode((isVec) ? AMDILISD::LCOMPLO2 : AMDILISD::LCOMPLO, DL, INTTY, x);
    // uint xlo = x & 0xFFFFFFFF
    SDValue xhi = DAG.getNode((isVec) ? AMDILISD::LCOMPHI2 : AMDILISD::LCOMPHI, DL, INTTY, x);
    // uint zhi = __clz_32bit(xhi)
    SDValue zhi = genCLZu32(xhi, DAG);
    // uint zlo = __clz_32bit(xlo)
    SDValue zlo = genCLZu32(xlo, DAG);
    // uint cmp = zhi == 32
    SDValue cmp = DAG.getSetCC(DL, INTTY, zhi, DAG.getConstant(32U, INTTY), ISD::SETEQ);
    // uint zlop32 = 32 + zlo
    SDValue zlop32 = DAG.getNode(ISD::ADD, DL, INTTY,
        DAG.getConstant(32U, INTTY), zlo);
    // return cmp ? zlop32: zhi
  return DAG.getSelect(DL, INTTY, cmp, zlop32, zhi);
  }

SDValue
AMDILTargetLowering::genf32toi64(SDValue RHS, SelectionDAG &DAG,
    bool includeSign) const
{
  DebugLoc DL = RHS.getDebugLoc();
  EVT RHSVT = RHS.getValueType();
  bool isVec = RHSVT.isVector();
  EVT LHSVT = (isVec) ? MVT::v2i64 : MVT::i64;
  EVT INTVT = (isVec) ? MVT::v2i32 : MVT::i32;
  //cf2ul(float f)
  //{
  //  float fh = f * 0x1.0p-32f;
  //  uint uh = (uint)fh;
  //  float fuh = (float)uh;
  //  float fl = mad(-0x1.0p+32f, fuh, f);
  //  uint ul = (uint)fl;
  //  return ((ulong)uh << 32) | (ulong)ul;
  //}
  // Signed
  //cf2l(float f)
  //{
  //  int s = as_int(f) & 0x80000000;
  //  ulong u = cf2ul(as_float(as_uint(f) ^ s));
  //  long ls = s ? -1L : 0L;
  //  return ((long)u + ls) ^ ls;
  //}
  SDValue fh, uh, fuh, fl, ul, r, s, f;
  f = RHS;
  if (includeSign) {
    SDValue fi = DAG.getNode(ISD::BITCAST, DL, INTVT, f);
    s = DAG.getNode(ISD::AND, DL, INTVT,
        fi, DAG.getConstant(0x80000000, INTVT));
    f = DAG.getNode(ISD::BITCAST, DL, RHSVT,
        DAG.getNode(ISD::XOR, DL, INTVT, fi, s));
  }
  fh = DAG.getNode(ISD::FMUL, DL, RHSVT,
      DAG.getNode(ISD::BITCAST, DL, RHSVT,
        DAG.getConstant(0x2F800000, INTVT)), f);
  uh = DAG.getNode(ISD::FP_TO_UINT, DL, INTVT, fh);
  fuh = DAG.getNode(ISD::UINT_TO_FP, DL, RHSVT, uh);
  fl = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, RHSVT,
      DAG.getConstant(AMDILIntrinsic::AMDIL_mad, MVT::i32),
      DAG.getNode(ISD::BITCAST, DL, RHSVT,
        DAG.getConstant(0xCF800000, INTVT)), fuh, f);
  ul = DAG.getNode(ISD::FP_TO_UINT, DL, INTVT, fl);
  r = DAG.getNode(ISD::OR, DL, LHSVT,
      DAG.getNode(ISD::SHL, DL, LHSVT,
        DAG.getZExtOrTrunc(uh, DL, LHSVT), DAG.getConstant(32, LHSVT)),
        DAG.getZExtOrTrunc(ul, DL, LHSVT));
  if (includeSign) {
    SDValue ls = DAG.getSelect(DL, LHSVT,
        DAG.getZExtOrTrunc(s, DL, LHSVT),
        DAG.getConstant(-1L, LHSVT),
        DAG.getConstant(0L, LHSVT));
    r = DAG.getNode(ISD::ADD, DL, LHSVT, r, ls);
    r = DAG.getNode(ISD::XOR, DL, LHSVT, r, ls);
  }
  return r;
}

SDValue AMDILTargetLowering::geni64tof32(SDValue RHS,
                                         SelectionDAG &DAG,
                                         bool IncludeSign) const {
  DebugLoc DL = RHS.getDebugLoc();
  EVT RHSVT = RHS.getValueType();
  bool IsVec = RHSVT.isVector();
  EVT LHSVT = IsVec ? MVT::v2f32 : MVT::f32;
  EVT INTVT = IsVec ? MVT::v2i32 : MVT::i32;
  // Unsigned
  // cul2f(ulong u)
  //{
  //  uint lz = clz(u);
  //  uint e = (u != 0) ? 127U + 63U - lz : 0;
  //  u = (u << lz) & 0x7fffffffffffffffUL;
  //  ulong t = u & 0xffffffffffUL;
  //  uint v = (e << 23) | (uint)(u >> 40);
  //  uint r = t > 0x8000000000UL ? 1U : (t == 0x8000000000UL ? v & 1U : 0U);
  //  return as_float(v + r);
  //}
  // Signed
  // cl2f(long l)
  //{
  //  long s = l >> 63;
  //  float r = cul2f((l + s) ^ s);
  //  return s ? -r : r;
  //}
  SDValue l = RHS;

  SDValue s;
  if (IncludeSign) {
    s = DAG.getNode(ISD::SRA, DL, RHSVT, l,
        DAG.getConstant(63, RHSVT));
    SDValue s_add = DAG.getNode(ISD::ADD, DL, RHSVT,
        l, s);
    l = DAG.getNode(ISD::XOR, DL, RHSVT, s_add, s);
  }

  SDValue ZeroI32 = DAG.getConstant(0U, INTVT);
  SDValue lz = genCLZu64(l, DAG);
  SDValue e = DAG.getSelect(DL, INTVT,
      DAG.getZExtOrTrunc(
        DAG.getSetCC(DL, getSetCCResultType(*DAG.getContext(), RHSVT), l,
                     DAG.getConstant(0, RHSVT), ISD::SETNE),
      DL, INTVT),
      DAG.getNode(ISD::SUB, DL, INTVT, DAG.getConstant(127U + 63U, INTVT), lz),
      ZeroI32);
  SDValue u = DAG.getNode(ISD::AND, DL, RHSVT,
      DAG.getNode(ISD::SHL, DL, RHSVT, l, lz),
      DAG.getConstant((-1ULL) >> 1, RHSVT));
  SDValue t = DAG.getNode(ISD::AND, DL, RHSVT, u,
      DAG.getConstant(0xffffffffffULL, RHSVT));
  SDValue v = DAG.getNode(ISD::OR, DL, INTVT,
      DAG.getNode(ISD::SHL, DL, INTVT, e, DAG.getConstant(23, INTVT)),
      DAG.getZExtOrTrunc(
        DAG.getNode(ISD::SRL, DL, RHSVT, u, DAG.getConstant(40, RHSVT)),
        DL, INTVT));
  SDValue C = DAG.getConstant(0x8000000000ULL, RHSVT);
  SDValue r_cmp = DAG.getZExtOrTrunc(
      DAG.getSetCC(DL, getSetCCResultType(*DAG.getContext(), RHSVT), t, C,
        ISD::SETUGT), DL, INTVT);
  SDValue t_cmp = DAG.getZExtOrTrunc(
      DAG.getSetCC(DL, getSetCCResultType(*DAG.getContext(), RHSVT), t, C,
        ISD::SETEQ), DL, INTVT);
  SDValue r = DAG.getSelect(DL, INTVT,
      r_cmp, DAG.getConstant(1U, INTVT),
      DAG.getSelect(DL, INTVT, t_cmp,
        DAG.getNode(ISD::AND, DL, INTVT, v, DAG.getConstant(1U, INTVT)),
        ZeroI32));
  r = DAG.getNode(ISD::ADD, DL, INTVT, v, r);
  r = DAG.getNode(ISD::BITCAST, DL, LHSVT, r);

  if (!IncludeSign) {
    return r;
  }

  SDValue r_neg = DAG.getNode(ISD::FNEG, DL, LHSVT, r);
  return DAG.getSelect(DL, LHSVT,
                       DAG.getSExtOrTrunc(s, DL, getSetCCResultType(*DAG.getContext(), LHSVT)),
                       r_neg, r);
}

SDValue
AMDILTargetLowering::genf64toi64(SDValue RHS, SelectionDAG &DAG,
    bool includeSign) const
{
  EVT INTVT;
  EVT LONGVT;
  SDValue DST;
  DebugLoc DL = RHS.getDebugLoc();
  EVT RHSVT = RHS.getValueType();
  bool isVec = RHSVT.isVector();
  if (isVec) {
    LONGVT = EVT(MVT::getVectorVT(MVT::i64, RHSVT
          .getVectorNumElements()));
    INTVT = EVT(MVT::getVectorVT(MVT::i32, RHSVT
          .getVectorNumElements()));
  } else {
    LONGVT = EVT(MVT::i64);
    INTVT = EVT(MVT::i32);
  }
  const AMDILSubtarget *stm = reinterpret_cast<const AMDILTargetMachine*>(
      &this->getTargetMachine())->getSubtargetImpl();
  if (0 && stm->getGeneration() > AMDIL::NORTHERN_ISLANDS) {
    // unsigned version:
    // uint uhi = (uint)(d * 0x1.0p-32);
    // uint ulo = (uint)(mad((double)uhi, -0x1.0p+32, d));
    // return as_ulong2((uint2)(ulo, uhi));
    //
    // signed version:
    // double ad = fabs(d);
    // long l = unsigned_version(ad);
    // long nl = -l;
    // return d == ad ? l : nl;
    SDValue d = RHS;
    if (includeSign) {
      d = DAG.getNode(ISD::FABS, DL, RHSVT, d);
    }
    uint64_t val = 0x3DF0000000000000ULL;
    double dval = *(double*)&val;
    SDValue uhid = DAG.getNode(ISD::FMUL, DL, RHSVT, d,
        DAG.getConstantFP(dval, RHSVT));
    SDValue uhi = DAG.getNode(ISD::FP_TO_UINT, DL, INTVT, uhid);
    SDValue ulod = DAG.getNode(ISD::UINT_TO_FP, DL, RHSVT, uhi);
    val = 0xC1F0000000000000ULL;
    dval = *(double*)&val;
    ulod = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, RHSVT,
        DAG.getConstant(AMDILIntrinsic::AMDIL_mad, MVT::i32),
        ulod, DAG.getConstantFP(dval, RHSVT), d);
    SDValue ulo = DAG.getNode(ISD::FP_TO_UINT, DL, INTVT, ulod);
    SDValue l = DAG.getNode((isVec) ? AMDILISD::LCREATE2 : AMDILISD::LCREATE, DL, LONGVT, ulo, uhi);
    if (includeSign) {
      SDValue nl = DAG.getNode(ISD::XOR, DL, LONGVT, l, DAG.getConstant(~0ULL, LONGVT));
      SDValue c = DAG.getSetCC(DL, getSetCCResultType(*DAG.getContext(), RHSVT), RHS, d, ISD::SETEQ);
      l = DAG.getSelect(DL, LONGVT, c, l, nl);
    }
    DST = l;
  } else {
    /*
       __attribute__((always_inline)) long
       cast_f64_to_i64(double d)
       {
    // Convert d in to 32-bit components
    long x = as_long(d);
    xhi = LCOMPHI(x);
    xlo = LCOMPLO(x);

    // Generate 'normalized' mantissa
    mhi = xhi | 0x00100000; // hidden bit
    mhi <<= 11;
    temp = xlo >> (32 - 11);
    mhi |= temp
    mlo = xlo << 11;

    // Compute shift right count from exponent
    e = (xhi >> (52-32)) & 0x7ff;
    sr = 1023 + 63 - e;
    srge64 = sr >= 64;
    srge32 = sr >= 32;

    // Compute result for 0 <= sr < 32
    rhi0 = mhi >> (sr &31);
    rlo0 = mlo >> (sr &31);
    temp = mhi << (32 - sr);
    temp |= rlo0;
    rlo0 = sr ? temp : rlo0;

    // Compute result for 32 <= sr
    rhi1 = 0;
    rlo1 = srge64 ? 0 : rhi0;

    // Pick between the 2 results
    rhi = srge32 ? rhi1 : rhi0;
    rlo = srge32 ? rlo1 : rlo0;

    // Optional saturate on overflow
    srlt0 = sr < 0;
    rhi = srlt0 ? MAXVALUE : rhi;
    rlo = srlt0 ? MAXVALUE : rlo;

    // Create long
    res = LCREATE( rlo, rhi );

    // Deal with sign bit (ignoring whether result is signed or unsigned value)
    if (includeSign) {
    sign = ((signed int) xhi) >> 31; fill with sign bit
    sign = LCREATE( sign, sign );
    res += sign;
    res ^= sign;
    }

    return res;
    }
    */
    SDValue c11 = DAG.getConstant( 63 - 52, INTVT );
    SDValue c32 = DAG.getConstant( 32, INTVT );

    // Convert d in to 32-bit components
    SDValue d = RHS;
    SDValue x = DAG.getNode(ISD::BITCAST, DL, LONGVT, d);
    SDValue xhi = DAG.getNode( (isVec) ? AMDILISD::LCOMPHI2 : AMDILISD::LCOMPHI, DL, INTVT, x );
    SDValue xlo = DAG.getNode( (isVec) ? AMDILISD::LCOMPLO2 : AMDILISD::LCOMPLO, DL, INTVT, x );

    // Generate 'normalized' mantissa
    SDValue mhi = DAG.getNode( ISD::OR, DL, INTVT,
        xhi, DAG.getConstant( 0x00100000, INTVT ) );
    mhi = DAG.getNode( ISD::SHL, DL, INTVT, mhi, c11 );
    SDValue temp = DAG.getNode( ISD::SRL, DL, INTVT,
        xlo, DAG.getConstant( 32 - (63 - 52), INTVT ) );
    mhi = DAG.getNode( ISD::OR, DL, INTVT, mhi, temp );
    SDValue mlo = DAG.getNode( ISD::SHL, DL, INTVT, xlo, c11 );

    // Compute shift right count from exponent
    SDValue e = DAG.getNode( ISD::SRL, DL, INTVT,
        xhi, DAG.getConstant( 52-32, INTVT ) );
    e = DAG.getNode( ISD::AND, DL, INTVT,
        e, DAG.getConstant( 0x7ff, INTVT ) );
    SDValue sr = DAG.getNode( ISD::SUB, DL, INTVT,
        DAG.getConstant( 1023 + 63, INTVT ), e );
    SDValue srge64 = DAG.getSetCC(DL, INTVT, sr, DAG.getConstant(64, INTVT), ISD::SETGE);
    SDValue srge32 = DAG.getSetCC(DL, INTVT, sr, DAG.getConstant(32, INTVT), ISD::SETGE);

    // Compute result for 0 <= sr < 32
    SDValue rhi0 = DAG.getNode( ISD::SRL, DL, INTVT, mhi, sr );
    SDValue rlo0 = DAG.getNode( ISD::SRL, DL, INTVT, mlo, sr );
    temp = DAG.getNode( ISD::SUB, DL, INTVT, c32, sr );
    temp = DAG.getNode( ISD::SHL, DL, INTVT, mhi, temp );
    temp = DAG.getNode( ISD::OR,  DL, INTVT, rlo0, temp );
    rlo0 = DAG.getNode( ISD::SELECT, DL, INTVT, sr, temp, rlo0 );

    // Compute result for 32 <= sr
    SDValue rhi1 = DAG.getConstant( 0, INTVT );
    SDValue rlo1 = DAG.getNode( ISD::SELECT, DL, INTVT,
        srge64, rhi1, rhi0 );

    // Pick between the 2 results
    SDValue rhi = DAG.getNode( ISD::SELECT, DL, INTVT,
        srge32, rhi1, rhi0 );
    SDValue rlo = DAG.getNode( ISD::SELECT, DL, INTVT,
        srge32, rlo1, rlo0 );

    // Create long
    SDValue res = DAG.getNode( (isVec) ? AMDILISD::LCREATE2 : AMDILISD::LCREATE, DL, LONGVT, rlo, rhi );

    // Deal with sign bit
    if (includeSign) {
      SDValue sign = DAG.getNode( ISD::SRA, DL, INTVT,
          xhi, DAG.getConstant( 31, INTVT ) );
      sign = DAG.getNode( (isVec) ? AMDILISD::LCREATE2 : AMDILISD::LCREATE, DL, LONGVT, sign, sign );
      res = DAG.getNode( ISD::ADD, DL, LONGVT, res, sign );
      res = DAG.getNode( ISD::XOR, DL, LONGVT, res, sign );
    }
    DST = res;
  }
  return DST;
}
SDValue
AMDILTargetLowering::genf64toi32(SDValue RHS, SelectionDAG &DAG,
    bool includeSign) const
{
  EVT INTVT;
  EVT LONGVT;
  DebugLoc DL = RHS.getDebugLoc();
  EVT RHSVT = RHS.getValueType();
  bool isVec = RHSVT.isVector();
  if (isVec) {
    LONGVT = EVT(MVT::getVectorVT(MVT::i64,
          RHSVT.getVectorNumElements()));
    INTVT = EVT(MVT::getVectorVT(MVT::i32,
          RHSVT.getVectorNumElements()));
  } else {
    LONGVT = EVT(MVT::i64);
    INTVT = EVT(MVT::i32);
  }
  /*
     __attribute__((always_inline)) int
     cast_f64_to_[u|i]32(double d)
     {
  // Convert d in to 32-bit components
  long x = as_long(d);
  xhi = LCOMPHI(x);
  xlo = LCOMPLO(x);

  // Generate 'normalized' mantissa
  mhi = xhi | 0x00100000; // hidden bit
  mhi <<= 11;
  temp = xlo >> (32 - 11);
  mhi |= temp

  // Compute shift right count from exponent
  e = (xhi >> (52-32)) & 0x7ff;
  sr = 1023 + 31 - e;
  srge32 = sr >= 32;

  // Compute result for 0 <= sr < 32
  res = mhi >> (sr &31);
  res = srge32 ? 0 : res;

  // Optional saturate on overflow
  srlt0 = sr < 0;
  res = srlt0 ? MAXVALUE : res;

  // Deal with sign bit (ignoring whether result is signed or unsigned value)
  if (includeSign) {
  sign = ((signed int) xhi) >> 31; fill with sign bit
  res += sign;
  res ^= sign;
  }

  return res;
  }
  */
  SDValue c11 = DAG.getConstant( 63 - 52, INTVT );

  // Convert d in to 32-bit components
  SDValue d = RHS;
  SDValue x = DAG.getNode(ISD::BITCAST, DL, LONGVT, d);
  SDValue xhi = DAG.getNode( (isVec) ? AMDILISD::LCOMPHI2 : AMDILISD::LCOMPHI, DL, INTVT, x );
  SDValue xlo = DAG.getNode( (isVec) ? AMDILISD::LCOMPLO2 : AMDILISD::LCOMPLO, DL, INTVT, x );

  // Generate 'normalized' mantissa
  SDValue mhi = DAG.getNode( ISD::OR, DL, INTVT,
      xhi, DAG.getConstant( 0x00100000, INTVT ) );
  mhi = DAG.getNode( ISD::SHL, DL, INTVT, mhi, c11 );
  SDValue temp = DAG.getNode( ISD::SRL, DL, INTVT,
      xlo, DAG.getConstant( 32 - (63 - 52), INTVT ) );
  mhi = DAG.getNode( ISD::OR, DL, INTVT, mhi, temp );

  // Compute shift right count from exponent
  SDValue e = DAG.getNode( ISD::SRL, DL, INTVT,
      xhi, DAG.getConstant( 52-32, INTVT ) );
  e = DAG.getNode( ISD::AND, DL, INTVT,
      e, DAG.getConstant( 0x7ff, INTVT ) );
  SDValue sr = DAG.getNode( ISD::SUB, DL, INTVT,
      DAG.getConstant( 1023 + 31, INTVT ), e );
  SDValue srge32 = DAG.getSetCC(DL, INTVT, sr, DAG.getConstant(32, INTVT), ISD::SETGE);

  // Compute result for 0 <= sr < 32
  SDValue res = DAG.getNode( ISD::SRL, DL, INTVT, mhi, sr );
  res = DAG.getNode( ISD::SELECT, DL, INTVT,
      srge32, DAG.getConstant(0,INTVT), res );

  // Deal with sign bit
  if (includeSign) {
    SDValue sign = DAG.getNode( ISD::SRA, DL, INTVT,
        xhi, DAG.getConstant( 31, INTVT ) );
    res = DAG.getNode( ISD::ADD, DL, INTVT, res, sign );
    res = DAG.getNode( ISD::XOR, DL, INTVT, res, sign );
  }
  return res;
}
SDValue
AMDILTargetLowering::LowerFP_TO_SINT(SDValue Op, SelectionDAG &DAG) const
{
  SDValue RHS = Op.getOperand(0);
  EVT RHSVT = RHS.getValueType();
  MVT RST = RHSVT.getScalarType().getSimpleVT();
  EVT LHSVT = Op.getValueType();
  MVT LST = LHSVT.getScalarType().getSimpleVT();
  DebugLoc DL = Op.getDebugLoc();
  SDValue DST;
  const AMDILTargetMachine*
    amdtm = reinterpret_cast<const AMDILTargetMachine*>
    (&this->getTargetMachine());
  const AMDILSubtarget*
    stm = dynamic_cast<const AMDILSubtarget*>(
        amdtm->getSubtargetImpl());
  if (RST == MVT::f64 && RHSVT.isVector()) {
    // We dont support vector 64bit floating point convertions.
    for (unsigned x = 0, y = RHSVT.getVectorNumElements(); x < y; ++x) {
      SDValue op = DAG.getNode(ISD::EXTRACT_VECTOR_ELT,
          DL, RST, RHS, DAG.getTargetConstant(x, MVT::i32));
      op = DAG.getNode(ISD::FP_TO_SINT, DL, LST, op);
      if (!x) {
        DST = DAG.getNode(AMDILISD::VBUILD, DL, LHSVT, op);
      } else {
        DST = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL, LHSVT,
            DST, op, DAG.getTargetConstant(x, MVT::i32));
      }
    }
  } else if (RST == MVT::f64
      && LST == MVT::i32) {
    if (!RHSVT.isVector() &&
        stm->getGeneration() >= AMDIL::EVERGREEN) {
      DST = SDValue(Op.getNode(), 0);
    } else {
      DST = genf64toi32(RHS, DAG, true);
    }
  } else if (RST == MVT::f64
      && LST == MVT::i64) {
    DST = genf64toi64(RHS, DAG, true);
  } else if (RST == MVT::f64
      && (LST == MVT::i8 || LST == MVT::i16)) {
    if (!RHSVT.isVector()) {
      DST = DAG.getNode(ISD::FP_TO_SINT, DL, MVT::i32, RHS);
      DST = DAG.getNode(ISD::TRUNCATE, DL, LHSVT, DST);
    } else {
      SDValue ToInt = genf64toi32(RHS, DAG, true);
      DST = DAG.getNode(ISD::TRUNCATE, DL, LHSVT, ToInt);
    }
  } else if (RST == MVT::f32
      && LST == MVT::i64) {
    DST = genf32toi64(RHS, DAG, true);
  } else {
    DST = SDValue(Op.getNode(), 0);
  }
  return DST;
}

SDValue
AMDILTargetLowering::LowerFP_TO_UINT(SDValue Op, SelectionDAG &DAG) const
{
  SDValue DST;
  SDValue RHS = Op.getOperand(0);
  EVT RHSVT = RHS.getValueType();
  MVT RST = RHSVT.getScalarType().getSimpleVT();
  EVT LHSVT = Op.getValueType();
  MVT LST = LHSVT.getScalarType().getSimpleVT();
  DebugLoc DL = Op.getDebugLoc();
  const AMDILTargetMachine*
    amdtm = reinterpret_cast<const AMDILTargetMachine*>
    (&this->getTargetMachine());
  const AMDILSubtarget*
    stm = dynamic_cast<const AMDILSubtarget*>(
        amdtm->getSubtargetImpl());
  if (RST == MVT::f64 && RHSVT.isVector()) {
    // We dont support vector 64bit floating point convertions.
    for (unsigned x = 0, y = RHSVT.getVectorNumElements(); x < y; ++x) {
      SDValue op = DAG.getNode(ISD::EXTRACT_VECTOR_ELT,
          DL, RST, RHS, DAG.getTargetConstant(x, MVT::i32));
      op = DAG.getNode(ISD::FP_TO_UINT, DL, LST, op);
      if (!x) {
        DST = DAG.getNode(AMDILISD::VBUILD, DL, LHSVT, op);
      } else {
        DST = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL, LHSVT,
            DST, op, DAG.getTargetConstant(x, MVT::i32));
      }

    }
  } else if (RST == MVT::f64
      && LST == MVT::i32) {
    if (!RHSVT.isVector() &&
        stm->getGeneration() >= AMDIL::EVERGREEN) {
      DST = SDValue(Op.getNode(), 0);
    } else {
      DST = genf64toi32(RHS, DAG, false);
    }
  } else if (RST == MVT::f64
      && LST == MVT::i64) {
    DST = genf64toi64(RHS, DAG, false);
  } else if (RST == MVT::f64
      && (LST == MVT::i8 || LST == MVT::i16)) {
    if (!RHSVT.isVector()) {
      DST = DAG.getNode(ISD::FP_TO_UINT, DL, MVT::i32, RHS);
      DST = DAG.getNode(ISD::TRUNCATE, DL, LHSVT, DST);
    } else {
      SDValue ToInt = genf64toi32(RHS, DAG, false);
      DST = DAG.getNode(ISD::TRUNCATE, DL, LHSVT, ToInt);
    }
  } else if (RST == MVT::f32
      && LST == MVT::i64) {
    DST = genf32toi64(RHS, DAG, false);
  } else {
    DST = SDValue(Op.getNode(), 0);
  }
  return DST;
}

SDValue
AMDILTargetLowering::genu32tof64(SDValue RHS, EVT LHSVT,
    SelectionDAG &DAG) const
{
  EVT RHSVT = RHS.getValueType();
  DebugLoc DL = RHS.getDebugLoc();
  EVT INTVT;
  EVT LONGVT;
  bool isVec = RHSVT.isVector();
  if (isVec) {
    LONGVT = EVT(MVT::getVectorVT(MVT::i64,
          RHSVT.getVectorNumElements()));
    INTVT = EVT(MVT::getVectorVT(MVT::i32,
          RHSVT.getVectorNumElements()));
  } else {
    LONGVT = EVT(MVT::i64);
    INTVT = EVT(MVT::i32);
  }
  SDValue x = RHS;
    // unsigned x = RHS;
    // ulong xd = (ulong)(0x4330_0000 << 32) | x;
    // double d = as_double( xd );
    // return d - 0x1.0p+52; // 0x1.0p+52 == 0x4330_0000_0000_0000
    SDValue xd = DAG.getNode( (isVec) ? AMDILISD::LCREATE2 : AMDILISD::LCREATE, DL, LONGVT, x,
        DAG.getConstant( 0x43300000, INTVT ) );
  SDValue d = DAG.getNode(ISD::BITCAST, DL, LHSVT, xd );
  SDValue offsetd = DAG.getNode(ISD::BITCAST, DL, LHSVT,
        DAG.getConstant( 0x4330000000000000ULL, LONGVT ) );
    return DAG.getNode( ISD::FSUB, DL, LHSVT, d, offsetd );
  }

SDValue
AMDILTargetLowering::genu64tof64(SDValue RHS, EVT LHSVT,
    SelectionDAG &DAG) const
{
  EVT RHSVT = RHS.getValueType();
  DebugLoc DL = RHS.getDebugLoc();
  EVT INTVT;
  EVT LONGVT;
  bool isVec = RHSVT.isVector();
  if (isVec) {
    INTVT = EVT(MVT::getVectorVT(MVT::i32,
          RHSVT.getVectorNumElements()));
  } else {
    INTVT = EVT(MVT::i32);
  }
  LONGVT = RHSVT;
  SDValue x = RHS;
  const AMDILSubtarget *stm = reinterpret_cast<const AMDILTargetMachine*>(
      &this->getTargetMachine())->getSubtargetImpl();
  if (0 && stm->getGeneration() > AMDIL::NORTHERN_ISLANDS) {
    // double dhi = (double)(as_uint2(x).y);
    // double dlo = (double)(as_uint2(x).x);
    // return mad(dhi, 0x1.0p+32, dlo)
    SDValue dhi = DAG.getNode((isVec) ? AMDILISD::LCOMPHI2 : AMDILISD::LCOMPHI, DL, INTVT, x);
    dhi = DAG.getNode(ISD::UINT_TO_FP, DL, LHSVT, dhi);
    SDValue dlo = DAG.getNode((isVec) ? AMDILISD::LCOMPLO2 : AMDILISD::LCOMPLO, DL, INTVT, x);
    dlo = DAG.getNode(ISD::UINT_TO_FP, DL, LHSVT, dlo);
    uint64_t val = 0x41f0000000000000ULL;
    double dval = *(double*)&val;
    return DAG.getNode(ISD::INTRINSIC_WO_CHAIN,
        DL, LHSVT,
        DAG.getConstant(AMDILIntrinsic::AMDIL_mad, MVT::i32),
        dhi, DAG.getConstantFP(dval, LHSVT), dlo);
  } else {
    // double lo = as_double( as_ulong( 0x1.0p+52) | (u & 0xffff_ffffUL));
    // double hi = as_double( as_ulong( 0x1.0p+84) | (u >> 32));
    // return (hi - (0x1.0p+84 + 0x1.0p+52)) + lo;
    SDValue xlo = DAG.getNode( (isVec) ? AMDILISD::LCOMPLO2 : AMDILISD::LCOMPLO, DL, INTVT, x );  // x & 0xffff_ffffUL
    SDValue xd = DAG.getNode( (isVec) ? AMDILISD::LCREATE2 : AMDILISD::LCREATE, DL, LONGVT, xlo, DAG.getConstant( 0x43300000, INTVT ) );
    SDValue lo = DAG.getNode( ISD::BITCAST, DL, LHSVT, xd );
    SDValue xhi = DAG.getNode((isVec) ? AMDILISD::LCOMPHI2 :  AMDILISD::LCOMPHI, DL, INTVT, x ); // x >> 32
    SDValue xe = DAG.getNode( (isVec) ? AMDILISD::LCREATE2 : AMDILISD::LCREATE, DL, LONGVT, xhi, DAG.getConstant( 0x45300000, INTVT ) );
    SDValue hi = DAG.getNode( ISD::BITCAST, DL, LHSVT, xe );
    SDValue c = DAG.getNode( ISD::BITCAST, DL, LHSVT,
        DAG.getConstant( 0x4530000000100000ULL, LONGVT ) );
    hi = DAG.getNode( ISD::FSUB, DL, LHSVT, hi, c );
    return DAG.getNode( ISD::FADD, DL, LHSVT, hi, lo );
  }
}
SDValue
AMDILTargetLowering::LowerUINT_TO_FP(SDValue Op, SelectionDAG &DAG) const
{
  SDValue RHS = Op.getOperand(0);
  EVT RHSVT = RHS.getValueType();
  MVT RST = RHSVT.getScalarType().getSimpleVT();
  EVT LHSVT = Op.getValueType();
  MVT LST = LHSVT.getScalarType().getSimpleVT();
  DebugLoc DL = Op.getDebugLoc();
  SDValue DST;
  EVT INTVT;
  EVT LONGVT;

  if (LST == MVT::f64 && LHSVT.isVector()) {
    // We dont support vector 64bit floating point convertions.
    DST = Op;
    for (unsigned x = 0, y = LHSVT.getVectorNumElements(); x < y; ++x) {
      SDValue op = DAG.getNode(ISD::EXTRACT_VECTOR_ELT,
          DL, RST, RHS, DAG.getTargetConstant(x, MVT::i32));
      op = DAG.getNode(ISD::UINT_TO_FP, DL, LST, op);
      if (!x) {
        DST = DAG.getNode(AMDILISD::VBUILD, DL, LHSVT, op);
      } else {
        DST = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL, LHSVT, DST,
            op, DAG.getTargetConstant(x, MVT::i32));
      }
    }
  } else if (RST == MVT::i32
      && LST == MVT::f64) {
      DST = SDValue(Op.getNode(), 0);
  } else if (RST == MVT::i64
      && LST == MVT::f64) {
    DST = genu64tof64(RHS, LHSVT, DAG);
  } else if (RST == MVT::i64
      && LST == MVT::f32) {
    DST = geni64tof32(RHS, DAG, false);
  } else {
    DST = SDValue(Op.getNode(), 0);
  }
  return DST;
}

SDValue
AMDILTargetLowering::LowerSINT_TO_FP(SDValue Op, SelectionDAG &DAG) const
{
  SDValue RHS = Op.getOperand(0);
  EVT RHSVT = RHS.getValueType();
  MVT RST = RHSVT.getScalarType().getSimpleVT();
  EVT INTVT;
  EVT LONGVT;
  SDValue DST;
  bool isVec = RHSVT.isVector();
  DebugLoc DL = Op.getDebugLoc();
  EVT LHSVT = Op.getValueType();
  MVT LST = LHSVT.getScalarType().getSimpleVT();

  if (LST == MVT::f64 && LHSVT.isVector()) {
    // We dont support vector 64bit floating point convertions.
    for (unsigned x = 0, y = LHSVT.getVectorNumElements(); x < y; ++x) {
      SDValue op = DAG.getNode(ISD::EXTRACT_VECTOR_ELT,
          DL, RST, RHS, DAG.getTargetConstant(x, MVT::i32));
      op = DAG.getNode(ISD::SINT_TO_FP, DL, LST, op);
      if (!x) {
        DST = DAG.getNode(AMDILISD::VBUILD, DL, LHSVT, op);
      } else {
        DST = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL, LHSVT, DST,
            op, DAG.getTargetConstant(x, MVT::i32));
      }
    }
  } else if (RST == MVT::i64
      && LST == MVT::f32) {
    DST = geni64tof32(RHS, DAG, true);
  } else {

    if (isVec) {
      LONGVT = EVT(MVT::getVectorVT(MVT::i64,
            RHSVT.getVectorNumElements()));
      INTVT = EVT(MVT::getVectorVT(MVT::i32,
            RHSVT.getVectorNumElements()));
    } else {
      LONGVT = EVT(MVT::i64);
      INTVT = EVT(MVT::i32);
    }
    MVT RST = RHSVT.getScalarType().getSimpleVT();
    if ((RST == MVT::i32 || RST == MVT::i64)
        && LST == MVT::f64) {
      if (RST == MVT::i32) {
        return SDValue(Op.getNode(), 0);
        }

      SDValue c31 = DAG.getConstant( 31, INTVT );
      SDValue cSbit = DAG.getConstant( 0x80000000, INTVT );

      SDValue S;      // Sign, as 0 or -1
      SDValue Sbit;   // Sign bit, as one bit, MSB only.
      if (RST == MVT::i32) {
        Sbit = DAG.getNode( ISD::AND, DL, INTVT, RHS, cSbit );
        S = DAG.getNode(ISD::SRA, DL, RHSVT, RHS, c31 );
      } else { // 64-bit case... SRA of 64-bit values is slow
        SDValue hi = DAG.getNode( (isVec) ? AMDILISD::LCOMPHI2 : AMDILISD::LCOMPHI, DL, INTVT, RHS );
        Sbit = DAG.getNode( ISD::AND, DL, INTVT, hi, cSbit );
        SDValue temp = DAG.getNode( ISD::SRA, DL, INTVT, hi, c31 );
        S = DAG.getNode( (isVec) ? AMDILISD::LCREATE2 : AMDILISD::LCREATE, DL, RHSVT, temp, temp );
      }

      // get abs() of input value, given sign as S (0 or -1)
      // SpI = RHS + S
      SDValue SpI = DAG.getNode(ISD::ADD, DL, RHSVT, RHS, S);
      // SpIxS = SpI ^ S
      SDValue SpIxS = DAG.getNode(ISD::XOR, DL, RHSVT, SpI, S);

      // Convert unsigned value to double precision
      SDValue R;
      if (RST == MVT::i32) {
        // r = cast_u32_to_f64(SpIxS)
        R = genu32tof64(SpIxS, LHSVT, DAG);
      } else {
        // r = cast_u64_to_f64(SpIxS)
        R = genu64tof64(SpIxS, LHSVT, DAG);
      }

      // drop in the sign bit
      SDValue t = DAG.getNode(ISD::BITCAST, DL, LONGVT, R );
      SDValue thi = DAG.getNode( (isVec) ? AMDILISD::LCOMPHI2 : AMDILISD::LCOMPHI, DL, INTVT, t );
      SDValue tlo = DAG.getNode( (isVec) ? AMDILISD::LCOMPLO2 : AMDILISD::LCOMPLO, DL, INTVT, t );
      thi = DAG.getNode( ISD::OR, DL, INTVT, thi, Sbit );
      t = DAG.getNode( (isVec) ? AMDILISD::LCREATE2 : AMDILISD::LCREATE, DL, LONGVT, tlo, thi );
      DST = DAG.getNode(ISD::BITCAST, DL, LHSVT, t );
    } else {
      DST = SDValue(Op.getNode(), 0);
    }
  }
  return DST;
}
SDValue
AMDILTargetLowering::LowerSUB(SDValue Op, SelectionDAG &DAG) const
{
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  DebugLoc DL = Op.getDebugLoc();
  EVT OVT = Op.getValueType();
  SDValue DST;
  bool isVec = RHS.getValueType().isVector();
  if (OVT.getScalarType() == MVT::i64) {
    /*const AMDILTargetMachine*
      amdtm = reinterpret_cast<const AMDILTargetMachine*>
      (&this->getTargetMachine());
      const AMDILSubtarget*
      stm = dynamic_cast<const AMDILSubtarget*>(
      amdtm->getSubtargetImpl());*/
    MVT INTTY = MVT::i32;
    if (OVT == MVT::v2i64) {
      INTTY = MVT::v2i32;
    }
    SDValue LHSLO, LHSHI, RHSLO, RHSHI, INTLO, INTHI;
    // TODO: need to turn this into a bitcast of i64/v2i64 to v2i32/v4i32
    LHSLO = DAG.getNode((isVec) ? AMDILISD::LCOMPLO2 : AMDILISD::LCOMPLO, DL, INTTY, LHS);
    RHSLO = DAG.getNode((isVec) ? AMDILISD::LCOMPLO2 : AMDILISD::LCOMPLO, DL, INTTY, RHS);
    LHSHI = DAG.getNode((isVec) ? AMDILISD::LCOMPHI2 : AMDILISD::LCOMPHI, DL, INTTY, LHS);
    RHSHI = DAG.getNode((isVec) ? AMDILISD::LCOMPHI2 : AMDILISD::LCOMPHI, DL, INTTY, RHS);
    INTLO = DAG.getNode(ISD::SUB, DL, INTTY, LHSLO, RHSLO);
    INTHI = DAG.getNode(ISD::SUB, DL, INTTY, LHSHI, RHSHI);
    //TODO: need to use IBORROW on Evergreen and later hardware
    SDValue cmp;
    if (OVT == MVT::i64) {
      cmp = DAG.getSetCC(DL, INTTY, LHSLO, RHSLO, ISD::SETULT);
    } else {
      SDValue cmplo;
      SDValue cmphi;
      SDValue LHSRLO = DAG.getNode(ISD::EXTRACT_VECTOR_ELT,
          DL, MVT::i32, LHSLO, DAG.getTargetConstant(0, MVT::i32));
      SDValue LHSRHI = DAG.getNode(ISD::EXTRACT_VECTOR_ELT,
          DL, MVT::i32, LHSLO, DAG.getTargetConstant(1, MVT::i32));
      SDValue RHSRLO = DAG.getNode(ISD::EXTRACT_VECTOR_ELT,
          DL, MVT::i32, RHSLO, DAG.getTargetConstant(0, MVT::i32));
      SDValue RHSRHI = DAG.getNode(ISD::EXTRACT_VECTOR_ELT,
          DL, MVT::i32, RHSLO, DAG.getTargetConstant(1, MVT::i32));
      cmplo = DAG.getSetCC(DL, MVT::i32, LHSRLO, RHSRLO, ISD::SETULT);
      cmphi = DAG.getSetCC(DL, MVT::i32, LHSRHI, RHSRHI, ISD::SETULT);
      cmp = DAG.getNode(AMDILISD::VBUILD, DL, MVT::v2i32, cmplo);
      cmp = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL, MVT::v2i32,
          cmp, cmphi, DAG.getTargetConstant(1, MVT::i32));
    }
    INTHI = DAG.getNode(ISD::ADD, DL, INTTY, INTHI, cmp);
    DST = DAG.getNode((isVec) ? AMDILISD::LCREATE2 : AMDILISD::LCREATE, DL, OVT,
        INTLO, INTHI);
  } else {
    DST = SDValue(Op.getNode(), 0);
  }
  return DST;
}
SDValue
AMDILTargetLowering::LowerFDIV(SDValue Op, SelectionDAG &DAG) const
{
  EVT OVT = Op.getValueType();
  SDValue DST;
  if (OVT.getScalarType() == MVT::f64) {
    DST = LowerFDIV64(Op, DAG);
  } else if (OVT.getScalarType() == MVT::f32) {
    DST = LowerFDIV32(Op, DAG);
  } else {
    DST = SDValue(Op.getNode(), 0);
  }
  return DST;
}

SDValue AMDILTargetLowering::LowerSDIV(SDValue Op, SelectionDAG &DAG) const {
  EVT VT = Op.getValueType().getScalarType();

  if (VT == MVT::i32) {
    if (DAG.ComputeNumSignBits(Op.getOperand(0)) > 8 &&
        DAG.ComputeNumSignBits(Op.getOperand(1)) > 8) {
      return LowerSDIV24(Op, DAG);
    }

    return LowerSDIV32(Op, DAG);
  }

  assert(VT == MVT::i64 && "Unexpected custom lowered type for SDIV");
  return LowerSDIV64(Op, DAG);
}

SDValue
AMDILTargetLowering::LowerUDIV(SDValue Op, SelectionDAG &DAG) const
{
  EVT OVT = Op.getValueType();
  SDValue DST;
  if (OVT.getScalarType() == MVT::i64) {
    DST = LowerUDIV64(Op, DAG);
  } else if (OVT.getScalarType() == MVT::i32) {
    DST = LowerUDIV32(Op, DAG);
  } else if (OVT.getScalarType() == MVT::i16
      || OVT.getScalarType() == MVT::i8) {
    DST = LowerUDIV24(Op, DAG);
  } else {
    DST = SDValue(Op.getNode(), 0);
  }
  return DST;
}

SDValue
AMDILTargetLowering::LowerSREM(SDValue Op, SelectionDAG &DAG) const
{
  EVT OVT = Op.getValueType();
  SDValue DST;
  if (OVT.getScalarType() == MVT::i64) {
    DST = LowerSREM64(Op, DAG);
  } else if (OVT.getScalarType() == MVT::i32) {
    DST = LowerSREM32(Op, DAG);
  } else if (OVT.getScalarType() == MVT::i16) {
    DST = LowerSREM16(Op, DAG);
  } else if (OVT.getScalarType() == MVT::i8) {
    DST = LowerSREM8(Op, DAG);
  } else {
    DST = SDValue(Op.getNode(), 0);
  }
  return DST;
}

SDValue
AMDILTargetLowering::LowerUREM(SDValue Op, SelectionDAG &DAG) const
{
  EVT OVT = Op.getValueType();
  SDValue DST;
  if (OVT.getScalarType() == MVT::i64) {
    DST = LowerUREM64(Op, DAG);
  } else if (OVT.getScalarType() == MVT::i32) {
    DST = LowerUREM32(Op, DAG);
  } else if (OVT.getScalarType() == MVT::i16) {
    DST = LowerUREM16(Op, DAG);
  } else if (OVT.getScalarType() == MVT::i8) {
    DST = LowerUREM8(Op, DAG);
  } else {
    DST = SDValue(Op.getNode(), 0);
  }
  return DST;
}
SDValue
AMDILTargetLowering::LowerMUL(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT OVT = Op.getValueType();
  SDValue DST;
  bool isVec = OVT.isVector();
  if (OVT.getScalarType() != MVT::i64)
  {
    DST = SDValue(Op.getNode(), 0);
  } else {
    assert(OVT.getScalarType() == MVT::i64 && "Only 64 bit mul should be lowered!");
    // TODO: This needs to be turned into a tablegen pattern
    SDValue LHS = Op.getOperand(0);
    SDValue RHS = Op.getOperand(1);

    MVT INTTY = MVT::i32;
    if (OVT == MVT::v2i64) {
      INTTY = MVT::v2i32;
    }
    // mul64(h1, l1, h0, l0)
    SDValue LHSLO = DAG.getNode((isVec) ? AMDILISD::LCOMPLO2 : AMDILISD::LCOMPLO,
        DL,
        INTTY, LHS);
    SDValue LHSHI = DAG.getNode((isVec) ? AMDILISD::LCOMPHI2 : AMDILISD::LCOMPHI,
        DL,
        INTTY, LHS);
    SDValue RHSLO = DAG.getNode((isVec) ? AMDILISD::LCOMPLO2 : AMDILISD::LCOMPLO,
        DL,
        INTTY, RHS);
    SDValue RHSHI = DAG.getNode((isVec) ? AMDILISD::LCOMPHI2 : AMDILISD::LCOMPHI,
        DL,
        INTTY, RHS);
    // MULLO_UINT_1 r1, h0, l1
    SDValue RHILLO = DAG.getNode(AMDILISD::UMUL,
        DL,
        INTTY, RHSHI, LHSLO);
    // MULLO_UINT_1 r2, h1, l0
    SDValue RLOHHI = DAG.getNode(AMDILISD::UMUL,
        DL,
        INTTY, RHSLO, LHSHI);
    // ADD_INT hr, r1, r2
    SDValue ADDHI = DAG.getNode(ISD::ADD,
        DL,
        INTTY, RHILLO, RLOHHI);
    // MULHI_UINT_1 r3, l1, l0
    SDValue RLOLLO = DAG.getNode(ISD::MULHU,
        DL,
        INTTY, RHSLO, LHSLO);
    // ADD_INT hr, hr, r3
    SDValue HIGH = DAG.getNode(ISD::ADD,
        DL,
        INTTY, ADDHI, RLOLLO);
    // MULLO_UINT_1 l3, l1, l0
    SDValue LOW = DAG.getNode(AMDILISD::UMUL,
        DL,
        INTTY, LHSLO, RHSLO);
    DST = DAG.getNode((isVec) ? AMDILISD::LCREATE2 : AMDILISD::LCREATE,
        DL,
        OVT, LOW, HIGH);
  }
  return DST;
}

// TODO: Move to ReplaceNodeResults when i8 / i16 vectors are illegal types like
// they should be.
SDValue AMDILTargetLowering::LowerSRL_SRA(SDValue Op, SelectionDAG &DAG,
                                          bool Signed) const {
  EVT VT = Op.getValueType();
  if (!VT.isSimple())
    return SDValue();

  MVT SimpleVT = VT.getSimpleVT();
  MVT ScalarVT = SimpleVT.getScalarType();
  if (ScalarVT != MVT::i8 && ScalarVT != MVT::i16)
    return SDValue();

  SDValue Op0 = Op.getOperand(0);
  SDValue Op1 = Op.getOperand(1);
  DebugLoc DL = Op.getDebugLoc();

  unsigned Size = ScalarVT.getSizeInBits();
  MVT MidVT = getInt32VT(SimpleVT);

  SDValue ExtOp0 = DAG.getNode(Signed ? ISD::SIGN_EXTEND : ISD::ZERO_EXTEND,
                               DL, MidVT, Op0);
  SDValue ShiftAmt = DAG.getNode(ISD::ZERO_EXTEND, DL, MidVT, Op1);
  SDValue Srl = DAG.getNode(Signed ? ISD::SRA : ISD::SRL, DL,
                            MidVT, ExtOp0, ShiftAmt);

  return DAG.getNode(ISD::TRUNCATE, DL, VT, Srl);
}

SDValue AMDILTargetLowering::LowerSRL(SDValue Op, SelectionDAG &DAG) const {
  return LowerSRL_SRA(Op, DAG, false);
}

SDValue AMDILTargetLowering::LowerSRA(SDValue Op, SelectionDAG &DAG) const {
  return LowerSRL_SRA(Op, DAG, true);
}

SDValue
AMDILTargetLowering::LowerBUILD_VECTOR( SDValue Op, SelectionDAG &DAG ) const
{
  EVT VT = Op.getValueType();
  //printSDValue(Op, 1);
  BuildVectorSDNode *BV = cast<BuildVectorSDNode>(Op.getNode());
  SDValue Nodes1;
  SDValue second;
  SDValue third;
  SDValue fourth;
  DebugLoc DL = Op.getDebugLoc();
  Nodes1 = DAG.getNode(AMDILISD::VBUILD,
      DL,
      VT, Op.getOperand(0));
  bool allEqual = true;

  for (unsigned x = 1, y = Op.getNumOperands(); x < y; ++x) {
    if (Op.getOperand(0) != Op.getOperand(x)) {
      allEqual = false;
      break;
    }
  }
  if (allEqual) {
    return Nodes1;
  }
  switch(Op.getNumOperands()) {
    default:
    case 1:
      break;
    case 4:
      fourth = Op.getOperand(3);
      if (fourth.getOpcode() != ISD::UNDEF) {
        Nodes1 = DAG.getNode(
            ISD::INSERT_VECTOR_ELT,
            DL,
            Op.getValueType(),
            Nodes1,
            fourth,
            DAG.getConstant(7, MVT::i32));
      }
    case 3:
      third = Op.getOperand(2);
      if (third.getOpcode() != ISD::UNDEF) {
        Nodes1 = DAG.getNode(
            ISD::INSERT_VECTOR_ELT,
            DL,
            Op.getValueType(),
            Nodes1,
            third,
            DAG.getConstant(6, MVT::i32));
      }
    case 2:
      second = Op.getOperand(1);
      if (second.getOpcode() != ISD::UNDEF) {
        Nodes1 = DAG.getNode(
            ISD::INSERT_VECTOR_ELT,
            DL,
            Op.getValueType(),
            Nodes1,
            second,
            DAG.getConstant(5, MVT::i32));
      }
      break;
  };
  return Nodes1;
}

SDValue
AMDILTargetLowering::LowerINSERT_VECTOR_ELT(SDValue Op,
    SelectionDAG &DAG) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT VT = Op.getValueType();
  const SDValue *ptr = NULL;
  const ConstantSDNode *CSDN = dyn_cast<ConstantSDNode>(Op.getOperand(2));
  uint32_t swizzleNum = 0;
  SDValue DST;
  if (!VT.isVector()) {
    SDValue Res = Op.getOperand(0);
    return Res;
  }

  if (Op.getOperand(1).getOpcode() != ISD::UNDEF) {
    ptr = &Op.getOperand(1);
  } else {
    ptr = &Op.getOperand(0);
  }
  if (CSDN) {
    swizzleNum = (uint32_t)CSDN->getZExtValue();
    uint32_t mask2 = 0x04030201 & ~(0xFF << (swizzleNum * 8));
    uint32_t mask3 = 0x01010101 & (0xFF << (swizzleNum * 8));
    DST = DAG.getNode(AMDILISD::VINSERT,
        DL,
        VT,
        Op.getOperand(0),
        *ptr,
        DAG.getTargetConstant(mask2, MVT::i32),
        DAG.getTargetConstant(mask3, MVT::i32));
  } else {
    uint32_t mask2 = 0x04030201 & ~(0xFF << (swizzleNum * 8));
    uint32_t mask3 = 0x01010101 & (0xFF << (swizzleNum * 8));
    SDValue res = DAG.getNode(AMDILISD::VINSERT,
        DL, VT, Op.getOperand(0), *ptr,
        DAG.getTargetConstant(mask2, MVT::i32),
        DAG.getTargetConstant(mask3, MVT::i32));

    EVT SetCCType = getSetCCResultType(*DAG.getContext(),
                                       ptr->getValueType());
    EVT CondType = EVT::getVectorVT(*DAG.getContext(),
                                    SetCCType,
                                    VT.getVectorNumElements());
    for (uint32_t x = 1; x < VT.getVectorNumElements(); ++x) {
      mask2 = 0x04030201 & ~(0xFF << (x * 8));
      mask3 = 0x01010101 & (0xFF << (x * 8));
      SDValue t = DAG.getNode(AMDILISD::VINSERT,
          DL, VT, Op.getOperand(0), *ptr,
          DAG.getTargetConstant(mask2, MVT::i32),
          DAG.getTargetConstant(mask3, MVT::i32));
      SDValue c = DAG.getSetCC(DL, SetCCType, Op.getOperand(2),
                               DAG.getConstant(x, MVT::i32), ISD::SETEQ);
      c = DAG.getNode(AMDILISD::VBUILD, DL, CondType, c);
      res = DAG.getSelect(DL, VT, c, t, res);
    }
    DST = res;
  }
  return DST;
}

SDValue
AMDILTargetLowering::LowerEXTRACT_VECTOR_ELT(SDValue Op,
    SelectionDAG &DAG) const
{
  EVT VT = Op.getValueType();
  //printSDValue(Op, 1);
  const ConstantSDNode *CSDN = dyn_cast<ConstantSDNode>(Op.getOperand(1));
  uint64_t swizzleNum = 0;
  DebugLoc DL = Op.getDebugLoc();
  SDValue Res;
  if (!Op.getOperand(0).getValueType().isVector()) {
    Res = Op.getOperand(0);
    return Res;
  }
  if (CSDN) {
    // Static vector extraction
    swizzleNum = CSDN->getZExtValue() + 1;
    Res = DAG.getNode(AMDILISD::VEXTRACT,
        DL, VT,
        Op.getOperand(0),
        DAG.getTargetConstant(swizzleNum, MVT::i32));
  } else {
    SDValue Op1 = Op.getOperand(1);
    uint32_t vecSize = 4;
    SDValue Op0 = Op.getOperand(0);
    SDValue res = DAG.getNode(AMDILISD::VEXTRACT,
        DL, VT, Op0,
        DAG.getTargetConstant(1, MVT::i32));
    if (Op0.getValueType().isVector()) {
      vecSize = Op0.getValueType().getVectorNumElements();
    }
    for (uint32_t x = 2; x <= vecSize; ++x) {
      SDValue t = DAG.getNode(AMDILISD::VEXTRACT,
          DL, VT, Op0,
          DAG.getTargetConstant(x, MVT::i32));
      SDValue c = DAG.getSetCC(DL, getSetCCResultType(*DAG.getContext(),
                                                      Op1.getValueType()),
          Op1, DAG.getConstant(x - 1, MVT::i32), ISD::SETEQ);
      res = DAG.getSelect(DL, VT, c, t, res);

    }
    Res = res;
  }
  return Res;
}

SDValue
AMDILTargetLowering::LowerEXTRACT_SUBVECTOR(SDValue Op,
    SelectionDAG &DAG) const
{
  uint32_t vecSize = Op.getValueType().getVectorNumElements();
  SDValue src = Op.getOperand(0);
  const ConstantSDNode *CSDN = dyn_cast<ConstantSDNode>(Op.getOperand(1));
  uint64_t offset = 0;
  EVT vecType = Op.getValueType().getVectorElementType();
  DebugLoc DL = Op.getDebugLoc();
  SDValue Result;
  if (CSDN) {
    offset = CSDN->getZExtValue();
    Result = DAG.getNode(ISD::EXTRACT_VECTOR_ELT,
        DL,vecType, src, DAG.getConstant(offset, MVT::i32));
    Result = DAG.getNode(AMDILISD::VBUILD, DL,
        Op.getValueType(), Result);
    for (uint32_t x = 1; x < vecSize; ++x) {
      SDValue elt = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, vecType,
          src, DAG.getConstant(offset + x, MVT::i32));
      if (elt.getOpcode() != ISD::UNDEF) {
        Result = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL,
            Op.getValueType(), Result, elt,
            DAG.getConstant(x, MVT::i32));
      }
    }
  } else {
    SDValue idx = Op.getOperand(1);
    Result = DAG.getNode(ISD::EXTRACT_VECTOR_ELT,
        DL, vecType, src, idx);
    Result = DAG.getNode(AMDILISD::VBUILD, DL,
        Op.getValueType(), Result);
    for (uint32_t x = 1; x < vecSize; ++x) {
      idx = DAG.getNode(ISD::ADD, DL, vecType,
          idx, DAG.getConstant(1, MVT::i32));
      SDValue elt = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, vecType,
          src, idx);
      if (elt.getOpcode() != ISD::UNDEF) {
        Result = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL,
            Op.getValueType(), Result, elt, idx);
      }
    }
  }
  return Result;
}
SDValue
AMDILTargetLowering::LowerSCALAR_TO_VECTOR(SDValue Op,
    SelectionDAG &DAG) const
{
  SDValue Res = DAG.getNode(AMDILISD::VBUILD,
      Op.getDebugLoc(),
      Op.getValueType(),
      Op.getOperand(0));
  return Res;
}

SDValue AMDILTargetLowering::LowerSETCC(SDValue Op, SelectionDAG &DAG) const {
  ISD::CondCode CC = dyn_cast<CondCodeSDNode>(Op.getOperand(2))->get();
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  DebugLoc DL = Op.getDebugLoc();
  EVT OpVT = Op.getValueType();
  EVT OpSVT = OpVT.getScalarType();
  EVT LHSVT = LHS.getValueType();
  EVT SVT = LHSVT.getScalarType();

  // Promote i8 and i16 compares to use i32.
  if (SVT == MVT::i8 || SVT == MVT::i16) {
    EVT ExtVT = MVT::i32;
    if (OpVT.isVector()) {
      ExtVT = EVT::getVectorVT(*DAG.getContext(),
                               MVT::i32,
                               OpVT.getVectorNumElements());
    }

    EVT SetCCVT = getSetCCResultType(*DAG.getContext(), LHSVT);

    unsigned ExtOpc
      = ISD::isSignedIntSetCC(CC) ? ISD::SIGN_EXTEND : ISD::ZERO_EXTEND;

    SDValue ExtLHS = DAG.getNode(ExtOpc, DL, ExtVT, LHS);
    SDValue ExtRHS = DAG.getNode(ExtOpc, DL, ExtVT, RHS);

    SDValue SetCC = DAG.getSetCC(DL, SetCCVT, ExtLHS, ExtRHS, CC);
    if (SetCCVT.getScalarType() == OpSVT)
      return SetCC;

    return DAG.getSExtOrTrunc(SetCC, DL, LHSVT);
  }

  if (!OpVT.isVector())
    return SDValue(Op.getNode(), 0);

  EVT ccSVT = getSetCCResultType(*DAG.getContext(), SVT);
  assert((SVT == MVT::f64 || SVT == MVT::i64) &&
      "we don't support expansion of SetCC on non-64bit types!");
  SDValue ccOp;
  for (unsigned x = 0, y = OpVT.getVectorNumElements(); x < y; ++x) {
    SDValue lhsComp = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, SVT,
        LHS, DAG.getTargetConstant(x, MVT::i32));
    SDValue rhsComp = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, SVT,
        RHS, DAG.getTargetConstant(x, MVT::i32));
    SDValue opComp = DAG.getSetCC(DL, ccSVT, lhsComp, rhsComp, CC);
    // Need to handle the case where we are splitting up a
    // setCC where the result is less than 32bits.
    if (ccSVT != OpSVT && SVT.isInteger()) {
        opComp = DAG.getSExtOrTrunc(opComp, DL, OpSVT);
    }
    if (!x) {
      ccOp = DAG.getNode(AMDILISD::VBUILD, DL, OpVT, opComp);
    } else {
      ccOp = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL, OpVT,
          ccOp, opComp, DAG.getTargetConstant(x, MVT::i32));
    }
  }
  if (OpSVT != SVT) {
      ccOp = DAG.getSExtOrTrunc(ccOp, DL, OpVT);
  }
  return ccOp;
}

SDValue AMDILTargetLowering::LowerSELECT_CC(SDValue Op,
                                            SelectionDAG &DAG) const {
  SDValue Cond;
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TRUE = Op.getOperand(2);
  SDValue FALSE = Op.getOperand(3);
  SDValue CC = Op.getOperand(4);
  DebugLoc DL = Op.getDebugLoc();
  bool skipCMov = false;
  bool genINot = false;
  EVT OVT = Op.getValueType();

  // Check for possible elimination of cmov
  if (TRUE.getValueType().getSimpleVT().SimpleTy == MVT::i32) {
    const ConstantSDNode *trueConst
      = dyn_cast<ConstantSDNode>( TRUE.getNode() );
    const ConstantSDNode *falseConst
      = dyn_cast<ConstantSDNode>( FALSE.getNode() );
    if (trueConst && falseConst) {
      // both possible result values are constants
      if (trueConst->isAllOnesValue()
          && falseConst->isNullValue()) { // and convenient constants
        skipCMov = true;
      }
      else if (trueConst->isNullValue()
          && falseConst->isAllOnesValue()) { // less convenient
        skipCMov = true;
        genINot = true;
      }
    }
  }
  ISD::CondCode SetCCOpcode = cast<CondCodeSDNode>(CC)->get();
  Cond = DAG.getSetCC(DL, getSetCCResultType(*DAG.getContext(),
                                             LHS.getValueType()),
      LHS, RHS, SetCCOpcode);
  if (genINot) {
    Cond = DAG.getNode(ISD::XOR, DL, OVT, Cond, DAG.getConstant(-1, OVT));
  }
  if (!skipCMov) {
    Cond = DAG.getSelect(DL, OVT, Cond, TRUE, FALSE);
}
  return Cond;
}

SDValue AMDILTargetLowering::LowerSIGN_EXTEND_INREG(SDValue Op,
                                                    SelectionDAG &DAG) const {
  EVT VT = Op.getValueType();
  if (!VT.isSimple())
    return SDValue();

  DebugLoc DL = Op.getDebugLoc();
  SDValue Src = Op.getOperand(0);

  EVT ExtraVT = cast<VTSDNode>(Op.getOperand(1))->getVT();
  unsigned SrcBits = ExtraVT.getScalarType().getSizeInBits();
  unsigned DestBits = VT.getScalarType().getSizeInBits();
  unsigned BitsDiff = DestBits - SrcBits;

  // Don't handle weird integers.
  if (DestBits > 64)
    return SDValue();

  if (SrcBits == 32) {
    // If the source is 32-bits, this is really half of a 2-register pair, and
    // we need to discard the unused half of the pair.
    SDValue TruncSrc = DAG.getNode(ISD::TRUNCATE, DL, MVT::i32, Src);
    return DAG.getNode(ISD::SIGN_EXTEND, DL, VT, TruncSrc);
  }

  unsigned NElts = VT.isVector() ? VT.getVectorNumElements() : 1;
  if (NElts > 4)
    return SDValue();

  if (SrcBits < 32 && DestBits <= 32) {
    // ubit_extract / ibit_extract extend to i32 vector.
    MVT ExtVT = (NElts == 1) ? MVT::i32 : MVT::getVectorVT(MVT::i32, NElts);
    MVT SimpleVT = VT.getSimpleVT();

    if (DestBits != 32)
      Src = DAG.getNode(ISD::ZERO_EXTEND, DL, ExtVT, Src);

    SDValue Extract = DAG.getNode(AMDILISD::IBIT_EXTRACT,
                                  DL,
                                  ExtVT,
                                  DAG.getTargetConstant(SrcBits, ExtVT),
                                  DAG.getTargetConstant(0, ExtVT),
                                  Src);
    if (ExtVT == SimpleVT)
      return Extract;

    // Truncate to the original size.
    return DAG.getNode(ISD::TRUNCATE, DL, VT, Extract);
  }

  // For small types, extend to 32-bits first.
  if (SrcBits < 32) {
    MVT ExtVT = VT.isVector()
              ? MVT::getVectorVT(MVT::i32, VT.getVectorNumElements())
              : MVT::i32;

    SDValue TruncSrc = DAG.getNode(ISD::TRUNCATE, DL, ExtVT, Src);
    SDValue Ext32 = DAG.getNode(AMDILISD::IBIT_EXTRACT,
                                DL,
                                ExtVT,
                                DAG.getTargetConstant(SrcBits, ExtVT),
                                DAG.getTargetConstant(0, ExtVT),
                                TruncSrc);

    return DAG.getNode(ISD::SIGN_EXTEND, DL, VT, Ext32);
  }

  SDValue Shift = DAG.getTargetConstant(BitsDiff, VT);
  // Shift left by 'Shift' bits.
  SDValue Data = DAG.getNode(ISD::SHL, DL, VT, Src, Shift);

  // Signed shift Right by 'Shift' bits.
  return DAG.getNode(ISD::SRA, DL, VT, Src, Shift);
}

EVT
AMDILTargetLowering::genIntType(uint32_t size, uint32_t numEle) const
{
  int iSize = (size * numEle);
  int vEle = (iSize >> ((size == 64) ? 6 : 5));
  if (!vEle) {
    vEle = 1;
  }
  if (size == 64) {
    if (vEle == 1) {
      return EVT(MVT::i64);
    } else {
      return EVT(MVT::getVectorVT(MVT::i64, vEle));
    }
  } else {
    if (vEle == 1) {
      return EVT(MVT::i32);
    } else {
      return EVT(MVT::getVectorVT(MVT::i32, vEle));
    }
  }
}

SDValue AMDILTargetLowering::LowerBITCAST(SDValue Op, SelectionDAG &DAG) const {
  SDValue Src = Op.getOperand(0);
  SDValue Dst = Op;
  SDValue Res;
  DebugLoc DL = Op.getDebugLoc();
  EVT SrcVT = Src.getValueType();
  EVT DstVT = Dst.getValueType();
  // Let's bitcast the floating point types to an equivalent integer type before
  // converting to vectors.
  // FIXME: This is done, and then isFloatingPoint() is still checked later.
  if (SrcVT.getScalarType().isFloatingPoint()) {
    SrcVT = getIntegerType(*DAG.getContext(), SrcVT);
    Src = DAG.getNode(ISD::BITCAST, DL, SrcVT, Src);
  }
  uint32_t ScalarSrcSize = SrcVT.getScalarType()
    .getSimpleVT().getSizeInBits();
  uint32_t ScalarDstSize = DstVT.getScalarType()
    .getSimpleVT().getSizeInBits();
  uint32_t SrcNumEle = SrcVT.isVector() ? SrcVT.getVectorNumElements() : 1;
  uint32_t DstNumEle = DstVT.isVector() ? DstVT.getVectorNumElements() : 1;
  bool isVec = SrcVT.isVector();
  if (DstVT.getScalarType().isInteger() &&
      (SrcVT.getScalarType().isInteger()
       || SrcVT.getScalarType().isFloatingPoint())) {
    if ((ScalarDstSize == 64 && SrcNumEle == 4 && ScalarSrcSize == 16)
        || (ScalarSrcSize == 64
          && DstNumEle == 4
          && ScalarDstSize == 16)) {
      // This is the problematic case when bitcasting i64 <-> <4 x i16>
      // This approach is a little different as we cannot generate a
      // <4 x i64> vector
      // as that is illegal in our backend and we are already past
      // the DAG legalizer.
      // So, in this case, we will do the following conversion.
      // Case 1:
      // %dst = <4 x i16> %src bitconvert i64 ==>
      // %tmp = <4 x i16> %src convert <4 x i32>
      // %tmp = <4 x i32> %tmp and 0xFFFF
      // %tmp = <4 x i32> %tmp shift_left <0, 16, 0, 16>
      // %tmp = <4 x i32> %tmp or %tmp.xz %tmp.yw
      // %dst = <2 x i32> %tmp bitcast i64
      // case 2:
      // %dst = i64 %src bitconvert <4 x i16> ==>
      // %tmp = i64 %src bitcast <2 x i32>
      // %tmp = <4 x i32> %tmp vinsert %tmp.xxyy
      // %tmp = <4 x i32> %tmp shift_right <0, 16, 0, 16>
      // %tmp = <4 x i32> %tmp and 0xFFFF
      // %dst = <4 x i16> %tmp bitcast <4 x i32>
      SDValue mask = DAG.getNode(AMDILISD::VBUILD, DL, MVT::v4i32,
          DAG.getConstant(0xFFFF, MVT::i32));
      SDValue const16 = DAG.getConstant(16, MVT::i32);
      if (ScalarDstSize == 64) {
        // case 1
        Op = DAG.getSExtOrTrunc(Src, DL, MVT::v4i32);
        Op = DAG.getNode(ISD::AND, DL, Op.getValueType(), Op, mask);
        SDValue x = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, MVT::i32,
            Op, DAG.getConstant(0, MVT::i32));
        SDValue y = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, MVT::i32,
            Op, DAG.getConstant(1, MVT::i32));
        y = DAG.getNode(ISD::SHL, DL, MVT::i32, y, const16);
        SDValue z = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, MVT::i32,
            Op, DAG.getConstant(2, MVT::i32));
        SDValue w = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, MVT::i32,
            Op, DAG.getConstant(3, MVT::i32));
        w = DAG.getNode(ISD::SHL, DL, MVT::i32, w, const16);
        x = DAG.getNode(ISD::OR, DL, MVT::i32, x, y);
        y = DAG.getNode(ISD::OR, DL, MVT::i32, z, w);
        Res = DAG.getNode((isVec) ? AMDILISD::LCREATE2 : AMDILISD::LCREATE, DL, MVT::i64, x, y);
        return Res;
      } else {
        // case 2
        SDValue lo = DAG.getNode((isVec) ? AMDILISD::LCOMPLO2 : AMDILISD::LCOMPLO, DL, MVT::i32, Src);
        SDValue lor16
          = DAG.getNode(ISD::SRL, DL, MVT::i32, lo, const16);
        SDValue hi = DAG.getNode((isVec) ? AMDILISD::LCOMPHI2 : AMDILISD::LCOMPHI, DL, MVT::i32, Src);
        SDValue hir16
          = DAG.getNode(ISD::SRL, DL, MVT::i32, hi, const16);
        SDValue resVec = DAG.getNode(AMDILISD::VBUILD, DL,
            MVT::v4i32, lo);
        SDValue idxVal = DAG.getNode(ISD::ZERO_EXTEND, DL,
            MVT::i32, DAG.getConstant(1, MVT::i32));
        resVec = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL, MVT::v4i32,
            resVec, lor16, idxVal);
        idxVal = DAG.getNode(ISD::ZERO_EXTEND, DL,
            MVT::i32, DAG.getConstant(2, MVT::i32));
        resVec = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL, MVT::v4i32,
            resVec, hi, idxVal);
        idxVal = DAG.getNode(ISD::ZERO_EXTEND, DL,
            MVT::i32, DAG.getConstant(3, MVT::i32));
        resVec = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL, MVT::v4i32,
            resVec, hir16, idxVal);
        resVec = DAG.getNode(ISD::AND, DL, MVT::v4i32, resVec, mask);
        Res = DAG.getSExtOrTrunc(resVec, DL, MVT::v4i16);
        return Res;
      }
    } else {
      // There are four cases we need to worry about for bitcasts
      // where the size of all
      // source, intermediates and result is <= 128 bits, unlike
      // the above case
      // 1) Sub32bit bitcast 32bitAlign
      // %dst = <4 x i8> bitcast i32
      // (also <[2|4] x i16> to <[2|4] x i32>)
      // 2) 32bitAlign bitcast Sub32bit
      // %dst = i32 bitcast <4 x i8>
      // 3) Sub32bit bitcast LargerSub32bit
      // %dst = <2 x i8> bitcast i16
      // (also <4 x i8> to <2 x i16>)
      // 4) Sub32bit bitcast SmallerSub32bit
      // %dst = i16 bitcast <2 x i8>
      // (also <2 x i16> to <4 x i8>)
      // This also only handles types that are powers of two
      if ((ScalarDstSize & (ScalarDstSize - 1))
          || (ScalarSrcSize & (ScalarSrcSize - 1))) {
      } else if (ScalarDstSize >= 32 && ScalarSrcSize < 32) {
        // case 1:
        EVT IntTy = genIntType(ScalarDstSize, SrcNumEle);
        SDValue res = DAG.getSExtOrTrunc(Src, DL, IntTy);
        SDValue mask = DAG.getNode(AMDILISD::VBUILD, DL, IntTy,
            DAG.getConstant((1 << ScalarSrcSize) - 1, MVT::i32));
        SDValue *newEle = new SDValue[SrcNumEle];
        res = DAG.getNode(ISD::AND, DL, IntTy, res, mask);
        for (uint32_t x = 0; x < SrcNumEle; ++x) {
          newEle[x] = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL,
              IntTy.getScalarType(), res,
              DAG.getConstant(x, MVT::i32));
        }
        uint32_t Ratio = SrcNumEle / DstNumEle;
        for (uint32_t x = 0; x < SrcNumEle; ++x) {
          if (x % Ratio) {
            newEle[x] = DAG.getNode(ISD::SHL, DL,
                IntTy.getScalarType(), newEle[x],
                DAG.getConstant(ScalarSrcSize * (x % Ratio),
                  MVT::i32));
          }
        }
        for (uint32_t x = 0; x < SrcNumEle; x += 2) {
          newEle[x] = DAG.getNode(ISD::OR, DL,
              IntTy.getScalarType(), newEle[x], newEle[x + 1]);
        }
        if (ScalarSrcSize == 8) {
          for (uint32_t x = 0; x < SrcNumEle; x += 4) {
            newEle[x] = DAG.getNode(ISD::OR, DL,
                IntTy.getScalarType(), newEle[x], newEle[x + 2]);
          }
          if (DstNumEle == 1) {
            Dst = newEle[0];
          } else {
            Dst = DAG.getNode(AMDILISD::VBUILD, DL, DstVT,
                newEle[0]);
            for (uint32_t x = 1; x < DstNumEle; ++x) {
              SDValue idx = DAG.getNode(ISD::ZERO_EXTEND, DL,
                  MVT::i32, DAG.getConstant(x, MVT::i32));
              Dst = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL,
                  DstVT, Dst, newEle[x * 4], idx);
            }
          }
        } else {
          if (DstNumEle == 1) {
            Dst = newEle[0];
          } else {
            Dst = DAG.getNode(AMDILISD::VBUILD, DL, DstVT,
                newEle[0]);
            for (uint32_t x = 1; x < DstNumEle; ++x) {
              SDValue idx = DAG.getNode(ISD::ZERO_EXTEND, DL,
                  MVT::i32, DAG.getConstant(x, MVT::i32));
              Dst = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL,
                  DstVT, Dst, newEle[x * 2], idx);
            }
          }
        }
        delete [] newEle;
        return Dst;
      } else if (ScalarDstSize < 32 && ScalarSrcSize >= 32) {
        // case 2:
        EVT IntTy = genIntType(ScalarSrcSize, DstNumEle);
        SDValue vec = DAG.getNode(AMDILISD::VBUILD, DL, IntTy,
            DAG.getTargetConstant(0, IntTy.getScalarType()));
        uint32_t mult = (ScalarDstSize == 8) ? 4 : 2;
        for (uint32_t x = 0; x < SrcNumEle; ++x) {
          for (uint32_t y = 0; y < mult; ++y) {
            SDValue idx = DAG.getNode(ISD::ZERO_EXTEND, DL,
                MVT::i32,
                DAG.getTargetConstant(x * mult + y, MVT::i32));
            SDValue t;
            if (SrcNumEle > 1) {
              t = DAG.getNode(ISD::EXTRACT_VECTOR_ELT,
                  DL, SrcVT.getScalarType(), Src,
                  DAG.getTargetConstant(x, MVT::i32));
            } else {
              t = Src;
            }
            if (y != 0) {
              t = DAG.getNode(ISD::SRL, DL, t.getValueType(),
                  t, DAG.getTargetConstant(y * ScalarDstSize,
                    MVT::i32));
            }
            vec = DAG.getNode(ISD::INSERT_VECTOR_ELT,
                DL, IntTy, vec, t, idx);
          }
        }
        Dst = DAG.getSExtOrTrunc(vec, DL, DstVT);
        return Dst;
      } else if (ScalarDstSize == 16 && ScalarSrcSize == 8) {
        // case 3:
        SDValue *numEle = new SDValue[SrcNumEle];
        for (uint32_t x = 0; x < SrcNumEle; ++x) {
          numEle[x] = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL,
              MVT::i8, Src, DAG.getConstant(x, MVT::i32));
          numEle[x] = DAG.getSExtOrTrunc(numEle[x], DL, MVT::i16);
          numEle[x] = DAG.getNode(ISD::AND, DL, MVT::i16, numEle[x],
              DAG.getConstant(0xFF, MVT::i16));
        }
        for (uint32_t x = 1; x < SrcNumEle; x += 2) {
          numEle[x] = DAG.getNode(ISD::SHL, DL, MVT::i16, numEle[x],
              DAG.getConstant(8, MVT::i16));
          numEle[x - 1] = DAG.getNode(ISD::OR, DL, MVT::i16,
              numEle[x-1], numEle[x]);
        }
        if (DstNumEle > 1) {
          // If we are not a scalar i16, the only other case is a
          // v2i16 since we can't have v8i8 at this point, v4i16
          // cannot be generated
          Dst = DAG.getNode(AMDILISD::VBUILD, DL, MVT::v2i16,
              numEle[0]);
          SDValue idx = DAG.getNode(ISD::ZERO_EXTEND, DL,
              MVT::i32, DAG.getConstant(1, MVT::i32));
          Dst = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL, MVT::v2i16,
              Dst, numEle[2], idx);
        } else {
          Dst = numEle[0];
        }
        delete [] numEle;
        return Dst;
      } else if (ScalarDstSize == 8 && ScalarSrcSize == 16) {
        // case 4:
        SDValue *numEle = new SDValue[DstNumEle];
        for (uint32_t x = 0; x < SrcNumEle; ++x) {
          numEle[x * 2] = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL,
              MVT::i16, Src, DAG.getConstant(x, MVT::i32));
          numEle[x * 2 + 1] = DAG.getNode(ISD::SRL, DL, MVT::i16,
              numEle[x * 2], DAG.getConstant(8, MVT::i16));
        }
        MVT ty = (SrcNumEle == 1) ? MVT::v2i16 : MVT::v4i16;
        Dst = DAG.getNode(AMDILISD::VBUILD, DL, ty, numEle[0]);
        for (uint32_t x = 1; x < DstNumEle; ++x) {
          SDValue idx = DAG.getNode(ISD::ZERO_EXTEND, DL,
              MVT::i32, DAG.getConstant(x, MVT::i32));
          Dst = DAG.getNode(ISD::INSERT_VECTOR_ELT, DL, ty,
              Dst, numEle[x], idx);
        }
        delete [] numEle;
        ty = (SrcNumEle == 1) ? MVT::v2i8 : MVT::v4i8;
        Res = DAG.getSExtOrTrunc(Dst, DL, ty);
        return Res;
      }
    }
  }
  Res = DAG.getNode(ISD::BITCAST,
      Dst.getDebugLoc(),
      Dst.getValueType(), Src);
  return Res;
}

SDValue
AMDILTargetLowering::LowerDYNAMIC_STACKALLOC(SDValue Op,
    SelectionDAG &DAG) const
{
  SDValue Chain = Op.getOperand(0);
  SDValue Size = Op.getOperand(1);
  unsigned int SPReg = AMDIL::SP;
  DebugLoc DL = Op.getDebugLoc();
  SDValue SP = DAG.getCopyFromReg(Chain,
      DL,
      SPReg, MVT::i32);
  SDValue NewSP = DAG.getNode(ISD::ADD,
      DL,
      MVT::i32, SP, Size);
  Chain = DAG.getCopyToReg(SP.getValue(1),
      DL,
      SPReg, NewSP);
  SDValue Ops[2] = {NewSP, Chain};
  Chain = DAG.getMergeValues(Ops, 2 ,DL);
  return Chain;
}

SDValue
AMDILTargetLowering::LowerBRCOND(SDValue Op, SelectionDAG &DAG) const
{
  SDValue Chain = Op.getOperand(0);
  SDValue Cond  = Op.getOperand(1);
  SDValue Jump  = Op.getOperand(2);

  return DAG.getNode(AMDILISD::BRANCH_COND,
                     Op.getDebugLoc(),
                     Op.getValueType(),
                     Chain, Jump, Cond);
}

SDValue
AMDILTargetLowering::LowerBR_CC(SDValue Op, SelectionDAG &DAG) const
{
  SDValue Chain = Op.getOperand(0);
  CondCodeSDNode *CCNode = cast<CondCodeSDNode>(Op.getOperand(1));
  SDValue LHS   = Op.getOperand(2);
  SDValue RHS   = Op.getOperand(3);
  SDValue JumpT  = Op.getOperand(4);
  SDValue CmpValue;
  ISD::CondCode CC = CCNode->get();
  SDValue Result;
  CmpValue = DAG.getSetCC(Op.getDebugLoc(),
                          getSetCCResultType(*DAG.getContext(),
                                             LHS.getValueType()),
      LHS, RHS, CC);
  Result = DAG.getNode(
      AMDILISD::BRANCH_COND,
      CmpValue.getDebugLoc(),
      MVT::Other, Chain,
      JumpT, CmpValue);
  return Result;
}

SDValue
AMDILTargetLowering::LowerFP_ROUND(SDValue Op, SelectionDAG &DAG) const
{
  assert(Op.getValueType().getScalarType() != MVT::f16);
  SDValue Result = DAG.getNode(
      AMDILISD::DP_TO_FP,
      Op.getDebugLoc(),
      Op.getValueType(),
      Op.getOperand(0),
      Op.getOperand(1));
  return Result;
}

SDValue
AMDILTargetLowering::LowerCONCAT_VECTORS(SDValue Op, SelectionDAG &DAG) const
{
  SDValue Result = DAG.getNode(
      AMDILISD::VCONCAT,
      Op.getDebugLoc(),
      Op.getValueType(),
      Op.getOperand(0),
      Op.getOperand(1));
  return Result;
}
// LowerRET - Lower an ISD::RET node.
SDValue AMDILTargetLowering::LowerReturn(
  SDValue Chain,
  CallingConv::ID CallConv,
  bool isVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs,
    const SmallVectorImpl<SDValue> &OutVals,
  DebugLoc dl,
  SelectionDAG &DAG) const {
  MachineFunction& MF = DAG.getMachineFunction();
  AMDILMachineFunctionInfo *FuncInfo
    = MF.getInfo<AMDILMachineFunctionInfo>();

  // CCValAssign - represent the assignment of the return value
  // to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), RVLocs, *DAG.getContext());

  // Analyze return values of ISD::RET
  CCInfo.AnalyzeReturn(Outs, CCAssignFnForNode(CallConv, true, false));
  // If this is the first return lowered for this function, add
  // the regs to the liveout set for the function
  MachineRegisterInfo &MRI = DAG.getMachineFunction().getRegInfo();
  for (unsigned int i = 0, e = RVLocs.size(); i != e; ++i) {
    if (RVLocs[i].isRegLoc() && !MRI.isLiveOut(RVLocs[i].getLocReg())) {
      MRI.addLiveOut(RVLocs[i].getLocReg());
    }
  }
  // FIXME: implement this when tail call is implemented
  // Chain = GetPossiblePreceedingTailCall(Chain, AMDILISD::TAILCALL);
  // both x86 and ppc implement this in ISelLowering

  const TargetRegisterInfo *TRI = getTargetMachine().getRegisterInfo();
  // first and last vector register used for input/output at a call site
  unsigned FirstReg = AMDIL::OUT0;
  unsigned LastReg = AMDIL::OUT7;
  const AMDILTargetMachine *AMDTM
    = reinterpret_cast<const AMDILTargetMachine*>(&getTargetMachine());
  if (!AMDTM->getSubtargetImpl()->isSupported(AMDIL::Caps::UseMacroForCall)) {
    FirstReg = AMDIL::R1;
    LastReg = AMDIL::R32;
  }
  // keep track of which vector registers are used for return value
  SmallSet<unsigned, 1> VectorRegs;

  // Regular return here
  SDValue Flag;
  for (unsigned int i = 0, e = RVLocs.size(); i != e; ++i) {
    CCValAssign &VA = RVLocs[i];
    SDValue ValToCopy = OutVals[i];
    assert(VA.isRegLoc() && "Can only return in registers!");
    unsigned Reg = VA.getLocReg();
    FuncInfo->addRetReg(Reg);
    unsigned VectorReg = getVectorReg(Reg, TRI);
    assert(VectorReg >= FirstReg && VectorReg <= LastReg &&
           "unexpected register assigned for call argument");
    VectorRegs.insert(VectorReg);

    // ISD::Ret => ret chain, (regnum1, val1), ...
    // So i * 2 + 1 index only the regnums
    Chain = DAG.getCopyToReg(Chain, dl, Reg, ValToCopy, Flag);
    // guarantee that all emitted copies are stuck together
    // avoiding something bad
    Flag = Chain.getValue(1);
  }
  FuncInfo->setRetNumVecRegs(VectorRegs.size());
  /*if (MF.getFunction()->hasStructRetAttr()) {
    llvm_unreachable("Struct returns are not yet implemented!");
  // Both MIPS and X86 have this
  }*/
  SmallVector<SDValue, 6> RetOps;
  RetOps.push_back(Chain);
  RetOps.push_back(DAG.getConstant(0/*getBytesToPopOnReturn()*/, MVT::i32));
  if (Flag.getNode())
    RetOps.push_back(Flag);

  Flag = DAG.getNode(AMDILISD::RET_FLAG,
      dl,
                     MVT::Other,
                     &RetOps[0],
                     RetOps.size());
  return Flag;
}

unsigned int
AMDILTargetLowering::getFunctionAlignment(const Function *) const
{
  return 0;
}

bool
AMDILTargetLowering::isLoadBitCastBeneficial(EVT lVT, EVT bVT) const
{
  return !(lVT.getSizeInBits() == bVT.getSizeInBits()
    && lVT.getScalarType().getSizeInBits() > bVT.getScalarType().getSizeInBits()
    && bVT.getScalarType().getSizeInBits() < 32
    && lVT.getScalarType().getSizeInBits() >= 32);
}

SDValue
AMDILTargetLowering::LowerSDIV24(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT OVT = Op.getValueType();
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  MVT INTTY;
  MVT FLTTY;
  if (!OVT.isVector()) {
    INTTY = MVT::i32;
    FLTTY = MVT::f32;
  } else if (OVT.getVectorNumElements() == 2) {
    INTTY = MVT::v2i32;
    FLTTY = MVT::v2f32;
  } else if (OVT.getVectorNumElements() == 4) {
    INTTY = MVT::v4i32;
    FLTTY = MVT::v4f32;
  }
  unsigned bitsize = OVT.getScalarType().getSizeInBits();
  // char|short jq = ia ^ ib;
  SDValue jq = DAG.getNode(ISD::XOR, DL, OVT, LHS, RHS);

  // jq = jq >> (bitsize - 2)
  jq = DAG.getNode(ISD::SRA, DL, OVT, jq, DAG.getConstant(bitsize - 2, OVT));

  // jq = jq | 0x1
  jq = DAG.getNode(ISD::OR, DL, OVT, jq, DAG.getConstant(1, OVT));

  jq = DAG.getSExtOrTrunc(jq, DL, OVT);

  // int ia = (int)LHS;
  SDValue ia = DAG.getSExtOrTrunc(LHS, DL, INTTY);

  // int ib, (int)RHS;
  SDValue ib = DAG.getSExtOrTrunc(RHS, DL, INTTY);

  // float fa = (float)ia;
  SDValue fa = DAG.getNode(ISD::SINT_TO_FP, DL, FLTTY, ia);

  // float fb = (float)ib;
  SDValue fb = DAG.getNode(ISD::SINT_TO_FP, DL, FLTTY, ib);

  // float fq = native_divide(fa, fb);
  SDValue fq = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, FLTTY,
      DAG.getConstant(AMDILIntrinsic::AMDIL_div, MVT::i32),
      fa, fb);

  // fq = trunc(fq);
  fq = DAG.getNode(ISD::FTRUNC, DL, FLTTY, fq);

  // float fqneg = -fq;
  SDValue fqneg = DAG.getNode(ISD::FNEG, DL, FLTTY, fq);

  // float fr = mad(fqneg, fb, fa);
  SDValue fr = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, FLTTY,
      DAG.getConstant(AMDILIntrinsic::AMDIL_mad, MVT::i32),
      fqneg, fb, fa);

  // int iq = (int)fq;
  SDValue iq = DAG.getNode(ISD::FP_TO_SINT, DL, INTTY, fq);

  // fr = fabs(fr);
  fr = DAG.getNode(ISD::FABS, DL, FLTTY, fr);

  // fb = fabs(fb);
  fb = DAG.getNode(ISD::FABS, DL, FLTTY, fb);

  // int cv = fr >= fb;
  SDValue cv = DAG.getSetCC(DL, INTTY, fr, fb, ISD::SETGE);
  // jq = (cv ? jq : 0);
  jq = DAG.getSelect(DL, OVT, cv, jq, DAG.getConstant(0, OVT));
  // dst = iq + jq;
  iq = DAG.getSExtOrTrunc(iq, DL, OVT);
  iq = DAG.getNode(ISD::ADD, DL, OVT, iq, jq);
  return iq;
}

SDValue
AMDILTargetLowering::LowerSDIV32(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT OVT = Op.getValueType();
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  // The LowerSDIV32 function generates equivalent to the following IL.
  // mov r0, LHS
  // mov r1, RHS
  // ilt r10, r0, 0
  // ilt r11, r1, 0
  // iadd r0, r0, r10
  // iadd r1, r1, r11
  // ixor r0, r0, r10
  // ixor r1, r1, r11
  // udiv r0, r0, r1
  // ixor r10, r10, r11
  // iadd r0, r0, r10
  // ixor DST, r0, r10

  // mov r0, LHS
  SDValue r0 = LHS;

  // mov r1, RHS
  SDValue r1 = RHS;

  // ilt r10, r0, 0
  SDValue r10 = DAG.getSetCC(DL, OVT, r0, DAG.getConstant(0, OVT), ISD::SETLT);
  // ilt r11, r1, 0
  SDValue r11 = DAG.getSetCC(DL, OVT, r1, DAG.getConstant(0, OVT), ISD::SETLT);
  // iadd r0, r0, r10
  r0 = DAG.getNode(ISD::ADD, DL, OVT, r0, r10);

  // iadd r1, r1, r11
  r1 = DAG.getNode(ISD::ADD, DL, OVT, r1, r11);

  // ixor r0, r0, r10
  r0 = DAG.getNode(ISD::XOR, DL, OVT, r0, r10);

  // ixor r1, r1, r11
  r1 = DAG.getNode(ISD::XOR, DL, OVT, r1, r11);

  // udiv r0, r0, r1
  r0 = DAG.getNode(ISD::UDIV, DL, OVT, r0, r1);

  // ixor r10, r10, r11
  r10 = DAG.getNode(ISD::XOR, DL, OVT, r10, r11);

  // iadd r0, r0, r10
  r0 = DAG.getNode(ISD::ADD, DL, OVT, r0, r10);

  // ixor DST, r0, r10
  SDValue DST = DAG.getNode(ISD::XOR, DL, OVT, r0, r10);
  return DST;
}

SDValue
AMDILTargetLowering::LowerSDIV64(SDValue Op, SelectionDAG &DAG) const
{
  return SDValue(Op.getNode(), 0);
}

SDValue
AMDILTargetLowering::LowerUDIV24(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT OVT = Op.getValueType();
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  MVT INTTY;
  MVT FLTTY;
  if (!OVT.isVector()) {
    INTTY = MVT::i32;
    FLTTY = MVT::f32;
  } else if (OVT.getVectorNumElements() == 2) {
    INTTY = MVT::v2i32;
    FLTTY = MVT::v2f32;
  } else if (OVT.getVectorNumElements() == 4) {
    INTTY = MVT::v4i32;
    FLTTY = MVT::v4f32;
  }

  // The LowerUDIV24 function implements the following CL.
  // int ia = (int)LHS
  // float fa = (float)ia
  // int ib = (int)RHS
  // float fb = (float)ib
  // float fq = native_divide(fa, fb)
  // fq = trunc(fq)
  // float t = mad(fq, fb, fb)
  // int iq = (int)fq - (t <= fa)
  // return (type)iq

  // int ia = (int)LHS
  SDValue ia = DAG.getZExtOrTrunc(LHS, DL, INTTY);

  // float fa = (float)ia
  SDValue fa = DAG.getNode(ISD::SINT_TO_FP, DL, FLTTY, ia);

  // int ib = (int)RHS
  SDValue ib = DAG.getZExtOrTrunc(RHS, DL, INTTY);

  // float fb = (float)ib
  SDValue fb = DAG.getNode(ISD::SINT_TO_FP, DL, FLTTY, ib);

  // float fq = native_divide(fa, fb)
  SDValue fq = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, FLTTY,
      DAG.getConstant(AMDILIntrinsic::AMDIL_div, MVT::i32),
      fa, fb);

  // fq = trunc(fq)
  fq = DAG.getNode(ISD::FTRUNC, DL, FLTTY, fq);

  // float t = mad(fq, fb, fb)
  SDValue t = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, FLTTY,
      DAG.getConstant(AMDILIntrinsic::AMDIL_mad, MVT::i32),
      fq, fb, fb);

  // int iq = (int)fq - (t <= fa) // This is sub and not add because GPU returns 0, -1
  SDValue iq;
  fq = DAG.getNode(ISD::FP_TO_SINT, DL, INTTY, fq);
  iq = DAG.getSetCC(DL, INTTY, t, fa, ISD::SETLE);
  iq = DAG.getNode(ISD::SUB, DL, INTTY, fq, iq);


  // return (type)iq
  iq = DAG.getZExtOrTrunc(iq, DL, OVT);
  return iq;

}

SDValue
AMDILTargetLowering::LowerUDIV32(SDValue Op, SelectionDAG &DAG) const
{
  return SDValue(Op.getNode(), 0);
}

SDValue
AMDILTargetLowering::LowerUDIV64(SDValue Op, SelectionDAG &DAG) const
{
  return SDValue(Op.getNode(), 0);
}
SDValue
AMDILTargetLowering::LowerSREM8(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT OVT = Op.getValueType();
  MVT INTTY = MVT::i32;
  if (OVT == MVT::v2i8) {
    INTTY = MVT::v2i32;
  } else if (OVT == MVT::v4i8) {
    INTTY = MVT::v4i32;
  }
  SDValue LHS = DAG.getSExtOrTrunc(Op.getOperand(0), DL, INTTY);
  SDValue RHS = DAG.getSExtOrTrunc(Op.getOperand(1), DL, INTTY);
  LHS = DAG.getNode(ISD::SREM, DL, INTTY, LHS, RHS);
  LHS = DAG.getSExtOrTrunc(LHS, DL, OVT);
  return LHS;
}

SDValue
AMDILTargetLowering::LowerSREM16(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT OVT = Op.getValueType();
  MVT INTTY = MVT::i32;
  if (OVT == MVT::v2i16) {
    INTTY = MVT::v2i32;
  } else if (OVT == MVT::v4i16) {
    INTTY = MVT::v4i32;
  }
  SDValue LHS = DAG.getSExtOrTrunc(Op.getOperand(0), DL, INTTY);
  SDValue RHS = DAG.getSExtOrTrunc(Op.getOperand(1), DL, INTTY);
  LHS = DAG.getNode(ISD::SREM, DL, INTTY, LHS, RHS);
  LHS = DAG.getSExtOrTrunc(LHS, DL, OVT);
  return LHS;
}

SDValue
AMDILTargetLowering::LowerSREM32(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT OVT = Op.getValueType();
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  // The LowerSREM32 function generates equivalent to the following IL.
  // mov r0, LHS
  // mov r1, RHS
  // ilt r10, r0, 0
  // ilt r11, r1, 0
  // iadd r0, r0, r10
  // iadd r1, r1, r11
  // ixor r0, r0, r10
  // ixor r1, r1, r11
  // udiv r20, r0, r1
  // umul r20, r20, r1
  // sub r0, r0, r20
  // iadd r0, r0, r10
  // ixor DST, r0, r10

  // mov r0, LHS
  SDValue r0 = LHS;

  // mov r1, RHS
  SDValue r1 = RHS;

  // ilt r10, r0, 0
  SDValue r10 = DAG.getSetCC(DL, OVT, r0, DAG.getConstant(0, OVT), ISD::SETLT);
  // ilt r11, r1, 0
  SDValue r11 = DAG.getSetCC(DL, OVT, r1, DAG.getConstant(0, OVT), ISD::SETLT);

  // iadd r0, r0, r10
  r0 = DAG.getNode(ISD::ADD, DL, OVT, r0, r10);

  // iadd r1, r1, r11
  r1 = DAG.getNode(ISD::ADD, DL, OVT, r1, r11);

  // ixor r0, r0, r10
  r0 = DAG.getNode(ISD::XOR, DL, OVT, r0, r10);

  // ixor r1, r1, r11
  r1 = DAG.getNode(ISD::XOR, DL, OVT, r1, r11);

  // udiv r20, r0, r1
  SDValue r20 = DAG.getNode(ISD::UREM, DL, OVT, r0, r1);

  // umul r20, r20, r1
  r20 = DAG.getNode(AMDILISD::UMUL, DL, OVT, r20, r1);

  // sub r0, r0, r20
  r0 = DAG.getNode(ISD::SUB, DL, OVT, r0, r20);

  // iadd r0, r0, r10
  r0 = DAG.getNode(ISD::ADD, DL, OVT, r0, r10);

  // ixor DST, r0, r10
  SDValue DST = DAG.getNode(ISD::XOR, DL, OVT, r0, r10);
  return DST;
}

SDValue
AMDILTargetLowering::LowerSREM64(SDValue Op, SelectionDAG &DAG) const
{
  return SDValue(Op.getNode(), 0);
}

SDValue
AMDILTargetLowering::LowerUREM8(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT OVT = Op.getValueType();
  MVT INTTY = MVT::i32;
  if (OVT == MVT::v2i8) {
    INTTY = MVT::v2i32;
  } else if (OVT == MVT::v4i8) {
    INTTY = MVT::v4i32;
  }
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  // The LowerUREM8 function generates equivalent to the following IL.
  // mov r0, as_u32(LHS)
  // mov r1, as_u32(RHS)
  // and r10, r0, 0xFF
  // and r11, r1, 0xFF
  // cmov_logical r3, r11, r11, 0x1
  // udiv r3, r10, r3
  // cmov_logical r3, r11, r3, 0
  // umul r3, r3, r11
  // sub r3, r10, r3
  // and as_u8(DST), r3, 0xFF

  // mov r0, as_u32(LHS)
  SDValue r0 = DAG.getSExtOrTrunc(LHS, DL, INTTY);

  // mov r1, as_u32(RHS)
  SDValue r1 = DAG.getSExtOrTrunc(RHS, DL, INTTY);

  // and r10, r0, 0xFF
  SDValue r10 = DAG.getNode(ISD::AND, DL, INTTY, r0,
      DAG.getConstant(0xFF, INTTY));

  // and r11, r1, 0xFF
  SDValue r11 = DAG.getNode(ISD::AND, DL, INTTY, r1,
      DAG.getConstant(0xFF, INTTY));

  // cmov_logical r3, r11, r11, 0x1
  SDValue r3 = DAG.getSelect(DL, INTTY, r11, r11,
      DAG.getConstant(0x01, INTTY));

  // udiv r3, r10, r3
  r3 = DAG.getNode(ISD::UREM, DL, INTTY, r10, r3);

  // cmov_logical r3, r11, r3, 0
  r3 = DAG.getSelect(DL, INTTY, r11, r3,
      DAG.getConstant(0, INTTY));

  // umul r3, r3, r11
  r3 = DAG.getNode(AMDILISD::UMUL, DL, INTTY, r3, r11);

  // sub r3, r10, r3
  r3 = DAG.getNode(ISD::SUB, DL, INTTY, r10, r3);

  // and as_u8(DST), r3, 0xFF
  SDValue DST = DAG.getNode(ISD::AND, DL, INTTY, r3,
      DAG.getConstant(0xFF, INTTY));
  DST = DAG.getZExtOrTrunc(DST, DL, OVT);
  return DST;
}

SDValue
AMDILTargetLowering::LowerUREM16(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT OVT = Op.getValueType();
  MVT INTTY = MVT::i32;
  if (OVT == MVT::v2i16) {
    INTTY = MVT::v2i32;
  } else if (OVT == MVT::v4i16) {
    INTTY = MVT::v4i32;
  }
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  // The LowerUREM16 function generatest equivalent to the following IL.
  // mov r0, LHS
  // mov r1, RHS
  // DIV = LowerUDIV16(LHS, RHS)
  // and r10, r0, 0xFFFF
  // and r11, r1, 0xFFFF
  // cmov_logical r3, r11, r11, 0x1
  // udiv as_u16(r3), as_u32(r10), as_u32(r3)
  // and r3, r3, 0xFFFF
  // cmov_logical r3, r11, r3, 0
  // umul r3, r3, r11
  // sub r3, r10, r3
  // and DST, r3, 0xFFFF

  // mov r0, LHS
  SDValue r0 = LHS;

  // mov r1, RHS
  SDValue r1 = RHS;

  // and r10, r0, 0xFFFF
  SDValue r10 = DAG.getNode(ISD::AND, DL, OVT, r0,
      DAG.getConstant(0xFFFF, OVT));

  // and r11, r1, 0xFFFF
  SDValue r11 = DAG.getNode(ISD::AND, DL, OVT, r1,
      DAG.getConstant(0xFFFF, OVT));

  // cmov_logical r3, r11, r11, 0x1
  SDValue r3 = DAG.getSelect(DL, OVT, r11, r11,
      DAG.getConstant(0x01, OVT));

  // udiv as_u16(r3), as_u32(r10), as_u32(r3)
  r10 = DAG.getZExtOrTrunc(r10, DL, INTTY);
  r3 = DAG.getZExtOrTrunc(r3, DL, INTTY);
  r3 = DAG.getNode(ISD::UREM, DL, INTTY, r10, r3);
  r3 = DAG.getZExtOrTrunc(r3, DL, OVT);
  r10 = DAG.getZExtOrTrunc(r10, DL, OVT);

  // and r3, r3, 0xFFFF
  r3 = DAG.getNode(ISD::AND, DL, OVT, r3,
      DAG.getConstant(0xFFFF, OVT));

  // cmov_logical r3, r11, r3, 0
  r3 = DAG.getSelect(DL, OVT, r11, r3,
      DAG.getConstant(0, OVT));
  // umul r3, r3, r11
  r3 = DAG.getNode(AMDILISD::UMUL, DL, OVT, r3, r11);

  // sub r3, r10, r3
  r3 = DAG.getNode(ISD::SUB, DL, OVT, r10, r3);

  // and DST, r3, 0xFFFF
  SDValue DST = DAG.getNode(ISD::AND, DL, OVT, r3,
      DAG.getConstant(0xFFFF, OVT));
  return DST;
}

SDValue
AMDILTargetLowering::LowerUREM32(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT OVT = Op.getValueType();
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  // The LowerUREM32 function generates equivalent to the following IL.
  // udiv r20, LHS, RHS
  // umul r20, r20, RHS
  // sub DST, LHS, r20

  // udiv r20, LHS, RHS
  SDValue r20 = DAG.getNode(ISD::UDIV, DL, OVT, LHS, RHS);

  // umul r20, r20, RHS
  r20 = DAG.getNode(AMDILISD::UMUL, DL, OVT, r20, RHS);

  // sub DST, LHS, r20
  SDValue DST = DAG.getNode(ISD::SUB, DL, OVT, LHS, r20);
  return DST;
}

SDValue
AMDILTargetLowering::LowerUREM64(SDValue Op, SelectionDAG &DAG) const
{
  return SDValue(Op.getNode(), 0);
}

SDValue
AMDILTargetLowering::LowerFDIV32(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc DL = Op.getDebugLoc();
  EVT OVT = Op.getValueType();
  MVT INTTY = MVT::i32;
  if (OVT == MVT::v2f32) {
    INTTY = MVT::v2i32;
  } else if (OVT == MVT::v4f32) {
    INTTY = MVT::v4i32;
  }
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue DST;

    // The following sequence of DAG nodes produce the following IL:
    // fabs r1, RHS
    // lt r2, 0x1.0p+96f, r1
    // cmov_logical r3, r2, 0x1.0p-23f, 1.0f
    // mul_ieee r1, RHS, r3
    // div_zeroop(infinity) r0, LHS, r1
    // mul_ieee DST, r0, r3

    // fabs r1, RHS
    SDValue r1 = DAG.getNode(ISD::FABS, DL, OVT, RHS);
    // lt r2, 0x1.0p+96f, r1
    SDValue cst1 = DAG.getConstant(0x6f800000, INTTY);
  cst1 = DAG.getNode(ISD::BITCAST, DL, OVT, cst1);
    SDValue r2 = DAG.getSetCC(DL, INTTY, cst1, r1, ISD::SETLT);
    // cmov_logical r3, r2, 0x1.0p-23f, 1.0f
    cst1 = DAG.getConstant(0x2f800000, INTTY);
  cst1 = DAG.getNode(ISD::BITCAST, DL, OVT, cst1);
    SDValue cst2 = DAG.getConstant(0x3f800000, INTTY);
  cst2 = DAG.getNode(ISD::BITCAST, DL, OVT, cst2);
    SDValue r3 = DAG.getSelect(DL, OVT, r2,
        cst1, cst2);
    // mul_ieee r1, RHS, r3
    r1 = DAG.getNode(ISD::FMUL, DL, OVT, RHS, r3);
    // div_zeroop(infinity) r0, LHS, r1
    SDValue r0 = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, OVT,
        DAG.getConstant(AMDILIntrinsic::AMDIL_div, MVT::i32),
        LHS, r1);
    // mul_ieee DST, r0, r3
  return DAG.getNode(ISD::FMUL, DL, OVT, r0, r3);
}

SDValue
AMDILTargetLowering::LowerFDIV64(SDValue Op, SelectionDAG &DAG) const
{
  return SDValue(Op.getNode(), 0);
}

static SDValue PerformMULADDCombine(SDNode *N, SelectionDAG &DAG,
                                    TargetLowering::DAGCombinerInfo& DCI)
{
  assert((N->getOpcode() == ISD::FADD || N->getOpcode() == ISD::FSUB) &&
         "Invoking MULADDCombine with wrong Node");

  DebugLoc dl = N->getDebugLoc();
  EVT VT = N->getValueType(0);

  // Let legalize expand this if it isn't a legal type yet.
  if (!DAG.getTargetLoweringInfo().isTypeLegal(VT))
    return SDValue();

  // No support to v2f64. Let legalizer expand it.
  if (VT.getSimpleVT().SimpleTy == MVT::v2f64)
    return SDValue();

  const TargetMachine& TM = DAG.getTargetLoweringInfo().getTargetMachine();
  const AMDILSubtarget *stm = reinterpret_cast<const AMDILTargetMachine*>
      (&TM)->getSubtargetImpl();
  bool hasDouble = stm->isSupported(AMDIL::Caps::DoubleOps);
  if ((TM.getOptLevel() == CodeGenOpt::None) ||
      !(TM.Options.LessPreciseFPMAD() ||
        (hasDouble && (TM.Options.AllowFPOpFusion == FPOpFusion::Fast))))
    return SDValue();

  SDValue Opr0, Opr1, Opr2;
  SDValue A0 = N->getOperand(0);
  SDValue A1 = N->getOperand(1);
  bool isAdd = (N->getOpcode() == ISD::FADD);
  if (A0.getOpcode() == ISD::FMUL) {
    Opr0 = A0.getOperand(0);
    Opr1 = A0.getOperand(1);
    Opr2 = isAdd ? A1
                 : DAG.getNode(ISD::FNEG, dl, VT, A1);
  } else if (A1.getOpcode() == ISD::FMUL) {
    Opr0 = isAdd ? A1.getOperand(0)
                 : DAG.getNode(ISD::FNEG, dl, VT, A1.getOperand(0));
    Opr1 = A1.getOperand(1);
    Opr2 = A0;
  } else {
    return SDValue();
  }

  return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, VT,
                     DAG.getConstant(
                       TM.Options.LessPreciseFPMAD()
                         ? AMDILIntrinsic::AMDIL_mad
                         : AMDILIntrinsic::AMDIL_fma,
                       MVT::i32),
                     Opr0, Opr1, Opr2);
}

static bool usesAllNormalStores(SDNode *LoadVal) {
  for (SDNode::use_iterator I = LoadVal->use_begin(); !I.atEnd(); ++I) {
    if (!ISD::isNormalStore(*I))
      return false;
  }

  return true;
}

SDValue AMDILTargetLowering::PerformDAGCombine(SDNode *N,
                                               DAGCombinerInfo &DCI) const {
  SelectionDAG &DAG = DCI.DAG;
  switch (N->getOpcode()) {
  default: break;
  case ISD::FADD:
  case ISD::FSUB:
    return PerformMULADDCombine(N, DAG, DCI);
    break;

  case ISD::STORE: {
    if (DCI.isBeforeLegalize()) {
      StoreSDNode *SN = cast<StoreSDNode>(N);
      SDValue Value = SN->getValue();
      EVT VT = Value.getValueType();

      // If we have a copy of an illegal type, replace it with a load / store of
      // an equivalently sized legal type. This avoids intermediate bit pack /
      // unpack instructions emitted when handling extloads and
      // truncstores. Ideally we could recognize the pack / unpack pattern to
      // eliminate it.
      if (!isTypeLegal(VT) &&
          ISD::isNormalLoad(Value.getNode()) &&
          usesAllNormalStores(Value.getNode())) {
        LoadSDNode *LoadVal = cast<LoadSDNode>(Value);
        EVT MemVT = LoadVal->getMemoryVT();
        EVT LoadVT = getEquivalentMemType(*DAG.getContext(), MemVT);

        SDValue NewLoad = DAG.getLoad(ISD::UNINDEXED, ISD::NON_EXTLOAD,
                                      LoadVT, LoadVal->getDebugLoc(),
                                      LoadVal->getChain(),
                                      LoadVal->getBasePtr(),
                                      LoadVal->getOffset(),
                                      LoadVT,
                                      LoadVal->getMemOperand());

        return DAG.getStore(SN->getChain(),
                            SN->getDebugLoc(),
                            NewLoad,
                            SN->getBasePtr(),
                            SN->getMemOperand());
      }
    }

    // Fall through.
  }
  case ISD::LOAD: {
    LSBaseSDNode *LSN = cast<LSBaseSDNode>(N);
    if (LSN->isIndexed())
      break;

    const TargetMachine &TM = getTargetMachine();
    if (TM.getOptLevel() == CodeGenOpt::None)
      break;

    if (!Subtarget.is64bit())
      break;

    unsigned AS = LSN->getAddressSpace();
    if (AS == AMDILAS::PRIVATE_ADDRESS || AS == AMDILAS::LOCAL_ADDRESS) {
      break;
      SDValue Ptr = LSN->getBasePtr();
      EVT PtrVT = Ptr.getValueType();
      assert(PtrVT != MVT::i32 && "Remove this with LLVM 3.4+");

      APInt Demanded = APInt::getLowBitsSet(PtrVT.getSizeInBits(), 32);
      APInt KnownZero, KnownOne;
      TargetLowering::TargetLoweringOpt TLO(DAG, !DCI.isBeforeLegalize(),
                                            !DCI.isBeforeLegalizeOps());
      const TargetLowering &TLI = DAG.getTargetLoweringInfo();

      if (TLO.ShrinkDemandedConstant(Ptr, Demanded) ||
          TLI.SimplifyDemandedBits(Ptr, Demanded, KnownZero, KnownOne, TLO)) {
        DCI.CommitTargetLoweringOpt(TLO);
      }

      break;
    }

    if (AS == AMDILAS::GLOBAL_ADDRESS || AS == AMDILAS::CONSTANT_ADDRESS) {
      if (!Subtarget.smallGlobalObjects())
        break;
      SDValue Ptr = LSN->getBasePtr();
      EVT PtrVT = Ptr.getValueType();
      if (PtrVT == MVT::i32)
        break;

      if (Ptr.getOpcode() != ISD::ADD)
        break;

      // Avoid create the vector ops that are custom lowered after legalization.
      // FIXME: BUILD_VECTOR should really be legal.
      if (!DCI.isBeforeLegalize())
        break;
      DebugLoc DL = N->getDebugLoc();
      SDValue Op0 = Ptr.getOperand(0);
      SDValue Op1 = Ptr.getOperand(1);

      // Try to figure out what the base pointer is based on the sign bits. If
      // we are able to compute this, such as if it's a constant or locally
      // computed value, the other operand is the base pointer.
      unsigned Op0SignBits = DAG.ComputeNumSignBits(Op0);
      unsigned Op1SignBits = DAG.ComputeNumSignBits(Op1);
      if (Op0SignBits == 1 && Op1SignBits == 1)
        break;

      SDValue Base;
      SDValue Offset;
      if (Op0SignBits > 1) {
        Base = Op0;
        Offset = Op1;
      } else if (Op1SignBits > 1) {
        Base = Op1;
        Offset = Op0;
      } else {
        // Can't figure out base pointer operand.
        break;
      }

      // Bitcast to v2i32 and use vector ops. These are currently lowered better
      // than the AMDIL build_pair equivalents, but stil sub-optimally.
      SDValue BC = DAG.getNode(ISD::BITCAST, DL, MVT::v2i32, Base);
      SDValue LoBasePtr = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, MVT::i32,
                                      BC, DAG.getConstant(0, MVT::i32));
      SDValue HiBasePtr = DAG.getNode(ISD::EXTRACT_VECTOR_ELT, DL, MVT::i32,
                                      BC, DAG.getConstant(1, MVT::i32));

      SDValue TruncOffset = DAG.getNode(ISD::TRUNCATE, DL, MVT::i32, Offset);

      // Make sure the truncate is revisited. It might allow shrinking more
      // operations leading up to this.
      DCI.AddToWorklist(TruncOffset.getNode());

      SDValue NewPtrCalc = DAG.getNode(ISD::ADD, DL, MVT::i32,
                                       LoBasePtr, TruncOffset);

      SDValue NewPtrV = DAG.getNode(ISD::BUILD_VECTOR, DL, MVT::v2i32,
                                    NewPtrCalc, HiBasePtr);
      SDValue NewPtr = DAG.getNode(ISD::BITCAST, DL, MVT::i64, NewPtrV);

      SmallVector<SDValue, 8> Ops;
      for (unsigned i = 0, e = N->getNumOperands(); i != e; ++i)
        Ops.push_back(N->getOperand(i));

      // TODO: Handle atomics and other memory instructions.
      if (N->getOpcode() == ISD::LOAD) {
        Ops[1] = NewPtr;
      } else if (N->getOpcode() == ISD::STORE) {
        Ops[2] = NewPtr;
      } else {
        llvm_unreachable("Unimplemented");
      }

      DAG.UpdateNodeOperands(N, Ops.data(), Ops.size());
      break;
    }
    break;
  }
  }

  return SDValue();
}
