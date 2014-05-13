//
//
// Copyright (c) 2011 Advanced Micro Devices, Inc. All rights reserved.
//

//
// This file contains the declaration of the Live Range class. 
//

#ifndef LLVM_AMDLIVEANALYSIS_H
#define LLVM_AMDLIVEANALYSIS_H

#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/Support/AlignOf.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"
#include "llvm/Function.h"
#include "llvm/Pass.h"
#include "llvm/User.h"
#include "llvm/Value.h"

#include <vector>

namespace llvm {

class BasicBlock;
class raw_ostream;

/*
  Used for computing live ranges.  Note that the first bit (bit 0) is reserved and
  valid bit starts from 1.

  TODO: Once computed, the memory for them is no longer used 
        and will be freed.
*/
class BasicBlockLiveInfo {
public:
  BitVector Ins;
  BitVector Outs;
  BitVector Defs;
  BitVector Uses;

  BasicBlockLiveInfo(unsigned BitVectorSize)
    : Ins (BitVectorSize),
      Outs(BitVectorSize),
      Defs(BitVectorSize),
      Uses(BitVectorSize)
  {}
};

class LiveRange {
public:
  // LiveBBVector[0] must be the definiting BB for this value.
  // Use pointer & int pair to encode live-in/out, used/defined
  // information.
  //    LiveBBVector[0] : defining BB, (not live-in, NOT TRUE !)
  //    LiveBBVector[1 : size()-1] : live-in
  //          LR_VALUE_USED : used
  //          LR_VALUE_LIVEOUT : live out
  //       
  // Note:  as 8/29/2011, int part of PointerIntPair is not used yet.
  //        (2-bit int isn't enough to save all needed info!)
  enum {
    LR_VALUE_UNUSED  = 0,
    LR_VALUE_USED    = 0x1,
    LR_VALUE_LIVEOUT = 0x2
  };
  typedef std::vector<PointerIntPair<BasicBlock*, 2> > LiveBBVector;  

  LiveRange (Value* Val, BasicBlock* DefiningBB)
    : V (Val),
      DefBB (DefiningBB)
  {
    // Make sure Definining BB appears first in this vector
    LiveBBs.insert(LiveBBs.begin(), 
      PointerIntPair<BasicBlock*, 2>(DefiningBB, LR_VALUE_UNUSED));
  }

  ~LiveRange() {}

  bool isGlobalLiveRange() {
    return (LiveBBs[0].getInt() & LR_VALUE_LIVEOUT);
  }

  void addBB(BasicBlock *BB, unsigned int attr) {
  
    if (BB == DefBB) {
      LiveBBs[0].setInt(attr);
    }
    else {
      LiveBBs.push_back(PointerIntPair<BasicBlock*, 2>(BB, attr));
    }
  }

  int size() { return (int)LiveBBs.size(); }

  void dump(raw_ostream *O = NULL, unsigned int indent=2);

private:
  Value *V;
  BasicBlock *DefBB;
  LiveBBVector  LiveBBs;

};

class LivenessAnalysisBase {
public:

  typedef DenseMap<Value*, int> ValueEncodingMap;   // Value --> int
  typedef DenseMap<int, Value*> EncodingValueMap;   // int --> Value

  typedef DenseMap<Value*, LiveRange*> ValueLiveRangeMap;
  typedef DenseMap<BasicBlock*, BasicBlockLiveInfo*> BBLiveInfoMap;
  typedef DenseMap<BasicBlock*, PointerIntPair<LiveRange*, 2> > BBLiveRangeMap;
  typedef DenseMap<std::pair<BasicBlock*, BasicBlock*>, BitVector*> PhiMaskMap;
  typedef DenseMap<BasicBlock *, int>  BB2IntMap;


  LivenessAnalysisBase(Function& Func, unsigned int threshold)
    : F(Func),
      NumValues(0),
      MinThreshold(threshold),
      ValueEncoding(NULL),
      EncodingValue(NULL),
      ValueLiveRanges(NULL),
      BBLiveInfo(NULL),
      PhiMasks(NULL),
      DummyEntryBB(NULL),
      DummyExitBB(NULL),
      Visited()
  {
    assert ((alignOf<char*>() >= 4) &&
            "Liveness Analysis requires a pointer to be aligned at least 4 bytes");
    DummyEntryBB = BasicBlock::Create(Func.getContext(), "DummyEntryBB");
    DummyExitBB  = BasicBlock::Create(Func.getContext(), "DummyExitBB");
  }

  ~LivenessAnalysisBase() {
    releaseMemory();
    delete DummyEntryBB;
    delete DummyExitBB;
  }

  // Return true if liveness computation has been done successfully.
  bool calculate();

  void releaseMemory();

  // Liveness Analysis considers variables that are potentially in registers.
  // So, it only needs to consider values that are either arguments or  Instructions
  // that have non-empty uses.
  bool isCandidateValue(Value *V) {
    if (dyn_cast<Argument>(V)) {
      return true;
    }
    else if (Instruction *Inst = dyn_cast<Instruction>(V)) {
      if (!Inst->use_empty()) {
        return true;
      }
    }
    return false;
  }

  // Return the Number of Live Ranges (NLR) right into BB
  unsigned getNLRBBIn(BasicBlock *BB) {
    BasicBlockLiveInfo *BBLI = getBBLiveInfo(BB);
    return BBLI->Ins.count();
  }

  // Return the Number of Live Ranges (NLR) right out of BB
  unsigned getNLRBBOut(BasicBlock *BB) {
    BasicBlockLiveInfo *BBLI = getBBLiveInfo(BB);
    return BBLI->Outs.count();
  }

  unsigned getNumAvailRegs() { return MinThreshold; }

  bool isValueInBBOut(Value *V, BasicBlock *BB) {
    BasicBlockLiveInfo *BBLI = getBBLiveInfo(BB);
    unsigned vix = getValueEncoding(V);
    return BBLI->Outs[vix];
  }

  // Given a BB, return the number of registers that is needed for
  // this BB without spilling.  If Fatpoint is not NULL, it will 
  // return the fatpoint.
  unsigned getBBNLRAtFatPoint(BasicBlock *BB, Instruction **Fatpoint);

  // Return the number of register needed for this Function and
  // if Fatpoint is not NULL, return the fatpoint.
  unsigned getNLRAtFatPoint(Instruction **Fatpoint);

private:
  Function &F;
  unsigned int NumValues;              // the number of values
  unsigned int MinThreshold;           // Do liveness if NumValues > MinThreshold
  ValueEncodingMap   *ValueEncoding;   // Uniquely numbering values.
  EncodingValueMap   *EncodingValue;   // Given an int, find its value, this is
                                       // the reverse mapping of ValueEncoding.

  ValueLiveRangeMap  *ValueLiveRanges; // Live ranges for each value.
  BBLiveInfoMap      *BBLiveInfo;      // Liveness for each BB
  PhiMaskMap         *PhiMasks;        // Mask bitvectors for BBs with Phi nodes. 

  // If BBLiveInfo is temporary,  we need the following BBLiveRanges
  // BBLiveRangeMap    *BBLiveRanges;    // Live ranges per BB

  // Pseudo BBs that are used for any value that is live across the 
  // boundary of this function, such as live into the function for 
  // incoming parameters and live out of the function for outgoing
  // parameters and return values.
  // So far, just a placeholder. 
  BasicBlock *DummyEntryBB;
  BasicBlock *DummyExitBB;

  BB2IntMap  Visited;

  unsigned getValueEncoding(Value *Val, bool Check = true) {
    ValueEncodingMap::iterator VI = ValueEncoding->find(Val);
    if (VI == ValueEncoding->end()) {
      if (Check) {
        assert(false && "Argument Value not in Value Encoding Map!");
      }
      return 0;
    }
    unsigned vix = VI->second;
    return vix;
  }

  Value* getEncodingValue(int Encoding, bool Check = true) {
    EncodingValueMap::iterator EI = EncodingValue->find(Encoding);
    if (EI == EncodingValue->end()) {
      if (Check) {
        assert(false && "Argument ID (int) not in Encoding Value Map!");
      }
      return NULL;
    }
     
    Value *Val = EI->second;
    return Val;
  }

  BasicBlockLiveInfo *getBBLiveInfo(BasicBlock* BB) {
    BBLiveInfoMap::iterator BBLII = BBLiveInfo->find(BB);
    assert(( BBLII != BBLiveInfo->end()) &&
             "BB LiveInfo not set up yet in BBLiveInfo Map!");
    BasicBlockLiveInfo *BBLI = BBLII->second;
    return BBLI;
  }

  void initValueEncoding();

  void initPhiMasks();

  void initLocalInfo();

  bool CFGITraversal(BasicBlock *BB);

public:
  // Debugging utilities
  void dump();
  void dump(BasicBlock *BB);
  void dump(BitVector *BV, int indent = 6);
  void dumpLR();
};


// Currently, there is no intend to update liveness information after
// each transformation.  Thus, Liveness analysis is made demand-driven.
// And we make it ImmutablePass for this purpsoe.
class LivenessAnalysis : public ImmutablePass {
public:
  static char ID; // Pass ID, replacement for typeid
  LivenessAnalysisBase* LAB;

  LivenessAnalysis() :
    ImmutablePass(ID),
    LAB(NULL),
    LivenessValid(false) {
    initializeLivenessAnalysisPass(*PassRegistry::getPassRegistry());
  }

  ~LivenessAnalysis() {
    if (LAB) {
      delete LAB;
    }
  }

  LivenessAnalysisBase* getBase() { return LAB; }

  void releaseMemory() {
    if (LAB) {
      delete LAB;
      LAB = NULL;
      LivenessValid = false;
    }
  }

  void calculateLiveness (Function& F);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
  }

  bool isLivenessValid() { return LivenessValid; }

private:
  bool LivenessValid;
};

} // namespace llvm

#endif
