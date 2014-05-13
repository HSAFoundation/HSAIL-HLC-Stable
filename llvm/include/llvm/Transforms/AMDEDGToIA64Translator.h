#ifndef LLVM_AMDEDGTOIA64TRANSLATOR_H
#define LLVM_AMDEDGTOIA64TRANSLATOR_H

#include "llvm/Module.h"
#include <string>

namespace llvm {

/// \brief Translate OCL builtin names from EDG mangling to IA64.
///
/// This should be done before linking on all user modules.
///
/// \param M Moduel to be transformed.
/// \param [out] errorMessage contains the error message if returning false.
/// \return true if no error was found.
bool AMDEDGToIA64Translate(
    llvm::Module &M,
    std::string *errorMessage = NULL);

}  // End llvm namespace
  
#endif
