//===-- AMDPeepholeOpt.cpp - AMD OpenCL Peephole Optimization pass --==//
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

#define DEBUG_TYPE "amd-peephole"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/GlobalVariable.h"
#include "llvm/InitializePasses.h"
#include "llvm/Instruction.h"
#include "llvm/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

// TODO: Use the common header when all address space definitions are
// merged into a common one.
enum {
  PRIVATE_ADDRESS  = 0, // Address space for private memory.
};

// The AMD OpenCL peephole optimization pass is used to do simple peephole
// optimizations that are required for correct code or to simplify code.
// These optimizations are taken out of AMDILPeepholeOpt into its own pass
// so that it can be called before target specific analysis info is created
// so that other optimization passes (such SCCP and CFGSimplification) can be
// called to optimize on the resulting IR of this pass.
// If these optimizations were in AMDILPeepholeOpt, which is done after target
// specific analysises (such as MachineFunctionAnalysis) are created, then a
// lot of optimizations passes cannot be called after this, as they will
// invalidate the target specific analysises.
// Currently this pass does two peephole optimizations:
// 1. If a sampler index is loaded from a global variable, take the value of
// the global variable as the sampler index and propagate it.
// 2. Replace __amdil_is_constant() calls with constants.
namespace {
class LLVM_LIBRARY_VISIBILITY AMDPeepholeOpt : public FunctionPass {
public:
  static char ID;
private:
  SmallVector<CallInst *, 16> isConstVec;
  unsigned mOptLevel;

public:
  AMDPeepholeOpt(unsigned OL = 0);
  ~AMDPeepholeOpt() {}
  const char *getPassName() const;
  bool runOnFunction(Function &F);
  void getAnalysisUsage(AnalysisUsage &AU) const;

private:
  // Function to initiate all of the instruction level optimizations.
  // Returns true if the iterator from the parent loop should not be moved,
  // and false otherwise. This function can only be called from a loop over
  // all instructions.
  bool instLevelOptimizations(Instruction *Inst);

  // Because __amdil_is_constant cannot be properly evaluated if
  // optimizations are disabled, the call's are placed in a vector
  // and evaluated after the __amdil_image* functions are evaluated
  // which should allow the __amdil_is_constant function to be
  // evaluated correctly.
  bool doIsConstCallConversion();

  // Run a series of tests to see if we can optimize a CALL instruction.
  bool optimizeCallInst(Instruction *Inst);

  // Replace a function call with a constant 0 / 1
  void replaceCallWithConstant(CallInst *CI, bool Value);

  // If we are in no opt mode, then we need to make sure that
  // local samplers are properly propagated as constant propagation
  // doesn't occur and we need to know the value of kernel defined
  // samplers at compile time.
  bool propagateSamplerInst(CallInst *CI);

}; // class AMDPeepholeOpt
} // anonymous namespace

char AMDPeepholeOpt::ID = 0;
INITIALIZE_PASS(AMDPeepholeOpt, "amdpeephole",
                "AMD OpenCL Peephole Optimization", false, false);

FunctionPass *llvm::createAMDPeepholeOpt(unsigned OL) {
  return new AMDPeepholeOpt(OL);
}

AMDPeepholeOpt::AMDPeepholeOpt(unsigned OL)
  : FunctionPass(ID), isConstVec(), mOptLevel(OL) {
  initializeAMDPeepholeOptPass(*PassRegistry::getPassRegistry());
}

const char *AMDPeepholeOpt::getPassName() const {
  return "AMD OpenCL Peephole Optimization";
}

bool AMDPeepholeOpt::doIsConstCallConversion() {
  if (isConstVec.empty()) {
    return false;
  }

  for (unsigned X = 0, Y = isConstVec.size(); X < Y; ++X) {
    CallInst *CI = isConstVec[X];
    Constant *CV = dyn_cast<Constant>(CI->getOperand(0));
    replaceCallWithConstant(CI, (CV != NULL));
  }

  isConstVec.clear();
  return true;
}

bool AMDPeepholeOpt::runOnFunction(Function &MF) {
  bool Changed = false;
  DEBUG(MF.dump());

  for (inst_iterator I = inst_end(MF), E = inst_begin(MF); I != E;) {
    inst_iterator nextI = I;
    Instruction *Inst = &*(--nextI);

    // If we don't optimize to a new instruction, decrement the
    // iterator, otherwise test the new instruction for further
    // optimizations.
    if (instLevelOptimizations(Inst)) {
      Changed = true;
      // We have to check against inst_begin at each iteration of the
      // loop as it can be invalidated and 'I' can point to the first
      // instruction.
      E = inst_begin(MF);

      if (I == E) {
        break;
      }
    } else {
      --I;
    }
  }

  Changed |= doIsConstCallConversion();

  DEBUG(MF.dump());

  return Changed;
}

bool AMDPeepholeOpt::optimizeCallInst(Instruction *Inst) {
  CallInst *CI = cast<CallInst>(Inst);

  if (propagateSamplerInst(CI)) {
    return true;
  }

  Function *F = CI->getCalledFunction();
  if (!F) {
    return false;
  }

  StringRef FuncName = F->getName();

  if (FuncName.startswith("__amdil_is_constant")) {
    // If we do not have optimizations, then this
    // cannot be properly evaluated, so we add the
    // call instruction to a vector and process
    // them at the end of processing after the
    // samplers have been correctly handled.
    isConstVec.push_back(CI);
    return false;
  }

  return false;
}

bool AMDPeepholeOpt::instLevelOptimizations(Instruction *Inst) {
  assert(Inst && "inst should not be NULL");

  if ((Inst->getOpcode() == Instruction::Call) && optimizeCallInst(Inst)) {
    return true;
  }

  return false;
}

void AMDPeepholeOpt::replaceCallWithConstant(CallInst *CI, bool Value) {
  IntegerType *BoolType = IntegerType::getInt32Ty(CI->getContext());
  ConstantInt *ConstVal = ConstantInt::get(BoolType, Value);

  CI->replaceAllUsesWith(ConstVal);
  CI->eraseFromParent();
}

bool AMDPeepholeOpt::propagateSamplerInst(CallInst *CI) {
  assert(CI && "Inst should not be NULL");

  Function *Func = CI->getCalledFunction();
  if (!Func) {
    return false;
  }

  StringRef CalleeName = Func->getName();
  if (CalleeName != "__amdil_image1d_read_norm"
      && CalleeName != "__amdil_image1d_read_unnorm"
      && CalleeName != "__amdil_image1d_array_read_norm"
      && CalleeName != "__amdil_image1d_array_read_unnorm"
      && CalleeName != "__amdil_image1d_buffer_read_norm"
      && CalleeName != "__amdil_image1d_buffer_read_unnorm"
      && CalleeName != "__amdil_image2d_read_norm"
      && CalleeName != "__amdil_image2d_read_unnorm"
      && CalleeName != "__amdil_image2d_array_read_norm"
      && CalleeName != "__amdil_image2d_array_read_unnorm"
      && CalleeName != "__amdil_image3d_read_norm"
      && CalleeName != "__amdil_image3d_read_unnorm") {
    return false;
  }

  unsigned SamplerIdx = 1;
  Value *Sampler = CI->getOperand(SamplerIdx);
  LoadInst *LInst = dyn_cast<LoadInst>(Sampler);

  if (!LInst) {
    return false;
  }

  if (LInst->getPointerAddressSpace() != PRIVATE_ADDRESS) {
    return false;
  }

  GlobalVariable *GV
    = dyn_cast<GlobalVariable>(LInst->getPointerOperand());

  // If we are loading from what is not a global value, then we
  // fail and return.
  if (!GV) {
    return false;
  }

  // If we don't have an initializer or we have an initializer and
  // the initializer is not a 32bit integer, we fail.
  if (!GV->hasInitializer()
      || !GV->getInitializer()->getType()->isIntegerTy(32)) {
    return false;
  }

  // Now that we have the global variable initializer, lets replace
  // all uses of the load instruction with the samplerVal and
  // reparse the __amdil_is_constant() function.
  Constant *SamplerVal = GV->getInitializer();
  LInst->replaceAllUsesWith(SamplerVal);
  return true;
}

void AMDPeepholeOpt::getAnalysisUsage(AnalysisUsage &AU) const {
  FunctionPass::getAnalysisUsage(AU);
  AU.setPreservesAll();
}
