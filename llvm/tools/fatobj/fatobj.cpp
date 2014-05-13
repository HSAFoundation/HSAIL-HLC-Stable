//===----------------------------------------------------------------------===//
//
// This program implements the "fat object file" join/split functionality
// in the AMD HSA compiler stack.
//
//===----------------------------------------------------------------------===//

#include "warncontrol.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#ifdef _MSC_VER
  #include <io.h>
  #include <SYS/Stat.h> /* _fstat() */
#endif
#ifdef __GNUC__
  #include <unistd.h>
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <errno.h>
  // emulate Windows CRT
  #define _lseek lseek
  #define _read  read
  #define _open  open
  #define _close close
  #define _write write
  #define _stat stat
  #define _fstat fstat
  #define _O_RDONLY O_RDONLY
  #define _O_BINARY 0x0 /* n/a for linux */
  #define _O_CREAT  O_CREAT
  #define _O_TRUNC  O_TRUNC
  #define _O_RDWR   O_RDWR
  #define _S_IFREG  S_IFREG
#endif

#include <malloc.h>
#include <string>
#include <vector>
#include <map>

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/ELF.h"
#include "warncontrol.h"

namespace {

using namespace llvm;

cl::opt<std::string>
Pathname(cl::Positional, cl::desc("<pathname>"), cl::Required);

enum Modes {
  M_Join,
  M_Split,
  M_Create,
  M_Extract,
  M_Detect
};

cl::opt<Modes>
Mode(
  cl::desc("Action to perform:"),
  cl::init(M_Join),
  cl::values(
    clEnumValN(M_Join, "join"
      ,"Combine Host and Device object files into a Fat object file (default, obsolete option)"),
    clEnumValN(M_Split, "split"
      ,"Split Fat object file into Host and Device object files (obsolete option)"),
    clEnumValN(M_Create, "create"
      ,"Create new Fat object file and put Host object, Device object and Option files into it"),
    clEnumValN(M_Extract, "extract"
      ,"Extract Host object, Device object and Option files from Fat object file"),
    clEnumValN(M_Detect, "detect-type"
      ,"Detect type of file (Fat object, 'old plain' ELF/BRIG, BIF...)"),
    clEnumValEnd));

cl::opt<bool>
ModeRaw("debug-elf-writeback"
  ,cl::Hidden
  ,cl::desc("Read elf into sections, build new elf, write back (with default header)")
  ,cl::init(false));
    
static cl::opt<std::string>
OutputPathname("debug-o"
  ,cl::Hidden
  ,cl::desc("Override output pathname(s)")
  ,cl::value_desc("output pathname")
  ,cl::init(""));

cl::opt<bool>
Trace("debug-trace-print"
  ,cl::Hidden
  ,cl::desc("Print trace information")
  ,cl::init(false));
    
} // namespace

typedef std::pair< void*, unsigned > SectionPdataAndSize;
typedef std::map< std::string, std::vector< SectionPdataAndSize > > SaveElfSections;

typedef std::map< std::string, std::vector< char > > LoadElfSections;

int loadData(void* dst, int fd, unsigned offset, unsigned size) {
    if ((unsigned)_lseek(fd, offset, SEEK_SET) != offset) { return 1; }
    if ((unsigned)_read(fd, dst, size) != size) { return 1; }
    return 0;
};
int loadData(std::vector<char> &dst, int fd, unsigned offset, unsigned size) {
    dst.resize(size);
    return loadData(&dst[0], fd, offset, size);
};

// TODO_HSA (3) Refactor: move load/saveElf functionality to some library.

int loadElf(const char* filename, LoadElfSections* sections, llvm::ELF::Elf32_Ehdr * const header ) {
    using namespace llvm::ELF;
    std::vector<Elf32_Shdr> sectionHeaders;
    std::vector<char> sectionNameTable;

    if (header != NULL) { memset(header,0,sizeof(*header)); }

    int fd = _open(filename, _O_RDONLY | _O_BINARY);
    if (fd < 0) {
        printf("Error %d opening %s\n", errno, filename);
        return 2; // allow the caller to distinguish "wrong ELF" from "missing file".
    }
    
    int rc = 0;
    Elf32_Ehdr elfHeader;
    if (_read(fd, &elfHeader, sizeof(elfHeader)) != sizeof(elfHeader)) {
        printf("Error %d reading ELF header of %s\n", errno, filename);
        rc = 1;
    }
    if (!rc) {
        if (header != NULL) { *header = elfHeader; }
        if (!elfHeader.checkMagic()) {
//            printf("Error %d verifying ELF header of %s\n", errno, filename);
            rc = 1;
        }
    }
    if (!rc) {
        sectionHeaders.resize(elfHeader.e_shnum);
        for(int i=0; i < elfHeader.e_shnum; ++i) {
            if (loadData(&sectionHeaders[i], fd
                         ,elfHeader.e_shoff + i * elfHeader.e_shentsize
                         ,sizeof(Elf32_Shdr))) {
                printf("Error %d reading section table of %s\n", errno, filename);
                rc = 1;
                break;
            }
        };
    }
    if (!rc) {
        if(loadData(sectionNameTable, fd
                   ,sectionHeaders[elfHeader.e_shstrndx].sh_offset
                   ,sectionHeaders[elfHeader.e_shstrndx].sh_size)) {
            printf("Error %d reading section name table of %s\n", errno, filename);
            rc = 1;
        }
    }
    if (!rc) {
        for(int i=1; i < elfHeader.e_shnum; ++i) {
            if (i == elfHeader.e_shstrndx) { continue; }
            const char* name = &sectionNameTable[sectionHeaders[i].sh_name];
            unsigned offset = sectionHeaders[i].sh_offset;
            unsigned size = sectionHeaders[i].sh_size;
            std::vector<char>& sectionData = (*sections)[name];
            sectionData.resize(size);
            if(loadData(&sectionData[0], fd, offset, size)) {
                printf("Error %d reading section %s in %s\n", errno, name, filename);
                rc = 1;
                break;
            }
            if (Trace) { printf("%s %u %u\n", name, offset, size); }
        };
    }
    _close(fd);
    return rc;
}

int alignPos(int fd, long align) {
    long pos = _lseek(fd, 0, SEEK_CUR);
    char *zeropad = "\0\0\0\0\0\0\0\0";
    int n = (-pos&(align-1));
    if (Trace) { printf("0x%lx align to %ld pad %d bytes\n", pos, align, n); }
    if (n == 0) { return 0; }
    if (_write(fd, zeropad, n) != n) {
        return 1;
    }
    return 0;
}

struct SaveElfParams {
  llvm::ELF::Elf32_Half header_e_type;
};

int saveElf(const char* filename, const SaveElfSections& sections, const SaveElfParams * const params) {
    using namespace llvm::ELF;

    Elf32_Ehdr elfHeader;
    memset(&elfHeader, 0, sizeof(elfHeader));
    memcpy(elfHeader.e_ident, ElfMagic, 4);
    elfHeader.e_ident[EI_CLASS] = ELFCLASS32;
    elfHeader.e_ident[EI_DATA] = ELFDATA2LSB;
    elfHeader.e_ident[EI_VERSION] = EV_CURRENT;
    elfHeader.e_version = EV_CURRENT;
    if (params) {
      elfHeader.e_type = params->header_e_type;
    }

    elfHeader.e_ehsize = sizeof(elfHeader);
    elfHeader.e_shentsize = sizeof(Elf32_Shdr);
    elfHeader.e_shnum = sections.size() + 2; // + NULL section + .shstrtab
    elfHeader.e_shstrndx = elfHeader.e_shnum - 1;
    enum { ALIGN = 4 };

    std::vector<Elf32_Shdr> sectionHeaders;
    sectionHeaders.resize(elfHeader.e_shnum);
    memset(&sectionHeaders[0], 0, elfHeader.e_shnum * elfHeader.e_shentsize);
    std::vector<char> sectionNameTable;
    sectionNameTable.push_back(0);
    int secIndex = 1;
    unsigned filePos = sizeof(Elf32_Ehdr);
    filePos = (filePos + ALIGN-1) &~ (ALIGN-1);
    SaveElfSections::const_iterator p;
    for(p = sections.begin(); p != sections.end(); ++p) {
        Elf32_Shdr &thisSec = sectionHeaders[secIndex];
        std::string name = p->first;
        thisSec.sh_type = (strcmp(name.c_str(), ".strtab") == 0 ? SHT_STRTAB : SHT_PROGBITS); // TBD HACK
        thisSec.sh_name = sectionNameTable.size();
        sectionNameTable.insert(sectionNameTable.end(), name.begin(), name.end());
        sectionNameTable.push_back(0);
        thisSec.sh_offset = filePos;
        if (Trace) { printf("%s at ofs %d\n", p->first.c_str(), (int)filePos); }
        for(unsigned i=0; i < p->second.size(); ++i) {
            thisSec.sh_size += p->second[i].second;
        }
        thisSec.sh_addralign = 1;
        filePos += thisSec.sh_size;
        filePos = (filePos + ALIGN-1) &~ (ALIGN-1);
        ++secIndex;
    };
    {
        std::string name = ".shstrtab";
        Elf32_Shdr &thisSec = sectionHeaders[secIndex];
        thisSec.sh_type = SHT_STRTAB;
        thisSec.sh_name = sectionNameTable.size();
        sectionNameTable.insert(sectionNameTable.end(), name.begin(), name.end());
        sectionNameTable.push_back(0);
        if (Trace) { printf("%s at ofs %d\n", name.c_str(), (int)filePos); }
        thisSec.sh_offset = filePos;
        thisSec.sh_size = sectionNameTable.size();
        thisSec.sh_addralign = 1;
        filePos += thisSec.sh_size;
        filePos = (filePos + ALIGN-1) &~ (ALIGN-1);
        ++secIndex;
    }

    elfHeader.e_shoff = filePos;
    if (Trace) { printf("ST ofs %d\n", filePos); }

    int fd = _open(filename, _O_CREAT|_O_TRUNC|_O_RDWR|_O_BINARY, 0666);
    if (fd < 0) {
        printf("Error %d opening %s\n", errno, filename);
        return 1;
    }

    int rc = 0;
    if (_write(fd, &elfHeader, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        printf("Error at line %d errno=%d\n", (int)__LINE__, errno);
        rc = 1; 
    }
    if (!rc) {
        if (alignPos(fd, ALIGN)) {
            printf("Error at line %d errno=%d\n", (int)__LINE__, errno);
            rc = 1;
        }
    }
    if (!rc) {
        for(p = sections.begin(); p != sections.end(); ++p) {
            if (Trace) { printf("%s w at ofs %d\n", p->first.c_str(), (int)_lseek(fd, 0, SEEK_CUR)); }
            for(unsigned i=0; i < p->second.size(); ++i) {
                // concatenate all data
                void* data = p->second[i].first;
                unsigned size = p->second[i].second;
                if ((unsigned)_write(fd, data, size) != size) {
                    printf("Error at line %d errno=%d\n", (int)__LINE__, errno);
                    rc = 1;
                    break;
                }
            }
            if (rc) { break; }
            if (alignPos(fd, ALIGN)) {
                printf("Error at line %d errno=%d\n", (int)__LINE__, errno);
                rc = 1;
                break;
            }
        }
    }
    if (!rc) {
        if (Trace) { printf("shstrtab w at ofs %d\n", (int)_lseek(fd, 0, SEEK_CUR)); }
        if ((unsigned)_write(fd, &sectionNameTable[0], sectionNameTable.size())
            != sectionNameTable.size()) 
        {
            printf("Error at line %d errno=%d\n", (int)__LINE__, errno);
            rc = 1;
        }
    }
    if (!rc) {
        if (alignPos(fd, ALIGN)) {
          printf("Error at line %d errno=%d\n", (int)__LINE__, errno);
          rc = 1;
        }
    }
    if (!rc) {
        if (Trace) { printf("ST ofs %d\n", (int)_lseek(fd, 0, SEEK_CUR)); }
        if (_write(fd, &sectionHeaders[0], elfHeader.e_shnum * elfHeader.e_shentsize)
            != elfHeader.e_shnum * elfHeader.e_shentsize)
        {
            printf("Error at line %d errno=%d\n", (int)__LINE__, errno);
            rc = 1; 
        }
    }
    _close(fd);
    return rc;
};

int fileLoadAll(const char* filename, const bool is_mandatory, std::vector<char>& content) {

    int fd = _open(filename, _O_RDONLY | _O_BINARY);
    if (fd < 0) {
        printf("%s: Can't open '%s' (%d)\n", is_mandatory ? "Error":"Info", filename, errno);
        return is_mandatory ? 1:2;
    }
    
    int rc = 0;
    struct _stat fs = {0};
    rc = _fstat(fd,&fs);
    if (rc) {
      printf("Error %d getting attributes of '%s'\n", errno, filename);
      rc = 1; // fstat may return negative value, let's normalize
    }
    if (!rc) {
      if (! (fs.st_mode & _S_IFREG)) {
        printf("Error: '%s' is not an ordinary file (st_mode=%d)\n", filename, (unsigned)fs.st_mode);
        rc = 1;
      }
    }
    if (!rc) {
      if (loadData(content, fd, 0, fs.st_size)) {
        printf("Error %d reading '%s'\n", errno, filename);
        rc = 1;
      }
    }
    _close(fd);
    if (!rc) {
        printf("Info: Ok '%s' read %u bytes\n", filename, (unsigned)fs.st_size);
    }
    return rc;
}

int fileCreateAndSave(
  const std::string& filename,
  const std::vector<char>& content)
{
  int fd = _open(filename.c_str(), _O_CREAT|_O_TRUNC|_O_RDWR|_O_BINARY, 0666);
  if (fd < 0) {
    printf("Error saving '%s' (open errno=%d)\n", filename.c_str(), errno);
    return 1;
  }
  int rc = 0;
  if ((unsigned)_write(fd, &content[0], content.size()) != content.size()) {
    printf("Error saving '%s' (write errno=%d)\n", filename.c_str(), errno);
    rc = 1;
  }
  _close(fd);
  if (rc == 0) {
    printf("Info: Ok '%s' saved %u bytes\n", filename.c_str(), (unsigned)(content.size()));
  }
  return rc;
};

#define APP_DESCRIPTION "HSA Fat Object File manipulation utility"
#define EXT_DEVICE "brig"
#define EXT_HOST   "obj"
#define EXT_OPTION "opt"
#define EXT_FATOBJ "fatobj"
#define ELF_SECNAME_DEVICE ".hsa_file_device"
#define ELF_SECNAME_HOST   ".hsa_file_host"
#define ELF_SECNAME_OPTION ".hsa_file_option"
#define ELF_SECNAME_BRIG_DIRECTIVES ".directives"
#define ELF_SECNAME_BIF_DIRECTIVES  ".brig_directives"
#define ELF_FILE_TYPE_HSA_FAT_OBJECT (llvm::ELF::ET_LOPROC+0xad)

static void printVersion() {
  using namespace llvm;
  (void)(outs() << APP_DESCRIPTION ".\n");
  outs() << "  (C) AMD 2012, all rights reserved.\n"; // TODO_HSA (3) YYYY-YYYY
  outs() << "  Built " << __DATE__ << " (" << __TIME__ << ").\n";
  // TODO_HSA (3) versioning?
  // outs() << "  Version " << "0.1" << ".\n";
}


static 
int fileLoadToSection(
   const std::string& file_ext
  ,const char * const section_name
  ,const bool is_mandatory
  ,std::vector<char>& file_contents // out
  ,SaveElfSections& sections) // in-out
{
  {
    // file_ext can be empty; omit check
    assert(strlen(section_name) > 0);
    assert(file_contents.size() == 0);
    SaveElfSections::iterator p = sections.find(section_name);
    assert(p == sections.end());
  }
  std::string filespec = Pathname + "." + file_ext;
  int rc  = fileLoadAll(filespec.c_str(),is_mandatory,file_contents);
  if (rc == 2) {
    rc = 0;  // it is ok if non-mandatory file not found
  }
  else if (rc == 0) {
    sections[section_name].push_back(
      SectionPdataAndSize(&file_contents[0],file_contents.size()));
  }
  return rc;
}

// Let's have application's return values in [0,255] range.
// [1...127] is for informational return values.
// 0 and [128...255] for error codes (0 means 'no error').

// Convert internal error code to application's return value.
// Input error code must be in [0...127]
static int errorCode2appRc(const int ec) {
    assert(ec >= 0 && ec <= 127); 
    return (ec == 0) ? 0 : ec + 0x80;
}

// Convert internal informational return value to application's return value.
// Input value must be in [0...126]
static int infoValue2appRc(const int val) {
    assert(val >= 0 && val <= 126);
    return val + 1;
}

static
bool isSectionFound (const LoadElfSections& in, std::string s_name) {
    bool found = false;
    for(LoadElfSections::const_iterator p = in.begin(), p_end = in.end(); p != p_end && !found; ++p) {
        found = (p->first == s_name);
    }
    return found;
}

int main(int argc, char **argv) {
  using namespace llvm;
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);
  cl::SetVersionPrinter(printVersion);
  cl::ParseCommandLineOptions(argc, argv, APP_DESCRIPTION "\n");
  int rc = 0;

  if (ModeRaw) {
      LoadElfSections in;
      rc = loadElf(Pathname.c_str(), &in, NULL);
      SaveElfSections out;
      if (!rc) {
        for(LoadElfSections::iterator p = in.begin(); p != in.end(); ++p) {
          out[p->first].push_back(SectionPdataAndSize(&p->second[0], p->second.size()));
        }
      }
      std::string out_pathname = OutputPathname;
      if (!rc) {
        if (out_pathname.size() == 0) {
          printf("Error: in raw mode, output filename must be set (-debug-o)\n");
          rc = 1;
        }
      }
      if (!rc) {
        rc = saveElf(out_pathname.c_str(), out, NULL);
      }
      rc = errorCode2appRc(rc);
  }
  else if (Mode == M_Join || Mode == M_Create) {
    const bool obsolete = (Mode == M_Join);
    SaveElfSections out;
    std::vector<char> fc_device;
    std::vector<char> fc_host;
    std::vector<char> fc_option;

    /*    */ rc = fileLoadToSection(EXT_DEVICE ,ELF_SECNAME_DEVICE ,false, fc_device ,out);
    if (!rc) rc = fileLoadToSection(EXT_HOST   ,ELF_SECNAME_HOST   ,!obsolete , fc_host   ,out);
    if (!rc) rc = fileLoadToSection(EXT_OPTION ,ELF_SECNAME_OPTION ,false, fc_option ,out);
    
    if (!rc) {
      // write output elf
      std::string op = Pathname;
      if (OutputPathname.size() != 0) { op = OutputPathname; }
      SaveElfParams sep = { ELF_FILE_TYPE_HSA_FAT_OBJECT };
      if (obsolete)
        rc = saveElf((op + "." EXT_FATOBJ).c_str(), out, &sep);
      else
        rc = saveElf(op.c_str(), out, &sep);
    }
    rc = errorCode2appRc(rc);
  }
  else if (Mode == M_Split || Mode == M_Extract) {
    LoadElfSections       in;
    llvm::ELF::Elf32_Ehdr in_header;
    const bool obsolete = (Mode == M_Split);
    std::string  in_filename = Pathname;
    if (obsolete) { in_filename += "." EXT_FATOBJ; }
    
    // load and check elf
    rc = loadElf(in_filename.c_str(), &in, &in_header);
    if (!rc) {
      if (in_header.e_type != ELF_FILE_TYPE_HSA_FAT_OBJECT) {
        printf("Error: '%s': bad ELF file type (0x%x)\n", in_filename.c_str(), (unsigned)in_header.e_type);
        rc = 1;
      }
    }
    if (!rc && !obsolete) {
      // check that mandatory section exists
      if (!isSectionFound(in,ELF_SECNAME_HOST)) {
        printf("Error: '%s': Host object not found\n", Pathname.c_str());
        rc = 1;
      }
    }
    if (!rc) {
      // iterate thru all sections and save files
      std::string op = Pathname;
      if (OutputPathname.size() != 0) { op = OutputPathname; }
     
      for(LoadElfSections::iterator p = in.begin(); p != in.end(); ++p) {
        if (p->first == ELF_SECNAME_DEVICE) {
          rc |= fileCreateAndSave(op + "." + EXT_DEVICE,p->second);
          // do not break on errors
        }
        else if (p->first == ELF_SECNAME_HOST) {
          rc |= fileCreateAndSave(op + "." + EXT_HOST,p->second);
          // do not break on errors
        }
        else if (p->first == ELF_SECNAME_OPTION) {
          rc |= fileCreateAndSave(op + "." + EXT_OPTION,p->second);
          // do not break on errors
        }
        else {
          printf("Warning: '%s': unknown section '%s'\n", Pathname.c_str(), p->first.c_str());
        }
      }
    }
    rc = errorCode2appRc(rc);
  }
  else if (Mode == M_Detect) {
    LoadElfSections in;
    llvm::ELF::Elf32_Ehdr in_header;
    enum {
       FT_UNKNOWN = 0
      ,FT_FATOBJ
      ,FT_BRIG
      ,FT_BIF
    } ft = FT_UNKNOWN;
    const char* ft_str[] = {
       "non-HSA"
      ,"Fat Object"
      ,"'old plain' ELF/BRIG"
      ,"BIF"
    };
    
    // load as elf and check e_type & e_machine
    rc = loadElf(Pathname.c_str(), &in, &in_header);
    if (rc) {
      ft = FT_UNKNOWN;
      if (rc == 1) {
//        printf("Info: '%s': Not an ELF file\n", Pathname.c_str());
        rc = 0; // just unknown file type but NOT an error.
      }
      else {
        printf("Info: '%s': Can't read file\n", Pathname.c_str());
      }
    }
    else {
//      printf("Info: '%s': ELF file type (0x%x), machine type (0x%x)\n"
//        ,Pathname.c_str(), (unsigned)in_header.e_type, (unsigned)in_header.e_machine);
      if (in_header.e_type    == ELF_FILE_TYPE_HSA_FAT_OBJECT
      &&  in_header.e_machine == llvm::ELF::EM_NONE) {
        ft = FT_FATOBJ;
      }
      else if (in_header.e_type    == llvm::ELF::ET_NONE
      &&       in_header.e_machine == llvm::ELF::EM_NONE) {
        ft = FT_BRIG;
      }
      else if (in_header.e_type    == llvm::ELF::ET_REL
      &&       in_header.e_machine == 0xAF5A) { // TODO_HSA (3) EM_HSAIL from compiler\lib\loaders\elf\utils\common\elfdefinitions.h
        ft = FT_BIF;
      }
      else {
        ft = FT_UNKNOWN;
      }
    }
    // Mandatory section validation is not performed by -detect-type command
    // and is performed by tools themselves (fatobj for *.fatobj, devlink for *.brig)
    // check that mandatory section(s) exists
    //if (!rc && FT_UNKNOWN != ft) {
    //  std::string section = "$$$";
    //  switch(ft) {
    //  case FT_FATOBJ: section = ELF_SECNAME_HOST;            break;
    //  case FT_BRIG:   section = ELF_SECNAME_BRIG_DIRECTIVES; break;
    //  case FT_BIF:    section = ELF_SECNAME_BIF_DIRECTIVES;  break;
    //  case FT_UNKNOWN: // never reached
    //  default:
    //    assert(!"Internal error\n"); 
    //    printf("INTERNAL ERROR\n");
    //    rc = 1; 
    //    break;
    //  }
    //  if (isSectionFound(in,section)) {
    //    printf("Info: '%s': mandatory section '%s' found\n", Pathname.c_str(), section.c_str());
    //  }
    //  else {
    //    ft = FT_UNKNOWN;
    //    printf("Info: '%s': mandatory section '%s' not found\n", Pathname.c_str(), section.c_str());
    //  }
    //}
    //// compose return value
    if (rc) {
      rc = errorCode2appRc(rc);
    }
    else {
      rc = infoValue2appRc(ft);
    }
    printf("Info: '%s': file type is: %s (%d)\n", Pathname.c_str(), ft_str[ft], rc);
  }
  else {
    assert(!"InternalError\n");
    printf("INTERNAL ERROR\n");
    rc = 1;
    rc = errorCode2appRc(rc);
  }
  return rc;
}
