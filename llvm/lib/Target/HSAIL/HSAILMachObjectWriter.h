//===-- HSAILMACHObjectWriter.h - HSAIL MACH Object Writer ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef _HSAIL_MACH_OBJECT_WRITER_H_
#define _HSAIL_MACH_OBJECT_WRITER_H_

#include "llvm/MC/MCMachObjectWriter.h"
using namespace llvm;

class HSAILMachObjectWriter : public MCMachObjectTargetWriter {
public:
  HSAILMachObjectWriter(bool Is64Bit,
                        uint32_t CPUType,
                        uint32_t CPUSubtype);
};
#endif // _HSAIL_MACH_OBJECT_WRITER_H_
