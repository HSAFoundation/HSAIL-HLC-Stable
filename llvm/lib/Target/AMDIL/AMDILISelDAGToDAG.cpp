//===-- AMDILISelDAGToDAG.cpp - A dag to dag inst selector for AMDIL ------===//
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
// This file defines an instruction selector for the AMDIL target.
//
//===----------------------------------------------------------------------===//
#include "AMDILMachineFunctionInfo.h"
#include "AMDILTargetMachine.h"
#include "AMDILUtilityFunctions.h"

#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;

//===----------------------------------------------------------------------===//
// Instruction Selector Implementation
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// AMDILDAGToDAGISel - AMDIL specific code to select AMDIL machine instructions
// //for SelectionDAG operations.
//
namespace {
class AMDILDAGToDAGISel : public SelectionDAGISel {
  // Subtarget - Keep a pointer to the AMDIL Subtarget around so that we can
  // make the right decision when generating code for different targets.
  const AMDILSubtarget *Subtarget;
public:
  explicit AMDILDAGToDAGISel(AMDILTargetMachine &TM, CodeGenOpt::Level OptLevel)
    : SelectionDAGISel(TM, OptLevel),
    Subtarget(&TM.getSubtarget<AMDILSubtarget>()) {}
  virtual ~AMDILDAGToDAGISel() {};
  inline SDValue getSmallIPtrImm(unsigned Imm);

  SDNode *Select(SDNode *N);
  // Complex pattern selectors
  bool SelectADDR(SDValue N, SDValue &R1);
  bool SelectADDR64(SDValue N, SDValue &R1);

  bool isGlobalStore(const StoreSDNode *N) const;
  bool isPrivateStore(const StoreSDNode *N) const;
  bool isLocalStore(const StoreSDNode *N) const;
  bool isRegionStore(const StoreSDNode *N) const;
  bool isFlatStore(const StoreSDNode *N) const;

  bool isCPLoad(const LoadSDNode *N) const;
  bool isConstantLoad(const LoadSDNode *N, int cbID) const;
  bool isGlobalLoad(const LoadSDNode *N) const;
  bool isPrivateLoad(const LoadSDNode *N) const;
  bool isLocalLoad(const LoadSDNode *N) const;
  bool isRegionLoad(const LoadSDNode *N) const;
  bool isFlatLoad(const LoadSDNode *N) const;
  bool isFlatASOverrideEnabled() const;

  virtual const char *getPassName() const;
private:
  SDNode *xformAtomicInst(SDNode *N);

  // Include the pieces autogenerated from the target description.
#include "AMDILGenDAGISel.inc"
};
}  // end anonymous namespace

// createAMDILISelDag - This pass converts a legalized DAG into a AMDIL-specific
// DAG, ready for instruction scheduling.
//
FunctionPass *llvm::createAMDILISelDag(AMDILTargetMachine &TM,
                                       llvm::CodeGenOpt::Level OptLevel) {
  return new AMDILDAGToDAGISel(TM, OptLevel);
}

SDValue AMDILDAGToDAGISel::getSmallIPtrImm(unsigned int Imm) {
  return CurDAG->getTargetConstant(Imm, MVT::i32);
}

bool AMDILDAGToDAGISel::SelectADDR(SDValue Addr, SDValue& R1) {
  if (Addr.getOpcode() == ISD::TargetExternalSymbol ||
      Addr.getOpcode() == ISD::TargetGlobalAddress) {
    return false;
  }

  if (Addr.getOpcode() == ISD::FrameIndex) {
    if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
      R1 = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i32);
    } else {
      R1 = Addr;
    }
  } else {
    R1 = Addr;
  }
  return true;
}


bool AMDILDAGToDAGISel::SelectADDR64(SDValue Addr, SDValue& R1) {
  if (Addr.getOpcode() == ISD::TargetExternalSymbol ||
      Addr.getOpcode() == ISD::TargetGlobalAddress) {
    return false;
  }

  if (Addr.getOpcode() == ISD::FrameIndex) {
    if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
      R1 = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i64);
    } else {
      R1 = Addr;
    }
  } else {
    R1 = Addr;
  }
  return true;
}


SDNode *AMDILDAGToDAGISel::Select(SDNode *N) {
  unsigned int Opc = N->getOpcode();
  if (N->isMachineOpcode()) {
    return NULL;   // Already selected.
  }
  if (Opc == ISD::FrameIndex
      && dyn_cast<FrameIndexSDNode>(N)) {
    FrameIndexSDNode *FI = dyn_cast<FrameIndexSDNode>(N);
    SDValue TFI = CurDAG->getTargetFrameIndex(FI->getIndex(),
        FI->getValueType(0));
    return CurDAG->SelectNodeTo(N, AMDIL::LOADFIi32, FI->getValueType(0), TFI);
    /*
     *
    return CurDAG->getNode(ISD::ADD, N->getDebugLoc(), FI->getValueType(0),
        CurDAG->getTargetFrameIndex(FI->getIndex(), FI->getValueType(0)),
        CurDAG->getConstant(0, FI->getValueType(0)));
        */
  }
  // For all atomic instructions, we need to add a constant
  // operand that stores the resource ID in the instruction
  if (Opc > AMDILISD::ADDADDR && Opc < AMDILISD::APPEND_ALLOC) {
    N = xformAtomicInst(N);
  }
  // for all atomic and append intrinsics
  if (Opc > AMDILISD::ADDADDR && Opc < AMDILISD::IMAGE2D_READ) {
    AMDILMachineFunctionInfo *MFI = MF->getInfo<AMDILMachineFunctionInfo>();
    MFI->setOutputInst();
  }
  return SelectCode(N);
}

bool AMDILDAGToDAGISel::isFlatASOverrideEnabled() const
{
  return Subtarget->overridesFlatAS();
}

bool AMDILDAGToDAGISel::isGlobalStore(const StoreSDNode *N) const {
  return check_type(N->getSrcValue(), AMDILAS::GLOBAL_ADDRESS)
    && !isFlatASOverrideEnabled();
}

bool AMDILDAGToDAGISel::isFlatStore(const StoreSDNode *N) const {
  return check_type(N->getSrcValue(), AMDILAS::FLAT_ADDRESS)
    || (isFlatASOverrideEnabled()
        && (check_type(N->getSrcValue(), AMDILAS::LOCAL_ADDRESS)
        || check_type(N->getSrcValue(), AMDILAS::CONSTANT_ADDRESS)
        || check_type(N->getSrcValue(), AMDILAS::PRIVATE_ADDRESS)
        || check_type(N->getSrcValue(), AMDILAS::GLOBAL_ADDRESS))
       );
}

bool AMDILDAGToDAGISel::isPrivateStore(const StoreSDNode *N) const {
  return (!check_type(N->getSrcValue(), AMDILAS::LOCAL_ADDRESS)
          && !check_type(N->getSrcValue(), AMDILAS::GLOBAL_ADDRESS)
          && !check_type(N->getSrcValue(), AMDILAS::REGION_ADDRESS))
    && !isFlatASOverrideEnabled();
}

bool AMDILDAGToDAGISel::isLocalStore(const StoreSDNode *N) const {
  return check_type(N->getSrcValue(), AMDILAS::LOCAL_ADDRESS)
    && !isFlatASOverrideEnabled();
}

bool AMDILDAGToDAGISel::isRegionStore(const StoreSDNode *N) const {
  return check_type(N->getSrcValue(), AMDILAS::REGION_ADDRESS);
}

bool AMDILDAGToDAGISel::isConstantLoad(const LoadSDNode *N, int cbID) const {
  if (isFlatASOverrideEnabled())
    return false;

  if (check_type(N->getSrcValue(), AMDILAS::CONSTANT_ADDRESS))
    return true;

  MachineMemOperand *MMO = N->getMemOperand();
  if (!MMO) // FIXME: Why would this happen?
    return false;

  const Value *V = MMO->getValue(); // FIXME: Or this?
  if (!V)
    return false;

  const Value *RealObject = GetUnderlyingObject(V, TM.getDataLayout(), 0);
  if (!isa<GlobalValue>(RealObject))
    return false;

  return check_type(N->getSrcValue(), AMDILAS::PRIVATE_ADDRESS);
}

bool AMDILDAGToDAGISel::isGlobalLoad(const LoadSDNode *N) const {
  return check_type(N->getSrcValue(), AMDILAS::GLOBAL_ADDRESS)
    && !isFlatASOverrideEnabled();
}

bool AMDILDAGToDAGISel::isFlatLoad(const LoadSDNode *N) const {
  return check_type(N->getSrcValue(), AMDILAS::FLAT_ADDRESS)
    || (isFlatASOverrideEnabled()
        && (check_type(N->getSrcValue(), AMDILAS::LOCAL_ADDRESS)
        || check_type(N->getSrcValue(), AMDILAS::CONSTANT_ADDRESS)
        || check_type(N->getSrcValue(), AMDILAS::PRIVATE_ADDRESS)
        || check_type(N->getSrcValue(), AMDILAS::GLOBAL_ADDRESS))
        );
}

bool AMDILDAGToDAGISel::isLocalLoad(const  LoadSDNode *N) const {
  return check_type(N->getSrcValue(), AMDILAS::LOCAL_ADDRESS)
    && !isFlatASOverrideEnabled();
}

bool AMDILDAGToDAGISel::isRegionLoad(const  LoadSDNode *N) const {
  return check_type(N->getSrcValue(), AMDILAS::REGION_ADDRESS)
    && !isFlatASOverrideEnabled();
}

bool AMDILDAGToDAGISel::isCPLoad(const LoadSDNode *N) const {
  MachineMemOperand *MMO = N->getMemOperand();
  if (check_type(N->getSrcValue(), AMDILAS::PRIVATE_ADDRESS)
      && !isFlatASOverrideEnabled()) {
    if (MMO) {
      const Value *V = MMO->getValue();
      const PseudoSourceValue *PSV = dyn_cast<PseudoSourceValue>(V);
      if (PSV && PSV == PseudoSourceValue::getConstantPool()) {
        return true;
      }
    }
  }
  return false;
}

bool AMDILDAGToDAGISel::isPrivateLoad(const LoadSDNode *N) const {
  if (check_type(N->getSrcValue(), AMDILAS::PRIVATE_ADDRESS)
      && !isFlatASOverrideEnabled()) {
    // Check to make sure we are not a constant pool load or a constant load
    // that is marked as a private load
    if (isCPLoad(N) || isConstantLoad(N, -1)) {
      return false;
    }
  }
  if (!check_type(N->getSrcValue(), AMDILAS::LOCAL_ADDRESS)
      && !check_type(N->getSrcValue(), AMDILAS::GLOBAL_ADDRESS)
      && !check_type(N->getSrcValue(), AMDILAS::REGION_ADDRESS)
      && !check_type(N->getSrcValue(), AMDILAS::CONSTANT_ADDRESS)
      && !isFlatASOverrideEnabled())
  {
    return true;
  }
  return false;
}

const char *AMDILDAGToDAGISel::getPassName() const {
  return "AMDIL DAG->DAG Pattern Instruction Selection";
}

SDNode*
AMDILDAGToDAGISel::xformAtomicInst(SDNode *N)
{
  uint32_t addVal = 1;
  bool addOne = false;
  unsigned opc = N->getOpcode();
  switch (opc) {
    default: return N;
    case AMDILISD::ATOM_G_ADD:
    case AMDILISD::ATOM_G_AND:
    case AMDILISD::ATOM_G_MAX:
    case AMDILISD::ATOM_G_UMAX:
    case AMDILISD::ATOM_G_MIN:
    case AMDILISD::ATOM_G_UMIN:
    case AMDILISD::ATOM_G_OR:
    case AMDILISD::ATOM_G_SUB:
    case AMDILISD::ATOM_G_RSUB:
    case AMDILISD::ATOM_G_XCHG:
    case AMDILISD::ATOM_G_XOR:
    case AMDILISD::ATOM_G_LOAD:
    case AMDILISD::ATOM_G_STORE:
    case AMDILISD::ATOM_G_ADD_NORET:
    case AMDILISD::ATOM_G_AND_NORET:
    case AMDILISD::ATOM_G_MAX_NORET:
    case AMDILISD::ATOM_G_UMAX_NORET:
    case AMDILISD::ATOM_G_MIN_NORET:
    case AMDILISD::ATOM_G_UMIN_NORET:
    case AMDILISD::ATOM_G_OR_NORET:
    case AMDILISD::ATOM_G_SUB_NORET:
    case AMDILISD::ATOM_G_RSUB_NORET:
    case AMDILISD::ATOM_G_XCHG_NORET:
    case AMDILISD::ATOM_G_XOR_NORET:
    case AMDILISD::ATOM_L_ADD:
    case AMDILISD::ATOM_L_AND:
    case AMDILISD::ATOM_L_MAX:
    case AMDILISD::ATOM_L_UMAX:
    case AMDILISD::ATOM_L_MIN:
    case AMDILISD::ATOM_L_UMIN:
    case AMDILISD::ATOM_L_OR:
    case AMDILISD::ATOM_L_SUB:
    case AMDILISD::ATOM_L_RSUB:
    case AMDILISD::ATOM_L_XCHG:
    case AMDILISD::ATOM_L_XOR:
    case AMDILISD::ATOM_L_ADD_NORET:
    case AMDILISD::ATOM_L_AND_NORET:
    case AMDILISD::ATOM_L_MAX_NORET:
    case AMDILISD::ATOM_L_UMAX_NORET:
    case AMDILISD::ATOM_L_MIN_NORET:
    case AMDILISD::ATOM_L_UMIN_NORET:
    case AMDILISD::ATOM_L_OR_NORET:
    case AMDILISD::ATOM_L_SUB_NORET:
    case AMDILISD::ATOM_L_RSUB_NORET:
    case AMDILISD::ATOM_L_XCHG_NORET:
    case AMDILISD::ATOM_L_XOR_NORET:
    case AMDILISD::ATOM_R_ADD:
    case AMDILISD::ATOM_R_AND:
    case AMDILISD::ATOM_R_MAX:
    case AMDILISD::ATOM_R_UMAX:
    case AMDILISD::ATOM_R_MIN:
    case AMDILISD::ATOM_R_UMIN:
    case AMDILISD::ATOM_R_OR:
    case AMDILISD::ATOM_R_SUB:
    case AMDILISD::ATOM_R_RSUB:
    case AMDILISD::ATOM_R_XCHG:
    case AMDILISD::ATOM_R_XOR:
    case AMDILISD::ATOM_R_ADD_NORET:
    case AMDILISD::ATOM_R_AND_NORET:
    case AMDILISD::ATOM_R_MAX_NORET:
    case AMDILISD::ATOM_R_UMAX_NORET:
    case AMDILISD::ATOM_R_MIN_NORET:
    case AMDILISD::ATOM_R_UMIN_NORET:
    case AMDILISD::ATOM_R_OR_NORET:
    case AMDILISD::ATOM_R_SUB_NORET:
    case AMDILISD::ATOM_R_RSUB_NORET:
    case AMDILISD::ATOM_R_XCHG_NORET:
    case AMDILISD::ATOM_R_XOR_NORET:
    case AMDILISD::ATOM_G_CMPXCHG:
    case AMDILISD::ATOM_G_CMPXCHG_NORET:
    case AMDILISD::ATOM_L_CMPXCHG:
    case AMDILISD::ATOM_L_CMPXCHG_NORET:
    case AMDILISD::ATOM_R_CMPXCHG:
    case AMDILISD::ATOM_R_CMPXCHG_NORET:
             break;
    case AMDILISD::ATOM_G_DEC:
    case AMDILISD::ATOM_G_INC:
    case AMDILISD::ATOM_G_DEC_NORET:
    case AMDILISD::ATOM_G_INC_NORET:
    case AMDILISD::ATOM_L_DEC:
    case AMDILISD::ATOM_L_INC:
    case AMDILISD::ATOM_L_DEC_NORET:
    case AMDILISD::ATOM_L_INC_NORET:
    case AMDILISD::ATOM_R_DEC:
    case AMDILISD::ATOM_R_INC:
    case AMDILISD::ATOM_R_DEC_NORET:
    case AMDILISD::ATOM_R_INC_NORET:
             addOne = true;
             addVal = (uint32_t)-1;
             break;
  }
  // The largest we can have is a cmpxchg w/ a return value and an output chain.
  // The cmpxchg function has 3 inputs and a single output along with an
  // output change and a target constant, giving a total of 6.
  SDValue Ops[12];
  unsigned x = 0;
  unsigned y = N->getNumOperands();
  for (x = 0; x < y; ++x) {
    Ops[x] = N->getOperand(x);
  }
  if (addOne) {
    Ops[x++] = SDValue(SelectCode(CurDAG->getConstant(addVal, MVT::i32).getNode()), 0);
  }
  Ops[x++] = CurDAG->getTargetConstant(0, MVT::i32);
  SDVTList Tys = N->getVTList();
  MemSDNode *MemNode = dyn_cast<MemSDNode>(N);
  assert(MemNode && "Atomic should be of MemSDNode type!");
  N = CurDAG->getMemIntrinsicNode(opc, N->getDebugLoc(), Tys, Ops, x,
      MemNode->getMemoryVT(), MemNode->getMemOperand()).getNode();
  return N;
}

#ifdef DEBUGTMP
#ifndef USE_APPLE
#undef INT64_C
#endif
#endif
#undef DEBUGTMP
