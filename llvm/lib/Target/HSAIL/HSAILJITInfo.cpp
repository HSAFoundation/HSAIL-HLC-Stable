//=== HSAILJITInfo.cpp - Implement the JIT interfaces for the HSAIL target ===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the JIT interfaces for the HSAIL target.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "jit"
#include "HSAILJITInfo.h"
#include "HSAILSubtarget.h"
#include "HSAILTargetMachine.h"
#include "llvm/Function.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Valgrind.h"
#include <cstdlib>
#include <cstring>
using namespace llvm;

// Determine the platform we're running on
#if defined (__x86_64__) || defined (_M_AMD64) || defined (_M_X64)
# define HSAIL_64_JIT
#elif defined(__i386__) || defined(i386) || defined(_M_IHSAIL)
# define HSAIL_32_JIT
#endif

void HSAILJITInfo::replaceMachineCodeForFunction(void *Old, void *New)
{
  assert(!"When do we hit this?");
}


// Get the ASMPREFIX for the current host.  This is often '_'.
#ifndef __USER_LABEL_PREFIX__
#define __USER_LABEL_PREFIX__
#endif
#define GETASMPREFIX2(X) #X
#define GETASMPREFIX(X) GETASMPREFIX2(X)
#define ASMPREFIX GETASMPREFIX(__USER_LABEL_PREFIX__)

// For ELF targets, use a .size and .type directive, to let tools
// know the extent of functions defined in assembler.
#if defined(__ELF__)
# define SIZE(sym) ".size " #sym ", . - " #sym "\n"
# define TYPE_FUNCTION(sym) ".type " #sym ", @function\n"
#else
# define SIZE(sym)
# define TYPE_FUNCTION(sym)
#endif

// Provide a convenient way for disabling usage of CFI directives.
// This is needed for old/broken assemblers (for example, gas on
// Darwin is pretty old and doesn't support these directives)
// FIXME: Disable this until we really want to use it. Also, we will
//        need to add some workarounds for compilers, which support
//        only subset of these directives.
# define CFI(x)

// Provide a wrapper for HSAILCompilationCallback2 that saves non-traditional
// callee saved registers, for the fastcc calling convention.
extern "C" {
#if defined(HSAIL_64_JIT)
# ifndef _MSC_VER
  // No need to save EAX/EDX for HSAIL-64.
  void HSAILCompilationCallback(void);
# else
  // No inline assembler support on this platform. The routine is in external
  // file.
  void HSAILCompilationCallback();
# endif
#elif defined (HSAIL_32_JIT)
# ifndef _MSC_VER
  void HSAILCompilationCallback(void);
  // Same as HSAILCompilationCallback but also saves XMM argument registers.
  void HSAILCompilationCallback_SSE(void);
# else
  void HSAILCompilationCallback2(intptr_t *StackPtr, intptr_t RetAddr);

  _declspec(naked) void HSAILCompilationCallback(void)
  {
    assert(!"When do we hit this?");
  }

# endif // _MSC_VER

#else // Not an i386 host
  void HSAILCompilationCallback()
  {
    assert(!"When do we hit this?");
  }
#endif
}

/// HSAILCompilationCallback2 - This is the target-specific function invoked by
/// the function stub when we did not know the real target of a call.  This
/// function must locate the start of the stub or call site and pass it into the
/// JIT compiler function.
extern "C" {
#if !(defined (HSAIL_64_JIT) && defined(_MSC_VER))
 // the following function is called only from this translation unit,
 // unless we are under 64bit Windows with MSC, where there is
 // no support for inline assembly
static
#endif
void LLVM_ATTRIBUTE_USED
HSAILCompilationCallback2(intptr_t *StackPtr, intptr_t RetAddr)
{
  assert(!"When do we hit this?");
}
}

TargetJITInfo::LazyResolverFn
HSAILJITInfo::getLazyResolverFunction(JITCompilerFn F)
{
  assert(!"When do we hit this?");
  return TargetJITInfo::LazyResolverFn();
}

HSAILJITInfo::HSAILJITInfo(HSAILTargetMachine &tm) : TM(tm) {}

void*
HSAILJITInfo::emitGlobalValueIndirectSym(const GlobalValue* GV,
                                         void *ptr,
                                         JITCodeEmitter &JCE)
{
  assert(!"When do we hit this?");
  return NULL;
}

TargetJITInfo::StubLayout
HSAILJITInfo::getStubLayout()
{
  assert(!"When do we hit this?");
  return TargetJITInfo::StubLayout();
}

void*
HSAILJITInfo::emitFunctionStub(const Function* F,
                               void *Target,
                               JITCodeEmitter &JCE)
{
  assert(!"When do we hit this?");
  return NULL;
}

/// getPICJumpTableEntry - Returns the value of the jumptable entry for the
/// specific basic block.
uintptr_t
HSAILJITInfo::getPICJumpTableEntry(uintptr_t BB, uintptr_t Entry)
{
  assert(!"When do we hit this?");
  return 0;
}

/// relocate - Before the JIT can run a block of code that has been emitted,
/// it must rewrite the code to contain the actual addresses of any
/// referenced global symbols.
void
HSAILJITInfo::relocate(void *Function,
                       MachineRelocation *MR,
                       unsigned NumRelocs,
                       unsigned char* GOTBase)
{
  assert(!"When do we hit this?");
}

char*
HSAILJITInfo::allocateThreadLocalMemory(size_t size)
{
  assert(!"When do we hit this?");
  return NULL;
}

bool
HSAILJITInfo::hasCustomConstantPool() const
{
  assert(!"When do we hit this?");
  return false;
}

bool
HSAILJITInfo::hasCustomJumpTables() const
{
  assert(!"When do we hit this?");
  return false;
}

bool
HSAILJITInfo::allocateSeparateGVMemory() const
{
  assert(!"When do we hit this?");
  return false;
}
