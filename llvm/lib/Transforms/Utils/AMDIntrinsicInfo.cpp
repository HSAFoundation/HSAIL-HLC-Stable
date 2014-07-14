//
// Copyright (c) 2011 Advanced Micro Devices, Inc. All rights reserved.
//

/*
   This file contains utility functions to work with EDG/Itanium mangled names
*/

#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>
#include <llvm/AMDIntrinsicInfo.h>
#include <cctype>
#include <string>
#include <sstream>

using namespace llvm;

template <size_t N>
static bool eatTerm(StringRef& mangledName, const char (&str)[N]) {
  if (mangledName.startswith(str)) {
    mangledName = mangledName.drop_front(N-1);
    return true;
  }
  return false;
}

static int eatLength(StringRef& s) {
  int len = 0;
  while (!s.empty() && isdigit(s.front())) { 
    len = len*10 + s.front()-'0';
    s = s.drop_front(1);
  }
  return len;
}

static StringRef eatLengthPrefixedName(StringRef& mangledName) {
  size_t const len = eatLength(mangledName);
  if (len==0 || len > mangledName.size()) {
    return StringRef();
  }
  StringRef res = mangledName.substr(0, len);
  mangledName = mangledName.drop_front(len);
  return res;
}

static StringRef getStrippedFuncName(StringRef funcName, AMDIntrinsicInfo::EKind *kind) {
  AMDIntrinsicInfo::EKind k = AMDIntrinsicInfo::NORMAL;
  if (eatTerm(funcName, "native_")) {
    k = AMDIntrinsicInfo::NATIVE;
  } else if (eatTerm(funcName, "half_")) {
    k = AMDIntrinsicInfo::HALF;
  } else if (eatTerm(funcName, "hsail_")) {
    k = AMDIntrinsicInfo::HSAIL;
  } else if (eatTerm(funcName, "amdil_")) {
    k = AMDIntrinsicInfo::AMDIL;
  } else if (eatTerm(funcName, "gcn_")) {
    k = AMDIntrinsicInfo::GCN;
  } 
  if (kind) {
    *kind = k;
  }
  return funcName;
}

static StringRef parseItaniumFunctionName(StringRef mangledName, AMDIntrinsicInfo *iInfo)
{
  StringRef name = eatLengthPrefixedName(mangledName);
  if (name.empty()) {
    return StringRef();
  }
  AMDIntrinsicInfo::EKind kind = AMDIntrinsicInfo::NORMAL;
  StringRef res = getStrippedFuncName(name, &kind);
  if (!iInfo) {
    return res;
  }
  iInfo->FKind = kind;
  switch(mangledName.front()) {
  case 'd': 
    iInfo->VectorSize = 1;
    iInfo->ArgType = Type::DoubleTyID;
  break;
  case 'f': 
    iInfo->VectorSize = 1;
    iInfo->ArgType = Type::FloatTyID;
  break;
  case 'D': {
    mangledName = mangledName.drop_front(1);
    int vec = 0;
    if (!eatTerm(mangledName, "v") || (vec=eatLength(mangledName))==0 || !eatTerm(mangledName, "_"))  {
      return StringRef();
    }
    iInfo->VectorSize = vec;
    switch(mangledName.front()) {
    case 'd': iInfo->ArgType = Type::DoubleTyID; break;
    case 'f': iInfo->ArgType = Type::FloatTyID; break;
    default: return StringRef();
    }
  } break;
  default: return StringRef();
  }
  return res;
}

/*
   Our math library function has the following name mangling
     FMangledName = __<FName>_<s><VSize><EType>[_<s><VSize><EType>]

        FName = API Function Name such as
                sin, cos, native_sin, half_sin, etc, which are given
                in the OpenCL spec.
        VSize = | <n>  (can be empty)
                blank: means scalar
                <n>=2,3,4,8,16 : vector size
        EType = f32 | f64 : element type
        s     = |g|l|p   (can be empty)
                used to indicate that func has a pointer argument.      

        Note that the first "<s><VSize><EType>" is the most important as it
        encodes the type of input arguments,  here we call it "key arg type"
        or simply "key type".  And optional part is for functions with pointer
        argument, such as remquo.

  parseFunctionName() parses the FMangledName, and return true with the following
  detail if if it finds one :
    GFName :  the generic function name with native_/half_ removed from API Function
              name if there is one. So, for half_sin/native_sin, GFName is sin.
    FKind :  normal/native/half
    VectorSize : 1 (scalar), 2,3,4,8,16,  for key arg type.
    EType : f32 (single-precision float) or f64 (double-precision float), for key
            arg type.
*/

static StringRef parseEDGFunctionName(StringRef mangledName, AMDIntrinsicInfo *iInfo)
{
  size_t const typeStart = mangledName.rfind('_');
  if (typeStart==StringRef::npos || typeStart==0) {
    return StringRef();
  }
  AMDIntrinsicInfo::EKind kind = AMDIntrinsicInfo::NORMAL;
  StringRef res = getStrippedFuncName(mangledName.substr(0, typeStart), &kind);
  if (!iInfo) {
    return res;
  }
  iInfo->FKind = kind;

  StringRef type = mangledName.substr(typeStart+1);

  char const f = type.front();
  if (f=='g' || f=='l' || f=='p') {
    type = type.drop_front(1);
    iInfo->HasPtr = true;
  }

  int const v = eatLength(type);
  iInfo->VectorSize = v > 0 ? v : 1;

  if (type.startswith("f32")) {
    iInfo->ArgType = Type::FloatTyID;
  } else
  if (type.startswith("f64")) {
    iInfo->ArgType  = Type::DoubleTyID;
  } else {
    return StringRef();
  }
  return res;
}

StringRef AMDIntrinsicInfo::parseName(StringRef mangledName, 
                                      AMDIntrinsicInfo *iInfo)
{
  StringRef res;
  if (eatTerm(mangledName, "_Z")) {
    iInfo->ManglingKind = AMDIntrinsicInfo::ITANIUM;
    res = parseItaniumFunctionName(mangledName, iInfo);
  } else
  if (eatTerm(mangledName, "__")) {
    iInfo->ManglingKind = AMDIntrinsicInfo::EDG;
    res = parseEDGFunctionName(mangledName, iInfo);
  }
  return res;
}

std::string AMDIntrinsicInfo::mangleName(const StringRef& name) const
{
  const char *pfx = NULL;
  switch(FKind) {
  case HALF:   pfx = "half_";   break;
  case NATIVE: pfx = "native_"; break;
  case HSAIL:  pfx = "hsail_";  break;
  case AMDIL:  pfx = "amdil_";  break;
  case GCN:    pfx = "gcn_";    break;
  default:;
  }
  std::stringstream s;
  if (ManglingKind==ITANIUM) {
    s << "_Z";
    if (pfx) {
      s << static_cast<int>(name.size() + strlen(pfx)) << pfx;
    } else {
      s << static_cast<int>(name.size());
    }
    s.write(name.data(), name.size());
    if (VectorSize > 1) {
        s << "Dv" << VectorSize << '_';
    }
    s << (ArgType == Type::FloatTyID ? 'f' : 'd');
    if (HasPtr) {
      if (VectorSize > 1) s << "PU3AS4S_"; else
      s << "PU3AS4" << (ArgType == Type::FloatTyID ? 'f' : 'd');
    }
  } else {
    s << "__";
    if (pfx) {
      s << pfx;
    }
    s.write(name.data(), name.size());
    s << '_';
    if (HasPtr) {
      s << 'p';
    }
    if (VectorSize > 1) {
      s << VectorSize;
    }
    s << (ArgType == Type::FloatTyID ? "f32" : "f64");
  }
  return s.str();
}

static void mangleName(raw_ostream& os, const StringRef& name) {
  os << static_cast<int>(name.size()) << name;
}

static void mangleParamType(raw_ostream& os, const Type* pt) {
  switch(pt->getTypeID()) {
  case Type::VoidTyID:    os << 'v';  break;
  case Type::HalfTyID:    os << "Dh"; break;
  case Type::FloatTyID:   os << 'f';  break;
  case Type::DoubleTyID:  os << 'd';  break;

  case Type::IntegerTyID: {
    const IntegerType *it = cast<const IntegerType>(pt);
    bool const isSigned = it->getSignBit()!=0ULL;
    switch(it->getBitWidth()) {
    case  8: os << (isSigned ? 'c' : 'h'); break;
    case 16: os << (isSigned ? 's' : 't'); break;
    case 32: os << (isSigned ? 'i' : 'j'); break;
    case 64: os << (isSigned ? 'l' : 'm'); break;
    }
  } break;

  case Type::StructTyID: {
    const StructType *st = cast<const StructType>(pt);
    mangleName(os, st->getName());
  } break;

  case Type::PointerTyID: {
    const PointerType *ptr = cast<const PointerType>(pt);
    os << 'P';
    if (ptr->getAddressSpace()!=0) {
       SmallString<64> buf;
       raw_svector_ostream uname(buf);
       uname << "AS" << static_cast<int>(ptr->getAddressSpace());
       os << 'U';
       mangleName(os, uname.str());
    }
    mangleParamType(os, pt->getPointerElementType());
  } break;

  case Type::ArrayTyID:
    os << 'A' << static_cast<int>(pt->getArrayNumElements()) << '_';
    mangleParamType(os, pt->getArrayElementType());
  break;

  case Type::VectorTyID:
    os << "Dv" << static_cast<int>(pt->getVectorNumElements()) << '_';
    mangleParamType(os, pt->getVectorElementType());
  break;

  default:;
  }
}

std::string llvm::mangleFuncNameItanium(const StringRef& name, FunctionType* type)
{
  std::string res;
  raw_string_ostream s(res);
  s << "_Z";
  mangleName(s, name);

  StringMap<int> substMap;
  int subsNum = 0;

  for(FunctionType::param_iterator i=type->param_begin(), e=type->param_end(); 
      i!=e; ++i) {

    SmallString<64> paramBuffer;
    raw_svector_ostream ps(paramBuffer);
    mangleParamType(ps, *i);
    StringRef paramStr = ps.str();

    StringMap<int>::MapEntryTy &x = substMap.GetOrCreateValue(paramStr, -1);
    if (x.getValue()!=-1) {
      if (x.getValue()>0) {
        s << 'S' << (x.getValue()-1) << '_';
      } else {
        s << "S_";
      }
    } else {
      s << paramStr;
      x.setValue(subsNum++);
    }
  }
  return s.str();
}
