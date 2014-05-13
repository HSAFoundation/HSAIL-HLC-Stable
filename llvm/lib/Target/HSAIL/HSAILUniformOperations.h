#ifndef HSAIL_UNIFORM_OPERATIONS_H
#define HSAIL_UNIFORM_OPERATIONS_H

#include "HSAIL.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "libHSAIL/Brig.h"
#include "HSAILInstrInfo.h"
#include "HSAILTargetMachine.h"
#include "HSAILControlDependencyAnalysis.h"

#define RET_1(x)\
  if ( Brig::BRIG_WIDTH_1 == x ) {\
    return x;\
  }

Brig::BrigWidth min(Brig::BrigWidth left, Brig::BrigWidth right)
{
  return left < right? left  : right;
}

namespace llvm {

  class HSAILUniformOperations : public MachineFunctionPass
  {
    public:

      static char ID;

      explicit HSAILUniformOperations() :
      MachineFunctionPass(ID), TM(TM) {};

      explicit HSAILUniformOperations(const HSAILTargetMachine& aTM) :
      MachineFunctionPass(ID), TM(aTM), CDA(NULL) {};

      virtual bool runOnMachineFunction(MachineFunction & );

      virtual const char * getPassName() const
      {
        return "HSAIL uniform operations";
      }

      virtual void getAnalysisUsage(AnalysisUsage & AU) const
      {
        AU.addRequired<DominatorTree>();
        AU.addRequired<LoopInfo>();
        AU.addRequired<HSAILControlDependencyAnalysis>();
        MachineFunctionPass::getAnalysisUsage(AU);
      }

    private:

      const HSAILTargetMachine &TM;
      const HSAILInstrInfo *TII;

      bool IsConstBase( MachineFunction &F, MachineOperand &MO );
      bool hasUseOutsideLoop(MachineFunction &, MachineInstr *, const BasicBlock *);

      Brig::BrigWidth getAddrspaceWidth( MachineInstr * );
      Brig::BrigWidth WalkUseDefTree(MachineFunction &, 
		      MachineOperand&, MachineInstr *, SmallPtrSet<MachineInstr *, 32>&, 
        Brig::BrigWidth, bool &, unsigned );
      Brig::BrigWidth getInstrWidth(MachineInstr *, Brig::BrigWidth );
      Brig::BrigWidth getCFWidth(MachineFunction &, const BasicBlock *, 
		      SmallPtrSet<MachineInstr *, 32>&, bool &, unsigned );
      Brig::BrigWidth analysePHI(MachineFunction &,  
		      MachineInstr *,  MachineInstr *, SmallPtrSet<MachineInstr *, 32>&, bool &, unsigned );
      
      Brig::BrigWidth processLoad(MachineFunction &,  MachineInstr *, 
        SmallPtrSet<MachineInstr *, 32> /* BY VALUE! */);

      void getMBBVectorForBB(MachineFunction &, const BasicBlock *, 
		      SmallVector<MachineBasicBlock*, 4> * );
      void buildStoreFront(MachineFunction &);

      DenseMap<MachineInstr *, Brig::BrigWidth> widthDefMap;
      SmallPtrSet<MachineInstr *, 32> constLoads;
      DenseMap<MachineBasicBlock *, MachineInstr *> storeFront;
      bool hasStoreAtThePath(MachineFunction &, MachineInstr *);
      bool WalkCFGUntilStore(MachineBasicBlock *, 
		      SmallPtrSet<MachineBasicBlock *, 32>&);

      DominatorTree * DT;
      HSAILControlDependencyAnalysis * CDA;
      LoopInfo * LI;
  };
}

#endif // HSAIL_UNIFORM_OPERATIONS_H
