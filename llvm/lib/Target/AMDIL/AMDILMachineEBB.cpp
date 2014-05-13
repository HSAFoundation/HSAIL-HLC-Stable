//===-- AMDILMachineEBB.cpp - AMDIL EBB Optimizations -*- C++ -*------===//
//
// Copyright (c) 2012, Advanced Micro Devices, Inc.
// All rights reserved.
//
//==------------------------------------------------------------------===//


#define DEBUG_TYPE "ebb"
#include "llvm/Support/Debug.h"
#include "AMDIL.h"
#include "AMDILSubtarget.h"
#include "AMDILUtilityFunctions.h"
#include "AMDILMachineEBB.h"
#include "AMDILMachineDCE.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/ValueTypes.h"

#include <set>

using namespace llvm;

static unsigned getNumberOfInst(MachineFunction *MF)
{
  unsigned Num = 0;
  for(MachineFunction::iterator I = MF->begin(), E = MF->end();
      I != E; ++I) {
    MachineBasicBlock *MBB = I;
    Num += MBB->size();
  }
  return Num;
}

static bool getRegClassSVT(int RegClassID, MVT::SimpleValueType& SVT)
{
  switch (RegClassID) {
  default:
    return false;

  case AMDIL::GPRI8RegClassID:
    SVT = MVT::i8;
    return true;

  case AMDIL::GPRI16RegClassID:
    SVT = MVT::i16;
    return true;

  case AMDIL::GPR_32RegClassID:
    SVT = MVT::i32;
    return true;

  case AMDIL::GPR_64RegClassID:
    SVT = MVT::i64;
    return true;

  case AMDIL::GPRV4I8RegClassID:
    SVT = MVT::v4i8;
    return true;

  case AMDIL::GPRV4I16RegClassID:
    SVT = MVT::v4i16;
    return true;

  case AMDIL::GPRV4I32RegClassID:
    SVT = MVT::v4i32;
    return true;

  case AMDIL::GPRV2I8RegClassID:
    SVT = MVT::v2i8;
    return true;

  case AMDIL::GPRV2I16RegClassID:
    SVT = MVT::v2i16;
    return true;

  case AMDIL::GPRV2I32RegClassID:
    SVT = MVT::v2i32;
    return true;

  case AMDIL::GPRV2I64RegClassID:
    SVT = MVT::v2i64;
    return true;
  }
  return false;
}

// The order of this initializers is important. Larger compoments
// must be before smaller compoments.
int const EBBOptimizer::SubRegIdx[6] = {
  AMDIL::sub_xy, AMDIL::sub_zw,
  AMDIL::sub_x, AMDIL::sub_y, AMDIL::sub_z, AMDIL::sub_w
};
int const EBBOptimizer::SubRegIdx_sz = 6;

// This is not neccessarily the complete list of COPY/MOVE instrs,
// rather, it is the list that this EBB can handle now.
bool EBBOptimizer::isCopyInst(MachineInstr *MI)
{
  // TODO: handle more COPY/move instrs.
  int opc = MI->getOpcode();
  if (opc == TargetOpcode::COPY) {
    // Sanity check
    if (!MI->getOperand(0).isReg() || !MI->getOperand(1).isReg())
      return  false;

    unsigned dstReg = MI->getOperand(0).getReg();
    unsigned srcReg = MI->getOperand(1).getReg();
    bool dstRegIsPReg = TargetRegisterInfo::isPhysicalRegister(dstReg);
    bool srcRegIsPReg = TargetRegisterInfo::isPhysicalRegister(srcReg);
    if (dstRegIsPReg && srcRegIsPReg) {
      // Make sure that COPY's dst and src is the same size if they are preg.
      // (We should not use COPY if the size of dst/src preg is not the same
      //  in the first place).
      //
      // For COPY instruction, MI->getDesc().OpInfo[index].RegClass
      // does not work (-1). (We'd better having move instructions for each type.)
      //
      // So, we have to be very conservative. The following code picks up
      // arbitrary RC that is the minimal RC which contains the reg.
      const TargetRegisterClass *srcRC = TRI->getMinimalPhysRegClass(srcReg);
      const TargetRegisterClass *dstRC = TRI->getMinimalPhysRegClass(dstReg);
      unsigned srcbytes = srcRC->getSize();
      unsigned dstbytes = dstRC->getSize();
      DEBUG(dbgs() << "  src=" << srcReg << ",  dst=" << dstReg << '\n'
                   << "  sizeof(src)=" << srcbytes << ",  sizeof(dst)="
                   << dstbytes << '\n');
      return (srcbytes == dstbytes);
    }
    if (!dstRegIsPReg && !srcRegIsPReg)
      return true;
  }

  return false;
}

bool EBBOptimizer::isSelfCopyInst(MachineInstr *MI)
{
  if (isCopyInst(MI) && MI->getOperand(1).isReg())
    return (MI->getOperand(0).getReg() == MI->getOperand(1).getReg());
  return false;
}

// EBB handles those VEXTRACT instrs.
bool EBBOptimizer::isVEXTRACTInst(MachineInstr *MI)
{
  // quick check for efficiency
  if (!isVectorOpInst(MI))
    return false;

  // TODO: handle immediate version as well.
  int opc = MI->getOpcode();
  return (opc == AMDIL::VEXTRACTv2f32r ||
          opc == AMDIL::VEXTRACTv2f64r ||
          opc == AMDIL::VEXTRACTv2i16r ||
          opc == AMDIL::VEXTRACTv2i32r ||
          opc == AMDIL::VEXTRACTv2i64r ||
          opc == AMDIL::VEXTRACTv2i8r  ||
          opc == AMDIL::VEXTRACTv4f32r ||
          opc == AMDIL::VEXTRACTv4i16r ||
          opc == AMDIL::VEXTRACTv4i32r ||
          opc == AMDIL::VEXTRACTv4i8r);
}

bool EBBOptimizer::isLLOi64rInst(MachineInstr *MI)
{
  return MI->getOpcode() == AMDIL::LLOi64r;
}

bool EBBOptimizer::isVCREATEInst(MachineInstr *MI)
{
  int opc = MI->getOpcode();
  return (opc >= AMDIL::VCREATEv2f32i &&
          opc <= AMDIL::VCREATEv4i8r);
}

bool EBBOptimizer::isShiftInst(MachineInstr *MI)
{
  int opc = MI->getOpcode();
  // assume enum for SHL/SHR are consecutive as they are now.
  return (opc >= AMDIL::SHLi16i16ri && opc <= AMDIL::SHLv4i8i8rr) ||
         (opc >= AMDIL::SHRi16i16ri && opc <= AMDIL::SHRv4i8i8rr);
}

bool EBBOptimizer::isANDInst(MachineInstr *MI)
{
  int opc = MI->getOpcode();
  return (opc >= AMDIL::ANDi16ii && opc <= AMDIL::ANDv4i8rr);
}

// Replace DstMO with SrcMV.
void EBBOptimizer::replaceMO(MachineOperand *DstMO, MValue *SrcMV)
{
  assert (DstMO->getType() == SrcMV->getType() &&
          "recreateMO() must have the type of operands");

  // In general, we need to create a new MO.
  // For now, setting register or imm is enough.
  if (DstMO->isReg()) {
    DstMO->setReg(SrcMV->getReg());
    DstMO->setSubReg(SrcMV->getSubReg());
  } else if (DstMO->isImm()) {
    DstMO->setImm(SrcMV->getImm());
  } else {
    assert(false && "Unprocessed MO");
  }
}

bool EBBOptimizer::isSpecialPReg(unsigned PhysReg)
{
  return TRI->isPhysicalRegister(PhysReg) &&
         (!TRI->isInAllocatableClass(PhysReg));
}

void EBBOptimizer::removeMVI(IntrusiveRefCntPtr<MVInfo> aMVI)
{
  MValue *MV = aMVI->MV;
  IntrusiveRefCntPtr<MVInfo> MVI = getMVI(MV);
  if (MVI) {
    if (MVI == aMVI) {
      (void)popMVI(MV);
      return;
    }

    IntrusiveRefCntPtr<MVInfo> N = MVI->Next;
    do {
      if (N == aMVI) {
        MVI->Next = N->Next;
        N->Next = NULL;
        return;
      }
      MVI = N;
      N = N->Next;
    } while (N != NULL);
  }

  llvm_unreachable("MVInfoMap[] should have this MVInfo!");
}

IntrusiveRefCntPtr<MVInfo> EBBOptimizer::createMVI(MValue *aMV, MachineInstr *aMI,
                                MachineOperand *aMO,
                                unsigned aFlags)
{
  IntrusiveRefCntPtr<MVInfo> mvinfo = new MVInfo(aMV, aMI, aMO, aFlags);
  pushMVI(mvinfo);

  bool isDummy = ((aFlags & (MVInfo::MVI_DUMMY_DEFINE |
                             MVInfo::MVI_DUMMY_IDEFINE)) != 0 );
  if (aMO->isReg() && TargetRegisterInfo::isPhysicalRegister(aMO->getReg())) {
    unsigned PReg = aMO->getReg();
    MCRegUnitIterator RUA(PReg, TRI);
    do {
      if (*RUA == PReg)
        continue;

      MValue *MV = MVManager.getOrInsertMValue(*RUA);
      IntrusiveRefCntPtr<MVInfo> MVI = new MVInfo(MV,
                                                  aMI,
                                                  aMO,
                                                  isDummy ? MVInfo::MVI_DUMMY_IDEFINE
                                                  : MVInfo::MVI_IDEFINE);
      pushMVI(MVI);
      mvinfo->OverlapMVIs.push_back(MVI);

      ++RUA;
    } while (RUA.isValid());
  }
  return mvinfo;
}

/*
   For any use reference, mark it as usedef. This is
   for the worse case.
 */
void EBBOptimizer::genMVIAsUseDef(MachineInstr *MI, MachineOperand *MO,
                                  MVOperand *MVO)
{
  if (MVO->isDef())
    return;

  // Don't need MVI, skip.
  if (MVO->OrigMVI[0] == NULL)
    return;

  MVO->setDef();
  MValue *MV = MVO->OrigMVI[0]->MV;
  MVO->OrigMVI[1] = createMVI(MV, MI, MO, MVInfo::MVI_DEFINE);
}

MVOperand *EBBOptimizer::createMVO(MachineInstr *MI, MachineOperand *MO)
{
  MVOperand *MVO = new MVOperand(MO);
  MValue *MV = MVManager.getOrInsertMValue(MO);
  IntrusiveRefCntPtr<MVInfo> MVI = NULL;

  if (MO->isReg()) {
    // For now, only create MVIs for reg operands
    if (MO->isDef()) {
      MVI = createMVI(MV, MI, MO, MVInfo::MVI_DEFINE);
      MVO->setDef();
    } else {
      MVI = getMVI(MV);
      if (!MVI || MVI->isIDef() || MVI->isDummyIDef()) {
        // If MVI->isIDef(), it is possible that we are
        // using uninitialized values
        MVI = createMVI(MV, MI, MO, MVInfo::MVI_DUMMY_DEFINE);
        UpwardExposedMVIs.push_back(MVI);
      }
      MVO->setUse();
    }
  } else {
    MVO->setUse();
  }
  MVO->OrigMVI[0] = MVI;
  return MVO;
}

void EBBOptimizer::perform(MachineBasicBlock *MBB)
{
  DEBUG(dbgs() << "EBB Optimizer For BB#" << MBB->getNumber() << "\n");

  MachineBasicBlock::iterator I = MBB->begin(), E = MBB->end();
  while (I != E) {
    MachineInstr *MI = I;
    ++I;

    DEBUG(dbgs() << "<EBB> " << *MI);

    if (isVEXTRACTInst(MI)) {
      MachineInstr *NewMI = convertVEXTRACT2COPY(MI);
      if (NewMI) {
        DEBUG(dbgs() << "<EBB>     VEXTRACT changed to: " << *NewMI);
        MI->eraseFromParent();
        MI = NewMI;
      }
    }
    else if (isLLOi64rInst(MI)) {
      MachineInstr *NewMI = convertLLOi64r2COPY(MI);
      if (NewMI) {
        DEBUG(dbgs() << "<EBB>     LLOi64r changed to: " << *NewMI);
        MI->eraseFromParent();
        MI = NewMI;
      }
    }

    if (isSelfCopyInst(MI)) {
      // It is nop
      DEBUG(dbgs() << "<EBB>     the COPY instruction removed\n");
      MI->eraseFromParent();
      continue;
    }

    MIInfo *miinfo = createMII(MI);


    // Visiting use operands before visiting def operands
    for (int i = (int)MI->getNumOperands() - 1; i >= 0; --i) {
      MachineOperand *MO = &(MI->getOperand(i));
      MVOperand *MVO = createMVO(MI, MO);
      miinfo->Operands.insert(miinfo->Operands.begin(), MVO);

      // Skip call
      if (MI->isCall())
        continue;

      // Skip EBB since it removes assignment to the register containing the
      // return value, which is needed for function support.
      if (MI->isReturn()) {
        const Function *F = MI->getParent()->getParent()->getFunction();
        if (F->getFnAttributes().hasAttribute(Attributes::NoInline)) {
          DEBUG(dbgs() << "Skip return for noinline function\n";);
          continue;
        }
      }

      if (MO->isReg()) {
        IntrusiveRefCntPtr<MVInfo> MVI = MVO->OrigMVI[0].getPtr();
        assert(MVI && "MVInfo shall not be NULL");
        IntrusiveRefCntPtr<MVInfo> aMVI = MVI->NewMVI;
        if (aMVI && MO->isUse() && isMVICurrent(aMVI)) {
          if (MI->getParent() == aMVI->CreatingMI->getParent()
              || (MI->getParent() == aMVI->CreatingMI->getParent()
                && aMVI->CreatingMI->getParent()->pred_size() == 1)) {
            // We do not want to propagate moves across a loop boundary.
            DEBUG(dbgs() << "<EBB>     " << *MO
                << " propagated from " << *(aMVI->MV)
                << "\n");
            MVO->NewMVI[0] = aMVI;
          }
        }
      }
    }

    if (isCopyInst(MI))
      processCopy(miinfo);
    else if (isShiftInst(MI))
      processShift(miinfo);
  }

  return;
}

void EBBOptimizer::mapRegAndSubreg(unsigned DstReg,
                                   unsigned SrcReg,
                                   IntrusiveRefCntPtr<MVInfo> SrcMVI)
{
  const TargetRegisterClass *sRC = TRI->getMinimalPhysRegClass(SrcReg);
  const TargetRegisterClass *dRC = TRI->getMinimalPhysRegClass(DstReg);
  unsigned bytes = sRC->getSize();
  assert ((bytes == dRC->getSize()) &&
          "DstReg and SrcReg's size should be the same");

  MValue *tMV = MVManager.getMValue(DstReg);
  assert (tMV && "MValue(tMV) should've been created already");
  IntrusiveRefCntPtr<MVInfo> tMVI = getMVI(tMV);
  assert (tMVI && "MVInfo(tMVI) should've been created already");
  tMVI->NewMVI = SrcMVI;

  DEBUG(dbgs() << "<EBB>     " << "copy: "
               << *(tMVI->MV)  << " = "
               << *(SrcMVI->MV) << "\n");

  int sIdx=0, dIdx=0;
  while ( (sIdx < SubRegIdx_sz) || (dIdx < SubRegIdx_sz) ) {
    unsigned sub_srcreg=0, sub_dstreg=0;
    while (sIdx < SubRegIdx_sz) {
      sub_srcreg = TRI->getSubReg(SrcReg, SubRegIdx[sIdx]);
      ++sIdx;
      if (sub_srcreg > 0) {
        // found
        break;
      }
    }

    while (dIdx < SubRegIdx_sz) {
      sub_dstreg = TRI->getSubReg(DstReg, SubRegIdx[dIdx]);
      ++dIdx;
      if (sub_dstreg > 0) {
        // found
        break;
      }
    }

    if ( (sub_srcreg > 0) && (sub_dstreg > 0) ) {
#ifndef NDEBUG
     const TargetRegisterClass *RC0 = TRI->getMinimalPhysRegClass(sub_srcreg);
     const TargetRegisterClass *RC1 = TRI->getMinimalPhysRegClass(sub_dstreg);
     assert ((RC0->getSize() == RC1->getSize()) &&
             "sub compoment registers are out of order");
#endif
      MValue *sub_sMV = MVManager.getMValue(sub_srcreg);
      assert(sub_sMV && "MValue for a use SubReg should be created already");
      IntrusiveRefCntPtr<MVInfo> sub_sMVI = getMVI(sub_sMV);
      assert(sub_sMVI && "MVInfo for a use SubReg should be created already");

      MValue *sub_dMV = MVManager.getMValue(sub_dstreg);
      assert(sub_dMV && "MValue for a Def SubReg should be created already");
      IntrusiveRefCntPtr<MVInfo> sub_dMVI = getMVI(sub_dMV);
      assert(sub_dMVI && "MVInfo for a Def SubReg should be created already");
      assert(sub_dMVI->NewMVI == NULL && "MVInfo's newMVI should be NULL");
      sub_dMVI->NewMVI = sub_sMVI;

      DEBUG(dbgs() << "<EBB>     " << "copy: "
                   << *(sub_dMVI->MV) << " = "
                   << *(sub_sMVI->MV) << "\n");

    } else if (sub_srcreg > 0 || sub_dstreg > 0) {
      assert(false && "Same size registers should have the same subregister structure");
    }
  }
}

void EBBOptimizer::processCopy(MIInfo *MII)
{
  MVOperand *dstMVO = MII->Operands[0];
  MVOperand *srcMVO = MII->Operands[1];
  IntrusiveRefCntPtr<MVInfo> dstMVI = dstMVO->OrigMVI[0];
  unsigned dstReg = dstMVI->MV->getReg();
  if (isSpecialPReg(dstReg)) {
    DEBUG(dbgs() << "<EBB>     copy with special regs: "
                 << *(dstMVI->MV)
                 << " don't process\n");
    return;
  }
  IntrusiveRefCntPtr<MVInfo> srcOrigMVI = srcMVO->OrigMVI[0];
  unsigned srcReg = srcOrigMVI->MV->getReg();
  if (isSpecialPReg(srcReg)) {
    DEBUG(dbgs() << "<EBB>     copy with special regs: "
                 << *(srcOrigMVI->MV)
                 << " don't process\n");
    return;
  }

  IntrusiveRefCntPtr<MVInfo> aMVI = srcMVO->NewMVI[0];
  if (aMVI && isMVICurrent(aMVI)) {
    dstMVI->NewMVI = aMVI;
    DEBUG(dbgs() << "<EBB>     " << "copy: "
                 << *(dstMVI->MV) << " = "
                 << *(aMVI->MV) << "\n");
  } else {
    aMVI = srcMVO->OrigMVI[0];
    if (aMVI && isMVICurrent(aMVI)) {
      dstMVI->NewMVI = aMVI;
      DEBUG(dbgs() << "<EBB>     " << "copy: "
                   << *(dstMVI->MV) << " = "
                   << *(aMVI->MV) << "\n");
    }
  }

  if (dstMVI->NewMVI == NULL || dstMVI->OverlapMVIs.size() == 0) {
    return;
  }

  srcReg = dstMVI->NewMVI->MV->getReg();
  if (!TargetRegisterInfo::isPhysicalRegister(srcReg) ||
      !TargetRegisterInfo::isPhysicalRegister(dstReg)) {
    return;
  }

  mapRegAndSubreg(dstReg, srcReg, dstMVI->NewMVI.getPtr());
#if 0
  // For COPY instruction, MI->getDesc().OpInfo[index].RegClass
  // does not work (-1). (We'd better having move instructions for each type.)
  //
  // So, we have to be very conservative. The following code picks up
  // arbitrary RC that is the minimal RC which contains the reg.
  const TargetRegisterClass *srcRC = TRI->getMinimalPhysRegClass(srcReg);
  const TargetRegisterClass *dstRC = TRI->getMinimalPhysRegClass(dstReg);
  unsigned srcbytes = srcRC->getSize();
  unsigned dstbytes = dstRC->getSize();

  if (dstbytes >= srcbytes) {
    // We still have this kind of COPY ?
    for (const uint16_t *SR = TRI->getSubRegisters(dstReg); *SR; ++SR) {
      const TargetRegisterClass *tRC = TRI->getMinimalPhysRegClass(*SR);
      if (srcbytes != tRC->getSize()) {
        continue;
      }

      mapRegAndSubreg(*SR, srcReg, dstMVI->NewMVI);
    }
  }
#endif
}

// Check to see if ValueMI is used between StartMI and EndMI,
// not including StartMI and EndMI.
bool EBBOptimizer::isMIUsed(MachineInstr *ValueMI,
                            MachineInstr *StartMI,
                            MachineInstr *EndMI)
{
  MachineBasicBlock::iterator II = StartMI, IE = EndMI;
  for (++II; II != IE; ++II) {
    MachineInstr *tMI = II;
    MIInfo *tMII = getMII(tMI);
    if (!tMII) {
      // assume they are used (worse case)
      return true;
    }
    for (unsigned k=0; k < tMII->Operands.size(); ++k) {
      IntrusiveRefCntPtr<MVInfo> tMVI = tMII->Operands[k]->OrigMVI[0];
      if (!tMVI || tMVI->isDummyDef() || tMVI->isDummyIDef())
        continue;
      if (tMVI->CreatingMI == ValueMI)
        return true;
      for (unsigned k1 = 0; k1 < tMVI->OverlapMVIs.size(); ++k1) {
        if (tMVI->OverlapMVIs[k1]->CreatingMI == ValueMI)
          return true;
      }
    }
  }
  return false;
}

void EBBOptimizer::processShift(MIInfo *MII)
{
  MachineInstr *MIshift = MII->MI;
  MVOperand *srcMVO1 = MII->Operands[2];

  IntrusiveRefCntPtr<MVInfo> srcMVI1 = srcMVO1->NewMVI[0];
  if (srcMVI1 == NULL)
    srcMVI1 = srcMVO1->OrigMVI[0];

  // If shift amount is constant, or it is not defined in this EBB; skip it.
  if ((srcMVI1 == NULL) || srcMVI1->isDummyDef() || srcMVI1->isDummyIDef())
    return;

  MachineInstr *MIand = srcMVI1->CreatingMI;
  if (!isANDInst(MIand))
    return;

  MVT::SimpleValueType svt0, svt;
  if (!getRegClassSVT(MIshift->getDesc().OpInfo[1].RegClass, svt0))
    return;
  if (!getRegClassSVT(MIshift->getDesc().OpInfo[2].RegClass, svt))
    return;

  MVT mtype0(svt0), mtype(svt);
  uint32_t mask = (mtype0.getScalarType() == MVT::i64) ? 0x3F : 0x1F;

  // 64 bit does not work yet (math/modf), sc bug8763
  if (mask == 0x3F)
    return;

  MIInfo *MIIand = getMII(MIand);
  if (!MIIand)
    return;

  int ix_reg;
  if (mtype.isVector()) {
    // Only check the case
    //   vcreate  r2, const
    //   iand r1, r2
    if (!MIand->getOperand(1).isReg() || !MIand->getOperand(2).isReg())
      return;

    MVOperand *copr = MIIand->Operands[2];
    IntrusiveRefCntPtr<MVInfo> tMVI = copr->NewMVI[0] ? copr->NewMVI[0] : copr->OrigMVI[0];
    if ((tMVI == NULL) || tMVI->isDummyDef() || tMVI->isDummyIDef())
      return;
    MachineInstr *tMI = tMVI->CreatingMI;
    if (!isVCREATEInst(tMI))
      return;
    if (!tMI->getOperand(1).isImm() ||
        (tMI->getOperand(1).getImm() != mask))
      return;
    ix_reg = 1;
  } else {
    // scalar
    if (MIand->getOperand(1).isImm() &&
        (MIand->getOperand(1).getImm() == mask)) {
      if (!MIand->getOperand(2).isReg())
        return;
      ix_reg = 2;
    } else if (MIand->getOperand(2).isImm() &&
               (MIand->getOperand(2).getImm() == mask)) {
      if (!MIand->getOperand(1).isReg())
        return;
      ix_reg = 1;
    } else
      return;
  }
  MVOperand *regOpr = MIIand->Operands[ix_reg];
  assert(regOpr && "MVOperand should not be NULL");
  IntrusiveRefCntPtr<MVInfo> replMVI = regOpr->NewMVI[0];
  if (!replMVI || !isMVICurrent(replMVI)) {
    replMVI = regOpr->OrigMVI[0];
    assert(replMVI && "MVInfo should not be NULL");
  }
  if (isMVICurrent(replMVI)) {
    DEBUG(dbgs() << "<EBB>     (shift) "
                 << *srcMVI1->MV << " ----> " << *replMVI->MV << '\n');
    srcMVO1->NewMVI[0] = replMVI;
  }
  else {
    // To make it simple, Skip it if srcMVO1 has a new replacement MVI.
    if (srcMVO1->NewMVI[0] != NULL)
      return;

    // Handle the case :
    //   1:  x = AND x, R_or_Lit
    //        ...
    //   2:  y shift v, x(kill)
    //
    //   since x is redefined,  input x at (1) cannot be propagated into
    //   shift at (2).
    //
    // If 1 and 2 are in the same MBB, and no use to x b/w (1) and (2), we
    // can safely delete 1.
    //
    // Note that we assume isKill() is accurate.
    //
    if (MIand->getParent() != MIshift->getParent())
      return;

    MValue *MV = MIIand->Operands[0]->OrigMVI[0]->MV;
    if ((MV != replMVI->MV) || !MIshift->getOperand(2).isKill())
      return;

    // Check to see if the value of MIand is used between 1 and 2
    if (isMIUsed(MIand, MIand, MIshift))
      return;

    if (replMVI->isDummyDef() && (replMVI->CreatingMI == MIand)) {
      // need to recreate a dummy MVI as replMVI is going to be removed.
      MachineOperand *MO = &MIshift->getOperand(2);
      IntrusiveRefCntPtr<MVInfo> MVI = createMVI(MV, MIshift, MO, MVInfo::MVI_DUMMY_DEFINE);
      UpwardExposedMVIs.push_back(MVI);
      srcMVO1->OrigMVI[0] = MVI;

    } else {
      srcMVO1->OrigMVI[0] = replMVI;
    }

    DEBUG(dbgs() << "<EBB>     (shift) deleted: " << *MIand
                 << "            " << *srcMVI1->MV
                 << " ----> " << *srcMVO1->OrigMVI[0]->MV << '\n');

    freeMIInfo(MIand, true);
    MIand->eraseFromParent();
  }
}

// Convert VEXTRACT to COPY
MachineInstr* EBBOptimizer::convertVEXTRACT2COPY(MachineInstr *MI)
{
  assert(isVEXTRACTInst(MI) && MI->getOperand(2).isImm() &&
         "VEXTRACT's 2nd Operand should be immediate!");

  MachineBasicBlock *MBB = MI->getParent();
  if (!MI->getOperand(1).isReg())
    return NULL;

  unsigned dstReg = MI->getOperand(0).getReg();
  unsigned srcReg = MI->getOperand(1).getReg();
  if (!TargetRegisterInfo::isPhysicalRegister(srcReg) ||
      !TargetRegisterInfo::isPhysicalRegister(dstReg))
    return NULL;

  // Don't do it if subreg is used. (Do we use SubReg ?)
  if (MI->getOperand(0).getSubReg() || MI->getOperand(1).getSubReg()) {
    return NULL;
  }

  unsigned subreg_idx = AMDIL::NoSubRegister;

  // imm must be 1,2,3,4
  int imm = MI->getOperand(2).getImm() - 1;
  assert (((imm >= 0) && (imm <= 3)) && "VEXTRACT's imm is out of range!");

  int opc = MI->getOpcode();
  switch (opc) {
  default:
    assert (false && "Not VEXTRAXT instruction");
    return NULL;

  case AMDIL::VEXTRACTv2f32r:
  case AMDIL::VEXTRACTv2i32r:
  case AMDIL::VEXTRACTv2i16r:
  case AMDIL::VEXTRACTv2i8r:
    {
      subreg_idx = EBBOptimizer::SubRegIdx[imm + 2];
      if (TRI->getSubReg(srcReg, subreg_idx) == 0) {
        //assert(false && "Wrong imm for EXTRACT!");
        // It is either z or w compoment
        subreg_idx = EBBOptimizer::SubRegIdx[imm+4];
      }
      break;
    }

  case AMDIL::VEXTRACTv4f32r:
  case AMDIL::VEXTRACTv4i32r:
  case AMDIL::VEXTRACTv4i16r:
  case AMDIL::VEXTRACTv4i8r:
    {
      subreg_idx = EBBOptimizer::SubRegIdx[imm + 2];
      break;
    }

  case AMDIL::VEXTRACTv2f64r:
  case AMDIL::VEXTRACTv2i64r:
    {
      subreg_idx = EBBOptimizer::SubRegIdx[imm];
      break;
    }
  }

  unsigned MovOpc = TargetOpcode::COPY;
  unsigned new_srcreg = TRI->getSubReg(srcReg, subreg_idx);
  assert ((new_srcreg > 0) && "Invalid source register from VEXTRAXT");
  MachineInstr *newMI = BuildMI(*MBB, MI, MI->getDebugLoc(),
    TM->getInstrInfo()->get(MovOpc), dstReg).addReg(new_srcreg);

  DEBUG(dbgs() << "<EBB> " << *MI
               << "          Replaced with "
               << *newMI);
  return newMI;
}

// Convert VEXTRACT to COPY
MachineInstr* EBBOptimizer::convertLLOi64r2COPY(MachineInstr *MI)
{
  if (!MI->getOperand(1).isReg())
    return NULL;

  MachineBasicBlock *MBB = MI->getParent();
  unsigned dstReg = MI->getOperand(0).getReg();
  unsigned srcReg = MI->getOperand(1).getReg();
  if (!TargetRegisterInfo::isPhysicalRegister(srcReg) ||
      !TargetRegisterInfo::isPhysicalRegister(dstReg))
    return NULL;

  // Don't do it if subreg is used. (Do we use SubReg ?)
  if (MI->getOperand(0).getSubReg() || MI->getOperand(1).getSubReg()) {
    return NULL;
  }

  // LLOi64r's lower 32bit is either x or z.
  unsigned subreg_idx = AMDIL::sub_x;
  if (TRI->getSubReg(srcReg, subreg_idx) == 0) {
    subreg_idx = AMDIL::sub_z;
  }

  unsigned MovOpc = TargetOpcode::COPY;
  unsigned new_srcreg = TRI->getSubReg(srcReg, subreg_idx);
  assert ((new_srcreg > 0) && "Invalid source register from VEXTRAXT");
  MachineInstr *newMI = BuildMI(*MBB, MI, MI->getDebugLoc(),
    TM->getInstrInfo()->get(MovOpc), dstReg).addReg(new_srcreg);

  DEBUG(dbgs() << "<EBB> " << *MI
               << "          Replaced with "
               << *newMI);
  return newMI;
}

bool EBBOptimizer::EmitMBB(MachineBasicBlock *MBB)
{
  DEBUG(dbgs() << "\n<EBB> EmitMBB BB#" << MBB->getNumber() << "\n");
  bool changed = false;
  for (MachineBasicBlock::iterator I = MBB->begin(), E = MBB->end();
       I != E; ++I)
  {
    MachineInstr *MI = I;
    if (isCopyInst(MI))
      continue;

    MIInfo *mii = getMII(MI);
    assert(mii && "MIInfo missing for MachineInstr!");

    DEBUG(dbgs() << "<EBB> " << *MI);
    bool oprchanged = false;
    for (int i=0; i < (int)mii->Operands.size(); ++i) {
      MVOperand *mvo = mii->Operands[i];
      IntrusiveRefCntPtr<MVInfo> aMVI = mvo->NewMVI[0];
      if (aMVI) {
        MachineOperand *MO = aMVI->MO;
        DEBUG(dbgs() << "           MO[" << i << "]: "
                     << MI->getOperand(i)
                     << " replaced with " << *(aMVI->MV) << "\n");
        replaceMO(&(MI->getOperand(i)),aMVI->MV);
        oprchanged = true;
        changed = true;
      }
    }
    DEBUG( if (oprchanged) { dbgs() << "           new MI: " << *MI ; });
  }
  return changed;
}

void EBBOptimizer::freeMBBMIInfo(MachineBasicBlock *MBB)
{
  for (MachineBasicBlock::iterator I = MBB->begin(), E = MBB->end();
       I != E; ++I) {
    MachineInstr *MI = I;
    freeMIInfo(MI, true);
  }
}

void EBBOptimizer::freeMIInfo(MachineInstr *MI, bool FromMVInfoMap)
{
  MIInfoMapType::iterator II = MIInfoMap.find(MI);
  assert((II != MIInfoMap.end()) && "Missing MIInfo!");
  MIInfo *mii = II->second;
  for (int i=0; i < (int)mii->Operands.size(); ++i) {
    MVOperand *MVO = mii->Operands[i];
    MachineOperand *MO = &MI->getOperand(i);
    IntrusiveRefCntPtr<MVInfo> MVI = MVO->OrigMVI[0];
    if (MO->isReg() &&
         (MO->isDef() || (MVI->isDummyDef() && (MVI->CreatingMI == MI)))) {
      //delete MVI;
    }
    IntrusiveRefCntPtr<MVInfo> wMVI = MVO->OrigMVI[1];
    if (wMVI) {
      //delete wMVI;
    }
    delete MVO;
  }
  delete mii;
  MIInfoMap.erase(II);
/*
  for (std::set<MVInfo*>::iterator I=ToBeDeleted.begin(), E=ToBeDeleted.end();
       I != E; ++I) {
    IntrusiveRefCntPtr<MVInfo> MVI = *I;
    if (MVI->isDummyDef())
      UpwardExposedMVIs.remove(MVI);

    if (FromMVInfoMap) {
      removeMVI(MVI);
      for (int i=0; i < (int)MVI->OverlapMVIs.size(); ++i) {
        removeMVI(MVI->OverlapMVIs[i]);
      }
    }
    delete MVI;
  }
  ToBeDeleted.clear();
  */
}

void EBBOptimizer::freeMIInfoMap()
{
  //std::set<MVInfo*> ToBeDeleted;
  for (MIInfoMapType::iterator I=MIInfoMap.begin(), E=MIInfoMap.end();
       I != E; ++I) {
    MIInfo *mii = I->second;
    assert(mii && "Should not have an enpty MIInfo in MIInfoMap");
    for (int i=0; i < (int)mii->Operands.size(); ++i) {
      MVOperand *MVO = mii->Operands[i];
      IntrusiveRefCntPtr<MVInfo> MVI = MVO->OrigMVI[0];
      if (MVI && MVO->isDefOnly()) {
        //ToBeDeleted.insert(MVI);
      }
      IntrusiveRefCntPtr<MVInfo> wMVI = MVO->OrigMVI[1];
      if (wMVI) {
        //ToBeDeleted.insert(wMVI);
      }
      delete MVO;
    }
    delete mii;
  }
/*
  for (std::set<MVInfo*>::iterator I = ToBeDeleted.begin(),
                                   E = ToBeDeleted.end();
       I != E; ++I) {
    IntrusiveRefCntPtr<MVInfo> tMVI = *I;
    delete tMVI;
  }
  */

  MIInfoMap.clear();
}

bool EBBOptimizer::appendMBB(MachineBasicBlock *MBB)
{
  bool changed = false;
  MBBVisit[MBB] = true;
  perform(MBB);

  for (MachineBasicBlock::succ_iterator SI = MBB->succ_begin(),
       SE = MBB->succ_end(); SI != SE;  ++SI) {
    MachineBasicBlock *SuccMBB = *SI;
    if (!MBBVisit[SuccMBB] && (SuccMBB->pred_size() == 1)) {
      appendMBB(SuccMBB);
    }
  }

  // Re-generate MBB
  changed = EmitMBB(MBB);

  // Free all EBB info associated with MBB
  freeMBBMIInfo(MBB);
  return changed;
}

bool EBBOptimizer::run()
{
  // First, check if the program is too big. If so, skip it.
  unsigned N = getNumberOfInst(MF);
  if (N > 5000) {
    DEBUG(dbgs() << " EBB Optimizer : off for function ("
                 << MF->getFunction()->getName()
                 << "), size is "
                 << N << " (over the limit : 5000)\n");
    return false;
  }

  bool changed = false;
  for(MachineFunction::iterator I = MF->begin(), E = MF->end();
      I != E; ++I) {
    MachineBasicBlock *MBB = I;
    if (!MBBVisit[MBB]) {
      bool tchanged = appendMBB(MBB);
      changed = changed || tchanged;
    }
  }

  if (changed) {
    (void) doMachineFunctionDCE(TM, MF);
  }
  return changed;
}

