#if 1 || defined (AMD_OPENCL_MEMCOMBINE)
//===- AMDAlignmentAnalysis.hpp - alignment analysis ---------------===//
//===----------------------------------------------------------------------===//
//
// This file defines the alignment analysis interface.
// 
//===----------------------------------------------------------------------===//

#ifndef LLVM_AMD_ALIGNMENT_ANALYSIS_H
#define LLVM_AMD_ALIGNMENT_ANALYSIS_H

#if 0
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#endif
#include "llvm/DataLayout.h"
#if 0
#include "../IPO/AMDSymbolName.h"
#include <set>
#include <sstream>
#include <stack>
#endif

namespace llvm {

class AlignmentAnalysis {
protected:
  const DataLayout* DL;

public:
  static char ID;

public:
  AlignmentAnalysis() : DL(NULL) {}
  virtual ~AlignmentAnalysis() {}
  virtual uint64_t getAlignment(Value& addr) = 0;
};
}

#endif
#endif // AMD_OPENCL_MEMCOMBINE
