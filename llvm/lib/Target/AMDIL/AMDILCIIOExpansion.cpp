//
// @file AMDILCIIOExpansion.cpp
// @details Implementation of the I/O expansion class for SI devices
//

#include "AMDILCIIOExpansion.h"
#include "AMDILCompilerErrors.h"
#include "AMDILKernelManager.h"
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
#include <cstdio>
using namespace llvm;

namespace llvm {
  extern void initializeAMDILCIIOExpansionPass(llvm::PassRegistry&);
}


char AMDILCIIOExpansion::ID = 0;
INITIALIZE_PASS(AMDILCIIOExpansion, "ci-io-expansion",
                "AMDIL CI IO Expansion", false, false)

AMDILCIIOExpansion::AMDILCIIOExpansion()
  : MachineFunctionPass(ID) {
  initializeAMDILCIIOExpansionPass(*PassRegistry::getPassRegistry());
}

const char *AMDILCIIOExpansion::getPassName() const {
  return "AMDIL SI IO Expansion Pass";
}

bool AMDILCIIOExpansion::runOnMachineFunction(MachineFunction& MF) {
  AMDILCIIOExpansionImpl Impl(MF);
  return Impl.run();

}

bool isFlatInst(const llvm::MachineInstr *MI) {
  return isLoadInst(MI) && (MI->getDesc().TSFlags & (1 << AMDID::FLAT));
}

bool AMDILCIIOExpansionImpl::isIOInstruction(llvm::MachineInstr *MI) {
  if (!MI) {
    return false;
  }

  if (isFlatInst(MI))
    return true;
  return AMDILSIIOExpansionImpl::isIOInstruction(MI);
}

void AMDILCIIOExpansionImpl::expandIOInstruction(MachineInstr *MI) {
  assert(isIOInstruction(MI) && "Must be an IO instruction to "
      "be passed to this function!");
  if (isFlatInst(MI)) {
    if (isLoadInst(MI)) {
      return expandFlatLoad(MI);
    } else if (isStoreInst(MI)) {
      return expandFlatStore(MI);
    } else {
      assert(!"Unknown flat instruction detected!");
    }
  }
  AMDILSIIOExpansionImpl::expandIOInstruction(MI);
}

void AMDILCIIOExpansionImpl::expandFlatLoad(MachineInstr *MI) {
  uint32_t AddyReg = MI->getOperand(1).getReg();
  uint32_t DataReg = MI->getOperand(0).getReg();
  // These instructions are generated before the current MI.
  expandLoadStartCode(MI, AddyReg);
  DebugLoc DL = MI->getDebugLoc();
  mMFI->setOutputInst();
  bool is64bit = is64bitLSOp(MI);
  switch (getMemorySize(MI)) {
    default:
      BuildMI(*mBB, MI, DL, mTII->get((is64bit) ?
            AMDIL::FLDv4i32P64U :
            AMDIL::FLDv4i32P32U), DataReg).addReg(AddyReg);
      break;
    case 1:
      BuildMI(*mBB, MI, DL, mTII->get(isSWSExtLoadInst(MI) ?
            (is64bit) ?  AMDIL::FLDi8P64U : AMDIL::FLDi8P32U :
            (is64bit) ?  AMDIL::FLDu8P64U : AMDIL::FLDu8P32U),
          DataReg).addReg(AddyReg);
      break;
    case 2:
      BuildMI(*mBB, MI, DL, mTII->get(isSWSExtLoadInst(MI) ?
            (is64bit) ?  AMDIL::FLDi16P64U : AMDIL::FLDi16P32U :
            (is64bit) ?  AMDIL::FLDu16P64U : AMDIL::FLDu16P32U),
          DataReg).addReg(AddyReg);
      break;
    case 4:
      BuildMI(*mBB, MI, DL, mTII->get((is64bit) ?
            AMDIL::FLDi32P64U :
            AMDIL::FLDi32P32U), DataReg).addReg(AddyReg);
      break;
    case 8:
      BuildMI(*mBB, MI, DL, mTII->get((is64bit) ?
            AMDIL::FLDv2i32P64U :
            AMDIL::FLDv2i32P32U), DataReg).addReg(AddyReg);
      break;
    case 12:
      BuildMI(*mBB, MI, DL, mTII->get((is64bit) ?
            AMDIL::FLDv3i32P64U :
            AMDIL::FLDv3i32P32U), DataReg).addReg(AddyReg);
      break;
  }
  expandPackedData(MI, DataReg, DataReg);
  expandExtendLoad(MI, DataReg, DataReg);
  MI->getOperand(0).setReg(DataReg);
}

void AMDILCIIOExpansionImpl::expandFlatStore(MachineInstr *MI) {
  uint32_t AddyReg = MI->getOperand(1).getReg();
  uint32_t DataReg = MI->getOperand(0).getReg();
  // These instructions are expanded before the current MI.
  expandStoreSetupCode(MI, AddyReg, DataReg);
  mMFI->setOutputInst();
  bool is64bit = is64bitLSOp(MI);
  DebugLoc DL = MI->getDebugLoc();
  switch (getMemorySize(MI)) {
    default:
      BuildMI(*mBB, MI, DL, mTII->get((is64bit ?
            AMDIL::FSTv4i32P64U :
            AMDIL::FSTv4i32P32U))).addReg(AddyReg)
        .addReg(DataReg);
      break;
    case 1:
      BuildMI(*mBB, MI, DL, mTII->get((is64bit ?
            AMDIL::FSTi8P64U :
            AMDIL::FSTi8P32U))).addReg(AddyReg)
        .addReg(DataReg);
      break;
    case 2:
      BuildMI(*mBB, MI, DL, mTII->get((is64bit ?
            AMDIL::FSTi16P64U :
            AMDIL::FSTi16P32U))).addReg(AddyReg)
        .addReg(DataReg);
      break;
    case 4:
      BuildMI(*mBB, MI, DL, mTII->get((is64bit ?
            AMDIL::FSTi32P64U :
            AMDIL::FSTi32P32U))).addReg(AddyReg)
        .addReg(DataReg);
      break;
    case 8:
      BuildMI(*mBB, MI, DL, mTII->get((is64bit ?
            AMDIL::FSTv2i32P64U :
            AMDIL::FSTv2i32P32U))).addReg(AddyReg)
        .addReg(DataReg);
      break;
    case 12:
      BuildMI(*mBB, MI, DL, mTII->get((is64bit ?
            AMDIL::FSTv3i32P64U :
            AMDIL::FSTv3i32P32U))).addReg(AddyReg)
        .addReg(DataReg);
      break;
  }
}

