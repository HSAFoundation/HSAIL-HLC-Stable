//
// @file AMDILSIIOExpansion.cpp
// @details Implementation of the I/O expansion class for SI devices
//
#include "AMDILSIIOExpansion.h"
#include "AMDILCompilerErrors.h"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILTargetMachine.h"
#include "AMDILUtilityFunctions.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Value.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/Support/DebugLoc.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdio>
using namespace llvm;

namespace llvm {
  extern void initializeAMDILSIIOExpansionPass(llvm::PassRegistry&);
}

char AMDILSIIOExpansion::ID = 0;
INITIALIZE_PASS(AMDILSIIOExpansion, "si-io-expansion",
                "AMDIL SI IO Expansion", false, false)

AMDILSIIOExpansion::AMDILSIIOExpansion()
  : MachineFunctionPass(ID) {
  initializeAMDILSIIOExpansionPass(*PassRegistry::getPassRegistry());
}

const char *AMDILSIIOExpansion::getPassName() const {
  return "AMDIL SI IO Expansion Pass";
}

bool AMDILSIIOExpansion::runOnMachineFunction(MachineFunction& MF) {
  AMDILSIIOExpansionImpl Impl(MF);
  DEBUG(MF.dump());
  bool Changed = Impl.run();
  DEBUG(MF.dump());
  return Changed;
}

bool AMDILSIIOExpansionImpl::isCacheableOp(MachineInstr *MI) {
  AMDILAS::InstrResEnc curRes;
  getAsmPrinterFlags(MI, curRes);
  return curRes.bits.CacheableRead;
}

bool AMDILSIIOExpansionImpl::isIOInstruction(MachineInstr *MI) {
  if (!MI) {
    return false;
  }
  if (is64BitImageInst(MI)) {
    return true;
  }
  switch (MI->getOpcode()) {
  default:
    return AMDILEGIOExpansionImpl::isIOInstruction(MI);
  case AMDIL::ATOM_G_LOADi8:
  case AMDIL::ATOM_G_STOREi8:
  case AMDIL::ATOM64_G_LOADi8:
  case AMDIL::ATOM64_G_STOREi8:
  case AMDIL::ATOM_G_LOADi16:
  case AMDIL::ATOM_G_STOREi16:
  case AMDIL::ATOM64_G_LOADi16:
  case AMDIL::ATOM64_G_STOREi16:
  case AMDIL::ATOM_G_LOADi32:
  case AMDIL::ATOM_G_STOREi32:
  case AMDIL::ATOM64_G_LOADi32:
  case AMDIL::ATOM64_G_STOREi32:
  case AMDIL::ATOM_G_LOADv2i32:
  case AMDIL::ATOM_G_STOREv2i32:
  case AMDIL::ATOM64_G_LOADv2i32:
  case AMDIL::ATOM64_G_STOREv2i32:
  case AMDIL::ATOM_G_LOADv4i32:
  case AMDIL::ATOM_G_STOREv4i32:
  case AMDIL::ATOM64_G_LOADv4i32:
  case AMDIL::ATOM64_G_STOREv4i32:
    return false;
  }
  return AMDILEGIOExpansionImpl::isIOInstruction(MI);
}

void AMDILSIIOExpansionImpl::expandIOInstruction(MachineInstr *MI) {
  assert(isIOInstruction(MI) && "Must be an IO instruction to "
      "be passed to this function!");
  if (is64BitImageInst(MI)) {
    if (isReadImageInst(MI) || isImageTXLDInst(MI)) {
      expandImageLoad(mBB, MI);
      return;
    }
    if (isWriteImageInst(MI)) {
      expandImageStore(mBB, MI);
      return;
    }
    if (isImageInfoInst(MI)) {
      expandImageParam(mBB, MI);
      return;
    }
  }
  AMDILEGIOExpansionImpl::expandIOInstruction(MI);
}


static bool isAlignedInst(MachineInstr *MI) {
  if (!MI->memoperands_empty()) {
    return ((*MI->memoperands_begin())->getAlignment()
        & ((*MI->memoperands_begin())->getSize() - 1)) == 0;
  }
  return true;
}

void AMDILSIIOExpansionImpl::expandGlobalLoad(MachineInstr *MI) {
  uint32_t AddyReg = MI->getOperand(1).getReg();
  uint32_t DataReg = MI->getOperand(0).getReg();
  expandLoadStartCode(MI, AddyReg);
  DebugLoc DL = MI->getDebugLoc();
  uint32_t ID = getPointerID(MI);
  bool Cacheable = isCacheableOp(MI);
  bool Is64Bit = is64bitLSOp(MI);
  uint32_t Reg = getPackedReg(DataReg, getPackedID(MI));
  bool Aligned = isAlignedInst(MI);
  mMFI->setOutputInst();
  DEBUG(dbgs() << "ID: " << ID << " is being mapped to "; MI->dump(););
  switch (getMemorySize(MI)) {
    default:
    {
      uint32_t opc;
      if (Cacheable) {
        if (Aligned) {
          opc = Is64Bit ? AMDIL::UAVRAW64LOADCACHEDALIGNEDv4i32
                        : AMDIL::UAVRAW32LOADCACHEDALIGNEDv4i32;
        } else {
          opc = Is64Bit ? AMDIL::UAVRAW64LOADCACHEDv4i32
                        : AMDIL::UAVRAW32LOADCACHEDv4i32;
        }
      } else {
        opc = Is64Bit ? AMDIL::UAVRAW64LOADv4i32 : AMDIL::UAVRAW32LOADv4i32;
      }
      assert(is128BitRegister(Reg) && "not 128bit register");
      BuildMI(*mBB, MI, DL, mTII->get(opc), Reg)
        .addReg(AddyReg).addImm(ID);
      break;
    }
    case 1: {
      uint32_t opc;
      Reg = get1stI32SubReg(Reg);
      if (!isXComponentReg(Reg))
        Reg = AMDIL::Rx1011;
      if (Cacheable) {
        opc = isSWSExtLoadInst(MI)
              ? (Is64Bit
                 ? AMDIL::UAVRAW64LOADCACHEDi8
                 : AMDIL::UAVRAW32LOADCACHEDi8)
              : (Is64Bit
                 ? AMDIL::UAVRAW64LOADCACHEDu8
                 : AMDIL::UAVRAW32LOADCACHEDu8);
      } else {
        opc = isSWSExtLoadInst(MI)
              ? (Is64Bit ? AMDIL::UAVRAW64LOADi8 : AMDIL::UAVRAW32LOADi8)
              : (Is64Bit ? AMDIL::UAVRAW64LOADu8 : AMDIL::UAVRAW32LOADu8);
      }
      BuildMI(*mBB, MI, DL, mTII->get(opc), Reg)
        .addReg(AddyReg)
        .addImm(ID);
      break;
    }
    case 2: {
      uint32_t opc;
      Reg = get1stI32SubReg(Reg);
      if (!isXComponentReg(Reg))
        Reg = AMDIL::Rx1011;
      if (Cacheable) {
        opc = isSWSExtLoadInst(MI)
              ? (Is64Bit
                 ? AMDIL::UAVRAW64LOADCACHEDi16
                 : AMDIL::UAVRAW32LOADCACHEDi16)
              : (Is64Bit
                 ? AMDIL::UAVRAW64LOADCACHEDu16
                 : AMDIL::UAVRAW32LOADCACHEDu16);
      } else {
        opc = isSWSExtLoadInst(MI)
              ? (Is64Bit ? AMDIL::UAVRAW64LOADi16 : AMDIL::UAVRAW32LOADi16)
              : (Is64Bit ? AMDIL::UAVRAW64LOADu16 : AMDIL::UAVRAW32LOADu16);
      }
      BuildMI(*mBB, MI, DL, mTII->get(opc), Reg)
        .addReg(AddyReg)
        .addImm(ID);
      break;
    }
    case 4: {
      uint32_t opc;
      Reg = get1stI32SubReg(Reg);
      if (!isXComponentReg(Reg))
        Reg = AMDIL::Rx1011;
      if (Cacheable) {
        opc = Is64Bit ? AMDIL::UAVRAW64LOADCACHEDi32
                      : AMDIL::UAVRAW32LOADCACHEDi32;
      } else {
        opc = Is64Bit ? AMDIL::UAVRAW64LOADi32
                      : AMDIL::UAVRAW32LOADi32;
      }
      BuildMI(*mBB, MI, DL, mTII->get(opc), Reg)
        .addReg(AddyReg)
        .addImm(ID);

    }
      break;
    case 8: {
      unsigned opc;
      Reg = get1stI64SubReg(Reg);
      if (!isXYComponentReg(Reg))
        Reg = AMDIL::Rxy1011;
      if (Cacheable) {
        if (Aligned) {
          opc = Is64Bit
                ? AMDIL::UAVRAW64LOADCACHEDALIGNEDv2i32
                : AMDIL::UAVRAW32LOADCACHEDALIGNEDv2i32;
        } else {
          opc = Is64Bit
                ? AMDIL::UAVRAW64LOADCACHEDv2i32
                : AMDIL::UAVRAW32LOADCACHEDv2i32;
        }
      } else {
        opc = Is64Bit ? AMDIL::UAVRAW64LOADv2i32 : AMDIL::UAVRAW32LOADv2i32;
      }
      BuildMI(*mBB, MI, DL, mTII->get(opc), Reg)
        .addReg(AddyReg)
        .addImm(ID);
    }
      break;
  }

  if (isPackedInst(MI)) {
    expandPackedData(MI, Reg, DataReg);
    Reg = DataReg;
  }
  if (isExtendLoad(MI)) {
    expandExtendLoad(MI, Reg, DataReg);
    MI->getOperand(0).setReg(DataReg);
  } else if (Reg != DataReg) {
    BuildMI(*mBB, MI, DL, mTII->get(AMDIL::COPY), DataReg).addReg(Reg);
  }
}

void AMDILSIIOExpansionImpl::expandGlobalStore(MachineInstr *MI) {
  uint32_t AddyReg = MI->getOperand(1).getReg();
  uint32_t DataReg = MI->getOperand(0).getReg();
  // These instructions are expandted before the current MI.
  expandStoreSetupCode(MI, AddyReg, DataReg);
  uint32_t ID = getPointerID(MI);
  mMFI->setOutputInst();
  bool Is64Bit = is64bitLSOp(MI);
  DebugLoc DL = MI->getDebugLoc();
  DEBUG(dbgs() << "ID: " << ID << " is being mapped to "; MI->dump(););
  switch (getMemorySize(MI)) {
    default:
      BuildMI(*mBB, MI, DL,
              mTII->get(Is64Bit
                        ? AMDIL::UAVRAW64STOREv4i32
                        : AMDIL::UAVRAW32STOREv4i32),
              AMDIL::MEM)
        .addReg(AddyReg)
        .addReg(DataReg)
        .addImm(ID);
      break;
    case 1:
      BuildMI(*mBB, MI, DL,
              mTII->get(Is64Bit
                        ? AMDIL::UAVRAW64STOREi8
                        : AMDIL::UAVRAW32STOREi8),
              AMDIL::MEMx)
        .addReg(AddyReg)
        .addReg(DataReg)
        .addImm(ID);
      break;
    case 2:
      BuildMI(*mBB, MI, DL,
              mTII->get(Is64Bit
                        ? AMDIL::UAVRAW64STOREi16
                        : AMDIL::UAVRAW32STOREi16),
              AMDIL::MEMx)
        .addReg(AddyReg)
        .addReg(DataReg)
        .addImm(ID);
      break;
    case 4:
      BuildMI(*mBB, MI, DL,
              mTII->get(Is64Bit
                        ? AMDIL::UAVRAW64STOREi32
                        : AMDIL::UAVRAW32STOREi32),
              AMDIL::MEMx)
        .addReg(AddyReg)
        .addReg(DataReg)
        .addImm(ID);
      break;
    case 8:
      BuildMI(*mBB, MI, DL,
              mTII->get(Is64Bit
                        ? AMDIL::UAVRAW64STOREv2i32
                        : AMDIL::UAVRAW32STOREv2i32),
              AMDIL::MEMxy)
        .addReg(AddyReg)
        .addReg(DataReg)
        .addImm(ID);
      break;
  };
}
