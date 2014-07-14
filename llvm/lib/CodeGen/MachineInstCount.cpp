//===-- MachineInstrCount.cpp - Collects the count of all instructions ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass collects the count of all machine instructions and reports them.
// This pass can also collect the count of sequences of instructions recursivelly
// and report the number of common sequences.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "machineinstcount"
#include <stdio.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Target/TargetInstrInfo.h"

#include <map>
using namespace llvm;

STATISTIC(TotalInsts , "Number of instructions (of all types)");
STATISTIC(TotalBlocks, "Number of basic blocks");
static cl::opt<bool>
RunOpt("count-histogram",
    cl::desc("Enable the instruction histogram counting"),
    cl::init(false));

static cl::opt<std::string>
DataFile("histogram-file",
    cl::desc("File that contains histogram information"),
    cl::init("inst_histogram_amdil.txt"),
    cl::Hidden);

static cl::opt<unsigned>
DepthCount("histogram-depth",
    cl::desc("The recursive depth of instructions to consider."),
    cl::init(0),
    cl::Hidden);

static cl::opt<unsigned>
CutoffCount("histogram-cutoff",
    cl::desc("Determine the number of hits required before an instruction is no longer dropped."),
    cl::init(0),
    cl::Hidden);

static cl::opt<unsigned>
OperandMinimum("histogram-op-min",
    cl::desc("Only process instructions that have at least min operands."),
    cl::init(0),
    cl::Hidden);

static cl::opt<unsigned>
OperandMaximum("histogram-op-max",
    cl::desc("Only process instructions that have at less than max operands."),
    cl::init(9),
    cl::Hidden);


namespace {
  class MachineInstrCount : public MachineFunctionPass {
  private:
    StringMap<unsigned> Opcodes; // Stringmap that holds the opcode count.
    std::map<unsigned, const MachineInstr *> regDefMap; // Holds the instruction that
                                                        // defines a register. This might
                                                        // have some problems with loops, but
                                                        // is sufficient for the task at hand.
    const TargetMachine *TM;

    std::string getOpname(const MachineInstr *I, unsigned level, unsigned &levels);
  public:
    static char ID; // Pass identification, replacement for typeid
    MachineInstrCount() : MachineFunctionPass(ID) {
      initializeMachineInstrCountPass(*PassRegistry::getPassRegistry());
      if (RunOpt) {
        OwningPtr<MemoryBuffer> File;
        if (error_code EC = MemoryBuffer::getFile(DataFile.c_str(), File)) {
        } else {
          MemoryBuffer *Buff = File.take();
          const char *Data = Buff->getBufferStart();
          size_t DataLen = Buff->getBufferSize();
          SmallVector<StringRef, 16> Lines;
          SplitString(StringRef(Data, DataLen), Lines, "\n\r");
          for (size_t i = 0, numLines = Lines.size(); i < numLines; i++) {
            SmallVector<StringRef, 2> Line;
            unsigned count;
            std::string opcode;
            SplitString(Lines[i], Line, ":");
            Line[1].getAsInteger(10, count);
            Opcodes[Line[0]] = count;
          }
        }

      }
    }
    virtual ~MachineInstrCount() {
      if (RunOpt) {
        FILE *fp = ::fopen(DataFile.c_str(), "w");
        if (fp) {
          char buffer[2048];
          for (StringMap<unsigned>::iterator opb = Opcodes.begin(),
            ope = Opcodes.end(); opb != ope; ++opb) {
            // Only emit the instructions that have more hits than the cutoff count.
            if (opb->getValue() > CutoffCount) {
              memset(buffer, 0, sizeof(buffer));
              ::sprintf(buffer,"%s:%d\n", opb->getKey().data(), opb->getValue());
              ::fwrite(buffer, strlen(buffer), 1, fp);
            }
          }
          ::fclose(fp);
        }
      }
    }

    virtual bool runOnMachineFunction(MachineFunction &F);

  };
}

char MachineInstrCount::ID = 0;
INITIALIZE_PASS(MachineInstrCount, "machineinstcount",
    "Counts the various types of Machine Instructions", false, true)
namespace llvm {
llvm::FunctionPass *createMachineInstrCountPass() {
  return new MachineInstrCount();
}
}

std::string MachineInstrCount::getOpname(const MachineInstr *I, unsigned level, unsigned &total)
{
  std::string name = TM->getInstrInfo()->getName(I->getOpcode());
  if (!level) {
    return name;
  }
  total |= (1 << level);
  bool skip = false;
  std::string subname = "";
  for (unsigned x = I->getNumOperands(), y = 0; x > y; --x) {
    if (x != 1 && x != I->getNumOperands() && !skip) subname = ", " + subname;
    const MachineOperand *MIB = &I->getOperand(x - 1);
    skip = false;
    switch (MIB->getType()) {
      case MachineOperand::MO_Register:
        if (MIB->isDef()) {
          regDefMap[MIB->getReg()] = I;
          skip = true;
        } else {
          const MachineInstr *MI = regDefMap[MIB->getReg()];
          if (MI != I) {
            if (MI) {
              // TODO: Should this check to see if we are referencing something
              // that is already part of the instruction?
              // For example:
              // ineg r3, r2
              // iadd r3, r3, r3
              // Shows up as:
              // IADD( INEG, INEG)
              // but could possible show up as:
              // IADD( INEG, SRC1)
              subname = getOpname(MI, level - 1, total) + subname;
            } else {
              subname = "livein" + subname;// + utostr(MIB->getReg()) + subname;
            }
          } else {
            subname = "reg" + subname;// + utostr(MIB->getReg()) + subname;
          }
        }
        break;
      default:                                    subname = "unk" + subname;   break;
      case MachineOperand::MO_Immediate:          subname = "imm" + subname;   break;
      case MachineOperand::MO_CImmediate:         subname = "cimm" + subname;  break;
      case MachineOperand::MO_FPImmediate:        subname = "fpimm" + subname; break;
      case MachineOperand::MO_MachineBasicBlock:  subname = "MBB" + subname;   break;
      case MachineOperand::MO_FrameIndex:         subname = "FI" + subname;    break;
      case MachineOperand::MO_ConstantPoolIndex:  subname = "CPI" + subname;   break;
      case MachineOperand::MO_JumpTableIndex:     subname = "JTI" + subname;   break;
      case MachineOperand::MO_ExternalSymbol:     subname = "ES" + subname;    break;
      case MachineOperand::MO_GlobalAddress:      subname = "GA" + subname;    break;
      case MachineOperand::MO_BlockAddress:       subname = "BA" + subname;    break;
      case MachineOperand::MO_RegisterMask:       subname = "RM" + subname;    break;
      case MachineOperand::MO_Metadata:           subname = "MD" + subname;    break;
      case MachineOperand::MO_MCSymbol:           subname = "SYM" + subname;   break;
    }
  }
  if (!subname.empty()) {
    name = name + "( " + subname + " )";
  }
  return name;
}
// MachineInstrCount::run - This is the main Analysis entry point for a
// function.
//
bool MachineInstrCount::runOnMachineFunction(MachineFunction &MF) {
  // If RunOpt is set, then this pass calculates the count of each type of machine instructions
  // that is generated and stores it in a file.
  if (RunOpt) {
    TM = &MF.getTarget();
    regDefMap.clear();
    for (MachineFunction::iterator MFB = MF.begin(), MFE = MF.end();
      MFB != MFE; ++MFB) {
      TotalBlocks++;
      for (MachineBasicBlock::iterator MBB = MFB->begin(), MBE = MFB->end();
        MBB != MBE; ++MBB) {
        TotalInsts++;
        // This skips all operations with operands less than the minimum
        // and more than the maximum.
        if (MBB->getNumOperands() >= OperandMinimum
            || MBB->getNumOperands() <= OperandMaximum) {
          unsigned total = 0;
          std::string OpName = getOpname(MBB, DepthCount, total);
          unsigned c;
          for (c = 0; total; total >>= 1)
          {
            c += total & 1;
          }
          total = c;
          if (total < DepthCount) continue;
          Opcodes[OpName]++;
        }
      }
    }
  }
  return false;
}
