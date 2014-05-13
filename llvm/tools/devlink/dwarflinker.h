#ifndef DWARFLINKER_H
#define DWARFLINKER_H

#include <vector>
#include <map>
#include <string>

//ELF support
#include "llvm/Support/ELF.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm::ELF;

#include "elfutils.h"

//TODO_HSA(4) make different verbosity levels
//#define DWARF_LINKER_TRACE printf  
#define DWARF_LINKER_TRACE(format, ...)

typedef std::vector<unsigned char>& DwarfDataBlockRef;
typedef const std::vector<unsigned char>& DwarfDataBlockConstRef;

namespace DebugInformationLinking
{

  template<typename BRIGOffsetTy, int bitness> class SpecificDwarfLinker : public ElfRoutineSet<bitness>
  {
  private:
    typedef ElfRoutineSet<bitness> ELF_RoutineSet;
    typedef typename ELF_RoutineSet::ElfHeader ElfHeader;
    typedef typename ELF_RoutineSet::SectionHeader SectionHeader;
    typedef typename ELF_RoutineSet::RelRecord RelRecord;
    typedef typename ELF_RoutineSet::SymRecord SymRecord;
    typedef typename ELF_RoutineSet::WordTy WordTy;
    typedef typename ELF_RoutineSet::XWordTy XWordTy;
    typedef typename ELF_RoutineSet::HalfTy HalfTy;
    typedef typename ELF_RoutineSet::OffsetTy OffsetTy;
    typedef typename ELF_RoutineSet::AddrTy AddrTy;
    typedef typename ELF_RoutineSet::SectionIndexTy SectionIndexTy;
    typedef typename ELF_RoutineSet::SymbolIndexTy SymbolIndexTy;
    typedef typename ELF_RoutineSet::SizeTy SizeTy;
    typedef typename ELF_RoutineSet::UnpackedRelocInfo UnpackedRelocInfo;

    // number of sections in linked ELF
    SectionIndexTy outputSectionsCounter;

    // mapping from section names to section numbers
    std::map<std::string, SectionIndexTy> outputSectionsNumbers;
    
    // reverse mapping (the same can be done using section header and shstrtab)
    std::vector<std::string> outputSectionsNames;

    // section headers string table
    SectionHeaderStringTable outputSHStringTable;

    // symbol table for sections
    BRIGDwarfSymbolTable<bitness> outputSymbolTable;

    // section headers
    std::vector<SectionHeader> outputSectionsHeaders;

    // mapping from section numbers to section data
    std::map<SectionIndexTy, std::vector<unsigned char> > outputSectionsData;
    typedef typename std::map<SectionIndexTy, std::vector<unsigned char> >::iterator SectionsDataIterator; 

    // mapping from section numbers to relocation records array for this section
    std::map<SectionIndexTy, std::vector<RelRecord> > outputRelocationRecords;
    typedef typename std::map<SectionIndexTy, std::vector<RelRecord> >::iterator RelocationRecordsIterator;

    // mapping from section numbers to corresponding relocation sections numbers
    std::map<SectionIndexTy, SectionIndexTy> outputRelocationSections;
    typedef typename std::map<SectionIndexTy, SectionIndexTy>::iterator RelocationSectionsIterator;

    // here the linker will store result
    std::vector<unsigned char> result;

    // section numbers in the resulting file
    SectionIndexTy brigCodeSection, brigDirectivesSection, shstrtabSection, symtabSection, nullSection;

    ///////////////////////////////////////////////////////////////////////////////

    // allocates symbol for a section within symbol table 
    // some types of sections do not need a symbol in the symtab
    void allocateSectionSymbol(SectionIndexTy sectionNumber, WordTy sectionType)
    {
      switch (sectionType)
      {
      case SHT_NOBITS:
      case SHT_PROGBITS:
      case SHT_REL:
        outputSymbolTable.addSectionSymbolEntry(sectionNumber);
      default:
        break;
      }
    }

    // returns number of section in the linked file
    SectionIndexTy getOutputSectionNumber(const std::string& name) const
    {
      typename std::map<std::string, SectionIndexTy>::const_iterator i = outputSectionsNumbers.find(name);
      assert(i != outputSectionsNumbers.end());
      if (i == outputSectionsNumbers.end()) {
        assert(!"when does it happen?");
        llvm::errs() << "Error: DWARF linker: unknown section " << name << "\n";
        return 0;
      }
      else return i->second;
    }


    // returns symbol for the section in the symbol table of the linked file
    SymbolIndexTy getSymbolForOutputSection(const std::string& name) const
    {
      SectionIndexTy sectionNumber = getOutputSectionNumber(name);
      return outputSymbolTable.getSymbolNumberForSection(sectionNumber); 
    }

    // allocates section in the linked file (allocates header, symbol, name in the shstrtab and so on)
    SectionIndexTy allocateOutputSection(const std::string& name, WordTy sectionType = SHT_NULL, WordTy shLink = 0, WordTy shInfo = 0) 
    {
      typename std::map<std::string, SectionIndexTy>::iterator i = outputSectionsNumbers.find(name);
      // if the section with given name has not been allocated yet
      if (i == outputSectionsNumbers.end()) {
        // allocate section number 
        SectionIndexTy sectionNumber = outputSectionsCounter++;
      
        // update mapping between section number and section name
        outputSectionsNumbers[name] = sectionNumber;
        assert(outputSectionsNames.size() == sectionNumber);
        outputSectionsNames.push_back(name);
      
        // allocate section's name in shstrtab
        WordTy shName = static_cast<WordTy>(outputSHStringTable.addHeaderName(name));
        // fill header
        SectionHeader header;
        fillSectionHeader(header, sectionType, shName, shLink, shInfo);
        outputSectionsHeaders.push_back(header);
        // allocate symbol in the symbol table
        allocateSectionSymbol(sectionNumber, sectionType);
        return sectionNumber;
      }
      else
        return i->second;
    }

    DwarfDataBlockRef getOrAllocateDataForOutputSection(SectionIndexTy resultingSectionNumber) {
      return outputSectionsData[resultingSectionNumber];
    }

    OffsetTy appendDataBlockToOutputSection(const std::string& name, const unsigned char* dataBegin, const unsigned char* dataEnd) 
    {
      SectionIndexTy resultingSectionNumber = allocateOutputSection(name, SHT_PROGBITS);
      DwarfDataBlockRef resultingSectionData = getOrAllocateDataForOutputSection(resultingSectionNumber);
      // calculate offset and remember it
      OffsetTy sectionOffset = static_cast<SizeTy>(resultingSectionData.size());
      // append to the resulting section
      resultingSectionData.insert(resultingSectionData.end(), dataBegin, dataEnd);
      // return offset that will be used in relocations
      return sectionOffset;
    }

    // @param codeOffsetFinal  :Maps "original" code offsets (i.e. offsets after trivial concatenation of 
    //                          brig containers) to the code offsets in optimized brig.
    // @param dirOffsetFinal   :Same as previous, but for directives.
    //
    // Usage: 
    //    final_offset = codeOffsetFinal.at(original_offset)
    //    if (final_offset == ((uint32_t)-1)) { /* this item have been optimized out */}
    //    else { /* item have been relocated to final_offset */ }
    //
    // TODO_HSA (3) create internal API for dwarf linker and re-use stuff like IS_OFFSET_DEAD() etc.
    //
    bool processElfDataBlock(DwarfDataBlockConstRef data, OffsetTy codeOffset, OffsetTy directivesOffset,
      const std::vector<BRIGOffsetTy>& codeOffsetFinal, const std::vector<BRIGOffsetTy>& dirOffsetFinal)
    {
      // TODO_HSA(3) refactor this function
      const SectionHeader* sectionsBegin;
      const SectionHeader* shstrtab;
      SectionIndexTy brigCodeScn = 0, brigDirectivesScn = 0;
      ELF_SANITY_CHECK( data.size(), "no ELF data found", return false);
      int sCnt = parseELFHeaderAndSections((const unsigned char*)&data.at(0), data.size(), &sectionsBegin, &shstrtab);
      if ( sCnt < 0 ) return false;
      const SectionIndexTy sectionCount = static_cast<SectionIndexTy>(sCnt);

      ELF_SANITY_CHECK( sectionCount > 1, "no ELF sections found", return false)
      ELF_SANITY_CHECK( sectionsBegin->sh_type == SHT_NULL, "wrong ELF format: no reserved section 0", return false);
      ELF_SECTION_HEADER_SANITY_CHECK(shstrtab, data.size(), "wrong section size or offset", return false);

      const char* const sectionNames = static_cast<const char*>((const void*)&data.at(shstrtab->sh_offset));
    
      // symbol tables (normally, there should be only one symbol table in BRIG DWARF container)
      std::map<SectionIndexTy, std::vector<SectionIndexTy> > symbol_tables;

      // relocation tables that apply to the section with given number
      std::map<SectionIndexTy, std::vector<UnpackedRelocInfo> > relocation_tables;
      typedef typename std::map<SectionIndexTy, std::vector<UnpackedRelocInfo> >::iterator RelocationTablesIterator;

      // offsets of regular sections and .brigcode and .brigdirectives
      std::map<SectionIndexTy, OffsetTy> dwarfSectionOffsets;
      
      // names of sections
      std::vector<std::string> sectionNamesStrings;
    
      sectionNamesStrings.resize(sectionCount);

      //skip reserved section 0
      for (SectionIndexTy sectionNum = 1; sectionNum < sectionCount; sectionNum++) {
        const SectionHeader* sec = sectionsBegin + sectionNum;
        // sanity check
        ELF_SECTION_HEADER_SANITY_CHECK(sec, data.size(), "wrong section size or offset", return false);
        ELF_SANITY_CHECK(sec->sh_name < shstrtab->sh_size, "wrong string table offset", return false);

        sectionNamesStrings[sectionNum] = std::string(sectionNames + sec->sh_name);
        const std::string& name = sectionNamesStrings[sectionNum];
        const unsigned char* dataBegin = &data.at(sec->sh_offset);
        const unsigned char* dataEnd = dataBegin + sec->sh_size;
        const SizeTy sectSize = sec->sh_size;
        const OffsetTy sectOffs = sec->sh_offset;

        switch (sec->sh_type) 
        {
        case SHT_PROGBITS:
          DWARF_LINKER_TRACE("regular section %s found at offset %u, %u bytes total\n", name.c_str(), sectOffs, sectSize);
          // append to the resulting section and remember offset
          // Please refer to the BRIG DWARF relocations design document for more information.
          // The following code represents the following step of algorithm:
          // 3.3.  Concatenate its DWARF sections to the corresponding DWARF sections of resulting object.
          //       At this step offsets of this object’s contributions to the DWARF sections of resulting 
          //       object are calculated (debug_line_offseti, debug_info_offseti, debug_abbrev_offseti as displayed on fig. 3);
          dwarfSectionOffsets[sectionNum] = appendDataBlockToOutputSection(name, dataBegin, dataEnd);
          break;
        case SHT_NOBITS:
          ELF_SANITY_CHECK(!sectSize && !sectOffs, "wrong size or offset of NOBITS section", return false);
          ELF_SANITY_CHECK(name == brigCode || name == brigDirectives, "unexpected section with type NOBITS", return false);
          if (name == brigCode) {
            ELF_SANITY_CHECK(!brigCodeScn, "two or more .brigcode sections", return false);
            brigCodeScn = sectionNum;
            DWARF_LINKER_TRACE("fake code section %u (%s) found\n", brigCodeScn, name.c_str());
          } else {
            ELF_SANITY_CHECK(!brigDirectivesScn, "two or more .brigdirectives sections", return false);
            brigDirectivesScn = sectionNum;
            DWARF_LINKER_TRACE("fake directives section %u (%s) found\n", brigDirectivesScn, name.c_str());
          } 
          break;
        case SHT_SYMTAB:
          //sec->sh_link is the link to string table
          DWARF_LINKER_TRACE("symbol table %s found at offset %u. %u items total\n", name.c_str(), sectOffs, sec->sh_info);
          if (symbol_tables.find(sectionNum) == symbol_tables.end())
          {
            if (!loadSymbolTable(dataBegin, sectSize, sec->sh_info, sectionCount, symbol_tables[sectionNum])) return false;
          }
          // else symbol table has already been loaded
          break;
        case SHT_REL:
          //sec->sh_link is the number of the symbol table
          //sec->sh_info is the number of section where relocations apply
          {
            const SectionIndexTy linkedScn = static_cast<SectionIndexTy>(sec->sh_info);
            const SectionIndexTy symtabScn = static_cast<SectionIndexTy>(sec->sh_link);
            ELF_SANITY_CHECK(linkedScn && linkedScn < sectionCount, "wrong link to relocatable section in relocation table", return false);
            ELF_SANITY_CHECK(sectionsBegin[linkedScn].sh_type == SHT_PROGBITS, "wrong link to relocatable section in relocation table", return false);

            ELF_SANITY_CHECK(symtabScn && symtabScn < sectionCount, "wrong link to symtab in relocation table", return false);
            //check whether referenced symbol table has already been loaded, otherwise load it
            if (symbol_tables.find(symtabScn) == symbol_tables.end())
            {
              const SectionHeader* symtab = &sectionsBegin[symtabScn];
              ELF_SANITY_CHECK(symtab->sh_type == SHT_SYMTAB, "wrong link to symtab in relocation table", return false);
              if (!loadSymbolTable( &data.at(symtab->sh_offset), symtab->sh_size, symtab->sh_info, sectionCount, symbol_tables[symtabScn])) 
                return false;
            }
            DWARF_LINKER_TRACE("relocation table %s found at offset %u. Relocation records apply to section %u, symbol table %u\n", name.c_str(), sectOffs, linkedScn, symtabScn);
            if (!loadRelocationTable(dataBegin, sectSize, symbol_tables[symtabScn], relocation_tables[linkedScn])) 
              return false;
            break;
          }
        case SHT_STRTAB:
          // nothing to do with string table
          break;
        default:
          assert(!"unexpected section type");
          llvm::errs() << "Error: DWARF linker: unexpected section type\n";
          return false;
        }
      }
      //apply relocations to DWARF sections
      // Detailed description of this process can be found in document describing BRIG DWARF relocations.
      // It can be found in perforce using the following path: 
      // //depot/stg/hsa/drivers/hsa/compiler/docs/BRIG_DWARF_relocations.docx
      // This loop corresponds to the following line
      // "3.4.  For each relocation section .rel.section from BRIG(i)’s .debug data do the following:"
      // in the paragraph 5 of document mentioned above
      for (RelocationTablesIterator i = relocation_tables.begin(); i != relocation_tables.end(); i++)
      {
        SectionIndexTy sectionNum = i->first;
        const std::vector<UnpackedRelocInfo>& relocRecords = i->second;
        // name of the section where relocations will be applied, or the .section in terms of design document
        std::string& name = sectionNamesStrings[sectionNum];
        // number of section in the linked ELF
        SectionIndexTy resultingSectionNum = getOutputSectionNumber(name);
        assert(resultingSectionNum != nullSection);
        // relocations apply only to DWARF sections
        assert(dwarfSectionOffsets.find(sectionNum) != dwarfSectionOffsets.end());
        // section_offseti - offset of this section in resulting section
        OffsetTy soffs = dwarfSectionOffsets[sectionNum];
        // pointer to the data chunk copied to the resulting section from current file 
        // or .debug + .section + section_offseti in the terms of design document
        unsigned char* resultingSectionChunk = &outputSectionsData[resultingSectionNum].at(soffs);
        // vector of relocation records in the resulting section
        // or .rel.section in the terms of design document
        std::vector<RelRecord>& resultingRelocRecords = outputRelocationRecords[resultingSectionNum];
        // reserve some memory for future relocations
        resultingRelocRecords.reserve(resultingRelocRecords.size() + relocRecords.size());
        DWARF_LINKER_TRACE("applying relocations to section %s: new location is %u\n", name.c_str(), soffs);
        // 3.4.2.	For each relocation record (rel_offset, type, symbol) do the following:
        for (size_t i = 0; i < relocRecords.size(); i++)
        {
          OffsetTy offsetToAdd = 0;
          UnpackedRelocInfo rr = relocRecords[i];
          std::string otherSection; 
          // the section which address would be used to adjust relocatable offset
          // also this section will be used to record relocations
          switch (rr.relType) {
          case R_HSA_DWARF_TO_BRIG_CODE32: 
            // relocations with this type are always relocations from DWARF to BRIG .code section
            // and they must always point to section .brigcode in DWARF container
            // 3.4.2.2.  If type == R_HSA_DWARF_TO_BRIG_CODE{32|64} 
            // then offset_to_add = c_offseti; new_symbol = .brigcode;
            ELF_SANITY_CHECK(rr.relSection == brigCodeScn, 
                              "relocation with type R_HSA_DWARF_TO_BRIG_CODE32 does not point to .brigcode",
                              return false);
            offsetToAdd = codeOffset; //c_offseti
            otherSection  = brigCode; //new_symbol = .brigcode
            break;
          case R_HSA_DWARF_TO_BRIG_DIRECTIVES32: 
            // relocations with this type are always relocations from DWARF to BRIG .directives section
            // and they must always point to section .brigdirectives in DWARF container
            // 3.4.2.3.  If type == R_HSA_DWARF_TO_BRIG_DIRECTIVES{32|64} 
            // then offset_to_add = d_offseti; new_symbol  = .brigdirectives;
            ELF_SANITY_CHECK(rr.relSection == brigDirectivesScn,
                              "relocation with type R_HSA_DWARF_TO_BRIG_DIRECTIVES32 does not point to .brigdirectives",
                              return false);
            offsetToAdd = directivesOffset; //d_offseti
            otherSection  = brigDirectives; //new_symbol = .brigdirectives
            break;
          case R_HSA_DWARF_32:
            // regular ELF relocations from DWARF section to another DWARF section
            // 3.4.2.4.  If type == R_HSA_DWARF_{32|64} then find out what section
            // .other_section is referenced by relocatable address.
            ELF_SANITY_CHECK(dwarfSectionOffsets.find(rr.relSection) != dwarfSectionOffsets.end(),
                              "relocation with type R_HSA_DWARF_32 does not point to valid DWARF section",
                              return false);
            offsetToAdd = dwarfSectionOffsets[rr.relSection];
            otherSection = sectionNamesStrings[rr.relSection]; //new_symbol = .other_section
            break;
          default:
            assert(!"unknown relocation type");
            llvm::errs() << "Error: DWARF linker: unexpected relocation type\n";
            return false;
          }
          // DWARF_LINKER_TRACE("\t%u: offset %u, section %u (%s)\n", i, rr.relOffset, rr.relSection, otherSection.c_str());
          // apply relocation
          // calculate address (.debug + .section + section_offseti + rel_offset) in terms of design document
          OffsetTy* relocatedAddr = static_cast<OffsetTy*>(static_cast<void*>(resultingSectionChunk + rr.relOffset));
          /* 3.4.2.5.	 Adjust the relocatable address in the resulting object’s DWARF:
             *(.debug + .section + section_offseti + rel_offset) += offset_to_add 
          */
          {
            // Consider displacements of brig instructions and directives
            // caused by linker optimizations (deDeading):
            const std::vector<BRIGOffsetTy> * brigFO = NULL; // ptr to vector of final offsets
            if (rr.relType == R_HSA_DWARF_TO_BRIG_CODE32) { brigFO = &codeOffsetFinal; }
            else if (rr.relType == R_HSA_DWARF_TO_BRIG_DIRECTIVES32) { brigFO = &dirOffsetFinal; }
            else {} // keep NULL
            if (brigFO) {
              OffsetTy brigOffset = *relocatedAddr;
              if (brigOffset == 0) { } // do not relocate zero brig offsets
              else {
                brigOffset = brigFO->at(brigOffset + offsetToAdd);
                if (brigOffset == ((OffsetTy)-1)) { brigOffset = 0; } // removed; reloc to unused offset
              }
              *relocatedAddr = brigOffset;
            }
            else { // non-brig relocations
              // DWARF offsets are 32 bit even for 64 bit target
              uint32_t* relocatedAddr32 = static_cast<uint32_t*>(static_cast<void*>(resultingSectionChunk + rr.relOffset));
              *relocatedAddr32 += static_cast<uint32_t>(offsetToAdd);
            }
          }
          /* 3.4.2.6.  Store new relocation record (section_offseti + rel_offset, type, new_symbol) 
            for memory location we’ve just updated in relocation table .rel.section in the resulting object.
          */
          // record new relocation
          RelRecord RR;
          // rr.relType remains intact
          RR.setType(static_cast<unsigned char>(rr.relType));
          // adjust offset 
          RR.r_offset = rr.relOffset + (OffsetTy)soffs;
          // find symbol number in resulting file
          RR.setSymbol(getSymbolForOutputSection(otherSection));
          resultingRelocRecords.push_back(RR);
        }
      }
      // uff
      return true;
    }

    void createRelocationSections() {
      const std::string relPrefix = ".rel";
      for (RelocationRecordsIterator i = outputRelocationRecords.begin(); i != outputRelocationRecords.end(); i++) 
      {
        std::string relSectionName = relPrefix + outputSectionsNames[i->first]; // .rel + original section
        DWARF_LINKER_TRACE("relocation section %s created\n", relSectionName.c_str());
        outputRelocationSections[i->first] = allocateOutputSection(relSectionName, SHT_REL, symtabSection, i->first);
      }
    }

    OffsetTy appendSectionBytes(DwarfDataBlockRef result, XWordTy alignment, const unsigned char* dataBegin, SizeTy dataSize)
    {
      // pad with zeros
      if (alignment)
      {
        SizeTy oldSize = static_cast<SizeTy>(result.size());
        SizeTy newSize = ((oldSize + alignment - 1)/alignment)*alignment;
        result.insert(result.end(), newSize - oldSize, 0);
      }
      const OffsetTy offs = static_cast<OffsetTy>(result.size());
      if (dataSize) {
        assert(dataBegin); // sanity check - dataBegin may be NULL only if dataSize is 0
        // copy
        result.insert(result.end(), dataBegin, dataBegin + dataSize);
      }
      return offs;
    }

    void appendSectionDataAndUpdateHeader(DwarfDataBlockRef result, SectionIndexTy sectionNumber, const unsigned char* dataBegin, SizeTy dataSize)
    {
      SectionHeader* header = &outputSectionsHeaders[sectionNumber];
      header->sh_size = dataSize;
      // this fixes handling of empty sections
      if (dataSize == 0) {
        dataBegin = 0;
      }
      header->sh_offset = appendSectionBytes(result, header->sh_addralign, dataBegin, dataSize);
      DWARF_LINKER_TRACE("DWARF section %s: size %08x, offset %08x\n", outputSectionsNames[sectionNumber].c_str(),\
                                     static_cast<unsigned>(header->sh_size),\
                                     static_cast<unsigned>(header->sh_offset));
    }


    void buildOutputSections(DwarfDataBlockRef data) {
      //calculate offset of shstrtab and write its data
      appendSectionDataAndUpdateHeader(data, shstrtabSection, 
                                        outputSHStringTable.rawHeaderData(),
                                        outputSHStringTable.rawHeaderSize());

      //calculate offset of symtab and write its data
      appendSectionDataAndUpdateHeader(data, symtabSection, 
                                        (const unsigned char*)outputSymbolTable.rawSymbolTableData(),
                                        outputSymbolTable.getSymbolTableSize());

      //fill info field in symtab section (it should contain number of symbols)
      outputSectionsHeaders[symtabSection].sh_info = outputSymbolTable.getNumberOfSymbols();

      //check that brigcode and brigdirectives has offset 0
      assert(outputSectionsHeaders[brigCodeSection].sh_offset == 0);
      assert(outputSectionsHeaders[brigDirectivesSection].sh_offset == 0);
      assert(outputSectionsHeaders[brigCodeSection].sh_size == 0);
      assert(outputSectionsHeaders[brigDirectivesSection].sh_size == 0);

      //calculate DWARF sections offsets and write their data
      for (SectionsDataIterator i = outputSectionsData.begin(); i != outputSectionsData.end(); i++)
      {
        if (i->second.size()) {
          // non-empty section
          appendSectionDataAndUpdateHeader(data, i->first, 
            &i->second.at(0), i->second.size());
        } 
        else {
          // empty section - i->second.at(0) will assert, so we pass NULL pointer instead
          appendSectionDataAndUpdateHeader(data, i->first, 
            static_cast<const unsigned char*>(0), 0);
        }
      }
  
      //calculate .rel sections offsets and write their data
      for (RelocationRecordsIterator i = outputRelocationRecords.begin(); i != outputRelocationRecords.end(); i++) 
      {
        SectionIndexTy relSectionIndex = outputRelocationSections[i->first];
        const unsigned char* relData = static_cast<const unsigned char*>((const void*)(&i->second.at(0)));
        const SizeTy relSize = i->second.size()*sizeof(RelRecord);
        appendSectionDataAndUpdateHeader(data, relSectionIndex, relData, relSize);
      }
    }


    void buildResultingElf(DwarfDataBlockRef data) {
      // create relocation sections
      createRelocationSections();
    
      // fill default fields ELF header
      ElfHeader header;
      fillELFheader(&header);

      // reserve place for ELF header
      data.resize(header.e_ehsize);

      // fill other fields of ELF header
      header.e_shnum = static_cast<HalfTy>(outputSectionsHeaders.size());
      assert(header.e_shnum == outputSectionsCounter);
      header.e_shstrndx = static_cast<HalfTy>(shstrtabSection);

      // emit all sections and update sizes and offsets in header
      buildOutputSections(data);

      // emit all section headers
      const unsigned char* shDataBegin = static_cast<const unsigned char*>((const void *)&outputSectionsHeaders.at(0));

      // calculate section headers offset and store it in elf header
      header.e_shoff = appendSectionBytes(data, 1, shDataBegin, header.e_shnum * header.e_shentsize);
      const unsigned char* headerDataBegin = static_cast<const unsigned char*>((const void *)&header);
      std::copy(headerDataBegin, headerDataBegin  + header.e_ehsize, data.begin());
      //OK, now data contains valid elf
    }


    void initOutputSections() {
      nullSection = allocateOutputSection("", SHT_NULL);
      assert(nullSection == 0);
      shstrtabSection = allocateOutputSection(".shstrtab", SHT_STRTAB);
      symtabSection = allocateOutputSection(".symtab", SHT_SYMTAB, shstrtabSection);
      brigDirectivesSection = allocateOutputSection(brigDirectives, SHT_NOBITS);
      brigCodeSection = allocateOutputSection(brigCode, SHT_NOBITS);
    }


  public:
    SpecificDwarfLinker() : outputSectionsCounter(0) {
      initOutputSections();
    }


    // clears all structures created by instance DwarfLinker during its operation
    void resetLinker() {
      // reset section counter
      outputSectionsCounter = 0;
      // clear all internal data structures
      outputSectionsNumbers.clear();
      outputSectionsNames.clear();
      outputSHStringTable.clear();
      outputSymbolTable.clear();
      outputSectionsHeaders.clear();
      outputSectionsData.clear();
      outputRelocationRecords.clear();
      outputRelocationSections.clear();
      // clear all linked data
      result.clear();
      // initialize output sections
      initOutputSections();
    }

    /* returns true if this ELF data is supported by this linker */
    static bool isValidElfData(DwarfDataBlockConstRef data) {
      return isCompatibleElfHeader(static_cast<const ElfHeader*>((const void*)&data.at(0)), data.size());
    }

    // appends ELF data to linkage
    bool appendDwarfElfData(DwarfDataBlockRef data, BRIGOffsetTy codeOffset, BRIGOffsetTy directivesOffset, 
      const std::vector<BRIGOffsetTy>& codeOffsetFinal, const std::vector<BRIGOffsetTy>& dirOffsetFinal)
    {
      return processElfDataBlock(data, static_cast<OffsetTy>(codeOffset), static_cast<OffsetTy>(directivesOffset), codeOffsetFinal, dirOffsetFinal);
    }

    // build linked ELF and return const reference to vector containing linked ELF
    DwarfDataBlockConstRef getResultingDwarfData() {
      buildResultingElf(result);
      return result;
    }

    // build linked ELF and store it in the given vector
    DwarfDataBlockRef getResultingDwarfData(DwarfDataBlockRef data) {
      buildResultingElf(data);
      return data;
    }

  }; //class SpecificDwarfLinker

  // DwarfLinker encapsulates difference between 32 and 64 bit ELF containers used to store DWARF
  class DwarfLinker {
  public:
    // currently we support only 32-bit BRIG offsets
    typedef uint32_t BRIGOffsetTy;

  private:
    enum {
      LinkModeUndefined,
      LinkModeELF32,
      LinkModeELF64
    } linkMode;

    typedef SpecificDwarfLinker<BRIGOffsetTy, 32> DwarfLinker32;
    typedef SpecificDwarfLinker<BRIGOffsetTy, 64> DwarfLinker64;

    DwarfLinker32 linker32;
    DwarfLinker64 linker64;
    const std::vector<unsigned char> emptyDataBlock;

  public:
    DwarfLinker();

    // clears all structures created by DwarfLinker instance during its operation
    void resetLinker();

    // appends ELF data to linkage
    bool appendDwarfElfData(DwarfDataBlockRef data, BRIGOffsetTy codeOffset, BRIGOffsetTy directivesOffset, 
                            const std::vector<BRIGOffsetTy>& codeOffsetFinal,
                            const std::vector<BRIGOffsetTy>& dirOffsetFinal);

    // build linked ELF and return const reference to vector containing linked ELF
    DwarfDataBlockConstRef getResultingDwarfData();

    // build linked ELF and store it in the given vector
    DwarfDataBlockRef getResultingDwarfData(DwarfDataBlockRef data);
  }; //class DwarfLinker

}; //namespace DebugInformationLinker

#else
#endif
