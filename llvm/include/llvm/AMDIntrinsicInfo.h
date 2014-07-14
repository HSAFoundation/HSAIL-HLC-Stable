//
// Copyright (c) 2011 Advanced Micro Devices, Inc. All rights reserved.
//

// This file defines an interface to hook-up objects specific to AMD OpenCL.
// AMD OpenCL creates a new context for each OpenCL build, and different builds
// have different contexts.
//
#ifndef _AMD_INTRINSIC_INFO_H_
#define _AMD_INTRINSIC_INFO_H_

#include "llvm/ADT/StringRef.h"
#include "llvm/Type.h"

namespace llvm {

// parseName and mangleName supports EDG and Itanium (de)mangling for functions 
// with one parameter of the kind:
//
// EDG:               __name_([pglc]?(\d+)?([uidf]\d+)) 
// Itanium (SPIR):    _Z\d+name(PU3AS\d)?(Dv\d+_)?([ijmltschdf]|Dh)
//
//       $1 - pointer modifier, $2 - vector, $3 - type 


struct AMDIntrinsicInfo {
  enum EMangling { EDG, ITANIUM };
  enum EKind { NORMAL, NATIVE, HALF , HSAIL, AMDIL, GCN };
  
  Type::TypeID   ArgType;
  EKind          FKind;
  int            VectorSize;
  bool           HasPtr;
  EMangling      ManglingKind;

  AMDIntrinsicInfo(Type::TypeID t=Type::FloatTyID, EKind k=NORMAL, int vec=1)
      : ArgType(t)
      , FKind(k)
      , VectorSize(vec)
      , HasPtr(false)
      , ManglingKind(EDG)
  {}

  std::string mangleName(const StringRef& name) const;

  static StringRef parseName(StringRef mangledName, 
                             AMDIntrinsicInfo *iInfo=NULL/*out*/);
};

class FunctionType;

std::string mangleFuncNameItanium(const StringRef& name, FunctionType* type);

}
#endif // _AMD_INTRINSIC_INFO_H_
