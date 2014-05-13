//===----------- AMDILIOExpansion.h - IO Expansion Pass ---------*- C++ -*-===//
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
// The AMDIL IO Expansion class expands pseudo IO instructions into a sequence
// of instructions that produces the correct results. These instructions are
// not expanded earlier in the backend because any pass before this can assume to
// be able to generate a load/store instruction. So this pass can only have
// passes that execute after it if no load/store instructions can be generated
// in those passes.
//===----------------------------------------------------------------------===//
#ifndef _AMDILIOEXPANSION_H_
#define _AMDILIOEXPANSION_H_

#define DEBUG_TYPE "ioexpansion"
#include "llvm/Type.h"
#include "AMDIL.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class MachineFunction;
  class AMDILMachineFunctionInfo;
  class AMDILSubtarget;
  class MachineInstr;
  class Constant;
  class TargetInstrInfo;
  class TargetRegisterInfo;
  typedef enum {
    NO_PACKING = 0,
    PACK_V2I8,
    PACK_V4I8,
    PACK_V2I16,
    PACK_V4I16,
    UNPACK_V2I8,
    UNPACK_V4I8,
    UNPACK_V2I16,
    UNPACK_V4I16,
    UNPACK_LAST
  } REG_PACKED_TYPE;
  class AMDILIOExpansionImpl
  {
    public:
      virtual ~AMDILIOExpansionImpl() {};
      bool run();
    protected:
      AMDILIOExpansionImpl(MachineFunction& mf);
      //
      // @param MI Machine instruction to check.
      // @brief checks to see if the machine instruction
      // is an I/O instruction or not.
      //
      // @return true if I/O, false otherwise.
      //
      virtual bool
        isIOInstruction(MachineInstr *MI);
      // Wrapper function that calls the appropriate I/O
      // expansion function based on the instruction type.
      virtual void
        expandIOInstruction(MachineInstr *MI);
       virtual void
        expandGlobalStore(MachineInstr *MI) = 0;
      virtual void
        expandLocalStore(MachineInstr *MI) = 0;
      virtual void
        expandRegionStore(MachineInstr *MI) = 0;
      virtual void
        expandGlobalLoad(MachineInstr *MI) = 0;
      virtual void
        expandRegionLoad(MachineInstr *MI) = 0;
      virtual void
        expandLocalLoad(MachineInstr *MI) = 0;

      virtual void expandPrivateLoad(MachineInstr *MI);
      virtual void expandPrivateStore(MachineInstr *MI);
      virtual void expandConstantLoad(MachineInstr *MI);
      virtual void expandConstantPoolLoad(MachineInstr *MI);

      bool
        isAddrCalcInstr(MachineInstr *MI);
      bool
        isExtendLoad(MachineInstr *MI);
      bool
        isHardwareRegion(MachineInstr *MI);
      bool
        isHardwareLocal(MachineInstr *MI);
      uint32_t
        getPackedReg(uint32_t &, uint32_t);
      bool
        isStaticCPLoad(MachineInstr *MI);
      bool
        isHardwareInst(MachineInstr *MI);
      uint32_t
        getMemorySize(MachineInstr *MI);
      REG_PACKED_TYPE
        getPackedID(MachineInstr *MI);
      uint32_t
        getShiftSize(MachineInstr *MI);
      uint32_t
        getPointerID(MachineInstr *MI);
      void
        expandTruncData(MachineInstr *MI, uint32_t &dataReg);
      void
        expandLoadStartCode(MachineInstr *MI, uint32_t &addyReg);
      void
        expandStoreSetupCode(MachineInstr *MI, uint32_t &addyReg, uint32_t &dataReg);
      void
        expandAddressCalc(MachineInstr *MI, uint32_t &addyReg);
      void
        expandLongExtend(MachineInstr *MI, uint32_t numComponents,
            uint32_t size, bool signedShift,
            uint32_t SrcReg, uint32_t &DstReg);
      void
        expandLongExtendSub32(MachineInstr *MI,
            unsigned SHLop, unsigned SHRop, unsigned USHRop,
            unsigned SHLimm, uint64_t SHRimm, unsigned USHRimm,
            unsigned LCRop, bool signedShift, bool vec2,
            uint32_t SrcReg, uint32_t &DstReg);
      void
        expandIntegerExtend(MachineInstr *MI, unsigned SHLop, unsigned SHRop,
            unsigned Offset, unsigned SrcReg, unsigned DstReg);
      void
        expandExtendLoad(MachineInstr *MI, uint32_t SrcReg, uint32_t &DstReg);
      virtual void
        expandPackedData(MachineInstr *MI, uint32_t SrcReg, uint32_t &DataReg)
          = 0;
       void
         emitCPInst(MachineInstr* MI, const Constant* C,
             int swizzle, bool ExtFPLoad, uint32_t &dataReg);

      MachineFunction& MF;
      MachineBasicBlock *mBB;
      const TargetMachine &TM;
      const AMDILSubtarget *mSTM;
      AMDILMachineFunctionInfo *mMFI;
      const TargetRegisterInfo *mTRI;
      const TargetInstrInfo *mTII;
      bool saveInst;
    protected:
      void
        emitStaticCPLoad(MachineInstr* MI, int swizzle, int id,
            bool ExtFPLoad, uint32_t &dataReg);
      uint32_t getCompReg(uint32_t reg,
          uint32_t subIdx0 = 0, uint32_t subIdx1 = 0);
      uint32_t get1stI32SubReg(uint32_t reg);
      uint32_t get1stI64SubReg(uint32_t reg);
      bool is32BitRegister(uint32_t reg);
      bool is64BitRegister(uint32_t reg);
      bool is128BitRegister(uint32_t reg);

      //
      // @param MI Instruction iterator that has the sample instruction
      // that needs to be taken care of.
      // @brief transforms the __amdil_sample_data function call into a
      // sample instruction in IL.
      //
      // @warning This function only works correctly if all functions get
      // inlined
      //
      virtual void
        expandImageLoad(MachineBasicBlock *BB, MachineInstr *MI);
      //
      // @param MI Instruction iterator that has the write instruction that
      // needs to be taken care of.
      // @brief transforms the __amdil_write_data function call into a
      // simple UAV write instruction in IL.
      //
      // @warning This function only works correctly if all functions get
      // inlined
      //
      virtual void
        expandImageStore(MachineBasicBlock *BB, MachineInstr *MI);
      //
      // @param MI Instruction interator that has the image parameter
      // instruction
      // @brief transforms the __amdil_get_image_params function call into
      // a copy of data from a specific constant buffer to the register
      //
      // @warning This function only works correctly if all functions get
      // inlined
      //
      virtual void
        expandImageParam(MachineBasicBlock *BB, MachineInstr *MI);

      //
      // @param MI Insturction that points to the image
      // @brief transforms __amdil_sample_data into a sequence of
      // if/else that selects the correct sample instruction.
      //
      // @warning This function is inefficient and works with no
      // inlining.
      //
      virtual void
        expandInefficientImageLoad(MachineBasicBlock *BB, MachineInstr *MI);

  private:
    void emitVectorAddressCalc(MachineInstr *MI,
                               bool is32bit,
                               bool needsSelect,
                               uint32_t &addy);

    void emitDataLoadSelect(MachineInstr *MI,
                            uint32_t DstReg,
                            uint32_t AddyReg);

    void emitComponentExtract(MachineInstr *MI,
                              unsigned src,
                              unsigned dst,
                              bool beforeInst);

    void emitVectorSwitchWrite(MachineInstr *MI,
                               bool is32bit,
                               uint32_t &addy,
                               uint32_t &data);

  }; // class AMDILIOExpansionImpl

  // Class that expands IO instructions for Evergreen and Northern
  // Island family of devices.
  class AMDILEGIOExpansionImpl : public AMDILIOExpansionImpl {
    public:
      AMDILEGIOExpansionImpl(MachineFunction& mf)
        : AMDILIOExpansionImpl(mf) {};

      virtual ~AMDILEGIOExpansionImpl() {};
    protected:
      virtual bool
        isIOInstruction(MachineInstr *MI);
      virtual void
        expandIOInstruction(MachineInstr *MI);
      virtual void
        expandGlobalStore(MachineInstr *MI);
      void
        expandLocalStore(MachineInstr *MI);
      void
        expandRegionStore(MachineInstr *MI);
      virtual void
        expandGlobalLoad(MachineInstr *MI);
      void
        expandRegionLoad(MachineInstr *MI);
      void
        expandLocalLoad(MachineInstr *MI);
      virtual bool
        isCacheableOp(MachineInstr *MI);
      virtual void
        expandPackedData(MachineInstr *MI, uint32_t SrcReg, uint32_t &DataReg)
          LLVM_OVERRIDE;
    private:
      bool
        isArenaOp(MachineInstr *MI);
      void
        expandArenaSetup(MachineInstr *MI, uint32_t &addy);
  }; // class AMDILEGIOExpansionImpl

  class AMDILEGIOExpansion : public MachineFunctionPass {
  public:
      static char ID;
  public:
      AMDILEGIOExpansion();
      virtual const char* getPassName() const;
      bool runOnMachineFunction(MachineFunction &MF);
  };
} // namespace llvm
#endif // _AMDILIOEXPANSION_H_
