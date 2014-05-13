//===- AMDBarrierRegionInfo.cpp - List all barrier regions ----------------===//
//
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
// This file implements the barrier-region related functions of the
// AMDWorkGroupLevelExecution pass.
//
// Barrier-Region - Constraints:
//   A barrier-region has an entry and exit blocks, that dominate the function's
//   exit block, and post-dominate the function's entry block.
//
// Barrier-Region - Types:
//   Barrier-regions are divided into 2 types: "simple" and "complex".
//   A barrier-region is considered "simple" iff it has no loose barrier blocks,
//   otherwise it is "complex".
//   - A "loose barrier" is a barrier inside a loop or under a branch. A barrier
//     that is not "loose" must be called from a block that dominates the
//     function's exit block.
//
// Barrier-Region - Calculation Algorithm:
//   1. Define the first barrier-region as the whole code contained in the given
//      function, excluding the allocas (Assumes all allocas are defined in the
//      beginning of the function's entry block).
//   2. For each call to barrier in the given function split the parent basic
//      block on the call instruction, and do:
//   2.1. If it is a "loose" barrier: add it to the containing barrier-region.
//   2.2. Else, split the containing barrier-region on that call. As a result
//        we get 2 new barrier-regions: [entry,barrier), [barrier,exit].
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "barrierregion"

#include "AMDWorkGroupLevelExecution.h"
#include "llvm/Instructions.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/Transforms/Utils/Local.h"
using namespace llvm;

// isValidEntry - Returns true if the basic block has a single parent which has
// a single child.
bool BarrierRegion::isValidEntry(const BasicBlock &BB)
{
  if (BB.getNumUses() == 1) {
    const BranchInst *BIPred = dyn_cast<BranchInst>(*BB.use_begin());
    return BIPred && BIPred->isUnconditional();
  }
  return false;
}

// isValidExit - Returns true if the basic block has a single child which has
// a single parent.
bool BarrierRegion::isValidExit(const BasicBlock &BB)
{
  if (const BranchInst *BI = dyn_cast<BranchInst>(BB.getTerminator())) {
    return BI->isUnconditional() && (BI->getSuccessor(0)->getNumUses() == 1);
  }
  return false;
}

// contains - Returns true if the basic block is contained in the region.
bool BarrierRegion::contains(const BasicBlock &BB) const
{
  // Both Entry and Exit dominate the function's exit node.
  // In that case, all basic blocks inside the region must be between them.
  return DT->dominates(Entry, &BB) && !DT->properlyDominates(Exit, &BB);
}

void BarrierRegion::print(raw_ostream& OS) const
{
  OS.indent(2) << "{";
  WriteAsOperand(OS, Entry, false);
  OS << " => ";
  WriteAsOperand(OS, Exit, false);
  OS << "}\n";
  for (const_loose_iterator I = loose_begin(), E = loose_end(); I != E; ++I) {
    OS.indent(4) << "[";
    WriteAsOperand(OS, I->Block, false);
    OS << "]\n";
  }
}

void BarrierRegion::dump() const
{
  print(dbgs());
}

void BarrierRegion::verify() const
{
  if (Filler)
    return;

  assert((isValidEntry(getEntryBlock()) && isValidExit(getExitBlock())) &&
         "BarrierRegion should be a single entry single exit region!");

  for (const_loose_iterator I = loose_begin(), E = loose_end(); I != E; ++I) {
    assert(contains(*I->Block) &&
           "Loose BarrierBlock should be inside the BarrierRegion!");
  }
}


BarrierRegion *AMDWorkGroupLevelExecution::createBarrierRegion(BasicBlock &Entry,
                                                               BasicBlock &Exit)
{
  return new BarrierRegion(Entry, Exit, *DT);
}

bool AMDWorkGroupLevelExecution::calculateBarrierRegions(Function &F)
{
  assert(!BlockTags.empty() && "Blocks must be tagged!");

  // Alloca's have already been isolated from the entry block.
  //
  // Return instruction has already been isolated, and located at the end.
  //
  BasicBlock *Entry = F.getEntryBlock().getTerminator()->getSuccessor(0);
  BasicBlock *Exit = F.back().getSinglePredecessor();
  if (!(Entry && Exit))
    return false;

  Regions.clear();

  typedef SmallVector<BasicBlock *, 8> BarrierBlocks;
  BarrierBlocks Barriers;
  findBarrierCalls(F, Barriers);

  BarrierRegion::BarrierBlocksList LooseBarriers;

  // The DominanceFrontier is only needed to determine if a a pair of entry-exit
  // may determine a valid region. That is, only if there may even be more than
  // one region - meaning if there is a call to barrier in the function.
  DominanceFrontier DF;
  if (!Barriers.empty()) {
    DF.calculate(*DT, DT->getRootNode());

    // There might have been a call to barrier in the exit block, so we need to
    // update it, as we split it.
    Exit = F.back().getSinglePredecessor();
  }

  Regions.push_front(createBarrierRegion(*Entry, *Exit));
  for (BarrierBlocks::iterator I = Barriers.begin(), E = Barriers.end();
        I != E; ++I) {
    BasicBlock *BB = *I;

    // Try to create a new region by splitting a current one at the specific
    // block, otherwise mark it as a loose barrier block.
    if (!insertBarrierBlock(BB, DF))
      LooseBarriers.push_back(new BarrierBlock(BB));
  }

  // Pass through all the barrier blocks that have not been successfully
  // define a valid region, and add them to the containing regions as loose blocks.
  while (!LooseBarriers.empty()) {
    BarrierBlock *BB = &LooseBarriers.back();
    BarrierRegion *BR = findContainingBarrierRegion(*BB->Block);
    assert(BR && "Containing region not found!");

    // LooseBarriers is a iplist, meaning that on remove from the iplist, the
    // node is deallocated, and then we'll need to reallocate the BarrierBlock
    // and push it into the BR->Loose (iplist).
    // The only way to avoid the reallocation is to transfer the node by using
    // splice().
    BR->Loose.splice(BR->Loose.end(), LooseBarriers, BB);
  }
  return true;
}

void AMDWorkGroupLevelExecution::printBarrierRegions(raw_ostream &OS) const
{
  OS << "Barrier Region list:\n";
  for (BarrierRegionList::const_iterator I = Regions.begin(), E = Regions.end();
       I != E; ++I)
    I->print(OS);
  OS << "End barrier region list\n";
}

void AMDWorkGroupLevelExecution::dumpBarrierRegions() const
{
  printBarrierRegions(dbgs());
}

void AMDWorkGroupLevelExecution::verifyBarrierRegions() const
{
  for (BarrierRegionList::const_iterator I = Regions.begin(), E = Regions.end();
       I != E; ++I)
    I->verify();
}

// findContainingBarrierRegion - Returns barrier region containing BB, if exists.
BarrierRegion *AMDWorkGroupLevelExecution::findContainingBarrierRegion(
                                                           const BasicBlock &BB)
{
  const BasicBlock *Entry = &BB.getParent()->getEntryBlock();
  const BasicBlock *Ctrl = &BB;

  // Get the properly dominating Control block of the last one
  while ((Ctrl = findControlBlock(*Ctrl))) {

    // Search for the region which this Control block is it's entry
    for (BarrierRegionList::iterator I = Regions.begin(), E = Regions.end();
         I != E; ++I) {
      if (&I->getEntryBlock() == Ctrl)
        return I;
    }

    if (Ctrl == Entry)
      break;

    Ctrl = --Function::const_iterator(Ctrl);
  }

  return 0;
}

// findBarrierCalls - Split at the call to barrier all blocks in F containing
// barrier calls and add those basic blocks (which now have a call to barrier
// as their first instruction) to Barriers.
void AMDWorkGroupLevelExecution::findBarrierCalls(
                           Function &F, SmallVectorImpl<BasicBlock *> &Barriers)
{
  Barriers.clear();

  // FBarrier - the barrier function.
  if (!FBarrier) return;

  // Find all the barrier uses inside F.
  //
  for (Value::use_iterator I = FBarrier->use_begin(), E = FBarrier->use_end();
        I != E; ++I) {
    // Calls to barrier are never "InvokeInst" and always direct!
    if (CallInst *CI = dyn_cast<CallInst>(*I)) {
      BasicBlock *BB = CI->getParent();
      if (BB->getParent() != &F)
        continue;

      // If the block doesn't start with the call to barrier or doesn't fit to
      // be a region's entry, then make it so.
      if (!(&*BB->begin() == CI && BarrierRegion::isValidEntry(*BB)))
        BB = splitBlock(BB, CI, ".barrier");

      Barriers.push_back(BB);
    }
  }
}

// isolateConditionUses - Sink the instructions which the branch is dependent on
// (in the branch's block) to a new basic block, and return it.
BasicBlock *AMDWorkGroupLevelExecution::isolateConditionUses(BranchInst *BI)
{
  // Split the owning basic block on BI, which "sinks" it to the new block - NB.
  //
  BasicBlock *BB = BI->getParent();
  BasicBlock *NB = splitBlock(BB, BI, ".latch");
  Instruction *IDep = dyn_cast<Instruction>(BI->getCondition());

  // Before we start sinking all the instructions that BI is dependent on, we
  // can ignore the cases of unconditional branches, and the trivial case, when
  // the condition is in another block. In these cases we are already done.
  if (!(IDep && IDep->getParent() == BB))
    return NB;


  // Do a Depth-first search of all the instructions BI is dependent on, that
  // are in BI's basic block (BB).
  // This is a bottom up search on BI's operands.
  //
  // Note: we do not use df_iterator as we cannot monitor its instructions
  //       pending queue filtering, and the general end point.
  //
  SmallPtrSet<const Instruction *, 8> Visited;
  SmallVector<const Instruction *, 32> Pending;
  Visited.insert(IDep);
  Pending.push_back(IDep);
  do {
    const Instruction *I = Pending.pop_back_val();

    // Pass through all of the instruction's operands.
    for (User::const_op_iterator O = I->op_begin(), E = I->op_end(); O != E; ++O) {
      // We only process instructions (we cannot sink other values,
      // like arguments).
      if (const Instruction *Inst = dyn_cast<Instruction>(*O)) {
        // We only consider non PHI instructions from BB (BI's block).
        if (Inst->getParent() == BB && !isa<PHINode>(Inst) && Visited.insert(Inst))
          Pending.push_back(Inst);
      }
    }
  } while (!Pending.empty());


  BasicBlock::InstListType &NInstList = NB->getInstList();
  BasicBlock::InstListType &InstList = BB->getInstList();


  // Move (sink) all the instructions in Visited from BB to NB (the new block).
  // We keep the correct order as we scan the instructions in BB in their
  // original order.
  //
  BasicBlock::iterator NTI = NB->getTerminator(),
                       I = BB->getFirstNonPHI(),
                       E = BB->getTerminator();
  while (I != E) {
    // I might be moved to the new block, so first increment it so we won't lose
    // its connection in the instruction list.
    BasicBlock::iterator IP = I++;
    if (Visited.count(IP)) {
      // Move the instruction to the end of the NB's instruction list (NInstList),
      // just before the terminator. That way we keep the correct order, as the
      // following instruction will be inserted between the last one and the
      // terminator.
      NInstList.splice(NTI, InstList, IP);
    }
  }

  return NB;
}

// punctureBarrierRegion - Create a hole in the barrier region containing both
// From and To, leaving two barrier regions behind (on success) -
// [entry,From] and [To,exit].
// - The barrier region is shrunk, as its exit block is set to From.
// - A new barrier region is created with its entry set to To, and exit set to
//   old region's exit.
// The function returns the new barrier region [To,exit] on success.
// Null on failure.
BarrierRegion *AMDWorkGroupLevelExecution::punctureBarrierRegion(
                        BasicBlock &From, BasicBlock &To, DominanceFrontier &DF)
{
  // The basic blocks must be part of an existing barrier region.
  BarrierRegion *BR = findContainingBarrierRegion(From);
  if (!BR)
    return 0;

  assert(findContainingBarrierRegion(To) == BR &&
         "From and To are not from the same region!");

  // Make sure the new two barrier regions are valid.
  if (!isBarrierRegion(BR->getEntryBlock(), From, DF) ||
      !isBarrierRegion(To, BR->getExitBlock(), DF))
    return 0;

  // Create the new barrier region and keep the order in the regions list correct.
  Regions.insertAfter(BR,
           createBarrierRegion(To, BR->getExitBlock()));
  BR->setExitBlock(From);
  return BR;
}

static void demotePHINodesToStack(BasicBlock &BB)
{
  BasicBlock::iterator I = BB.begin();
  while (PHINode *PN = dyn_cast<PHINode>(I)) {
    ++I;
    DemotePHIToStack(PN);
  }
}

// insertBarrierBlock - BB is a barrier block, so try to split the containing
// barrier region accordingly. Returns true on success.
bool AMDWorkGroupLevelExecution::insertBarrierBlock(BasicBlock *BB,
                                                    DominanceFrontier &DF)
{
  // A Region Entry must dominate the Exit block!
  if (getBlockTag(*BB) != ControlBlock)
    return false;

  // Try to split the containing barrier region into two valid barrier regions.
  //
  if (BarrierRegion *BR = findContainingBarrierRegion(*BB)) {
    // Make sure the new barrier region will be valid.
    if (isBarrierRegion(*BB, BR->getExitBlock(), DF)) {
      if (!BR->degeneratedSplit(BB)) {
        // Create the new barrier region and keep the order in the regions list correct.
        Regions.insertAfter(BR,
          createBarrierRegion(*BB, BR->getExitBlock()));
        BR->setExitBlock(**pred_begin(BB));
      }
      return true;
    }
  }
  return false;
}

// hasBarrier - Returns true if the current state of regions (calculated for a
// specific function) contains a barrier call.
bool AMDWorkGroupLevelExecution::hasBarrier() const
{
  // If there is more than one region, we must have a barrier - as this is the
  // only reason for splitting the main region.
  //
  // If we have only one region, we must check if it has no loose barrier blocks.
  //
  return Regions.size() > 1 || !(Regions.empty() || Regions.front().loose_empty());
}

// isBarrierRegion - Check if Entry and Exit surround a valid region, based on
// dominance tree and dominance frontier.
bool AMDWorkGroupLevelExecution::isBarrierRegion(BasicBlock &Entry,
                                                 BasicBlock &Exit,
                                                 DominanceFrontier &DF) const
{
  typedef DominanceFrontier::DomSetType DST;

  if (!DT->dominates(&Entry, &Exit))
    return false;

  DST *EntrySuccs = &DF.find(&Entry)->second;
  DST *ExitSuccs = &DF.find(&Exit)->second;

  // Do not allow edges leaving the region.
  for (DST::iterator SI = EntrySuccs->begin(), SE = EntrySuccs->end();
       SI != SE; ++SI) {
    if (*SI == &Exit || *SI == &Entry)
      continue;
    if (ExitSuccs->find(*SI) == ExitSuccs->end())
      return false;
    if (!isCommonDomFrontier(*SI, Entry, Exit))
      return false;
  }

  // Do not allow edges pointing into the region.
  for (DST::iterator SI = ExitSuccs->begin(), SE = ExitSuccs->end();
       SI != SE; ++SI)
    if (DT->properlyDominates(&Entry, *SI) && *SI != &Exit)
      return false;

  return true;
}

// isCommonDomFrontier - Returns true if BB is in the dominance frontier of
// entry, because it was inherited from exit. In the other case there is an
// edge going from entry to BB without passing exit.
bool AMDWorkGroupLevelExecution::isCommonDomFrontier(BasicBlock *BB,
                                                     BasicBlock &Entry,
                                                     BasicBlock &Exit) const
{
  for (pred_iterator PI = pred_begin(BB), PE = pred_end(BB); PI != PE; ++PI) {
    BasicBlock *P = *PI;
    if (DT->dominates(&Entry, P) && !DT->dominates(&Exit, P))
      return false;
  }
  return true;
}
