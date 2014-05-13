//===- AMDFenceInfoAnalysis.h -----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// The AMDFenceInfoAnalysis pass does analysis to find out all functions
// that directly or indirectly call a barrier() function or an AMDIL memory
// fence intrinsic. For each such function, it stores the
// info (address space, read/write) of all fences that's called.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_AMD_FENCE_INFO_ANALYSIS_H
#define LLVM_ANALYSIS_AMD_FENCE_INFO_ANALYSIS_H

#include "llvm/ADT/ValueMap.h"
#include "llvm/Pass.h"

namespace llvm {
/// AMDFenceInfoAnalysis - Find out address space etc info about memory
/// fences called directly or indirectly by each function.
class AMDFenceInfoAnalysis : public ModulePass  {
public:
  enum {
    LOCAL_MEM_FENCE  = 1 << 0,
    GLOBAL_MEM_FENCE = 1 << 1,
    REGION_MEM_FENCE = 1 << 2,
    READ_MEM_FENCE   = 1 << 3,
    WRITE_MEM_FENCE  = 1 << 4,
    GL_MEM_FENCE     = LOCAL_MEM_FENCE | GLOBAL_MEM_FENCE,
    RG_MEM_FENCE     = REGION_MEM_FENCE | GLOBAL_MEM_FENCE,
    RL_MEM_FENCE     = REGION_MEM_FENCE | LOCAL_MEM_FENCE,
    ALL_AS_MEM_FENCE = LOCAL_MEM_FENCE | GLOBAL_MEM_FENCE | REGION_MEM_FENCE,
    RW_MEM_FENCE     = READ_MEM_FENCE | WRITE_MEM_FENCE,
    FULL_MEM_FENCE   = ALL_AS_MEM_FENCE | RW_MEM_FENCE,
  };

  static char ID; // Class identification

#ifndef NDEBUG
  static void dumpFenceFlags(unsigned Flags);
#endif

  AMDFenceInfoAnalysis() : ModulePass(ID) {
    initializeAMDFenceInfoAnalysisPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();                         // Does not transform code
  }

  virtual bool runOnModule(Module &M) {
    _ModuleHasFence = false;
    _FenceFlagsMap.clear();
    buildFenceFlagsMap(M);
    return false;
  }

  // Interface for querying the analysis result.
  // Returns if the given function directly or indirectly calls any memory
  // fence intrinsics.
  // A zero return value indicates that F does not call any memory fences.
  // A non-zero return value's bit values contain the address space information
  // and read/write property of the fences called by F.
  unsigned getFenceFlags(const Function &F) const {
    ValueMap<const Function*, unsigned>::const_iterator I
      = _FenceFlagsMap.find(&F);
    if (I == _FenceFlagsMap.end()) return 0;
    return I->second;
  }

  // Does this module have any calls to memory fences?
  bool ModuleHasFenceCalls() const {
    return _ModuleHasFence;
  }

private:
  void buildFenceFlagsMap(const Module &M);

private:
  // Does this module have any memory fence calls?
  bool _ModuleHasFence;
  // For each function that directly or indirectly calls a memory fence,
  // or is a memory fence intrinsic function itself,
  // it stores the fence flags - the address space that the fences
  // applies, is it a read fence or a write fence or both?
  // One can query this map to find out what kind of fences a function
  // calls, if it calls any.
  ValueMap<const Function*, unsigned> _FenceFlagsMap;
};
} // namespace llvm
#endif //LLVM_ANALYSIS_AMD_FENCE_INFO_ANALYSIS_H
