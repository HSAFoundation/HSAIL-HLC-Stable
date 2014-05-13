//===- AMDWorkGroupLevelExecution.cpp - AMD WorkGroup level execution -----===//
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

#define DEBUG_TYPE "wglevelexec"
#include "AMDWorkGroupLevelExecution.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Constants.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Transforms/Utils/AMDArgumentUtils.h"
#include "llvm/Transforms/Utils/AMDWorkGroupUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/DataLayout.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/AMDOpenCLSymbols.h"
#include "llvm/Assembly/Writer.h"
#include "AMDSymbolName.h"
using namespace llvm;

char AMDWorkGroupLevelExecution::ID = 0;
INITIALIZE_PASS_BEGIN(AMDWorkGroupLevelExecution, "amdwglevelexec",
                      "AMD WorkGroup Level Execution", false, false)
INITIALIZE_AG_DEPENDENCY(OpenCLSymbols)
INITIALIZE_PASS_DEPENDENCY(DataLayout)
INITIALIZE_PASS_END(AMDWorkGroupLevelExecution, "amdwglevelexec",
                    "AMD WorkGroup Level Execution", false, false)

ModulePass *llvm::createAMDWorkGroupLevelExecutionPass() {
  return new AMDWorkGroupLevelExecution();
}

void AMDWorkGroupLevelExecution::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.addRequired<OpenCLSymbols>();

  AU.addRequired<DataLayout>();
  AU.addPreserved<DataLayout>();
}

// addWorkGroupArguments - Append to the beginning of F's arguments list, the
// required arguments ("private-buffer" & "WorkGroup size").
Function *AMDWorkGroupLevelExecution::addWorkGroupArguments(Function &F)
{
  CallInst *CI = cast<CallInst>(*F.use_begin());
  Function &FStub = *CI->getParent()->getParent();

  LLVMContext &C = F.getContext();
  const unsigned NUM_WGARGS = 2;
  NewArgInfo WGArgs[NUM_WGARGS] = {
    { Type::getInt8PtrTy(C), SARG_PRIVATE_NAME },
    { Type::getInt32Ty(C),   SARG_WGSIZE_NAME  }
  };

  NamedMDNode *MD = F.getParent()->getNamedMetadata(FStub.getName() + ".noffset");
  assert(MD && "Stub function metadata - new-offset not found!");

  ConstantInt *CIOffset = cast<ConstantInt>(MD->getOperand(0)->getOperand(0));
  unsigned Offset = CIOffset->getZExtValue();

  LoadInst *LI[NUM_WGARGS];
  EmitLoadAggregateArguments(FStub.arg_begin(), WGArgs, LI, Offset,
                             getAnalysis<DataLayout>(), *CI);

  Function *NF = CreateFunctionWithNewArguments(&F, WGArgs, true);

  SmallVector<Value*, 16> Args;
  Args.reserve(NUM_WGARGS + CI->getNumArgOperands());
  Args.resize(NUM_WGARGS);

  for (unsigned Idx = 0; Idx < NUM_WGARGS; ++Idx)
    Args[Idx] = LI[Idx];

  CallSite CS(CI);
  AddArgumentsToCallSite(CS, Args, NF);
  F.eraseFromParent();

  Function::arg_iterator AI = NF->arg_begin();
  VPrivatePtr = AI;
  VWGSize = ++AI;
  return NF;
}

bool AMDWorkGroupLevelExecution::runOnModule(Module& M)
{
  bool Changed = false;

  // FBarrier - the barrier function must be present in the module. This required
  // us to mark the function (if existed) as NoInline (if inlining was present
  // before the call to this pass), otherwise we will miss the calls to barrier.

  // TODO: The following macro should be removed when Clang becomes the
  // default compiler for building x86 built-ins. Only the check for the
  // mangled names should be retained.
#ifdef BUILD_X86_WITH_CLANG
  FBarrier = M.getFunction("_Z7barrierj");
#else
  FBarrier = M.getFunction("barrier");
#endif

  // FGetTIBLocalId - this function extracts the local_id from the
  // thread_info_block. This function if exists, should be named, before the
  // call to this pass. It is required for the "WorkGroup Invariant" analysis.
  FGetTIBLocalId = M.getFunction(FUNC_GET_TIB_LOCALID_NAME);

  InlineFunctionInfo IFI(0, &getAnalysis<DataLayout>());

  // Run through all stub functions and process their kernel delegates.
  //
  OpenCLSymbols &OCLS = getAnalysis<OpenCLSymbols>();
  DominatorTree DomTree;
  DT = &DomTree;
  LoopInfo LInfo;
  LI = &LInfo;
  for (OpenCLSymbols::stubs_iterator I = OCLS.stubs_begin(),
       E = OCLS.stubs_end(); I != E; ++I) {
    Function *F = const_cast<Function *>(*I);

    // Get the kernel delegate function, we will work only on delegates.
    F = M.getFunction(GetKernelDelegateName(F));
    if (!F)
      continue;

    F = addWorkGroupArguments(*F);
    assert(F->hasOneUse() && "Delegate function should have only one use!");

    DT->runOnFunction(*F);
    LI->releaseMemory();
    LI->getBase().Analyze(DT->getBase());

    if (doInitialization(*F))
      runOnFunction(*F);

    // Inline the delegate function, as there is only one call to it from the
    // stub function.
    IFI.reset();
    InlineFunction(cast<CallInst>(*F->use_begin()), IFI);

    // the function is useless, as it is "internal" and no one calls it.
    F->eraseFromParent();
    Changed = true;
  }

  // We do not need to reference the function anymore. Inline it, as it is
  // a one instruction function anyhow.
  //
  if (FGetTIBLocalId) {
    while (!FGetTIBLocalId->use_empty()) {
      IFI.reset();
      InlineFunction(cast<CallInst>(*FGetTIBLocalId->use_begin()), IFI);
    }
    FGetTIBLocalId->eraseFromParent();
    Changed = true;
  }

  return Changed;
}

void AMDWorkGroupLevelExecution::runOnFunction(Function &F)
{
  bool HasBarrier = hasBarrier();
  if (HasBarrier)
    eraseBarrierCalls();

  typedef SmallVector<BarrierRegion *, 16> BRVect;
  BRVect Complex;

  for (BarrierRegionList::iterator I = Regions.begin(), E = Regions.end();
       I != E; ++I) {
    BarrierRegion &BR = *I;

    if (BR.degenerated())
      continue;

    bool Simple = BR.simple();
    if (HasBarrier && Simple)
      preserveWorkGroupInvariantsSimple(BR);

    if (!BR.filler()) {
      emitWorkGroupLoop(BR);

      if (!Simple)
        Complex.push_back(&BR);
    }
  }

  if (!Complex.empty()) {
    DT->runOnFunction(F);
    for (BRVect::iterator I = Complex.begin(), E = Complex.end(); I != E; ++I)
      preserveWorkGroupInvariantsComplex(**I);

    preserveNotDominatedValues(F);
  }

  if (!HasBarrier)
    return;

  // Update the stub's "nature" exported variable, with the PrivateSize. The
  // default is (already) set to 0.
  //
  if (unsigned PrivateSize = preserveWorkGroupVariants()) {
    GlobalVariable *GV =
      F.getParent()->getNamedGlobal(
        AMDSymbolNames::decorateNatureName(
          AMDSymbolNames::undecorateStubFunctionName(
            UndecorateKernelDelegateName(&F))));

    if (!GV)
      return;

    LLVMContext &C = F.getContext();
    ConstantStruct *CNature = cast<ConstantStruct>(GV->getInitializer());
    Constant *CElems[] = {
      CNature->getOperand(0),
      ConstantInt::get(Type::getInt32Ty(C), PrivateSize)
    };
    GV->setInitializer(ConstantStruct::get(CNature->getType(), CElems));
  }
}

bool AMDWorkGroupLevelExecution::doInitialization(Function &F)
{
  initializeWorkGroupValues(F);

  tagBasicBlocks(F);
  if (!calculateBarrierRegions(F) || Regions.empty())
    return false;

  removeLifetimeIntrinsics(F);
  AIWorkItemIdx = new AllocaInst(VWGSize->getType(),
                                 RESERVE_NAME("workitem"),
                                 F.getEntryBlock().begin());

  NonPreservable.clear();
  NonPreservable.insert(AIWorkItemIdx);
  NonPreservable.insert(VWGSize);
  NonPreservable.insert(VPrivatePtr);
  return true;
}

// isolateReturnInstruction - Isolates the return instruction into a single
// basic block.
BasicBlock *AMDWorkGroupLevelExecution::isolateReturnInstruction(Function &F)
{
  BasicBlock *BBReturnIDom = 0;

  // BBReturn - Pointer to an already isolated return instruction in a block
  BasicBlock *BBReturn = 0;

  // ReturnBBs - List of all basic blocks that contain a return instruction
  SmallVector<BasicBlock *, 8> ReturnBBs;

  // Scan all F's basic blocks and add all the ones with return instruction
  // to ReturnBBs.
  // If we find an isolated return instruction in a block on the way, save it
  // in BBReturn.
  //
  for (Function::iterator I = F.begin(), E = F.end(); I != E; ++I) {
    BasicBlock *BB = I;
    if (isa<ReturnInst>(BB->getTerminator())) {
      if (BB->size() == 1 && !BBReturn)
        BBReturn = BB;
      else
        ReturnBBs.push_back(BB);

      if (BBReturnIDom)
        BBReturnIDom = DT->findNearestCommonDominator(BBReturnIDom, BB);
      else
        BBReturnIDom = BB;
    }
  }

  assert((BBReturn || !ReturnBBs.empty()) &&
         "No return instruction found in function");

  // If no isolated return block found, create one.
  //
  if (!BBReturn) {
    LLVMContext &C = F.getContext();
    BBReturn = BasicBlock::Create(C, "isolated.return", &F);
    ReturnInst::Create(C, BBReturn);

    DT->addNewBlock(BBReturn, BBReturnIDom);
  }
  else if (!ReturnBBs.empty()) {
    DT->changeImmediateDominator(BBReturn, BBReturnIDom);
  }

  // Create an edge from each of the blocks in ReturnBBs to BBReturn.
  //
  for (unsigned i = 0, e = ReturnBBs.size(); i < e; ++i) {
    BasicBlock *BB = ReturnBBs[i];
    TerminatorInst *TI = BB->getTerminator();
    TI->eraseFromParent();
    BranchInst::Create(BBReturn, BB);
  }

  return BBReturn;
}

// isolateEntryBlock - Isolates the non initialization instructions in the
// function's entry block into a new basic block, and return the entry.
BasicBlock *AMDWorkGroupLevelExecution::isolateEntryBlock(Function &F)
{
  BasicBlock *BB = &F.getEntryBlock();
  BasicBlock::iterator I = BB->begin();

  // Skip all Alloca instructions, as we only need to do them once, in the
  // function's actual entry block.
  while (isa<AllocaInst>(I))
    ++I;

  // If we reached an unconditional branch to a valid-entry block, return that block.
  if (BranchInst *BI = dyn_cast<BranchInst>(I))
    if (BI->isUnconditional() && BarrierRegion::isValidEntry(*BI->getSuccessor(0)))
      return BB;

  // Sink all the instructions after all the Allocas into a new basic block.
  splitBlock(BB, I)->takeName(BB);
  BB->setName("entry");
  return BB;
}

// isolateExitBlock - Isolates the all exit paths in the function into a single
// exit basic block.
BasicBlock *AMDWorkGroupLevelExecution::isolateExitBlock(Function &F)
{
  BasicBlock *BB = isolateReturnInstruction(F);

  // If there is more than one edge to isolated return block, split it, and make
  // sure it doesn't.
  if (!BarrierRegion::isValidEntry(*BB))
    BB = splitBlock(BB, BB->getTerminator());

  BB->setName("exit");
  return BB;
}

// eraseBarrierCalls - Erase all calls to barrier in the already processed
// function. This requires us to calculate all barrier regions first.
void AMDWorkGroupLevelExecution::eraseBarrierCalls() const
{
  // A call to barrier may be present only in a region's entry, as we split
  // regions on these calls.
  // Another case is when they are present as the first instruction in loose
  // blocks of a barrier region.
  //
  for (BarrierRegionList::const_iterator I = Regions.begin(), E = Regions.end();
       I != E; ++I) {
    const BarrierRegion &BR = *I;
    if (CallInst *CI = dyn_cast<CallInst>(&BR.getEntryBlock().front()))
      if (CI->getCalledFunction() == FBarrier)
        CI->eraseFromParent();

    if (BR.loose_empty())
      continue;

    for (BarrierRegion::const_loose_iterator B = BR.loose_begin(),
         E = BR.loose_end(); B != E; ++B)
      if (CallInst *CI = dyn_cast<CallInst>(&B->Block->front()))
        if (CI->getCalledFunction() == FBarrier)
          CI->eraseFromParent();
  }
}

// emitWorkGroupLoop - Emit code to wrap BR in a loop which changes the
// global/local ID in each iteration. The number of iterations is as the
// WorkGroup size (VWGSize).
void AMDWorkGroupLevelExecution::emitWorkGroupLoop(BarrierRegion &BR)
{
  assert(!isa<PHINode>(BR.getEntryBlock().begin()) &&
         "PHI nodes are not supported.");

  BasicBlock *BBInit = &BR.getEntryBlock();
  BasicBlock *BBEntry = BBInit->splitBasicBlock(BBInit->begin());
  BBEntry->takeName(BBInit);
  BBInit->setName("wgloop.init");

  // Initialize the Iterator
  new StoreInst(ConstantInt::get(VWGSize->getType(), 0), AIWorkItemIdx,
                BBInit->getTerminator());

  DEBUG((dbgs() << "Emit WorkGroup-loop:\n"));
  if (BR.simple()) {
    BasicBlock *BBExit = &BR.getExitBlock();
    if (BBExit == BBInit)
      BBExit = BBEntry;

    DEBUG((dbgs() << "  latch: "));
    emitLatchBlock(*BBExit, *BBEntry);
  }
  else {
    DEBUG((dbgs() << "  switch:\n"));
    emitSwitchBarrierBlocks(BR, *BBEntry);
  }
}

// emitLatchBlock - Emit latch code for the WorkGroup loop. The code is inserted
// into the end of Cond, loop to Body or exit to Cond's single successor.
//
//-------------------------------------------
// Body:
//   ...
// Cond:
//   ...
//   ++WorkItemIdx;
//   if (WorkItemIdx < WGSize) goto Body;
//-------------------------------------------
//
void AMDWorkGroupLevelExecution::emitLatchBlock(BasicBlock &Cond, BasicBlock &Body)
{
  TerminatorInst *TI = Cond.getTerminator();
  BasicBlock *BBTerm = TI->getSuccessor(0);
  TI->eraseFromParent();

  Instruction *I;

  // Increment WorkItemIdx by 1.
  //
  I = new LoadInst(AIWorkItemIdx, 0, &Cond);
  I = BinaryOperator::Create(Instruction::Add,
                             I, ConstantInt::get(I->getType(), 1), "", &Cond);
  new StoreInst(I, AIWorkItemIdx, &Cond);

  // Compare WorkItemIdx with WGSize: If (WorkItemIdx < WGSize) goto Body.
  //
  I = new ICmpInst(Cond, ICmpInst::ICMP_ULT, I, VWGSize);
  BranchInst::Create(&Body, BBTerm, I, &Cond);

  DEBUG((dbgs() <<  "[loop: ", WriteAsOperand(dbgs(), &Body, false),
         dbgs() << ", exit: ", WriteAsOperand(dbgs(), BBTerm, false),
         dbgs() << "]\n"));
}

// emitSwitchBarrierBlocks - Emit WorkGroup loop that handles complex barrier
// regions. Every loose barrier block gets a unique state that identifies it
// in a switch statement that added at the beginning. The state is common for
// all WorkItems, as specified by the OpenCL specifications.
//
//-------------------------------------------
// Entry:
//   int State;
// ...
//
// BR.Entry_Init:
//   State = WGSize;
//   WorkItemIdx = 0;
//
// BR.Entry_Switch:
//   switch(State) {
//   default: goto Default;
//   0: goto BR.loose_0;
//   1: goto BR.loose_1;
//   }
//
// Default:
//   ...
//   ++WorkItemIdx;
//   if (WorkItemIdx < WGSize) goto BR.Entry_Switch;
//   WorkItemIdx = 0;
//   State = 0;
//
// BR.loose_0:
//   ...
//   ++WorkItemIdx;
//   if (WorkItemIdx < WGSize) goto BR.Entry_Switch;
//   WorkItemIdx = 0;
//   State = 1;
//
// BR.loose_1:
//   ...
//   ++WorkItemIdx;
//   if (WorkItemIdx < WGSize) goto BR.Entry_Switch;
//
// BR.Exit:
// ...
//-------------------------------------------
//
void AMDWorkGroupLevelExecution::emitSwitchBarrierBlocks(BarrierRegion &BR,
                                                         BasicBlock &Default)
{
  IntegerType *IntTy = cast<IntegerType>(VWGSize->getType());
  BasicBlock *BBInit = &BR.getEntryBlock();
  BasicBlock *BBExit = &BR.getExitBlock();
  if (BBExit == BBInit)
    BBInit = BBExit->splitBasicBlock(BBExit->getTerminator());

  Instruction *I = BBInit->getTerminator();
  Value *VStatePtr = new AllocaInst(IntTy, RESERVE_NAME("state"), AIWorkItemIdx);
  new StoreInst(ConstantInt::get(IntTy, BR.loose_size()), VStatePtr, I);

  NonPreservable.insert(VStatePtr);

  BasicBlock *BBSwitch = BBInit->splitBasicBlock(I);
  BBSwitch->setName("switch.barriers");
  BBSwitch->getTerminator()->eraseFromParent();
  I = new LoadInst(VStatePtr, 0, BBSwitch);
  SwitchInst *SI = SwitchInst::Create(I, &Default, BR.loose_size(), BBSwitch);

  DEBUG((dbgs() << "  default:", WriteAsOperand(dbgs(), &Default, false),
         dbgs() << "\n"));

  ConstantInt *Zero = ConstantInt::get(IntTy, 0);
  unsigned Idx = 0;
  for (BarrierRegion::loose_iterator B = BR.loose_begin(), E = BR.loose_end();
       B != E; ++B, ++Idx) {
    BasicBlock *BB = B->Block;

    DEBUG((dbgs() << "  case " << Idx << ": "));
    
    BBInit = BB->splitBasicBlock(BB->begin());
    emitLatchBlock(*BB, *BBSwitch);

    ConstantInt *CIdx = ConstantInt::get(IntTy, Idx);
    BB = BBInit->splitBasicBlock(BBInit->begin());

    I = BBInit->getTerminator();
    new StoreInst(Zero, AIWorkItemIdx, I);
    new StoreInst(CIdx, VStatePtr, I);
    SI->addCase(CIdx, BB);
  }

  DEBUG((dbgs() << "  exit latch: "));
  emitLatchBlock(*BBExit, *BBSwitch);
}

// splitBlock - Split the specified block at the specified instruction - every
// thing before SplitPt stays in Old and everything starting with SplitPt moves
// to a new block. The two blocks are joined by an unconditional branch.
//
// Note: This function is similar to llvm::SplitBlock() but it updates the
//       loop info and dominator tree members.
//
BasicBlock *AMDWorkGroupLevelExecution::splitBlock(BasicBlock *Old,
                                                   Instruction *SplitPt,
                                                   const char *Suffix)
{
  if (!Suffix || !Suffix[0])
    Suffix = ".split";
  BasicBlock *New = Old->splitBasicBlock(SplitPt, Old->getName() + Suffix);

  // The new block lives in whichever loop the old one did. This preserves
  // LCSSA as well, because we force the split point to be after any PHI nodes.
  if (Loop *L = LI->getLoopFor(Old))
    L->addBasicBlockToLoop(New, LI->getBase());

  // Old dominates New. New node dominates all other nodes dominated by Old.
  if (DomTreeNode *OldNode = DT->getNode(Old)) {
    SmallVector<DomTreeNode *, 64> Children;
    for (DomTreeNode::iterator I = OldNode->begin(), E = OldNode->end();
         I != E; ++I) 
      Children.push_back(*I);

    DomTreeNode *NewNode = DT->addNewBlock(New,Old);
    for (SmallVector<DomTreeNode *, 64>::iterator I = Children.begin(),
         E = Children.end(); I != E; ++I) 
      DT->changeImmediateDominator(*I, NewNode);
  }

  // Duplicate the Old block's Tag (if exists).
  BlockTag Tag = getBlockTag(*Old);
  if (Tag != UnknownBlock)
    BlockTags[New] = Tag;

  return New;
}
