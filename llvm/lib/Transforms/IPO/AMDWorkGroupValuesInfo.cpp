//===- AMDWorkGroupValuesInfo.cpp - WorkGroup values information ----------===//
//
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
// This file implements the WorkGroup variant (and invariant) values detection
// functions of the AMDWorkGroupLevelExecution pass. The detection of these
// values is vital for preserving only the cross barrier values, between
// barrier-regions.
// An example of a trivially preserved value is the global ID.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "crossbarrier"
#include "AMDWorkGroupLevelExecution.h"
#include "llvm/Instructions.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Support/DataFlow.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/AMDWorkGroupUtils.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/DataLayout.h"
using namespace llvm;

// initializeWorkGroupValues - Initialize all WorkGroup values information
// needed for calling all other functions in this file, for F.
void AMDWorkGroupLevelExecution::initializeWorkGroupValues(Function &F)
{
  CrossBarrier.clear();
  WGVariants.clear();
  if (!FGetTIBLocalId)
    return;

  SmallVector<const Value*, 8> Pending;
  for (Value::use_iterator UI = FGetTIBLocalId->use_begin(),
       UE = FGetTIBLocalId->use_end(); UI != UE; ++UI) {
    const CallInst *CI = cast<CallInst>(*UI);
    if (CI->getParent()->getParent() != &F)
      continue;

    Pending.push_back(CI);
    do {
      const Value *V = Pending.pop_back_val();

      for (df_ext_iterator<const Value*, VariantsSet>
           I = df_ext_begin(V, WGVariants), E = df_ext_begin(V, WGVariants);
           I != E; ++I) {

        // A pointer which we store that value to, is also a WorkGroup variant.
        //
        if (const StoreInst *SI = dyn_cast<StoreInst>(*I)) {
          if (SI->getValueOperand() == V)
            Pending.push_back(SI->getPointerOperand());
        }
      }
    } while (!Pending.empty());
  }

  DEBUG((dbgs() << "Found " << WGVariants.size() << " WorkGroup variants.\n"));
}

// findFirstWorkGroupVariant - Returns the first WorkGroup variant instruction
// in BB.
Instruction *AMDWorkGroupLevelExecution::findFirstWorkGroupVariant(
                                                           BasicBlock &BB) const
{
  for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E; ++I) {
    if (!isWorkGroupInvariant(I))
      return I;
  }
  return 0;
}

// findLastWorkGroupVariant - Returns the last WorkGroup variant instruction
// in BB.
Instruction *AMDWorkGroupLevelExecution::findLastWorkGroupVariant(
                                                           BasicBlock &BB) const
{
  BasicBlock::InstListType &InstList = BB.getInstList();
  for (BasicBlock::InstListType::reverse_iterator I = InstList.rbegin(),
       E = InstList.rend(); I != E; ++I) {
    if (!isWorkGroupInvariant(&*I))
      return &*I;
  }
  return 0;
}

static bool ValueEscapes(const Instruction *Inst)
{
  const BasicBlock *BB = Inst->getParent();
  for (Value::const_use_iterator UI = Inst->use_begin(),E = Inst->use_end();
       UI != E; ++UI) {
    const Instruction *I = cast<Instruction>(*UI);
    if (I->getParent() != BB || isa<PHINode>(I))
      return true;
  }
  return false;
}

// calculateComplex - Find all cross barrier values in BB. BR is the containing
// "complex" barrier-region.
void AMDWorkGroupLevelExecution::calculateComplex(BarrierRegion &BR,
                                                  BasicBlock &BB)
{
  bool isWGVariantBlock = (getBlockTag(BB) == WGVariantBlock);

  for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E; ++I) {
    for (User::op_iterator O = I->op_begin(), E = I->op_end(); O != E; ++O) {
      if (Instruction *Inst = dyn_cast<Instruction>(*O)) {
        if (ValueEscapes(Inst) && (isWGVariantBlock || !isWorkGroupInvariant(Inst))) {
          if (!NonPreservable.count(Inst) && CrossBarrier.insert(Inst)) {
            DEBUG((dbgs() << "   cross-internal-barrier value: ",
                   WriteAsOperand(dbgs(), Inst, false), dbgs() << "\n"));
          }
        }
#if 0
        else {
          if (!isa<AllocaInst>(Inst) &&
              BR.contains(*Inst) && !DT->dominates(Inst, I)) {
            if (!NonPreservable.count(Inst) && CrossBarrier.insert(Inst)) {
              DEBUG((dbgs() << "   cross-internal-barrier value: ",
                     WriteAsOperand(dbgs(), Inst, false), dbgs() << "\n"));
            }
          }
        }
#endif
      }
    }
  }
}

// calculateSimple - Find all cross barrier values in BB. BR is the containing
// "simple" barrier-region.
void AMDWorkGroupLevelExecution::calculateSimple(BarrierRegion &BR,
                                                 BasicBlock &BB)
{
  bool isWGVariantBlock = (getBlockTag(BB) == WGVariantBlock);

  for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E; ++I) {
    DEBUG((dbgs() << "  WorkGroup invariant: ", I->dump()));

    for (User::op_iterator O = I->op_begin(), E = I->op_end(); O != E; ++O) {
      if (Instruction *Inst = dyn_cast<Instruction>(*O)) {
        BasicBlock *InstParent = Inst->getParent();
        bool InBRegion = BR.contains(*InstParent);
        if (!InBRegion) {
          if (isWGVariantBlock || !isWorkGroupInvariant(Inst)) {
            if (!NonPreservable.count(Inst) && CrossBarrier.insert(Inst)) {
              DEBUG((dbgs() << "    Cross-barrier value: ",
                     WriteAsOperand(dbgs(), Inst, false), dbgs() << "\n"));
            }
          }
        }
      }
    }
  }
}

void AMDWorkGroupLevelExecution::preserveNotDominatedValues(Function &F)
{
  for (inst_iterator II = inst_begin(F), E = inst_end(F); II != E; ++II) {
    Instruction *I = &*II;

    if (CrossBarrier.count(I))
      continue;

    for (Value::use_iterator U = I->use_begin(), E = I->use_end(); U != E; ++U) {
      if (Instruction *Inst = dyn_cast<Instruction>(*U)) {
        if (!DT->dominates(I, Inst)) {
          DEBUG((dbgs() << "   Non-dominated value: ",
                 WriteAsOperand(dbgs(), I, false), dbgs() << "\n"));
          CrossBarrier.insert(I);
          break;
        }
      }
    }
  }
}

// preserveWorkGroupInvariantsComplex - Find all cross barrier values in BR.
// BR must be a "complex" barrier-region.
void AMDWorkGroupLevelExecution::preserveWorkGroupInvariantsComplex(
                                                              BarrierRegion &BR)
{
  assert(BR.complex() && "Barrier region must be complex!");

  for (BarrierRegion::iterator I = BR.begin(), E = BR.end(); I != E; ++I) {
    DEBUG((dbgs() << "Cross-barrier values for block ",
           WriteAsOperand(dbgs(), I, false), dbgs() << ":\n"));

    calculateComplex(BR, *I);
  }
}

// preserveWorkGroupInvariantsSimple - Find all cross barrier values in BR.
// BR must be a "simple" barrier-region.
void AMDWorkGroupLevelExecution::preserveWorkGroupInvariantsSimple(
                                                              BarrierRegion &BR)
{
  assert(BR.simple() && "Barrier region must be simple!");

  for (BarrierRegion::iterator I = BR.begin(), E = BR.end(); I != E; ++I) {
    DEBUG((dbgs() << "Cross-barrier values for block ",
           WriteAsOperand(dbgs(), I, false), dbgs() << ":\n"));
    
    calculateSimple(BR, *I);
  }
}

static inline uint32_t AlignDown_32(uint32_t Value, uint32_t Alignment) {
  return Value & ~(Alignment - 1);
}

static inline uint32_t AlignUp_32(uint32_t Value, uint32_t Alignment) {
  return AlignDown_32(Value + Alignment - 1, Alignment);
}

static bool IsV2f(Type *Ty)
{
  if (VectorType *VTy = dyn_cast<VectorType>(Ty)) {
    if (VTy->getNumElements() == 2 && VTy->getElementType()->isFloatTy())
      return true;
  }
  else if (ArrayType *ATy = dyn_cast<ArrayType>(Ty)) {
    return IsV2f(ATy->getElementType());
  }
  else if (StructType *STy = dyn_cast<StructType>(Ty)) {
    for (unsigned Idx = 0, Num = STy->getNumElements(); Idx != Num; ++Idx) {
      if (IsV2f(STy->getElementType(Idx)))
        return true;
    }
  }

  return false;
}

static inline unsigned GetTypeAlignment(Type *Ty, DataLayout &DL)
{
   unsigned Align = DL.getABITypeAlignment(Ty);
   if (Align < 16 && IsV2f(Ty))
     Align = 16;
   return Align;
}

static inline unsigned GetTypeSize(Type *Ty, DataLayout &DL)
{
  return DL.getTypeAllocSize(Ty);
}

// preserveWorkGroupVariants - Emit code for preserving the gathered cross-
// barrier values. Returns the private memory size of a single WorkItem.
unsigned AMDWorkGroupLevelExecution::preserveWorkGroupVariants()
{
  // Preserve all allocas after AIWorkItemIdx (meaning - allocas that are not
  // part of the WorkGroup loop).
  //
  BasicBlock::iterator AI = AIWorkItemIdx;
  while (isa<AllocaInst>(++AI)) {
    if (!AI->getName().startswith(RESERVED_NAME_PREFIX))
      CrossBarrier.insert(AI);
  }

  DEBUG(dbgs() << "Preserving " << CrossBarrier.size() << " cross-barrier values.\n");
  if (CrossBarrier.empty())
    return 0;

  DataLayout &DL = getAnalysis<DataLayout>();
  LLVMContext &C = VPrivatePtr->getContext();

  unsigned Offset = 0;
  unsigned StructSize = calculateCrossBarrierStructSize();
  PointerType *PrivateTy =
         PointerType::getUnqual(ArrayType::get(Type::getInt8Ty(C), StructSize));

  IntegerType *Int32Ty = Type::getInt32Ty(C);
  Value *Idxs[2];
  for (unsigned Idx = 0, Num = CrossBarrier.size(); Idx != Num; ++Idx) {
    Instruction *I = CrossBarrier[Idx];
    Type *Ty = !isa<AllocaInst>(I) ? I->getType() :
                                     cast<AllocaInst>(I)->getAllocatedType();

    VectorType *VTyVi1 = 0;
    if (VectorType *VTy = dyn_cast<VectorType>(Ty)) {
      unsigned EltBits = VTy->getElementType()->getPrimitiveSizeInBits();
      if (EltBits == 1) {
        VTyVi1 = VTy;
        Ty = VectorType::get(Int32Ty, VTy->getNumElements());
      }
    }

    PointerType *PtrTy = PointerType::getUnqual(Ty);

    Offset = AlignUp_32(Offset, GetTypeAlignment(Ty, DL));
    Idxs[1] = ConstantInt::get(Int32Ty, Offset);

    bool IsAlloca = isa<AllocaInst>(I);
    while (!I->use_empty()) {
      User *U = I->use_back();
      assert(isa<Instruction>(U) && "Incompatible use of instruction!");
      Instruction *Inst = cast<Instruction>(U);

      unsigned IdxPNVal = 0;
      PHINode *PN = dyn_cast<PHINode>(Inst);
      if (PN) {
        IdxPNVal = unsigned(&I->use_begin().getUse() - PN->op_begin());
        BasicBlock *BBIncoming = PN->getIncomingBlock(IdxPNVal);
        Inst = BBIncoming->getTerminator();
      }

      Idxs[0] = new LoadInst(AIWorkItemIdx, 0, Inst);
      Value *VPrivate = new BitCastInst(VPrivatePtr, PrivateTy, "", Inst);
      Value *V = GetElementPtrInst::Create(VPrivate, Idxs, "", Inst);
      V = new BitCastInst(V, PtrTy, "", Inst);
      if (!IsAlloca)
        V = new LoadInst(V, 0, Inst);

      // The x86 backend doesn't support loads and stores of i1 vectors, so we
      // patch them, by extending to i32 vectors. Therefore, we need to truncate
      // them back, before use.
      //
      if (VTyVi1)
        V = new TruncInst(V, VTyVi1, "", Inst);

      if (!PN)
        Inst->replaceUsesOfWith(I, V);
      else
        PN->setIncomingValue(IdxPNVal, V);
    }

    if (IsAlloca)
      I->eraseFromParent();
    else {
      BasicBlock::iterator InsertBefore = I;
      do {
        ++InsertBefore;
      } while (isa<PHINode>(InsertBefore));

      if (VTyVi1)
        I = new SExtInst(I, Ty, "", InsertBefore);

      Idxs[0] = new LoadInst(AIWorkItemIdx, 0, InsertBefore);
      Value *VPrivate = new BitCastInst(VPrivatePtr, PrivateTy, "", InsertBefore);
      Value *GEP = GetElementPtrInst::Create(VPrivate, Idxs, "", InsertBefore);
      VPrivate = new BitCastInst(GEP, PtrTy, "", InsertBefore);
      new StoreInst(I, VPrivate, InsertBefore);
    }

    Offset += GetTypeSize(Ty, DL);
  }

  CrossBarrier.clear();
  return StructSize;
}

unsigned AMDWorkGroupLevelExecution::calculateCrossBarrierStructSize() const
{
  unsigned Size = 0;
  DataLayout &DL = getAnalysis<DataLayout>();
  for (unsigned Idx = 0, Num = CrossBarrier.size(); Idx != Num; ++Idx) {
    Instruction *I = CrossBarrier[Idx];
    Type *Ty = !isa<AllocaInst>(I) ? I->getType() :
                                     cast<AllocaInst>(I)->getAllocatedType();

    if (VectorType *VTy = dyn_cast<VectorType>(Ty)) {
      unsigned EltBits = VTy->getElementType()->getPrimitiveSizeInBits();
      if (EltBits == 1) {
        Type *EltTy = IntegerType::getInt32Ty(VTy->getContext());
        Ty = VectorType::get(EltTy, VTy->getNumElements());
      }
    }

    Size = AlignUp_32(Size, GetTypeAlignment(Ty, DL));
    Size += GetTypeSize(Ty, DL);
  }

  return AlignUp_32(Size, 64);
}

// tagWGVariantBlocks - Tag all blocks between From and To as WGVariantBlock.
void AMDWorkGroupLevelExecution::tagWGVariantBlocks(BasicBlock &From,
                                                    BasicBlock &To,
                                                    PostDominatorTree &PDT)
{
  assert((BlockTags[&To] != UnknownBlock) && "To block must already be tagged!");

  Function::BasicBlockListType &FBlocks = From.getParent()->getBasicBlockList();

  SmallVector<BasicBlock *, 32> Pending;
  Pending.push_back(&From);
  do {
    BasicBlock *BB = Pending.pop_back_val();

    for (succ_iterator I = succ_begin(BB), E = succ_end(BB); I != E; ++I) {
      BasicBlock *BBSucc = *I;

      if (BBSucc == &To)
        continue;

      BlockTag &Tag = BlockTags[BBSucc];
      if (Tag == UnknownBlock || Tag == SimpleBlock) {
        Tag = WGVariantBlock;
        Pending.push_back(BBSucc);
        FBlocks.splice(&To, FBlocks, BBSucc);
      }
    }
  } while (!Pending.empty());
}

// tagSimpleBlocks - Tag all blocks between From and To as Simple, while
// considering possible WGVariantBlocks.
void AMDWorkGroupLevelExecution::tagSimpleBlocks(BasicBlock &From,
                                                 BasicBlock &To,
                                                 PostDominatorTree &PDT)
{
  assert((BlockTags[&To] != UnknownBlock) && "To block must already be tagged!");

  Function::BasicBlockListType &FBlocks = From.getParent()->getBasicBlockList();

  SmallVector<BasicBlock *, 64> Pending;
  Pending.push_back(&From);
  do {
    BasicBlock *BB = Pending.pop_back_val();
    if (BlockTags[BB] == WGVariantBlock)
      continue;

    TerminatorInst *TI = BB->getTerminator();
    if (TI->getNumSuccessors() > 1 && !isWorkGroupInvariant(TI)) {

      // We do a lazy initialization of the post-dominator tree, as we might
      // never use it, by never reaching this point!
      if (PDT.getRoots().empty())
        PDT.runOnFunction(*From.getParent());

      BasicBlock *BranchExit = PDT.getNode(BB)->getIDom()->getBlock();
      assert(PDT.dominates(&To, BranchExit) && "Invalid control block!");

      BlockTag &Tag = BlockTags[BranchExit];
      if (Tag == UnknownBlock) {
        Tag = SimpleBlock;
        Pending.push_back(BranchExit);
        FBlocks.splice(&To, FBlocks, BranchExit);
      }

      tagWGVariantBlocks(*BB, *BranchExit, PDT);
      continue;
    }

    for (succ_iterator I = succ_begin(BB), E = succ_end(BB); I != E; ++I) {
      BasicBlock *BBSucc = *I;

      BlockTag &Tag = BlockTags[BBSucc];
      if (Tag == UnknownBlock) {
        Tag = SimpleBlock;
        Pending.push_back(BBSucc);
        FBlocks.splice(&To, FBlocks, BBSucc);
      }
    }
  } while (!Pending.empty());
}

// tagBasicBlocks - Tag all basic blocks in F with BlockTag, while sorting the
// basic blocks list in a depth-first order.
void AMDWorkGroupLevelExecution::tagBasicBlocks(Function &F)
{
  BlockTags.clear();

  BasicBlock *Entry = isolateEntryBlock(F);
  BasicBlock *Exit  = isolateExitBlock(F);

  Function::BasicBlockListType &FBlocks = F.getBasicBlockList();

  // Find all control blocks and tag them
  SmallVector<BasicBlock *, 32> CtrlBlocks;
  for (DomTreeNode *DTN = DT->getNode(Exit); DTN->getBlock() != Entry;
       DTN = DTN->getIDom()) {
    BasicBlock *BB = DTN->getBlock();
    CtrlBlocks.push_back(BB);
    BlockTags[BB] = ControlBlock;
    FBlocks.splice(F.begin(), FBlocks, BB);
  }
  BlockTags[Entry] = ControlBlock;
  FBlocks.splice(F.begin(), FBlocks, Entry);

  PostDominatorTree PDT;
  for (BasicBlock *CtrlBegin = Entry; !CtrlBlocks.empty();
       CtrlBegin = CtrlBlocks.pop_back_val())
    tagSimpleBlocks(*CtrlBegin, *CtrlBlocks.back(), PDT);
}

// findControlBlock - Returns the first (closest) dominating Control block of BB.
const BasicBlock *AMDWorkGroupLevelExecution::findControlBlock(
                                                     const BasicBlock &BB) const
{
  for (Function::const_iterator I = &BB, B = BB.getParent()->begin(); I != B;
       --I) {
    if (getBlockTag(*I) == ControlBlock)
      return I;
  }
  return 0;
}

// removeLifetimeIntrinsics - Removes all uses of lifetime/invariant intrinsics
// in F.
void AMDWorkGroupLevelExecution::removeLifetimeIntrinsics(Function &F) const
{
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E;)
    if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(&*I++))
      switch (II->getIntrinsicID()) {
      case Intrinsic::lifetime_start:
      case Intrinsic::lifetime_end:
      case Intrinsic::invariant_start:
      case Intrinsic::invariant_end:
        II->eraseFromParent();
      default:
        break;
      }
}
