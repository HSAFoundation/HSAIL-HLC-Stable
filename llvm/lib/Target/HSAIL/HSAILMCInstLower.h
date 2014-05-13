//===-- HSAILMCInstLower.h - Lower MachineInstr to MCInst -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _HSAIL_MC_INST_LOWER_H_
#define _HSAIL_MC_INST_LOWER_H_

#include "llvm/Support/Compiler.h"

namespace llvm {
  class MCAsmInfo;
  class MCContext;
  class MCInst;
  class MCOperand;
  class MCSymbol;
  class MachineInstr;
  class MachineFunction;
  class MachineModuleInfoMachO;
  class MachineOperand;
  class Mangler;
  class TargetMachine;
  class HSAILAsmPrinter;

/// HSAILMCInstLower - This class is used to lower an MachineInstr into an
/// MCInst.
class LLVM_LIBRARY_VISIBILITY HSAILMCInstLower {
  MCContext &Ctx;
  Mangler *Mang;
  const MachineFunction &MF;
  const TargetMachine &TM;
  const MCAsmInfo &MAI;
  HSAILAsmPrinter &AsmPrinter;
public:
  HSAILMCInstLower(Mangler *mang,
                   const MachineFunction &MF,
                   HSAILAsmPrinter &asmprinter);

  void
    Lower(const MachineInstr *MI, MCInst &OutMI) const;

  MCSymbol*
    GetSymbolFromOperand(const MachineOperand &MO) const;

  MCOperand
    LowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym) const;

private:
  MachineModuleInfoMachO &getMachOMMI() const;
};

}

#endif // _HSAIL_MC_INST_LOWER_H_
