//===-- HSAILParseMetadata.cpp - Parse SPIR metadata ------------*- C++ -*-===//
//
//
//===----------------------------------------------------------------------===//
///
/// \file
/// 
/// \brief This file implements the API for parsing SPIR 1.2 metadata
/// and storing that information in an HSAILModuleInfo object.
///
//===----------------------------------------------------------------------===//

#include "HSAILMetadataUtils.h"
#include "HSAILModuleInfo.h"
#include "HSAILParseMetadata.h"
#include "HSAILUtilityFunctions.h"

#include "llvm/Metadata.h"

using namespace llvm;

/// \brief Set user-specified values for required workgroup size
static void parseRequiredWorkgroupSize(HSAILKernel *K, const MDNode *Q)
{
  assert(!K->sgv);
  K->sgv = new HSAILKernelAttr();

  assert(Q->getNumOperands() == 4);
  
  // SPIR requires that all three dimensions be set in the metadata.
  // Skip first operand, which is the name of the MDNode
  for (unsigned i = 1; i != 4; ++i) {
    ConstantInt *C = cast<ConstantInt>(Q->getOperand(i));
    K->sgv->reqGroupSize[i-1] = (uint32_t)C->getZExtValue();
  }

  K->sgv->mHasRWG = true;
}

/// \brief Track read/write access qualifiers for kernel arguments of
/// image type.
///
/// The SPIR metadata contains one access qualifier for each kernel
/// argument in sequence, irrespective of whether it is an image or
/// not. HSAILModuleInfo assigns a sequence to all image-type
/// arguments while skipping over other arguments. The indexes into
/// this sequence are stored in separate sets for read-only and
/// write-only images respectively.
static void parseAccessQualifiers(HSAILKernel *K, const MDNode *Q)
{
  assert(K->readOnly.empty());
  assert(K->writeOnly.empty());
  
  // Incremented only when an image-type argument is encountered.
  unsigned ImgCount = 0;

  // Skip first operand, which is the name of the MDNode
  for (unsigned i = 1, e = Q->getNumOperands(); i != e; ++i) {
    MDString *QualNode = cast<MDString>(Q->getOperand(i));
    StringRef Qual = QualNode->getString();

    if (Qual.equals("read_only"))
      K->readOnly.insert(ImgCount++);
    else if (Qual.equals("write_only"))
      K->writeOnly.insert(ImgCount++);
    else 
      assert(Qual.equals("none"));
    // "read_write" cannot occur in OCL 1.2
  }
}

/// \brief Retrieve and store the type names of kernel arguments
static void parseArgTypeNames(HSAILKernel *K, const MDNode *Q) {
  // Skip first operand, which is the name of the MDNode
  for (unsigned i = 1, e = Q->getNumOperands(); i != e; ++i) {
    MDString *QualNode = cast<MDString>(Q->getOperand(i));
    StringRef Qual = QualNode->getString();

    K->ArgTypeNames.push_back(Qual.str());
  }
}

/// \brief Parse list of metadata nodes corresponding to one kernel
static void parseKernelInfo(HSAILKernel *K, const MDNode *KInfo)
{
  // skip the first operand, which is the kernel signature
  for (unsigned i = 1, e = KInfo->getNumOperands(); i != e; ++i) {
    const MDNode *Q = cast<MDNode>(KInfo->getOperand(i));

    if (matchName(Q, "reqd_work_group_size")) {
      parseRequiredWorkgroupSize(K, Q);
      continue;
    }
 
    if (matchName(Q, "kernel_arg_access_qual")) {
      parseAccessQualifiers(K, Q);
      continue;
    }

    if (matchName(Q, "kernel_arg_type")) {
      parseArgTypeNames(K, Q);
      continue;
    }
  }
}

/// \brief Entry-point for parsing SPIR metadata stored in
/// "opencl.kernels"
void llvm::parseMetadata(HSAILModuleInfo *mInfo, const Module *M)
{
  const NamedMDNode *MD = M->getNamedMetadata("opencl.kernels");
  assert(MD);

  for (unsigned i = 0, e = MD->getNumOperands(); i != e; ++i) {
    const MDNode *KInfo = MD->getOperand(i);

    const Function *F = cast<Function>(KInfo->getOperand(0));
    assert(isKernelFunc(F));
    StringRef Name = F->getName();
    
    HSAILKernel *K = new HSAILKernel();
    K->mKernel = true;
    K->mName = Name;
    mInfo->mKernels[Name] = K;
    
    parseKernelInfo(K, KInfo);
  }
}

