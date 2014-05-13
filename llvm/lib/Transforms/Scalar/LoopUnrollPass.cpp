//===-- LoopUnroll.cpp - Loop unroller pass -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass implements a simple loop unroller.  It works best when loops have
// been canonicalized by the -indvars pass, allowing it to determine the trip
// counts of loops easily.
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "loop-unroll"
#include "llvm/IntrinsicInst.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/CodeMetrics.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"
#include "llvm/DataLayout.h"
#include <climits>

#if 1 || defined(AMD_OPENCL)
#include "llvm/AMDLLVMContextHook.h"
#include "llvm/Analysis/AMDPragmaInfo.h"
#endif

using namespace llvm;

static cl::opt<unsigned>
UnrollThreshold("unroll-threshold", cl::init(150), cl::Hidden,
  cl::desc("The cut-off point for automatic loop unrolling"));

#if defined(AMD_OPENCL) || 1
static cl::opt<unsigned>
UnrollScratchThreshold("unroll-scratch-threshold", cl::init(500), cl::Hidden,
  cl::desc("The cut-off point for automatic loop unrolling of loops using alloca arrays"));
#endif

static cl::opt<unsigned>
UnrollCount("unroll-count", cl::init(0), cl::Hidden,
  cl::desc("Use this unroll count for all loops, for testing purposes"));

static cl::opt<bool>
UnrollAllowPartial("unroll-allow-partial", cl::init(false), cl::Hidden,
  cl::desc("Allows loops to be partially unrolled until "
           "-unroll-threshold loop size is reached."));

static cl::opt<bool>
UnrollRuntime("unroll-runtime", cl::ZeroOrMore, cl::init(false), cl::Hidden,
  cl::desc("Unroll loops with run-time trip counts"));

namespace {
  class LoopUnroll : public LoopPass {
  public:
    static char ID; // Pass ID, replacement for typeid
#if defined(AMD_OPENCL) || 1
    LoopUnroll(int T = -1, int C = -1,  int P = -1, int S = -1)
      : LoopPass(ID)
#else
    LoopUnroll(int T = -1, int C = -1,  int P = -1) : LoopPass(ID)
#endif
    {
      CurrentThreshold = (T == -1) ? UnrollThreshold : unsigned(T);
      CurrentCount = (C == -1) ? UnrollCount : unsigned(C);
      CurrentAllowPartial = (P == -1) ? UnrollAllowPartial : (bool)P;

      UserThreshold = (T != -1) || (UnrollThreshold.getNumOccurrences() > 0);

#if defined(AMD_OPENCL) || 1
      LUScratchThreshold = (S == -1) ? UnrollScratchThreshold : (unsigned)S;
#endif
      initializeLoopUnrollPass(*PassRegistry::getPassRegistry());
    }

    /// A magic value for use with the Threshold parameter to indicate
    /// that the loop unroll should be performed regardless of how much
    /// code expansion would result.
    static const unsigned NoThreshold = UINT_MAX;

    // Threshold to use when optsize is specified (and there is no
    // explicit -unroll-threshold).
    static const unsigned OptSizeUnrollThreshold = 50;

    // Default unroll count for loops with run-time trip count if
    // -unroll-count is not set
    static const unsigned UnrollRuntimeCount = 8;

    unsigned CurrentCount;
    unsigned CurrentThreshold;
    bool     CurrentAllowPartial;
    bool     UserThreshold;        // CurrentThreshold is user-specified.

#if defined(AMD_OPENCL) || 1
    unsigned LUScratchThreshold;
#endif

    bool runOnLoop(Loop *L, LPPassManager &LPM);

    /// This transformation requires natural loop information & requires that
    /// loop preheaders be inserted into the CFG...
    ///
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LoopInfo>();
      AU.addPreserved<LoopInfo>();
      AU.addRequiredID(LoopSimplifyID);
      AU.addPreservedID(LoopSimplifyID);
      AU.addRequiredID(LCSSAID);
      AU.addPreservedID(LCSSAID);
      AU.addRequired<ScalarEvolution>();
      AU.addPreserved<ScalarEvolution>();
      // FIXME: Loop unroll requires LCSSA. And LCSSA requires dom info.
      // If loop unroll does not preserve dom info then LCSSA pass on next
      // loop will receive invalid dom info.
      // For now, recreate dom info, if loop is unrolled.
      AU.addPreserved<DominatorTree>();
    }
  };
}

char LoopUnroll::ID = 0;
INITIALIZE_PASS_BEGIN(LoopUnroll, "loop-unroll", "Unroll loops", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfo)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_DEPENDENCY(LCSSA)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolution)
INITIALIZE_PASS_END(LoopUnroll, "loop-unroll", "Unroll loops", false, false)

#if defined(AMD_OPENCL) || 1
Pass *llvm::createLoopUnrollPass(int Threshold, int Count, int AllowPartial,
  int ScratchThreshold) {
  return new LoopUnroll(Threshold, Count, AllowPartial, ScratchThreshold);
}
#else
Pass *llvm::createLoopUnrollPass(int Threshold, int Count, int AllowPartial) {
  return new LoopUnroll(Threshold, Count, AllowPartial);
}
#endif

/// ApproximateLoopSize - Approximate the size of the loop.
static unsigned ApproximateLoopSize(const Loop *L, unsigned &NumCalls,
                                    const DataLayout *TD) {
  CodeMetrics Metrics;
  for (Loop::block_iterator I = L->block_begin(), E = L->block_end();
       I != E; ++I)
    Metrics.analyzeBasicBlock(*I, TD);
  NumCalls = Metrics.NumInlineCandidates;

  unsigned LoopSize = Metrics.NumInsts;

  // Don't allow an estimate of size zero.  This would allows unrolling of loops
  // with huge iteration counts, which is a compile time problem even if it's
  // not a problem for code quality.
  if (LoopSize == 0) LoopSize = 1;

  return LoopSize;
}

#if defined(AMD_OPENCL) || 1
// Is this GetElementPtr based on only constants and the loop induction value?
static bool BasedOnIndVar(const Loop *L, const GetElementPtrInst *GEPI) {

  for (unsigned i = 1, e = GEPI->getNumOperands(); i != e; ++i) {
    if (isa<Constant>(GEPI->getOperand(i)) || GEPI->getOperand(i) == L->getCanonicalInductionVariable()) {
      continue;
    } else if (const BinaryOperator *binop = dyn_cast<BinaryOperator>(GEPI->getOperand(i))) {
      // If it's a simple ofset from the induction variable, it's also acceptable.
      if (binop->getOperand(0) == L->getCanonicalInductionVariable() && 
          isa<Constant>(binop->getOperand(1))) {
          DEBUG(dbgs() << "Found good: " << *binop << "\n");
          continue;
      }
    }
    DEBUG(dbgs() << "Failing due to: " << GEPI->getOperand(i) << "\n");
    return false;
  }
  return true;
}

// Does this loop contain an address computation into an alloca'ed array
// that is based only on the induction variable and constants?
static bool ContainsAllocaCandidate(const Loop *L) {
  for (Loop::block_iterator BB = L->block_begin(), BE = L->block_end();
       BB != BE; ++BB) {
    for (BasicBlock::const_iterator II = (*BB)->begin(), E = (*BB)->end();
         II != E; ++II) {
    
      if (const GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(II)) {
        DEBUG(dbgs() << "GEP: " << *II << "\n");
        // Ok, this instruction is computing an address. Check if it's an address
        // into an alloca'ed object.
        if (isa<AllocaInst>(GEPI->getOperand(0))) {
          DEBUG(dbgs() << *GEPI->getOperand(0) << "\n"); 
          bool found = BasedOnIndVar(L, GEPI);

          if (found) {
            return true;
          }
        }
      }
    }
  }
  return false;
}
#endif

bool LoopUnroll::runOnLoop(Loop *L, LPPassManager &LPM) {
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  ScalarEvolution *SE = &getAnalysis<ScalarEvolution>();

  BasicBlock *Header = L->getHeader();
  DEBUG(dbgs() << "Loop Unroll: F[" << Header->getParent()->getName()
        << "] Loop %" << Header->getName() << "\n");
  (void)Header;

  // Determine the current unrolling threshold.  While this is normally set
  // from UnrollThreshold, it is overridden to a smaller value if the current
  // function is marked as optimize-for-size, and the unroll threshold was
  // not user specified.
  unsigned Threshold = CurrentThreshold;
  if (!UserThreshold &&
      Header->getParent()->getFnAttributes().
        hasAttribute(Attributes::OptimizeForSize))
    Threshold = OptSizeUnrollThreshold;

  // Find trip count and trip multiple if count is not available
  unsigned TripCount = 0;
  unsigned TripMultiple = 1;
  // Find "latch trip count". UnrollLoop assumes that control cannot exit
  // via the loop latch on any iteration prior to TripCount. The loop may exit
  // early via an earlier branch.
  BasicBlock *LatchBlock = L->getLoopLatch();
  if (LatchBlock) {
    TripCount = SE->getSmallConstantTripCount(L, LatchBlock);
    TripMultiple = SE->getSmallConstantTripMultiple(L, LatchBlock);
  }
  // Use a default unroll-count if the user doesn't specify a value
  // and the trip count is a run-time value.  The default is different
  // for run-time or compile-time trip count loops.
  unsigned Count = CurrentCount;
  if (UnrollRuntime && CurrentCount == 0 && TripCount == 0)
    Count = UnrollRuntimeCount;

#if defined(AMD_OPENCL) || 1
  AMDLLVMContextHook *amdhook = static_cast<AMDLLVMContextHook*>(
    Header->getParent()->getContext().getAMDLLVMContextHook());
  bool SaveLog = false;
  if (LoopPragmaInfo *PI = L->getPragmaInfo()) {
    // Pragma overrides the command line.
    if (PI->hasPragmaUnrollCount()) {
      SaveLog = (amdhook && amdhook->LLVMBuildLog);
      if (SaveLog) {
        amdhook->LLVMBuildLog->append("LOOP UNROLL: pragma unroll (line ");
        amdhook->LLVMBuildLog->append(
          AMDLLVMContextHook::getAsCString(PI->getLoopLineNo()));
        amdhook->LLVMBuildLog->append(")\n");
      }
      if (PI->hasPragmaUnrollCountUsed()) {
        DEBUG(dbgs() <<
          "  NOT UNROLL : loop with #pragma unroll was unrolled before\n");
        if (SaveLog) {
          amdhook->LLVMBuildLog->append(
            "   Not applied because pragma was applied before\n");
        }
        return false;
      }
      Count = PI->getPragmaUnrollCount();
      if (Count == 1) {
        DEBUG(dbgs() << "  NOT UNROLL : #pragma unroll requires no unroll !\n");

        if (SaveLog) {
          amdhook->LLVMBuildLog->append(
              "    Not unrolled because pragma requests no unroll\n");
        }
        
        return false;
      }
      Threshold = NoThreshold;
      PI->setHasPragmaUnrollCountUsed(1);
    }
    else {
      // Only show log if pragma is present
      SaveLog = false;
    }
  }
  if (!SE->getSmallConstantTripCount(L, LatchBlock)) Count = 0;
#endif

  if (Count == 0) {
    // Conservative heuristic: if we know the trip count, see if we can
    // completely unroll (subject to the threshold, checked below); otherwise
    // try to find greatest modulo of the trip count which is still under
    // threshold value.
    if (TripCount == 0) {
#if defined(AMD_OPENCL) || 1
      if (SaveLog) {
        amdhook->LLVMBuildLog->append(
            "    Not unrolled because its trip count is unknown!\n");
      }
#endif
      return false;
    }
    Count = TripCount;
  }

  // Enforce the threshold.
  if (Threshold != NoThreshold) {
#if defined(AMD_OPENCL) || 1
    bool found_alloca = ContainsAllocaCandidate(L);
    if (found_alloca) Threshold = LUScratchThreshold;
#endif
    const DataLayout *TD = getAnalysisIfAvailable<DataLayout>();
    unsigned NumInlineCandidates;
    unsigned LoopSize = ApproximateLoopSize(L, NumInlineCandidates, TD);
    DEBUG(dbgs() << "  Loop Size = " << LoopSize << "\n");
    if (NumInlineCandidates != 0
#if defined(AMD_OPENCL) || 1
      && !found_alloca     //If we found an alloca candidate then we will 
						   //consider the loop even if it's got a call in it.
#endif
     ) {
      DEBUG(dbgs() << "  Not unrolling loop with inlinable calls.\n");

#if defined(AMD_OPENCL) || 1
      if (SaveLog) {
        amdhook->LLVMBuildLog->append(
            "    Not unrolled because it has function calls!\n");
      }
#endif

      return false;
    }
    uint64_t Size = (uint64_t)LoopSize*Count;
    if (TripCount != 1 && Size > Threshold) {
      DEBUG(dbgs() << "  Too large to fully unroll with count: " << Count
            << " because size: " << Size << ">" << Threshold << "\n");
      if (!CurrentAllowPartial && !(UnrollRuntime && TripCount == 0)) {
        DEBUG(dbgs() << "  will not try to unroll partially because "
              << "-unroll-allow-partial not given\n");
        return false;
      }
      if (TripCount) {
        // Reduce unroll count to be modulo of TripCount for partial unrolling
        Count = Threshold / LoopSize;
        while (Count != 0 && TripCount%Count != 0)
          Count--;
      }
      else if (UnrollRuntime) {
        // Reduce unroll count to be a lower power-of-two value
        while (Count != 0 && Size > Threshold) {
          Count >>= 1;
          Size = LoopSize*Count;
        }
      }
      if (Count < 2) {
        DEBUG(dbgs() << "  could not unroll partially\n");
        return false;
      }
      DEBUG(dbgs() << "  partially unrolling with count: " << Count << "\n");
    }
  }

  // Unroll the loop.
  if (!UnrollLoop(L, Count, TripCount, UnrollRuntime, TripMultiple, LI, &LPM)) {
#if defined(AMD_OPENCL) || 1
      if (SaveLog) {
        amdhook->LLVMBuildLog->append(
            "    Not unrolled, possible due to some errors!\n");
      }
#endif
    return false;
  }

#if defined(AMD_OPENCL) || 1
  if (SaveLog) {
    amdhook->LLVMBuildLog->append( "    Unrolled as requested!\n");
  }
#endif

  return true;
}
