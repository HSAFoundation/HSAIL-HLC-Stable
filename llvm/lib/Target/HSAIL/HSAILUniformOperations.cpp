#define DEBUG_TYPE "hsail-uniform-ops"

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Module.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "HSAILUtilityFunctions.h"
#include "HSAILUniformOperations.h"


using namespace llvm;

namespace llvm {
  unsigned UniformOperationsAnalysisDepth;

  cl::opt<unsigned, true> UniformOpsAnalysisDepth(
    "hsail-uniform-ops-depth", cl::Hidden,
    cl::desc("Uniform operation analysis depth treshold"),
    cl::location(UniformOperationsAnalysisDepth),
    cl::init(20));
}

char HSAILUniformOperations::ID = 0;
INITIALIZE_PASS_BEGIN(HSAILUniformOperations,
  "hsail-uniform-operations", "HSAIL uniform operations", false, false)
  INITIALIZE_PASS_DEPENDENCY(DominatorTree)
  INITIALIZE_PASS_DEPENDENCY(LoopInfo)
INITIALIZE_PASS_END(HSAILUniformOperations,
  "hsail-uniform-operations", "HSAIL uniform operations", false, false)

FunctionPass *
llvm::createHSAILUniformOperations(const HSAILTargetMachine &TM)
{
  return new HSAILUniformOperations(TM);
}

namespace llvm
{

  bool HSAILUniformOperations::runOnMachineFunction( MachineFunction& F )
  {
    bool result = false;
    DEBUG(F.viewCFG());

    TII = TM.getInstrInfo();
    CDA = &getAnalysis<HSAILControlDependencyAnalysis>();
    DEBUG(CDA->dump());
    LI = &getAnalysis<LoopInfo>();

    widthDefMap.clear();
    constLoads.clear();
    storeFront.clear();

    buildStoreFront(F);
    for (MachineFunction::iterator bb_it = F.begin(), bb_end = F.end();
         bb_it != bb_end; ++bb_it)
    {
      for (MachineBasicBlock::iterator inst_it = bb_it->begin(),
           inst_end = bb_it->end(); inst_it != inst_end; )
      {
        MachineInstr * currInstr = &*inst_it++;

        if(
          isLoad(currInstr)             &&
          currInstr->hasOneMemOperand() &&
          !currInstr->hasOrderedMemoryRef()
          )
        {
          SmallPtrSet<MachineInstr *, 32> walk;
          Brig::BrigWidth currWidth = processLoad(F, currInstr, walk);
          if ( currWidth != Brig::BRIG_WIDTH_1 ) {
            if (!constLoads.count(currInstr)) {
              if( !hasStoreAtThePath(F, currInstr) &&
                  isKernelFunc(F.getFunction()) ) // A store may be in a caller
              {
                constLoads.insert(currInstr);
              }
            }

            getWidth(currInstr).setImm(currWidth);
            DEBUG(dbgs() << "\nWIDTH " << currWidth << " BB#" <<
            currInstr->getParent()->getNumber() << " : INST: " <<
            *currInstr << "\n");
          }
          result = result || (currWidth != Brig::BRIG_WIDTH_1 );
        }
      }
    }
    SmallPtrSet<MachineInstr*, 32>::iterator CI = constLoads.begin();
    SmallPtrSet<MachineInstr*, 32>::iterator CE = constLoads.end();
    for(;CI!=CE;CI++)
    {
      MachineInstr * MI = *CI;
      getLoadConstQual(MI).setImm(true);
    }

    return result;
  }

bool
HSAILUniformOperations::IsConstBase( MachineFunction &F, MachineOperand &MO )
{
  StringRef varName = "";
  if ( MO.isGlobal() ) {
    varName = MO.getGlobal()->getName();
    if ( !varName.empty() ) {
      GlobalVariable * GV1 =
        F.getFunction()->getParent()->getNamedGlobal(varName);
      if (GV1 && GV1->isConstant() ) {
        DEBUG(dbgs() << "Const load base global: " << varName << "\n");
        return true;
      }
    }
  } else if ( MO.isSymbol() ) {
    varName = MO.getSymbolName();
    HSAILMachineFunctionInfo * MFI = F.getInfo<HSAILMachineFunctionInfo>();
    kernel_md_iterator KMI = MFI->kernel_md_begin();
    kernel_md_iterator KME = MFI->kernel_md_end();
    for(;KMI != KME;++KMI)
    {
      if(KMI->find(varName) != std::string::npos ) {
        if( 0 == KMI->compare(0, 7, "argmap:") ) {
          const Function * Func = F.getFunction();
          Function::const_arg_iterator AI = Func->arg_begin();
          Function::const_arg_iterator AE = Func->arg_end();
          for(;AI!=AE;++AI)
          {
            std::string s = AI->getName().str() + ":";
            
            if( 0 == KMI->compare( 7, s.length(), s ) ) {
              if ( MFI->isConstantArgument(AI) ) {
                const Type* type = AI->getType();
                if ( type->isPointerTy() ) {
                  DEBUG(dbgs() << "Const load base argument: " << varName << "\n");
                  return true;
                }
              }
            }
          }
        }
      }
    }
  }
  return false;
}


  Brig::BrigWidth
  HSAILUniformOperations::processLoad(MachineFunction & F, MachineInstr * currInstr, SmallPtrSet<MachineInstr *, 32> /* BY VALUE */walk)
  {
    bool isLoop = false;
    Brig::BrigWidth currWidth = getAddrspaceWidth(currInstr);
    if ( currWidth != Brig::BRIG_WIDTH_1 ) {
      currWidth =  WalkUseDefTree(F, getIndex(currInstr),
        currInstr, /* BY REF */walk, currWidth, isLoop, 0);
      widthDefMap[currInstr] = currWidth;
    }
    return currWidth;
  }

  Brig::BrigWidth
  HSAILUniformOperations::getAddrspaceWidth(
    MachineInstr * inst
  )
  {
    assert(inst->hasOneMemOperand());
    MachineInstr::mmo_iterator mmoIt = inst->memoperands_begin();
    MachineMemOperand* mmo = *mmoIt;
    HSAILAS::AddressSpaces addrspace =
      (HSAILAS::AddressSpaces)mmo->getPointerInfo().getAddrSpace();
    switch(addrspace) {
    case HSAILAS::GROUP_ADDRESS:
      return Brig::BRIG_WIDTH_WAVESIZE;
    case HSAILAS::GLOBAL_ADDRESS:
      return Brig::BRIG_WIDTH_ALL;
    case HSAILAS::CONSTANT_ADDRESS:
    case HSAILAS::KERNARG_ADDRESS:
      constLoads.insert(inst);
      return Brig::BRIG_WIDTH_ALL;
    default:
      return Brig::BRIG_WIDTH_1;
    }
  }

  Brig::BrigWidth
  HSAILUniformOperations::WalkUseDefTree(
    MachineFunction& F,
    MachineOperand& MO,
    MachineInstr * currInstr,
    SmallPtrSet<MachineInstr *, 32> & walk,
    Brig::BrigWidth currWidth,
    bool & isLoop,
    unsigned depth
  )
  {
    if ( depth > UniformOperationsAnalysisDepth ) {
      return Brig::BRIG_WIDTH_1;
    }

    if (!MO.isReg()) {
      if (isLoad(currInstr) && IsConstBase(F, MO))
        constLoads.insert(currInstr);
      return currWidth;
    }

    MachineInstr * def = F.getRegInfo().getVRegDef(MO.getReg());

    if(def) {
      if ( widthDefMap.count(def) ) {
        return widthDefMap[def];
      }
      if ( walk.count(def) ) {
        // we have a loop so just skip and keep the current value unchanged
        isLoop = true;
        return currWidth;
      }

      /* add curent def with the initial width
         this is necessary to mark current def as 'visited' to prevent
         infinite loop.
         getCFWidth uses WalkUseDefTree to calculate DU-width of the
         branch condition
       */
      walk.insert(def);

      if(isLoad(def))
        return min(currWidth, processLoad(F, def, walk));

      currWidth = getInstrWidth(def, currWidth);

      bool isPhi = def->isPHI();
      if ( isPhi ) {
        // take care of the point where merged values did diverge.
        currWidth = min(analysePHI(F,def,currInstr, 
          /* BY REF */walk, isLoop,depth), currWidth);
      }

      for(unsigned int i=1;
          i < def->getNumOperands() && (currWidth != Brig::BRIG_WIDTH_1);
      i+=1+isPhi)
      {
        bool save = isLoop;
        currWidth = min( currWidth ,
           WalkUseDefTree(F, def->getOperand(i),
           currInstr, walk, currWidth, isLoop, depth + 1));
        isLoop = save || isLoop;
      }
      if ( !isLoop ) {
        widthDefMap[def] = currWidth;
      }
    }
    return currWidth;
  }

  Brig::BrigWidth
  HSAILUniformOperations::getInstrWidth(
    MachineInstr * inst,
    Brig::BrigWidth currWidth
  )
  {
    switch(inst->getOpcode()) {
        case HSAIL::get_global_id_i:
        case HSAIL::get_workitemid_flat:
        case HSAIL::get_local_id_i:
        case HSAIL::get_lane_id:
        case HSAIL::get_dynwave_id:
            return Brig::BRIG_WIDTH_1;
        case HSAIL::get_group_id_i:
        case HSAIL::get_cu:
            return Brig::BRIG_WIDTH_WAVESIZE;
      }

      // TODO_HSA: Process uniform call values.
      //           Return arguments are actually can have uniform values as
      //           long as the call target is uniform, has readnone attribute,
      //           and all its arguments are uniform.

      // HSA TODO: uncomment as soon call support is ready
      //if ( inst->isCall())
      //{
      //  const GlobalValue * G = inst->getOperand(0).getGlobal();
      //  const Function * calleeFunc = static_cast<const Function*>(G);
      //
      //  if ( !calleeFunc || !( calleeFunc->hasFnAttr(Attribute::NoUnwind) &&
      //         (
      //           calleeFunc->hasFnAttr(Attribute::ReadNone) ||
      //           calleeFunc->hasFnAttr(Attribute::ReadOnly)
      //         )
      //       )
      //   ) {
      //  return Brig::BRIG_WIDTH_1;
      //}
      //}
      
      // HSA_TODO:  To be able to perform interprocedural
      // analysis it's necessary to compute and cache width attribute
      // for the function returns. So, any time we encounter function 
      // call as a definition we immediately know if it is uniform.
      //inst->getOpcode() == HSAIL::arg_ret_u32 ||
      //inst->getOpcode() == HSAIL::arg_ret_u64 ||
      //inst->getOpcode() == HSAIL::arg_ret_f32 ||
      //inst->getOpcode() == HSAIL::arg_ret_f64

      return currWidth;
  }

  void
  HSAILUniformOperations::getMBBVectorForBB(
    MachineFunction  &  F,
    const BasicBlock * bb,
    SmallVector<MachineBasicBlock*, 4> * mbbs
  )
  {
    for(MachineFunction::iterator i = F.begin(); i != F.end(); ++i) {
      if ( i->getBasicBlock() == bb) {
        mbbs->push_back(i);
      }
    }
  }


  Brig::BrigWidth
  HSAILUniformOperations::getCFWidth(
    MachineFunction & F,
    const BasicBlock * BB,
    SmallPtrSet<MachineInstr *, 32>& walk,
    bool & isLoop,
    unsigned depth
  )
  {
    Brig::BrigWidth result = Brig::BRIG_WIDTH_ALL;
    typedef SmallVector<MachineBasicBlock *, 4> mbblist;
    mbblist mbbs;
    getMBBVectorForBB(F, BB, &mbbs);
    for(mbblist::const_iterator I = mbbs.begin(), E = mbbs.end(); I != E; ++I)
    {
      MachineBasicBlock * MBB = *I;
      MachineBasicBlock::iterator termIt = MBB->getFirstTerminator();
      MachineInstr * term = termIt;
      if ( term->isConditionalBranch() ) {
        if ( widthDefMap.count(term)) {
          return widthDefMap[term];
        }
        RET_1(( result = min(result,
           WalkUseDefTree(F, term->getOperand(0),
           term, /* BY REF */walk, result, isLoop, depth + 1))
          )
        );
        if ( !isLoop ) {
          widthDefMap[term] = result;
        }
      }
    }
    return result;
  }

  bool
  HSAILUniformOperations::hasUseOutsideLoop(MachineFunction & F, 
  MachineInstr * load, const BasicBlock * lh)
  {
    bool analyzeCD = true;
    Loop * li = LI->getLoopFor(load->getParent()->getBasicBlock());
    Loop * currLoop = LI->getLoopFor(lh);
    if ( li == currLoop ) // PHI ( loop header ) belongs same loop as load
    {
      // We'd not check control dependencies unless 
      // the load result is live outside the loop
      analyzeCD = false; 
      MachineRegisterInfo::use_iterator U  = 
      F.getRegInfo().use_begin(load->getOperand(0).getReg());
      for( ;!analyzeCD && !U.atEnd(); ++U )
        if(!currLoop->contains(U->getParent()->getBasicBlock()))
          analyzeCD = true;
    }
    return analyzeCD;
  }

  Brig::BrigWidth
  HSAILUniformOperations::analysePHI(
    MachineFunction& F,
    MachineInstr * phi,
    MachineInstr * currInstr,
    SmallPtrSet<MachineInstr *, 32>& walk,
    bool & isLoop,
    unsigned depth
  )
  {
    Brig::BrigWidth result = Brig::BRIG_WIDTH_ALL;
    DT = &getAnalysis<DominatorTree>();
    SetVector<const BasicBlock *> visited;
    const BasicBlock * join = phi->getParent()->getBasicBlock();
    // <result> = phi <ty> [ <val0>, <label0> ], [ <val1>, <label1> ], ...
    // where <valN> is a value that gets merged
    // and <labelN> is the BB the value originates from.
    for(unsigned i = 1; i < phi->getNumOperands(); i+=2 )
    {
      MachineOperand inVal = phi->getOperand(i);
      assert(inVal.isReg());
      MachineInstr * inValueDef = F.getRegInfo().getVRegDef(inVal.getReg());
      const BasicBlock * src = inValueDef->getParent()->getBasicBlock();
      if( !DT->properlyDominates(src, join))
      {
        const HSAILControlDependencyAnalysis::PDFSet * joinCDlist = CDA->getPDF(join);
        const HSAILControlDependencyAnalysis::PDFSet * srcCDlist = CDA->getPDF(src);
        HSAILControlDependencyAnalysis::PDFIterator BI = srcCDlist->begin();
        HSAILControlDependencyAnalysis::PDFIterator BE = srcCDlist->end();
        for(; BI != BE; ++BI)
        {
          if ( !visited.count(*BI) ) {
            
            if ( !joinCDlist->count(*BI) || // Don't care of the common CDs 
                                            // except the backedge
                 ( DT->dominates(join, *BI) /* is loop */ && 
                   isLoad(currInstr)                      && 
                   hasUseOutsideLoop(F, currInstr, join) // check if the load
                   // belongs to current loop and if so, 
                   // does it have a any use outside the loop    
                 )  
            )
            {
              RET_1(( result = min( result, getCFWidth(F, *BI, walk, isLoop, depth)) ));
            }
            visited.insert(*BI);
          }
        }
      }
    }
    Loop * L = LI->getLoopFor(join);
    if(L && LI->isLoopHeader(const_cast<BasicBlock*>(join)))
    {
      SmallVector<BasicBlock*,4> exits;
      L->getExitingBlocks(exits);
      if(exits.size() > 1 ) { // multiple exit loops
        // make reverse CFG irreducible
        // we unable to rely on results
        // from the control dependency analysis
        // so we have to check all exits.
        SmallVector<BasicBlock*,4> ::const_iterator E = exits.begin();
        SmallVector<BasicBlock*,4> ::const_iterator EE = exits.end();
        for(;E!=EE;++E)
        {
          RET_1(( result = min( result, getCFWidth(F, *E, walk, isLoop, depth)) ));
        }
      }
    }
    return result;
  }

  // HSA_TODO: It may be profitable to change the algorithm
  // to collect all const loads in one initial DFS.
  void
  HSAILUniformOperations::buildStoreFront(MachineFunction & F)
  {
    for (MachineFunction::iterator bb_it = F.begin(), bb_end = F.end();
         bb_it != bb_end; ++bb_it)
    {
      for (MachineBasicBlock::iterator inst_it = bb_it->begin(),
           inst_end = bb_it->end(); inst_it != inst_end; inst_it++ )
      {
        MachineInstr * currInstr = inst_it;
        if ( 
          currInstr->mayStore() ||
          // for now we consider each call as possible memory write
          // further we can inspect FunctionAttributes
          // for that the analysis should operate on the Instr 
          // not MachineInstr level
          currInstr->getOpcode() == HSAIL::target_call 
        ) {
          storeFront.insert(std::make_pair(currInstr->getParent(), currInstr));
          break;
        }
      }
    }
  }

  bool
  HSAILUniformOperations::WalkCFGUntilStore(
    MachineBasicBlock * runner,
    SmallPtrSet<MachineBasicBlock *, 32> & visited
  )
  {
    MachineBasicBlock::const_pred_iterator P = runner->pred_begin();
    MachineBasicBlock::const_pred_iterator E = runner->pred_end();
    for( ;P != E; ++P )
    {
      if ( visited.count(*P))  return false;
      if ( storeFront.count(*P) )
      {
        return true;
      }
      visited.insert(*P);
      return WalkCFGUntilStore(*P, visited);
    }
    return false;
  }

  bool
  HSAILUniformOperations::hasStoreAtThePath(
    MachineFunction & F,
    MachineInstr * inst)
  {
    MachineBasicBlock * mbb = inst->getParent();
    if ( storeFront.count(mbb)) {
      MachineInstr * sentinel = mbb->getFirstNonPHI();
      MachineInstr * store = storeFront[mbb];
      MachineBasicBlock::reverse_instr_iterator it(inst);
      MachineBasicBlock::reverse_instr_iterator end(sentinel);
      MachineBasicBlock::reverse_instr_iterator found(store);

      while( it!=found && it!=end ) ++it;

      if(it==found) return true;
    }
    SmallPtrSet<MachineBasicBlock *, 32> visited;
    visited.insert(mbb);
    return WalkCFGUntilStore(mbb, visited);
  }
}
