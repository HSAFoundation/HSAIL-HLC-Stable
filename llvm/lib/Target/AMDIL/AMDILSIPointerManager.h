//===------ AMDILSIPointerManager.h - Manage Pointers for SI --*- C++ -*-===//
#ifndef _AMDIL_SI_POINTER_MANAGER_H_
#define _AMDIL_SI_POINTER_MANAGER_H_

#include "AMDILPointerManager.h"
namespace llvm {
  class MachineFunction;

  // The pointer manager for Southern Island
  // devices. This pointer manager allocates and trackes
  // cached memory, raw resources and
  // whether multi-uav is utilized or not.
  class AMDILSIPointerManager : public MachineFunctionPass {
    public:
      AMDILSIPointerManager();
      virtual ~AMDILSIPointerManager() {};
      virtual const char *getPassName() const;
      virtual bool runOnMachineFunction(MachineFunction &F);
      virtual void getAnalysisUsage(AnalysisUsage &AU) const;
      static char ID;
    private:
  }; // class AMDILSIPointerManager
} // end llvm namespace
#endif // _AMDIL_SI_POINTER_MANAGER_H_
