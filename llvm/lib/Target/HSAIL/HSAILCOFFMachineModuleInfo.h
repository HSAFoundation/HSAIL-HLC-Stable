//===------------- HSAILCOFFMachineModuleInfo.h -----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is an MMI implementation for HSAIL COFF (windows) targets.
//
//===----------------------------------------------------------------------===//

#ifndef _HSAIL_COFF_MACHINE_MODULE_INFO_H_
#define _HSAIL_COFF_MACHINE_MODULE_INFO_H_

#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/ADT/DenseSet.h"
#include "HSAILMachineFunctionInfo.h"

namespace llvm {
  class HSAILMachineFunctionInfo;
  class DataLayout;

/// HSAILCOFFMachineModuleInfo - This is a MachineModuleInfoImpl implementation
/// for HSAIL COFF targets.
class HSAILCOFFMachineModuleInfo : public MachineModuleInfoImpl {
  DenseSet<MCSymbol const *> Externals;
public:
  HSAILCOFFMachineModuleInfo(const MachineModuleInfo &) {}

  virtual ~HSAILCOFFMachineModuleInfo();

  void
  addExternalFunction(MCSymbol* Symbol)
  {
    Externals.insert(Symbol);
  }

  typedef DenseSet<MCSymbol const *>::const_iterator externals_iterator;

  externals_iterator
  externals_begin() const
  {
    return Externals.begin();
  }

  externals_iterator
  externals_end() const
  {
    return Externals.end();
  }
};
} // end namespace llvm

#endif // _HSAIL_COFF_MACHINE_MODULE_INFO_H_
