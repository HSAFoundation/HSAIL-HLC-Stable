//===-- AMDMetadataUtils.h - Manage metadata across passes -------*- C++ -*-===//
//
//
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Provide utilities to update LLVM metadata such as debug
///        info and SPIR metadata when the module is modified.
///
//===----------------------------------------------------------------------===//

#ifndef __AMD_METADATA_UTILS_H__
#define __AMD_METADATA_UTILS_H__

#include <llvm/DebugInfo.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringRef.h>

namespace llvm {
  class Function;
  class Module;
  class MDNode;

  class DebugInfoManager {
    /// Map a function to corresponding debug info.
    typedef DenseMap<Function*, DISubprogram> FunctionDIMap;
    FunctionDIMap FunctionDIs;

  public:
    void collectFunctionDIs(Module &M);
    void replaceFunctionDI(Function *OldF, Function *NewF);
  };

  void updateFunctionAnnotations(Module &M, Function *From, Function *To);
  void updateSPIRMetadata(Module &M, Function *OldF, Function *NewF);
  bool matchName(const MDNode *Node, StringRef Name);
}

#endif
