//===-- inflate.cpp - Inflates user host program with device code binary data.//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"

using namespace llvm;

namespace {

cl::opt<std::string>
HostFilename("h"
  ,cl::Hidden
  ,cl::Optional
  ,cl::desc("Host source file name (C++/text)")
  ,cl::init("")
  ,cl::value_desc("pathname"));

cl::opt<bool>
Obsolete("obsolete"
  ,cl::Hidden
  ,cl::Optional
  ,cl::desc("Work just like obsolete versions:"
            "\n\t\t\t\t > Requires Host source file name."
            "\n\t\t\t\t > Populates Host source by generated array."
            "\n\t\t\t\t > Default array name is '__device_object'."
            "\n\t\t\t\t > C storage class for array is 'static'."
            )
  ,cl::init(false));

cl::opt<std::string>
DeviceFilename("d"
  ,cl::Required
  ,cl::desc("Device object file name (ELF/binary)")
  ,cl::init("")
  ,cl::value_desc("pathname"));

cl::opt<std::string>
OutputFilename("o"
  ,cl::Required
  ,cl::desc("Output file name (C++/text)")
  ,cl::init("")
  ,cl::value_desc("pathname"));

cl::opt<std::string>
ElfDataName("n"
  ,cl::Optional
  ,cl::desc("Name of generated C/C++ array."
            "\n\t\t\t > The array will hold the contents of device object file."
            "\n\t\t\t > Should be a valid C identifier."
            "\n\t\t\t > Default is '__hsa_device_object'." /* TODO_HSA */
            )
  ,cl::value_desc("identifier")
  ,cl::init("")); // default is set explicitly as affected by -obsolete

cl::opt<bool>
Trace("debug-trace-print"
  ,cl::Hidden
  ,cl::desc("Print trace information")
  ,cl::init(false));

} // namespace;
    
#define APP_DESCRIPTION "HSA HLC Inflate utility"

// Replaces in s_source all occurences of s1 with s2
std::string ReplaceMultiple(const std::string &s_source, std::string s1, std::string s2)
{
    if (0 == s_source.size())
        return s_source;
    size_t s1_size = s1.size();
    if (0 == s1_size)
        return s_source;
    std::string s_dest = s_source;
    size_t iFind = 0;
    while (std::string::npos != iFind)
    {
        iFind = s_dest.find(s1);
        if (std::string::npos != iFind)
            s_dest.replace(iFind, s1.size(), s2);
    }
    return s_dest;
}

static void printVersion() {
  using namespace llvm;
  printf(APP_DESCRIPTION ".\n");
  printf("  (C) AMD 2012, all rights reserved.\n"); // TODO_HSA (3) YYYY-YYYY
  printf("  Built %s (%s)\n", __DATE__, __TIME__);
  printf("  Version 0.2\n"); // TODO_HSA (3)
}

int main(int argc, char **argv) {
  // Print a stack trace if we signal out.
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);
  llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
  cl::SetVersionPrinter(printVersion);

  // Parse the command line.
  cl::ParseCommandLineOptions(argc, argv, APP_DESCRIPTION "\n");

  int rc = 0;
  // try to analyze as much options as possible
  if (ElfDataName.size() == 0) {
    if (Obsolete) {
      ElfDataName = "__device_object";
    }
    else {
      ElfDataName = "__hsa_device_object";
    }
  }
  if (Obsolete && HostFilename.size() == 0) {
    printf("Error: Host file name is not specified (hint: -h).\n");
    rc = EXIT_FAILURE;
  }
  if (!Obsolete && HostFilename.size() != 0) {
    printf("Warning: Host file name specified but ignored (specify -obsolete for old inflate behavior).\n");
  }
  if (DeviceFilename.size() == 0) {
    printf("Error: Device code file name is not specified (hint: -d).\n");
    rc = EXIT_FAILURE;
  }
  if (OutputFilename.size() == 0) {
    printf("Error: Output file name is not specified (hint: -o).\n");
    rc = EXIT_FAILURE;
  }
  if (HostFilename.size() != 0
  &&  strcmp(OutputFilename.c_str(),HostFilename.c_str()) == 0) {
    printf("Error: Output and Host file names should be different.\n");
    printf("Info:   Host file name == '%s'\n",  HostFilename.c_str());
    printf("Info: Output file name == '%s'\n",OutputFilename.c_str());
    rc = EXIT_FAILURE;
  }
  if (rc) { return rc; }

  if (Trace) {
    printf("Trace: HostFilename == '%s'\n",  HostFilename.c_str());
    printf("Trace: DeviceFilename == '%s'\n",DeviceFilename.c_str());
    printf("Trace: OutputFilename == '%s'\n",OutputFilename.c_str());
    printf("Trace: ElfDataName == '%s'\n",ElfDataName.c_str());
  }

  FILE *fp_dev = fopen( DeviceFilename.c_str(), "rb" );
  
  if (!fp_dev ) {
    printf("Error - Cannot read device file.\n");
    return EXIT_FAILURE;
  }
  if (!Obsolete) {
    rc = fseek(fp_dev, 0, SEEK_END);
    if ( rc !=0 || ftell(fp_dev) <= 0 ) {
      fclose(fp_dev);
      printf("Error: Empty Device object file.\n");
      return EXIT_FAILURE;
    }
  }
    
  FILE *fp_hos = NULL;
  FILE *fp_out = fopen( OutputFilename.c_str(), "w" );
  
  if (Obsolete) {
    fp_hos = fopen( HostFilename.c_str(), "rb" );  
    if (!fp_hos) {
      fclose(fp_dev);
      printf("Error - Cannot read host file.\n");
      return EXIT_FAILURE;
    }
  }

  if (!fp_out) {
    fclose(fp_dev);
    if (fp_hos) { fclose(fp_hos); }
    printf("Error - Cannot open output file.\n");
    return EXIT_FAILURE;
  }

  rc = fseek(fp_dev, 0, SEEK_SET);
  if (rc != 0) {
    fclose(fp_out);
    fclose(fp_dev);
    if (fp_hos) { fclose(fp_hos); }
    printf("Error: Can't access Device object file.\n");
    return EXIT_FAILURE;
  }
  
  if (Obsolete) {
    fprintf(fp_out, "\nstatic char %s[] = {\n  ", ElfDataName.c_str());
  }
  else {
    fprintf(fp_out, "#include <stddef.h>\n");
    fprintf(fp_out, "static char device_object[] = {\n  ");
  }
  int byte;
  unsigned int count = 0;
  while( (byte =  fgetc(fp_dev)) != EOF) { 
    if (Obsolete) {
      // leads to "warning C4309: 'initializing' : truncation of constant value"
      fprintf(fp_out, "0x%.2x, ", byte); 
    } else {
      fprintf(fp_out
        ,"%4d, " // -128...127, 4 chars max
        ,(int)(char)byte // getc reads character as unsigned char
        ); 
    }
    if( (++count % 10) == 0 ) fprintf(fp_out, "\n  ");
  }
  fprintf(fp_out, "\n};\n");
  
  if (!Obsolete) {
    fprintf(fp_out, "extern char * const %s = &device_object[0];\n"
      ,ElfDataName.c_str());
    fprintf(fp_out, "extern size_t const %s = sizeof(device_object);\n"
      ,(ElfDataName + "_size").c_str());
  }

  if (Obsolete) {
    rc = fseek(fp_hos, 0, SEEK_END);
    if (rc) {
      printf("Error accessing Host source file (seek rc=%d).\n", rc);
      rc = EXIT_FAILURE;
    }

    long hos_ftell_rc = 0;
    if (!rc) {
      hos_ftell_rc = ftell(fp_hos);
      if ( hos_ftell_rc <= 0 ) {
        printf("Error: Empty Host source file (ftell rc=%ld).\n", hos_ftell_rc);
        rc = EXIT_FAILURE;
      }
    }
    
    size_t hos_length = 0;
    if (!rc) {
      hos_length = (size_t)hos_ftell_rc;
      fprintf(fp_out, "#line 0 \"%s\"\n", ReplaceMultiple(HostFilename.c_str(), "\\", "/").c_str());
      rc = fseek(fp_hos, 0, SEEK_SET);
      if (rc) {
        printf("Error accessing Host source file (seek rc=%d).\n", rc);
        rc = EXIT_FAILURE;
      }
    }
    
    char* ptr = NULL;
    if (!rc) {
      ptr = new char[hos_length];
      size_t read_size = fread(&ptr[0], 1, hos_length, fp_hos);
      if (hos_length != read_size) {
        printf("Error: Cannot read Host source file.\n");
        rc = EXIT_FAILURE; 
      }
    }
    
    if (!rc) {
      if (hos_length != fwrite(&ptr[0], 1, hos_length, fp_out)) {
        printf("Error: Cannot write Output file.\n");
        rc = EXIT_FAILURE; 
      }
    }
    
    if (rc) {
      if (ptr) { delete [] ptr; }
      fclose(fp_hos);
      fclose(fp_dev);
      fclose(fp_out);
      return rc;
    }
  }
  
  if (fp_hos) { fclose(fp_hos); }
  fclose(fp_dev);
  fclose(fp_out);  
  return EXIT_SUCCESS;
}
