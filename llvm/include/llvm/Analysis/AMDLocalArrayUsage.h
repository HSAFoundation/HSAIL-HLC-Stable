#ifndef LLVM_AMDLOCALARRAYUSAGE_H
#define LLVM_AMDLOCALARRAYUSAGE_H

#include "llvm/Module.h"
#include <string>

namespace llvm {

/// \brief Check if kernel containing local arrays are called by another kernel.
///
/// This should be done after linking.
///
/// \param M linked module to be checked.
/// \param [out] errorMessage contains the error message if returning false.
/// \return true if no error was found.
bool AMDCheckLocalArrayUsage(
    const llvm::Module &M,
    std::string *errorMessage = NULL);

}  // End llvm namespace
  
#endif
