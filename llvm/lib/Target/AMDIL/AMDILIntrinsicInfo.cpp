//===- AMDILIntrinsicInfo.cpp - AMDIL Intrinsic Information ------*- C++ -*-===//
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
// This file contains the AMDIL Implementation of the IntrinsicInfo class.
//
//===-----------------------------------------------------------------------===//

#include "AMDIL.h"
#include "AMDILIntrinsicInfo.h"
#include "AMDILTargetMachine.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Intrinsics.h"
#include "llvm/Module.h"

using namespace llvm;

#define GET_LLVM_INTRINSIC_FOR_GCC_BUILTIN
#include "AMDILGenIntrinsics.inc"
#undef GET_LLVM_INTRINSIC_FOR_GCC_BUILTIN


AMDILIntrinsicInfo::AMDILIntrinsicInfo(AMDILTargetMachine *TM)
  : TargetIntrinsicInfo(), mTM(TM) {
}

std::string AMDILIntrinsicInfo::getName(unsigned int IntrID,
                                        Type **Tys,
                                        unsigned NumTys) const {
  static const char *const Names[] = {
#define GET_INTRINSIC_NAME_TABLE
#include "AMDILGenIntrinsics.inc"
#undef GET_INTRINSIC_NAME_TABLE
  };

  //assert(!isOverloaded(IntrID)
  //&& "AMDIL Intrinsics are not overloaded");
  if (IntrID < Intrinsic::num_intrinsics) {
    return "";
  }
  assert(IntrID < AMDILIntrinsic::num_AMDIL_intrinsics
      && "Invalid intrinsic ID");

  return std::string(Names[IntrID - Intrinsic::num_intrinsics]);
}

static unsigned needTruncateSuffixLength(StringRef Name) {
  assert(!Name.startswith("__atom")
         && "Atomic intrinsics should not be truncated");

  StringRef::size_type Underscore = Name.find_last_of('_');
  if (Underscore == StringRef::npos) {
    return 0;
  }

  StringRef Suffix = Name.drop_front(Underscore + 1);
  if (   Suffix.find("i32") != StringRef::npos
      || Suffix.find("u32") != StringRef::npos
      || Suffix.find("i64") != StringRef::npos
      || Suffix.find("u64") != StringRef::npos
      || Suffix.find("f32") != StringRef::npos
      || Suffix.find("f64") != StringRef::npos
      || Suffix.find("i8") != StringRef::npos
      || Suffix.find("u8") != StringRef::npos) {
    return Suffix.size() + 1;
  }

  return 0;
}

// We don't want to support both the OpenCL 1.0 atomics
// and the 1.1 atomics with different names, so we translate
// the 1.0 atomics to the 1.1 naming here if needed. (__atom_* -> __atomic_*)
static unsigned getAtomicIntrinsic(StringRef Name) {
  StringRef Atom("__atom");

  if (Name.drop_front(Atom.size()).startswith("ic")) {
    return getIntrinsicForGCCBuiltin("AMDIL", Name.data());
  }

  SmallString<32> Buffer("__atomic");
  Buffer += Name.drop_front(Atom.size());

  return getIntrinsicForGCCBuiltin("AMDIL", Buffer.c_str());
}

static StringRef stripVectorSizeFromName(StringRef Name,
                                         const char* TypeName) {
  SmallString<8> SuffixV2("v2");
  SmallString<8> SuffixV4("v4");

  SuffixV2 += TypeName;
  SuffixV4 += TypeName;

  if (Name.endswith(SuffixV2.c_str()) ||
      Name.endswith(SuffixV4.c_str())) {
    SmallString<32> Buffer(Name.drop_back(5));
    Buffer += TypeName;
    return StringRef(Buffer);
  } else {
    return StringRef();
  }
}

unsigned AMDILIntrinsicInfo::lookupName(const char *Name,
                                        unsigned Len) const {
#define GET_FUNCTION_RECOGNIZER
#include "AMDILGenIntrinsics.inc"
#undef GET_FUNCTION_RECOGNIZER
  StringRef NameRef(Name, Len);

  if (NameRef.startswith("__atom")) {
    // We don't want to truncate on atomic instructions
    return getAtomicIntrinsic(NameRef);
  }

  unsigned TruncateSize = needTruncateSuffixLength(NameRef);
  if (TruncateSize != 0) {
    SmallString<32> Buffer(NameRef.drop_back(TruncateSize));
    return getIntrinsicForGCCBuiltin("AMDIL", Buffer.c_str());
  }

  // Handle f16 intrinsics a little different becuase they cannot be
  // overloaded with the regular floating point, since they use
  // integer operands. Keep the _f16 suffix in the intrinsics, but
  // remove the vector portion of the name.
  StringRef IntName = stripVectorSizeFromName(NameRef, "f16");
  if(IntName.empty())
    IntName = stripVectorSizeFromName(NameRef, "i16");
  if(IntName.empty())
    IntName = stripVectorSizeFromName(NameRef, "u16");

  if(!IntName.empty())
    return getIntrinsicForGCCBuiltin("AMDIL", IntName.data());

  return getIntrinsicForGCCBuiltin("AMDIL", Name);
}

bool AMDILIntrinsicInfo::isOverloaded(unsigned IntrID) const {
  if (IntrID == 0)
    return false;
  // Overload Table
  unsigned id = IntrID - Intrinsic::num_intrinsics + 1;
#define GET_INTRINSIC_OVERLOAD_TABLE
#include "AMDILGenIntrinsics.inc"
#undef GET_INTRINSIC_OVERLOAD_TABLE
}

/// This defines the "getAttributes(ID id)" method.
#define GET_INTRINSIC_ATTRIBUTES
#include "AMDILGenIntrinsics.inc"
#undef GET_INTRINSIC_ATTRIBUTES

Function *AMDILIntrinsicInfo::getDeclaration(Module *M,
                                             unsigned IntrID,
                                             Type **Tys,
                                             unsigned NumTys) const {
  assert(!isOverloaded(IntrID) && "AMDIL intrinsics are not overloaded");
  LLVMContext &Context = M->getContext();
  AttrListPtr AList = getAttributes(Context, (AMDILIntrinsic::ID) IntrID);
  unsigned id = IntrID;
  Type *ResultTy = NULL;
  std::vector<Type *> ArgTys;
  bool IsVarArg = false;

#define GET_INTRINSIC_GENERATOR
#include "AMDILGenIntrinsics.inc"
#undef GET_INTRINSIC_GENERATOR
  // We need to add the resource ID argument for atomics.
  if (id >= AMDILIntrinsic::AMDIL_atomic_add_gi32
        && id <= AMDILIntrinsic::AMDIL_atomic_xor_ru64_noret) {
    ArgTys.push_back(IntegerType::get(Context, 32));
  }

  return cast<Function>(M->getOrInsertFunction(
                          getName(IntrID),
                          FunctionType::get(ResultTy, ArgTys, IsVarArg),
                          AList));
}
