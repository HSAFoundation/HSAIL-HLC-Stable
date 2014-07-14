//===-- RuntimeDyldELF.h - Run-time dynamic linker for MC-JIT ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// ELF support for MC-JIT runtime dynamic linker.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_RUNTIME_DYLD_ELF_H
#define LLVM_RUNTIME_DYLD_ELF_H

#include "RuntimeDyldImpl.h"

using namespace llvm;

namespace llvm {

namespace {
  // Helper for extensive error checking in debug builds.
  error_code Check(error_code Err) {
    if (Err) {
      report_fatal_error(Err.message());
    }
    return Err;
  }
} // end anonymous namespace

class RuntimeDyldELF : public RuntimeDyldImpl {
protected:
  void resolveX86_64Relocation(const SectionEntry &Section,
                               uint64_t Offset,
                               uint64_t Value,
                               uint32_t Type,
                               int64_t  Addend,
                               uint64_t SymOffset);


  void resolveX86Relocation(const SectionEntry &Section,
                            uint64_t Offset,
                            uint32_t Value,
                            uint32_t Type,
                            int32_t Addend,
                            uint32_t SymOffset);

  void resolveARMRelocation(const SectionEntry &Section,
                            uint64_t Offset,
                            uint32_t Value,
                            uint32_t Type,
                            int32_t Addend);

  void resolveMIPSRelocation(const SectionEntry &Section,
                             uint64_t Offset,
                             uint32_t Value,
                             uint32_t Type,
                             int32_t Addend);

  void resolvePPC64Relocation(const SectionEntry &Section,
                              uint64_t Offset,
                              uint64_t Value,
                              uint32_t Type,
                              int64_t Addend);

  virtual void resolveRelocation(const SectionEntry &Section,
                                 uint64_t Offset,
                                 uint64_t Value,
                                 uint32_t Type,
                                 int64_t Addend,
                                 uint64_t SymOffset);

  virtual void resolveRelocation(const SectionEntry &Section,
                                 uint64_t Offset,
                                 uint64_t Value,
                                 uint32_t Type,
                                 int64_t Addend);

  virtual void processRelocationRef(const ObjRelocationInfo &Rel,
                                    ObjectImage &Obj,
                                    ObjSectionToIDMap &ObjSectionToID,
                                    const SymbolTableMap &Symbols,
                                    StubMap &Stubs);

  unsigned getCommonSymbolAlignment(const SymbolRef &Sym);

  virtual ObjectImage *createObjectImage(ObjectBuffer *InputBuffer);

  uint64_t findPPC64TOC() const;
  void findOPDEntrySection(ObjectImage &Obj,
                           ObjSectionToIDMap &LocalSections,
                           RelocationValueRef &Rel);

  uint64_t findGOTEntry(uint64_t LoadAddr, uint64_t Offset);
  size_t getGOTEntrySize();

  virtual void updateGOTEntries(StringRef Name, uint64_t Addr);

  SmallVector<RelocationValueRef, 2>  GOTEntries;
  unsigned GOTSectionID;

 public:
  RuntimeDyldELF(RTDyldMemoryManager *mm) : RuntimeDyldImpl(mm),
                                            GOTSectionID(0)
                                          {}

  virtual void finalizeLoad();
  virtual ~RuntimeDyldELF();

  bool isCompatibleFormat(const ObjectBuffer *Buffer) const;
};

} // end namespace llvm

#endif