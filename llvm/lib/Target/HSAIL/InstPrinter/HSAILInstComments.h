//===-- HSAILInstComments.h - Generate verbose-asm comments for instrs ----===//
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

#ifndef HSAIL_INST_COMMENTS_H
#define HSAIL_INST_COMMENTS_H

namespace llvm {
  class MCInst;
  class raw_ostream;
  void EmitAnyHSAILInstComments(const MCInst *MI,
                                raw_ostream &OS,
                                const char *(*getRegName)(unsigned));
}

#endif
