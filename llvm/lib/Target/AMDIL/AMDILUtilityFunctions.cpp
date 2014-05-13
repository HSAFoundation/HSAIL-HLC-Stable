//===-- AMDILUtilityFunctions.cpp - AMDIL Utility Functions       ---------===//
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

#include "AMDILUtilityFunctions.h"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILISelLowering.h"
#include "AMDILCompilerErrors.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/Instruction.h"
#include "llvm/Type.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"

#include <cstdio>
#include <queue>
#include <list>
using namespace llvm;

unsigned getVectorReg(unsigned Reg, const TargetRegisterInfo *TRI) {
  for (MCSuperRegIterator Supers(Reg, TRI); Supers.isValid(); ++Supers) {
    return getVectorReg(*Supers, TRI);
  }
  return Reg;
}

void printSDNode(const SDNode *Node) {
  printf("Opcode: %d isTargetOpcode: %d isMachineOpcode: %d\n",
      Node->getOpcode(), Node->isTargetOpcode(), Node->isMachineOpcode());
  printf("Empty: %d OneUse: %d Size: %d NodeID: %d\n",
      Node->use_empty(), Node->hasOneUse(), (int)Node->use_size(), Node->getNodeId());

  for (unsigned I = 0, N = Node->getNumOperands(); I < N; ++I) {
    printf("OperandNum: %d ValueCount: %d ValueType: %d\n",
        I, Node->getNumValues(), Node->getValueType(0).getSimpleVT().SimpleTy);
    printSDValue(Node->getOperand(I), 0);
  }
}

void printSDValue(const SDValue &Op, int Level) {
  dbgs() << "\nOp: " << &Op << '\n'
         << " OpCode: " << Op.getOpcode() << '\n'
         << " NumOperands: " << Op.getNumOperands() << '\n'
         << " IsTarget: " << Op.isTargetOpcode() << '\n'
         << " IsMachine: " << Op.isMachineOpcode() << '\n';

  if (Op.isMachineOpcode()) {
    dbgs() << "MachineOpcode: " << Op.getMachineOpcode() << '\n';
  }

  EVT VT = Op.getValueType();
  dbgs() << "ValueType: " << VT.getSimpleVT().SimpleTy << '\n'
         << "UseEmpty: " << Op.use_empty() << '\n'
         << "OneUse: " << Op.hasOneUse() << '\n';

  if (Level != 0) {
    dbgs() << "Children for " << Level << ":\n";
    for (unsigned I = 0, N = Op.getNumOperands(); I < N; ++I) {
      dbgs() << "Child " << Level << "->" << I << ':';
      printSDValue(Op.getOperand(I), Level - 1);
    }
  }
}

bool check_type(const Value *Ptr, unsigned AddrSpace) {
  if (!Ptr) {
    return false;
  }

  Type *PtrType = Ptr->getType();
  return dyn_cast<PointerType>(PtrType)->getAddressSpace() == AddrSpace;
}

size_t getNumElements(Type *const T) {
  if (!T) {
    return 0;
  }

  switch (T->getTypeID()) {
  case Type::X86_FP80TyID:
  case Type::FP128TyID:
  case Type::PPC_FP128TyID:
  case Type::LabelTyID:
    llvm_unreachable("These types are not supported by this backend");
  default:
  case Type::FloatTyID:
  case Type::DoubleTyID:
    return 1;
  case Type::PointerTyID:
    return getNumElements(dyn_cast<PointerType>(T));
  case Type::IntegerTyID:
    return getNumElements(dyn_cast<IntegerType>(T));
  case Type::StructTyID:
    return getNumElements(dyn_cast<StructType>(T));
  case Type::ArrayTyID:
    return getNumElements(dyn_cast<ArrayType>(T));
  case Type::FunctionTyID:
    return getNumElements(dyn_cast<FunctionType>(T));
  case Type::VectorTyID:
    return getNumElements(dyn_cast<VectorType>(T));
  }
}

size_t getNumElements(StructType *const ST) {
  if (!ST) {
    return 0;
  }

  size_t Size = 0;

  for (StructType::element_iterator I = ST->element_begin(),
      E = ST->element_end(); I != E; ++I) {
    Type *CurType = *I;
    Size += getNumElements(CurType);
  }

  return Size;
}

size_t getNumElements(IntegerType *const IT) {
  return IT ? 1 : 0;
}

size_t getNumElements(FunctionType *const FT) {
  llvm_unreachable("Should not be able to calculate the number of "
      "elements of a function type");
  return 0;
}

size_t getNumElements(ArrayType *const AT) {
  return AT ? (size_t)(getNumElements(AT->getElementType())
      * AT->getNumElements()) : 0;
}

size_t getNumElements(VectorType *const VT) {
  assert(VT && "null type");
  size_t nElem = (size_t)RoundUpToAlignment(VT->getNumElements(), 2);
  return nElem * getNumElements(VT->getElementType());
}

size_t getNumElements(PointerType *const PT) {
  if (!PT) {
    return 0;
  }

  size_t Size = 0;

  for (size_t I = 0, N = PT->getNumContainedTypes(); I < N; ++I) {
    Size += getNumElements(PT->getContainedType(I));
  }

  return Size;
}

// TODO: Remove this
static const llvm::Value *getBasePointerValue(const llvm::Value *V) {
  if (!V) {
    return NULL;
  }

  ValueMap<const Value *, bool> ValueBitMap;
  std::queue<const Value *, std::list<const Value *> > ValueQueue;
  ValueQueue.push(V);

  while (!ValueQueue.empty()) {
    V = ValueQueue.front();

    if (ValueBitMap.find(V) == ValueBitMap.end()) {
      ValueBitMap[V] = true;

      if (dyn_cast<Argument>(V) && dyn_cast<PointerType>(V->getType())) {
        return V;
      } else if (dyn_cast<GlobalVariable>(V)) {
        return V;
      } else if (dyn_cast<Constant>(V)) {
        const ConstantExpr *CE = dyn_cast<ConstantExpr>(V);
        if (CE) {
          ValueQueue.push(CE->getOperand(0));
        }
      } else if (const AllocaInst *AI = dyn_cast<AllocaInst>(V)) {
        return AI;
      } else if (const Instruction *I = dyn_cast<Instruction>(V)) {
        uint32_t NumOps = I->getNumOperands();

        for (uint32_t X = 0; X < NumOps; ++X) {
          ValueQueue.push(I->getOperand(X));
        }
      } else {
        // llvm_unreachable("Found a Value that we didn't know how to handle!");
      }
    }

    ValueQueue.pop();
  }

  return NULL;
}

const llvm::Value *getMemOpUnderlyingObject(const llvm::MachineInstr *MI,
                                            const DataLayout *DL) {
  if (MI->memoperands_empty())
    return 0;

  MachineMemOperand *MemOp = *MI->memoperands_begin();
  if (!MemOp)
    return 0;

  // This might happen if it's a load from undef.
  if (!MemOp->getValue())
    return 0;

  SmallVector<Value *, 2> Objects;
  GetUnderlyingObjects(const_cast<Value *>(MemOp->getValue()), Objects, DL, 0);

  // FIXME: What should we do if we actually do have multiple underlying values?
  if (Objects.size() != 1)
    return 0;

  const Value *Object = Objects.front();

  // Follow back in the case the pointer comes from a series of ptrtoint /
  // inttoptr
  // TODO: shouldn't need to do this
  if (isa<CastInst>(Object))
    return getBasePointerValue(Object);

  return Object;
}

bool commaPrint(int I, raw_ostream &O) {
  O << ":" << I;
  return false;
}

bool isLoadInst(const llvm::MachineInstr *MI) {
  return !(MI->getDesc().TSFlags & (1ULL << AMDID::LOADCONST))
      && (MI->getDesc().TSFlags & (1ULL << AMDID::LOAD));
}

bool isPtrLoadInst(const llvm::MachineInstr *MI) {
  return isLoadInst(MI)
      && !(MI->getDesc().TSFlags & (1ULL << AMDID::IMAGE));
}

bool isSWSExtLoadInst(const llvm::MachineInstr *MI) {
  return isPtrLoadInst(MI) && (MI->getDesc().TSFlags & (1ULL << AMDID::SWSEXTLD));
}

bool isExtLoadInst(const llvm::MachineInstr *MI) {
  return isPtrLoadInst(MI) && (MI->getDesc().TSFlags & AMDID::EXTLOAD);
}

bool isSExtLoadInst(const llvm::MachineInstr *MI) {
  return isPtrLoadInst(MI) && (MI->getDesc().TSFlags & (1ULL << AMDID::SEXTLOAD))
      && !(MI->getDesc().TSFlags & (1ULL << AMDID::ZEXTLOAD));
}

bool isAExtLoadInst(const llvm::MachineInstr *MI) {
  return isPtrLoadInst(MI) && (MI->getDesc().TSFlags & AMDID::AEXTLOAD);
}

bool isZExtLoadInst(const llvm::MachineInstr *MI) {
  return isPtrLoadInst(MI) && (MI->getDesc().TSFlags & (1ULL << AMDID::ZEXTLOAD))
      && !(MI->getDesc().TSFlags & (1ULL << AMDID::SEXTLOAD));
}

bool isPtrStoreInst(const llvm::MachineInstr *MI) {
  return isStoreInst(MI) && !(MI->getDesc().TSFlags & (1ULL << AMDID::IMAGE));
}

bool isStoreInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::STORE);
}

bool isArenaInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::ARENAUAV);
}

bool isTruncStoreInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::TRUNCATE);
}

bool isAtomicInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::ATOMIC);
}

bool isVolatileInst(const llvm::MachineInstr *MI) {
  if (MI->memoperands_empty()) {
    return false;
  }

  // If there is a volatile mem operand, this is a volatile instruction.
  for (MachineInstr::mmo_iterator I = MI->memoperands_begin(),
      E = MI->memoperands_end(); I != E; ++I) {
    MachineMemOperand *Operand = *I;

    if (Operand->isVolatile()) {
      return true;
    }
  }

  return false;
}

bool isGlobalInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::GLOBAL);
}

bool isPrivateInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::PRIVATE);
}

bool isConstantInst(const llvm::MachineInstr *MI) {
  return (MI->getDesc().TSFlags & (1ULL << AMDID::CONSTANT))
      || isConstantPoolInst(MI);
}

bool is64bitLSOp(const llvm::MachineInstr *MI) {
  return (isPtrLoadInst(MI) || isPtrStoreInst(MI))
      && is64BitInst(MI);
}

bool isConstantPoolInst(const llvm::MachineInstr *MI) {
  return (MI->getDesc().TSFlags & (1ULL << AMDID::CPOOL));
}

bool isRegionInst(const llvm::MachineInstr *MI) {
  return (MI->getDesc().TSFlags & (1ULL << AMDID::REGION));
}

bool isGWSInst(const llvm::MachineInstr *MI) {
  return (MI->getDesc().TSFlags & (1ULL << AMDID::GWS));
}

bool isLocalInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::LOCAL);
}

bool isLDSInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::LDS);
}

bool isGDSInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::GDS);
}

bool isUAVArenaInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::ARENAUAV);
}

bool isUAVRawInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::RAWUAV);
}

bool isCBInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::CBMEM);
}

bool isScratchInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::SCRATCH);
}

bool isImageInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::IMAGE);
}

bool is64BitImageInst(const llvm::MachineInstr *MI) {
  return isImageInst(MI) && is64BitInst(MI);
}

bool isReadImageInst(const llvm::MachineInstr *MI) {
  return isImageInst(MI) && isLoadInst(MI) && !isImageTXLDInst(MI);
}

bool isWriteImageInst(const llvm::MachineInstr *MI) {
  return isImageInst(MI) && isStoreInst(MI);
}

bool isImageInfoInst(const llvm::MachineInstr *MI) {
  return isImageInst(MI)
      && (MI->getDesc().TSFlags & AMDID::INFO);
}

bool isImageInfo0Inst(const llvm::MachineInstr *MI) {
  return isImageInst(MI)
      && (MI->getDesc().TSFlags & (1ULL << AMDID::INFO0));
}

bool isImageInfo1Inst(const llvm::MachineInstr *MI) {
  return isImageInst(MI)
      && (MI->getDesc().TSFlags & (1ULL << AMDID::INFO1));
}

bool isImageTXLDInst(const llvm::MachineInstr *MI) {
  return isImageInst(MI)
      && MI->getDesc().TSFlags & (1ULL << AMDID::TXLD);
}

bool isSemaphoreInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::SEMA);
}

bool isAppendInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::APPEND);
}

bool isRegionAtomic(const llvm::MachineInstr *MI) {
  return isAtomicInst(MI) && isRegionInst(MI);
}

bool is64BitRegionAtomic(const llvm::MachineInstr *MI) {
  return isRegionAtomic(MI) && is64BitInst(MI);
}

bool isLocalAtomic(const llvm::MachineInstr *MI) {
  return isAtomicInst(MI) && isLocalInst(MI);
}

bool is64BitLocalAtomic(const llvm::MachineInstr *MI) {
  return isLocalAtomic(MI) && is64BitInst(MI);
}

bool isGlobalAtomic(const llvm::MachineInstr *MI) {
  return isAtomicInst(MI) && (isGlobalInst(MI)
      || isArenaInst(MI));
}

bool is64BitGlobalAtomic(const llvm::MachineInstr *MI) {
  return isGlobalAtomic(MI) && is64BitInst(MI);
}

bool isArenaAtomic(const llvm::MachineInstr *MI) {
  return isAtomicInst(MI) && isArenaInst(MI);
}

bool is64BitInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::ADDR64);
}

bool isPackedInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::PACKED);
}

bool isSub32BitIOInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::SUB32BITS);
}

bool isPackV2I8Inst(const llvm::MachineInstr *MI) {
  return (MI->getDesc().TSFlags & AMDID::TYPEMASK) == AMDID::TYPEV2I8
      && isPackedInst(MI) && isStoreInst(MI);
}

bool isPackV2I16Inst(const llvm::MachineInstr *MI) {
  return (MI->getDesc().TSFlags & AMDID::TYPEMASK) == AMDID::TYPEV2I16
      && isPackedInst(MI) && isStoreInst(MI);
}

bool isPackV4I8Inst(const llvm::MachineInstr *MI) {
  return (MI->getDesc().TSFlags & AMDID::TYPEMASK) == AMDID::TYPEV4I8
      && isPackedInst(MI) && isStoreInst(MI);
}

bool isPackV4I16Inst(const llvm::MachineInstr *MI) {
  return (MI->getDesc().TSFlags & AMDID::TYPEMASK) == AMDID::TYPEV4I16
      && isPackedInst(MI) && isStoreInst(MI);
}

bool isUnpackV2I8Inst(const llvm::MachineInstr *MI) {
  return (MI->getDesc().TSFlags & AMDID::TYPEMASK) == AMDID::TYPEV2I8
      && isPackedInst(MI) && isLoadInst(MI);
}

bool isUnpackV2I16Inst(const llvm::MachineInstr *MI) {
  return (MI->getDesc().TSFlags & AMDID::TYPEMASK) == AMDID::TYPEV2I16
      && isPackedInst(MI) && isLoadInst(MI);
}

bool isUnpackV4I8Inst(const llvm::MachineInstr *MI) {
  return (MI->getDesc().TSFlags & AMDID::TYPEMASK) == AMDID::TYPEV4I8
      && isPackedInst(MI) && isLoadInst(MI);
}

bool isUnpackV4I16Inst(const llvm::MachineInstr *MI) {
  return (MI->getDesc().TSFlags & AMDID::TYPEMASK) == AMDID::TYPEV4I16
      && isPackedInst(MI) && isLoadInst(MI);
}

bool isVectorOpInst(const llvm::MachineInstr *MI) {
  return MI->getDesc().TSFlags & (1ULL << AMDID::VECTOR);
}

bool isSkippedLiteral(const llvm::MachineInstr *MI, uint32_t OpNum) {
  uint32_t OpCode = MI->getOpcode();

  if ((OpCode >= AMDIL::VEXTRACTv2f32i
      && OpCode <= AMDIL::VEXTRACTv4i8r)
      && (OpNum == 2)) {
    return true;
  } else if ((OpCode >= AMDIL::VINSERTv2f32ii)
      && (OpCode <= AMDIL::VINSERTv4i8rr)
      && ((OpNum == 3)  || (OpNum == 4))) {
    return true;
  }

  return false;
}

bool isBypassedLiteral(const llvm::MachineInstr *MI, uint32_t OpNum) {
  uint32_t OpCode = MI->getOpcode();

  if (((int)OpNum == (int)(MI->getNumOperands() - 1))
      && (isAtomicInst(MI)
          || isScratchInst(MI)
          || isLDSInst(MI)
          || isGDSInst(MI)
          || isUAVArenaInst(MI)
          || isUAVRawInst(MI)
          || isCBInst(MI)
          || OpCode == AMDIL::CASE)) {
    return true;
  } else if (OpNum == 1 &&
      (isAppendInst(MI)
          || isReadImageInst(MI)
          || isImageTXLDInst(MI)
          || isCBInst(MI)))  {
    return true;
  } else if (OpNum == 0 &&
      (isSemaphoreInst(MI)
          || isReadImageInst(MI)
          || isWriteImageInst(MI))) {
    return true;
  } else if (OpNum == 2 && isReadImageInst(MI)) {
    return true;
  } else {
    return false;
  }
}

bool isXComponentReg(unsigned Reg) {
  return (Reg >= AMDIL::Rx1 && Reg < AMDIL::Rxy1) ||
         (Reg >= AMDIL::CB1_x0 && Reg < AMDIL::CB1_xy0) ||
         (Reg >= AMDIL::INx0 && Reg < AMDIL::INxy0) ||
         (Reg >= AMDIL::OUTx0 && Reg < AMDIL::OUTxy0) ||
         (Reg == AMDIL::MEMx);
}

bool isYComponentReg(unsigned Reg) {
  return (Reg >= AMDIL::Ry1 && Reg < AMDIL::Rz1) ||
         (Reg >= AMDIL::CB1_y0 && Reg < AMDIL::CB1_z0) ||
         (Reg >= AMDIL::INy0 && Reg < AMDIL::INz0) ||
         (Reg >= AMDIL::OUTy0 && Reg < AMDIL::OUTz0);
}

bool isZComponentReg(unsigned Reg) {
  return (Reg >= AMDIL::Rz1 && Reg < AMDIL::Rzw1) ||
         (Reg >= AMDIL::CB1_z0 && Reg < AMDIL::CB1_zw0) ||
         (Reg >= AMDIL::INz0 && Reg < AMDIL::INzw0) ||
         (Reg >= AMDIL::OUTz0 && Reg < AMDIL::OUTzw0);
}

bool isWComponentReg(unsigned Reg) {
  return (Reg >= AMDIL::Rw1 && Reg < AMDIL::Rx1) ||
         (Reg >= AMDIL::CB1_w0 && Reg < AMDIL::CB1_x0) ||
         (Reg >= AMDIL::INw0 && Reg < AMDIL::INx0) ||
         (Reg >= AMDIL::OUTw0 && Reg < AMDIL::OUTx0);
}

bool isXYComponentReg(unsigned Reg) {
  return (Reg >= AMDIL::Rxy1 && Reg < AMDIL::Ry1) ||
         (Reg >= AMDIL::CB1_xy0 && Reg < AMDIL::CB1_y0) ||
         (Reg >= AMDIL::INxy0 && Reg < AMDIL::INy0) ||
         (Reg >= AMDIL::OUTxy0 && Reg < AMDIL::OUTy0) ||
         Reg == AMDIL::MEMxy;
}

bool isZWComponentReg(unsigned Reg) {
  return (Reg >= AMDIL::Rzw1 && Reg < AMDIL::SDP) ||
         (Reg >= AMDIL::CB1_zw0 && Reg < AMDIL::CFG1) ||
         (Reg >= AMDIL::INzw0 && Reg < AMDIL::MEM) ||
         (Reg >= AMDIL::OUTzw0 && Reg < AMDIL::PRINTF);
}

const char *getSrcSwizzle(unsigned Idx) {
  static const char *SrcSwizzles[AMDIL_SRC_LAST]  = {
    "",
    ".x000", ".0x00", ".00x0", ".000x", ".y000", ".0y00", ".00y0", ".000y",
    ".z000", ".0z00", ".00z0", ".000z", ".w000", ".0w00", ".00w0", ".000w",
    ".xy00", ".00xy", ".zw00", ".00zw", ".xyz0", ".0xyz",
    ".xzxz", ".ywyw", ".x0y0", ".0x0y", ".0yzw", ".x0zw", ".xy0w",
    ".x"   , ".y"   , ".z"   , ".w"   , ".xyxy", ".zwzw", ".yzw0",
    ".z0w0", ".0z0w",
  };
  assert(Idx < sizeof(SrcSwizzles) / sizeof(SrcSwizzles[0])
      && "Idx passed in is invalid!");
  return SrcSwizzles[Idx];
}

const char *getDstSwizzle(unsigned Idx) {
  static const char *DstSwizzles[AMDIL_DST_LAST] = {
    "", ".x___", "._y__", ".__z_", ".___w", ".xy__", ".__zw",
    ".xyz_"
  };
  assert(Idx < sizeof(DstSwizzles) / sizeof(DstSwizzles[0])
      && "Idx passed in is invalid!");
  return DstSwizzles[Idx];
}

/// Helper function to get the currently set flags
void getAsmPrinterFlags(MachineInstr *MI, AMDILAS::InstrResEnc &CurRes) {
  // We need 16 bits of information, but LLVMr127097 cut the field in half.
  // So we have to use two different fields to store all of our information.
  uint16_t Upper = MI->getAsmPrinterFlags() << 8;
  uint16_t Lower = MI->getFlags();
  CurRes.u16all = Upper | Lower;
}
/// Helper function to clear the currently set flags and add the new flags.
void setAsmPrinterFlags(MachineInstr *MI, AMDILAS::InstrResEnc &CurRes) {
  // We need 16 bits of information, but LLVMr127097 cut the field in half.
  // So we have to use two different fields to store all of our information.
  MI->clearAsmPrinterFlags();
  MI->setFlags(0);
  uint8_t Lower = CurRes.u16all & 0xFF;
  uint8_t Upper = (CurRes.u16all >> 8) & 0xFF;
  MI->setFlags(Lower);
  MI->setAsmPrinterFlag((llvm::MachineInstr::CommentFlag)Upper);
}

// SymTab is a dummy arg to ease the transition...
const char *getTypeName(Type *Ptr,
                        const char *SymTab,
                        AMDILMachineFunctionInfo *MFI,
                        bool SignedType) {
  Type *Name = Ptr;

  switch (Ptr->getTypeID()) {
  case Type::StructTyID: {
    const StructType *ST = cast<StructType>(Ptr);
    if (!ST->isOpaque()) {
      return "struct";
    }

    // Ptr is a pre-LLVM 3.0 "opaque" type.
    StringRef Name = ST->getName();
    return StringSwitch<const char *>(Name)
      .StartsWith("struct._event_t", "event")
      .StartsWith("struct._image1d_t", "image1d")
      .StartsWith("struct._image1d_array_t", "image1d_array")
      .StartsWith("struct._image2d_t", "image2d")
      .StartsWith("struct._image2d_array_t", "image2d_array")
      .StartsWith("struct._image3d_t", "image3d")
      .StartsWith("struct._sema_t", "semaphore")
      .StartsWith("struct._counter32_t", "counter32")
      .StartsWith("struct._counter64_t", "counter64")
      .Default("opaque");
  }
  case Type::FloatTyID:
    return "float";
  case Type::DoubleTyID:
    return "double";
  case Type::HalfTyID:
    return "half";
  case Type::IntegerTyID: {
    LLVMContext &Ctx = Ptr->getContext();
    if (Name == Type::getInt8Ty(Ctx)) {
      return SignedType ? "i8" : "u8";
    } else if (Name == Type::getInt16Ty(Ctx)) {
      return SignedType ? "i16" : "u16";
    } else if (Name == Type::getInt32Ty(Ctx)) {
      return SignedType ? "i32" : "u32";
    } else if (Name == Type::getInt64Ty(Ctx)) {
      return SignedType ? "i64" : "u64";
    }

    break;
  }
  default:
    break;
  case Type::ArrayTyID: {
    const ArrayType *AT = cast<ArrayType>(Ptr);
    Name = AT->getElementType();
    return getTypeName(Name, SymTab, MFI, SignedType);
  }
  case Type::VectorTyID: {
    const VectorType *VT = cast<VectorType>(Ptr);
    Name = VT->getElementType();
    return getTypeName(Name, SymTab, MFI, SignedType);
  }
  case Type::PointerTyID: {
    const PointerType *PT = cast<PointerType>(Ptr);
    Name = PT->getElementType();
    return getTypeName(Name, SymTab, MFI, SignedType);
  }
  case Type::FunctionTyID: {
    const FunctionType *FT = cast<FunctionType>(Ptr);
    Name = FT->getReturnType();
    return getTypeName(Name, SymTab, MFI, SignedType);
  }
  }

  Name->dump();

  if (MFI) {
    MFI->addErrorMsg(amd::CompilerErrorMessage[UNKNOWN_TYPE_NAME]);
  }

  return "unknown";
}

#if LLVM_VERSION < 3316
bool isImageType(const Type *Ty, const TypeSymbolTable *SymTab) {
  if (!isa<StructType>(Ty) && !isa<OpaqueType>(Ty)) {
    return false;
  }
  const Type *i1d_type  = SymTab.lookup("struct._image1d_t");
  const Type *i1da_type = SymTab.lookup("struct._image1d_array_t");
  const Type *i1db_type = SymTab.lookup("struct._image1d_buffer_t");
  const Type *i2d_type = SymTab.lookup( "struct._image2d_t" );
  const Type *i2da_type = SymTab.lookup("struct._image2d_array_t");
  const Type *i3d_type = SymTab.lookup( "struct._image3d_t" );
  bool is_image = (Ty == i2d_type || Ty == i3d_type || Ty == i1d_type ||
                   Ty == i1da_type || Ty == i1db_type || Ty == i2da_type);
}
#else
bool isImageType(const Type *Ty) {
  if (!isa<StructType>(Ty)) {
    return false;
  }
  const StructType *ST = cast<StructType>(Ty);
  return ST->getName().startswith("struct._image1d_t") ||
         ST->getName().startswith("struct._image1d_array_t") ||
         ST->getName().startswith("struct._image1d_buffer_t") ||
         ST->getName().startswith("struct._image2d_t") ||
         ST->getName().startswith("struct._image2d_array_t") ||
         ST->getName().startswith("struct._image3d_t");
#endif
}
