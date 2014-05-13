//
// Copyright (c) 2008 Advanced Micro Devices, Inc. All rights reserved.
//
#ifndef _BC_LINKER_HPP_
#define _BC_LINKER_HPP_
#include <vector>

namespace llvm {
  class Module;

  namespace BCLinker {
    int link(llvm::Module* input, std::vector<llvm::Module*> &libs);
  };
}; // namespace llvm
#endif // _BC_LINKER_HPP_
