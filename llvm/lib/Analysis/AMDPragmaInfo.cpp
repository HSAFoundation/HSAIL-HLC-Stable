#define DEBUG_TYPE "looppragma"
#include <llvm/Support/Debug.h>
#include "llvm/ADT/GraphTraits.h"
#include <llvm/Instructions.h>
#include <llvm/BasicBlock.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/AMDPragmaInfo.h>

using namespace llvm;

LoopPragmaInfo::LoopPragmaInfo (MDNode *LPIM)
  : Flags(0U),
    IsFlagsChanged(false),
    UnrollCount(0U)
{
  ConstantInt *VC;
  MDString    *MDS;

  // SoureceFileName
  if ((MDS = dyn_cast_or_null<MDString>(LPIM->getOperand(0)))) {
    SourceFileName = MDS->getString();
  }

  // LoopLineNo
  VC = dyn_cast<ConstantInt>(LPIM->getOperand(1));
  LoopLineNo = (unsigned)VC->getZExtValue();

  // Flags
  VC = dyn_cast<ConstantInt>(LPIM->getOperand(2));
  Flags = (unsigned)VC->getZExtValue();

  // UnrollCount
  VC = dyn_cast<ConstantInt>(LPIM->getOperand(3));
  UnrollCount = (unsigned)VC->getZExtValue();

  if (!getSourceFileName().empty() && (getLoopLineNo() > 0)) {
    DEBUG(dbgs() << "\nLoop Pragmas for Loop at "
                 << getSourceFileName().str()
                 << ":"
                 << getLoopLineNo() << "\n");
  } else {
    DEBUG(dbgs() << "Loop Pragmas \n");
  }
  if (hasPragmaUnrollCount()) {
    DEBUG(dbgs() << "  Pragma:  #pragma unroll ("
                 << getPragmaUnrollCount() << ")\n");
  }
}

/*
  Write out the LoopPragmaInfo by attaching it to the
  first back-edge branch.
*/
void LoopPragmaInfo::setMetadata(Loop *TheLoop)
{
  // H may not be NULL. However, since we invoke this function
  // when deleting the loop, the loop may be invalid already !
  BasicBlock *H = TheLoop->getHeader();
  if (!H || 
      (TheLoop->block_begin() == TheLoop->block_end())) {
    return;
  }
  
  BranchInst *BranchI = dyn_cast<BranchInst>(H->getTerminator());
  if (!BranchI) {
    return;
  }

  // Create a new LoopPragmaInfo metadata
  LLVMContext& context = H->getContext();
  IntegerType *int32ty = Type::getInt32Ty(context);
  Value* MDNode_oprands[4];
  MDNode_oprands[0] = MDString::get(context, getSourceFileName());
  MDNode_oprands[1] = ConstantInt::get(int32ty, getLoopLineNo());
  MDNode_oprands[2] = ConstantInt::get(int32ty, Flags);
  MDNode_oprands[3] = ConstantInt::get(int32ty, getPragmaUnrollCount());

  MDNode* LPIM = MDNode::get(context, ArrayRef<Value*>(MDNode_oprands, 4));
  BranchI->setMetadata("LoopPragmaInfo", LPIM);
}

void LoopPragmaInfo::initLoopPragmaInfo(Loop *TheLoop)
{
  // Find out whether there is a LoopPragmaInfo metadata. If we have
  // it, it should be attached to a branch of the loop's header
  MDNode *LPIM = NULL;
  BasicBlock *H = TheLoop->getHeader();

  BranchInst *BranchI = dyn_cast<BranchInst>(H->getTerminator());
  if (BranchI) {
    LPIM = BranchI->getMetadata("LoopPragmaInfo");
  }
  
  if (LPIM) {
    TheLoop->setPragmaInfo(new LoopPragmaInfo(LPIM));

#if defined(RELEASE)
    // Remove the original metadata
    BranchI->setMetadata("LoopPragmaInfo", NULL);
#endif
  }

  for (Loop::iterator I = TheLoop->begin(), E = TheLoop->end(); I != E; ++I) {
     initLoopPragmaInfo(*I);
  }
}

