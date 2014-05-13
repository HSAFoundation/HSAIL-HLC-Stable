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

#define DEBUG_TYPE "amdilinline"
#include "AMDIL.h"
#include "AMDILCompilerErrors.h"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILSubtarget.h"
#include "llvm/Instructions.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Function.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/DataLayout.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;

namespace {
class LLVM_LIBRARY_VISIBILITY AMDILInlinePass: public FunctionPass {
public:
  TargetMachine &TM;
  static char ID;
  AMDILInlinePass(TargetMachine &TM, CodeGenOpt::Level OL);
  ~AMDILInlinePass();
  virtual const char *getPassName() const;
  virtual bool runOnFunction(Function &F);
  bool doInitialization(Module &M);
  bool doFinalization(Module &M);
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;
private:
  typedef
  DenseMap<const ArrayType *, SmallVector<AllocaInst *, DEFAULT_VEC_SLOTS> >
  InlinedArrayAllocasTy;

  bool AMDILInlineCallIfPossible(CallSite CS,
      const DataLayout *DL,
      InlinedArrayAllocasTy &InlinedArrayAllocas);
  CodeGenOpt::Level OptLevel;
};
char AMDILInlinePass::ID = 0;
} // anonymous namespace


namespace llvm {
FunctionPass *createAMDILInlinePass(TargetMachine &TM,
    CodeGenOpt::Level OL) {
  return new AMDILInlinePass(TM, OL);
}
} // llvm namespace

AMDILInlinePass::AMDILInlinePass(TargetMachine &TM_,
    CodeGenOpt::Level OL)
  : FunctionPass(ID), TM(TM_) {
  OptLevel = OL;
}

AMDILInlinePass::~AMDILInlinePass() {

}


bool AMDILInlinePass::AMDILInlineCallIfPossible(
  CallSite CS,
    const DataLayout *DL,
    InlinedArrayAllocasTy &InlinedArrayAllocas) {
  Function *Callee = CS.getCalledFunction();
  Function *Caller = CS.getCaller();

  // Try to inline the function.  Get the list of static allocas that were
  // inlined.
  SmallVector<AllocaInst *, 16> StaticAllocas;
  InlineFunctionInfo IFI;

  if (!InlineFunction(CS, IFI)) {
    return false;
  }

  DEBUG(dbgs() << "<amdilinline> function " << Caller->getName()
      << ": inlined call to " << Callee->getName() << "\n");

  // If the inlined function had a higher stack protection level than the
  // calling function, then bump up the caller's stack protection level.
  if (Callee->getFnAttributes().hasAttribute(Attributes::StackProtectReq)) {
    Caller->addFnAttr(Attributes::StackProtectReq);
  } else if (Callee->getFnAttributes().hasAttribute(Attributes::StackProtect) &&
      !Caller->getFnAttributes().hasAttribute(Attributes::StackProtectReq)) {
    Caller->addFnAttr(Attributes::StackProtect);
  }

  // Look at all of the allocas that we inlined through this call site.  If we
  // have already inlined other allocas through other calls into this function,
  // then we know that they have disjoint lifetimes and that we can merge them.
  //
  // There are many heuristics possible for merging these allocas, and the
  // different options have different tradeoffs.  One thing that we *really*
  // don't want to hurt is SRoA: once inlining happens, often allocas are no
  // longer address taken and so they can be promoted.
  //
  // Our "solution" for that is to only merge allocas whose outermost type is an
  // array type.  These are usually not promoted because someone is using a
  // variable index into them.  These are also often the most important ones to
  // merge.
  //
  // A better solution would be to have real memory lifetime markers in the IR
  // and not have the inliner do any merging of allocas at all.  This would
  // allow the backend to do proper stack slot coloring of all allocas that
  // *actually make it to the backend*, which is really what we want.
  //
  // Because we don't have this information, we do this simple and useful hack.
  //
  SmallPtrSet<AllocaInst *, 16> UsedAllocas;

  // Loop over all the allocas we have so far and see if they can be merged with
  // a previously inlined alloca.  If not, remember that we had it.

  for (unsigned AllocaNo = 0, NAlloca = IFI.StaticAllocas.size();
      AllocaNo != NAlloca; ++AllocaNo) {
    AllocaInst *AI = IFI.StaticAllocas[AllocaNo];

    // Don't bother trying to merge array allocations (they will usually be
    // canonicalized to be an allocation *of* an array), or allocations whose
    // type is not itself an array (because we're afraid of pessimizing SRoA).
    const ArrayType *ATy = dyn_cast<ArrayType>(AI->getAllocatedType());

    if (ATy == 0 || AI->isArrayAllocation()) {
      continue;
    }

    // Get the list of all available allocas for this array type.
    SmallVector<AllocaInst *, DEFAULT_VEC_SLOTS> &AllocasForType
      = InlinedArrayAllocas[ATy];

    // Loop over the allocas in AllocasForType to see if we can reuse one.  Note
    // that we have to be careful not to reuse the same "available" alloca for
    // multiple different allocas that we just inlined, we use the 'UsedAllocas'
    // set to keep track of which "available" allocas are being used by this
    // function.  Also, AllocasForType can be empty of course!
    bool MergedAwayAlloca = false;

    for (unsigned I = 0, E = AllocasForType.size(); I != E; ++I) {
      AllocaInst *AvailableAlloca = AllocasForType[I];

      // The available alloca has to be in the right function, not in some other
      // function in this SCC.
      if (AvailableAlloca->getParent() != AI->getParent()) {
        continue;
      }

      // If the inlined function already uses this alloca then we can't reuse
      // it.
      if (!UsedAllocas.insert(AvailableAlloca)) {
        continue;
      }

      // Otherwise, we *can* reuse it, RAUW AI into AvailableAlloca and declare
      // success!
      DEBUG(dbgs() << "    ***MERGED ALLOCA: " << *AI);

      AI->replaceAllUsesWith(AvailableAlloca);
      AI->eraseFromParent();
      MergedAwayAlloca = true;
      break;
    }

    // If we already nuked the alloca, we're done with it.
    if (MergedAwayAlloca) {
      continue;
    }

    // If we were unable to merge away the alloca either because there are no
    // allocas of the right type available or because we reused them all
    // already, remember that this alloca came from an inlined function and mark
    // it used so we don't reuse it for other allocas from this inline
    // operation.
    AllocasForType.push_back(AI);
    UsedAllocas.insert(AI);
  }

  return true;
}

bool AMDILInlinePass::runOnFunction(Function &MF) {
  Function *F = &MF;
  const AMDILSubtarget &STM = TM.getSubtarget<AMDILSubtarget>();

  const DataLayout *DL = getAnalysisIfAvailable<DataLayout>();

  SmallVector<CallSite, 16> CallSites;

  for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
    for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
      CallSite CS = CallSite(cast<Value>(I));

      // If this isn't a call, or it is a call to an intrinsic, it can
      // never be inlined.
      if (CS.getInstruction() == 0 || isa<IntrinsicInst>(I)) {
        continue;
      }

      // If this is a direct call to an external function, we can never inline
      // it.  If it is an indirect call, inlining may resolve it to be a
      // direct call, so we keep it.
      if (CS.getCalledFunction() && CS.getCalledFunction()->isDeclaration()) {
        continue;
      }

      // We don't want to inline if we are recursive.
      if (CS.getCalledFunction()
          && CS.getCalledFunction()->getName() == MF.getName()) {
        AMDILMachineFunctionInfo *MFI =
          getAnalysis<MachineFunctionAnalysis>().getMF()
          .getInfo<AMDILMachineFunctionInfo>();
        MFI->addErrorMsg(amd::CompilerErrorMessage[RECURSIVE_FUNCTION]);
        continue;
      }

      // Support function call in amdil.
      if (CS.getCaller() &&
          !CS.getCaller()->getFnAttributes().hasAttribute(Attributes::NoInline) &&
          CS.getCalledFunction() && CS.isNoInline()) {
        DEBUG_WITH_TYPE("noinline",
          llvm::dbgs() << "[AMDILInlinePass::runOnFunction] "
            << CS.getCalledFunction()->getName()
            << " (noinline) is called by"
            << MF.getName() << " (not noinline). "
            << " Do not inline.\n");
        continue;
      }

      CallSites.push_back(CS);
    }
  }

  InlinedArrayAllocasTy InlinedArrayAllocas;
  bool Changed = false;

  for (unsigned CSi = 0; CSi != CallSites.size(); ++CSi) {
    CallSite CS = CallSites[CSi];
    Function *Callee = CS.getCalledFunction();

    // We can only inline direct calls to non-declarations.
    if (Callee == 0 || Callee->isDeclaration()) {
      continue;
    }

    // Attempt to inline the function...
    if (!AMDILInlineCallIfPossible(CS, DL, InlinedArrayAllocas)) {
      continue;
    }

    Changed = true;
  }

  return Changed;
}

const char *AMDILInlinePass::getPassName() const {
  return "AMDIL Inline Function Pass";
}

bool AMDILInlinePass::doInitialization(Module &M) {
  return false;
}

bool AMDILInlinePass::doFinalization(Module &M) {
  return false;
}

void AMDILInlinePass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<MachineFunctionAnalysis>();
  FunctionPass::getAnalysisUsage(AU);
  AU.setPreservesAll();
}
