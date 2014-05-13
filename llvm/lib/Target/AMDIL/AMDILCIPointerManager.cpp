#include "AMDILPointerManagerImpl.h"
#include "AMDILCIPointerManager.h"
#include "AMDILCompilerErrors.h"
#include "AMDILDeviceInfo.h"
#include "AMDILKernelManager.h"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILModuleInfo.h"
#include "AMDILTargetMachine.h"
#include "AMDILUtilityFunctions.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/GlobalValue.h"
#include "llvm/Metadata.h"
#include "llvm/Module.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/Twine.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/FormattedStream.h"

#include <list>
#include <map>
#include <queue>
#include <set>

using namespace llvm;

namespace llvm {
  extern void initializeAMDILCIPointerManagerPass(llvm::PassRegistry&);
}

namespace {
  class AMDILCIPointerManagerImpl : public AMDILPointerManagerImpl {
  public:
    AMDILCIPointerManagerImpl(MachineFunction &mf, const TargetMachine &tm)
      : AMDILPointerManagerImpl(mf, tm) {}
    virtual ~AMDILCIPointerManagerImpl() {}
    virtual bool perform();
  protected:
  };
}

char AMDILCIPointerManager::ID = 0;

INITIALIZE_PASS(AMDILCIPointerManager, "ci-pointer-manager",
                    "AMDIL CI Pointer Manager", false, false);

AMDILCIPointerManager::AMDILCIPointerManager()
  : MachineFunctionPass(ID) {
  initializeAMDILCIPointerManagerPass(*PassRegistry::getPassRegistry());
}

void AMDILCIPointerManager::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredID(MachineDominatorsID);
  MachineFunctionPass::getAnalysisUsage(AU);
}

const char *AMDILCIPointerManager::getPassName() const {
  return "AMD IL CI Pointer Manager Pass";
}

bool AMDILCIPointerManagerImpl::perform() {
  bool Changed = false;
  const AMDILTargetMachine *ATM
    = reinterpret_cast<const AMDILTargetMachine*>(&TM);
  const AMDILSubtarget *STM = ATM->getSubtargetImpl();
  for (MachineFunction::iterator I = MF.begin(),
      E = MF.end(); I != E; ++I) {
    MachineBasicBlock *MB = I;
    for (MachineBasicBlock::iterator MBB = MB->begin(), MBE = MB->end();
        MBB != MBE; ++MBB) {
      MachineInstr *MI = MBB;
      if (isRegionInst(MI) || isRegionAtomic(MI)) {
        AMDILAS::InstrResEnc CurRes;
        getAsmPrinterFlags(MI, CurRes);
        CurRes.bits.ResourceID = STM->getResourceID(AMDIL::GDS_ID);
        mMFI->setUsesGDS();
        if (isAtomicInst(MI)) {
          assert(CurRes.bits.ResourceID && "Atomic resource ID "
              "cannot be zero!");
          MI->getOperand((MI)->getNumOperands() - 1)
              .setImm(CurRes.bits.ResourceID);
        }
        mMFI->setUsesGDS();
        Changed = true;
      }
    }
  }
  return Changed;
}

bool AMDILCIPointerManager::runOnMachineFunction(MachineFunction &MF) {
  const TargetMachine& TM = MF.getTarget();
  const AMDILTargetMachine *ATM
    = reinterpret_cast<const AMDILTargetMachine*>(&TM);
  const AMDILSubtarget *STM = ATM->getSubtargetImpl();

  DEBUG(
    dbgs() << getPassName() << "\n";
    dbgs() << MF.getFunction()->getName() << "\n";
    MF.dump();
  );

  // If we are not overloading with the flat address space,
  // run the SI Pointer manager.
  if (!STM->overridesFlatAS()) {
    AMDILSIPointerManagerImpl Impl(MF, TM);
    return Impl.perform();
  }

  AMDILCIPointerManagerImpl Impl(MF, TM);
  return Impl.perform();
}
