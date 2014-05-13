//
// Copyright (c) 2013 Advanced Micro Devices, Inc. All rights reserved.
//
// Look in llvm/include/llvm/Transforms/IPO/AMDOptOptions.h for a description of this file
// 
#include "llvm/Support/CommandLine.h"

using namespace llvm;
unsigned HLC_Unroll_Threshold;
unsigned HLC_Unroll_Count;
bool HLC_Unroll_Allow_Partial;
unsigned HLC_Unroll_Scratch_Threshold;
bool HLC_Disable_Amd_Inline_All;
bool HLC_Force_Always_Inliner_Pass;
bool HLC_Experimental_Enable_Calls;
bool DisableInline;

static cl::opt<unsigned, true>
HLCUnrollThreshold("hlc-unroll-threshold", cl::Hidden,
  cl::desc("The cut-off point for automatic loop unrolling"), 
  cl::location(HLC_Unroll_Threshold), cl::init(800));

static cl::opt<unsigned, true>
HLCUnrollCount("hlc-unroll-count", cl::Hidden,
  cl::desc("Use this unroll count for all loops, for testing purposes"),
  cl::location(HLC_Unroll_Count), cl::init(0));

static cl::opt<unsigned, true>
HLCUnrollScratchThreshold("hlc-unroll-scratch-threshold", cl::Hidden,
  cl::desc("The cut-off point for automatic loop unrolling of loops using alloca arrays"),
  cl::location(HLC_Unroll_Scratch_Threshold), cl::init(700));

static cl::opt<bool, true>
HLCUnrollAllowPartial("hlc-unroll-allow-partial" , cl::Hidden,
  cl::desc("Allows loops to be partially unrolled until "
           "-hlc-unroll-threshold loop size is reached."),
  cl::location(HLC_Unroll_Allow_Partial), cl::init(false));

static cl::opt<bool, true>
DisableAmdInlineAll("disable-amd-inline-all", cl::Hidden,
  cl::desc("Disable default AMDInlineAll pass"),
  cl::location(HLC_Disable_Amd_Inline_All), cl::init(false));

static cl::opt<bool, true>
ForceAlwaysInlinerPass("force-always-inliner-pass", cl::Hidden,
  cl::desc("Force AlwaysInline pass instead of SimpleInline"),
  cl::location(HLC_Force_Always_Inliner_Pass), cl::init(false));

static cl::opt<bool, true>
ExperimentalEnableCalls("experimental-enable-calls", cl::Hidden,
  cl::desc("enables both -disable-amd-inline-all and "
  "-force-always-inliner-pass options"),
  cl::location(HLC_Experimental_Enable_Calls), cl::init(false));

static cl::opt<bool,true>
  DisableInlineOpt("disable-inlining", cl::desc("Do not run the inliner pass"),
  cl::location(DisableInline), cl::init(false));

