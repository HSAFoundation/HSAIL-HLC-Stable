#ifndef ELFUTILS_H
#define ELFUTILS_H

#include <vector>
#include <map>
#include <cassert>

//ELF support
#include "llvm/Support/ELF.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm::ELF;

// some useful macros
#define ELF_SANITY_CHECK(condition, message, action) \
  if (!(condition)) {\
    assert(!message);\
    llvm::errs() << "Error: DWARF linker: " << message << "\n"; \
    action;          \
  } else { }

#define ELF_SECTION_HEADER_SANITY_CHECK(section_header, datasize, message, action) \
  if ( section_header->sh_offset > datasize\
        || section_header->sh_size >= datasize\
        || section_header->sh_offset + section_header->sh_size > datasize ) { \
        assert(!message);\
        llvm::errs() << "Error: DWARF linker: " << message << "\n"; \
        action; \
  } else { }


namespace DebugInformationLinking
{
  template<int bitness> struct ElfTraits;

  template<> struct ElfTraits<32> {
    typedef Elf32_Ehdr  ElfHeader;
    typedef Elf32_Shdr  SectionHeader;
    typedef Elf32_Rel   RelRecord;
    typedef Elf32_Sym   SymRecord;
    typedef Elf32_Word  WordTy;
    typedef Elf32_Word  XWordTy;
    typedef Elf32_Half  HalfTy;
    typedef Elf32_Off   OffsetTy;
    typedef Elf32_Addr  AddrTy;
    typedef Elf32_Half  SectionIndexTy;
    typedef Elf32_Word  SymbolIndexTy;
    typedef Elf32_Word  SizeTy;
  };

  template<> struct ElfTraits<64> {
    typedef Elf64_Ehdr  ElfHeader;
    typedef Elf64_Shdr  SectionHeader;
    typedef Elf64_Rel   RelRecord;
    typedef Elf64_Sym   SymRecord;
    typedef Elf64_Word  WordTy;
    typedef Elf64_Xword XWordTy;
    typedef Elf64_Half  HalfTy;
    typedef Elf64_Off   OffsetTy;
    typedef Elf64_Addr  AddrTy;
    typedef Elf64_Half  SectionIndexTy;
    typedef Elf64_Xword SymbolIndexTy;
    typedef Elf64_Xword SizeTy;
  };

  //utility classes and functions mostly taken from HSAILasm and libBRIGDwarf
  //TODO_HSA(3) use EM_HSAIL and EM_HSAIL_64 from llvm/support/ELF.h (requires changes in assembler and other tools)
  enum {
    EM_HSAIL     = 0xAF5A,  // HSAIL 32bit
    EM_HSAIL_64  = 0x81     // HSAIL 64bit
  };

  //TODO_HSA(3) use HSA relocation types enumeration from ELF headers (elfdefinitions.h)
  enum hsa_relocation_types {
    R_HSA_DWARF_32 = 3,
    R_HSA_DWARF_TO_BRIG_CODE32 = 4,
    R_HSA_DWARF_TO_BRIG_DIRECTIVES32 = 5
  };

  // names of fake sections representing .code and .directives
  extern const std::string brigCode;
  extern const std::string brigDirectives;

  //TODO_HSA this is copied from libBRIGDWARF, we should move this to the separate module
  class SectionHeaderStringTable
  {
  private:
	  std::vector<unsigned char> m_data;

  public:
	  SectionHeaderStringTable();
	  virtual ~SectionHeaderStringTable();

    void clear();
	  size_t addHeaderName( const std::string & headerName );
	  const unsigned char * rawHeaderData();
	  size_t rawHeaderSize();

  }; //class SectionHeaderStringTable


  //TODO_HSA(3) this is copied from libBRIGDWARF, we should move this to the separate module
  /* MT-unsafe implementation of symbol table for BRIG DWARF */
  template<int bitness> class BRIGDwarfSymbolTable
  {
  public:
    typedef ElfTraits<bitness> ELF_Traits;
    typedef typename ELF_Traits::SymRecord SymRecord;
    typedef typename ELF_Traits::HalfTy HalfTy;
    typedef typename ELF_Traits::SectionIndexTy SectionIndexTy;
    typedef typename ELF_Traits::SymbolIndexTy SymbolIndexTy;
    typedef typename ELF_Traits::SizeTy SizeTy;

  private:
    /* pre-allocate memory for 10 items */
    static const unsigned initialSize = 10; 
    std::vector< SymRecord > m_data;
    std::map< SectionIndexTy, SymbolIndexTy > m_sectionToSymbol;
    typedef typename std::map< SectionIndexTy, SymbolIndexTy >::const_iterator ssm_iterator;

    //TODO_HSA(3) this is copied from libBRIGDWARF, we should move this to the separate module
    SymbolIndexTy addSymbolTableEntry(const SymRecord& sym) {
      SymbolIndexTy symbolNum = m_data.size();
      m_data.push_back(sym);
      m_sectionToSymbol[sym.st_shndx] = symbolNum;
      return symbolNum;
    }

  public:

    void clear() {
      m_data.clear();
      SymRecord sym0;
      /* reserve some items */
      m_data.reserve(initialSize);
      /* prepare entry #0, see 32-bit ELF spec, page 1-20 */
      ::memset(&sym0, 0, sizeof(sym0));
      sym0.st_shndx = SHN_UNDEF;
      (void)addSymbolTableEntry(sym0);
    }

    BRIGDwarfSymbolTable() {
      clear();
    }

    ~BRIGDwarfSymbolTable() {}

    const void* rawSymbolTableData() const {
      return &m_data.at(0);
    }

    SizeTy getNumberOfSymbols() const {
      return static_cast<SizeTy>(m_data.size());
    }

    SizeTy getSymbolTableSize() const {
      return static_cast<SizeTy>(m_data.size()*sizeof(SymRecord));
    }

    SymbolIndexTy addSectionSymbolEntry(unsigned index) {
      SymRecord sectionSym;
      // not sure whether we want to change binding and offset fields
      sectionSym.st_name  = 0;
      sectionSym.st_value = 0;
      sectionSym.st_size  = 0;
      sectionSym.setBindingAndType(STB_LOCAL, STT_SECTION);
      sectionSym.st_other = 0;
      sectionSym.st_shndx = static_cast<HalfTy>(index);
      return addSymbolTableEntry(sectionSym);
    }

    SymbolIndexTy getSymbolNumberForSection(SectionIndexTy sectionNum) const {
      ssm_iterator i = m_sectionToSymbol.find(sectionNum);
      if(i != m_sectionToSymbol.end())
        return i->second;
      else {
        assert(!"when does it happen?");
        return 0;
      }
    }

  };

  template<int bitness> struct ElfRoutineSet {

    typedef ElfTraits<bitness> ELF_Traits;
    typedef typename ELF_Traits::ElfHeader ElfHeader;
    typedef typename ELF_Traits::SectionHeader SectionHeader;
    typedef typename ELF_Traits::RelRecord RelRecord;
    typedef typename ELF_Traits::SymRecord SymRecord;
    typedef typename ELF_Traits::WordTy WordTy;
    typedef typename ELF_Traits::XWordTy XWordTy;
    typedef typename ELF_Traits::HalfTy HalfTy;
    typedef typename ELF_Traits::OffsetTy OffsetTy;
    typedef typename ELF_Traits::AddrTy AddrTy;
    typedef typename ELF_Traits::SectionIndexTy SectionIndexTy;
    typedef typename ELF_Traits::SymbolIndexTy SymbolIndexTy;
    typedef typename ELF_Traits::SizeTy SizeTy;

    typedef struct tagUnpackedRelocInfo
    {
      OffsetTy       relOffset; // offset from the beginning of original section
      WordTy         relType;   // type of relocation
      SectionIndexTy relSection;// number of section (not the symbol)
    } UnpackedRelocInfo;

    static const int elfClass = (bitness == 32) ? ELFCLASS32 : (bitness == 64 ? ELFCLASS64 : ELFCLASSNONE);
    static const int elfMachine = (bitness == 32) ? (int)EM_HSAIL :
                                  (bitness == 64 ? (int)EM_HSAIL_64 : (int)EM_NONE);

    // fill section header for given type
    static void fillSectionHeader(SectionHeader& header, WordTy sectionType, WordTy shName, WordTy shLink, WordTy shInfo) 
    {
      ::memset(&header, 0, sizeof(SectionHeader));
      header.sh_type = sectionType;
      /* section-specific setup */
      switch(sectionType)
      {
      case SHT_STRTAB:
        header.sh_flags = SHF_STRINGS;
        header.sh_addralign = 1;
        break;
      case SHT_SYMTAB:
        header.sh_entsize = header.sh_addralign = sizeof(SymRecord);
        break;
      case SHT_REL:
        header.sh_entsize = sizeof(RelRecord);
        header.sh_addralign = bitness / 8; //TODO_HSA(3) should we use 4 or sizeof(Elf32_Rel)?
        break;
      case SHT_NULL:
        break;
      default:
        header.sh_addralign = bitness / 8;
      };
      header.sh_name = shName;
      header.sh_info = shInfo;
      header.sh_link = shLink;
    };

    static bool isCompatibleElfHeader(const ElfHeader* h, SizeTy size) 
    {
      return (size > sizeof(ElfHeader) 
              && h->e_ehsize == sizeof(ElfHeader) 
              && h->e_ident[EI_CLASS] == elfClass);
              // currently elfMachine may differ between different DWARF producers
              // && header->e_machine = elfMachine
    }

    // fill default fields in ELF header
    static void fillELFheader(ElfHeader *header) {
      ::memset(header, 0, sizeof(ElfHeader));
      header->e_ident[EI_MAG0] = ElfMagic[EI_MAG0]; 
      header->e_ident[EI_MAG1] = ElfMagic[EI_MAG1]; 
      header->e_ident[EI_MAG2] = ElfMagic[EI_MAG2]; 
      header->e_ident[EI_MAG3] = ElfMagic[EI_MAG3]; 
      header->e_ident[EI_CLASS] = elfClass; 
      header->e_ident[EI_DATA] = ELFDATA2LSB;
      header->e_ident[EI_VERSION] = EV_CURRENT;

      header->e_machine = elfMachine; /* TODO: add HSAIL target support to both libelf and libdwarf */
      header->e_type = ET_REL;      /* BRIG DWARF is a relocatable data */
      header->e_version = EV_CURRENT;
      header->e_ehsize = sizeof(ElfHeader);
      header->e_shentsize = sizeof(SectionHeader);
    }

    // parses ELF header in given memory block, returns number of sections or -1 if some check failed
    // pointers to the begin of section headers and shstrtab section header are passed in sections and shstrtab correspondingly
    static int parseELFHeaderAndSections(const void* data, SizeTy size, const SectionHeader** sections, const SectionHeader** shstrtab)
    {
      const ElfHeader* h = static_cast<const ElfHeader*>(data);
      const unsigned char* dataBytes = static_cast<const unsigned char*>(data);

      // paranoid sanity check
      ELF_SANITY_CHECK(size > sizeof(ElfHeader), "insufficient amount of data to load ELF", return -1)
      ELF_SANITY_CHECK(h->checkMagic(), "elf magic number check failed", return -1)
      ELF_SANITY_CHECK(h->e_shoff < size, "some section headers are out of bounds", return -1)
      ELF_SANITY_CHECK((SizeTy)(h->e_shentsize * h->e_shnum) < size, "some section headers are out of bounds", return -1)
      ELF_SANITY_CHECK(h->e_ehsize == sizeof(ElfHeader), "wrong size of ELF header", return -1)
      ELF_SANITY_CHECK(h->e_shoff + h->e_shentsize * h->e_shnum <= (OffsetTy)size, "some section headers are out of bounds", return -1)
      ELF_SANITY_CHECK(h->e_shentsize == sizeof(SectionHeader), "wrong size of section header", return -1)
      ELF_SANITY_CHECK(h->e_shstrndx < h->e_shnum, "string table index out of bounds", return -1)

      *sections = static_cast<const SectionHeader*>((const void*)(dataBytes + h->e_shoff));
      *shstrtab = static_cast<const SectionHeader*>((const void*)(dataBytes + h->e_shoff + h->e_shentsize*h->e_shstrndx));
      return (int)h->e_shnum;
    }


    // parses ELF symbol table in given memory block and creates mapping from symbol number to section number in symToScn
    // returns true if succeeded, false otherwise
    static bool loadSymbolTable(const void* data, SizeTy size, SizeTy nItems, 
                                SectionIndexTy nSections, std::vector<SectionIndexTy>& symToScn)
    {
      const SymRecord* sym = static_cast<const SymRecord*>(data);
      ELF_SANITY_CHECK(nItems*sizeof(SymRecord) <= size, "too much symbols in the symbol table", return false);
      ELF_SANITY_CHECK(!(sym->st_info || sym->st_name || sym->st_other || sym->st_shndx || sym->st_size || sym->st_value), "reserved symbol is not null", return false);
      symToScn.resize(nItems);
      symToScn[0] = 0;
      //skip reserved symbol #0
      for(SizeTy i = 1; i < nItems; i++) {
        ELF_SANITY_CHECK(sym[i].getType() == STT_SECTION, "found unexpected symbol which is not a section", return false);
        ELF_SANITY_CHECK(sym[i].st_shndx < nSections, "found symbol that points to non-existing section", return false );
        symToScn[i] = sym[i].st_shndx;
      }
      return true;
    }

    // parses ELF relocations table in given memory block and creates vector of relocation records
    // where number of symbol is replaced with number of section using the mapping provided in symbolTable
    // returns true if succeeded, false otherwise
    static bool loadRelocationTable(const void* data, SizeTy size, 
                                    const std::vector<SectionIndexTy>& symbolTable, 
                                    std::vector<UnpackedRelocInfo>& relRecords)
    {
      const RelRecord* rr = static_cast<const RelRecord*>(data);
      const SizeTy nSyms = static_cast<SizeTy>(symbolTable.size());
      const SizeTy nRels = static_cast<SizeTy>(size / sizeof(RelRecord));
      relRecords.resize(nRels);
      for (SizeTy i = 0; i < nRels; i++) {
        SymbolIndexTy sym = rr[i].getSymbol();
        ELF_SANITY_CHECK(sym < nSyms, "wrong symbol reference in relocation table", return false);
        relRecords[i].relOffset = rr[i].r_offset;
        relRecords[i].relSection = symbolTable[sym];
        relRecords[i].relType = rr[i].getType();
      }
      return true;
    }
 
  };

  } // namespace DebugInformationLinker

#endif //ELFUTILS_H
