//===- AMDILRegisterInfo.h - AMDIL Register Information Impl ----*- C++ -*-===//
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
// This file contains the AMDIL implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef AMDILREGISTERINFO_H_
#define AMDILREGISTERINFO_H_

#ifdef USE_APPLE
#include "AMDILLLVMApple.h"
#else
#include "AMDILLLVMPC.h"
#endif

#include "llvm/Config/config.h"
#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "AMDILGenRegisterInfo.inc"

namespace llvm {
  class AMDILTargetMachine;
  class TargetInstrInfo;
  class Type;

  /// DWARFFlavour - Flavour of dwarf regnumbers
  ///
  namespace DWARFFlavour {
    enum {
      AMDIL_Generic = 0
    };
  }

  struct AMDILRegisterInfo : public AMDILGenRegisterInfo {
    AMDILTargetMachine &TM;
    const TargetInstrInfo &TII;

    AMDILRegisterInfo(AMDILTargetMachine &tm, const TargetInstrInfo &tii);
    /// Code Generation virtual methods...
    const uint16_t *getCalleeSavedRegs(const MachineFunction *MF = NULL) const;
    const uint32_t *getCallPreservedMask(CallingConv::ID) const;

    const TargetRegisterClass* const* getCalleeSavedRegClasses(
      const MachineFunction *MF = NULL) const;

    virtual const TargetRegisterClass *getPointerRegClass(
      const MachineFunction &MF,
      unsigned Kind = 0) const;

    BitVector getReservedRegs(const MachineFunction &MF) const;

    void eliminateCallFramePseudoInstr(MachineFunction &MF,
                                       MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator I) const;
      void eliminateFrameIndex(MachineBasicBlock::iterator II,
                               int SPAdj,
                               RegScavenger *RS = NULL) const;

    void processFunctionBeforeFrameFinalized(MachineFunction &MF) const;

    bool canRealignStack(const MachineFunction &MF) const;
    virtual bool needsStackRealignment(const MachineFunction &MF) const;

    // Debug information queries.
    unsigned getRARegister() const;

    unsigned getFrameRegister(const MachineFunction &MF) const;

    // Exception handling queries.
    unsigned getEHExceptionRegister() const;
    unsigned getEHHandlerRegister() const;

    int64_t getStackSize() const;
#if 0
    bool requiresRegisterScavenging(const MachineFunction&) const {
      return true;
    }

    bool requireFrameIndexScavenging(const MachineFunction&)const {
      return true;
    }

    bool requiresVirtualBaseRegisters(const MachineFunction&) const {
      return true;
    }
#endif

    static const TargetRegisterClass *getRegClassFromID(unsigned ID);
    static const TargetRegisterClass *getRegClassFromType(MVT VT);
    static unsigned getRegClassIDFromType(MVT VT) {
      return getRegClassFromType(VT)->getID();
    }

    static unsigned getRegClassFromName(StringRef Name);

    private:
    mutable int64_t baseOffset;
    mutable int64_t nextFuncOffset;
  };

} // end namespace llvm

#endif // AMDILREGISTERINFO_H_
