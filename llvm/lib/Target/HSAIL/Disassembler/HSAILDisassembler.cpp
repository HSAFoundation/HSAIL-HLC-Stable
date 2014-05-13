//===- HSAILDisassembler.cpp - Disassembler for hsail  ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is part of the HSAIL Disassembler.
// It contains code to translate the data produced by the decoder into MCInsts.
// Documentation for the disassembler can be found in HSAILDisassembler.h.
//
//===----------------------------------------------------------------------===//

#include "HSAILDisassembler.h"
#include "HSAILDisassemblerDecoder.h"

#include "llvm/MC/EDInstInfo.h"
#include "llvm/MC/MCDisassembler.h"
#include "llvm/MC/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Target/TargetRegistry.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MemoryObject.h"
#include "llvm/Support/raw_ostream.h"

#include "HSAILGenRegisterNames.inc"
//#include "HSAILGenEDInfo.inc"

using namespace llvm;
using namespace llvm::HSAILDisassembler;

void
HSAILDisassemblerDebug(const char *file,
                       unsigned line,
                       const char *s)
{
  assert(!"When do we hit this?");
}

#define debug(s) DEBUG(hsailDisassemblerDebug(__FILE__, __LINE__, s));

namespace llvm {

// Fill-ins to make the compiler happy.  These constants are never actually
//   assigned; they are just filler to make an automatically-generated switch
//   statement work.
namespace HSAIL {
  enum {
    BX_SI = 500,
    BX_DI = 501,
    BP_SI = 502,
    BP_DI = 503,
    sib   = 504,
    sib64 = 505
  };
}

extern Target TheHSAIL_32Target, TheHSAIL_64Target;

}

HSAILGenericDisassembler::HSAILGenericDisassembler(DisassemblerMode mode)
  : MCDisassembler(), fMode(mode) {}

HSAILGenericDisassembler::~HSAILGenericDisassembler() {}

EDInstInfo*
HSAILGenericDisassembler::getEDInfo() const
{
  assert(!"When do we hit this?");
  return NULL;
}

//
// Public interface for the disassembler
//

bool
HSAILGenericDisassembler::getInstruction(MCInst &instr,
                                         uint64_t &size,
                                         const MemoryObject &region,
                                         uint64_t address,
                                         raw_ostream &vStream) const
{
  assert(!"When do we hit this?");
  return false;
}

extern "C" void LLVMInitializeHSAILDisassembler()
{
  //assert(!"When do we hit this?");
  return;
}
