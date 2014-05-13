//===- CloneModule.cpp - Clone an entire module ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the CloneModule interface which makes a copy of an
// entire module.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Module.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Constant.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
using namespace llvm;

/// CloneModule - Return an exact copy of the specified module.  This is not as
/// easy as it might seem because we have to worry about making copies of global
/// variables and functions, and making their (initializers and references,
/// respectively) refer to the right globals.
///
Module *llvm::CloneModule(const Module *M) {
  // Create the value map that maps things from the old module over to the new
  // module.
  ValueToValueMapTy VMap;
  return CloneModule(M, VMap);
}

Module *llvm::CloneModule(const Module *M, ValueToValueMapTy &VMap) {
  // First off, we need to create the new module.
  Module *New = new Module(M->getModuleIdentifier(), M->getContext());
  New->setDataLayout(M->getDataLayout());
  New->setTargetTriple(M->getTargetTriple());
  New->setModuleInlineAsm(M->getModuleInlineAsm());
   
  // Copy all of the dependent libraries over.
  for (Module::lib_iterator I = M->lib_begin(), E = M->lib_end(); I != E; ++I)
    New->addLibrary(*I);

  // Loop over all of the global variables, making corresponding globals in the
  // new module.  Here we add them to the VMap and to the new Module.  We
  // don't worry about attributes or initializers, they will come later.
  //
  for (Module::const_global_iterator I = M->global_begin(), E = M->global_end();
       I != E; ++I) {
    GlobalVariable *GV = new GlobalVariable(*New, 
                                            I->getType()->getElementType(),
                                            I->isConstant(), I->getLinkage(),
                                            (Constant*) 0, I->getName(),
                                            (GlobalVariable*) 0,
                                            I->getThreadLocalMode(),
                                            I->getType()->getAddressSpace());
    GV->copyAttributesFrom(I);
    VMap[I] = GV;
  }

  // Loop over the functions in the module, making external functions as before
  for (Module::const_iterator I = M->begin(), E = M->end(); I != E; ++I) {
    Function *NF =
      Function::Create(cast<FunctionType>(I->getType()->getElementType()),
                       I->getLinkage(), I->getName(), New);
    NF->copyAttributesFrom(I);
    VMap[I] = NF;
  }

  // Loop over the aliases in the module
  for (Module::const_alias_iterator I = M->alias_begin(), E = M->alias_end();
       I != E; ++I) {
    GlobalAlias *GA = new GlobalAlias(I->getType(), I->getLinkage(),
                                      I->getName(), NULL, New);
    GA->copyAttributesFrom(I);
    VMap[I] = GA;
  }
  
  // Now that all of the things that global variable initializer can refer to
  // have been created, loop through and copy the global variable referrers
  // over...  We also set the attributes on the global now.
  //
  for (Module::const_global_iterator I = M->global_begin(), E = M->global_end();
       I != E; ++I) {
    GlobalVariable *GV = cast<GlobalVariable>(VMap[I]);
    if (I->hasInitializer())
      GV->setInitializer(MapValue(I->getInitializer(), VMap));
  }

  // Similarly, copy over function bodies now...
  //
  for (Module::const_iterator I = M->begin(), E = M->end(); I != E; ++I) {
    Function *F = cast<Function>(VMap[I]);
    if (!I->isDeclaration()) {
      Function::arg_iterator DestI = F->arg_begin();
      for (Function::const_arg_iterator J = I->arg_begin(); J != I->arg_end();
           ++J) {
        DestI->setName(J->getName());
        VMap[J] = DestI++;
      }

      SmallVector<ReturnInst*, 8> Returns;  // Ignore returns cloned.
      CloneFunctionInto(F, I, VMap, /*ModuleLevelChanges=*/true, Returns);
    }
  }

  // And aliases
  for (Module::const_alias_iterator I = M->alias_begin(), E = M->alias_end();
       I != E; ++I) {
    GlobalAlias *GA = cast<GlobalAlias>(VMap[I]);
    if (const Constant *C = I->getAliasee())
      GA->setAliasee(MapValue(C, VMap));
  }

  // And named metadata....
  for (Module::const_named_metadata_iterator I = M->named_metadata_begin(),
         E = M->named_metadata_end(); I != E; ++I) {
    const NamedMDNode &NMD = *I;
    NamedMDNode *NewNMD = New->getOrInsertNamedMetadata(NMD.getName());
    for (unsigned i = 0, e = NMD.getNumOperands(); i != e; ++i)
      NewNMD->addOperand(MapValue(NMD.getOperand(i), VMap));
  }

  return New;
}

#if defined(AMD_OPENCL_MCWHOOK) || 1

bool llvm::CopyModule(Module& dest, Module* src)  {
  if(!src) return false;
  if( (dest.getDataLayout() != src->getDataLayout())
      || (dest.getTargetTriple() != src->getTargetTriple()) 
      || (dest.getModuleInlineAsm() != src->getModuleInlineAsm()) ) {
    delete src;
    return false;
  } 

  ValueToValueMapTy VMap;
  // clean module
  dest.dropAllReferences();
  std::vector<Function *> functions;
  for(Module::iterator I = dest.begin(), E = dest.end(); I != E; ++I)
     functions.push_back(I);

  for(std::vector<Function *>::iterator F=functions.begin(), Fe=functions.end(); F!=Fe; ++F) {
    (*F)->eraseFromParent();
  }
 
  std::vector<GlobalVariable *> globals;
  for(Module::global_iterator I = dest.global_begin(), E = dest.global_end(); I != E; ++I)
     globals.push_back(I);

  for(std::vector<GlobalVariable *>::iterator G=globals.begin(), Ge=globals.end(); G!=Ge; ++G) {
     (*G)->eraseFromParent();
  }

  std::vector<GlobalAlias*> aliases; 
  for(Module::alias_iterator I = dest.alias_begin(), E = dest.alias_end(); I != E; ++I)
     aliases.push_back(I);

 
  for(std::vector<GlobalAlias *>::iterator A=aliases.begin(), Ae=aliases.end(); A!=Ae; ++A) {
     (*A)->eraseFromParent();
  }
  
#if 0
  TypeSymbolTable& dstTST = dest.getTypeSymbolTable();
  // Copy all of the type symbol table entries over.
  const TypeSymbolTable &TST = src->getTypeSymbolTable();
  for (TypeSymbolTable::const_iterator TI = TST.begin(), TE = TST.end(); 
       TI != TE; ++TI)
    if(dest.addTypeName(TI->first, TI->second)) {
      dstTST.remove(dstTST.find(TI->first));
      dest.addTypeName(TI->first, TI->second);
    }
#endif
  
  // Copy all of the dependent libraries over.
  for (Module::lib_iterator I = src->lib_begin(), E = src->lib_end(); I != E; ++I)
    dest.addLibrary(*I);

  // Loop over all of the global variables, making corresponding globals in the
  // new module.  Here we add them to the VMap and to the new Module.  We
  // don't worry about attributes or initializers, they will come later.
  //
  for (Module::const_global_iterator I = src->global_begin(), E = src->global_end();
       I != E; ++I) {
    GlobalVariable *GV = new GlobalVariable(dest, 
                                            I->getType()->getElementType(),
                                            false,
                                            GlobalValue::ExternalLinkage, 0,
                                            I->getName(), 0 , GlobalVariable::NotThreadLocal,
                                            I->getType()->getAddressSpace());
    GV->setAlignment(I->getAlignment());
    VMap[I] = GV;
  }

  // Loop over the functions in the module, making external functions as before
  for (Module::const_iterator I = src->begin(), E = src->end(); I != E; ++I) {
    Function *NF =
      Function::Create(cast<FunctionType>(I->getType()->getElementType()),
                       GlobalValue::ExternalLinkage, I->getName(), &dest);
    NF->copyAttributesFrom(I);
    VMap[I] = NF;
  }

  // Loop over the aliases in the module
  for (Module::const_alias_iterator I = src->alias_begin(), E = src->alias_end();
       I != E; ++I)
    VMap[I] = new GlobalAlias(I->getType(), GlobalAlias::ExternalLinkage,
                                  I->getName(), NULL, &dest);
  
  // Now that all of the things that global variable initializer can refer to
  // have been created, loop through and copy the global variable referrers
  // over...  We also set the attributes on the global now.
  //
  for (Module::const_global_iterator I = src->global_begin(), E = src->global_end();
       I != E; ++I) {
    GlobalVariable *GV = cast<GlobalVariable>(VMap[I]);
    if (I->hasInitializer())
      GV->setInitializer(cast<Constant>(MapValue(I->getInitializer(),
                                                 VMap, RF_None)));
    GV->setLinkage(I->getLinkage());
    GV->setThreadLocal(I->isThreadLocal());
    GV->setConstant(I->isConstant());
    if(I->hasSection()) GV->setSection(I->getSection());
  }

  // Similarly, copy over function bodies now...
  //
  for (Module::const_iterator I = src->begin(), E = src->end(); I != E; ++I) {
    Function *F = cast<Function>(VMap[I]);
    if (!I->isDeclaration()) {
      Function::arg_iterator DestI = F->arg_begin();
      for (Function::const_arg_iterator J = I->arg_begin(); J != I->arg_end();
           ++J) {
        DestI->setName(J->getName());
        VMap[J] = DestI++;
      }

      SmallVector<ReturnInst*, 8> Returns;  // Ignore returns cloned.
      CloneFunctionInto(F, I, VMap, /*ModuleLevelChanges=*/true, Returns);
    }

    F->setLinkage(I->getLinkage());
  }

  // And aliases
  for (Module::const_alias_iterator I = src->alias_begin(), E = src->alias_end();
       I != E; ++I) {
    GlobalAlias *GA = cast<GlobalAlias>(VMap[I]);
    GA->setLinkage(I->getLinkage());
    if (const Constant* C = I->getAliasee())
      GA->setAliasee(cast<Constant>(MapValue(C, VMap, RF_None)));
  }

  // And named metadata....
  for (Module::const_named_metadata_iterator I = src->named_metadata_begin(),
         E = src->named_metadata_end(); I != E; ++I) {
    const NamedMDNode &NMD = *I;
    NamedMDNode *NewNMD = dest.getOrInsertNamedMetadata(NMD.getName());
    for (unsigned i = 0, e = NMD.getNumOperands(); i != e; ++i)
      NewNMD->addOperand(cast<MDNode>(MapValue(NMD.getOperand(i), VMap,
                                               RF_None)));
  }

  delete src;
  return true;
} //CopyModule

#endif
