//===-- HSAILInstComments.cpp - Generate verbose-asm comments for instrs --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This defines functionality used to emit comments about HSAIL instructions to
// an output stream for -fverbose-asm.
//
//===----------------------------------------------------------------------===//

#include "HSAILInstComments.h"
//#include "HSAILGenInstrNames.inc"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

//===----------------------------------------------------------------------===//
// Top Level Entrypoint
//===----------------------------------------------------------------------===//

/// EmitAnyHSAILInstComments - This function decodes hsail instructions and
/// prints newline terminated strings to the specified string if desired.  This
/// information is shown in disassembly dumps when verbose assembly is enabled.
void
llvm::EmitAnyHSAILInstComments(const MCInst *MI,
                               raw_ostream &OS,
                               const char *(*getRegName)(unsigned))
{
  assert(!"When do we hit this?");
}
