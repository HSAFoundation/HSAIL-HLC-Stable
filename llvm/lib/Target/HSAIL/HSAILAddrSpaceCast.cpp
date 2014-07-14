
//===-- HSAILAddrSpaceCast.cpp - Optimize and Lower AddrSpaceCast  -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// \file
// \brief This file implements a FunctionPass to optimize and lower address
// space cast (AddrSpaceCast) to support OpenCL 2.0 generic address feature.
//
//===----------------------------------------------------------------------===//

#include "HSAIL.h"
#include "HSAILUtilityFunctions.h"

#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Operator.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/AMDMetadataUtils.h"


using namespace llvm;

/// We do AddrSpaceCast optimizations by default. Use this flag
/// to turn it off.
static cl::opt<bool> DisableAddrSpaceCastOpts("disable-addrspacecast-opts",
  cl::desc("Disable AddrSpaceCast Optimizations"));

namespace {

class HSAILAddrSpaceCast : public FunctionPass {
public:
  static char ID;
  // TODO: access CLK_GLOBAL_MEM_FENCE, CLK_LOCAL_MEM_FENCE.
  static const int G_FENCE = (1 << 0);
  static const int L_FENCE = (1 << 1);

  explicit HSAILAddrSpaceCast() : FunctionPass(ID) {}

  virtual bool doInitialization(Module &M) LLVM_OVERRIDE;
  virtual bool runOnFunction(Function &F) LLVM_OVERRIDE;
  virtual const char *getPassName() const LLVM_OVERRIDE {
    return "Optimize and lower AddrSpaceCast";
  }

private:
  Module *Mod;

  Type *getAnyPtrType (Type *SrcTy);
  Instruction *hoistConstantExpr(ConstantExpr *CE, Instruction *InsertPt);
  Instruction *expandAddrSpaceCastInst(BitCastInst *I);
  Instruction *genAddrSpaceCastIntrinsic(Operator *Cast, Instruction *InsertPt);
  bool hoistAddrSpaceCastFromGEP(GEPOperator *GEP);
  bool optimizeMemoryInst(Instruction *MI, unsigned Idx);
  bool optimizeGenericAddrBuiltin(Instruction *MI);
  void expandToSegmentBuiltin(CallInst *Call, Value *InValue, unsigned AS);
  void optimizeToSegmentBuiltin(CallInst *Call, Value *InValue, unsigned AS);
  bool lowerAddrSpaceCastToIntrinsic(Function &F);
  bool expandPtrToIntInst(PtrToIntInst *PTII);
  bool expandIntToPtrInst(IntToPtrInst *ITPI);
};
} // End anonymous namespace.

char HSAILAddrSpaceCast::ID = 0;

namespace llvm {
void initializeHSAILAddrSpaceCastPass(PassRegistry &);
}
INITIALIZE_PASS(HSAILAddrSpaceCast, "hsail-address-space-cast",
                "Optimize and lower AddrSpaceCast", false, false);

/// Create HSAILAddrSpaceCast object.
FunctionPass *llvm::createHSAILAddrSpaceCastPass() {
  return new HSAILAddrSpaceCast();
}

/// Initialization.
bool HSAILAddrSpaceCast::doInitialization(Module &M) {
  Mod = &M;
  return false;
}

/// Determine whether this is a valid AddrSpaceCast Operation.
/// Stof_only means we only want segment to generic casting.
static bool isAddrSpaceCast(Operator *Cast, bool Stof_only) {
  // CLANG (clc2) is using BitCast for AddrSpaceCast prior to LLVM 3.4.
  if (!Cast || Cast->getOpcode() != Instruction::BitCast) {
    return false;
  }
  Value *Src = Cast->getOperand(0);
  PointerType *SrcPtrTy = dyn_cast<PointerType>(Src->getType());
  PointerType *DstPtrTy = dyn_cast<PointerType>(Cast->getType());
  // Both should be of pointer types.
  if (!SrcPtrTy || !DstPtrTy) {
    return false;
  }

  // Src is of segment and dest is of generic.
  unsigned SrcAS = SrcPtrTy->getAddressSpace();
  unsigned DstAS = DstPtrTy->getAddressSpace();

  if (SrcAS == DstAS) { // Same address space.
    return false;
  }

  return (DstAS == HSAILAS::FLAT_ADDRESS || // stof
          (SrcAS == HSAILAS::FLAT_ADDRESS && !Stof_only)); // ftos
}

/// Determine whether V is a ConstantExpr that constains AddrSpaceCast.
/// Return ConstantExpr of V if DOES HAVE.
static ConstantExpr *hasConstantAddrSpaceCast(Value *V) {
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(V)) {
    if (isAddrSpaceCast(dyn_cast<Operator>(V), false)) {
      return CE;
    }
    unsigned NumOperands = CE->getNumOperands();
    for (unsigned index = 0; index < NumOperands; ++index) {
      if (hasConstantAddrSpaceCast(CE->getOperand(index))) {
        return CE;
      }
    }
  }
  return (ConstantExpr *)0;
}

/// Extract the source operand from a AddrSpaceCast operation.
/// Return the source operand directly if both source and destination operands
/// of Cast have the same element type (within different address space).
/// Otherwise, insert a new BitCast to cast source operand to new value with
/// destination's element type and source's address space, and return the new
/// value.
///
/// Example 1: OutPtr has the same element type with InPtr, return InPtr.
///   %OutPtr = bitcast i32 addrspace(1)* %InPtr to i32 addrspace(4)*
///   // NOTE: return InPtr.
///
/// Example 2: OutPtr has a different element type from InPtr, need a bitcast.
///  %OutPtr = bitcast f32 addrspace(1)* %InPtr to i32 addrspace(4)*
///  ===>
///  %NewCast = bitcast f32 addrspace(1)* %InPtr to i32 addrspace(1)*
///  %OutPtr =  bitcast i32 addrspace(1)* %NewCast to i32 addrspace(4)*
///  // NOTE: return NewCast.
///
static Value *extractSrcOperand(Operator *Cast, Instruction *InsertPt) {
  assert(isAddrSpaceCast(Cast, false) && "Not an AddrSpaceCast!");
  Value *Src = Cast->getOperand(0);
  PointerType *SrcPtrTy = cast<PointerType>(Src->getType()->getScalarType());
  PointerType *DstPtrTy = cast<PointerType>(Cast->getType()->getScalarType());

  Type *SrcElemTy = SrcPtrTy->getElementType();
  Type *DstElemTy = DstPtrTy->getElementType();
  // Same element type.
  if (SrcElemTy == DstElemTy) {
    return Src;
  }
  // New pointer type with dst's element type and src's address space!
  Type *CastTy = PointerType::get(DstElemTy, SrcPtrTy->getAddressSpace());
  if (Cast->getType()->isVectorTy()) { // Handle vectors of pointers.
    CastTy = VectorType::get(CastTy, Cast->getType()->getVectorNumElements());
  }
  return new BitCastInst(Src, CastTy, "", InsertPt);
}

/// Get a new pointer type of <i8, SrcTy->getAddressSpace()>. This is used
/// to help generate AddrSoaceCast intrinsics.
Type *HSAILAddrSpaceCast::getAnyPtrType (Type *SrcTy) {
  PointerType *SrcPtrTy = cast<PointerType>(SrcTy);
  return PointerType::get(Type::getInt8Ty(Mod->getContext()),
                          SrcPtrTy->getAddressSpace());
}

/// Hoist AddrSpaceCast from GetElementPtr (GEP). GEP could be either
/// an instruction or a constant expression.
bool HSAILAddrSpaceCast::hoistAddrSpaceCastFromGEP(GEPOperator *GEP) {
  Operator *Cast = dyn_cast<Operator>(GEP->getPointerOperand());
  if (!Cast) { // GEP has nullptr Pointer Operand.
    return false;
  }

  // Not a segment to generic address space casting.
  if (!isAddrSpaceCast(Cast, true)) {
    return false;
  }

  SmallVector<Value *, 8> Indices(GEP->idx_begin(), GEP->idx_end());
  Type *GEPTy = GEP->getType();
  if (Instruction *GEPI = dyn_cast<Instruction>(GEP)) {
    // %1 = gep (addrspacecast X), indices
    // =>
    // %0 = gep X, indices
    // %1 = addrspacecast %0
    GetElementPtrInst *NewGEPI = GetElementPtrInst::Create(
        extractSrcOperand(Cast, GEPI), Indices, GEP->getName(), GEPI);
    NewGEPI->setIsInBounds(GEP->isInBounds());
    GEP->replaceAllUsesWith(new BitCastInst(NewGEPI, GEPTy, "", GEPI));
  } else {
    // GEP is a constant expression.
    // Don't handle pointers to different element types for now.
    // TODO: Investigate how to use something like "extractSrcOperand" in
    //       ConstantExpr in this case.
    PointerType *SrcPtrTy = cast<PointerType>(Cast->getOperand(0)->getType());
    PointerType *DstPtrTy = cast<PointerType>(Cast->getType());
    if (SrcPtrTy->getElementType() != DstPtrTy->getElementType()) {
      return false;
    }
    Constant *NewGEPCE = ConstantExpr::getGetElementPtr(
        cast<Constant>(Cast->getOperand(0)), Indices, GEP->isInBounds());
    GEP->replaceAllUsesWith(ConstantExpr::getBitCast(NewGEPCE, GEPTy));
  }
  return true;
}

/// Optimize a load/store.
bool HSAILAddrSpaceCast::optimizeMemoryInst(Instruction *MI, unsigned Idx) {
  // If the pointer operand is a GEP, hoist the addrspacecast if any from the
  // GEP to expose more optimization opportunites.
  if (GEPOperator *GEP = dyn_cast<GEPOperator>(MI->getOperand(Idx))) {
    hoistAddrSpaceCastFromGEP(GEP);
  }

  // Load/store (addrspacecast X) => load/store X if shortcutting the
  // addrspacecast is valid and can improve performance.
  //
  // e.g.,
  // %1 = addrspacecast float addrspace(3)* %0 to float*
  // %2 = load float* %1
  // ->
  // %2 = load float addrspace(3)* %0
  //
  // Note: the addrspacecast can also be a constant expression.
  if (Operator *Cast = dyn_cast<Operator>(MI->getOperand(Idx))) {
    if (isAddrSpaceCast(Cast, true)) {
      // The AddrSpaceCast may become dead after this optimization.
      MI->setOperand(Idx, extractSrcOperand(Cast, MI));
      return true;
    }
  }
  return false;
}

/// Get the corresponding segmentp intrinsic name.
static StringRef selectSegmentPName(unsigned OutAS) {
  switch (OutAS) {
  case HSAILAS::GLOBAL_ADDRESS:
    return "__hsail_segmentp_global";
  case HSAILAS::GROUP_ADDRESS:
    return "__hsail_segmentp_local";
  case HSAILAS::PRIVATE_ADDRESS:
    return "__hsail_segmentp_private";
  default:
    llvm_unreachable("Invalid address space");
  }
}

/// Get the corresponding stof/ftos intrinsic name.
static StringRef selectAddrSpaceCastName(unsigned AS, bool Stof) {
  switch (AS) {
  case HSAILAS::GLOBAL_ADDRESS:
    return Stof ? "__hsail_addrspacecast_g2f" : "__hsail_addrspacecast_f2g";
  case HSAILAS::GROUP_ADDRESS:
    return Stof ? "__hsail_addrspacecast_l2f" : "__hsail_addrspacecast_f2l";
  case HSAILAS::PRIVATE_ADDRESS:
    return Stof ? "__hsail_addrspacecast_p2f" : "__hsail_addrspacecast_f2p";
  default:
    llvm_unreachable("Invalid address space");
  }
}

/// Optimize away to_segment builtins.
void HSAILAddrSpaceCast::optimizeToSegmentBuiltin(CallInst *Call,
                                                  Value *InValue,
                                                  unsigned OutAS) {
  PointerType *InPtrTy = cast<PointerType>(InValue->getType());
  unsigned InAS = InPtrTy->getAddressSpace();
  // Must be a segment address space!
  assert(InAS == HSAILAS::GLOBAL_ADDRESS || InAS == HSAILAS::GROUP_ADDRESS ||
         InAS == HSAILAS::PRIVATE_ADDRESS);
  if (InAS != OutAS) { // return NULL if not match.
    Call->replaceAllUsesWith(Constant::getNullValue(Call->getType()));
  } else if (InValue->getType() != Call->getType()) {
    // Cast InValue to the type of Call.
    BitCastInst *BCI = new BitCastInst(InValue, Call->getType(), "", Call);
    Call->replaceAllUsesWith(BCI);
  } else { // Simply replace call with InValue.
    Call->replaceAllUsesWith(InValue);
  }
}

/// Lower the to_segment builtins to segmentp_segment, ftos_segment and select
///  sequence.
void HSAILAddrSpaceCast::expandToSegmentBuiltin(CallInst *Call, Value *InValue,
                                                unsigned OutAS) {

  Type *InValueAnyPtrTy = getAnyPtrType(InValue->getType());
  Type *CallAnyPtrTy = getAnyPtrType(Call->getType());
  // Convert InValue to "AnyPtrType".
  BitCastInst *CastedInValue = new BitCastInst(InValue, InValueAnyPtrTy,
                                               "", Call);
  // Generate segmentp
  StringRef SegmentpName = selectSegmentPName(OutAS);
  Constant *CST1 =
      Mod->getOrInsertFunction(SegmentpName, Type::getInt1Ty(Mod->getContext()),
                               InValueAnyPtrTy, (Type *)0);
  Function *SegmentpFunc = cast<Function>(CST1);
  SegmentpFunc->addFnAttr(Attributes::ReadNone);
  Value *Arg1[] = { CastedInValue };
  CallInst *CI1 = CallInst::Create(SegmentpFunc, Arg1, "segmentp", Call);

  // Generate ftos
  StringRef FtosName = selectAddrSpaceCastName(OutAS, /*not stof*/ false);
  Constant *CST2 = Mod->getOrInsertFunction(FtosName, CallAnyPtrTy,
                                            InValueAnyPtrTy, (Type *)0);
  Function *FtosFunc = cast<Function>(CST2);
  FtosFunc->addFnAttr(Attributes::ReadNone);
  Value *Arg2[] = { CastedInValue };
  CallInst *CI2 = CallInst::Create(FtosFunc, Arg2, "ftos", Call);

  // Generate a select: segmentp ? ftos : null.
  SelectInst *SELI = SelectInst::Create(
      CI1, CI2, Constant::getNullValue(CallAnyPtrTy), "", Call);
  // Cast back from "AnyPtrType" to Call->getType().
  Call->replaceAllUsesWith(new BitCastInst(SELI, Call->getType(), "", Call));
}

/// Optimize away get_fence builtin.
static void optimizeGetFenceBuiltin(CallInst *Call) {
  int fence = 0;
  SmallVector<Value *, 2> Objs;
  GetUnderlyingObjects(const_cast<Value *>(Call->getArgOperand(0)), Objs);
  for (SmallVectorImpl<Value *>::iterator I = Objs.begin(), E = Objs.end();
       I != E; ++I) {
    Value *V = *I;
    PointerType *SrcPtrTy = cast<PointerType>(V->getType());
    unsigned InAS = SrcPtrTy->getAddressSpace();
    switch (InAS) {
    case HSAILAS::GLOBAL_ADDRESS:
      fence |= HSAILAddrSpaceCast::G_FENCE;
      break;
    case HSAILAS::GROUP_ADDRESS:
    case HSAILAS::PRIVATE_ADDRESS:
      fence |= HSAILAddrSpaceCast::L_FENCE;
      break;
    case HSAILAS::FLAT_ADDRESS:
      fence = HSAILAddrSpaceCast::G_FENCE | HSAILAddrSpaceCast::L_FENCE;
    default:
      // Don't add new fence.
      break;
    }
  }
  // Replace the Call with the "fence" calculated above.
  Call->replaceAllUsesWith(ConstantInt::get(Call->getType(), fence, false));
}

/// Handle builtins: to_global, to_local, to_private and genfence.
/// TODO: Use SPIR name mangling to identify builtin functions!
bool HSAILAddrSpaceCast::optimizeGenericAddrBuiltin(Instruction *MI) {
  CallInst *Call = cast<CallInst>(MI);
  Function *Func = Call->getCalledFunction();
  if (!Func) {
    return false;
  }
  StringRef FuncName = Func->getName();

  // Optimize get_fence.
  if (FuncName.startswith("_Z9get_fence")) {
    optimizeGetFenceBuiltin(Call);
    // Yes, we need to delete the Call here.
    Call->eraseFromParent();
    return true;
  }

  unsigned OutAS = 0;
  if (FuncName.startswith("_Z9to_global")) {
    OutAS = HSAILAS::GLOBAL_ADDRESS;
  } else if (FuncName.startswith("_Z8to_local")) {
    OutAS = HSAILAS::GROUP_ADDRESS;
  } else if (FuncName.startswith("_Z10to_private")) {
    OutAS = HSAILAS::PRIVATE_ADDRESS;
  } else {
    return false;
  }

  // Try to optimize away to_global/to_local/to_private.
  bool OptimizedAway = false;
  if (!DisableAddrSpaceCastOpts) {
    if (Operator *Arg0 = dyn_cast<Operator>(Call->getArgOperand(0))) {
      if (isAddrSpaceCast(Arg0, /*stof*/ true)) {
        optimizeToSegmentBuiltin(Call, extractSrcOperand(Arg0, Call), OutAS);
        OptimizedAway = true;
      } else if (Arg0->getOpcode() == Instruction::BitCast) { // cast to void *?
        if (GEPOperator *GEP = dyn_cast<GEPOperator>(Arg0->getOperand(0))) {
          hoistAddrSpaceCastFromGEP(GEP);
        }
        if (isAddrSpaceCast(dyn_cast<Operator>(Arg0->getOperand(0)), true)) {
          Value *Src =
              extractSrcOperand(cast<Operator>(Arg0->getOperand(0)), Call);
          optimizeToSegmentBuiltin(Call, Src, OutAS);
          OptimizedAway = true;
        }
      }
    }
  }
  // Optimization is not successful, we lower the call to corresponding hsail
  // intrinsics (segmentp, ftos).
  if (!OptimizedAway) {
    expandToSegmentBuiltin(Call, Call->getArgOperand(0), OutAS);
  }

  Call->eraseFromParent();
  return true;
}

/// Generate AddrSpaceCast operation to intrinsic.
Instruction *HSAILAddrSpaceCast::genAddrSpaceCastIntrinsic(Operator *Cast,
                                                      Instruction *InsertPt) {
  assert(isAddrSpaceCast(Cast, false) && "Not an AddrSpaceCast!");
  PointerType *SrcPtrTy = cast<PointerType>(Cast->getOperand(0)->getType());
  PointerType *DstPtrTy = cast<PointerType>(Cast->getType());
  unsigned SrcAS = SrcPtrTy->getAddressSpace();
  unsigned DstAS = DstPtrTy->getAddressSpace();

  StringRef IntrName;
  if (DstAS == HSAILAS::FLAT_ADDRESS) {
    IntrName = selectAddrSpaceCastName(SrcAS, /*stof*/ true);
  } else if (SrcAS == HSAILAS::FLAT_ADDRESS) {
    IntrName = selectAddrSpaceCastName(DstAS, /*ftos*/ false);
  } else {
    llvm_unreachable("Invalid address space cast");
  }

  Value *Src = extractSrcOperand(Cast, InsertPt);

  Type *SrcAnyPtrTy = getAnyPtrType(Src->getType());
  Type *DstAnyPtrTy = getAnyPtrType(Cast->getType());
  // Convert Src to "AnyPtrType".
  BitCastInst *CastedSrc = new BitCastInst(Src, SrcAnyPtrTy, "", InsertPt);
  Function *IntrFunc = cast<Function>(Mod->getOrInsertFunction(
      IntrName, DstAnyPtrTy, SrcAnyPtrTy, (Type *)0));
  IntrFunc->addFnAttr(Attributes::ReadNone);
  Value *Args[] = { CastedSrc };
  CallInst *Call = CallInst::Create(IntrFunc, Args, "AddrSpaceCast", InsertPt);
  // Cast back from "AnyPtrType" to Cast->getType().
  return new BitCastInst(Call, Cast->getType(), "", InsertPt);
}

/// Expand an AddrSpaceCast instruction to the corresponding intrinsic.
Instruction *HSAILAddrSpaceCast::expandAddrSpaceCastInst(BitCastInst *Inst) {
  Instruction *ASCI = genAddrSpaceCastIntrinsic(cast<Operator>(Inst), Inst);
  Inst->replaceAllUsesWith(ASCI);
  Inst->eraseFromParent();
  return ASCI;
}

///
/// Convert a ConstantExpr with AddrSpaceCast into an instruction.
/// This function does NOT perform any recursion, so the resulting
/// instruction may have constant expression operands.
///
Instruction *HSAILAddrSpaceCast::hoistConstantExpr(ConstantExpr *CE,
                                                   Instruction *InsertPt) {

  // CE is AddrSpaceCast, generate intrinsic directly.
  // This is also to differentiate AddrSpaceCast with normal BitCast.
  if (isAddrSpaceCast(dyn_cast<Operator>(CE), false)) {
    return genAddrSpaceCastIntrinsic(cast<Operator>(CE), InsertPt);
  }

  Instruction *NewI = (Instruction *)0;
  switch (CE->getOpcode()) {
  case Instruction::GetElementPtr: {
    SmallVector<Value *, 4> Indices;
    unsigned NumOperands = CE->getNumOperands();
    for (unsigned index = 1; index < NumOperands; ++index) {
      Indices.push_back(CE->getOperand(index));
    }
    NewI = GetElementPtrInst::Create(CE->getOperand(0), Indices, CE->getName(),
                                     InsertPt);

    break;
  }
  case Instruction::Add:
  case Instruction::Sub:
  case Instruction::Mul:
  case Instruction::UDiv:
  case Instruction::SDiv:
  case Instruction::FDiv:
  case Instruction::URem:
  case Instruction::SRem:
  case Instruction::FRem:
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor: {
    Instruction::BinaryOps Op = (Instruction::BinaryOps)(CE->getOpcode());
    NewI = BinaryOperator::Create(Op, CE->getOperand(0), CE->getOperand(1),
                                  CE->getName(), InsertPt);
    break;
  }
  case Instruction::BitCast: {
    NewI = new BitCastInst(CE->getOperand(0), CE->getType(), CE->getName(),
                           InsertPt);
    break;
  }

  default:
    llvm_unreachable("Unhandled constant expression");
  }
  return NewI;
}

/// If the optimization is not successful, we will generate flat memory
/// operations. To support this, all live AddrSpaceCast operations have
/// to be lowered to AddrSpaceCast intrinsics to generate stof/ftos.
///
/// NOTE: For LLVM 3.4, we can directly emit stof/ftos from AddrSpaceCast
/// operation.
///
bool HSAILAddrSpaceCast::lowerAddrSpaceCastToIntrinsic(Function &F) {
  bool Changed = false;
  // Second pass to do the translation.
  std::vector<Instruction *> Worklist;
  for (Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B) {
    for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE;) {
      Instruction *CurI = I++;
      if (isAddrSpaceCast(dyn_cast<Operator>(CurI), false)) {
        Instruction *NewI = expandAddrSpaceCastInst(cast<BitCastInst>(CurI));
        Worklist.push_back(NewI);
        Changed = true;
      } else { // Check for constant expression with AddrSpaceCast!
        unsigned NumOperands = CurI->getNumOperands();
        for (unsigned index = 0; index < NumOperands; ++index) {
          if (hasConstantAddrSpaceCast(CurI->getOperand(index))) {
            Worklist.push_back(CurI);
            Changed = true;
          }
        }
      }
    }
  }

  while (Worklist.size()) {
    Instruction *I = Worklist.back();
    Worklist.pop_back();
    unsigned NumOperands = I->getNumOperands();
    for (unsigned index = 0; index < NumOperands; ++index) {
      // Scan through the operands of this instruction and convert each into an
      // instruction.  Note that this works a little differently for phi
      // instructions because the new instruction must be added to the
      // appropriate predecessor block.
      if (PHINode *PHI = dyn_cast<PHINode>(I)) {
        // For PHI Nodes, if an operand is a constant expression of cast, we
        // want to insert the new instructions in the predecessor basic block.
        //
        // Note: It seems that it's possible for a phi to have the same
        // incoming basic block listed multiple times; this seems okay as long
        // the same value is listed for the incoming block.
        Instruction *InsertPt = PHI->getIncomingBlock(index)->getTerminator();
        if (ConstantExpr *CE =
                  hasConstantAddrSpaceCast(PHI->getIncomingValue(index))) {
          Instruction *NewI = hoistConstantExpr(CE, InsertPt);
          for (unsigned i = index; i < NumOperands; ++i) {
            if (PHI->getIncomingBlock(i) == PHI->getIncomingBlock(index)) {
              PHI->setIncomingValue(i, NewI);
            }
          }
          Worklist.push_back(NewI);
        }
      } else {
        // For other instructions, we want to insert instructions replacing
        // constant expressions immediently before the instruction using the
        // constant expression.
        if (ConstantExpr *CE = hasConstantAddrSpaceCast(I->getOperand(index))) {
          Instruction *NewI = hoistConstantExpr(CE, I);
          I->replaceUsesOfWith(CE, NewI);
          Worklist.push_back(NewI);
        }
      }
    }
  } // End while
  return Changed;
}

/// Treat integer as of generic address space. Expand ptrtoint to a bitcast
/// plus a new ptrtoint.
///
/// Example
/// From:
///     %1 = ptrtoint i8 addrspace(1)* %0 to i64
/// To:
///     %tmp = bitcast i8 addrspace(1)* %0 to i8 addrspace(4)*
///     %1 = ptrtoint i8 addrspace(4)* %tmp to i64
bool HSAILAddrSpaceCast::expandPtrToIntInst(PtrToIntInst *PTII) {
  if (PTII->getPointerAddressSpace() != HSAILAS::FLAT_ADDRESS) {
    Value *Src = PTII->getPointerOperand();
    assert (!Src->getType()->isVectorTy());
    PointerType *SrcPtrTy = cast<PointerType>(Src->getType());
    Type *SrcElemTy = SrcPtrTy->getElementType();
    Type *CastTy = PointerType::get(SrcElemTy, HSAILAS::FLAT_ADDRESS);
    PTII->setOperand(PTII->getPointerOperandIndex(),
                new BitCastInst(Src, CastTy, PTII->getName() + ".stof", PTII));
    return true;
  }
  return false;
}

/// Treat integer as of generic address space. Expand inttoptr to a new
/// inttoptr plus a bitcast.
///
/// Example
/// From:
///      %1 = inttoptr i32 %0 to i8 addrspace(1)*
/// To:
///     %tmp = inttoptr i32 %0 to i8 addrspace(1)*
///     %1 = bitcast i8 addrspace(4)* %tmp to i8 addrspace(4)*
bool HSAILAddrSpaceCast::expandIntToPtrInst(IntToPtrInst *ITPI) {
  if (ITPI->getAddressSpace() != HSAILAS::FLAT_ADDRESS) {
    PointerType *DstPtrTy = cast<PointerType>(ITPI->getType());
    Type *DstElemTy = DstPtrTy->getElementType();
    Type *CastTy = PointerType::get(DstElemTy, HSAILAS::FLAT_ADDRESS);
    IntToPtrInst *NewITPI = new IntToPtrInst(ITPI->getOperand(0),
                                             CastTy, ITPI->getName(), ITPI);
    ITPI->replaceAllUsesWith(new BitCastInst(NewITPI, ITPI->getType(),
                                             ITPI->getName() + ".ftos", ITPI));
    return true;
  }
  return false;
}

/// Return true if the pointer of inttoptr/ptrtoint points to generic address.
static bool hasGenericPtr(Instruction *I) {
  if (IntToPtrInst *ITPI = dyn_cast<IntToPtrInst>(I)) {
    return ITPI->getAddressSpace() == HSAILAS::FLAT_ADDRESS;
  }

  PtrToIntInst *PTII = cast<PtrToIntInst>(I);
  return PTII->getPointerAddressSpace() == HSAILAS::FLAT_ADDRESS;
}

/// Do AddrSpaceCast related optimnization and lowering.
bool HSAILAddrSpaceCast::runOnFunction(Function &F) {
  bool HasGenericPtr = false;
  std::vector<Instruction *> PtrIntList;
  bool Changed = false;
  for (Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B) {
    for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE;) {
      Instruction *CurInst = I++;
      if (isa<CallInst>(CurInst)) {
        Changed |= optimizeGenericAddrBuiltin(CurInst);
      } else if (isa<IntToPtrInst>(CurInst) || isa<PtrToIntInst>(CurInst)) {
        HasGenericPtr |= hasGenericPtr(CurInst);
        PtrIntList.push_back(CurInst);
      } else if (!DisableAddrSpaceCastOpts) {
        if (isa<LoadInst>(CurInst) || isa<AtomicRMWInst>(CurInst) ||
            isa<AtomicCmpXchgInst>(CurInst)) {
          Changed |= optimizeMemoryInst(CurInst, 0);
        } else if (isa<StoreInst>(CurInst)) {
          Changed |= optimizeMemoryInst(CurInst, 1);
        } 
      }
    }
  }

  // If we saw a generic pointer in any of the inttoptr/ptrtoint instructions,
  // we have to look at all inttoptr/ptrtoint to recover the AddrSpaceCast, if
  // necessary.
  // TODO: This has an assumption that a ptrtoint/inttoptr pair must be in the
  // same function. Is this true?
  if (HasGenericPtr) {
    while (PtrIntList.size()) {
      Instruction *PI = PtrIntList.back();
      PtrIntList.pop_back();
      if (IntToPtrInst *ITPI = dyn_cast<IntToPtrInst>(PI)) {
        Changed |= expandIntToPtrInst(ITPI);
      } else if (PtrToIntInst *PTII = dyn_cast<PtrToIntInst>(PI)) {
        Changed |= expandPtrToIntInst(PTII);
      }
    }
  }

  // Finally, lower any AddrSpaceCast to intrinsics to emit stof/ftos.
  Changed |= lowerAddrSpaceCastToIntrinsic(F);
  return Changed;
}
