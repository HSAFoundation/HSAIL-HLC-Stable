//===------------------------------------------------------*- C++ -*-------===//

#ifndef _AMDIL_CIIO_EXPANSION_H_
#define _AMDIL_CIIO_EXPANSION_H_
#include "AMDILSIIOExpansion.h"

namespace llvm {
  // Class that expands IO instructions for the CI family of devices.
  // This function implements expansion of flat load/store and everything
  // else is normal.
  class AMDILCIIOExpansionImpl : public AMDILSIIOExpansionImpl {
    public:
      AMDILCIIOExpansionImpl(MachineFunction& mf)
        : AMDILSIIOExpansionImpl(mf) {};
      virtual ~AMDILCIIOExpansionImpl() {};
    protected:
      virtual bool
        isIOInstruction(MachineInstr *MI);
      virtual void
        expandIOInstruction(MachineInstr *MI);
      void
        expandFlatStore(MachineInstr *MI);
      void
        expandFlatLoad(MachineInstr *MI);
  }; // class AMDILSIIOExpansionImpl

  class AMDILCIIOExpansion : public MachineFunctionPass {
  public:
      static char ID;
  public:
      AMDILCIIOExpansion();
      virtual const char* getPassName() const;
      bool runOnMachineFunction(MachineFunction &MF);
  };
} // namespace llvm
#endif // _AMDIL_CIIO_EXPANSION_H_
