#include "HSAIL.h"
#include "HSAILMetadataUtils.h"
#include "HSAILUtilityFunctions.h"

#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

namespace {
  const unsigned int NumOffsets = 3;
  const std::string GlobalOffset0 = "global_offset_0";
  const std::string GlobalOffset1 = "global_offset_1";
  const std::string GlobalOffset2 = "global_offset_2";

  const std::string GlobalOffsetIntrinsic = "__hsail_get_global_offset";
  const std::string GlobalIDIntrinsic = "__hsail_get_global_id";

  /// \brief Modify all kernels in a module to accept global offsets
  class HSAILGlobalOffsetInsertionPass : public ModulePass {
  public:
    static char ID;
    explicit HSAILGlobalOffsetInsertionPass() : ModulePass(ID), HSATM(0) {};
    explicit HSAILGlobalOffsetInsertionPass(HSAILTargetMachine *HSATM)
      : ModulePass(ID), HSATM(HSATM) {};

  private:
    const HSAILTargetMachine *HSATM;
    DebugInfoManager DIManager;

    virtual bool runOnModule(Module &M);

    virtual const char* getPassName() const {
      return "Insert OpenCL global offsets as HSAIL kernargs";
    }
  };
} // end anonymous namespace

char HSAILGlobalOffsetInsertionPass::ID = 0;

INITIALIZE_PASS(HSAILGlobalOffsetInsertionPass, "hsail-global-offsets",
                "Insert OpenCL global offsets as HSAIL kernargs",
                false, false);

ModulePass* llvm::createHSAILGlobalOffsetInsertionPass(HSAILTargetMachine &HSATM) {
  return new HSAILGlobalOffsetInsertionPass(&HSATM);
}

/// \brief Force inlining even if optimizations are off.
///
/// This is a workaround used by debugger tests when inlining is
/// disabled. It allows us to insert additional arguments into the
/// kernel by first inlining the two function. The workaround assumes
/// that these functions are only called from a kernel, and not from a
/// user function.
static void inlineGlobalIDFuncs(Module &M)
{
  InlineFunctionInfo ignored;
  Function *Func = NULL;

  // Look for the function under both mangling schemes: EDG and SPIR
  Func = M.getFunction("get_global_offset");
  if (!Func) Func = M.getFunction("_Z17get_global_offsetj");
  if (Func) {
    assert(!Func->empty() &&
           "missing definition for get_global_offset()");
    while (!Func->use_empty()) {
      CallInst *Call = cast<CallInst>(Func->use_back());
      Function *Caller = Call->getParent()->getParent();
      assert(isKernelFunc(Caller) &&
             "debugger workaround when inlining is disabled: "
             "can't call get_global_offset() from non-kernel function");
      bool success = InlineFunction(Call, ignored, false);
      assert(success && "Unable to inline get_global_offset()");
    }
    Func->eraseFromParent();
  }

  // Look for the function under both mangling schemes: EDG and SPIR
  Func = M.getFunction("get_global_id");
  if (!Func) Func = M.getFunction("_Z13get_global_idj");
  if (Func) {
    assert(!Func->empty() &&
           "missing definition for get_global_id()");
    while (!Func->use_empty()) {
      CallInst *Call = cast<CallInst>(Func->use_back());
      Function *Caller = Call->getParent()->getParent();
      assert(isKernelFunc(Caller) &&
             "debugger workaround when inlining is disabled: "
             "can't call get_global_id() from non-kernel function");
      bool success = InlineFunction(Call, ignored, false);
      assert(success && "Unable to inline get_global_id()");
    }
    Func->eraseFromParent();
  }
}

/// \brief Copy attributes from old function to new
///
/// The AttrListPtr is an opaque structure in LLVM 3.0, that contains
/// the attributes for the function, its return value and parameters.
/// These attributes have to be accessed from hard-coded locations as
/// seen in the body below.
static void adjustAttributes(Function *From, Function *To)
{
  const AttrListPtr &PAL = From->getAttributes();
  AttrListPtr NewPAL;

  // Copy the result attribute
  NewPAL = NewPAL.addAttr(To->getContext(),0, PAL.getRetAttributes());

  // Move attributes for existing arguments by three places. Note that
  // argument indices in the attributes start with a 1, not 0.
  for (unsigned i = 1, e = From->arg_size() + 1; i != e; ++i) {
    NewPAL = NewPAL.addAttr(To->getContext(),i + NumOffsets, PAL.getParamAttributes(i));
  }

  // Copy function attributes
  NewPAL = NewPAL.addAttr(To->getContext(),~0U, PAL.getFnAttributes());
  
  // This copies some additional attributes, not handled in the
  // earlier code. Do this before updating the main attributes.
  To->copyAttributesFrom(From);

  To->setAttributes(NewPAL);
}

/// \brief Insert the body of the given function into a new function
///        with three extra arguments.
///
/// It is not possible in LLVM to modify the signature of an existing
/// Function instance. Instead we create a new instance with the
/// appropriate FunctionType and then "transplant" the body of the old
/// instance to the new.
static Function* insertNewArguments(Function *F, Type *SizeTType)
{
  FunctionType *FType = F->getFunctionType();

  std::vector<Type*> ArgTypes;
  ArgTypes.insert(ArgTypes.end(), NumOffsets, SizeTType);
  ArgTypes.insert(ArgTypes.end(), FType->param_begin(), FType->param_end());

  FunctionType *NewFType = FunctionType::get(FType->getReturnType(), ArgTypes,
                                             F->getFunctionType()->isVarArg());
  Function *NewF = Function::Create(NewFType, F->getLinkage());

  NewF->setCallingConv(F->getCallingConv());
  adjustAttributes(F, NewF);
  
  // Splice the body of the old Function into the new Function. Note
  // that the old function becomes empty after this.
  NewF->getBasicBlockList().splice(NewF->begin(), F->getBasicBlockList());

  Function::arg_iterator NewI = NewF->arg_begin();
  NewI->setName(GlobalOffset0); ++NewI;
  NewI->setName(GlobalOffset1); ++NewI;
  NewI->setName(GlobalOffset2); ++NewI;

  // Replace old arguments with new ones. We reuse the NewI iterator
  // which has already skipped the first three new arguments.
  for (Function::arg_iterator OldI = F->arg_begin(), OldE = F->arg_end();
       OldI != OldE; ++OldI, ++NewI) {
    OldI->replaceAllUsesWith(NewI);
    NewI->takeName(OldI);
  }

  NewF->takeName(F);
  return NewF;
}

static void addOffsetToID(Function *IDF, Function *OFF)
{
  for (Function::use_iterator UI = IDF->use_begin(), UE = IDF->use_end();
       UI != UE; ++UI) {
    CallInst *Call = cast<CallInst>(*UI);
    BasicBlock *BB = Call->getParent();
    Function *F = BB->getParent();

    ConstantInt *CallArg = cast<ConstantInt>(Call->getArgOperand(0));
    unsigned Dim = CallArg->getZExtValue();
    assert(Dim < NumOffsets);

    Value *Args[] = { CallArg };
    CallInst *CallOffset = CallInst::Create(OFF, Args, "", Call);


    BinaryOperator  *GID;
    if (Call->getType() != CallOffset->getType()) {
      // workaround for EPR# 383087.
      // Fixes the mismatch between the return type of get_global_id(), 
      // which is 64-bit for large models, while the extra kernel argument 
      // to store global offset is 32-bit.
      // A typecast for the argument is inserted as a workaround.
      CastInst *CI = CastInst::CreateIntegerCast(CallOffset, Call->getType(),
                                                          false, "", Call);
      GID = BinaryOperator::Create(Instruction::Add,Call, CI);
    }
    else {
     GID = BinaryOperator::Create(Instruction::Add,Call, CallOffset);
    }

    BB->getInstList().insertAfter(Call, GID);
    Call->replaceAllUsesWith(GID);
    GID->setOperand(0, Call);
  }
}

/// \brief Replace calls get_global_offset() with access to
///        appropriate kernel arguments
///
/// The kernel argument to access is identified by the argument to
/// get_global_offset(), which must be a constant in the range [0..2].
/// 
/// Assumption: The only use of the function is in call instructions.
static void replaceCallsWithKernelArgument(Function *GF)
{
  while (!GF->use_empty()) {
    CallInst *Call = cast<CallInst>(GF->use_back());
    Function *F = Call->getParent()->getParent();

    assert(isKernelFunc(F));

    ConstantInt *CallArg0 = cast<ConstantInt>(Call->getArgOperand(0));
    unsigned Dim = CallArg0->getZExtValue();
    assert(Dim < NumOffsets);

    // Pretty lousy way to access the i'th Argument of a Function.
    Function::arg_iterator AI = F->arg_begin();
    for (unsigned i = 0; i < Dim; ++i)
      ++AI;

    CastInst *CI = CastInst::CreateIntegerCast(AI, Call->getType(),
                                               false, "", Call);
    Call->replaceAllUsesWith(CI);
    Call->eraseFromParent();
  }
}

/// \brief Update the "argtypenames" variable for the new function
///
/// LLVM metadata uses one global constant per function to store
/// information about the function arguments. This is an array of
/// strings that correspond to the function arguments. Each string is
/// itself a global constant, and stored in the array as a bitcast to
/// char* pointer.
///
/// We recreate this variable with three new elements at the
/// beginning. Recreation is necessary because the type of a
/// ConstantArray changes when its size changes.
static void updateEDGArgTypeNames(Module &M, Function *To)
{
  SmallString<128> ArgTypeName("llvm.argtypename.annotations.");
  ArgTypeName += To->getName();

  GlobalVariable *GV = M.getNamedGlobal(ArgTypeName);
  if (!GV)
    return;

  ConstantArray *CA = dyn_cast_or_null<ConstantArray>(GV->getInitializer());
  if (!CA)
    return;

  // Look at the first operand (it's a bitcast) and retrieve the
  // global string constant that it points to.
  ConstantExpr *FirstCE = cast<ConstantExpr>(CA->getOperand(0));
  assert(Instruction::BitCast == FirstCE->getOpcode());
  GlobalVariable *First = cast<GlobalVariable>(FirstCE->getOperand(0));
  PointerType *FirstTy = cast<PointerType>(First->getType());

  // We need to create another string just like this one.
  Constant *SizeTString = ConstantDataArray::getString(M.getContext(), "size_t");
  GlobalVariable *SizeTGV
    = new GlobalVariable(M, SizeTString->getType(), First->isConstant(),
                         First->getLinkage(), SizeTString, ".str", First,
                         First->getThreadLocalMode(), FirstTy->getAddressSpace());
  Constant *SizeTBitCast = ConstantExpr::getBitCast(SizeTGV, FirstCE->getType());

  // Initialize a vector with three copies of the new constant
  SmallVector<Constant*, 16> Elements(NumOffsets, SizeTBitCast);

  // Now append all the existing string constants
  for (unsigned i = 0, e = CA->getNumOperands(); i != e; ++i) {
    Constant *C = CA->getOperand(i);
    Elements.append(1, C);
  }

  // Create the new constant initializer
  ArrayType *NewInitType = ArrayType::get(FirstCE->getType(), Elements.size());
  Constant *NewInit = ConstantArray::get(NewInitType, Elements);
  
  // Create a new global variable
  PointerType *GVTy = cast<PointerType>(GV->getType());
  GlobalVariable *NewGV
    = new GlobalVariable(M, NewInitType, GV->isConstant(), GV->getLinkage(),
                         NewInit, "", GV, GV->getThreadLocalMode(),
                         GVTy->getAddressSpace());

  // Replace the existing variable.
  NewGV->takeName(GV);
  GV->eraseFromParent();
}

/// \brief Update the "kernel_arg_types" SPIR metadata if it exists.
static void updateSPIRArgTypeNames(Module &M, const Function *F) {
  NamedMDNode *MD = M.getNamedMetadata("opencl.kernels");
  assert(MD);

  // Locate the metadata for the kernel.
  MDNode *KernelData = NULL;
  for (unsigned i = 0, e = MD->getNumOperands(); i != e; ++i) {
    MDNode *KD = cast<MDNode>(MD->getOperand(i));
    if (KD->getOperand(0) != F) continue;
    KernelData = KD;
    break;
  }
  assert(KernelData);

  // Locate the arg type names within the kernel metadata.
  MDNode *ArgNames = NULL;
  unsigned Count = 1;
  // skip the first operand, which is the kernel signature
  for (unsigned e = KernelData->getNumOperands(); Count != e; ++Count) {
    MDNode *Node = cast<MDNode>(KernelData->getOperand(Count));
    if (!matchName(Node, "kernel_arg_type")) continue;
    ArgNames = Node;
    break;
  }
  if (!ArgNames) return;

  // Create a copy of the arg type names,and insert three instances of
  // "size_t" at the beginning.
  MDString* SizeT = MDString::get(M.getContext(), "size_t");
  SmallVector<Value*, 16> Vals;
  Vals.push_back(ArgNames->getOperand(0));
  Vals.append(3, SizeT);
  for (unsigned i = 1, e = ArgNames->getNumOperands(); i != e; ++i) {
    Vals.push_back(ArgNames->getOperand(i));
  }
  KernelData->replaceOperandWith(Count, MDNode::get(M.getContext(), Vals));
}

bool HSAILGlobalOffsetInsertionPass::runOnModule(Module &M)
{
  DIManager.collectFunctionDIs(M);

  inlineGlobalIDFuncs(M);

  Type *SizeT = HSATM->Subtarget.is64Bit()
    ? Type::getInt64Ty(M.getContext())
    : Type::getInt32Ty(M.getContext());

  for (Module::iterator II = M.begin(), IE = M.end();
       II != IE;) {
    Function *F = II++; // Note the post-increment operator
  
    if (!isKernelFunc(F)) continue;

    Function *NewF = insertNewArguments(F, SizeT);

    DIManager.replaceFunctionDI(F, NewF);

    if (isSPIRModule(M)) {
      updateSPIRMetadata(M, F, NewF);
      updateSPIRArgTypeNames(M, NewF);
    } else {
      updateFunctionAnnotations(M, F, NewF);
      updateEDGArgTypeNames(M, NewF);
    }

    M.getFunctionList().insert(F, NewF); // Insert before F
    F->eraseFromParent();
  }

  Function *OffsetFunc = M.getFunction(GlobalOffsetIntrinsic);
  Function *IDFunc = M.getFunction(GlobalIDIntrinsic);

  if (IDFunc) {
    if (!OffsetFunc) {
      OffsetFunc = cast<Function>(M.getOrInsertFunction(GlobalOffsetIntrinsic,
                                                        IDFunc->getFunctionType(),
                                                        IDFunc->getAttributes()));
    }
    addOffsetToID(IDFunc, OffsetFunc);
  }

  if (OffsetFunc)
    replaceCallsWithKernelArgument(OffsetFunc);

  return true;
}

