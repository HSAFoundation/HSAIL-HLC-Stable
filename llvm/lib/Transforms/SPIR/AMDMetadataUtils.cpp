//===-- AMDMetadataUtils.cpp - Manage metadata across passes -----*- C++ -*-===//
//
//
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implement utilities to update LLVM metadata such as debug
///        info and SPIR metadata when the module is modified.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/AMDMetadataUtils.h"

#include <llvm/Constants.h>
#include <llvm/Module.h>

using namespace llvm;

/// \brief Map each function in the module to its debug info
///
/// This function was copied directly from the DAE pass in LLVM 3.2
void DebugInfoManager::collectFunctionDIs(Module &M)
{
  FunctionDIs.clear();

  for (Module::named_metadata_iterator I = M.named_metadata_begin(),
       E = M.named_metadata_end(); I != E; ++I) {
    NamedMDNode &NMD = *I;
    for (unsigned MDIndex = 0, MDNum = NMD.getNumOperands();
         MDIndex < MDNum; ++MDIndex) {
      MDNode *Node = NMD.getOperand(MDIndex);
      if (!DIDescriptor(Node).isCompileUnit())
        continue;
      DICompileUnit CU(Node);
      const DIArray &SPs = CU.getSubprograms();
      for (unsigned SPIndex = 0, SPNum = SPs.getNumElements();
           SPIndex < SPNum; ++SPIndex) {
        DISubprogram SP(SPs.getElement(SPIndex));
        if (!SP.Verify())
          continue;
        if (Function *F = SP.getFunction())
          FunctionDIs[F] = SP;
      }
    }
  }
}

void DebugInfoManager::replaceFunctionDI(Function *OldF, Function *NewF) {
    FunctionDIMap::iterator DI = FunctionDIs.find(OldF);
    if (DI != FunctionDIs.end())
      DI->second.replaceFunction(NewF);
}

/// \brief Update the "global annotations" variable with the new function
///
/// LLVM metadata uses one global constant to store annotations for
/// all the functions in the module. This is an array of structs that
/// contain the relevant information. The first element of the struct
/// is a pointer to the function, stored as a bitcast expression.
void llvm::updateFunctionAnnotations(Module &M, Function *From, Function *To)
{
  GlobalVariable *GV = M.getNamedGlobal("llvm.global.annotations");
  if (!GV)
    return;

  ConstantArray *CA = dyn_cast_or_null<ConstantArray>(GV->getInitializer());
  if (!CA)
    return;

  for (ConstantArray::op_iterator OI = CA->op_begin(), OE = CA->op_end();
       OI != OE; ++OI) {
    ConstantStruct *CS = cast<ConstantStruct>(OI);

    // The first element is a bitcast of the function.
    ConstantExpr *CE = cast<ConstantExpr>(CS->getOperand(0));
    assert(Instruction::BitCast == CE->getOpcode());
    Function *F = cast<Function>(CE->getOperand(0));

    if (F != From)
      continue;
    
    // Replace the operand of the bitcast.
    CE->setOperand(0, To);
    break;
  }
}

void llvm::updateSPIRMetadata(Module &M, Function *F, Function *NewF)
{
  NamedMDNode *Kernels = M.getNamedMetadata("opencl.kernels");
  assert(Kernels);

  for (unsigned i = 0, e = Kernels->getNumOperands(); i != e; ++i) {
    MDNode *Node = Kernels->getOperand(i);
    Value *V = Node->getOperand(0);
    assert(isa<Function>(V));
    if (F != V)
      continue;
    Node->replaceOperandWith(0, NewF);
    return;
  }
}

bool llvm::matchName(const MDNode *Node, StringRef Name) {
  const MDString *NodeName = cast<MDString>(Node->getOperand(0));
  StringRef NameRef = NodeName->getString();
  return NameRef.equals(Name);
}
