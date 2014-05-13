//===-- HSAILParseMetadata.h - Parse SPIR metadata --------------*- C++ -*-===//
//
//
//===----------------------------------------------------------------------===//
///
/// \file
/// 
/// \brief This file declares the API for parsing SPIR 1.2 metadata
/// and storing that information in an HSAILModuleInfo object.
///
//===----------------------------------------------------------------------===//

#ifndef __HSAIL_PARSE_METADATA_H__
#define __HSAIL_PARSE_METADATA_H__

namespace llvm {
  class HSAILModuleInfo;
  class Module;

  void parseMetadata(llvm::HSAILModuleInfo *mInfo, const llvm::Module *M);
  
}

#endif
