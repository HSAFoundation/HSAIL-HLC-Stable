//=== HSAILResizeLocalPointerPass.cpp -*- -*-====//
//===----------------------------------------------------------------------===//
// A pass that replaces 64-bit addresses to 32-bit addresses for local memory
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "64to32"

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
#include <set>

using namespace llvm;

static cl::opt<bool> EnableHsail64Or4to8("hsail-resize-local-pointers",
  cl::Hidden,
  cl::desc("HSAIL: enable 32-bit local memory pointers in 64-bit code"),
  cl::init(true));

namespace {
  class HSAILResizeLocalPointer : public MachineFunctionPass {
  public:
    static char ID;
    explicit HSAILResizeLocalPointer(const HSAILTargetMachine &aTM) :
      MachineFunctionPass(ID), TM(aTM) {}
    explicit HSAILResizeLocalPointer() : MachineFunctionPass(ID), TM(TM) { }
    virtual bool runOnMachineFunction(MachineFunction &F);
    virtual const char* getPassName() const {
      return "64Or4To8 HSAIL resize pointer pass";
    }
  private:
    bool verifySupportedInstruction(
                    const MachineInstr* mi);
    bool populateLocalPointerArithmetic(MachineFunction &F,
                std::set<MachineInstr*>& addr_insts);
    bool resizePointerArithmeticInWorklist(MachineFunction& F,
                std::set<MachineInstr*>& addr_insts);
    const HSAILTargetMachine &TM;
  };
  char HSAILResizeLocalPointer::ID = 0;

}

INITIALIZE_PASS (HSAILResizeLocalPointer, "hsail-resize-local-pointers",
"Replace 32-bit for 64-bit pointers for local memory accesses in HSAIL-64",
false, false)

FunctionPass *llvm::createHSAILResizeLocalPointer(
    const HSAILTargetMachine &TM) {
  return new HSAILResizeLocalPointer(TM);
}

bool HSAILResizeLocalPointer::verifySupportedInstruction(
    const MachineInstr* mi) {
  /* We only support if following condition is satisfied
   only address space for group/private memory 
   (ex: for the __local keyword in OpenCL) is supported at this
   time. Only standard forms of ld/st are supported at this time.
   support for other instructions will follow.
   */
  if (mi->getDesc().mayLoad() || mi->getDesc().mayStore()) {
    if (mi->hasOneMemOperand()) {
      MachineMemOperand *memop = *mi->memoperands_begin();
      const int miOpCode = mi->getOpcode();
      if (HSAILAS::GROUP_ADDRESS == memop->getPointerInfo().getAddrSpace()) {
        if ((miOpCode == HSAIL::group_ld_u32_v1 ||
            miOpCode == HSAIL::group_ld_u64_v1 ||
            miOpCode == HSAIL::group_ld_f32_v1 ||
            miOpCode == HSAIL::group_ld_f64_v1 ||
            miOpCode == HSAIL::group_st_u32_v1 ||
            miOpCode == HSAIL::group_st_u64_v1 ||
            miOpCode == HSAIL::group_st_f32_v1 ||
            miOpCode == HSAIL::group_st_f64_v1) ||
            (miOpCode == HSAIL::group_ext_ld_u32_u16_v1 ||
            miOpCode == HSAIL::group_ext_ld_u32_u8_v1 ||
            miOpCode == HSAIL::group_sext_ld_s32_s16_v1 ||
            miOpCode == HSAIL::group_sext_ld_s32_s8_v1 ||
            miOpCode == HSAIL::group_zext_ld_u32_u16_v1 ||
            miOpCode == HSAIL::group_zext_ld_u32_u8_v1) ||
            (miOpCode == HSAIL::group_truncst_u32_u16_v1 ||
            miOpCode == HSAIL::group_truncst_u32_u8_v1) ||
            (miOpCode == HSAIL::group_ld_u32_v4 ||
            miOpCode == HSAIL::group_ld_u64_v4 ||
            miOpCode == HSAIL::group_ld_f32_v4 ||
            miOpCode == HSAIL::group_ld_f64_v4 ||
            miOpCode == HSAIL::group_st_u32_v4 ||
            miOpCode == HSAIL::group_st_u64_v4 ||
            miOpCode == HSAIL::group_st_f32_v4 ||
            miOpCode == HSAIL::group_st_f64_v4) ||
            (miOpCode == HSAIL::group_ext_ld_u32_u16_v4 ||
            miOpCode == HSAIL::group_ext_ld_u32_u8_v4 ||
            miOpCode == HSAIL::group_sext_ld_s32_s16_v4 ||
            miOpCode == HSAIL::group_sext_ld_s32_s8_v4 ||
            miOpCode == HSAIL::group_zext_ld_u32_u16_v4 ||
            miOpCode == HSAIL::group_zext_ld_u32_u8_v4) ||
            (miOpCode == HSAIL::group_truncst_u32_u16_v4 ||
            miOpCode == HSAIL::group_truncst_u32_u8_v4)) {
          return true;
        } else if ((miOpCode == HSAIL::group_ld_u32_v3 ||
            miOpCode == HSAIL::group_ld_u64_v3 ||
            miOpCode == HSAIL::group_ld_f32_v3 ||
            miOpCode == HSAIL::group_ld_f64_v3 ||
            miOpCode == HSAIL::group_st_u32_v3 ||
            miOpCode == HSAIL::group_st_u64_v3 ||
            miOpCode == HSAIL::group_st_f32_v3 ||
            miOpCode == HSAIL::group_st_f64_v3) ||
            (miOpCode == HSAIL::group_ext_ld_u32_u16_v3 ||
            miOpCode == HSAIL::group_ext_ld_u32_u8_v3 ||
            miOpCode == HSAIL::group_sext_ld_s32_s16_v3 ||
            miOpCode == HSAIL::group_sext_ld_s32_s8_v3 ||
            miOpCode == HSAIL::group_zext_ld_u32_u16_v3 ||
            miOpCode == HSAIL::group_zext_ld_u32_u8_v3) ||
            (miOpCode == HSAIL::group_truncst_u32_u16_v3 ||
            miOpCode == HSAIL::group_truncst_u32_u8_v3) ||
            (miOpCode == HSAIL::group_ld_u32_v2 ||
            miOpCode == HSAIL::group_ld_u64_v2 ||
            miOpCode == HSAIL::group_ld_f32_v2 ||
            miOpCode == HSAIL::group_ld_f64_v2 ||
            miOpCode == HSAIL::group_st_u32_v2 ||
            miOpCode == HSAIL::group_st_u64_v2 ||
            miOpCode == HSAIL::group_st_f32_v2 ||
            miOpCode == HSAIL::group_st_f64_v2) ||
            (miOpCode == HSAIL::group_ext_ld_u32_u16_v2 ||
            miOpCode == HSAIL::group_ext_ld_u32_u8_v2 ||
            miOpCode == HSAIL::group_sext_ld_s32_s16_v2 ||
            miOpCode == HSAIL::group_sext_ld_s32_s8_v2 ||
            miOpCode == HSAIL::group_zext_ld_u32_u16_v2 ||
            miOpCode == HSAIL::group_zext_ld_u32_u8_v2) ||
            (miOpCode == HSAIL::group_truncst_u32_u16_v2 ||
            miOpCode == HSAIL::group_truncst_u32_u8_v2)) {
          return true;
        }
      }
      if (HSAILAS::PRIVATE_ADDRESS == memop->getPointerInfo().getAddrSpace()) {
        if ((miOpCode == HSAIL::private_ld_u32_v1 ||
            miOpCode == HSAIL::private_ld_u64_v1 ||
            miOpCode == HSAIL::private_ld_f32_v1 ||
            miOpCode == HSAIL::private_ld_f64_v1 ||
            miOpCode == HSAIL::private_st_u32_v1 ||
            miOpCode == HSAIL::private_st_u64_v1 ||
            miOpCode == HSAIL::private_st_f32_v1 ||
            miOpCode == HSAIL::private_st_f64_v1) ||
            (miOpCode == HSAIL::private_ld_u32_v4 ||
            miOpCode == HSAIL::private_ld_u64_v4 ||
            miOpCode == HSAIL::private_ld_f32_v4 ||
            miOpCode == HSAIL::private_ld_f64_v4 ||
            miOpCode == HSAIL::private_st_u32_v4 ||
            miOpCode == HSAIL::private_st_u64_v4 ||
            miOpCode == HSAIL::private_st_f32_v4 ||
            miOpCode == HSAIL::private_st_f64_v4) ||
            (miOpCode == HSAIL::private_ld_u32_v3 ||
            miOpCode == HSAIL::private_ld_u64_v3 ||
            miOpCode == HSAIL::private_ld_f32_v3 ||
            miOpCode == HSAIL::private_ld_f64_v3 ||
            miOpCode == HSAIL::private_st_u32_v3 ||
            miOpCode == HSAIL::private_st_u64_v3 ||
            miOpCode == HSAIL::private_st_f32_v3 ||
            miOpCode == HSAIL::private_st_f64_v3) ||
            (miOpCode == HSAIL::private_ld_u32_v2 ||
            miOpCode == HSAIL::private_ld_u64_v2 ||
            miOpCode == HSAIL::private_ld_f32_v2 ||
            miOpCode == HSAIL::private_ld_f64_v2 ||
            miOpCode == HSAIL::private_st_u32_v2 ||
            miOpCode == HSAIL::private_st_u64_v2 ||
            miOpCode == HSAIL::private_st_f32_v2 ||
            miOpCode == HSAIL::private_st_f64_v2)) {
          return true;
        }
      }
    }
  }
  return false;
}

bool HSAILResizeLocalPointer::populateLocalPointerArithmetic(
    MachineFunction &F, 
    std::set<MachineInstr*>& addr_insts) {
  /* 
   * Do a walk over the function and collect 
   * a supported list of ld/st that address
   * local memory.
   * Pattern supported are:
   *  cvt_s64_s32 vreg1-64, vreg2-32
   *  shl_u64 vreg1-64, vreg1-64, 2
   *  add_u64 pointer-64, localptr-64, vreg1-64
   *  st/ld_group_xxx load-xxx, pointer-64
   * 
   * and:
   * 
   *  cvt_s64_s32 vreg1-64, vreg2-32
   *  shl_u64 vreg1-64, vreg1-64, 2
   *  st/ld_group_xxx load-xxx, base, vreg1-64
   * Insert the add_u64 or shl_u64 to a work-list.
   * The work-list is processed by 
   * resizePointerArithmeticInWorklist()
   * private load/stores are also supported. 
   */
  for (MachineFunction::iterator bb_it = F.begin(),
      bb_end = F.end(); bb_it != bb_end; ++bb_it) {
    for (MachineBasicBlock::iterator ldst_it = bb_it->begin(),
         ldst_end = bb_it->end(); ldst_it != ldst_end;) {
      MachineInstr* ldst_inst = &*ldst_it++;
      if (verifySupportedInstruction(ldst_inst) == true) {
        MachineInstr* addop = F.getRegInfo().getVRegDef(
            getIndex(ldst_inst).getReg());
        if (addop) {
          if (addr_insts.find(addop) != addr_insts.end()) 
            continue;
          bool checksout = false;
          if (addop->getOpcode() == HSAIL::add_u64) {
            if (!addop->getOperand(1).isReg()) continue;
            if (!addop->getOperand(2).isReg()) continue;
            MachineInstr* shiftop =
              F.getRegInfo().getVRegDef(addop->getOperand(2).getReg());
            if (!shiftop) continue;
            if (shiftop->getOpcode() != HSAIL::shl_ri_u64) continue;
            if (!shiftop->getOperand(1).isReg()) continue;
            MachineInstr* convop =
              F.getRegInfo().getVRegDef(shiftop->getOperand(1).getReg());
            if (!convop) continue;
            if (convop->getOpcode() != HSAIL::cvt_s32_s64 &&
               convop->getOpcode() != HSAIL::cvt_u32_u64) continue;
            if (!(convop->getOperand(0).isReg() &&
                  convop->getOperand(1).isReg())) continue;
            checksout = true;
          } else if (addop->getOpcode() == HSAIL::shl_ri_u64) {
            if (!addop->getOperand(1).isReg()) continue;
            MachineInstr* convop =
              F.getRegInfo().getVRegDef(addop->getOperand(1).getReg());
            if (!convop) continue;
            if ((convop->getOpcode() != HSAIL::cvt_s32_s64 &&
                 convop->getOpcode() != HSAIL::cvt_u32_u64)) continue;
            if (!(convop->getOperand(0).isReg() &&
                  convop->getOperand(1).isReg())) continue;
            checksout = true;
          }
          if (checksout) {
            unsigned add_reg = getIndex(ldst_inst).getReg();
            bool bail = false;
            const MachineRegisterInfo* MRI = &F.getRegInfo();
            for (MachineRegisterInfo::use_iterator I =
                MRI->use_begin(add_reg), E = MRI->use_end(); 
                I != E; ++I) {
              MachineOperand& Use = I.getOperand();
              MachineInstr* UseMI = Use.getParent();
              if (UseMI == ldst_inst) continue;
              if (verifySupportedInstruction(UseMI) == false) {
                bail = true;
                break;
              }
            }
            if (bail == true) continue;
            addr_insts.insert(addop);
          }
        }
      }
    }
  }
  return false;
}

bool HSAILResizeLocalPointer::resizePointerArithmeticInWorklist(
    MachineFunction& F, std::set<MachineInstr*>& addr_insts) {
  /*
   * Take the worklist for the entire function containing the 
   * add_64 or the shiftl_64 that define the index register.
   * For a supported load/store, find the operation tree
   * generating the index register, which will look like
   * add-64(reg1,shiftl-64(conv32To64(reg2))
   * convert that to add-32(truncate64To32(reg1),shiftl-32(reg2))
   * Another pattern is just the shift, without the add. For which
   * covert shift-64 to shift-32 after dropping the 
   * conv32To64 operation.
   * The whole function/worklist algorithm works because the 
   * code is in SSA form.
   */
  bool ret = true;
  const HSAILInstrInfo *TII = TM.getInstrInfo();
  for (std::set<MachineInstr*>::iterator sit = addr_insts.begin(), 
      sie = addr_insts.end(); sit != sie; ++sit) {
    MachineInstr* addop = *sit;
    if (addop->getOpcode() == HSAIL::add_u64) {
      DEBUG(dbgs() << "Original add instruction:" << addop 
                        << ':' << *addop);
      MachineInstr* shiftop =
        F.getRegInfo().getVRegDef(addop->getOperand(2).getReg());
      DEBUG(dbgs() << "Original shift instruction:" << shiftop 
                        << ':' << *shiftop);
      MachineInstr* convop =
        F.getRegInfo().getVRegDef(shiftop->getOperand(1).getReg());
      const TargetRegisterClass *reg_class =
         F.getRegInfo().getRegClass(convop->getOperand(1).getReg());
      MachineInstrBuilder shf_builder = BuildMI(
                    *shiftop->getParent(),
                    shiftop, shiftop->getDebugLoc(),
                    TII->get(HSAIL::shl_ri_u32),
                    F.getRegInfo().createVirtualRegister(reg_class));
      shf_builder.addReg(convop->getOperand(1).getReg());
      shf_builder.addOperand(shiftop->getOperand(2));
      MachineInstrBuilder conv_builder = BuildMI(
                      *addop->getParent(),
                      addop, addop->getDebugLoc(),
                      TII->get(HSAIL::cvt_u64_u32),
                      F.getRegInfo().createVirtualRegister(reg_class));
      conv_builder.addReg(addop->getOperand(1).getReg());

      MachineInstrBuilder add_builder = BuildMI(
                      *addop->getParent(),
                      addop, addop->getDebugLoc(),
                      TII->get(HSAIL::add_u32),
                      F.getRegInfo().createVirtualRegister(reg_class));
      add_builder.addReg(conv_builder->getOperand(0).getReg());
      add_builder.addReg(shf_builder->getOperand(0).getReg());
      unsigned reg = addop->getOperand(0).getReg();
      const MachineRegisterInfo* MRI = &F.getRegInfo();
      std::set<MachineInstr*> memop_wrklist;
      for (MachineRegisterInfo::use_iterator I = MRI->use_begin(reg),
        E = MRI->use_end(); I != E; ++I) {
        MachineOperand& Use = I.getOperand();
        MachineInstr* ldst_inst = Use.getParent();
        memop_wrklist.insert(ldst_inst);
      }
      DEBUG(dbgs() << "modified add instruction:" << *add_builder);
      DEBUG(dbgs() << "modified shift instruction:" << *shf_builder);
      for (std::set<MachineInstr*>::iterator wi = memop_wrklist.begin(),
          we = memop_wrklist.end(); wi != we; ++wi) {
        MachineInstr* ldst_inst = *wi;
        DEBUG(dbgs() << "Original ld/st instr:" << ldst_inst 
                        << ':' << *ldst_inst);
        ldst_inst->substituteRegister(reg,
                      add_builder->getOperand(0).getReg(),
                      addop->getOperand(0).getSubReg(),
                      *F.getTarget().getRegisterInfo());
        ret = false;
        DEBUG(dbgs() << "LD/ST after modifying pointer arithmetic:" << ldst_inst 
                        << ':' << *ldst_inst);
      }
      
      if (F.getRegInfo().hasOneUse(
            convop->getOperand(0).getReg()) &&
          F.getRegInfo().hasOneUse(
            shiftop->getOperand(0).getReg())) {
        convop->eraseFromParent();
        shiftop->eraseFromParent();
      }
      addop->eraseFromParent();
    } else if (addop->getOpcode() == HSAIL::shl_ri_u64) {
      MachineInstr* shiftop = addop;
      if (!shiftop->getOperand(1).isReg()) continue;
      if (!shiftop->getOperand(0).isReg()) continue;
      MachineInstr* convop =
        F.getRegInfo().getVRegDef(shiftop->getOperand(1).getReg());
      if (!convop->getOperand(1).isReg()) continue;
      if (!convop->getOperand(0).isReg()) continue;
      DEBUG(dbgs() << "Original shift instruction:" << shiftop 
                        << ':' << *shiftop);
      const TargetRegisterClass *reg_class =
         F.getRegInfo().getRegClass(convop->getOperand(1).getReg());
      MachineInstrBuilder shf_builder = BuildMI(
                    *shiftop->getParent(),
                    shiftop, shiftop->getDebugLoc(),
                    TII->get(HSAIL::shl_ri_u32),
                    F.getRegInfo().createVirtualRegister(reg_class));
      shf_builder.addReg(convop->getOperand(1).getReg());
      shf_builder.addOperand(shiftop->getOperand(2));
      DEBUG(dbgs() << "modified shift instruction:" << *shf_builder);
      unsigned reg = shiftop->getOperand(0).getReg();
      const MachineRegisterInfo* MRI = &F.getRegInfo();
      std::set<MachineInstr*> memop_wrklist;
      for (MachineRegisterInfo::use_iterator I = MRI->use_begin(reg),
        E = MRI->use_end(); I != E; ++I) {
        MachineOperand& Use = I.getOperand();
        MachineInstr* ldst_inst = Use.getParent();
        memop_wrklist.insert(ldst_inst);
      }
      for (std::set<MachineInstr*>::iterator wi =
          memop_wrklist.begin(), we = memop_wrklist.end();
            wi != we; ++wi) {
        MachineInstr* ldst_inst = *wi;
        DEBUG(dbgs() << "Original ld/st instr:" << ldst_inst 
                        << ':' << *ldst_inst);
        ldst_inst->substituteRegister(reg,
                      shf_builder->getOperand(0).getReg(),
                      shiftop->getOperand(0).getSubReg(),
                      *F.getTarget().getRegisterInfo());
        ret = false;
        DEBUG(dbgs() << "After modifying pointer arithmetic:" << ldst_inst 
                        << ':' << *ldst_inst);
      }
      if (F.getRegInfo().hasOneUse(convop->getOperand(0).getReg()))
        convop->eraseFromParent();
      shiftop->eraseFromParent();
    }
  }
  return ret;
}

bool HSAILResizeLocalPointer::runOnMachineFunction(
    MachineFunction &F) {
  if (!EnableHsail64Or4to8) return false;
  if (!F.getRegInfo().isSSA()) return false;
  std::set<MachineInstr*> addr_insts;
  populateLocalPointerArithmetic(F,addr_insts);
  bool didsth = !(resizePointerArithmeticInWorklist(F,addr_insts));
  return didsth;
}
