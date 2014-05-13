//
// Copyright (c) 2011 Advanced Micro Devices, Inc. All rights reserved.
//

/*
   This file does library function optimizations. The client of this optimization will
   need to define a derived class of AMDLibCalls and implement replaceCall() function.
*/

#define DEBUG_TYPE "simplifylibcall"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IRBuilder.h"
#include "llvm/ValueSymbolTable.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/LLVMContext.h"
#include "llvm/AMDLLVMContextHook.h"
#include "llvm/Transforms/Utils/AMDLibCalls.h"
#include <vector>
#include <string>
#include <sstream>
#include <cmath>

using namespace llvm;

#if 0
/* Function optimization table */
#define FOPTTBL(f) __opttbl_##f

/* Function Entry Macro */
#define femacro(f) \
{ \
STRVAL(f), \
FIDSTR(f), \
(int)sizeof(FOPTTBL(f)), \
FOPTTBL(f) },
#endif

#define MATH_PI     3.14159265358979323846264338327950288419716939937511
#define MATH_E      2.71828182845904523536028747135266249775724709369996
#define MATH_SQRT2  1.41421356237309504880168872420969807856967187537695

#define MATH_LOG2E     1.4426950408889634073599246810018921374266459541529859
#define MATH_LOG10E    0.4342944819032518276511289189166050822943970058036665
#define MATH_LOG2_10   3.3219280948873623478703194294893901758648313930245806  // Value of log2(10)
#define MATH_RLOG2_10  0.3010299956639811952137388947244930267681898814621085  // Value of 1 / log2(10)
#define MATH_RLOG2_E   0.6931471805599453094172321214581765680755001343602552  // Value of 1 / M_LOG2E_F = 1 / log2(e)

namespace {

/* a list of {result, input} */
AMDLibCalls::FuncTbl __tbl_acos[] = {
  {MATH_PI/2.0, 0.0},
  {MATH_PI/2.0, -0.0},
  {0.0, 1.0},
  {MATH_PI, -1.0}
};
  
AMDLibCalls::FuncTbl __tbl_acosh[] = {
  {0.0, 1.0}
};

AMDLibCalls::FuncTbl __tbl_acospi[] = {
  {0.5, 0.0},
  {0.5, -0.0},
  {0.0, 1.0},
  {1.0, -1.0}
};

AMDLibCalls::FuncTbl __tbl_asin[] = {
  {0.0, 0.0},
  {-0.0, -0.0},
  {MATH_PI/2.0, 1.0},
  {-MATH_PI/2.0, -1.0}
};

AMDLibCalls::FuncTbl __tbl_asinh[] = {
  {0.0, 0.0},
  {-0.0, -0.0}
};

AMDLibCalls::FuncTbl __tbl_asinpi[] = {
  {0.0, 0.0},
  {-0.0, -0.0},
  {0.5, 1.0},
  {-0.5, -1.0}
};
 
AMDLibCalls::FuncTbl __tbl_atan[] = {
  {0.0, 0.0},
  {-0.0, -0.0},
  {MATH_PI/4.0, 1.0},
  {-MATH_PI/4.0, -1.0}
};

AMDLibCalls::FuncTbl __tbl_atanh[] = {
  {0.0, 0.0},
  {-0.0, -0.0}
};

AMDLibCalls::FuncTbl __tbl_atanpi[] = {
  {0.0, 0.0},
  {-0.0, -0.0},
  {0.25, 1.0},
  {-0.25, -1.0}
};

AMDLibCalls::FuncTbl __tbl_cbrt[] = {
  {0.0, 0.0},
  {-0.0, -0.0},
  {1.0, 1.0},
  {-1.0, -1.0},
};

AMDLibCalls::FuncTbl __tbl_cos[] = {
  {1.0, 0.0},
  {1.0, -0.0}
};
 
AMDLibCalls::FuncTbl __tbl_cosh[] = {
  {1.0, 0.0},
  {1.0, -0.0}
};
 
AMDLibCalls::FuncTbl __tbl_cospi[] = {
  {1.0, 0.0},
  {1.0, -0.0}
};

AMDLibCalls::FuncTbl __tbl_erfc[] = {
  {1.0, 0.0},
  {1.0, -0.0}
};

AMDLibCalls::FuncTbl __tbl_erf[] = {
  {0.0, 0.0},
  {-0.0, -0.0}
};

AMDLibCalls::FuncTbl __tbl_exp[] = {
  {1.0, 0.0},
  {1.0, -0.0},
  {MATH_E, 1.0}
};

AMDLibCalls::FuncTbl __tbl_exp2[] = {
  {1.0, 0.0},
  {1.0, -0.0},
  {2.0, 1.0}
};

AMDLibCalls::FuncTbl __tbl_exp10[] = {
  {1.0, 0.0},
  {1.0, -0.0},
  {10.0, 1.0}
};

AMDLibCalls::FuncTbl __tbl_expm1[] = {
  {0.0, 0.0},
  {-0.0, -0.0}
};

AMDLibCalls::FuncTbl __tbl_log[] = {
  {0.0, 1.0},
  {1.0, MATH_E}
};

AMDLibCalls::FuncTbl __tbl_log2[] = {
  {0.0, 1.0},
  {1.0, 2.0}
};

AMDLibCalls::FuncTbl __tbl_log10[] = {
  {0.0, 1.0},
  {1.0, 10.0}
};

AMDLibCalls::FuncTbl __tbl_rsqrt[] = {
  {1.0, 1.0},
  {1.0/MATH_SQRT2, 2.0}
};

AMDLibCalls::FuncTbl __tbl_sin[] = {
  {0.0, 0.0},
  {-0.0, -0.0}
};

AMDLibCalls::FuncTbl __tbl_sinh[] = {
  {0.0, 0.0},
  {-0.0, -0.0}
};

AMDLibCalls::FuncTbl __tbl_sinpi[] = {
  {0.0, 0.0},
  {-0.0, -0.0}
};

AMDLibCalls::FuncTbl __tbl_sqrt[] = {
  {0.0, 0.0},
  {1.0, 1.0},
 {MATH_SQRT2, 2.0}
};

AMDLibCalls::FuncTbl __tbl_tan[] = {
  {0.0, 0.0},
  {-0.0, -0.0}
};

AMDLibCalls::FuncTbl __tbl_tanh[] = {
  {0.0, 0.0},
  {-0.0, -0.0}
};

AMDLibCalls::FuncTbl __tbl_tanpi[] = {
  {0.0, 0.0},
  {-0.0, -0.0}
};

AMDLibCalls::FuncTbl __tbl_tgamma[] = {
  {1.0, 1.0},
  {1.0, 2.0},
  {2.0, 3.0},
  {6.0, 4.0}
};

}  // namespace

sys::SmartMutex<true> AMDLibCalls::LibCallLock;

AMDLibCalls::TDO_Entry AMDLibCalls::OptimTbl[AMDLibCalls::BFID_TDO_LAST+1];
AMDLibCalls::BltinLibDescriptor 
  AMDLibCalls::BltinFuncDesc[AMDLibCalls::BFID_LENGTH];
bool AMDLibCalls::isInitialized = false;

Str2IntMap AMDLibCalls::FName2IDMap;
//const char *AMDLibCalls::ID2FName[AMDLibCalls::BFID_LENGTH];

Constant *AMDLibCalls::getFunction(Module *M,
                                   FunctionType *FuncTy,
                                   StringRef FuncName)
{
  AMDLLVMContextHook *amdhook =
    static_cast<AMDLLVMContextHook*>(M->getContext().getAMDLLVMContextHook());
  bool prelinkopt = amdhook ? amdhook->amdoptions.IsPreLinkOpt : false;
  bool wholeprogram = amdhook ? amdhook->amdoptions.WholeProgram : false;

  // If the function exists (could be of local linkage), return it. 
  ValueSymbolTable &MSymTab = M->getValueSymbolTable();
  Function *F = cast_or_null<Function>(MSymTab.lookup(FuncName));
  if (F && !F->isDeclaration() && 
      !FuncTy->isVarArg() && !F->isVarArg() &&
      (FuncTy->getNumParams() == F->arg_size())) {
    // Found it
    return F;
  }

  // If we are doing PreLinkOpt, the function is external. So it is safe to
  // use getOrInsertFunction() at this stage. The same is true for
  // non-whole program mode.
  if (prelinkopt || !wholeprogram || FuncName.startswith("__hsail_")) {

      // We are supposed to pass only single instruction intrinsics with
      // "__hsail_" prefix for this to work correctly. These intrinsics are
      // resolvable by instruction selection through HSAILIntrinsics.td
      // even without library linking. Other functions supposed to be passed
      // by other names (i.e. "__sqrt_f32" vs "__hsail_nsqrt_f32").

      // Do not set extra attributes for functions with pointer arguments.
      FunctionType::param_iterator PI = FuncTy->param_begin();
      FunctionType::param_iterator PE = FuncTy->param_end();
      for (; PI != PE; ++PI) {
        const Type* argTy = static_cast<const Type*>(*PI);
        if (argTy->isPointerTy()) {
          return M->getOrInsertFunction(FuncName, FuncTy);
        }
      }

      Attributes::AttrVal AVs[2]={Attributes::ReadOnly, Attributes::NoUnwind };
      AttributeWithIndex AWI = AttributeWithIndex::get(M->getContext(), 
                                   AttrListPtr::FunctionIndex,
                                   ArrayRef<Attributes::AttrVal>(AVs, 2));

      return M->getOrInsertFunction(FuncName, 
                                    FuncTy,
                                    AttrListPtr::get(M->getContext(),AWI));
  }
  return NULL;
}

Constant *AMDLibCalls::getFunction(Module *M, 
                                   StringRef FuncName,
                                   Type *RetTy, ...)
{
  va_list Args;
  va_start(Args, RetTy);

  std::vector<Type*> ArgTys;
  while (Type *ArgTy = va_arg(Args, Type*)) {
    ArgTys.push_back(ArgTy);
  }
  va_end(Args);

  return getFunction(M, FunctionType::get(RetTy, ArgTys, false), FuncName);
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
bool AMDLibCalls::parseFunctionName(StringRef FMangledName, FuncInfo &FInfo)
{
  // Get rid of non-candidates quickly
  if (!FMangledName.startswith("__")) {
    return false;
  }

  size_t sz = FMangledName.size();
  if (sz > MAX_FNAME_SIZE) {
    return false;
  }

  // pos to start and end of generic function name, respectively.
  size_t pos_start, pos_end;
  size_t end;  // pos to the end of key arg type
  if (FMangledName.startswith("__native_")) {
    FInfo.FKind = FK_NATIVE;
    pos_start = 9;
  } else if (FMangledName.startswith("__half_")) {
    FInfo.FKind = FK_HALF;
    pos_start = 7;
  } else if (FMangledName.startswith("__hsail_")) {
    FInfo.FKind = FK_HSAIL;
    pos_start = 8;
  } else {
    pos_start = 2;
    FInfo.FKind = FK_NORMAL;
  }

  end = FMangledName.rfind('_');
  if (pos_start >= end) {
    return false;
  }
  pos_end = FMangledName.rfind('_', end);
  if (pos_start >= pos_end) {
    // No optional type encoding
    pos_end = end;
    end = FMangledName.size();
  }
  /* 
  else {
    // has 2nd argument encoding, check the type encoding
  }
  */

  if ((end - pos_end) < 3) {
    return false;
  }

  if (FMangledName[end-3] != 'f') {
    return false;
  }
  if ((FMangledName[end - 2] == '3') && (FMangledName[end - 1] == '2')) {
    FInfo.EType = ET_F32;
  } else if ((FMangledName[end - 2] == '6') && (FMangledName[end - 1] == '4')) {
    FInfo.EType = ET_F64;
  } else {
    return false;
  }

  size_t endm4 = end - 4;
  if (endm4 == pos_end ||
      (endm4 == (pos_end + 1) && isPtrEncode(FMangledName[endm4]))) {
    // scalar form __<name>_f32 or __<name>_f64
    FInfo.VectorSize = 1;
  } else if (endm4 == (pos_end + 1) ||
             (endm4 == (pos_end + 2) && isPtrEncode(FMangledName[endm4-1]))) {
    char c = FMangledName[endm4];
    switch (c) {
    case '2':
        FInfo.VectorSize = 2;
        break;
    case '3':
        FInfo.VectorSize = 3;
        break;
    case '4':
        FInfo.VectorSize = 4;
        break;
    case '8':
        FInfo.VectorSize = 8;
        break;
    default:
        return false;
    }
  } else if (endm4 == (pos_end + 2) ||
             (endm4 == (pos_end + 3) && isPtrEncode(FMangledName[endm4-2]))) {
    char c1 = FMangledName[endm4];
    char c0 = FMangledName[endm4-1];
    if ( (c0 == '1') && (c1 == '6')) {
      FInfo.VectorSize = 16;
    } else {
      return false;
    }
  } else {
    return false;
  }

  // Get generic function name
  int ix=0;
  for (size_t i=pos_start; i < pos_end; ++i, ++ix) {
    FInfo.GFName[ix] = FMangledName[i];
  }
  FInfo.GFName[ix] = 0;

  Str2IntMap::iterator it = FName2IDMap.find(FInfo.GFName);
  if (it == FName2IDMap.end()) {
    return false;
  }

  FInfo.FID = (BLTIN_FUNC_ID)it->second;

  if ((FInfo.FKind != FK_HSAIL) &&
      (BltinFuncDesc[FInfo.FID].flags & BLTINDESC_HsailOnly)) {
    return false;
  }

  return true;
}

// Create a library function name, given a generic function name,
// element type, kind, and vector size.
void AMDLibCalls::getFunctionName(
  std::string& FuncName, // out 
  const char* GFName, BLTIN_FUNC_KIND FKind, ELEMENT_TYPE EType, int VSize)
{
  std::stringstream sstr;
  if (FKind == FK_HALF)
    sstr << "__half_";
  else if (FKind == FK_NATIVE)
    sstr << "__native_";
  else if (FKind == FK_HSAIL)
    sstr << "__hsail_";
  else
    sstr << "__";
  sstr << GFName << "_";
  if (VSize > 1)
    sstr << VSize;
  sstr << (EType == ET_F32 ? "f32" : "f64");
  FuncName = sstr.str();
}

AMDLibCalls::AMDLibCalls()
  : UseNativeFuncs(),
    AllNative(false),
    UnsafeMathOpt(false),
    NoSignedZeros(false),
    FiniteMathOnly(false),
    FastRelaxedMath(false)
{
  if (!isInitialized) {
    init();
  }
}

AMDLibCalls::~AMDLibCalls() { }

void AMDLibCalls::init()
{
  sys::SmartScopedLock<true> CS(LibCallLock);
  if (!isInitialized) {
    /* Initialize FName2IDMap */
    // functions for TDO
    FName2IDMap["acos"] = BFID_acos;
    FName2IDMap["acosh"] = BFID_acosh;
    FName2IDMap["acospi"] = BFID_acospi;
    FName2IDMap["asin"] = BFID_asin;
    FName2IDMap["asinh"] = BFID_asinh;
    FName2IDMap["asinpi"] = BFID_asinpi;
    FName2IDMap["atan"] = BFID_atan;
    FName2IDMap["atanh"] = BFID_atanh;
    FName2IDMap["atanpi"] = BFID_atanpi;
    FName2IDMap["cbrt"] = BFID_cbrt;
    FName2IDMap["cos"] = BFID_cos;
    FName2IDMap["cosh"] = BFID_cosh;
    FName2IDMap["cospi"] = BFID_cospi;
    FName2IDMap["erfc"] = BFID_erfc;
    FName2IDMap["erf"] = BFID_erf;
    FName2IDMap["exp"] = BFID_exp;
    FName2IDMap["exp2"] = BFID_exp2;
    FName2IDMap["exp10"] = BFID_exp10;
    FName2IDMap["expm1"] = BFID_expm1;
    FName2IDMap["log"] = BFID_log;
    FName2IDMap["log2"] = BFID_log2;
    FName2IDMap["log10"] = BFID_log10;
    FName2IDMap["rsqrt"] = BFID_rsqrt;
    FName2IDMap["sin"] = BFID_sin;
    FName2IDMap["sinh"] = BFID_sinh;
    FName2IDMap["sinpi"] = BFID_sinpi;
    FName2IDMap["sqrt"] = BFID_sqrt;
    FName2IDMap["tan"] = BFID_tan;
    FName2IDMap["tanh"] = BFID_tanh;
    FName2IDMap["tanpi"] = BFID_tanpi;
    FName2IDMap["tgamma"] = BFID_tgamma;

    // other functions
    FName2IDMap["recip"] = BFID_recip;
    FName2IDMap["divide"] = BFID_divide;
    FName2IDMap["pow"] = BFID_pow;
    FName2IDMap["powr"] = BFID_powr;
    FName2IDMap["pown"] = BFID_pown;
    FName2IDMap["rootn"] = BFID_rootn;
    FName2IDMap["sincos"] = BFID_sincos;
    FName2IDMap["fma"] = BFID_fma;
    FName2IDMap["mad"] = BFID_mad;
    FName2IDMap["ncos"] = BFID_ncos;
    FName2IDMap["nexp2"] = BFID_nexp2;
    FName2IDMap["nfma"] = BFID_nfma;
    FName2IDMap["nlog2"] =  BFID_nlog2;
    FName2IDMap["nrsqrt"] =  BFID_nrsqrt;
    FName2IDMap["nsin"] =  BFID_nsin;
    FName2IDMap["nsqrt"] =  BFID_nsqrt;
   
#if 0
    /* Initialize ID2FName[] */
    ID2FName[BFID_acos] = "acos";
    ID2FName[BFID_acosh] = "acosh";
    ID2FName[BFID_acospi] = "acospi";
    ID2FName[BFID_asin] = "asin";
    ID2FName[BFID_asinh] = "asinh";
    ID2FName[BFID_asinpi] = "asinpi";
    ID2FName[BFID_atan] = "atan";
    ID2FName[BFID_atanh] = "atanh";
    ID2FName[BFID_atanpi] = "atanpi";
    ID2FName[BFID_cbrt] = "cbrt";
    ID2FName[BFID_cos] = "cos";
    ID2FName[BFID_cosh] = "cosh";
    ID2FName[BFID_cospi] = "cospi";
    ID2FName[BFID_erfc] = "erfc";
    ID2FName[BFID_erf] = "erf";
    ID2FName[BFID_exp] = "exp";
    ID2FName[BFID_exp2] = "exp2";
    ID2FName[BFID_exp10] = "exp10";
    ID2FName[BFID_expm1] = "expm1";
    ID2FName[BFID_log] = "log";
    ID2FName[BFID_log2] = "log2";
    ID2FName[BFID_log10] = "log10";
    ID2FName[BFID_rsqrt] = "rsqrt";
    ID2FName[BFID_sin] = "sin";
    ID2FName[BFID_sinh] = "sinh";
    ID2FName[BFID_sinpi] = "sinpi";
    ID2FName[BFID_sqrt] = "sqrt";
    ID2FName[BFID_tan] = "tan";
    ID2FName[BFID_tanh] = "tanh";
    ID2FName[BFID_tanpi] = "tanpi";
    ID2FName[BFID_tgamma] = "tgamma";

    ID2FName[BFID_pow] = "pow";
    ID2FName[BFID_powr] = "powr";
    ID2FName[BFID_pown] = "pown";
    ID2FName[BFID_rootn] = "rootn";
    ID2FName[BFID_fma] = "fma";
    ID2FName[BFID_mad] = "mad";
#endif

    // Initialize BltinFunDesc

    // 1. Set all entries to have HasNormal
    for (int i=0; i < BFID_LENGTH; ++i) {
      BltinFuncDesc[i].flags = BLTINDESC_HasNormal;
    }
    // 2. set for Native/half
    BltinFuncDesc[BFID_divide].flags = (BLTINDESC_HasNative|BLTINDESC_HasHalf); 

    BltinFuncDesc[BFID_cos].flags |= (BLTINDESC_HasNative|BLTINDESC_HasHalf); 
    BltinFuncDesc[BFID_exp].flags |= (BLTINDESC_HasNative|BLTINDESC_HasHalf); 
    BltinFuncDesc[BFID_exp2].flags |= (BLTINDESC_HasNative|BLTINDESC_HasHalf); 
    BltinFuncDesc[BFID_exp10].flags |= (BLTINDESC_HasNative|BLTINDESC_HasHalf); 
    BltinFuncDesc[BFID_log].flags |= (BLTINDESC_HasNative|BLTINDESC_HasHalf); 
    BltinFuncDesc[BFID_log2].flags |= (BLTINDESC_HasNative|BLTINDESC_HasHalf); 
    BltinFuncDesc[BFID_log10].flags |= (BLTINDESC_HasNative|BLTINDESC_HasHalf); 
    BltinFuncDesc[BFID_powr].flags |= (BLTINDESC_HasNative|BLTINDESC_HasHalf); 
    BltinFuncDesc[BFID_recip].flags |= (BLTINDESC_HasNative|BLTINDESC_HasHalf); 
    BltinFuncDesc[BFID_rsqrt].flags |= (BLTINDESC_HasNative|BLTINDESC_HasHalf); 
    BltinFuncDesc[BFID_sin].flags |= (BLTINDESC_HasNative|BLTINDESC_HasHalf); 
    BltinFuncDesc[BFID_sqrt].flags |= (BLTINDESC_HasNative|BLTINDESC_HasHalf); 
    BltinFuncDesc[BFID_tan].flags |= (BLTINDESC_HasNative|BLTINDESC_HasHalf); 

    BltinFuncDesc[BFID_ncos].flags |= BLTINDESC_HsailOnly;
    BltinFuncDesc[BFID_nexp2].flags |= BLTINDESC_HsailOnly;
    BltinFuncDesc[BFID_nfma].flags |= BLTINDESC_HsailOnly;
    BltinFuncDesc[BFID_nlog2].flags |= BLTINDESC_HsailOnly;
    BltinFuncDesc[BFID_nrsqrt].flags |= BLTINDESC_HsailOnly;
    BltinFuncDesc[BFID_nsin].flags |= BLTINDESC_HsailOnly;
    BltinFuncDesc[BFID_nsqrt].flags |= BLTINDESC_HsailOnly;

    // functions w/ ptr arguments
    BltinFuncDesc[BFID_sincos].flags |= BLTINDESC_HasPtrArg;


    /* Initialize OptimTbl[] for Table-driven optimizations */ 
    OptimTbl[BFID_acos].opttbl        = __tbl_acos;
    OptimTbl[BFID_acos].opttbl_size   = (int)(sizeof(__tbl_acos)/sizeof(FuncTbl));
    OptimTbl[BFID_acosh].opttbl       = __tbl_acosh;
    OptimTbl[BFID_acosh].opttbl_size  = (int)(sizeof(__tbl_acosh)/sizeof(FuncTbl));
    OptimTbl[BFID_acospi].opttbl      = __tbl_acospi;
    OptimTbl[BFID_acospi].opttbl_size = (int)(sizeof(__tbl_acospi)/sizeof(FuncTbl));
    OptimTbl[BFID_asin].opttbl        = __tbl_asin;
    OptimTbl[BFID_asin].opttbl_size   = (int)(sizeof(__tbl_asin)/sizeof(FuncTbl));
    OptimTbl[BFID_asinh].opttbl       = __tbl_asinh;
    OptimTbl[BFID_asinh].opttbl_size  = (int)(sizeof(__tbl_asinh)/sizeof(FuncTbl));
    OptimTbl[BFID_asinpi].opttbl      = __tbl_asinpi;
    OptimTbl[BFID_asinpi].opttbl_size = (int)(sizeof(__tbl_asinpi)/sizeof(FuncTbl));
    OptimTbl[BFID_atan].opttbl        = __tbl_atan;
    OptimTbl[BFID_atan].opttbl_size   = (int)(sizeof(__tbl_atan)/sizeof(FuncTbl));
    OptimTbl[BFID_atanh].opttbl       = __tbl_atanh;
    OptimTbl[BFID_atanh].opttbl_size  = (int)(sizeof(__tbl_atanh)/sizeof(FuncTbl));
    OptimTbl[BFID_atanpi].opttbl      = __tbl_atanpi;
    OptimTbl[BFID_atanpi].opttbl_size = (int)(sizeof(__tbl_atanpi)/sizeof(FuncTbl));
    OptimTbl[BFID_cbrt].opttbl        = __tbl_cbrt;
    OptimTbl[BFID_cbrt].opttbl_size   = (int)(sizeof(__tbl_cbrt)/sizeof(FuncTbl));
    OptimTbl[BFID_cos].opttbl         = __tbl_cos;
    OptimTbl[BFID_cos].opttbl_size    = (int)(sizeof(__tbl_cos)/sizeof(FuncTbl));
    OptimTbl[BFID_cosh].opttbl        = __tbl_cosh;
    OptimTbl[BFID_cosh].opttbl_size   = (int)(sizeof(__tbl_cosh)/sizeof(FuncTbl));
    OptimTbl[BFID_cospi].opttbl       = __tbl_cospi;
    OptimTbl[BFID_cospi].opttbl_size  = (int)(sizeof(__tbl_cospi)/sizeof(FuncTbl));
    OptimTbl[BFID_erfc].opttbl        = __tbl_erfc;
    OptimTbl[BFID_erfc].opttbl_size   = (int)(sizeof(__tbl_erfc)/sizeof(FuncTbl));
    OptimTbl[BFID_erf].opttbl         = __tbl_erf;
    OptimTbl[BFID_erf].opttbl_size    = (int)(sizeof(__tbl_erf)/sizeof(FuncTbl));
    OptimTbl[BFID_exp].opttbl         = __tbl_exp;
    OptimTbl[BFID_exp].opttbl_size    = (int)(sizeof(__tbl_exp)/sizeof(FuncTbl));
    OptimTbl[BFID_exp2].opttbl        = __tbl_exp2;
    OptimTbl[BFID_exp2].opttbl_size   = (int)(sizeof(__tbl_exp2)/sizeof(FuncTbl));
    OptimTbl[BFID_exp10].opttbl       = __tbl_exp10;
    OptimTbl[BFID_exp10].opttbl_size  = (int)(sizeof(__tbl_exp10)/sizeof(FuncTbl));
    OptimTbl[BFID_expm1].opttbl       = __tbl_expm1;
    OptimTbl[BFID_expm1].opttbl_size  = (int)(sizeof(__tbl_expm1)/sizeof(FuncTbl));
    OptimTbl[BFID_log].opttbl         = __tbl_log;
    OptimTbl[BFID_log].opttbl_size    = (int)(sizeof(__tbl_log)/sizeof(FuncTbl));
    OptimTbl[BFID_log2].opttbl        = __tbl_log2;
    OptimTbl[BFID_log2].opttbl_size   = (int)(sizeof(__tbl_log2)/sizeof(FuncTbl));
    OptimTbl[BFID_log10].opttbl       = __tbl_log10;
    OptimTbl[BFID_log10].opttbl_size  = (int)(sizeof(__tbl_log10)/sizeof(FuncTbl));
    OptimTbl[BFID_rsqrt].opttbl       = __tbl_rsqrt;
    OptimTbl[BFID_rsqrt].opttbl_size  = (int)(sizeof(__tbl_rsqrt)/sizeof(FuncTbl));
    OptimTbl[BFID_sin].opttbl         = __tbl_sin;
    OptimTbl[BFID_sin].opttbl_size    = (int)(sizeof(__tbl_sin)/sizeof(FuncTbl));
    OptimTbl[BFID_sinh].opttbl        = __tbl_sinh;
    OptimTbl[BFID_sinh].opttbl_size   = (int)(sizeof(__tbl_sinh)/sizeof(FuncTbl));
    OptimTbl[BFID_sinpi].opttbl       = __tbl_sinpi;
    OptimTbl[BFID_sinpi].opttbl_size  = (int)(sizeof(__tbl_sinpi)/sizeof(FuncTbl));
    OptimTbl[BFID_sqrt].opttbl        = __tbl_sqrt;
    OptimTbl[BFID_sqrt].opttbl_size   = (int)(sizeof(__tbl_sqrt)/sizeof(FuncTbl));
    OptimTbl[BFID_tan].opttbl         = __tbl_tan;
    OptimTbl[BFID_tan].opttbl_size    = (int)(sizeof(__tbl_tan)/sizeof(FuncTbl));
    OptimTbl[BFID_tanh].opttbl        = __tbl_tanh;
    OptimTbl[BFID_tanh].opttbl_size   = (int)(sizeof(__tbl_tanh)/sizeof(FuncTbl));
    OptimTbl[BFID_tanpi].opttbl       = __tbl_tanpi;
    OptimTbl[BFID_tanpi].opttbl_size  = (int)(sizeof(__tbl_tanpi)/sizeof(FuncTbl));
    OptimTbl[BFID_tgamma].opttbl      = __tbl_tgamma;
    OptimTbl[BFID_tgamma].opttbl_size = (int)(sizeof(__tbl_tgamma)/sizeof(FuncTbl));
    OptimTbl[BFID_ncos].opttbl        = __tbl_cos;
    OptimTbl[BFID_ncos].opttbl_size   = (int)(sizeof(__tbl_cos)/sizeof(FuncTbl));
    OptimTbl[BFID_nexp2].opttbl       = __tbl_exp2;
    OptimTbl[BFID_nexp2].opttbl_size  = (int)(sizeof(__tbl_exp2)/sizeof(FuncTbl));
    OptimTbl[BFID_nlog2].opttbl       = __tbl_log2;
    OptimTbl[BFID_nlog2].opttbl_size  = (int)(sizeof(__tbl_log2)/sizeof(FuncTbl));
    OptimTbl[BFID_nrsqrt].opttbl      = __tbl_rsqrt;
    OptimTbl[BFID_nrsqrt].opttbl_size = (int)(sizeof(__tbl_rsqrt)/sizeof(FuncTbl));
    OptimTbl[BFID_nsin].opttbl        = __tbl_sin;
    OptimTbl[BFID_nsin].opttbl_size   = (int)(sizeof(__tbl_sin)/sizeof(FuncTbl));
    OptimTbl[BFID_nsqrt].opttbl       = __tbl_sqrt;
    OptimTbl[BFID_nsqrt].opttbl_size  = (int)(sizeof(__tbl_sqrt)/sizeof(FuncTbl));
    isInitialized = true;
  }
}

void AMDLibCalls::setFuncNames(const char* V)
{
  if (V == NULL) {
    return;
  } else if ((V[0] == 'a') && (V[1] == 'l') && (V[2] == 'l') && (V[3] == 0)) {
    AllNative = true;
    return;
  }

  // 64 is more than enough for all OpenCL intrinsic name.
  char ctmp[64];
  int j = 0;
  for (int i = 0; V[i] != 0; ++i) {
    char c = V[i];
    if (c == ',') {
      if (j > 0) {
        ctmp[j] = 0;
        UseNativeFuncs[ctmp] = 1;
        j = 0;
      }
      /* else { wrong arguments } */
    } else {
      ctmp[j] = c;
      ++j;
      if (j > 63) {
        // Ignore the entry (error message)
        j = 0;
      }
    }
  }
  if (j > 0) {
    // last entry
    ctmp[j] = 0;
    UseNativeFuncs[ctmp] = 1;
  }
}

bool AMDLibCalls::sincosUseNative(CallInst *aCI, FuncInfo &FInfo,
                                  const char *Suffix)
{
  std::string tname;
  bool native_sin = (UseNativeFuncs.lookup("sin") == 1);
  bool native_cos = (UseNativeFuncs.lookup("cos") == 1);
  if (native_sin || native_cos) {
    Module *M = aCI->getParent()->getParent()->getParent();
    Value *opr0 = aCI->getArgOperand(0);

    tname = native_sin ? "__native_sin_" : "__sin_";
    tname += Suffix;
    Constant *sinExpr = getFunction(M,
                                    tname,
                                    aCI->getType(),
                                    opr0->getType(),
                                    NULL);

    tname = native_cos ? "__native_cos_" : "__cos_";
    tname += Suffix;
    Constant *cosExpr = getFunction(M,
                                    tname,
                                    aCI->getType(),
                                    opr0->getType(),
                                    NULL);
    if (sinExpr && cosExpr) {
      Value *sinval = CallInst::Create(sinExpr, opr0, "splitsin", aCI);
      Value *cosval = CallInst::Create(cosExpr, opr0, "splitcos", aCI);
      new StoreInst(cosval, aCI->getArgOperand(1), aCI); 

      DEBUG_WITH_TYPE("usenative", dbgs() << "<useNative> replace " << *aCI
                                          << " with native version of sin/cos");

      replaceCall(sinval);
      return true;
    }
  }
  return false;
}

bool AMDLibCalls::useNative(CallInst *aCI, const DataLayout *DL)
{
  this->CI = aCI;
  Function *Callee = CI->getCalledFunction();
  StringRef MangledFName = Callee->getName();

  FuncInfo FInfo;

  if (!parseFunctionName(MangledFName, FInfo)) {
    return false;
  }

  // If this func isn't a normal one or is f64.
  if (FInfo.FKind != FK_NORMAL || FInfo.EType == ET_F64)
    return false;

  if (FInfo.FID == BFID_sincos) {
    // __sincos_<s>[10][11]...
    return sincosUseNative(aCI, FInfo, MangledFName.data()+10);
  }

  // no native or not requested, skip
  if ((BltinFuncDesc[FInfo.FID].flags & BLTINDESC_HasNative) == 0 ||
      (!AllNative && UseNativeFuncs.lookup(FInfo.GFName) != 1)) {
    return false;
  }

  std::string nativeFunc;
  nativeFunc = "__native_";
  nativeFunc += (MangledFName.data() + 2);   
  Module *M = CI->getParent()->getParent()->getParent();
  Constant *func = M->getOrInsertFunction(nativeFunc, Callee->getFunctionType());
  CI->setCalledFunction(func);
  DEBUG_WITH_TYPE("usenative", dbgs() << "<useNative> replace " << *CI
                                      << " with native version");
  return true;
}

/*
   This function returns false if no change; return true otherwise.

   virtual function replaceCall() will decide what and how to do with the
   original call instruction, and it must be defined in a derived class.
*/
bool
AMDLibCalls::fold(CallInst *CI, const DataLayout *DL, AliasAnalysis *AA)
{
  this->CI = CI;
  Function *Callee = CI->getCalledFunction();

  // Ignore indirect calls.
  if (Callee == 0) return false;

  StringRef MangledFName = Callee->getName();
  // const FunctionType *FT = Callee->getFunctionType();
  BasicBlock *BB = CI->getParent();
  LLVMContext &Context = CI->getParent()->getContext();
  IRBuilder<> B(Context);

  // Set the builder to the instruction after the call.
  B.SetInsertPoint(BB, CI);

  FuncInfo FInfo;

  if (!parseFunctionName(MangledFName, FInfo)) {
    return false;
  }

  BLTIN_FUNC_ID FID = FInfo.FID;

  // Further check the number of arguments to see if they match.
  if (CI->getNumArgOperands() != getNumOfArgs(FID)) {
    return false;
  }

  if ((BFID_TDO_FIRST <= FID) && (FID <= BFID_TDO_LAST) &&
      TDOFold(CI, DL, FInfo)) {
     return true;
  }

  // Under unsafe-math, evaluate calls if possible.
  // According to Brian Sumner, we can do this for all f32 function calls
  // using host's double function calls.
  if ((UnsafeMathOpt /* || FInfo.EType == ET_F32 */) &&
      evaluateCall(CI, FInfo))
    return true;

  // Specilized optimizations for each function call
  switch (FID) {
  case BFID_recip:
    // skip vector function
    assert ((FInfo.FKind == FK_NATIVE || FInfo.FKind == FK_HALF) &&
            "recip must be an either native or half function");
    return (FInfo.VectorSize != 1) ? false : fold_recip(CI, B, FInfo);

  case BFID_divide:
    // skip vector function
    assert ((FInfo.FKind == FK_NATIVE || FInfo.FKind == FK_HALF) &&
            "divide must be an either native or half function");
    return (FInfo.VectorSize != 1) ? false : fold_divide(CI, B, FInfo);

  case BFID_pow:
  case BFID_powr:
  case BFID_pown:
    return fold_pow(CI, B, FInfo);

  case BFID_rootn:
    // skip vector function
    return (FInfo.VectorSize != 1) ? false : fold_rootn(CI, B, FInfo);

  case BFID_fma:
  case BFID_mad:
    // skip vector function
    return (FInfo.VectorSize != 1) ? false : fold_fma_mad(CI, B, FInfo);

  case BFID_exp:
    return UnsafeMathOpt && (replaceWithNative(CI, FInfo) || fold_exp(CI, B, FInfo));
  case BFID_exp2:
    return UnsafeMathOpt && (replaceWithNative(CI, FInfo) || fold_exp2(CI, B, FInfo));
  case BFID_exp10:
    return UnsafeMathOpt && (replaceWithNative(CI, FInfo) || fold_exp10(CI, B, FInfo));
  case BFID_log:
    return UnsafeMathOpt && (replaceWithNative(CI, FInfo) || fold_log(CI, B, FInfo));
  case BFID_log2:
    return UnsafeMathOpt && (replaceWithNative(CI, FInfo) || fold_log2(CI, B, FInfo));
  case BFID_log10:
    return UnsafeMathOpt && (replaceWithNative(CI, FInfo) || fold_log10(CI, B, FInfo));
  case BFID_sqrt:
    return UnsafeMathOpt && fold_sqrt(CI, B, FInfo);

  case BFID_cos:
  case BFID_sin:
    if (((FInfo.EType == ET_F32) || (FInfo.EType == ET_F64))
      && (FInfo.FKind != FK_NATIVE)
      && (FInfo.FKind != FK_HSAIL)) {
      return fold_sincos(CI, B, AA);
    }
    break;

  default:
    break;
  }

  return false;
}

bool AMDLibCalls::TDOFold(CallInst *CI, const DataLayout *DL, FuncInfo &FInfo)
{
  BLTIN_FUNC_ID FID = FInfo.FID;

  assert ((BFID_TDO_FIRST <= FID) && (FID <= BFID_TDO_LAST) &&
          "FID is out of range of TDO functions ID range");

  // Table-Driven optimization
  int sz = OptimTbl[FID].opttbl_size;
  struct FuncTbl *ftbl = OptimTbl[FID].opttbl;
  Value *opr0 = CI->getArgOperand(0);

  if (FInfo.VectorSize > 1) {
    if (ConstantDataVector *CV = dyn_cast<ConstantDataVector>(opr0)) {
      SmallVector<double, 0> DVal;
      for (int eltNo = 0; eltNo < FInfo.VectorSize; ++eltNo) {
        ConstantFP *eltval = dyn_cast<ConstantFP>(
                               CV->getElementAsConstant((unsigned)eltNo));
        assert(eltval && "Non-FP arguments in math function!");
        bool found = false;
        for (int i=0; i < sz; ++i) {
          if (eltval->isExactlyValue(ftbl[i].input)) {
            DVal.push_back(ftbl[i].result);
            found = true;
            break;
          }
        }
        if (!found) {
          // This vector constants not handled yet.
          return false;
        }
      }
      LLVMContext &context = CI->getParent()->getParent()->getContext();
      Constant *nval;
      if (FInfo.EType == ET_F32) {
        SmallVector<float, 0> FVal;
        for (unsigned i=0; i < DVal.size(); ++i) {
          FVal.push_back((float)DVal[i]);
        }
        ArrayRef<float> tmp(FVal);
        nval = ConstantDataVector::get(context, tmp);
      } else { // ET_F64
        ArrayRef<double> tmp(DVal);
        nval = ConstantDataVector::get(context, tmp);
      }
      DEBUG(errs() << "AMDIC: " << *CI
                   << " ---> " << *nval << "\n");
      replaceCall(nval);
      return true;
    }
  }
  else {
    // Scalar version
    if (ConstantFP *CF = dyn_cast<ConstantFP>(opr0)) {
      for (int i=0; i < sz; ++i) {
        if (CF->isExactlyValue(ftbl[i].input)) {
          Value *nval = ConstantFP::get(CF->getType(), ftbl[i].result);
          DEBUG(errs() << "AMDIC: " << *CI
                       << " ---> " << *nval << "\n");
          replaceCall(nval);
          return true;
        }
      }
    }
  }

  return false;
}

bool AMDLibCalls::replaceWithNative(CallInst *CI, FuncInfo &FInfo)
{
  Module *M = CI->getParent()->getParent()->getParent();
  std::string nativeFN;
  if (FInfo.EType != ET_F32  || FInfo.FKind != FK_NORMAL ||
      (BltinFuncDesc[FInfo.FID].flags & BLTINDESC_HasNative) == 0)
    return false;
      
  getFunctionName(nativeFN, FInfo.GFName, FK_NATIVE,
                  FInfo.EType, FInfo.VectorSize);
  FunctionType *FTy = CI->getCalledFunction()->getFunctionType();
  if (Constant *FPExpr = getFunction(M, FTy, nativeFN)) {

    DEBUG(dbgs() << "AMDIC: " << *CI << " ---> ");

    CI->setCalledFunction(FPExpr);

    DEBUG(dbgs() << *CI << '\n');

    return true;
  }
  return false;
}

//  [native_]half_recip(c) ==> 1.0/c
bool AMDLibCalls::fold_recip(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo)
{
  Value *opr0 = CI->getArgOperand(0);
  if (ConstantFP *CF = dyn_cast<ConstantFP>(opr0)) {
    // Just create a normal div. Later, InstCombine will be able
    // to compute the divide into a constant (avoid check float infinity
    // or subnormal at this point).
    Value *nval = B.CreateFDiv(ConstantFP::get(CF->getType(), 1.0),
                               opr0,
                               "recip2div");
    DEBUG(errs() << "AMDIC: " << *CI
                 << " ---> " << *nval << "\n");
    replaceCall(nval);
    return true;
  }
  return false;
}

//  [native_]half_divide(x, c) ==> x/c
bool AMDLibCalls::fold_divide(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo)
{
  Value *opr0 = CI->getArgOperand(0);
  Value *opr1 = CI->getArgOperand(1);
  ConstantFP *CF0 = dyn_cast<ConstantFP>(opr0);
  ConstantFP *CF1 = dyn_cast<ConstantFP>(opr1);

  if ((CF0 && CF1) ||  // both are constants
      (CF1 && (FInfo.EType == ET_F32)))  // CF1 is constant && f32 divide
  {
    Value *nval1 = B.CreateFDiv(ConstantFP::get(opr1->getType(), 1.0),
                                opr1, "__div2recip");
    Value *nval  = B.CreateFMul(opr0, nval1, "__div2mul");
    replaceCall(nval);
    return true;
  }
  return false;
}

bool AMDLibCalls::fold_pow(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo)
{
  BLTIN_FUNC_ID FID  = FInfo.FID;
  assert((FID == BFID_pow || FID == BFID_powr || FID == BFID_pown) &&
         "fold_pow: encounter a wrong function call");

  Value *opr0, *opr1;
  ConstantFP *CF;
  ConstantInt *CINT;
  ConstantAggregateZero *CZero;
  Type *eltType;

  opr0 = CI->getArgOperand(0);
  opr1 = CI->getArgOperand(1);
  CZero = dyn_cast<ConstantAggregateZero>(opr1);
  if (FInfo.VectorSize == 1) {
    eltType = opr0->getType();
    CF = dyn_cast<ConstantFP>(opr1);
    CINT = dyn_cast<ConstantInt>(opr1);
  } else {
    VectorType *VTy = dyn_cast<VectorType>(opr0->getType());
    assert(VTy && "Oprand of vector function should be of vectortype");
    eltType = VTy->getElementType();
    ConstantDataVector *CDV = dyn_cast<ConstantDataVector>(opr1);

    // Now, only Handle vector const whose elements have the same value.
    CF = CDV ? dyn_cast_or_null<ConstantFP>(CDV->getSplatValue()) : NULL;
    CINT = CDV ? dyn_cast_or_null<ConstantInt>(CDV->getSplatValue()) : NULL;
  }

  // No unsafe math , no constant argument, do nothing
  if (!UnsafeMathOpt && !CF && !CINT && !CZero)
    return false;

  // 0x1111111 means that we don't do anything for this call.
  int ci_opr1 = (CINT ? (int)CINT->getSExtValue() : 0x1111111); 

  if ((CF && CF->isZero()) || (CINT && ci_opr1 == 0) || CZero) {
    //  pow/powr/pown(x, 0) == 1
    DEBUG(errs() << "AMDIC: " << *CI << " ---> 1\n");
    Constant *cnval = ConstantFP::get(eltType, 1.0);
    if (FInfo.VectorSize > 1) {
      cnval = ConstantDataVector::getSplat(FInfo.VectorSize, cnval);
    }
    replaceCall(cnval);
    return true;
  }
  if ((CF && CF->isExactlyValue(1.0)) || (CINT && ci_opr1 == 1)) {
    // pow/powr/pown(x, 1.0) = x
    DEBUG(errs() << "AMDIC: " << *CI
                 << " ---> " << *opr0 << "\n");
    replaceCall(opr0);
    return true;
  }
  if ((CF && CF->isExactlyValue(2.0)) || (CINT && ci_opr1 == 2)) {
    // pow/powr/pown(x, 2.0) = x*x
    DEBUG(errs() << "AMDIC: " << *CI
                 << " ---> " << *opr0 << " * " << *opr0 << "\n");
    Value *nval = B.CreateFMul(opr0, opr0, "__pow2");
    replaceCall(nval);
    return true;
  }
  if ((CF && CF->isExactlyValue(-1.0)) || (CINT && ci_opr1 == -1)) {
    // pow/powr/pown(x, -1.0) = 1.0/x
    DEBUG(errs() << "AMDIC: " << *CI
                 << " ---> 1 / " << *opr0 << "\n");
    Constant *cnval = ConstantFP::get(eltType, 1.0);
    if (FInfo.VectorSize > 1) {
      cnval = ConstantDataVector::getSplat(FInfo.VectorSize, cnval);
    }
    Value *nval = B.CreateFDiv(cnval, opr0, "__powrecip");
    replaceCall(nval);
    return true;
  }

  Module *M = CI->getParent()->getParent()->getParent();
  if (CF && (CF->isExactlyValue(0.5) || CF->isExactlyValue(-0.5))) {
    // pow[r](x, [-]0.5) = sqrt(x)
    std::string tname;
    bool issqrt = CF->isExactlyValue(0.5);
    if (issqrt)
      getFunctionName(tname, "sqrt",
                      FInfo.FKind, FInfo.EType, FInfo.VectorSize);
    else
      getFunctionName(tname, "rsqrt",
                      FInfo.FKind, FInfo.EType, FInfo.VectorSize);

    if (Constant *FPExpr = getFunction(M,
                                       tname, 
                                       CI->getType(),
                                       opr0->getType(),
                                       NULL)) {
      DEBUG(errs() << "AMDIC: " << *CI << " ---> "
                   << tname.c_str() << "(" << *opr0 << ")\n");
      Value *nval = B.CreateCall(FPExpr, opr0, issqrt ? "__pow2sqrt"
                                                      : "__pow2rsqrt");
      replaceCall(nval);
      return true;
    }
  }

  if (!UnsafeMathOpt)
    return false;

  // Unsafe Math optimization

  // Remember that ci_opr1 is set if opr1 is integral
  if (CF) {
    double dval = (FInfo.EType == ET_F32)
                    ? (double)CF->getValueAPF().convertToFloat()
                    : CF->getValueAPF().convertToDouble();
    int ival = (int)dval;
    if ((double)ival == dval) {
      ci_opr1 = ival;
    } else
      ci_opr1 = 0x11111111;
  }

  // pow/powr/pown(x, c) = [1/](x*x*..x); where
  //   trunc(c) == c && the number of x == c && |c| <= 12
  unsigned abs_opr1 = (ci_opr1 < 0) ? -ci_opr1 : ci_opr1;
  if (abs_opr1 <= 12) {
    Constant *cnval;
    Value *nval;
    if (abs_opr1 == 0) {
      cnval = ConstantFP::get(eltType, 1.0);
      if (FInfo.VectorSize > 1) {
        cnval = ConstantDataVector::getSplat(FInfo.VectorSize, cnval);
      }
      nval = cnval;
    } else {
      Value *valx2 = NULL;
      nval = NULL;
      while (abs_opr1 > 0) {
        valx2 = valx2 ? B.CreateFMul(valx2, valx2, "__powx2") : opr0;
        if (abs_opr1 & 1) {
          nval = nval ? B.CreateFMul(nval, valx2, "__powprod") : valx2;
        }
        abs_opr1 >>= 1;
      }
    }
      
    if (ci_opr1 < 0) {
      cnval = ConstantFP::get(eltType, 1.0);
      if (FInfo.VectorSize > 1) {
        cnval = ConstantDataVector::getSplat(FInfo.VectorSize, cnval);
      }
      nval = B.CreateFDiv(cnval, nval, "__1powprod");
    }
    DEBUG(errs() << "AMDIC: " << *CI << " ---> "
                 <<  ((ci_opr1 < 0) ? "1/prod(" : "prod(") << *opr0 << ")\n");
    replaceCall(nval);
    return true;
  }

  // pow/powr ---> exp(y * log(x))
  std::string expfname;
  getFunctionName(expfname, "exp",
                  FInfo.FKind, FInfo.EType, FInfo.VectorSize);
  Constant *ExpExpr = getFunction(M,
                                  expfname, 
                                  CI->getType(),
                                  opr0->getType(),
                                  NULL);
  if (!ExpExpr)
    return false;

  bool needlog = false;
  Constant *cnval;
  if (FInfo.VectorSize == 1) {
    CF = dyn_cast<ConstantFP>(opr0);

    // Don't do pown if opr0 isn't const, or const is negative.
    if (FID == BFID_pown && (!CF || CF->isNegative()))
      return false;

    if (CF) {
      double V = (FInfo.EType == ET_F32)
                   ? (double)CF->getValueAPF().convertToFloat()
                   : CF->getValueAPF().convertToDouble();
      
      V = log(V);
      cnval = ConstantFP::get(eltType, V);
    } else {
      needlog = true;
    }
  } else {
    ConstantDataVector *CDV = dyn_cast<ConstantDataVector>(opr0);

    // If opr0 isn't constant, or any of opr0's constant is negative,
    // don't do pown
    if (FID == BFID_pown) {
      if (!CDV)
        return false;

      for (int i=0; i < FInfo.VectorSize; ++i) {
        if (CDV->getElementAsAPFloat(i).isNegative())
          return false;
      }
    }

    if (!CDV) {
      needlog = true;
    } else {
      assert ((int)CDV->getNumElements() == FInfo.VectorSize &&
              "Wrong vector size detected");


      SmallVector<double, 0> DVal;
      for (int i=0; i < FInfo.VectorSize; ++i) {
        double V = (FInfo.EType == ET_F32)
                     ? (double)CDV->getElementAsFloat(i)
                     : CDV->getElementAsDouble(i);
        V = log(V);
        DVal.push_back(V);
      }
      if (FInfo.EType == ET_F32) {
        SmallVector<float, 0> FVal;
        for (unsigned i=0; i < DVal.size(); ++i) {
          FVal.push_back((float)DVal[i]);
        }
        ArrayRef<float> tmp(FVal);
        cnval = ConstantDataVector::get(M->getContext(), tmp);
      } else {
        ArrayRef<double> tmp(DVal);
        cnval = ConstantDataVector::get(M->getContext(), tmp);
      }
    }
  }

  Value *nval;
  if (needlog) {
    std::string logfname;
    getFunctionName(logfname, "log",
                    FInfo.FKind, FInfo.EType, FInfo.VectorSize);
    Constant *LogExpr = getFunction(M,
                                    logfname, 
                                    CI->getType(),
                                    opr0->getType(),
                                    NULL);
    if (!LogExpr)
      return false;
    nval = B.CreateCall(LogExpr, opr0, logfname);
  } else {
    nval = cnval;
  }

  if (FID == BFID_pown) {
    // convert int(32) to fp(f32 or f64)
    opr1 = B.CreateSIToFP(opr1, nval->getType(), "pownI2F");
  }
  nval = B.CreateFMul(opr1, nval, "__ylogx");
  nval = B.CreateCall(ExpExpr, nval, expfname);
  
  DEBUG(errs() << "AMDIC: " << *CI << " ---> "
               << "exp(" << *opr1 << " * log(" << *opr0 << "))\n");
  replaceCall(nval);
        
  return true;
}

bool AMDLibCalls::fold_rootn(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo)
{
  Value *opr0 = CI->getArgOperand(0);
  Value *opr1 = CI->getArgOperand(1);
  BLTIN_FUNC_ID FID = FInfo.FID;

  ConstantInt *CINT = dyn_cast<ConstantInt>(opr1);
  if (!CINT) {
    return false;
  }
  int ci_opr1 = (int)CINT->getSExtValue();
  if (ci_opr1 == 1) {  // rootn(x, 1) = x
    DEBUG(errs() << "AMDIC: " << *CI
                 << " ---> " << *opr0 << "\n");
    replaceCall(opr0);
    return true;
  }
  if (ci_opr1 == 2) {  // rootn(x, 2) = sqrt(x)
    std::vector<const Type*> ParamsTys;
    ParamsTys.push_back(opr0->getType());
    Module *M = CI->getParent()->getParent()->getParent();
    if (Constant *FPExpr = getFunction(
         M,
         (FInfo.EType == ET_F32) ? "__sqrt_f32" : "__sqrt_f64", 
         CI->getType(),
         opr0->getType(),
         NULL)) {
      DEBUG(errs() << "AMDIC: " << *CI
                   << " ---> sqrt(" << *opr0 << ")\n");
      Value *nval = B.CreateCall(FPExpr, opr0, "__rootn2sqrt");
      replaceCall(nval);
      return true;
    }
  } else if (ci_opr1 == 3) { // rootn(x, -1) = 1.0/x
    Module *M = CI->getParent()->getParent()->getParent();
    if (Constant *FPExpr = getFunction(
         M,
         (FInfo.EType == ET_F32) ? "__cbrt_f32" : "__cbrt_f64", 
         CI->getType(),
         opr0->getType(),
         NULL)) {
      DEBUG(errs() << "AMDIC: " << *CI
                   << " ---> cbrt(" << *opr0 << ")\n");
      Value *nval = B.CreateCall(FPExpr, opr0, "__rootn2cbrt");
      replaceCall(nval);
      return true;
    }
  }
  return false;
}

bool AMDLibCalls::fold_fma_mad(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo)
{
  Value *opr0 = CI->getArgOperand(0);
  Value *opr1 = CI->getArgOperand(1);
  Value *opr2 = CI->getArgOperand(2);

  BLTIN_FUNC_ID FID = FInfo.FID;

  ConstantFP *CF0 = dyn_cast<ConstantFP>(opr0);
  ConstantFP *CF1 = dyn_cast<ConstantFP>(opr1);
  if ((CF0 && CF0->isZero()) || (CF1 && CF1->isZero())) {
    // fma/mad(a, b, c) = c if a=0 || b=0
    DEBUG(errs() << "AMDIC: " << *CI << " ---> " << *opr2 << "\n");
    replaceCall(opr2);
    return true;
  }
  if (CF0 && CF0->isExactlyValue(1.0f)) {
    // fma/mad(a, b, c) = b+c if a=1
    DEBUG(errs() << "AMDIC: " << *CI << " ---> " 
                 << *opr1 << " + " << *opr2 << "\n");
    Value *nval = B.CreateFAdd(opr1, opr2, "fmaadd");
    replaceCall(nval);
    return true;
  }
  if (CF1 && CF1->isExactlyValue(1.0f)) {
    // fma/mad(a, b, c) = a+c if b=1
    DEBUG(errs() << "AMDIC: " << *CI << " ---> " 
                 << *opr0 << " + " << *opr2 << "\n");
    Value *nval = B.CreateFAdd(opr0, opr2, "fmaadd");
    replaceCall(nval);
    return true;
  }
  if (ConstantFP *CF = dyn_cast<ConstantFP>(opr2)) {
    if (CF->isZero()) {
      // fma/mad(a, b, c) = a*b if c=0
      DEBUG(errs() << "AMDIC: " << *CI << " ---> " 
                   << *opr0 << " * " << *opr1 << "\n");
      Value *nval = B.CreateFMul(opr0, opr1, "fmamul");
      replaceCall(nval);
      return true;
    }
  }

  return false;
}

// Get a scalar native or HSAIL builtin signle argument FP function
Constant* AMDLibCalls::getNativeOrHsailFunction(Module* M, StringRef Name, ELEMENT_TYPE EType)
{
    std::string func;
    std::string base(Name);
    BLTIN_FUNC_KIND kind = FK_NATIVE;
    Triple::ArchType Arch = Triple(M->getTargetTriple()).getArch();
    if (Arch == Triple::hsail || Arch == Triple::hsail_64) {
      // These hsail native builtins have leading 'n'.
      // We may need to create a string map if there is a general use of this utility.
      base.insert(0, "n");
      kind = FK_HSAIL;
    }
    getFunctionName(func, base.c_str(), kind, EType, 1);
    Type* FTy;
    switch (EType) {
    case ET_F32:
      FTy = Type::getFloatTy(M->getContext());
      break;
    case ET_F64:
      FTy = Type::getDoubleTy(M->getContext());
      break;
    default:
      return NULL;
    }
    return getFunction(M, func, FTy, FTy, NULL);
}

// fold exp -> native_exp2 (x * LOG2E)
bool AMDLibCalls::fold_exp(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo)
{
  if ((FInfo.EType == ET_F32) && (FInfo.VectorSize == 1) &&
      (FInfo.FKind != FK_NATIVE)) {
    if (Constant *FPExpr = getNativeOrHsailFunction(
        CI->getParent()->getParent()->getParent(), "exp2", FInfo.EType)) {
      Value *opr0 = CI->getArgOperand(0);
      DEBUG(errs() << "AMDIC: " << *CI << " ---> "
                   << "exp2(" << *opr0 << " * M_LOG2E)\n");
      Value *nval1 = B.CreateFMul(opr0,
                                  ConstantFP::get(opr0->getType(), MATH_LOG2E),
                                  "__exp_arg");
      Value *nval  = B.CreateCall(FPExpr, nval1, "__exp");
      replaceCall(nval);
      return true;
    }
  }
  return false;
}

// fold exp2 -> native_exp2
bool AMDLibCalls::fold_exp2(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo)
{
  if ((FInfo.EType == ET_F32) && (FInfo.VectorSize == 1) &&
      (FInfo.FKind != FK_NATIVE) && (FInfo.FKind != FK_HSAIL)) {
    if (Constant *FPExpr = getNativeOrHsailFunction(
        CI->getParent()->getParent()->getParent(), "exp2", FInfo.EType)) {
      Value *opr0 = CI->getArgOperand(0);
      DEBUG(errs() << "AMDIC: " << *CI << " ---> "
                   << "exp2(" << *opr0 << ")\n");
      Value *nval  = B.CreateCall(FPExpr, opr0, "__exp2");
      replaceCall(nval);
      return true;
    }
  }
  return false;
}

// fold exp10 -> native_exp2 (x * M_LOG2_10)
bool AMDLibCalls::fold_exp10(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo)
{
  if ((FInfo.EType == ET_F32) && (FInfo.VectorSize == 1) &&
      (FInfo.FKind != FK_NATIVE)) {
    if (Constant *FPExpr = getNativeOrHsailFunction(
        CI->getParent()->getParent()->getParent(), "exp2", FInfo.EType)) {
      Value *opr0 = CI->getArgOperand(0);
      DEBUG(errs() << "AMDIC: " << *CI << " ---> "
                   << "exp2(" << *opr0 << " * M_LOG2_10\n");
      Value *nval1 = B.CreateFMul(opr0,
                                  ConstantFP::get(opr0->getType(), MATH_LOG2_10),
                                  "__exp10_arg");
      Value *nval  = B.CreateCall(FPExpr, nval1, "__exp10");
      replaceCall(nval);
      return true;
    }
  }
  return false;
}

// fold log -> native_log2 (x) * M_RLOG2_E
bool AMDLibCalls::fold_log(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo)
{
  if ((FInfo.EType == ET_F32) && (FInfo.VectorSize == 1) &&
      (FInfo.FKind != FK_NATIVE)) {
    if (Constant *FPExpr = getNativeOrHsailFunction(
        CI->getParent()->getParent()->getParent(), "log2", FInfo.EType)) {
      Value *opr0 = CI->getArgOperand(0);
      DEBUG(errs() << "AMDIC: " << *CI << " ---> "
                   << "log2(" << *opr0 << ") * M_RLOG2_E\n");
      Value *nval1 = B.CreateCall(FPExpr, opr0, "__log2");
      Value *nval  = B.CreateFMul(nval1,
                                  ConstantFP::get(opr0->getType(), MATH_RLOG2_E),
                                  "__log");
      replaceCall(nval);
      return true;
    }
  }
  return false;
}

// fold log2 -> native_log2
bool AMDLibCalls::fold_log2(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo)
{
  if ((FInfo.EType == ET_F32) && (FInfo.VectorSize == 1) &&
      (FInfo.FKind != FK_NATIVE) && (FInfo.FKind != FK_HSAIL)) {
    if (Constant *FPExpr = getNativeOrHsailFunction(
        CI->getParent()->getParent()->getParent(), "log2", FInfo.EType)) {
      Value *opr0 = CI->getArgOperand(0);
      DEBUG(errs() << "AMDIC: " << *CI << " ---> "
                   << "log2(" << *opr0 << ")\n");
      Value *nval  = B.CreateCall(FPExpr, opr0, "__log2");
      replaceCall(nval);
      return true;
    }
  }
  return false;
}

// fold log10 -> native_log2 (x) * M_RLOG2_10
bool AMDLibCalls::fold_log10(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo)
{
  if ((FInfo.EType == ET_F32) && (FInfo.VectorSize == 1) &&
      (FInfo.FKind != FK_NATIVE)) {
    if (Constant *FPExpr = getNativeOrHsailFunction(
        CI->getParent()->getParent()->getParent(), "log2", FInfo.EType)) {
      Value *opr0 = CI->getArgOperand(0);
      DEBUG(errs() << "AMDIC: " << *CI << " ---> "
                   << "log2(" << *opr0 << ") * M_RLOG2_10\n");
      Value *nval1 = B.CreateCall(FPExpr, opr0, "__log2");
      Value *nval  = B.CreateFMul(nval1,
                                  ConstantFP::get(opr0->getType(), MATH_RLOG2_10),
                                  "__log10");
      replaceCall(nval);
      return true;
    }
  }
  return false;
}

// fold sqrt -> native_sqrt (x)
bool AMDLibCalls::fold_sqrt(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo)
{
  if ((FInfo.EType == ET_F32 || FInfo.EType == ET_F64) &&
      (FInfo.VectorSize == 1) &&
      (FInfo.FKind != FK_NATIVE) && (FInfo.FKind != FK_HSAIL)) {
    if (Constant *FPExpr = getNativeOrHsailFunction(
        CI->getParent()->getParent()->getParent(), "sqrt", FInfo.EType)) {
      Value *opr0 = CI->getArgOperand(0);
      DEBUG(errs() << "AMDIC: " << *CI << " ---> "
                   << "sqrt(" << *opr0 << ")\n");
      Value *nval = B.CreateCall(FPExpr, opr0, "__sqrt");
      replaceCall(nval);
      return true;
    }
  }
  return false;
}

// fold sin, cos -> sincos.
bool AMDLibCalls::fold_sincos(CallInst *CI, IRBuilder<> &B, AliasAnalysis *AA)
{
  Value * CArgVal = CI->getArgOperand(0);
  BasicBlock * CBB = CI->getParent();
  int MaxScan = getMaxInstsToScan();

  // fold in load value.
  if (isa<LoadInst>(CArgVal)) {
    LoadInst * LI = dyn_cast<LoadInst>(CArgVal);
    if (LI->getParent() == CBB) {
      BasicBlock::iterator BBI = LI;
      if (Value * AvailableVal = llvm::FindAvailableLoadedValue(LI->getOperand(0), CBB, BBI, MaxScan, AA)) {
        CArgVal->replaceAllUsesWith(AvailableVal);
        if (CArgVal->getNumUses() == 0)
          LI->eraseFromParent();
        CArgVal = CI->getArgOperand(0);
      }
    }
  }

  Function * CCallee = CI->getCalledFunction();
  StringRef CName = CCallee->getName();
  std::string PairName = genSinOrCosName(CName);

  for (Value::use_iterator I = CArgVal->use_begin(), E = CArgVal->use_end();
    I != E; I++) {
    if (!isa<Instruction>(*I))
      continue;

    Instruction * User = cast<Instruction>(*I);
    CallInst * UI = dyn_cast_or_null<CallInst>(User);

    if (!UI || (UI == CI))
      continue;

    if (UI->getParent() != CBB)
      continue;

    Function * UCallee = UI->getCalledFunction();
    StringRef UName = UCallee->getName();
    if (!UName.equals(PairName))
      continue;

    BasicBlock::iterator BBI = CI;
    Instruction * Inst;
    int Count = MaxScan;
    while (BBI != CBB->begin()) {
      Inst = --BBI;
      if (Count-- == 0)
        break;
      if (Inst == UI) {
        std::string SinCosName = genSinCosName(UName);
        Function * Fsincos = getSinCosFunc(UI, SinCosName);
        if (!Fsincos)
          return false;

        DEBUG(errs() << "AMDIC: " << *CI << "\n");

        // Merge the sin and cos.
        BasicBlock::iterator ItOld = B.GetInsertPoint();
        AllocaInst * Alloc = insertAlloca(UI, B);
        B.SetInsertPoint(UI);
        CallInst * Call = B.CreateCall2(Fsincos, UI->getArgOperand(0), Alloc);
        Call->setCallingConv(Fsincos->getCallingConv());
        Instruction * Reload;
        Value * UArgVal = UI->getArgOperand(0);

        if (UName.startswith(getSinPrefix())) {
          B.SetInsertPoint(ItOld);
          UI->replaceAllUsesWith(Call);
          Reload = B.CreateLoad(Alloc);
          CI->replaceAllUsesWith(Reload);
          UI->eraseFromParent();
          CI->eraseFromParent();
        }
        else {
          Reload = B.CreateLoad(Alloc);
          UI->replaceAllUsesWith(Reload);
          CI->replaceAllUsesWith(Call);
          UI->eraseFromParent();
          CI->eraseFromParent();
          B.SetInsertPoint(ItOld);
        }

        return true;
      }
    }
  }

  return false;
}

// Generate name of the sincos function from the name of a sin or cos function.
std::string AMDLibCalls::genSinCosName(StringRef Name)
{
  std::pair<StringRef, StringRef> Pair;
  std::string SinPrefix = getSinPrefix();

  if (Name.startswith(SinPrefix))
    Pair = Name.split(SinPrefix);
  else
    Pair = Name.split(getCosPrefix());

  std::string SinCosName = getSinCosPrefix() + "p" + Pair.second.str();
  return SinCosName;
}

// Generate name of the sin or cos function from the name of a cos or sin function.
std::string AMDLibCalls::genSinOrCosName(StringRef Name)
{
  std::pair<StringRef, StringRef> Pair;
  std::string SinPrefix = getSinPrefix();

  if (Name.startswith(SinPrefix)) {
    Pair = Name.split(SinPrefix);
    return getCosPrefix() + Pair.second.str();
  }
  else {
    Pair = Name.split(getCosPrefix());
    return SinPrefix + Pair.second.str();
  }
}

// Get a sincos Function with the given name and whose type matches UI.
Function * AMDLibCalls::getSinCosFunc(CallInst * UI, std::string Name)
{
  Module * M = UI->getParent()->getParent()->getParent();
  Function * Fsincos = NULL;
  Function * UCallee = UI->getCalledFunction();
  Type * RetTy = UCallee->getReturnType();
  Type * ArgTy = UI->getArgOperand(0)->getType();
  Constant *Vsincos = getFunction(M, Name, 
                                  RetTy, ArgTy,
                                  ArgTy->getPointerTo(), NULL);
  Fsincos = Vsincos ? dyn_cast_or_null<Function>(Vsincos) : NULL;
  if (Fsincos) {
    Fsincos->removeFnAttr(Attributes::get(Fsincos->getContext(),Attributes::ReadOnly));
  }
  return Fsincos;
}

// Get insertion point at entry.
BasicBlock::iterator AMDLibCalls::getEntryIns(CallInst * UI) {
  Function * Func = UI->getParent()->getParent();
  BasicBlock * BB = &Func->getEntryBlock();
  assert(BB && "Entry block not found!");
  BasicBlock::iterator ItNew = BB->begin();
  assert(ItNew && "Entry instruction not found!");
  return ItNew;
}

// Insert a AllocsInst at the beginning of function entry block.
AllocaInst * AMDLibCalls::insertAlloca(CallInst * UI, IRBuilder<> &B)
{
  BasicBlock::iterator ItNew = getEntryIns(UI);
  Function * UCallee = UI->getCalledFunction();
  Type * RetType = UCallee->getReturnType();
  B.SetInsertPoint(ItNew);
  AllocaInst * Alloc = B.CreateAlloca(RetType, 0,
    getSinCosPrefix() + UI->getName());
  Alloc->setAlignment(RetType->getPrimitiveSizeInBits()/8);
  return Alloc;
}

bool AMDLibCalls::evaluateScalarMathFunc(FuncInfo &FInfo,
  double& Res0, double& Res1,
  Constant *copr0, Constant *copr1, Constant *copr2)
{
  // By default, opr0/opr1/opr3 holds values of float/doble type.
  // If they are not float/double, each function has to its
  // operand separately.
  double opr0=0.0, opr1=0.0, opr2=0.0;
  ConstantFP *fpopr0 = dyn_cast_or_null<ConstantFP>(copr0);
  ConstantFP *fpopr1 = dyn_cast_or_null<ConstantFP>(copr1);
  ConstantFP *fpopr2 = dyn_cast_or_null<ConstantFP>(copr2);
  if (fpopr0) {
    opr0 = (FInfo.EType == ET_F64)
             ? fpopr0->getValueAPF().convertToDouble()
             : (double)fpopr0->getValueAPF().convertToFloat();
  }

  if (fpopr1) {
    opr1 = (FInfo.EType == ET_F64)
             ? fpopr1->getValueAPF().convertToDouble()
             : (double)fpopr1->getValueAPF().convertToFloat();
  }

  if (fpopr2) {
    opr2 = (FInfo.EType == ET_F64)
             ? fpopr2->getValueAPF().convertToDouble()
             : (double)fpopr2->getValueAPF().convertToFloat();
  }

  BLTIN_FUNC_ID FID = FInfo.FID;
  switch (FInfo.FID) {
  default : return false;

  case BFID_acos:
    Res0 = acos(opr0);
    return true;

  case BFID_acosh:
    // acosh(x) == log(x + sqrt(x*x - 1))
    Res0 = log(opr0 + sqrt(opr0*opr0 - 1.0));
    return true;

  case BFID_acospi:
    Res0 = acos(opr0) / MATH_PI;
    return true;

  case BFID_asin:
    Res0 = asin(opr0);
    return true;

  case BFID_asinh:
    // asinh(x) == log(x + sqrt(x*x + 1))
    Res0 = log(opr0 + sqrt(opr0*opr0 + 1.0));
    return true;

  case BFID_asinpi:
    Res0 = asin(opr0) / MATH_PI;
    return true;

  case BFID_atan:
    Res0 = atan(opr0);
    return true;

  case BFID_atanh:
    // atanh(x) == (log(x+1) - log(x-1))/2;
    Res0 = (log(opr0 + 1.0) - log(opr0 - 1.0))/2.0;
    return true;

  case BFID_atanpi:
    Res0 = atan(opr0) / MATH_PI;
    return true;

  case BFID_cbrt:
    Res0 = (opr0 < 0.0) ? -pow(-opr0, 1.0/3.0) : pow(opr0, 1.0/3.0);
    return true;

  case BFID_cos:
    Res0 = cos(opr0);
    return true;

  case BFID_cosh:
    Res0 = cosh(opr0);
    return true;

  case BFID_cospi:
    Res0 = cos(MATH_PI * opr0);
    return true;

  case BFID_exp:
    Res0 = exp(opr0);
    return true;

  case BFID_exp2:
    Res0 = pow(2.0, opr0);
    return true;

  case BFID_exp10:
    Res0 = pow(10.0, opr0);
    return true;

  case BFID_expm1:
    Res0 = exp(opr0) - 1.0;
    return true;

  case BFID_log:
    Res0 = log(opr0);
    return true;

  case BFID_log2:
    Res0 = log(opr0) / log(2.0);
    return true;

  case BFID_log10:
    Res0 = log(opr0) / log(10.0);
    return true;

  case BFID_rsqrt:
    Res0 = 1.0 / sqrt(opr0);
    return true;

  case BFID_sin:
    Res0 = sin(opr0);
    return true;

  case BFID_sinh:
    Res0 = sinh(opr0);
    return true;

  case BFID_sinpi:
    Res0 = sin(MATH_PI * opr0);
    return true;

  case BFID_sqrt:
    Res0 = sqrt(opr0);
    return true;

  case BFID_tan:
    Res0 = tan(opr0);
    return true;

  case BFID_tanh:
    Res0 = tanh(opr0);
    return true;

  case BFID_tanpi:
    Res0 = tan(MATH_PI * opr0);
    return true;

  case BFID_recip:
    Res0 = 1.0 / opr0;
    return true;

  // two-arg functions
  case BFID_divide:
    Res0 = opr0 / opr1;
    return true;

  case BFID_pow:
  case BFID_powr:
    Res0 = pow(opr0, opr1);
    return true;

  case BFID_pown:
    {
      if (ConstantInt *iopr1 = dyn_cast_or_null<ConstantInt>(copr1)) {
        double val = (double)iopr1->getSExtValue();
        Res0 = pow(opr0, val);
        return true;
      }
      return false;
    }
      
  case BFID_rootn:
    {
      if (ConstantInt *iopr1 = dyn_cast_or_null<ConstantInt>(copr1)) {
        double val = (double)iopr1->getSExtValue();
        Res0 = pow(opr0, 1.0 / val);
        return true;
      }
      return false;
    }

  // with ptr arg
  case BFID_sincos:
    Res0 = sin(opr0);
    Res1 = cos(opr0);
    return true;
  
  // three-arg functions
  case BFID_fma:
  case BFID_mad:
    Res0 = opr0 * opr1 + opr2;
    return true;
  }

  return false;
}

bool AMDLibCalls::evaluateCall(CallInst *aCI, FuncInfo &FInfo)
{
  BLTIN_FUNC_ID FID = FInfo.FID;
  // Ignore function with ptr arg except sincos.
  if (FID != BFID_sincos &&
      (BltinFuncDesc[FInfo.FID].flags & BLTINDESC_HasPtrArg))
    return false;

  int numArgs = (int)aCI->getNumArgOperands();
  if (numArgs > 3)
    return false;

  Constant *copr0 = NULL;
  Constant *copr1 = NULL;
  Constant *copr2 = NULL;
  if (numArgs > 0) {
    if ((copr0 = dyn_cast<Constant>(aCI->getArgOperand(0))) == NULL)
      return false;
  }

  if (numArgs > 1) {
    if ((copr1 = dyn_cast<Constant>(aCI->getArgOperand(1))) == NULL) {
      if (FID != BFID_sincos)
        return false;
    }
  }

  if (numArgs > 2) {
    if ((copr2 = dyn_cast<Constant>(aCI->getArgOperand(2))) == NULL)
      return false;
  }

  // At this point, all arguments to aCI are constants.

  // max vector size is 16, and sincos will generate two results.
  double DVal0[16], DVal1[16];
  bool hasTwoResults = (FID == BFID_sincos);
  if (FInfo.VectorSize == 1) {
    if (!evaluateScalarMathFunc(FInfo, DVal0[0],
                                DVal1[0], copr0, copr1, copr2)) {
      return false;
    }
  } else {
    ConstantDataVector *CDV0 = dyn_cast_or_null<ConstantDataVector>(copr0);
    ConstantDataVector *CDV1 = dyn_cast_or_null<ConstantDataVector>(copr1);
    ConstantDataVector *CDV2 = dyn_cast_or_null<ConstantDataVector>(copr2);
    for (int i=0; i < FInfo.VectorSize; ++i) {
      Constant *celt0 = CDV0 ? CDV0->getElementAsConstant(i) : NULL;
      Constant *celt1 = CDV1 ? CDV1->getElementAsConstant(i) : NULL;
      Constant *celt2 = CDV2 ? CDV2->getElementAsConstant(i) : NULL;
      if (!evaluateScalarMathFunc(FInfo, DVal0[i],
                                  DVal1[i], celt0, celt1, celt2)) {
        return false;
      }
    }
  }

  LLVMContext &context = CI->getParent()->getParent()->getContext();
  Constant *nval0, *nval1;
  if (FInfo.VectorSize == 1) {
    nval0 = ConstantFP::get(CI->getType(), DVal0[0]);
    if (hasTwoResults)
      nval1 = ConstantFP::get(CI->getType(), DVal1[0]);
  } else {
    if (FInfo.EType == ET_F32) {
      SmallVector <float, 0> FVal0, FVal1;
      for (int i=0; i < FInfo.VectorSize; ++i)
        FVal0.push_back((float)DVal0[i]);
      ArrayRef<float> tmp0(FVal0);
      nval0 = ConstantDataVector::get(context, tmp0);
      if (hasTwoResults) {
        for (int i=0; i < FInfo.VectorSize; ++i)
          FVal1.push_back((float)DVal1[i]);
        ArrayRef<float> tmp1(FVal1);
        nval1 = ConstantDataVector::get(context, tmp1);
      }
    } else {
      ArrayRef<double> tmp0(DVal0);
      nval0 = ConstantDataVector::get(context, tmp0);
      if (hasTwoResults) {
        ArrayRef<double> tmp1(DVal1);
        nval1 = ConstantDataVector::get(context, tmp1);
      }
    }
  }

  if (hasTwoResults) {
    // sincos
    assert(FID == BFID_sincos &&
           "math function with ptr arg not supported yet");
    new StoreInst(nval1, aCI->getArgOperand(1), aCI);
  }

  replaceCall(nval0);
  return true;
}
