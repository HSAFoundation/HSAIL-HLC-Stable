//===---- HSAILMACHO_ObjectFile.cpp - HSAIL Macho Object File ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HSAIL32_MachoObjectFile and
// HSAIL64_MachoObjectFile classes.
//
//===----------------------------------------------------------------------===//

#include "HSAILMachoTargetObjectFile.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
using namespace llvm;

void
HSAIL32_MachoTargetObjectFile::Initialize(MCContext &ctx,
                                    const TargetMachine &TM)
{
  assert(!"When do we hit this?");
}

const MCSection*
HSAIL32_MachoTargetObjectFile::getEHFrameSection() const
{
  assert(!"When do we hit this?");
  return NULL;
}

const MCSection*
HSAIL32_MachoTargetObjectFile::SelectSectionForGlobal(const GlobalValue *GV,
                                                SectionKind Kind,
                                                Mangler *Mang,
                                                const TargetMachine &TM) const
{
  assert(!"When do we hit this?");
  return NULL;
}

const MCSection*
HSAIL32_MachoTargetObjectFile::getExplicitSectionGlobal(const GlobalValue *GV,
                                                  SectionKind Kind,
                                                  Mangler *Mang,
                                                  const TargetMachine &TM) const
{
  assert(!"When do we hit this?");
  return NULL;
}

const MCSection*
HSAIL32_MachoTargetObjectFile::getSectionForConstant(SectionKind Kind) const
{
  assert(!"When do we hit this?");
  return NULL;
}

bool
HSAIL32_MachoTargetObjectFile::shouldEmitUsedDirectiveFor(const GlobalValue *GV,
                                                    Mangler *) const
{
  assert(!"When do we hit this?");
  return false;
}

const MCExpr*
HSAIL32_MachoTargetObjectFile::getExprForDwarfGlobalReference(const GlobalValue *GV,
                                                        Mangler *Mang,
                                                        MachineModuleInfo *MMI,
                                                        unsigned Encoding,
                                                        MCStreamer &Streamer) const
{
  assert(!"When do we hit this?");
  return NULL;
}

unsigned
HSAIL32_MachoTargetObjectFile::getPersonalityEncoding() const
{
  assert(!"When do we hit this?");
  return 0;
}

unsigned
HSAIL32_MachoTargetObjectFile::getLSDAEncoding() const
{
  assert(!"When do we hit this?");
  return 0;
}

unsigned
HSAIL32_MachoTargetObjectFile::getFDEEncoding() const
{
  assert(!"When do we hit this?");
  return 0;
}

unsigned
HSAIL32_MachoTargetObjectFile::getTTypeEncoding() const
{
  assert(!"When do we hit this?");
  return 0;
}

void
HSAIL64_MachoTargetObjectFile::Initialize(MCContext &ctx,
                                    const TargetMachine &TM)
{
  assert(!"When do we hit this?");
}

const MCSection*
HSAIL64_MachoTargetObjectFile::getEHFrameSection() const
{
  assert(!"When do we hit this?");
  return NULL;
}

const MCSection*
HSAIL64_MachoTargetObjectFile::SelectSectionForGlobal(const GlobalValue *GV,
                                                SectionKind Kind,
                                                Mangler *Mang,
                                                const TargetMachine &TM) const
{
  assert(!"When do we hit this?");
  return NULL;
}

const MCSection*
HSAIL64_MachoTargetObjectFile::getExplicitSectionGlobal(const GlobalValue *GV,
                                                  SectionKind Kind,
                                                  Mangler *Mang,
                                                  const TargetMachine &TM) const
{
  assert(!"When do we hit this?");
  return NULL;
}

const MCSection*
HSAIL64_MachoTargetObjectFile::getSectionForConstant(SectionKind Kind) const
{
  assert(!"When do we hit this?");
  return NULL;
}

bool
HSAIL64_MachoTargetObjectFile::shouldEmitUsedDirectiveFor(const GlobalValue *GV,
                                                    Mangler *) const
{
  assert(!"When do we hit this?");
  return false;
}

const MCExpr*
HSAIL64_MachoTargetObjectFile::getExprForDwarfGlobalReference(const GlobalValue *GV,
                                                        Mangler *Mang,
                                                        MachineModuleInfo *MMI,
                                                        unsigned Encoding,
                                                        MCStreamer &Streamer) const
{
  assert(!"When do we hit this?");
  return NULL;
}

unsigned
HSAIL64_MachoTargetObjectFile::getPersonalityEncoding() const
{
  assert(!"When do we hit this?");
  return 0;
}

unsigned
HSAIL64_MachoTargetObjectFile::getLSDAEncoding() const
{
  assert(!"When do we hit this?");
  return 0;
}

unsigned
HSAIL64_MachoTargetObjectFile::getFDEEncoding() const
{
  assert(!"When do we hit this?");
  return 0;
}

unsigned
HSAIL64_MachoTargetObjectFile::getTTypeEncoding() const
{
  assert(!"When do we hit this?");
  return 0;
}
