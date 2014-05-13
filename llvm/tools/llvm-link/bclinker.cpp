#if defined(AMD_OPENCL) || 1
//
// Copyright (c) 2013 Advanced Micro Devices, Inc. All rights reserved.
//

#include "bclinker.h"

#include "llvm/Instructions.h"
#include "llvm/Linker.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Module.h"
#include "llvm/Constants.h"
#include "llvm/PassManager.h"
#include "llvm/LLVMContext.h"

#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/DataLayout.h"
#include "llvm/AMDLLVMContextHook.h"
#include <cassert>
#include <string>
#include <list>
#include <map>
#include <set>

using namespace llvm;

static cl::opt<bool>
PreLinkOpt("prelink-opt", cl::desc("Enable pre-link optimizations"));

static cl::opt<bool>
DisableSimplifyLibCalls("disable-simplify-libcalls",
                        cl::desc("Disable simplify-libcalls"));

static cl::opt<bool>
EnableWholeProgram("whole", cl::desc("Enable whole program mode"));

static cl::opt<bool>
EnableUnsafeFPMath("enable-unsafe-fp-math",
          cl::desc("Enable optimizations that may decrease FP precision"));

/*
   Linking all library modules with the application module uses "selective extraction",
   in which only functions that are referenced from the application get extracted into
   the final module.

   The selective extraction is achieved by Reference Map (ModuleRefMap).  A Reference Map
   for the application module keeps track of all functions that are referenced from the 
   application but not defined in the application module.

   Reference Map is implemented by the following three functions: InitReferenceMap(), 
   AddReferences(), and AddFuncReferences(). It hanles both extern and local functions.
   */

/* Get a callee function or an alias to a function) */
static Value*
GetCalledFunction(CallInst* CI)
{
  Value* Callee = CI->getCalledValue();
  if (Function *F = dyn_cast<Function>(Callee)) {
    return F;
  }
  if (GlobalAlias *GAV = dyn_cast<GlobalAlias>(Callee)) {
    return GAV;
  }
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(Callee)) {
    if (CE->getOpcode() == Instruction::BitCast) {
      if (Function *F = dyn_cast<Function>(CE->getOperand(0))) {
        return F;
      }
      if (GlobalAlias *GAV = dyn_cast<GlobalAlias>(CE->getOperand(0))) {
        return GAV;
      }
    }
  }

  // Aha, indirect call, unexpected !
  return 0;
}

static void
InitReferenceMap(llvm::Module *Src,
    std::map<const std::string, bool> &InExternFuncs,
    std::list<std::string> &ExternFuncs)
{
  for (llvm::Module::iterator SF = Src->begin(), E = Src->end(); SF != E; ++SF) {
    Function *F = SF;
    if (!F || !F->hasName())
      continue;

    // All no-body functions should be library functions. Initialize ExternFuncs
    // with all those functions.
    //
    // Weak functions are also treated as no-body functions.
    if ( !F->hasLocalLinkage() &&
        (F->isDeclaration() || F->isWeakForLinker())) {
      InExternFuncs[F->getName()] = true;
      ExternFuncs.push_back(F->getName());
    }
  }
}

static void
AddFuncReferences(llvm::Module *M, Function *SF,
    std::map<const std::string, bool> &InExternFuncs,
    std::list<std::string> &ExternFuncs,
    std::map<const Value*, bool> &ModuleRefMap)
{
  /*
     Add this function's direct extern reference into ExternFuncs if it is not
     in ExternFuncs yet. For a local reference, invoke itself to visit that
     local function body if it has not been visited. ModuleRefMap tells which
     local functions have been visited already.
     */
  for (Function::iterator BB = SF->begin(), BE = SF->end(); BB != BE; ++BB) {
    for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
      if (CallInst* CI = dyn_cast<CallInst>(&*I)) {
        Value* Callee = GetCalledFunction(CI);
        if (!Callee)
          continue;

        Function *F;
        if (GlobalAlias *GAV = dyn_cast<GlobalAlias>(Callee)) {
          assert(GAV && "invoke non-func/non-alias");
          ModuleRefMap[GAV] = true;
          if (!GAV->hasLocalLinkage()) {
            if (!InExternFuncs[GAV->getName()]) {
              InExternFuncs[GAV->getName()] = true;
              ExternFuncs.insert(ExternFuncs.end(), 1, GAV->getName());
            }
          }

          GlobalValue *AV = const_cast<GlobalValue*>(GAV->getAliasedGlobal());
          F = dyn_cast<Function>(AV);
        } else {
          F = dyn_cast<Function>(Callee);
        }
        assert (F && "Should invoke either a func or an alias to a func");

        M->Materialize(F);
        if (!F->hasLocalLinkage()) {
          ModuleRefMap[F] = true;
          if (!InExternFuncs[F->getName()]) {
            InExternFuncs[F->getName()] = true;
            ExternFuncs.insert(ExternFuncs.end(), 1, F->getName());
          }
        }
        else if (/* F->hasLocalLinkage() && */ !ModuleRefMap[F]) {
          ModuleRefMap[F] = true;
          AddFuncReferences(M, F, InExternFuncs, ExternFuncs, ModuleRefMap);
        }
      }
    }
  }
}

static void
AddReferences(std::vector<llvm::Module *> LibMs,
    std::map<const std::string, bool> &InExternFuncs,
    std::map<const Value*, bool> *ModuleRefMaps,
    std::list<std::string> &ExternFuncs)
{
  /*
     ExternFuncs is a list of library functions referenced from the Application.
Input:
a list of the extern library functions directly referenced by the application.
Ouput:
a list of all extern library functions needed by the application (directly and
indirectly referenced by the application).

InExternFuncs is a helper map that simply returns true if an extern function
is in ExternFuncs.

ModuleRefMaps[0:num-of-libs-1]
ModuleRefMaps[i]: tells which functions in Library[i] should be in the final
module. It includes both extern and local functions.

The standard linker links one library at a time when there are more than one libraries
to link. Here we use a different approach in which it first calculates which functions 
should be linked in from each library, then does the actual linking.  

For example, assume there are two libraries A and B, where A defines function f, which
references function g; and B defines function g, which references function k. Both A
and B defines function k (f, g, k are all externs). A is linked first, and is followed
by B.

Apps       A         B
------    -----     ------
use f    def f     def g
use g     use k
def k     def k

This approach will extract f and k from A, and g from B. 

To calculate which functions should be extracted from which library, it starts from
directly-referenced extern functions from the application module, and calculates
their transitive closure.   The following loop iterates over the ExternFuncs list
and in each iteration, a function body is visited and every new extern function, 
if any, is added into the end of ExternFuncs (insertion does not invalidates
std::list's iterator) so that the new function will be visited later.

AddFuncReferences() actually visits a function's body. It adds any extern function
in ExternFuncs. For any local function, it recursively call itself so that all local
functions get visited.
*/
  for (std::list<std::string>::iterator it = ExternFuncs.begin(), IE = ExternFuncs.end();
      it != IE; ++it)
  {
    std::string FName = *it;

    // In case there're multiple definitions to FName, only the first strong
    // definition needs to be scanned (scan its function body). The libraries
    // following this strong definition does not need to scan another function
    // body if FName is defined again; but they do need to add FName to their
    // reference map if FName is referenced. For this reason, we keep looping
    // over all iterations and guard body-scanning with ScanFuncBody.
    bool ScanFuncBody = true;

    // Search library in order
    for (unsigned int i=0; i < LibMs.size(); i++) {
      llvm::Module* Src = LibMs[i];
      ValueSymbolTable &SrcSymTab = Src->getValueSymbolTable();
      GlobalValue *SGV = cast_or_null<GlobalValue>( SrcSymTab.lookup( FName ) );
      if ( SGV ) {
        // If SGV has local linkage, it is not the same as FName, skip it.
        if (SGV->hasLocalLinkage())
          continue;

        // Found a declaration or definition for extern FName.
        ModuleRefMaps[i][SGV] = true;

        Function* SF = 0;
        if (GlobalAlias *GAV = dyn_cast<GlobalAlias>(SGV)) {
          // Don't allow multiple defs to any one alias (like Lib1:x = y; Lib2:x = z),
          // nor an alias def and a normal def (like Lib1: x {}; Lib2:x=z).
          // If this happens, linking may have a undefined behavior and LLVM may
          // give an error message.
          GlobalValue *AV = const_cast<GlobalValue*>(GAV->getAliasedGlobal());
          // SF = Src->getFunction(AV->getName());
          SF = dyn_cast<Function>(AV);
          Src->Materialize( SF );

          if (!SF->hasLocalLinkage()) {
            ModuleRefMaps[i][SF] = true;
            if (!InExternFuncs[SF->getName()]) {
              InExternFuncs[SF->getName()] = true;
              ExternFuncs.insert(IE, 1, SF->getName());
            }
          } else if (/*SF->hasLocalLinkage() && */ !ModuleRefMaps[i][SF]) {
            // It is local and not yet scanned, scan it now.
            ModuleRefMaps[i][SF] = true;
            AddFuncReferences(Src, SF, InExternFuncs, ExternFuncs, ModuleRefMaps[i]);
          }

          // If this definition of FName is weak, continue finding another definition
          if (SGV->isWeakForLinker())
            continue;
          // For alias, don't set ScanFuncBody. We assume only one alias def for any
          // symbol in the entire application (user code + libs)
        } else if ( (SF = dyn_cast<Function>(SGV)) ) {
          if (!ScanFuncBody)
            continue;

          Src->Materialize( SF );

          if (!SF->isDeclaration())
            AddFuncReferences(Src, SF, InExternFuncs, ExternFuncs, ModuleRefMaps[i]);

          // If this definition of FName is weak, continue finding another definition
          if (SF->isWeakForLinker())
            continue;

          // Found a strong definition, don't scan anymore.
          if (!SF->isDeclaration())
            ScanFuncBody = false;
        } else {
          assert ( false && "Unknown global references");
        }
      }
    }
  }
}

static void 
UnlinkGlobals(llvm::Module* M, std::set<GlobalVariable*> AliveGlobals)
{
  std::vector<GlobalVariable*> DeadGlobals;
  for (llvm::Module::global_iterator I = M->global_begin(), E = M->global_end();
      I != E; ++I) {
    GlobalVariable* GVar = I;
    if (GVar->use_empty() && (AliveGlobals.count(GVar) == 0)) {
      DeadGlobals.push_back(GVar);
    }
  }

  for (int i=0, e = (int)DeadGlobals.size(); i < e; i++) {
    M->getGlobalList().erase(DeadGlobals[i]);
  }
}

static bool
linkWithModule(
    llvm::Module* Dst, llvm::Module* Src,
    std::map<const llvm::Value*, bool> *ModuleRefMap)
{
  std::string ErrorMessage;
  if ( Linker::LinkModules(Dst, Src, llvm::Linker::PreserveSource,
                               *ModuleRefMap, &ErrorMessage, true)) {
    errs() << "Linking bc libraries failed!\n";
    return true;
  }

#ifndef NDEBUG
  if (verifyModule(*Dst)) {
    errs() << "Internal Error: verification failed after linking libraries!\n";
    return true;
  }
#endif
  return false;
}

namespace llvm {

static void
prelinkOpt(llvm::Module *M)
{
  // Since AMDSimplifyLibCallsPass() behaves slightly differently b/w
  // PreLink and PostLink opt, set flag to tell LLVM pass whether it is
  // in PreLink or PostLink.
  AMDLLVMContextHook AmdHook;
  M->getContext().setAMDLLVMContextHook(&AmdHook);
  AmdHook.amdrtFunctions = NULL;
  AmdHook.amdoptions.WholeProgram = EnableWholeProgram;
  AmdHook.amdoptions.IsPreLinkOpt = true;
  AmdHook.amdoptions.UnsafeMathOpt = EnableUnsafeFPMath;

  llvm::PassManager Passes;
  Passes.add(new llvm::DataLayout(M));

  // AliasAnalysis will be created on demand 
  // we just need it presence in pass queue
  Passes.add(llvm::createTypeBasedAliasAnalysisPass());
  Passes.add(llvm::createBasicAliasAnalysisPass());
  
  if (!DisableSimplifyLibCalls) {
    Passes.add(llvm::createAMDSimplifyLibCallsPass(
      AmdHook.amdoptions.UnsafeMathOpt));
  }
  Passes.run(*M);
}

namespace BCLinker {
int
link(llvm::Module* input, std::vector<llvm::Module*> &libs) 
{
  int ret = 0;

  if (PreLinkOpt) {
    prelinkOpt(input);
  }

  std::list<std::string> ExternFuncs;
  std::map<const std::string, bool> InExternFuncs;
  std::string ErrorMessage;

  std::map<const llvm::Value*, bool>
    *modRefMaps = new std::map<const llvm::Value*, bool>[libs.size()];

  InitReferenceMap(input, InExternFuncs, ExternFuncs);
  AddReferences(libs, InExternFuncs, modRefMaps, ExternFuncs);

  // A quick workaround to unlink useless globals from libraries.
  std::set<llvm::GlobalVariable*> AliveGlobals;
  for (llvm::Module::global_iterator I = input->global_begin(),
      E = input->global_end();
      I != E; ++I) {
    llvm::GlobalVariable* GVar = I;
    AliveGlobals.insert(GVar);
  }
  
  // Link libraries to get every functions that are referenced.
  for (unsigned int i=0; i < libs.size(); i++) {
    llvm::Module* Library = libs[i];
    if (linkWithModule(input, Library, &modRefMaps[i])) {
      errs() << "Linking bc libraries failed!\n";
      delete[] modRefMaps;
      return 1;
    }

    delete Library;
    libs[i] = NULL;
  }

  delete[] modRefMaps;

  // Now, unlink those useless globals linked in from libraries
  UnlinkGlobals(input, AliveGlobals);
  AliveGlobals.clear();

  return 0;
}
} // namespace BCLinker
} // namespace llvm
#endif // AMD_OPENCL
