//===-------- AMDILAsmPrinter.h --- AMDIL Asm Printer class ----*- C++ -*--===//
// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// If you use the software (in whole or in part), you shall adhere to all
// applicable U.S., European, and other export laws, including but not limited
// to the U.S. Export Administration Regulations (EAR), (15 C.F.R. Sections
// 730 through 774), and E.U. Council Regulation (EC) No 1334/2000 of 22 June
// 2000.  Further, pursuant to Section 740.6 of the EAR, you hereby certify
// that, except pursuant to a license granted by the United States Department
// of Commerce Bureau of Industry and Security or as otherwise permitted
// pursuant to a License Exception under the U.S. Export Administration
// Regulations ("EAR"), you will not (1) export, re-export or release to a
// national of a country in Country Groups D:1, E:1 or E:2 any restricted
// technology, software, or source code you receive hereunder, or (2) export to
// Country Groups D:1, E:1 or E:2 the direct product of such technology or
// software, if such foreign produced direct product is subject to national
// security controls as identified on the Commerce Control List (currently
// found in Supplement 1 to Part 774 of EAR).  For the most current Country
// Group listings, or for additional information about the EAR or your
// obligations under those regulations, please refer to the U.S. Bureau of
// Industry and Securitys website at http://www.bis.doc.gov/.
//
//==-----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//
#ifndef _AMDIL_ASM_PRINTER_H_
#define _AMDIL_ASM_PRINTER_H_
#include "AMDIL.h"
#include "macrodata.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/FormattedStream.h"


namespace llvm
{
    class AMDILKernelManager;
    class AMDILTargetMachine;
    class AMDILMachineFunctionInfo;
    class AMDILModuleInfo;
    class AnalysisUsage;
    class Constant;
    class Function;
    class Module;
    class MachineInstr;
    class MachineBasicBlock;
    class MachineConstantPoolValue;
    class MachineFunction;
    class MachineJumptableInfo;
    class raw_ostream;
    class MCStreamer;
    class MCSymbol;
    class MCInst;
    class MCContext;


  class LLVM_LIBRARY_VISIBILITY AMDILAsmPrinter : public AsmPrinter {
    public:
      //
      // Constructor for the AMDIL specific AsmPrinter class.
      // Interface is defined by LLVM proper and should reference
      // there for more information.
      //
      explicit AMDILAsmPrinter(TargetMachine& TM, MCStreamer &Streamer);

      //
      // Destructor for the AsmPrinter class that deletes all the
      // allocated memory
      //
      virtual ~AMDILAsmPrinter();

      //
      // @param MI Machine instruction to print the operand of
      // @param opNum operand to print from the specified machine instruciton
      // @param O The output stream for the operand
      // @brief Based on the register type, print out register specific
      // information
      // and add swizzle information in the cases that require it
      //
      void printOperand(const MachineInstr *MI,
                        int OpNum,
                        raw_ostream &O);

      virtual void EmitGlobalVariable(const GlobalVariable *GV) LLVM_OVERRIDE;
      virtual void EmitStartOfAsmFile(Module &M) LLVM_OVERRIDE;
      virtual void EmitEndOfAsmFile(Module &M) LLVM_OVERRIDE;

      virtual void EmitInstruction(const MachineInstr *MI) LLVM_OVERRIDE;
      virtual void EmitFunctionBodyStart() LLVM_OVERRIDE;
      virtual void EmitFunctionBodyEnd() LLVM_OVERRIDE;
      virtual void EmitConstantPool() LLVM_OVERRIDE;
      virtual void EmitFunctionEntryLabel() LLVM_OVERRIDE;

      bool getDebugResourceID(const Value *V, uint32_t& RID) const;
      unsigned correctDebugAS(unsigned AS, const Value *V) const;

      //
      // @param MI Machine instruction to print memory operand of
      // @param opNum operand to print from the specified machine instruction
      // @param Modifier optional modifier for the memory operand
      // @brief Print the memory operand based on the register type
      //
      void printMemOperand(const MachineInstr *MI,
                           int OpNum,
                           raw_ostream &O,
                           const char *Modifier = NULL);

      //
      // @param MI Machine instruction to print to the buffer
      // @brief autogenerated function from tablegen files that prints out
      // the assembly format of the specified instruction
      //
      void printInstruction(const MachineInstr *MI , raw_ostream &O); // autogenerated

      const char *getRegisterName(unsigned RegNo);

      //
      // @param F MachineFunction to print the assembly for
      // @brief parse the specified machine function and print
      // out the assembly for all the instructions in the function
      //
      bool runOnMachineFunction(MachineFunction &F);

      //
      // @param MI Machine Instruction to determine if it a macro call
      // @brief Query to see if the instruction is a Macro or not
      // @return true if instruction is a macro
      //
      bool isMacroCall(const MachineInstr *MI);

      //
      // @param MI Machine Instruction to determine if the fucntion is a macro
      // @brief determine if the function is a macro function or a normal
      // function
      // @return true if the function call should be transformed to a macro,
      // false otherwise
      //
      bool isMacroFunc(const MachineInstr *MI);


      //
      // @param MI Machine instruction to print swizzle for
      // @param opNum the operand number to print swizzle for
      // @brief print out the swizzle for a scalar register class
      //
      const char *getSwizzle(const MachineInstr *MI, int OpNum);

      //
      // @return the name of this specific pass
      //
      virtual const char *getPassName() const LLVM_OVERRIDE;

      /// EmitDwarfRegOp - Emit dwarf register operation
      virtual void EmitDwarfRegOp(const MachineLocation &MLoc) const LLVM_OVERRIDE;

    protected:
      //
      // @param MI Machine instruction to emit the macro code for
      //
      // Emits a fully functional macro function that uses the argument
      // registers as the macro arguments.
      //
      void emitMacroFunc(const MachineInstr *MI,
                         raw_ostream &O);

      void printCopy(const MachineInstr *MI, raw_ostream &O);
      //
      //
      void emitMCallInst(const MachineInstr *MI,
                         raw_ostream &O,
                         StringRef Name);

      //
      // @param name
      // @brief strips KERNEL_PREFIX and KERNEL_SUFFIX from the name
      // and returns that name if both of the tokens are present.
      //
      StringRef stripKernelPrefix(StringRef Name);

      // Set of all macros that are used in this compilation unit.
      llvm::DenseSet<uint32_t> mMacroIDs;

      /// Pointer to the Target Machine that the asm printer
      /// should be printing compatible code for.
      AMDILTargetMachine *mTM;

      /// pointer to the kernel manager that keeps track
      /// of the metadata required by the runtime to
      /// call a kernel correctly.
      AMDILKernelManager *mMeta;

      /// Class that holds information about the current
      /// function that is being processed.
      AMDILMachineFunctionInfo *mMFI;

      /// Class that holds information about the current
      /// module that is being processed.
      AMDILModuleInfo *mAMI;

      /// Name of the current function being printed
      /// by the asm printer
      std::string mName;

      /// name of the kernel wrapper of the current function
      std::string mKernelName;

      // Flag whether to print out debug information
      // or not.
      bool mDebugMode;

    private:
      void addCPoolLiteral(const Constant *C);
      void addGlobalConstantArrayLiterals();
      void EmitCallRegOps(const MachineInstr *MI,
          unsigned OpIdxBegin, unsigned OpIdxEnd, bool IsDef, raw_ostream &O);

      /// The constant buffer that the data should be
      /// allocated in by the runtime
      int mBuffer;
  };


} // end of llvm namespace

#endif // _AMDIL_ASM_PRINTER_H_
