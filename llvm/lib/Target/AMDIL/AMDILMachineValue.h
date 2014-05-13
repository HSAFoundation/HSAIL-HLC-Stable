//===------- AMDILMachineValue.h AMDIL Machine Value -*- C++ -*------===//
//
// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//
//===----------------------------------------------------------------===//

#ifndef _AMDILMACHINEVALUE_H_
#define _AMDILMACHINEVALUE_H_

#include "llvm/ADT/DenseMap.h"
#include "llvm/CodeGen/MachineOperand.h"

#include <map>

namespace llvm {
//
// Machine Value (MValue): data represention for machine instruction operands.  The Same
// operands in the same or different machine instructions share the same MValue. It is a
// convenient abstraction for MachineOperand.
//
// MValue can be used for computing global use/def chains as it is now used in MI level
// dead code elimination.


class MValueManager;

class  MValue {
  friend class MValueManager;

public:
  explicit MValue(const TargetMachine *aTM, MachineOperand* aMO) :
    TM(aTM),
    MO(*aMO),
    Next(NULL) {
    MO.clearParent();
  }

  MachineOperand::MachineOperandType getType() const {
    return MO.getType();
  }

  bool isReg() const { return MO.isReg(); }
  bool isImm() const { return MO.isImm(); }
  bool isCImm() const { return MO.isCImm(); }
  bool isFPImm() const { return MO.isFPImm(); }
  bool isMBB() const { return MO.isMBB(); }
  bool isFI() const { return MO.isFI(); }
  bool isCPI() const { return MO.isCPI(); }
  bool isJTI() const { return MO.isJTI(); }
  bool isGlobal() const { return MO.isGlobal(); }
  bool isSymbol() const { return MO.isSymbol(); }
  bool isBlockAddress() const { return MO.isBlockAddress(); }
  bool isRegMask() const { return MO.isRegMask(); }
  bool isMetadata() const { return MO.isMetadata(); }
  bool isMCSymbol() const { return MO.isMCSymbol(); }

  unsigned getReg() const { return MO.getReg(); }
  unsigned getSubReg() const { return MO.getSubReg(); }
  int64_t getImm() const { return MO.getImm(); }
  const ConstantInt *getCImm() const { return MO.getCImm(); }
  const ConstantFP *getFPImm() const { return MO.getFPImm(); }
  MachineBasicBlock *getMBB() const { return MO.getMBB(); }
  int getIndex() const { return MO.getIndex(); }
  const GlobalValue *getGlobal() const { return MO.getGlobal(); }
  const BlockAddress *getBlockAddress() const { return MO.getBlockAddress(); }
  MCSymbol *getMCSymbol() const { return MO.getMCSymbol(); }
  int64_t getOffset() const { return MO.getOffset(); }
  const uint32_t *getRegMask() const { return MO.getRegMask(); }
  const char *getSymbolName() const { return MO.getSymbolName(); }
  const MDNode *getMetadata() const { return MO.getMetadata(); }

  void setReg(unsigned Reg) { MO.setReg(Reg); }
  void setSubReg(unsigned SubReg) { MO.setSubReg(SubReg); }
  void setImm(int64_t immVal) { MO.setImm(immVal); }
  void setOffset(int64_t Offset) { MO.setOffset(Offset); }
  void setIndex(int Idx) { MO.setIndex(Idx); }
  void setMBB(MachineBasicBlock *MBB) { MO.setMBB(MBB); }

#if 0
  static MValue *CreateImm(int64_t Val) {
    MachineOperand Op = MachineOperand::CreateImm(Val);
    MValue *MV = new MValue(&Op);
    return MV;
  }

  static MValue *CreateCImm(const ConstantInt *CI) {
    MachineOperand Op = MachineOperand::CreateCImm(CI);
    MValue *MV = new MValue(&Op);
    return MV;
  }

  static MValue *CreateFPImm(const ConstantFP *CFP) {
    MachineOperand Op = MachineOperand::CreateFPImm(CFP);
    MValue *MV = new MValue(&Op);
    return MV;
  }
#endif

  static MValue *CreateReg(unsigned Reg, const TargetMachine *aTM) {
    MachineOperand Op = MachineOperand::CreateReg(Reg, false);
    MValue *MV = new MValue(aTM, &Op);
    return MV;
  }

  void print(raw_ostream &os) const {
    MO.print(os, TM);
  }

  // For debug
  void dump() const;

private:
  const TargetMachine *TM;

  // Using MachineOperand as a quick and simple way to keep all needed
  // information for an machine value.
  MachineOperand MO;

  // Used to link all MValue with the same Map key.
  MValue *Next;
};

static inline raw_ostream &operator<<(raw_ostream &OS, const MValue &MV) {
  MV.print(OS);
  return OS;
}

class MValueManager {
public:
  typedef DenseMap<unsigned int, MValue*>  RegMapType;
  //typedef DenseMap<int64_t, MValue*>       ImmMapType;
  typedef std::map<int64_t, MValue*>       ImmMapType;
  //typedef DenseMap<int, MValue*>           IndexMapType;
  typedef std::map<int, MValue*>           IndexMapType;
  typedef DenseMap<const void *, MValue*>  AddrMapType;

private:
  const TargetMachine* TM;
  RegMapType RegMap;
  ImmMapType ImmMap;

  // Each mapped value is a linked list
  IndexMapType IndexMap;
  AddrMapType  AddrMap;


  // Workhorse that calculates MValue for a given MachineOperand.
  MValue *getOrInsert(MachineOperand *MO, bool DoInsert);
  MValue *getOrInsert(unsigned reg, bool DoInsert);

  void resetIndexMap() {
    for (IndexMapType::iterator I = IndexMap.begin(), E = IndexMap.end();
         I != E; ++I) {
      MValue *MV = I->second;
      while (MV) {
        MValue *tMV = MV;
        MV = MV->Next;
        delete tMV;
      }
    }
    IndexMap.clear();
  }

  void resetAddrMap() {
    for (AddrMapType::iterator I = AddrMap.begin(), E = AddrMap.end();
         I != E; ++I) {
      MValue *MV = I->second;
      while (MV) {
        MValue *tMV = MV;
        MV = MV->Next;
        delete tMV;
      }
    }
    AddrMap.clear();
  }

  void reset() {
    // free memory for all MValue
    for (RegMapType::iterator I = RegMap.begin(), E = RegMap.end();
         I != E; ++I) {
      delete I->second;
    }
    RegMap.clear();

    for (ImmMapType::iterator I = ImmMap.begin(), E = ImmMap.end();
         I != E; ++I) {
      delete I->second;
    }
    ImmMap.clear();

    resetIndexMap();
    resetAddrMap();
  }

public:

  MValueManager(const TargetMachine* aTM) :
    TM(aTM),
    RegMap(),
    ImmMap(),
    IndexMap(),
    AddrMap() {}

  ~MValueManager() {
    reset();
  }

  MValue *getMValue(MachineOperand *MO) {
    return getOrInsert(MO, false);
  }
  MValue *getMValue(unsigned reg) {
    return getOrInsert(reg, false);
  }
  MValue *getOrInsertMValue(MachineOperand *MO) {
    return getOrInsert(MO, true);
  }
  MValue *getOrInsertMValue(unsigned reg) {
    return getOrInsert(reg, true);
  }

  bool isMOIdentical(MachineOperand *MO1, MachineOperand *MO2);

  void clear() {
    reset();
  }
};

}

#endif
