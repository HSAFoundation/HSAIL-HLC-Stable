//===-- AMDILMachineEBB.h - AMDIL EBB Optimizations header   -*- C++ -*-===//
// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//
//==--------------------------------------------------------------------===//

/*
   AMDIL Extended Basic Block Optimizations works on a sequence of Basic Block
   that has a single entry and may have multiple exits.  This EBB Optimizer
   will work on both SSA form or non-SSA form (after register allocation).

   It will perform optimizations like copy and constant propagation, constant
   folding, CSE, DCE, special instruction optimizations, and etc.
*/

#ifndef _AMDILMACHINEEBB_H_
#define _AMDILMACHINEEBB_H_

#include "AMDIL.h"
#include "AMDILSubtarget.h"
#include "AMDILUtilityFunctions.h"
#include "AMDILMachineValue.h"
#include "llvm/Function.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

#include <vector>
#include <list>

namespace llvm {
class TargetMachine;

/*
   MVInfo denotes a unique value for each MV.  So if a MV is defined,
   a new MVI is created for it.  If a MV is upward exposed,  there is
   also a MVInfo for it denoting that it is upward exposed used.

   Note that it is also used to denote a new constant value.
 */
class MVInfo : public RefCountedBase<class MVInfo> {
  friend class MValueManager;
public:
  enum {
    MVI_DEFINE = 0x1,          // set if defined w/ explicit MO in MI.
    MVI_IDEFINE = 0x2,         // set if defined w/o explicit MO in MI.
    MVI_DUMMY_DEFINE = 0x4,    // Used for dummy define of a upward exposed
    MVI_DUMMY_IDEFINE = 0x8    // Used for dummy implicit define of
                               // a upward exposed use.
  };
  uint32_t       Flags;        // bits are defined by the above enum.

  MValue         *MV;

  MachineInstr  *CreatingMI;   // this MVI is created by this MI.
                               // Flags indicates if it is write or not.
  MachineOperand *MO;          // Original MO that is used to create this MV
  IntrusiveRefCntPtr<MVInfo> NewMVI; // MVInfo for new and replacement MV


  // Implicit MVIs that are overlapped by this MVI.
  std::vector<IntrusiveRefCntPtr<MVInfo> > OverlapMVIs;

  IntrusiveRefCntPtr<MVInfo>       Next;
  int  ID;                     // Internal use

  MVInfo(MValue *aMV, MachineInstr *aMI, MachineOperand *aMO, unsigned aFlags)
    : Flags(aFlags),
      MV(aMV),
      CreatingMI(aMI),
      MO(aMO),
      NewMVI(NULL),
      OverlapMVIs(),
      Next(NULL),
      ID (-1) {}

  ~MVInfo() { }

  bool isDummyDef() { return Flags & MVI_DUMMY_DEFINE; }
  bool isDummyIDef() { return Flags & MVI_DUMMY_IDEFINE; }
  bool isIDef() { return Flags & MVI_IDEFINE; }
  bool isDef()  { return Flags & MVI_DEFINE; }
};

class MVOperand {
public:
  enum {
    MVO_UNDEFINED  = 0x0,
    MVO_USE	   = 0x1,
    MVO_DEF	   = 0x2
  };
  uint32_t       Flags;       // bits are defined by the above enu.

  // OrigMVI is for MVIs of the original MO (before optimization),
  //   OrigMVI[0] : used if MO is either use or def, not both.
  //   OrigMVI[1] : used if MO is read first, then write.
  //                OrigMVI[1] is that write MVI.
  // NewMVI[2] corresponds to OrigMVI[2] after optimization.
  IntrusiveRefCntPtr<MVInfo> OrigMVI[2];
  IntrusiveRefCntPtr<MVInfo> NewMVI[2];

  MVOperand(MachineOperand *MO)
    : Flags (0)
  {
    OrigMVI[0] = OrigMVI[1] = 0;
    NewMVI[0]  = NewMVI[1]  = 0;
  }

  ~MVOperand() {}

  bool isUse()  { return Flags & MVO_USE; }
  void setUse() { Flags |= MVO_USE; }
  bool isDef()  { return Flags & MVO_DEF; }
  void setDef() { Flags |= MVO_DEF; }
  bool isDefOnly() { return isDef() && !isUse(); }
};

// Machine Instruction Info
class MIInfo {
public:
  MachineInstr *MI; // MI for which this info is

  std::vector<MVOperand*>  Operands;  // operands

  MIInfo(MachineInstr *aMI)
    : MI(aMI),
      Operands(),
      Next(NULL) {}

private:
  // link all MIInfo that has the same hashing value.
  // This is for value-based optimizations.
  MIInfo *Next;
};

class EBBOptimizer {
public:
  typedef std::list<IntrusiveRefCntPtr<MVInfo> > MVIListType;    // list of MVI
  typedef DenseMap<MachineBasicBlock *, bool> MBBVisitMapType;
  typedef DenseMap<MValue*, IntrusiveRefCntPtr<MVInfo> > MV2MVIMapType;
  typedef DenseMap<MachineInstr*, MIInfo*> MIInfoMapType;

private:
  const TargetMachine* TM;
  MachineFunction* MF;
  const TargetRegisterInfo *TRI;

  MValueManager MVManager;
  MV2MVIMapType MVInfoMap;
  MIInfoMapType MIInfoMap;
  MVIListType   UpwardExposedMVIs;  // All upward-exposed MVIs


  MBBVisitMapType MBBVisit;   // temp

  // CurEBBs[0] is the entry MBB for the current EBB
  std::vector<MachineBasicBlock*> CurEBBs;  // A sequence of MBBs

  bool appendMBB(MachineBasicBlock *MBB);

  void perform(MachineBasicBlock *MBB);

  bool EmitMBB(MachineBasicBlock *MBB);

  void removeMVIFromMap(IntrusiveRefCntPtr<MVInfo> aMVI);
  void eraseMVInfoMap();

  bool isMIUsed(MachineInstr *ValueMI,
                MachineInstr *StartMI, MachineInstr *EndMI);

  bool isSpecialPReg(unsigned PhysReg);
  bool isVEXTRACTInst(MachineInstr *MI);
  bool isLLOi64rInst(MachineInstr *MI);
  bool isVCREATEInst(MachineInstr *MI);
  bool isANDInst(MachineInstr *MI);
  bool isCopyInst(MachineInstr *MI);
  bool isSelfCopyInst(MachineInstr *MI);
  bool isShiftInst(MachineInstr *MI);
  void processCopy(MIInfo *MII);
  void processShift(MIInfo *MII);
  void mapRegAndSubreg(unsigned DstReg, unsigned SrcReg, IntrusiveRefCntPtr<MVInfo> SrcMVI);

  void replaceMO(MachineOperand *DstMO, MValue *SrcMV);

  MachineInstr *convertVEXTRACT2COPY(MachineInstr *MI);
  MachineInstr *convertLLOi64r2COPY(MachineInstr *MI);

  static const int SubRegIdx_sz;
  static const int SubRegIdx[6];

public:
  EBBOptimizer(const TargetMachine* aTM, MachineFunction* aMF) :
    TM(aTM),
    MF(aMF),
    MVManager(aTM) {
    TRI = aTM->getRegisterInfo();
  }

  bool isMVICurrent(IntrusiveRefCntPtr<MVInfo> MVI) { return (MVI == getMVI(MVI->MV)); }

  MIInfo * getMII(MachineInstr *MI) { return MIInfoMap[MI]; }

  // Return true if it performs any optimizations.
  bool run();

  // Get a hashing value for MIInfo
  // uint32_t HashValue(MIInfo* miinfo);

  void freeMIInfo(MachineInstr *MI, bool FromMIInfoMap=false);
  void freeMBBMIInfo(MachineBasicBlock *MBB);
  void freeMIInfoMap();

  // Get the current MVI associated with MV
  IntrusiveRefCntPtr<MVInfo> getMVI(MValue *MV) {
    MV2MVIMapType::iterator I = MVInfoMap.find(MV);
    return (I != MVInfoMap.end()) ? I->second : NULL;
  }

  void pushMVI(IntrusiveRefCntPtr<MVInfo> MVI) {
    MValue *MV = MVI->MV;
    MVI->Next = MVInfoMap[MV];
    MVInfoMap[MV] = MVI;
  }

  IntrusiveRefCntPtr<MVInfo> popMVI(MValue *aMV) {
    IntrusiveRefCntPtr<MVInfo> MVI = MVInfoMap[aMV];
    assert((MVI != NULL) && "MVI should be in the MVInfoMap");
    MVInfoMap[aMV] = MVI->Next.getPtr();
    return MVI;
  }

  void removeMVI(IntrusiveRefCntPtr<MVInfo> aMVI);
  IntrusiveRefCntPtr<MVInfo> popMVI(IntrusiveRefCntPtr<MVInfo> aMVI) { return popMVI(aMVI->MV); }

  IntrusiveRefCntPtr<MVInfo> createMVI(MValue *aMV, MachineInstr *aMI,
                    MachineOperand *aMO,
                    unsigned aFlags);

  void genMVIAsUseDef(MachineInstr *MI, MachineOperand *MO, MVOperand *MVO);
  MVOperand *createMVO(MachineInstr *MI, MachineOperand *MO);

  MIInfo *createMII(MachineInstr *aMI) {
    MIInfo *miinfo = new MIInfo(aMI);
    MIInfoMap[aMI] = miinfo;
    return miinfo;
  }

};

}

#endif
