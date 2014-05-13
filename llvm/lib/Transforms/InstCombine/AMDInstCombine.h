//
// Copyright (c) 2011 Advanced Micro Devices, Inc. All rights reserved.
//

#ifndef INSTCOMBINE_AMDINSTCOMBINE_H
#define INSTCOMBINE_AMDINSTCOMBINE_H

#include "llvm/Constants.h"

namespace llvm {
  class Function;
  class DataLayout;
  class BinaryOperator;

/*
   AMDInstCombiner is designed to work only during InstCombiner::runOnFunction()
*/
class AMDInstCombiner {
private:
  Function    *FP;   // Initialized at the begining of runOnFunction()
  DataLayout  *DL;
  IRBuilder<> *MyIRBuilder;

public:
  AMDInstCombiner() :
    FP(0), 
    DL(0),
    MyIRBuilder(0), 
    IsGPU(false),
    EnableFDiv2FMul(false)
  { }

  ~AMDInstCombiner() {
    finalize();
  }

  void initialize(Function *aFunc, DataLayout *aDL);
  void finalize();

  Value *SimplifyFDiv(BinaryOperator *BI);

private:

  bool IsGPU;
  bool EnableFDiv2FMul;

  bool isInfinity(ConstantFP *CF) { return CF->getValueAPF().isInfinity(); }

  bool isFloatSubnormal(ConstantFP *CF) {
    uint32_t ubits = static_cast<uint32_t>(
      CF->getValueAPF().bitcastToAPInt().getZExtValue());
    return ((ubits & 0x7f800000) == 0) && ((ubits & 0x007fffff) != 0);
  }
}; 

} // end of namespace llvm

#endif
