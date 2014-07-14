//
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
//

#include <llvm/Constants.h>
#include <llvm/DataLayout.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Metadata.h>
#include <llvm/Module.h>
#include <llvm/PassManager.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Triple.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar.h>

using namespace llvm;

static bool isSPIRTriple(const Triple &Triple) {
  return Triple.getArch() == Triple::spir
    || Triple.getArch() == Triple::spir64;
}

static bool isAMDILTriple(const Triple &Triple) {
  return Triple.getArch() == Triple::amdil
    || Triple.getArch() == Triple::amdil64;
}

static bool isX86Triple(const Triple &Triple) {
  return Triple.getArch() == Triple::x86
    || Triple.getArch() == Triple::x86_64;
}

static bool isHSAILTriple(const Triple &Triple) {
  return Triple.getArch() == Triple::hsail
    || Triple.getArch() == Triple::hsail_64;
}

static void CheckSPIRVersion(const Module *M,
                             const Triple &TargetTriple) {
  const NamedMDNode *SPIRVersion
    = M->getNamedMetadata("opencl.spir.version");
  assert(SPIRVersion);
  assert(SPIRVersion->getNumOperands() == 1);

  const MDNode *VersionMD = SPIRVersion->getOperand(0);
  assert(VersionMD->getNumOperands() == 2);

  const ConstantInt *CMajor
    = cast<ConstantInt>(VersionMD->getOperand(0));
  assert(CMajor->getType()->getIntegerBitWidth() == 32);

  unsigned VersionMajor = CMajor->getZExtValue();
  switch (VersionMajor) {
  case 1:
    break;
  case 2:
    assert(!isAMDILTriple(TargetTriple));
    break;
  default:
    llvm_unreachable("Unknown SPIR version");
    break;
  }
}

static void runAdapters(llvm::Module *M,
                        llvm::StringRef TargetTripleStr,
                        llvm::StringRef TargetLayoutStr,
                        bool RunSPIRLoader,
                        bool DemangleBuiltins,
                        bool RunEDGAdapter,
                        bool SetSPIRCallingConv,
                        bool RunX86Adapter,
                        bool RunPrintfRuntimeBinding,
                        bool RunPrintfCpuLowering,
                        bool RunLowerEnqueueKernel,
                        bool RunGenDevEnqMetadata) {

  M->setTargetTriple(TargetTripleStr);
  M->setDataLayout(TargetLayoutStr);

  PassManager Passes;
  Passes.add(new DataLayout(M));

  if (RunPrintfRuntimeBinding == true || RunPrintfCpuLowering == true)
    Passes.add(llvm::createAMDPrintfRuntimeBinding(RunPrintfCpuLowering));

  Passes.add(createAMDLowerAtomicsPass());
  Passes.add(createAMDLowerPipeBuiltinsPass());

  if (RunEDGAdapter) {
    assert(!RunSPIRLoader);
    Passes.add(createAMDEDGToIA64TranslatorPass(SetSPIRCallingConv));
  }

  if (RunSPIRLoader) {
    assert(!RunEDGAdapter);
    Passes.add(createSPIRLoader(DemangleBuiltins));
  }

  if (RunX86Adapter) {
    //One of them should run before the AMDX86Adapter Pass.
    assert(RunSPIRLoader || RunEDGAdapter);
    Passes.add(llvm::createAMDX86AdapterPass());
  }

  if (RunLowerEnqueueKernel) {
    Passes.add(llvm::createAMDLowerEnqueueKernelPass());
  }

  if (RunGenDevEnqMetadata) {
    Passes.add(llvm::createAMDGenerateDevEnqMetadataPass());
  }

  Passes.run(*M);
}

namespace llvm {

void fixUpModule(Module *M, StringRef TargetTripleStr,
                 StringRef TargetLayoutStr) {

  // We need to fix-up the kernel module so that it matches the
  // builtin library.
  //
  // The decisions here are identical to the decisions taken in
  // compiler/lib/backends/common/linker.cpp,
  // amdcl::OCLLinker::link(). See the body of that function for
  // details.

                                    // Enabled for:
  bool RunSPIRLoader = false;       // SPIR     -> x86/HSAIL/AMDIL
  bool DemangleBuiltins = false;    // SPIR     -> AMDIL
  bool RunEDGAdapter = false;       // EDG      -> x86/HSAIL
  bool SetSPIRCallingConv = false;  // EDG      -> HSAIL
  bool RunX86Adapter = false;       // SPIR/EDG -> x86
  bool RunLowerEnqueueKernel = false;
  bool RunPrintfRuntimeBinding = false;
  bool RunPrintfCpuLowering = false;
  bool RunGenDevEnqMetadata = false;

  assert(!TargetTripleStr.empty());
  Triple TargetTriple(TargetTripleStr);
  Triple ModuleTriple(M->getTargetTriple());

  if (isSPIRTriple(ModuleTriple)) {
    CheckSPIRVersion(M, TargetTriple);
    RunSPIRLoader = true;
#if OPENCL_MAJOR >= 2 // this will become default
    DemangleBuiltins |= isAMDILTriple(TargetTriple);
#ifdef BUILD_HSA_TARGET // special case for HSA build
    DemangleBuiltins |= isHSAILTriple(TargetTriple);
#endif
    // Never demangle for x86 target on 200 build.
#else // OpenCL 1.2 build (this will go away)
    DemangleBuiltins = true;
#endif
  } else {
#if OPENCL_MAJOR >= 2
    // Decide if we need to adapt the non-SPIR (EDG) kernel module.
    //
    // FIXME: Remove the #ifdef when x86 and HSAIL libraries are
    // always built by Clang.
#ifndef BUILD_HSA_TARGET
    // Run the adapter for HSAIL only if this is an ORCA build!
    //
    // On an HSA build, the HSAIL library is always built with EDG.
    // This assumption must match the settings in
    // "opencl/library/hsa/hsail/build/Makefile.hsail"
    RunEDGAdapter |= isHSAILTriple(TargetTriple);
#endif
    // HSAIL requires SPIR calling conventions since the library is in
    // SPIR format. This doesn't matter if the EDGAdapter is not run.
    SetSPIRCallingConv = isHSAILTriple(TargetTriple);

    // Run the EDG Adapter if OPENCL_MAJOR >= 2 and for x86 target.
    RunEDGAdapter |= isX86Triple(TargetTriple);
#endif // OPENCL_MAJOR >= 2
  }

// FIXME: RunX86Adapter bool should be removed when x86 builtins
// is always built by Clang on OpenCL 1.2 builds.
#if OPENCL_MAJOR >= 2
  RunX86Adapter = isX86Triple(TargetTriple);
  RunLowerEnqueueKernel = isSPIRTriple(ModuleTriple);
  // Printf is bound to a buffer
  // for OpenCL2.0 when compiled with the 
  // HSAIL target triple.
  // Check for OpenCL2.0 version
  // is inside the printf lowering
  // pass.
  // For X86, printf formats are 
  // lowered for opencl vector
  // formats.
  if (isHSAILTriple(TargetTriple)) {
    RunPrintfRuntimeBinding = true;
  } else if (isX86Triple(TargetTriple)) {
    RunPrintfCpuLowering = true;
  }
  RunGenDevEnqMetadata = isHSAILTriple(TargetTriple);
#endif

  runAdapters(M, TargetTripleStr, TargetLayoutStr,
              RunSPIRLoader, DemangleBuiltins,
              RunEDGAdapter, SetSPIRCallingConv,
              RunX86Adapter, RunPrintfRuntimeBinding,
              RunPrintfCpuLowering, RunLowerEnqueueKernel,
              RunGenDevEnqMetadata);
}

} // end namespace llvm
