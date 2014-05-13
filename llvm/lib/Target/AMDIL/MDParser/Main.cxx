#include "AMDILMDInterface.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <string>

static char* readFile(std::string source_filename, size_t& size) {
  FILE *fp = ::fopen( source_filename.c_str(), "rb" );
  unsigned int length;
  size_t offset = 0;
  char *ptr;

  if (!fp) {
    return NULL;
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
  return ptr;
}

int main(int argc, char* argv[]) {
  llvm::CompUnit* mdFile;
  size_t size;
  char* s = readFile(std::string(argv[1]), size);
  if (s == NULL) {
    std::cerr << "Error: failed reading input file" << std::endl;
    return -1;
  }
  try {
    mdFile = new llvm::CompUnit(s);
  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << std::endl;
    return -1;
  }
  if (mdFile->hasError()) {
    std::cerr << "Error: " << mdFile->errorMsg() << std::endl;
    return -1;
  }
  ::free(s);
  mdFile->setLineNumberDisplay(false);
  for (size_t x = 0, y = mdFile->getNumKernels(); x < y; ++x) {
    std::string name = mdFile->getKernelName(x) + ".il";
    std::ofstream output(name.c_str());
    if (output.is_open()) {
      std::string ptr = mdFile->getKernelStr(x);
      output << ptr;
      output.close();
    }

    //std::cout << mdFile->getKernelStr(x) << '\n';
  }
#if 0
  std::string ErrorInfo;
  const char *filename = "debugData.dbg";
  llvm::raw_fd_ostream debug(filename, ErrorInfo,
                             llvm::raw_fd_ostream::F_Binary);
  if (!debug.has_error()) {
    debug << (*mdFile->getDebugData());
    debug << mdFile->getILStr();
//    llvm::CompUnit* newMD = new llvm::CompUnit(mdFile->getILStr());
//    debug << *newMD;
//    delete newMD;
    debug.close();
  }
  mdFile->setLineNumberDisplay(true);

  delete mdFile;
#endif
  return 0;
}
