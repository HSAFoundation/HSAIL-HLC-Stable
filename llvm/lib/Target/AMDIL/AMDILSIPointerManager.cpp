#include "AMDILSIPointerManager.h"
#include "AMDILPointerManagerImpl.h"
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
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"

#include <list>
#include <map>
#include <queue>
#include <set>


using namespace llvm;

// A byte pointer is a pointer that along the pointer path has a
// byte store assigned to it.
void AMDILSIPointerManagerImpl::annotateBytePtrs() {
  llvm_unreachable("byte pointers tracked on SI");
}

// A raw pointer is any pointer that does not have byte store in its path.
// This function is unique to SI devices as arena is not part of it.
void
AMDILSIPointerManagerImpl::annotateRawPtrs()
{
  assert(bytePtrs.empty() && "byte ptrs tracked");
  SortedPtrSet::iterator siBegin, siEnd;
  std::vector<MachineInstr*>::iterator miBegin, miEnd;
  // Now all of the raw pointers will go their own uav ID
  unsigned id = STM->getResourceID(AMDIL::GLOBAL_ID);
  for (siBegin = rawPtrs.begin(), siEnd = rawPtrs.end();
      siBegin != siEnd; ++siBegin) {
    const PointerType *PT = dyn_cast<PointerType>((*siBegin)->getType());
    if (!PT) {
      continue;
    }
    unsigned ResID;
    if (STM->usesHardware(AMDIL::Caps::ConstantMem) &&
        PT->getAddressSpace() == AMDILAS::CONSTANT_ADDRESS) {
      // TODO: if constant buffer is found to conflict with other constant
      // buffers, need to coordicate with AMDILModuleInfo to adjust
      // constant buffer id assignments of the conflicting buffers

      // If hardware constant mem is enabled, then we need to
      // get the constant pointer CB number and use that to specify
      // the resource ID.
      const StringRef funcName = MF.getFunction()->getName();
      const AMDILKernel *krnl = mAMI->getKernel(funcName);
      ResID = mAMI->getConstPtrCB(krnl, (*siBegin)->getName());
    } else if (STM->usesHardware(AMDIL::Caps::LocalMem) &&
               PT->getAddressSpace() == AMDILAS::LOCAL_ADDRESS) {
      // If hardware local mem is enabled, get the local mem ID from
      // the device to use as the ResourceID
      ResID = STM->getResourceID(AMDIL::LDS_ID);
    } else if (STM->usesHardware(AMDIL::Caps::RegionMem) &&
               PT->getAddressSpace() == AMDILAS::REGION_ADDRESS) {
      // If hardware region mem is enabled, get the gds mem ID from
      // the device to use as the ResourceID
      ResID = STM->getResourceID(AMDIL::GDS_ID);
    } else if (STM->usesHardware(AMDIL::Caps::PrivateMem) &&
               PT->getAddressSpace() == AMDILAS::PRIVATE_ADDRESS) {
      ResID = STM->getResourceID(AMDIL::SCRATCH_ID);
    } else {
      if (hasFunctionCalls && useDefaultResId) {
        if (PT->getAddressSpace() == AMDILAS::CONSTANT_ADDRESS) {
          ResID = STM->getResourceID(AMDIL::CONSTANT_ID);
        } else {
          ResID = STM->getResourceID(AMDIL::GLOBAL_ID);
        }
      } else {
        DEBUG_WITH_TYPE("pointermanager", dbgs() << "[annotateRawPtrs] " <<
            MF.getFunction()->getName().str() << " : ";
            (*siBegin)->dump();
            dbgs() << '\n';);
        const Argument *curArg = dyn_cast<Argument>(*siBegin);
        // Allocate global constant buffers in the UAV dedicated for
        // software constant buffer.
        if (!curArg && PT->getAddressSpace() == AMDILAS::CONSTANT_ADDRESS) {
          ResID = STM->getResourceID(AMDIL::CONSTANT_ID);
        } else if (!hasFunctionCalls){
          if (conflictPtrs.count(*siBegin) ||
              (curArg && !curArg->hasNoAliasAttr() &&
               !STM->isSupported(AMDIL::Caps::NoAlias))) {
            ResID = STM->getResourceID(AMDIL::GLOBAL_ID);
          } else {
            ResID = ++id;
          }
        } else {
          if ((conflictPtrs.count(*siBegin) &&
              (PT->getAddressSpace() != AMDILAS::GLOBAL_ADDRESS)) ||
              (curArg && !curArg->hasNoAliasAttr() &&
               !STM->isSupported(AMDIL::Caps::NoAlias))) {
            ResID = STM->getResourceID(AMDIL::GLOBAL_ID);
            DEBUG(dbgs() << "use default res id\n");
          } else if (PT->getAddressSpace() != AMDILAS::GLOBAL_ADDRESS) {
            ResID = ++id;
            DEBUG(dbgs() << "use non-default res id\n");
          } else {
            ResID = ptrEqSet->assignResId(*siBegin, id);
          }
        }
      }
      mMFI->uav_insert(ResID);
      mMFI->setUAVID(*siBegin, ResID);
    }

    DEBUG(
      dbgs() << "Setting pointer to resource ID " << ResID << ": ";
      (*siBegin)->dump();
    );
    for (miBegin = PtrToInstMap[*siBegin].begin(),
           miEnd = PtrToInstMap[*siBegin].end();
         miBegin != miEnd; ++miBegin) {
      DEBUG(
        dbgs() << "Annotating pointer as raw. Inst: ";
        (*miBegin)->dump();
      );
      AMDILAS::InstrResEnc curRes;
      getAsmPrinterFlags(*miBegin, curRes);
      if (STM->usesHardware(AMDIL::Caps::ConstantMem)
          && PT->getAddressSpace() == AMDILAS::CONSTANT_ADDRESS) {
        curRes.bits.HardwareInst = 1;
      }
      curRes.bits.ResourceID = ResID;
      setAsmPrinterFlags(*miBegin, curRes);
      if (isAtomicInst(*miBegin)) {
        assert(ResID && "Atomic resource ID cannot be non-zero!");
        (*miBegin)->getOperand((*miBegin)->getNumOperands()-1).setImm(ResID);
      }
    }
  }
}

// This function annotates the cacheable pointers with the
// CacheableRead bit.
void
AMDILSIPointerManagerImpl::annotateCacheablePtrs()
{
  assert(bytePtrs.empty() && "byte ptrs tracked");
  PtrSet::iterator siBegin, siEnd;
  std::vector<MachineInstr*>::iterator miBegin, miEnd;
  // First we can check the cacheable pointers
  for (siBegin = cacheablePtrs.begin(), siEnd = cacheablePtrs.end();
      siBegin != siEnd; ++siBegin) {
    // If we have any kind of conflict, don't add it as cacheable.
    if (conflictPtrs.count(*siBegin)) {
        continue;
    }
    for (miBegin = PtrToInstMap[*siBegin].begin(),
      miEnd = PtrToInstMap[*siBegin].end();
      miBegin != miEnd; ++miBegin) {
      DEBUG(
        dbgs() << "Annotating pointer as cacheable. Inst: ";
        (*miBegin)->dump();
      );
      AMDILAS::InstrResEnc curRes;
      getAsmPrinterFlags(*miBegin, curRes);
      assert(!curRes.bits.ByteStore && "No cacheable pointers should have the "
              "byte Store flag set!");
      curRes.bits.CacheableRead = 1;
      setAsmPrinterFlags(*miBegin, curRes);
    }
  }
}

void AMDILSIPointerManagerImpl::annotateCacheableInstrs() {
  CacheableInstrSet::iterator miBegin, miEnd;

  for (miBegin = cacheableSet.begin(),
         miEnd = cacheableSet.end();
       miBegin != miEnd; ++miBegin) {
    DEBUG(
      dbgs() << "Annotating instr as cacheable. Inst: ";
      (*miBegin)->dump();
    );
    AMDILAS::InstrResEnc curRes;
    getAsmPrinterFlags(*miBegin, curRes);
    assert(!curRes.bits.ByteStore && "No cacheable pointers should have the "
            "byte Store flag set!");
    curRes.bits.CacheableRead = 1;
    setAsmPrinterFlags(*miBegin, curRes);
  }
}

namespace llvm
{
  extern void initializeAMDILSIPointerManagerPass(llvm::PassRegistry&);
}

char AMDILSIPointerManager::ID = 0;
INITIALIZE_PASS(AMDILSIPointerManager, "si-pointer-manager",
                "AMDIL SI Pointer Manager", false, false)

AMDILSIPointerManager::AMDILSIPointerManager()
  : MachineFunctionPass(ID) {
  initializeAMDILSIPointerManagerPass(*PassRegistry::getPassRegistry());
}

void AMDILSIPointerManager::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredID(MachineDominatorsID);
  MachineFunctionPass::getAnalysisUsage(AU);
}

const char *AMDILSIPointerManager::getPassName() const {
  return "AMD IL SI Pointer Manager Pass";
}

bool AMDILSIPointerManager::runOnMachineFunction(MachineFunction &MF) {
  DEBUG(
    dbgs() << getPassName() << "\n";
    dbgs() << MF.getFunction()->getName() << "\n";
    MF.dump();
  );


  const TargetMachine& TM = MF.getTarget();
  AMDILSIPointerManagerImpl Impl(MF, TM);
  bool Changed = Impl.perform();
  DEBUG(MF.dump(););

  return Changed;
}
