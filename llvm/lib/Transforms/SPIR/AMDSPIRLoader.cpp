#if defined(AMD_SPIR_LOADER) || 1
//===- AMDSPIRLoader.cpp - Load a SPIR binary for the device --------------===//
//===----------------------------------------------------------------------===//
//
// Load a SPIR binary for the given target triple and materialize into
// a valid LLVMIR binary.
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "spirloader"
#include "AMDSPIRLoader.h"
#include "AMDSPIRMutator.h"
#include "spir.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/SPIRVerifier.h"
#include "llvm/Constants.h"
#include "llvm/DataLayout.h"
#include "llvm/DerivedTypes.h"
#include "llvm/GlobalValue.h"
#include "llvm/InstrTypes.h"
#include "llvm/Instructions.h"
#include "llvm/IRBuilder.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Type.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/AMDMetadataUtils.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/AMDLLVMContextHook.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream> // TODO: Remove this
#include <iterator>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include "cxxabi.h"

#define verboseDebug(x) x
#define CHKASSERT(x,y) assert(x);

// This header file is required for generating global variables for kernel
// argument info for CPU.
namespace clk {
typedef unsigned int uint;
typedef uint32_t cl_mem_fence_flags;
//kernel arg access qualifier and type qualifier
typedef enum clk_arg_qualifier_t
{
    Q_NONE = 0,

    //for image type only, access qualifier
    Q_READ = 1,
    Q_WRITE = 2,

    //for pointer type only
    Q_CONST = 4, // pointee
    Q_RESTRICT = 8,
    Q_VOLATILE = 16,  // pointee
    Q_PIPE = 32  // pipe

} clk_arg_qualifier_t;

typedef enum clk_value_type_t
{
    T_VOID,   T_CHAR,   T_SHORT,  T_INT,
    T_LONG,   T_FLOAT,  T_DOUBLE, T_POINTER,
    T_CHAR2,  T_CHAR3,  T_CHAR4,  T_CHAR8,  T_CHAR16,
    T_SHORT2, T_SHORT3, T_SHORT4, T_SHORT8, T_SHORT16,
    T_INT2,   T_INT3,   T_INT4,   T_INT8,   T_INT16,
    T_LONG2,  T_LONG3,  T_LONG4,  T_LONG8,  T_LONG16,
    T_FLOAT2, T_FLOAT3, T_FLOAT4, T_FLOAT8, T_FLOAT16,
    T_DOUBLE2, T_DOUBLE3, T_DOUBLE4, T_DOUBLE8, T_DOUBLE16,
    T_SAMPLER, T_SEMA, T_STRUCT, T_QUEUE
} clk_value_type_t;

typedef enum clk_address_space_t
{
    A_PRIVATE, A_LOCAL, A_CONSTANT, A_GLOBAL, A_REGION
} clk_address_space_t;


// #include <amdocl/cl_kernel.h>
} // end of namespace clk

//#include <khronos/headers/opencl1.2/CL/spir.h>

using namespace llvm;
using namespace spir;

namespace AMDSpir {

#ifndef AMDIL_ADDR_SPACE_DEFINED
// Copied from AMDIL.h
enum AddressSpaces {
  PRIVATE_ADDRESS  = 0, // Address space for private memory.
  GLOBAL_ADDRESS   = 1, // Address space for global memory.
  CONSTANT_ADDRESS = 2, // Address space for constant memory.
  LOCAL_ADDRESS    = 3, // Address space for local memory.
  REGION_ADDRESS   = 4, // Address space for region memory.
  GLOBAL_HOST_ADDRESS = 5, // Address space with global host endianness.
  CONSTANT_HOST_ADDRESS = 6, // Address space with constant host endianness.
  FLAT_ADDRESS     = 7, // Address space for flat memory.
  ADDRESS_NONE     = 8  // Address space for unknown memory.
};
#define AMDIL_ADDR_SPACE_DEFINED 1
#endif

#ifndef AMDIL_IMAGE_ACCESS_TYPE_DEFINED
// ToDo: The following values are hardcoded in AMDILModuleInfo.cpp
// It is better to be defined in AMDIL.h
enum AMDILImageAccessType {
  IMAGE_TYPE_READONLY  = 1,
  IMAGE_TYPE_WRITEONLY = 2,
  IMAGE_TYPE_READWRITE = 3,
};
#define AMDIL_IMAGE_ACCESS_TYPE_DEFINED 1
#endif

// Map SPIR addr space to AMDIL address space
int mapSpirAddrSpace(int spirAddrSpace) {
  switch(spirAddrSpace) {
  case(CLS_PRIVATE_AS):       return PRIVATE_ADDRESS;
  case(CLS_GLOBAL_AS):        return GLOBAL_ADDRESS;
  case(CLS_CONSTANT_AS):      return CONSTANT_ADDRESS;
  case(CLS_LOCAL_AS):         return LOCAL_ADDRESS;
  default: assert("unexpected address space"==NULL); return -1;
  }
}

// Map SPIR addr space to clk address code
int mapSpirAddrSpaceToRuntimeValue(int spirAddrSpace) {
  switch(spirAddrSpace) {
  case(CLS_PRIVATE_AS):   return clk::A_PRIVATE;
  case(CLS_GLOBAL_AS):    return clk::A_GLOBAL;
  case(CLS_CONSTANT_AS):  return clk::A_CONSTANT;
  case(CLS_LOCAL_AS):     return clk::A_LOCAL;
  default: assert("unexpected address space"==NULL); return -1;
  }
}

int mapSpirAccessQualifierToAMDILImageType(int spirAccQual)
{
  int amdilImgTy = 0;
  switch (spirAccQual) {
  case CLS_WRITE_ONLY:  amdilImgTy = IMAGE_TYPE_WRITEONLY; break;
  case CLS_READ_ONLY:
  case CLS_NONE:        amdilImgTy = IMAGE_TYPE_READONLY; break;
  case CLS_READ_WRITE:  amdilImgTy = IMAGE_TYPE_READWRITE; break;
  default: assert("unexpected image access type"==NULL); break;
  }
  return amdilImgTy;
}

int mapSpirAccessQualifierToAMDILImageType(const std::string& spirAccQual)
{
  int amdilImgTy = 0;
  if (spirAccQual == "write_only")
    amdilImgTy = IMAGE_TYPE_WRITEONLY;
  else if (spirAccQual == "read_only" || spirAccQual == "none")
    amdilImgTy = IMAGE_TYPE_READONLY;
  else if (spirAccQual == "read_write")
    amdilImgTy = IMAGE_TYPE_READWRITE;
  else
    assert("unexpected image access type"==NULL);
  return amdilImgTy;
}

// Map SPIR access qualifier and type qualifier to AMD OCL type qualifier
int mapSpirAccessAndTypeQualifier(int accQual, int typeQual) {
  int tqCode = clk::Q_NONE;
  switch(accQual) {
  case CLS_READ_ONLY:   tqCode |= clk::Q_READ; break;
  case CLS_WRITE_ONLY:  tqCode |= clk::Q_WRITE; break;
  case CLS_READ_WRITE:  tqCode |= clk::Q_READ|clk::Q_WRITE; break;
  default: break;
  }
  switch(typeQual) {
  case CLS_ARG_CONST:     tqCode |= clk::Q_CONST; break;
  case CLS_ARG_RESTRICT:  tqCode |= clk::Q_RESTRICT; break;
  case CLS_ARG_VOLATILE:  tqCode |= clk::Q_VOLATILE; break;
  case CLS_ARG_PIPE:      tqCode |= clk::Q_PIPE; break;
  default: break;
  }
  return tqCode;
}

std::string mapLLVMTypeToOpenCLType(Type* type, bool isSigned) {
  if (type->isHalfTy())
    return "half";
  if (type->isFloatTy())
    return "float";
  if (type->isDoubleTy())
    return "double";
  if (IntegerType* intTy = dyn_cast<IntegerType>(type)) {
    std::string signPrefix;
    std::string stem;
    if (!isSigned)
      signPrefix = "u";
    switch (intTy->getIntegerBitWidth()) {
    case 8:
      stem = "char";
      break;
    case 16:
      stem = "short";
      break;
    case 32:
      stem = "int";
      break;
    case 64:
      stem = "long";
      break;
    default:
      stem = "invalid_type";
      break;
    }
    return signPrefix + stem;
  }
  if (VectorType* vecTy = dyn_cast<VectorType>(type)) {
    Type* eleTy = vecTy->getElementType();
    unsigned size = vecTy->getVectorNumElements();
    std::stringstream ss;
    ss << mapLLVMTypeToOpenCLType(eleTy, isSigned) << size;
    return ss.str();
  }
  return "invalid_type";
}

bool isDecoratedKernelName(const std::string& name) {
  int len = name.length();
  return name.substr(0, 9) == "__OpenCL_" &&
      name.substr(len-7, 7) == "_kernel";
}

bool isKernel(Function& F) {
  return F.getCallingConv() == CallingConv::SPIR_KERNEL ||
      (F.hasName() && F.getName().startswith("__OpenCL_") &&
      F.getName().endswith("_kernel"));
}

// Map SPIR access qualifier and type qualifier to AMD OCL type qualifier
int mapSpirAccessAndTypeQualifier(const std::string& spirAccQual,
                                  const std::string& spirTypeQuals,
                                  llvm::Type* type) {
  int tqCode = clk::Q_NONE;
  if (spirAccQual == "write_only")
    tqCode |= clk::Q_WRITE;
  else if (spirAccQual == "read_only")
    tqCode |= clk::Q_READ;
  else if (spirAccQual == "read_write")
    tqCode |= clk::Q_READ|clk::Q_WRITE;

  std::istringstream is(spirTypeQuals);
  std::string spirTypeQual;
  while (!is.eof()) {
    is >> spirTypeQual;
    if (spirTypeQual == "const") {
      tqCode |= clk::Q_CONST;
    }
    else if (spirTypeQual == "restrict")
      tqCode |= clk::Q_RESTRICT;
    else if (spirTypeQual == "volatile")
      tqCode |= clk::Q_VOLATILE;
    else if (spirTypeQual == "pipe")
      tqCode |= clk::Q_PIPE;
  };

  return tqCode;
}

// Check if a type name is of unsigned type or if the type is pointer whether
// its pointee is unsigned.
bool isTypeNameUnsigned(const std::string& typeNames) {
  std::istringstream is(typeNames);
  bool isUnsigned = false;
  while (!is.eof()) {
    std::string typeName;
    is >> typeName;
    typeName.erase(std::remove(typeName.begin(), typeName.end(), '*'),
        typeName.end());
    if (typeName == "struct") {
      break;
    }
    isUnsigned |= typeName.find("unsigned") == 0;
    isUnsigned |= typeName.find("uchar") == 0;
    isUnsigned |= typeName.find("ushort") == 0;
    isUnsigned |= typeName.find("uint") == 0;
    isUnsigned |= typeName.find("ulong") == 0;
    if (isUnsigned)
      break;
  }
  return isUnsigned;
}

// Check if a type name is of signed type or if it is a pointer whether its
// pointee is signed.
bool isTypeNameSigned(const std::string& typeNames) {
  std::istringstream is(typeNames);
  bool isSigned = false;
  bool isUnsigned = false;
  while (!is.eof()) {
    std::string typeName;
    is >> typeName;
    typeName.erase(std::remove(typeName.begin(), typeName.end(), '*'),
        typeName.end());
    if (typeName == "struct") {
      isSigned = false;
      break;
    }
    isUnsigned |= typeName.find("unsigned") == 0;
    isUnsigned |= typeName.find("uchar") == 0;
    isUnsigned |= typeName.find("ushort") == 0;
    isUnsigned |= typeName.find("uint") == 0;
    isUnsigned |= typeName.find("ulong") == 0;
    if (isUnsigned) {
      isSigned = false;
      break;
    }
    isSigned |= typeName.find("char") == 0;
    isSigned |= typeName.find("short") == 0;
    isSigned |= typeName.find("int") == 0;
    isSigned |= typeName.find("long") == 0;
    if (isSigned) {
      break;
    }
  }
  return isSigned;
}

// Check whether a type name is of an image type
bool isTypeNameImage(const std::string typeName) {
  bool isImage = false;
  isImage |= typeName == "image1d_t";
  isImage |= typeName == "image1d_array_t";
  isImage |= typeName == "image1d_buffer_t";
  isImage |= typeName == "image2d_t";
  isImage |= typeName == "image2d_array_t";
  isImage |= typeName == "image3d_t";
  return isImage;
}

// Check if a global variable used by a function
bool isUsedBy(GlobalVariable& GV, Function& F) {
  for (GlobalVariable::use_iterator UI = GV.use_begin(),
      UE = GV.use_end(); UI != UE; ++UI) {
    User* user = *UI;
    if (Instruction* inst = dyn_cast<Instruction>(user)) {
      if (inst->getParent()->getParent() == &F) {
        return true;
      }
    }
  }
  return false;
}

// Remove __OpenCL_ prefix and _kernel postfix from kernel name
std::string getOriginalKernelName(const std::string& name) {
  if (isDecoratedKernelName(name))
    return name.substr(9, name.size()-16);
  else
    return name;
}

// Get all kernel names by calling convention.
// This function needs to be run before kernel calling convention is transformed
// and kernel names are changed.
void getKernelNames(Module& M, std::set<std::string>& names) {
  for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
    if (I->getCallingConv() == CallingConv::SPIR_KERNEL) {
      names.insert(I->getName());
    }
  }
}

// Get prefix of a name before a char ch
std::string getPrefix(const std::string& name, char divider) {
  size_t pos = name.find(divider);
  if (pos != std::string::npos) {
    return name.substr(0, pos-1);
  } else
    return "";
}

// Check if a string starts with a function name and followed by '.'
// if so returns true and prefix is assigned the function name
bool startswith(const std::string& name, std::set<std::string>& prefixes,
    std::string& prefix) {
  prefix = getPrefix(name, '.');
  if (prefix == "")
    return false;
  if (prefixes.find(prefix) != prefixes.end())
    return true;
  else
    return false;
}

// If the string starts with a number, returns the string dropping the number
// otherwise return the same string.
StringRef dropNumber(StringRef& str) {
  int offset = 0;
  int number;
  int num = sscanf(str.data(), "%d%n", &number, &offset);
  if (offset)
    return str.drop_front(offset);
  else
    return str;
}

Function* findFunctionWithName(Module& module, const std::string& name) {
  Function* func = NULL;
  for (Module::iterator I = module.begin(), E = module.end(); I != E; ++I) {
    if (I->getName() == name) {
      func = I;
      break;
    }
  }
  return func;
}

Function* findFunctionWithName(Module& module, std::set<std::string>& names) {
  Function* func = NULL;
  for (Module::iterator I = module.begin(), E = module.end(); I != E; ++I) {
    if (names.find(I->getName()) != names.end()) {
      func = I;
      break;
    }
  }
  return func;
}


// change auto local array name from func.var to func_cllocal_var
void changeAutoArrayName(GlobalVariable& GV, const std::string& funcName,
    unsigned int addrSpace, bool isGPU) {
  std::string newName =  GV.getName().str().substr(funcName.size() + 1);
  if (isGPU) {
    std::string midfix = addrSpace == CLS_LOCAL_AS?"_cllocal_":"_clregion_";
    newName = funcName + midfix + newName;
  }
  DEBUG(dbgs() << " change name: " << GV.getName() << " -> " << newName << "\n");
  GV.setName(newName);
}

// Check if a global variable is auto local/region array

// Get auto local/region arrays for a function
// auto local/region arrays are transformed to a global variable
// with name func.var
void getAutoArraysByAddrSpace(Function& F, std::vector<GlobalVariable*>& found,
    unsigned int addrSpace, bool isGPU) {
  Module& M = *(F.getParent());
  for (Module::global_iterator I = M.global_begin(), E = M.global_end(); I != E;
      ++I) {
    GlobalVariable& GV = *I;
    if (GV.hasName() && GV.isInternalLinkage(GV.getLinkage()) &&
        GV.getType()->getPointerAddressSpace() == addrSpace) {
      std::string kernelName = getOriginalKernelName(F.getName());
      if (GV.getName().startswith(kernelName + ".")) {
        DEBUG (dbgs() << "[getAutolArraysByAddrSpace] " << addrSpace << ": " <<
            GV.getName() << " added to auto array of " << F.getName());
        changeAutoArrayName(GV, kernelName, addrSpace, isGPU);
        found.push_back(&GV);
      }
    }
  }
}

// get auto local arrays of a function
void getAutoLocalArrays(Function& F, std::vector<GlobalVariable*>& found,
    bool isGPU) {
  getAutoArraysByAddrSpace(F, found, LOCAL_ADDRESS, isGPU);
}

// get auto region arrays of a function
void getAutoRegionArrays(Function& F, std::vector<GlobalVariable*>& found,
    bool isGPU) {
  getAutoArraysByAddrSpace(F, found, REGION_ADDRESS, isGPU);
}

void getArgumentTypes(Function& func, std::vector<llvm::Type*>& argTys) {
  argTys.clear();
  for (Function::arg_iterator I = func.arg_begin(), E = func.arg_end(); I != E;
      ++I) {
    argTys.push_back(I->getType());
  }
}

void getParameters(CallInst& inst, std::vector<Value*>& params) {
  params.clear();
  int numParams = inst.getNumArgOperands();
  for (int i = 0; i < numParams; ++i) {
    params.push_back(inst.getArgOperand(i));
  }
}

Constant* getConstantStr(Module &M, const std::string& str,
    const char* tmpName) {
  Constant* res = ConstantDataArray::getString(M.getContext(), str);
  return res;
}

GlobalVariable* getConstantStrVar(Module &M, const std::string& str,
    const char* tmpName) {
  Constant* res = getConstantStr(M, str, tmpName);
  // Create a global variable for this string
  return new GlobalVariable(M, res->getType(), true,
                            GlobalValue::InternalLinkage,
                            res, tmpName,
                            0,
                            GlobalVariable::NotThreadLocal,
                            CONSTANT_ADDRESS);
}

GlobalVariable* getConstantStrVar(Module &M, ConstantArray* carray,
                            const char* tmpName) {
  return new GlobalVariable(M, carray->getType(), true,
                            GlobalValue::InternalLinkage,
                            carray, tmpName,
                            0,
                            GlobalVariable::NotThreadLocal,
                            CONSTANT_ADDRESS);
}

// LLVM code emitter ported from e2lBuild
class AMDLLVMBuilder {
  Module& module;
  LLVMContext& context;
  IRBuilder<> llvmBuilder;
  DataLayout DL;
  const static char* convVarName;
  const static char* tmpVarName;
  const static char* ptrVarName;
  const static char* cmpVarName;

public:
  AMDLLVMBuilder(Module& aModule):module(aModule),
    context(module.getContext()), llvmBuilder(context), DL(&module)
  {}

  void setInsertPoint(BasicBlock* bb) {
    llvmBuilder.SetInsertPoint(bb);
  }

  void setInsertPoint(Instruction* inst) {
    llvmBuilder.SetInsertPoint(inst);
  }

  // Set insert point just before first non-alloca instruction
  Instruction* setInsertPointAtAlloca(Function* F) {
    BasicBlock* BB = F->begin();
    BasicBlock::iterator I = BB->begin(), E = BB->end();
    for (; I != E; ++I) {
      if (I->getOpcode() != Instruction::Alloca)
        break;
    }
    llvmBuilder.SetInsertPoint(I);
    return I;
  }

  Instruction* cloneWithOperands(Instruction* inst) {
    Instruction* newInst = inst->clone();
    llvmBuilder.Insert(newInst, inst->getName());
    for (unsigned i = 0; i < inst->getNumOperands(); ++i)
      newInst->setOperand(i, inst->getOperand(i));
    return newInst;
  }

  AllocaInst* emitAlloca(llvm::Type* type, uint64_t size, const std::string& name) {
    Value* sizeVal = ConstantInt::get(llvm::Type::getInt32Ty(context), size,
        false);
    AllocaInst* inst = llvmBuilder.CreateAlloca(type, sizeVal, name);
    inst->setAlignment(getAlign(type));
    return inst;
  }

  Value* emitBitCast(Value* src, llvm::Type* dst, const std::string& name = convVarName) {
    return llvmBuilder.CreateCast(Instruction::BitCast, src, dst, name);
  }

  CallInst* emitCall(llvm::Value* callee, llvm::ArrayRef<llvm::Value*> args) {
    return verboseDebug(llvmBuilder.Insert(llvm::CallInst::Create(callee, args)));
  }

  CallInst* emitCall(const std::string& funcName, Value* arg,
      const std::string& name) {
    Function* func = getFunc(funcName);
    return llvmBuilder.CreateCall(func, arg, name);
  }

  CallInst* emitCall(const std::string& funcName, long arg,
      const std::string& name) {
    Function* func = getFunc(funcName);
    llvm::Type* argTy = func->arg_begin()->getType();
    Value* value = ConstantInt::get(argTy, arg, true);
    return emitCall(funcName, value, name);
  }

  llvm::Value* emitIntCast(llvm::Value* src, llvm::Type* dst,
                          bool isSigned,
                          const char* name = convVarName) {
    return verboseDebug(llvmBuilder.CreateIntCast(src, dst, isSigned, name));
  }

  llvm::Value* emitIntToPtr(llvm::Value* src, llvm::Type* dst,
                            const char* name = convVarName) {
    return verboseDebug(llvmBuilder.CreateCast(llvm::Instruction::IntToPtr, src,
        dst, name));
  }

  LoadInst* emitLoad(Value* src, bool isVolatile = false) {
    LoadInst* inst = llvmBuilder.CreateLoad(src, "tmp");
    inst->setAlignment(getAlign(src->getType()));
    return inst;
  }

  llvm::Value* emitPtrToInt(llvm::Value* src, llvm::Type* dst,
                         const char* name = convVarName) {
    return verboseDebug(llvmBuilder.CreateCast(llvm::Instruction::PtrToInt,
        src, dst, name));
  }

  // convert a scalar to a vector by padding with undef values
  Value* emitScalar2Vector(Value* src, VectorType* dst,
                           const std::string& name = convVarName) {
    Value* value = NULL;
    llvm::Type* srcTy = src->getType();
    if (srcTy == dst)
      return src;
    int numElem = dst->getNumElements();
    if (numElem == 1)
      return emitBitCast(src, dst, name);
    else {
      value = UndefValue::get(dst);
      for (int i = 0; i < numElem; ++i) {
        Value* index = ConstantInt::get(llvm::Type::getInt32Ty(context), i);
        value = llvmBuilder.CreateInsertElement(value, src, index, name);
      }
    }
    return value;
  }

  inline llvm::Value* emitFPTrunc(llvm::Value* src, llvm::Type* dst,
                           const char* name = convVarName);
  inline llvm::Value* emitFPExt(llvm::Value* src, llvm::Type* dst,
                         const char* name = convVarName);

  llvm::Value* emitSIToFP(llvm::Value* src, llvm::Type* dst,
                          const char* name = convVarName) {
    return verboseDebug(llvmBuilder.CreateCast(llvm::Instruction::SIToFP, src,
        dst, name));
  }

    llvm::Value* emitUIToFP(llvm::Value* src, llvm::Type* dst,
                            const char* name = convVarName) {
    return verboseDebug(llvmBuilder.CreateCast(llvm::Instruction::UIToFP, src,
        dst, name));
  }

  llvm::Value* emitFPToSI(llvm::Value* src, llvm::Type* dst,
                          const char* name = convVarName) {
    return verboseDebug(llvmBuilder.CreateCast(llvm::Instruction::FPToSI, src,
        dst, name));
  }

  llvm::Value* emitFPToUI(llvm::Value* src, llvm::Type* dst,
                          const char* name = convVarName) {
    return verboseDebug(llvmBuilder.CreateCast(llvm::Instruction::FPToUI, src,
        dst, name));
  }

  StoreInst* emitStore(Value* src, Value* dst, bool isVolatile = false) {
    StoreInst* inst = llvmBuilder.CreateStore(src, dst, isVolatile);
    inst->setAlignment(getAlign(dst->getType()));
    return inst;
  }

  llvm::Value* emitSub(llvm::Value* src1, llvm::Value* src2,
                       const char* name = tmpVarName) {
    return verboseDebug(llvmBuilder.CreateSub(src1, src2, tmpVarName));
  }

  LLVMContext& getLLVMContext() { return context; }

  // Get type size in bytes
  uint64_t getSize(llvm::Type* type) {
    return DL.getTypeSizeInBits(type)/8;
  }

  unsigned getAlign(llvm::Type* type) {
    return DL.getPrefTypeAlignment(type);
  }

  ConstantInt* getUint32(unsigned value) {
    IntegerType* int32Ty = IntegerType::getInt32Ty(context);
    return ConstantInt::get(int32Ty, value, false);
  }

  llvm::Function* getFunctionWithTypename(llvm::FunctionType* funcType,
                         const char* funcName) {
    llvm::Function* llvmFunc = module.getFunction(funcName);
    if (llvmFunc == NULL) {
      llvmFunc = llvm::Function::Create(funcType,
                                   llvm::Function::ExternalLinkage,
                                   funcName,
                                   &module);
    }
    return llvmFunc;
  }

  // Get an external function by name. If it does not exist create it.
  Function* getFunc(const std::string& name, Type* returnTy,
      std::vector<Type*> argTy) {
    Function* oldFun = findFunctionWithName(module, name);
    if (oldFun != NULL)
      return oldFun;

    FunctionType* newFunTy = FunctionType::get(returnTy, argTy, false);
    Function* newFun = Function::Create(newFunTy, Function::ExternalLinkage,
        name, &module);
    return newFun;
  }

  Function* getFunc(const std::string& name) {
    llvm::Type* returnTy = llvm::Type::getVoidTy(context);
    std::vector<llvm::Type*> argTy;
    if (name == "__amd_get_local_mem_addr") {
      returnTy = PointerType::get(llvmBuilder.getInt8Ty(), LOCAL_ADDRESS);
      argTy.push_back(llvmBuilder.getInt64Ty());
    }
    Function* func = getFunc(name, returnTy, argTy);
    return func;
  }

  // convert a constant expr to instruction
  Value* convertConstExpr(ConstantExpr* expr, Value* From, Value* To) {
    assert(expr->getOperand(0) == From);

    unsigned numIndex = expr->getNumOperands() - 1;
    std::vector<Value*> index(numIndex);
    for (unsigned i = 0; i < numIndex; ++i) {
      index[i] = expr->getOperand(i+1);
    }
    unsigned opCode = expr->getOpcode();
    Value* retVal;
    switch (opCode) {
    case Instruction::GetElementPtr:
      retVal = llvmBuilder.CreateGEP(To, index, "tmp");
      break;
    case Instruction::BitCast:
      retVal = llvmBuilder.CreateBitCast(To, expr->getType(),
          "tmp");
      break;
    case Instruction::IntToPtr:
      retVal = llvmBuilder.CreateCast(llvm::Instruction::IntToPtr, To,
               expr->getType(), "tmp");
      break;
    case Instruction::PtrToInt:
      retVal = llvmBuilder.CreateCast(llvm::Instruction::PtrToInt, To,
               expr->getType(), "tmp");
      break;
    default:
      DEBUG(dbgs() << "ConstExpr " << *expr << " not handled\n");
      assert(NULL);
      retVal = expr;
    }
    return retVal;
  }

  void replaceUsesOfWith(User* user, Value *From, Value *To) {
    if (From == To) return;
    for (unsigned i = 0, E = user->getNumOperands(); i != E; ++i)
      if (user->getOperand(i) == From) {
        user->setOperand(i, To);
        DEBUG(dbgs() << "[replaceUsesOfWith] " << *From << " => " << *To
            << "\n");
      }
  }

  // Uses in a constant expression cannot be replaced by Use::replaceAllUsesWith.
  // Two passes are used. In the first pass, constant expressions containing the
  // use From is converted to instruction using the value To recursively. 
  // In the second pass, a normal replaceAllUsesWith is done.
  void replaceAllUsesWith(Value* From, Value* To) {
    for (Value::use_iterator UI = From->use_begin(),
        UE = From->use_end(); UI != UE; ++UI) {
      if (ConstantExpr* user = dyn_cast<ConstantExpr>(*UI)) {
        ConstantExpr* expr = dyn_cast<ConstantExpr>(user);
        Instruction* inst = dyn_cast<Instruction>(convertConstExpr(expr,
          From, To));
        // Recursive call to handle users that are ConstantExprs themselves
        replaceAllUsesWith(expr, inst);
        expr->dropAllReferences();
      }
    }

    From->replaceAllUsesWith(To);
  }
};

const char* AMDLLVMBuilder::convVarName = "conv";
const char* AMDLLVMBuilder::tmpVarName = "tmp";
const char* AMDLLVMBuilder::ptrVarName = "ptr";
const char* AMDLLVMBuilder::cmpVarName = "cmp";

llvm::Value*
AMDLLVMBuilder::emitFPTrunc(llvm::Value* src, llvm::Type* dst,
                      const char* name)
{
  return verboseDebug(llvmBuilder.CreateCast(llvm::Instruction::FPTrunc, src,
      dst, name));
}

llvm::Value*
AMDLLVMBuilder::emitFPExt(llvm::Value* src, llvm::Type* dst,
                    const char* name )
{
  return verboseDebug(llvmBuilder.CreateCast(llvm::Instruction::FPExt, src,
      dst, name));
}

class TypeNameChanger {
  std::set<const llvm::Type*> changed;
public:
  // get stem of opencl struct type name
  static std::string getStem(const llvm::Type* type) {
    std::string stem = "";
    if (const StructType* structType = dyn_cast<StructType>(type)) {
      StringRef structName = structType->getName();
      if (structName.startswith("opencl.")) {
        stem = structName.substr(7, structName.size()-9);
      }
    }
    return stem;
  }

  // map opencl.* struct name to struct._*
  void change(const llvm::Type* type) {
    if (type == NULL || changed.find(type) != changed.end())
      return;
    if (const PointerType* ptrType = dyn_cast<PointerType>(type)) {
      change(ptrType->getElementType());
      changed.insert(ptrType);
    } else if (const StructType* structType = dyn_cast<StructType>(type)) {
      StringRef structName = structType->getName();
      std::string stem = getStem(structType);
      if (stem != "") {
        std::string newName = "struct._";
        newName = newName + stem + "_t";
        const_cast<StructType*>(structType)->setName(newName);
        DEBUG(dbgs() << " change type: " << structName << " => " << newName
            << "\n");
      } else {
        for (llvm::StructType::element_iterator I = structType->element_begin(),
            E = structType->element_end(); I != E; ++I) {
          change(*I);
        }
      }
      changed.insert(structType);
    } else {
      changed.insert(type);
    }
  }

  void change(Argument& arg) {
    change(dyn_cast<llvm::Type>(arg.getType()));
  }

  void change(Function& fun) {
    for (Function::arg_iterator I = fun.arg_begin(), E = fun.arg_end();
        I != E; ++I) {
      change(*I);
    }
  }

  void change(Module& M) {
    for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
      change(*I);
    }
  }
};

std::string mapKernelArgTypeName(Argument& arg, StringRef spirTypeName) {
  StringRef newName = spirTypeName;
  llvm::Type* type = arg.getType();
  std::string result;

  if (newName.startswith("struct ")) {
    newName = newName.drop_front(7);
  }
  if (newName.startswith("spir.image")) {
    newName = newName.substr(5, newName.size()-8);
    result = newName.str() + "_t";
  } else if (newName.startswith("struct._image")) {
    newName = newName.substr(8, newName.size()-10);
    result = newName.str() + "_t";
  } else if (newName.equals("struct._counter32_t")) {
    newName = "counter32";
    result = newName.str() + "_t";
  } else {
    result = newName.str();
  }

  return result;
}

// The library builtin function name is decorated with argument type names.
// The argument type name is encoded by this function.
std::string mapStructTypeNameForBuiltin(StructType* type) {
  StringRef structName = type->getStructName();
  std::string stem = TypeNameChanger::getStem(type);
  if (stem == "image2d_array") {
    return "image2da";
  } else if (stem == "image1d_array") {
    return "image1da";
  } else if (stem == "image1d_buffer") {
    return "image1db";
  } else if (stem != "") {
    return stem;
  } else if(structName.startswith("struct._image")) {
      return structName.substr(8, structName.size()-10);
  } else if(structName.equals("struct._counter32_t")) {
    return "counter32";
  }
  return "unknown";
}

// Expression transformations ported from e2lExpr
class ExprTransform {
private:
  DataLayout target;
  AMDLLVMBuilder* parBuilder;
public:
  ExprTransform(Module& M, AMDLLVMBuilder* aBuilder): target(&M),
    parBuilder(aBuilder) {}
  bool is_bool_type (llvm::Type* type) { return type->isIntegerTy(1); }
  bool isTrivialConv(CallInst* callInst);
  llvm::Value* transBuiltinConv(Function* func, Value* src);
  llvm::Value* transCastScalar(llvm::Value* srcValue, llvm::Type* srcType,
      bool srcIsSigned, llvm::Type* dstType, bool dstIsSigned);
  llvm::Value* transVectorConvert(llvm::Value* srcValue,
      llvm::Type* llvmSrcType, bool srcIsSigned, llvm::Type* llvmDstType,
      bool dstIsSigned);
  llvm::Value* transBuiltinPrintfptr(Function* func);
  void isConvFunSigned(Function* convFunc, bool& isSrcSigned,
      bool& isDestSigned);
};

// get signed/unsignedness of conversion functions
void ExprTransform::isConvFunSigned(Function* convFunc, bool& srcSigned,
    bool& dstSigned) {
  StringRef name = convFunc->getName();
  name = name.drop_front(10);
  StringRef srcTypeName = name.drop_front(name.find_last_of('_')+1);
  StringRef dstTypeName = name.drop_back(name.size()-name.find_first_of('_'));

  //bool srcSigned = func->paramHasAttr(0, Attribute::SExt);
  StringRef srcEleTypeName = dropNumber(srcTypeName);
  srcSigned = !srcEleTypeName.startswith("u");

  //const AttrListPtr &Attrs = func->getAttributes();
  //Attributes RetAttrs = Attrs.getRetAttributes();
  //bool dstSigned = (RetAttrs & Attribute::SExt) != 0;
  StringRef dstEleTypeName = dstTypeName;
  dstSigned = !dstEleTypeName.startswith("u");

  DEBUG(dbgs() << "srcTypeName=" << srcTypeName <<
      " srcEleTypeName=" << srcEleTypeName <<
      " srcSigned=" << srcSigned <<
      " dstTypeName=" << dstTypeName <<
      " dstEleTypeName=" << dstEleTypeName <<
      " dstSigned=" << dstSigned << "\n");
}

llvm::Value*
ExprTransform::transCastScalar(llvm::Value* srcValue, llvm::Type* srcType,
    bool srcIsSigned, llvm::Type* dstType, bool dstIsSigned)
{
  CHKASSERT(srcValue->getType() == srcType, "Type mismatch")

  if (srcType == dstType)
    return srcValue;

  if (dstType->isVoidTy())
    return NULL;

  llvm::Type* llvmDstType = dstType;
#ifdef VECTOR3_TO_STRUCT
  if (VectorType* vecType = dyn_cast<VectorType>(dstType))
  {
    if (vecType->getNumElements() == 3)
      llvmDstType = (llvm::cast<llvm::StructType>(llvmDstType))->
        getElementType(0);
  }
#endif
  // Ignore conversions like int -> uint.
  if (srcValue->getType() == llvmDstType)
    return srcValue;

  //scalar to vector cast
  if (llvm::isa<llvm::VectorType>(llvmDstType))
  {
    if (llvm::isa<llvm::VectorType>(srcValue->getType()))
    {
      //vector to vector
      //sse headers has this usage, gcc defines its semantics
      return parBuilder->emitBitCast(srcValue, llvmDstType);
    } else {
      if (is_bool_type(srcType)) {
        //bool to vector

        //trunc to i1 as bool only has 0 or 1
        //srcValue = parBuilder->emitIntCast(srcValue,
        //  llvm::Type::getInt1Ty(parBuilder->getLLVMContext()), TRUE, "tobool");

        llvm::Type* vecEleType =
          (llvm::cast<llvm::VectorType>(llvmDstType))->getElementType();

        if (llvm::isa<llvm::IntegerType>(vecEleType))
          srcValue = parBuilder->emitIntCast(srcValue, vecEleType, true, "frombool");
        else
          srcValue = parBuilder->emitSIToFP(srcValue, vecEleType, "frombool");
      }
      return parBuilder->emitScalar2Vector(srcValue,
        llvm::dyn_cast<llvm::VectorType>(llvmDstType));
    }
  } else if (llvm::isa<llvm::VectorType>(srcValue->getType()))
  {
     //vector to scalar,
     //sse headers has this usage, gcc defines its semantics
     return parBuilder->emitBitCast(srcValue, llvmDstType);
  }

  // Handle pointer conversions next: pointers can only be converted to/from
  // other pointers and integers.
  if (llvm::isa<llvm::PointerType>(llvmDstType))
  {
    // The source value may be an integer, or a pointer.
    if (llvm::isa<llvm::PointerType>(srcValue->getType()))
      return parBuilder->emitBitCast(srcValue, llvmDstType);

    assert((srcType==NULL || srcType->isIntegerTy()) &&
      "Problem with ptr target conversion: expect either ptr->ptr or int->ptr conversion" != NULL);

    return parBuilder->emitIntToPtr(srcValue, llvmDstType);
  }

  if (llvm::isa<llvm::PointerType>(srcValue->getType()))
  {
    // Must be an ptr to int cast.
    assert(llvm::isa<llvm::IntegerType>(llvmDstType) &&
      "Problem with ptr source conversion: expect ptr->int conversion" != NULL);

    return parBuilder->emitPtrToInt(srcValue, llvmDstType);
  }

  // Finally, we have the arithmetic types: real int/float.
  if (llvm::isa<llvm::IntegerType>(srcValue->getType()))
  {
    bool inputSigned = true;

    // bool=>other-scalar, bool is unsigned
    if (srcType)
      inputSigned = srcIsSigned && (is_bool_type(srcType) == false);

    if (llvm::isa<llvm::IntegerType>(llvmDstType)) {
      if (is_bool_type(dstType) && is_bool_type(srcType)== false)
      {
        // int to bool, front-end already insert the != 0 comparison
        // here remove the zext
        assert(NULL && "convert to bool not implemented");
        //srcValue = parBuilder->emitConvToBool(srcValue, srcType);
      }
      return parBuilder->emitIntCast(srcValue, llvmDstType, inputSigned);
    } else if (inputSigned)
      return parBuilder->emitSIToFP(srcValue, llvmDstType);
    else
      return parBuilder->emitUIToFP(srcValue, llvmDstType);
  }

  CHKASSERT(srcValue->getType()->isFloatingPointTy(), "Unknown real conversion")
  if (llvm::isa<llvm::IntegerType>(llvmDstType))
  {
    if (is_bool_type(dstType)) {
      //float => bool
      //this should not be reached
      //as front-end already insert the != 0 comparison
      CHKASSERT(NULL, "Convert to bool type not implemented")
      //srcValue = parBuilder->emitConvToBool(srcValue, srcType);
      //parBuilder->emitIntCast(srcValue, llvmDstType, FALSE);
    } else if (dstIsSigned)
      return parBuilder->emitFPToSI(srcValue, llvmDstType);
    else
      return parBuilder->emitFPToUI(srcValue, llvmDstType);
  }

  CHKASSERT(llvmDstType->isFloatingPointTy(),"Unknown real conversion")
  if (llvmDstType->getTypeID() < srcValue->getType()->getTypeID())
    return parBuilder->emitFPTrunc(srcValue, llvmDstType);
  else
    return parBuilder->emitFPExt(srcValue, llvmDstType);

  return NULL;
} /*transCastScalar*/

bool
 ExprTransform::isTrivialConv(CallInst* callInst) {
#if 1 // change it to 0 to always return false
#define INCLUDE_FLOAT_DOUBLE //consider float<=>double trivial
  Function* func = callInst->getCalledFunction();
  FunctionType* funcTy = func->getFunctionType();
  llvm::Type* srcTy = callInst->getArgOperand(0)->getType();
  llvm::Type* dstTy = func->getReturnType();
  llvm::Type* dstEleTy;
  llvm::Type* srcEleTy;
  unsigned dstsize;
  unsigned srcsize;
  bool srcSigned = true;
  bool dstSigned = true;
  isConvFunSigned(func, srcSigned, dstSigned);

  //convert to the same type
  if (dstTy == srcTy && srcSigned == dstSigned)
    return true;

  if (srcTy->isVectorTy()) {
    srcEleTy = srcTy->getVectorElementType();
    dstEleTy = dstTy->getVectorElementType();
  } else {
    srcEleTy = srcTy;
    dstEleTy = dstTy;
  }

  if (dstEleTy == srcEleTy && srcSigned == dstSigned)
    return true;

  dstsize = target.getTypeSizeInBits(dstEleTy) / 8;
  srcsize = target.getTypeSizeInBits(srcEleTy) / 8;
  std::string funcName(func->getName());

  if (dstEleTy->isIntegerTy()) {
    if (srcEleTy->isIntegerTy()) {
      //1. int => int
      if (dstsize > srcsize) {
        //1.1 dstsize > srcsize
        if (srcSigned
            && dstSigned == false
            && funcName.find("_sat") != std::string::npos)
          return false;
        //else return true;
      } else {
        //1.2 dstsize <= srcsize
        if (funcName.find("_sat") != std::string::npos)
          return false;
        //else return true;
      }
      //else return true;
    } else {
      //2. fp => int
      if (funcName.find("_sat") != std::string::npos
          || funcName.find("_rtp") != std::string::npos
          || funcName.find("_rtn") != std::string::npos
          || funcName.find("_rte") != std::string::npos)
        return false;
      // else return true; "_rtz" is ok
    }
  } else {
    if (srcEleTy->isIntegerTy()) {
      //3. int => fp, no _sat for =>fp
      if (dstsize > srcsize) {
        //3.1 dstsize > srcsize, no == is excluded
        //return true;
      } else {
        //4.2 dstsize <= srcsize, no _sat for =>fp
        if (funcName.find("_rtp") != std::string::npos
            || funcName.find("_rtn") != std::string::npos
            || funcName.find("_rtz") != std::string::npos)
          return false;
        // else return true; "_rte" is ok
      }
    } else {
#ifdef INCLUDE_FLOAT_DOUBLE
      //4. fp => fp
      if (dstsize >= srcsize) {
        //4.1 dstsize >= srcsize
        //return true;
      } else {
        //4.2 dstsize < srcsize, no _sat for =>fp
        if (funcName.find("_rtp") != std::string::npos
            || funcName.find("_rtn") != std::string::npos
            || funcName.find("_rtz") != std::string::npos)
          return false;
      }
      // else return true; "_rte" is ok
#undef INCLUDE_FLOAT_DOUBLE //consider float<=>double trivial
#else
      return false;
#endif
    }
  }
  return true;
#else
  return false;
#endif
} //E2lExpr::isTrivialConv

Value*
ExprTransform::transBuiltinConv(Function* func, Value* srcVal)
{
  llvm::Type* srcType = srcVal->getType();
  llvm::Type* dstType = func->getReturnType();

  bool srcSigned = true;
  bool dstSigned = true;
  isConvFunSigned(func, srcSigned, dstSigned);

  //vector conv is limited to types with the same numEle
  llvm::VectorType* llvmVecType = llvm::dyn_cast<llvm::VectorType>(srcVal->getType());
  int numEle = llvmVecType ? llvmVecType->getNumElements() : 1;

  //transCastScalar assume vector cast in source is bitcast, can't use it for vector
  if (numEle == 1)
    srcVal = transCastScalar(srcVal, srcType, srcSigned, dstType, dstSigned);
  else
    srcVal = transVectorConvert(srcVal, srcType, srcSigned, dstType, dstSigned);

  //if (isRvalue == false)
  //  rvalue2lvalue(srcVal, dstType);

  return srcVal;
} //transBuiltinConv

llvm::Value*
ExprTransform::transVectorConvert(llvm::Value* srcValue,
                                  llvm::Type* llvmSrcType,
                                  bool srcIsSigned,
                                  llvm::Type* llvmDstType,
                                  bool dstIsSigned)
{
  // Ignore conversions like int -> uint.
  if (llvmSrcType == llvmDstType)
    return srcValue;

  llvm::Type* llvmDstEleType = dyn_cast<VectorType>(llvmDstType)->getElementType() ;
  llvm::Type* llvmSrcEleType = dyn_cast<VectorType>(llvmSrcType)->getElementType();

  // arithmetic types: real int/float.
  if (llvm::isa<llvm::IntegerType>(llvmSrcEleType))
  {
    // bool=>other-scalar, bool is unsigned
    // simplify the logic by assume not vector of bool
    bool inputSigned = srcIsSigned;

    if (llvm::isa<llvm::IntegerType>(llvmDstEleType)) {
      return parBuilder->emitIntCast(srcValue, llvmDstType, inputSigned);
    } else if (inputSigned)
      return parBuilder->emitSIToFP(srcValue, llvmDstType);
    else
      return parBuilder->emitUIToFP(srcValue, llvmDstType);
  }

  CHKASSERT(llvmSrcEleType->isFloatingPointTy(), "Unknown real conversion")
  if (llvm::isa<llvm::IntegerType>(llvmDstEleType))
  {
    if (dstIsSigned)
      return parBuilder->emitFPToSI(srcValue, llvmDstType);
    else
      return parBuilder->emitFPToUI(srcValue, llvmDstType);
  }

  CHKASSERT(llvmDstEleType->isFloatingPointTy(),"Unknown real conversion")
  if (llvmDstEleType->getTypeID() < llvmSrcEleType->getTypeID())
    return parBuilder->emitFPTrunc(srcValue, llvmDstType);

  return parBuilder->emitFPExt(srcValue, llvmDstType);

} /*transVectorConvert*/

llvm::Value*
ExprTransform::transBuiltinPrintfptr(Function* func)
{
  Module* module = func->getParent();
  std::string funcName("__amd_get_builtin_fptr");
  llvm::Type* resType = func->getFunctionType();
  resType = llvm::PointerType::getUnqual(resType);

  std::vector<llvm::Type*> argTys;
  argTys.push_back(llvm::Type::getInt32Ty(parBuilder->getLLVMContext()));

  llvm::FunctionType* funcType = llvm::FunctionType::get(resType, argTys, false);

  llvm::Function* llvmFunc = parBuilder->getFunctionWithTypename(funcType,
      funcName.c_str());

  verboseDebug(llvmFunc);
  std::vector<llvm::Value*> args;
  args.push_back(llvm::Constant::getNullValue(llvm::Type::getInt32Ty(
      parBuilder->getLLVMContext())));

  llvm::CallInst* res = parBuilder->emitCall(llvmFunc, args);
  res->setName("call");

  return res;
}

class MathFunctionTransform {
  Module& module;
  LLVMContext& context;
  std::set<std::string> funcNames;  // a set of function names that needs
                                    // transformation
  std::set<Function*> candidates;   // functions to be transformed
public:
  MathFunctionTransform(Module& aModule)
    : module(aModule), context(module.getContext()) {
    initFuncNames();
  }

  void initFuncNames() {
    static const char* funcArray[] = {
        "fmax",
        "fmin",
        "max",
        "min",
        "step",
        NULL
    };
    for (const char** I = funcArray; *I != NULL; ++I) {
      funcNames.insert(*I);
    }
  }

  // Check if a function needs transform. If true added it to candidates
  void collect(Function* func, const std::string& demangledName) {
    if (funcNames.find(demangledName) != funcNames.end()) {
      candidates.insert(func);
    }
  }

  void transform() {
    for (std::set<Function*>::iterator I = candidates.begin(),
        E = candidates.end(); I != E; ++I) {
      transform(*I);
    }
  }

  void transform(Function* oldFun) {
    std::vector<llvm::Type*> argTy;
    getArgumentTypes(*oldFun, argTy);

    if (argTy.size()<2)
      return;

    // Find the first vector arg and all scalar args
    std::set<int> scalars;
    llvm::VectorType* vecTy = NULL;
    for (unsigned int i = 0; i < argTy.size(); ++i) {
      if (vecTy == NULL && argTy[i]->isVectorTy()) {
        vecTy = dyn_cast<VectorType>(argTy[i]);
      } else {
        scalars.insert(i);
      }
    }
    if (vecTy == NULL || scalars.empty())
      return;

    std::string name = oldFun->getName();
    oldFun->setName(std::string(".") + name);
    llvm::Type* returnTy = oldFun->getReturnType();

    AMDLLVMBuilder builder(module);
    // Change all scalar args type to vector
    for (std::set<int>::iterator I = scalars.begin(), E = scalars.end(); I != E;
        ++I) {
      argTy[*I] = vecTy;
    }

    FunctionType* newFunTy = FunctionType::get(returnTy, argTy, false);
    Function* newFun = Function::Create(newFunTy, Function::ExternalLinkage,
        name, &module);
    newFun->setAttributes(oldFun->getAttributes());

    for (Function::use_iterator I = oldFun->use_begin(), E = oldFun->use_end();
        I != E; ++I) {
      if (CallInst* oldInst = dyn_cast<CallInst>(*I)) {

        builder.setInsertPoint(oldInst);
        std::vector<Value*> params;
        getParameters(*oldInst, params);

        for (std::set<int>::iterator SI = scalars.begin(), SE = scalars.end();
            SI != SE; ++SI) {
          params[*SI] = builder.emitScalar2Vector(params[*SI], vecTy, "conv");
        }

        BasicBlock* BB = oldInst->getParent();
        CallInst* newInst = CallInst::Create(newFun, params, "");
        newInst->setTailCall(oldInst->isTailCall());
        newInst->setCallingConv(oldInst->getCallingConv());
        newInst->setAttributes(oldInst->getAttributes());
        BB->getInstList().insert(oldInst, newInst);
        oldInst->replaceAllUsesWith(newInst);
        oldInst->dropAllReferences();
        oldInst->removeFromParent();
      }
    }
    oldFun->removeFromParent();
  }
};

// Transform auto local arrays for CPU
class AutoArrayTransform {
  Module& module;
  LLVMContext& context;
public:
  AutoArrayTransform(Module& aModule)
    :module(aModule),context(module.getContext()) {
  }
  void transform() {
    for (Module::iterator I = module.begin(), E = module.end(); I != E; ++I) {
      if (isKernel(*I))
        transform(*I);
    }
  }

  void transform(Function& F) {
    std::vector<GlobalVariable*> locals;
    getAutoLocalArrays(F, locals, false);
    AMDLLVMBuilder builder(module);
    builder.setInsertPointAtAlloca(&F);
    int size = 0;
    for (std::vector<GlobalVariable*>::iterator I = locals.begin(),
        E = locals.end(); I != E; ++I) {
      GlobalVariable* GV = *I;
      llvm::PointerType* type = GV->getType();
      std::string name = GV->getName();
      GV->setName(name + ".tmp");
      Value* call = builder.emitCall("__amd_get_local_mem_addr", size, "call");
      Value* conv = builder.emitBitCast(call, type, "conv");
      builder.replaceAllUsesWith(GV, conv);
      GV->dropAllReferences();
      GV->removeFromParent();
      size += builder.getSize(type->getElementType());
    }
  }

};

// Convert a value to int
void convertValue(Value* value, int& x) {
  x = dyn_cast<ConstantInt>(value)->getZExtValue();
}

int getMDOperandAsInt(MDNode* node, int index) {
  int value;
  convertValue(node->getOperand(index), value);
  return value;
}

std::string getMDOperandAsString(MDNode* node, int index) {
  Value* operand = node->getOperand(index);
  if (MDString* str = dyn_cast<MDString>(operand)) {
    return str->getString().str();
  } else
    return "";
}

Type* getMDOperandAsType(MDNode* node, int index) {
  return node->getOperand(index)->getType();
}

Constant* convertValue(Value* value, llvm::Type* type) {
  int tmp;
  convertValue(value, tmp);
  return ConstantInt::get(type, tmp, false);
}

void decodeMDNode (MDNode* node, int& x, int& y, int& z) {
  if (node == NULL)
    return;
  convertValue(node->getOperand(1), x);
  convertValue(node->getOperand(2), y);
  convertValue(node->getOperand(3), z);
}

void decodeMDNode (MDNode* node, llvm::Type* type,
    std::vector<Constant*>& values) {
  assert(values.size() == 3);
  if (node == NULL) {
    for (int i = 0; i < 3; ++i)
      values[i] = ConstantInt::get(type, 0, false);
  } else {
    for (int i = 0; i < 3; ++i)
      values[i] = convertValue(node->getOperand(i+1), type);
  }
}

std::string decodeVecTypeHintMDNode(MDNode* vth) {
  Type* hintType = getMDOperandAsType(vth, 1);
  int isSigned = getMDOperandAsInt(vth, 2);
  return mapLLVMTypeToOpenCLType(hintType, isSigned);
}

// Check whether a function name is a valid builtin convert function
// which is in the format convert_destType[n][_sat][_rte|_rtz|_rtp|_rtn]
// If isDecorated is true, the name has postfix _srcType which uses shorted
// type names.
class AMDConvertFunctionNameParser {
  StringRef funcName;
  bool isDecorated;
  StringRef dstType;
  unsigned dstVecSize;
  bool isSat;
  StringRef roundingMode;
  StringRef srcType;
  unsigned srcVecSize;

  void getVecSize(StringRef& name, unsigned& size) {
    // Valid vector size
    unsigned validSize[] = {2, 3, 4, 8, 16, 0};

   // check next char is valid vector size
    int i;
    int offset = 0;
    size = 1;
    int num = sscanf(name.data(), "%u%n", &size, &offset);
    if (num != EOF && num == 1) {
      for (i = 0; validSize[i] != 0; ++i) {
        if (size == validSize[i]) {
          break;
        }
      }

      // skip vector size
      if (validSize[i] != 0)
        name = name.substr(offset);
    }
  }
public:
    AMDConvertFunctionNameParser(bool aIsDecorated = false):
      isDecorated(aIsDecorated) {
    }

    StringRef signedType(StringRef& type) {
      return type.startswith("u")?type.substr(1):type;
    }
    // If the destination type is the same as the source type,
    // the conversion function is redundant.
    // Or if the signed type of dest type and the signed type of src type is
    // the same and the conversion is not saturation.
    // Must be called right after parse()
    bool isRedundant() {
      bool redundant = (srcType == dstType ||
          (signedType(srcType) == signedType(dstType) && !isSat))
          && srcVecSize == dstVecSize;
      DEBUG(dbgs() << funcName << " isRedundant=" << redundant << "\n");
      return redundant;
    }

    // return a string which is str with substr removed
    std::string erase(StringRef str, StringRef substr) {
      size_t location = str.find(substr);
      return std::string(str.substr(0, location))
        + std::string(str.substr(location+substr.size()));
    }

    // convert_[float|double][n]_rte is alias to convert_[float|double][n]
    // convert_{integer}[_sat]_rtz is alias to convert_{integer}[_sat]
    // returns the name of the real function if belongs to the above two cases
    // otherwise return empty string
    // must be called right after parse()
    std::string aliasName() {
      if (dstType == "float" || dstType == "double") {
        if (roundingMode == "_rte") {
          return erase(funcName, "_rte");
        }
      } else if (roundingMode == "_rtz") {
        return erase(funcName, "_rtz");
      }
      return "";
    }

    bool parse(StringRef name) {
      // Valid destination type for convert function
      struct NamePair {
        const char* first;
        const char* second;
      };
      NamePair validType[] = {
          {"char",    "i8"},
          {"uchar",   "u8"},
          {"short",   "i16"},
          {"ushort",  "u16"},
          {"int",     "i32"},
          {"uint",    "u32"},
          {"long",    "i64"},
          {"ulong",   "u64"},
          {"float",   "f32"},
          {"double",  "f64"},
          {"half",    "f16"},
          {NULL,      NULL},
      };

      // Valid rounding mode
      const char* validRounding[] = {
          "_rte",
          "_rtz",
          "_rtp",
          "_rtn",
          NULL
      };

      funcName = name;
      dstType = "";
      dstVecSize = 1;
      isSat = false;
      roundingMode = "";
      srcType = "";
      srcVecSize = 1;

      if (isDecorated) {
        if (!name.startswith("__"))
          return false;
        name = name.substr(2);
      }

      if (!name.startswith("convert_"))
        return false;

      // skip convert_
      name = name.substr(8);

      // check next substring is valid destination type
      NamePair* type;
      for (type = validType; type->first != NULL; ++type) {
        if (name.startswith(type->first)) {
          dstType = type->first;
          break;
        }
      }
      if (type->first == NULL)
        return false;

      // skip destination type name
      name = name.substr(strlen(type->first));

      if (!isDecorated && name.empty())
        return true;

      getVecSize(name, dstVecSize);

      if (!isDecorated && name.empty())
        return true;

      // check next substring is _sat and skip it
      if (name.startswith("_sat")) {
        isSat = true;
        name = name.substr(4);
      }

      if (!isDecorated && name.empty())
        return true;

      // check next substring is valid rounding mode
      int i;
      for (i = 0; validRounding[i] != NULL; ++i) {
        if (name.startswith(validRounding[i])) {
          roundingMode = validRounding[i];
          break;
        }
      }

      // skip rounding mode
      if (validRounding[i] != NULL)
        name = name.substr(strlen(validRounding[i]));

      if (!isDecorated && name.empty())
        return true;

      // The following checks decorated function name
      if (!name.startswith("_"))
        return false;
      name = name.substr(1);

      // get vector size of src type
      getVecSize(name, srcVecSize);

      // get short name of src type
      for (type = validType; type->first != NULL; ++type) {
        if (name.startswith(type->second)) {
          srcType = type->first;
          break;
        }
      }
      if (type->first == NULL)
        return false;

      // skip src short type name
      name = name.substr(strlen(type->second));
      if (name.empty())
        return true;

      return false;
    }
};

// map name of overloaded builtin functions
enum AMDBuiltinType {
  AMD_BUILTIN_PAR1,       // mangling function name by parameter 1
  AMD_BUILTIN_PAR2,       // mangling function name by parameter 2
  AMD_BUILTIN_PAR3,       // mangling function name by parameter 3
  AMD_BUILTIN_PAR1_PAR2,  // mangling function name by parameter 1 and 2
  AMD_BUILTIN_PAR1_PAR3,  // mangling function name by parameter 1 and 3
  AMD_BUILTIN_PAR1_PARN,  // mangling function name by parameter 1 and N(last)
  AMD_BUILTIN_PAR3_PAR1,  // mangling function name by parameter 3 and 1
  AMD_BUILTIN_INVALID
};

const char* amdBuiltinsPar1[] = {
    /* math functions */
    "acos",
    "acosh",
    "acospi",
    "asin",
    "asinh",
    "asinpi",
    "atan",
    "atan2",
    "atanh",
    "atanpi",
    "atan2pi",
    "cbrt",
    "ceil",
    "copysign",
    "cos",
    "cosh",
    "cospi",
    "erfc",
    "erf",
    "exp",
    "exp2",
    "exp10",
    "expm1",
    "fabs",
    "fdim",
    "floor",
    "fma",
    "__builtin_fma",
    "fmax",
    "fmin",
    "fmod",
    "hypot",
    "ilogb",
    "ldexp",
    "lgamma",
    "log",
    "log2",
    "log10",
    "log1p",
    "logb",
    "mad",
    "maxmag",
    "minmag",
    "nan",
    "nextafter",
    "pow",
    "pown",
    "powr",
    "remainder",
    "rint",
    "rootn",
    "round",
    "rsqrt",
    "sin",
    "sinh",
    "sinpi",
    "sqrt",
    "tan",
    "tanh",
    "tanpi",
    "tgamma",
    "trunc",
    "half_cos",
    "half_divide",
    "half_exp",
    "half_exp2",
    "half_exp10",
    "half_log",
    "half_log2",
    "half_log10",
    "half_powr",
    "half_recip",
    "half_rsqrt",
    "half_sin",
    "half_sqrt",
    "half_tan",
    "native_cos",
    "native_divide",
    "native_exp",
    "native_exp2",
    "native_exp10",
    "native_log",
    "native_log2",
    "native_log10",
    "native_powr",
    "native_recip",
    "native_rsqrt",
    "native_sin",
    "native_sqrt",
    "native_tan",
    /* integer functions */
    "abs",
    "abs_diff",
    "add_sat",
    "hadd",
    "rhadd",
    "clamp",
    "clz",
    "mad_hi",
    "mad_sat",
    "max",
    "min",
    "mul_hi",
    "rotate",
    "sub_sat",
    "upsample",
    "popcount",
    "mad24",
    "mul24",
    /* common functions */
    "degrees",
    "mix",
    "radians",
    "smoothstep",
    "sign",
    /* geometric functions */
    "cross",
    "dot",
    "distance",
    "length",
    "normalize",
    "fast_distance",
    "fast_length",
    "fast_normalize",
    /* relational functions */
    "isequal",
    "isnotequal",
    "isgreater",
    "isgreaterequal",
    "isless",
    "islessequal",
    "islessgreater",
    "isfinite",
    "isinf",
    "isnan",
    "isnormal",
    "isordered",
    "isunordered",
    "signbit",
    "any",
    "all",
    "bitselect",
    /* 32 bit atomic functions */
    "atomic_add",
    "atomic_sub",
    "atomic_xchg",
    "atomic_cmpxchg",
    "atomic_min",
    "atomic_max",
    "atomic_and",
    "atomic_or",
    "atomic_xor",
    "atomic_inc",
    "atomic_dec",
    /* 64 bit atomic functions */
    "atom_inc",
    "atom_dec",
    "atom_add",
    "atom_sub",
    "atom_xchg",
    "atom_min",
    "atom_max",
    "atom_and",
    "atom_or",
    "atom_xor",
    "atom_cmpxchg",
    /* image functions */
    "write_imagef",
    "write_imagei",
    "write_imageui",
    "get_image_width",
    "get_image_height",
    "get_image_channel_data_type",
    "get_image_channel_order",
    "get_image_dim",
    "get_image_array_size",
    /* async copies */
    "async_work_group_copy",
    "async_work_group_strided_copy",
    "prefetch",
    NULL
};

// Builtin functions that are mangled by its second argument
const char* amdBuiltinsPar2[] = {
    /* math functions */
    "fract",
    "modf",
    "sincos",
    /* common functions */
    "step",
    /* vload functions */
    "vload",
    "vload2",
    "vload3",
    "vload4",
    "vload8",
    "vload16",
    "vload_half",
    "vload_half2",
    "vload_half3",
    "vload_half4",
    "vload_half8",
    "vload_half16",
    "vloada_half",
    "vloada_half2",
    "vloada_half3",
    "vloada_half4",
    "vloada_half8",
    "vloada_half16",
    NULL,
};

// Builtin functions that are mangled by its third argument
const char* amdBuiltinsPar3[] = {
    /* vstore functions */
    "vstore",
    "vstore2",
    "vstore3",
    "vstore4",
    "vstore8",
    "vstore16",
    NULL
};

// Builtin functions that are mangled by its first and second arguments
const char* amdBuiltinsPar1Par2[] = {
    "frexp",
    "lgamma_r",
    "shuffle",
    NULL
};

// Builtin functions that are mangled by its first and third arguments
const char* amdBuiltinsPar1Par3[] = {
    "remquo",
    "select",
    "shuffle2",
    NULL
};

// Builtin functions that are mangled by its first and last arguments
const char* amdBuiltinsPar1ParN[] = {
    NULL
};

// Builtin functions that are mangled by its third and first arguments
const char* amdBuiltinsPar3Par1[] = {
    "vstore_half",
    "vstore_half2",
    "vstore_half3",
    "vstore_half4",
    "vstore_half8",
    "vstore_half16",
    "vstore_half_rte",
    "vstore_half2_rte",
    "vstore_half3_rte",
    "vstore_half4_rte",
    "vstore_half8_rte",
    "vstore_half16_rte",
    "vstore_half_rtz",
    "vstore_half2_rtz",
    "vstore_half3_rtz",
    "vstore_half4_rtz",
    "vstore_half8_rtz",
    "vstore_half16_rtz",
    "vstore_half_rtp",
    "vstore_half2_rtp",
    "vstore_half3_rtp",
    "vstore_half4_rtp",
    "vstore_half8_rtp",
    "vstore_half16_rtp",
    "vstore_half_rtn",
    "vstore_half2_rtn",
    "vstore_half3_rtn",
    "vstore_half4_rtn",
    "vstore_half8_rtn",
    "vstore_half16_rtn",
    "vstorea_half",
    "vstorea_half2",
    "vstorea_half3",
    "vstorea_half4",
    "vstorea_half8",
    "vstorea_half16",
    "vstorea_half_rte",
    "vstorea_half2_rte",
    "vstorea_half3_rte",
    "vstorea_half4_rte",
    "vstorea_half8_rte",
    "vstorea_half16_rte",
    "vstorea_half_rtz",
    "vstorea_half2_rtz",
    "vstorea_half3_rtz",
    "vstorea_half4_rtz",
    "vstorea_half8_rtz",
    "vstorea_half16_rtz",
    "vstorea_half_rtp",
    "vstorea_half2_rtp",
    "vstorea_half3_rtp",
    "vstorea_half4_rtp",
    "vstorea_half8_rtp",
    "vstorea_half16_rtp",
    "vstorea_half_rtn",
    "vstorea_half2_rtn",
    "vstorea_half3_rtn",
    "vstorea_half4_rtn",
    "vstorea_half8_rtn",
    "vstorea_half16_rtn",
    NULL
};

struct AMDBuiltinMapperTableEntry {
  const char** names;
  AMDBuiltinType type;
} AMDBuiltinMapperTable[] = {
    {amdBuiltinsPar1, AMD_BUILTIN_PAR1},
    {amdBuiltinsPar2, AMD_BUILTIN_PAR2},
    {amdBuiltinsPar3, AMD_BUILTIN_PAR3},
    {amdBuiltinsPar1Par2, AMD_BUILTIN_PAR1_PAR2},
    {amdBuiltinsPar1Par3, AMD_BUILTIN_PAR1_PAR3},
    {amdBuiltinsPar1ParN, AMD_BUILTIN_PAR1_PARN},
    {amdBuiltinsPar3Par1, AMD_BUILTIN_PAR3_PAR1},
    {NULL, AMD_BUILTIN_INVALID}
};

class AMDBuiltinMapper {
  public:
    AMDBuiltinMapper() {
      if (builtinMap.size() == 0) {
        initMap();
      }
    }
    std::string map(const std::string& name, llvm::Function* func,
        const std::string& nameWithPar) {
      AMDBuiltinType builtinMapType;
      AMDConvertFunctionNameParser convParser;

      // read_image function has two mangling schemes
      if (name == "read_imagef" ||
          name == "read_imagei" ||
          name == "read_imageui" ||
          name == "read_imageh") {
          if (func->getFunctionType()->getNumParams() == 2) {
            builtinMapType = AMD_BUILTIN_PAR1;
          } else if (func->getFunctionType()->getNumParams() == 3) {
            builtinMapType = AMD_BUILTIN_PAR1_PARN;
          } else {
            assert (0 && "invalid read_image function name");
          }
      } else if (convParser.parse(name)) {
        builtinMapType = AMD_BUILTIN_PAR1;
      } else {
        std::map<std::string, AMDBuiltinType>::iterator iter
          = builtinMap.find(name);
        if (iter == builtinMap.end()) {
          return name;
        } else
          builtinMapType = iter->second;
      }

      SmallVector<bool, 5> isUnsigned;
      getUnsignedness(nameWithPar, isUnsigned);

      //ToDo: remove this after development is done
      if (getenv("AMD_OCL_DUMP_BUILTIN") != NULL) {
        llvm::errs() << "[map] " << nameWithPar << "\n";
      }

      if (builtinMapType == AMD_BUILTIN_PAR1) {
        return std::string("__") + name + "_"
          + mapType(func->getFunctionType()->getFunctionParamType(0),
              isUnsigned[0]);
      }
      else if (builtinMapType == AMD_BUILTIN_PAR2) {
        return std::string("__") + name + "_"
          + mapType(func->getFunctionType()->getFunctionParamType(1),
              isUnsigned[1]);
      }
      else if (builtinMapType == AMD_BUILTIN_PAR3) {
        return std::string("__") + name + "_"
          + mapType(func->getFunctionType()->getFunctionParamType(2),
              isUnsigned[2]);
      }
      else if (builtinMapType == AMD_BUILTIN_PAR1_PAR2) {
        return std::string("__") + name + "_"
          + mapType(func->getFunctionType()->getFunctionParamType(0),
              isUnsigned[0])
          + "_"
          + mapType(func->getFunctionType()->getFunctionParamType(1),
              isUnsigned[1]);
      }
      else if (builtinMapType == AMD_BUILTIN_PAR1_PAR3) {
        return std::string("__") + name + "_"
          + mapType(func->getFunctionType()->getFunctionParamType(0),
              isUnsigned[0])
          + "_"
          + mapType(func->getFunctionType()->getFunctionParamType(2),
              isUnsigned[2]);
      }
      else if (builtinMapType == AMD_BUILTIN_PAR1_PARN) {
        int last = func->getFunctionType()->getNumParams();
        return std::string("__") + name + "_"
          + mapType(func->getFunctionType()->getFunctionParamType(0),
              isUnsigned[0])
          + "_"
          + mapType(func->getFunctionType()->getFunctionParamType(last-1),
              isUnsigned[last-1]);
      }
      else if (builtinMapType == AMD_BUILTIN_PAR3_PAR1) {
        return std::string("__") + name + "_"
          + mapType(func->getFunctionType()->getFunctionParamType(2),
              isUnsigned[2])
          + "_"
          + mapType(func->getFunctionType()->getFunctionParamType(0),
              isUnsigned[0]);
      }
      else {
        DEBUG(dbgs() << "unknown builtin type: " << name << "\n");
        return "unknown";
      }
    }
  private:
    static std::map<std::string, AMDBuiltinType> builtinMap;
    static void initMap () {
      for (int i = 0; AMDBuiltinMapperTable[i].names != NULL; ++i) {
        for (int j = 0; AMDBuiltinMapperTable[i].names[j] != NULL; ++j) {
#if 0
          llvm::errs() << "map " << AMDBuiltinMapperTable[i].names[j]
            << " to " << AMDBuiltinMapperTable[i].type << "\n";
#endif
          builtinMap[std::string(AMDBuiltinMapperTable[i].names[j])] =
              AMDBuiltinMapperTable[i].type;
        }
      }
    }

    char mapAddrSpace(unsigned int addrSpace) {
      switch(addrSpace) {
      case 0: // private
        return 'p';
      case 1: // global
        return 'g';
      case 2: // constant
        return 'c';
      case 3: // local
        return 'l';
      case 4: // global with endian host
        return 'G';
      case 5: // constant with endian host
        return 'C';
      default: // unknown
        return 'x';
      }
    }

    std::string mapType(llvm::Type* type, bool isUnsigned) {
      if (type->isFloatTy()) {
        return "f32";
      }
      else if (type->isDoubleTy()) {
        return "f64";
      }
      else if (type->isHalfTy()) {
        return "f16";
      }
      else if (type->isIntegerTy()) {
        std::ostringstream stream;
        if (isUnsigned)
          stream << "u";
        else
          stream << "i";
        stream << type->getIntegerBitWidth();
        return stream.str();
      }
      else if (llvm::PointerType* ptrTy = dyn_cast<llvm::PointerType>(type)) {
        std::ostringstream stream;
        llvm::Type* elTy = ptrTy->getElementType();
        // Do not map address space for image type
        if (!elTy->isStructTy())
          stream << mapAddrSpace(ptrTy->getAddressSpace());
        stream << mapType(elTy, isUnsigned);
        return stream.str();
      }
      else if (type->isVectorTy()) {
        llvm::VectorType* vecTy = dyn_cast<llvm::VectorType>(type);
        std::ostringstream stream;
        stream << vecTy->getNumElements()
            << mapType(vecTy->getElementType(), isUnsigned);
        return stream.str();
      }
      else if (llvm::StructType* struTy = dyn_cast<llvm::StructType>(type)) {
        return mapStructTypeNameForBuiltin(struTy);
      }
      return "unknown";
    }

    void getUnsignedness(const std::string& demangledName,
        llvm::SmallVector<bool, 5>& unsignedness) {
      llvm::StringRef nameWithPar(demangledName);
      llvm::SmallVector<llvm::StringRef, 5> splitStr;
      nameWithPar = nameWithPar.substr(nameWithPar.find_first_of("(")+1);
      nameWithPar.split(splitStr, ",");
      for (unsigned int i = 0; i < splitStr.size(); ++i) {
        //llvm::errs() << "split " << splitStr[i] << "\n";
        // pointer argument with address space has two items in the demangled
        // name, skip the first item
        // TODO: fix the demangler for address space
        if (splitStr[i].find("*")!=StringRef::npos &&
            splitStr[i].find("AS")!=StringRef::npos) {
          continue;
        }
        bool isUnsigned = splitStr[i].find("unsigned")!=StringRef::npos;
        isUnsigned |= splitStr[i].find("uchar")!=StringRef::npos;
        isUnsigned |= splitStr[i].find("ushort")!=StringRef::npos;
        isUnsigned |= splitStr[i].find("uint")!=StringRef::npos;
        isUnsigned |= splitStr[i].find("ulong")!=StringRef::npos;
        unsignedness.push_back(isUnsigned);
      }
    }
};

std::map<std::string, AMDBuiltinType> AMDBuiltinMapper::builtinMap;

struct Demangler {
  MathFunctionTransform& mathTran;
  public:
    Demangler(MathFunctionTransform& aMathTran):mathTran(aMathTran){
    }
    void operator()(Function &fn) {
      if (!getenv("AMD_SPIR_DEMANGLE_KERNEL") &&
          fn.getCallingConv() == CallingConv::SPIR_KERNEL)
        return;

      AMDBuiltinMapper mapper;
      std::string mangledName = fn.getName().str();
      int status = 0;
      const char* tmpname = __cxxabiv1::__cxa_demangle(
          mangledName.c_str(), 0, 0, &status);
      if (status || !tmpname) return;
      std::string unmangledName(tmpname);
      std::string funcName = unmangledName.substr(0, unmangledName.find('('));
      std::string mappedName = mapper.map(funcName, &fn, unmangledName);
      mathTran.collect(&fn, funcName);
      fn.setName(mappedName);
      DEBUG(llvm::errs() << "[Demangler] " << unmangledName << " -> "
          << mappedName << "\n";);
#ifdef DEBUG
      // Dump mapping of EDG mangled builtin function name to IA64 mangled name.
      if (getenv("AMD_SPIR_DUMP_BUILTIN")) {
        static FILE* file = 0;
        if (file == 0) {
          file = fopen("_builtin.txt", "w");
        }
        fprintf(file, "[builtin] \"%s\", \"%s\",\n", mappedName.c_str(),
            mangledName.c_str());
        fflush(file);
      }
#endif
    }
};

void
demangleNames(Module &M, MathFunctionTransform& mathTran)
{
  Demangler fn_dm(mathTran);
  if (M.empty()) return;
  std::for_each(M.begin(), M.end(), fn_dm);
}

void
replaceTypesInModule(Module &M, Type **type_table)
{
  TypeMutator<GlobalVariable> gv_tm(type_table);
  TypeMutator<GlobalAlias> ga_tm(type_table);
  TypeMutator<Function> fn_tm(type_table);
  // Iterate through all of the globals, mutating the types.
  if (!M.global_empty())
    std::for_each(M.global_begin(), M.global_end(), gv_tm);

  // Iterate through all of the functions, mutating the types.
  if (!M.empty())
    std::for_each(M.begin(), M.end(), fn_tm);

  // Iterator through all of the global aliases, mutating the types.
  if (!M.alias_empty())
    std::for_each(M.alias_begin(), M.alias_end(), ga_tm);

}

// The ValueRemover class removes from the module all functions
// and values that have references to the SPIR data type. At
// the point where this is executed, all SPIR specific data types
// should be lowered to the device specific data type.
template <typename T>
struct ValueRemover {
  ValueRemover(); // Do Not Implement.
  public:
    ValueRemover(Type **tbl) : type_table(tbl) {
    }

    ValueRemover(const ValueRemover &tm) {
      type_table = tm.type_table;
    }

    virtual ~ValueRemover() {
      clear();
    }

    void clear() {
      for (std::set<GlobalValue*>::iterator bi = deadValues.begin(),
          be = deadValues.end(); bi != be; ++bi) {
        DEBUG(
          dbgs() << "Erasing: ";
          (*bi)->dump();
          dbgs() << "\n";
        );
        (*bi)->eraseFromParent();
      }
      deadValues.clear();
    }

  protected:
    std::set<GlobalValue*> deadValues;
    Type **type_table;

    void removeFunction(Function *F)
    {
      // If the function is still used or we aren't a function, then
      // return without processing.
      if (!F || F->getNumUses()) return;
      uint32_t x = 0;
      // if either the return value or any of the arguments are a
      // SPIR type and we don't have any uses, then we should
      // remove the function.
      if (isSPIRType(type_table, F->getReturnType(), x)) {
        deadValues.insert(F);
        return;
      }
      for (Function::const_arg_iterator ab = F->arg_begin(),
          ae = F->arg_end(); ab != ae; ++ab) {
        x = 0;
        if (isSPIRType(type_table, ab->getType(), x)) {
          deadValues.insert(F);
          return;
        }
      }
      StringRef name = F->getName();
      // FIXME: This needs to be removed once the frontend/libraries
      // use the correct name mangling scheme.
      if (name.find("__spir") != StringRef::npos
          && name.endswith("_spir")) {
          deadValues.insert(F);
          return;
      }
    }

    void remove(T *V)
    {
      removeFunction(dyn_cast<Function>(V));
      uint32_t x = 0;
      if (!V) return;
      if (isSPIRType(type_table, V->getType(), x)
          && !V->getNumUses())
        deadValues.insert(V);
    }

  public:
    bool mDebug;
    void operator()(T &V) { remove(&V); }
    void operator()(T *V) { remove(V); }
}; // struct ValueRemover

void
removeUnusedGlobals(Module &M, Type **type_table)
{
  ValueRemover<GlobalVariable> gv_rm(type_table);
  ValueRemover<Function> fn_rm(type_table);
  ValueRemover<GlobalAlias> ga_rm(type_table);
  // Iterate through all of the globals, removing the unused types.
  if (!M.global_empty())
    std::for_each(M.global_begin(), M.global_end(), gv_rm);

  // Iterate through all of the functions, removing the unused types.
  if (!M.empty())
    std::for_each(M.begin(), M.end(), fn_rm);

  // Iterator through all of the global aliases, removing the unused types.
  if (!M.alias_empty())
    std::for_each(M.alias_begin(), M.alias_end(), ga_rm);
}

// Find functions that are SPIR kernels and rename them using the
// naming convention currently used by opencl by prefixing all
// the kernels with '__OpenCL_' and postfixing with '_kernel'.
void
renameSPIRFunctions(Module &M, Type **type_table)
{
  AMDBuiltinMapper mapper;
  if (!M.empty()) {
    std::set<Function*> deadFunc;
    for (Module::iterator fi = M.begin(), fe = M.end();
        fi != fe; ++fi) {
      Function *oldFunc = fi;
      StringRef name = oldFunc->getName();
      bool replace = false;
      std::string newName;
      if (fi->getCallingConv() == CallingConv::SPIR_KERNEL
          && !(name.startswith("__OpenCL_")
            && name.endswith("_kernel"))) {
        std::string newName = name;
        newName = "__OpenCL_" + newName + "_kernel";
        fi->setName(newName);
      }
      else if (name.endswith("_spir")) {
        newName = name.substr(0, name.size()-5);
        replace = true;
      }
      if (replace && deadFunc.find(oldFunc) == deadFunc.end()) {
        FunctionType *oldFT = oldFunc->getFunctionType();
        Type *ret = oldFunc->getReturnType();
        SmallVector<Type *, 10> args(oldFT->param_begin(), oldFT->param_end());
        FunctionType *newFT = FunctionType::get(ret, args, oldFT->isVarArg());
        Function *newFunc = M.getFunction(newName);
        if (newFunc == NULL) {
          newFunc = Function::Create(newFT, oldFunc->getLinkage(), newName,
              oldFunc->getParent());
          newFunc->setCallingConv(oldFunc->getCallingConv());
        }
        DEBUG(llvm::errs() << "[renameSPIRfunctions]" << *oldFunc << " -> " <<
            *newFunc << "\n";);
        oldFunc->replaceAllUsesWith(newFunc);
        deadFunc.insert(oldFunc);
      }
    }
    std::for_each(deadFunc.begin(), deadFunc.end(),
        std::mem_fun(&Function::eraseFromParent));
  }
}

class AnnotationGlobalVarGenerator {
  Module& M;
  bool targetIsGPU;
  LLVMContext& llvmContext;
  DataLayout DL;
  PointerType* pointerTy;
  StructType* structTy;

  Constant* getConstantExpr(std::string str) {
    return ConstantExpr::getBitCast(getConstantStrVar(M, str, ".str"),
        pointerTy);
  }

  void addConstVar(std::vector<Constant*>& vec, const std::string& str) {
    vec.push_back(getConstantExpr(str));
  }

  void addConstVar(std::vector<Constant*>& vec, Constant* strVal,
      int intVal) {
    std::vector<llvm::Constant*> fieldVal;
    fieldVal.push_back(strVal);
    fieldVal.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(
      llvmContext), intVal));
    vec.push_back(llvm::ConstantStruct::get(structTy, fieldVal));
  }

  void initPointerType() {
    pointerTy = PointerType::getUnqual(Type::getInt8Ty(llvmContext));
  }

  void initStructType() {
    std::vector<llvm::Type*> fields;
    fields.push_back(pointerTy);
    fields.push_back(llvm::Type::getInt32Ty(llvmContext));
    structTy = StructType::get(llvmContext, fields, true);
  }

  std::string createSgvString(llvm::MDNode* rgsh, llvm::MDNode* rwgs,
      llvm::MDNode* rwrs, llvm::MDNode* vth);

  // get auto arrays of a function with certain address space
  void getAutoArraysByAddrSpace(Function& F, std::vector<Constant*>&,
      unsigned int);

  // get auto local arrays of a function
  void getAutoLocalArrays(Function& F, std::vector<Constant*>& found) {
    getAutoArraysByAddrSpace(F, found, LOCAL_ADDRESS);
  }

  // get auto region arrays of a function
  void getAutoRegionArrays(Function& F, std::vector<Constant*>& found) {
    getAutoArraysByAddrSpace(F, found, REGION_ADDRESS);
  }

  // cast a vector of global variables to a vector of pointers
  void cast(std::vector<GlobalVariable*>& in, std::vector<Constant*>& out) {
    out.clear();
    for (unsigned i = 0; i < in.size(); ++i)
      out.push_back(ConstantExpr::getBitCast(in[i], pointerTy));
  }

  class ArgNameExpr {
    AnnotationGlobalVarGenerator& generator;
    Constant* argNameExpr;
  public:
    ArgNameExpr(AnnotationGlobalVarGenerator& aGenerator)
      :generator(aGenerator),
       argNameExpr(NULL)
    {}
    Constant* get(const std::string& name) {
      if (argNameExpr == NULL) {
        argNameExpr = generator.getConstantExpr(name);
      }
      return argNameExpr;
    }
  };

public:
  AnnotationGlobalVarGenerator(Module& aModule, bool aTargetIsGPU):
    M(aModule),
    targetIsGPU(aTargetIsGPU),
    llvmContext(M.getContext()),
    DL(&M){
    initPointerType();
    initStructType();
  }
  PointerType* getPointerTy() {
    return pointerTy;
  }
  void generate(Function* F, std::vector<Constant*>&);
  void generate(void);
  void emitMetaDataAnnotation(std::vector<Constant*>& eles,
                              Type* eleTy,
                              const std::string& metaNameC);

  static void fixKernelArgTypeName(std::string& typeName, llvm::Type* argType);
  static void fixKernelArgAddrSpace(int& addrSpace, llvm::Type* argType);
};

// Create constants for a kernel argument and add them to a vector
class ArgToConstForCPU {
  LLVMContext& context;
  Module& M;
  PointerType* paramNameType;
  StructType* paramStructType;
  MDNode* mdTypeName;
  MDNode* mdAddrQual;
  MDNode* mdAccQual;
  MDNode* mdTypeQual;
  MDNode* mdBaseType;
  std::vector<Constant*>& results;
  int count;
  Constant* dummyName;
public:
  ArgToConstForCPU(LLVMContext& aContext, Module& aModule, Constant* aDummyName,
      PointerType* aParamNameType,
      StructType* aParamStructType, MDNode* aMdTypeName, MDNode* aMdAddrQual,
      MDNode* aMdAccQual, MDNode* aMdTypeQual, MDNode* aMdBaseType,
      std::vector<Constant*>& theResults)
    :context(aContext), M(aModule),
     paramNameType(aParamNameType),
     paramStructType(aParamStructType), mdTypeName(aMdTypeName),
     mdAddrQual(aMdAddrQual), mdAccQual(aMdAccQual), mdTypeQual(aMdTypeQual),
     mdBaseType(aMdBaseType), results(theResults), count(0),
     dummyName(aDummyName){
  }

  // Map llvm integer type to clk type id
  int map(IntegerType* type) {
    int bitWidth = type->getBitWidth();
    switch(bitWidth) {
    case 8: return clk::T_CHAR;
    case 16: return clk::T_SHORT;
    case 32: return clk::T_INT;
    case 64: return clk::T_LONG;
    default: assert("unexpected integer size"==NULL);
    }
    return 0;
  }

  // Map llvm vector type to clk type id
  int map(VectorType* type) {
    static int vectorCode[6][5] =
      {{clk::T_CHAR2,  clk::T_CHAR3,  clk::T_CHAR4,  clk::T_CHAR8,  clk::T_CHAR16},
      {clk::T_SHORT2, clk::T_SHORT3, clk::T_SHORT4, clk::T_SHORT8, clk::T_SHORT16},
      {clk::T_INT2,   clk::T_INT3,   clk::T_INT4,   clk::T_INT8,   clk::T_INT16},
      {clk::T_LONG2,  clk::T_LONG3,  clk::T_LONG4,  clk::T_LONG8,  clk::T_LONG16},
      {clk::T_FLOAT2, clk::T_FLOAT3, clk::T_FLOAT4, clk::T_FLOAT8, clk::T_FLOAT16},
      {clk::T_DOUBLE2, clk::T_DOUBLE3, clk::T_DOUBLE4, clk::T_DOUBLE8, clk::T_DOUBLE16}};
    int numEle = type->getNumElements();
    int idx;
    switch(numEle) {
    case 2: idx = 0; break;
    case 3: idx = 1; break;
    case 4: idx = 2; break;
    case 8: idx = 3; break;
    case 16: idx = 4; break;
    default: idx = -1; assert("unexpected vector size"==NULL); break;
    }
    int eleTy = map(type->getElementType());
    assert (eleTy >= clk::T_CHAR && eleTy <= clk::T_DOUBLE);
    return vectorCode[eleTy - clk::T_CHAR][idx];
  }

  // Map llvm type to clk type id
  int map(llvm::Type* type) {
    llvm::Type::TypeID typeId = type->getTypeID();
    switch(typeId) {
    case llvm::Type::VoidTyID:    return clk::T_VOID;
    case llvm::Type::FloatTyID:   return clk::T_FLOAT;
    case llvm::Type::DoubleTyID:  return clk::T_DOUBLE;
    case llvm::Type::PointerTyID: return clk::T_POINTER;
    default: break;
    }
    if (IntegerType* intTy = dyn_cast<IntegerType>(type))
      return map(intTy);
    else if (VectorType* vecTy = dyn_cast<VectorType>(type))
      return map(vecTy);
    assert("unhandled type"==NULL);
    return -1;
  }

  // Add one entry to results knowing the type code
  void add(int typeCode, int addrSpace, unsigned qual, Constant* name) {
    std::vector<Constant*> fields;
    fields.push_back(ConstantInt::get(context, APInt(32, typeCode, true)));
    fields.push_back(ConstantInt::get(context, APInt(32, addrSpace, true)));
    fields.push_back(ConstantInt::get(context, APInt(32,
        static_cast<uint64_t>(qual), false)));
    fields.push_back(name);
    results.push_back(ConstantStruct::get(paramStructType, fields));
// TODO: remove this after development is done
#ifdef DEBUG
    if (getenv("AMD_OCL_DUMP_CPUMETA") != NULL) {
        llvm::errs() << "[ArgToConstForCPU] "
            << " typeCode=" << typeCode
            << " addrSpaceCode=" << addrSpace
            << " typeQualifierCode=" << qual
            << " name=" << *name << "\n";
    }
#endif
  }

  // Add one entry to results
  void add(llvm::Type* type, int addrSpace, int qual, Constant* name) {
    if (StructType* structTy = dyn_cast<StructType>(type)) {
      for (StructType::element_iterator I = structTy->element_begin(),
          E = structTy->element_end(); I != E; ++I) {
        add(*I, addrSpace, clk::Q_NONE, name);
      }
    } else if (ArrayType* arrayTy = dyn_cast<ArrayType>(type)) {
      llvm::Type* eleTy = arrayTy->getElementType();
      for (uint64_t i = 0, e = arrayTy->getNumElements(); i < e; ++i) {
        add(eleTy, addrSpace, clk::Q_NONE, name);
      }
    } else if (PointerType* pointerTy = dyn_cast<PointerType>(type)) {
      if (pointerTy->getAddressSpace() == PRIVATE_ADDRESS) {
        if (StructType* structTy = dyn_cast<StructType>(pointerTy->getElementType())) {
          add(clk::T_STRUCT, clk::A_PRIVATE, clk::Q_NONE, name);
          add(structTy, clk::A_PRIVATE, clk::Q_NONE, name);
          add(clk::T_VOID, clk::A_PRIVATE, clk::Q_NONE, name);
        } else {
          add(map(type), addrSpace, qual, name);
        }
      } else {
        add(map(type), addrSpace, qual, name);
      }
    } else {
      add(map(type), addrSpace, qual, name);
    }
  }

  Constant* convert(std::string aString) {
    Constant* var = getConstantStrVar(M, aString, ".str");
    Value* idxs[] = {
        ConstantInt::get(llvm::Type::getInt32Ty(context), 0),
        ConstantInt::get(llvm::Type::getInt32Ty(context), 0),
    };
    return ConstantExpr::getGetElementPtr(var, idxs);
  }

  void operator()(Argument &arg) {
    llvm::Type* argTy = arg.getType();
    int addrSpaceToken = CLS_PRIVATE_AS;
    if (mdAddrQual)
      addrSpaceToken = getMDOperandAsInt(mdAddrQual, count+1);
    AnnotationGlobalVarGenerator::fixKernelArgAddrSpace(addrSpaceToken, argTy);
    addrSpaceToken = mapSpirAddrSpaceToRuntimeValue(addrSpaceToken);

    int qualifier;
    std::string accQual;
    if (mdAccQual)
      accQual = getMDOperandAsString(mdAccQual, count+1);
    std::string typeQual;
    if (mdTypeQual)
      typeQual = getMDOperandAsString(mdTypeQual, count+1);
    qualifier = mapSpirAccessAndTypeQualifier(accQual, typeQual, argTy);

    std::string typeName;
    if (mdTypeName)
      typeName = getMDOperandAsString(mdTypeName, count+1);
    AnnotationGlobalVarGenerator::fixKernelArgTypeName(typeName, argTy);

    std::string baseType;
    if (mdBaseType) {
      baseType = getMDOperandAsString(mdBaseType, count+1);
    }
    else {
      baseType = typeName;
      DEBUG(dbgs() << "Warning: kernel_arg_base_type missing!\n");
    }

    // add global var for argument name
    std::string argName = arg.getName();
    if (argName.empty()) {
      std::stringstream stream;
      stream << "arg" << count+1;
      argName = stream.str();
    }
    if (baseType == "sampler_t") {
      add(clk::T_SAMPLER, addrSpaceToken, qualifier, convert(argName));
    } else {
      add(argTy, addrSpaceToken, qualifier, convert(argName));
    }

    // add global var for argument type name
    add(clk::T_VOID, clk::A_PRIVATE, clk::Q_NONE, convert(typeName));
    count++;
  }
};


// Generate global variables containing kernel information for CPU
void generateOpenCLGlobalsForCPU(Module& M, Function& F, MDNode* rgsh,
    MDNode* rwgs, MDNode* rwrs, MDNode* vth, MDNode* atn, MDNode* addq,
    MDNode* accq, MDNode* atq, MDNode* abt) {
  LLVMContext& context = M.getContext();
  DataLayout DL(&M);

  unsigned numArgs = F.getFunctionType()->getNumParams();

  // Create some basic types which we will use
  IntegerType* sizetType = DL.getIntPtrType(context);

  // Create structure type for workgroup size
  std::vector<llvm::Type*> fields;
  for (int i = 0; i < 3; ++i)
    fields.push_back(sizetType);
  StructType* workgroupStructType =StructType::get(context, fields, true);
  std::vector<ConstantInt*> rqWgSize(3);

  // Create structure type for kernel argument
  fields.clear();
  for (int i = 0; i < 3; ++i)
    fields.push_back(llvm::Type::getInt32Ty(context));
  PointerType* pointerType = PointerType::get(llvm::Type::getInt8Ty(context),
      CONSTANT_ADDRESS);
  fields.push_back(pointerType);
  StructType* paramStructType = StructType::get(context, fields, true);

  Constant* dummyName;
  dummyName = ConstantInt::get(context, APInt(32, 0, true));
  dummyName = ConstantExpr::getCast(Instruction::IntToPtr,
      dummyName, pointerType);

  std::vector<Constant*> argInfo;
  ArgToConstForCPU argToConst(context, M, dummyName, pointerType,
      paramStructType, atn, addq, accq, atq, abt, argInfo);
  std::for_each(F.arg_begin(), F.arg_end(), argToConst);
  argToConst.add(clk::T_VOID, clk::A_PRIVATE, clk::Q_NONE, dummyName);

  // create string for vec_type_hint
  GlobalVariable* vthConst = NULL;
  if (vth != NULL) {
    vthConst = getConstantStrVar(M, decodeVecTypeHintMDNode(vth), ".str");
  }

  ArrayType* paramArrayType = ArrayType::get( paramStructType, argInfo.size());

  // Create structure type for the global variable
  fields.clear();
  fields.push_back(sizetType); // size
  fields.push_back(sizetType); // local mem size
  fields.push_back(workgroupStructType); // reqd_workgroup_size
  fields.push_back(workgroupStructType); // workgroup_size_hint
  fields.push_back(paramArrayType); // kernel arg info
  unsigned metaStructSize = DL.getTypeSizeInBits(sizetType) * 2 +
      DL.getTypeSizeInBits(workgroupStructType) * 2 +
      DL.getTypeSizeInBits(paramArrayType);
  if (vthConst != NULL) {
    fields.push_back(vthConst->getType()); // vec_type_hint
    metaStructSize += DL.getTypeSizeInBits(vthConst->getType());
  }
  StructType* metaStructType = StructType::get(context, fields, true);

  // Create the global variable
  std::string name = F.getName();
  name = name.substr(0, name.rfind("_")) + "_metadata";

  GlobalValue::LinkageTypes Linkage = GlobalValue::isWeakLinkage(F.getLinkage())
                                    ? GlobalValue::WeakAnyLinkage
                                    : GlobalValue::ExternalLinkage;
  GlobalVariable* glb = new GlobalVariable(M,
                                           metaStructType,
                                           false,
                                           Linkage,
                                           0,
                                           name,
                                           0,
                                           GlobalVariable::NotThreadLocal,
                                           0);
  glb->setAlignment(DL.getPointerABIAlignment());

  // Initialize the global variable
  std::vector<Constant*> values;
  std::vector<Constant*> wgValues(3);
  values.push_back(ConstantInt::get(sizetType, metaStructSize/8, false));
  values.push_back(ConstantInt::get(sizetType, 0, false));
  decodeMDNode(rwgs, sizetType, wgValues);
  values.push_back(ConstantStruct::get(workgroupStructType, wgValues));
  decodeMDNode(rgsh, sizetType, wgValues);
  values.push_back(ConstantStruct::get(workgroupStructType, wgValues));
  values.push_back(ConstantArray::get(paramArrayType, argInfo));
  if (vthConst != NULL) {
    values.push_back(vthConst);
  }
  glb->setInitializer(ConstantStruct::get(metaStructType, values));
#ifdef DEBUG
  if (getenv("AMD_OCL_DUMP_CPUMETA")) {
  llvm::errs() << "[CPU llvmmeta] size=" << metaStructSize/8 << "\n";
  }
#endif
}



// Check each internal global variable with local address space. If it is used
// by a function then added it to the auto array annotation for that
// function.
void AnnotationGlobalVarGenerator::getAutoArraysByAddrSpace(Function& F,
    std::vector<Constant*>& found, unsigned int addrSpace) {
  std::vector<GlobalVariable*> GVs;
  AMDSpir::getAutoArraysByAddrSpace(F, GVs, addrSpace, true);
  cast(GVs, found);
}

// create sgv based on metadata
// rgsh - required work group size hint
// rwgs - required work group size
// rwrs - required work region size
// vth - vector type hint
std::string AnnotationGlobalVarGenerator::createSgvString(llvm::MDNode* rgsh,
    llvm::MDNode* rwgs, llvm::MDNode* rwrs, llvm::MDNode* vth) {
  int x, y, z;
  std::ostringstream sgvString;

  x = y = z = 0;
  decodeMDNode(rwgs, x, y, z);

  //encode user request work group size
  if (x != 0 || y != 0 || z != 0)
  {
      sgvString << "RWG" << x << "," << y << "," << z;
  }

  x = y = z = 0;
  decodeMDNode(rgsh, x, y, z);

  //encode user hint work group size
  if (x != 0 || y != 0 || z != 0)
  {
      sgvString << "WGH" << x << "," << y << "," << z;
  }

  x = y = z = 0;
  decodeMDNode(rwrs, x, y, z);

  // encode user request work region size
  if (x != 0 || y != 0 || z != 0) {
      sgvString << "RWR" << x << "," << y << "," << z;
  }

  // encode vector type hint
  if (vth != NULL) {
    sgvString << "VTH" << decodeVecTypeHintMDNode(vth);
  }

  return sgvString.str();
}

void
AnnotationGlobalVarGenerator::generate(Function* F,
                                       std::vector<Constant*> &annotations)
{
  bool inferArgInfo = getenv("AMD_SPIR_INFER")!=NULL;

  if (F->getCallingConv() != CallingConv::SPIR_KERNEL) return;
  std::string funcName = F->getName();
  NamedMDNode *SPIRFunctions = M.getNamedMetadata(SPIR_METADATA_KERNEL_QUALIFIERS);
  assert (SPIRFunctions != NULL);
  MDNode *funcPtr;
  for (unsigned x = 0, y = SPIRFunctions->getNumOperands(); x < y; ++x) {
    MDNode *ptr = SPIRFunctions->getOperand(x);
    if (ptr->getNumOperands() == 0) {
      continue;
    }
    Function *mdF = dyn_cast<Function>(ptr->getOperand(0));
    if (!F) {
      continue;
    }
    if (F == mdF) {
      funcPtr = ptr;
      break;
    }
  }

  if (!funcPtr) return;
  MDNode *ckai = NULL; // cl-kernel-arg-info
  MDNode *wgsh = NULL; // work_group_size_hint
  MDNode *rwgs = NULL; // reqd_work_group_size
  MDNode *rwrs = NULL; // reqd_work_region_size
  MDNode *vth = NULL;  // vec_type_hint
  MDNode *atn = NULL;  // arg_type_name
  MDNode *abt = NULL;  // arg_base_type
  MDNode *an = NULL;   // arg_name
  MDNode *atq = NULL;  // arg_type_qualifier
  MDNode *addq = NULL; // address_qualifier
  MDNode *accq = NULL; // access_qualifier
  for (unsigned x = 1, y = funcPtr->getNumOperands(); x < y; ++x) {
    MDNode *mdptr = dyn_cast<MDNode>(funcPtr->getOperand(x));
      if (!mdptr) {
        continue;
      }
      MDString *mdname = dyn_cast<MDString>(mdptr->getOperand(0));
      if (!mdname) {
        continue;
      }
      StringRef id = mdname->getString();
      if (id == "work_group_size_hint") {
        wgsh = mdptr;
      } else if (id == "reqd_work_group_size") {
        rwgs = mdptr;
      } else if (id == "reqd_work_region_size") {
        rwrs = mdptr;
      } else if (id == "vec_type_hint") {
        vth = mdptr;
      } else if (id == SPIR_METADATA_KERNEL_ARG_ADDR_SPACE) {
        addq = mdptr;
      } else if (id == SPIR_METADATA_KERNEL_ARG_ACCESS_QUAL) {
        accq = mdptr;
      } else if (id == SPIR_METADATA_KERNEL_ARG_TYPE) {
        atn = mdptr;
      } else if (id == SPIR_METADATA_KERNEL_ARG_BASE_TYPE) {
        abt = mdptr;
      } else if (id == SPIR_METADATA_KERNEL_ARG_TYPE_QUAL) {
        atq = mdptr;
      } else if (id == SPIR_METADATA_KERNEL_ARG_NAME) {
        an = mdptr;
      }
  }

  if (targetIsGPU) {
    int numOp = accq->getNumOperands();

    std::vector<Constant*> imageMetaEles;
    std::vector<Constant*> samplerMetaEles;
    std::vector<Constant*> constPointerEles;
    std::vector<Constant*> readonlyPointerEles;
    std::vector<Constant*> signedOrSignedPointeeEles;
    std::vector<Constant*> volatilePointerEles;
    std::vector<Constant*> restrictPointerEles;
    std::vector<Constant*> argTypeNameEles;
    std::vector<Constant*> argTypeConstEles;

    Function::arg_iterator AI = F->arg_begin(), AE = F->arg_end();
    for (int i = 1; i < numOp; ++i, ++AI) {
      Attributes attr = F->getParamAttributes(i-1);
      llvm::Type* llvmType = AI->getType();

      // Get argument name. If no name create a name
      std::string argName = AI->getName();
      if (argName == "") {
        std::ostringstream stream;
        stream << "arg" << i-1;
        argName = stream.str();
        AI->setName(argName);
      }
      ArgNameExpr argNameExpr(*this);

      // Get type name
      std::string typeName;
      if (atn) {
        typeName = dyn_cast<MDString>(atn->getOperand(i))->getString().str();
      }
      fixKernelArgTypeName(typeName, AI->getType());

      // Get base type name
      std::string baseTypeName;
      if (abt) {
        baseTypeName = dyn_cast<MDString>(abt->getOperand(i))->getString().str();
      } else {
        baseTypeName = typeName;
        DEBUG(dbgs() << "Warning: kernel_arg_base_type is missing!\n");
      }

      // Get type qualifier
      std::string typeQual;
      if (atq) {
        typeQual = dyn_cast<MDString>(atq->getOperand(i))->getString().str();
      }

      // Get access qualifier
      std::string accQual;
      if (accq) {
        accQual = dyn_cast<MDString>(accq->getOperand(i))->getString().str();
      }

      int accTypeQual = mapSpirAccessAndTypeQualifier(accQual, typeQual,
          NULL);

      // Create annotations for image type
      if (isTypeNameImage(baseTypeName)) {
        int amdilImgTy = mapSpirAccessQualifierToAMDILImageType(accQual);
        addConstVar (imageMetaEles, argNameExpr.get(argName), amdilImgTy);
      }

      // Create annotations for const arrays
      if (accTypeQual & clk::Q_CONST) {
        if (ArrayType* arrType = dyn_cast<ArrayType>(llvmType)) {
          addConstVar (constPointerEles, argNameExpr.get(argName),
              DL.getTypeSizeInBits(arrType));
        }
      }

      // Create annotations for sampler type
      if (baseTypeName == "sampler_t") {
        samplerMetaEles.push_back(argNameExpr.get(argName));
      }

      // Create annotations for read only pointer type
      if ((accTypeQual & clk::Q_RESTRICT) && (accTypeQual & clk::Q_CONST)) {
        readonlyPointerEles.push_back(argNameExpr.get(argName));
      }

      // Create annotations for signed or pointer-to-signed type
      if ((!inferArgInfo && isTypeNameSigned(baseTypeName))
          || (inferArgInfo && attr.hasAttribute(Attributes::SExt))) {
        signedOrSignedPointeeEles.push_back(argNameExpr.get(argName));
      }

      // Create annotations for restrict pointer type
      if (accTypeQual & clk::Q_RESTRICT) {
        restrictPointerEles.push_back(argNameExpr.get(argName));
      }

      // Create annotations for pointer-to-volatile type
      if (accTypeQual & clk::Q_VOLATILE) {
        volatilePointerEles.push_back(argNameExpr.get(argName));
      }

      // Create annotations for type name
      if (!typeName.empty()) {
        addConstVar(argTypeNameEles, typeName);
      }

      // Create annotations for pointer-to-const type
      if (accTypeQual & clk::Q_CONST) {
        argTypeConstEles.push_back(argNameExpr.get(argName));
      }
    }

    emitMetaDataAnnotation(imageMetaEles, structTy,
        std::string("llvm.image.annotations.") + funcName);
    emitMetaDataAnnotation(samplerMetaEles, pointerTy,
        std::string("llvm.sampler.annotations.") + funcName);
    emitMetaDataAnnotation(constPointerEles, structTy,
        std::string("llvm.constpointer.annotations.") + funcName);
    emitMetaDataAnnotation(readonlyPointerEles, pointerTy,
        std::string("llvm.readonlypointer.annotations.") + funcName);
    emitMetaDataAnnotation(signedOrSignedPointeeEles, pointerTy,
        std::string("llvm.signedOrSignedpointee.annotations.") + funcName);
    emitMetaDataAnnotation(restrictPointerEles, pointerTy,
        std::string("llvm.restrictpointer.annotations.") + funcName);
    emitMetaDataAnnotation(volatilePointerEles, pointerTy,
        std::string("llvm.volatilepointer.annotations.") + funcName);
    emitMetaDataAnnotation(argTypeNameEles, pointerTy,
        std::string("llvm.argtypename.annotations.") + funcName);
    emitMetaDataAnnotation(argTypeConstEles, pointerTy,
        std::string("llvm.argtypeconst.annotations.") + funcName);
  }

  if (!targetIsGPU) {
    generateOpenCLGlobalsForCPU(M, *F, wgsh, rwgs, rwrs, vth, atn, addq, accq,
        atq, abt);
  } else {
    llvm::Constant* sgvConst;
    sgvConst = getConstantStrVar(M, createSgvString(wgsh, rwgs, rwrs, vth), "sgv");

    Constant* fgvConst;
    std::string emptyStr;
    fgvConst = getConstantStrVar(M, emptyStr, "fgv");

    std::vector<Constant*> autoLocalArrayConst;
    std::vector<Constant*> autoRegionArrayConst;
    if (targetIsGPU) {
      getAutoLocalArrays(*F, autoLocalArrayConst);
      getAutoRegionArrays(*F, autoRegionArrayConst);
    }
    Constant* lvgvConst = ConstantArray::get(
          ArrayType::get(pointerTy, autoLocalArrayConst.size()), autoLocalArrayConst);
    lvgvConst = new GlobalVariable(M, lvgvConst->getType(), true,
        GlobalValue::InternalLinkage,
        lvgvConst, "lvgv");
    //llvmConstantExpr::getExtractElement

    Constant* rvgvConst = ConstantArray::get(
          ArrayType::get(pointerTy, autoRegionArrayConst.size()), autoRegionArrayConst);
    rvgvConst = new GlobalVariable(M, rvgvConst->getType(), true,
        GlobalValue::InternalLinkage,
        rvgvConst, "rvgv");
    // create the ConstantStruct
    // {i8*, i8*, i8*, i8*, i8*, i8*, i32)
    Constant* llvmField[6] = {
      //pointer to the kernel, cast to i8*
      ConstantExpr::getBitCast(F, pointerTy),
      //sgv, pointer to first element in a char array
      //the ending is comma separated user request work group size:  ,x, y, z
      ConstantExpr::getBitCast(sgvConst, pointerTy),
      //fgv, intend to be file name (not filled currently), pointer to first element
      ConstantExpr::getBitCast(fgvConst, pointerTy),
      //lvgv, array of i8*, cast to i8*, a list of variables "__local array with auto storage"
      ConstantExpr::getBitCast(lvgvConst, pointerTy),
      //rvgv, array of i8*, cast to i8*, a list of variables "__region array with auto storage"
      ConstantExpr::getBitCast(rvgvConst, pointerTy),
      //i32 0 as separator
      ConstantInt::get(Type::getInt32Ty(llvmContext), 0)
    };
    Constant* annoInfo = ConstantStruct::getAnon(llvmField, false);
    annotations.push_back(annoInfo);
  }
}

// FIXME: This needs to be removed when the backends can understand actual
// SPIR metadata.
void
AnnotationGlobalVarGenerator::generate(void)
{
  std::vector<Constant *> annotationInfo;
  for (Module::iterator fb = M.begin(),
      fe = M.end(); fb != fe; ++fb) {
    generate(fb, annotationInfo);
  }
  if (!annotationInfo.empty()) {
    llvm::Constant* globalInit =
      llvm::ConstantArray::get(
          llvm::ArrayType::get(annotationInfo[0]->getType(),
                               annotationInfo.size()),
                               annotationInfo);
    // the global.annotations variable
    llvm::GlobalValue* globalVar =
      new llvm::GlobalVariable(M, globalInit->getType(), false,
                           llvm::GlobalValue::AppendingLinkage, globalInit,
                           "llvm.global.annotations");

    globalVar->setSection("llvm.metadata");
  }
  annotationInfo.clear();
}

// This emits annotation data that is backward compatible with what
// the AMD GPU backend expects.
void
AnnotationGlobalVarGenerator::emitMetaDataAnnotation(
    std::vector<Constant*>& eles,
    Type* eleTy,
    const std::string& metaNameC) {
  int metaSize = static_cast<int>(eles.size());
  if (metaSize) {
    std::string metaName(metaNameC);
    ArrayType* metaType = ArrayType::get(eleTy, metaSize);
    GlobalVariable* metaGlb =
        new GlobalVariable(M, metaType, false,
                           GlobalValue::ExternalLinkage,
                           0, metaName, 0,
                           GlobalVariable::NotThreadLocal, 0);

    metaGlb->setSection("llvm.metadata");
    metaGlb->setInitializer(
        ConstantArray::get(metaType, eles));
  }
}

void
AnnotationGlobalVarGenerator::fixKernelArgTypeName(std::string& typeName,
    llvm::Type* argTy) {
  if (argTy->isPointerTy()) {
    llvm::Type* pointeeTy = argTy->getPointerElementType();
    if (pointeeTy->isStructTy()) {
      std::string llvmTyName = pointeeTy->getStructName();
      // Clang generates kernel arg type name with typedef name of
      // opencl struct type, therefore need to check llvm struct type
      // name to get the correct opencl struct type name.
      if (typeName != llvmTyName) {
        DEBUG(dbgs() << "Warning: argument type name mismatch: metadata: "
            << typeName << " llvm: " << llvmTyName << "\n");
        if (llvmTyName.find("opencl.") == 0) {
          typeName = llvmTyName.substr(7);
          DEBUG(dbgs() << "use llvm type name: " << typeName << "\n");
        }
      }
    }
  }
}

void
AnnotationGlobalVarGenerator::fixKernelArgAddrSpace(int& addrSpace,
    llvm::Type* argTy) {
  int llvmAddrSpace = CLS_PRIVATE_AS;
  if (argTy->isPointerTy()) {
    llvmAddrSpace = argTy->getPointerAddressSpace();
  }
    // Clang generates kernel arg type name with typedef name of
    // opencl struct type, therefore need to check llvm struct type
    // name to get the correct opencl struct type name.
    if (addrSpace != llvmAddrSpace) {
      DEBUG(dbgs() << "Warning: argument addr space mismatch: metadata: "
          << addrSpace << " llvm: " << llvmAddrSpace << "\n");
      DEBUG(dbgs() << "use llvm addr space " << llvmAddrSpace << "\n");
      addrSpace = llvmAddrSpace;
    }
}

// Change calling convention from SPIR_KERNEL and spirfun to default
// calling convention.
void changeCallingConventions(Module &M) {
  if (M.empty())
    return;

  for (Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi) {
    if (fi->getCallingConv() == CallingConv::SPIR_KERNEL
        || fi->getCallingConv() == CallingConv::SPIR_FUNC) {
      fi->setCallingConv(CallingConv::C);
      for (Value::use_iterator ui = fi->use_begin(), ue = fi->use_end();
          ui != ue; ++ui) {
        if (CallInst *Inst = dyn_cast<CallInst>(*ui)) {
          Inst->setCallingConv(CallingConv::C);
        }
      }
    }
  }
}

// Load kernel argument in stub function
Value* generateStubFunctionLoadKernelArg(LLVMContext& context,
    IRBuilder<>& builder, DataLayout& DL, Type* argTy,
    Value* stubArgAddrVal, unsigned alignment, unsigned stubOffset) {
  unsigned varSize = static_cast<unsigned>(DL.getTypeSizeInBits(argTy)
      /8);
  LoadInst* loadInst = builder.CreateLoad(stubArgAddrVal, false, "tmp");
  loadInst->setAlignment(alignment);

  Value* value = loadInst;
  Value* offsetValue;
  unsigned unitOffset;
  if (stubOffset % varSize == 0) {
    unitOffset = stubOffset / varSize;
  } else {
    value = builder.CreateBitCast(value,
        PointerType::getUnqual(Type::getInt8Ty(context)), "conv");
    offsetValue = ConstantInt::get(Type::getInt32Ty(context), stubOffset);
    value = builder.CreateGEP(value, offsetValue, "ptr");
    unitOffset = 0;
  }

  value = builder.CreateBitCast(value, PointerType::getUnqual(argTy), "conv");
  offsetValue = ConstantInt::get(Type::getInt32Ty(context), unitOffset);
  value = builder.CreateGEP(value, offsetValue, "ptr");
  loadInst = builder.CreateLoad(value, "tmp");
  loadInst->setAlignment(alignment);

  return loadInst;
}

// Generate stub function body
void generateStubFunctionBody(LLVMContext& context, DataLayout& DL,
    Function& kernelFunc, Function& stubFunc) {
  IRBuilder<> builder(context);

  // Create basic block and get insert position
  BasicBlock* entryBlk = BasicBlock::Create(context, "entry", &stubFunc);
  builder.SetInsertPoint(entryBlk);
  Value* undefVal = UndefValue::get(Type::getInt32Ty(context));
  Instruction* allocaInsertPos = new BitCastInst(undefVal,
      Type::getInt32Ty(context), "", entryBlk);

  // Allocate stack var and store address of function argument
  Value* stubArgValue = stubFunc.arg_begin();
  std::string stubArgName = "_stubArgs";
  stubArgValue->setName(stubArgName);

  // Create call instruction to the kernel
  PointerType* stubArgType = PointerType::getUnqual(Type::getInt8Ty(context));
  unsigned ptrAlign = static_cast<unsigned>(DL.getTypeSizeInBits(
      stubArgType)/8);
  AllocaInst* stubArgAddrValue = new AllocaInst(stubArgType, NULL,
      stubArgName + ".addr", allocaInsertPos);
  stubArgAddrValue->setAlignment(ptrAlign);
  StoreInst* storeInst = builder.CreateStore(stubArgValue, stubArgAddrValue,
      false);
  storeInst->setAlignment(ptrAlign);

  unsigned fieldOffset = 0;
  unsigned stackMaxAlign = 16;
  std::vector<Value*> args;
  for (Function::arg_iterator I = kernelFunc.arg_begin(),
      E = kernelFunc.arg_end(); I != E; ++I) {
    llvm::Type* argTy = I->getType();
    unsigned argAlign = DL.getABITypeAlignment(argTy);
    unsigned fieldAlign = std::min(argAlign, stackMaxAlign);
    fieldOffset = (fieldOffset + (fieldAlign - 1)) & ~(fieldAlign -1);
    assert((fieldOffset & (fieldAlign - 1)) == 0);
    Value* actVal = generateStubFunctionLoadKernelArg(context, builder,
        DL, argTy, stubArgAddrValue, fieldAlign, fieldOffset);
    fieldOffset += static_cast<unsigned>(DL.getTypeSizeInBits(argTy)/8);
    if (argTy->isStructTy()) {
      AllocaInst* tmpVal = new AllocaInst(argTy, 0, "tmp", allocaInsertPos);
      tmpVal->setAlignment(argAlign);
      StoreInst* storeInst = builder.CreateStore(actVal, tmpVal, false);
      storeInst->setAlignment(fieldAlign);
      args.push_back(tmpVal);
    } else
      args.push_back(actVal);
  }
  CallInst* callInst = builder.Insert(CallInst::Create(&kernelFunc, args));
  callInst->setAttributes(kernelFunc.getAttributes());
  callInst->setCallingConv(kernelFunc.getCallingConv());

  // Create return instruction
  llvm::Type* voidTy = llvm::Type::getVoidTy(context);
  if (kernelFunc.getReturnType() == voidTy)
    builder.CreateRetVoid();
  else
    builder.CreateRet(UndefValue::get(kernelFunc.getReturnType()));

  allocaInsertPos->eraseFromParent();
}

// Generate stub function for a kernel
void generateStubFunction(Module& M, Function& F) {
  DataLayout DL(&M);
  std::string kernelName = F.getName();
  std::string funcName = kernelName.substr(0, kernelName.rfind("_"))
      + "_stub";
  LLVMContext& context = M.getContext();

  // Create stub function type
  std::vector<Type*> argTys;
  argTys.push_back(PointerType::getUnqual(Type::getInt8Ty(context)));
  Type* retTy = Type::getVoidTy(context);
  FunctionType* funcTy = FunctionType::get(retTy, argTys, false);

  // Create stub function declaration
  Function* func = M.getFunction(funcName);
  assert(func == NULL);
  func = Function::Create(funcTy, Function::ExternalLinkage, funcName, &M);

  // Set stub function attributes and linkage
  func->setDoesNotThrow();
  if (GlobalValue::isWeakLinkage(F.getLinkage()))
    func->setLinkage(GlobalValue::WeakAnyLinkage);
  else
    func->setLinkage(GlobalValue::ExternalLinkage);

  generateStubFunctionBody(context, DL, F, *func);

}

// Generate stub functions for kernels
void generateStubFunctions(Module& M) {
  if (M.empty())
    return;
  for (Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi) {
    if (fi->getCallingConv() == CallingConv::SPIR_KERNEL) {
      generateStubFunction(M, *fi);
    }
  }
}

void transformPrintf(Module& M) {
  AMDLLVMBuilder builder(M);
  ExprTransform exprTran(M, &builder);
  LLVMContext& context = M.getContext();
  Function* oldFun = M.getFunction("printf");
  if (oldFun == NULL)
    return;

  oldFun->setName(".printf");
  for (Function::use_iterator I = oldFun->use_begin(), E = oldFun->use_end();
      I != E; ++I) {
    if (CallInst* oldInst = dyn_cast<CallInst>(*I)) {
      BasicBlock* BB = oldInst->getParent();
      builder.setInsertPoint(oldInst);
      Function* newFun = (Function*) exprTran.transBuiltinPrintfptr(oldFun);
      std::vector<llvm::Value*> args;
      for (size_t i = 0; i < oldInst->getNumArgOperands(); ++i) {
        args.push_back(oldInst->getArgOperand(i));
      }
      CallInst* newInst = builder.emitCall(newFun, args);
      newInst->setTailCall(oldInst->isTailCall());
      newInst->setCallingConv(oldInst->getCallingConv());
      newInst->setAttributes(oldInst->getAttributes());
      oldInst->replaceAllUsesWith(newInst);
      oldInst->dropAllReferences();
      oldInst->removeFromParent();
    }
  }
  oldFun->removeFromParent();
}

static bool isCastToConstandAddrSpace(Instruction* inst) {
  if (BitCastInst* cast = dyn_cast<BitCastInst>(inst)) {
    PointerType* dstTy = dyn_cast<PointerType>(cast->getDestTy());
    if (dstTy->getAddressSpace() == CONSTANT_ADDRESS) {
      return true;
    }
  }
  return false;
}
// Transform private string constants casted to constant addr space to
// string in constant addr space
void transformStringBitcast(Module& M) {
  std::map<GlobalVariable*, std::set<Value*> > candidates;
  for (Module::global_iterator I = M.global_begin(), E = M.global_end(); I != E;
      ++I) {
    GlobalVariable* G = I;

    if (!G->hasUnnamedAddr())
      continue;

    if (!G->hasInitializer())
      continue;

    DEBUG(dbgs() << "\nGlobal var: " << *G);
    DEBUG(dbgs() << " hasUnnamedAddr");
    DEBUG(dbgs() << " hasInitializer");

    for (GlobalVariable::use_iterator UI = G->use_begin(), UE = G->use_end();
        UI != UE; ++UI) {
      Instruction* inst = NULL;
      if (ConstantExpr* expr = dyn_cast<ConstantExpr>(*UI)) {
        if (expr->isCast()) {
          if (PointerType* dstTy = dyn_cast<PointerType>(expr->getType())) {
            if (dstTy->getAddressSpace() == CONSTANT_ADDRESS) {
              DEBUG(dbgs() << " bitcast expr: " << *expr);
              candidates[G].insert(expr);
            }
          }
        }
      } else if (Instruction* inst = dyn_cast<Instruction>(*UI)) {
        if (BitCastInst* cast = dyn_cast<BitCastInst>(inst)) {
          PointerType* dstTy = dyn_cast<PointerType>(cast->getDestTy());
          if (dstTy->getAddressSpace() == CONSTANT_ADDRESS) {
            DEBUG(dbgs() << " bitcast: " << *cast);
            candidates[G].insert(cast);
          }
        }
      }
    }
  }
  DEBUG(dbgs() << "\n");

  for (std::map<GlobalVariable*, std::set<Value*> >::iterator
      I = candidates.begin(), E = candidates.end(); I != E; ++I) {
    GlobalVariable* src = I->first;
    std::string name = src->hasName()?src->getName():".str";
    GlobalVariable* newGV
      = new GlobalVariable(M,
                           src->getType()->getPointerElementType(),
                           src->isConstant(),
                           src->getLinkage(),
                           src->getInitializer(),
                           name + ".const",
                           src,
                           src->getThreadLocalMode(),
                           CONSTANT_ADDRESS);
    std::set<Value*>& insts = I->second;
    for (std::set<Value*>::iterator SI = insts.begin(), SE = insts.end();
        SI != SE; ++SI) {
      if (BitCastInst* cast = dyn_cast<BitCastInst>(*SI)) {
        cast->setOperand(0, newGV);
      } else if (ConstantExpr* expr = dyn_cast<ConstantExpr>(*SI)) {
        expr->setOperand(0, newGV);
      }
      DEBUG(dbgs() << "replaced global var: " << **SI << "\n");
    }
    if (src->use_empty()) {
      src->dropAllReferences();
      src->eraseFromParent();
      newGV->setName(name);
    }
  }

}

namespace EnumSpir {
enum ENUM_SAMPLER {
  CLK_ADDRESS_NONE            = 0x0000,
  CLK_ADDRESS_CLAMP           = 0x0004,
  CLK_ADDRESS_CLAMP_TO_EDGE   = 0x0002,
  CLK_ADDRESS_REPEAT          = 0x0006,
  CLK_ADDRESS_MIRRORED_REPEAT = 0x0008,
  CLK_NORMALIZED_COORDS_FALSE = 0x0000,
  CLK_NORMALIZED_COORDS_TRUE  = 0x0001,
  CLK_FILTER_NEAREST          = 0x0010,
  CLK_FILTER_LINEAR           = 0x0020,
};
}

namespace EnumAmd {
enum ENUM_SAMPLER {
  CLK_NORMALIZED_COORDS_FALSE = 0U,
  CLK_NORMALIZED_COORDS_TRUE  = 1U,

  CLK_ADDRESS_NONE            = 0U,
  CLK_ADDRESS_REPEAT          = 1U << 1, //0x0002
  CLK_ADDRESS_CLAMP_TO_EDGE   = 1U << 2, //0x0004
  CLK_ADDRESS_CLAMP           = CLK_ADDRESS_CLAMP_TO_EDGE | CLK_ADDRESS_REPEAT, //0x0006
  CLK_ADDRESS_MIRRORED_REPEAT = 1U << 3, //0x0008

  CLK_FILTER_NEAREST          = 1U << 4, //0x0010
  CLK_FILTER_LINEAR           = 1U << 5  //0x0020
};
}

// Convert sampler value from SPIR enum to AMD enum
unsigned convertSamplerValue(unsigned value) {
  unsigned newVal;
  newVal = value & 0xfff9;    // CLK_NORMALIZED_COORDS_TRUE
  value &= 0x0006;
  if (value == 6) // CLK_ADDRESS_REPEAT
    newVal |= 2;
  else if (value == 4) // CLK_ADDRESS_CLAMP
    newVal |= 6;
  else if (value == 2) // CLK_ADDRESS_CLAMP_TO_EDGE
    newVal |= 4;
  return newVal;
}

// Transform sampler builtin parameter from SPIR enum to AMD enum
void transformSampler(Module& M) {
  AMDLLVMBuilder builder(M);
  LLVMContext& context = M.getContext();
  for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
    Function* func = dyn_cast<Function>(I);
    if (func == NULL)
      continue;
    if (func->hasName() && func->getName().startswith("__read_image") &&
        func->getFunctionType()->getFunctionNumParams() == 3) {
      for (Function::use_iterator I = func->use_begin(), E = func->use_end();
          I != E; ++I) {
        if (CallInst* callInst = dyn_cast<CallInst>(*I)) {
          builder.setInsertPoint(callInst);
          Value* sampler = callInst->getArgOperand(1);
          Value* newPar = NULL;
          unsigned intValue = ~0U;
          if (ConstantInt* constInt = dyn_cast<ConstantInt>(sampler)) {
            intValue = constInt->getZExtValue();
          } else if (LoadInst* loadInst = dyn_cast<LoadInst>(sampler)) {
            if (GlobalVariable* GV = dyn_cast<GlobalVariable>(loadInst->getOperand(0))) {
              if (GV->hasInitializer()) {
                if (ConstantInt* constInt = dyn_cast<ConstantInt>(GV->getInitializer())) {
                  intValue = constInt->getZExtValue();
                }
              }
            }
          }
          if (intValue != ~0U) {
            newPar = builder.getUint32(convertSamplerValue(intValue));
            DEBUG(dbgs() << "[transformSampler] " << *callInst);
            callInst->setArgOperand(1, newPar);
            DEBUG(dbgs() << " => " << *callInst << "\n");
          }
        }
      }
    }
  }
}

void deleteFunctions(std::set<Function*>& dead) {
  for (std::set<Function*>::iterator I = dead.begin(), E = dead.end(); I != E;
      ++I) {
    Function* F = *I;
    if (F->use_empty()) {
      DEBUG(dbgs() << "removed " << F->getName() << "\n");
      F->dropAllReferences();
      F->removeFromParent();
    }
  }
}

void replaceTrivialConversionFunc(Module& M) {
  AMDConvertFunctionNameParser parser(true);
  AMDLLVMBuilder builder(M);
  ExprTransform exprTran(M, &builder);
  std::set<Function*> dead;

  // rename/replace rtz and rte functions
  for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
    Function* F = I;
    if (F->hasName()) {
      std::string name = F->getName();
      if (parser.parse(name)) {
        std::string realName = parser.aliasName();
        if (realName != "") {
          Function* realFunc = M.getFunction(realName);
          if (realFunc == NULL) {
            F->setName(realName);
          } else {
            F->replaceAllUsesWith(realFunc);
            F->dropAllReferences();
            dead.insert(F);
          }
          DEBUG(dbgs() << "[replaceTrivialConversionFunc] replace " << name
              << " with " << realName << "\n");
        }
      }
    }
  }
  deleteFunctions(dead);

  dead.clear();
  for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
    Function* F = I;
    if (F->hasName()) {
      std::string name = F->getName();
      if (parser.parse(name)) {
        bool isRedundant = parser.isRedundant();
        bool isTrivial = false;
        for (Function::use_iterator UI = F->use_begin(), UE = F->use_end();
            UI != UE; ++UI) {
          if (CallInst* callInst = dyn_cast<CallInst>(*UI)) {
            llvm::Value* newValue = callInst->getOperand(0);
            if (!isRedundant && exprTran.isTrivialConv(callInst)) {
              builder.setInsertPoint(callInst);
              newValue = exprTran.transBuiltinConv(F, newValue);
              isTrivial = true;
            }
            if (isRedundant || isTrivial) {
              DEBUG(dbgs() << "[replaceTrivialConversionFunc] " << *callInst
                  << " => " << *newValue << "\n");
              callInst->replaceAllUsesWith(newValue);
              callInst->dropAllReferences();
              callInst->removeFromParent();
            }
          }
        }
        if (isRedundant || isTrivial)
          dead.insert(F);
      }
    }
  }
  deleteFunctions(dead);
}

static bool changeStructAddrSpace(Function &F, DebugInfoManager& DIManager) {
  if (F.getCallingConv() != CallingConv::SPIR_KERNEL)
    return false;

  bool changed = false;
  FunctionType *FTy = F.getFunctionType();
  std::vector<Type*> ArgTy;
  for (Function::arg_iterator I = F.arg_begin(), E = F.arg_end(); I != E; ++I) {
    Type *Ty = I->getType();
    PointerType *PTy = dyn_cast<PointerType>(Ty);
    if (PTy && I->hasByValAttr()) {
      Type *EltTy = PTy->getElementType();
      unsigned addrSpace = PTy->getAddressSpace();
      if (isa<StructType>(EltTy) && addrSpace == PRIVATE_ADDRESS &&
          EltTy->isSized()) {
        Ty = PointerType::get(EltTy, GLOBAL_ADDRESS);
        AMDLLVMBuilder Builder(*F.getParent());
        Builder.setInsertPointAtAlloca(&F);
        AllocaInst *AI = Builder.emitAlloca(EltTy, 1,
                                            std::string(I->getName()) + ".pvt");
        I->replaceAllUsesWith(AI);
        LoadInst *LI = Builder.emitLoad(I);
        StoreInst *SI = Builder.emitStore(LI, AI);
        I->mutateType(Ty);
        changed = true;
        DEBUG(dbgs() <<
          "Fixed struct by val address space from private to global, kernel: " <<
           F.getName() << " parameter: " << I->getName() << "\n");
      }
    }
    ArgTy.push_back(Ty);
  }
  if (changed) {
    FTy = FunctionType::get(FTy->getReturnType(), ArgTy, FTy->isVarArg());
    Function* NewF = Function::Create(FTy, F.getLinkage(), "", F.getParent());
    NewF->getBasicBlockList().splice(NewF->begin(), F.getBasicBlockList());
    Function::arg_iterator NewI = NewF->arg_begin();
    for (Function::arg_iterator I = F.arg_begin(), E = F.arg_end(); I != E;
         ++I, ++NewI) {
      I->replaceAllUsesWith(NewI);
      NewI->takeName(I);
    }
    NewF->setCallingConv(F.getCallingConv());
    NewF->setAttributes(F.getAttributes());
    NewF->takeName(&F);

    DIManager.replaceFunctionDI(&F, NewF);
    updateSPIRMetadata(*F.getParent(), &F, NewF);
  }
  return changed;
}

// Change address space of struct by val pointers passed into a SPIR_KERNEL
// from provate as generated clang to global as passed by RT.
static bool changeStructAddrSpace(Module &M) {
  if (M.empty())
    return false;

  bool changed = false;
  std::set<Function*> deadFunc;
  DebugInfoManager DIManager;
  DIManager.collectFunctionDIs(M);
  for (Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi) {
    if (deadFunc.find(fi) == deadFunc.end() &&
        changeStructAddrSpace(*fi, DIManager)) {
      deadFunc.insert(fi);
      changed = true;
    }
  }
  if (changed)
    deleteFunctions(deadFunc);

  return changed;
}

} // end of namespace AMDSpir

using namespace AMDSpir;
namespace spir {
bool isSPIRType(Type **type_table, Type *cmp, uint32_t &x)
{
  while (type_table[x] != 0) {
    if (cmp == type_table[x]) return true;
    if (Type::StructTyID == cmp->getTypeID() &&
        Type::StructTyID == type_table[x]->getTypeID()) {
      StringRef cmp_name = cmp->getStructName();
      StringRef type_table_name = type_table[x]->getStructName();
      //cmp_name doesn't match type_table_name
      //see if it matches type_table_name.[0-9]+
      if (cmp_name.startswith(type_table_name)) {
        size_t len = type_table_name.size();
        APInt dummy;
        if (cmp_name.size() > len+1 && '.' == cmp_name[len] &&
            !cmp_name.substr(len+1).getAsInteger(10,dummy)) {
          DEBUG(dbgs() << "type_table_name.dummy " << type_table_name << "."
                << dummy << " matches cmp_name " << cmp_name << "\n");
          return true;
        } else {
          //cmp_name should match type_table_name.[0-9]+
          //so cmp_name's length is greater than len+1
          DEBUG(dbgs() << "type_table_name " << type_table_name <<
                " does not match cmp_name " << cmp_name << "\n");
        }
      }
    }
    x += 2;
  }
  return false;
}
}

char SPIRLoader::ID = 0;
INITIALIZE_PASS_BEGIN(SPIRLoader, "spirloader", "SPIR Binary Loader",
    false, false);
INITIALIZE_PASS_END(SPIRLoader, "spirloader", "SPIR Binary Loader",
    false, false);

SPIRLoader::SPIRLoader() :
  ModulePass(ID),
  mDemangleBuiltin(true){
}

SPIRLoader::SPIRLoader(bool demangleBuiltin)
  : ModulePass(ID),
    mDemangleBuiltin(demangleBuiltin){
  initializeSPIRLoaderPass(*PassRegistry::getPassRegistry());
}

bool
SPIRLoader::runOnModule(Module &M)
{
  static int count = 0;
  //TODO: remove this after development is done
  if (getenv("AMD_SPIR_KEEP_TEMP") != 0) {
    char tmpFileName[20];
    sprintf(tmpFileName, "_spir_in%02d.ll", count);
    std::string errorInfo;
    llvm::raw_fd_ostream os(tmpFileName, errorInfo);
    llvm::errs() << "[SPIRLoader] Save temp SPIR to " << tmpFileName << "\n";
    M.print(os, 0);
    os.close();
    llvm::errs().flush();
  }

  Triple TargetTriple(M.getTargetTriple());
  Triple::ArchType Arch = TargetTriple.getArch();

  assert(Arch != Triple::spir && Arch != Triple::spir64
         && "Linker must set target triple before calling this pass");

  if (Arch == Triple::hsail || Arch == Triple::hsail_64) {
    DEBUG(llvm::errs() << "[changeStructAddrSpace]\n";);
    return changeStructAddrSpace(M);
  }

  bool usesGPU = (Arch == Triple::amdil || Arch == Triple::amdil64);

  LLVMContext &ctx = M.getContext();
#define STRUCT_TYPE(A) (M.getTypeByName(A) ? M.getTypeByName(A) \
    : StructType::create(ctx, A))
  // The type table holds the mapping of the opaque SPIR types to the
  // AMDIL types that are generated by the front-end in non-SPIR mode.
  Type* type_table[] = {
    Type::getHalfTy(ctx), Type::getInt16Ty(ctx),
    STRUCT_TYPE("spir.sampler_t"), Type::getInt32Ty(ctx),
    STRUCT_TYPE("spir.event_t"), Type::getInt32Ty(ctx),
    0
  };
#undef STRUCT_TYPE


  // FIXME: Once we move to the real name mangling scheme, remove this code
  if (mDemangleBuiltin) {
  DEBUG(llvm::errs() << "[demangleNames]\n";);
  MathFunctionTransform mathTran(M);
  demangleNames(M, mathTran);

  // Transform math functions
  DEBUG(llvm::errs() << "[MathFunctionTransform]\n");
  mathTran.transform();
  }

  // Transform auto local/region arrays for CPU
  if (!usesGPU) {
    DEBUG(llvm::errs() << "[AutoArrayTransform]\n");
    AutoArrayTransform autoTran(M);
    autoTran.transform();
  }

  // Replace all of the SPIR types with non-SPIR types.
  DEBUG(llvm::errs() << "[replaceTypesInModule]\n";);
  replaceTypesInModule(M, reinterpret_cast<Type**>(type_table));

  // Remove all of the global functions that are no longer used.
  // These are usually SPIR functions.
  DEBUG(llvm::errs() << "[removeUnusedGlobals]\n";);
  removeUnusedGlobals(M, reinterpret_cast<Type**>(type_table));

  // Remove functions that were temporarily renamed with the '_spir'
  // suffix so that the functions did not conflict with what was
  // already defined in the module.
  DEBUG(llvm::errs() << "[renameSPIRFunctions]\n";);
  renameSPIRFunctions(M, reinterpret_cast<Type**>(type_table));

  // TODO: This only needs to be run until the backends can understand
  // the SPIR metadata.
  DEBUG(llvm::errs() << "[generateOpenCLGlobas]\n";);
  AnnotationGlobalVarGenerator annoGen(M, usesGPU);
  annoGen.generate();

  // Generates stub functions for CPU
  if (!usesGPU) {
    DEBUG(llvm::errs() << "[generateStubFunctions]\n";);
    generateStubFunctions(M);
  }

  DEBUG(llvm::errs() << "[changeCallingConventions]\n";);
  changeCallingConventions(M);

  if (mDemangleBuiltin) {
  DEBUG(llvm::errs() << "[TypeNameChanger]\n";);
  TypeNameChanger typeNameChanger;
  typeNameChanger.change(M);
  }

  DEBUG(llvm::errs() << "[removeTrivialConversionFunc]\n");
  replaceTrivialConversionFunc(M);

  if (!usesGPU) {
    DEBUG(llvm::errs() << "[transformPrintf]\n");
    transformPrintf(M);
  }

  DEBUG(llvm::dbgs() << "[transformSampler]\n");
  transformSampler(M);

  DEBUG(llvm::dbgs() << "[transformStringBitcast]\n");
  transformStringBitcast(M);

  //TODO: remove this after development is done
  if (getenv("AMD_SPIR_KEEP_TEMP") != 0) {
    char tmpFileName[20];
    sprintf(tmpFileName, "_spir_out%02d.ll", count);
    std::string errorInfo;
    llvm::raw_fd_ostream os(tmpFileName, errorInfo);
    llvm::errs() << "[SPIRLoader] Save converted LLVMIR to " << tmpFileName
        << "\n";
    M.print(os, 0);
    os.close();
    llvm::errs().flush();
    count++;
  }

  return true;
}

// createSPIRLoader - Public interface to this file
ModulePass *
llvm::createSPIRLoader(bool needDemangleBuiltin)
{
  return new SPIRLoader(needDemangleBuiltin);
}
#endif
