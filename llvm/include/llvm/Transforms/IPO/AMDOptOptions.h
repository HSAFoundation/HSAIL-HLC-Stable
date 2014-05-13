//
// Copyright (c) 2013 Advanced Micro Devices, Inc. All rights reserved.
//

// This file contains flags that are available for command line 
// invocations of tools like opt, and the llvm optimization 
// libraries used by AMD online and offline compilers.
// Please look at the companian file
// llvm/lib/Transforms/IPO/AMDOptOptions.cpp for an example usage
// of one of these flags. 
// This header and the companian cpp provide 
// developers with a common place 
// to put LLVM opt options, which will be exported to command line using 
// cl::opt.
// The flags are initialized in the cpp file 
// like any cl::opt flag found in the 
// different llvm optimizations. The flags listed here are 
// bound to the cl::opt flags, but individual tools may 
// use their defaults and change the cl::opt bound variable's 
// default values.
// The AMD pass manager will pick up these flags and 
// pass them to optimization passes during pass construction.
// For an example, refer to lib/Transforms/IPO/AMDPassManagerBuilder.cpp
// 
// This is close to the convention promoted by LLVM. 
// For this LLVM usage example please refer to:
// http://llvm.org/releases/3.1/docs/CommandLine.html
// Please look under the section titled:
// "Internal vs External Storage".

#ifndef _AMD_OPT_OPTIONS
#define _AMD_OPT_OPTIONS

extern unsigned HLC_Unroll_Threshold;
extern unsigned HLC_Unroll_Count;
extern bool HLC_Unroll_Allow_Partial;
extern unsigned HLC_Unroll_Scratch_Threshold;

extern bool HLC_Disable_Amd_Inline_All;
extern bool HLC_Force_Always_Inliner_Pass;
extern bool HLC_Experimental_Enable_Calls;
extern bool DisableInline;

#endif
