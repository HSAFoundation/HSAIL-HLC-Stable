//===-- HSAILInstPrinter.cpp - assembly instruction printing --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file includes code for rendering MCInst instances as assembly.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "asm-printer"
#include "HSAILInstPrinter.h"
#include "HSAILInstComments.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
//#include "HSAILGenInstrNames.inc"
#include <cctype>
using namespace llvm;

// Include the auto-generated portion of the assembly writer.
//#define GET_INSTRUCTION_NAME
//#include "HSAILGenAsmWriter1.inc"

void
HSAILInstPrinter::printInst(const MCInst *MI,
                            raw_ostream &OS, StringRef annot)
{
  assert(!"When do we hit this?");
}

StringRef
HSAILInstPrinter::getOpcodeName(unsigned Opcode) const
{
  assert(!"When do we hit this?");
  return StringRef();
}

void
HSAILInstPrinter::printOperand(const MCInst *MI,
                               unsigned OpNo,
                               raw_ostream &O)
{
  assert(!"When do we hit this?");
}

void
HSAILInstPrinter::printMemReference(const MCInst *MI,
                                    unsigned Op,
                                    raw_ostream &O)
{
  assert(!"When do we hit this?");
}
