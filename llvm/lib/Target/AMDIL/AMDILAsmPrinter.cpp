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
#define DEBUG_TYPE "asm-printer"
#include "AMDILAsmPrinter.h"
#include "AMDILAlgorithms.tpp"
#include "AMDILCompilerErrors.h"
#include "AMDILKernelManager.h"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILModuleInfo.h"
#include "AMDILUtilityFunctions.h"
#include "llvm/Constants.h"
#include "llvm/Metadata.h"
#include "llvm/Type.h"
#include "llvm/DebugInfo.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/DebugLoc.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/TargetRegistry.h"
#include "../../Transforms/IPO/AMDSymbolName.h"

using namespace llvm;

#if 0
// FIXME: Create the wrapper function for kernels before lowering so that
// setup of kernel arguments are done during lowering instead of being done
// during AsmPrinter pass.
extern unsigned getActualArgReg(unsigned FormalReg);
#endif
extern void printRegName(AMDILAsmPrinter *RegNames,
                  unsigned Reg,
                  raw_ostream &O,
                  bool Dst,
                  bool Dupe = false);

/// createAMDILCodePrinterPass - Returns a pass that prints the AMDIL
/// assembly code for a MachineFunction to the given output stream,
/// using the given target machine description. This should work
/// regardless of whether the function is in SSA form.
///

ASMPRINTER_RETURN_TYPE createAMDILCodePrinterPass(TargetMachine& TM, MCStreamer &Streamer) {
  return new AMDILAsmPrinter(TM, Streamer);
}

#include "AMDILGenAsmWriter.inc"
// Force static initialization
extern "C" void LLVMInitializeAMDILAsmPrinter() {
  llvm::TargetRegistry::RegisterAsmPrinter(TheAMDILTarget,
                                           createAMDILCodePrinterPass);
  llvm::TargetRegistry::RegisterAsmPrinter(TheAMDIL64Target,
                                           createAMDILCodePrinterPass);
}

AMDILInstPrinter *llvm::createAMDILInstPrinter(const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  return new AMDILInstPrinter(MAI, MII, MRI);
}

StringRef AMDILAsmPrinter::stripKernelPrefix(StringRef Name) {
  StringRef Prefix("__OpenCL_");
  StringRef Suffix("_kernel");
  size_t Start = Name.find(Prefix);
  size_t End = Name.rfind(Suffix);
  if (Start == StringRef::npos ||
      End == StringRef::npos ||
      Start == End) {
    return Name;
  }

  return Name.drop_front(Prefix.size())
             .drop_back(Suffix.size());
}

// TODO: Add support for verbose.
AMDILAsmPrinter::AMDILAsmPrinter(TargetMachine &TM, MCStreamer &Streamer)
  : AsmPrinter(TM, Streamer),
    mMacroIDs(),
    mTM(reinterpret_cast<AMDILTargetMachine *>(&TM)),
    mMeta(NULL),
    mMFI(NULL),
    mAMI(NULL),
    mName(),
    mKernelName(),
    mDebugMode(false),
    mBuffer(0) {
//  DEBUG(mDebugMode = true);
}

AMDILAsmPrinter::~AMDILAsmPrinter() {

}

const char *AMDILAsmPrinter::getPassName() const {
  return "AMDIL Assembly Printer";
}

// Emit the list of vector registers as arguments or return values
// for a call site.
// MI - the call instruction
// OpIdxBegin, OpIdxEnd - the range of MI's operand used for the call's
//                        arguments or returns
// IsDef - true if this is called for emitting returns for the call
//         false if this is called for emitting arguments for the call
// O - the output stream
void AMDILAsmPrinter::EmitCallRegOps(const MachineInstr *MI,
    unsigned OpIdxBegin, unsigned OpIdxEnd, bool IsDef, raw_ostream &O) {
  const TargetRegisterInfo *TRI = mTM->getRegisterInfo();
  // first and last vector register used for input/output at a call site
  unsigned FirstReg = AMDIL::R1;
  unsigned LastReg = AMDIL::R255;
  // First find out the list of vector registers (e.g. r1) used by the
  // list of operands
  std::set<unsigned> VectorRegs;
  for (unsigned OpIdx = OpIdxBegin; OpIdx < OpIdxEnd; ++OpIdx) {
    const MachineOperand & MO = MI->getOperand(OpIdx);
    if (MO.getType() != MachineOperand::MO_Register)
      continue;
    unsigned Reg = MO.getReg();
    assert((signed)Reg > 0 && "register not allocated");
    unsigned VectorReg = getVectorReg(Reg, TRI);
    if (VectorReg < FirstReg || VectorReg > LastReg ||
      TRI->getRegClass(AMDIL::ReservedTempRegClassID)->contains(Reg) ||
      TRI->getRegClass(AMDIL::ReservedReadwriteRegClassID)->contains(Reg)) {
      continue;
    }
    assert(((IsDef && MO.isImplicit() && MO.isDef()) || (!IsDef && MO.isUse()))
           && "bad operand");
    VectorRegs.insert(VectorReg);
  }
  // Next emit the list of vector registers
  std::set<unsigned>::iterator F = VectorRegs.begin();
  std::set<unsigned>::iterator E = VectorRegs.end();
  for (std::set<unsigned>::iterator I = F; I != E; ++I) {
    if (I != F) {
      O << ", ";
    }
    O << getRegisterName(*I);
  }
}

void AMDILAsmPrinter::EmitInstruction(const MachineInstr *II) {
  std::string FunStr;
  raw_string_ostream OFunStr(FunStr);
  formatted_raw_ostream O(OFunStr);
  const AMDILSubtarget *curTarget = mTM->getSubtargetImpl();
  if (mDebugMode) {
    O << ";" ;
    II->print(O);
  }
  DEBUG(dbgs() << *II << '\n');

  if (isMacroFunc(II)) {
    emitMacroFunc(II, O);
    O.flush();
    OutStreamer.EmitRawText(StringRef(FunStr));
    return;
  }

  if (isMacroCall(II)) {
    OpSwizzle OpSwiz, OldSwiz;
    const char *Name = mTM->getInstrInfo()->getName(II->getOpcode());
    assert(strlen(Name) >= 5);
    Name += 5;

    int MacroNum = amd::MacroDBFindMacro(Name);
    O << "\t;" << Name << '\n';
    O << "\tmcall(" << MacroNum << ")";
    unsigned Reg = II->getOperand(0).getReg();
    unsigned NewDst = AMDIL::R1000;
    OldSwiz.u8all = OpSwiz.u8all = mMFI->getMOSwizzle(II->getOperand(0));
    if (isXComponentReg(Reg)) {
      NewDst = AMDIL::Rx1000;
      OpSwiz.bits.swizzle = AMDIL_DST_X___;
    } else if (isYComponentReg(Reg)) {
      NewDst = AMDIL::Ry1000;
      OpSwiz.bits.swizzle = AMDIL_DST_X___;
    } else if (isZComponentReg(Reg)) {
      NewDst = AMDIL::Rz1000;
      OpSwiz.bits.swizzle = AMDIL_DST_X___;
    } else if (isWComponentReg(Reg)) {
      NewDst = AMDIL::Rw1000;
      OpSwiz.bits.swizzle = AMDIL_DST_X___;
    } else if (isXYComponentReg(Reg)) {
      NewDst = AMDIL::Rxy1000;
      OpSwiz.bits.swizzle = AMDIL_DST_XY__;
    } else if (isZWComponentReg(Reg)) {
      NewDst = AMDIL::Rzw1000;
      OpSwiz.bits.swizzle = AMDIL_DST_XY__;
    } else {
      OpSwiz.bits.swizzle = AMDIL_DST_DFLT;
    }
    for (unsigned I = 0, N = II->getNumOperands(); I < N; ++I) {
      if (I == 0) {
        O << "(";
        O << getRegisterName(NewDst);
        O << getDstSwizzle(OpSwiz.bits.swizzle);
      } else {
        printOperand(II, I, O);
      }
      if (I == 0) {
        O << "), (";
      } else if (I != N - 1) {
        O << ", ";
      } else {
        O << ")\n";
      }
    }

    O << "\tmov " << getRegisterName(Reg) << getDstSwizzle(OldSwiz.bits.swizzle)
      << ", " << getRegisterName(NewDst);

    if (isXComponentReg(Reg)) {
      O << getSrcSwizzle(AMDIL_SRC_X000);
    } else if (isYComponentReg(Reg)) {
      O << getSrcSwizzle(AMDIL_SRC_0X00);
    } else if (isZComponentReg(Reg)) {
      O << getSrcSwizzle(AMDIL_SRC_00X0);
    } else if (isWComponentReg(Reg)) {
      O << getSrcSwizzle(AMDIL_SRC_000X);
    } else if (isXYComponentReg(Reg)) {
      O << getSrcSwizzle(AMDIL_SRC_XY00);
    } else if (isZWComponentReg(Reg)) {
      O << getSrcSwizzle(AMDIL_SRC_00XY);
    } else {
      O << getSrcSwizzle(AMDIL_SRC_DFLT);
    }

    O << '\n';
    if (curTarget->isSupported(AMDIL::Caps::MacroDB)) {
      mMacroIDs.insert(MacroNum);
    } else {
      mMFI->addCalledIntr(MacroNum);
    }
  } else if (mMeta->useCompilerWrite(II) &&
             (curTarget->getGeneration() == AMDIL::EVERGREEN)) {
    // TODO: This is a hack to get around some
    // conformance failures.
    O << "\tif_logicalz cb0[0].x\n";
    if (mMFI->usesMem(AMDIL::RAW_UAV_ID)) {
      O << "\tuav_raw_store_id("
        << curTarget->getResourceID(AMDIL::RAW_UAV_ID)
        << ") ";
      O << "mem0.x___, cb0[3].x, r0.0\n";
    } else {
      O << "\tuav_arena_store_id("
        << curTarget->getResourceID(AMDIL::ARENA_UAV_ID)
        << ")_size(dword) ";
      O << "cb0[3].x, r0.0\n";
    }
    O << "\tendif\n";
    mMFI->addMetadata(";memory:compilerwrite");
  } else if (II->getOpcode() == AMDIL::COPY) {
    printCopy(II, O);
  } else if (II->getOpcode() == AMDIL::CALL) {
    // Op 0 is the callee
    const MachineOperand &MO = II->getOperand(0);
    assert(MO.getType() == MachineOperand::MO_GlobalAddress &&
           "unexpected callee oprand of call inst");
    const GlobalValue *GV = MO.getGlobal();
    // when calling another kernel, don't call the wrapper, call the
    // kernel function directly
    StringRef Name = GV->getName();
    if (AMDSymbolNames::isKernelFunctionName(Name))
      Name = AMDSymbolNames::undecorateKernelFunctionName(Name);
    uint32_t FuncNum = Name.empty()
                     ? mAMI->getOrCreateFunctionID(GV)
                     : mAMI->getOrCreateFunctionID(Name);
    mMFI->addCalledFunc(FuncNum);
    if (curTarget->isSupported(AMDIL::Caps::UseMacroForCall)) {
      O << "\tmcall(" << FuncNum << ") (";

      // search for the RegMask Op
      unsigned RegMaskOpIdx = 1;
      unsigned NumOps = II->getNumOperands();
      for (; RegMaskOpIdx < NumOps; ++RegMaskOpIdx) {
        if (II->getOperand(RegMaskOpIdx).isRegMask()) {
          break;
        }
      }

      // Print the returns from the callee.
      // Return registers are the general register ops following the RegMask
      EmitCallRegOps(II, RegMaskOpIdx + 1, NumOps, true/*Def*/, O);
      O << "), (";
      // Print the inputs into the callee.
      // The input args are the ones following the callee arg until the RegMask.
      EmitCallRegOps(II, 1, RegMaskOpIdx, false/*Use*/, O);
      O << ") ; "<< Name;
  } else {
      O << "\tcall " << FuncNum << " ; " << Name;
    }
  } else {
    // Print the assembly for the instruction.
    // We want to make sure that we do HW constants
    // before we do arena segment
    printInstruction(II, O);
  }

  O.flush();
  OutStreamer.EmitRawText(StringRef(FunStr));
}

void AMDILAsmPrinter::printCopy(const MachineInstr *MI, raw_ostream &O) {
  O << "\tmov ";
  printOperand(MI, 0, O);
  O << ", ";
  printOperand(MI, 1, O);
  O << "\n";
}

void AMDILAsmPrinter::emitMacroFunc(const MachineInstr *MI,
                                    raw_ostream &O) {
  const MachineOperand &MO = MI->getOperand(0);
  assert(MO.isGlobal());

  StringRef Name = MO.getGlobal()->getName();
  if (mTM->getSubtargetImpl()->usesHardware(AMDIL::Caps::FMA) &&
      Name.startswith("__fma_f32")) {
    Name = "__hwfma_f32";
  }

  emitMCallInst(MI, O, Name);
}

bool AMDILAsmPrinter::runOnMachineFunction(MachineFunction &lMF) {
  this->MF = &lMF;
  mMeta = new AMDILKernelManager(&lMF);
  mMFI = lMF.getInfo<AMDILMachineFunctionInfo>();

  SetupMachineFunction(lMF);
#ifdef USE_APPLE
  StringRef kernelName = CurrentFnSym->getName();
#else
  StringRef kernelName = MF->getFunction()->getName();
#endif
  mName = kernelName;
  if (AMDSymbolNames::isKernelFunctionName(mName) &&
    !AMDSymbolNames::isStubFunctionName(mName)) {
    mName = AMDSymbolNames::undecorateKernelFunctionName(mName);
  }

  mKernelName = kernelName;
#ifndef USE_APPLE
  EmitFunctionHeader();
#else
  // Until the runtime can remove the ;DEBUGSTART/;DEBUGEND tokens
  // this needs to be called manually
  EmitConstantPool();
  EmitFunctionEntryLabel();
#endif
  EmitFunctionBody();

  DEBUG_WITH_TYPE("noinlines", dbgs() << "[stack and scratch size] " <<
      mKernelName << " : " << mMFI->getStackSize() << " : " <<
      mMFI->getScratchSize() << '\n');

  delete mMeta;
  return false;
}

void AMDILAsmPrinter::addCPoolLiteral(const Constant *C) {
  if (const ConstantFP *CFP = dyn_cast<ConstantFP>(C)) {
    if (CFP->getType()->isFloatTy()) {
      mMFI->addf32Literal(CFP);
    } else {
      mMFI->addf64Literal(CFP);
    }
  } else if (const ConstantInt *CI = dyn_cast<ConstantInt>(C)) {
    int64_t Val = CI ? CI->getSExtValue() : 0;
    if (CI->getBitWidth() == (int64_t)64) {
      mMFI->addi64Literal(Val);
    } else if (CI->getBitWidth() == (int64_t)8) {
      mMFI->addi32Literal((uint32_t)Val, AMDIL::LOADCONSTi8);
    } else if (CI->getBitWidth() == (int64_t)16) {
      mMFI->addi32Literal((uint32_t)Val, AMDIL::LOADCONSTi16);
    } else {
      mMFI->addi32Literal((uint32_t)Val, AMDIL::LOADCONSTi32);
    }
  } else if (const ConstantArray *CA = dyn_cast<ConstantArray>(C)) {
    for (uint32_t I = 0, Size = CA->getNumOperands(); I < Size; ++I) {
      addCPoolLiteral(CA->getOperand(I));
    }
  } else if (const ConstantAggregateZero *CAZ
      = dyn_cast<ConstantAggregateZero>(C)) {
    if (CAZ->isNullValue()) {
      mMFI->addi32Literal(0, AMDIL::LOADCONSTi32);
      mMFI->addi64Literal(0);
      mMFI->addf64Literal((uint64_t)0);
      mMFI->addf32Literal((uint32_t)0);
    }
  } else if (const ConstantStruct *CS = dyn_cast<ConstantStruct>(C)) {
    uint32_t size = CS->getNumOperands();
    for (uint32_t x = 0; x < size; ++x) {
      addCPoolLiteral(CS->getOperand(x));
    }
  } else if (const ConstantVector *CV = dyn_cast<ConstantVector>(C)) {
    // TODO: Make this handle vectors natively up to the correct
    // size
    for (uint32_t I = 0, Size = CV->getNumOperands(); I < Size; ++I) {
      addCPoolLiteral(CV->getOperand(I));
    }
  } else {
    // TODO: Do we really need to handle ConstantPointerNull?
    // What about BlockAddress, ConstantExpr and Undef?
    // How would these even be generated by a valid CL program?
    llvm_unreachable("Found a constant type that I don't know how to handle");
  }
}

void AMDILAsmPrinter::EmitGlobalVariable(const GlobalVariable *GV) {
  llvm::StringRef GVname = GV->getName();
  SmallString<1024> Str;
  raw_svector_ostream O(Str);
  int32_t AutoSize = mAMI->getArrayOffset(GVname);
  int32_t ConstSize = mAMI->getConstOffset(GVname);
  O << ".global@" << GVname;
  if (AutoSize != -1) {
    O << ":" << AutoSize << "\n";
  } else if (ConstSize != -1) {
    O << ":" << ConstSize << "\n";
  }
  O.flush();
  OutStreamer.EmitRawText(O.str());
}

void AMDILAsmPrinter::printOperand(const MachineInstr *MI,
                                   int OpNum,
                                   raw_ostream &O) {
  unsigned RegClass = MI->getDesc().OpInfo[OpNum].RegClass;
  const MachineOperand &MO = MI->getOperand(OpNum);

  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    if ((signed)MO.getReg() < 0) {
      // FIXME: we need to remove all virtual register creation after register allocation.
      // This is a work-around to make sure that the virtual register range does not
      // clobber the physical register range.
      O << "r" << ((MO.getReg() & 0x7FFFFFFF)  + 2048) << getSwizzle(MI, OpNum);
    } else if (OpNum == 0 && isAtomicInst(MI) && isStoreInst(MI)) {
      const MachineOperand &MO = MI->getOperand(OpNum);
      unsigned Reg = MI->getOperand(2).getReg();
      O << "mem0";
      if (isXComponentReg(Reg)) {
        O << getDstSwizzle(AMDIL_DST_X___);
      } else if (isYComponentReg(Reg)) {
        O << getDstSwizzle(AMDIL_DST__Y__);
      } else if (isZComponentReg(Reg)) {
        O << getDstSwizzle(AMDIL_DST___Z_);
      } else if (isWComponentReg(Reg)) {
        O << getDstSwizzle(AMDIL_DST____W);
      } else if (isXYComponentReg(Reg)) {
        O << getDstSwizzle(AMDIL_DST_XY__);
      } else if (isZWComponentReg(Reg)) {
        O << getDstSwizzle(AMDIL_DST___ZW);
      } else {
        O << getDstSwizzle(AMDIL_DST_DFLT);
      }
      O << ", " << getRegisterName(MO.getReg()) << getSwizzle(MI, OpNum);
    } else if (OpNum == 0 && isScratchInst(MI) && isStoreInst(MI)) {
      O << getRegisterName(MO.getReg()) << ".x]";
      uint32_t Reg = MI->getOperand(1).getReg();
      // If we aren't a vector register, print the dst swizzle.
      if (Reg < AMDIL::R1 || Reg > AMDIL::R1012) {
        O << getSwizzle(MI, OpNum);
      }
    } else {
      O << getRegisterName(MO.getReg()) << getSwizzle(MI, OpNum);
    }
    break;
  case MachineOperand::MO_Immediate:
  case MachineOperand::MO_FPImmediate: {
    if (isSkippedLiteral(MI, OpNum)) {
    } else if (isBypassedLiteral(MI, OpNum)) {
      O << MO.getImm();
    } else if (MO.isImm() || MO.isFPImm()) {
      O << "l" << MO.getImm() << getSwizzle(MI, OpNum);
    } else {
      llvm_unreachable("Invalid literal/constant type");
      mMFI->addErrorMsg(amd::CompilerErrorMessage[INTERNAL_ERROR]);
    }
  }
    break;
  case MachineOperand::MO_MachineBasicBlock:
    EmitBasicBlockStart(MO.getMBB());
    return;
  case MachineOperand::MO_GlobalAddress: {
    int Offset = 0;
    const GlobalValue *GV = MO.getGlobal();
    // Here we look up by the name for the corresponding number
    // and we print that out instead of the name or the address
    if (MI->getOpcode() == AMDIL::CALL) {
      // when calling another kernel, don't call the wrapper, call the
      // kernel function directly
      StringRef Name = GV->getName();
      if (AMDSymbolNames::isKernelFunctionName(Name))
        Name = AMDSymbolNames::undecorateKernelFunctionName(Name);
      uint32_t FuncNum = Name.empty()
                       ? mAMI->getOrCreateFunctionID(GV)
                       : mAMI->getOrCreateFunctionID(Name);

      mMFI->addCalledFunc(FuncNum);
      O << FuncNum <<" ; "<< Name;
    } else if ((mAMI->getArrayOffset(GV->getName()) != -1) ||
      mAMI->getConstOffset(GV->getName()) != -1) {
      if (mAMI->getArrayOffset(GV->getName()) != -1) {
        Offset = mAMI->getArrayOffset(GV->getName());
        mMFI->setUsesLDS();
      } else {
        Offset = mAMI->getConstOffset(GV->getName());
        mMFI->setUsesConstant();
        mMFI->addMetadata(";memory:datareqd");
      }
      assert((RegClass == AMDIL::GPR_32RegClassID
              || RegClass == AMDIL::GPR_64RegClassID)
             && "unexpected reg class");
      if (RegClass == AMDIL::GPR_32RegClassID) {
        O << "l" << mMFI->getLitIdx((uint32_t)Offset) << ".x";
      } else {
        O << "l" << mMFI->getLitIdx((uint64_t)Offset) << ".xy";
      }
    } else {
      llvm_unreachable("GlobalAddress without a function call!");
      mMFI->addErrorMsg(amd::CompilerErrorMessage[MISSING_FUNCTION_CALL]);
    }
    break;
  }

  case MachineOperand::MO_ExternalSymbol: {
    if (MI->getOpcode() == AMDIL::CALL) {
      // when calling another kernel, don't call the wrapper, call the
      // kernel function directly
      StringRef Name = MO.getSymbolName();
      if (AMDSymbolNames::isKernelFunctionName(Name))
        Name = AMDSymbolNames::undecorateKernelFunctionName(Name);
      uint32_t FuncNum = mAMI->getOrCreateFunctionID(Name);
      mMFI->addCalledFunc(FuncNum);

      O << FuncNum << " ; " << MO.getSymbolName();
      // This is where pointers should get resolved
    } else {
      llvm_unreachable("ExternalSymbol without a function call!");
      mMFI->addErrorMsg(amd::CompilerErrorMessage[MISSING_FUNCTION_CALL]);
    }
    break;
  }

  case MachineOperand::MO_ConstantPoolIndex: {
    // Copies of constant buffers need to be done here
    const AMDILKernel *Tmp = mAMI->getKernel(mKernelName);
    uint32_t Offset = Tmp->CPOffsets[MO.getIndex()].first;
    assert((RegClass == AMDIL::GPR_32RegClassID
            || RegClass == AMDIL::GPR_64RegClassID)
           && "unexpected reg class");
    if (RegClass == AMDIL::GPR_32RegClassID) {
      O << "l" << mMFI->getLitIdx(Offset);
    } else {
      O << "l" << mMFI->getLitIdx((uint64_t)Offset);
    }
    break;
  }

  default:
    O << "<unknown operand type>"; break;
  }
}

void AMDILAsmPrinter::printMemOperand(const MachineInstr *MI,
                                      int OpNum,
                                      raw_ostream &O,
                                      const char *Modifier) {
  const MachineOperand &MO = MI->getOperand(OpNum);
  if (OpNum != 1) {
    printOperand(MI, OpNum, O);
  } else {
    switch (MO.getType()) {
    case MachineOperand::MO_Register:
      if ((signed)MO.getReg() < 0) {
        // FIXME: we need to remove all virtual register creation after register allocation.
        // This is a work-around to make sure that the virtual register range does not
        // clobber the physical register range.
        O << "r" << ((MO.getReg() & 0x7FFFFFFF) + 2048) << getSwizzle(MI, OpNum);
      } else if (OpNum == 0 && isScratchInst(MI)) {
        O << getRegisterName(MO.getReg()) << ".x]";
        uint32_t reg = MI->getOperand(1).getReg();
        // If we aren't the vector register, print the dst swizzle.
        if (reg < AMDIL::R1 || reg > AMDIL::R1012) {
          O << getSwizzle(MI, OpNum);
        }
      } else {
        O << getRegisterName(MO.getReg()) << getSwizzle(MI, OpNum);
      }
      break;
    case MachineOperand::MO_Immediate:
    case MachineOperand::MO_FPImmediate: {
      if (isSkippedLiteral(MI, OpNum)) {

      } else if (isBypassedLiteral(MI, OpNum)) {
        O << MO.getImm();
      } else if (MO.isImm() || MO.isFPImm()) {
        O << "l" << MO.getImm() << getSwizzle(MI, OpNum);
      } else {
        llvm_unreachable("Invalid literal/constant type");
        mMFI->addErrorMsg(amd::CompilerErrorMessage[INTERNAL_ERROR]);
      }
      break;
    }

    case MachineOperand::MO_ConstantPoolIndex: {
      // Copies of constant buffers need to be done here
      const AMDILKernel *Tmp = mAMI->getKernel(mKernelName);
      uint32_t Offset = Tmp->CPOffsets[MO.getIndex()].first;
      unsigned RegClass = MI->getDesc().OpInfo[OpNum].RegClass;
      assert((RegClass == AMDIL::GPR_32RegClassID
              || RegClass == AMDIL::GPR_64RegClassID)
             && "unexpected reg class");
      if (RegClass == AMDIL::GPR_32RegClassID) {
        O << "l" << mMFI->getLitIdx(Offset);
      } else {
        O << "l" << mMFI->getLitIdx((uint64_t)Offset);
      }
      break;
    }

    default:
      O << "<unknown operand type>";
      break;
    }
  }
}

const char *AMDILAsmPrinter::getSwizzle(const MachineInstr *MI, int OpNum) {
  const MachineOperand &MO = MI->getOperand(OpNum);
  OpSwizzle Swiz;
  Swiz.u8all = mMFI->getMOSwizzle(MI->getOperand(OpNum));

  return Swiz.bits.dst
       ? getDstSwizzle(Swiz.bits.swizzle)
       : getSrcSwizzle(Swiz.bits.swizzle);
}

void AMDILAsmPrinter::EmitStartOfAsmFile(Module &M) {
  SmallString<1024> Str;
  raw_svector_ostream O(Str);
  const AMDILSubtarget *CurTarget = mTM->getSubtargetImpl();
  mAMI = &MMI->getObjFileInfo<AMDILModuleInfo>();
  mAMI->processModule(&M, mTM);

  for (Module::const_iterator I = M.begin(),
      E = M.end(); I != E; ++I) {
    // Map all the known names to a unique number
    mAMI->getOrCreateFunctionID(I->getName());
  }

  // Since we are using the macro db as well as using outlined macro
  // to define functions, SC requires the first token must be a macro.
    // So we make up a macro that is never used.
    // I originally picked -1, but the IL text translater treats them as
    // unsigned integers.
    O << "mdef(16383)_out(1)_in(2)\n";
    O << "mov r0, in0\n";
    O << "mov r1, in1\n";
    O << "div_zeroop(infinity) r0.x___, r0.x, r1.x\n";
    O << "mov out0, r0\n";
    O << "mend\n";

  // We need to increase the number of reserved literals for
  // any literals we output manually instead of via the
  // emitLiteral function. This function should never
  // have any executable code in it. Only declarations
  // and the main function patch symbol.
  if (CurTarget->getGeneration() == AMDIL::HDTEST) {
    O << "il_cs_3_0\n";
  } else {
    O << "il_cs_2_0\n";
  }
  O << "dcl_cb cb0[15] ; Constant buffer that holds ABI data\n";
  O << "dcl_literal l0, 0x00000004, 0x00000001, 0x00000002, 0x00000003\n";
  O << "dcl_literal l1, 0x0000FFFF, 0xFFFFFFFE, 0x000000FF, 0xFFFFFFFC\n";
  O << ";$$$$$$$$$$\n";
  O << "endmain\n";
  O << ";DEBUGSTART\n";
  OutStreamer.EmitRawText(O.str());
}

void AMDILAsmPrinter::EmitEndOfAsmFile(Module &M) {
  SmallString<1024> Str;
  raw_svector_ostream O(Str);
  const AMDILSubtarget *CurTarget = mTM->getSubtargetImpl();
  O << ";DEBUGEND\n";
  if (CurTarget->isSupported(AMDIL::Caps::MacroDB)) {
    for (llvm::DenseSet<uint32_t>::iterator MSB = mMacroIDs.begin(),
           MSE = mMacroIDs.end(); MSB != MSE; ++MSB) {
      int Lines;
      int Idx = *MSB;

      const char* *Macro = amd::MacroDBGetMacro(&Lines, Idx);
      for (int K = 0; K < Lines; ++K) {
        O << Macro[K];
      }
    }
  }

  if (mAMI)
    mAMI->dumpDataSection(O, mMFI);
  O << "\nend\n";
  OutStreamer.EmitRawText(O.str());
}

void AMDILAsmPrinter::addGlobalConstantArrayLiterals() {
  bool Is64Bit = mTM->getSubtargetImpl()->is64bit();

  // Add the literals for the offsets and sizes of
  // all the globally scoped constant arrays
  for (StringMap<AMDILConstPtr>::iterator CMB = mAMI->consts_begin(),
         CME = mAMI->consts_end(); CMB != CME; ++CMB) {
    if (Is64Bit)
      mMFI->addi64Literal(CMB->second.offset);
    else
      mMFI->addi32Literal(CMB->second.offset);
    mMFI->addi32Literal(CMB->second.size);
    mMFI->addMetadata(";memory:datareqd");
    mMFI->setUsesConstant();
  }
}

void AMDILAsmPrinter::EmitFunctionBodyStart() {
  const AMDILSubtarget *curTarget = mTM->getSubtargetImpl();
  bool UseMacroForCall = curTarget->isSupported(AMDIL::Caps::UseMacroForCall);
  bool Is64Bit = curTarget->is64bit();
  SmallString<1024> Str;
  raw_svector_ostream O(Str);

  O << "";
  O << ";DEBUGEND\n";
  ++mBuffer;
  bool IsKernel = mMFI->isKernel();
  uint32_t id = mName.empty()
    ? mAMI->getOrCreateFunctionID(MF->getFunction())
    : mAMI->getOrCreateFunctionID(mName);
  mMeta->setKernel(IsKernel);
  mMeta->setID(id);

  if (mName.empty()) {
    mName = Twine("unknown_").concat(Twine(id)).str();
  }
  mMeta->setName(mName);

  addGlobalConstantArrayLiterals();

  // Print the wrapper function
  if (IsKernel) {
    mMeta->printWrapperHeader(this, O);
    mMeta->getIntrinsicSetup(this, O);
    mMeta->processArgMetadata(O, mBuffer);
    mMeta->printGroupSize(O);
    mMeta->printWavefrontSize(O);
    if (!UseMacroForCall) {
      addGlobalConstantArrayLiterals();
      mMeta->emitLiterals(O);
    }
    mMeta->printArgCopies(O, this);
    assert(mMFI->getNumRetRegs() == 0 && "kernel return type not void");
    // make call to stub
    std::string StubName = UseMacroForCall?std::string(mMeta->getStubName()):mName;
    uint32_t StubID = mAMI->getOrCreateFunctionID(StubName);
    if (UseMacroForCall) {
      O << "mcall(" << StubID << ") (), () ; " << StubName << "\n";
    } else {
      O << "call " << StubID << " ; " << StubName << "\n";
    }
    mMeta->printWrapperFooter(O);
    mMeta->printMetaData(O, mMeta->getID(), true);
  }

  // emit the current function's header IL instruction
  if (UseMacroForCall) {
    O << "mdef(" << id << ")_out(" << mMFI->getRetNumVecRegs()
      << ")_in(" << mMFI->getArgNumVecRegs() << ")_outline ; " << mName << "\n";
  } else {
    O << "func " << id << " ; " << mName << "\n";
  }

  if (IsKernel) {
    AMDILKernel &tmp = *mMFI->getKernel();
    // add the literals for the offsets and sizes of
    // all kernel declared local arrays
    if (tmp.lvgv) {
      AMDILLocalArg *lptr = tmp.lvgv;
      llvm::SmallVector<AMDILArrayMem*, DEFAULT_VEC_SLOTS>::iterator LMB, LME;
      for (LMB = lptr->local.begin(), LME = lptr->local.end();
          LMB != LME; ++LMB) {
        if (Is64Bit)
          mMFI->addi64Literal((*LMB)->offset);
        else
          mMFI->addi32Literal((*LMB)->offset);
        mMFI->addi32Literal((*LMB)->vecSize);
        mMFI->setUsesLDS();
      }
    }
    // Add the literals for the offsets and sizes of
    // all the kernel constant arrays
    llvm::SmallVector<AMDILConstPtr, DEFAULT_VEC_SLOTS>::const_iterator CPB, CPE;
    for (CPB = tmp.constPtr.begin(), CPE = tmp.constPtr.end();
         CPB != CPE; ++CPB) {
      if (Is64Bit)
        mMFI->addi64Literal(CPB->offset);
      else
        mMFI->addi32Literal(CPB->offset);
      mMFI->addi32Literal(CPB->size);
      mMFI->setUsesConstant();
    }
    }
    mMeta->printDecls(this, O);
  mMeta->emitLiterals(O);

  O.flush();
  OutStreamer.EmitRawText(O.str());
}

void AMDILAsmPrinter::EmitFunctionBodyEnd() {
  SmallString<1024> Str;
  raw_svector_ostream O(Str);
  uint32_t ID = mName.empty()
              ? mAMI->getOrCreateFunctionID(MF->getFunction())
              : mAMI->getOrCreateFunctionID(mName);
  if (mName.empty()) {
    mName = Twine("unknown_").concat(Twine(ID)).str();
  }
  if (mTM->getSubtargetImpl()->isSupported(AMDIL::Caps::UseMacroForCall)) {
    O << "mend ; " << mName << "\n";
  } else {
    O << "ret\nendfunc ; " << mName << "\n";
  }
  if (mAMI->isKernel(mKernelName)) {
    mMeta->setName(mName);
  }
    mMeta->printMetaData(O, ID, false);

  O << ";DEBUGSTART\n";
  O.flush();
  OutStreamer.EmitRawText(O.str());
}

void AMDILAsmPrinter::EmitConstantPool() {
  if (!mAMI->getKernel(mKernelName)) {
    return;
  }

  AMDILKernel *Tmp = mAMI->getKernel(mKernelName);
  if (!Tmp || !Tmp->mKernel) {
    return;
  }

  bool Is64Bit = mTM->getSubtargetImpl()->is64bit();

  mAMI->calculateCPOffsets(MF, Tmp);
  // Add all the constant pool offsets to the literal table
  for (uint32_t I = 0, Size = Tmp->CPOffsets.size(); I < Size; ++I) {
    mMFI->addMetadata(";memory:datareqd");
    if (Is64Bit)
      mMFI->addi64Literal(Tmp->CPOffsets[I].first);
    else
      mMFI->addi32Literal(Tmp->CPOffsets[I].first);
  }

  // Add all the constant pool constants to the literal tables
  {
    const MachineConstantPool *MCP = MF->getConstantPool();
    const std::vector<MachineConstantPoolEntry> &Consts= MCP->getConstants();
    for (uint32_t x = 0, s = Consts.size(); x < s; ++x) {
      addCPoolLiteral(Consts[x].Val.ConstVal);
    }
  }
}

void AMDILAsmPrinter::EmitFunctionEntryLabel() {
  return;
  assert(0 && "When is this function hit!");
}

/// getDebugResourceLocation - Get resource id information encoded in
/// target flags.
bool AMDILAsmPrinter::getDebugResourceID(const Value *V, uint32_t &RID) const {
  if (mMFI) {
    RID = mMFI->getUAVID(V);
    return true;
  }

  return mAMI->getGlobalValueRID(this, V, RID);
}

unsigned AMDILAsmPrinter::correctDebugAS(unsigned AS, const Value *V) const {
  if (AS) return AS;
  // This only needs to be corrected for private AS.
  if (const GlobalVariable *GV = dyn_cast<GlobalVariable>(V)) {
    // A global variable with an initializer that has address space 0 cannot
    // exist in OpenCL, instead it must have part of the constant address
    // space.
    if (GV->hasInitializer()) {
      return AMDILAS::CONSTANT_ADDRESS;
    }
  }
  return AS;
}

bool AMDILAsmPrinter::isMacroCall(const MachineInstr *MI) {
  StringRef Name = mTM->getInstrInfo()->getName(MI->getOpcode());
  return Name.startswith("MACRO");
}

bool AMDILAsmPrinter::isMacroFunc(const MachineInstr *MI) {
  if (MI->getOpcode() != AMDIL::CALL) {
    return false;
  }

  if (!MI->getOperand(0).isGlobal()) {
    return false;
  }

  StringRef Name = MI->getOperand(0).getGlobal()->getName();
  if (Name.startswith("__atom_") || Name.startswith("__atomic_")) {
    mMFI->setOutputInst();
  }

  return amd::MacroDBFindMacro(Name.data()) != -1;
}

static const char *getRegSwizzle(unsigned Reg, bool Dst) {
  if (Reg >= AMDIL::Rx1 && Reg < AMDIL::Rxy1) {
    return ".x";
  }

  if (Reg >= AMDIL::Ry1 && Reg < AMDIL::Rz1) {
    return ".y";
  }

  if (Reg >= AMDIL::Rz1 && Reg < AMDIL::Rzw1) {
    return ".z";
  }

  if (Reg >= AMDIL::Rw1 && Reg < AMDIL::Rx1) {
    return ".w";
  }

  if (Reg >= AMDIL::Rxy1 && Reg < AMDIL::Ry1) {
    return Dst ? ".xy__" : ".xy00";
  }

  if (Reg >= AMDIL::Rzw1 && Reg < AMDIL::SDP) {
    return Dst ? ".__zw" : ".00zw";
  }

  return  "";
}

void AMDILAsmPrinter::emitMCallInst(const MachineInstr *MI,
                                    raw_ostream &O,
                                    StringRef Name) {
  const AMDILSubtarget *CurTarget = mTM->getSubtargetImpl();
  int MacroNum = amd::MacroDBFindMacro(Name.data()); // FIXME
  int NumIn = amd::MacroDBNumInputs(MacroNum);
  int NumOut = amd::MacroDBNumOutputs(MacroNum);
  if (MacroNum == -1) {
    return;
  }

  if (CurTarget->isSupported(AMDIL::Caps::MacroDB)) {
    mMacroIDs.insert(MacroNum);
  } else {
    mMFI->addCalledIntr(MacroNum);
  }
  const TargetRegisterClass *TRC = NULL;
  if (Name.find("4f32") != StringRef::npos
      || Name.find("4i32") != StringRef::npos) {
    TRC = MF->getTarget()
      .getRegisterInfo()->getRegClass(AMDIL::GPRV4I32RegClassID);
  } else if (Name.find("2f32") != StringRef::npos
             || Name.find("2i32") != StringRef::npos) {
    TRC = MF->getTarget()
      .getRegisterInfo()->getRegClass(AMDIL::GPRV2I32RegClassID);
  } else {
    TRC = MF->getTarget()
      .getRegisterInfo()->getRegClass(AMDIL::GPR_32RegClassID);
  }
  O << "\tmcall(" << MacroNum << ")(";

  int I;
  for (I = 0; I < NumOut - 1; ++I) {
    O << getRegisterName(TRC->getRegister(I))
      << getRegSwizzle(TRC->getRegister(I), true) << ", ";
  }
  O << getRegisterName(TRC->getRegister(I))
    << getRegSwizzle(TRC->getRegister(I), true) << "),(";
  for (I = 0; I < NumIn - 1; ++I) {
    O << getRegisterName(TRC->getRegister(I))
      << getRegSwizzle(TRC->getRegister(I), false) << ", ";
  }
  O << getRegisterName(TRC->getRegister(I))
    << getRegSwizzle(TRC->getRegister(I), false) << ")";
  O << " ;" << Name << '\n';
}

#if defined(USE_APPLE)
void AMDILAsmPrinter::EmitDwarfRegOp(const MachineLocation &MLoc) const {

}
#else
void AMDILAsmPrinter::EmitDwarfRegOp(const MachineLocation &MLoc) const {
  const TargetRegisterInfo *RI = TM.getRegisterInfo();
  unsigned Reg = MLoc.getReg();
  unsigned BaseReg = AMDIL::R1;
  const char *RegXStr = NULL;
  unsigned Offset = 0;
  unsigned Size = 32;
  const char *OffStr = NULL;
  if (isXComponentReg(Reg)) {
    BaseReg += (Reg - AMDIL::Rx1);
    RegXStr = "DW_OP_regx for x component of register";
    Offset = 0;
    OffStr = "DW_OP_bit_piece 32 0";
  } else if (isYComponentReg(Reg)) {
    BaseReg += (Reg - AMDIL::Ry1);
    RegXStr = "DW_OP_regx for y component of register";
    Offset = 32;
    OffStr = "DW_OP_bit_piece 32 32";
  } else if (isZComponentReg(Reg)) {
    BaseReg += (Reg - AMDIL::Rz1);
    RegXStr = "DW_OP_regx for z component of register";
    Offset = 64;
    OffStr = "DW_OP_bit_piece 32 64";
  } else if (isWComponentReg(Reg)) {
    BaseReg += (Reg - AMDIL::Rw1);
    RegXStr = "DW_OP_regx for w component of register";
    Offset = 96;
    OffStr = "DW_OP_bit_piece 32 96";
  } else if (isXYComponentReg(Reg)) {
    BaseReg += (Reg - AMDIL::Rxy1);
    RegXStr = "DW_OP_regx for xy component of register";
    Offset = 0;
    Size = 64;
    OffStr = "DW_OP_bit_piece 64 0";
  } else if (isZWComponentReg(Reg)) {
    BaseReg += (Reg - AMDIL::Rzw1);
    RegXStr = "DW_OP_regx for zw component of register";
    Offset = 64;
    Size = 64;
    OffStr = "DW_OP_bit_piece 64 64";
  } else {
    BaseReg = Reg;
    RegXStr = "DW_OP_regx for xyzw component of register";
    Offset = 0;
    Size = 128;
    OffStr = "DW_OP_bit_piece 128 0";
  }
  BaseReg = RI->getDwarfRegNum(BaseReg, false);
  OutStreamer.AddComment("Loc expr Size");
  unsigned OffsetSize = MCAsmInfo::getULEB128Size(Size)
    + MCAsmInfo::getULEB128Size(Offset);
  if (int Offset = MLoc.getOffset()) {
    OffsetSize += Offset ?  MCAsmInfo::getSLEB128Size(Offset) : 1;
    OutStreamer.AddComment("Loc expr Size");
    EmitInt16(OffsetSize);
    OutStreamer.AddComment(
        dwarf::OperationEncodingString(dwarf::DW_OP_fbreg));
    EmitInt8(dwarf::DW_OP_fbreg);
    OutStreamer.AddComment("Offset");
    EmitSLEB128(Offset);
  } else if (BaseReg < 32) {
    EmitInt16(2 + OffsetSize);
    OutStreamer.AddComment(
        dwarf::OperationEncodingString(dwarf::DW_OP_reg0 + BaseReg));
    EmitInt8(dwarf::DW_OP_reg0 + BaseReg);
  } else {
    EmitInt16(2 +  MCAsmInfo::getULEB128Size(BaseReg) + OffsetSize);
    OutStreamer.AddComment(RegXStr);
    EmitInt8(dwarf::DW_OP_regx);
    OutStreamer.AddComment(Twine(BaseReg));
    EmitULEB128(BaseReg);
  }

  OutStreamer.AddComment(OffStr);
  EmitInt8(dwarf::DW_OP_bit_piece);
  EmitULEB128(Size);
  EmitULEB128(Offset);
}
#endif


