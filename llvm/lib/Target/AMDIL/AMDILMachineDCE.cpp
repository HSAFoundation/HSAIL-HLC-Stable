//==------ AMDILMachineDCE.cpp : MachineFunction Dead Code Elimination ----===//
//
// Copyright (c) 2012, Advanced Micro Devices, Inc.
// All rights reserved.
//
//==-----------------------------------------------------------------------===//

//==-----------------------------------------------------------------------===//
// This file provides a dead code elimination for a machine function.  It uses
// data-flow information such as DU Chain to decide which instructions are
// dead.  The data-flow inforation is collected using an efficient iterative
// approach. This data-flow analysis can be made a standalone analysis pass
// if needed.
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "ebb"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Function.h"
#include "AMDILMachineValue.h"
#include "AMDILMachineDCE.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

#include <list>
#include <vector>

using namespace llvm;

static inline bool MIHasSideEffect(MachineInstr *MI)
{
  const MCInstrDesc &MCID = MI->getDesc();
  return (MI->hasUnmodeledSideEffects() ||
          MCID.isBarrier() || MCID.isCall() ||
          MCID.mayLoad() || MCID.mayStore());
}

static inline bool MIIsCFInstr(MachineInstr *MI)
{
  const MCInstrDesc &MCID = MI->getDesc();
  return (MCID.isCall() || MCID.isReturn() || MCID.isBarrier() ||
          MCID.isTerminator() || MCID.isBranch() ||
          MCID.isIndirectBranch());
}

namespace {

class MVRef;
typedef std::vector<MVRef *> ChainType;

class MVRef {
public:
  unsigned int isReg:1;
  unsigned int isDef:1;
  unsigned int isOverlapRef:1;

  MachineInstr *MI;
  int OperandIdx;
  MValue *MV;

  int def_id;   // temp used to compute UD/DU chain.
  MVRef *Next;  // temp used to link all refs to MV

  // For a physical register ref, OverlapMVRefs has all its overlapping MVRefs.
  // This field is set only if MI != NULL && !isOverlapRef.
  std::vector<MVRef *> OverlapMVRefs;

  // UD chain for use;  DU chain for def
  ChainType Chain;

  MVRef() :
    isReg(0), isDef(0), isOverlapRef(0),
    MI(NULL), OperandIdx(-1), MV(NULL),
    def_id(-1), Next(NULL),
    OverlapMVRefs(), Chain() {}

  ~MVRef() {
    for (unsigned int i = 0; i < OverlapMVRefs.size(); ++i) {
      MVRef *tMVR = OverlapMVRefs[i];
      delete tMVR;
    }
    OverlapMVRefs.clear();
    Chain.clear();
  }

  bool DUChainIsEmpty() {
    assert(isDef && "Use reference does not have a DU Chain!");
    if (Chain.size() > 0) {
      return false;
    }
    for (unsigned i=0; i < OverlapMVRefs.size(); ++i) {
      MVRef *tMVR = OverlapMVRefs[i];
      if (tMVR->Chain.size() > 0) {
        return false;
      }
    }
    return true;
  }
};

class MIUseDefInfo {
public:
  std::vector<MVRef*> Operands;
};

// MachineFunction DCE
class MFDCE {
public:
  typedef DenseMap<MachineInstr *, MIUseDefInfo *> MIUseDefMapType;
  typedef std::list<MVRef *> MVRefListType;
  typedef DenseMap<MValue *, MVRef *>  MV2MVRefMapType;
  typedef DenseMap<MachineBasicBlock *, MVRefListType> MBBMVRefType;
  typedef DenseMap<MachineBasicBlock *, BitVector> MBBBitVectorType;

private:
  const TargetMachine* TM;
  const TargetRegisterInfo *TRI;
  MachineFunction* MF;
  MValueManager MVManager;
  MIUseDefMapType MIUseDefs;
  MV2MVRefMapType DefForUEs;  // Dummy MVRef for UEs for this function.

  // Tempoaries used during computing UD/DU chains.
  MV2MVRefMapType CurrentMVDefs;
  MBBMVRefType MBBDefs;
  MBBMVRefType MBBUEs;


  // Create MVRef for a register reference.
  MVRef* createRegMVRef(MValue *aMV, MachineInstr *aMI, int Idx,
                        int isOverlapMVRef);

  // Calculate Use & Def info for each MBB
  void calculateLocalUseDefInfo();

  // Compute UD/DU chains.
  void calculateUseDefChain();

  // Update UD/DU info if MI is deleted.
  void updateAfterDelete(MachineInstr *MI);

  // free memory
  void clear();

public:
  MFDCE (const TargetMachine* aTM, const TargetRegisterInfo *theTRI,
         MachineFunction *aMF) :
    TM(aTM), TRI(theTRI), MF(aMF),
    MVManager(aTM), MIUseDefs(), DefForUEs(),
    CurrentMVDefs(), MBBDefs(), MBBUEs()
  {}

  ~MFDCE() {
    clear();
  }

  void updateForDelete(MachineInstr *MI);

  bool run();
};

} // namespace

ChainType::iterator chain_find(ChainType& Chain,  MVRef* aMVR)
{
  for (unsigned int i = 0; i < Chain.size(); ++i) {
    MVRef *MVR = Chain[i];
    if (MVR == aMVR) {
      return Chain.begin() + i;
    }
  }
  return Chain.end();
}

void MFDCE::clear()
{
  for (MV2MVRefMapType::iterator I = DefForUEs.begin(), E = DefForUEs.end();
       I != E; ++I) {
    MVRef *MVR = I->second;
    delete MVR;
  }
  DefForUEs.clear();

  for (MIUseDefMapType::iterator I = MIUseDefs.begin(), E = MIUseDefs.end();
       I != E; ++I) {
    MIUseDefInfo *udinfo = I->second;
    assert(udinfo && "MIUseDefs should not have empty entry");
    for (int i = 0; i < (int)udinfo->Operands.size(); ++i) {
      MVRef *MVR = udinfo->Operands[i];
      delete MVR;
    }
    delete udinfo;
  }
  MIUseDefs.clear();
}

MVRef* MFDCE::createRegMVRef(MValue *aMV, MachineInstr *aMI, int Idx,
                             int isOverlapMVRef)
{
  MachineOperand *MO = &(aMI->getOperand(Idx));
  assert(MO->isReg() && "MO should be a register MO");

  MVRef *MVR = new MVRef();
  MVR->isReg = 1;
  MVR->isDef = (MO->isDef() ? 1 : 0);
  MVR->isOverlapRef = isOverlapMVRef;
  MVR->OperandIdx = Idx;
  MVR->MI = aMI;
  MVR->MV = aMV;

  // Skip OverlapMVRefs if this MVRef is itself an overlapped MVRef.
  if (isOverlapMVRef) {
    return MVR;
  }

  unsigned Reg = MO->getReg();
  if (TargetRegisterInfo::isPhysicalRegister(Reg)) {
   // For DU/UD purpose,  using subRegisters is enough. No need to use
   // overlapRegisters.
   //for (const uint16_t *RO = TRI->getSubRegisters(Reg); *RO; ++RO) {
   MCSubRegIterator SubReg(Reg, TRI);
   for (; SubReg.isValid(); ++SubReg) {
      MValue *MV = MVManager.getOrInsertMValue(*SubReg);
      MVRef *tMVR = createRegMVRef(MV, aMI, Idx, 1);
      MVR->OverlapMVRefs.push_back(tMVR);
    }
  }
  return MVR;
}

void MFDCE::calculateLocalUseDefInfo()
{
  for (MachineFunction::iterator II = MF->begin(), IE = MF->end();
       II != IE; ++II) {
    MachineBasicBlock *MBB = II;
    CurrentMVDefs.clear();

    for (MachineBasicBlock::iterator I = MBB->begin(), E = MBB->end();
         I != E; ++I) {
      MachineInstr *MI = I;
      MIUseDefInfo *miinfo = new MIUseDefInfo();
      MIUseDefs[MI] = miinfo;
      // process use operands before defs
      // Operands are not always in the "def" then "use" order (eg. call insts).
      // So we have to do it in two loops.
      for (int ix = 0, n = (int)MI->getNumOperands(); ix < n; ++ix) {
        MachineOperand *MO = &(MI->getOperand(ix));
        MVRef *MVR = NULL;
        MValue *MV = NULL;
        if (!MO->isReg()) {
          // Dummy MVRef() for non-reg operand.
          MVR = new MVRef();
          MVR->MI = MI;
          MVR->OperandIdx = ix;
        } else {
          MV = MVManager.getOrInsertMValue(MO);
          MVR = createRegMVRef(MV, MI, ix, 0);
        }
        miinfo->Operands.push_back(MVR);
        if (!MO->isReg() || MO->isDef()) continue;

        MV2MVRefMapType::iterator I = CurrentMVDefs.find(MV);
        if (I == CurrentMVDefs.end()) {
          // Upward exposed uses
          MBBUEs[MBB].push_back(MVR);
        } else {
          // UD/DU for a local use.
          MVRef *defMVR = I->second;
          MVR->Chain.push_back(defMVR);
          defMVR->Chain.push_back(MVR);
        }

        for (int i=0; i < (int)MVR->OverlapMVRefs.size(); ++i) {
          MVRef *tMVR = MVR->OverlapMVRefs[i];
          MValue *tMV = tMVR->MV;
          I = CurrentMVDefs.find(tMV);
          if (I == CurrentMVDefs.end()) {
            MBBUEs[MBB].push_back(tMVR);
          } else {
            MVRef *def = I->second;
            tMVR->Chain.push_back(def);
            def->Chain.push_back(tMVR);
          }
        }
      }

      for (int ix = 0, n = (int)MI->getNumOperands(); ix < n; ++ix) {
        MachineOperand *MO = &(MI->getOperand(ix));
        if (!MO->isReg() || !MO->isDef()) continue;
        MVRef *MVR = miinfo->Operands[ix];
        MValue *MV = MVR->MV;
        MVR->Next = CurrentMVDefs[MV];
        CurrentMVDefs[MV] = MVR;
        for (int i=0; i < (int)MVR->OverlapMVRefs.size(); ++i) {
          MVRef *tMVR = MVR->OverlapMVRefs[i];
          MValue *tMV = tMVR->MV;
          tMVR->Next = CurrentMVDefs[tMV];
          CurrentMVDefs[tMV] = tMVR;
        }
      }
    }

    // Get all global defs that are live at the end of this MBB.
    for (MV2MVRefMapType::iterator I = CurrentMVDefs.begin(),
                                   E = CurrentMVDefs.end();
         I != E; ++I) {
      MVRef *defMVR = I->second;
      MBBDefs[MBB].push_back(defMVR);
    }
  }
  CurrentMVDefs.clear();
}

void MFDCE::calculateUseDefChain()
{
  std::vector<MVRef*> AllGlobalDefs; // index ---> MVR
  MV2MVRefMapType MVDefs;            // MV --> its MVRefs

  calculateLocalUseDefInfo();

//#undef DEBUG_TYPE
//#define DEBUG_TYPE "ducomp"
  // Collect all global defs
  int bitsize = 0;
  for (MachineFunction::iterator MII = MF->begin(), MIE = MF->end();
       MII != MIE; ++MII) {
    MachineBasicBlock *MBB = MII;
    for (MVRefListType::iterator I = MBBDefs[MBB].begin(),
                                 E = MBBDefs[MBB].end();
         I != E; ++I, ++bitsize) {
      MVRef *defMVR = *I;
      defMVR->def_id = bitsize;
      AllGlobalDefs.push_back(defMVR);

      MValue *MV = defMVR->MV;
      defMVR->Next = MVDefs[MV];
      MVDefs[MV] = defMVR;
    }
  }

  // Initialize Kills, Gens, and LiveIns;
  MBBBitVectorType Kills, Gens, LiveIns;
  for (MachineFunction::iterator MII = MF->begin(), MIE = MF->end();
       MII != MIE; ++MII) {
    MachineBasicBlock *MBB = MII;
    Kills[MBB].resize(bitsize);
    Gens[MBB].resize(bitsize);
    for (MVRefListType::iterator I = MBBDefs[MBB].begin(),
                                 E = MBBDefs[MBB].end();
         I != E; ++I, ++bitsize) {
      MVRef *defMVR = *I;
      Gens[MBB].set(defMVR->def_id);
      MValue *MV = defMVR->MV;
      for (MVRef *tMVR = MVDefs[MV]; tMVR != NULL; tMVR = tMVR->Next) {
        Kills[MBB].set(tMVR->def_id);
      }
    }
  }

  for (MachineFunction::iterator MII = MF->begin(), MIE = MF->end();
       MII != MIE; ++MII) {
    MachineBasicBlock *MBB = MII;
    LiveIns[MBB].resize(bitsize);
    for (MachineBasicBlock::pred_iterator PI = MBB->pred_begin(),
                                          PE = MBB->pred_end();
         PI != PE;  ++PI) {
      MachineBasicBlock *PredMBB = *PI;

      // After this, Gens is no longer needed.
      LiveIns[MBB] |= Gens[PredMBB];
    }
  }

  // Use reverse postorder to traverse CFG so that it will reach
  // the fixed point sooner.
  MachineBasicBlock *EntryMBB = MF->begin();
  ReversePostOrderTraversal<MachineBasicBlock*> RPOT(EntryMBB);
  bool changed = true;
  while (changed) {
    changed = false;
    for (ReversePostOrderTraversal<MachineBasicBlock*>::rpo_iterator
         MBBI = RPOT.begin(), MBBE = RPOT.end(); MBBI != MBBE; ++MBBI) {
      MachineBasicBlock *MBB = *MBBI;
      BitVector newLiveIns = LiveIns[MBB];
      for (MachineBasicBlock::pred_iterator PI = MBB->pred_begin(),
                                            PE = MBB->pred_end();
           PI != PE;  ++PI) {
        MachineBasicBlock *PredMBB = *PI;

        BitVector tbv(LiveIns[PredMBB]);

        // tbv &= ~Kills[PredMBB];
        // No need to do "tbv |= Gens[PredMBB]"
        tbv.reset(Kills[PredMBB]);
        newLiveIns |= tbv;
      }
      if (newLiveIns != LiveIns[MBB]) {
        changed = true;
        LiveIns[MBB] = newLiveIns;
      }
    }
  }

  // Set up UD/DU chains
  for (MachineFunction::iterator MII = MF->begin(), MIE = MF->end();
       MII != MIE; ++MII) {
    MachineBasicBlock *MBB = MII;
    MVRefListType& UEs = MBBUEs[MBB];
    for (MVRefListType::iterator I = UEs.begin(), E = UEs.end();
         I != E; ++I) {
      MVRef *useMVR = *I;
      assert (!useMVR->isDef && "MVRef should be a use reference");
      MValue *MV = useMVR->MV;
      bool hasDefined = false;
      for (MVRef *defMVR = MVDefs[MV]; defMVR != NULL; defMVR = defMVR->Next) {
        if (LiveIns[MBB].test(defMVR->def_id)) {
          hasDefined = true;
          useMVR->Chain.push_back(defMVR);
          defMVR->Chain.push_back(useMVR);
        }
      }
      if (!hasDefined) {
        MVRef *dummyDefMVR = DefForUEs[MV];
        if (dummyDefMVR == NULL) {
          // set up a dummy def MVR
          dummyDefMVR = new MVRef();
          dummyDefMVR->isReg = 1;
          dummyDefMVR->isDef = 1;
          dummyDefMVR->MV = MV;
          DefForUEs[MV] = dummyDefMVR;
        }
        useMVR->Chain.push_back(dummyDefMVR);
        dummyDefMVR->Chain.push_back(useMVR);
      }
    }
  }

#ifndef NDEBUG
#undef DEBUG_TYPE
#define DEBUG_TYPE "udchain"

  typedef DenseMap<MachineInstr *, int> InstrIDMapType;
  InstrIDMapType InstrIDs;
  int instrid = -1;
  for (MachineFunction::iterator MII = MF->begin(), MIE = MF->end();
       MII != MIE; ++MII) {
    MachineBasicBlock *MBB = MII;
    for (MachineBasicBlock::iterator I = MBB->begin(), E = MBB->end();
         I != E; ++I) {
      MachineInstr *MI = I;
      MIUseDefInfo *udinfo = MIUseDefs[MI];
      assert(udinfo && " UseDefInfo is missing");
      InstrIDs[MI] = ++instrid;
    }
  }

  for (MachineFunction::iterator MII = MF->begin(), MIE = MF->end();
       MII != MIE; ++MII) {
    MachineBasicBlock *MBB = MII;
    DEBUG(dbgs() << "BB#" << MBB->getNumber() << ": Preds: ");
    for (MachineBasicBlock::pred_iterator PI = MBB->pred_begin(),
                                          PE = MBB->pred_end();
         PI != PE; ++PI) {
      DEBUG(dbgs() << " BB#" << (*PI)->getNumber());
    }
    DEBUG(dbgs() << '\n');

    for (MachineBasicBlock::iterator I = MBB->begin(), E = MBB->end();
         I != E; ++I) {
      MachineInstr *MI = I;
      MIUseDefInfo *udinfo = MIUseDefs[MI];
      instrid = InstrIDs[MI];
      DEBUG(dbgs() << instrid << " : " << *MI);
      for (unsigned n = 0; n < udinfo->Operands.size(); ++n) {
        MVRef *MVR = udinfo->Operands[n];
        if (!MVR->isReg) {
          continue;
        }

        DEBUG(dbgs() << (MVR->isDef ? "  DU Chain " : "  UD Chain ")
                     << *(MVR->MV)
                     << "(" << n << "): ");

        int nPerLine;
        for (int i = -1; i < (int)MVR->OverlapMVRefs.size(); ++i)
        {
          MVRef *currMVR = (i == -1) ? MVR : MVR->OverlapMVRefs[i];
          if (i >= 0)
            DEBUG(dbgs() << "\n    (overlap " << *currMVR->MV << ") ");
          nPerLine = -1;
          unsigned sz = currMVR->Chain.size();
          for (unsigned j = 0; j < sz; ++j) {
            ++nPerLine;
            if (nPerLine == 5) {
              // 5 items per line
              DEBUG(dbgs() << "\n      ");
              nPerLine = -1;
            }
            MVRef *tMVR = currMVR->Chain[j];
            instrid = InstrIDs[tMVR->MI];
            DEBUG(dbgs() << *tMVR->MV
                         << "(" << instrid
                         << ", " << tMVR->OperandIdx
                         <<") ");
          }
        }
        DEBUG(dbgs() <<"\n");
      }
    }

    DEBUG(dbgs() << "     Succs: ");
    for (MachineBasicBlock::succ_iterator SI = MBB->succ_begin(),
                                          SE = MBB->succ_end();
         SI != SE; ++SI)
      DEBUG(dbgs() << " BB#" << (*SI)->getNumber());
    DEBUG(dbgs() << '\n');
  }

#undef DEBUG_TYPE
#define DEBUG_TYPE "ebb"
#endif

  CurrentMVDefs.clear();
  MBBDefs.clear();
  MBBUEs.clear();
}

void MFDCE::updateAfterDelete(MachineInstr *MI)
{
  MIUseDefInfo *udinfo = MIUseDefs[MI];
  assert (udinfo && "UseDefInfo for MI is missing");

  for (unsigned n = 0; n < udinfo->Operands.size(); ++n) {
    MVRef *MVR = udinfo->Operands[n];
    if (MVR->isDef) {
#ifndef NDEBUG
      assert((MVR->Chain.size() == 0) &&
             "DU chain for a dead MI should be empty");

      for (unsigned i = 0; i < MVR->OverlapMVRefs.size(); ++i) {
        MVRef *tMVR = MVR->OverlapMVRefs[i];
        assert((tMVR->Chain.size() == 0) &&
               "DU chain for overlapped ref of a dead MI should be empty");
      }
#endif
    }
    else {
      for (unsigned m = 0; m < MVR->Chain.size(); ++m) {
        MVRef *defMVR = MVR->Chain[m];
        std::vector<MVRef*>::iterator I = chain_find(defMVR->Chain, MVR);
        assert((I != defMVR->Chain.end()) &&
               "A def 'D' in UD(U), but 'U' is not in DU(D)");
        defMVR->Chain.erase(I);
      }

      for (unsigned i = 0; i < MVR->OverlapMVRefs.size(); ++i) {
        MVRef *tMVR = MVR->OverlapMVRefs[i];
        for (unsigned k = 0; k < tMVR->Chain.size(); ++k) {
          MVRef *def = tMVR->Chain[k];
          std::vector<MVRef*>::iterator I = chain_find(def->Chain, tMVR);
          assert((I != def->Chain.end()) &&
               "A def 'D' in UD(U), but 'U' is not in DU(D) (overlapped ref)");
          def->Chain.erase(I);
        }
      }
    }

    delete MVR;
  }

  delete udinfo;

  MIUseDefs.erase(MI);
}

bool MFDCE::run()
{
  DEBUG(dbgs() << "\n<EBB> BEGIN -- MachineFunction DCE : function "
               << MF->getFunction()->getName() << "\n\n");

  const TargetRegisterInfo *TRI = MF->getTarget().getRegisterInfo();
  BitVector ReservedRegs = TRI->getReservedRegs(*MF);
  int nInstrDeleted = 0;

  bool ret = false;
  calculateUseDefChain();

  std::vector<MachineInstr *> ToBeDeleted;
  bool changed = false;
  do {
    for(MachineFunction::reverse_iterator RI = MF->rbegin(),
                                          RE = MF->rend();
        RI != RE; ++RI) {
      MachineBasicBlock *MBB = &*RI;

      for (MachineBasicBlock::reverse_iterator II = MBB->rbegin(),
                                               IE = MBB->rend();
           II != IE; ++II)
      {
        MachineInstr *MI = &*II;
        if (MI->isDebugValue() || MIHasSideEffect(MI) || MIIsCFInstr(MI)) {
          DEBUG(dbgs()
            << "<EBB> " << *MI
            <<  "     not checked due to CF/sideeffect/debug instr\n");
          continue;
        }

        bool isDead = true;
        MIUseDefInfo *udinfo = MIUseDefs[MI];
        for (unsigned int n = 0; n < udinfo->Operands.size(); ++n) {
          MVRef *MVR = udinfo->Operands[n];
          if (MVR->isDef &&
              (!MVR->DUChainIsEmpty() ||
               // Don't delete defs of any reserved registers.
               (MVR->isReg && ReservedRegs.test(MVR->MV->getReg())))) {
            isDead = false;
            break;
          }
        }

        DEBUG(dbgs() << "<EBB> " << *MI);
        if (isDead) {
          DEBUG(dbgs() << "<EBB>     deleted!\n");
          updateAfterDelete(MI);
          ToBeDeleted.push_back(MI);
        }
      }
    }

    for (int i = 0; i < (int)ToBeDeleted.size(); ++i) {
      MachineInstr *MI = ToBeDeleted[i];
      MI->eraseFromParent();
    }
    changed = (ToBeDeleted.size() > 0);
    if (changed) {
      ret = true;
    }

    nInstrDeleted += ToBeDeleted.size();

    ToBeDeleted.clear();
  } while (changed);

  DEBUG(dbgs() << "<EBB> END -- MachineFunction DCE : "
               << nInstrDeleted << " Machine Instructions deleted\n\n");
  return ret;
}

bool llvm::doMachineFunctionDCE(const TargetMachine* aTM, MachineFunction *aMF)
{
  const TargetRegisterInfo *aTRI = aTM->getRegisterInfo();
  MFDCE aMFDCE(aTM, aTRI, aMF);
  return aMFDCE.run();
}
