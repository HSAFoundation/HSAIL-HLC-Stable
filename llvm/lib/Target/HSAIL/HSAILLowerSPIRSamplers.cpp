//===-- HSAILLowerSPIRSamplers.cpp - Lower SPIR samplers --------*- C++ -*-===//
//
//
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implement a pass that converts sampler types encoded as i32
///        in SPIR to HSAIL-friendly opaque types in LLVM IR.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "hsail-lower-samplers"

#include "HSAILUtilityFunctions.h"

#include <llvm/GlobalVariable.h>
#include <llvm/Module.h>
#include <llvm/ADT/SmallBitVector.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
#include "llvm/Transforms/AMDMetadataUtils.h"

#include <map>

using namespace llvm;

namespace {

  /// \brief Translate i32 samplers in SPIR to opaque pointers
  ///
  /// SPIR represents samplers as i32 values, and specifies the
  /// meaning of each bit in the value. But the HSAIL backend expects
  /// a pointer to an opaque struct, and so we need a way to detect
  /// and translate SPIR samplers into the corresponding opaque type.
  ///
  /// The only way to detect a sampler type is either at the kernel
  /// signature (by examining metadata) or at the sites that call an
  /// image builtin. The inferred type has to be propagated from there
  /// to entire callgraph. But with separate compilation, it may happen
  /// that a function or a sequence of functions may be called only from
  /// extern functions, and in turn calls only other extern functions.
  /// Any sampler types in such functions will be invisible to the
  /// lowering pass, producing an incompatible compilation unit.
  ///
  /// *** This is a limitation in SPIR 1.2. There is no workaround. ***
  ///
  /// The current implementation depends on the fact that currently,
  /// function calls are not supported in the HSAIL backend. Instead,
  /// the entire call-graph is inlined into each kernel. This greatly
  /// simplifies the task, since all we need to do is examine calls to
  /// the __hsail_rdimage intrinsic, and mark the values being passed
  /// in those calls.
  ///
  class HSAILLowerSPIRSamplers : public ModulePass {
  public:
    static char ID;
    explicit HSAILLowerSPIRSamplers() : ModulePass(ID), SamplerType(NULL) {}

  private:
    // Track sampler arguments per kernel.
    std::map<Function*, SmallBitVector> SamplerArgsMap;

    // Map old values to new values when updating the sampler type.
    ValueMap<Function*, Function*> IMap; // intrinsics
    ValueMap<Function*, Function*> KMap; // kernels

    void clearState(Module &M);
    void propagateSampler(Function *Intrinsic);
    void createNewIntrinsic(Function *F);
    void replaceSamplers();
    void updateKernelArgs(Function *F, SmallBitVector &Args);

    PointerType *SamplerType;
    Module *CurrentModule;
    DebugInfoManager DIManager;

    virtual bool runOnModule(Module &M);
  };

} // end anonymous namespace

char HSAILLowerSPIRSamplers::ID = 0;

INITIALIZE_PASS(HSAILLowerSPIRSamplers, "hsail-lower-spir-samplers",
                "Lower i32 SPIR samplers to HSAIL opaque types",
                false, false);

namespace llvm {
  ModulePass* createHSAILLowerSPIRSamplersPass() {
    return new HSAILLowerSPIRSamplers();
  }
}

/// \brief Propagate the sampler type "upwards" in the callgraph.
///
/// Once we know that a particular value is being passed to read_image
/// builtin, we can check its uses to see if it is a kernel argument
/// or a global variable.
void HSAILLowerSPIRSamplers::propagateSampler(Function *Intrinsic) {
  for (Function::use_iterator UI = Intrinsic->use_begin(),
         UE = Intrinsic->use_end(); UI != UE; ++UI) {
    CallInst *Call = cast<CallInst>(*UI);
    Function *Caller = Call->getParent()->getParent();

    assert(isKernelFunc(Caller)); // everything is inlined

    // Starting with "0", the second argument is a sampler.
    Value *Arg = Call->getArgOperand(1);

    if (Argument *CallerArg = dyn_cast<Argument>(Arg)) {
      // The caller passed its own argument forward.
      unsigned ArgNo = CallerArg->getArgNo();
      SamplerArgsMap[Caller].set(ArgNo);
      DEBUG(dbgs() << "marked argument " << ArgNo << " on kernel "
            << Caller->getName() << "\n");
    } else {
      // The caller passed a global variable, which eventually got
      // optimized to a constant integer.
      assert(isa<Constant>(Arg));
      DEBUG(dbgs() << "intrinsic received a constant: "
            << cast<ConstantInt>(Arg)->getZExtValue() << "\n");
    }
  }
}

/// \brief Update the intrinsic declaration itself to reflect the new
///        sampler type.
void HSAILLowerSPIRSamplers::createNewIntrinsic(Function *F) {
  if (IMap.count(F))
    return;

  DEBUG(dbgs() << "Original Intrinsic:"; F->dump());
  FunctionType *FType = F->getFunctionType();
  SmallVector<Type*, 8> ArgTypes(FType->param_begin(), FType->param_end());

  // Replace the second argument type since it is the sampler.
  assert(isa<IntegerType>(ArgTypes[1]));
  ArgTypes[1] = SamplerType;

  FunctionType *NewFType = FunctionType::get(FType->getReturnType(),
                                             ArgTypes, FType->isVarArg());
  // Create the new intrinsic, but don't insert in Module yet.
  Function *NewF = Function::Create(NewFType, F->getLinkage());
  NewF->setAttributes(F->getAttributes());
  NewF->takeName(F);
  IMap[F] = NewF;
  DEBUG(dbgs() << "New Intrinsic:"; NewF->dump());
}

/// \brief Create a new kernel body, updating all the arguments that
///        are known to be samplers.
///
/// This leaves the old empty body in the FunctionList, since all the
/// calls to the old kernel have not been replaced yet. The old body
/// will be deleted after all the uses have been updated.
void HSAILLowerSPIRSamplers::updateKernelArgs(Function *F,
                                              SmallBitVector &Args) {
  FunctionType *FType = F->getFunctionType();

  std::vector<Type*> ArgTypes;
  ArgTypes.insert(ArgTypes.end(), FType->param_begin(), FType->param_end());

  for (int I = Args.find_first(); I != -1; I = Args.find_next(I)) {
    ArgTypes[I] = SamplerType;
  }

  FunctionType *NewFType = FunctionType::get(FType->getReturnType(), ArgTypes,
                                             F->getFunctionType()->isVarArg());
  Function *NewF
    = Function::Create(NewFType, F->getLinkage(), "", CurrentModule);
  NewF->setCallingConv(F->getCallingConv());
  NewF->setAttributes(F->getAttributes());
  NewF->takeName(F);
  KMap[F] = NewF;

  // Splice the body of the old Function into the new Function. Note
  // that the old function becomes empty after this.
  NewF->getBasicBlockList().splice(NewF->begin(), F->getBasicBlockList());

  // Replace old arguments with new ones. Skip over the arguments that
  // are known to be samplers. After this, instructions in the new
  // kernel that use samplers still point to the sampler arguments in
  // the old kernel. These will be updated later.
  for (Function::arg_iterator OldI = F->arg_begin(), OldE = F->arg_end(),
         NewI = NewF->arg_begin();
       OldI != OldE; ++OldI, ++NewI) {
    NewI->takeName(OldI);
    if (Args[OldI->getArgNo()])
      continue;
    OldI->replaceAllUsesWith(NewI);
  }
}

// There is no method in the Function class to retrieve the n'th
// argument. We must do it the hard way.
Argument* getNthArgument(Function *F, unsigned n) {
  return next(F->arg_begin(), n);
}

///\brief Convert sampler value from SPIR enum to AMD enum
///
/// Lifted from AMDSPIRLoader.cpp
static uint16_t convertSamplerValue(uint16_t value) {
  unsigned newVal;
  newVal = value & 0xfff9;    // CLK_NORMALIZED_COORDS_TRUE
  value &= 0x0006;
  if (value == 6)             // CLK_ADDRESS_REPEAT
    newVal |= 2;
  else if (value == 4)        // CLK_ADDRESS_CLAMP
    newVal |= 6;
  else if (value == 2)        // CLK_ADDRESS_CLAMP_TO_EDGE
    newVal |= 4;
  return newVal;
}

/// \brief Convert SPIR i32 constant into EDG sampler
static Constant* getEDGSamplerExpr(Constant *C, Type *SamplerType,
                                   bool isSPIR) {
  ConstantInt *CInt = cast<ConstantInt>(C);
  uint16_t Init = CInt->getZExtValue();
  if (isSPIR)
    Init = convertSamplerValue(Init);
  ConstantInt *EInt = ConstantInt::get(CInt->getType(), Init);
  return ConstantExpr::getIntToPtr(EInt, SamplerType);
}

/// \brief Update the intrinsic and propagate upwards.
///
/// For each read-image intrinsic, we have created a new intrinsic
/// that takes the opaque type. Here we replace every call to the old
/// intrinsic with a call to the new intrinsic. While doing so, we
/// also update the samplers being used --- i32 constants are replaced
/// with EDG-style expressions, and arguments from the old kernel are
/// replaced with arguments from the new kernel.
///
/// After this transformation is done on all intrinsics, the
/// cross-over to the new kernel bodies is complete, and the old
/// kernels can be deleted eventually.
static void
replaceUsesAndArgs(Function *OldIntrinsic, Function *NewIntrinsic,
                   Type *SamplerType, bool isSPIR) {
  while (!OldIntrinsic->use_empty()) {
    CallInst *Call = cast<CallInst>(OldIntrinsic->use_back());

    // Gather all the arguments to the call, but drop the last
    // argument. It is actually the called function.
    SmallVector<Value*, 8> Args(Call->value_op_begin(),
                                Call->value_op_end());
    Args.pop_back();

    Value *SArg = Args[1];
    if (Constant *C = dyn_cast<Constant>(SArg)) {
      Args[1] = getEDGSamplerExpr(C, SamplerType, isSPIR);
    } else {
      Argument *A = cast<Argument>(SArg);
      Function *K = Call->getParent()->getParent();
      Args[1] = getNthArgument(K, A->getArgNo());
      assert(Args[1]->getType() == SamplerType);
    }

    CallInst *NewCall = CallInst::Create(NewIntrinsic, Args, "", Call);
    NewCall->setAttributes(Call->getAttributes());
    Call->replaceAllUsesWith(NewCall);
    Call->eraseFromParent();
  }
}

/// \brief Update the kernel/intrinsic call graph to use the new
/// sampler types. This is the main driver function that ties
/// everything together.
void HSAILLowerSPIRSamplers::replaceSamplers() {
  bool isSPIR = isSPIRModule(*CurrentModule);

  // First create the new kernels but with bodies still pointing to
  // the old sampler arguments in the old kernel.
  for (std::map<Function*, SmallBitVector>::iterator KI = SamplerArgsMap.begin(),
         KE = SamplerArgsMap.end(); KI != KE; ++KI) {
    Function *Kernel = KI->first;
    SmallBitVector &Args = KI->second;
    if (Args.empty()) // No sampler arguments
      continue;
    updateKernelArgs(Kernel, Args);
  }

  // Update the kernel bodies to use the new sampler arguments along
  // with the new intrinsics. The old intrinsics can then be deleted.
  for (ValueMap<Function*, Function*>::iterator II = IMap.begin(),
         IE = IMap.end(); II != IE; ++II) {
    Function *OldIntrinsic = II->first;
    Function *NewIntrinsic = II->second;

    replaceUsesAndArgs(OldIntrinsic, NewIntrinsic, SamplerType, isSPIR);
    CurrentModule->getFunctionList().insert(OldIntrinsic, NewIntrinsic);
    OldIntrinsic->eraseFromParent();
  }

  // The old kernels can now be deleted after updating metadata.
  for (ValueMap<Function*, Function*>::iterator KI = KMap.begin(),
         KE = KMap.end(); KI != KE; ++KI) {
    DIManager.replaceFunctionDI(KI->first, KI->second);
    if (isSPIR)
      updateSPIRMetadata(*CurrentModule, KI->first, KI->second);
    else
      updateFunctionAnnotations(*CurrentModule, KI->first, KI->second);

    KI->first->eraseFromParent();
  }
}

void HSAILLowerSPIRSamplers::clearState(Module &M) {
  SamplerArgsMap.clear();
  IMap.clear();
  KMap.clear();
  CurrentModule = &M;

  StructType *ST = CurrentModule->getTypeByName("struct._sampler_t");
  if (!ST)
    ST = StructType::create(CurrentModule->getContext(), "struct._sampler_t");
  assert(ST->isOpaque());
  DEBUG(dbgs() << "Retrieved named type: "; ST->dump());
  SamplerType = PointerType::get(ST, HSAILAS::CONSTANT_ADDRESS);
}

bool HSAILLowerSPIRSamplers::runOnModule(Module &M) {

  bool Changed = false;
  clearState(M);
  DIManager.collectFunctionDIs(M);

  for (Module::iterator FI = CurrentModule->begin(), FE = CurrentModule->end();
       FI != FE; ++FI) {
    Function *F = FI;

    if (isKernelFunc(F)) {
      // Need to initialize the SmallBitVector's early to prevent
      // out-of-bounds operations later.
      SamplerArgsMap[F].resize(F->arg_size());
      continue;
    }

    if (!F->getName().startswith("__hsail_rdimage"))
      continue;
    assert(F->isDeclaration());
    DEBUG(dbgs() << "examining intrinsic: " << F->getName() << "\n");

    createNewIntrinsic(F);
    propagateSampler(F);
    Changed = true;
  }

  replaceSamplers();

  return Changed;
}
