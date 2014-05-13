//===- AMDWorkGroupLevelExecution.h - WorkGroup level execution -*- C++ -*-===//
//
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
// This pass emits the WorkGroup loops for each barrier-region detected in the
// kernel's delegate functions (created by the previously called
// AMDLowerThreadInfoBlock pass).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_AMDWORKGROUPLEVELEXECUTION_H
#define LLVM_ANALYSIS_AMDWORKGROUPLEVELEXECUTION_H

#include "llvm/Pass.h"
#include "llvm/Instruction.h"
#include "llvm/Function.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"

namespace llvm {

class StructType;
class AllocaInst;
class BranchInst;
class BarrierRegion;
class DominatorTree;
class DominanceFrontier;
class LoopInfo;
class Loop;
class AMDWorkGroupLevelExecution;
struct PostDominatorTree;

struct BarrierBlock : public ilist_node<BarrierBlock>
{
  BasicBlock *Block;

  BarrierBlock(BasicBlock *BB = 0) : Block(BB) {}
};

class BarrierRegion : public ilist_node<BarrierRegion>
{
  friend class AMDWorkGroupLevelExecution;
public:
  typedef iplist<BarrierBlock> BarrierBlocksList;

private:
  BasicBlock *Entry;
  BasicBlock *Exit;
  DominatorTree *DT;
  BarrierBlocksList Loose;
  bool Filler;

public:

  BarrierRegion() : Entry(0), Exit(0), DT(0),
                    Filler(false)
  { }
  BarrierRegion(BasicBlock &BBEntry, BasicBlock &BBExit, DominatorTree &DTree) :
                                                                Entry(&BBEntry),
                                                                Exit(&BBExit),
                                                                DT(&DTree),
                                                                Filler(false)
  { }

  void setEntryBlock(BasicBlock &BB) { Entry = &BB; }
  BasicBlock &getEntryBlock() const { return *Entry; }

  void setExitBlock(BasicBlock &BB) { Exit = &BB; }
  BasicBlock &getExitBlock() const { return *Exit; }

  void setIsFiller(bool F) { Filler = F; }
  bool filler() const { return Filler; }

  // contains - Returns true if the basic block is contained in the region.
  bool contains(const BasicBlock &BB) const;
  // contains - Returns true if the instruction is contained in the region.
  bool contains(const Instruction &I) const { return contains(*I.getParent()); }

  void print(raw_ostream& OS) const;
  void dump() const;
  void verify() const;

  typedef Function::iterator iterator;
  typedef Function::const_iterator const_iterator;
  //===--------------------------------------------------------------------===//
  // Blocks iterator forwarding functions
  //
  iterator        begin()       { return Entry; }
  const_iterator  begin() const { return Entry; }
  iterator        end  ()       { return ++iterator(Exit);   }
  const_iterator  end  () const { return ++const_iterator(Exit);   }

  size_t          size () const { return empty() ? 0 : std::distance(begin(), end()); }
  bool            empty() const { return !Entry; }


  typedef BarrierBlocksList::iterator loose_iterator;
  typedef BarrierBlocksList::const_iterator const_loose_iterator;
  //===--------------------------------------------------------------------===//
  // Loose barrier blocks iterator forwarding functions
  //
  loose_iterator        loose_begin()       { return Loose.begin(); }
  const_loose_iterator  loose_begin() const { return Loose.begin(); }
  loose_iterator        loose_end  ()       { return Loose.end();   }
  const_loose_iterator  loose_end  () const { return Loose.end();   }

  size_t                loose_size () const { return Loose.size();  }
  bool                  loose_empty() const { return Loose.empty(); }


  // A region is considered "simple" iff it has no loose barrier blocks,
  // otherwise it is "complex".
  bool simple()  const { return loose_empty(); }
  bool complex() const { return !simple(); }

  // degenerated - Returns true if the region has only one basic block that only
  // contains an unconditional branch.
  bool degenerated() const { return degeneratedSplit(Entry); }

  // degeneratedSplit - Returns true if splitting at BB results in a
  // degenerated region.
  bool degeneratedSplit(const BasicBlock *BB) const {
    return BB == Exit && BB->size() == 1;
  }


  // isValidEntry - Returns true if the basic block has a single parent which has
  // a single child.
  static bool isValidEntry(const BasicBlock &BB);

  // isValidExit - Returns true if the basic block has a single child which has
  // a single parent.
  static bool isValidExit(const BasicBlock &BB);
};

class AMDWorkGroupLevelExecution : public ModulePass {
private:
  enum BlockTag {
    // UnknownBlock - This is an invalid tag, and should only be used to specify
    // untagged blocks.
    UnknownBlock,

    // SimpleBlock - Simple blocks are the ones that do not classify into the
    // other types of tags.
    SimpleBlock,

    // ControlBlock - Control blocks are the ones that dominate the parent
    // function's exit block (and post-dominate the entry block).
    ControlBlock,

    // WGVariantBlock - These blocks are the ones present under a conditional
    // branch, which depend on the result of WorkGroup variant value.
    WGVariantBlock
  };
  typedef DenseMap<const BasicBlock *, BlockTag> BlockTagsMap;

  Function *FBarrier;
  Function *FGetTIBLocalId;
  Value *VPrivatePtr;
  Value *VWGSize;
  AllocaInst *AIWorkItemIdx;
  DominatorTree *DT;
  LoopInfo *LI;

  BlockTagsMap BlockTags;

  SmallPtrSet<const Value *, 16> NonPreservable;

public:
  static char ID; // Pass identification, replacement for typeid
  AMDWorkGroupLevelExecution() : ModulePass(ID) {
    initializeAMDWorkGroupLevelExecutionPass(*PassRegistry::getPassRegistry());
  }

  virtual bool runOnModule(Module& M);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  BlockTag getBlockTag(const BasicBlock &BB) const {
    BlockTagsMap::const_iterator I = BlockTags.find(&BB);
    return I != BlockTags.end() ? I->second : UnknownBlock;
  }

private:
  bool doInitialization(Function &F);
  void runOnFunction(Function &F);

  // tagBasicBlocks - Tag all basic blocks in F with BlockTag, while sorting the
  // basic blocks list in a depth-first order.
  void tagBasicBlocks(Function &F);

  // tagSimpleBlocks - Tag all blocks between From and To as Simple, while
  // considering possible WGVariantBlocks.
  void tagSimpleBlocks(BasicBlock &From, BasicBlock &To, PostDominatorTree &PDT);

  // tagWGVariantBlocks - Tag all blocks between From and To as WGVariantBlock.
  void tagWGVariantBlocks(BasicBlock &From, BasicBlock &To, PostDominatorTree &PDT);

  // findControlBlock - Returns the first (closest) dominating Control block of BB.
  const BasicBlock *findControlBlock(const BasicBlock &BB) const;

  // isolateEntryBlock - Isolates the non initialization instructions in the
  // function's entry block into a new basic block, and return the entry.
  BasicBlock *isolateEntryBlock(Function &F);

  // isolateExitBlock - Isolates the all exit paths in the function into a single
  // exit basic block.
  BasicBlock *isolateExitBlock(Function &F);

  // isolateReturnInstruction - Isolates the return instruction into a single
  // basic block.
  BasicBlock *isolateReturnInstruction(Function &F);

  // eraseBarrierCalls - Erase all calls to barrier in the already processed
  // function. This requires us to calculate all barrier regions first.
  void eraseBarrierCalls() const;

  // emitWorkGroupLoop - Emit code to wrap BR in a loop which changes the
  // global/local ID in each iteration. The number of iterations is as the
  // WorkGroup size (VWGSize).
  void emitWorkGroupLoop(BarrierRegion &BR);

  // emitLatchBlock - Emit latch code for the WorkGroup loop. The code is inserted
  // into the end of Cond, loop to Body or exit to Cond's single successor.
  void emitLatchBlock(BasicBlock &Cond, BasicBlock &Body);

  // emitSwitchBarrierBlocks - Emit WorkGroup loop that handles complex barrier
  // regions. Every loose barrier block gets a unique state that identifies it
  // in a switch statement that added at the beginning. The state is common for
  // all WorkItems, as specified by the OpenCL specifications.
  void emitSwitchBarrierBlocks(BarrierRegion &BR, BasicBlock &Default);

  // addWorkGroupArguments - Append to the beginning of F's arguments list, the
  // required arguments ("private-buffer" & "WorkGroup size").
  Function *addWorkGroupArguments(Function &F);

  // splitBlock - Split the specified block at the specified instruction - every
  // thing before SplitPt stays in Old and everything starting with SplitPt moves
  // to a new block. The two blocks are joined by an unconditional branch.
  //
  // Note: This function is similar to llvm::SplitBlock() but it updates the
  //       loop info and dominator tree members.
  //
  BasicBlock *splitBlock(BasicBlock *Old, Instruction *SplitPt,
                         const char *Suffix = 0);



  //===--------------------------------------------------------------------===//
  // Implemented in AMDBarrierRegionInfo.cpp
  //
  friend class BarrierRegion;
  typedef iplist<BarrierRegion> BarrierRegionList;
  BarrierRegionList Regions;

  bool calculateBarrierRegions(Function &F);
  void printBarrierRegions(raw_ostream &OS) const;
  void dumpBarrierRegions() const;
  void verifyBarrierRegions() const;

  BarrierRegion *createBarrierRegion(BasicBlock &Entry, BasicBlock &Exit);

  // findContainingBarrierRegion - Returns barrier region containing BB, if exists.
  BarrierRegion *findContainingBarrierRegion(const BasicBlock &BB);

  // hasBarrier - Returns true if the current state of regions (calculated for a
  // specific function) contains a barrier call.
  bool hasBarrier() const;

  // findBarrierCalls - Split at the call to barrier all blocks in F containing
  // barrier calls and add those basic blocks (which now have a call to barrier
  // as their first instruction) to Barriers.
  void findBarrierCalls(Function &F, SmallVectorImpl<BasicBlock *> &Barriers);

  // insertBarrierBlock - BB is a barrier block, so try to split the containing
  // barrier region accordingly. Returns true on success.
  bool insertBarrierBlock(BasicBlock *BB, DominanceFrontier &DF);

  // isolateConditionUses - Sink the instructions which the branch is dependent
  // on (in the branch's block) to a new basic block, and return it.
  BasicBlock *isolateConditionUses(BranchInst *BI);

  // punctureBarrierRegion - Create a hole in the barrier region containing both
  // From and To, leaving two barrier regions behind (on success) -
  // [entry,From] and [To,exit].
  // - The barrier region is shrunk, as its exit block is set to From.
  // - A new barrier region is created with its entry set to To, and exit set to
  //   old region's exit.
  // The function returns the new barrier region [To,exit] on success.
  // Null on failure.
  BarrierRegion *punctureBarrierRegion(BasicBlock &From, BasicBlock &To,
                                       DominanceFrontier &DF);

  // isBarrierRegion - Check if Entry and Exit surround a valid region, based on
  // dominance tree and dominance frontier.
  bool isBarrierRegion(BasicBlock &Entry, BasicBlock &Exit, DominanceFrontier &DF) const;

  // isCommonDomFrontier - Returns true if BB is in the dominance frontier of
  // Entry, because it was inherited from Exit. In the other case there is an
  // edge going from Entry to BB without passing exit.
  bool isCommonDomFrontier(BasicBlock *BB, BasicBlock &Entry, BasicBlock &Exit) const;



  //===--------------------------------------------------------------------===//
  // Implemented in AMDWorkGroupValuesInfo.cpp
  //
  typedef SmallPtrSet<const Value *, 16> VariantsSet;
  typedef SetVector<Instruction *,
                    SmallVector<Instruction *, 64>,
                    SmallPtrSet<Instruction *, 8> > CrossBarrierVect;

  CrossBarrierVect CrossBarrier;
  VariantsSet WGVariants;

  // initializeWorkGroupValues - Initialize all WorkGroup values information
  // needed for calling all other functions in this file, for F.
  void initializeWorkGroupValues(Function &F);

  // preserveWorkGroupVariants - Emit code for preserving the gathered cross-
  // barrier values. Returns the private memory size of a single WorkItem.
  unsigned preserveWorkGroupVariants();

  // preserveWorkGroupInvariantsSimple - Find all cross barrier values in BR.
  // BR must be a "simple" barrier-region.
  void preserveWorkGroupInvariantsSimple(BarrierRegion &BR);

  // preserveWorkGroupInvariantsComplex - Find all cross barrier values in BR.
  // BR must be a "complex" barrier-region.
  void preserveWorkGroupInvariantsComplex(BarrierRegion &BR);
  void preserveNotDominatedValues(Function &F);

  // isWorkGroupInvariant - Returns true if I is a WorkGroup invariant.
  bool isWorkGroupInvariant(const Instruction *I) const {
    return !WGVariants.count(I);
  }

  // isWorkGroupInvariant - Returns true if all of the instructions in BB are
  // WorkGroup invariants.
  bool isWorkGroupInvariant(BasicBlock &BB) const {
    return !findFirstWorkGroupVariant(BB);
  }

  unsigned calculateCrossBarrierStructSize() const;

  // calculateComplex - Find all cross barrier values in BB. BR is the containing
  // "complex" barrier-region.
  void calculateComplex(BarrierRegion &BR, BasicBlock &BB);

  // calculateSimple - Find all cross barrier values in BB. BR is the containing
  // "simple" barrier-region.
  void calculateSimple(BarrierRegion &BR, BasicBlock &BB);

  // findFirstWorkGroupVariant - Returns the first WorkGroup variant instruction
  // in BB.
  Instruction *findFirstWorkGroupVariant(BasicBlock &BB) const;

  // findLastWorkGroupVariant - Returns the last WorkGroup variant instruction
  // in BB.
  Instruction *findLastWorkGroupVariant(BasicBlock &BB) const;

  // removeLifetimeIntrinsics - Removes all uses of lifetime/invariant intrinsics
  // in F.
  void removeLifetimeIntrinsics(Function &F) const;
};

} // End llvm namespace

#endif
