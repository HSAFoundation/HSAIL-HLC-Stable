//=== HSAILKernelArgInsertion.cpp -- For openCL -- Add printf, vqueue, aqlwrap
// as the default arguments to every kernel.
//===----------------------------------------------------------------------===//
//

#define DEBUG_TYPE "printfToRuntime"
#include "HSAIL.h"
#include "HSAILMachineFunctionInfo.h"
#include "HSAILUtilityFunctions.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/AMDLLVMContextHook.h"
#include "llvm/Function.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/AMDMetadataUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

namespace {
  const int NumNewArgs = 3;
  class HSAILPrintfRuntimeBindingKernArg : public ModulePass {
  public:
    static char ID;
    explicit HSAILPrintfRuntimeBindingKernArg() : ModulePass(ID), HSATM(0) {};
    explicit HSAILPrintfRuntimeBindingKernArg(HSAILTargetMachine *HSATM)
      : ModulePass(ID), HSATM(HSATM) {};

    virtual bool runOnModule(Module &M);
  private:
    const HSAILTargetMachine *HSATM;
    DebugInfoManager DIManager;

    virtual const char* getPassName() const {
      return "HSAIL Printf lowering part 2";
    }
    bool confirmOpenCLVersion200(Module& M) const;
    bool confirmSpirModule(Module& M) const;
  };
} // end anonymous namespace

INITIALIZE_PASS(HSAILPrintfRuntimeBindingKernArg,
                "hsail-printf-binding-kernarg",
                "Insert printf buffer HSAIL kernargs",
                false, false);

ModulePass* llvm::createHSAILPrintfRuntimeBindingKernArg(
  HSAILTargetMachine &HSATM) {
  return new HSAILPrintfRuntimeBindingKernArg(&HSATM);
}

char HSAILPrintfRuntimeBindingKernArg::ID = 0;
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
    NewPAL = NewPAL.addAttr(To->getContext(),
                            i + NumNewArgs, PAL.getParamAttributes(i));
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
static Function* insertNewArguments (Function *F, Type* to_insert, Type *SizeT) {
  FunctionType *FType = F->getFunctionType();

  std::vector<Type*> ArgTypes;
  ArgTypes.push_back(to_insert);
  ArgTypes.push_back(SizeT);
  ArgTypes.push_back(SizeT);
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
  NewI->setName("printf_buffer"); ++NewI;
  NewI->setName("vqueue_pointer"); ++NewI;
  NewI->setName("aqlwrap_pointer"); ++NewI;

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
static void updateEDGArgTypeNames(Module &M, Function *To) {
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
  SmallVector<Constant*, 16> Elements(NumNewArgs, SizeTBitCast);

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
  if (!KernelData) return;

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

  // Create a copy of the arg type names,and insert an instance of
  // "size_t" at the beginning.
  MDString* SizeT = MDString::get(M.getContext(), "size_t");
  SmallVector<Value*, 16> Vals;
  Vals.push_back(ArgNames->getOperand(0));
  Vals.push_back(SizeT);
  Vals.push_back(SizeT);
  Vals.push_back(SizeT);
  for (unsigned i = 1, e = ArgNames->getNumOperands(); i != e; ++i) {
    Vals.push_back(ArgNames->getOperand(i));
  }
  KernelData->replaceOperandWith(Count, MDNode::get(M.getContext(), Vals));
}

bool HSAILPrintfRuntimeBindingKernArg::confirmSpirModule(Module& M) const {
  NamedMDNode *SPIRVersion = M.getNamedMetadata("opencl.spir.version");
  if (!SPIRVersion) return false;
  else return true;
}

bool HSAILPrintfRuntimeBindingKernArg::confirmOpenCLVersion200(Module& M)
  const {
  NamedMDNode *OCLVersion = M.getNamedMetadata("opencl.ocl.version");
  if (!OCLVersion) {
    return false;
  }
  if (OCLVersion->getNumOperands() < 1) {
    return false;
  }
  MDNode *ver = OCLVersion->getOperand(0);
  if (ver->getNumOperands() != 2) {
    return false;
  }
  ConstantInt *major = dyn_cast<ConstantInt>(ver->getOperand(0));
  ConstantInt *minor = dyn_cast<ConstantInt>(ver->getOperand(1));
  if (0 == major || 0 == minor) {
    return false;
  }
  if (major->getZExtValue() == 2) {
    return true;
  } else {
    return false;
  }
}

bool HSAILPrintfRuntimeBindingKernArg::runOnModule(Module &M) {
  if (!confirmOpenCLVersion200(M)) return false;
  if (!confirmSpirModule(M)) return false;
  DIManager.collectFunctionDIs(M);
  LLVMContext &Ctx = M.getContext();
  Type *I8Ptr = PointerType::get( Type::getInt8Ty(Ctx), 1);

  Type *SizeT = HSATM->Subtarget.is64Bit()
    ? Type::getInt64Ty(M.getContext())
    : Type::getInt32Ty(M.getContext());

  for (Module::iterator II = M.begin(), IE = M.end();
       II != IE;) {
    Function *F = II++; // Note the post-increment operator

    if (!isKernelFunc(F)) continue;

    Function *NewF = insertNewArguments(F, I8Ptr, SizeT);

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

  int printf_uniqnum = 0;
  Function *printf_id = M.getFunction("__printf_id");
  while (printf_id && !printf_id->use_empty()) {
    printf_uniqnum++;
    CallInst *Call = cast<CallInst>(
                       printf_id->use_back());
    Type *Int32Ty = Type::getInt32Ty(Ctx);
    MDNode *idmd = Call->getMetadata("prnFmt");
    ConstantDataArray *CA = dyn_cast<ConstantDataArray>(idmd->getOperand(0));
    assert(CA && "printf_id meta data required in ConstantDataArray");
    StringRef str = CA->getAsCString();
    std::string mstr;
    raw_string_ostream strstrm(mstr);
    strstrm << "printf_fmt:";
    strstrm << printf_uniqnum << ':';
    strstrm << str.str();
    str = StringRef(strstrm.str().c_str());
    Constant *newConst = ConstantInt::getSigned(Call->getType(),
                                                printf_uniqnum);
    Instruction *use_beg = cast<Instruction>(*Call->use_begin());
    NamedMDNode *metaD = M.getOrInsertNamedMetadata("llvm.printf.fmts");
    Constant *fmtStrArray
        = ConstantDataArray::getString( Ctx, str, true);
    MDNode *myMD = MDNode::get(Ctx,fmtStrArray);
    metaD->addOperand(myMD);
    use_beg->setMetadata("prnFmt", myMD);
    Call->replaceAllUsesWith(newConst);
    Call->eraseFromParent();
    DEBUG(dbgs() << "taking metadata out of printf_id calls\n");
  }
  if (printf_id) printf_id->eraseFromParent();
  return true;
}
