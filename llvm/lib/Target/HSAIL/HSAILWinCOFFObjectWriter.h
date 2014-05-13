//===-- HSAILWinCOFFObjectWriter.h - HSAIL Win COFF Writer ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// !!!
// There is NO specific `MCTargetDesc' as other targets have from LLVM 3.0
// onwards. So this file is one of the dirty hacks to overcome the issue.
// That is, there are no TargetDesc related data like relocation types, etc.
#ifndef _HSAIL_WIN_COFF_OBJECT_WRITER_H_
#define _HSAIL_WIN_COFF_OBJECT_WRITER_H_

#include "HSAILFixupKinds.h"
#include "llvm/MC/MCWinCOFFObjectWriter.h"
#include "llvm/Support/COFF.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace llvm {
  class MCObjectWriter;
}
  
namespace {
  class HSAILWinCOFFObjectWriter : public MCWinCOFFObjectTargetWriter {
    const bool Is64Bit;
    
  public:
    HSAILWinCOFFObjectWriter(bool Is64Bit_);
    ~HSAILWinCOFFObjectWriter();

    virtual unsigned getRelocType(unsigned FixupKind) const;
  };
}
  
HSAILWinCOFFObjectWriter::HSAILWinCOFFObjectWriter(bool Is64Bit_)
  : MCWinCOFFObjectTargetWriter(Is64Bit_ ? COFF::IMAGE_FILE_MACHINE_AMD64 :
                                COFF::IMAGE_FILE_MACHINE_I386),
    Is64Bit(Is64Bit_) {}
  
HSAILWinCOFFObjectWriter::~HSAILWinCOFFObjectWriter() {}

unsigned HSAILWinCOFFObjectWriter::getRelocType(unsigned FixupKind) const {
  // Currently I did not find any Reloc Type for HSAIL,
  // so return 0 - which means none.
  return 0;
}

MCObjectWriter *createHSAILWinCOFFObjectWriter(raw_ostream &OS,
                                                   bool Is64Bit) {
  MCWinCOFFObjectTargetWriter *MOTW = new HSAILWinCOFFObjectWriter(Is64Bit);
  return createWinCOFFObjectWriter(MOTW, OS);
}                                        
#endif