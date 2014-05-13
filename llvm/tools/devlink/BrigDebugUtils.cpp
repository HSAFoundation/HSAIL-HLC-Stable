#pragma warning(disable:4350) //behavior change: 'function template' is called instead of 'function'

#include "Brig.h"
#include "BrigDebugUtils.h"
#include "HSAILBrigantine.h"

namespace DebugInformationLinking {

  // TODO_HSA(3) move this code to libHSAIL or to libBRIGDWARF because it is shared with HSAILasm

  // search for a BlockString with a name of "hsa_dwarf_debug", this marks
  // the beginning of a group of BlockNumerics which make up the elf container
  //
  // Input: d: any directive inside the debug section
  //        d_end: directive at the end of debug section
  // Returns: the directive following the matching BlockString (the beginning of
  //          the BlockNumerics), or the end directive of the section if no
  //          matching BlockString is found
  //
  Directive findHsaDwarfDebugBlock( Directive d, Directive d_end )
  {
    for ( ; d != d_end ; d = d.next() )
    {
      if  ( d.brig()->kind == Brig::BrigEDirectiveBlockString )
      {
        BlockString bStr( d );
        if ( std::string( bStr.name() ) == "hsa_dwarf_debug" )
           return d.next();
        }
    }
    return d;
  }

  // TODO_HSA(3) move this code to libHSAIL or to libBRIGDWARF because it is shared with HSAILasm
  // extracts the BlockNumeric data, appending it to the accumulated dbg bytes
  //
  // Input: a BlockNumeric directive (d)
  //        a directive denoting the end of debug section (d_end)
  //        the dbg info data container
  //
  // Returns: the first non-BlockNumeric directive following "d" or the end
  //          of the section
  //
  Directive extractDataFromHsaDwarfDebugBlock( Directive d,
                                               Directive d_end,
                                               std::vector<unsigned char>  & dbgElfBytes )
  {
      for ( ; d != d_end && (d.brig()->kind == Brig::BrigEDirectiveBlockNumeric); d = d.next() )
      {
          BlockNumeric dn( d );
          assert( dn.type() == Brig::Brigb8 );
          dbgElfBytes.insert( dbgElfBytes.end(), dn.u8().begin(), dn.u8().end() );
      }
      return d;
  }

  // TODO_HSA(3) move this code to libHSAIL or to libBRIGDWARF because it is shared with HSAILasm
  // The hsa BRIG DWARF debug information is an ELF disk-image bytestream stored
  // in a block in the Brig container's .debug section with a BlockString value
  // of "hsa_dwarf_debug".
  // Find this block, concatenate the byteValues of all the
  // BlockNumerics (all are Brig::Brigb8), .
  //
  // format of hsa_dwarf_debug blocks (required!):
  // BlockStart, BlockString ("hsa_dwarf_debug"), BlockNumeric (type = b8), ..., BlockEnd
  //
  // Any other formats/arrangements of blocks are skipped.
  // 
  void loadDwarfDebugData(BrigContainer& from, std::vector<unsigned char>& dbgElfBytes)
  {
      for ( Directive d = from.debugChunks().begin(), d_end = from.debugChunks().end(); d != d_end; )
      {
          d = findHsaDwarfDebugBlock( d, d_end );
          d = extractDataFromHsaDwarfDebugBlock( d, d_end, dbgElfBytes );
      }
  }

  // TODO_HSA(3) move this code to libHSAIL or to libBRIGDWARF because it is shared with HSAILasm
  bool storeDwarfDebugData(BrigContainer& c, const std::vector<unsigned char>& dbgElfBytes)
  {
      HSAIL_ASM::Brigantine b(c);
      b.storeDWARF(dbgElfBytes);
      return true;
  }

}; //namespace DebugInformationLinker
