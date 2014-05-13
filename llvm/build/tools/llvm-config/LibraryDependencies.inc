//===- llvm-build generated file --------------------------------*- C++ -*-===//
//
// Component Library Depenedency Table
//
// Automatically generated file, do not edit!
//
//===----------------------------------------------------------------------===//

struct AvailableComponent {
  /// The name of the component.
  const char *Name;

  /// The name of the library for this component (or NULL).
  const char *Library;

  /// Whether the component is installed.
  bool IsInstalled;

  /// The list of libraries required when linking this component.
  const char *RequiredLibraries[14];
} AvailableComponents[53] = {
  { "all", 0, 1, { "mcjit", "bitwriter", "linker", "ipo", "interpreter", "instrumentation", "mcdisassembler", "engine", "debuginfo", "nativecodegen", "tablegen", "asmparser", "all-targets" } },
  { "all-targets", 0, 1, { "amdil", "hsail", "x86" } },
  { "amdil", 0, 1, { "amdilmacrodb", "amdilcodegen", "amdilinfo", "amdilinternal", "amdilmdparser", "amdildesc" } },
  { "amdilcodegen", "libLLVMAMDILCodeGen.a", 1, { "amdilmacrodb", "asmprinter", "codegen", "core", "mc", "selectiondag", "support", "target" } },
  { "amdildesc", "libLLVMAMDILDesc.a", 1, { "mc", "support", "amdilinfo" } },
  { "amdilinfo", "libLLVMAMDILInfo.a", 1, { "mc", "support", "target" } },
  { "amdilinternal", "libLLVMAMDILInternal.a", 1, { "asmprinter", "codegen", "core", "mc", "selectiondag", "support", "target" } },
  { "amdilmacrodb", "libLLVMAMDILMacroDB.a", 1, {  } },
  { "amdilmdparser", "libLLVMAMDILMDParser.a", 1, { "mc", "support", "target" } },
  { "analysis", "libLLVMAnalysis.a", 1, { "core", "support", "target" } },
  { "archive", "libLLVMArchive.a", 1, { "bitreader", "core", "support" } },
  { "asmparser", "libLLVMAsmParser.a", 1, { "core", "support" } },
  { "asmprinter", "libLLVMAsmPrinter.a", 1, { "analysis", "codegen", "core", "mc", "mcparser", "support", "target" } },
  { "bitreader", "libLLVMBitReader.a", 1, { "core", "support" } },
  { "bitwriter", "libLLVMBitWriter.a", 1, { "core", "support" } },
  { "codegen", "libLLVMCodeGen.a", 1, { "analysis", "core", "mc", "scalaropts", "support", "target", "transformutils" } },
  { "core", "libLLVMCore.a", 1, { "support" } },
  { "debuginfo", "libLLVMDebugInfo.a", 1, { "support" } },
  { "engine", 0, 1, { "jit", "native" } },
  { "executionengine", "libLLVMExecutionEngine.a", 1, { "core", "mc", "support", "target" } },
  { "hsail", 0, 1, { "hsailcodegen", "hsailinfo" } },
  { "hsailcodegen", "libLLVMHSAILCodeGen.a", 1, { "asmprinter", "codegen", "core", "mc", "selectiondag", "support", "target" } },
  { "hsailinfo", "libLLVMHSAILInfo.a", 1, { "mc", "support", "target" } },
  { "instcombine", "libLLVMInstCombine.a", 1, { "analysis", "core", "support", "target", "transformutils" } },
  { "instrumentation", "libLLVMInstrumentation.a", 1, { "analysis", "core", "support", "transformutils" } },
  { "interpreter", "libLLVMInterpreter.a", 1, { "codegen", "core", "executionengine", "support", "target" } },
  { "ipa", "libLLVMipa.a", 1, { "analysis", "core", "support" } },
  { "ipo", "libLLVMipo.a", 1, { "analysis", "core", "ipa", "instcombine", "scalaropts", "vectorize", "support", "target", "transformutils" } },
  { "jit", "libLLVMJIT.a", 1, { "codegen", "core", "executionengine", "mc", "runtimedyld", "support", "target" } },
  { "linker", "libLLVMLinker.a", 1, { "archive", "bitreader", "core", "support", "transformutils" } },
  { "mc", "libLLVMMC.a", 1, { "object", "support" } },
  { "mcdisassembler", "libLLVMMCDisassembler.a", 1, { "mc", "mcparser", "support" } },
  { "mcjit", "libLLVMMCJIT.a", 1, { "core", "executionengine", "runtimedyld", "support", "target" } },
  { "mcparser", "libLLVMMCParser.a", 1, { "mc", "support" } },
  { "native", 0, 1, { "x86" } },
  { "nativecodegen", 0, 1, { "x86codegen" } },
  { "object", "libLLVMObject.a", 1, { "support" } },
  { "runtimedyld", "libLLVMRuntimeDyld.a", 1, { "object", "support" } },
  { "scalaropts", "libLLVMScalarOpts.a", 1, { "analysis", "core", "instcombine", "support", "target", "transformutils" } },
  { "selectiondag", "libLLVMSelectionDAG.a", 1, { "analysis", "codegen", "core", "mc", "support", "target", "transformutils" } },
  { "support", "libLLVMSupport.a", 1, {  } },
  { "tablegen", "libLLVMTableGen.a", 1, { "support" } },
  { "target", "libLLVMTarget.a", 1, { "core", "mc", "support" } },
  { "transformutils", "libLLVMTransformUtils.a", 1, { "analysis", "core", "ipa", "support", "target" } },
  { "vectorize", "libLLVMVectorize.a", 1, { "analysis", "core", "instcombine", "support", "target", "transformutils" } },
  { "x86", 0, 1, { "x86utils", "x86asmprinter", "x86info", "x86desc", "x86codegen", "x86asmparser", "x86disassembler" } },
  { "x86asmparser", "libLLVMX86AsmParser.a", 1, { "mc", "mcparser", "support", "x86desc", "x86info" } },
  { "x86asmprinter", "libLLVMX86AsmPrinter.a", 1, { "mc", "support", "x86utils" } },
  { "x86codegen", "libLLVMX86CodeGen.a", 1, { "analysis", "asmprinter", "codegen", "core", "mc", "selectiondag", "support", "target", "x86asmprinter", "x86desc", "x86info", "x86utils" } },
  { "x86desc", "libLLVMX86Desc.a", 1, { "mc", "support", "x86asmprinter", "x86info" } },
  { "x86disassembler", "libLLVMX86Disassembler.a", 1, { "mc", "support", "x86desc", "x86info" } },
  { "x86info", "libLLVMX86Info.a", 1, { "mc", "support", "target" } },
  { "x86utils", "libLLVMX86Utils.a", 1, { "core", "support" } },
};
