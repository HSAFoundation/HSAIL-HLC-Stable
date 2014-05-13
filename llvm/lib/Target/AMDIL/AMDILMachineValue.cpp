//===------- AMDILMachineValue.cpp - AMDIL Machine Value -*- C++ -*------===//
//
// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//===--------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "AMDILMachineValue.h"
#include "llvm/Support/ErrorHandling.h"


using namespace llvm;

void MValue::dump() const
{
  print(dbgs());
  dbgs() << '\n';
}

// Return true if two MachineOperands refers to the same
bool MValueManager::isMOIdentical(MachineOperand *MO1, MachineOperand *MO2)
{
  if (MO1->getType() != MO2->getType()) {
    return false;
  }

  switch (MO1->getType()) {
  default: llvm_unreachable("Unrecognized operand type");
  case MachineOperand::MO_Register:
    return (MO1->getReg() == MO2->getReg()) &&
           (MO1->getSubReg() == MO2->getSubReg());
  case MachineOperand::MO_Immediate:
    return MO1->getImm() == MO2->getImm();
  case MachineOperand::MO_FPImmediate:
    return MO1->getFPImm() == MO2->getFPImm();
  case MachineOperand::MO_MachineBasicBlock:
    return MO1->getMBB() == MO2->getMBB();
  case MachineOperand::MO_FrameIndex:
    return MO1->getIndex() == MO2->getIndex();
  case MachineOperand::MO_ConstantPoolIndex:
    return (MO1->getIndex() == MO2->getIndex()) &&
           (MO1->getOffset() == MO2->getOffset());
  case MachineOperand::MO_JumpTableIndex:
    return MO1->getIndex() == MO2->getIndex();
  case MachineOperand::MO_GlobalAddress:
    return (MO1->getGlobal() == MO2->getGlobal()) &&
           (MO1->getOffset() == MO2->getOffset());
  case MachineOperand::MO_ExternalSymbol:
    return !strcmp(MO1->getSymbolName(), MO2->getSymbolName()) &&
           (MO1->getOffset() == MO2->getOffset());
  case MachineOperand::MO_BlockAddress:
    return MO1->getBlockAddress() == MO2->getBlockAddress();
  case MachineOperand::MO_RegisterMask:
    return MO1->getRegMask() == MO2->getRegMask();
  case MachineOperand::MO_MCSymbol:
    return MO1->getMCSymbol() == MO2->getMCSymbol();
  case MachineOperand::MO_Metadata:
    return MO1->getMetadata() == MO2->getMetadata();
  }
}

MValue* MValueManager::getOrInsert(unsigned reg, bool DoInsert)
{
  MValue *MV = NULL;
  RegMapType::iterator it = RegMap.find(reg);
  if (it != RegMap.end()) {
    MV = it->second;
  }
  if ((MV == NULL) && DoInsert) {
    MV = MValue::CreateReg(reg, TM);
    RegMap[reg] = MV;
  }
  return MV;
}


MValue* MValueManager::getOrInsert(MachineOperand *MO, bool DoInsert)
{
  bool useIndexMap = false;
  bool useAddrMap  = false;
  const void *addrKey = NULL;
  int  indexKey = 0;

  MValue *MV = NULL;
  switch (MO->getType()) {
  default: llvm_unreachable("Unrecognized operand type");
  case MachineOperand::MO_Register:
    return getOrInsert(MO->getReg(), DoInsert);

  case MachineOperand::MO_Immediate:
    {
      ImmMapType::iterator it = ImmMap.find(MO->getImm());
      if (it != ImmMap.end()) {
        MV = it->second;
      }
      if ((MV == NULL) && DoInsert) {
        MV = new MValue(TM, MO);
        ImmMap[MO->getImm()] = MV;
      }
      return MV;
    }

  case MachineOperand::MO_FPImmediate:
    addrKey = MO->getFPImm();
    useAddrMap = true;
    break;

  case MachineOperand::MO_MachineBasicBlock:
    addrKey = MO->getMBB();
    useAddrMap = true;
    break;

  case MachineOperand::MO_GlobalAddress:
    addrKey = MO->getGlobal();
    useAddrMap = true;
    break;

  case MachineOperand::MO_ExternalSymbol:
    addrKey = MO->getSymbolName();
    useAddrMap = true;
    break;

  case MachineOperand::MO_BlockAddress:
    addrKey = MO->getBlockAddress();
    useAddrMap = true;
    break;

  case MachineOperand::MO_RegisterMask:
    addrKey = MO->getRegMask();
    useAddrMap = true;
    break;

  case MachineOperand::MO_MCSymbol:
    addrKey = MO->getMCSymbol();
    useAddrMap = true;
    break;

  case MachineOperand::MO_Metadata:
    addrKey = MO->getMetadata();
    useAddrMap = true;
    break;

  case MachineOperand::MO_FrameIndex:
    indexKey = MO->getIndex();
    useIndexMap = true;
    break;

  case MachineOperand::MO_ConstantPoolIndex:
    indexKey = MO->getIndex();
    useIndexMap = true;
    break;

  case MachineOperand::MO_JumpTableIndex:
    indexKey = MO->getIndex();
    useIndexMap = true;
    break;
  }

  MValue *Head = NULL;
  if (useAddrMap) {
    AddrMapType::iterator it = AddrMap.find(addrKey);
    if (it != AddrMap.end()) {
      Head = it->second;
    }
  } else if (useIndexMap) {
    IndexMapType::iterator it = IndexMap.find(indexKey);
    if (it != IndexMap.end()) {
      Head = it->second;
    }
  }

  if (Head) {
    for (MV = Head; MV; MV = MV->Next) {
      if (isMOIdentical(&MV->MO, MO)) {
        break;
      }
    }
  }

  if ((MV == NULL) && DoInsert) {
    MV = new MValue(TM, MO);
    MV->Next = Head;
    if (useAddrMap) {
      AddrMap[addrKey] = MV;
    } else if (useIndexMap) {
      IndexMap[indexKey] = MV;
    } else {
      // NOT REACHABLE
      llvm_unreachable("getOrInsert() : not reachable");
    }
  }
  return MV;
}
