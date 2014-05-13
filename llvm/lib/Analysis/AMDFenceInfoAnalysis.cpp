//===- AMDFenceInfoAnalysis.cpp - Implement AMD Fence Info Analysis----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the AMDFenceInfoAnalysis pass, which does analysis
// to find out all functions that directly or indirectly call a barrier
// function or a fence intrinsic. For each such function, it stores the
// info (address space, read/write) of all fences that's called.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "amd-fence-info"
#include "llvm/Analysis/AMDFenceInfoAnalysis.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <set>
#include <vector>

using namespace llvm;

// Register this pass...
char AMDFenceInfoAnalysis::ID = 0;
INITIALIZE_PASS(AMDFenceInfoAnalysis, "amd-fence-info",
                "AMD Fence Info Analysis", false, true)

ModulePass *llvm::createAMDFenceInfoAnalysisPass() {
  return new AMDFenceInfoAnalysis();
}

typedef struct {
  const char* Name;
  unsigned Flags;
} FenceFuncInfoTy;

// Fence info of all amdil intrinsic fence functions.
static const FenceFuncInfoTy FenceIntrinsicsInfo[] = {
  {"__amdil_barrier", AMDFenceInfoAnalysis::FULL_MEM_FENCE},
  {"__amdil_barrier_global", AMDFenceInfoAnalysis::GLOBAL_MEM_FENCE |
                             AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_barrier_local", AMDFenceInfoAnalysis::LOCAL_MEM_FENCE |
                            AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_barrier_region", AMDFenceInfoAnalysis::REGION_MEM_FENCE |
                             AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_barrier_global_local", AMDFenceInfoAnalysis::GL_MEM_FENCE |
                                   AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_barrier_region_local", AMDFenceInfoAnalysis::RL_MEM_FENCE |
                                   AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_barrier_global_region", AMDFenceInfoAnalysis::RG_MEM_FENCE |
                                   AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amd_barrier", AMDFenceInfoAnalysis::FULL_MEM_FENCE},
  {"__amdil_mem_fence", AMDFenceInfoAnalysis::FULL_MEM_FENCE},
  {"__amdil_mem_fence_global", AMDFenceInfoAnalysis::GLOBAL_MEM_FENCE |
                               AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_mem_fence_local", AMDFenceInfoAnalysis::LOCAL_MEM_FENCE |
                              AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_mem_fence_region", AMDFenceInfoAnalysis::REGION_MEM_FENCE |
                               AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_mem_fence_global_local", AMDFenceInfoAnalysis::GL_MEM_FENCE |
                                     AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_mem_fence_region_global", AMDFenceInfoAnalysis::RG_MEM_FENCE |
                                      AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_mem_fence_region_local", AMDFenceInfoAnalysis::RL_MEM_FENCE |
                                     AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_read_mem_fence", AMDFenceInfoAnalysis::ALL_AS_MEM_FENCE |
                             AMDFenceInfoAnalysis::READ_MEM_FENCE},
  {"__amdil_read_mem_fence_global", AMDFenceInfoAnalysis::GLOBAL_MEM_FENCE |
                                    AMDFenceInfoAnalysis::READ_MEM_FENCE},
  {"__amdil_read_mem_fence_local", AMDFenceInfoAnalysis::LOCAL_MEM_FENCE |
                                   AMDFenceInfoAnalysis::READ_MEM_FENCE},
  {"__amdil_read_mem_fence_region", AMDFenceInfoAnalysis::REGION_MEM_FENCE |
                                    AMDFenceInfoAnalysis::READ_MEM_FENCE},
  {"__amdil_read_mem_fence_global_local", AMDFenceInfoAnalysis::GL_MEM_FENCE |
                                          AMDFenceInfoAnalysis::READ_MEM_FENCE},
  {"__amdil_read_mem_fence_region_global", AMDFenceInfoAnalysis::RG_MEM_FENCE |
                                          AMDFenceInfoAnalysis::READ_MEM_FENCE},
  {"__amdil_read_mem_fence_region_local", AMDFenceInfoAnalysis::RL_MEM_FENCE |
                                          AMDFenceInfoAnalysis::READ_MEM_FENCE},
  {"__amdil_write_mem_fence", AMDFenceInfoAnalysis::ALL_AS_MEM_FENCE |
                              AMDFenceInfoAnalysis::WRITE_MEM_FENCE},
  {"__amdil_write_mem_fence_global",
   AMDFenceInfoAnalysis::GLOBAL_MEM_FENCE |
   AMDFenceInfoAnalysis::WRITE_MEM_FENCE},
  {"__amdil_write_mem_fence_local", AMDFenceInfoAnalysis::LOCAL_MEM_FENCE |
                                    AMDFenceInfoAnalysis::WRITE_MEM_FENCE},
  {"__amdil_write_mem_fence_region",
   AMDFenceInfoAnalysis::REGION_MEM_FENCE |
   AMDFenceInfoAnalysis::WRITE_MEM_FENCE},
  {"__amdil_write_mem_fence_global_local",
   AMDFenceInfoAnalysis::GL_MEM_FENCE |
   AMDFenceInfoAnalysis::WRITE_MEM_FENCE},
  {"__amdil_write_mem_fence_region_global",
   AMDFenceInfoAnalysis::RG_MEM_FENCE | AMDFenceInfoAnalysis::WRITE_MEM_FENCE},
  {"__amdil_write_mem_fence_region_local",
   AMDFenceInfoAnalysis::RL_MEM_FENCE |
   AMDFenceInfoAnalysis::WRITE_MEM_FENCE},
  {"__amdil_gws", AMDFenceInfoAnalysis::FULL_MEM_FENCE},
  {"__amdil_gws_global", AMDFenceInfoAnalysis::GLOBAL_MEM_FENCE |
                         AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_gws_local", AMDFenceInfoAnalysis::LOCAL_MEM_FENCE |
                        AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_gws_region", AMDFenceInfoAnalysis::REGION_MEM_FENCE |
                         AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_gws_global_local", AMDFenceInfoAnalysis::GL_MEM_FENCE |
                               AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_gws_global_region", AMDFenceInfoAnalysis::RG_MEM_FENCE |
                                AMDFenceInfoAnalysis::RW_MEM_FENCE},
  {"__amdil_gws_local_region", AMDFenceInfoAnalysis::RL_MEM_FENCE |
                               AMDFenceInfoAnalysis::RW_MEM_FENCE},
};
static const unsigned NumFenceIntrinsics
  = array_lengthof(FenceIntrinsicsInfo);

#ifndef NDEBUG
void AMDFenceInfoAnalysis::dumpFenceFlags(unsigned Flags) {
  if (Flags & LOCAL_MEM_FENCE)
    dbgs() << " local ";
  if (Flags & GLOBAL_MEM_FENCE)
    dbgs() << " global ";
  if (Flags & REGION_MEM_FENCE)
    dbgs() << " region ";
  if (Flags & READ_MEM_FENCE)
    dbgs() << " read ";
  if (Flags & WRITE_MEM_FENCE)
    dbgs() << " write ";
}
#endif

// Build the _FenceFlagsMap by propagating any called memory fence
// intrinsic's fence flags (which indicates the fence's address
// spaces and read/write property) to all its direct and indirect callers.
void AMDFenceInfoAnalysis::buildFenceFlagsMap(const Module &M) {
  std::vector<const Function*> Worklist;
  std::set<const Instruction*> Visited;

  DEBUG(dbgs() << "-- start building fence flags map\n";);
  for (unsigned i = 0; i < NumFenceIntrinsics; ++i) {
    const FenceFuncInfoTy &IntrinsicInfo = FenceIntrinsicsInfo[i];
    const Function *F = M.getFunction(IntrinsicInfo.Name);
    if (!F) continue;
    _ModuleHasFence = true;
    _FenceFlagsMap[F] = IntrinsicInfo.Flags;
    Worklist.push_back(F);
    DEBUG(dbgs() << "found fence intrinsic " << F->getName() << "(): (";
          dumpFenceFlags(IntrinsicInfo.Flags); dbgs() << ")\n");
  }
  if (!_ModuleHasFence) return;

  while (!Worklist.empty()) {
    const Function *F = Worklist.back();
    Worklist.pop_back();
    DEBUG(dbgs() << F->getName() << "(): pushing fence flags to callers\n");
    for (Value::const_use_iterator I = F->use_begin(), E = F->use_end();
      I != E; ++I) {
      const User *U = *I;
      if (!isa<CallInst>(U) && !isa<InvokeInst>(U)) continue;
      const Instruction *Inst = cast<Instruction>(U);
      if (Visited.count(Inst)) continue;
      Visited.insert(Inst);

      unsigned CalleeFlags = getFenceFlags(*F);
      const Function *Caller = Inst->getParent()->getParent();
      unsigned CallerFlags = getFenceFlags(*Caller);
      if (CallerFlags != CalleeFlags) {
        _FenceFlagsMap[Caller] = CallerFlags | CalleeFlags;
        Worklist.push_back(Caller);
        DEBUG(dbgs() << "  caller " << Caller->getName() << " old flags (";
              dumpFenceFlags(CallerFlags); dbgs() << ") new flags (";
              dumpFenceFlags(CallerFlags|CalleeFlags); dbgs() << ")\n");
      }
    }
  }
  DEBUG(dbgs() << "-- end building fence flags map\n";);
}
