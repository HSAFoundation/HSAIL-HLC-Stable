//===-- llvm-objgen.cpp - Creates an Object file -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This program is an utility that generates an object file.
// This is experimental code and will be superseded by an assembler that will
// also generate the object by calling out to WriteObject. 
// Only supports ELF as object format.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Triple.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCObjectStreamer.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/system_error.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ELF.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetAsmParser.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

namespace {
  cl::opt<std::string>
  InputFilename(cl::Positional, cl::desc("<input HSAIL file>"), cl::init("-"));

  cl::opt<std::string>
  OutputFilename("o", cl::desc("Output filename"),
                 cl::value_desc("filename"));

  cl::opt<bool>
  HSAIL_64("hsail_64", cl::desc("HSAIL-64 (HSAIL-32 is default)"));
}

static tool_output_file *GetOutputStream() {
  if (OutputFilename == "")
    OutputFilename = "-";

  std::string Err;
  tool_output_file *Out = new tool_output_file(OutputFilename.c_str(), Err,
                                               raw_fd_ostream::F_Binary);

  if (!Err.empty()) {
    errs() << Err << '\n';
    delete Out;
    return 0;
  }

  return Out;
}

static std::string readFile(const std::string src, size_t& size) {
  const char *source = src.c_str();
  FILE *fp = ::fopen( source, "rb" );
  unsigned int length;
  size_t offset = 0;
  char *ptr;
  std::string result = "";

  if (!fp) {
    return result;
  }

  // obtain file size.
  ::fseek (fp , 0 , SEEK_END);
  length = ::ftell (fp);
  ::rewind (fp);

  ptr = new char[offset + length + 1];

  if (length != fread(&ptr[offset], 1, length, fp))
  {
    delete [] ptr;
    return NULL;
  }

  ptr[offset + length] = '\0';
  size = offset + length;
  ::fclose(fp);

  result = ptr;
  return result;
}

int main(int argc, char **argv) {
  // Print a stack trace if we signal out.
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);
  llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.

  // Initialize targets. and assembly printers/parsers.
  llvm::InitializeAllTargets();

  // Parse the command line.
  cl::ParseCommandLineOptions(argc, argv, "llvm object file generator\n");
  
  // Read the input file.
  size_t size = 0;
  std::string Source = readFile(InputFilename, size);
  if (!size) {
    printf("Error - Cannot open input HSAIL assembly file %s or file is empty.\n", argv[1]);
    return EXIT_FAILURE;
  }

  // Create the output stream.
  OwningPtr<tool_output_file> Out(GetOutputStream());
  if (!Out) {
    errs() << argv[0] << ": Unable to get output stream.\n";
    return 1;
  }

  // Create HSAIL only target related info.
  std::string TripleName = 
    HSAIL_64 ? "hsail_64-unknown-unknown" : "hsail-unknown-unknown";
  std::string Error;
  const Target *TheTarget = TargetRegistry::lookupTarget(TripleName, Error);
  if (!TheTarget) {
    errs() << argv[0] << ": error: unable to get target for '" << 
      TripleName << "'.\n";
    return 1;
  }
  MCAsmBackend *TAB = TheTarget->createMCAsmBackend(TripleName);
  MCObjectWriter *MOW = TAB->createObjectWriter(Out->os());

  SourceMgr SrcMgr;

  MCAsmInfo *MAI = TheTarget->createMCAsmInfo(TripleName);

  MCRegisterInfo *MRI = TheTarget->createMCRegInfo(TripleName);

  // FIXME: This is not pretty. MCContext has a ptr to MCObjectFileInfo and
  // MCObjectFileInfo needs a MCContext reference in order to initialize itself.
  MCObjectFileInfo *MOFI = new MCObjectFileInfo();
  MCContext Ctx(*MAI, *MRI, MOFI, &SrcMgr);

  std::string FeaturesStr;
  TargetOptions Options;
  TargetMachine *TM(TheTarget->createTargetMachine(StringRef(TripleName), NULL,
                 StringRef(FeaturesStr), Options));

  const MCAsmInfo *tai = new MCAsmInfo(*TM->getMCAsmInfo());

  OwningPtr<MCInstrInfo> MCII(TheTarget->createMCInstrInfo());
  OwningPtr<MCSubtargetInfo>
    STI(TheTarget->createMCSubtargetInfo(TripleName, NULL, FeaturesStr));

  MCCodeEmitter *CE = TheTarget->createMCCodeEmitter(*MCII, *STI, Ctx);
  MCObjectStreamer *MOS = 
    (MCObjectStreamer *)TheTarget->createMCObjectStreamer(TripleName, Ctx, *TAB,
							Out->os(), CE, false, 
							false);
  MCAssembler &MAS = MOS->getAssembler();
  MCAsmLayout MAL(MAS);

  // Create the .hsail section.
  // In the future, the .hsail section may contain groups of kernels.
  const StringRef SectionName = ".hsail";
  const MCSectionELF *Section;
  Section = Ctx.getELFSection(SectionName, ELF::SHT_PROGBITS, 0, 
			      SectionKind::getReadOnly(), size, "");

  MCSectionData &SD = MAS.getOrCreateSectionData(*Section);
  SD.setAlignment( HSAIL_64 ? 8 : 4 );

  // Extract the .version string. Add this to every new kernel code.
  unsigned int version_npos = Source.find("version");
  unsigned int version_end_npos = Source.find(";");
  assert(strstr(Source.c_str(), "version") &&
         "Expected version string in the HSAIL text.");
  std::string version_string = Source.substr(version_npos, version_end_npos);
  version_string += "\n";

  // skip over declarations to first kernel
  Source = Source.substr(Source.find("kernel &"), Source.length()-1);
  
  // For all kernels, create a label pointing to our kernel in the .hsail
  // section.
  int kernel_npos;
  int kernel_end_npos;
  std::string  kernel_name;
  int kernel_code_end_npos;
  std::string  kernel_code;
  unsigned int Offset = 0;
  MCDataFragment *DF = new MCDataFragment(&SD);

  while(strstr(Source.c_str(), "kernel &") != NULL) {
    // Find out the name of the kernel.
    kernel_npos = Source.find("kernel &");
    kernel_end_npos = Source.find("(");
    kernel_name = Source.substr(kernel_npos+8,kernel_end_npos-kernel_npos-8);
    kernel_name = "__FSA_" + kernel_name + "_hsail";

    StringRef kernelName(kernel_name);
    MCSymbol *Symbol = Ctx.GetOrCreateSymbol(kernelName);
    MOS->SwitchSection(Section);
    MOS->EmitLabel(Symbol);
    MOS->getAssembler().getSymbolData(*Symbol).setOffset(Offset); 

    // Find out the code for this kernel.
    kernel_code_end_npos = Source.find("};");
    kernel_code = Source.substr(kernel_npos, kernel_code_end_npos+2-kernel_npos);

    std::string fragment_string = version_string + kernel_code;

    // Write out our source as raw data in the .hsail section.
    DF->getContents().append(fragment_string.begin(), fragment_string.end());

    Offset += fragment_string.length();

    Source = Source.substr(Source.find("};")+2, Source.length()-1);
  }
  
  SD.setHasInstructions(true);

  // Finally, call the object writer to write out the object.
  // TODO: check if fail; expected to pass always.
  MOW->WriteObject(MAS, MAL);

  Out->keep();

  return 0;
}
