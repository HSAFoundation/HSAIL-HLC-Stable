//===-- HSAILMCAsmInfo.h - HSAIL asm properties -----------------*- C++ -*--==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the HSAILMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef _HSAIL_MC_ASM_INFO_H_
#define _HSAIL_MC_ASM_INFO_H_

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCAsmInfoCOFF.h"
#include "llvm/MC/MCAsmInfoDarwin.h"

namespace llvm {
  class Triple;

  struct HSAILMCAsmInfoDarwin : public MCAsmInfoDarwin {
    explicit HSAILMCAsmInfoDarwin(const Triple &Triple);
  };

  struct HSAILELFMCAsmInfo : public MCAsmInfo {
    explicit HSAILELFMCAsmInfo(const Triple &Triple, bool is64Bit);
    virtual const MCSection *getNonexecutableStackSection(MCContext &Ctx) const;
  };

  struct HSAILMCAsmInfoCOFF : public MCAsmInfoCOFF {
    explicit HSAILMCAsmInfoCOFF(const Triple &Triple);
  };
} // namespace llvm

#endif // _HSAIL_MC_ASM_INFO_H_
