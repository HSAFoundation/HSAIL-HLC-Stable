
#include "HSAIL.h"
#include "HSAILTargetMachine.h"
#include "HSAILSubtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Analysis/PostDominators.h"
#include "HSAILControlDependencyAnalysis.h"

using namespace llvm;

char HSAILControlDependencyAnalysis::ID = 0;

INITIALIZE_PASS_BEGIN(HSAILControlDependencyAnalysis,
                "hsail-control-dependency-analysis",
                "HSAIL control dependency analysis", false, false)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTree)
INITIALIZE_PASS_END(HSAILControlDependencyAnalysis,
                "hsail-control-dependency-analysis",
                "HSAIL control dependency analysis", false, false)

FunctionPass *llvm::createHSAILControlDependencyAnalysis()
{
  return new HSAILControlDependencyAnalysis();
}

namespace llvm {

  void HSAILControlDependencyAnalysis::dump()
  {
    for( PDFMapIterator I = PDF.begin(), E = PDF.end(); I != E; I++) {
      dbgs() << "PDF for BB < " << I->first->getName() << " > : [ ";
      for(PDFIterator J = I->second.begin(), JE = I->second.end();
          J != JE; J++ ) {
        dbgs() << " < " << (*J)->getName() << " > ";
      }
      dbgs() << " ]\n";
    }
  }

  bool
  HSAILControlDependencyAnalysis::runOnMachineFunction(MachineFunction &F)
  {
    PDF.clear(); // clear previous results
    PDT = &getAnalysis<PostDominatorTree>();

    // Compute Post Dominance Frontiers for each BB using iterative algorithm
    // introduced by Keith D. Cooper & Co
    // Google for "A Simple, Fast Dominance Algorith"
    // by Keith D. Cooper, Timothy J. Harvey, and Ken Kennedy for details
    // It is prooven to be of [ O(n^2) ] complexity.
    // It much simplier than the canonical Lenauer-Tarjan's approach.
    // We tune it to compute Post Dominance by using Post Dominator Tree.
    for(MachineFunction::const_iterator I = F.begin(), E = F.end(); I != E; I++)
    {
      const MachineBasicBlock * B = I;
      if ( B->succ_size() > 1 ) // branch point
      {
        for( MachineBasicBlock::const_succ_iterator S = B->succ_begin(),
             ES = B->succ_end(); S != ES; S++ )
        {
          DomTreeNode * runner =
            PDT->getNode(const_cast<BasicBlock*>((*S)->getBasicBlock()));
          DomTreeNode * sentinel =
            PDT->getNode(const_cast<BasicBlock*>(B->getBasicBlock()))->getIDom();
          while ( runner != sentinel ) {
            PDF[runner->getBlock()].insert(B->getBasicBlock());
            runner = runner->getIDom();
          }
        }
      }
    }
 
    return true;
  }

}
