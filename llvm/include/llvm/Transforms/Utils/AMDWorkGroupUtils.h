//===- Transforms/Utils/AMDWorkGroupUtils.h - WorkGroup Utils ---*- C++ -*-===//
//
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_AMDWORKGROUPUTILS_H
#define LLVM_TRANSFORMS_UTILS_AMDWORKGROUPUTILS_H
#include "llvm/Function.h"
#include "llvm/ADT/StringRef.h"

namespace llvm {

  enum {
    SARG_TIB_IDX,
    SARG_PRIVATE_IDX,
    SARG_WGSIZE_IDX,

    NUM_SPECIAL_ARGS_STUB,


    SARG_LOCALIDS_IDX = SARG_PRIVATE_IDX,

    NUM_SPECIAL_ARGS_FUNC
  };

# define RESERVED_NAME_PREFIX "cl@"
# define RESERVE_NAME(name) RESERVED_NAME_PREFIX name

# ifdef NDEBUG
#   define DEBUG_RESERVE_NAME(name) ""
# else
#   define DEBUG_RESERVE_NAME(name) RESERVE_NAME(name)
# endif

# define SARG_TIB_NAME        DEBUG_RESERVE_NAME("tib")
# define SARG_PRIVATE_NAME    DEBUG_RESERVE_NAME("private")
# define SARG_WGSIZE_NAME     DEBUG_RESERVE_NAME("wgsize")

# define FUNC_GET_TIB_LOCALID_NAME  RESERVE_NAME("get_tib_local_id")

inline std::string GetKernelDelegateName(const Function *FStub) {
  return "delegate@" + FStub->getName().str();
}

inline StringRef UndecorateKernelDelegateName(const Function *FDlg) {
  return FDlg->getName().substr(strlen("delegate@"),
                                FDlg->getName().size() - strlen("delegate@"));
}

} // End llvm namespace

#endif
