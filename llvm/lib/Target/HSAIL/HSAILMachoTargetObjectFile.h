//===---- HSAILMACHO_ObjectFile.h - HSAIL Macho Object File -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _HSAIL_MACHO_OBJECT_FILE_H_
#define _HSAIL_MACHO_OBJECT_FILE_H_

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetLoweringObjectFile.h"

namespace llvm {
class HSAILMachine;

/// HSAIL32_MachoTargetObjectFile - This TLOF implementation is used for Darwin
/// x86.
class HSAIL32_MachoTargetObjectFile : public TargetLoweringObjectFileMachO {
public:
  /// Initialize - this method must be called before any actual lowering is
  /// done.  This specifies the current context for codegen, and gives the
  /// lowering implementations a chance to set up their default sections.
  virtual void
  Initialize(MCContext &ctx, const TargetMachine &TM);

  virtual const MCSection*
  getEHFrameSection() const;

  virtual const MCSection*
  SelectSectionForGlobal(const GlobalValue *GV,
                         SectionKind Kind,
                         Mangler *Mang,
                         const TargetMachine &TM) const;

  virtual const MCSection*
  getExplicitSectionGlobal(const GlobalValue *GV,
                           SectionKind Kind,
                           Mangler *Mang,
                           const TargetMachine &TM) const;

  virtual const MCSection*
  getSectionForConstant(SectionKind Kind) const;

  /// shouldEmitUsedDirectiveFor - This hook allows targets to selectively
  /// decide not to emit the UsedDirective for some symbols in llvm.used.
  /// FIXME: REMOVE this (rdar://7071300)
  virtual bool
  shouldEmitUsedDirectiveFor(const GlobalValue *GV,
                             Mangler *) const;

  /// getExprForDwarfGlobalReference - The mach-o version of this method
  /// defaults to returning a stub reference.
  virtual const MCExpr*
  getExprForDwarfGlobalReference(const GlobalValue *GV,
                                 Mangler *Mang,
                                 MachineModuleInfo *MMI,
                                 unsigned Encoding,
                                 MCStreamer &Streamer) const;

  virtual unsigned
  getPersonalityEncoding() const;

  virtual unsigned
  getLSDAEncoding() const;

  virtual unsigned
  getFDEEncoding() const;

  virtual unsigned
  getTTypeEncoding() const;
};

/// HSAIL64_MachoTargetObjectFile - This TLOF implementation is used for Darwin
/// x86-64.
class HSAIL64_MachoTargetObjectFile : public TargetLoweringObjectFileMachO {
public:

  /// Initialize - this method must be called before any actual lowering is
  /// done.  This specifies the current context for codegen, and gives the
  /// lowering implementations a chance to set up their default sections.
  virtual void
  Initialize(MCContext &ctx, const TargetMachine &TM);

  virtual const MCSection*
  getEHFrameSection() const;

  virtual const MCSection*
  SelectSectionForGlobal(const GlobalValue *GV,
                         SectionKind Kind,
                         Mangler *Mang,
                         const TargetMachine &TM) const;

  virtual const MCSection*
  getExplicitSectionGlobal(const GlobalValue *GV,
                           SectionKind Kind,
                           Mangler *Mang,
                           const TargetMachine &TM) const;

  virtual const MCSection*
  getSectionForConstant(SectionKind Kind) const;

  /// shouldEmitUsedDirectiveFor - This hook allows targets to selectively
  /// decide not to emit the UsedDirective for some symbols in llvm.used.
  /// FIXME: REMOVE this (rdar://7071300)
  virtual bool
  shouldEmitUsedDirectiveFor(const GlobalValue *GV,
                             Mangler *) const;

  /// getExprForDwarfGlobalReference - The mach-o version of this method
  /// defaults to returning a stub reference.
  virtual const MCExpr*
  getExprForDwarfGlobalReference(const GlobalValue *GV,
                                 Mangler *Mang,
                                 MachineModuleInfo *MMI,
                                 unsigned Encoding,
                                 MCStreamer &Streamer) const;

  virtual unsigned
  getPersonalityEncoding() const;

  virtual unsigned
  getLSDAEncoding() const;

  virtual unsigned
  getFDEEncoding() const;

  virtual unsigned
  getTTypeEncoding() const;
};

} // end namespace llvm

#endif // _HSAIL_MACHO_OBJECT_FILE_H_
