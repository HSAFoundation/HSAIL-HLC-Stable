//===- HSAILDisassembler.h - Disassembler for HSAIL -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// TODO: Need to rewrite the paragraphs to explain the HSAIL disassembler.
//
//===----------------------------------------------------------------------===//

#ifndef HSAILDISASSEMBLER_H
#define HSAILDISASSEMBLER_H

#define INSTRUCTION_SPECIFIER_FIELDS  \
  const char*             name;

#define INSTRUCTION_IDS               \
  const InstrUID *instructionIDs;

#include "HSAILDisassemblerDecoderCommon.h"

#undef INSTRUCTION_SPECIFIER_FIELDS
#undef INSTRUCTION_IDS

#include "llvm/MC/MCDisassembler.h"

struct InternalInstruction;

namespace llvm {

class MCInst;
class MemoryObject;
class raw_ostream;

struct EDInstInfo;

namespace HSAILDisassembler {

/// HSAILGenericDisassembler - Generic disassembler for all HSAIL platforms.
///   All each platform class should have to do is subclass the constructor, and
///   provide a different disassemblerMode value.
class HSAILGenericDisassembler : public MCDisassembler {
protected:
  /// Constructor     - Initializes the disassembler.
  ///
  /// @param mode     - The HSAIL architecture mode to decode for.
  HSAILGenericDisassembler(DisassemblerMode mode);
public:
  ~HSAILGenericDisassembler();

  /// getInstruction - See MCDisassembler.
  bool
  getInstruction(MCInst &instr,
                 uint64_t &size,
                 const MemoryObject &region,
                 uint64_t address,
                 raw_ostream &vStream) const;

  /// getEDInfo - See MCDisassembler.
  EDInstInfo *getEDInfo() const;
private:
  DisassemblerMode              fMode;
};

/// HSAIL_16Disassembler - 16-bit HSAIL disassembler.
class HSAIL_16Disassembler : public HSAILGenericDisassembler {
public:
  HSAIL_16Disassembler() :
    HSAILGenericDisassembler(MODE_16BIT) {}
};

/// HSAIL_16Disassembler - 32-bit HSAIL disassembler.
class HSAIL_32Disassembler : public HSAILGenericDisassembler {
public:
  HSAIL_32Disassembler() :
    HSAILGenericDisassembler(MODE_32BIT) {}
};

/// HSAIL_16Disassembler - 64-bit HSAIL disassembler.
class HSAIL_64Disassembler : public HSAILGenericDisassembler {
public:
  HSAIL_64Disassembler() :
    HSAILGenericDisassembler(MODE_64BIT) {}
};

} // namespace HSAILDisassembler

} // namespace llvm

#endif
