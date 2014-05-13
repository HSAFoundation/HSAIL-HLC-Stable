//===-- AMDILTargetMachine.cpp - Define TargetMachine for AMDIL -----------===//
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
//===----------------------------------------------------------------------===//


#include "AMDILTargetMachine.h"
#include "AMDILFrameLowering.h"
#include "AMDILMCAsmInfo.h"
#include "AMDILSubtarget.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/SchedulerRegistry.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/DataLayout.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"

#ifdef AMD_LLVM_INTERNAL
#include "AMDILInternal.h"
#endif

using namespace llvm;

static MCAsmInfo* createMCAsmInfo(const Target &T, StringRef TT)
{
  Triple TheTriple(TT);
  switch (TheTriple.getOS()) {
    default:
    case Triple::UnknownOS:
      return new AMDILMCAsmInfo(TheTriple);
  }
}

// MC related code probably should be in MCTargetDesc subdir
static MCCodeGenInfo *createAMDILMCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                               CodeModel::Model CM,
                                               CodeGenOpt::Level OL)
{
  MCCodeGenInfo *X = new MCCodeGenInfo();
  X->InitMCCodeGenInfo(RM, CM, OL);
  return X;
}

extern "C" void LLVMInitializeAMDILTarget() {
  // Register the target
  RegisterTargetMachine<TheAMDILTargetMachine> X(TheAMDILTarget);
  RegisterTargetMachine<TheAMDILTargetMachine> X64(TheAMDIL64Target);

  // Register the target asm info
  RegisterMCAsmInfoFn A(TheAMDILTarget, createMCAsmInfo);
  RegisterMCAsmInfoFn A64(TheAMDIL64Target, createMCAsmInfo);
  RegisterMCCodeGenInfoFn B(TheAMDILTarget, createAMDILMCCodeGenInfo);
  RegisterMCCodeGenInfoFn B64(TheAMDIL64Target, createAMDILMCCodeGenInfo);

  // Register the code emitter
  //TargetRegistry::RegisterCodeEmitter(TheAMDILTarget,
  //createAMDILMCCodeEmitter);
}

TheAMDILTargetMachine::TheAMDILTargetMachine(const Target &T,
    StringRef TT, StringRef CPU, StringRef FS, const TargetOptions &Options,
    Reloc::Model RM, CodeModel::Model CM, CodeGenOpt::Level OL)
  : AMDILTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL) {
}

/// AMDILTargetMachine ctor -
///
AMDILTargetMachine::AMDILTargetMachine(const Target &T,
    StringRef TT, StringRef CPU, StringRef FS,
    const TargetOptions &Options,
    Reloc::Model RM, CodeModel::Model CM,
    CodeGenOpt::Level OL)
:
  LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
  Subtarget(TT, CPU, FS),
  DL(Subtarget.getDataLayout()),
  FrameLowering(TargetFrameLowering::StackGrowsUp,
      Subtarget.getStackAlignment(), 0),
  InstrInfo(*this), //JITInfo(*this),
  TLInfo(*this),
  IntrinsicInfo(this)
#if 0
  ELFWriterInfo(false, true)
#endif
{
  setAsmVerbosityDefault(true);
  setMCUseLoc(false);

  // FIXME: This should be true. Disabled until problems with CFG structurizer
  // are fixed, which ironically is what this should be helping with avoiding.
  setRequiresStructuredCFG(false);
}

AMDILTargetLowering *AMDILTargetMachine::getTargetLowering() const {
  return const_cast<AMDILTargetLowering*>(&TLInfo);
}

const AMDILInstrInfo *AMDILTargetMachine::getInstrInfo() const {
  return &InstrInfo;
}

const AMDILFrameLowering *AMDILTargetMachine::getFrameLowering() const {
  return &FrameLowering;
}

const AMDILSubtarget *AMDILTargetMachine::getSubtargetImpl() const {
  return &Subtarget;
}

const AMDILRegisterInfo *AMDILTargetMachine::getRegisterInfo() const {
  return &InstrInfo.getRegisterInfo();
}

const DataLayout *AMDILTargetMachine::getDataLayout() const {
  return &DL;
}

#if 0
const AMDILELFWriterInfo *AMDILTargetMachine::getELFWriterInfo() const {
  return Subtarget.isTargetELF() ? &ELFWriterInfo : 0;
}
#endif

TargetPassConfig*
AMDILTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new AMDILPassConfig(this, PM);
}

const AMDILIntrinsicInfo *AMDILTargetMachine::getIntrinsicInfo() const {
  return &IntrinsicInfo;
}

AMDILPassConfig::AMDILPassConfig(AMDILTargetMachine *TM, PassManagerBase &PM)
  : TargetPassConfig(TM, PM) {
  // FIXME: This is a work-around for a problem with the CFG Structurizer unable
  // to handle certain types of flow control. The case in question occurs with
  // BLAS and causes a jump from the else branch to the body of the loop
  // in the if branch.
  // The pseudo code is:
  // if (a)
  // while(b) {
  // c:
  // if (d) break;
  // }
  // else
  // d = true
  // jump c
  // endif
  setEnableTailMerge(false);
}

bool AMDILPassConfig::addPreISel() {
  AMDILTargetMachine &AMDTM = getAMDILTargetMachine();
  if (AMDTM.getSubtargetImpl()->isSupported(AMDIL::Caps::UseMacroForCall)) {
    addPass(createAMDILCreateKernelStubPass());
  }
  // Vector Coarsening as the current implementation does not support
  // big endian yet.
#ifdef AMD_LLVM_INTERNAL
  if (getAMDILTargetMachine().getOptLevel() != CodeGenOpt::None &&
      getAMDILTargetMachine().getDataLayout()->isLittleEndian()) {
    addPass(createVectorCoarseningPass());
    return true;
  }
#endif
  return false;
}

bool AMDILPassConfig::addInstSelector() {
  AMDILTargetMachine &AMDTM = getAMDILTargetMachine();
  const AMDILSubtarget &SubTarget = *AMDTM.getSubtargetImpl();
  addPass(createAMDILBarrierDetect(AMDTM, AMDTM.getOptLevel()));
  addPass(createAMDILPrintfConvert(AMDTM, AMDTM.getOptLevel()));
  addPass(createAMDILInlinePass(AMDTM, AMDTM.getOptLevel()));
  addPass(createAMDILPeepholeOpt(AMDTM, AMDTM.getOptLevel()));
  addPass(createAMDILISelDag(AMDTM, AMDTM.getOptLevel()));
  return false;
}

bool AMDILPassConfig::addPreRegAlloc() {
  // If debugging, reduce code motion. Use less aggressive pre-RA scheduler
  if (getOptLevel() == CodeGenOpt::None) {
    llvm::RegisterScheduler::setDefault(&llvm::createSourceListDAGScheduler);
  }

  addPass(createAMDILMachinePeephole());
  addPass(createAMDILPointerManager(getAMDILTargetMachine(),
				    getAMDILTargetMachine().getOptLevel()));
  return false;
}

bool AMDILPassConfig::addPostRegAlloc() {
  return false;  // -print-machineinstr should print after this.
}

/// addPreEmitPass - This pass may be implemented by targets that want to run
/// passes immediately before machine code is emitted.  This should return
/// true if -print-machineinstrs should print out the code after the passes.
bool AMDILPassConfig::addPreEmitPass() {
  const AMDILTargetMachine &AMDTM = getTM<AMDILTargetMachine>();
  if (getOptLevel() != CodeGenOpt::None && AMDTM.Options.EnableEBB) {
    // FIXME: Why bother with EnableEBB option?
    addPass(createAMDILMachineEBBPass(AMDTM));
  }

  addPass(createAMDILCFGPreparationPass());
  // ToDo: Fix AMDILRegisterUseValidate and reenable it
#if 0 //ndef NDEBUG
  if (!AMDTM.Options.UseMacroForCalls)
  addPass(createAMDILRegisterUseValidate(getAMDILTargetMachine(),
            getAMDILTargetMachine().getOptLevel()));
#endif
  addPass(createAMDILCFGStructurizerPass());
  addPass(createAMDILLiteralManager(getAMDILTargetMachine(),
				    getAMDILTargetMachine().getOptLevel()));
  addPass(createAMDILIOExpansion(getAMDILTargetMachine(),
				 getAMDILTargetMachine().getOptLevel()));
  addPass(createAMDILSwizzleEncoder(AMDTM));
  if (!AMDTM.getSubtargetImpl()->isSupported(AMDIL::Caps::UseMacroForCall))
  addPass(createAMDILRenumberRegister(getAMDILTargetMachine(),
            getAMDILTargetMachine().getOptLevel()));
  return true;
}

extern "C" void LLVMInitializeAMDILTargetMC() {}

