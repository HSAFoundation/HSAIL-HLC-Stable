//===----- HSAILCOFFObjectFile.cpp - HSAIL COFF Object File -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "HSAILCOFFObjectFile.h"
using namespace llvm;

HSAIL_COFFObjectFile::HSAIL_COFFObjectFile(const HSAILTargetMachine &tm)
  : TM(tm) {}

/// Initialize - this method must be called before any actual lowering is
/// done.  This specifies the current context for codegen, and gives the
/// lowering implementations a chance to set up their default sections.
void
HSAIL_COFFObjectFile::Initialize(MCContext &ctx, const TargetMachine &TM)
{
  assert(!"When do we hit this?");
}

/// getExplicitSectionGlobal - Targets should implement this method to assign
/// a section to globals with an explicit section specfied.  The
/// implementation of this method can assume that GV->hasSection() is true.
const MCSection*
HSAIL_COFFObjectFile::getExplicitSectionGlobal(const GlobalValue *GV,
                                               SectionKind Kind,
                                               Mangler *Mang,
                                               const TargetMachine &TM) const
{
  assert(!"When do we hit this?");
  return NULL;
}

/// getSpecialCasedSectionGlobals - Allow the target to completely override
/// section assignment of a global.
const MCSection*
HSAIL_COFFObjectFile::getSpecialCasedSectionGlobals(const GlobalValue *GV,
                                                    Mangler *Mang,
                                                    SectionKind Kind) const
{
  assert(!"When do we hit this?");
  return NULL;
}

/// getExprForDwarfGlobalReference - Return an MCExpr to use for a reference
/// to the specified global variable from exception handling information.
///
const MCExpr*
HSAIL_COFFObjectFile::getExprForDwarfGlobalReference(const GlobalValue *GV,
                                                     Mangler *Mang,
                                                     MachineModuleInfo *MMI,
                                                     unsigned Encoding,
                                                     MCStreamer &Streamer) const
{
  assert(!"When do we hit this?");
  return NULL;
}

const MCExpr*
HSAIL_COFFObjectFile::getExprForDwarfReference(const MCSymbol *Sym,
                                               Mangler *Mang,
                                               MachineModuleInfo *MMI,
                                               unsigned Encoding,
                                               MCStreamer &Streamer) const
{
  assert(!"When do we hit this?");
  return NULL;
}

unsigned HSAIL_COFFObjectFile::getPersonalityEncoding() const
{
  assert(!"When do we hit this?");
  return 0;
}

unsigned HSAIL_COFFObjectFile::getLSDAEncoding() const
{
  assert(!"When do we hit this?");
  return 0;
}

unsigned HSAIL_COFFObjectFile::getFDEEncoding() const
{
  assert(!"When do we hit this?");
  return 0;
}

unsigned HSAIL_COFFObjectFile::getTTypeEncoding() const
{
  assert(!"When do we hit this?");
  return 0;
}

const MCSection*
HSAIL_COFFObjectFile::SelectSectionForGlobal(const GlobalValue *GV,
                                             SectionKind Kind,
                                             Mangler *Mang,
                                             const TargetMachine &TM) const
{
  assert(!"When do we hit this?");
  return NULL;
}
