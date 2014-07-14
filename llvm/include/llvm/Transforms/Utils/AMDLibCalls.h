//
// Copyright (c) 2011 Advanced Micro Devices, Inc. All rights reserved.
//

// This file has code to do AMD built-in library optimizations. Mostly, it
// does built-in math library folding.
// 

#ifndef TRANSFORMS_UTILS_AMDLIBCALLS_H
#define TRANSFORMS_UTILS_AMDLIBCALLS_H

#include "llvm/IRBuilder.h"
#include "llvm/Support/Mutex.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Function.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/Loads.h"
#include "llvm/AMDIntrinsicInfo.h"
#include <map>
#include <string>
#include <cstdarg>

namespace llvm {

#if 0
#define _QUOTE(x)	#x
#define STRVAL(x)	_QUOTE(x)

/*
   MATHFUNC1() is not a complete list of all functions. It contains only
   1-input-argument ones that will be considered for table-driven optimization.

   Note that acos MUST be the first one and tgamma MUST be the last one !!!
*/
#define MATHFUNC1( bfmacro ) \
bfmacro(acos) \
bfmacro(acosh) \
bfmacro(acospi) \
bfmacro(asin) \
bfmacro(asinh) \
bfmacro(asinpi) \
bfmacro(atan) \
bfmacro(atanh) \
bfmacro(atanpi) \
bfmacro(cbrt) \
bfmacro(cos) \
bfmacro(cosh) \
bfmacro(cospi) \
bfmacro(erfc) \
bfmacro(erf) \
bfmacro(exp) \
bfmacro(exp2) \
bfmacro(exp10) \
bfmacro(expm1) \
bfmacro(log) \
bfmacro(log2) \
bfmacro(log10) \
bfmacro(rsqrt) \
bfmacro(sqrt) \
bfmacro(tgamma)


#define BUILTIN_FUNCS( bfmacro ) \
MATHFUNC1(bfmacro) \
\
bfmacro(pow) \
bfmacro(powr) \
bfmacro(pown) \
bfmacro(rootn) \
\
bfmacro(fma) \
bfmacro(mad)

#define BFIDSTR(x) BFID_##x ,
#define MAPENTRY(x) MFName2IDMap[ #x ] = BFID_##x ;

#endif

#define MAX_FNAME_SIZE 64

class Module;
class Value;
class Constant;
class DataLayout;

typedef  StringMap<int> Str2IntMap;
  
class AMDLibCalls {
public:
  /*
    IDs will start from 1-arg functions, then 2-arg functions,
    etc. in order.

    For 1-arg functions, all table-driven functions are listed 
    together.
   */
  enum BLTIN_FUNC_ID {
    BFID_undefined = 0, // ID starts from 1

    /* 1-input functions */
    // Begin of functions for table-driven optimizations
    BFID_acos,
    BFID_acosh,
    BFID_acospi,
    BFID_asin,
    BFID_asinh,
    BFID_asinpi,
    BFID_atan,
    BFID_atanh,
    BFID_atanpi,
    BFID_cbrt,
    BFID_cos,
    BFID_cosh,
    BFID_cospi,
    BFID_erfc,
    BFID_erf,
    BFID_exp,
    BFID_exp2,
    BFID_exp10,
    BFID_expm1,
    BFID_log,
    BFID_log2,
    BFID_log10,
    BFID_rsqrt,
    BFID_sin,
    BFID_sinh,
    BFID_sinpi,
    BFID_sqrt,
    BFID_tan,
    BFID_tanh,
    BFID_tanpi,
    BFID_ncos,
    BFID_nexp2,
    BFID_nlog2,
    BFID_nrsqrt,
    BFID_nsin,
    BFID_nsqrt,
    BFID_tgamma,
    /* End of functions for table-driven optimizations */

    BFID_recip,

    /* 2-arg functions */
    BFID_divide,
    BFID_pow,
    BFID_powr,
    BFID_pown,
    BFID_rootn,
    // with ptr argument
    BFID_sincos,

    /* 3-arg functions */
    BFID_fma,
    BFID_mad,
    BFID_nfma,


    // End of functions
    BFID_LENGTH,

    // For Table-Driven Optimization
    BFID_TDO_FIRST = BFID_acos,   // inclusive
    BFID_TDO_LAST  = BFID_tgamma, // inclusive

    BFID_1IN_START = BFID_acos,   // inclusive
    BFID_2IN_START = BFID_divide, // inclusive
    BFID_3IN_START = BFID_fma,    // inclusive

    // Define the begin and the last (inclusive)
    BFID_FIRST = 1,
    BFID_LAST  = BFID_LENGTH - 1
  };

  // Builtin Math library Descriptor.
  // Definition for flags's bit;
  enum {
    BLTINDESC_HasNormal = 0x1,
    BLTINDESC_HasNative = 0x2,
    BLTINDESC_HasHalf   = 0x4,
    BLTINDESC_HasPtrArg = 0x8,
    BLTINDESC_HsailOnly = 0x10  // builtins existing in HSAIL only
  };

  struct BltinLibDescriptor {
    unsigned flags;
  };

  struct FuncInfo : AMDIntrinsicInfo {
    BLTIN_FUNC_ID FID;
  };
   

  /* 
     Data structures for table-driven optimizations.
     FuncTbl works for both f32 and f64 functions with 1 input argument
   */
  struct FuncTbl {
    double   result;
    double   input;
  };

  struct TDO_Entry {
    /* int ID; */
    int opttbl_size;
    struct FuncTbl *opttbl; // variable size: from 0 to (opttbl_size - 1)
  };

private:
  static bool isInitialized;

  static sys::SmartMutex<true> LibCallLock;

  static TDO_Entry OptimTbl[BFID_TDO_LAST+1];
  static Str2IntMap FName2IDMap;
  // static const char *ID2FName[BFID_LENGTH];
  static BltinLibDescriptor BltinFuncDesc[BFID_LENGTH];

  // -fuse-native.
  Str2IntMap UseNativeFuncs;
  bool AllNative;

  // Caching options
  bool UnsafeMathOpt;
  bool NoSignedZeros;
  bool FiniteMathOnly;
  bool FastRelaxedMath;

  void init();

  // Return a pointer (pointer expr) to the function if function defintion with
  // "FuncName" exists. It may creates a new function prototype based on return
  // type "RetTy" and parameter types given in va arg list. The va arg list ends
  // with a '0' (NULL) arg.
  Constant* getFunction(Module *M, StringRef FuncName, const AMDIntrinsicInfo& fInfo, Type *RetTy, ...);
  Constant* getFunction(Module *M, FunctionType *FuncTy, StringRef FuncName, const AMDIntrinsicInfo& fInfo);

  std::string getFunctionName(const char* GFName, 
                       AMDIntrinsicInfo::EKind kind, Type::TypeID type, int VSize);

  unsigned int getNumOfArgs(BLTIN_FUNC_ID FID) {
    return (FID < BFID_2IN_START)
             ? 1 : ((FID < BFID_3IN_START) ? 2 : 3);
  }

  // Replace a normal function with its native version.
  bool replaceWithNative(CallInst *CI, const StringRef& bareName, const FuncInfo &FInfo);

  StringRef parseFunctionName(const StringRef& FMangledName, FuncInfo *FInfo=NULL /*out*/);

  bool TDOFold(CallInst *CI, const DataLayout *DL, FuncInfo &FInfo);

  /* Specialized optimizations */

  // recip (half or native)
  bool fold_recip(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo);

  // divide (half or native)
  bool fold_divide(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo);

  // pow/powr/pown
  bool fold_pow(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo);

  // rootn
  bool fold_rootn(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo);

  // fma/mad
  bool fold_fma_mad(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo);

  // -fuse-native for sincos
  bool sincosUseNative(CallInst *aCI, const FuncInfo &FInfo);

  // evaluate calls if calls' arguments are constants.
  bool evaluateScalarMathFunc(FuncInfo &FInfo, double& Res0,
    double& Res1, Constant *copr0, Constant *copr1, Constant *copr2);
  bool evaluateCall(CallInst *aCI, FuncInfo &FInfo);

  // exp
  bool fold_exp(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo);

  // exp2
  bool fold_exp2(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo);

  // exp10
  bool fold_exp10(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo);

  // log
  bool fold_log(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo);

  // log2
  bool fold_log2(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo);

  // log10
  bool fold_log10(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo);

  // sqrt
  bool fold_sqrt(CallInst *CI, IRBuilder<> &B, FuncInfo &FInfo);

  // sin/cos
  bool fold_sincos(CallInst * CI, IRBuilder<> &B, AliasAnalysis * AA);

private:
  // Get a sincos function.
  Function * getSinCosFunc(CallInst * UI, const AMDIntrinsicInfo& fInfo, unsigned AS);
  // Get insertion point at entry.
  BasicBlock::iterator getEntryIns(CallInst * UI);
  // Maximum instruction to scan.
  int getMaxInstsToScan() { return 30; }
  // Insert an Alloc instruction.
  AllocaInst * insertAlloca(CallInst * UI, IRBuilder<> &B, const char *prefix);
  // Get a scalar native or HSAIL builtin signle argument FP function
  Constant* getNativeOrHsailFunction(Module* M, StringRef Name, Type::TypeID EType);

protected:
  CallInst *CI;
  virtual void replaceCall(Value *With) = 0;

public:
  AMDLibCalls();
  virtual ~AMDLibCalls();
  bool fold(CallInst *CI, const DataLayout *DL, AliasAnalysis *AA = NULL);

  void setFuncNames(const char *V);
  void enableUnsafeMathOpt() { UnsafeMathOpt = true; }
  void enableNoSignedZeros() { NoSignedZeros = true; }
  void enableFiniteMathOnly() { FiniteMathOnly = true; }
  void enableFastRelaxedMath() { FastRelaxedMath = true; }

  // Replace a normal math function call with that native version
  bool useNative(CallInst *CI, const DataLayout *DL);
};

}

#endif
