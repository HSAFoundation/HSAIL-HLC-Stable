//===  HSAILEarlyCFGOpts.cpp - HSAIL control flow optimizations -*- C++ -*-===//
//===----------------------------------------------------------------------===//
//
// This file defines pass for specific HSAIL optimizations 
// which are used to simplify shader compiler job. 
// Optimizations implemented:
//   - Transform code in such way that each loop has only one exit block
//   - Swap loop exit branches so that back edge goes first
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "hsail-cfg-opts"

#include "HSAIL.h"

#include "llvm/LLVMContext.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/Pass.h"
#include "llvm/Instructions.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/InstrTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/IntrinsicInst.h"
#include <stack>
using namespace llvm;

static cl::opt<bool> DisableLoopCondSimplification("disable-loop-cond-simpl",
  cl::desc("Disable loop condition simplification"));
static cl::opt<bool> DisableLoopExitUnification("disable-loop-exit-unify",
  cl::desc("Disable loop exit unification"));

static cl::opt<unsigned> MaxExitsToUnify("hsail-max-loop-exits-to-unify", 
  cl::Hidden,
  cl::desc("Maximum number of exits to unify"), cl::init(5));

namespace 
{
  class HSAILEarlyCFGOpts: public LoopPass 
  {
  public:
    static char ID;
    bool isTransformedToSingleExit;
    explicit HSAILEarlyCFGOpts(): LoopPass (ID) { 
      isTransformedToSingleExit = false;}

    virtual bool runOnLoop(Loop *L, LPPassManager &LPM);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const
    {
      AU.addRequired<LoopInfo>();
      AU.addRequiredID(LCSSAID);
    }

    virtual const char *getPassName() const 
    { 
      return "HSAIL specific control flow optimizations";
    }

  private:
    bool unifyLoopExitBlocks(Function &F, Loop *L);
    bool simplifyLoopCond(Loop *cur_loop);
    BasicBlock *getCFGExitBlock(Loop *L);
    bool TransformToSingleExit(LPPassManager &LPM);

  private:

    LoopInfo *LI;
  };

  char HSAILEarlyCFGOpts::ID = 0;
}
  // Register pass in passRegistry so that the pass info gets populated for printing debug info 
  INITIALIZE_PASS(HSAILEarlyCFGOpts, "hsail-cfg-opts",
                  "HSAIL specific control flow optimizations", false, false)

LoopPass *llvm::createHSAILEarlyCFGOpts() 
{
  return new HSAILEarlyCFGOpts();
}

struct _instructionCompare
{
  Instruction *compare_to;

  _instructionCompare(Instruction *i): compare_to(i) {}

  bool operator()(Instruction &i)
  {
    return &i == compare_to;
  }
};

bool HSAILEarlyCFGOpts::unifyLoopExitBlocks(Function &F, Loop *L) 
{
  LLVMContext &context = F.getContext();

  // Get loop exit and exiting blocks
  SmallVector<BasicBlock*, 8> ExitingBlocks, ExitBlocks;
  SmallVector<Loop::Edge, 8> ExitEdges;
  L->getExitingBlocks(ExitingBlocks);
  L->getExitBlocks(ExitBlocks);
  L->getExitEdges(ExitEdges);

  SmallSetVector<BasicBlock *, 8> ExitBlockSet(ExitBlocks.begin(),
    ExitBlocks.end());

  // If there is already only one exit than return
  if (ExitBlockSet.size() <= 1)
    return false;

  // If there is too much ExitBlocks than return
  if (ExitBlockSet.size() >= MaxExitsToUnify)
    return false;

  // Verifier will fall on fairly correct code in some rare cases
  if (L->getLoopDepth() > 1)
    return false;

  // If there is basic blocks inside loop that branch to
  // multiple different locations outside loop give up this transformation
  // TODO_HSA: We can support this case but that probably not worth it
  if (ExitingBlocks.size() < ExitEdges.size())
    return false;

  // Create new basic block outside the loop
  BasicBlock *unifying_bb = BasicBlock::Create(
    context, "unified_loop_exit", &F);

  // Insert phi node with integer parameter for each ExitingBlock
  Type *phi_type = Type::getInt32Ty(context);
  PHINode *phi_node = PHINode::Create(phi_type, 
    ExitingBlocks.size(), "", unifying_bb);

  for (unsigned i = 0, e = ExitEdges.size(); i != e; ++i)
    phi_node->addIncoming(ConstantInt::get(phi_type, i), 
		    const_cast<BasicBlock*>(ExitEdges[i].first));

  // Insert switch with all ExitBlocks
  SwitchInst  *exit_blocks_switch = SwitchInst::Create(
    phi_node, const_cast<BasicBlock*>(ExitEdges[0].second), 
	ExitEdges.size(), unifying_bb);

  for (unsigned i = 0, e = ExitEdges.size(); i != e; ++i)
    exit_blocks_switch->addCase(
      cast<ConstantInt>(ConstantInt::get(phi_type, i)),
      const_cast<BasicBlock*>(ExitEdges[i].second));

  // Make all ExitingBlocks branch to unifying_bb
  for (unsigned i = 0, e = ExitEdges.size(); i != e; ++i)
  {
    const_cast<BasicBlock*>(ExitEdges[i].first)->getTerminator()->
      replaceUsesOfWith(const_cast<BasicBlock*>(ExitEdges[i].second), 
		      unifying_bb);
  }

  // Update phi nodes of exit blocks
  ValueMap<Value *, PHINode *> inserted_phis;
  for (unsigned i = 0, e = ExitEdges.size(); i != e; ++i)
  {
    BasicBlock *exit_bb = const_cast<BasicBlock*>(ExitEdges[i].second),
      *exiting_bb = const_cast<BasicBlock*>(ExitEdges[i].first);

    for (BasicBlock::iterator I = exit_bb->begin(); isa<PHINode>(I); )
    {
      PHINode *old_phi = cast<PHINode>(I++);
      if (old_phi->getBasicBlockIndex(exiting_bb) < 0) continue;
      Value *v = old_phi->getIncomingValueForBlock(exiting_bb);
      PHINode *new_phi = NULL;

      // If there is already phi node for the same value
      if (inserted_phis.count(v))
      {
        PHINode *temp_phi = inserted_phis.lookup(v);
        unsigned id;
        for (id=0; id < temp_phi->getNumIncomingValues(); id++) {
          if(temp_phi->getIncomingBlock(id) == exiting_bb && temp_phi->getIncomingValue(id) == v) 
            break;
        }  
        if (id < temp_phi->getNumIncomingValues())
          new_phi = temp_phi;
      }
      if(new_phi == NULL)
      {
        new_phi = PHINode::Create(old_phi->getType(), ExitingBlocks.size(), 
          old_phi->getName() + ".loop_unified", 
          unifying_bb->getFirstNonPHI());

        for (unsigned i = 0; i < ExitingBlocks.size(); ++i) {
          if (old_phi->getBasicBlockIndex(ExitingBlocks[i]) >= 0) {
            Value *inc_val =  old_phi->getIncomingValueForBlock(ExitingBlocks[i]);        
            new_phi->addIncoming(inc_val, ExitingBlocks[i]);
            inserted_phis.insert(std::pair<Value *, PHINode *>(inc_val, new_phi));
            old_phi->removeIncomingValue(ExitingBlocks[i], false);
          }
          else {
            new_phi->addIncoming(
              UndefValue::get(old_phi->getType()), ExitingBlocks[i]);
          }
        }
      }
      else {
        for (unsigned i = 0; i < ExitingBlocks.size(); ++i) {
          if (old_phi->getBasicBlockIndex(ExitingBlocks[i]) >= 0) {
            old_phi->removeIncomingValue(ExitingBlocks[i], false);
          }
        }  
      }          
      old_phi->addIncoming(new_phi, unifying_bb);

      // First exit block will have two unifying_bb predecessors.
      // And every phi node in it should have two incoming unifying_bb's
      // and if it is is not true then verifier will fall.
      // This is strange llvm glitch caused by default switch target being 
      // same as one of it's real targets.
      unsigned num_predecessors = 
        std::distance(pred_begin(exit_bb), pred_end(exit_bb));
      if (old_phi->getNumIncomingValues() < num_predecessors)
      {
        for (unsigned i = old_phi->getNumIncomingValues(); 
             i < num_predecessors; ++i)
        {
          old_phi->addIncoming(new_phi, unifying_bb);
        }
      }
    }
  }

  DEBUG(dbgs() << "Unified loop exit for loop " << 
    L->getHeader()->getName() << "\n");

  return true;
}

bool HSAILEarlyCFGOpts::simplifyLoopCond(Loop *cur_loop) 
{
  bool Changed = false;
  SmallVector<BasicBlock*, 8> ExitingBlocks;
  cur_loop->getExitingBlocks(ExitingBlocks);  

  for (SmallVectorImpl<BasicBlock *>::iterator I = ExitingBlocks.begin(),
    E = ExitingBlocks.end(); I != E; ++I)
  {
    BranchInst *BI = dyn_cast<BranchInst>((*I)->getTerminator());

    if (BI && BI->isConditional() && !cur_loop->contains(BI->getSuccessor(0)))
    {
      BI->setCondition(
        BinaryOperator::CreateNot(
          BI->getCondition(), "negate_loop_exit_cond", BI));
      BI->swapSuccessors();
      Changed = true;

      DEBUG(
        dbgs() << "Reversed loop condition in %" << (*I)->getName() << '\n');
    }
  }

  return Changed;
}

static void addLoopIntoStack(Loop *L, std::stack<Loop *> &LS) {
  LS.push(L);
  for (Loop::iterator I = L->begin(), E = L->end(); I != E; ++I)
    addLoopIntoStack(*I, LS);
}
static bool IsSameCondition(BasicBlock *tailBlkExtRgn, 
                            BasicBlock *hdrBlkExtRgn,
                            BasicBlock *updateHdrBlk){

  BasicBlock::iterator BBI1 = tailBlkExtRgn->getTerminator();
  BasicBlock::iterator BBI2 = updateHdrBlk->getTerminator();

  if(!isa<BranchInst>(&*BBI1) && !isa<BranchInst>(&*BBI2))
    return false;

  --BBI1;
  --BBI2;

  if (!isa<CmpInst>(&(*BBI1))){
    if(isa<CmpInst>(&*BBI2)){
      CmpInst *cmpInst =  dyn_cast<CmpInst>(&*BBI2);
      for (User::op_iterator i = cmpInst->op_begin(), e = cmpInst->op_end();
           i != e; ++i) {
        Instruction *OpI = dyn_cast<Instruction>(*i);
        if (OpI &&  OpI->isUsedInBasicBlock(tailBlkExtRgn))
          return true;
      }
    }
  }
  return false;
} 

// The Psuedo Inner Loop is being marked by HLC optimization which makes 
// the inner loop multiple Exit. This multiple exit will be transformed 
// to Single Exit by transforming the backedge of the inner loop to the
// original loop as same condition is checked in the backedge of
// the inner loop to the header of the original loop.
     
bool HSAILEarlyCFGOpts::TransformToSingleExit(LPPassManager &LPM){

  std::stack<Loop *> LS;
  LI = &getAnalysis<LoopInfo>();
  bool Changed = false;
  BasicBlock *backedge = NULL;
  BasicBlock *backedgeHeader = NULL;
  bool found_with_if_region_break = false;
  Loop *exitingLoop = NULL;
  // Populate Loop Stack 
  for (LoopInfo::iterator I = LI->begin(), E = LI->end(); I != E; ++I)
    addLoopIntoStack(*I, LS);

  while (!LS.empty()) {
    Loop *L = LS.top();
    if(found_with_if_region_break) 
    {
      for (pred_iterator SI = pred_begin(L->getHeader()), 
           SE = pred_end(L->getHeader()); SI != SE; ++SI){
        if(exitingLoop->contains(*SI) && 
           IsSameCondition(backedge, backedgeHeader,L->getHeader())){
          backedge->getTerminator()->replaceUsesOfWith(
            exitingLoop->getHeader(),L->getHeader());
          isTransformedToSingleExit = true;
          LPM.deleteLoopFromQueue(exitingLoop);
          found_with_if_region_break = false;
          break;
        } 
      }
    } 
    else if(getCFGExitBlock(L)){
      found_with_if_region_break = true;
      exitingLoop = L;
      for (pred_iterator SI = pred_begin(L->getHeader()),
           SE = pred_end(L->getHeader()); SI != SE; ++SI){
        if (L->contains(*SI)){
         backedge = *SI;
         backedgeHeader = L->getHeader();
        }
      }
    }
    LS.pop();
  }
  return isTransformedToSingleExit;
}

BasicBlock *HSAILEarlyCFGOpts::getCFGExitBlock(Loop *L)
{
  SmallVector<BasicBlock*, 8> ExitingBlocks;
  L->getExitingBlocks(ExitingBlocks); 
  for (SmallVectorImpl<BasicBlock *>::iterator I = ExitingBlocks.begin(),
    E = ExitingBlocks.end(); I != E; ++I){ 
       BranchInst *BI = dyn_cast<BranchInst>((*I)->getTerminator());
       if (BI && BI->isConditional() && (BI->getSuccessor(0)))
       {
          if(isa<ReturnInst>((BI->getSuccessor(0))->getTerminator()))
            return *I;
       }
   }
  return NULL;
}
       

bool HSAILEarlyCFGOpts::runOnLoop(Loop *L, LPPassManager &LPM)
{
  LI = &getAnalysis<LoopInfo>();

  bool Changed = false;

  if (L->getNumBlocks() == 0)
    return false;
  if(!isTransformedToSingleExit)
    Changed |= TransformToSingleExit(LPM);
  if (!DisableLoopExitUnification){
    Changed |= unifyLoopExitBlocks(*L->getHeader()->getParent(), L);
  }
  if (!DisableLoopCondSimplification){
    Changed |= simplifyLoopCond(L);
  }

  return Changed;
}
