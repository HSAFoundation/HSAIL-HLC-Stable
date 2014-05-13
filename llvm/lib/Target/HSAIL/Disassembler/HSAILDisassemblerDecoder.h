/*===- HSAILDisassemblerDecoderInternal.h - Disassembler decoder ----*- C -*-==*
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 *===----------------------------------------------------------------------===*
 *
 * This file is part of the HSAIL Disassembler.
 * It contains the public interface of the instruction decoder.
 * Documentation for the disassembler can be found in HSAILDisassembler.h.
 *
 *===----------------------------------------------------------------------===*/

#ifndef HSAILDISASSEMBLERDECODER_H
#define HSAILDISASSEMBLERDECODER_H

#ifdef __cplusplus
extern "C" {
#endif

#define INSTRUCTION_SPECIFIER_FIELDS  \
  const char*             name;

#define INSTRUCTION_IDS     \
  const InstrUID *instructionIDs;

#include "HSAILDisassemblerDecoderCommon.h"

#undef INSTRUCTION_SPECIFIER_FIELDS
#undef INSTRUCTION_IDS

/*
 * TODO: Accessor functions for various fields of an Intel instruction
 */


/*
 * TODO: These enums represent GPU registers for use by the decoder.
 */


/*
 * EABase - All possible values of the base field for effective-address
 *   computations, a.k.a. the Mod and R/M fields of the ModR/M byte.  We
 *   distinguish between bases (EA_BASE_*) and registers that just happen to be
 *   referred to when Mod == 0b11 (EA_REG_*).
 */
typedef enum {
  EA_BASE_NONE,
#if 0
#define ENTRY(x) EA_BASE_##x,
  ALL_EA_BASES
#undef ENTRY
#define ENTRY(x) EA_REG_##x,
  ALL_REGS
#undef ENTRY
#endif
  EA_max
} EABase;

/*
 * SIBIndex - All possible values of the SIB index field.
 *   Borrows entries from ALL_EA_BASES with the special case that
 *   sib is synonymous with NONE.
 */
typedef enum {
  SIB_INDEX_NONE,
#if 0
#define ENTRY(x) SIB_INDEX_##x,
  ALL_EA_BASES
#undef ENTRY
#endif
  SIB_INDEX_max
} SIBIndex;

/*
 * SIBBase - All possible values of the SIB base field.
 */
typedef enum {
  SIB_BASE_NONE,
#if 0
#define ENTRY(x) SIB_BASE_##x,
  ALL_SIB_BASES
#undef ENTRY
#endif
  SIB_BASE_max
} SIBBase;

/*
 * EADisplacement - Possible displacement types for effective-address
 *   computations.
 */
typedef enum {
  EA_DISP_NONE,
  EA_DISP_8,
  EA_DISP_16,
  EA_DISP_32
} EADisplacement;

/*
 * Reg - All possible values of the reg field in the ModR/M byte.
 */
typedef enum {
#if 0
#define ENTRY(x) MODRM_REG_##x,
  ALL_REGS
#undef ENTRY
#endif
  MODRM_REG_max
} Reg;

/*
 * SegmentOverride - All possible segment overrides.
 */
typedef enum {
  SEG_OVERRIDE_NONE,
  SEG_OVERRIDE_CS,
  SEG_OVERRIDE_SS,
  SEG_OVERRIDE_DS,
  SEG_OVERRIDE_ES,
  SEG_OVERRIDE_FS,
  SEG_OVERRIDE_GS,
  SEG_OVERRIDE_max
} SegmentOverride;

typedef uint8_t BOOL;

/*
 * byteReader_t - Type for the byte reader that the consumer must provide to
 *   the decoder.  Reads a single byte from the instruction's address space.
 * @param arg     - A baton that the consumer can associate with any internal
 *                  state that it needs.
 * @param byte    - A pointer to a single byte in memory that should be set to
 *                  contain the value at address.
 * @param address - The address in the instruction's address space that should
 *                  be read from.
 * @return        - -1 if the byte cannot be read for any reason; 0 otherwise.
 */
typedef int (*byteReader_t)(void* arg, uint8_t* byte, uint64_t address);

/*
 * dlog_t - Type for the logging function that the consumer can provide to
 *   get debugging output from the decoder.
 * @param arg     - A baton that the consumer can associate with any internal
 *                  state that it needs.
 * @param log     - A string that contains the message.  Will be reused after
 *                  the logger returns.
 */
typedef void (*dlog_t)(void* arg, const char *log);

/*
 * The hsail internal instruction, which is produced by the decoder.
 */
struct InternalInstruction {
  /* Reader interface (C) */
  byteReader_t reader;
  /* Opaque value passed to the reader */
  void* readerArg;
  /* The address of the next byte to read via the reader */
  uint64_t readerCursor;

  /* Logger interface (C) */
  dlog_t dlog;
  /* Opaque value passed to the logger */
  void* dlogArg;

  /* General instruction information */

  /* The mode to disassemble for (64-bit, protected, real) */
  DisassemblerMode mode;
  /* The start of the instruction, usable with the reader */
  uint64_t startLocation;
  /* The length of the instruction, in bytes */
  size_t length;

  /* Prefix state */

  /* 1 if the prefix byte corresponding to the entry is present; 0 if not */
  uint8_t prefixPresent[0x100];
  /* contains the location (for use with the reader) of the prefix byte */
  uint64_t prefixLocations[0x100];
  /* The value of the REX prefix, if present */
  uint8_t rexPrefix;
  /* The location of the REX prefix */
  uint64_t rexLocation;
  /* The location where a mandatory prefix would have to be (i.e., right before
     the opcode, or right before the REX prefix if one is present) */
  uint64_t necessaryPrefixLocation;
  /* The segment override type */
  SegmentOverride segmentOverride;

  /* Sizes of various critical pieces of data, in bytes */
  uint8_t registerSize;
  uint8_t addressSize;
  uint8_t displacementSize;
  uint8_t immediateSize;

  /* opcode state */

  /* The value of the two-byte escape prefix (usually 0x0f) */
  uint8_t twoByteEscape;
  /* The value of the three-byte escape prefix (usually 0x38 or 0x3a) */
  uint8_t threeByteEscape;
  /* The last byte of the opcode, not counting any ModR/M extension */
  uint8_t opcode;
  /* The ModR/M byte of the instruction, if it is an opcode extension */
  uint8_t modRMExtension;

  /* decode state */

  /* The type of opcode, used for indexing into the array of decode tables */
  OpcodeType opcodeType;
  /* The instruction ID, extracted from the decode table */
  uint16_t instructionID;
  /* The specifier for the instruction, from the instruction info table */
  const struct InstructionSpecifier *spec;

  /* state for additional bytes, consumed during operand decode.  Pattern:
     consumed___ indicates that the byte was already consumed and does not
     need to be consumed again */

  /* The ModR/M byte, which contains most register operands and some portion of
     all memory operands */
  BOOL                          consumedModRM;
  uint8_t                       modRM;

  /* The SIB byte, used for more complex 32- or 64-bit memory operands */
  BOOL                          consumedSIB;
  uint8_t                       sib;

  /* The displacement, used for memory operands */
  BOOL                          consumedDisplacement;
  int32_t                       displacement;

  /* Immediates.  There can be two in some cases */
  uint8_t                       numImmediatesConsumed;
  uint8_t                       numImmediatesTranslated;
  uint64_t                      immediates[2];

  /* A register or immediate operand encoded into the opcode */
  BOOL                          consumedOpcodeModifier;
  uint8_t                       opcodeModifier;
  Reg                           opcodeRegister;

  /* Portions of the ModR/M byte */

  /* These fields determine the allowable values for the ModR/M fields, which
     depend on operand and address widths */
  EABase                        eaBaseBase;
  EABase                        eaRegBase;
  Reg                           regBase;

  /* The Mod and R/M fields can encode a base for an effective address, or a
     register.  These are separated into two fields here */
  EABase                        eaBase;
  EADisplacement                eaDisplacement;
  /* The reg field always encodes a register */
  Reg                           reg;

  /* SIB state */
  SIBIndex                      sibIndex;
  uint8_t                       sibScale;
  SIBBase                       sibBase;
};

/* decodeInstruction - Decode one instruction and store the decoding results in
 *   a buffer provided by the consumer.
 * @param insn      - The buffer to store the instruction in.  Allocated by the
 *                    consumer.
 * @param reader    - The byteReader_t for the bytes to be read.
 * @param readerArg - An argument to pass to the reader for storing context
 *                    specific to the consumer.  May be NULL.
 * @param logger    - The dlog_t to be used in printing status messages from the
 *                    disassembler.  May be NULL.
 * @param loggerArg - An argument to pass to the logger for storing context
 *                    specific to the logger.  May be NULL.
 * @param startLoc  - The address (in the reader's address space) of the first
 *                    byte in the instruction.
 * @param mode      - The mode (16-bit, 32-bit, 64-bit) to decode in.
 * @return          - Nonzero if there was an error during decode, 0 otherwise.
 */
int
decodeInstruction(struct InternalInstruction* insn,
                  byteReader_t reader,
                  void* readerArg,
                  dlog_t logger,
                  void* loggerArg,
                  uint64_t startLoc,
                  DisassemblerMode mode);

/* hsailDisassemblerDebug - C-accessible function for printing a message to
 *   debugs()
 * @param file  - The name of the file printing the debug message.
 * @param line  - The line number that printed the debug message.
 * @param s     - The message to print.
 */

void
hsailDisassemblerDebug(const char *file,
                       unsigned line,
                       const char *s);

#ifdef __cplusplus
}
#endif

#endif
