//===-- HSAILMCInstLower.cpp - Convert HSAIL MachineInstr to an MCInst ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower HSAIL MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "InstPrinter/HSAILInstPrinter.h"
#include "HSAILMCInstLower.h"
#include "HSAILAsmPrinter.h"
#include "HSAILCOFFMachineModuleInfo.h"
#include "HSAILMCAsmInfo.h"
#include "llvm/CodeGen/MachineModuleInfoImpls.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Target/Mangler.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Type.h"
using namespace llvm;

HSAILMCInstLower::HSAILMCInstLower(Mangler *mang,
                                   const MachineFunction &mf,
                                   HSAILAsmPrinter &asmprinter)
  : Ctx(mf.getContext()), Mang(mang), MF(mf), TM(mf.getTarget()),
    MAI(*TM.getMCAsmInfo()), AsmPrinter(asmprinter) {}

MachineModuleInfoMachO&
HSAILMCInstLower::getMachOMMI() const
{
  assert(!"When do we hit this?");
  return MF.getMMI().getObjFileInfo<MachineModuleInfoMachO>();
}

/// GetSymbolFromOperand - Lower an MO_GlobalAddress or MO_ExternalSymbol
/// operand to an MCSymbol.
MCSymbol*
HSAILMCInstLower::GetSymbolFromOperand(const MachineOperand &MO) const
{
  assert(!"When do we hit this?");
  return NULL;
}

MCOperand
HSAILMCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                     MCSymbol *Sym) const
{
  assert(!"When do we hit this?");
  return MCOperand();
}

void
HSAILMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const
{
  assert(!"When do we hit this?");
}
