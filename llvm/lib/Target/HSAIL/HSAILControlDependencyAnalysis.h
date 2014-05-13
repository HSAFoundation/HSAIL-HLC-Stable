#ifndef LLVM_ANALYSIS_CONTROL_DEPS_ANALYSIS_H
#define LLVM_ANALYSIS_CONTROL_DEPS_ANALYSIS_H

#include "llvm/Analysis/PostDominators.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SetVector.h"

namespace llvm {  

  class HSAILControlDependencyAnalysis : public MachineFunctionPass {

  public:

    static char ID;

    // Default Constructor required for initiallizing PASS
    explicit HSAILControlDependencyAnalysis():
      MachineFunctionPass(ID) { }

    virtual bool runOnMachineFunction(MachineFunction &F);

    virtual const char *getPassName() const 
    { 
     return "HSAIL control dependency analysis";
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<PostDominatorTree>();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

    void dump();

    typedef SetVector<const BasicBlock *> PDFSet;
    typedef PDFSet::const_iterator PDFIterator;

    const PDFSet * getPDF( const BasicBlock * mbb )
    {
      return &PDF[mbb];
    }
 
  private:

    typedef DenseMap<const BasicBlock *, PDFSet > PDFMap;
    typedef PDFMap::iterator PDFMapIterator;

    typedef SmallVector<MachineBasicBlock*, 2> BlockList;
    typedef BlockList::const_iterator BlockIterator;

    PostDominatorTree *PDT;
    PDFMap PDF;
  
  };
}
#endif //LLVM_ANALYSIS_CONTROL_DEPS_ANALYSIS_H
