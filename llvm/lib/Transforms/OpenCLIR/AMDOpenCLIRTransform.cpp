//===- AMDOpenCLIRTransform - Transform non-OpenCL-style IRs into OpenCL-style IRS ---===//
//
//                    The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------------------===//
// Transform  ARM-ABI based LLVM IR binary into SPIR.
//
//===---------------------------------------------------------------------------------===//

#define DEBUG_TYPE "openclir"
#include "llvm/Support/Debug.h"
#if !defined(NDEBUG)
#define DEBUGME (DebugFlag &&isCurrentDebugType(DEBUG_TYPE))
#else
#define DEBUGME 0
#endif
#include "AMDOpenCLIRTransform.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Constants.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/DerivedTypes.h"
#include "llvm/IRBuilder.h"
#include "llvm/Support/circular_raw_ostream.h"
#include "llvm/Attributes.h"
#include "llvm/ValueSymbolTable.h"

using namespace llvm;
using namespace openclir;

char OpenCLIRTransform::ID = 0;
INITIALIZE_PASS_BEGIN(OpenCLIRTransform, "openclirtransform",
                      "OpenCLIR Binary Transform", false, false);
INITIALIZE_PASS_END(OpenCLIRTransform, "openclirtransform",
                    "OpenCLIR Binary Transform", false, false);

OpenCLIRTransform::OpenCLIRTransform() : ModulePass(ID) {
  initializeOpenCLIRTransformPass(*PassRegistry::getPassRegistry());
}

static void dump_module(Module &M) { M.dump(); }

// Check whether MD contains the pair (Key, Val) as its operands.
static bool matchKeyVal(MDNode *MD, StringRef Key, StringRef Val) {
  int NumOpnds = MD->getNumOperands();
  for (int i = 0; i < NumOpnds - 1; i++) {
    Value *Opnd = MD->getOperand(i);
    if (MDString *MDStr = dyn_cast<MDString>(Opnd)) {
      StringRef Str = MDStr->getString();
      if (Str.equals(Key)) {
        Opnd = MD->getOperand(i + 1);
        MDStr = dyn_cast<MDString>(Opnd);
        if (MDStr) {
          StringRef Str = MDStr->getString();
          if (Str.equals(Val))
            return true;
        }
      }
    }
  }
  return false;
}

struct NamePair {
  const char *first;
  const char *second;
};

NamePair LLVMTyToOpenCLTy[] = {
  { "i1", "bool" }, { "i8", "char" }, { "u8", "uchar" }, { "i16", "short" },
  { "u16", "ushort" }, { "i32", "int" }, { "u32", "uint" }, { "i64", "long" },
  { "u64", "ulong" }, { "f32", "float" }, { "f64", "double" },
  { "f16", "half" }, { NULL, NULL }
};

static void printOpenCLType(Type *Ty, raw_ostream &OS) {
  switch (Ty->getTypeID()) {
  case Type::PointerTyID: {
    PointerType *PTy = cast<PointerType>(Ty);
    printOpenCLType(PTy->getElementType(), OS);
    if (unsigned AddressSpace = PTy->getAddressSpace())
      OS << " addrspace(" << AddressSpace << ')';
    OS << '*';
    break;
  }
  case Type::VectorTyID: {
    VectorType *VTy = cast<VectorType>(Ty);
    int Size = VTy->getNumElements();
    Type *ETy = VTy->getElementType();
    printOpenCLType(ETy, OS);
    OS << Size;
    break;
  }
  case Type::IntegerTyID:
  case Type::FloatTyID:
  case Type::DoubleTyID: {
    std::string Name;
    raw_string_ostream TOS(Name);
    Ty->print(TOS);
    std::string Str = TOS.str();
    bool Found = false;
    for (NamePair *ETy = LLVMTyToOpenCLTy; ETy->first != NULL; ++ETy) {
      if (!Str.compare(ETy->first)) {
        OS << ETy->second;
        Found = true;
        break;
      }
    }
    if (!Found)
      Ty->print(OS);
    break;
  }
  default:
    Ty->print(OS);
  }
}

// Add call to get_global_id(0)
void OpenCLIRTransform::addGlobalId(Function *F) {
  IRBuilder<> Builder(F->getEntryBlock().begin());
  LLVMContext &Context = curModule->getContext();
  Function *Func = curModule->getFunction("_Z13get_global_idj");
  Type *RetTy = Type::getInt32Ty(Context);

  if (!Func) {
    std::vector<Type *> Args;
    Args.push_back(RetTy);
    FunctionType *FuncTy = FunctionType::get(RetTy, Args, false);
    Func = Function::Create(FuncTy, Function::ExternalLinkage, 
                            "_Z13get_global_idj",
                            curModule);
    SmallVector<AttributeWithIndex, 8> attrib;
    Attributes Attrs = Attributes::get(Context, Attributes::NoUnwind);
    attrib.push_back(AttributeWithIndex::get(~0, Attrs));
    Func->setAttributes(AttrListPtr::get(Context, attrib));
  }

  std::vector<Value *> Ops;
  Ops.push_back(ConstantInt::get(RetTy, 0));
  CallInst *Call = Builder.CreateCall(Func, Ops);

  for (Function::arg_iterator ab = F->arg_begin(), ae = F->arg_end(); ab != ae;
       ++ab) {
    Argument *ArgPtr = &(*ab);
    Value *Val = Builder.CreateGEP(ArgPtr, Call);
    for (Value::use_iterator I = ArgPtr->use_begin(), E = ArgPtr->use_end();
         I != E; I++) {
      Use *Use = &I.getUse();
      User *User = Use->getUser();
      if (User != Val) {
        User->replaceUsesOfWith(ArgPtr, Val);
      }
    }
  }
}

void OpenCLIRTransform::emitSPIR(llvm::Function *F) {
  LLVMContext &Context = curModule->getContext();
  SmallVector<llvm::Value *, 5> kernelMDArgs;

  // MDNode for the kernel argument address space qualifiers.
  SmallVector<llvm::Value *, 8> addressQuals;
  addressQuals.push_back(llvm::MDString::get(Context, "kernel_arg_addr_space"));

  // MDNode for the kernel argument access qualifiers.
  SmallVector<llvm::Value *, 8> accessQuals;
  accessQuals.push_back(llvm::MDString::get(Context, "kernel_arg_access_qual"));

  // MDNode for the kernel argument type names.
  SmallVector<llvm::Value *, 8> argTypeNames;
  argTypeNames.push_back(llvm::MDString::get(Context, "kernel_arg_type"));

  // MDNode for the kernel argument type qualifiers.
  SmallVector<llvm::Value *, 8> argTypeQuals;
  argTypeQuals.push_back(llvm::MDString::get(Context, "kernel_arg_type_qual"));

  // MDNode for the kernel argument names.
  SmallVector<llvm::Value *, 8> argNames;
  argNames.push_back(llvm::MDString::get(Context, "kernel_arg_name"));

  std::vector<Type *> NewArgTypes;
  IRBuilder<> Builder(Context);
  int ParamCnt = 0;
  for (Function::arg_iterator ab = F->arg_begin(), ae = F->arg_end(); ab != ae;
       ++ab) {
    Type *ArgType = ab->getType();
    ParamCnt++;
    std::string TypeName;
    raw_string_ostream OS(TypeName);
    std::string TypeQual;
    Attributes Attrs = Attributes::get(Context, Attributes::ReadOnly);

    if (F->getAttributes().paramHasAttr(ParamCnt, Attrs))
      TypeQual = "const";

    if (PointerType *Ptr = dyn_cast<PointerType>(ArgType)) {
      Type *ContainedTy = Ptr->getContainedType(0);
      // Get argument type name
      printOpenCLType(Ptr, OS);
      // Hardwire kernel argument's address space as global.
      PointerType *NewPtr = ContainedTy->getPointerTo(AS_GLOBAL);
      ab->mutateType(NewPtr);
      Type *NewArgType = ab->getType();
      NewArgTypes.push_back(NewArgType);
      // Get address qualifier.
      unsigned AddrSpace = NewArgType->getPointerAddressSpace();
      addressQuals.push_back(Builder.getInt32(AddrSpace));
      if (F->doesNotAlias(ParamCnt)) {
        TypeQual += "restrict";
      }
    } else {
      addressQuals.push_back(Builder.getInt32(0));
      // Get argument type name
      printOpenCLType(ArgType, OS);
    }

    // Remove all spaces.
    std::string Str = OS.str();
    std::string::size_type pos = Str.find(" ");
    while (pos != std::string::npos) {
      Str.erase(pos, 1);
      pos = Str.find(" ");
    }

    argTypeNames.push_back(llvm::MDString::get(Context, Str));
    argTypeQuals.push_back(llvm::MDString::get(Context, TypeQual));

    // Get access qualifier
    accessQuals.push_back(llvm::MDString::get(Context, "none"));
    // Get argument name.
    argNames.push_back(llvm::MDString::get(Context, ab->getName()));
  }

  FunctionType *NewFTy =
      FunctionType::get(F->getReturnType(), NewArgTypes, F->isVarArg());
  F->mutateType(PointerType::getUnqual(NewFTy));
  F->setCallingConv(CallingConv::SPIR_KERNEL);

  kernelMDArgs.push_back(F);
  kernelMDArgs.push_back(llvm::MDNode::get(Context, addressQuals));
  kernelMDArgs.push_back(llvm::MDNode::get(Context, accessQuals));
  kernelMDArgs.push_back(llvm::MDNode::get(Context, argTypeNames));
  kernelMDArgs.push_back(llvm::MDNode::get(Context, argTypeQuals));
  kernelMDArgs.push_back(llvm::MDNode::get(Context, argNames));

  MDNode *kernelMDNode = MDNode::get(Context, kernelMDArgs);
  NamedMDNode *OpenCLKernelMetadata =
      curModule->getOrInsertNamedMetadata("opencl.kernels");
  OpenCLKernelMetadata->addOperand(kernelMDNode);

  // Add SPIR version (1.2)
  Value *SPIRVerElts[] = {
    ConstantInt::get(Type::getInt32Ty(curModule->getContext()), 1),
    ConstantInt::get(Type::getInt32Ty(curModule->getContext()), 2)
  };
  llvm::NamedMDNode *SPIRVerMD =
    curModule->getOrInsertNamedMetadata("opencl.spir.version");
  SPIRVerMD->addOperand(llvm::MDNode::get(curModule->getContext(), SPIRVerElts));
}

bool OpenCLIRTransform::parseMetaData(Module &M) {
  bool flag = false;
  for (Module::const_named_metadata_iterator I = M.named_metadata_begin(),
                                             E = M.named_metadata_end();
       I != E; ++I) {
    const NamedMDNode *NMD = I;
    StringRef Name = NMD->getName();
    int NumOpnds = NMD->getNumOperands();

    if (Name.startswith("#pragma")) {
      bool VerVerified = false;
      for (int i = 0; i < NumOpnds; i++) {
        MDNode *Opnd = NMD->getOperand(i);
        if (matchKeyVal(Opnd, StringRef("version"), StringRef("1"))) {
          VerVerified = true;
          break;
        }
      }
      if (!VerVerified) {
        assert(false && "Unsupported Renderscript version!");
        return false;
      }
    } else if (Name.startswith("#rs_export_foreach_name")) {
      for (int i = 0; i < NumOpnds; i++) {
        MDNode *Opnd = NMD->getOperand(i);
        Value *Val = Opnd->getOperand(0);
        MDString *MDStr = dyn_cast<MDString>(Val);
        if (MDStr) {
          Function *Func = M.getFunction(MDStr->getString());
          if (Func) {
            flag = true;
            emitSPIR(Func);
            addGlobalId(Func);
          }
        }
      }
    }
  }
  
  if (!flag) return false;

  return true;
}

void setCallingConv(Module &M) {
  for (Module::iterator I = M.begin(), E = M.end(); I != E; ++ I) {
    if (I->getCallingConv() != CallingConv::SPIR_KERNEL) {
      I->setCallingConv(CallingConv::SPIR_FUNC);
      for (Value::use_iterator ui = I->use_begin(), ue = I->use_end();
           ui != ue; ++ui) {
        if (CallInst *Inst = dyn_cast<CallInst>(*ui)) {
          Inst->setCallingConv(CallingConv::SPIR_FUNC);
        }
      }
    }
  }

  return;
}

bool OpenCLIRTransform::runOnModule(Module &M) {
  curModule = &M;

  if (!parseMetaData(M))
    return false;

  setCallingConv(M);

  llvm::PassManager SPIRPasses;
  const std::string OldTriple = M.getTargetTriple();
  const char *TripleStr = OldTriple.c_str();
  const char *NewTriple = "spir-unknown-unknown";
  M.setTargetTriple(NewTriple);
  SPIRPasses.add(new llvm::DataLayout(M.getDataLayout()));
  SPIRPasses.run(M);

  return true;
}

// createOpenCLIR - Public interface to this file
ModulePass *llvm::createOpenCLIRTransform() { return new OpenCLIRTransform(); }
