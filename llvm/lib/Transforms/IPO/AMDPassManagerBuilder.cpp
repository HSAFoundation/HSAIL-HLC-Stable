	//===- AMDPassManagerBuilder.cpp - Build AMD-specific Standard Pass -------===//
//
// Copyright (c) 2012, Advanced Micro Devices, Inc.
// All rights reserved.
//
//===----------------------------------------------------------------------===//
//
// This file defines the AMD-specific members of PassManagerBuilder class.
//
//===----------------------------------------------------------------------===//


#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm-c/Transforms/PassManagerBuilder.h"

#include "llvm/LLVMContext.h"
#include "llvm/AMDLLVMContextHook.h"
#include "llvm/PassManager.h"
#include "llvm/DefaultPasses.h"
#include "llvm/PassManager.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AMDOptOptions.h"

#include <string>
#include <set>

#include "../../../../lib/backends/common/library.hpp"


using namespace llvm;

void PassManagerBuilder::AMDpopulateFunctionPassManager(
  FunctionPassManager &FPM, LLVMContext* TheContext)
{
  addExtensionsToPM(EP_EarlyAsPossible, FPM);

  // Add LibraryInfo if we have some.
  if (LibraryInfo) FPM.add(new TargetLibraryInfo(*LibraryInfo));

  if (OptLevel == 0) return;

  addInitialAliasAnalysisPasses(FPM);

  FPM.add(createCFGSimplificationPass());
  FPM.add(createScalarReplAggregatesPass());
  FPM.add(createEarlyCSEPass());
  FPM.add(createLowerExpectIntrinsicPass());
}

void PassManagerBuilder::AMDpopulateModulePassManager(
  PassManagerBase &MPM, LLVMContext* TheContext, Module *pModule)
{
  AMDLLVMContextHook *amdhook = static_cast<AMDLLVMContextHook*>(
    TheContext->getAMDLLVMContextHook());
  assert(amdhook && "AMDContextHook isn't set up");
  AMDOptions *amdopts = &(amdhook->amdoptions);

  bool isHsail = AMDOptions::isTargetHSAIL(pModule->getTargetTriple());

  // Remove Noalias if necessary to make barrier work correctly
  MPM.add(createAMDFenceInfoAnalysisPass());
  if (!amdopts->AAForBarrier)
    MPM.add(createAMDRemoveNoaliasPass());

  bool GPUInlineAll = (Inliner != NULL);
  if (HLC_Experimental_Enable_Calls) {
    HLC_Disable_Amd_Inline_All = true;
  }
  GPUInlineAll = GPUInlineAll && !HLC_Disable_Amd_Inline_All;

  // If all optimizations are disabled, just run the always-inline pass.
  if (OptLevel == 0) {
    MPM.add(createAMDSymbolLinkagePass(amdopts->WholeProgram,
                                       amdhook->amdrtFunctions));
    if (Inliner) {
      if (amdopts->IsGPU) {
        MPM.add(createAMDSimplifyCallPass());
        MPM.add(createAMDInlineAllPass(amdopts->WholeProgram));
        MPM.add(createAMDDbgmovePass());
        if (amdopts->OptMem2reg) {
          MPM.add(createPromoteMemoryToRegisterPass());
        }

        // Need to do PreISel Peephole even for O0 as propagate sampler value is
        // needed, as we need to know the value of kernel defined samplers at
        // compile time
        MPM.add(createAMDPeepholeOpt(OptLevel));

        //MPM.add(createCFGSimplificationPass());
        //MPM.add(createScalarReplAggregatesPass(SRThreshold));
        //MPM.add(createInstructionCombiningPass());
      } else {
        // CPU
        MPM.add(Inliner);
      }
    }
    Inliner = 0;
    return;
  }

  // Add LibraryInfo if we have some.
  if (LibraryInfo) MPM.add(new TargetLibraryInfo(*LibraryInfo));

  // Add AMDAliasAnalysis before standard alias analysises so that
  // standard alias analysises win if they disagree.
  MPM.add(createAMDAliasAnalysisPass(amdopts->AAForBarrier));
  addInitialAliasAnalysisPasses(MPM);

  // In the whole program mode, mark non-kernel/non-reserved globals
  // as having internal linkage, so normal LLVM passes will be able
  // to delete them whenever possible.
  MPM.add(createAMDSymbolLinkagePass(amdopts->WholeProgram,
                                     amdhook->amdrtFunctions));

  // For folding lib functions.
  if (amdopts->OptSimplifyLibCall) {
    MPM.add(createAMDSimplifyLibCallsPass(amdopts->UnsafeMathOpt));
  }


  if (!DisableUnitAtATime) {
    MPM.add(createGlobalOptimizerPass());     // Optimize out global vars

    MPM.add(createIPSCCPPass());              // IP SCCP
    MPM.add(createDeadArgEliminationPass());  // Dead argument elimination

    MPM.add(createInstructionCombiningPass(amdopts->UnsafeMathOpt));// Clean up after IPCP & DAE
    MPM.add(createCFGSimplificationPass());   // Clean up after IPCP & DAE
  }

  // Start of CallGraph SCC passes.
  if (!DisableUnitAtATime)
    MPM.add(createPruneEHPass());             // Remove dead EH info
  if (Inliner) {
    MPM.add(Inliner);
    Inliner = 0;
  }
  if (!DisableUnitAtATime)
    MPM.add(createFunctionAttrsPass());       // Set readonly/readnone attrs
  if (OptLevel > 2) {
    MPM.add(createArgumentPromotionPass(amdopts->APThreshold));   // Scalarize uninlined fn args
  }

  // Start of function pass.
  // Break up aggregate allocas, using SSAUpdater.
  MPM.add(createScalarReplAggregatesPass(amdopts->SRThreshold, isHsail));
  MPM.add(createEarlyCSEPass());              // Catch trivial redundancies
  if (!DisableSimplifyLibCalls)
    MPM.add(createSimplifyLibCallsPass());    // Library Call Optimizations

  if (!amdopts->IsGPU && amdopts->WGLevelExecution) {
    MPM.add(createCFGSimplificationPass());
    MPM.add(createLoopRotatePass());
#if 0
    MPM.add(createSinkingPass());
#endif
    MPM.add(createLoopInstSimplifyPass());
    MPM.add(createLCSSAPass());
    MPM.add(createAMDWorkGroupLevelExecutionPass());
  }

  MPM.add(createInstructionCombiningPass(amdopts->UnsafeMathOpt)); // Cleanup for scalarrepl.
  MPM.add(createJumpThreadingPass());         // Thread jumps.
  MPM.add(createCorrelatedValuePropagationPass()); // Propagate conditionals
  MPM.add(createCFGSimplificationPass());     // Merge & remove BBs
  MPM.add(createInstructionCombiningPass(amdopts->UnsafeMathOpt));  // Combine silly seq's

  if (amdopts->IsGPU && OptLevel > 1 && amdopts->SRAEThreshold > 0 && !isHsail) {
    MPM.add(createAMDScalarReplArrayElemPass(4, 32, amdopts->SRAEThreshold));
  }

  MPM.add(createTailCallEliminationPass());   // Eliminate tail calls
  MPM.add(createCFGSimplificationPass());     // Merge & remove BBs
  MPM.add(createReassociatePass());           // Reassociate expressions

  MPM.add(createLoopRotatePass());            // Rotate Loop
  if (amdopts->OptLICM) {
    MPM.add(createLICMPass());                // Hoist loop invariants
  }
  MPM.add(createLoopUnswitchPass(SizeLevel || OptLevel < 3));
  MPM.add(createInstructionCombiningPass(amdopts->UnsafeMathOpt));
  MPM.add(createIndVarSimplifyPass());        // Canonicalize indvars
  // MPM.add(createLoopIdiomPass());             // Recognize idioms like memset.
  MPM.add(createLoopDeletionPass());          // Delete dead loops

  if (!DisableUnrollLoops) {
    // Unroll samll loops
    if (isHsail) {
      MPM.add(createLoopUnrollPass(HLC_Unroll_Threshold,
                                   HLC_Unroll_Count,
                                   HLC_Unroll_Allow_Partial,
                                   HLC_Unroll_Scratch_Threshold));
    } else {
      MPM.add(createLoopUnrollPass(amdopts->LUThreshold,
                                   amdopts->LUCount,
                                   amdopts->LUAllowPartial,
                                   amdopts->UnrollScratchThreshold));
    }
  }
  addExtensionsToPM(EP_LoopOptimizerEnd, MPM);

  MPM.add(createInstructionCombiningPass(amdopts->UnsafeMathOpt));  // Clean up after the unroller
  if (OptLevel > 1)
    MPM.add(createGVNPass()); 

  // On pre-SI, constant and private loads/stores uses register copy
  // IL instructions such as mov r65, cb0[r1021], which requires alignment
  // of load/store size. So the benefit from this opt on pre-SI would be
  // limited. For simplisity, enable MemCombine for SI+ only.
  // HSA is always SI+
  if (amdopts->IsGPU && (isHsail || amdopts->GPUArch >= amd::GPU_Library_SI) &&
     amdopts->OptMemCombineMaxVecGen > 1) {
    MPM.add(createAMDAlignmentAnalysisPass());
    // Memory vectorization
    MPM.add(createAMDMemCombinePass(amdopts->OptMemCombineMaxVecGen, 4));
    if (OptLevel > 1)
      MPM.add(createGVNPass());                 // Remove redundancies
  }
  if (!amdopts->IsGPU) {
    MPM.add(createMemCpyOptPass());           // Remove memcpy / form memset
  }
  MPM.add(createSCCPPass());                  // Constant prop with SCCP

  // Run instcombine after redundancy elimination to exploit opportunities
  // opened up by them.
  MPM.add(createInstructionCombiningPass(amdopts->UnsafeMathOpt));
  MPM.add(createJumpThreadingPass());         // Thread jumps
  MPM.add(createCorrelatedValuePropagationPass());
  MPM.add(createDeadStoreEliminationPass());  // Delete dead stores

  addExtensionsToPM(EP_ScalarOptimizerLate, MPM);

  MPM.add(createAggressiveDCEPass());         // Delete dead instructions
  MPM.add(createCFGSimplificationPass());     // Merge & remove BBs
  MPM.add(createInstructionCombiningPass(amdopts->UnsafeMathOpt));  // Clean up after everything.

  if (!DisableUnitAtATime) {
    // FIXME: We shouldn't bother with this anymore.
    MPM.add(createStripDeadPrototypesPass()); // Get rid of dead prototypes

    // GlobalOpt already deletes dead functions and globals, at -O3 try a
    // late pass of GlobalDCE.  It is capable of deleting dead cycles.
    if (OptLevel > 2)
      MPM.add(createGlobalDCEPass());         // Remove dead fns and globals.

    if (OptLevel > 1)
      MPM.add(createConstantMergePass());     // Merge dup global constants
  }

  if (amdopts->IsGPU && GPUInlineAll) {
    // Inline all functions and then do a few cleanup passes.
    MPM.add(createAMDInlineAllPass(amdopts->WholeProgram));

    MPM.add(createAMDPeepholeOpt(OptLevel));

    // Do SCCP as AMD OpenCL Peephole opt may have replaced calls to
    // __amdil_is_constant() with constant value.
    // This happens a lot with image read library functions.
    // Do this before CFGSimplification as SCCP may leave dead blocks behind
    // which can be removed by CFGSimplification.
    MPM.add(createSCCPPass());

    MPM.add(createCFGSimplificationPass());
    MPM.add(createScalarReplAggregatesPass(amdopts->SRThreshold));
    MPM.add(createInstructionCombiningPass(amdopts->UnsafeMathOpt));
    MPM.add(createGlobalDCEPass());         // Remove dead fns and globals.
  }

  if (amdopts->OptPrintLiveness) {
    MPM.add(createAMDLivenessPrinterPass());
  }
}
