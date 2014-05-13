#ifndef LLVM_ANALYSIS_PRAGMA_INFO_H
#define LLVM_ANALYSIS_PRAGMA_INFO_H

#include "llvm/Metadata.h"
#include "llvm/BasicBlock.h"
#include "llvm/Constants.h"

namespace llvm {

class Loop;

/*
  LoopPragmaInfo

  Loop pragmas are represented by LoopPragmaInfo. The front end processes
  all loop pragmas for a loop to generate a single LoopPragmaInfo, and writes
  that LoopPragmaInfo as metadata to LLVM IR. This metadata, with metadata ID
  !LoopPragmaInfo, is attached to the terminator (branch inst) of the loop's
  header basic block.

  For example,

    header:
        br <sth> 
         
    LoopPragmaInfo can be attached to header's br, or

    header:
         br <sth> !LoopPragmaInfo !0

    where !0 is the metadata for LoopPragmaInfo like the following:

      !0 = metadata !{
          metadata string,  ;; name of source file containing loop pragmas
          i32,              ;; Line Number of the loop for which pragmas apply.
          i32,              ;; Flags.
          i32,              ;; unroll_count
      }
*/
class LoopPragmaInfo {
private:
  LoopPragmaInfo (const LoopPragmaInfo &);	// DO NOT IMPLEMENT

  enum {
    PragmaUnrollCount         = 0x1, // 1: Has pragam unroll;   0: otherwise
    PragmaUnrollCountUsed     = 0x2  // 1: Pragma unroll used;  0: otherwise.
  };

  // For debugging / analysis purpose.
  StringRef  SourceFileName;
  unsigned   LoopLineNo;

  // Each bit of 'Flags' is defined by the above enum.
  unsigned Flags;
  bool IsFlagsChanged;

  // The unroll_count for #pragma unroll 
  unsigned UnrollCount;

  void setHasBit(unsigned bit, unsigned value) {
      Flags = (Flags & ~bit) | value;
      IsFlagsChanged = true;
  }
  bool hasBit(unsigned bit) const { return (Flags & bit) == bit; }

public:
  explicit LoopPragmaInfo (MDNode *LPIM);

  StringRef getSourceFileName() const { return SourceFileName; }
  unsigned getLoopLineNo () const { return LoopLineNo; }
  unsigned getPragmaUnrollCount() const { return UnrollCount; }
  void    setMetadata(Loop* TheLoop);

  bool hasPragmaUnrollCount()     { return hasBit(PragmaUnrollCount); }
  bool hasPragmaUnrollCountUsed() { return hasBit(PragmaUnrollCountUsed); }
  void setHasPragmaUnrollCountUsed (unsigned isSet) {
    setHasBit(PragmaUnrollCountUsed, isSet ? PragmaUnrollCountUsed : 0);
  }

  static void initLoopPragmaInfo(Loop *AnyLoop);
};


}  // End llvm namespace
  
#endif
