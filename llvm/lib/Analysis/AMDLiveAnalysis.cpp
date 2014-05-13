//
// Copyright (c) 2011 Advanced Micro Devices, Inc. All rights reserved.
//

#define DEBUG_TYPE "liveness"
#include "llvm/Support/Debug.h"
#include "llvm/LLVMContext.h"
#include "llvm/Analysis/AMDLiveAnalysis.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/AMDLLVMContextHook.h"

#include <string>

using namespace llvm;

namespace {

void getValueName(const Module *M, const Value *Val, std::string& VName)
{
  std::string SBuf;
  raw_string_ostream OB(SBuf);
  WriteAsOperand(OB, Val, 0, M);
  VName = OB.str();
}

} // namespace

void LiveRange::dump(raw_ostream *OP, unsigned int indent)
{
  if (OP == NULL) {
    OP = &(dbgs());
  }

  Module *M;
  if (Instruction *InstVal = dyn_cast<Instruction>(V)) {
    M = InstVal->getParent()->getParent()->getParent();
  }
  else if (Argument *ArgVal = dyn_cast<Argument>(V)) {
    M = ArgVal->getParent()->getParent();
  }
  else {
    assert(0 && "Invalid value as liveness's candidate");
    return;
  }
  std::string namestr;
  getValueName(M, V, namestr);
  OP->indent(indent);
  *OP << "LR (" << namestr.c_str() << ") : ";
  int nbytes = indent + 8 + namestr.size();
  bool isFirst = true;
  char ending = '\n';
  for (int i=0, sz = (int)LiveBBs.size(); i < sz; ++i) {
    BasicBlock *BB = LiveBBs[i].getPointer();
    getValueName(M, (Value*)BB, namestr);
    *OP << (isFirst ? "BB " : ", BB ") << namestr.c_str();
    isFirst = false;
    nbytes += namestr.size() + 5;
    if ((nbytes > 70) && (i != (sz-1))) {
      nbytes = indent + 2;
      *OP << "\n";
      OP->indent(indent + 2);
      isFirst = true;
      ending = ' ';
    }
    else {
      ending = '\n';
    }
  }
  *OP << ending;
}

void
LivenessAnalysisBase::initValueEncoding()
{
  ValueEncoding = new ValueEncodingMap(64);
  EncodingValue = new EncodingValueMap(64);

  // Starts from 1. (0 is reserved.)
  unsigned int ix = 1;
  Value *Val;
  for (Function::arg_iterator AI = F.arg_begin(), AE = F.arg_end();
       AI != AE; ++AI) {
    Val = &*AI;
    if (isCandidateValue(Val)) {
      ValueEncoding->insert(std::make_pair(Val, ix));
      EncodingValue->insert(std::make_pair(ix, Val));
      ++ix;
    }
  }

  for (inst_iterator II = inst_begin(F), IE = inst_end(F); II != IE; ++II) {
    Instruction *Inst = &*II;
    if (isCandidateValue(Inst)) {
      Val = Inst;
      ValueEncoding->insert(std::make_pair(Val, ix));
      EncodingValue->insert(std::make_pair(ix, Val));
      ++ix;
    }
  }
  NumValues = ix;
}

void
LivenessAnalysisBase::initPhiMasks()
{
  PhiMasks = new PhiMaskMap(16);

  for (Function::iterator IT = F.begin(), IE = F.end(); IT != IE; ++IT) {
    BasicBlock *BB = IT;
    for (BasicBlock::iterator II = BB->begin(), IE = BB->end();
         II != IE;
         ++II) {
      Instruction *Inst = II;
      if (PHINode *PhiInst = dyn_cast<PHINode>(Inst)) {
        for (unsigned ix = 0, ix_end = PhiInst->getNumIncomingValues();
             ix < ix_end;
             ++ix) {
          BasicBlock *PredBB = PhiInst->getIncomingBlock(ix);
          std::pair<BasicBlock*, BasicBlock*> key = std::make_pair(BB, PredBB);
          PhiMaskMap::iterator MI = PhiMasks->find(key);
          BitVector *BVec;
          if (MI == PhiMasks->end()) {
            // First time, create a bitvector with all 1's
            BVec = new BitVector(NumValues, true);
            PhiMasks->insert(std::make_pair(key, BVec));
          }
          else {
            BVec = MI->second;
          }

          // For this mask bit vector, mask off all the other values
          // since they are not live in this PredBB.
          for (unsigned ix1 = 0; ix1 < ix_end; ++ix1) {
            if (ix1 == ix) {
              continue;
            }
            Value *OtherVal = PhiInst->getIncomingValue(ix1);
            if (Instruction *OtherInst = dyn_cast<Instruction>(OtherVal)) {
              // Only instruction needs to be considered
              unsigned vix = getValueEncoding(OtherInst);
              BVec->reset(vix);
            }
          }
        }
      }
    }
  }
}

void
LivenessAnalysisBase::initLocalInfo()
{
  ValueLiveRanges = new ValueLiveRangeMap(64);
  BBLiveInfo = new BBLiveInfoMap(64);

  // Create liveInfo structure. 
  for (Function::iterator IT = F.begin(), IE = F.end(); IT != IE; ++IT) {
    BasicBlock *BB = IT;
    BasicBlockLiveInfo *BBLI = new BasicBlockLiveInfo(NumValues);
    BBLiveInfo->insert(std::make_pair(BB, BBLI));
  }
  // Generate BBLiveInfo for DummyEntryBB and DummyExitBB.
  BasicBlockLiveInfo *DummyEntryLI = new BasicBlockLiveInfo(NumValues);
  BasicBlockLiveInfo *DummyExitLI = new BasicBlockLiveInfo(NumValues);
  BBLiveInfo->insert(std::make_pair(DummyEntryBB, DummyEntryLI));
  BBLiveInfo->insert(std::make_pair(DummyExitBB, DummyExitLI));

  // *** Init all basic block live info and all live ranges
  //   1.  Incoming arguments
  //   2.  Outgoing return values
  //   3.  Normal values defined/used within this function

  // Incoming arguments
  for (Function::arg_iterator AI = F.arg_begin(), AE = F.arg_end();
         AI != AE; ++AI) {
    Value *ArgVal = AI;
    LiveRange *LR = new LiveRange(ArgVal, DummyEntryBB);
    ValueLiveRanges->insert(std::make_pair(ArgVal, LR));

    unsigned vix = getValueEncoding(ArgVal);

    DummyEntryLI->Defs.set(vix);

    for (Value::use_iterator UI=ArgVal->use_begin(), 
                             UE=ArgVal->use_end();
         UI != UE;
         ++UI) {
      if (Instruction *Inst = dyn_cast<Instruction>(*UI)) {
        BasicBlock *UseBB = Inst->getParent();
        BasicBlockLiveInfo *BBLI = getBBLiveInfo(UseBB);
        BBLI->Uses.set(vix);

        // It is a upward exposed use, let's set Ins as well.
        BBLI->Ins.set(vix);

        // Outgoing return value
        if (Instruction *RetInst = dyn_cast<ReturnInst>(Inst)) {
          DummyExitLI->Uses.set(vix);
        }
      }
      else {
        // TDOO:  need it to put it into a LOG
        errs() << "WARNING: the use of a value is not an instruction.\n";
      }
    }
  }

  for (inst_iterator II = inst_begin(F), IE = inst_end(F); II != IE; ++II) {
    Instruction *DefInst = &*II;
    if (isCandidateValue(DefInst)) {
      Value *Val = DefInst;
      BasicBlock *DefBB = DefInst->getParent();
      LiveRange *LR = new LiveRange(Val, DefBB);
      ValueLiveRanges->insert(std::make_pair(Val, LR));

      unsigned vix = getValueEncoding(Val);
      BasicBlockLiveInfo *DefBBLI = getBBLiveInfo(DefBB);
      DefBBLI->Defs.set(vix);

      for (Value::use_iterator UI=Val->use_begin(),
                               UE=Val->use_end();
        UI != UE;
        ++UI) {
        if (Instruction *UseInst = dyn_cast<Instruction>(*UI)) {
          BasicBlock *UseBB = UseInst->getParent();
          BasicBlockLiveInfo *UseBBLI = getBBLiveInfo(UseBB);
          UseBBLI->Uses.set(vix);

          if (UseBB != DefBB) {
            UseBBLI->Ins.set(vix);
          }
          else {
            // If it is a upward exposed use, set Ins as well.
            for (BasicBlock::iterator BI = DefBB->begin(), BE = DefBB->end();
                 BI != BE; ++BI) {
              Instruction *tmpInst = &*BI;
              if (tmpInst == UseInst) {
                // Upward exposed.
                DefBBLI->Ins.set(vix);
                break;
              }
              else if (tmpInst == DefInst) {
                // Not upward exposed.
                break;
              }
            }
          }

          // Outgoing return value
          if (Instruction *RetInst = dyn_cast<ReturnInst>(UseInst)) {
            DummyExitLI->Uses.set(vix);
          }
        }
        else {
          // TODO: Put error message into log
          errs() << "WARNING: the use of a value is not an instruction.\n";
        }
      }
    }
  }
}

bool
LivenessAnalysisBase::CFGITraversal(BasicBlock *BB)
{
  bool change = false;

  // TODO: make sure (Uses - Defs) is needed !
  BasicBlockLiveInfo *BBLI = getBBLiveInfo(BB);
  BitVector old = BBLI->Ins;
  BBLI->Ins |= (BBLI->Outs & (~BBLI->Defs)); 
  Visited[BB] = 1;
  if (BBLI->Ins != old) {
    change = true;
  }

  for (pred_iterator PI = pred_begin(BB), PE = pred_end(BB);
       PI != PE; ++PI) {
    BasicBlock *Pred = *PI;
    BasicBlockLiveInfo *PredLI = getBBLiveInfo(Pred);
    PhiMaskMap::iterator MI = PhiMasks->find(std::make_pair(BB, Pred));

    if (!change) {
      old = PredLI->Outs;
    }

    if (MI == PhiMasks->end()) {
      PredLI->Outs |= BBLI->Ins;
    }
    else {
      BitVector *BVec = MI->second;
      PredLI->Outs |= (BBLI->Ins & *BVec);
    }
    if (!change) {
      if (old != PredLI->Outs) {
        change = true;
      }
    }

    if (Visited[Pred] == 0) {
      if (CFGITraversal(Pred)) {
        change = true;
      }
    }
  }
  return change;
}

bool
LivenessAnalysisBase::calculate()
{
  // *** Compute the size of bit vector
  initValueEncoding();

  // If the number of values is no greater than the 
  // threshold,  do not do liveness analysis.
  if (NumValues <= MinThreshold) {
    // Do not perform Liveness Analysis!
    releaseMemory();
    return false;
  }

  // Special handling for BBs that have PHI nodes
  initPhiMasks();

  // Liveness Analysis Setup
  initLocalInfo();

  // *** Iterative Algorithm to compute all live information
  //  1) Initial Entry and Exit BBs
  //  2) Iterate over control flow until there is no change 
  BasicBlock *EntryBB = &F.getEntryBlock();
  BasicBlockLiveInfo *DummyEntryLI = getBBLiveInfo(DummyEntryBB);
  (*BBLiveInfo)[EntryBB]->Ins |= DummyEntryLI->Defs;

  SmallVector<BasicBlock*, 8> ExitBBs;
  for(Function::iterator FI = F.begin(),
                         FE = F.end();
      FI != FE; ++FI) {
    BasicBlock *BB = FI;
    succ_iterator SI = succ_begin(BB);
    if (SI == succ_end(BB)) {
      ExitBBs.push_back(BB);

      TerminatorInst *TInst = BB->getTerminator();
      if (ReturnInst *RetInst = dyn_cast<ReturnInst>(TInst)) {
        if (Value *RetVal = RetInst->getReturnValue()) {
          if (isCandidateValue(RetVal)) {
            unsigned vix = getValueEncoding(RetVal);
            BasicBlockLiveInfo *BBLI = getBBLiveInfo(BB);
            BBLI->Outs.set(vix);
          }
        }
      }
    }
  }

  bool change = true;
  while (change) {
    Visited.clear();
    change = false;
    for (int i=0; i < (int)ExitBBs.size(); ++i) {
      if (CFGITraversal(ExitBBs[i])) {
        change = true;
      }
    }
  }

  // TODO: Defining BB may have exposed use as well !!!
  // Complete LR information
  for (BBLiveInfoMap::iterator BI = BBLiveInfo->begin(), BE = BBLiveInfo->end();
       BI != BE; ++BI) {
    BasicBlock *BB = BI->first;
    BasicBlockLiveInfo* BBLI = BI->second;
    for (int ix = 1; ix < (int)NumValues; ++ix) {
      bool isUse = (BBLI->Uses[ix] != 0);
      bool isOut = (BBLI->Outs[ix] != 0);
      if (isUse || isOut) {
        Value *Val = getEncodingValue(ix);
        if (isUse && isOut) {
          (*ValueLiveRanges)[Val]->addBB(BB, 
            LiveRange::LR_VALUE_USED | LiveRange::LR_VALUE_LIVEOUT);
        }
        else if (isUse) {
          (*ValueLiveRanges)[Val]->addBB(BB, LiveRange::LR_VALUE_USED);
        }
        else {
          (*ValueLiveRanges)[Val]->addBB(BB, LiveRange::LR_VALUE_LIVEOUT);
        }
      }
    }
  }

  DEBUG( dbgs() << "IR Dump for Liveness Analysis : Function "
                << F.getName() << "\n" << static_cast<Value&>(F) );
  DEBUG( dump() );
  DEBUG( dumpLR() );

  return true;
}

unsigned
LivenessAnalysisBase::getBBNLRAtFatPoint(
  BasicBlock *BB,
  Instruction **Fatpoint)
{
  BasicBlockLiveInfo *BBLI = getBBLiveInfo(BB);

  // Initialize the fatpoint to the first instruction
  unsigned int max_nlr = BBLI->Ins.count();
  Instruction *fpoint = BB->begin();

  // Going back from the last instruction to the first, keep track of 
  // the number of live ranges that are live at any instruction point.
  // And the instruction that has the max number of live ranges is the
  // fat point.
  BitVector LiveValues(BBLI->Outs);
  unsigned int nlr = BBLI->Outs.count();
  BasicBlock::InstListType &InstList = BB->getInstList();
  for (BasicBlock::InstListType::reverse_iterator RI = InstList.rbegin(),
                                                  RE = InstList.rend();
       RI != RE; ++RI) {
    Instruction *Inst = &*RI;
    unsigned def_vix, use_vix;
    def_vix = getValueEncoding(Inst, false);
    if (def_vix > 0) {
      if (!BBLI->Ins[def_vix] && LiveValues[def_vix]) {
        LiveValues.reset(def_vix);
        --nlr;
      }
    }
    for (int i=0, NumOperands = Inst->getNumOperands();
         i< NumOperands; ++i) {
      Value *useVal = Inst->getOperand(i);
      use_vix = getValueEncoding(useVal, false);
      if (use_vix > 0) {
        if (!LiveValues[use_vix]) {
          LiveValues.set(use_vix);
          ++nlr;
        }
      }
    }
    if (nlr > max_nlr) {
      max_nlr = nlr;
      fpoint = Inst;
    }
  }
  
  *Fatpoint = fpoint;
  return max_nlr;
}

unsigned
LivenessAnalysisBase::getNLRAtFatPoint(Instruction **Fatpoint)
{
  unsigned NumRegs = 0;
  Instruction *point = NULL, *inst;
  for (Function::iterator FI = F.begin(), FE = F.end();
       FI != FE; ++FI) {
    BasicBlock *BB = FI;
    unsigned Num = getBBNLRAtFatPoint(BB, &inst);
    if (Num > NumRegs) {
      NumRegs = Num;
      point = inst;
    }
  }
  *Fatpoint = point;
  return NumRegs;
}

void
LivenessAnalysisBase::releaseMemory() {
  if (ValueEncoding) {
    delete ValueEncoding;

    ValueEncoding = NULL;
  }

  if (EncodingValue) {
    delete EncodingValue;
    EncodingValue = NULL;
  }

  if (ValueLiveRanges) {
    for (ValueLiveRangeMap::iterator I = ValueLiveRanges->begin(),
                                     E = ValueLiveRanges->end();
         I != E; ++I) {
      LiveRange *lr = I->second;
      delete lr;
    }
    delete ValueLiveRanges;
    ValueLiveRanges = NULL;
 }
 if (BBLiveInfo) {
    for (BBLiveInfoMap::iterator I = BBLiveInfo->begin(), E = BBLiveInfo->end();
         I != E; ++I) {
      BasicBlockLiveInfo *bbli = I->second;
      delete bbli;
    }
    delete BBLiveInfo;
    BBLiveInfo = NULL;
  }

  if (PhiMasks) {
    for (PhiMaskMap::iterator I = PhiMasks->begin(), E = PhiMasks->end();
         I != E; ++I) {
      BitVector *BVec = I->second;
      delete BVec;
    }
    delete PhiMasks;
    PhiMasks = NULL;
  }
}

void
LivenessAnalysisBase::dump()
{
  Instruction *fatpoint = NULL;
  unsigned nlr_fatpoint = getNLRAtFatPoint(&fatpoint);
  
  dbgs() << "<liveness> Function: " << F.getName() << "\n"
         << "  #Basic Blocks: " << F.size() << "\n"
         << "  #Values: " << NumValues << "\n"
         << "  #Fat Point: BB (" 
             << fatpoint->getParent()->getName() << ") "
             << nlr_fatpoint << "\n";

  for (Function::iterator FI = F.begin(), FE = F.end();
       FI != FE; ++FI) {
    BasicBlock *BB = FI;
    dump(BB);
  }

  dbgs() << "<liveness> End of Function: " 
         << F.getName() << "\n\n";
}

void
LivenessAnalysisBase::dump(BasicBlock *BB)
{
  dbgs() << "\n  BB " << BB->getName() << "\n";
  BasicBlockLiveInfo *BBLI = getBBLiveInfo(BB);
  if (BBLI) {
    dbgs() << "    Ins (" << BBLI->Ins.count() << ") : ";
    dump(&BBLI->Ins);
    dbgs() << "    Defs (" << BBLI->Defs.count() << ") : ";
    dump(&BBLI->Defs);
    dbgs() << "    Uses (" << BBLI->Uses.count() << ") : ";
    dump(&BBLI->Uses);
    dbgs() << "    Outs (" << BBLI->Outs.count() << ") : ";
    dump(&BBLI->Outs);
  }

  // Phi Masks
  for (pred_iterator PI = pred_begin(BB), PE = pred_end(BB);
       PI != PE; ++PI) {
    BasicBlock *Pred = *PI;
    PhiMaskMap::iterator MI = PhiMasks->find(std::make_pair(BB, Pred));
    if (MI != PhiMasks->end()) {
      BitVector* BV = MI->second;
      dbgs() << "\n    PHI Nodes Mask Bit Vectors (reversed):\n" 
             << "      Pred : " << Pred->getName()
             << "\n           ";
      BitVector tBV = ~(*BV) & BBLI->Ins;
      dump(&tBV, 8);
    }
  }
}

void 
LivenessAnalysisBase::dump(BitVector* BV, int indent)
{
  bool isEmpty = true;

  int nbytes = indent;
  for (int ix = 1; ix < (int)NumValues; ++ix) {
    if ((*BV)[ix]) {
      Value *Val = getEncodingValue(ix);
      std::string namestr;
      getValueName(F.getParent(), Val, namestr);
      dbgs() << namestr << ", ";
      nbytes += namestr.size() + 2;
      if (nbytes > 70) {
        nbytes = indent;
        dbgs() << "\n";
        dbgs().indent(indent);
      }
      isEmpty = false;
    }
  }
  if (isEmpty || nbytes != indent) {
    dbgs() << "\n";
  }
}

void 
LivenessAnalysisBase::dumpLR()
{
  dbgs() << "\n<liveness> Live Ranges:\n";
  if (ValueLiveRanges) {
    int nlocal = 0;
    std::string LocalLRs,  GlobalLRs;
    raw_string_ostream LO(LocalLRs), GO(GlobalLRs);     
    for (ValueLiveRangeMap::iterator I = ValueLiveRanges->begin(),
                                     E = ValueLiveRanges->end();
      I != E; ++I) {
      LiveRange *LR = I->second;
      if (LR->isGlobalLiveRange()) {
        LR->dump(&GO, 4);
      }
      else {
        ++nlocal;
        LR->dump(&LO, 4);
      }
    }

    // dump local LRs first
    dbgs() << "  Local Live Ranges (total : " << nlocal
           << ") :\n" << LO.str() << "\n"
           << "  Global Live Ranges (total : " 
           << ValueLiveRanges->size() - nlocal
           << ") :\n" << GO.str() << "\n"; 
  }
  dbgs() << "\n";
}


char LivenessAnalysis::ID = 0;
INITIALIZE_PASS(LivenessAnalysis, "LivenessAnalysis", "Liveness Analysis", false, true)

void LivenessAnalysis::calculateLiveness (Function& F)
{ 
  AMDLLVMContextHook *amdhook = static_cast<AMDLLVMContextHook*>(
    F.getContext().getAMDLLVMContextHook());
  // TODO: should run it when opt is invoked.
  if (!amdhook || !amdhook->amdoptions.OptLiveness) {
    return;
  }

  if (!LivenessValid) {
    unsigned threshold = 0;
    if (amdhook) {
      threshold = amdhook->amdoptions.NumAvailGPRs;
    }
    LAB = new LivenessAnalysisBase(F, threshold);
    LivenessValid = LAB->calculate();
  }
}

namespace {
  // A simple pass to get the register usage for a function
  class AMDLivenessPrinter : public FunctionPass {
  public:
    static char ID; // Pass identification.
    AMDLivenessPrinter() : FunctionPass(ID) {
      initializeAMDLivenessPrinterPass(*PassRegistry::getPassRegistry());
    }

    virtual bool runOnFunction(Function &F);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LivenessAnalysis>();
      AU.setPreservesAll();
    }
  };
}

char AMDLivenessPrinter::ID = 0;
INITIALIZE_PASS_BEGIN(AMDLivenessPrinter, "regest", "Register Usage Estimator", false, true)
INITIALIZE_PASS_DEPENDENCY(LivenessAnalysis);
INITIALIZE_PASS_END(AMDLivenessPrinter, "regest", "Register Usage Estimator", false, true)

bool AMDLivenessPrinter::runOnFunction(Function &F)
{
  LivenessAnalysis *LA = &getAnalysis<LivenessAnalysis>();
  LA->releaseMemory();
  LA->calculateLiveness(F);

  if (LA->isLivenessValid()) {
    LA->getBase()->dump();
  }
  LA->releaseMemory();
  return false;
}

FunctionPass *llvm::createAMDLivenessPrinterPass() {
  return new AMDLivenessPrinter();
}


