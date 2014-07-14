//===----------- AMDILIOExpansion.h - SI IO Expansion Pass ------*- C++ -*-===//
#ifndef _AMDIL_SIIO_EXPANSION_H_
#define _AMDIL_SIIO_EXPANSION_H_

#include "AMDILIOExpansion.h"

namespace llvm {
  // Class that expands IO instructions for the SI family of devices.
  // The Global Load/Store functions need to be overloaded from the EG
  // class as an arena is not a valid operation on SI, but are valid
  // on the EG/NI devices.
  class AMDILSIIOExpansionImpl : public AMDILEGIOExpansionImpl {
    public:
      AMDILSIIOExpansionImpl(MachineFunction& MF)
        : AMDILEGIOExpansionImpl(MF) {};
      virtual ~AMDILSIIOExpansionImpl() {};
    protected:
      virtual bool isIOInstruction(MachineInstr *MI);
      virtual void expandIOInstruction(MachineInstr *MI);
      virtual void expandGlobalStore(MachineInstr *MI) LLVM_OVERRIDE;
      virtual void expandGlobalLoad(MachineInstr *MI) LLVM_OVERRIDE;
      virtual void expandConstantLoad(MachineInstr *MI) LLVM_OVERRIDE;
    virtual void expandPrivateLoad(MachineInstr *MI) LLVM_OVERRIDE;
    virtual void expandPrivateStore(MachineInstr *MI) LLVM_OVERRIDE;

     virtual bool isCacheableOp(MachineInstr* MI);
  }; // class AMDILSIIOExpansionImpl

  class AMDILSIIOExpansion : public MachineFunctionPass {
  public:
      static char ID;
  public:
      AMDILSIIOExpansion();
      virtual const char *getPassName() const;
      bool runOnMachineFunction(MachineFunction &MF);
  };
} // namespace llvm

#endif // _AMDIL_SIIO_EXPANSION_H_
