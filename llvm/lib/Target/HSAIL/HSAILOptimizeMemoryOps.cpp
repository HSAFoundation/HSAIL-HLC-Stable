//===  HSAILOptimizeMemoryOps.cpp - Optimize HSAIL memory operations -*- C++ -*-===//
//===----------------------------------------------------------------------===//
//
// Perform HSAIL memory optimizations:
// 1. Merge scalar loads and stores into their vector equivalents.
//    Pass expects that loads and stores 
//    from same base pointers were glued together
//    during instruction scheduling.
// 2. Eliminate dead stores in swizzles 
//    left after scalarization in instruction selection
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "hsail-memory-opts"

#include "HSAIL.h"
#include "HSAILInstrInfo.h"
#include "HSAILTargetMachine.h"
#include "HSAILSubtarget.h"
#include "HSAILUtilityFunctions.h"

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/LoopInfo.h"

using namespace llvm;

static cl::opt<bool> EnableHsailDSE("hsail-enable-dse", cl::Hidden,
    cl::desc("Enable dead store elimination in HSAIL machine code"),
    cl::init(true));

static cl::opt<bool> EnableMoveKernargs("hsail-enable-move-kernargs", cl::Hidden,
    cl::desc("Move ld_kernargs with one use under condition directly into this condition"),
    cl::init(true));

static cl::opt<bool> EnableVecLdSt("hsail-enable-vec-ld-st", cl::Hidden,
    cl::desc("Enable vector load store in HSAIL machine code"),
    cl::init(true));

static cl::opt<bool> EnableAlignmentInfo("hsail-enable-ld-st-alignment-info", cl::Hidden,
    cl::desc("Enable vector load store in HSAIL machine code"),
    cl::init(true));

namespace 
{
  class HSAILOptimizeMemoryOps: public MachineFunctionPass
  {
  public:
    static char ID;

    explicit HSAILOptimizeMemoryOps(const HSAILTargetMachine &aTM): 
      MachineFunctionPass(ID), TM(aTM) { }

    // Default Constructor required for initiallizing PASS
    explicit HSAILOptimizeMemoryOps():
      MachineFunctionPass(ID), TM(TM) { }

    virtual bool runOnMachineFunction(MachineFunction &F);

    virtual const char *getPassName() const 
    { 
     return "HSAIL memory optimizations";
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<AliasAnalysis>();
      AU.addRequired<LoopInfo>();
      AU.addRequired<PostDominatorTree>();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

  private:
    // Check that load/store memoperands are equal
    // Important: It does not check offsets for equality
    bool IsMemOpsEqual(MachineMemOperand *memop1, MachineMemOperand *memop2);
    
    // Remove redundant vector loads/stores left after scalarization
    bool EliminateDeadStores(MachineFunction &F);

    // Merge loads/stores into their vector equivalents
    bool MergeLoadsStores(MachineFunction &F);
    // Converts scalar load or store opcode to the vector
    unsigned getVectorLdStOpcode(unsigned Opc, unsigned Size, bool &isOk);
    // Add fake operations into worklist in case it is profitable to
    // make wider load
    // Returns true if it performed some changes
    bool ExtendSwizzles(SmallVectorImpl<MachineInstr *> &Worklist,
      MachineRegisterInfo &MRI, MachineFunction &F);

    bool MoveKernargs(MachineFunction &F);

    // Add alignment info for loads and stores 
    // to later use it in _align() modifier
    void EmitAlignmentInfo(MachineFunction &F);

  private:
    const HSAILTargetMachine &TM;
    const HSAILInstrInfo *TII;

    AliasAnalysis *AA;
    PostDominatorTree *PDT;
    LoopInfo *LI;
  };

  char HSAILOptimizeMemoryOps::ID = 0;
}
  
// Register pass in passRegistry so that 
// the pass info gets populated for printing debug info 
INITIALIZE_PASS(HSAILOptimizeMemoryOps, "hsail-memory-opts",
                "HSAIL memory optimizations", false, false)

FunctionPass *llvm::createHSAILOptimizeMemoryOps(const HSAILTargetMachine &TM) 
{
  return new HSAILOptimizeMemoryOps(TM);
}

unsigned HSAILOptimizeMemoryOps::getVectorLdStOpcode(unsigned Opc, 
                                                     unsigned Size,
                                                     bool &isOk)
{
  isOk = true;
  
  #include "HSAILGenVecMap.inc"
 
  isOk = false;
  return 0;
}

bool HSAILOptimizeMemoryOps::IsMemOpsEqual(
  MachineMemOperand *memop1, 
  MachineMemOperand *memop2)
{
  if (memop1->getValue() == NULL ||  memop2->getValue() == NULL) 
  {
    return false;
  }
  if (memop1->getSize() != memop2->getSize() ||
      memop1->getPointerInfo().getAddrSpace() != 
        memop2->getPointerInfo().getAddrSpace() ||
      AA->alias(memop1->getValue(), memop2->getValue()) != 
        AliasAnalysis::MustAlias)
  {
    return false;
  }

  return true;
}

bool HSAILOptimizeMemoryOps::ExtendSwizzles(
  SmallVectorImpl<MachineInstr *> &Worklist,
  MachineRegisterInfo &MRI,
  MachineFunction &MF)
{
  assert(Worklist.size() > 0 && Worklist.size() <= 4);
  
  MachineMemOperand *base_memop = *(Worklist.front()->memoperands_begin());
  Type *memop_type = base_memop->getValue()->getType()->getPointerElementType();
  
  // Check if it's profitable to extend
  // For now assume that _v3 is less effective than _v4
  // but not for 32 bit values - we use secret knowledge that 
  // finalizer will have _load_dwordx3 operations.
  // TODO_HSA: Check memory segment. 
  //           We don't have some operations in finalizer for the group segment.
  if (!(isa<VectorType>(memop_type) && 
        cast<VectorType>(memop_type)->getNumElements() == 4 && 
        Worklist.size() == 3 &&
        memop_type->getScalarSizeInBits() < 32))
    return false;

  // Missing load is in the end of the worklist
  bool end_missing = base_memop->getOffset() == 0;
  // Missing load is in the beginning of the worklist
  bool begin_missing = 
    base_memop->getOffset() * 8 == memop_type->getScalarSizeInBits();

  if (end_missing || begin_missing)
  {
    // Create "fake" load
    MachineInstrBuilder fake_load = BuildMI(
      *Worklist.front()->getParent()->getParent(), 
      Worklist.front()->getDebugLoc(),
      TII->get(Worklist.front()->getOpcode()));

    unsigned fake_reg = MRI.createVirtualRegister(
      MRI.getRegClass(Worklist.front()->getOperand(0).getReg()));
    fake_load.addReg(fake_reg, RegState::Define);

    if (end_missing)
      Worklist.push_back(fake_load);
    else if (begin_missing)
    {
      // Add address and the rest of operands
      for (unsigned i = addressOpNum(Worklist.front()); 
           i < Worklist.front()->getNumOperands(); ++i)
      {
        fake_load.addOperand(Worklist.front()->getOperand(i));
      }      

      // Set up correct offset (one element back)
      assert(getOffset(fake_load).isImm());
      int64_t element_size = memop_type->getScalarSizeInBits() / 8;
      // in instruction operands
      getOffset(fake_load).setImm(
        getOffset(fake_load).getImm() - element_size);
      // in memory operands
      fake_load->addMemOperand(MF, MF.getMachineMemOperand(base_memop, 
        -element_size, base_memop->getSize()));

      Worklist.insert(Worklist.begin(), fake_load);
    }

    return true;
  }

  return false;
}

bool HSAILOptimizeMemoryOps::MergeLoadsStores(MachineFunction &F)
{
  MachineRegisterInfo &MRI = F.getRegInfo();
  bool is_changed = false;
  
  for (MachineFunction::iterator bb_it = F.begin(), bb_end = F.end(); 
       bb_it != bb_end; ++bb_it)
  {
    for (MachineBasicBlock::iterator inst_it = bb_it->begin(), 
         inst_end = bb_it->end(); inst_it != inst_end; /* no increment */)
    {
      SmallVector<MachineInstr*, 4> worklist;
      MachineMemOperand *base_memop = 0;
      const MCInstrDesc *base_mcid = 0;
      MachineInstr *base_inst = 0;
      int64_t base_offset = 0;

      // Compute worklist of loads which can be merged into vectors
      while (inst_it != inst_end && worklist.size() < 4)
      {
        MachineInstr *inst = inst_it;
        
        const MCInstrDesc &MCID = TII->get(inst->getOpcode());
        // Instructions in worklist should have only one mem operand and 
        // all of them should be either mayLoad or mayStore or both
        if (!inst->hasOneMemOperand() || 
            (base_mcid == 0 && !MCID.mayLoad() && !MCID.mayStore()) ||
            (base_mcid != 0 && base_mcid->Flags != MCID.Flags))
        {
          if (worklist.empty())
          {
            ++inst_it;
            continue;
          }
          break;
        }

        if (worklist.empty())
        {
          worklist.push_back(inst_it);
          base_memop = *inst->memoperands_begin();
          base_offset = base_memop->getOffset();
          base_mcid = &MCID;
          base_inst = inst;
          ++inst_it;
          continue;
        }

        // It should be load or store with same segment, type and length
        // as others in worklist
        MachineMemOperand *new_memop = *inst->memoperands_begin();
        if (!IsMemOpsEqual(new_memop, base_memop) ||
            new_memop->getFlags() != base_memop->getFlags())
          break;

        // All registers should have same register size
        bool same_reg_classes = true;
        unsigned inst_op = 0, base_op = 0;
        while (inst_op < inst_it->getNumOperands() && 
               base_op < inst_it->getNumOperands())
        {
          if (!inst_it->getOperand(inst_op).isReg() ||
              inst_it->getOperand(inst_op).getReg() == 0)
          {
            ++inst_op;
            continue;
          }
          if (!base_inst->getOperand(base_op).isReg() ||
              base_inst->getOperand(base_op).getReg() == 0)
          {
            ++base_op;
            continue;
          }

          if (
            MRI.getRegClass(inst_it->getOperand(inst_op).getReg())->getSize() != 
            MRI.getRegClass(base_inst->getOperand(base_op).getReg())->getSize())
          {
            same_reg_classes = false;
            break;
          }
          ++inst_op;
          ++base_op;
        }
        if (!same_reg_classes)
          break;

        // Check that it is exactly next vector element
        int64_t new_offset = new_memop->getOffset();
        unsigned num_ops_between = ((new_offset - base_offset) / 
          new_memop->getSize()) - worklist.size();
        bool not_complete_load = ((new_offset - base_offset) %
          (new_memop->getSize())) != 0;

        if (base_mcid->mayLoad() && 
            num_ops_between <= 4 - worklist.size() - 1 &&
            !not_complete_load)
        {
          // Insert "fake" loads in order to fill gaps in vector
          for (unsigned i = 0; i < num_ops_between; i++)
          {
            MachineInstrBuilder fake_load = BuildMI(
              F, 
              worklist.front()->getDebugLoc(), 
              *base_mcid);

            unsigned fake_reg = MRI.createVirtualRegister(
              MRI.getRegClass(inst_it->getOperand(0).getReg()));
            fake_load.addReg(fake_reg, RegState::Define);

            worklist.push_back(fake_load);
          }
        }
        else if (num_ops_between != 0 || not_complete_load)
          break;

        // Add new item to the worklist
        worklist.push_back(inst_it);
        ++inst_it;
      }

      // Now we are ready to perform merging
      if (worklist.size() <= 1)
        continue;

      // In case if there was swizzles some loads may be already deleted
      // in some case it's better to restore them
      // for example to avoid ld_v3
      // TODO_HSA: Check if it is profitable for stores
      if (!base_mcid->mayStore())
      {
        if (ExtendSwizzles(worklist, MRI, F))
        {
          // It is possible that base instruction has changed
          base_inst = worklist.front();
          base_memop = *base_inst->memoperands_begin();
          base_offset = base_memop->getOffset();
          base_mcid = &TII->get(base_inst->getOpcode());
        }
      }

      DEBUG(dbgs() << "Merging " << worklist.size() << 
        "instructions starting with " << worklist[0] << "\n");

      // Create vector instruction
      bool isOk;
      unsigned vec_opc = 
        getVectorLdStOpcode(base_inst->getOpcode(), 
        worklist.size(), isOk);
      if (!isOk)
        continue;

      MachineInstrBuilder builder = 
        BuildMI(*bb_it, (MachineBasicBlock::iterator) *inst_it, 
                base_inst->getDebugLoc(), TII->get(vec_opc));

      // Insert def registers
      for (unsigned i = 0; i < worklist.size(); ++i)
      {
        builder.addOperand(worklist[i]->getOperand(0));

        MachineOperand &inserted_op = 
          builder->getOperand(builder->getNumOperands() - 1);
        if (inserted_op.isDef())
          inserted_op.setIsEarlyClobber();
      }

      // Insert source address and offset if necessary
      for (unsigned i = 1; i < base_inst->getNumOperands(); ++i)
        builder.addOperand(base_inst->getOperand(i));
      // Construct new correct one memory operand
      builder.addMemOperand(F.getMachineMemOperand(base_memop, 
        0, base_memop->getSize() * worklist.size()));

      // Remove old scalar loads
      for (unsigned i = 0; i < worklist.size(); ++i)
      {
        if (worklist[i]->getParent())
          worklist[i]->eraseFromParent();
        else
          F.DeleteMachineInstr(worklist[i]);
      }
      is_changed = true;
    }
  }

  return is_changed;
}

bool HSAILOptimizeMemoryOps::EliminateDeadStores(MachineFunction &F)
{
  bool ret_code = false;
  
  for (MachineFunction::iterator bb_it = F.begin(), bb_end = F.end(); 
       bb_it != bb_end; ++bb_it)
  {
    // For each store search for it opposite load
    for (MachineBasicBlock::iterator store_it = bb_it->begin(), 
         store_end = bb_it->end(); store_it != store_end; )
    {
      MachineInstr *store_inst = &*store_it++;
      if (!store_inst->getDesc().mayStore())
        continue;

      // Get definition of stored register
      if (store_inst->getNumOperands() < 1)
        continue;
      MachineOperand &src_op = store_inst->getOperand(0);
      if (!src_op.isReg())
        continue;
      MachineInstr *src_inst = 
        F.getRegInfo().getVRegDef(src_op.getReg());
      if (!src_inst)
        continue;

      // Get memory operands
      if (!src_inst->hasOneMemOperand() ||
          !store_inst->hasOneMemOperand())
      {
        continue;
      }

      MachineMemOperand *src_memop = *src_inst->memoperands_begin(), 
        *store_memop = *store_inst->memoperands_begin();

      // Check that we can remove this pair of load and store
      bool saw_store = false;
      if (src_inst->getDesc().mayLoad() &&
          IsMemOpsEqual(src_memop, store_memop) &&
          src_memop->getOffset() == store_memop->getOffset() &&
          src_inst->isSafeToMove(TII, AA, saw_store))
      {
        if (F.getRegInfo().hasOneUse(src_op.getReg()))
        {
          DEBUG(dbgs() << "Removing dead load " << src_inst << "\n");
          src_inst->eraseFromParent();
        }
        DEBUG(dbgs() << "Removing dead store " << store_inst << "\n");
        store_inst->eraseFromParent();
        ret_code = true;
      }
    }
  }
  
  return ret_code;
}

bool HSAILOptimizeMemoryOps::MoveKernargs(MachineFunction &F)
{
  bool made_change = false;
  
  LI = &getAnalysis<LoopInfo>();
  PDT = &getAnalysis<PostDominatorTree>();

  MachineBasicBlock *entry_bb = &F.front();
  if (entry_bb->pred_size() != 0)
    return false; // First basicblock is not an entry to the function

  for (MachineBasicBlock::iterator it = entry_bb->begin(), 
       it_end = entry_bb->end(); it != it_end; )
  {
    MachineInstr *kernarg_inst = &*it++;

    if (!HSAILisKernargInst(TM, kernarg_inst))
      continue;

    // Do not bother with multiple uses
    if (!F.getRegInfo().hasOneUse(kernarg_inst->getOperand(0).getReg()))
      continue;

    // Determine place where to move 
    MachineInstr *user = &*F.getRegInfo().use_begin(
      kernarg_inst->getOperand(0).getReg());
    MachineBasicBlock *new_position = user->getParent();

    if (user->isPHI())
    {
      // Jump to the correct phi predeccessor 
      unsigned i, e;
      for (i = 1, e = user->getNumOperands(); i < e; i += 2)
        if (user->getOperand(i).getReg() == kernarg_inst->getOperand(0).getReg())
        {
          new_position = user->getOperand(i + 1).getMBB();
          break;
        }
      assert(i < e && "Unable to find use in PHI node");
    }

    // Check that user is inside any control flow
    if (PDT->dominates(
          const_cast<BasicBlock*>(new_position->getBasicBlock()),
          const_cast<BasicBlock*>(entry_bb->getBasicBlock())))
    {
      continue;
    }

    // If it is inside loop
    if (LI->getLoopFor(new_position->getBasicBlock()))
      continue;

    // Ok, it's safe to move this kernarg into the begginning of new_position
    kernarg_inst->removeFromParent();
    new_position->insert(new_position->getFirstNonPHI(), kernarg_inst);
    made_change = true;

    // Reset iterator
    it = entry_bb->begin();
  }

  return made_change;
}

void HSAILOptimizeMemoryOps::EmitAlignmentInfo(MachineFunction &F)
{
  for (MachineFunction::iterator bb_it = F.begin(), bb_end = F.end(); 
       bb_it != bb_end; ++bb_it)
  {
    for (MachineBasicBlock::iterator it = bb_it->begin(), 
         end = bb_it->end(); it != end; )
    {
      MachineInstr *ldst_inst = &*it++;
      if (!isLoad(ldst_inst) && !isStore(ldst_inst))
        continue;

      MachineMemOperand *memop = *ldst_inst->memoperands_begin();

      // TODO_HSA: Use AMDAlignmentAnalysis. 
      //           Currently there is some problems with 
      //           llvm pass scheduling and OpenCLSymbols pass
      uint64_t new_alignment = memop->getAlignment();
      uint64_t old_alignment = getLdStAlign(ldst_inst).getImm();
      // Check that we are improving alignment information
      if (new_alignment <= old_alignment || old_alignment == 1)
        // TODO_HSA: Replace 128 with constant from libHSAIL
        getLdStAlign(ldst_inst).setImm(std::min((uint64_t)128, new_alignment));
    }
  }
}

bool HSAILOptimizeMemoryOps::runOnMachineFunction(MachineFunction &F)
{
  bool ret_code = false;

  AA = &getAnalysis<AliasAnalysis>();
  TII = TM.getInstrInfo();

  if (EnableHsailDSE)
    ret_code |= EliminateDeadStores(F);
  if (EnableVecLdSt)
    ret_code |= MergeLoadsStores(F);
  if (EnableMoveKernargs)
    ret_code |= MoveKernargs(F);
  if (EnableAlignmentInfo)
    EmitAlignmentInfo(F);

  return ret_code;
}
