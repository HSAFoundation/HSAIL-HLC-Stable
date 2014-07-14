//===- AMDLowerPipeBuiltins.cpp - Lowers Pipe Library Calls ------------===//
//
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief Translate CLANG frontend generated pipe builtin function calls to
/// internal library functions.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "AMDLowerPipeBuiltins"

#include <iostream>
#include "llvm/ADT/StringRef.h" 
#include "llvm/ADT/ValueMap.h" 
#include "llvm/Pass.h" 
#include "llvm/PassManager.h" 
#include "llvm/Value.h" 
#include "llvm/Module.h" 
#include "llvm/Instructions.h" 
#include "llvm/IRBuilder.h" 
#include "llvm/Transforms/Scalar.h" 
#include "llvm/Transforms/Utils/AMDArgumentUtils.h" 
#include "../../lib/Transforms/SPIR/cxa_demangle.h" 

#include <string>
using namespace llvm;

namespace {
  class AMDLowerPipeBuiltins : public ModulePass {

  public:
    static char ID; // Pass identification, replacement for typeid

    AMDLowerPipeBuiltins() : ModulePass(ID) {
      initializeAMDLowerPipeBuiltinsPass(*PassRegistry::getPassRegistry());
    }

    virtual bool runOnModule(Module &M);
  private:

    llvm::DenseMap<llvm::Value *, llvm::Function*> ResolvedPipeFunctions;

    llvm::Value* CreatePipeLibraryCall(Module &M, CallInst *CI);

    llvm::Function* CreateFunctionWithArguments(Module &M,
                                                StringRef FunctionName,
                                                std::vector<Type*> ArgumentTypes,
                                                Type *ReturnType);

    void GetPipeProperties(llvm::CallInst *CI,unsigned &PipePacketSize,
                           unsigned &PipePacketAlign);

    bool GetLibraryName(llvm::CallInst *CI, std::string FunctionName,
                        std::string &LibraryName,bool &AddAlign);

  };

  struct pipeBuiltinLoweringDescr_t {
    const char *builtinFunction; // The pipe builtin function name such as
                                 // read_pipe, write_pipe.

    int numParams;               // number of parameters expected for this
                                 // function call .It includes the parameters
                                 // added by frontend to confirm with the SPIR.
                                 // Currently frontend adds two new parameters
                                 // size and alignment to the pipe builtin
                                 // function calls.

    const char *internalFunction;// The internal function name to which this
                                 // call will  be changed to.

    int addAlign;                // field indicates if there is need to add
                                 // alignment field to the internal function
                                 // call. (e.g read/write pipe internal
                                 // function calls.

  } PipeBuiltinLoweringDescr[] = {
    {"read_pipe",4,"__read_pipe_internal_",1},
    {"read_pipe",6,"__read_pipe_reserved_internal_",1},
    {"write_pipe",4,"__write_pipe_internal_",1},
    {"write_pipe",6,"__write_pipe_reserved_internal_",1},
    {"reserve_read_pipe",4, "__reserve_read_pipe_internal_",0},
    {"reserve_write_pipe",4, "__reserve_write_pipe_internal_",0},
    {"commit_read_pipe",4, "__commit_read_pipe_internal_",0},
    {"commit_write_pipe",4, "__commit_write_pipe_internal_",0},
    {"work_group_reserve_read_pipe",4,
                   "__work_group_reserve_read_pipe_internal_",0},
    {"work_group_reserve_write_pipe",4,
                   "__work_group_reserve_write_pipe_internal_",0},
    {"work_group_commit_read_pipe",4,
                   "__work_group_commit_read_pipe_internal_",0},
    {"work_group_commit_write_pipe",4,
                   "__work_group_commit_write_pipe_internal_",0},
    {"get_pipe_num_packets",3,"__get_pipe_num_packets_internal_",0},
    {"get_pipe_max_packets",3,"__get_pipe_max_packets_internal_",0},
    {"sub_group_reserve_read_pipe",4,
                   "__sub_group_reserve_read_pipe_internal_",0},
    {"sub_group_reserve_write_pipe",4,
                   "__sub_group_reserve_write_pipe_internal_",0},
    {"sub_group_commit_read_pipe",4,
                   "__sub_group_commit_read_pipe_internal_",0},
    {"sub_group_commit_write_pipe",4,
                   "__sub_group_commit_write_pipe_internal_",0},
    {NULL,0,NULL,0}
  };
} // end anonymous namespace

INITIALIZE_PASS(AMDLowerPipeBuiltins, "amd-lower-opencl-pipe-builtins",
                "Lower OpenCL 2.0 pipe builtins to internal library functions",
                false, false);

char AMDLowerPipeBuiltins::ID = 0;

// Implements the lowering of pipe builtin function calls to internal
// library function calls.

// pipe buitlin functions can have generic types(scalar, vector of basic
// types) or user defined pipe types.

// The builtin functions which uses generic types are lowered
// to <builtin>_internal_<packetsize> library calls.

// The builtin functions which uses user defined types are lowered
// to <builtin>_internal_user library calls with additional arguments
// packet size and struct alignment.

// this pass traverses all call statments and checks if they are calls
// to pipe builtin functions. The pipe builtins are lowered as per
// internal library requirements.

bool AMDLowerPipeBuiltins::runOnModule(Module& M) {

  bool Changed = false;
  for(Module::iterator MF = M.begin(), E = M.end(); MF != E; ++MF) { 
    for(Function::iterator BB=MF->begin(), MFE=MF->end(); BB != MFE; ++BB) { 
      for(BasicBlock::iterator instr = BB->begin(), instr_end = BB->end(); 
          instr != instr_end; ) { 
        CallInst *CI = dyn_cast<CallInst>(instr); 
        if(!(CI && CI->getCalledFunction() 
              && CI->getCalledFunction()->hasName())) { 
            instr++; 
            continue; 
        } 
        instr++; 
        Value *newPipeLibCall = CreatePipeLibraryCall(M,CI);
        if (newPipeLibCall) {
          CI->replaceAllUsesWith(newPipeLibCall);
          CI->eraseFromParent();
          Changed = true;
        }
      } 
    } 
  } 
  // remove the pipe function declarations, which are replaced by new functions
  for (llvm::DenseMap<llvm::Value *, 
        llvm::Function *>::iterator it = ResolvedPipeFunctions.begin();
        it != ResolvedPipeFunctions.end(); ++it) {
    llvm::Function *OriginalPipeFn=(llvm::dyn_cast<llvm::Function>(it->first));
    OriginalPipeFn->eraseFromParent();
  }
  return Changed; 
}

// Function to crete the pipe function lowering pass
ModulePass *llvm::createAMDLowerPipeBuiltinsPass() {
  return new AMDLowerPipeBuiltins();
}


// If the call instruction is to be lowered to new internal library call,
// this function creates such new call instruction and returns the same.
// The function also handles if any extra arguements need to be added 
// or if any arguments are to be removed.

// e.g  %call1 = call spir_func i32 @_Z10write_pipePvS_j(..) will be
// replaced with call %3 = call i32 @__write_pipe_internal_4(..) when
// the packetsize is 4 bytes. The internal library function will be
// implemented in the library.

// Function returns NULL if call is not replaced.

using namespace std;

llvm::Value*
AMDLowerPipeBuiltins::CreatePipeLibraryCall(Module &M, CallInst *CI)
{

  llvm::Function *CF = CI->getCalledFunction();

  StringRef MangledName = CF->getName();
  int status = 0;

  const char* DemangledName =
            __cxxabiv1::__cxa_demangle(MangledName.data(), 0, 0, &status);

  if (status || !DemangledName) return NULL;
  StringRef DemangledString(DemangledName);

  size_t FunctionNameEnd = DemangledString.find("(");
  if (FunctionNameEnd == std::string::npos) return NULL;
  std::string FunctionName = DemangledString.substr(0,FunctionNameEnd);
  std::string LibraryName;

  int numParams = CI->getNumArgOperands();
  IRBuilder<> B(CI);
  SmallVector<llvm::Value*,16> NewArgs;
  std::vector<llvm::Type*> ArgumentTypes;
  int newNumParams=numParams;
  bool AddAlign  = false;

  if (GetLibraryName(CI,FunctionName,LibraryName,AddAlign)) {

  // SPIR 2.0 specifies that there will be extra argument specifying the
  // pipe size. In case of generic pipe types, the packet size will be
  // part of library function (e.g read_pipe_internal_1 specifies
  // packetsize is 1 byte.

  // In case of basic pipe types we have to remove one argument from the
  // call and function declaration.

  // for pipe functions with user defined type, we have to add additional
  // alignment parameter.
  //   e.g read_pipe_internal_user(...,pipesize,pipealignment)

    unsigned PipeDataSize = 0;
    unsigned PipeDataAlignment = 0;
    unsigned PipeDataAccess = 0;
    int params = 0;
    GetPipeProperties(CI,PipeDataSize,PipeDataAlignment);

    // If the pipedata size and the pipe data alignements are same,
    // we would use the internal_<size> functions which will be
    // implemented efficiently by the library.
    if (PipeDataAlignment == PipeDataSize ) {
       newNumParams-=2;  // the packetsize will be part of library name and
                         // alignment field is not needed.
       LibraryName += APInt(32,PipeDataSize).toString(10,false);
    } else {
       LibraryName += "user";
       if (!AddAlign)
         newNumParams-=2;  // only for read and write functions,the alignment
                           // field is needed in the library function.
    }

    for (int i=0; i<newNumParams; i++)
      NewArgs.push_back(CI->getArgOperand(i));

    for (llvm::Function::arg_iterator AI = CF->arg_begin(), E = CF->arg_end();
                               (params++ < newNumParams) && (AI != E); ++AI)  {
        ArgumentTypes.push_back(AI->getType());
    }

    llvm::Function *NF ;

    // we have to use same function for all calls to same function.
    // This information is cached in ResolvedPipeFunctions vector.
    if (ResolvedPipeFunctions[CF])
      NF = ResolvedPipeFunctions[CF];
    else {
      NF = CreateFunctionWithArguments(M,LibraryName,ArgumentTypes,
                                       CF->getReturnType());
      NF->setCallingConv(CallingConv::SPIR_FUNC);
    }

    llvm::CallInst *newCI = B.CreateCall(NF,NewArgs);
    newCI->setCallingConv(CallingConv::SPIR_FUNC);

    ResolvedPipeFunctions[CF] = NF;

    return newCI;
  }
  // handle is_valid_reserve_id to generate function call to
  // __is_valid_reserve_id function
  if (FunctionName.compare("is_valid_reserve_id") == 0) {
     llvm::Function *CF = CI->getCalledFunction();
     CF->setName("__" + FunctionName);
  }

  return NULL;
}

// Given the call instruction with the mangled function name
// this function returns the library function name to which
// this call is to be lowered to.

// e.g @_Z10write_pipePvS_j will return "__write_pipe_internal_"

bool AMDLowerPipeBuiltins::GetLibraryName(llvm::CallInst *CI,
                      std::string FunctionName,std::string &LibraryName,
                      bool &AddAlign)
{
   int numParams = CI->getNumArgOperands();
   IRBuilder<> B(CI);

   for (int i=0; PipeBuiltinLoweringDescr[i].builtinFunction != NULL; i++) {
    if (FunctionName.compare(PipeBuiltinLoweringDescr[i].builtinFunction)==0) {
      // if number of parameters doesnt match with required arguments
      // dont change the call.
      if (numParams != PipeBuiltinLoweringDescr[i].numParams) continue;

      LibraryName = PipeBuiltinLoweringDescr[i].internalFunction;
      AddAlign = PipeBuiltinLoweringDescr[i].addAlign;

      return true;
     }
   }
   return false;
}

// The properties of pipe <alignment, packet size)
// are returned by this function.

void AMDLowerPipeBuiltins::GetPipeProperties(llvm::CallInst *CI,
                       unsigned &PipeDataSize,
                       unsigned &PipeDataAlign)
{
   int numParams = CI->getNumArgOperands();
   // packetsize is the last-1 argument of the library call added by frontend
   ConstantInt *CIntPacketDataSize
                  = dyn_cast<ConstantInt>(CI->getArgOperand(numParams-2));
   if (CIntPacketDataSize)
      PipeDataSize  = CIntPacketDataSize->getSExtValue();

   // packet data alignment is the last argument of the library call
   // added by frontend
   ConstantInt *CIntPacketDataAlign
                  = dyn_cast<ConstantInt>(CI->getArgOperand(numParams-1));
   if (CIntPacketDataAlign)
      PipeDataAlign  = CIntPacketDataAlign->getSExtValue();

}

// Function to create a new llvm function with given name and argument types
// ToDo: This function can be moved to AMDArgumentUtils.cpp

llvm::Function *AMDLowerPipeBuiltins::CreateFunctionWithArguments(Module &M,
                                      StringRef FunctionName,
                                      std::vector<llvm::Type*> ArgumentTypes,
                                      llvm::Type *ReturnType)
{
  llvm::FunctionType* FunctionType=
        llvm::FunctionType::get(ReturnType,ArgumentTypes,false);

  llvm::Function *NewFunction = llvm::Function::Create(FunctionType,
                                            llvm::Function::ExternalLinkage,
                                            FunctionName, &M);
  return NewFunction;

}



