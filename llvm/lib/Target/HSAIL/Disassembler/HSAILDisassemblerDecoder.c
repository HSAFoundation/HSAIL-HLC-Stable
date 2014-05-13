/*===- HSAILDisassemblerDecoder.c - Disassembler decoder ------------*- C -*-==*
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 *===----------------------------------------------------------------------===*
 *
 * This file is part of the HSAIL Disassembler.
 * It contains the implementation of the instruction decoder.
 * Documentation for the disassembler can be found in HSAILDisassembler.h.
 *
 *===----------------------------------------------------------------------===*/

#include <stdarg.h>   /* for va_*()       */
#include <stdio.h>    /* for vsnprintf()  */
#include <stdlib.h>   /* for exit()       */
#include <string.h>   /* for memset()     */
#include <assert.h>

#include "HSAILDisassemblerDecoder.h"

/* #include "HSAILGenDisassemblerTables.inc" */

/*
 * decodeInstruction - Reads and interprets a full instruction provided by the
 *   user.
 *
 * @param insn      - A pointer to the instruction to be populated.  Must be
 *                    pre-allocated.
 * @param reader    - The function to be used to read the instruction's bytes.
 * @param readerArg - A generic argument to be passed to the reader to store
 *                    any internal state.
 * @param logger    - If non-NULL, the function to be used to write log messages
 *                    and warnings.
 * @param loggerArg - A generic argument to be passed to the logger to store
 *                    any internal state.
 * @param startLoc  - The address (in the reader's address space) of the first
 *                    byte in the instruction.
 * @param mode      - The mode (real mode, IA-32e, or IA-32e in 64-bit mode) to
 *                    decode the instruction in.
 * @return          - 0 if the instruction's memory could be read; nonzero if
 *                    not.
 */
int
decodeInstruction(struct InternalInstruction* insn,
                  byteReader_t reader,
                  void* readerArg,
                  dlog_t logger,
                  void* loggerArg,
                  uint64_t startLoc,
                  DisassemblerMode mode)
{
  assert(!"When do we hit this?");
  return 0;
}
