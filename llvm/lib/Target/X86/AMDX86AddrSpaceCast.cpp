
//===-- AMDX86AddrSpaceCast.cpp - Optimize and Lower AddrSpaceCast  ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// \file
// \brief This file implements a FunctionPass to handle generic address space
// builtins.
//
//===----------------------------------------------------------------------===//

#include "X86.h"


#include "llvm/Function.h"
#include "llvm/Instructions.h"

using namespace llvm;

namespace {

class AMDX86AddrSpaceCast : public FunctionPass {
public:
  static char ID;
  // TODO: access CLK_GLOBAL_MEM_FENCE.
  static const int G_FENCE = (1 << 0);

  explicit AMDX86AddrSpaceCast() : FunctionPass(ID) {}

  virtual bool runOnFunction(Function &F) LLVM_OVERRIDE;
  virtual const char *getPassName() const LLVM_OVERRIDE {
    return "Lower Generic Address Builtins";
  }

private:
  bool handleGenericAddrBuiltins(CallInst *Call);
};
} // End anonymous namespace.

char AMDX86AddrSpaceCast::ID = 0;

namespace llvm {
void initializeAMDX86AddrSpaceCastPass(PassRegistry &);
}
INITIALIZE_PASS(AMDX86AddrSpaceCast, "x86-lower-generic-address-builtins",
                "Lower Generic Address Builtins", false, false);

/// Create X86AddrSpaceCast object.
FunctionPass *llvm::createAMDX86AddrSpaceCastPass() {
  return new AMDX86AddrSpaceCast();
}

/// Handle builtins: to_global, to_local, to_private and genfence.
bool AMDX86AddrSpaceCast::handleGenericAddrBuiltins(CallInst *Call) {
  Function *Func = Call->getCalledFunction();
  if (!Func) {
    return false;
  }
  StringRef FuncName = Func->getName();

  // Handle get_fence.
  if (FuncName.startswith("_Z9get_fence")) {
     // Replace the Call with CLK_GLOBAL_MEM_FENCE.
     // TODO: Should we always return CLK_GLOBAL_MEM_FENCE? What if the argument
     // is of null pointer?
    Call->replaceAllUsesWith(ConstantInt::get(Call->getType(), G_FENCE, false));
  } else if (FuncName.startswith("_Z9to_global") ||
             FuncName.startswith("_Z8to_local")  ||
             FuncName.startswith("_Z10to_private")) {
    Call->replaceAllUsesWith(new BitCastInst(Call->getArgOperand(0),
                                             Call->getType(), "", Call));
  } else {
    return false;
  }

  Call->eraseFromParent();
  return true;
}

/// Driver to lower generic address builtins.
bool AMDX86AddrSpaceCast::runOnFunction(Function &F) {
  bool Changed = false;
  for (Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B) {
    for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE;) {
      Instruction *CurInst = I++;
      if (CallInst *Call = dyn_cast<CallInst>(CurInst)) {
        Changed |= handleGenericAddrBuiltins(Call);
      }
    }
  }

  return Changed;
}
