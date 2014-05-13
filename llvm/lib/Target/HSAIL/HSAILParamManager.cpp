//=== HSAILParamManager.cpp - kernel/function arguments ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HSAILParamManager class.
//
//===----------------------------------------------------------------------===//

#include "HSAIL.h"
#include "HSAILParamManager.h"
#include "llvm/ADT/StringExtras.h"

using namespace llvm;

HSAILParamManager::HSAILParamManager() {
}

HSAILParamManager::~HSAILParamManager() {
  // Special handling for teardown of ParamNames
  for (names_iterator I = ParamNames.begin(), E = ParamNames.end(); 
    I != E; ++I) {
    // Delete malloc'ed name strings
    free(I->second);
  }
  ParamNames.clear();
}

unsigned HSAILParamManager::addArgumentParam(unsigned Size) {
  HSAILParam Param;
  Param.Type = HSAIL_PARAM_TYPE_ARGUMENT;
  Param.Size = Size;

  std::string Name;
  Name = "arg_p";
  Name += utostr(ArgumentParams.size());

  unsigned Index = AllParams.size();
  AllParams[Index] = Param;
  ArgumentParams.push_back(Index);

  addParamName(Name, Index);

  return Index;
}

unsigned HSAILParamManager::addReturnParam(unsigned Size) {
  HSAILParam Param;
  Param.Type = HSAIL_PARAM_TYPE_RETURN;
  Param.Size = Size;

  std::string Name;
  Name = "ret_val";
  Name += utostr(ReturnParams.size());

  unsigned Index = AllParams.size();
  AllParams[Index] = Param;
  ReturnParams.push_back(Index);

  addParamName(Name, Index);

  return Index;
}

void HSAILParamManager::addParamName(std::string Name, unsigned Index) {
  // malloc arg name string so that it persists through compilation
  char* name = (char*)malloc(Name.length()+1);
  strcpy(name, Name.c_str());
  ParamNames[Index] = name;
}

void HSAILParamManager::addParamType(const Type * pTy, unsigned Index) {
  ParamTypes[Index] = pTy;
}
