//===-- AMDILPeepholeOpt.cpp - Peephole Optimization pass --===//
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

#define DEBUG_TYPE "peepholeopt"

#include "AMDILAlgorithms.tpp"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILModuleInfo.h"
#include "AMDILUtilityFunctions.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Instructions.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Transforms/Utils/Local.h"

STATISTIC(LocalFuncs,
    "Number of get_local_size(N) functions removed");

using namespace llvm;

// The Peephole optimization pass is used to do simple last minute
// optimizations that are required for correct code or to remove
// redundant functions
namespace {
class LLVM_LIBRARY_VISIBILITY AMDILPeepholeOpt : public FunctionPass {
public:
  const TargetMachine &TM;
  static char ID;
private:
  LLVMContext *mCTX;
  Function *mF;
  const AMDILSubtarget *mSTM;
  AMDILModuleInfo *mAMI;
  CodeGenOpt::Level mOptLevel;
  bool mChanged;
  bool mRWGOpt;
  unsigned EnableBFO : 1;

public:
  AMDILPeepholeOpt();
  AMDILPeepholeOpt(TargetMachine &TM, CodeGenOpt::Level OL);
  ~AMDILPeepholeOpt();
  const char *getPassName() const;
  bool runOnFunction(Function &F);
  bool doInitialization(Module &M);
  bool doFinalization(Module &M);
  void getAnalysisUsage(AnalysisUsage &AU) const;

private:
  // Function to initiate all of the instruction level optimizations.
  // Returns true if the iterator from the parent loop should not be moved, and false
  // otherwise. This function can only be called from a loop over all instructions.
  bool instLevelOptimizations(Instruction *Inst);

  // Run a series of tests to see if we can optimize a CALL
  // instruction.
  bool optimizeCallInst(Instruction *Inst);

  // A peephole optimization to optimize bit extract sequences.
  bool optimizeBitExtract(Instruction *Inst);

  // A peephole optimization that does the following transform:
  // ((((B ^ -1) | C) & A) ^ -1)
  // ==>
  // BFI(A, (B & (C ^ -1)), -1)
  bool optimizeBFI(Instruction *Inst);

  // A peephole optimization to optimize [d]class calls that or the
  // results.
  bool optimizeClassInst(Instruction *Inst);

  // Generate F2U4 intrinisic
  bool genIntrF2U4(Instruction *Inst);

  // On 8XX operations, we do not have 24 bit signed
  // operations. So in this case we need to expand them. These
  // functions check for 24bit functions and then expand.
  bool isSigned24BitOps(CallInst *CI);
  void expandSigned24BitOps(CallInst *CI);

  // One optimization that can occur is that if the required workgroup
  // size is specified then the result of get_local_size is known at
  // compile time and can be returned accordingly.
  bool isRWGLocalOpt(CallInst *CI);
  void expandRWGLocalOpt(CallInst *CI);

  // On Northern Island cards, the division is slightly less accurate
  // than on previous generations, so we need to utilize a more
  // accurate division. So we can translate the accurate divide to a
  // normal divide on all other cards.
  bool convertAccurateDivide(CallInst *CI);
  void expandAccurateDivide(CallInst *CI);

  // Replace a function call with a constant 0 / 1
  void replaceCallWithConstant(CallInst *CI, bool Value);

  // If the alignment is set incorrectly, it can produce really
  // inefficient code. This checks for this scenario and fixes it if
  // possible.
  bool correctMisalignedMemOp(Instruction *Inst);

  // Replace call with offset into reserved LDS/GDS
  void replaceReserveSharedCall(CallInst *CI, bool Region);

  // Since the GWS hardware instruction has the region size encoded in
  // it, we need to use an alternate implementation if we don't know
  // the region size at compile time.
  bool selectRegionBarrierImpl(CallInst *CI);
}; // class AMDILPeepholeOpt
} // anonymous namespace

char AMDILPeepholeOpt::ID = 0;
INITIALIZE_PASS(AMDILPeepholeOpt, "amdilpeephole",
                "AMDIL Peephole Optimization", false, false);

/*
   getMaskBitfield() returns true if 'val' is a mask, which is defined
   to be a value whose 1 bits (in binary format) are all next to each
   other.  And 'start_bit' set to the first 1 bit and 'bitwidth' sets
   to the width of all the 1-bits (the number of 1 bits). Bit number
   always starts from 0.

   For example:
   given that val = 0xFF00; start_bit = 8 and bitwidth = 8.
*/
bool getMaskBitfield(unsigned Val, unsigned &StartBit, unsigned &BitWidth) {
  if (Val == 0) {
    // zero, no bitfield
    return false;
  }

  BitWidth = 0;
  StartBit = 0;

  while ((Val & 1) == 0) {
    ++StartBit;
    Val = (Val >> 1);
  }

  if (Val > 0) {
    while ((Val & 1)  == 1) {
      ++BitWidth;
      Val = (Val >> 1);
    }
  }

  if (Val > 0) {
    // non-continguous 1 bits.
    return false;
  }

  return true;
}

static bool getIntValue(Instruction *Inst, Value *& Src, unsigned &SrcStart,
    unsigned &DstStart, unsigned &DstWidth) {
  Value *IntVal, *Opnd1;
  bool HasMask = false;

  if (!Inst->getType()->isIntegerTy(32))
    return false;

  IntVal = Inst;

  unsigned StartPos = 0;
  unsigned NBits = 32;

  if (Inst->getOpcode() == Instruction::And) {
    IntVal = Inst->getOperand(0);
    Opnd1 = Inst->getOperand(1);

    ConstantInt *CI0 = dyn_cast<ConstantInt>(IntVal);
    ConstantInt *CI1 = dyn_cast<ConstantInt>(Opnd1);

    if ((!CI0 && !CI1) || (CI0 && CI1)) {
      return false;
    }

    if (CI0) {
      Value *Tmp = IntVal;
      IntVal = Opnd1;
      Opnd1 = Tmp;
      CI1 = CI0;
    }

    unsigned mask = CI1->getZExtValue();
    HasMask = getMaskBitfield(mask, StartPos, NBits);

    if (!HasMask) {
      return false;
    }
  }

  Instruction *TInst = dyn_cast<Instruction>(IntVal);

  if (!TInst) {
    return false;
  }

  unsigned SrcPos = StartPos;

  if (TInst->getOpcode() == Instruction::Shl) {
    ConstantInt *CI = dyn_cast<ConstantInt>(TInst->getOperand(1));

    if (HasMask && CI) {
      unsigned Amt = CI->getZExtValue();

      if (Amt > SrcPos) {
        return false;
      }

      SrcPos -= Amt;
    } else if (!HasMask && CI) {
      unsigned Amt = CI->getZExtValue();
      Instruction *TInst1 = dyn_cast<Instruction>(TInst->getOperand(0));

      if (TInst1 && TInst1->getOpcode() == Instruction::LShr) {
        //  {shl; Lshr} pattern
        ConstantInt *CI1 = dyn_cast<ConstantInt>(TInst1->getOperand(1));

        if (CI1) {
          unsigned Amt1 = CI1->getZExtValue();

          if (Amt >= Amt1) {
            StartPos = Amt - Amt1;
            SrcPos   = 0;
            NBits = 32 - Amt;

            IntVal = TInst1->getOperand(0);
          } else {
            return false;
          }
        }
      } else if (Amt < 32) {
        // Only shl
        StartPos = Amt;
        SrcPos   = 0;
        NBits = 32 - Amt;

        IntVal = TInst->getOperand(0);
      }
    } else {
      return false;
    }

    IntVal = TInst->getOperand(0);
  }

  Src = IntVal;
  SrcStart = SrcPos;
  DstStart = StartPos;
  DstWidth = NBits;
  return true;
}

namespace llvm {
FunctionPass *createAMDILPeepholeOpt(TargetMachine &TM,
                                     CodeGenOpt::Level OL) {
  return new AMDILPeepholeOpt(TM, OL);
}
} // llvm namespace

AMDILPeepholeOpt::AMDILPeepholeOpt()
  : FunctionPass(ID),
    TM(getAnalysis<MachineFunctionAnalysis>().getMF().getTarget()),
    mCTX(NULL),
    mF(NULL),
    mSTM(NULL),
    mAMI(NULL),
    mOptLevel(CodeGenOpt::None),
    mChanged(false),
    mRWGOpt(false),
    EnableBFO(TM.Options.EnableBFO) {
  initializeAMDILPeepholeOptPass(*PassRegistry::getPassRegistry());
}

AMDILPeepholeOpt::AMDILPeepholeOpt(TargetMachine &tm,
                                   CodeGenOpt::Level OL)
  : FunctionPass(ID),
    TM(tm),
    mCTX(NULL),
    mF(NULL),
    mSTM(NULL),
    mAMI(NULL),
    mOptLevel(OL),
    mChanged(false),
    mRWGOpt(false),
    EnableBFO(tm.Options.EnableBFO) {
  initializeAMDILPeepholeOptPass(*PassRegistry::getPassRegistry());
}

AMDILPeepholeOpt::~AMDILPeepholeOpt() {
}

const char *AMDILPeepholeOpt::getPassName() const {
  return "AMDIL PeepHole Optimization Pass";
}

bool AMDILPeepholeOpt::runOnFunction(Function &MF) {
  mChanged = false;
  mF = &MF;
  mSTM = &TM.getSubtarget<AMDILSubtarget>();
  DEBUG(MF.dump());
  mCTX = &MF.getType()->getContext();

  mAMI = &(getAnalysis<MachineFunctionAnalysis>().getMF()
      .getMMI().getObjFileInfo<AMDILModuleInfo>());
  const AMDILKernel *Kernel = mAMI->getKernel(MF.getName());

  if (Kernel && Kernel->mKernel && Kernel->sgv) {
    mRWGOpt = Kernel->sgv->mHasRWG;
  }

  for (inst_iterator I = inst_end(MF), E = inst_begin(MF); I != E;) {
    inst_iterator nextI = I;
    Instruction *Inst = &*(--nextI);

    // If we don't optimize to a new instruction, decrement the
    // iterator, otherwise test the new instruction for further
    // optimizations.
    if (instLevelOptimizations(Inst)) {
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

  DEBUG(MF.dump());

  return mChanged;
}

bool AMDILPeepholeOpt::optimizeCallInst(Instruction *Inst) {
  CallInst *CI = cast<CallInst>(Inst);

  if (isSigned24BitOps(CI)) {
    expandSigned24BitOps(CI);
    CI->eraseFromParent();
    return true;
  }

  if (isRWGLocalOpt(CI)) {
    expandRWGLocalOpt(CI);
    CI->eraseFromParent();
    return true;
  }

  if (convertAccurateDivide(CI)) {
    expandAccurateDivide(CI);
    CI->eraseFromParent();
    return true;
  }

  if (selectRegionBarrierImpl(CI)) {
    return true;
  }

  Function *F = CI->getCalledFunction();
  if (!F) {
    return false;
  }

  StringRef FuncName = F->getName();

  if (FuncName.startswith("__atom") && !CI->getNumUses()
      && !FuncName.startswith("__atomic_load")
      && !FuncName.startswith("__atomic_store")
      && FuncName.find("_xchg") == StringRef::npos
      && FuncName.find("_noret") == StringRef::npos) {
    std::string Buffer(FuncName.str() + "_noret");
    SmallVector<Type *, 4> CallTypes;
    FunctionType *Ptr = F->getFunctionType();
    CallTypes.insert(CallTypes.begin(),
                     Ptr->param_begin(),
                     Ptr->param_end());
    FunctionType *NewFunc
      = FunctionType::get(Type::getVoidTy(F->getContext()), CallTypes, false);
    SmallVector<Value *, 4> Args;

    for (unsigned X = 0, Y = CI->getNumArgOperands(); X < Y; ++X) {
      Args.push_back(CI->getArgOperand(X));
    }

    Function *NewF = dyn_cast<Function>(
        F->getParent()->getOrInsertFunction(Buffer, NewFunc));
    CallInst *NewCI = CallInst::Create(NewF, Args);
    NewCI->insertAfter(CI);
    CI->eraseFromParent();
    return true;
  }

  if (!mSTM->isSupported(AMDIL::Caps::ArenaSegment) &&
      !mSTM->isSupported(AMDIL::Caps::MultiUAV)) {
    return false;
  }

  return false;
}

bool AMDILPeepholeOpt::optimizeClassInst(Instruction *Inst) {
  assert(Inst && (Inst->getOpcode() == Instruction::Or) &&
      "optimizeClassInst() expects OR instruction");

  if (mOptLevel == CodeGenOpt::None) {
    return false;
  }

  // We want to optimize multiple __amdil_class_f[32|64] that are
  // seperated by 'or' instructions into a single call with the
  // second argument or'd together.
  CallInst *LHS = dyn_cast<CallInst>(Inst->getOperand(0));
  CallInst *RHS = dyn_cast<CallInst>(Inst->getOperand(1));

  if (!LHS || !RHS) {
    return false;
  }

  Function *LHSFunc = LHS->getCalledFunction();
  Value *LHSConst = dyn_cast<Constant>(LHS->getOperand(1));
  Value *LHSVar = LHS->getOperand(0);

  Function *RHSFunc = RHS->getCalledFunction();
  Value *RHSConst = dyn_cast<Constant>(RHS->getOperand(1));
  Value *RHSVar = RHS->getOperand(0);

  if (!LHSFunc || !RHSFunc) {
    return false;
  }

  // If the functions aren't the class intrinsic, then fail.
  // If the names are not the same, then fail.
  if ((!LHSFunc->getName().startswith("__amdil_class_f")
      || !RHSFunc->getName().startswith("__amdil_class_f"))
      || LHSFunc->getName() != RHSFunc->getName()) {
    return false;
  }

  // We don't want to merge two class calls from different variables.
  if (LHSVar != RHSVar) {
    return false;
  }

  // If we don't have two constants, then fail.
  if (!LHSConst || !RHSConst) {
    return false;
  }

  Value *Operands[2] = {
    LHSVar,
    LHSConst
  };

  CallInst *NewCall = CallInst::Create(LHSFunc, Operands, "new_class");
  // Or the constants together, and then call the function all over
  // again.
  Inst->setOperand(0, LHSConst);
  Inst->setOperand(1, RHSConst);
  Inst->replaceAllUsesWith(NewCall);
  NewCall->insertAfter(Inst);
  NewCall->setOperand(1, Inst);

  // We need to remove the functions if they only have a single
  // use.
  if (LHS->use_empty()) {
    LHS->eraseFromParent();
  }

  if (RHS->use_empty()) {
    RHS->eraseFromParent();
  }

  return true;
}

bool AMDILPeepholeOpt::optimizeBFI(Instruction *Inst) {
  assert(Inst && (Inst->getOpcode() == Instruction::Xor) &&
      "optimizeBitExtract() expects Xor instruction");

  DEBUG(dbgs() << "\nInst: "; Inst->dump());

  if (mOptLevel == CodeGenOpt::None) {
    return false;
  }

  Type *AType = Inst->getType();

  // This optimization only works on 32-bit integers.
  if (!AType->getScalarType()->isIntegerTy(32))
    return false;

  int NumEle = 1;

  if (AType->isVectorTy()) {
    NumEle = dyn_cast<VectorType>(AType)->getNumElements();

    if (NumEle >= 3) {
      return false;
    }
  }

  // The optimization we are doing is:
  // B` = B ^ -1
  // C` = B` | C
  // A` = C` & A
  // inst = A` ^ -1
  // (((B` | C) & A) ^ -1)
  // ==>
  // BFI(A, (B & (C ^ -1)), -1)
  Constant *ApNeg1 = dyn_cast<Constant>(Inst->getOperand(1));
  Instruction *Ap = dyn_cast<Instruction>(Inst->getOperand(0));

  // Not a -1 or an 'AND' instruction, so can't proceed.
  if (!ApNeg1 || (Ap && Ap->getOpcode() != Instruction::And)) {
    // Inverted operands, swap them.
    ApNeg1 = dyn_cast<Constant>(Inst->getOperand(0));
    Ap = dyn_cast<Instruction>(Inst->getOperand(1));
  }

  if (!ApNeg1 || !Ap || Ap->getOpcode() != Instruction::And) {
    return false;
  }

  DEBUG(
    dbgs() << "Ap: ";
    Ap->dump();
    dbgs() << "Ap-1: ";
    ApNeg1->dump();
  );

  Instruction *Cp = dyn_cast<Instruction>(Ap->getOperand(0));
  Instruction *A = dyn_cast<Instruction>(Ap->getOperand(1));

  if (!Cp || !A) {
    return false;
  }

  DEBUG(
    dbgs() << "A: ";
    A->dump();
    dbgs() << "Cp: ";
    Cp->dump()
  );

  if (Cp->getOpcode() != Instruction::Or
      && A->getOpcode() == Instruction::Or) {
    // Operands are inverted, lets swap them.
    Cp = dyn_cast<Instruction>(Ap->getOperand(1));
    A = dyn_cast<Instruction>(Ap->getOperand(0));
  }

  if (Cp->getOpcode() != Instruction::Or) {
    // We don't have the right opcode.
    return false;
  }

  Instruction *Bp = dyn_cast<Instruction>(Cp->getOperand(0));
  Instruction *C = dyn_cast<Instruction>(Cp->getOperand(1));

  if (Bp || Bp->getOpcode() != Instruction::Xor) {
    // Operands are inverted, lets swap them.
    Bp = dyn_cast<Instruction>(Cp->getOperand(1));
    C = dyn_cast<Instruction>(Cp->getOperand(0));
  }

  if (!Bp || Bp->getOpcode() != Instruction::Xor) {
    return false;
  }

  DEBUG(
    dbgs() << "C: ";
    C->dump();
    dbgs() << "Bp: ";
    Bp->dump();
  );

  Constant *BpNeg1 = dyn_cast<Constant>(Bp->getOperand(1));
  Instruction *B = dyn_cast<Instruction>(Bp->getOperand(0));

  if (!B || !BpNeg1) {
    B = dyn_cast<Instruction>(Bp->getOperand(1));
    BpNeg1 = dyn_cast<Constant>(Bp->getOperand(0));
  }

  if (!B || !BpNeg1) {
    return false;
  }

  DEBUG(
    dbgs() << "B: ";
    B->dump();
    dbgs() << "Bp-1: ";
    BpNeg1->dump()
  );

  if (AType->isVectorTy()) {
    ConstantDataVector *BpNeg1v = dyn_cast<ConstantDataVector>(BpNeg1);
    ConstantDataVector *ApNeg1v = dyn_cast<ConstantDataVector>(ApNeg1);

    if (!BpNeg1v || !ApNeg1v) {
      return false;
    }

    for (size_t X = 0, Y = BpNeg1v->getNumElements(); X < Y; ++X) {
      ConstantInt *Neg1
      = dyn_cast<ConstantInt>(BpNeg1v->getElementAsConstant(X));

      if (!Neg1) {
        return false;
      }

      uint32_t MaskVal = (uint32_t)Neg1->getZExtValue();

      if (!isMask_32(MaskVal) || CountTrailingOnes_32(MaskVal) != 32) {
        return false;
      }
    }

    for (size_t X = 0, Y = ApNeg1v->getNumElements(); X < Y; ++X) {
      ConstantInt *Neg1
      = dyn_cast<ConstantInt>(ApNeg1v->getElementAsConstant(X));

      if (!Neg1) {
        return false;
      }

      uint32_t MaskVal = (uint32_t)Neg1->getZExtValue();

      if (!isMask_32(MaskVal) || CountTrailingOnes_32(MaskVal) != 32) {
        return false;
      }
    }
  } else {
    ConstantInt *BpNeg1i = dyn_cast<ConstantInt>(BpNeg1);
    ConstantInt *ApNeg1i = dyn_cast<ConstantInt>(ApNeg1);

    if (!BpNeg1i || !ApNeg1i) {
      return false;
    }

    uint32_t MaskVal = BpNeg1i->getZExtValue();

    if (!isMask_32(MaskVal) || CountTrailingOnes_32(MaskVal) != 32) {
      return false;
    }

    MaskVal = ApNeg1i->getZExtValue();

    if (!isMask_32(MaskVal) || CountTrailingOnes_32(MaskVal) != 32) {
      return false;
    }
  }

  DEBUG(dbgs() << "Creating pattern BFI(A, (B & (C ^ -1)), -1)\n");

  // Now that we have verified everything, lets create our result.
  SmallVector<Type *, 3> CallTypes;
  CallTypes.push_back(AType);
  CallTypes.push_back(AType);
  CallTypes.push_back(AType);
  FunctionType *FuncType = FunctionType::get(AType, CallTypes, false);
  std::string Name = "__amdil_bfi";

  if (AType->isVectorTy()) {
    Name += "_v" + itostr(NumEle) + "u32";
  } else {
    Name += "_u32";
  }

  Function *Func =
    dyn_cast<Function>(Inst->getParent()->getParent()->getParent()->
        getOrInsertFunction(llvm::StringRef(Name), FuncType));
  Func->addFnAttr(Attributes::ReadNone);
  C = BinaryOperator::Create(Instruction::Xor, C, BpNeg1, "bfiXor", Inst);
  B = BinaryOperator::Create(Instruction::And, B, C, "bfiAnd", Inst);
  Value *Operands[3] = {
    A,
    B,
    BpNeg1
  };
  CallInst *CI = CallInst::Create(Func, Operands, "BFI");
  DEBUG(
    dbgs() << "Old Inst: ";
    Inst->dump();
    dbgs() << "New Inst: ";
    CI->dump();
    dbgs() << "\n\n");
  CI->insertBefore(Inst);
  Inst->replaceAllUsesWith(CI);
  Inst->eraseFromParent();
  return true;
}

bool AMDILPeepholeOpt::optimizeBitExtract(Instruction *Inst) {
  assert(Inst && (Inst->getOpcode() == Instruction::And) &&
      "optimizeBitExtract() expects And instruction");

  if (mOptLevel == CodeGenOpt::None) {
    return false;
  }

  // We want to do some simple optimizations on Shift right/And
  // patterns. The basic optimization is to turn (A >> B) & C where A
  // is a 32-bit type, B is a value smaller than 32 and C is a mask. If
  // C is a constant value, then the following transformation can
  // occur. For signed integers, it turns into the function call dst =
  // __amdil_ibit_extract(log2(C), B, A) For unsigned integers, it
  // turns into the function call dst = __amdil_ubit_extract(log2(C),
  // B, A) The function __amdil_[u|i]bit_extract can be found in
  // Section 7.9 of the ATI IL spec of the stream SDK for Evergreen
  // hardware.
  Type *AType = Inst->getType();
  bool IsVector = AType->isVectorTy();
  int NumEle = 1;

  // This only works on 32-bit integers
  if (!AType->getScalarType()->isIntegerTy(32))
    return false;

  if (IsVector) {
    const VectorType *VT = dyn_cast<VectorType>(AType);
    NumEle = VT->getNumElements();

    // We currently cannot support more than 4 elements in a intrinsic
    // and we cannot support Vec3 types.
    if (NumEle >= 3) {
      return false;
    }
  }

  BinaryOperator *ShiftInst
  = dyn_cast<BinaryOperator>(Inst->getOperand(0));

  // If the first operand is not a shift instruction, then we can
  // return as it doesn't match this pattern.
  if (!ShiftInst || !ShiftInst->isShift()) {
    return false;
  }

  // If we are a shift left, then we need don't match this pattern.
  if (ShiftInst->getOpcode() == Instruction::Shl) {
    return false;
  }

  bool IsSigned = ShiftInst->isArithmeticShift();
  Constant *AndMask = dyn_cast<Constant>(Inst->getOperand(1));
  Constant *ShrVal = dyn_cast<Constant>(ShiftInst->getOperand(1));

  // Let's make sure that the shift value and the and mask are
  // constant integers.
  if (!AndMask || !ShrVal) {
    return false;
  }

  Constant *NewMaskConst;
  Constant *ShiftValConst;

  if (IsVector) {
    // Handle the vector case
    SmallVector<Constant *, 4> MaskVals;
    SmallVector<Constant *, 4> ShiftVals;
    ConstantDataVector *AndMaskVec = dyn_cast<ConstantDataVector>(AndMask);
    ConstantDataVector *ShrValVec = dyn_cast<ConstantDataVector>(ShrVal);
    Type *ScalarType = AndMaskVec->getType()->getScalarType();
    assert(AndMaskVec->getNumElements() ==
        ShrValVec->getNumElements() && "cannot have a "
        "combination where the number of elements to a "
        "shift and an and are different!");

    for (size_t X = 0, Y = AndMaskVec->getNumElements(); X < Y; ++X) {
      ConstantInt *AndCI
      = dyn_cast<ConstantInt>(AndMaskVec->getElementAsConstant(X));
      ConstantInt *ShiftIC
      = dyn_cast<ConstantInt>(ShrValVec->getElementAsConstant(X));

      if (!AndCI || !ShiftIC) {
        return false;
      }

      uint32_t MaskVal = (uint32_t)AndCI->getZExtValue();

      if (!isMask_32(MaskVal)) {
        return false;
      }

      MaskVal = (uint32_t)CountTrailingOnes_32(MaskVal);
      uint32_t ShiftVal = (uint32_t)ShiftIC->getZExtValue();

      // If the mask or shiftval is greater than the bitcount, then
      // break out.
      if (MaskVal >= 32 || ShiftVal >= 32) {
        return false;
      }

      // If the mask val is greater than the the number of original
      // bits left then this optimization is invalid.
      if (MaskVal > (32 - ShiftVal)) {
        return false;
      }

      MaskVals.push_back(ConstantInt::get(ScalarType, MaskVal, IsSigned));
      ShiftVals.push_back(ConstantInt::get(ScalarType, ShiftVal, IsSigned));
    }

    NewMaskConst = ConstantVector::get(MaskVals);
    ShiftValConst = ConstantVector::get(ShiftVals);
  } else {
    // Handle the scalar case
    uint32_t MaskVal = (uint32_t)dyn_cast<ConstantInt>(AndMask)->getZExtValue();

    // This must be a mask value where all lower bits are set to 1 and then any
    // bit higher is set to 0.
    if (!isMask_32(MaskVal)) {
      return false;
    }

    MaskVal = (uint32_t)CountTrailingOnes_32(MaskVal);

    // Count the number of bits set in the mask, this is the width of
    // the resulting bit set that is extracted from the source value.

    uint32_t ShiftVal = (uint32_t)dyn_cast<ConstantInt>(ShrVal)->getZExtValue();

    // If the mask or shift val is greater than the bitcount, then
    // break out.
    if (MaskVal >= 32 || ShiftVal >= 32) {
      return false;
    }

    // If the mask val is greater than the the number of original bits
    // left then this optimization is invalid.
    if (MaskVal > (32 - ShiftVal)) {
      return false;
    }

    NewMaskConst = ConstantInt::get(AType, MaskVal, IsSigned);
    ShiftValConst = ConstantInt::get(AType, ShiftVal, IsSigned);
  }

  // Let's create the function signature.
  SmallVector<Type *, 3> CallTypes;
  CallTypes.push_back(AType);
  CallTypes.push_back(AType);
  CallTypes.push_back(AType);
  FunctionType *FuncType = FunctionType::get(AType, CallTypes, false);
  std::string Name = "__amdil_ubit_extract";

  if (IsVector) {
    Name += "_v" + itostr(NumEle) + "i32";
  } else {
    Name += "_i32";
  }

  // Let's create the function.
  Function *Func =
    dyn_cast<Function>(Inst->getParent()->getParent()->getParent()->
        getOrInsertFunction(llvm::StringRef(Name), FuncType));
  Func->addFnAttr(Attributes::ReadNone);
  Value *Operands[3] = {
    NewMaskConst,
    ShiftValConst,
    ShiftInst->getOperand(0)
  };

  // Let's create the Call with the operands
  CallInst *CI = CallInst::Create(Func, Operands, "ByteExtractOpt");
  CI->insertBefore(Inst);
  Inst->replaceAllUsesWith(CI);
  Inst->eraseFromParent();
  return true;
}

static bool getVectorComponent(Instruction *Inst,
                               int TypeID,
                               unsigned NumElem,
                               Value *&VecVal,
                               unsigned &WhichElem) {
  ExtractElementInst *EInst = dyn_cast<ExtractElementInst>(Inst);

  if (!EInst) {
    return false;
  }

  VecVal = EInst->getVectorOperand();
  VectorType *VT = dyn_cast<VectorType>(VecVal->getType());
  assert(VT && "ExtractElementInst must "
               "have a vector type as first argument");
  Type *ET = VT->getElementType();

  if (VT->getNumElements() != NumElem || ET->getTypeID() != TypeID) {
    return false;
  }

  ConstantInt *CV = dyn_cast<ConstantInt>(EInst->getIndexOperand());

  if (!CV) {
    return false;
  }

  WhichElem = (unsigned)CV->getZExtValue();
  return true;
}

/*
   format:     f_2_u4 dst, src
   semantics:  dist.xyzw =
                 (((uint32)src.x) & 0xFF) |
                 ((((uint32)src.y) & 0xFF) << 8) |
                 ((((uint32)src.z) & 0xFF) << 16) |
                 ((((uint32)src.w) & 0xFF) << 24);

   If this pattern is found, change the sequence of operations into a
   intrinic call to u32 __amdil_f_2_u4 (v4f32)
   (int_AMDIL_media_convert_f2v4u8).

   TODO: if src are not from the same vector, create a new vector.
*/
bool AMDILPeepholeOpt::genIntrF2U4(Instruction *Inst) {
  // Try to handle the pattern:
  //   inst = Or0 | Or1 | Or2 | Or3
  Instruction *Or0, *Or1, *Or2, *Or3;
  Or0 = dyn_cast<Instruction>(Inst->getOperand(0));
  Or1 = dyn_cast<Instruction>(Inst->getOperand(1));

  if (!Or0 || !Or1) {
    return false;
  }

  bool Is_Or0 = (Or0->getOpcode() == Instruction::Or);
  bool Is_Or1 = (Or1->getOpcode() == Instruction::Or);

  if (Is_Or0 && Is_Or1) {
    Instruction *T0 = Or0, *T1 = Or1;
    Or0 = dyn_cast<Instruction>(T0->getOperand(0));
    Or1 = dyn_cast<Instruction>(T0->getOperand(1));
    Or2 = dyn_cast<Instruction>(T1->getOperand(0));
    Or3 = dyn_cast<Instruction>(T1->getOperand(1));
  } else if (Is_Or0 || Is_Or1) {
    if (Is_Or0) {
      // swap Or0 and Or1
      Or2 = Or0;
      Or0 = Or1;
      Or1 = Or2;
    }

    Or2 = dyn_cast<Instruction>(Or1->getOperand(0));
    Or1 = dyn_cast<Instruction>(Or1->getOperand(1));

    if (!Or1 || !Or2) {
      return false;
    } else {
      bool b1 = (Or1->getOpcode() == Instruction::Or);
      bool b2 = (Or2->getOpcode() == Instruction::Or);

      if ((b1 && b2) || (!b1 && !b2)) {
        return false;
      } else {
        if (b1) {
          // swap Or1 and Or2
          Or3 = Or1;
          Or1 = Or2;
          Or2 = Or3;
        }

        Or3 = dyn_cast<Instruction>(Or2->getOperand(0));
        Or2 = dyn_cast<Instruction>(Or2->getOperand(1));
      }
    }
  } else {
    return false;
  }

  // Sanity check
  if (!Or0 || !Or1 || !Or2 || !Or3) {
    return false;
  }

  // Check to see if all or's are from the same vector (v4f32), and
  // each one is converted to 8 bit integer...
  unsigned DstStart[4], DstWidth[4], SrcStart[4];
  Value *Src[4];
  Instruction *Dst[4] = { Or0, Or1, Or2, Or3 };

  Value *V4f32Val = NULL;

  for (int I = 0; I < 4; ++I) {
    if (!getIntValue(Dst[I], Src[I], SrcStart[I],
        DstStart[I], DstWidth[I]) ||
        (DstWidth[I] != 8) || (SrcStart[I] != 0) ||
        (SrcStart[I] > 24) || ((SrcStart[I] % 8) != 0)) {
      return false;
    }

    Instruction *TInst = dyn_cast<Instruction>(Src[I]);

    if (!TInst || (TInst->getOpcode() != Instruction::FPToUI)) {
      return false;
    }

    Src[I] = TInst->getOperand(0);
    TInst = dyn_cast<Instruction>(Src[I]);

    if (!TInst) {
      return false;
    }

    Value *VecVal;
    unsigned Which;

    if (!getVectorComponent(TInst,
                            Type::FloatTyID,
                            4,
                            VecVal,
                            Which)) {
      return false;
    }

    if (!V4f32Val) {
      V4f32Val = VecVal;
    } else if (V4f32Val != VecVal) {
      return false;
    }

    if (Which != (DstStart[I] / 8)) {
      return false;
    }
  }

  // Check and record the correct order in Pos[].
  int Pos[4] = { -1, -1, -1, -1 };

  for (int I = 0; I < 4; ++I) {
    unsigned Ix = (DstStart[I] / 8);

    if (Pos[Ix] != -1) {
      return false;
    }

    Pos[Ix] = I;
  }

  // Generate the intrinsic
  Type *RType = Inst->getType();
  SmallVector<Type *, 4> ArgTypes;
  ArgTypes.push_back(V4f32Val->getType());
  FunctionType *FuncType = FunctionType::get(RType, ArgTypes, false);
  Function *Proto_f2u4 = dyn_cast<Function>(
      mF->getParent()->getOrInsertFunction("__amdil_f_2_u4", FuncType));

  Proto_f2u4->addFnAttr(Attributes::ReadNone);
  CallInst *Call_f2u4
    = CallInst::Create(Proto_f2u4, V4f32Val, "F_2_U4", Inst);
  Inst->replaceAllUsesWith(Call_f2u4);
  Inst->eraseFromParent();

  return true;
}

bool AMDILPeepholeOpt::instLevelOptimizations(Instruction *Inst) {
  assert(Inst && "inst should not be NULL");

  bool IsDebug = (mOptLevel == CodeGenOpt::None);
  bool IsEGOrLater
  = (mSTM->getGeneration() >= AMDIL::EVERGREEN);

  // Remove dead inst (probably should do it in caller)
  if (!IsDebug && isInstructionTriviallyDead(Inst)) {
    Inst->eraseFromParent();
    return true;
  }

  const unsigned OpC = Inst->getOpcode();

  if ((OpC == Instruction::Or) && !IsDebug && IsEGOrLater &&
      genIntrF2U4(Inst)) {
    return true;
  }

  if ((OpC == Instruction::Call) && optimizeCallInst(Inst)) {
    return true;
  }

  if (EnableBFO &&
      (OpC == Instruction::And) && optimizeBitExtract(Inst)) {
    return true;
  }

  if (EnableBFO &&
      (OpC == Instruction::Xor) && optimizeBFI(Inst)) {
    return true;
  }

  if ((OpC == Instruction::Load || OpC == Instruction::Store) &&
      correctMisalignedMemOp(Inst)) {
    return false;
  }

  // If we are loading from a NULL pointer, replace the load with 0.
  if (OpC == Instruction::Load) {
    const Value *Ptr = dyn_cast<LoadInst>(Inst)->getPointerOperand();

    if (Ptr && dyn_cast<ConstantPointerNull>(Ptr)) {
      Inst->replaceAllUsesWith(Constant::getNullValue(Inst->getType()));
      Inst->eraseFromParent();
      return true;
    }
  }

  // If we are storing to a NULL pointer, then drop the store.
  if (OpC == Instruction::Store) {
    const Value *Ptr = dyn_cast<StoreInst>(Inst)->getPointerOperand();

    if (Ptr && dyn_cast<ConstantPointerNull>(Ptr)) {
      Inst->eraseFromParent();
      return true;
    }
  }

  if ((OpC == Instruction::Or) && optimizeClassInst(Inst)) {
    return true;
  }

  return false;
}

bool AMDILPeepholeOpt::correctMisalignedMemOp(Instruction *Inst) {
  LoadInst *LInst = dyn_cast<LoadInst>(Inst);
  StoreInst *SInst = dyn_cast<StoreInst>(Inst);
  unsigned Alignment;
  Type *Ty = Inst->getType();

  if (LInst) {
    Alignment = LInst->getAlignment();
    Ty = Inst->getType();
  } else if (SInst) {
    Alignment = SInst->getAlignment();
    Ty = SInst->getValueOperand()->getType();
  } else {
    return false;
  }

  unsigned size = TM.getDataLayout()->getTypeAllocSize(Ty);

  if (size == Alignment || size < Alignment) {
    return false;
  }

  if (!Ty->isStructTy()) {
    return false;
  }

  if (Alignment < 4) {
    if (LInst) {
      LInst->setAlignment(0);
      return true;
    } else if (SInst) {
      SInst->setAlignment(0);
      return true;
    }
  }

  return false;
}
bool AMDILPeepholeOpt::isSigned24BitOps(CallInst *CI) {
  if (!CI) {
    return false;
  }

  Function *LHS = CI->getCalledFunction();
  if (!LHS) {
    return false;
  }

  StringRef Name = LHS->getName();
  if (!Name.startswith("__amdil_imad24")
      && !Name.startswith("__amdil_imul24")
      && !Name.startswith("__amdil__imul24_high")) {
    return false;
  }

  return !mSTM->usesHardware(AMDIL::Caps::Signed24BitOps);
}

void AMDILPeepholeOpt::expandSigned24BitOps(CallInst *CI) {
  assert(isSigned24BitOps(CI) && "Must be a "
      "signed 24 bit operation to call this function!");
  Function *LHS = CI->getCalledFunction();
  if (!LHS) {
    return;
  }

  // On 8XX we do not have signed 24bit, so we need to
  // expand it to the following:
  // imul24 turns into 32bit imul
  // imad24 turns into 32bit imad
  // imul24_high turns into 32bit imulhigh

  StringRef LHSName = LHS->getName();

  if (LHSName.startswith("__amdil_imad24")) {
    Type *AType = CI->getOperand(0)->getType();
    bool IsVector = AType->isVectorTy();
    int NumEle = IsVector
               ? dyn_cast<VectorType>(AType)->getNumElements()
               : 1;

    SmallVector<Type *, 3> CallTypes;
    CallTypes.push_back(CI->getOperand(0)->getType());
    CallTypes.push_back(CI->getOperand(1)->getType());
    CallTypes.push_back(CI->getOperand(2)->getType());
    FunctionType *FuncType =
      FunctionType::get(CI->getOperand(0)->getType(),
          CallTypes,
          false);

    std::string Name = "__amdil_imad";

    if (IsVector) {
      Name += "_v" + itostr(NumEle) + "i32";
    } else {
      Name += "_i32";
    }

    Function *Func = dyn_cast<Function>(
        CI->getParent()->getParent()->getParent()->
        getOrInsertFunction(llvm::StringRef(Name), FuncType));
    Func->addFnAttr(Attributes::ReadNone);
    Value *Operands[3] = {
      CI->getOperand(0),
      CI->getOperand(1),
      CI->getOperand(2)
    };
    CallInst *nCI = CallInst::Create(Func, Operands, "imad24");
    nCI->insertBefore(CI);
    CI->replaceAllUsesWith(nCI);
  } else if (LHSName.startswith("__amdil_imul24")) {
    BinaryOperator *mulOp =
      BinaryOperator::Create(Instruction::Mul, CI->getOperand(0),
          CI->getOperand(1), "imul24", CI);
    CI->replaceAllUsesWith(mulOp);
  } else if (LHSName.startswith("__amdil_imul24_high")) {
    Type *AType = CI->getOperand(0)->getType();

    bool IsVector = AType->isVectorTy();
    int NumEle = IsVector
        ? dyn_cast<VectorType>(AType)->getNumElements()
        : 1;

    SmallVector<Type *, 2> CallTypes;
    CallTypes.push_back(CI->getOperand(0)->getType());
    CallTypes.push_back(CI->getOperand(1)->getType());
    FunctionType *FuncType =
      FunctionType::get(CI->getOperand(0)->getType(),
          CallTypes,
          false);

    std::string name = "__amdil_imul_high";

    if (IsVector) {
      name += "_v" + itostr(NumEle) + "i32";
    } else {
      name += "_i32";
    }

    Function *Func = dyn_cast<Function>(
        CI->getParent()->getParent()->getParent()->
        getOrInsertFunction(llvm::StringRef(name), FuncType));
    Func->addFnAttr(Attributes::ReadNone);

    Value *Operands[2] = {
      CI->getOperand(0),
      CI->getOperand(1)
    };
    CallInst *nCI = CallInst::Create(Func, Operands, "imul24_high");
    nCI->insertBefore(CI);
    CI->replaceAllUsesWith(nCI);
  }
}

bool AMDILPeepholeOpt::isRWGLocalOpt(CallInst *CI) {
  if (!CI || !mRWGOpt) {
    return false;
  }

  Function *Func = CI->getCalledFunction();
  if (!Func) {
    return false;
  }

  // We have to check if we are a kernel currently
  // because we inline everything and only kernels
  // should be left. However, in some cases, other
  // functions exist and we don't want to
  // optimize them because we don't track that
  // information.
  return (Func->getName() == "__amdil_get_local_size_int" &&
          mAMI->getKernel(mF->getName()));
}

void AMDILPeepholeOpt::expandRWGLocalOpt(CallInst *CI) {
  assert(isRWGLocalOpt(CI) && "This optimization only works when "
      "the call inst is get_local_size!");
  SmallVector<Constant *, 3> Consts;
  Type *Int32Type = Type::getInt32Ty(*mCTX);
  const AMDILKernel *Kernel = mAMI->getKernel(mF->getName());

  for (uint32_t I = 0; I < 3; ++I) {
    // We don't have to check if sgv is valid or not as we
    // checked this case before we set mRWGOpt to true.
    uint32_t Val = Kernel->sgv->reqGroupSize[I];
    Consts.push_back(ConstantInt::get(Int32Type, Val));
  }

  Consts.push_back(ConstantInt::get(Int32Type, 0));
  Value *CVec = ConstantVector::get(Consts);
  CI->replaceAllUsesWith(CVec);
  ++LocalFuncs;
  return;
}

bool AMDILPeepholeOpt::convertAccurateDivide(CallInst *CI) {
  if (!CI) {
    return false;
  }

  if (mSTM->getGeneration() == AMDIL::NORTHERN_ISLANDS &&
      (mSTM->getDeviceFlag() == OCL_DEVICE_CAYMAN ||
       mSTM->getDeviceFlag() == OCL_DEVICE_TRINITY ||
       mSTM->getDeviceName() == "kauai")) { // TODO: ???
    return false;
  }

  Function *Func = CI->getCalledFunction();
  if (!Func) {
    return false;
  }

  return Func->getName().startswith("__amdil_improved_div");
}

void AMDILPeepholeOpt::expandAccurateDivide(CallInst *CI) {
  assert(convertAccurateDivide(CI)
      && "Expanding accurate divide can "
         "only happen if it's expandable!");
  BinaryOperator *divOp = BinaryOperator::Create(Instruction::FDiv,
                                                 CI->getOperand(0),
                                                 CI->getOperand(1),
                                                 "fdiv32",
                                                 CI);
  CI->replaceAllUsesWith(divOp);
}

void AMDILPeepholeOpt::replaceCallWithConstant(CallInst *CI, bool Value) {
  IntegerType *BoolType = IntegerType::getInt32Ty(*mCTX);
  ConstantInt *ConstVal = ConstantInt::get(BoolType, Value);

  CI->replaceAllUsesWith(ConstVal);
  CI->eraseFromParent();
}

void AMDILPeepholeOpt::replaceReserveSharedCall(CallInst *CI, bool Region) {
  const ConstantInt *Size = cast<ConstantInt>(CI->getOperand(0));

  StringRef ReservedName = Region
                         ? "__amd_sw_region_barrier_gds_reserved"
                         : "__amd_sw_region_barrier_lds_reserved";

  const APInt &APValue = Size->getValue();
  size_t MaxSize = Region
      ? mSTM->getMaxGDSSize()
      : mSTM->getMaxLDSSize();
  uint64_t NInt = APValue.getLimitedValue(MaxSize);

  unsigned Alignment = mSTM->getStackAlignment();
  AMDILKernel *Kernel = mAMI->getKernel(mF->getName());
  uint32_t Offset = mAMI->reserveShared(Kernel,
      NInt * sizeof(int32_t),
      Alignment,
      ReservedName,
      Region);


  unsigned AddressSpace = Region
      ? AMDILAS::REGION_ADDRESS
      : AMDILAS::LOCAL_ADDRESS;

  // FIXME: Get correct smaller pointer size for GDS/LDS
  unsigned LDSPtrSize = TM.getDataLayout()->getPointerSizeInBits();

  IntegerType *PtrIntType = IntegerType::get(*mCTX, LDSPtrSize);
  ConstantInt *OffsetValue = ConstantInt::get(PtrIntType,
                                              Offset,
                                              false);
  Type *Int32Type = Type::getInt32Ty(*mCTX);
  PointerType *PtrType = PointerType::get(Int32Type, AddressSpace);
  IntToPtrInst *Cast = new IntToPtrInst(OffsetValue,
                                        PtrType,
                                        ReservedName,
                                        CI);
  CI->replaceAllUsesWith(Cast);
  CI->eraseFromParent();
}

bool AMDILPeepholeOpt::selectRegionBarrierImpl(CallInst *CI) {
  Function *Func = CI->getCalledFunction();
  if (!Func) {
    return false;
  }

  StringRef CalleeName = Func->getName();

  if (CalleeName == "__amdil_use_sw_region_barrier") {
    const AMDILKernel *Kernel = mAMI->getKernel(mF->getName());

    // region size known at compile time
    bool UseNative = (Kernel->sgv->mHasRWR
                      || mSTM->hasDynamicRegionSize());
    replaceCallWithConstant(CI, !UseNative);
  } else if (CalleeName == "__amdil_reserve_lds_int") {
    replaceReserveSharedCall(CI, false);
  } else if (CalleeName == "__amdil_reserve_gds_int") {
    replaceReserveSharedCall(CI, true);
  } else if (CalleeName.startswith("__amdil_reserve_semaphore")) {
    uint32_t MaxSemaphores = mSTM->getMaxNumSemaphores();
    assert(MaxSemaphores >= 2);

    // TODO: Assumes using __amdil_reserve_semaphore0 or
    // __amdil_reserve_semaphore1
    assert(CalleeName.equals("__amdil_reserve_semaphore0")
        || CalleeName.equals("__amdil_reserve_semaphore1"));

    uint32_t SemaID = CalleeName.back() == '0'
        ? MaxSemaphores - 1
        : MaxSemaphores - 2;

    AMDILMachineFunctionInfo *MFI
    = getAnalysis<MachineFunctionAnalysis>().getMF()
      .getInfo<AMDILMachineFunctionInfo>();
    MFI->sema_insert(SemaID);

    IntegerType *IDType = IntegerType::get(*mCTX, 32);
    ConstantInt *SemaValue = ConstantInt::get(IDType, SemaID, false);
    CI->replaceAllUsesWith(SemaValue);
    CI->eraseFromParent();
  } else {
    return false;
  }

  return true;
}

bool AMDILPeepholeOpt::doInitialization(Module &M) {
  return false;
}

bool AMDILPeepholeOpt::doFinalization(Module &M) {
  return false;
}

void AMDILPeepholeOpt::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<MachineFunctionAnalysis>();
  FunctionPass::getAnalysisUsage(AU);
  AU.setPreservesAll();
}

