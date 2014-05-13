//===----- HSAILCOFFObjectFile.h - HSAIL COFF Object File -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _HSAIL_OBJECT_FILE_H_
#define _HSAIL_OBJECT_FILE_H_

#include "HSAILTargetMachine.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetLoweringObjectFile.h"

namespace llvm {
class HSAILMachine;

class HSAIL_COFFObjectFile : public TargetLoweringObjectFileCOFF {
  const HSAILTargetMachine &TM;
public:
  HSAIL_COFFObjectFile(const HSAILTargetMachine &tm);

  /// Initialize - this method must be called before any actual lowering is
  /// done.  This specifies the current context for codegen, and gives the
  /// lowering implementations a chance to set up their default sections.
  virtual void
    Initialize(MCContext &ctx, const TargetMachine &TM);

  /// getExplicitSectionGlobal - Targets should implement this method to assign
  /// a section to globals with an explicit section specfied.  The
  /// implementation of this method can assume that GV->hasSection() is true.
  virtual const MCSection*
    getExplicitSectionGlobal(const GlobalValue *GV,
                             SectionKind Kind,
                             Mangler *Mang,
                             const TargetMachine &TM) const;

  /// getSpecialCasedSectionGlobals - Allow the target to completely override
  /// section assignment of a global.
  virtual const MCSection*
    getSpecialCasedSectionGlobals(const GlobalValue *GV,
                                  Mangler *Mang,
                                  SectionKind Kind) const;

  /// getExprForDwarfGlobalReference - Return an MCExpr to use for a reference
  /// to the specified global variable from exception handling information.
  virtual const MCExpr*
    getExprForDwarfGlobalReference(const GlobalValue *GV,
                                   Mangler *Mang,
                                   MachineModuleInfo *MMI,
                                   unsigned Encoding,
                                   MCStreamer &Streamer) const;

  const MCExpr *
    getExprForDwarfReference(const MCSymbol *Sym,
                             Mangler *Mang,
                             MachineModuleInfo *MMI,
                             unsigned Encoding,
                             MCStreamer &Streamer) const;

  virtual unsigned getPersonalityEncoding() const;

  virtual unsigned getLSDAEncoding() const;

  virtual unsigned getFDEEncoding() const;

  virtual unsigned getTTypeEncoding() const;

  virtual const MCSection*
    SelectSectionForGlobal(const GlobalValue *GV,
                           SectionKind Kind,
                           Mangler *Mang,
                           const TargetMachine &TM) const;
};
} // end namespace llvm

#endif // _HSAIL_COFF_OBJECT_FILE_H_
