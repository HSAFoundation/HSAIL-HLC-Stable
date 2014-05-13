//===------ AMDILSIPointerManager.h - Manage Pointers for CI --*- C++ -*-===//
#ifndef _AMDIL_CI_POINTER_MANAGER_H_
#define _AMDIL_CI_POINTER_MANAGER_H_
#include "AMDILSIPointerManager.h"
namespace llvm {
  class MachineFunction;

  // The pointer manager for Southern Island
  // devices. This pointer manager allocates and trackes
  // cached memory, raw resources and
  // whether multi-uav is utilized or not.
  class AMDILCIPointerManager : public MachineFunctionPass {
    public:
      AMDILCIPointerManager();
      virtual ~AMDILCIPointerManager() {};
      virtual const char *getPassName() const;
      virtual bool runOnMachineFunction(MachineFunction &F);
      virtual void getAnalysisUsage(AnalysisUsage &AU) const;
      static char ID;
    private:
  }; // class AMDILCIPointerManager
} // end llvm namespace
#endif // _AMDIL_CI_POINTER_MANAGER_H_
