//===- llvm-link.cpp - Low-level LLVM linker ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This utility may be invoked in the following manner:
//  llvm-link a.bc b.bc c.bc -o x.bc
//
//===----------------------------------------------------------------------===//

#include "llvm/Linker.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/SystemUtils.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/Path.h"
#include "llvm/Transforms/AMDEDGToIA64Translator.h"
#if defined(AMD_OPENCL) || 1
#include "llvm/AMDLLVMContextHook.h"
#include "llvm/AMDPrelinkOpt.h"
#include "llvm/AMDResolveLinker.h"
#include "llvm/ADT/Triple.h"
#endif // AMD_OPENCL
#include <memory>
using namespace llvm;

static cl::list<std::string>
InputFilenames(cl::Positional, cl::OneOrMore,
               cl::desc("<input bitcode files>"));

static cl::opt<std::string>
OutputFilename("o", cl::desc("Override output filename"), cl::init("-"),
               cl::value_desc("filename"));

static cl::opt<bool>
Force("f", cl::desc("Enable binary output on terminals"));

static cl::opt<bool>
OutputAssembly("S",
         cl::desc("Write output as LLVM assembly"), cl::Hidden);

static cl::opt<bool>
Verbose("v", cl::desc("Print information about actions taken"));

static cl::opt<bool>
DumpAsm("d", cl::desc("Print assembly as linked"), cl::Hidden);

#if defined(AMD_OPENCL) || 1
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

static cl::opt<bool>
LowerToPreciseFunctions("fp32-correctly-rounded-divide-sqrt",
     cl::desc("lower sqrt and divide functions to precise library functions"));

static cl::list<std::string> Libraries("l", cl::Prefix,
                                       cl::desc("Specify libraries to link to"),
                                       cl::value_desc("library prefix"));

namespace llvm {
  // Defined in FixUpModule.cpp
  void fixUpModule(Module *M, StringRef TargetTripleStr,
                   StringRef TargetLayoutStr);
}

#endif // AMD_OPENCL


// LoadFile - Read the specified bitcode file in and return it.  This routine
// searches the link path for the specified file to try to find it...
//
static inline std::auto_ptr<Module> LoadFile(const char *argv0,
                                             const std::string &FN, 
                                             LLVMContext& Context) {
  sys::Path Filename;
  if (!Filename.set(FN)) {
    errs() << "Invalid file name: '" << FN << "'\n";
    return std::auto_ptr<Module>();
  }

  SMDiagnostic Err;
  if (Verbose) errs() << "Loading '" << Filename.c_str() << "'\n";
  Module* Result = 0;
  
  const std::string &FNStr = Filename.str();
  Result = ParseIRFile(FNStr, Err, Context);
  if (Result) return std::auto_ptr<Module>(Result);   // Load successful!

  Err.print(argv0, errs());
  return std::auto_ptr<Module>();
}

int main(int argc, char **argv) {
  // Print a stack trace if we signal out.
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);
  
  LLVMContext &Context = getGlobalContext();
  llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
  cl::ParseCommandLineOptions(argc, argv, "llvm linker\n");

  unsigned BaseArg = 0;
  std::string ErrorMessage;

  std::auto_ptr<Module> Composite(LoadFile(argv[0],
                                           InputFilenames[BaseArg], Context));
  if (Composite.get() == 0) {
    errs() << argv[0] << ": error loading file '"
           << InputFilenames[BaseArg] << "'\n";
    return 1;
  }

  for (unsigned i = BaseArg+1; i < InputFilenames.size(); ++i) {
    std::auto_ptr<Module> M(LoadFile(argv[0],
                                     InputFilenames[i], Context));
    if (M.get() == 0) {
      errs() << argv[0] << ": error loading file '" <<InputFilenames[i]<< "'\n";
      return 1;
    }

#if defined(AMD_OPENCL) || 1
    if (Verbose) errs() << "Appending '" << InputFilenames[i] << "'\n";
#else
    if (Verbose) errs() << "Linking in '" << InputFilenames[i] << "'\n";
#endif // AMD_OPENCL

    if (Linker::LinkModules(Composite.get(), M.get(), Linker::DestroySource,
                            &ErrorMessage)) {
      errs() << argv[0] << ": link error in '" << InputFilenames[i]
             << "': " << ErrorMessage << "\n";
      return 1;
    }
  }

  // TODO: Iterate over the -l list and link in any modules containing
  // global symbols that have not been resolved so far.

#if defined(AMD_OPENCL) || 1

  // Link unresolved symbols from libraries
  std::vector<Module*> Libs;
  for (std::vector<std::string>::iterator i = Libraries.begin(),
       e = Libraries.end(); i != e; ++i) {
    std::auto_ptr<Module> M(LoadFile(argv[0], *i, Context));
    if (M.get() == 0) {
      SMDiagnostic Err(*i, SourceMgr::DK_Error, "error loading file");
      Err.print(argv[0], errs());
      return 1;
    }
    if (Verbose) errs() << "Linking in '" << *i << "'\n";
    Libs.push_back(M.get());
    M.release();
  }

  if (Libs.size() > 0) {
    // The first member in the list of libraries is assumed to be
    // representative of the target device.
    fixUpModule(Composite.get(), Libs[0]->getTargetTriple(),
                Libs[0]->getDataLayout());

    if (PreLinkOpt) {
      AMDLLVMContextHook AmdHook;
      Composite.get()->getContext().setAMDLLVMContextHook(&AmdHook);
      AmdHook.amdrtFunctions = NULL;
      AMDPrelinkOpt(Composite.get(), EnableWholeProgram, DisableSimplifyLibCalls,
                 EnableUnsafeFPMath, NULL /*UseNative*/,
                 LowerToPreciseFunctions);
    }

    std::string ErrorMsg;
    if (resolveLink(Composite.get(), Libs, &ErrorMsg)) {
      SMDiagnostic Err(InputFilenames[BaseArg], SourceMgr::DK_Error, ErrorMsg);
      Err.print(argv[0], errs());
      return 1;
    }
  }

#endif // AMD_OPENCL

  if (DumpAsm) errs() << "Here's the assembly:\n" << *Composite;

  std::string ErrorInfo;
  tool_output_file Out(OutputFilename.c_str(), ErrorInfo,
                       raw_fd_ostream::F_Binary);
  if (!ErrorInfo.empty()) {
    errs() << ErrorInfo << '\n';
    return 1;
  }

  if (verifyModule(*Composite)) {
    errs() << argv[0] << ": linked module is broken!\n";
    return 1;
  }

  if (Verbose) errs() << "Writing bitcode...\n";
  if (OutputAssembly) {
    Out.os() << *Composite;
  } else if (Force || !CheckBitcodeOutputToConsole(Out.os(), true))
    WriteBitcodeToFile(Composite.get(), Out.os());

  // Declare success.
  Out.keep();

  return 0;
}
