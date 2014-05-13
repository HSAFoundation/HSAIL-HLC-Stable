//===-- HSAILMachObjectWriter.cpp - HSAIL Mach-O Writer -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "HSAIL.h"
#include "HSAILMachObjectWriter.h"
#include "llvm/MC/MCMachObjectWriter.h"
using namespace llvm;

HSAILMachObjectWriter::HSAILMachObjectWriter(bool Is64Bit,
                                             uint32_t CPUType,
                                             uint32_t CPUSubtype)
  : MCMachObjectTargetWriter(Is64Bit, CPUType, CPUSubtype,
                             /*UseAggressiveSymbolFolding=*/Is64Bit) {}

MCObjectWriter*
llvm::createHSAILMachObjectWriter(raw_ostream &OS,
                                  bool Is64Bit,
                                  uint32_t CPUType,
                                  uint32_t CPUSubtype)
{
  assert(!"When do we hit this?");
  return NULL;
}
