//===------------- AMDILMDTypes.cpp - AMDIL Metadata Types ---------------===//
// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// If you use the software (in whole or in part), you shall adhere to all
// applicable U.S., European, and other export laws, including but not limited
// to the U.S. Export Administration Regulations (EAR), (15 C.F.R. Sections
// 730 through 774), and E.U. Council Regulation (EC) No 1334/2000 of 22 June
// 2000.  Further, pursuant to Section 740.6 of the EAR, you hereby certify
// that, except pursuant to a license granted by the United States Department
// of Commerce Bureau of Industry and Security or as otherwise permitted
// pursuant to a License Exception under the U.S. Export Administration
// Regulations ("EAR"), you will not (1) export, re-export or release to a
// national of a country in Country Groups D:1, E:1 or E:2 any restricted
// technology, software, or source code you receive hereunder, or (2) export to
// Country Groups D:1, E:1 or E:2 the direct product of such technology or
// software, if such foreign produced direct product is subject to national
// security controls as identified on the Commerce Control List (currently
// found in Supplement 1 to Part 774 of EAR).  For the most current Country
// Group listings, or for additional information about the EAR or your
// obligations under those regulations, please refer to the U.S. Bureau of
// Industry and Securitys website at http://www.bis.doc.gov/.
//
//==-----------------------------------------------------------------------===//
//
//  Implementation of the AMDIL Metadata Types. The AMDIL Metadata types are
//  types that are used by the Metadata parser. These types represent either
//  metadata nodes, debug nodes, data nodes, or IL nodes. These nodes are
//  then used to reconstruct the IL stream for each kernel with a fully
//  analyzed call structure.
//
//===---------------------------------------------------------------------===//

#define DEBUG_TYPE "amdilmdtypes"

#include "AMDILMDTypes.h"

#include "AMDILDwarfParser.h"
#include "AMDILMDScanner.h"
#include "AMDILMDInterface.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <list>
#include <queue>
#include <string>


using namespace llvm;

//
// @param name
// @brief strips KERNEL_PREFIX and KERNEL_SUFFIX from the name
// and returns that name if both of the tokens are present.
//

static std::string StripWrapper(const std::string &name) {
  size_t start = name.find("__OpenCL_");
  size_t end = name.find("_kernel");
  if (start == std::string::npos
      || end == std::string::npos
      || (start == end)) {
    return name;
  } else {
    return name.substr(9, name.length()-16);
  }
}

//===----------------------------- MDType --------------------------------===//
MDType::MDType(std::string name) {
  TypeID_ = MDDefaultTy;
  Name_ = name;
}

MDType::~MDType()
{
}
/// The default dump functions, only
/// prints out name and ID
  void
MDType::dump()
{
  errs() << "Node = "
         << Name_
         << " Type(" << TypeID_ << ") "
         << typeIDToName(TypeID_)
         << '\n';
}

  const char*
MDType::typeIDToName(unsigned ID)
{
  switch(ID) {
    default:
      break;
    case MDStringsTy:
      return "String";
    case MDMemoryTy:
      return "Memory";
    case MDFlagTy:
      return "Flag";
    case MDIntTy:
      return "Integer";
    case MDReflectionTy:
      return "Reflection";
    case MDConstArgTy:
      return "ConstArg";
    case MDIntListTy:
      return "IntegerList";
    case MDPrintfTy:
      return "Printf";
    case MDArgTy:
      return "GenericArg";
    case MDCBArgTy:
      return "ArgWCB";
    case MDSamplerTy:
      return "Sampler";
    case MDImageTy:
      return "Image";
    case MDCounterTy:
      return "Counter";
    case MDSemaphoreTy:
      return "Semaphore";
    case MDValueTy:
      return "Value";
    case MDPointerTy:
      return "Pointer";
    case MDDefaultTy:
      return "Default";
  };
  return "unknown";
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDType& MDNode)
{
  if (MDNode.getMDStrings()) {
    os << *MDNode.getMDStrings();
  } else if (MDNode.getMDMemory()) {
    os << *MDNode.getMDMemory();
  } else if (MDNode.getMDFlag()) {
    os << *MDNode.getMDFlag();
  } else if (MDNode.getMDInt()) {
    os << *MDNode.getMDInt();
  } else if (MDNode.getMDReflection()) {
    os << *MDNode.getMDReflection();
  } else if (MDNode.getMDConstArg()) {
    os << *MDNode.getMDConstArg();
  } else if (MDNode.getMDIntList()) {
    os << *MDNode.getMDIntList();
  } else if (MDNode.getMDArg()) {
    os << *MDNode.getMDArg();
  }

  return os;
}
//===--------------------------- MDStrings -------------------------------===//
  MDStrings::MDStrings(std::string name)
: MDType(name)
{
  TypeID_ = MDStringsTy;
}
MDStrings::~MDStrings()
{
}
  void
MDStrings::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDStrings& MDNode)
{
  os << MDNode.Name_ << ":";
  os << MDNode.Str_;
  os << "\n";
  return os;
}
//===--------------------------- MDMemory --------------------------------===//
  MDMemory::MDMemory(std::string name)
: MDType(name)
{
  TypeID_ = MDMemoryTy;
}
MDMemory::~MDMemory()
{
}
  void
MDMemory::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDMemory& MDNode)
{
  os << MDNode.Name_ << ":" << MDNode.AS_ << ":" << MDNode.Size_ << "\n";
  return os;
}
//===--------------------------- MDFlag ----------------------------------===//
  MDFlag::MDFlag(std::string name)
: MDType(name)
{
  TypeID_ = MDFlagTy;
}
MDFlag::~MDFlag()
{
}
  void
MDFlag::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDFlag& MDNode)
{
  if (MDNode.Name_ != ";limitworkgroupsize") {
    os << MDNode.Name_ << "\n";
  } else {
    os << ";memory" << ":" << MDNode.Name_ << "\n";
  }
  return os;
}
//===--------------------------- MDInt -----------------------------------===//
  MDInt::MDInt(std::string name)
: MDType(name)
{
  TypeID_ = MDIntTy;
}
MDInt::~MDInt()
{
}
  void
MDInt::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDInt& MDNode)
{
  os << MDNode.Name_ << ":" << MDNode.Int_ << "\n";
  return os;
}
//===--------------------------- MDReflection -----------------------------------===//
  MDReflection::MDReflection(std::string name)
: MDInt(name)
{
  TypeID_ = MDReflectionTy;
}
MDReflection::~MDReflection()
{
}
  void
MDReflection::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDReflection& MDNode)
{
  os << ";reflection:" << MDNode.Int_ << ":" << MDNode.Name_ << "\n";
  return os;
}
//===--------------------------- MDConstArg -----------------------------------===//
  MDConstArg::MDConstArg(std::string name)
: MDInt(name)
{
  TypeID_ = MDConstArgTy;
}
MDConstArg::~MDConstArg()
{
}
  void
MDConstArg::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDConstArg& MDNode)
{
  os << ";constarg:" << MDNode.Int_ << ":" << MDNode.Name_ << "\n";
  return os;
}

//===--------------------------- MDIntList -------------------------------===//
  MDIntList::MDIntList(std::string name)
: MDType(name)
{
  TypeID_ = MDIntListTy;
  IntList_ = NULL;
}
MDIntList::~MDIntList()
{
  if (IntList_) {
    delete IntList_;
  }
}
  void
MDIntList::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDIntList& MDNode)
{
  if (MDNode.getMDPrintf()) {
    os << *MDNode.getMDPrintf();
  } else {
    os << MDNode.Name_;
    if (MDNode.IntList_) {
      for (unsigned x = 0; x < MDNode.IntList_->size(); ++x) {
        os << ":" << (*MDNode.IntList_)[x];
      }
    }
    os << "\n";
  }
  return os;
}
//===--------------------------- MDPrintf --------------------------------===//
  MDPrintf::MDPrintf(std::string name)
: MDIntList(name)
{
  TypeID_ = MDPrintfTy;
  Str_ = NULL;
}
MDPrintf::~MDPrintf()
{
  if (Str_) {
    delete [] Str_;
  }
}
  void
MDPrintf::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDPrintf& MDNode)
{
  os << MDNode.Name_;
  if (MDNode.IntList_) {
    for (unsigned x = 0; x < MDNode.IntList_->size(); ++x) {
      os << ":" << (*MDNode.IntList_)[x];
    }
  }
  if (MDNode.Str_) {
    os << MDNode.Str_;
  }
  os << "\n";
  return os;
}
//===--------------------------- MDArg -----------------------------------===//
  MDArg::MDArg(std::string name)
: MDType(name)
{
  TypeID_ = MDArgTy;
}
MDArg::~MDArg()
{
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDArg& MDNode)
{
  if (MDNode.getMDSampler()) {
    os << *MDNode.getMDSampler();
  } else if (MDNode.getMDCBArg()) {
    os << *MDNode.getMDCBArg();
  }  else {
    os << MDNode.Name_ << ":" << MDNode.Arg_;
  }
  return os;
}
//===--------------------------- MDSampler -------------------------------===//
  MDSampler::MDSampler(std::string name)
: MDArg(name)
{
  TypeID_ = MDSamplerTy;
}
MDSampler::~MDSampler()
{
}
  void
MDSampler::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDSampler& MDNode)
{
  os << MDNode.Name_ << ":" << MDNode.Arg_ << ":" << MDNode.ID_ << ":" ;
  os << ((MDNode.isArg_) ? 0 : 1) << ":" << MDNode.Val_ << "\n";
  return os;
}
//===--------------------------- MDCBArg ---------------------------------===//
  MDCBArg::MDCBArg(std::string name)
: MDArg(name)
{
  TypeID_ = MDCBArgTy;
}
MDCBArg::~MDCBArg()
{
}
  void
MDCBArg::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDCBArg& MDNode)
{
  if (MDNode.getMDImage()) {
    os << *MDNode.getMDImage();
  } else if (MDNode.getMDCounter()) {
    os << *MDNode.getMDCounter();
  } else if (MDNode.getMDSemaphore()) {
    os << *MDNode.getMDSemaphore();
  } else if (MDNode.getMDValue()) {
    os << *MDNode.getMDValue();
  } else if (MDNode.getMDPointer()) {
    os << *MDNode.getMDPointer();
  } else {
    os << ":" << MDNode.CBNum_ << ":" << MDNode.CBOffset_ << "\n";
  }
  return os;
}
//===--------------------------- MDImage ---------------------------------===//
  MDImage::MDImage(std::string name)
: MDCBArg(name)
{
  TypeID_ = MDImageTy;
}
MDImage::~MDImage()
{
}
  void
MDImage::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDImage& MDNode)
{
  os << MDNode.Name_ << ":" << MDNode.Arg_ << ":";
  os << MDNode.Dim_ << ":" << MDNode.Type_ << ":" << MDNode.ID_;
  os << ":" << MDNode.CBNum_ << ":" << MDNode.CBOffset_ << "\n";
  return os;
}
//===--------------------------- MDSemaphore -------------------------------===//
  MDSemaphore::MDSemaphore(std::string name)
: MDCBArg(name)
{
  TypeID_ = MDSemaphoreTy;
}
MDSemaphore::~MDSemaphore()
{
}
  void
MDSemaphore::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDSemaphore& MDNode)
{
  os << MDNode.Name_ << ":" << MDNode.Arg_ << ":" << MDNode.ID_;
  os << ":" << MDNode.CBNum_ << ":" << MDNode.CBOffset_ << "\n";
  return os;
}
//===--------------------------- MDCounter -------------------------------===//
  MDCounter::MDCounter(std::string name)
: MDCBArg(name)
{
  TypeID_ = MDCounterTy;
}
MDCounter::~MDCounter()
{
}
  void
MDCounter::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDCounter& MDNode)
{
  os << MDNode.Name_ << ":" << MDNode.Arg_ << ":" << MDNode.Size_ << ":" << MDNode.ID_;
  os << ":" << MDNode.CBNum_ << ":" << MDNode.CBOffset_ << "\n";
  return os;
}
//===--------------------------- MDValue ---------------------------------===//
  MDValue::MDValue(std::string name)
: MDCBArg(name)
{
  TypeID_ = MDValueTy;
}
MDValue::~MDValue()
{
}
  void
MDValue::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDValue& MDNode)
{
  os << MDNode.Name_ << ":" << MDNode.Arg_ << ":" << MDNode.Type_ << ":" << MDNode.Size_;
  os << ":" << MDNode.CBNum_ << ":" << MDNode.CBOffset_ << "\n";
  return os;
}
//===--------------------------- MDPointer -------------------------------===//
  MDPointer::MDPointer(std::string name)
: MDValue(name)
{
  TypeID_ = MDPointerTy;
  AccessType_ = "RW";
  Volatile_ = 0;
  Restrict_ = 0;
}
MDPointer::~MDPointer()
{
}
  void
MDPointer::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MDPointer& MDNode)
{
  os << MDNode.Name_ << ":" << MDNode.Arg_ << ":";
  os << MDNode.Type_ << ":" << MDNode.Size_;
  os << ":" << MDNode.CBNum_ << ":" << MDNode.CBOffset_;
  os << ":" << MDNode.MemType_ << ":";
  os << MDNode.BufNum_ << ":" << MDNode.Alignment_;
  os << ":" << MDNode.AccessType_ << ":";
  os << MDNode.Volatile_ << ":" << MDNode.Restrict_;
  os << "\n";
  return os;
}
//===----------------------------- Component -----------------------------===//
Component::Component(std::string name, unsigned SubClassID)
  : FuncName_(name), SubClassID_(SubClassID)
{
}
Component::~Component()
{
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, Component& C)
{
  if (MDBlock *Meta = dyn_cast<MDBlock>(&C)) {
    os << *Meta;
  } else if (ILFunc *IL = dyn_cast<ILFunc>(&C)) {
    os << *IL;
  } else if (DBSection *DBG = dyn_cast<DBSection>(&C)) {
    os << *DBG;
  } else if (Macro *M = dyn_cast<Macro>(&C)) {
    os << *M;
  } else if (DataSection *Data = dyn_cast<DataSection>(&C)) {
    os << *Data;
  }
  return os;
}
//===----------------------------- StmtBlock -----------------------------===//
  StmtBlock::StmtBlock(std::string name, unsigned SubClassID)
: Component(name, SubClassID)
{
  baseLineNum_ = 0;
  lineNum_ = 0;
  useLineNum_ = false;
  Stmts_ = NULL;
  LineNums_.clear();
}
StmtBlock::~StmtBlock()
{
  if (Stmts_) {
    std::list<std::string*>::iterator sIter;
    for (sIter = Stmts_->begin(); sIter != Stmts_->end(); ++sIter) {
      if (*sIter) {
        delete *sIter;
      }
    }
    delete Stmts_;
  }
}
  void
StmtBlock::dump()
{
  errs() << FuncName_ << "\n";
}
  void
StmtBlock::setLineNum(unsigned i)
{
  DEBUG(dbgs() <<  FuncName_ << "-LineNum start: " << i << " ");
  baseLineNum_ = i;
  lineNum_ = 0;
}
  unsigned
StmtBlock::getBaseLineNum()
{
  return baseLineNum_;
}
  unsigned
StmtBlock::getLineNum()
{
  DEBUG(dbgs() << "LineNum End: " << lineNum_ + baseLineNum_
               << ":" << lineNum_ << ":" << baseLineNum_ << "\n");
  return lineNum_ + baseLineNum_;
}
  void
StmtBlock::useLineNum(bool b)
{
  useLineNum_ = b;
}

  StmtBlock::iterator
StmtBlock::begin()
{
  if (!Stmts_) {
    Stmts_ = new std::list<std::string*>;
    Stmts_->clear();
  }
  return Stmts_->begin();
}

  StmtBlock::iterator
StmtBlock::end()
{
  if (!Stmts_) {
    Stmts_ = new std::list<std::string*>;
    Stmts_->clear();
  }
  return Stmts_->end();
}

  size_t
StmtBlock::size()
{
  if (!Stmts_) {
    Stmts_ = new std::list<std::string*>;
    Stmts_->clear();
  }
  return Stmts_->size();
}

  void
StmtBlock::computeLineNumbers()
{
  lineNum_ = 0;
  LineNums_.clear();
  for (StmtBlock::iterator sb = begin(),
      se = end(); sb != se; ++sb) {
    LineNums_.push_back(baseLineNum_ + lineNum_++);
  }
  assert(LineNums_.size() == size()
      && "Somehow the line number vector and the statement"
      " vector fell out of sync!");
}

  void
StmtBlock::setStatements(std::list<std::string*>* stmts)
{
  if (Stmts_) {
    std::list<std::string*>::iterator stIter;
    for (stIter = Stmts_->begin();
        stIter != Stmts_->end();
        ++stIter) {
      if (*stIter) {
        delete *stIter;
      }
    }
    delete Stmts_;
  }
  Stmts_ = stmts;
}

  void
StmtBlock::process()
{
}

  raw_ostream&
llvm::operator<<(raw_ostream& os, StmtBlock& stm)
{
  unsigned lineNum = (unsigned)-1;
  for (StmtBlock::iterator sb = stm.begin(),
      se = stm.end(); sb != se; ++sb) {
    os << (**sb);
    if (stm.useLineNum_ ) {
      lineNum++;
      os << " ; " << (stm.baseLineNum_ + lineNum);
    }
    os << "\n";
  }
  return os;
}
//===------------------------------ MDBlock ------------------------------===//
  MDBlock::MDBlock(std::string name)
: Component(name, MDBlockClass)
{
}
MDBlock::~MDBlock()
{
  if (!MDVals) {
    return;
  }
  std::vector<MDType*>::iterator mdIter;
  for (mdIter = MDVals->begin(); mdIter != MDVals->end(); ++mdIter) {
    MDType* ptr = (*mdIter);
    if (ptr) {
      delete ptr;
    }
  }
  delete MDVals;
}
  void
MDBlock::dump()
{
  errs() << "MD Block: " << FuncName_ << "\n";
  if (!MDVals) {
    return;
  }
  for (size_t x = 0; x < MDVals->size(); ++x) {
    (*MDVals)[x]->dump();
  }
}
  void
MDBlock::process()
{
  if (!MDVals) {
    return;
  }
  for (std::vector<MDType*>::iterator mdb = MDVals->begin(),
      mde = MDVals->end(); mdb != mde; ++mdb) {
    MDValMap_[(*mdb)->Name_] = *mdb;
  }
}

static
llvm::MemoryType strToMemType(std::string &MemType_) {
  if (MemType_ == "g") {
    return llvm::PTR_MT_GLOBAL;
  } else if (MemType_ == "p") {
    return llvm::PTR_MT_SCRATCH_EMU;
  } else if (MemType_ == "l") {
    return llvm::PTR_MT_LDS_EMU;
  } else if (MemType_ == "uav") {
    return llvm::PTR_MT_UAV;
  } else if (MemType_ == "c") {
    return llvm::PTR_MT_CONSTANT_EMU;
  } else if (MemType_ == "r") {
    return llvm::PTR_MT_GDS_EMU;
  } else if (MemType_ == "hl") {
    return llvm::PTR_MT_LDS;
  } else if (MemType_ == "hp") {
    return llvm::PTR_MT_SCRATCH;
  } else if (MemType_ == "hc") {
    return llvm::PTR_MT_CONSTANT;
  } else if (MemType_ == "hr") {
    return llvm::PTR_MT_GDS;
  } else if (MemType_ == "up") {
    return llvm::PTR_MT_UAV_SCRATCH;
  } else if (MemType_ == "uc") {
    return llvm::PTR_MT_UAV_CONSTANT;
  } else if (MemType_ == "flat") {
    return llvm::PTR_MT_FLAT;
  } else {
    llvm_unreachable("Found a MemType that we don't handle correctly!");
  }
  return llvm::PTR_MT_UAV;
}

  static
llvm::DataType strToDataType(std::string &dataType)
{
  const char* str = dataType.c_str();
#if defined(AMD_OPENCL) || 1
  if (!memcmp(str, "i1", 2) && (dataType.size() == 2)) {
#else
  if (!memcmp(str, "i1", 2)) {
#endif 
	  return llvm::DATATYPE_i1;
  } else if (!memcmp(str, "i8", 2)) {
    return llvm::DATATYPE_i8;
  } else if (!memcmp(str, "i16", 3)) {
    return llvm::DATATYPE_i16;
  } else if (!memcmp(str, "i32", 3)) {
    return llvm::DATATYPE_i32;
  } else if (!memcmp(str, "i64", 3)) {
    return llvm::DATATYPE_i64;
  } else if (!memcmp(str, "u8", 2)) {
    return llvm::DATATYPE_u8;
  } else if (!memcmp(str, "u16", 3)) {
    return llvm::DATATYPE_u16;
  } else if (!memcmp(str, "u32", 3)) {
    return llvm::DATATYPE_u32;
  } else if (!memcmp(str, "u64", 3)) {
    return llvm::DATATYPE_u64;
  } else if (!memcmp(str, "half", 4)) {
    return llvm::DATATYPE_half;
  } else if (!memcmp(str, "float", 5)) {
    return llvm::DATATYPE_float;
  } else if (!memcmp(str, "double", 6)) {
    return llvm::DATATYPE_double;
  } else if (!memcmp(str, "struct", 6)) {
    return llvm::DATATYPE_struct;
  } else if (!memcmp(str, "union", 5)) {
    return llvm::DATATYPE_union;
  } else if (!memcmp(str, "event", 5)) {
    return llvm::DATATYPE_event;
  } else if (!memcmp(str, "opaque", 6)) {
    return llvm::DATATYPE_opaque;
  } else {
    llvm_unreachable("Found a data type string that isn't handled correctly!");
  }
  return llvm::DATATYPE_unknown;
}

void MDBlock::updateMetadata(AMDILMetadata *MD) {
  if (!MDVals) {
    return;
  }

  for (std::vector<MDType*>::iterator mdb = MDVals->begin(),
      mde = MDVals->end(); mdb != mde; ++mdb) {
    MDType* curMD = *mdb;
    const char* nameStr = curMD->Name_.c_str();
    switch (curMD->getType()) {
      default:
        llvm_unreachable("Found an MDType that isn't handled correctly!");
        break;
      case llvm::MDStringsTy:
        if (!memcmp(nameStr, ";device", 7)) {
          MD->deviceName = curMD->getMDStrings()->Str_;
        } else if (!memcmp(nameStr, ";error", 6)) {
          MD->errorMsgs.insert(curMD->getMDStrings()->Str_);
        } else if (!memcmp(nameStr, ";warning", 8)) {
          MD->warningMsgs.insert(curMD->getMDStrings()->Str_);
        } else {
          llvm_unreachable("Found an MDType that isn't handled correctly!");
        }
        break;
      case llvm::MDIntListTy:
        if (!memcmp(nameStr, ";version", 8)) {
          MD->major = ((*curMD->getMDIntList()->IntList_)[0]);
          MD->minor = ((*curMD->getMDIntList()->IntList_)[1]);
          MD->revision = ((*curMD->getMDIntList()->IntList_)[2]);
        } else if (!memcmp(nameStr, ";function", 9)) {
        } else if (!memcmp(nameStr, ";intrinsic", 10)) {
        } else if (!memcmp(nameStr, ";cws", 4)) {
          MD->hasCWS = true;
          MD->cws[0] = ((*curMD->getMDIntList()->IntList_)[0]);
          MD->cws[1] = ((*curMD->getMDIntList()->IntList_)[1]);
          MD->cws[2] = ((*curMD->getMDIntList()->IntList_)[2]);
        } else {
          llvm_unreachable("Found an MDType that isn't handled correctly!");
        }
        break;
      case llvm::MDMemoryTy:
        if (!memcmp(nameStr, ";memory", 7)) {
          const char* SA = curMD->getMDMemory()->AS_.c_str();
          uint32_t size = curMD->getMDMemory()->Size_;
          if (!memcmp(SA, "local", 5)) {
            MD->localMem += size;
          } else if (!memcmp(SA, "region", 6)) {
            MD->regionMem += size;
          } else if (!memcmp(SA, "private", 7)) {
            MD->privateMem += size;
          } else if (!memcmp(SA, "hwprivate", 9)) {
            MD->hwprivateMem += size;
          } else if (!memcmp(SA, "uavprivate", 10)) {
            MD->hwprivateMem += size;
          } else if (!memcmp(SA, "hwregion", 8)) {
            MD->hwregionMem += size;
          } else if (!memcmp(SA, "hwlocal", 7)) {
            MD->hwlocalMem += size;
          } else {
            llvm_unreachable("Found an AS that isn't handled correctly!");
          }
        } else {
          llvm_unreachable("Found an MDType that isn't handled correctly!");
        }
        break;
      case llvm::MDFlagTy:
        if (!memcmp(nameStr, "compilerwrite", 13)) {
          MD->compilerWrite = 1;
        } else if (!memcmp(nameStr, "datareqd", 8)) {
          MD->datareqd = 1;
        } else if (!memcmp(nameStr, "packedreg", 9)) {
          MD->packedReg = 1;
        } else if (!memcmp(nameStr, "gws", 3)) {
          MD->gws = 1;
        } else if (!memcmp(nameStr, "swgws", 5)) {
          MD->swgws = 1;
        } else if (!memcmp(nameStr, "64bitABI", 8)) {
          MD->abi64 = 1;
        } else {
          llvm_unreachable("Found an MDType that isn't handled correctly!");
        }
        break;
      case llvm::MDReflectionTy:
        MD->reflection[curMD->getMDReflection()->Int_] = nameStr;
        break;
      case llvm::MDConstArgTy:
        MD->constarg[nameStr] = curMD->getMDConstArg()->Int_;
        break;
      case llvm::MDIntTy:
        if (!memcmp(nameStr, ";uniqueid", 9)) {
          // We don't need to overwrite this if it is already set.
          // This is because every metadata block has a unique ID,
          // but we only want the ID of the kernel and that should
          // always get parsed first.
          if (!MD->funcID) {
            MD->funcID = curMD->getMDInt()->Int_;
          }
        } else if (!memcmp(nameStr, ";uavid", 6)) {
          MD->uavID = curMD->getMDInt()->Int_;
        } else if (!memcmp(nameStr, ";privateid", 10)) {
          MD->privateID = curMD->getMDInt()->Int_;
        } else if (!memcmp(nameStr, ";printfid", 9)) {
          MD->printfID = curMD->getMDInt()->Int_;
        } else if (!memcmp(nameStr, ";cbid", 5)) {
          MD->cbID = curMD->getMDInt()->Int_;
        } else if (!memcmp(nameStr, ";lws", 4)) {
          MD->lws = curMD->getMDInt()->Int_;
        } else {
          llvm_unreachable("Found an MDType that isn't handled correctly!");
        }
        break;
      case llvm::MDPrintfTy:
        if (!memcmp(nameStr, ";printf", 7)) {
          printfFmt curPF;
          curPF.ID = (*(curMD->getMDIntList()->IntList_))[0];
          for (unsigned i = 2, e = (curMD->getMDIntList()
                ->IntList_)->size(); i < e; ++i) {
            curPF.argSizes.push_back((*curMD->getMDIntList()->IntList_)[i]);
          }
          curPF.fmtStr = std::string(curMD->getMDPrintf()->Str_,
              curMD->getMDPrintf()->StrLen_);
          MD->printfStrs.push_back(curPF);
        } else {
          llvm_unreachable("Found an MDType that isn't handled correctly!");
        }
        break;
      case llvm::MDSamplerTy:
        if (!memcmp(nameStr, ";sampler", 8)) {
          argType curArg;
          class MDSampler* sampler = curMD->getMDSampler();
          curArg.type = llvm::ARG_TYPE_SAMPLER;
          curArg.isConst = false;
          curArg.argName = curMD->getMDArg()->Arg_;
          curArg.arg.sampler.ID = sampler->ID_;
          curArg.arg.sampler.isKernelDefined = sampler->isArg_;
          curArg.arg.sampler.value = sampler->Val_;
          MD->arguments.push_back(curArg);
        } else {
          llvm_unreachable("Found an MDType that isn't handled correctly!");
        }
        break;
      case llvm::MDImageTy:
        if (!memcmp(nameStr, ";image", 6)) {
          argType curArg;
          curArg.type = llvm::ARG_TYPE_IMAGE;
          curArg.isConst = false;
          class MDImage* image = curMD->getMDImage();
          curArg.argName = curMD->getMDArg()->Arg_;
          if (image->Dim_ == "2D") {
          curArg.arg.image.is2D = true;
          } else if (image->Dim_ == "2DA") {
          curArg.arg.image.is2D = true;
          curArg.arg.image.isArray = true;
          } else if (image->Dim_ == "1D") {
          curArg.arg.image.is1D = true;
          } else if (image->Dim_ == "1DA") {
          curArg.arg.image.is2D = true;
          curArg.arg.image.isArray = true;
          } else if (image->Dim_ == "1DB") {
          curArg.arg.image.is2D = true;
          curArg.arg.image.isBuffer = true;
          }
          if (image->Type_ == "RO") {
            curArg.arg.image.type = llvm::ACCESS_TYPE_RO;
          } else if (image->Type_ == "WO") {
            curArg.arg.image.type = llvm::ACCESS_TYPE_WO;
          } else if (image->Type_ == "RW") {
            curArg.arg.image.type = llvm::ACCESS_TYPE_RW;
          } else {
            llvm_unreachable("Found an image type that isn't handled correctly!");
          }
          curArg.arg.image.resID = image->ID_;
          curArg.arg.image.cbNum = image->CBNum_;
          curArg.arg.image.cbOffset = image->CBOffset_;
          MD->arguments.push_back(curArg);
        } else {
          llvm_unreachable("Found an MDType that isn't handled correctly!");
        }
        break;
      case llvm::MDCounterTy:
        if (!memcmp(nameStr, ";counter", 8)) {
          argType curArg;
          curArg.type = llvm::ARG_TYPE_COUNTER;
          curArg.isConst = false;
          curArg.argName = curMD->getMDArg()->Arg_;
          class MDCounter* counter = curMD->getMDCounter();
          curArg.arg.counter.is32bit = (counter->Size_ == 32);
          curArg.arg.counter.resID = counter->ID_;
          curArg.arg.counter.cbNum = counter->CBNum_;
          curArg.arg.counter.cbOffset = counter->CBOffset_;
          MD->arguments.push_back(curArg);
        } else {
          llvm_unreachable("Found an MDType that isn't handled correctly!");
        }
        break;
      case llvm::MDSemaphoreTy:
        if (!memcmp(nameStr, ";sema", 8)) {
          argType curArg;
          curArg.type = llvm::ARG_TYPE_COUNTER;
          curArg.isConst = false;
          curArg.argName = curMD->getMDArg()->Arg_;
          class MDSemaphore *sema = curMD->getMDSemaphore();
          curArg.arg.sema.resID = sema->ID_;
          curArg.arg.sema.cbNum = sema->CBNum_;
          curArg.arg.sema.cbOffset = sema->CBOffset_;
          MD->arguments.push_back(curArg);
        } else {
          llvm_unreachable("Found an MDType that isn't handled correctly!");
        }
        break;
      case llvm::MDValueTy:
        if (!memcmp(nameStr, ";value", 6)) {
          argType curArg;
          curArg.type = llvm::ARG_TYPE_VALUE;
          curArg.isConst = false;
          class MDValue* value = curMD->getMDValue();
          curArg.argName = curMD->getMDArg()->Arg_;
          curArg.arg.value.data = strToDataType(value->Type_);
          curArg.arg.value.numElements = value->Size_;
          curArg.arg.value.cbNum = value->CBNum_;
          curArg.arg.value.cbOffset = value->CBOffset_;
          MD->arguments.push_back(curArg);
        } else {
          llvm_unreachable("Found an MDType that isn't handled correctly!");
        }
        break;
      case llvm::MDPointerTy:
        if (!memcmp(nameStr, ";pointer", 8)) {
          argType curArg;
          curArg.type = llvm::ARG_TYPE_POINTER;
          curArg.isConst = false;
          class MDPointer* pointer = curMD->getMDPointer();
          curArg.argName = pointer->Arg_;
          curArg.arg.pointer.data = strToDataType(pointer->Type_);
          curArg.arg.pointer.numElements = 1;
          curArg.arg.pointer.cbNum = pointer->CBNum_;
          curArg.arg.pointer.cbOffset = pointer->CBOffset_;
          curArg.arg.pointer.bufNum = pointer->BufNum_;
          curArg.arg.pointer.align = pointer->Alignment_;
          curArg.arg.pointer.memory = strToMemType(pointer->MemType_);
         if (pointer->AccessType_ == "RO") {
            curArg.arg.pointer.type = llvm::ACCESS_TYPE_RO;
          } else if (pointer->AccessType_ == "WO") {
            curArg.arg.pointer.type = llvm::ACCESS_TYPE_WO;
          } else {
            curArg.arg.pointer.type = llvm::ACCESS_TYPE_RW;
          }
         curArg.arg.pointer.volatile_ = pointer->Volatile_;
         curArg.arg.pointer.restrict_ = pointer->Restrict_;
          MD->arguments.push_back(curArg);
        } else {
          llvm_unreachable("Found an MDType that isn't handled correctly!");
        }
        break;
    };
  }
  for (unsigned x = 0, y = MD->arguments.size(); x < y; ++x) {
    if (MD->constarg.find(MD->arguments[x].argName) != MD->constarg.end()) {
      MD->arguments[x].isConst = true;
    }
  }
  unsigned arg_size = MD->arguments.size();
  for (std::map<unsigned, std::string>::iterator rB = MD->reflection.begin(),
    rE = MD->reflection.end(); rB != rE; ++rB) {
    if (rB->first < arg_size) {
      MD->arguments[rB->first].reflectStr = rB->second;
    }
  }
}

  raw_ostream&
llvm::operator<<(raw_ostream& os, MDBlock& md)
{
  os << ";ARGSTART:" << md.FuncName_ << "\n";
  if (md.MDVals) {
    for (std::vector<class MDType*>::iterator mdb = md.MDVals->begin(),
        mde = md.MDVals->end(); mdb != mde; ++mdb) {
      MDType* ptr = (*mdb);
      os << *ptr;
    }
  }
  os << ";ARGEND:" << md.FuncName_ << "\n";
  return os;
}
//===----------------------------- DBSection -----------------------------===//
  DBSection::DBSection(std::string name)
: StmtBlock(name, DBSectionClass)
{
}
DBSection::~DBSection()
{
}
  void
DBSection::dump()
{
  errs() << *this;
}
  void
DBSection::process()
{
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, DBSection& db)
{
  os << ";DEBUGSTART\n";
  os << *cast<StmtBlock>(&db);
  os << ";DEBUGEND\n";
  return os;
}
//===------------------------------ ILFunc -------------------------------===//
  ILFunc::ILFunc(unsigned ID, std::string name)
: FuncBase(name, ILFuncClass)
{
  ID_ = ID;
  lineNum_ = 0;
  data_ = NULL;
  metadata_ = NULL;
  mainFunc_ = NULL;
}
ILFunc::~ILFunc()
{
}
  void
ILFunc::dump()
{
  errs() << *this;
}
  bool
ILFunc::isKernel()
{
  return isKernel_;
}
  void
ILFunc::process()
{
  computeLineNumbers();
  size_t nameSize = FuncName_.size();
  isKernel_ = !FuncName_.compare(0, 8, "__OpenCL") &&
    !FuncName_.compare(nameSize - 7, nameSize, "_kernel");
  unsigned b = 0;
  for (StmtBlock::iterator bIter = begin(), eIter = end();
      bIter != eIter; ++bIter, ++b) {
    if (!(*bIter)->compare(0, 2, ";.")) {
        std::string str = *(*bIter);
        size_t size = str.find_first_of(':');
        labelMap_[str.substr(0, size)] = LineNums_[b];
    }
  }
}

  unsigned
ILFunc::getTmpLineNumber(const std::string& tmp)
{
  std::map<std::string, unsigned>::iterator lmIter;
  lmIter = labelMap_.find(tmp);
  if (lmIter == labelMap_.end()) {
    return (unsigned)-1;
  }
  return lmIter->second;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, ILFunc& i)
{
  DEBUG(
    os << "func " << i.ID_ << " ;" << i.FuncName_;
    if (i.useLineNum_) {
      os << ";" << i.baseLineNum_ << " ";
    }
    os << "\n";
  );
  os << *cast<StmtBlock>(&i);

  DEBUG(
    for (std::map<std::string, unsigned>::iterator lmb = i.labelMap_.begin(),
           lme = i.labelMap_.end(); lmb != lme; ++lmb) {
      os << ';' << lmb->first;
      if (i.useLineNum_) {
        os << ':' << lmb->second;
      }
      os << '\n';
    }
    os << ";" << i.FuncName_ << "\n";
  );

  return os;
}
///===------------------------------ MainFunc -------------------------------===//
  MainFunc::MainFunc(std::string name)
: StmtBlock(name, MainFuncClass)
{
  callID_ = (unsigned)-1;
}
MainFunc::~MainFunc()
{
}
  void
MainFunc::dump()
{
  errs() << *this;
}
  void
MainFunc::setFuncID(unsigned id)
{
  callID_ = id;
}
  void
MainFunc::process()
{
  computeLineNumbers();
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, MainFunc& i)
{
  unsigned lineNum_ = 0;
  for (StmtBlock::iterator sb = i.begin(), se = i.end(); sb != se; ++sb) {
    if (i.callID_ != (unsigned)-1
        && !memcmp((*sb)->c_str(), ";$$$$$$$$$$", 11)) {
      os << "call " << i.callID_;
      if (i.useLineNum_) {
        os << " ; " << lineNum_++ << " ";
      }
    } else {
      os << *(*sb);
      if (i.useLineNum_) {
        os << " ; " << lineNum_++ << " ";
      }
    }
    os << "\n";
  }
  return os;
}
//===---------------------------- DummyMacro -----------------------------===//
  DummyMacro::DummyMacro(std::string name)
: Macro(name, DummyMacroClass)
{
}
DummyMacro::~DummyMacro()
{
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, DummyMacro& m)
{
  os << m.header_ << m.FuncName_ << "\n";
  os << *cast<StmtBlock>(&m);
  os << m.footer_ << "\n";
  return os;
}
//===------------------------------- Macro -------------------------------===//
  Macro::Macro(std::string name)
: FuncBase(name, MacroClass)
{
}

  Macro::Macro(std::string name, unsigned SubClassID)
: FuncBase(name, SubClassID)
{
}

Macro::~Macro()
{
}

  void
Macro::dump()
{
  errs() << *this;
}
  void
Macro::process()
{
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, Macro& m)
{
  os << m.header_ << m.FuncName_ << "\n";
  os << *cast<StmtBlock>(&m);
  os << m.footer_ << "\n";
  return os;
}

//===------------------------------- Data --------------------------------===//
Data::Data(const std::string& name)
  : Size_(0),
    Offset_(0),
    Type_(name),
    Data_(NULL) {
}

Data::~Data() {
  if (Data_) {
    std::list<std::string*>::iterator dIter;
    for (dIter = Data_->begin(); dIter != Data_->end(); ++dIter) {
      if (*dIter) {
        delete *dIter;
      }
    }
    delete Data_;
  }
}
  void
Data::dump()
{
  errs() << *this;
}
  void
Data::process()
{
  if (Data_->size() < 2) {
    return;
  }

  std::list<std::string *>::iterator dBegin = Data_->begin();

  StringRef(**dBegin).getAsInteger(10, Offset_);
  dBegin = Data_->erase(dBegin);
  StringRef(**dBegin).getAsInteger(10, Size_);

  Data_->erase(dBegin);
}

  raw_ostream&
llvm::operator<<(raw_ostream& os, Data& d)
{
  if (d.Data_) {
    std::list<std::string*>::iterator dBegin = d.Data_->begin();
    std::list<std::string*>::iterator dEnd = d.Data_->end();
    os << ";#" << d.Type_ << ":" << d.Offset_ << ":" << d.Size_;
    while (dBegin != dEnd) {
      os << ":" << (**dBegin);
      ++dBegin;
    }
    os << "\n";
  }
  return os;
}
//===---------------------------- DataSection ----------------------------===//
  DataSection::DataSection(std::string name)
: Component(name, DataSectionClass)
{
}
DataSection::~DataSection()
{
  if (Data_) {
    std::vector<Data*>::iterator dIter;
    for (dIter = Data_->begin(); dIter != Data_->end(); ++dIter) {
      if (*dIter) {
        delete *dIter;
      }
    }
    delete Data_;
  }
}

  void
DataSection::dump()
{
  errs() << *this;
}
  void
DataSection::process()
{
    if (Data_) {
        std::for_each(Data_->begin(), Data_->end(),
                      std::mem_fun(&Data::process));
    }
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, DataSection& m)
{
  if (m.FuncName_ == "Software") {
    os << ";#DATASTART:" << m.Size_ << "\n";
  } else {
    os << ";#DATASTART:" << m.FuncName_ << ":" << m.Size_ << "\n";
  }
  if (m.Data_) {
    std::vector<Data*>::iterator dsb, dse;
    for (dsb = m.Data_->begin(), dse = m.Data_->end();
        dsb != dse; ++dsb) {
      os << *(*dsb);
    }
  }
  if (m.FuncName_ == "Software") {
    os << ";#DATAEND\n";
  } else {
    os << ";#DATAEND:" << m.FuncName_ << "\n";
  }
  return os;
}
//===---------------------------- AMDILDwarf -----------------------------===//

template <typename T>
static
void emit(
    char*& tempBuf,
    size_t& offset,
    size_t& size,
    T byte)
{
  if (size <= (offset + sizeof(T))) {
    // TODO: should we put these onto a
    // memory managment list so we don't have to
    // keep reallocating?
    char* tmpBuf = new char[size * 2];
    memset(tmpBuf, 0, size * 2);
    memcpy(tmpBuf, tempBuf, offset);
    size = size * 2;
    if (tempBuf) {
      delete [] tempBuf;
    }
    tempBuf = tmpBuf;
  }
  memcpy(tempBuf + offset, &byte, sizeof(T));
  offset += sizeof(T);
}
static
void emitStr(
    char*& tempBuf,
    size_t& offset,
    size_t& size,
    const char* ptr,
    size_t strSize)
{
  for (size_t x = 0; x < strSize; ++x) {
    if (!x && ptr[x] == '"') {
      continue;
    } else if (x == strSize-1 && ptr[x] == '"') {
      continue;
    }
    // We have an escaped char that we need to handle
    // seperately
    if (ptr[x] == '\\') {
      // Let's get the next character
      ++x;
      switch(ptr[x]) {
        default:
          // If the escape character is unrecognized, then
          // we should just emit the character with the '\'
          // skipped.
          emit(tempBuf, offset, size, ptr[x]);
          break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          {
            // Per the GAS spec, both '9' and '8' are
            // recognized
            unsigned number = ptr[x] - '0';
            // We have an octal number
            if ((ptr[x + 1] - '0') < 8) {
              ++x;
              number <<= 3;
              number += ptr[x] - '0';
            }
            if ((ptr[x + 1] - '0') < 8) {
              ++x;
              number <<= 3;
              number += ptr[x] - '0';
            }
            if (number < 256) {
              emit(tempBuf, offset, size, (unsigned char)number);
            } else {
              emit(tempBuf, offset, size, number);
            }
          }
          break;
        case 'b':
          emit(tempBuf, offset, size, '\b');
          break;
        case 'f':
          emit(tempBuf, offset, size, '\f');
          break;
        case 'n':
          emit(tempBuf, offset, size, '\n');
          break;
        case 'r':
          emit(tempBuf, offset, size, '\r');
          break;
        case 't':
          emit(tempBuf, offset, size, '\t');
          break;
        case 'X':
        case 'x':
          // We have a hex number
          {
            unsigned long long number = 0;
            unsigned numDigits = 0;
            unsigned pos = x + 1;
            while (pos < strSize) {
              if ((ptr[pos] >= '0' && ptr[pos] <= '9')) {
                number += (ptr[pos] - '0');
                number <<= 4;
                ++numDigits;
                ++pos;
              } else if ((ptr[pos] >= 'a' && ptr[pos] <= 'f')
                  || (ptr[pos] >= 'A' && ptr[pos] <= 'F')) {
                number += (ptr[pos] - '0');
                number <<= 4;
                ++numDigits;
                ++pos;
              } else {
                break;
              }
            }
            if (numDigits <= 8) {
              emit(tempBuf, offset, size, (unsigned)number);
            } else {
              emit(tempBuf, offset, size, number);
            }
          }
          break;
      };

    } else {
      emit(tempBuf, offset, size, ptr[x]);
    }
  }
}

// Tokens that mark the begin and end of a token string
static const char* startEndTokens[llvm::AMDILDwarf::DEBUG_LAST][2] =
{
  { ";.info_begin", ";.info_end" },
  { ";.abbrev_begin", ";.abbrev_end" },
  { ";.line_begin", ";.line_end" },
  { ";.pubnames_begin", ";.pubnames_end" },
  { ";.pubtypes_begin", ";.pubtypes_end" },
  { ";.loc_begin", ";.loc_end" },
  { ";.aranges_begin", ";.aranges_end" },
  { ";.ranges_begin", ";.ranges_end" },
  { ";.macinfo_begin", ";.macinfo_end" },
  { ";.str_begin", ";.str_end" },
  { ";.debug_frame_common_begin", ";.debug_frame_common_end"},
  { ";.static_func_begin", ";.static_func_end" },
  { ";.static_vars_begin", ";.static_vars_end" },
  { ";.weaknames_begin", ";.weaknames_end"},
};

static const char* dwarfNames[llvm::AMDILDwarf::DEBUG_LAST] =
{
  ".debug_info",
  ".debug_abbrev",
  ".debug_line",
  ".debug_pubnames",
  ".debug_pubtypes",
  ".debug_loc",
  ".debug_aranges",
  ".debug_ranges",
  ".debug_macinfo",
  ".debug_str",
  ".debug_frame",
  ".debug_static_func",
  ".debug_static_vars",
  ".debug_weaknames"
};

AMDILDwarf::AMDILDwarf(std::vector<DBSection*>& debugData,
    std::vector<ILFunc*>& ilData)
: mILData(ilData)
{
  for (size_t x = 0; x < AMDILDwarf::DEBUG_LAST; ++x) {
    dwarfSections[x] = new char[1024];
    memset(dwarfSections[x], 0, 1024);
    dwarfSizes[x] = 1024;
    dwarfOffsets[x] = 0;
    dwarfStrings[x].clear();
  }
  // most of the information is in the last debug block including all
  // of the debug sections for elf.
  ILFunc *currentFunc = NULL, *currentFuncBody = NULL;
  DwarfSection cds = DEBUG_INFO; // <-- short for currentDwarfSection
  AMDILDwarfParser parser(debugData);
  while (parser.next()) {
    switch (parser.getTokenType()) {
      case DWARF_ASCII:
      {
          const std::string &val = parser.getValue();
          assert(parser.isString(parser.getValueType())
              && "The type of value should be an ascii value!");
          size_t s = val.length();
          emitStr(dwarfSections[cds], dwarfOffsets[cds],
              dwarfSizes[cds], val.c_str(), s);
        }
        break;
      case DWARF_ASCIZ:
        {
          const std::string &val = parser.getValue();
          assert(parser.isString(parser.getValueType())
              && "The type of value should be an ascii value!");
          size_t s = val.length();
          emitStr(dwarfSections[cds], dwarfOffsets[cds],
              dwarfSizes[cds], val.c_str(), s);
          emit(dwarfSections[cds], dwarfOffsets[cds],
              dwarfSizes[cds], (char)0);
        }
        break;
      case DWARF_SECTION:
        {
          assert((parser.isLabel(parser.getValueType())
              || parser.isString(parser.getValueType()))
              && "The value of a dwarf section should always be a label or a string");
          if (parser.isLabel(parser.getValueType())) {
            const std::string& val = parser.getValue();
            DEBUG(dbgs() << "!!---- Section " << val);
            for (unsigned x = 0; x < DEBUG_LAST; ++x) {
              if (val.find(dwarfNames[x]) != std::string::npos) {
                cds = (DwarfSection)x;
                break;
              }
            }
            DEBUG(dbgs() << " Num: " << (unsigned)cds << " ----!!\n");
          } else if (parser.isString(parser.getValueType())) {
              // FIXME: Should we handle this case?
              // e.g. .section	".text.@__dot_4f32",#alloc,#execinstr
          }
        }
        break;
      case DWARF_LABEL:
        {
          const std::string& tkn = parser.getToken();
          if (!tkn.compare(0, 12, ";.func_begin")) {
            const std::string& val = parser.getValue();
            assert(parser.isText(parser.getValueType())
                && "The parser type for a func_begin token value must be "
                "dwarf text.");
            DEBUG(
              dbgs() << "Token: " << tkn << " ";
              dbgs() << "Changing function from \'"
              << (currentFunc ? currentFunc->FuncName_ : "main");
              dbgs() << "\' and body from \'"
              << (currentFuncBody ? currentFuncBody->FuncName_ : "main") << "\'";
            );
            size_t namePos = val.find_first_of('@') + 1;
            std::string currentFunction = val.substr(namePos, val.length());
            std::string currentFunctionBody = StripWrapper(currentFunction);
            for (std::vector<ILFunc*>::iterator
                ilBegin = mILData.begin(), ilEnd = mILData.end();
                ilBegin != ilEnd; ++ilBegin) {
              if ((*ilBegin)->FuncName_ == currentFunction) {
                currentFunc = (*ilBegin);
                // The currentFuncBody always appears after the
                // wrapper function.
                if (currentFunction != currentFunctionBody) {
                  currentFuncBody = *(++ilBegin);
                } else {
                  // We don't have a wrapper for this function,
                  // so the function and the body are the same.
                  currentFuncBody = currentFunc;
                }
                break;
              }
            }
            if (currentFunc) {
              // We want to trigger on the wrapper because that always comes first,
              // but we only want to keep track of line numbers on the actual body
              // since we can never debug into the kernel wrapper.
              currentFunc->labelMap_[tkn.substr(0, tkn.find_first_of(':')-1)]
                = currentFuncBody->getBaseLineNum();
              tokenDefMap_[tkn.substr(0, tkn.find_first_of(':'))] = currentFuncBody->getBaseLineNum();
            } else {
              tokenDefMap_[tkn.substr(0, tkn.find_first_of(':'))] = dwarfOffsets[cds];
            }

            DEBUG(
              dbgs() << " to "
                     << (currentFunc ? currentFunc->FuncName_ : "main" )
                     << " and "
                     << (currentFuncBody ? currentFuncBody->FuncName_ : "main")
                     << "\n";
            );
          } else if (!tkn.compare(0, 10, ";.func_end")) {
           // when the a label is processed, the line number of
            // the current function needs to be stored in the map with
            // the label.
            if (currentFunc) {
              currentFunc->labelMap_[tkn.substr(0, tkn.find_first_of(':'))]
                = currentFuncBody->getLineNum();
              tokenDefMap_[tkn.substr(0, tkn.find_first_of(':'))] = currentFuncBody->getLineNum();
            } else {
              tokenDefMap_[tkn.substr(0, tkn.find_first_of(':'))] = dwarfOffsets[cds];
            }
          } else if (!tkn.compare(0, 12, ";.text_begin")
                  || !tkn.compare(0, 10, ";.text_end")
                  || !tkn.compare(0, 12, ";.data_begin")
                  || !tkn.compare(0, 10, ";.data_end")) {
            // when the a label is processed, the line number of
            // the current function needs to be stored in the map with
            // the label.
            if (currentFunc) {
              currentFunc->labelMap_[tkn.substr(0, tkn.find_first_of(':'))]
                = currentFuncBody->getLineNum();
            }
          } else if (!tkn.compare(0, strlen(startEndTokens[cds][0]),
                startEndTokens[cds][0])) {
            size_t tokenSize = tkn.find_first_of(':');
            if (tokenSize == std::string::npos) {
              tokenSize = strlen(startEndTokens[cds][0]);
            }
            tokenDefMap_[tkn.substr(0, tokenSize)] = dwarfOffsets[cds];
            parser.markStart();
          } else if (!tkn.compare(0, strlen(startEndTokens[cds][1]),
                startEndTokens[cds][1])) {
            size_t tokenSize = tkn.find_first_of(':');
            if (tokenSize == std::string::npos) {
              tokenSize = strlen(startEndTokens[cds][0]);
            }
            tokenDefMap_[tkn.substr(0, tokenSize)] = dwarfOffsets[cds];
            parser.markStop();
            dwarfStrings[cds] = parser.getSectionStr();
          } else {
            tokenDefMap_[tkn.substr(0, tkn.length()-1)] = dwarfOffsets[cds];
          }
        }
        break;
      case DWARF_GLOBAL:
        {
          assert(parser.isGlobalLabel(parser.getValueType())
              && "The value of the global must be a global label!");
          const std::string& Tkn = parser.getToken();
          assert(Tkn == ".global"
              && "The global token must be of the form \'.global\'!");
          const std::string& Val = parser.getValue();
          size_t endPos = Val.find_first_of(':');
          if (endPos != std::string::npos) {
            // There is a size associated with this global variable
            // label, so we need to strip the size out and add the
            // size to the value map of the current function.
            std::string name = Val.substr(0, endPos);
            std::stringstream ss;
            ss << Val.substr(endPos + 1, Val.size());
            unsigned value;
            ss >> value;
            if (currentFunc) {
              currentFunc->labelMap_[name] = value;
            }
          } else {
            // The token is a function call or a label with no size.
            // If the token is a function call, then the IL line number
            // is used as the value for the label.
            std::string funcName = Val.substr(0, endPos);
            for (std::vector<ILFunc*>::iterator
                ilBegin = mILData.begin(), ilEnd = mILData.end();
                ilBegin != ilEnd; ++ilBegin) {
              if ((*ilBegin)->FuncName_ == StripWrapper(funcName) && currentFunc) {
                currentFunc->labelMap_[funcName] = (*ilBegin)->getBaseLineNum();
              }
            }

          }
        }
        break;
      case DWARF_BYTE:
      case DWARF_SHORT:
        {
          assert(parser.isNumber(parser.getValueType())
              && "The value of a byte/short must be a number");
          unsigned short val;
          std::stringstream ss;
          ss << parser.getValue();
          ss >> val;
          if (parser.getTokenType() == DWARF_BYTE) {
            char byte = val;
            emit(dwarfSections[cds], dwarfOffsets[cds],
                dwarfSizes[cds], byte);
          } else {
            emit(dwarfSections[cds], dwarfOffsets[cds],
                dwarfSizes[cds], val);
          }
        }
        break;
      case DWARF_OFFSET:
        tokenUseMap_[parser.getValue()].insert(
            std::make_pair(cds, dwarfOffsets[cds]));
        emit(dwarfSections[cds], dwarfOffsets[cds],
            dwarfSizes[cds], (unsigned int)0);
        break;
      case DWARF_LONG:
        if (parser.isNumber(parser.getValueType())) {
          std::stringstream ss;
          unsigned int Long;
          ss << parser.getValue();
          ss >> Long;
          emit(dwarfSections[cds], dwarfOffsets[cds],
              dwarfSizes[cds], Long);
        } else if (parser.isCalc(parser.getValueType())) {
          calcMap_[parser.getValue()].insert(
              std::make_pair(cds, dwarfOffsets[cds]));
          emit(dwarfSections[cds], dwarfOffsets[cds],
              dwarfSizes[cds], (unsigned int)0);
        } else if (parser.isLabel(parser.getValueType())) {
          tokenUseMap_[parser.getValue()].insert(
              std::make_pair(cds, dwarfOffsets[cds]));
          emit(dwarfSections[cds], dwarfOffsets[cds],
              dwarfSizes[cds], (unsigned int)0);
        } else {
          llvm_unreachable("Found an case we don't handle!");
        }
        break;
       case DWARF_QUAD:
        if (parser.isNumber(parser.getValueType())) {
          std::stringstream ss;
          unsigned long long Long;
          ss << parser.getValue();
          ss >> Long;
          emit(dwarfSections[cds], dwarfOffsets[cds],
              dwarfSizes[cds], Long);
        } else if (parser.isCalc(parser.getValueType())) {
          calcMap_[parser.getValue()].insert(
              std::make_pair(cds, dwarfOffsets[cds]));
          emit(dwarfSections[cds], dwarfOffsets[cds],
              dwarfSizes[cds], (unsigned long long)0);
        } else if (parser.isLabel(parser.getValueType())) {
          tokenUseMap_[parser.getValue()].insert(
              std::make_pair(cds, dwarfOffsets[cds]));
          emit(dwarfSections[cds], dwarfOffsets[cds],
              dwarfSizes[cds], (unsigned long long)0);
        } else {
          llvm_unreachable("Found an case we don't handle!");
        }
        break;
      case DWARF_SLEB128:
        {
          assert(parser.isNumber(parser.getValueType())
                 && "The value type must be a number");
          long long Value;
          std::stringstream ss;
          ss << parser.getValue();
          ss >> Value;
          int Sign = Value >> (8 * sizeof(Value)-1);
          bool IsMore;
          do {
            char Byte = (char)(Value & 0x7F);
            Value >>= 7;
            IsMore = Value != Sign
              || ((Byte ^ Sign) & 0x40) != 0;
            if (IsMore) Byte |= 0x80;
            emit(dwarfSections[cds], dwarfOffsets[cds], dwarfSizes[cds], Byte);
          } while (IsMore);
        }
        break;
      case DWARF_ULEB128:
        {
          assert(parser.isNumber(parser.getValueType())
                 && "The value type must be a number");
          unsigned long long Value;
          std::stringstream ss;
          ss << parser.getValue();
          ss >> Value;
          do {
            char Byte = (char)(Value & 0x7F);
            Value >>= 7;
            if (Value) Byte |= 0x80;
            emit(dwarfSections[cds], dwarfOffsets[cds], dwarfSizes[cds], Byte);
          } while (Value);
        }
        break;
      case DWARF_ZERO:
      case DWARF_ALIGN:
      case DWARF_VAR_TYPE:
      case DWARF_SIZE:
      case DWARF_WEAK_REF:
      case DWARF_WEAK_DEF:
      case DWARF_TEXT:
      case DWARF_DATA:
      case DWARF_LOC:
      case DWARF_FILE:
      case DWARF_DEBUG_RANGE:
      case DWARF_COMMENT:
        // Nothing is done here yet
        break;
      case DWARF_CALCULATION:
      case DWARF_STRING:
      case DWARF_NUMBER:
        llvm_unreachable("These should never be token types, only value types");
      default:
      case DWARF_UNKNOWN:
        errs() << "Token: " << parser.getToken()
          << " Value: " << parser.getValue() << "\n";
        llvm_unreachable("Unknown dwarf type found!");
        break;

    };
  }

  DEBUG(
    dbgs() << "\nDumping normal token defs that were found\n";
    for (std::map<std::string, unsigned>::iterator
           tmb = tokenDefMap_.begin(), tme = tokenDefMap_.end(); tmb != tme; ++tmb) {
      dbgs() << "Token: " << tmb->first << " Offsets: " << tmb->second;
      dbgs() << "\n";
    }
    dbgs() << "\n";
    dbgs() << "\nDumping normal token uses that were found\n";
    for (std::map<std::string,
                  std::set<std::pair<DwarfSection, unsigned> > >::iterator
           tmb = tokenUseMap_.begin(), tme = tokenUseMap_.end(); tmb != tme; ++tmb) {
      dbgs() << "Token: " << tmb->first << " Offsets: ";
      for (std::set<std::pair<DwarfSection, unsigned> >::iterator
             sei = tmb->second.begin(), see = tmb->second.end();
           sei != see; ++sei) {
        dbgs() << "(" << sei->first << "," << sei->second << ") ";
      }
      dbgs() << "\n";
    }
    dbgs() << "\n";
    dbgs() << "Dumping calculation tokens that were found\n";
    for (std::map<std::string,
                  std::set<std::pair<DwarfSection, unsigned> > >::iterator
           tmb = calcMap_.begin(), tme = calcMap_.end(); tmb != tme; ++tmb) {
      dbgs() << "Token: " << tmb->first << " Offsets: ";
      for (std::set<std::pair<DwarfSection, unsigned> >::iterator
             sei = tmb->second.begin(), see = tmb->second.end();
           sei != see; ++sei) {
        dbgs() << "(" << sei->first << "," << sei->second << ") ";
      }
      dbgs() << "\n";
    }
    dbgs() << "\n";
  );

  // First we process all of the normal tokens and patch the location that they
  // were found with the line number that they exist in the IL or the correct
  // offset in the debug information.
  for (std::map<std::string,
      std::set<std::pair<DwarfSection, unsigned> > >::iterator
      tmb = tokenUseMap_.begin(), tme = tokenUseMap_.end(); tmb != tme; ++tmb) {
    std::string curTkn = (*tmb).first;
    std::set<std::pair<DwarfSection, unsigned> >& tknSet = (*tmb).second;
    for (std::set<std::pair<DwarfSection, unsigned> >::iterator
        dsb = tknSet.begin(), dse = tknSet.end(); dsb != dse; ++dsb) {
      char *patchSection = dwarfSections[dsb->first];
      size_t offset = dsb->second;
      size_t size = dwarfSizes[dsb->first];
      unsigned val = tokenToValue(curTkn);
      if (val == ~0U) {
        errs() << "Token: " << curTkn << "\n";
        llvm_unreachable("Found a case where we could not find the definition or the"
                         " line number for the token offset");
      }
      DEBUG(
        dbgs() << "Patching variable " << curTkn << " at ";
        dbgs() << "location " << offset << " section " << dsb->first;
        dbgs() << " with value " << val << "\n";
      );
      emit(patchSection, offset, size, val);
      if (size != dwarfSizes[dsb->first]) {
        dwarfSizes[dsb->first] = size;
        dwarfSections[dsb->first] = patchSection;
      }
    }
  }
  // The next step is to process the calculation tokens, which requires both
  // patching the string and generating a simple add/sub calculator.
  for (std::map<std::string,
      std::set<std::pair<DwarfSection, unsigned> > >::iterator
      tmb = calcMap_.begin(), tme = calcMap_.end(); tmb != tme; ++tmb) {
    std::string curTkn = (*tmb).first;
    std::string calcTkn = tokenToFormula(curTkn);
    unsigned val = evalFormula(calcTkn);
    if (val == ~0U) {
      DEBUG(
        dbgs() << "Path: " << curTkn << " -> "
               << calcTkn << " -> " << val << "\n";
      );

      val = 0;
      llvm_unreachable("Evaluation of the formula failed!");
    }
    std::set<std::pair<DwarfSection, unsigned> >& tknSet = (*tmb).second;
    for (std::set<std::pair<DwarfSection, unsigned> >::iterator
        dsb = tknSet.begin(), dse = tknSet.end(); dsb != dse; ++dsb) {
      char *patchSection = dwarfSections[dsb->first];
      size_t offset = dsb->second;
      size_t size = dwarfSizes[dsb->first];
      DEBUG(
        dbgs() << "Patching variable " << curTkn << " at ";
        dbgs() << "location " << offset << " section " << dsb->first;
        dbgs() << " with value " << val << "\n";
      );
      emit(patchSection, offset, size, val);
      if (size != dwarfSizes[dsb->first]) {
        dwarfSizes[dsb->first] = size;
        dwarfSections[dsb->first] = patchSection;
      }
     }
  }
}

std::string
AMDILDwarf::tokenToFormula(const std::string &curTkn)
{
  std::string tmpTkn = curTkn;
  std::string::iterator ctBegin = tmpTkn.begin();
  std::string::iterator ctEnd = tmpTkn.end();
  while(ctBegin != ctEnd) {
    if (*ctBegin == ';') {
      std::string::iterator endTkn = ctBegin;
      while(endTkn != ctEnd) {
        if (*endTkn == '-'
         || *endTkn == '+'
         || *endTkn == ')'
         || *endTkn == '(') {
          std::string substr = std::string(ctBegin, endTkn);
          unsigned val = tokenToValue(substr);
          if (val == ~0U) {
            DEBUG(dbgs() << "Token: " << substr << "\n");
            val = 0;
            llvm_unreachable("Found a case where we could not find the definition or the"
                             " line number for the token offset");
          }
          std::stringstream ss;
          ss << val;
          tmpTkn.replace(ctBegin, endTkn, ss.str());
          ctEnd = tmpTkn.end();
          ctBegin = tmpTkn.begin();
          DEBUG(
            dbgs() << "Found token: " << substr
                   << " and replacing it with value: " << ss.str() << "."
                   << " New token is: " << tmpTkn << "\n";
          );
          break;
        }
        ++endTkn;
      }
      // We need to handle the last token if it exists.
      if (endTkn == ctEnd) {
        std::string substr = std::string(ctBegin, endTkn);
        unsigned val = tokenToValue(substr);
        if (val == ~0U) {
          DEBUG(dbgs() << "Token: " << substr << "\n");
          val = 0;
          llvm_unreachable("Found a case where we could not find the definition or the"
                           " line number for the token offset");
        }
        std::stringstream ss;
        ss << val;
        tmpTkn.replace(ctBegin, endTkn, ss.str());
        ctEnd = tmpTkn.end();
        ctBegin = tmpTkn.begin();
        DEBUG(
          dbgs() << "Found token: " << substr
                 << " and replacing it with value: " << ss.str() << "."
                 << " New token is: " << tmpTkn << "\n";
        );
      }
    }
    ++ctBegin;
  }

  DEBUG(dbgs() << "Returning formula: " << tmpTkn << "\n");
  return tmpTkn;
}

unsigned
AMDILDwarf::evalFormula(const std::string &curTkn)
{
  unsigned lhsVal = ~0U;
  unsigned rhsVal = ~0U;
  char opCode = '0';
  std::string::const_iterator ctBegin = curTkn.begin();
  std::string::const_iterator ctEnd = curTkn.end();
  // This approach does not handle nested '(..)'
  // constructs as I haven't seen them in any
  // dwarf output yet.
  // TODO: Support (symbol-(symbol+symbol))+symbol constructs
  while (ctBegin != ctEnd) {
    if (*ctBegin == '(') {
      std::string::const_iterator endTkn = ctBegin++;
      while (endTkn != ctEnd) {
        if (*endTkn == ')') {
          std::string subTkn = std::string(ctBegin, endTkn);
          DEBUG(dbgs() << "Found a sub-formula: " << subTkn << "\n");
          unsigned val = evalFormula(subTkn);
          DEBUG(dbgs() << " Val: " << val <<"\n");
          if (lhsVal == ~0U) {
            lhsVal = val;
          } else {
            DEBUG(dbgs() << "Computing " << lhsVal
                         << " " << opCode << " " << val);
            lhsVal += ((opCode == '-') ? -val : val);
            DEBUG(dbgs() << " = " << lhsVal << "\n");
          }
          ctBegin = endTkn;
          break;
        }
        ++endTkn;
      }
    } else if (*ctBegin >= '0' && *ctBegin <= '9') {
      std::string::const_iterator endTkn = ctBegin;
      while (endTkn != ctEnd && *endTkn >= '0' && *endTkn <= '9') {
        ++endTkn;
      }
      std::string val = std::string(ctBegin, ctEnd);
      std::stringstream ss;
      ss << val;
      if (lhsVal == ~0U) {
        ss >> lhsVal;
      } else {
        ss >> rhsVal;
        assert((opCode == '-' || opCode == '+')
            && "Only \'-\' and \'+\' opcodes are supported!");
        DEBUG(dbgs() << "Computing " << lhsVal << " " << opCode << " " << val);
        lhsVal += ((opCode == '-') ? -rhsVal : rhsVal);
        DEBUG(dbgs() << " = " << lhsVal << "\n");
      }
      ctBegin = endTkn;
    } else if (*ctBegin == '-' || *ctBegin == '+') {
      opCode = *ctBegin;
      ++ctBegin;
    } else {
      ++ctBegin;
    }
  }
  return lhsVal;
}

unsigned
AMDILDwarf::tokenToValue(const std::string &curTkn)
{
  if (tokenDefMap_.find(curTkn) != tokenDefMap_.end()) {
    return tokenDefMap_[curTkn];
  } else {
    unsigned tmpID;
    for (std::vector<ILFunc*>::iterator funcIB = mILData.begin(),
        funcIE = mILData.end(); funcIB != funcIE; ++funcIB) {
        if ((tmpID = (*funcIB)->getTmpLineNumber(curTkn)) != ~0U) {
          return tmpID;
        }
    }
  }
  return ~0U;
}

AMDILDwarf::~AMDILDwarf()
{
  for (size_t x = 0; x < AMDILDwarf::DEBUG_LAST; ++x) {
    if (dwarfSections[x]) {
      delete [] dwarfSections[x];
    }
  }
}

  std::string
AMDILDwarf::getDwarfString(DwarfSection idxNum)
{
  assert(idxNum < AMDILDwarf::DEBUG_LAST &&
      "Must only request a section that is valid!");
  std::string val = dwarfStrings[idxNum];
  return val;
}

  const char*
AMDILDwarf::getDwarfBitstream(DwarfSection idxNum, size_t& size)
{
  assert(idxNum < AMDILDwarf::DEBUG_LAST &&
      "Must only request a section that is valid!");
  size = dwarfOffsets[idxNum];
  return dwarfSections[idxNum];
}
  void
AMDILDwarf::dump()
{
  errs() << *this;
}
  raw_ostream&
llvm::operator<<(raw_ostream& os, AMDILDwarf& dd)
{
  for (size_t x = 0; x < AMDILDwarf::DEBUG_LAST; ++x) {
    os << "\t.section\t";
    os << dwarfNames[x];
    os << "\n";
    os << ".text\n";
    os << dd.dwarfStrings[x];
    os << ".binary\n";
    os.write(dd.dwarfSections[x], dd.dwarfOffsets[x]);
    os << "\n";
  }
  return os;
}
//===----------------------------- CompUnit ------------------------------===//
CompUnit::CompUnit(std::istream& in)
  : mError(false), mMsg("")
{
  init(in);
}

CompUnit::CompUnit(const std::string& in)
{
  std::stringstream ss;
  ss << in;
  init(ss);
}

  void
CompUnit::init(std::istream& in)
{
  dispLineNum_ = false;
  compBlocks_ = new std::list<Component*>;
  main_ = NULL;
  debugData_ = NULL;
  dummyMacro_ = NULL;
  AMDILMDScanner scanner(&in);
  AMDILMDParser parser(scanner, this, false, "");

#if YYDEBUG
  DEBUG(parser.set_debug_level(true));
#endif
  parser.parse();
  mError = scanner.hasError();
  mMsg = scanner.getErrorStr();
  process();
}

CompUnit::~CompUnit()
{
  kernels_.clear();
  std::list<Component*>::iterator compBIter;
  for (compBIter = compBlocks_->begin();
      compBIter != compBlocks_->end();
      ++compBIter) {
    Component* comp = *compBIter;
      delete comp;
    }
  if (debugData_) {
    delete debugData_;
  }
  if (main_) {
    delete main_;
  }
  if (compBlocks_) {
    delete compBlocks_;
  }
}
  void
CompUnit::addComponents(std::list<Component*>* newComps,
    ComponentStates State)
{
  switch(State) {
    default:
      llvm_unreachable("Invalid state passed in!");
      break;
    case IL_STATE:
      for (std::list<Component*>::iterator lBegin = newComps->begin(),
          lEnd = newComps->end(); lBegin != lEnd; ++lBegin) {
        Component* C = *lBegin;
        if (ILFunc *IL = dyn_cast<ILFunc>(C)) {
          ilfuncs_.push_back(IL);
        } else if (DummyMacro *DM = dyn_cast<DummyMacro>(C)) {
          assert(dummyMacro_ == NULL && "dummy macro already defined");
          dummyMacro_ = DM;
        } else {
          Macro *M = dyn_cast<Macro>(C);
          assert(M && "unexpected type");
          macros_.push_back(M);
        }
      }
      break;
    case DBG_STATE:
      for (std::list<Component*>::iterator lBegin = newComps->begin(),
          lEnd = newComps->end(); lBegin != lEnd; ++lBegin) {
        debug_.push_back(cast<DBSection>(*lBegin));
      }
      break;
    case MD_STATE:
      for (std::list<Component*>::iterator lBegin = newComps->begin(),
          lEnd = newComps->end(); lBegin != lEnd; ++lBegin) {
        metadata_.push_back(cast<MDBlock>(*lBegin));
      }
      break;
    case DATA_STATE:
      for (std::list<Component*>::iterator lBegin = newComps->begin(),
          lEnd = newComps->end(); lBegin != lEnd; ++lBegin) {
        data_.push_back(cast<DataSection>(*lBegin));
      }
      break;
  }

  for (std::list<Component*>::iterator cib = newComps->begin(),
      cie = newComps->end(); cib != cie; ++cib) {
    compBlocks_->push_back((*cib));
  }
}
  void
CompUnit::dump()
{
  if (main_) {
    main_->dump();
  }
  std::list<Component*>::iterator cib, cie;
  for (cib = compBlocks_->begin(), cie = compBlocks_->end();
      cib != cie; ++cib) {
    (*cib)->dump();
  }
}

void CompUnit::setMain(MainFunc* mf)
{
  main_ = mf;
}

  void
CompUnit::process()
{
  unsigned lineNum = 0;
  if (main_) {
    if (dummyMacro_) {
      main_->setLineNum(2); // account for 2 line dummy macro definition
    } else {
      main_->setLineNum(0);
    }
    main_->process();
    lineNum = main_->getLineNum();
  }
  // Let's loop over all of the components so that
  // we can correctly calculate the line numbers and
  // process each node accordingly.
  std::list<Component*>::iterator cib, cie;
  for (cib = compBlocks_->begin(), cie = compBlocks_->end();
      cib != cie; ++cib) {
    if (ILFunc *IL = dyn_cast<ILFunc>(*cib)) {
      IL->mainFunc_ = main_;
      IL->setLineNum(lineNum);
      IL->process();
      lineNum = IL->getLineNum();
      if (IL->isKernel()) {
        kernels_.push_back(IL);
      }
      funcsMap_[IL->ID_] = IL;
    } else if (MDBlock *MD = dyn_cast<MDBlock>(*cib)) {
      MD->process();
    } else if (DBSection *DBG = dyn_cast<DBSection>(*cib)) {
      DBG->process();
    } else if (Macro *M = dyn_cast<Macro>(*cib)) {
      M->process();
    } else if (DataSection *Data = dyn_cast<DataSection>(*cib)) {
      Data->process();
    }
  }
  for (std::map<unsigned, ILFunc*>::iterator fib = funcsMap_.begin(),
      fie = funcsMap_.end(); fib != fie; ++fib) {
    std::map<std::string, MDType*>::iterator MDiter;
    ILFunc* func = fib->second;
    if (!func) {
      continue;
    }
    func->metadata_ = NULL;
    // First let's find all of the metadata blocks for each
    // IL func
    for (size_t mdb = 0, mde = metadata_.size(); mdb < mde; ++mdb) {
      MDBlock* metadata = metadata_[mdb];
      if (func->metadata_) {
        break;
      }
      MDiter = metadata->MDValMap_.find(";uniqueid");
      if (metadata->MDValMap_.end() == MDiter) {
        assert(0 && "Could not find uniqueid metadata for the func");
      }
      if (MDiter->second->getMDInt()->Int_ == func->ID_) {
        func->metadata_ = metadata;
        break;
      }
    }
    if (func->metadata_) {
      MDiter = func->metadata_->MDValMap_.find(";function");
      if (MDiter != func->metadata_->MDValMap_.end()) {
        // Now that we have the correct Metadata block, let's use that
        // information to build the rest of the call tree.
        // The first value that was parsed with the ';function' metadata
        // is ignored as it is the number of functions that exists
        // in the list.
        if (MDiter->second->getMDIntList()->IntList_) {
          for (size_t cb = 1, ce = MDiter->second->
              getMDIntList()->IntList_->size(); cb < ce; ++cb) {
            unsigned offset = (*MDiter->second->getMDIntList()->IntList_)[cb];
            ILFunc* depFunc = funcsMap_[offset];
            func->dependentBlocks_.insert(depFunc);
          }
        }
      }
      MDiter = func->metadata_->MDValMap_.find(";intrinsic");
      if (MDiter != func->metadata_->MDValMap_.end()) {
        // Now that we found an intrinsic entry, let's use that
        // to find out which macros are required.
        if (MDiter->second->getMDIntList()->IntList_) {
          for (size_t cb = 1, ce = MDiter->second->
              getMDIntList()->IntList_->size(); cb < ce; ++cb) {
            unsigned offset = (*MDiter->second->getMDIntList()->IntList_)[cb];
            func->macros_.insert(offset);
          }
        }
      }
      if (func->metadata_->MDValMap_.find("compilerwrite")
          != func->metadata_->MDValMap_.end()) {
        func->data_ = &data_;
      }
    }
  }
}
  std::set<unsigned>
CompUnit::getAllDepFuncs(ILFunc* func)
{
  std::set<unsigned> funcSet;
  std::queue<ILFunc*, std::list<ILFunc*> > ILqueue;
  ILqueue.push(func);
  while (!ILqueue.empty()) {
    func = ILqueue.front();
    funcSet.insert(func->ID_);
    for (std::set<ILFunc*>::iterator sib = func->dependentBlocks_.begin(),
        sie = func->dependentBlocks_.end(); sib != sie; ++sib) {
      ILFunc* ptr = (*sib);
      if (!ptr) {
        continue;
      }
      if (funcSet.count((*sib)->ID_)) {
        continue;
      }
      ILqueue.push(*sib);
    }
    ILqueue.pop();
  }
  return funcSet;
}

  size_t
CompUnit::getNumKernels()
{
  return kernels_.size();
}

  bool
CompUnit::empty()
{
  return kernels_.empty();
}
  void
CompUnit::setLineNumberDisplay(bool val)
{
  dispLineNum_ = val;
}

  raw_ostream&
llvm::operator<<(raw_ostream& os, CompUnit& CU)
{
  if (CU.dummyMacro_) {
    os << *CU.dummyMacro_;
  }
  unsigned lineNum = 0;
  if (CU.main_) {
    CU.main_->useLineNum(CU.dispLineNum_);
    CU.main_->setFuncID(-1);
    os << *CU.main_;
    lineNum = CU.main_->getLineNum() + 1;
  }
  std::list<Component*>::iterator cib, cie;
  for (cib = CU.compBlocks_->begin(), cie = CU.compBlocks_->end();
      cib != cie; ++cib) {
    if (ILFunc *IL = dyn_cast<ILFunc>(*cib)) {
      IL->useLineNum(CU.dispLineNum_);
      os << *(*cib);
      lineNum = IL->getLineNum();
    } else if (isa<DBSection>(*cib)) {
    } else {
      os << *(*cib);
    }
  }
  os << "end ";
  if (CU.dispLineNum_) {
    os << ";" << lineNum;
  }
  os << "\n";
  return os;
}
  std::string
CompUnit::getILStr()
{
  std::string str;
  raw_string_ostream ss(str);

  if (dummyMacro_) {
    ss << *dummyMacro_;
  }
  if (main_) {
    main_->setFuncID(-1);
    ss << *main_;
  }
  std::list<Component*>::iterator cib, cie;
  for (cib = compBlocks_->begin(), cie = compBlocks_->end();
      cib != cie; ++cib) {
    if (ILFunc *IL = dyn_cast<ILFunc>(*cib)) {
      ss << *IL;
    } else if (isa<Macro>(*cib) && !isa<DummyMacro>(*cib)) {
      ss << *cast<Macro>(*cib);
    }
  }
  ss << "end\n";
  return ss.str();
}
  std::string
CompUnit::getKernelStr(unsigned id)
{
  assert(id < getNumKernels() && "Cannot pass in an argument where"
      " the id is larger than the size of the array!");
  std::string str("");
  raw_string_ostream ss(str);

  //if (dummyMacro_ && !macros_.empty()) {
  if (dummyMacro_) {
    ss << *dummyMacro_;
  }
  ILFunc* func = kernels_[id];
  if (main_) {
    main_->setFuncID(func->ID_);
    main_->useLineNum(dispLineNum_);
    ss << *main_;
  }
  std::set<unsigned> funcSet = getAllDepFuncs(func);
  for (std::set<unsigned>::iterator fsb = funcSet.begin(),
      fse = funcSet.end(); fsb != fse; ++fsb) {
    func = funcsMap_[*fsb];
    func->useLineNum(dispLineNum_);
    ss << *func;
  }
  // Attach all macros for any kernel in the source to this kernel
  for (std::vector<Macro*>::iterator mvb = macros_.begin(),
      mve = macros_.end(); mvb != mve; ++mvb) {
    ss << **mvb;
  }
  ss << "end";
  if (dispLineNum_) {
    ss << " ; " << func->getLineNum();
  }
  ss << "\n";
  if (getenv("AMD_DUMP_IL")) {
    std::cout << "[getKernelStr]" << id << ' ' << func->FuncName_ << '\n' <<
        ss.str() << '\n';
  }
  return ss.str();
}

  std::string
CompUnit::getKernelMD(unsigned id)
{
  assert(id < getNumKernels() && "Cannot pass in an argument where"
         " the id is larger than the size of the array!");
  std::string str;
  raw_string_ostream ss(str);

  ILFunc* func = kernels_[id];
  std::set<unsigned> funcSet = getAllDepFuncs(func);
  for (std::set<unsigned>::iterator fsb = funcSet.begin(),
      fse = funcSet.end(); fsb != fse; ++fsb) {
    func = funcsMap_[*fsb];
    if (func->metadata_) {
      ss << *(func->metadata_);
    }
  }
  return ss.str();
}

  std::string
CompUnit::getMD()
{
  assert(getNumKernels() == 0 && "Kernel parse, use getKernelMD() instead");
  if (metadata_.empty()) return std::string();

  assert(metadata_.size() == 1 && "multiple metadata blocks present");

  std::string str;
  raw_string_ostream ss(str);
  ss << *metadata_[0];
  return ss.str();
}

  std::string
CompUnit::getKernelName(unsigned id)
{
  ILFunc* func = kernels_[id];
  return func->FuncName_;
}

  AMDILDwarf*
CompUnit::getDebugData()
{
  if (!debugData_) {
    debugData_ = new AMDILDwarf(debug_, ilfuncs_);
  }
  return debugData_;
}

  AMDILDataSection*
CompUnit::getDataSections(unsigned *numSections)
{
  if (!numSections) {
    return NULL;
  }
  (*numSections) = data_.size();
  if (!*numSections) {
    return NULL;
  }
  AMDILDataSection* AMDILDS = new AMDILDataSection[*numSections];
  unsigned num = 0;
  unsigned curByte = 0;
  for (std::vector<DataSection*>::iterator dsb = data_.begin(),
      dse = data_.end(); dsb != dse; ++dsb, ++num) {
    AMDILDS[num].size = (*dsb)->Size_;
    if ((*dsb)->FuncName_ == "Software") {
      AMDILDS[num].cbNum = 0;
    } else {
      StringRef Name((*dsb)->FuncName_);
      Name.getAsInteger(10, AMDILDS[num].cbNum);
    }
    AMDILDS[num].data = new unsigned char[(*dsb)->Size_ + 1];
    memset(AMDILDS[num].data, 0, (*dsb)->Size_);
    for (std::vector<Data*>::iterator db = (*dsb)->Data_->begin(),
        de = (*dsb)->Data_->end(); db != de; ++db) {
      unsigned offset = (*db)->Offset_;
      curByte = offset;
      for (std::list<std::string*>::iterator lBegin = (*db)->Data_->begin(),
          lEnd = (*db)->Data_->end(); lBegin != lEnd; ++lBegin, ++curByte) {
        StringRef S(**lBegin);
        unsigned byte;
        S.getAsInteger(10, byte);
        AMDILDS[num].data[curByte] = static_cast<unsigned char>(byte & 0xFF);
      }
    }
  }
  return AMDILDS;
}

  bool
CompUnit::releaseDataSections(AMDILDataSection** data, unsigned numSections)
{
  if (!data) {
    return false;
  }
  if (!numSections) {
    return false;
  }
  AMDILDataSection *ptr = (*data);
  for (unsigned i = 0; i < numSections; ++i) {
    if (ptr[i].data) {
      delete [] ptr->data;
    }
    memset(ptr + i, 0, sizeof (*ptr));
  }
  if (*data) {
    delete [] (*data);
    (*data) = NULL;
  }
  return true;
}

  AMDILMetadata
CompUnit::getKernelMDStruct(unsigned id)
{
  assert(id < getNumKernels() && "Cannot pass in an argument where"
         " the id is larger than the number of kernels!");
  AMDILMetadata currentMD;
  ILFunc* func = kernels_[id];
  std::set<unsigned> funcSet = getAllDepFuncs(func);
  if (func->metadata_ && !funcSet.count(func->ID_)) {
    func->metadata_->updateMetadata(&currentMD);
  }
  currentMD.kernelName = func->FuncName_;
  for (std::set<unsigned>::iterator sb = funcSet.begin(), se = funcSet.end();
      sb != se; ++sb) {
    func = funcsMap_[*sb];
    if (func->metadata_) {
      func->metadata_->updateMetadata(&currentMD);
    }
  }
  return currentMD;
}

  AMDILMetadata
CompUnit::getMDStruct()
{
  assert(getNumKernels() == 0 && "Kernel parse, use getKernelMD() instead");
  if (metadata_.empty()) return AMDILMetadata();

  assert(metadata_.size() == 1 && "multiple metadata blocks present");

  AMDILMetadata currentMD;
  metadata_[0]->updateMetadata(&currentMD);
  return currentMD;
}
