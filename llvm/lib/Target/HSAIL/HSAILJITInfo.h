//== HSAILJITInfo.h - HSAIL implementation of the JIT interface  -*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the HSAIL implementation of the TargetJITInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef _HSAIL_JIT_INFO_H_
#define _HSAIL_JIT_INFO_H_

#include "llvm/Function.h"
#include "llvm/CodeGen/JITCodeEmitter.h"
#include "llvm/Target/TargetJITInfo.h"

namespace llvm {
  class HSAILTargetMachine;
  class HSAILSubtarget;

  class HSAILJITInfo : public TargetJITInfo {
    HSAILTargetMachine &TM;
    const HSAILSubtarget *Subtarget;
    uintptr_t PICBase;
    char* TLSOffset;
  public:
    explicit HSAILJITInfo(HSAILTargetMachine &tm);

    /// replaceMachineCodeForFunction - Make it so that calling the function
    /// whose machine code is at OLD turns into a call to NEW, perhaps by
    /// overwriting OLD with a branch to NEW.  This is used for self-modifying
    /// code.
    ///
    virtual void
    replaceMachineCodeForFunction(void *Old, void *New);

    /// emitGlobalValueIndirectSym - Use the specified JITCodeEmitter object
    /// to emit an indirect symbol which contains the address of the specified
    /// ptr.
    virtual void*
    emitGlobalValueIndirectSym(const GlobalValue* GV,
                               void *ptr,
                               JITCodeEmitter &JCE);

    // getStubLayout - Returns the size and alignment of the largest call stub
    // on HSAIL.
    virtual StubLayout
    getStubLayout();

    /// emitFunctionStub - Use the specified JITCodeEmitter object to emit a
    /// small native function that simply calls the function at the specified
    /// address.
    virtual void*
    emitFunctionStub(const Function* F, void *Target, JITCodeEmitter &JCE);

    /// getPICJumpTableEntry - Returns the value of the jumptable entry for the
    /// specific basic block.
    virtual uintptr_t
    getPICJumpTableEntry(uintptr_t BB, uintptr_t JTBase);

    /// getLazyResolverFunction - Expose the lazy resolver to the JIT.
    virtual LazyResolverFn
    getLazyResolverFunction(JITCompilerFn);

    /// relocate - Before the JIT can run a block of code that has been emitted,
    /// it must rewrite the code to contain the actual addresses of any
    /// referenced global symbols.
    virtual void
    relocate(void *Function,
             MachineRelocation *MR,
             unsigned NumRelocs,
             unsigned char* GOTBase);

    /// allocateThreadLocalMemory - Each target has its own way of
    /// handling thread local variables. This method returns a value only
    /// meaningful to the target.
    virtual char*
    allocateThreadLocalMemory(size_t size);

   /// hasCustomConstantPool - Allows a target to specify that constant
    /// pool address resolution is handled by the target.
    virtual bool
    hasCustomConstantPool() const;

    /// hasCustomJumpTables - Allows a target to specify that jumptables
    /// are emitted by the target.
    virtual bool
    hasCustomJumpTables() const;

    /// allocateSeparateGVMemory - If true, globals should be placed in
    /// separately allocated heap memory rather than in the same
    /// code memory allocated by JITCodeEmitter.
    virtual bool
    allocateSeparateGVMemory() const;
  };
}

#endif // _HSAIL_JIT_INFO_H_
