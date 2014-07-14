//===-------- AMDILPointerManager.cpp - Manage Pointers for HW-------------===//
// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// If you use the software (in whole or in part), you shall adhere to all
// applicable U.S., European, and other export laws, including but not limited
// to the U.S. Export Administration Regulations (EAR), (15 C.F.R. Sections
// 730 through 774), and E.U. Council Regulation (EC) No 1334/2000 of 22 June
// 2000.  Further, pursuant to Section 740.6 of the EAR, you hereby certify
// that, except pursuant to a license granted by the United States Department
// of Commerce Bureau of Industry and Security or as otherwise permitted
// pursuant to a License Exception under the U.S. Export Administration
// Regulations ("EAR"), you will not (1) export, re-export or release to a
// national of a country in Country Groups D:1, E:1 or E:2 any restricted
// technology, software, or source code you receive hereunder, or (2) export to
// Country Groups D:1, E:1 or E:2 the direct product of such technology or
// software, if such foreign produced direct product is subject to national
// security controls as identified on the Commerce Control List (currently
// found in Supplement 1 to Part 774 of EAR).  For the most current Country
// Group listings, or for additional information about the EAR or your
// obligations under those regulations, please refer to the U.S. Bureau of
// Industry and Securitys website at http://www.bis.doc.gov/.
//
//==-----------------------------------------------------------------------===//
// Implementation for the AMDILPointerManager classes. See header file for
// more documentation of class.
// TODO: This fails when function calls are enabled, must always be inlined
//===----------------------------------------------------------------------===//

#include "AMDILPointerManager.h"
#include "AMDILPointerManagerImpl.h"

#include "AMDILCIPointerManager.h"
#include "AMDILCompilerErrors.h"
#include "AMDILDeviceInfo.h"
#include "AMDILIntrinsicInfo.h"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILModuleInfo.h"
#include "AMDILSIPointerManager.h"
#include "AMDILTargetMachine.h"
#include "AMDILUtilityFunctions.h"

#include "llvm/AMDLLVMContextHook.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/GlobalValue.h"
#include "llvm/Instructions.h"
#include "llvm/LLVMContext.h"
#include "llvm/Metadata.h"
#include "llvm/Module.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/Twine.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetIntrinsicInfo.h"

#include <cstdio>
#include <list>
#include <map>
#include <set>
#include <queue>


#define SAMPLER_INDEX 2
#define SAMPLER_ARG_COUNT 4
using namespace llvm;

namespace llvm {
  FunctionPass *createAMDILPointerManager(TargetMachine &TM,
                                          CodeGenOpt::Level OL) {
    const AMDILSubtarget &STM = TM.getSubtarget<AMDILSubtarget>();

    switch (STM.getGeneration()) {
    case AMDIL::EVERGREEN:
    case AMDIL::NORTHERN_ISLANDS:
      return new AMDILEGPointerManager();
    case AMDIL::SOUTHERN_ISLANDS:
      return new AMDILSIPointerManager();
    case AMDIL::SEA_ISLANDS:
    case AMDIL::VOLCANIC_ISLANDS:
    case AMDIL::HDTEST:
      return new AMDILCIPointerManager();

    case AMDIL::INVALID_GPU_FAMILY:
    default:
      llvm_unreachable("Bad generation");
    }
  }
}

namespace llvm
{
  extern void initializeAMDILPointerManagerPass(llvm::PassRegistry&);
}

static bool isCalledByKernel(const Function* F,
    std::map<const Function*, bool>& work);

static void inline printReg(unsigned reg) {
  if (TargetRegisterInfo::isVirtualRegister(reg))
    dbgs() << "vreg" << TargetRegisterInfo::virtReg2Index(reg);
  else
    dbgs() << "reg" << reg;
}

char AMDILPointerManager::ID = 0;
INITIALIZE_PASS(AMDILPointerManager, "pointer-manager",
                "AMDIL Pointer Manager", false, false)

AMDILPointerManager::AMDILPointerManager()
  : MachineFunctionPass(ID) {
  initializeAMDILPointerManagerPass(*PassRegistry::getPassRegistry());
}

const char *AMDILPointerManager::getPassName() const {
  return "AMD IL Default Pointer Manager Pass";
}

void AMDILPointerManager::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredID(MachineDominatorsID);
  MachineFunctionPass::getAnalysisUsage(AU);
}

// The default pointer manager just assigns the default ID's to
// each load/store instruction and does nothing else. This is
// the pointer manager for the 7XX series of cards.
bool AMDILPointerManager::runOnMachineFunction(MachineFunction &MF) {
  const TargetMachine& TM = MF.getTarget();
  DEBUG(
    dbgs() << getPassName() << "\n";
    dbgs() << MF.getFunction()->getName() << "\n";
    MF.dump();
  );
  // On the 7XX we don't have to do any special processing, so we
  // can just allocate the default ID and be done with it.
  AMDILPointerManagerImpl impl(MF, TM);
  bool changed = impl.perform();
  return changed;
}

namespace llvm {
  extern void initializeAMDILEGPointerManagerPass(llvm::PassRegistry&);
}

char AMDILEGPointerManager::ID = 0;
INITIALIZE_PASS(AMDILEGPointerManager, "eg-pointer-manager",
                "AMDIL EG Pointer Manager", false, false)

AMDILEGPointerManager::AMDILEGPointerManager()
  : MachineFunctionPass(ID) {
  initializeAMDILEGPointerManagerPass(*PassRegistry::getPassRegistry());
}

void AMDILEGPointerManager::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredID(MachineDominatorsID);
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool AMDILEGPointerManager::runOnMachineFunction(MachineFunction &MF) {
  DEBUG(
    dbgs() << getPassName() << "\n";
    dbgs() << MF.getFunction()->getName() << "\n";
    MF.dump();
  );

  const TargetMachine& TM = MF.getTarget();
  AMDILEGPointerManagerImpl impl(MF, TM);
  bool changed = impl.perform();
  return changed;
}

const char*
AMDILEGPointerManager::getPassName() const
{
  return "AMD IL EG Pointer Manager Pass";
}

static bool containsPointerType(Type *Ty) {
  if (!Ty) {
    return false;
  }

  switch (Ty->getTypeID()) {
  default:
    return false;
  case Type::StructTyID: {
    const StructType *ST = dyn_cast<StructType>(Ty);

    for (StructType::element_iterator stb = ST->element_begin(),
        ste = ST->element_end(); stb != ste; ++stb) {
      if (containsPointerType(*stb)) {
        return true;
      }
    }
    break;
  }
  case Type::VectorTyID:
  case Type::ArrayTyID:
    return containsPointerType(Ty->getSequentialElementType());
  case Type::PointerTyID:
    return true;
  }

  return false;
}

AMDILPointerManagerImpl::AMDILPointerManagerImpl(MachineFunction &MF_,
                                                 const TargetMachine &TM_)
  : MF(MF_), TM(TM_) {
  mMFI = MF.getInfo<AMDILMachineFunctionInfo>();
  ATM = reinterpret_cast<const AMDILTargetMachine*>(&TM);
  STM = ATM->getSubtargetImpl();
  mAMI = &(MF.getMMI().getObjFileInfo<AMDILModuleInfo>());
}

bool AMDILPointerManagerImpl::perform() {
  allocateDefaultIDs();
  clearTempMIFlags(MF);
  return false;
}

void AMDILPointerManagerImpl::clearTempMIFlags(MachineFunction &MF) {
  for (MachineFunction::iterator mfBegin = MF.begin(),
         mfEnd = MF.end(); mfBegin != mfEnd; ++mfBegin) {
    MachineBasicBlock *MB = mfBegin;
    for (MachineBasicBlock::instr_iterator mbb = MB->instr_begin(),
           mbe = MB->instr_end(); mbb != mbe; ++mbb) {
      MachineInstr *MI = mbb;
      AMDILAS::InstrResEnc curRes;
      getAsmPrinterFlags(MI, curRes);
      // Clear temporary flas
      curRes.bits.isImage = 0;
      curRes.bits.ConflictPtr = 0;
      curRes.bits.ByteStore = 0;
      curRes.bits.PointerPath = 0;
      setAsmPrinterFlags(MI, curRes);
    }
  }
}

AMDILEGPointerManagerImpl::AMDILEGPointerManagerImpl(MachineFunction &MF_,
                                                     const TargetMachine &TM_)
  : AMDILPointerManagerImpl(MF_, TM_),
  PtrToInstMap(), InstToPtrMap(), FIToPtrMap(), bytePtrs(pointer2IndexMap),
  cacheablePtrs(), rawPtrs(pointer2IndexMap), conflictPtrs(), localPtrs(),
  images(), counters(), semaphores(),
  cpool(), bbCacheable(), cacheableSet(), lookupTable(), localPtrMap(),
  localPtrSets(), localPtr2SetIdMap(), localSetId2InstMap(), AddriVec(),
  printfRegSet(), printfInsts(), pointer2IndexMap(), nextPointerIndex(0),
  numWriteImages(0), doPerPointerLDS(false),
  trackBytePtrs(true), trackPrintfs(false) {
}

std::string AMDILEGPointerManagerImpl::findSamplerNameFromReg(unsigned reg,
                                                              unsigned &val) {
  std::string sampler = "unknown";
  val = ~0U;
  // If this register points to an argument, then
  // we can return the argument name.
  if (dyn_cast_or_null<Argument>(lookupTable[reg].second)) {
    return lookupTable[reg].second->getName();
  }
  // Otherwise the sampler is coming from memory somewhere.
  // If the sampler memory location can be tracked, then
  // we ascertain the sampler name that way.
  // The most common case is when optimizations are disabled
  // or mem2reg is not enabled, then the sampler when it is
  // an argument is passed through the frame index.

  // In the optimized case, the instruction that defined
  // register from operand #3 is a private load.
  MachineRegisterInfo &regInfo = MF.getRegInfo();
  assert(!regInfo.def_empty(reg)
      && "We don't have any defs of this register, but we aren't an argument!");

  MachineRegisterInfo::reg_iterator MRI = regInfo.reg_begin(reg);
  const MachineOperand *defOp = &MRI.getOperand();
  const MachineInstr *defMI = defOp->getParent();

  if (isPrivateInst(defMI) && isPtrLoadInst(defMI)) {
    if (defMI->getOperand(1).isFI()) {
      IntValPair fiRVP;
      // trace back through spills to the original def instruction
      do {
        fiRVP = FIToPtrMap[defMI->getOperand(1).getIndex()];
        if (fiRVP.second || !fiRVP.first)
          break;
        MachineRegisterInfo::reg_iterator RB = regInfo.reg_begin(fiRVP.first);
        defOp = &RB.getOperand();
        defMI = defOp->getParent();
      } while (defMI->getOperand(1).isFI());

      if (dyn_cast_or_null<Argument>(fiRVP.second)) {
        return fiRVP.second->getName();
      }
      if (!fiRVP.second && fiRVP.first) {
        if (defMI->getOperand(1).isImm()) {
          val = defMI->getOperand(1).getImm();
          char buffer[1024];
          sprintf(buffer, "_%d", val);
          return sampler + std::string(buffer);
        }
        // FIXME: Fix the case where a sampler is loaded from
        // a frame index, but the source instruction was not
        // created from the AMDdbgmove pass.
        llvm_unreachable("Found a case of the AMDdbgmove pass that we don't handle!");
      } else {
        // FIXME: Fix the case where the value stored is not a kernel argument and not a situation which is modified by AMDdbgmove pass.
        llvm_unreachable("Found a private load of a sampler where the value isn't an argument!");
      }
    } else {
      // FIXME: Fix the case where someone dynamically loads a sampler value
      // from private memory. This is problematic because we need to know the
      // sampler value at compile time and if it is dynamically loaded, we won't
      // know what sampler value to use.
      llvm_unreachable("Found a private load of a sampler that isn't from a frame index!");
    }
  } else if (isConstantInst(defMI) && isPtrLoadInst(defMI)) {
    if (defMI->hasOneMemOperand()) {
      const Value *memRef = (*defMI->memoperands_begin())->getValue();
      if (dyn_cast<Argument>(memRef)) {
        return memRef->getName();
      } else if (dyn_cast<GlobalVariable>(memRef)) {
        const GlobalVariable *gvRef = dyn_cast<GlobalVariable>(memRef);
        if (gvRef->hasInitializer()) {
          if (dyn_cast<ConstantInt>(gvRef->getInitializer())) {
            char buffer[1024];
            val = (uint32_t)dyn_cast<ConstantInt>(gvRef->getInitializer())
              ->getZExtValue();
            sprintf(buffer, "_%u", val);
            return sampler + std::string(buffer);
          } else {
            // FIXME: Found a case where a non-integer initializer
            // was found!
            llvm_unreachable("Found a case we don't handle!");
          }
        } else {
          // FIXME: Found a global sampler from the constant address space
          // that does not have an initializer, this isn't legal in OpenCL.
          llvm_unreachable("Found a constant global sampler without an initializer!");
        }
      } else {
        // FIXME: We are loading from a constant pointer that is not
        // a global variable or an argument, how is that possible?
        llvm_unreachable("Found a constant load for a value type that isn't understood.");
      }
    } else {
      // FIXME: What do we do if we have a load from a constant that
      // does not memory operand associated with it!
      llvm_unreachable("Found a constant load but no memory operand!");
    }
  } else if (defMI->getOpcode() == TargetOpcode::COPY) {
    // Somehow are a copy instruction, we need to further parse up and
    // see if we can determine the sampler name.
    return findSamplerNameFromReg(defMI->getOperand(1).getReg(), val);
  } else if (defMI->getOpcode() == AMDIL::LOADCONSTi32) {
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    ::sprintf(buffer,"_%d", (int32_t)defMI->getOperand(1).getImm());
    sampler += buffer;
  } else {
    // FIXME: Handle the case where the def is neither a private instruction
    // and not a load instruction. This shouldn't occur, but putting an assertion
    // just to make sure that it doesn't.
    llvm_unreachable("Found a case which we don't handle.");
  }
  return sampler;
}

std::string AMDILEGPointerManagerImpl::findSamplerName(MachineInstr* MI,
                                                       unsigned &val) {
  assert(MI->getNumOperands() == SAMPLER_ARG_COUNT && "Only an "
      "image read instruction with SAMPLER_ARG_COUNT arguments can "
      "have a sampler.");
  assert(MI->getOperand(SAMPLER_INDEX).isReg() &&
      "Argument SAMPLER_INDEX must be a register to call this function");
  unsigned reg = MI->getOperand(SAMPLER_INDEX).getReg();
  return findSamplerNameFromReg(reg, val);
}

// initialize localPtrSets and localPtr2SetIdMap
// initialize localPtrSets so that each global local pointer is in its own set,
// and all argument local pointers are in the default local buffer set
// then map the local pointers to the id of the set that it belongs
void AMDILEGPointerManagerImpl::initializeLocalPtrSets() {
  const AMDILKernel *krnl = mAMI->getKernel(MF.getFunction()->getName());
  if (krnl == NULL) return;
  const AMDILLocalArg* lvgv = krnl->lvgv;
  if (lvgv == NULL) return;
  localPtrSets.reserve(lvgv->local.size() + 1);

  // first put local pointer arguments to the default buffer set
  localPtrSets.push_back(SmallValSet());
  for (ValueSet::iterator siBegin = localPtrs.begin(), siEnd = localPtrs.end();
       siBegin != siEnd; ++siBegin) {
    const Value* val = *siBegin;
    if (isa<GlobalValue>(val)) continue;

    localPtrSets.back().insert(val);
    localPtr2SetIdMap[val] = 0;
  }

  // next put each non-argument local pointer in its own set
  for (ValueSet::iterator siBegin = localPtrs.begin(), siEnd = localPtrs.end();
       siBegin != siEnd; ++siBegin) {
    const Value* val = *siBegin;
    if (!isa<GlobalValue>(val)) continue;

    localPtrSets.push_back(SmallValSet());
    localPtrSets.back().insert(val);
    localPtr2SetIdMap[val] = localPtrSets.size() - 1;
  }
}

// Helper function to determine if the current pointer is from the
// local, region or private address spaces.
static bool isLRPInst(MachineInstr *MI, const AMDILSubtarget *STM) {
  if ((isRegionInst(MI) && STM->usesHardware(AMDIL::Caps::RegionMem)) ||
      (isLocalInst(MI) && STM->usesHardware(AMDIL::Caps::LocalMem)) ||
      (isPrivateInst(MI) && STM->usesHardware(AMDIL::Caps::PrivateMem))
      || isSemaphoreInst(MI)) {
    // FIXME: This is a hack since the frontend doesn't recognize semaphores
    // yet.
    return true;
  }
  return false;
}

/// Helper function to determine if the I/O instruction uses
/// global device memory or not.
static bool usesGlobal(const TargetMachine &TM,
                       const AMDILTargetMachine *ATM,
                       MachineInstr *MI) {
  const AMDILSubtarget *STM = ATM->getSubtargetImpl();
  return (isGlobalInst(MI) ||
          (isRegionInst(MI) && !STM->usesHardware(AMDIL::Caps::RegionMem)) ||
          (isLocalInst(MI) && !STM->usesHardware(AMDIL::Caps::LocalMem)) ||
          (isConstantInst(MI) && !STM->usesHardware(AMDIL::Caps::ConstantMem)) ||
          (isPrivateInst(MI) && !STM->usesHardware(AMDIL::Caps::PrivateMem)));
}

// Helper function that allocates the default resource ID for the
// respective I/O types.
void AMDILPointerManagerImpl::allocateDefaultID(AMDILAS::InstrResEnc &curRes,
                                                MachineInstr *MI) {
  DEBUG(
    dbgs() << "Assigning instruction to default ID. Inst:";
    MI->dump();
  );
  // If we use global memory, lets set the Operand to
  // the ARENA_UAV_ID.
  if (usesGlobal(TM, ATM, MI) ||
      isGlobalAtomic(MI) ||
      is64BitGlobalAtomic(MI) ||
      isArenaAtomic(MI)) {
    if (isConstantInst(MI)) {
      curRes.bits.ResourceID = STM->getResourceID(AMDIL::CONSTANT_ID);
    } else {
      curRes.bits.ResourceID = STM->getResourceID(AMDIL::GLOBAL_ID);
    }
    if (isAtomicInst(MI)) {
      MI->getOperand(MI->getNumOperands()-1)
        .setImm(curRes.bits.ResourceID);
    }
    if (curRes.bits.ResourceID == 8 &&
        !STM->isSupported(AMDIL::Caps::ArenaSegment)) {
    }
    mMFI->uav_insert(curRes.bits.ResourceID);
  } else if (isPrivateInst(MI)) {
    curRes.bits.ResourceID = STM->getResourceID(AMDIL::SCRATCH_ID);
    mMFI->setUsesScratch();
  } else if (isLocalInst(MI) || isLocalAtomic(MI) || is64BitLocalAtomic(MI)) {
    curRes.bits.ResourceID = STM->getResourceID(AMDIL::LDS_ID);
    mMFI->setUsesLDS();
    if (isAtomicInst(MI)) {
      assert(curRes.bits.ResourceID && "Atomic resource ID "
          "cannot be zero!");
      MI->getOperand(MI->getNumOperands()-1)
        .setImm(curRes.bits.ResourceID);
    }
    mMFI->setUsesLDS();
  } else if (isRegionInst(MI) ||
             isRegionAtomic(MI) ||
             is64BitRegionAtomic(MI)) {
    curRes.bits.ResourceID = STM->getResourceID(AMDIL::GDS_ID);
    mMFI->setUsesGDS();
    if (isAtomicInst(MI)) {
      assert(curRes.bits.ResourceID && "Atomic resource ID "
          "cannot be zero!");
      (MI)->getOperand((MI)->getNumOperands()-1)
        .setImm(curRes.bits.ResourceID);
    }
    mMFI->setUsesGDS();
  } else if (isConstantInst(MI)) {
    // If we are unknown constant instruction and the base pointer is known.
    // Set the resource ID accordingly, otherwise use the default constant ID.
    // FIXME: this should not require the base pointer to know what constant
    // it is from.
    const Value *V = getMemOpUnderlyingObject(MI, TM.getDataLayout());
    if (V && dyn_cast<AllocaInst>(V)) {
      llvm_unreachable("AllocaInst in constant address space");
        // FIXME: Need a better way to fix this. Requires a rewrite of how
        // we lower global addresses to various address spaces.
        // So for now, let's assume that there is only a single
        // constant buffer that can be accessed from a load instruction
        // that is derived from an alloca instruction.
      curRes.bits.ResourceID = 2;
    } else {
      assert(!isPtrStoreInst(MI) && "store to constant memory");
      curRes.bits.ResourceID = STM->getResourceID(AMDIL::CONSTANT_ID);
    }
    curRes.bits.HardwareInst = 1;
    mMFI->setUsesConstant();
  } else if (isAppendInst(MI)) {
    unsigned opcode = MI->getOpcode();
    if (opcode == AMDIL::APPEND_ALLOC || opcode == AMDIL::APPEND64_ALLOC) {
      curRes.bits.ResourceID = 1;
    } else {
      curRes.bits.ResourceID = 2;
    }
  }
  DEBUG(dbgs() << " => ResourceID:" << curRes.bits.ResourceID << "\n");
  setAsmPrinterFlags(MI, curRes);
}

// add module-level constant pointers into "cacheablePtrs" set
void AMDILEGPointerManagerImpl::parseConstantPtrs() {
  if (!STM->isSupported(AMDIL::Caps::CachedMem))
    return;

  llvm::StringMap<AMDILConstPtr>::iterator I = mAMI->consts_begin();
  llvm::StringMap<AMDILConstPtr>::iterator E = mAMI->consts_end();

  for (; I != E; ++I) {
    AMDILConstPtr &ConstP = I->second;
    cacheablePtrs.insert(ConstP.base);
    pointer2IndexMap[ConstP.base] = nextPointerIndex++;
  }
}

// Function that parses the arguments and updates the lookupTable with the
// add local arrays that belong to the given function into "localPtrs" set
void AMDILEGPointerManagerImpl::parseLocalArrays() {
  const Function* func = MF.getFunction();
  const AMDILKernel *krnl = mAMI->getKernel(func->getName());
  if (!krnl)
    return;
  const AMDILLocalArg* lvgv = krnl->lvgv;
  if (!lvgv)
    return;

  llvm::SmallVector<AMDILArrayMem *, DEFAULT_VEC_SLOTS>::const_iterator it
    = lvgv->local.begin();
  llvm::SmallVector<AMDILArrayMem *, DEFAULT_VEC_SLOTS>::const_iterator end
    = lvgv->local.end();
  for (; it != end; ++it) {
    const AMDILArrayMem* local = *it;
    localPtrs.insert(local->base);
    pointer2IndexMap[local->base] = nextPointerIndex++;
  }
}

static inline unsigned getArgReg(unsigned RegNum,
                                 const AMDILMachineFunctionInfo &mMFI)
{
  if (RegNum < mMFI.getNumArgRegs())
    return mMFI.getArgReg(RegNum);
  return RegNum;
}

static void dump(const Argument* CurArg, MachineFunction& MF) {
  CurArg->dump();
  dbgs() << "in function: " << MF.getFunction()->getName() << '\n';
  MF.getFunction()->dump();
  MF.dump();
}

// Function that parses the arguments and updates the lookupTable with the
// pointer -> register mapping. This function also checks for cacheable
// pointers and updates the CacheableSet with the arguments that
// can be cached based on the readonlypointer annotation. The final
// purpose of this function is to update the images and counters
// with all pointers that are either images or atomic counters.
uint32_t AMDILEGPointerManagerImpl::parseArguments() {
  bool isKernel = mMFI->isKernel();
  uint32_t WriteOnlyImages = 0;
  uint32_t ReadOnlyImages = 0;
  std::string CachedKernelName = "llvm.readonlypointer.annotations.";
  CachedKernelName.append(MF.getFunction()->getName());
  GlobalVariable *GV
    = MF.getFunction()->getParent()->getGlobalVariable(CachedKernelName);
  unsigned CBNum = 0;
  unsigned RegNum = 0;
  bool AssumeConflict = false;
  for (Function::const_arg_iterator I = MF.getFunction()->arg_begin(),
      E = MF.getFunction()->arg_end(); I != E; ++I) {
    const Argument *CurArg = I;
    DEBUG(
      dbgs() << "Argument: ";
      CurArg->dump();
    );
    pointer2IndexMap[CurArg] = nextPointerIndex++;
    Type *CurType = CurArg->getType();
    if (CurType->isFPOrFPVectorTy() || CurType->isIntOrIntVectorTy()) {
      // For a scalar or vector type that is passed by value that is not a
      // opaque/struct type. We just need to increment regNum
      // the correct number of times to match the number
      // of registers that it takes up.
      size_t LoopCount = 1; // scalar takes 1 vector register
      if (CurType->isVectorTy()) {
        // Our vector register is 4x4 bytes wide. Calcuate the number of
        // vector registers needed for this vector type argument.
        VectorType *VT = dyn_cast<VectorType>(CurType);
        LoopCount = VT->getNumElements();
        if (VT->getScalarSizeInBits() == 64) {
          LoopCount = (LoopCount + 1) >> 1;
        } else {
          LoopCount = (LoopCount + 3) >> 2;
        }
      } else if (StructType *ST = dyn_cast<StructType>(CurType)){
        const DataLayout* DL = TM.getDataLayout();
        LoopCount = DL->RoundUpAlignment(DL->getTypeAllocSize(ST), 16) >> 4;
        DEBUG_WITH_TYPE("func_struct_arg",
            dbgs() << "Struct argument: Number of registers: " <<
              LoopCount << '\n';
            dump(CurArg, MF);
        );
      }

      assert(LoopCount > 0 && "Invalid number of registers for argument");

      CBNum += LoopCount;
      while (LoopCount--) {
        unsigned ArgReg = getArgReg(RegNum, *mMFI);
        lookupTable[ArgReg] = std::make_pair(~0U, CurArg);
        ++RegNum;
      }
    } else if (CurType->isPointerTy()) {
      Type *CT = dyn_cast<PointerType>(CurType)->getElementType();
      const StructType *ST = dyn_cast<StructType>(CT);
      if (ST && ST->isOpaque()) {
        StringRef Name = ST->getName();
        bool i1d_type  = Name.startswith("struct._image1d_t");
        bool i1da_type = Name.startswith("struct._image1d_array_t");
        bool i1db_type = Name.startswith("struct._image1d_buffer_t");
        bool i2d_type  = Name.startswith("struct._image2d_t");
        bool i2da_type = Name.startswith("struct._image2d_array_t");
        bool i3d_type  = Name.startswith("struct._image3d_t");
        bool c32_type  = Name.startswith("struct._counter32_t");
        bool c64_type  = Name.startswith("struct._counter64_t");
        bool sema_type = Name.startswith("struct._sema_t");
        unsigned idx = 0;
        if (i2d_type || i3d_type || i2da_type ||
            i1d_type || i1db_type || i1da_type) {
          images.insert(I);
          uint32_t ImageNum = ReadOnlyImages + WriteOnlyImages;
          if (mAMI->isReadOnlyImage(MF.getFunction()->getName(), ImageNum)) {
            DEBUG(
              dbgs() << "Pointer: '" << CurArg->getName()
                << "' is a read only image # " << ReadOnlyImages << "!\n";
            );
            // We store the CBNum along with the image number so that we can
            // correctly encode the 'info' intrinsics.
            idx = ReadOnlyImages++;
          } else if (mAMI->isWriteOnlyImage(MF.getFunction()->getName(), ImageNum)) {
            DEBUG(
              dbgs() << "Pointer: '" << CurArg->getName()
                << "' is a write only image # " << WriteOnlyImages << "!\n";
            );
            // We store the CBNum along with the image number so that we can
            // correctly encode the 'info' intrinsics.
            idx = WriteOnlyImages++;
          } else {
            llvm_unreachable("Read/Write images are not supported!");
          }
          unsigned ArgReg = getArgReg(RegNum, *mMFI);
          lookupTable[ArgReg] = std::make_pair((CBNum << 16 | idx), CurArg);
          ++RegNum;
          CBNum += 2;
          continue;
        } else if (c32_type || c64_type) {
          DEBUG(
            dbgs() << "Pointer: '" << CurArg->getName()
              << "' is a " << (c32_type ? "32" : "64")
              << " bit atomic counter type!\n";
          );
          counters.push_back(I);
        } else if (sema_type) {
          DEBUG(
            dbgs() << "Pointer: '" << CurArg->getName()
              << "' is a semaphore type!\n";
          );
          semaphores.push_back(I);
        }
      } else if (isa<StructType>(CT)) {
        if (containsPointerType(CT)) {
          // Because a pointer inside of a struct/union may be
          // aliased to another pointer we need to take the conservative
          // and assume all pointers alias.
          AssumeConflict = true;
        }
      }

      uint32_t AS = dyn_cast<PointerType>(CurType)->getAddressSpace();
      if (STM->isSupported(AMDIL::Caps::CachedMem)) {
        bool cacheable = false;
        if (AS == AMDILAS::CONSTANT_ADDRESS) {
          cacheable = true;
        } else if (GV && GV->hasInitializer()) {
          const ConstantArray *NameArray
            = dyn_cast_or_null<ConstantArray>(GV->getInitializer());
          if (NameArray) {
            for (unsigned I = 0, N = NameArray->getNumOperands(); I < N; ++I) {
              const GlobalVariable *GV
                = dyn_cast_or_null<GlobalVariable>(
                  NameArray->getOperand(I)->getOperand(0));
              const ConstantDataArray *ArgName =
                dyn_cast_or_null<ConstantDataArray>(GV->getInitializer());
              if (!ArgName) {
                continue;
              }
              StringRef ArgStr = ArgName->getAsString();
              StringRef CurStr = CurArg->getName();
              if (ArgStr == CurStr) {
                cacheable = true;
                break;
              }
            }
          }
        }
        if (cacheable) {
          DEBUG(dbgs() << "Pointer: '" << CurArg->getName()
                       << "' is cacheable!\n";);
          cacheablePtrs.insert(CurArg);
        }
      }

      // Handle the case where the kernel argument is a pointer
      unsigned ArgReg = getArgReg(RegNum, *mMFI);
      DEBUG(
        dbgs() << "Pointer: " << CurArg->getName() << " is assigned ";
        if (AS == AMDILAS::GLOBAL_ADDRESS) {
          dbgs() << "uav "
                 << STM->getResourceID(AMDIL::GLOBAL_ID);
        } else if (AS == AMDILAS::PRIVATE_ADDRESS) {
          dbgs() << "scratch "
                 << STM->getResourceID(AMDIL::SCRATCH_ID);
        } else if (AS == AMDILAS::LOCAL_ADDRESS) {
          dbgs() << "lds "
                 << STM->getResourceID(AMDIL::LDS_ID);
        } else if (AS == AMDILAS::CONSTANT_ADDRESS) {
          dbgs() << "cb " <<
            STM->getResourceID(AMDIL::CONSTANT_ID);
        } else if (AS == AMDILAS::REGION_ADDRESS) {
          dbgs() << "gds "
                 << STM->getResourceID(AMDIL::GDS_ID);
        } else {
          llvm_unreachable("Found an address space that we don't support!");
        }
        dbgs() << " @ register " << ArgReg << ". Inst: ";
        CurArg->dump();
      );

      switch (AS) {
        default:
          lookupTable[ArgReg]
            = std::make_pair(STM->getResourceID(AMDIL::GLOBAL_ID), CurArg);
          break;
        case AMDILAS::LOCAL_ADDRESS:
          lookupTable[ArgReg]
            = std::make_pair(STM->getResourceID(AMDIL::LDS_ID), CurArg);
          mMFI->setHasLDSArg();
          localPtrs.insert(CurArg);
          localPtrMap[ArgReg].insert(CurArg);
          break;
        case AMDILAS::REGION_ADDRESS:
          lookupTable[ArgReg]
            = std::make_pair(STM->getResourceID(AMDIL::GDS_ID), CurArg);
          mMFI->setHasGDSArg();
          break;
        case AMDILAS::CONSTANT_ADDRESS:
          lookupTable[ArgReg]
            = std::make_pair(STM->getResourceID(AMDIL::CONSTANT_ID), CurArg);
          mMFI->setHasConstantArg();
          break;
        case AMDILAS::PRIVATE_ADDRESS:
          lookupTable[ArgReg]
            = std::make_pair(STM->getResourceID(AMDIL::SCRATCH_ID), CurArg);
          mMFI->setHasScratchArg();
          break;
      }
      // In this case we need to increment it once.
      ++RegNum;
      ++CBNum;
    } else {
      // Is anything missing that is legal in CL?
#ifndef NDEBUG
      dbgs() << "Error: Cannot handle argument: ";
      CurArg->dump();
      dbgs() << "in function: " << MF.getFunction()->getName() << '\n';
      MF.getFunction()->dump();
      MF.dump();
#endif
      llvm_unreachable("Current type is not supported!");
      unsigned ArgReg = getArgReg(RegNum, *mMFI);
      lookupTable[ArgReg]
        = std::make_pair(STM->getResourceID(AMDIL::GLOBAL_ID), CurArg);
      ++RegNum;
      ++CBNum;
    }
  }

  // if we need to assume all pointers aliasing, add all pointer args to
  // the conflictPtrs set
  if (AssumeConflict) {
    for (Function::const_arg_iterator I = MF.getFunction()->arg_begin(),
        E = MF.getFunction()->arg_end(); I != E; ++I) {
      const Argument *CurArg = I;
      if (!CurArg->hasNoAliasAttr()) {
        conflictPtrs.insert(I);
        DEBUG(dbgs() << "[conflictPtrs.insert] " << CurArg->getName()
            << "(parseArguments AssumeConflict)\n");
      }
    }
  }

  return WriteOnlyImages;
}

// Given a load, store or atomic instruction, if it's a local instruction,
// and if its pointer oper derives from multiple local pointers, then
// merge the sets that the conflicting local pointers belong to,
// so that in the end local pointers that conflict with each other
// are in the same set.
void AMDILEGPointerManagerImpl::detectConflictLocalPtrs(
  MachineInstr *MI,
  unsigned Reg,
  const AMDILSubtarget *STM) {
  assert((isLoadInst(MI) || isStoreInst(MI) || isAtomicInst(MI)) &&
         "Unexpected instruction type");
  assert(isLocalInst(MI) && STM->usesHardware(AMDIL::Caps::LocalMem) && "not local");
  Reg2ValSet::iterator FoundLocal = localPtrMap.find(Reg);
  if (FoundLocal == localPtrMap.end()) {
    // Lost track of which local pointers we are loading from/storing to.
    // For example, the local pointer address has been stored to a private
    // variable and loaded back. Conservatively disable per pointer LDS.
    doPerPointerLDS = false;
    return;
  }

  SmallValSet& Locals = FoundLocal->second;
  assert(!Locals.empty() && "sanity");

  // see if one of the local pointers is a kernel argument
  bool HasKernelArg = false;
  for (SmallValSet::iterator I = Locals.begin(), E = Locals.end();
       I != E; ++I) {
    if (!isa<GlobalValue>(*I)) {
      HasKernelArg = true;
      break;
    }
  }
  // if one of the local pointers is an argument, merge to the default
  // local buffer set, otherwise merge to the set that the first local
  // belongs to
  unsigned DstSetId;
  if (HasKernelArg) {
    DstSetId = 0;
  } else {
    DstSetId = localPtr2SetIdMap[*Locals.begin()];
  }

  for (SmallValSet::iterator I = Locals.begin(), E = Locals.end();
       I != E; ++I) {
    const Value* Local = *I;
    unsigned CurSetId = localPtr2SetIdMap[Local];
    if (CurSetId == DstSetId)
      continue;

    // Point the local pointers in current set to dst set
    for (SmallValSet::iterator J = localPtrSets[CurSetId].begin(),
         JE = localPtrSets[CurSetId].end();
         J != JE; ++J) {
      localPtr2SetIdMap[*J] = DstSetId;
    }

    // merge the set current local belongs to the set dst local belongs
    localPtrSets[DstSetId].insert(localPtrSets[CurSetId].begin(),
                                  localPtrSets[CurSetId].end());
    localPtrSets[CurSetId].clear();

    // merge instructions current set accesses to dst set
    localSetId2InstMap[DstSetId].insert(
      localSetId2InstMap[DstSetId].end(),
      localSetId2InstMap[CurSetId].begin(),
      localSetId2InstMap[CurSetId].end());
  }

  // add MI to list of instructions dst set accesses
  localSetId2InstMap[DstSetId].push_back(MI);
}

// The call stack is interesting in that even in SSA form, it assigns
// registers to the same value's over and over again. So we need to
// ignore the values that are assigned and just deal with the input
// and return registers.
// Returns true if the call instruction is successfully parsed, false otherwise.
bool AMDILEGPointerManagerImpl::parseCall(const MachineInstr *MI) {
  unsigned NumOps = MI->getNumOperands();
  for (unsigned i = 0, n = MI->getNumOperands(); i < n; ++i) {
    const MachineOperand &MO = MI->getOperand(i);
    if (!MO.isReg())
      continue;
    unsigned reg = MO.getReg();
    RVPVec::const_iterator I = lookupTable.find(reg);
    if (I == lookupTable.end())
      continue;
    if (MO.isDef()) {
      // this is a kill of "reg"
      lookupTable.erase(reg);
      DEBUG(printReg(reg); dbgs() << " is killed at call: " << *MI);
    } else {
      // if pointer is passed into a function call, conservatively mark
      // the pointer as having conflict
      const Value *V = I->second.second;
      if (V && isa<PointerType>(V->getType()) &&
         (V->getType()->getPointerAddressSpace() != AMDILAS::GLOBAL_ADDRESS ||
          ptrEqSet->requiresDefaultResId(V))) {
        conflictPtrs.insert(V);
        DEBUG(printReg(reg); dbgs() << " is killed at call: " << *MI);
        DEBUG(dbgs() << *V << " passed into call through "; printReg(reg);
              dbgs() << ". Mark pointer as having conflict.\n");
      }
    }
  }
  return true;
}

// Detect if the current instruction conflicts with another instruction
// and add the instruction to the correct location accordingly.
void AMDILEGPointerManagerImpl::detectConflictInst(MachineInstr *MI,
                                                   AMDILAS::InstrResEnc &curRes,
                                                   bool isLoadStore,
                                                   unsigned reg,
                                                   unsigned dstReg) {
  // If the instruction does not have a point path flag
  // associated with it, then we know that no other pointer
  // hits this instruciton.
  if (!curRes.bits.PointerPath) {
    if (dyn_cast<PointerType>(lookupTable[reg].second->getType())) {
      curRes.bits.PointerPath = 1;
    }
    // We don't want to transfer to the register number
    // between load/store because the load dest can be completely
    // different pointer path and the store doesn't have a real
    // destination register.
    if (!isLoadStore) {
      DEBUG(
        if (dyn_cast<PointerType>(lookupTable[reg].second->getType())) {
          dbgs() << "Pointer: " << lookupTable[reg].second->getName();
          assert(dyn_cast<PointerType>(lookupTable[reg].second->getType())
              && "Must be a pointer type for an instruction!");
          switch (dyn_cast<PointerType>(
                lookupTable[reg].second->getType())->getAddressSpace())
          {
            case AMDILAS::GLOBAL_ADDRESS:  dbgs() << " UAV: "; break;
            case AMDILAS::LOCAL_ADDRESS: dbgs() << " LDS: "; break;
            case AMDILAS::REGION_ADDRESS: dbgs() << " GDS: "; break;
            case AMDILAS::PRIVATE_ADDRESS: dbgs() << " SCRATCH: "; break;
            case AMDILAS::CONSTANT_ADDRESS: dbgs() << " CB: "; break;

          }
          dbgs() << lookupTable[reg].first << " ";
          printReg(reg);
          dbgs()  << " assigned to ";
          printReg(dstReg);
          dbgs()  << ". Inst: " << *MI;
        }
      );
      // We don't want to do any copies if the register is not virtual
      // as it is the result of a CALL. ParseCallInst handles the
      // case where the input and output need to be linked up
      // if it occurs. The easiest way to check for virtual
      // is to check the top bit.
      lookupTable[dstReg] = lookupTable[reg];
    }
  } else {
    if (dyn_cast<PointerType>(lookupTable[reg].second->getType())) {
      // Otherwise we have a conflict between two pointers somehow.
      curRes.bits.ConflictPtr = 1;
      DEBUG(
        dbgs() << "Pointer: " << lookupTable[reg].second->getName();
        assert(dyn_cast<PointerType>(lookupTable[reg].second->getType())
            && "Must be a pointer type for a conflict instruction!");
        switch (dyn_cast<PointerType>(
              lookupTable[reg].second->getType())->getAddressSpace()) {
          case AMDILAS::GLOBAL_ADDRESS:
            dbgs() << " UAV: ";
            break;
          case AMDILAS::LOCAL_ADDRESS:
            dbgs() << " LDS: ";
            break;
          case AMDILAS::REGION_ADDRESS:
            dbgs() << " GDS: ";
            break;
          case AMDILAS::PRIVATE_ADDRESS:
            dbgs() << " SCRATCH: "; break;

          case AMDILAS::CONSTANT_ADDRESS:
            dbgs() << " CB: ";
            break;
        }
        dbgs() << lookupTable[reg].first << " "; printReg(reg);
        if (!InstToPtrMap[MI].empty()) {
          dbgs() << " conflicts with:\n ";
          for (PtrSet::iterator psib = InstToPtrMap[MI].begin(),
              psie = InstToPtrMap[MI].end(); psib != psie; ++psib) {
            dbgs() << "\t\tPointer: " << (*psib)->getName() << " ";
            assert(dyn_cast<PointerType>((*psib)->getType())
                && "Must be a pointer type for a conflict instruction!");
            (*psib)->dump();
          }
        } else {
          dbgs() << ".";
        }
        dbgs() << " Inst: ";
        MI->dump();
      );
    }
    // Add the conflicting values to the pointer set for the instruction
    if (dyn_cast<PointerType>(lookupTable[reg].second->getType())) {
      InstToPtrMap[MI].insert(lookupTable[reg].second);
    }
    // We don't want to add the destination register if
    // we are a load or store.
    if (!isLoadStore && lookupTable[dstReg].second &&
      dyn_cast<PointerType>(lookupTable[dstReg].second->getType())) {
      InstToPtrMap[MI].insert(lookupTable[dstReg].second);
    }
  }
  setAsmPrinterFlags(MI, curRes);
}

// In this case we want to handle a load instruction.
// Returns true if the load instruction is successfully parsed, false otherwise.
bool AMDILEGPointerManagerImpl::parseLoadInst(MachineInstr *MI) {
  assert(isPtrLoadInst(MI) && "Only a load instruction can be parsed by "
      "the parseLoadInst function.");
  AMDILAS::InstrResEnc curRes;
  getAsmPrinterFlags(MI, curRes);
  unsigned dstReg = MI->getOperand(0).getReg();
  unsigned idx = 0;
  const Value *basePtr = NULL;
  if (MI->getOperand(1).isReg()) {
    idx = MI->getOperand(1).getReg();
    assert(!printfRegSet.count(idx) && "Loading from Printf buffer");
    basePtr = lookupTable[idx].second;
    // If we don't know what value the register
    // is assigned to, then we need to special case
    // this instruction.
  } else if (MI->getOperand(1).isFI()) {
    DEBUG(
      dbgs() << "Found an instruction with a frame index #"
             << MI->getOperand(1).getIndex() << " with ";
      printReg(dstReg); dbgs() << "!\n";
    );
    idx = MI->getOperand(1).getIndex();
    lookupTable[dstReg] = FIToPtrMap[idx];
  } else if (MI->getOperand(1).isCPI()) {
    DEBUG(
      dbgs() << "Found an instruction with a CPI index #"
        << MI->getOperand(1).getIndex() << " with ";
      printReg(dstReg); dbgs() << "!\n";
    );
    cpool.insert(MI);
  }

  // if this is a local inst, detect if we find conflicting local ptrs
  if (doPerPointerLDS && isLocalInst(MI) &&
      STM->usesHardware(AMDIL::Caps::LocalMem) &&
      MI->getOperand(1).isReg()) {
    detectConflictLocalPtrs(MI, idx, STM);
    return true;
  }

  // If we are a hardware region or private, then we don't need to track
  // as there is only one resource ID that we need to know about, so we
  // map it using allocateDefaultID, which maps it to the default.
  if (isLRPInst(MI, STM) || !basePtr) {
    allocateDefaultID(curRes, MI);
    return true;
  }
  // We have a load instruction so we map this instruction
  // to the pointer and insert it into the set of known
  // load instructions.
  if (dyn_cast<PointerType>(basePtr->getType())) {
    InstToPtrMap[MI].insert(basePtr);
    PtrToInstMap[basePtr].push_back(MI);
  }

  if (isGlobalInst(MI)) {
    // Add to the cacheable set for the block. If there was a store earlier
    // in the block, this call won't actually add it to the cacheable set.
    bbCacheable[MI->getParent()].addPossiblyCacheableInst(MI);
  }

  DEBUG(
    dbgs() << "Assigning instruction to load pointer ";
    dbgs() << basePtr->getName() << ". Inst: ";
    MI->dump();
  );
  detectConflictInst(MI, curRes, true, idx, dstReg);
  return true;
}

// In this case we want to handle a store instruction.
// Returns true if the store instruction is successfully parsed, false otherwise
bool AMDILEGPointerManagerImpl::parseStoreInst(MachineInstr *MI) {
  assert(isPtrStoreInst(MI) && "Only a store instruction can be parsed by "
      "the parseStoreInst function.");
  AMDILAS::InstrResEnc curRes;
  getAsmPrinterFlags(MI, curRes);
  unsigned dstReg = ~0U;
  if (MI->getOperand(0).isFI()) {
    dstReg = MI->getOperand(0).getIndex();
  } else if (MI->getOperand(0).isReg()) {
    dstReg = MI->getOperand(0).getReg();
    assert(!printfRegSet.count(dstReg) && "Storing Printf buffer address");
  } else {
  }

  // If the data part of the store instruction is known to
  // be a pointer, then we need to mark this pointer as being
  // a byte pointer. This is the conservative case that needs
  // to be handled correctly.
  if (dstReg != ~0U && lookupTable[dstReg].second && lookupTable[dstReg].first != ~0U) {
    curRes.bits.ConflictPtr = 1;
    DEBUG(
      dbgs() << "Found a case where the pointer is being stored!\n";
      MI->dump();
      dbgs() << "Pointer is ";
      lookupTable[dstReg].second->print(dbgs());
      dbgs() << "\n";
    );
    if (lookupTable[dstReg].second->getType()->isPointerTy()) {
      conflictPtrs.insert(lookupTable[dstReg].second);
      DEBUG(dbgs() << "[conflictPtrs.insert] " << *lookupTable[dstReg].second
        << "(parseStoreInst)\n");
    }
  }

  // Before we go through the special cases, for the cacheable information
  // all we care is if the store if global or not.
  if (!isLRPInst(MI, STM)) {
    bbCacheable[MI->getParent()].setReachesExit();
  }

  // If the address is not a register address,
  // then we need to lower it as an unknown id.
  if (!MI->getOperand(1).isReg()) {
    if (MI->getOperand(1).isCPI() || MI->getOperand(1).isFI()) {
      DEBUG(
        dbgs() << "Found an instruction with a ";
        if (MI->getOperand(1).isCPI())
          dbgs() << "CPI";
        else
          dbgs() << "frame";
        dbgs() << " index #" << MI->getOperand(1).getIndex() << " with ";
        if (MI->getOperand(0).isReg()) {
          printReg(dstReg); dbgs() << "\n";
        }
        else if (MI->getOperand(0).isFI())
          dbgs() << "frameindex " << dstReg << "\n";
      );
    }
    if (MI->getOperand(1).isCPI()) {
      cpool.insert(MI);
    } else if (MI->getOperand(1).isFI()) {
      // If we are a frame index and we are storing a pointer there, lets
      // go ahead and assign the pointer to the location within the frame
      // index map so that we can get the value out later.
      IntValPair &tmp = lookupTable[dstReg];
      if (MI->getOperand(0).isFI()) {
        tmp = FIToPtrMap[dstReg];
      }
      if (!tmp.second) {
        // If we don't have a valid pointer, then renumber the
        // register from 0 to the VREG/FI that we are
        // storing the data of.
        tmp.first = dstReg;
      }
      FIToPtrMap[MI->getOperand(1).getIndex()] = tmp;
    }

    allocateDefaultID(curRes, MI);
    return true;
  }

  unsigned reg = MI->getOperand(1).getReg();
  if (trackPrintfs && printfRegSet.count(reg)) {
    printfInsts.insert(MI);
    return true;
  }

  // if this is a local inst, detect if we find conflicting local ptrs
  if (doPerPointerLDS && isLocalInst(MI) &&
      STM->usesHardware(AMDIL::Caps::LocalMem)) {
    detectConflictLocalPtrs(MI, reg, STM);
    return true;
  }

  // If we don't know what value the register
  // is assigned to, then we need to special case
  // this instruction.
  if (!lookupTable[reg].second ||
    !dyn_cast<PointerType>(lookupTable[reg].second->getType())) {
    allocateDefaultID(curRes, MI);
    return true;
  }

  // If we are a hardware region or private, then we don't need to track
  // as there is only one resource ID that we need to know about, so we
  // map it using allocateDefaultID, which maps it to the default.
  // This is also the case for REGION_ADDRESS and PRIVATE_ADDRESS.
  if (isLRPInst(MI, STM)) {
    allocateDefaultID(curRes, MI);
    return true;
  }

  // We have a store instruction so we map this instruction
  // to the pointer and insert it into the set of known
  // store instructions.
  InstToPtrMap[MI].insert(lookupTable[reg].second);
  PtrToInstMap[lookupTable[reg].second].push_back(MI);
  uint16_t RegClass = MI->getDesc().OpInfo[0].RegClass;
  switch (RegClass) {
    default:
      break;
    case AMDIL::GPRI8RegClassID:
    case AMDIL::GPRV2I8RegClassID:
    case AMDIL::GPRI16RegClassID:
      if (usesGlobal(TM, ATM, MI)) {
        DEBUG(
          dbgs() << "Annotating instruction as Byte Store. Inst: ";
          MI->dump();
        );
        curRes.bits.ByteStore = 1;
        setAsmPrinterFlags(MI, curRes);
        const PointerType *PT = dyn_cast<PointerType>(
            lookupTable[reg].second->getType());
        if (trackBytePtrs && PT) {
          bytePtrs.insert(lookupTable[reg].second);
        }
      }
      break;
  };
  // If we are a truncating store, then we need to determine the
  // size of the pointer that we are truncating to, and if we
  // are less than 32 bits, we need to mark the pointer as a
  // byte store pointer.
  if (isGlobalInst(MI) && isStoreInst(MI) && isSub32BitIOInst(MI)) {
      curRes.bits.ByteStore = 1;
      setAsmPrinterFlags(MI, curRes);
      if (trackBytePtrs)
        bytePtrs.insert(lookupTable[reg].second);
  }

  DEBUG(
    dbgs() << "Assigning instruction to store pointer ";
    dbgs() << lookupTable[reg].second->getName() << ". Inst: ";
    MI->dump();
  );
  if (dstReg != ~0U) {
    detectConflictInst(MI, curRes, true, reg, dstReg);
  }
  return true;
}

// In this case we want to handle an atomic instruction.
// Returns true if the atomic instruction is successfully parsed, false
// otherwise
bool AMDILEGPointerManagerImpl::parseAtomicInst(MachineInstr *MI) {
  assert(isAtomicInst(MI) && "Only an atomic instruction can be parsed by "
      "the parseAtomicInst function.");
  AMDILAS::InstrResEnc curRes;
  unsigned dstReg = MI->getOperand(0).getReg();
  assert(!printfRegSet.count(dstReg) && "Atomic op on Printf buffer");
  unsigned reg = 0;
  getAsmPrinterFlags(MI, curRes);
  int numOps = MI->getNumOperands() - 1;
  bool found = false;
  while (--numOps >= 0) {
    MachineOperand &Op = MI->getOperand(numOps);
    if (!Op.isReg()) {
      continue;
    }
    reg = Op.getReg();
    assert(!printfRegSet.count(dstReg) && "Atomic op on Printf buffer");

    if (doPerPointerLDS && isLocalInst(MI) &&
        STM->usesHardware(AMDIL::Caps::LocalMem)) {
      detectConflictLocalPtrs(MI, reg, STM);
      continue;
    }

    // If the register is not known to be owned by a pointer
    // then we can ignore it
    if (!lookupTable[reg].second ||
      !dyn_cast<PointerType>(lookupTable[reg].second->getType())) {
      continue;
    }

    // if the pointer is known to be region or private, then we
    // can ignore it.  Although there are no private atomics, we still
    // do this check so we don't have to write a new function to check
    // for only region.
    if (isLRPInst(MI, STM)) {
      continue;
    }
    found = true;
    InstToPtrMap[MI].insert(lookupTable[reg].second);
    PtrToInstMap[lookupTable[reg].second].push_back(MI);

    // We now know we have an atomic operation on global memory.
    // This is a store so must update the cacheable information.
    bbCacheable[MI->getParent()].setReachesExit();

    // Force pointers that are used by atomics to be in the arena.
    // If they were allowed to be accessed as RAW they would cause
    // all access to use the slow complete path.
    DEBUG(
      dbgs() << __LINE__ << ": Setting byte store bit on atomic instruction: ";
      MI->dump();
    );
    curRes.bits.ByteStore = 1;
    if (trackBytePtrs)
      bytePtrs.insert(lookupTable[reg].second);

    DEBUG(
      dbgs() << "Assigning instruction to atomic ";
      dbgs() << lookupTable[reg].second->getName()<< ". Inst: ";
      MI->dump();
    );
    detectConflictInst(MI, curRes, true, reg, dstReg);
  }
  if (!found) {
    allocateDefaultID(curRes, MI);
  }
  return true;
}

// In this case we want to handle a counter instruction.
// Returns true if the append instruction is successfully parsed, false
// otherwise
bool AMDILEGPointerManagerImpl::parseAppendInst(MachineInstr *MI) {
  assert(isAppendInst(MI) && "Only an atomic counter instruction can be "
      "parsed by the parseAppendInst function.");
  AMDILAS::InstrResEnc curRes;
  unsigned dstReg = MI->getOperand(0).getReg();
  unsigned reg = MI->getOperand(1).getReg();
  assert(!printfRegSet.count(dstReg) && "Counter op on Printf buffer");
  assert(!printfRegSet.count(reg) && "Counter op on Printf buffer");
  getAsmPrinterFlags(MI, curRes);
  // If the register is not known to be owned by a pointer
  // then we set it to the default
  if (!lookupTable[reg].second ||
    !dyn_cast<PointerType>(lookupTable[reg].second->getType())) {
    allocateDefaultID(curRes, MI);
    return true;
  }
  InstToPtrMap[MI].insert(lookupTable[reg].second);
  PtrToInstMap[lookupTable[reg].second].push_back(MI);
  DEBUG(
    dbgs() << "Assigning instruction to append ";
    dbgs() << lookupTable[reg].second->getName() << ". Inst: ";
    MI->dump();
  );
  detectConflictInst(MI, curRes, true, reg, dstReg);
  return true;
}

/// In this case we want to handle a counter instruction.
// Returns true if the sema instruction is successfully parsed, false otherwise
bool AMDILEGPointerManagerImpl::parseSemaInst(MachineInstr *MI) {
  assert(isSemaphoreInst(MI) && "Only an semaphore instruction can be "
      "parsed by the parseSemaInst function.");
  AMDILAS::InstrResEnc curRes;
  unsigned dstReg = MI->getOperand(0).getReg();
  assert(!printfRegSet.count(dstReg) && "Semaphore op on Printf buffer");
  getAsmPrinterFlags(MI, curRes);
  // If the register is not known to be owned by a pointer
  // then we set it to the default
  if (!lookupTable[dstReg].second ||
    !dyn_cast<PointerType>(lookupTable[dstReg].second->getType())) {
    allocateDefaultID(curRes, MI);
    return true;
  }
  InstToPtrMap[MI].insert(lookupTable[dstReg].second);
  PtrToInstMap[lookupTable[dstReg].second].push_back(MI);
  DEBUG(
    dbgs() << "Assigning instruction to semaphore ";
    dbgs() << lookupTable[dstReg].second->getName() << ". Inst: ";
    MI->dump();
  );
  return true;
}

// In this case we want to handle an Image instruction.
// Returns true if the image instruction is successfully parsed, false otherwise
bool AMDILEGPointerManagerImpl::parseImageInst(MachineInstr *MI) {
  assert(isImageInst(MI) && "Only an image instruction can be "
      "parsed by the parseImageInst function.");
  AMDILAS::InstrResEnc curRes;
  getAsmPrinterFlags(MI, curRes);
  unsigned reg;
  if (isWriteImageInst(MI)) {
    reg = MI->getOperand(0).getReg();
  } else {
    reg = MI->getOperand(1).getReg();
  }
  assert(!printfRegSet.count(reg) && "Image op on Printf buffer");

  // If the register is not known to be owned by a pointer
  // then we set it to the default
  if (!lookupTable[reg].second ||
    !dyn_cast<PointerType>(lookupTable[reg].second->getType())) {
#ifndef NDEBUG
    // FIXME: this unreachable call should go away after inter-procedure
    // analysis of pointer manager is implemented
    std::map<const Function*, bool> work;
    bool CalledByKernel = isCalledByKernel(MF.getFunction(), work);
    if (mMFI->isKernel() || CalledByKernel)
      llvm_unreachable("This should not happen for images!");
#endif
    allocateDefaultID(curRes, MI);
    return true;
  }
  curRes.bits.ResourceID = lookupTable[reg].first & 0xFFFF;
  curRes.bits.isImage = 1;
  InstToPtrMap[MI].insert(lookupTable[reg].second);
  PtrToInstMap[lookupTable[reg].second].push_back(MI);
  DEBUG(
    dbgs() << "Assigning instruction to image ";
    dbgs() << lookupTable[reg].second->getName() << ". Inst: ";
    MI->dump();
  );

  if (isWriteImageInst(MI)) {
    setAsmPrinterFlags(MI, curRes);
    return true;
  }

  if (isReadImageInst(MI)) {
      if (MI->getOperand(SAMPLER_INDEX).isReg()) {
        // Our sampler is not a literal value.
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        std::string sampler_name = "";
        unsigned reg = MI->getOperand(SAMPLER_INDEX).getReg();
        if (lookupTable[reg].second) {
          sampler_name = lookupTable[reg].second->getName();
        }
        uint32_t val = ~0U;
        if (sampler_name.empty()) {
          sampler_name = findSamplerName(MI, val);
        }
        val = mMFI->addSampler(sampler_name, val);
        DEBUG(
          dbgs() << "Mapping kernel sampler " << sampler_name
            << " to sampler number " << val << " for Inst:\n";
          MI->dump();
        );
        MI->getOperand(SAMPLER_INDEX).ChangeToImmediate(val);
      } else {
        // Our sampler is known at runtime as a literal, lets make sure
        // that the metadata for it is known.
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        ::sprintf(buffer,"_%d", (int32_t)MI->getOperand(SAMPLER_INDEX).getImm());
        std::string sampler_name = std::string("unknown") + std::string(buffer);
        uint32_t val = mMFI->addSampler(sampler_name, MI->getOperand(SAMPLER_INDEX).getImm());
        DEBUG(
          dbgs() << "Mapping internal sampler " << sampler_name
            << " to sampler number " << val << " for Inst:\n";
          MI->dump();
        );
        MI->getOperand(SAMPLER_INDEX).setImm(val);
      }
  } else if (isImageInfo0Inst(MI)) {
      curRes.bits.ResourceID = lookupTable[reg].first >> 16;
  } else if (isImageInfo1Inst(MI)) {
      curRes.bits.ResourceID = (lookupTable[reg].first >> 16) + 1;
  }
  setAsmPrinterFlags(MI, curRes);
  return true;
}

// if addri's address is a local array, map addri's dest reg to the local array
// if addri's address is a module-level constant pointer, map addri's dest reg
// to the constant pointer.
// Returns true if the addri instruction is successfully parsed, false otherwise
bool AMDILEGPointerManagerImpl::parseAddriInst(MachineInstr *MI) {
  assert(isAddriInst(MI) && "Only a Addri instruction can be parsed by "
         "the parseAddriInst function.");
  unsigned dstReg = MI->getOperand(0).getReg();
  assert(!printfRegSet.count(dstReg) && "Addri inst on Printf buffer address");
  MachineOperand &MemOp = MI->getOperand(1);
  // skip if not module-level pointer
  if (!MemOp.isGlobal()) return false;
  const GlobalValue* GV = MemOp.getGlobal();
  // track local pointers
  if (localPtrs.count(GV)) {
    DEBUG(printReg(dstReg); dbgs() << " map to " << GV->getName() << "\n";);
    localPtrMap[dstReg].insert(GV);
    AddriVec.push_back(GVInstPair(GV, MI));
    mMFI->setUsesLDS();
    return true;
  }

  // track global constant pointers
  const Value *BasePtr = mAMI->getConstBase(GV->getName());
  if (!BasePtr) return false;

  assert(BasePtr == GV && "unexpected global pointer type");
  DEBUG(printReg(dstReg); dbgs() << " map to " << GV->getName() << "\n";);
  unsigned constID = STM->getResourceID(AMDIL::CONSTANT_ID);
  lookupTable[dstReg] = std::make_pair(constID, GV);
  AddriVec.push_back(GVInstPair(GV, MI));
  return true;
}

// This case handles the rest of the instructions
void AMDILEGPointerManagerImpl::parseInstruction(MachineInstr *MI) {
  assert(!isAtomicInst(MI) && !isPtrStoreInst(MI) && !isPtrLoadInst(MI) &&
         !isAppendInst(MI) && !isImageInst(MI) &&
      "Atomic/Load/Store/Append/Image insts should not be handled here!");
  unsigned numOps = MI->getNumOperands();
  // If we don't have any operands, we can skip this instruction
  if (!numOps) {
    return;
  }
  // if the dst operand is not a register, then we can skip
  // this instruction. That is because we are probably a branch
  // or jump instruction.
  if (!MI->getOperand(0).isReg()) {
    return;
  }
  // If we are a LOADCONSTi32, we might be a sampler, so we need
  // to propogate the LOADCONST to IMAGE[1|2|3]D[A|B][64]_READ instructions.
  if (MI->getOpcode() == AMDIL::LOADCONSTi32) {
    uint32_t val = MI->getOperand(1).getImm();

    const MachineRegisterInfo &RI = MF.getRegInfo();
    for (MachineRegisterInfo::reg_iterator I
           = RI.reg_begin(MI->getOperand(0).getReg()); !I.atEnd();) {
      MachineOperand &O = I.getOperand();
      ++I;
      if (isReadImageInst(O.getParent())) {
        DEBUG(
          dbgs() << "Found a constant sampler for image read inst: ";
          O.getParent()->print(dbgs());
        );

        O.ChangeToImmediate(val);
      }
    }
  }

  AMDILAS::InstrResEnc curRes;
  getAsmPrinterFlags(MI, curRes);
  unsigned dstReg = MI->getOperand(0).getReg();
  unsigned reg = 0;
  while (--numOps) {
    MachineOperand &Op = MI->getOperand(numOps);
    // if the operand is not a register, then we can ignore it
    if (!Op.isReg()) {
      if (Op.isCPI()) {
        cpool.insert(MI);
      }
      continue;
    }
    reg = Op.getReg();
    // propagate local ptr set from oper to dst
    Reg2ValSet::iterator it = localPtrMap.find(reg);
    if (it != localPtrMap.end()) {
      SmallValSet& locals = it->second;
      localPtrMap[dstReg].insert(locals.begin(), locals.end());
    }
    // if oper is printf address, add dst to printf reg set too
    if (trackPrintfs && printfRegSet.count(reg)) {
      printfRegSet.insert(dstReg);
    }
    // If the register is not known to be owned by a pointer
    // then we can ignore it
    if (!lookupTable[reg].second) {
      continue;
    }
    detectConflictInst(MI, curRes, false, reg, dstReg);

  }
}

// This function parses the basic block and based on the instruction type,
// calls the function to finish parsing the instruction.
void AMDILEGPointerManagerImpl::parseBasicBlock(MachineBasicBlock *MB) {
  for (MachineBasicBlock::iterator mbb = MB->begin(), mbe = MB->end();
      mbb != mbe; ++mbb) {
    MachineInstr *MI = mbb;
    bool parsed = false;
    if (MI->getOpcode() == AMDIL::CALL) {
      parsed = parseCall(MI);
    } else if (isPtrLoadInst(MI)) {
      parsed = parseLoadInst(MI);
    } else if (isPtrStoreInst(MI)) {
      parsed = parseStoreInst(MI);
    } else if (isAtomicInst(MI)) {
      parsed = parseAtomicInst(MI);
    } else if (isAppendInst(MI)) {
      parsed = parseAppendInst(MI);
    } else if (isSemaphoreInst(MI)) {
      parsed = parseSemaInst(MI);
    } else if (isImageInst(MI)) {
      parsed = parseImageInst(MI);
    } else if (isAddriInst(MI)) {
      parsed = parseAddriInst(MI);
    }
    if (!parsed) {
      parseInstruction(MI);
    }
    if (trackPrintfs &&
      (MI->getOpcode() == AMDIL::GET_PRINTF_OFFSETi32 ||
       MI->getOpcode() == AMDIL::GET_PRINTF_OFFSETi64)) {
      unsigned dstReg = MI->getOperand(0).getReg();
      printfRegSet.insert(dstReg);
    }
  }
}

// Follows the Reverse Post Order Traversal of the basic blocks to
// determine which order to parse basic blocks in.
void AMDILEGPointerManagerImpl::parseFunction() {
  std::list<MachineBasicBlock*> prop_worklist;

  ReversePostOrderTraversal<MachineFunction*> RPOT(&MF);
  for (ReversePostOrderTraversal<MachineFunction*>::rpo_iterator
      curBlock = RPOT.begin(), endBlock = RPOT.end();
      curBlock != endBlock; ++curBlock) {
    MachineBasicBlock *MB = (*curBlock);
    BlockCacheableInfo &bci = bbCacheable[MB];
    for (MachineBasicBlock::pred_iterator mbbit = MB->pred_begin(),
        mbbitend = MB->pred_end();
        mbbit != mbbitend;
        mbbit++) {
      MBBCacheableMap::const_iterator mbbcmit = bbCacheable.find(*mbbit);
      if (mbbcmit != bbCacheable.end() &&
          mbbcmit->second.storeReachesExit()) {
        bci.setReachesTop();
        break;
      }
    }

    DEBUG(
      dbgs() << "[BlockOrdering] Parsing CurrentBlock: "
      << MB->getNumber() << "\n";
    );
    parseBasicBlock(MB);

    if (bci.storeReachesExit())
      prop_worklist.push_back(MB);

    DEBUG(
      dbgs() << "BCI info: Top: " << bci.storeReachesTop() << " Exit: "
        << bci.storeReachesExit() << "\n Instructions:\n";
      for (CacheableInstrSet::const_iterator cibit = bci.cacheableBegin(),
          cibitend = bci.cacheableEnd();
          cibit != cibitend;
          ++cibit) {
        (*cibit)->dump();
      }
    );
  }

  // This loop pushes any "storeReachesExit" flags into successor
  // blocks until the flags have been fully propagated. This will
  // ensure that blocks that have reachable stores due to loops
  // are labeled appropriately.
  while (!prop_worklist.empty()) {
    MachineBasicBlock *wlb = prop_worklist.front();
    prop_worklist.pop_front();
    for (MachineBasicBlock::succ_iterator mbbit = wlb->succ_begin(),
        mbbitend = wlb->succ_end();
        mbbit != mbbitend;
        mbbit++)
    {
      BlockCacheableInfo &blockCache = bbCacheable[*mbbit];
      if (!blockCache.storeReachesTop()) {
        blockCache.setReachesTop();
        prop_worklist.push_back(*mbbit);
      }
      DEBUG(
        dbgs() << "BCI Prop info: " << (*mbbit)->getNumber() << " Top: "
          << blockCache.storeReachesTop() << " Exit: "
          << blockCache.storeReachesExit()
          << "\n";
      );
    }
  }
}

// Helper function that dumps to dbgs() information about
// a pointer set.
template <class T>
void AMDILEGPointerManagerImpl::dumpPointers(T &Ptrs, const char *str) {
  typedef typename T::iterator iterator;
  if (Ptrs.empty()) {
    return;
  }
  dbgs() << "[Dump]" << str << " found: " << "\n";
  for (iterator sb = Ptrs.begin(); sb != Ptrs.end(); ++sb) {
    (*sb)->dump();
  }
  dbgs() << "\n";
}

// Add all byte pointers to bytePtrs set
void AMDILEGPointerManagerImpl::detectBytePointers(
  std::set<const MachineInstr*>& byteInsts)
{
  bool changed = true;
  while (changed) {
    changed = false;
    for (InstPMap::iterator
        mapIter = InstToPtrMap.begin(), iterEnd = InstToPtrMap.end();
        mapIter != iterEnd; ++mapIter) {
      MachineInstr* MI = mapIter->first;
      if (byteInsts.count(MI)) {
        // already detected as byte-inst
        continue;
      }
      if (isLRPInst(MI, STM)) {
        // We don't need to deal with pointers to local/region/private memory regions
        continue;
      }
      AMDILAS::InstrResEnc curRes;
      getAsmPrinterFlags(MI, curRes);
      if (curRes.bits.isImage) {
        continue;
      }
      bool byte = false;
      // We might have a case where more than 1 pointers is going to the same
      // I/O instruction
      for (PtrSet::iterator cfIter = mapIter->second.begin(),
          cfEnd = mapIter->second.end(); cfIter != cfEnd; ++cfIter) {
        const Value *ptr = *cfIter;
        const PointerType *PT = dyn_cast<PointerType>(ptr->getType());
        assert(PT && "not a pointer");
        if (bytePtrs.count(ptr)) {
          DEBUG(
            dbgs() << "Instruction: ";
            (mapIter)->first->dump();
            dbgs() << "Base Pointer[s]:\n";
            ptr->dump();
            dbgs() << "Byte pointer found!\n";
          );
          byte = true;
          break;
        }
      }
      if (byte) {
        byteInsts.insert(MI);
        for (PtrSet::iterator cfIter = mapIter->second.begin(),
            cfEnd = mapIter->second.end(); cfIter != cfEnd; ++cfIter) {
          const Value *ptr = *cfIter;
          const PointerType *PT = dyn_cast<PointerType>(ptr->getType());
          if (PT && !bytePtrs.count(ptr)) {
            DEBUG(
              dbgs() << "Adding pointer " << (ptr)->getName()
                << " to byte set!\n";
            );
            bytePtrs.insert(ptr);
            changed = true;
          }
        }
      }
    }
  }
}

// Function that detects all the conflicting pointers and adds
// the pointers that are detected to the conflict set, otherwise
// they are added to the raw or byte set based on their usage.
void AMDILEGPointerManagerImpl::detectConflictingPointers() {
  if (InstToPtrMap.empty()) {
    return;
  }

  std::set<const MachineInstr*> byteInsts;
  if (trackBytePtrs) {
    detectBytePointers(byteInsts);
  }

  PtrSet aliasedPtrs;
  for (InstPMap::iterator
      mapIter = InstToPtrMap.begin(), iterEnd = InstToPtrMap.end();
      mapIter != iterEnd; ++mapIter) {
    DEBUG(
      dbgs() << "Instruction: ";
      (mapIter)->first->dump();
    );
    MachineInstr* MI = mapIter->first;
    AMDILAS::InstrResEnc curRes;
    getAsmPrinterFlags(MI, curRes);
    if (curRes.bits.isImage) {
      continue;
    }
    bool byte = byteInsts.count(MI);
    if (!byte) {
      // We might have a case where more than 1 pointers is going to the same
      // I/O instruction
      DEBUG(dbgs() << "Base Pointer[s]:\n");
      for (PtrSet::iterator cfIter = mapIter->second.begin(),
          cfEnd = mapIter->second.end(); cfIter != cfEnd; ++cfIter) {
        const Value *ptr = *cfIter;
        assert(isa<PointerType>(ptr->getType()) && "not a pointer");
        DEBUG(ptr->dump());

        // bool aliased = false;
        if (isLRPInst(mapIter->first, STM)) {
          // We don't need to deal with pointers to local/region/private
          // memory regions
          continue;
        }
        const Argument *arg = dyn_cast_or_null<Argument>(ptr);
        if (!STM->isSupported(AMDIL::Caps::NoAlias)
            && arg && !arg->hasNoAliasAttr()) {
          DEBUG(dbgs() << "Possible aliased pointer found!\n");
          aliasedPtrs.insert(ptr);
        }
        if (mapIter->second.size() > 1) {
          const PointerType *PT = dyn_cast<PointerType>(ptr->getType());
          if (PT) {
            DEBUG(
              dbgs() << "Adding pointer " << ptr->getName()
                << " to conflict set!\n";
            );
            conflictPtrs.insert(ptr);
            DEBUG(dbgs() << "[conflictPtrs.insert] " << ptr->getName() <<
                "(detectConflictingPointers)\n");
          }
        }
        const PointerType *PT = dyn_cast<PointerType>(ptr->getType());
        if (PT) {
          DEBUG(
            dbgs() << "Adding pointer " << ptr->getName()
              << " to raw set!\n";
          );
          rawPtrs.insert(ptr);
        }
      }
    }

    DEBUG(dbgs() << '\n');
  }
  // If we have any aliased pointers and byte pointers exist,
  // then make sure that all of the aliased pointers are
  // part of the byte pointer set.
  if (!bytePtrs.empty()) {
    for (PtrSet::iterator aIter = aliasedPtrs.begin(),
        aEnd = aliasedPtrs.end(); aIter != aEnd; ++aIter) {
      DEBUG(
        dbgs() << "Moving " << (*aIter)->getName()
          << " from raw to byte.\n";
      );
      bytePtrs.insert(*aIter);
      rawPtrs.erase(*aIter);
    }
  }
  assert((trackBytePtrs || bytePtrs.empty()) && "unexpected byte ptrs");
}
// Function that detects aliased constant pool operations.
void AMDILEGPointerManagerImpl::detectAliasedCPoolOps() {
  DEBUG(
    if (!cpool.empty()) {
      dbgs() << "Instructions w/ CPool Ops: \n";
    }
  );
  // The algorithm for detecting aliased cpool is as follows.
  // For each instruction that has a cpool argument
  // follow def-use chain
  //   if instruction is a load and load is a private load,
  //      switch to constant pool load
  for (CPoolSet::iterator cpb = cpool.begin(), cpe = cpool.end();
      cpb != cpe; ++cpb) {
    DEBUG((*cpb)->dump());

    std::queue<MachineInstr *> queue;
    std::set<MachineInstr *> visited;
    queue.push(*cpb);
    MachineInstr *cur;
    while (!queue.empty()) {
      cur = queue.front();
      queue.pop();
      if (visited.count(cur)) {
        continue;
      }
      if (isPtrLoadInst(cur) && isPrivateInst(cur)) {
        // If we are a private load and the register is
        // used in the address register, we need to
        // switch from private to constant pool load.
        DEBUG(
          dbgs() << "Found an instruction that is a private load "
            << "but should be a constant pool load.\n";
          cur->print(dbgs());
          dbgs() << '\n';
        );
        AMDILAS::InstrResEnc curRes;
        getAsmPrinterFlags(cur, curRes);
        curRes.bits.ResourceID = STM->getResourceID(AMDIL::GLOBAL_ID);
        curRes.bits.ConflictPtr = 1;
        setAsmPrinterFlags(cur, curRes);
        cur->setDesc(TM.getInstrInfo()->get(
              (cur->getOpcode() - AMDIL::PRIVATEAEXTLOAD64f32r)
              + AMDIL::CPOOLAEXTLOAD64f32r));
      } else {
        if (cur->getOperand(0).isReg()) {
          for(MachineRegisterInfo::reg_iterator
              RI = MF.getRegInfo().reg_begin(cur->getOperand(0).getReg()),
              RE = MF.getRegInfo().reg_end();
              RI != RE && RI.getOperand().isDef() && RI.getOperand().isReg(); ++RI) {
            queue.push(RI.getOperand().getParent());
          }
        }
      }
      visited.insert(cur);
    }
  }
}
// Function that detects fully cacheable pointers. Fully cacheable pointers
// are pointers that have no writes to them and no-alias is specified.
void AMDILEGPointerManagerImpl::detectFullyCacheablePointers() {
  if (PtrToInstMap.empty()) {
    return;
  }

  // 4XXX hardware doesn't support cached uav opcodes and we assume
  // no aliasing for this to work. Also in debug mode we don't do
  // any caching.
  if (!STM->isSupported(AMDIL::Caps::CachedMem)) {
    return;
  }

  if (STM->isSupported(AMDIL::Caps::NoAlias)) {
    for (PtrIMap::iterator mapIter = PtrToInstMap.begin(),
        iterEnd = PtrToInstMap.end(); mapIter != iterEnd; ++mapIter) {
      DEBUG(
        dbgs() << "Instruction: ";
        mapIter->first->dump();
      );
      // Skip the pointer if we have already detected it.
      if (cacheablePtrs.count(mapIter->first)) {
        continue;
      }
      bool cacheable = true;
      for (std::vector<MachineInstr*>::iterator
          miBegin = mapIter->second.begin(),
          miEnd = mapIter->second.end(); miBegin != miEnd; ++miBegin) {
        if (isPtrStoreInst(*miBegin)  ||
            isImageInst(*miBegin)  ||
            isAtomicInst(*miBegin) ||
            isAppendInst(*miBegin) ||
            isSemaphoreInst(*miBegin)) {
          cacheable = false;
          break;
        }
      }
      // we aren't cacheable, so lets move on to the next instruction
      if (!cacheable) {
        continue;
      }

      if (hasFunctionCalls && !useDefaultResId) {
        if (ptrEqSet->requiresDefaultResId(mapIter->first))
          continue;

        if (ptrEqSet->hasWrite(mapIter->first))
          continue;
      } else {
        // If we are in the conflict set, lets move to the next instruction
        // FIXME: we need to check to see if the pointers that conflict with
        // the current pointer are also cacheable. If they are, then add them
        // to the cacheable list and not fail.
        if (conflictPtrs.count(mapIter->first))
            continue;
      }
      // Otherwise if we have no stores and no conflicting pointers, we can
      // be added to the cacheable set.
      DEBUG(
        dbgs() << "Adding pointer " << mapIter->first->getName();
        dbgs() << " to cached set!\n";
      );
      const PointerType *PT = dyn_cast<PointerType>(mapIter->first->getType());
      if (PT) {
        cacheablePtrs.insert(mapIter->first);
      }
    }
  }
}

// Are any of the pointers in PtrSet also in the BytePtrs or the CachePtrs?
bool AMDILEGPointerManagerImpl::ptrSetIntersectsByteOrCache(PtrSet &cacheSet) {
  for (PtrSet::const_iterator psit = cacheSet.begin(),
      psitend = cacheSet.end();
      psit != psitend;
      psit++) {
    if (bytePtrs.find(*psit) != bytePtrs.end() ||
        cacheablePtrs.find(*psit) != cacheablePtrs.end()) {
      return true;
    }
  }
  return false;
}

// Function that detects which instructions are cacheable even if
// all instructions of the pointer are not cacheable. The resulting
// set of instructions will not contain Ptrs that are in the cacheable
// ptr set (under the assumption they will get marked cacheable already)
// or pointers in the byte set, since they are not cacheable.
void AMDILEGPointerManagerImpl::detectCacheableInstrs() {
  for (MBBCacheableMap::const_iterator mbbcit = bbCacheable.begin(),
         mbbcitend = bbCacheable.end(); mbbcit != mbbcitend; mbbcit++) {
    for (CacheableInstrSet::const_iterator bciit
           = mbbcit->second.cacheableBegin(),
           bciitend = mbbcit->second.cacheableEnd();
         bciit != bciitend; bciit++) {
      if (!ptrSetIntersectsByteOrCache(InstToPtrMap[*bciit])) {
        cacheableSet.insert(*bciit);
      }
    }
  }
}
// This function annotates the cacheable pointers with the
// CacheableRead bit. The cacheable read bit is set
// when the number of write images is not equal to the max
// or if the default RAW_UAV_ID is equal to 11. The first
// condition means that there is a raw uav between 0 and 7
// that is available for cacheable reads and the second
// condition means that UAV 11 is available for cacheable
// reads.
void AMDILEGPointerManagerImpl::annotateCacheablePtrs() {
  PtrSet::iterator siBegin, siEnd;
  std::vector<MachineInstr*>::iterator miBegin, miEnd;
  // First we can check the cacheable pointers
  for (siBegin = cacheablePtrs.begin(), siEnd = cacheablePtrs.end();
      siBegin != siEnd; ++siBegin) {
    assert(!bytePtrs.count(*siBegin) && "Found a cacheable pointer "
        "that also exists as a byte pointer!");
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
      // If UAV11 is enabled, then we can enable cached reads.
      if (STM->getResourceID(AMDIL::RAW_UAV_ID) == 11) {
        curRes.bits.CacheableRead = 1;
        curRes.bits.ResourceID = 11;
        setAsmPrinterFlags(*miBegin, curRes);
        mMFI->uav_insert(curRes.bits.ResourceID);
      }
    }
  }
}

static unsigned switchAtomicToArena(unsigned op) {
#define ATOM_CASE(OP) \
  case AMDIL::ATOM_G_##OP: return AMDIL::ATOM_A_##OP; \
  case AMDIL::ATOM_G_##OP##_NORET: return AMDIL::ATOM_A_##OP##_NORET;
  switch (op) {
    default: break;
    ATOM_CASE(ADD);
   ATOM_CASE(AND);
   ATOM_CASE(CMPXCHG);
   ATOM_CASE(DEC);
   ATOM_CASE(INC);
   ATOM_CASE(MAX);
   ATOM_CASE(MIN);
   ATOM_CASE(OR);
   ATOM_CASE(RSUB);
   ATOM_CASE(SUB);
   ATOM_CASE(UMAX);
   ATOM_CASE(UMIN);
   ATOM_CASE(XOR);
    case AMDIL::ATOM_G_XCHG: return AMDIL::ATOM_A_XCHG;
  }
  llvm_unreachable("Unknown atomic opcode found!");
  return 0;
}

// A byte pointer is a pointer that along the pointer path has a
// byte store assigned to it.
void AMDILEGPointerManagerImpl::annotateBytePtrs() {
  SortedPtrSet::iterator siBegin, siEnd;
  std::vector<MachineInstr*>::iterator miBegin, miEnd;
  uint32_t arenaID = STM->getResourceID(AMDIL::ARENA_UAV_ID);
  if (STM->isSupported(AMDIL::Caps::ArenaSegment)) {
    arenaID = ARENA_SEGMENT_RESERVED_UAVS + 1;
  }
  for (siBegin = bytePtrs.begin(), siEnd = bytePtrs.end();
      siBegin != siEnd; ++siBegin) {
    const Value* val = *siBegin;
    const PointerType *PT = dyn_cast<PointerType>(val->getType());
    if (!PT) {
        continue;
    }
    const Argument *CurArg = dyn_cast<Argument>(val);
    assert(!rawPtrs.count(val) && "Found a byte pointer "
        "that also exists as a raw pointer!");
    bool arenaInc = false;
    for (miBegin = PtrToInstMap[val].begin(),
        miEnd = PtrToInstMap[val].end();
        miBegin != miEnd; ++miBegin) {
      DEBUG(
        dbgs() << "Annotating pointer as arena. Inst: ";
        (*miBegin)->dump();
      );
      AMDILAS::InstrResEnc curRes;
      getAsmPrinterFlags(*miBegin, curRes);

      if (STM->usesHardware(AMDIL::Caps::ConstantMem)
          && PT->getAddressSpace() == AMDILAS::CONSTANT_ADDRESS) {
        // TODO: if constant buffer is found to conflict with other constant
        // buffers, need to coordicate with AMDILModuleInfo to adjust
        // constant buffer id assignments of the conflicting buffers

        // If hardware constant mem is enabled, then we need to
        // get the constant pointer CB number and use that to specify
        // the resource ID.
        const StringRef funcName = MF.getFunction()->getName();
        const AMDILKernel *krnl = mAMI->getKernel(funcName);
        curRes.bits.ResourceID = mAMI->getConstPtrCB(krnl, val->getName());
        curRes.bits.HardwareInst = 1;
        mMFI->setUsesConstant();
      } else if (STM->usesHardware(AMDIL::Caps::LocalMem)
          && PT->getAddressSpace() == AMDILAS::LOCAL_ADDRESS) {
        // If hardware local mem is enabled, get the local mem ID from
        // the device to use as the ResourceID
        curRes.bits.ResourceID = STM->getResourceID(AMDIL::LDS_ID);
        if (isAtomicInst(*miBegin)) {
          assert(curRes.bits.ResourceID && "Atomic resource ID "
              "cannot be non-zero!");
          (*miBegin)->getOperand((*miBegin)->getNumOperands()-1)
            .setImm(curRes.bits.ResourceID);
        }
        mMFI->setUsesLDS();
      } else if (STM->usesHardware(AMDIL::Caps::RegionMem)
          && PT->getAddressSpace() == AMDILAS::REGION_ADDRESS) {
        // If hardware region mem is enabled, get the gds mem ID from
        // the device to use as the ResourceID
        curRes.bits.ResourceID = STM->getResourceID(AMDIL::GDS_ID);
        if (isAtomicInst(*miBegin)) {
          assert(curRes.bits.ResourceID && "Atomic resource ID "
              "cannot be non-zero!");
          (*miBegin)->getOperand((*miBegin)->getNumOperands()-1)
            .setImm(curRes.bits.ResourceID);
        }
        mMFI->setUsesGDS();
      } else if (STM->usesHardware(AMDIL::Caps::PrivateMem)
          && PT->getAddressSpace() == AMDILAS::PRIVATE_ADDRESS) {
        curRes.bits.ResourceID = STM->getResourceID(AMDIL::SCRATCH_ID);
        mMFI->setUsesScratch();
      } else {
        DEBUG(
          dbgs() << __LINE__ << ": Setting byte store bit on instruction: ";
          (*miBegin)->print(dbgs());
        );
        curRes.bits.ByteStore = 1;
        curRes.bits.ResourceID = (CurArg
            && (STM->isSupported(AMDIL::Caps::NoAlias)
            || CurArg->hasNoAliasAttr())) ?
          arenaID : STM->getResourceID(AMDIL::ARENA_UAV_ID);
        if (STM->isSupported(AMDIL::Caps::ArenaSegment)) {
          arenaInc = true;
        }
        if (isAtomicInst(*miBegin) &&
            STM->isSupported(AMDIL::Caps::ArenaUAV)) {
          (*miBegin)->getOperand((*miBegin)->getNumOperands()-1)
            .setImm(curRes.bits.ResourceID);
          // If we are an arena instruction, we need to switch the atomic opcode
          // from the global version to the arena version.
          MachineInstr *MI = *miBegin;
          MI->setDesc(TM.getInstrInfo()->get(
                switchAtomicToArena(MI->getOpcode())));
        }
        DEBUG(
          dbgs() << "Annotating pointer as arena. Inst: ";
          (*miBegin)->dump();
        );
      }
      setAsmPrinterFlags(*miBegin, curRes);
      mMFI->setUAVID(val, curRes.bits.ResourceID);
      mMFI->uav_insert(curRes.bits.ResourceID);
    }
    if (arenaInc) {
      ++arenaID;
    }
  }
}
// A semaphore pointer is a opaque object that has semaphore instructions
// in its path.
void AMDILEGPointerManagerImpl::annotateSemaPtrs() {
  unsigned currentSemaphore = 1;
  for (SemaSet::iterator asBegin = semaphores.begin(),
      asEnd = semaphores.end(); asBegin != asEnd; ++asBegin) {
    const Value* curVal = *asBegin;
    DEBUG(
      dbgs() << "Semaphore: " << curVal->getName()
        << " assigned the counter " << currentSemaphore << "\n";
    );
    for (std::vector<MachineInstr*>::iterator
        miBegin = PtrToInstMap[curVal].begin(),
        miEnd = PtrToInstMap[curVal].end(); miBegin != miEnd; ++miBegin) {
      MachineInstr *MI = *miBegin;
      unsigned opcode = MI->getOpcode();
      switch (opcode) {
        default:
          DEBUG(
            dbgs() << "Skipping instruction: ";
            MI->dump();
          );
          break;
        case AMDIL::SEMAPHORE_WAIT:
        case AMDIL::SEMAPHORE_SIGNAL:
          MI->getOperand(0).ChangeToImmediate(currentSemaphore);
          mMFI->sema_insert(currentSemaphore);
          DEBUG(
            dbgs() << "Assigning semaphore " << currentSemaphore << " to Inst: ";
            MI->dump();
          );
          break;
      };
    }
    if (currentSemaphore >= OPENCL_MAX_NUM_SEMAPHORES) {
      mMFI->addErrorMsg(
          amd::CompilerErrorMessage[INSUFFICIENT_SEMAPHORE_RESOURCES]);
    }
    ++currentSemaphore;
  }
}
/// An append pointer is a opaque object that has append instructions
// in its path.
void AMDILEGPointerManagerImpl::annotateAppendPtrs() {
  unsigned currentCounter = 0;
  for (AppendSet::iterator asBegin = counters.begin(),
      asEnd = counters.end(); asBegin != asEnd; ++asBegin) {
    bool usesWrite = false;
    bool usesRead = false;
    const Value* curVal = *asBegin;
    DEBUG(
      dbgs() << "Counter: " << curVal->getName()
        << " assigned the counter " << currentCounter << "\n";
    );
    for (std::vector<MachineInstr*>::iterator
        miBegin = PtrToInstMap[curVal].begin(),
        miEnd = PtrToInstMap[curVal].end(); miBegin != miEnd; ++miBegin) {
      MachineInstr *MI = *miBegin;
      unsigned opcode = MI->getOpcode();
      switch (opcode) {
        default:
          DEBUG(
            dbgs() << "Skipping instruction: ";
            MI->dump();
          );
          break;
        case AMDIL::APPEND_ALLOC:
        case AMDIL::APPEND64_ALLOC:
          usesWrite = true;
          MI->getOperand(1).ChangeToImmediate(currentCounter);
          DEBUG(
            dbgs() << "Assigning counter " << currentCounter << " to Inst: ";
            MI->dump();
          );
          break;
        case AMDIL::APPEND_CONSUME:
        case AMDIL::APPEND64_CONSUME:
          usesRead = true;
          MI->getOperand(1).ChangeToImmediate(currentCounter);
          DEBUG(
            dbgs() << "Assigning counter " << currentCounter << " to Inst: ";
            MI->dump();
          );
          break;
      };
    }
    if (usesWrite && usesRead) {
      mMFI->addErrorMsg(amd::CompilerErrorMessage[INCORRECT_COUNTER_USAGE]);
    }
    ++currentCounter;
  }
}

// A raw pointer is any pointer that does not have byte store in its path.
void AMDILEGPointerManagerImpl::annotateRawPtrs() {
  SortedPtrSet::iterator siBegin, siEnd;
  std::vector<MachineInstr*>::iterator miBegin, miEnd;

  // Now all of the raw pointers will go to the raw uav.
  for (siBegin = rawPtrs.begin(), siEnd = rawPtrs.end();
      siBegin != siEnd; ++siBegin) {
    const Value *val = *siBegin;
    const PointerType *PT = dyn_cast<PointerType>(val->getType());
    if (!PT) {
      continue;
    }
    assert(!bytePtrs.count(val) && "Found a raw pointer "
        " that also exists as a byte pointers!");
    for (miBegin = PtrToInstMap[val].begin(),
        miEnd = PtrToInstMap[val].end();
        miBegin != miEnd; ++miBegin) {
      DEBUG(
        dbgs() << "Annotating pointer as raw. Inst: ";
        (*miBegin)->dump();
      );
      AMDILAS::InstrResEnc curRes;
      getAsmPrinterFlags(*miBegin, curRes);
      if (!curRes.bits.ConflictPtr) {
        assert(!curRes.bits.ByteStore
            && "Found a instruction that is marked as "
            "raw but has a byte store bit set!");
      } else if (curRes.bits.ConflictPtr) {
        if (curRes.bits.ByteStore) {
          curRes.bits.ByteStore = 0;
        }
      }
      if (STM->usesHardware(AMDIL::Caps::ConstantMem)
          && PT->getAddressSpace() == AMDILAS::CONSTANT_ADDRESS) {
        // TODO: if constant buffer is found to conflict with other constant
        // buffers, need to coordicate with AMDILModuleInfo to adjust
        // constant buffer id assignments of the conflicting buffers

        // If hardware constant mem is enabled, then we need to
        // get the constant pointer CB number and use that to specify
        // the resource ID.
        const StringRef funcName = MF.getFunction()->getName();
        const AMDILKernel *krnl = mAMI->getKernel(funcName);
        curRes.bits.ResourceID = mAMI->getConstPtrCB(krnl, val->getName());
        curRes.bits.HardwareInst = 1;
        mMFI->setUsesConstant();
      } else if (STM->usesHardware(AMDIL::Caps::LocalMem)
          && PT->getAddressSpace() == AMDILAS::LOCAL_ADDRESS) {
        // If hardware local mem is enabled, get the local mem ID from
        // the device to use as the ResourceID
        curRes.bits.ResourceID = STM->getResourceID(AMDIL::LDS_ID);
        if (isAtomicInst(*miBegin)) {
          assert(curRes.bits.ResourceID && "Atomic resource ID "
              "cannot be non-zero!");
          (*miBegin)->getOperand((*miBegin)->getNumOperands()-1)
            .setImm(curRes.bits.ResourceID);
        }
        mMFI->setUsesLDS();
      } else if (STM->usesHardware(AMDIL::Caps::RegionMem)
          && PT->getAddressSpace() == AMDILAS::REGION_ADDRESS) {
        // If hardware region mem is enabled, get the gds mem ID from
        // the device to use as the ResourceID
        curRes.bits.ResourceID = STM->getResourceID(AMDIL::GDS_ID);
        if (isAtomicInst(*miBegin)) {
          assert(curRes.bits.ResourceID && "Atomic resource ID "
              "cannot be non-zero!");
          (*miBegin)->getOperand((*miBegin)->getNumOperands()-1)
            .setImm(curRes.bits.ResourceID);
        }
        mMFI->setUsesGDS();
      } else if (STM->usesHardware(AMDIL::Caps::PrivateMem)
          && PT->getAddressSpace() == AMDILAS::PRIVATE_ADDRESS) {
        curRes.bits.ResourceID = STM->getResourceID(AMDIL::SCRATCH_ID);
        mMFI->setUsesScratch();
      } else if (!STM->isSupported(AMDIL::Caps::MultiUAV)) {
        // If multi uav is enabled, then the resource ID is either the
        // number of write images that are available or the device
        // raw uav id if it is 11.
        if (STM->getResourceID(AMDIL::RAW_UAV_ID) >
            STM->getResourceID(AMDIL::ARENA_UAV_ID)) {
          curRes.bits.ResourceID = STM->getResourceID(AMDIL::RAW_UAV_ID);
        } else if (numWriteImages != STM->getMaxNumWriteImages()) {
          if (STM->getResourceID(AMDIL::RAW_UAV_ID)
              < numWriteImages) {
            curRes.bits.ResourceID = numWriteImages;
          } else {
            curRes.bits.ResourceID = STM->getResourceID(AMDIL::RAW_UAV_ID);
          }
        } else {
          DEBUG(
            dbgs() << __LINE__ << ": Setting byte store bit on instruction: ";
            (*miBegin)->print(dbgs());
          );
          curRes.bits.ByteStore = 1;
          curRes.bits.ResourceID = STM->getResourceID(AMDIL::ARENA_UAV_ID);
        }
        if (isAtomicInst(*miBegin)) {
          (*miBegin)->getOperand((*miBegin)->getNumOperands()-1)
            .setImm(curRes.bits.ResourceID);
          if (curRes.bits.ResourceID
              == STM->getResourceID(AMDIL::ARENA_UAV_ID)) {
            llvm_unreachable("Found an atomic instruction that has "
                             "an arena uav id!");
          }
        }
        mMFI->setUAVID(val, curRes.bits.ResourceID);
        mMFI->uav_insert(curRes.bits.ResourceID);
      }
      DEBUG(
        dbgs() << "Setting pointer to resource ID "
          << curRes.bits.ResourceID << ": ";
        val->dump();
      );
      setAsmPrinterFlags(*miBegin, curRes);
    }
  }

}

void AMDILEGPointerManagerImpl::annotatePrintfInsts() {
  if (printfInsts.empty()) return;

  unsigned id = STM->getResourceID(AMDIL::PRINTF_ID);
  for (std::set<MachineInstr*>::iterator I = printfInsts.begin(),
    E = printfInsts.end(); I != E; ++I) {
    MachineInstr* MI = *I;
    assert(isGlobalInst(MI) && "Printf buffer not in global address space");
    AMDILAS::InstrResEnc curRes;
    getAsmPrinterFlags(MI, curRes);
    curRes.bits.ResourceID = id;
    setAsmPrinterFlags(MI, curRes);
    DEBUG(dbgs() << "Setting printf inst to resource ID " << id << ": ";
      MI->dump());
  }
  mMFI->uav_insert(id);
}

void AMDILEGPointerManagerImpl::annotateCacheableInstrs() {
  CacheableInstrSet::iterator miBegin, miEnd;

  for (miBegin = cacheableSet.begin(), miEnd = cacheableSet.end();
       miBegin != miEnd; ++miBegin) {
    DEBUG(
      dbgs() << "Annotating instr as cacheable. Inst: ";
      (*miBegin)->dump();
    );
    AMDILAS::InstrResEnc curRes;
    getAsmPrinterFlags(*miBegin, curRes);
    // If UAV11 is enabled, then we can enable cached reads.
    if (STM->getResourceID(AMDIL::RAW_UAV_ID) == 11) {
      curRes.bits.CacheableRead = 1;
      curRes.bits.ResourceID = 11;
      setAsmPrinterFlags(*miBegin, curRes);
      mMFI->uav_insert(curRes.bits.ResourceID);
    }
  }
}

// A local pointer is a pointer that point to the local address space
// For local pointers that don't conflict with other local pointers,
// allocate a new local buffer for each such pointer. For all local
// pointers that conflict with another local pointer, allocate all local
// pointers that conflict with each other into their own local buffer.
void AMDILEGPointerManagerImpl::annotateLocalPtrs() {
  DEBUG_WITH_TYPE("lds", dbgs() << "[annotateLocalPtrs] " <<
      MF.getFunction()->getName() << '\n');
  assert(STM->usesHardware(AMDIL::Caps::LocalMem)
         && "not checked before calling this");
  std::vector<MachineInstr*>::iterator miBegin, miEnd;
  unsigned setId = 0;
  for (SmallValSets::iterator siBegin = localPtrSets.begin(),
       siEnd = localPtrSets.end();
       siBegin != siEnd; ++siBegin, ++setId) {
    const SmallValSet& set = *siBegin;
    if (set.empty()) continue;

    // populate the next local buffer with the current set of local pointers
    bool isDefaultBuf = setId == 0;
    uint32_t resourceID = mAMI->populateNextLocalBuffer(set, isDefaultBuf);

    // mark resource id of all instructions that accesses local pointers
    // in the set to the local buffer id
    for (miBegin = localSetId2InstMap[setId].begin(),
         miEnd = localSetId2InstMap[setId].end();
         miBegin != miEnd; ++miBegin) {
      DEBUG_WITH_TYPE("lds",
        dbgs() << "Annotating local pointer as " << resourceID << ". Inst: ";
        (*miBegin)->dump();
      );
      AMDILAS::InstrResEnc curRes;
      getAsmPrinterFlags(*miBegin, curRes);

      if (curRes.bits.ResourceID != resourceID) {
        curRes.bits.ResourceID = resourceID;
        setAsmPrinterFlags((*miBegin), curRes);
        if (isAtomicInst(*miBegin)) {
          (*miBegin)->getOperand((*miBegin)->getNumOperands()-1)
            .setImm(resourceID);
        }
      }
    }
  }
}

// Now replace the Addri instruction that corresponds to each local or const
// array with a loadconst instruction
// Addri instruction was: Addr[ip] <Local/Const Array>*, <Offset>
// Now we know the offset as constant value for local arrays, replace Addr[ip].
// If <Offset> is an immediate value:
//   replace Addri with LoadConst <Array Offset>+<Offset>
// Else:
//   If <Array Offset> is 0:
//     replace Addrp with <Offset>
//   Else:
//     replace Addrp with Addri <Offset>, <Array Offset>
void AMDILEGPointerManagerImpl::replaceAddri() {
  MachineRegisterInfo &regInfo = MF.getRegInfo();
  for (GVInstPairVec::iterator it = AddriVec.begin(), end = AddriVec.end();
    it != end; ++it) {
    const GlobalValue* GV = (*it).first;
    MachineInstr* addri = (*it).second;
    unsigned dstReg = addri->getOperand(0).getReg();
    if (regInfo.use_empty(dstReg)) {
      addri->eraseFromParent();
      continue;
    }
    MachineBasicBlock* mb = addri->getParent();
    short ptrRegClassID = addri->getDesc().OpInfo[0].RegClass;
    assert((ptrRegClassID == AMDIL::GPR_32RegClassID
            || ptrRegClassID == AMDIL::GPR_64RegClassID)
           && "unexpected reg class for pointer type");
    int32_t arrayOffset = mAMI->getArrayOffset(GV->getName().str());
    if (arrayOffset != -1) {
      if (!STM->usesHardware(AMDIL::Caps::LocalMem)) {
        continue;
      }
    } else {
      arrayOffset = mAMI->getConstOffset(GV->getName().str());
      if (arrayOffset != -1) {
        if (!STM->usesHardware(AMDIL::Caps::ConstantMem)) {
          continue;
        }
      }
    }
    int64_t baseOffset;
    unsigned baseOffsetReg = 0;
    MachineInstr *baseOffsetInst = NULL;
    if (addri->getOperand(2).isImm()) {
      baseOffset = addri->getOperand(2).getImm();
    } else {
      baseOffsetReg = addri->getOperand(2).getReg();
      assert(regInfo.isSSA() && "not SSA");
      baseOffsetInst = regInfo.getVRegDef(baseOffsetReg);
      short offsetRegClassID = baseOffsetInst->getDesc().OpInfo[0].RegClass;
      assert(offsetRegClassID == ptrRegClassID
             && "src & dst reg not same class");
      // if arrayOffset == 0, replace Addrp with baseOffset
      if (arrayOffset == 0) {
        regInfo.replaceRegWith(dstReg, baseOffsetReg);
        addri->eraseFromParent();
        continue;
      }
      if (isLoadConstInst(baseOffsetInst)) {
        baseOffset = baseOffsetInst->getOperand(1).getImm();
      } else {
        regInfo.replaceRegWith(dstReg, baseOffsetReg);
        // replace Addrp with Addri baseOffset, arrayOffset
        unsigned Op = (ptrRegClassID == AMDIL::GPR_32RegClassID)
                      ? AMDIL::ADDi32ri : AMDIL::ADDi64ri;
        BuildMI(*mb, addri, addri->getDebugLoc(),
                TM.getInstrInfo()->get(Op), dstReg)
          .addReg(baseOffsetReg)
          .addImm(arrayOffset);
        addri->eraseFromParent();
        continue;
      }
    }
    // baseOffset is an immediate value, replace Addri with
    // LoadConst arrayOffset+baseOffset
    unsigned loadconstOp = (ptrRegClassID == AMDIL::GPR_32RegClassID)
                           ? AMDIL::LOADCONSTi32 : AMDIL::LOADCONSTi64;
    BuildMI(*mb, addri, addri->getDebugLoc(),
          TM.getInstrInfo()->get(loadconstOp), dstReg)
      .addImm(arrayOffset + baseOffset);
    addri->eraseFromParent();
    if (baseOffsetInst && regInfo.use_empty(baseOffsetReg)) {
      baseOffsetInst->eraseFromParent();
    }
  }
}

// Annotate the instructions along various pointer paths. The paths that
// are handled are the raw, byte, cacheable and local pointer paths.
void AMDILEGPointerManagerImpl::annotatePtrPath() {
  DEBUG_WITH_TYPE("ptripa", dbgs() << "[annotatePtrPath] PtrToInstMap:" <<
      PtrToInstMap.size() << " rawPtrs:" << rawPtrs.size() << '\n');
  if (!PtrToInstMap.empty()) {
    // First we can check the cacheable pointers
    if (!cacheablePtrs.empty()) {
      annotateCacheablePtrs();
    }

    // Next we annotate the byte pointers
    if (!bytePtrs.empty()) {
      annotateBytePtrs();
    }

    // Next we annotate the raw pointers
    if (!rawPtrs.empty()) {
      annotateRawPtrs();
    }
  }

  if (trackPrintfs && !printfInsts.empty()) {
    annotatePrintfInsts();
  }

  // Next we annotate the local pointers
  if(STM->usesHardware(AMDIL::Caps::LocalMem)) {
    if (doPerPointerLDS && !localPtrSets.empty()) {
      annotateLocalPtrs();
    }
  }
}

// Allocate MultiUAV pointer ID's for the raw/conflict pointers.
void AMDILEGPointerManagerImpl::allocateMultiUAVPointers() {
  if (PtrToInstMap.empty()) {
    return;
  }
  uint32_t curUAV = numWriteImages;
  bool increment = true;
  // If the RAW_UAV_ID is a value that is larger than the max number of write
  // images, then we use that UAV ID.
  if (numWriteImages >= STM->getMaxNumWriteImages()) {
    curUAV = STM->getResourceID(AMDIL::RAW_UAV_ID);
    increment = false;
  }
  SortedPtrSet::iterator siBegin, siEnd;
  std::vector<MachineInstr*>::iterator miBegin, miEnd;
  // First lets handle the raw pointers.
  for (siBegin = rawPtrs.begin(), siEnd = rawPtrs.end();
      siBegin != siEnd; ++siBegin) {
    assert((*siBegin)->getType()->isPointerTy() && "We must be a pointer type "
        "to be processed at this point!");
    const Value* val = *siBegin;
    const PointerType *PT = dyn_cast<PointerType>(val->getType());
    if (conflictPtrs.count(val) || !PT) {
      continue;
    }
    // We only want to process global address space pointers
    if (PT->getAddressSpace() != AMDILAS::GLOBAL_ADDRESS) {
      if ((PT->getAddressSpace() == AMDILAS::LOCAL_ADDRESS
            && STM->usesSoftware(AMDIL::Caps::LocalMem))
          || (PT->getAddressSpace() == AMDILAS::CONSTANT_ADDRESS
            && STM->usesSoftware(AMDIL::Caps::ConstantMem))
          || (PT->getAddressSpace() == AMDILAS::REGION_ADDRESS
            && STM->usesSoftware(AMDIL::Caps::RegionMem))) {
        // If we are using software emulated hardware features, then
        // we need to specify that they use the raw uav and not
        // zero-copy uav. The easiest way to do this is to assume they
        // conflict with another pointer. Any pointer that conflicts
        // with another pointer is assigned to the raw uav or the
        // arena uav if no raw uav exists.
        const PointerType *PT = dyn_cast<PointerType>(val->getType());
        if (PT) {
          conflictPtrs.insert(val);
        }
      }
      if (PT->getAddressSpace() == AMDILAS::PRIVATE_ADDRESS) {
        if (STM->usesSoftware(AMDIL::Caps::PrivateMem)) {
          const PointerType *PT = dyn_cast<PointerType>(val->getType());
          if (PT) {
            conflictPtrs.insert(val);
          }
        } else {
          DEBUG(
            dbgs() << "Scratch Pointer '" << val->getName()
              << "' being assigned uav "<<
              STM->getResourceID(AMDIL::SCRATCH_ID) << "\n";
          );
          for (miBegin = PtrToInstMap[val].begin(),
              miEnd = PtrToInstMap[val].end();
              miBegin != miEnd; ++miBegin) {
            AMDILAS::InstrResEnc curRes;
            getAsmPrinterFlags(*miBegin, curRes);
            curRes.bits.ResourceID = STM->getResourceID(AMDIL::SCRATCH_ID);
            DEBUG(
              dbgs() << "Updated instruction to bitmask ";
              dbgs().write_hex(curRes.u16all);
              dbgs() << " with ResID " << curRes.bits.ResourceID;
              dbgs() << ". Inst: ";
              (*miBegin)->dump();
            );
            setAsmPrinterFlags((*miBegin), curRes);
            mMFI->setUAVID(val, curRes.bits.ResourceID);
            mMFI->uav_insert(curRes.bits.ResourceID);
          }
          mMFI->setUsesScratch();
        }
      }
      continue;
    }
    // If more than just UAV 11 is cacheable, then we can remove
    // this check.
    if (cacheablePtrs.count(val)) {
      DEBUG(
        dbgs() << "Raw Pointer '" << val->getName()
          << "' is cacheable, not allocating a multi-uav for it!\n";
      );
      continue;
    }

    DEBUG(
      dbgs() << "Raw Pointer '" << val->getName()
        << "' being assigned uav " << curUAV << "\n";
    );
    if (PtrToInstMap[val].empty()) {
      mMFI->setUAVID(val, curUAV);
      mMFI->uav_insert(curUAV);
    }
    // For all instructions here, we are going to set the new UAV to the curUAV
    // number and not the value that it currently is set to.
    for (miBegin = PtrToInstMap[val].begin(),
        miEnd = PtrToInstMap[val].end();
        miBegin != miEnd; ++miBegin) {
      AMDILAS::InstrResEnc curRes;
      getAsmPrinterFlags(*miBegin, curRes);
      curRes.bits.ResourceID = curUAV;
      if (isAtomicInst(*miBegin)) {
        (*miBegin)->getOperand((*miBegin)->getNumOperands()-1)
          .setImm(curRes.bits.ResourceID);
        if (curRes.bits.ResourceID == STM->getResourceID(AMDIL::ARENA_UAV_ID)) {
          llvm_unreachable("Found an atomic instruction that has "
                           "an arena uav id!");
        }
      }
      if (curUAV == STM->getResourceID(AMDIL::ARENA_UAV_ID)) {
        DEBUG(
          dbgs() << __LINE__ << ": Setting byte store bit on instruction: ";
          (*miBegin)->print(dbgs());
        );
        curRes.bits.ByteStore = 1;
        curRes.bits.CacheableRead = 0;
      }

      DEBUG(
        dbgs() << "Updated instruction to bitmask ";
        dbgs().write_hex(curRes.u16all);
        dbgs() << " with ResID " << curRes.bits.ResourceID;
        dbgs() << ". Inst: ";
        (*miBegin)->dump();
      );

      setAsmPrinterFlags(*miBegin, curRes);
      mMFI->setUAVID(val, curRes.bits.ResourceID);
      mMFI->uav_insert(curRes.bits.ResourceID);
    }
    // If we make it here, we can increment the uav counter if we are less
    // than the max write image count. Otherwise we set it to the default
    // UAV and leave it.
    if (increment && curUAV < (STM->getMaxNumWriteImages() - 1)) {
      ++curUAV;
    } else {
      curUAV = STM->getResourceID(AMDIL::RAW_UAV_ID);
      increment = false;
    }
  }
  if (numWriteImages == 8) {
    curUAV = STM->getResourceID(AMDIL::RAW_UAV_ID);
  }
  // Now lets handle the conflict pointers
  PtrSet::iterator siBegin2, siEnd2;
  for (siBegin2 = conflictPtrs.begin(), siEnd2 = conflictPtrs.end();
      siBegin2 != siEnd2; ++siBegin2) {
    assert((*siBegin2)->getType()->isPointerTy() && "We must be a pointer type "
        "to be processed at this point!");
    const Value *val = *siBegin2;
    const PointerType *PT = dyn_cast<PointerType>(val->getType());
    // We only want to process global address space pointers
    if (!PT || PT->getAddressSpace() != AMDILAS::GLOBAL_ADDRESS) {
      continue;
    }

    DEBUG(
      dbgs() << "Conflict Pointer '" << val->getName()
        << "' being assigned uav " << curUAV << "\n";
    );

    if (PtrToInstMap[val].empty()) {
      mMFI->setUAVID(val, curUAV);
      mMFI->uav_insert(curUAV);
    }
    for (miBegin = PtrToInstMap[val].begin(),
        miEnd = PtrToInstMap[val].end();
        miBegin != miEnd; ++miBegin) {
      AMDILAS::InstrResEnc curRes;
      getAsmPrinterFlags(*miBegin, curRes);
      curRes.bits.ResourceID = curUAV;
      if (isAtomicInst(*miBegin)) {
        (*miBegin)->getOperand((*miBegin)->getNumOperands()-1)
          .setImm(curRes.bits.ResourceID);
        if (curRes.bits.ResourceID == STM->getResourceID(AMDIL::ARENA_UAV_ID)) {
          llvm_unreachable("Found an atomic instruction that has "
                           "an arena uav id!");
        }
      }
      if (curUAV == STM->getResourceID(AMDIL::ARENA_UAV_ID)) {
        DEBUG(
          dbgs() << __LINE__ << ": Setting byte store bit on instruction: ";
          (*miBegin)->print(dbgs());
        );
        curRes.bits.ByteStore = 1;
      }

      DEBUG(
        dbgs() << "Updated instruction to bitmask ";
        dbgs().write_hex(curRes.u16all);
        dbgs() << " with ResID " << curRes.bits.ResourceID;
        dbgs() << ". Inst: ";
        (*miBegin)->dump();
      );
      setAsmPrinterFlags(*miBegin, curRes);
      mMFI->setUAVID(val, curRes.bits.ResourceID);
      mMFI->uav_insert(curRes.bits.ResourceID);
    }
  }
}
// The first thing we should do is to allocate the default
// ID for each load/store/atomic instruction so that
// it is correctly allocated. Everything else after this
// is just an optimization to more efficiently allocate
// resource ID's.
void AMDILPointerManagerImpl::allocateDefaultIDs() {
  std::string longName = std::string("llvm.sampler.annotations.") +
    std::string(MF.getFunction()->getName());
  llvm::StringRef funcName = longName;
  std::set<std::string> *samplerNames = mAMI->getSamplerForKernel(funcName);
  if (samplerNames) {
    for (std::set<std::string>::iterator b = samplerNames->begin(),
        e = samplerNames->end(); b != e; ++b) {
      mMFI->addSampler((*b), ~0U);
    }
  }

  for (MachineFunction::iterator mfBegin = MF.begin(),
      mfEnd = MF.end(); mfBegin != mfEnd; ++mfBegin) {
    MachineBasicBlock *MB = mfBegin;
    for (MachineBasicBlock::iterator mbb = MB->begin(), mbe = MB->end();
        mbb != mbe; ++mbb) {
      MachineInstr *MI = mbb;
      if (isPtrLoadInst(MI)
          || isPtrStoreInst(MI)
          || isAtomicInst(MI)) {
        AMDILAS::InstrResEnc curRes;
        getAsmPrinterFlags(MI, curRes);
        allocateDefaultID(curRes, MI);
      }
    }
  }
}

bool AMDILEGPointerManagerImpl::perform() {
  const Module* module = MF.getFunction()->getParent();
  hasFunctionCalls = moduleHasFunctionCalls(module);
  AMDLLVMContextHook* hook = static_cast<AMDLLVMContextHook*>
    (module->getContext().getAMDLLVMContextHook());
  if (hook) {
    useDefaultResId = hasFunctionCalls && hook->amdoptions.
    AmdilUseDefaultResId;
  } else {
    useDefaultResId = false;
  }
  DEBUG(dbgs() << "hasFunctionCalls=" << hasFunctionCalls <<
      " useDefaultResId=" << useDefaultResId << '\n');

  if (hasFunctionCalls && !useDefaultResId) {
    ptrEqSet = static_cast<PtrEqSet*>(mAMI->getPtrEqSet());
    if (ptrEqSet == NULL) {
      ptrEqSet = new PtrEqSet();
      ptrEqSet->build(module, TM);
      mAMI->setPtrEqSet(ptrEqSet);
    }
  }

  // Start out by allocating the default ID's to all instructions in the
  // function.
  allocateDefaultIDs();

  // First we need to go through all of the arguments and assign the
  // live in registers to the lookup table and the pointer mapping.
  if (mMFI->isKernel())
    numWriteImages = parseArguments();

  parseConstantPtrs();

  doPerPointerLDS = !mAMI->moduleHasCalls();

  // If hardware supports local memory, remember local arrays that belongs
  // to this function into "localPtrs"
  if (STM->usesHardware(AMDIL::Caps::LocalMem)) {
#ifndef PER_POINTER_LDS_WITH_KERNEL_ARG
    // As of now, the meta data we pass to the runtime only has the total
    // amount of lds buffer allocated by the kernel. If both lds pointer type
    // kernel arguments and lds arrays exist, the runtime needs to also know
    // the amount allocated by the kernel in the lds buffer where kernel
    // arguments will be allocated. But we don't want to change the ABI to
    // break ABI compatibility.
    // So for now, disable per-pointer lds buffer allocation if lds pointer
    // kernel arguments exist, until we move away from meta data.
    if (!localPtrs.empty()) {
      doPerPointerLDS = false;
      localPtrs.clear();
    }
#endif
    if (doPerPointerLDS) {
      parseLocalArrays();
      // initialize localPtrSets
      initializeLocalPtrSets();
    }
  }

  // Lets do some error checking on the results of the parsing.
  if (counters.size() > OPENCL_MAX_NUM_ATOMIC_COUNTERS) {
    mMFI->addErrorMsg(
        amd::CompilerErrorMessage[INSUFFICIENT_COUNTER_RESOURCES]);
  }
  if (semaphores.size() > OPENCL_MAX_NUM_SEMAPHORES) {
    mMFI->addErrorMsg(
        amd::CompilerErrorMessage[INSUFFICIENT_SEMAPHORE_RESOURCES]);
  }
  if (numWriteImages > STM->getMaxNumWriteImages()
      || (images.size() - numWriteImages > STM->getMaxNumReadImages())) {
    mMFI->addErrorMsg(
        amd::CompilerErrorMessage[INSUFFICIENT_IMAGE_RESOURCES]);
  }

  // Now lets parse all of the instructions and update our
  // lookup tables.
  parseFunction();

  // We need to go over our pointer map and find all the conflicting
  // pointers that have byte stores and put them in the bytePtr map.
  // All conflicting pointers that don't have byte stores go into
  // the rawPtr map.
  detectConflictingPointers();

  // The next step is to detect whether the pointer should be added to
  // the fully cacheable set or not. A pointer is marked as cacheable if
  // no store instruction exists.
  detectFullyCacheablePointers();

  // Disable partially cacheable for now when multiUAV is on.
  // SC versions before SC139 have a bug that generates incorrect
  // addressing for some cached accesses.
  if (!STM->isSupported(AMDIL::Caps::MultiUAV)) {
    // Now we take the set of loads that have no reachable stores and
    // create a list of additional instructions (those that aren't already
    // in a cacheablePtr set) that are safe to mark as cacheable.
    detectCacheableInstrs();

    // Annotate the additional instructions computed above as cacheable.
    // Note that this should not touch any instructions annotated in
    // annotatePtrPath.
    annotateCacheableInstrs();
  }

  // Now that we have detected everything we need to detect, lets go through an
  // annotate the instructions along the pointer path for each of the
  // various pointer types.
  annotatePtrPath();

  // Annotate the atomic counter path if any exists.
  annotateAppendPtrs();

  // Annotate the semaphore path if any exists.
  annotateSemaPtrs();

  // If we support MultiUAV, then we need to determine how
  // many write images exist so that way we know how many UAV are
  // left to allocate to buffers.
  if (STM->isSupported(AMDIL::Caps::MultiUAV)) {
    // We now have (OPENCL_MAX_WRITE_IMAGES - numPtrs) buffers open for
    // multi-uav allocation.
    allocateMultiUAVPointers();
  }

  // The last step is to detect if we have any alias constant pool operations.
  // This is not likely, but does happen on occasion with double precision
  // operations.
  detectAliasedCPoolOps();

  // Add all of the fully read-only pointers to the machine function information
  // structure so that we can emit it in the metadata.
  // FIXME: this assumes NoAlias, need to also detect cases where NoAlias
  // is not set, but there are exclusively only reads or writes to the pointer.
  for (CacheableSet::iterator csBegin = cacheablePtrs.begin(),
      csEnd = cacheablePtrs.end(); csBegin != csEnd; ++csBegin) {
    mMFI->add_read_ptr(*csBegin);
  }

  // clear temporary machine instruction flags
  // Because some of the temp flag bits overlap with llvm flags, we need to
  // clear the temp flag bits before replaceAddri() below, where
  // MachineInstr::eraseFromParent() is called, which passes control into llvm,
  // where the Flags field of MachineInstr is checked.
  clearTempMIFlags(MF);

  // Now replace the Addri instruction that corresponds to each local array
  // with a loadconst instruction.
  // Note that this should be called after annotateLocalPtrs()
  if (STM->usesHardware(AMDIL::Caps::LocalMem) ||
      STM->usesHardware(AMDIL::Caps::ConstantMem)) {
    replaceAddri();
  }

  DEBUG(
    dumpPointers(bytePtrs, "Byte Store Ptrs");
    dumpPointers(rawPtrs, "Raw Ptrs");
    dumpPointers(cacheablePtrs, "Cache Load Ptrs");
    dumpPointers(counters, "Atomic Counters");
    dumpPointers(semaphores, "Semaphores");
    dumpPointers(images, "Images");
  );

  return true;
}

// Check if a function is kernel
static bool isKernel(const Function* F) {
  return F->getName().startswith("__OpenCL_") &&
         F->getName().endswith("_kernel");
}

// Check if a function is kernel stub
static bool isKernelStub(const Function* F) {
  return F->getName().startswith("__OpenCL_") &&
         F->getName().endswith("_stub");
}

// Check if a function is called directly or indirectly by a kernel
static bool isCalledByKernel(const Function* F,
    std::map<const Function*, bool>& work) {
  std::map<const Function*, bool>::iterator loc = work.find(F);
  if (loc != work.end())
    return loc->second;
  for (Function::const_use_iterator I = F->use_begin(), E = F->use_end(); I != E;
      ++I) {
    const User *UI = *I;
    if (isa<CallInst>(UI) || isa<InvokeInst>(UI)) {
      ImmutableCallSite CS(cast<Instruction>(UI));
      const Function* caller = CS.getCaller();
      if (isKernel(caller) || isCalledByKernel(caller, work)) {
        work[F] = true;
        return true;
      }
    }
  }
  work[F] = false;
  return false;
}

bool AMDILEGPointerManagerImpl::moduleHasFunctionCalls(const Module* M) {
  const TargetIntrinsicInfo *TII = TM.getIntrinsicInfo();
  std::map<const Function*, bool> work;
  for (Module::const_iterator I = M->begin(), E = M->end(); I != E; ++I) {
    // Intrinsics have already been lowered, so ignore them
    if (I->isIntrinsic()) {
      continue;
    }
    // look for target specific intrinsics
    unsigned IntrinsicID = TII->getIntrinsicID(const_cast<Function*>(&(*I)));
    if (IntrinsicID != Intrinsic::not_intrinsic) {
      continue;
    }
    if (isCalledByKernel(I, work)) {
      return true;
    }
  }
  return false;
}

static bool hasGlobalPtr(llvm::Type* type) {
  if (PointerType* ptrTy = dyn_cast<PointerType>(type)) {
    if (ptrTy->getAddressSpace() == AMDILAS::GLOBAL_ADDRESS) {
      return true;
    }
    return hasGlobalPtr(ptrTy->getElementType());
  } else if (StructType* struTy = dyn_cast<StructType>(type)) {
    for (unsigned I = 0, E = struTy->getStructNumElements(); I != E; ++I) {
      if (hasGlobalPtr(struTy->getElementType(I)))
          return true;
    }
    return false;
  } else if (ArrayType* arrTy = dyn_cast<ArrayType>(type)) {
    return hasGlobalPtr(arrTy->getElementType());
  }
  return false;
}

static std::string toStr(const llvm::Value* value) {
  if (const Argument* arg = dyn_cast<Argument>(value)) {
    std::string str;
    raw_string_ostream oss(str);
    oss << arg->getParent()->getName().str() << ":arg" << arg->getArgNo()
        << ":" << arg->getName().str();
    return oss.str();
  }
  return value->getName();
}

static std::string toStr(std::map<const Value*, unsigned>& valueMap) {
  std::string str;
  raw_string_ostream oss(str);
  for (std::map<const Value*, unsigned>::iterator I = valueMap.begin(),
      E = valueMap.end(); I != E; ++I) {
    oss << toStr(I->first) << " => " << I->second << '\n';
  }
  return oss.str();
}

static std::string toStr(std::map<unsigned, std::set<const Value*> >& valueSet) {
  std::string str;
  raw_string_ostream oss(str);
  for (std::map<unsigned, std::set<const Value*> >::iterator
      I = valueSet.begin(), E = valueSet.end(); I != E; ++I) {
    oss << I->first << '\n';
    for (std::set<const Value*>::iterator II = (I->second).begin(),
        EE = (I->second).end(); II != EE; ++II) {
      oss << "       => " << toStr(*II) << '\n';
    }
  }
  return oss.str();
}

// Merge value sets id1 and id2. The smaller id will be kept
unsigned AMDILEGPointerManagerImpl::PtrEqSet::merge (unsigned id1, unsigned id2) {
  DEBUG_WITH_TYPE("ptripa", dbgs() << "[merge] " << id1 << " <= " << id2 <<
      '\n');
  if (id2 == id1)
    return id1;

  if (id2 < id1) {
    unsigned tmp = id2;
    id2 = id1;
    id1 = tmp;
  }

  assert(id1 != ~0U && id2 != ~0U);
  DEBUG_WITH_TYPE("ptripax",
        dbgs() << "[merge " << id1 << " <= " << id2 << ": before\n" <<
        toStr() << '\n');
  ValueSet& set1 = idValueMap[id1];
  IdValueMap::iterator loc2 = idValueMap.find(id2);
  ValueSet& set2 = loc2->second;
  for (ValueSet::iterator I = set2.begin(), E = set2.end(); I != E; ++I) {
    DEBUG_WITH_TYPE("ptripa",
        if (valueIdMap[*I] != id2)
          dbgs() << toStr() << '\n');
    assert(valueIdMap[*I] == id2 && "Inconsistent PtrSet");
    valueIdMap[*I] = id1;
  }
  set1.insert(set2.begin(), set2.end());
  idValueMap.erase(loc2);
  DEBUG_WITH_TYPE("ptripax",
        dbgs() << "[merge " << id1 << " <= " << id2 << ": after\n" <<
        toStr() << '\n');
  return id1;
}

void AMDILEGPointerManagerImpl::PtrEqSet::clear() {
  valueIdMap.clear();
  idValueMap.clear();
  idIdMap.clear();
  idAssigned.clear();
}

bool AMDILEGPointerManagerImpl::PtrEqSet::contains(const Value* value,
    unsigned* id) {
  ValueIdMap::iterator loc = valueIdMap.find(value);
  if (loc == valueIdMap.end())
    return false;
  if (id != NULL) {
    *id = loc->second;
  }
  return true;
}

unsigned AMDILEGPointerManagerImpl::PtrEqSet::update(const Value* value,
    unsigned id) {
  ValueIdMap::iterator loc = valueIdMap.find(value);
  if (loc == valueIdMap.end()) {
    valueIdMap[value] = id;
    idValueMap[id].insert(value);
  } else {
    unsigned oldId = loc->second;
    if (oldId == id)
      return id;
    id = merge (id, oldId);
  }
  DEBUG_WITH_TYPE("pointermanager", dbgs() << "[PtrEqSet::update] " << ::toStr(value) <<
      " => " << id << '\n');

  for (Value::const_use_iterator UI = value->use_begin(), UE = value->use_end();
      UI != UE; ++UI) {
    const User* user = *UI;
    if (const GetElementPtrInst* GEP = dyn_cast<GetElementPtrInst>(user)) {
      if (GEP->getPointerOperand() == value) {
        update(user, id);
      }
    }
  }

  DEBUG_WITH_TYPE("ptripax", dbgs() << toStr() << '\n');
  return id;
}

// Check if a value is a writing to target
static bool isWrite(const llvm::Value* value, const llvm::Value* target) {
  if (const StoreInst* store = dyn_cast<StoreInst>(value)) {
    return store->getOperand(1) == target;
  }
  return false;
}

// Check if an equivalence set has write operation and update idWrite
void AMDILEGPointerManagerImpl::PtrEqSet::updateWrite() {
  for (IdValueMap::iterator I = idValueMap.begin(), E = idValueMap.end();
      I != E; ++I) {
    unsigned vid = I->first;
    ValueSet& valueSet = I->second;
    for (ValueSet::iterator VI = valueSet.begin(), VE = valueSet.end();
        VI != VE; ++VI) {
      const Value* V = *VI;
      for (Value::const_use_iterator UI = V->use_begin(), UE = V->use_end();
          UI != UE; ++UI) {
        const User* user = *UI;
        DEBUG(dbgs() << "[isWrite] " << *V << " <= " << *user << " " <<
            isWrite(user, V) << '\n');
        if (isWrite(user, V)) {
          DEBUG(dbgs() << "[updateWrite] resid=" << vid << " has write: " <<
              *user << '\n');
          setWrite(vid, true);
        }
      }
    }
  }
}

bool AMDILEGPointerManagerImpl::PtrEqSet::requiresDefaultResId(
    const Value* value) {
  ValueIdMap::iterator loc = valueIdMap.find(value);
  DEBUG(if (loc == valueIdMap.end()) {
    dbgs() << "Error: valueIdMap does not contain " << *value << '\n';
    llvm_unreachable("value not found in PtrEqSet");
  });
  if (loc == valueIdMap.end())
    return true;
  return loc->second == 0;
}

unsigned AMDILEGPointerManagerImpl::PtrEqSet::assignResId(const Value* value,
    unsigned& id) {
  DEBUG_WITH_TYPE("pointermanager", dbgs() << "[PtrEqSet::assignResId] attempt " <<
      ::toStr(value) << " => " << id+1);
  ValueIdMap::iterator loc = valueIdMap.find(value);
  assert(loc != valueIdMap.end());
  unsigned vid = loc->second;
  IdSet::iterator vidLoc = idAssigned.find(vid);
  if (vidLoc == idAssigned.end()) {
    assert(idIdMap.find(vid) == idIdMap.end());
    idIdMap[vid] = ++id;
    idAssigned.insert(vid);
    DEBUG_WITH_TYPE("pointermanager", dbgs() << ", use new id " << id << '\n');
    return id;
  }
  assert(idIdMap.find(vid) != idIdMap.end());
  unsigned oldId = idIdMap[vid];
  DEBUG_WITH_TYPE("pointermanager", dbgs() << ", use old id " << oldId << '\n');
  return oldId;
}

std::string AMDILEGPointerManagerImpl::PtrEqSet::toStr() {
  std::string str;
  raw_string_ostream oss(str);
  oss << "valueIdMap:\n" << ::toStr(valueIdMap) << '\n' <<
      "idValueMap:\n" << ::toStr(idValueMap) << '\n';
  oss << "id with write: ";
  for (IdSet::iterator I = idWrite.begin(), E = idWrite.end(); I != E; ++I) {
    oss << *I << ' ';
  }
  oss << '\n';
  return oss.str();
}

void AMDILEGPointerManagerImpl::PtrEqSet::init() {
  unsigned id = 1;
  for (Module::const_iterator I = parent->begin(), E = parent->end(); I != E; ++I) {
    if (!isKernel(I)) {
      continue;
    }
    const Function* F = I;
    for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
        I != E; ++I) {
      const Argument* arg = I;
      if (!hasGlobalPtr(arg->getType()))
        continue;
      unsigned actualId = update(arg, id);
      assert(actualId == id);
      id++;
    }
  }
  DEBUG_WITH_TYPE("ptripa", dbgs() << "CollectKernelPtrs\n" << toStr());
}

unsigned AMDILEGPointerManagerImpl::PtrEqSet::getId(const Value* value) {

  unsigned id = ~0U;
  const DataLayout *DL = targetMachine->getDataLayout();
  SmallVector<Value *, 2> Objs;
  GetUnderlyingObjects(const_cast<Value *>(value), Objs, DL, 0);
  DEBUG_WITH_TYPE("ptripa", dbgs() << "[getId] " << *value << '\n');
  for (SmallVectorImpl<Value *>::iterator I = Objs.begin(), E = Objs.end();
      I != E; ++I) {
    unsigned tmpId = ~0U;
    Value* V = *I;
    DEBUG_WITH_TYPE("ptripa", dbgs() << "  " << *V);
    if (contains(V, &tmpId)) {
      // Do nothing.
    } else if (isa<LoadInst>(V) || isa<AllocaInst>(V)) {
      tmpId = 0;
    } else if (!V->getType()->isPointerTy()) {
      continue;
    } else if (Constant *C = dyn_cast<Constant>(V)) {
      assert(C->isNullValue() && "Non-Null absolute global address not allowed");
      tmpId = 0;
    }
    if (id == ~0U || tmpId == 0) {
      id = tmpId;
    }
    DEBUG_WITH_TYPE("ptripa", dbgs() << " tmpId=" << tmpId <<
        " id=" << id << '\n');
    id = update(V, id);
  }

  assert(id != ~0U && "ptripa cannot find id for pointer");
  id = update(value, id);
  return id;
}

void AMDILEGPointerManagerImpl::PtrEqSet::update(const Function* F,
    std::set<const Function*>& visited) {
  if (visited.find(F) != visited.end())
    return;
  visited.insert(F);
  DEBUG_WITH_TYPE("ptripa", dbgs() << "Analyze function " << F->getName() <<
      '\n');
  SmallVector<std::pair<const Argument*, unsigned>, 10> globalArgNo;

  // Collect used global pointer arguments
  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
      I != E; ++I) {
    const Argument* arg = I;
    if (!hasGlobalPtr(arg->getType()))
      continue;

    unsigned argNo = arg->getArgNo();
    DEBUG_WITH_TYPE("ptripa", dbgs() << "  " << F->getName() << ":arg" <<
        argNo << " " << *arg << '\n');
    if (arg->use_empty()) {
      DEBUG_WITH_TYPE("ptripa", dbgs() << "   is useless\n");
      continue;
    }

    globalArgNo.push_back(std::pair<const Argument*, unsigned>(arg, argNo));
  }

  if (globalArgNo.empty())
    return;

  // Get equivalent sets from caller
  for (Function::const_use_iterator I = F->use_begin(), E = F->use_end();
      I != E; ++I) {
    const User *UI = *I;
    if (isa<CallInst>(UI) || isa<InvokeInst>(UI)) {
      ImmutableCallSite CS(cast<Instruction>(UI));
      const Function* caller = CS.getCaller();
      if (isKernelStub(caller))
        continue;
      DEBUG_WITH_TYPE("ptripa", dbgs() << "    caller: " << caller->getName() <<
          " callsite: " << *UI << '\n');
      for (unsigned I = 0, E = globalArgNo.size(); I != E; ++I) {
        unsigned argNo = globalArgNo[I].second;
        const Argument* arg = globalArgNo[I].first;
        const Value* argVal = CS.getArgument(argNo);
        if (!contains(argVal)) {
          update(caller, visited);
        }
        DEBUG_WITH_TYPE("ptripa", dbgs() << "      arg" << argNo << ": " <<
            F->getName() << ":" << *arg << " <= " <<
            caller->getName() << ":" << *argVal << '\n');
        unsigned id = getId(argVal);
        assert(id != ~0U && "ptripa cannot find id for pointer");
        update(arg, id);
      }
    }
  }
}

void AMDILEGPointerManagerImpl::PtrEqSet::build(const Module* M,
    const TargetMachine& TM) {
  if (M == parent)
    return;
  parent = M;
  targetMachine = &TM;
  clear();
  init();

  std::set<const Function*> visited;
  const TargetIntrinsicInfo *TII = TM.getIntrinsicInfo();
  for (Module::const_iterator I = M->begin(), E = M->end(); I != E; ++I) {
    // Intrinsics have already been lowered, so ignore them
    if (I->isIntrinsic()) {
      continue;
    }
    // look for target specific intrinsics
    unsigned IntrinsicID = TII->getIntrinsicID(const_cast<Function*>(&(*I)));
    if (IntrinsicID != Intrinsic::not_intrinsic) {
      continue;
    }
    update(I, visited);
  }
  updateWrite();

  DEBUG(dbgs() << "ptrEqSet:\n" << toStr() << '\n');
}

bool AMDILEGPointerManagerImpl::PtrEqSet::hasWrite(unsigned id) {
  return idWrite.count(id);
}

bool AMDILEGPointerManagerImpl::PtrEqSet::setWrite(unsigned id, bool write) {
  if (write) idWrite.insert(id);
  return hasWrite(id);
}

bool AMDILEGPointerManagerImpl::PtrEqSet::hasWrite(const Value* value) {
  unsigned vid;
  bool tmp = contains(value, &vid);
  assert(tmp && "Value not in equivalence map");
  return hasWrite(vid);
}
