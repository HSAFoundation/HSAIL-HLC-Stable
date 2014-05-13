#pragma warning(disable:4350) //behavior change: 'function template' is called instead of 'function'

#include "dwarflinker.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace DebugInformationLinking
{

DwarfLinker::DwarfLinker() : linkMode(LinkModeUndefined) {}

// clears all structures created by DwarfLinker instance during its operation
void DwarfLinker::resetLinker() {
  linker32.resetLinker();
  linker64.resetLinker();
  linkMode = LinkModeUndefined;
}

// appends ELF data to linkage
bool DwarfLinker::appendDwarfElfData(DwarfDataBlockRef data, 
                                     BRIGOffsetTy codeOffset, BRIGOffsetTy directivesOffset, 
                                     const std::vector<BRIGOffsetTy>& codeOffsetFinal,
                                     const std::vector<BRIGOffsetTy>& dirOffsetFinal) 
{
  switch(linkMode) {
  case LinkModeUndefined:
    if (linker32.isValidElfData(data)) {
      // this is 32-bit ELF container - start 32-bit linking
      linkMode = LinkModeELF32;
      return linker32.appendDwarfElfData(data, codeOffset, directivesOffset, codeOffsetFinal, dirOffsetFinal);
    }
    else if (linker64.isValidElfData(data)) {
      // this is 64-bit ELF container - start 64-bit linking
      linkMode = LinkModeELF64;
      return linker64.appendDwarfElfData(data, codeOffset, directivesOffset, codeOffsetFinal, dirOffsetFinal);
    }
    break;
  case LinkModeELF32:
    if (linker32.isValidElfData(data)) {
      // this is 32-bit ELF container - continue 32-bit linking
      return linker32.appendDwarfElfData(data, codeOffset, directivesOffset, codeOffsetFinal, dirOffsetFinal);
    }
    else if (linker64.isValidElfData(data)) {
      errs() << "Error: DWARF linker: can't link 32-bit and 64-bit ELF containers together\n";
      return false;
    }
    break;
  case LinkModeELF64:
    if (linker64.isValidElfData(data)) {
      // this is 64-bit ELF container - continue 64-bit linking
      return linker64.appendDwarfElfData(data, codeOffset, directivesOffset, codeOffsetFinal, dirOffsetFinal);
    }
    else if (linker32.isValidElfData(data)) {
      errs() << "Error: DWARF linker: can't link 32-bit and 64-bit ELF containers together\n";
      return false;
    }
    break;
  default:;
  };
  errs() << "Error: DWARF linker: unknown ELF format\n";
  return false;
}

// build linked ELF and return const reference to vector containing linked ELF
DwarfDataBlockConstRef DwarfLinker::getResultingDwarfData() {
  switch(linkMode) {
  case LinkModeELF32:
    return linker32.getResultingDwarfData();
  case LinkModeELF64:
    return linker64.getResultingDwarfData();
  default:
    assert(!"no valid data blocks were provided");
    return emptyDataBlock;
  };
}

// build linked ELF and store it in the given vector
DwarfDataBlockRef DwarfLinker::getResultingDwarfData(DwarfDataBlockRef data) {
  switch(linkMode) {
  case LinkModeELF32:
    return linker32.getResultingDwarfData(data);
  case LinkModeELF64:
    return linker64.getResultingDwarfData(data);
  default:
    assert(!"no valid data blocks were provided");
    return data;
  };
}

}; //namespace DebugInformationLinker

