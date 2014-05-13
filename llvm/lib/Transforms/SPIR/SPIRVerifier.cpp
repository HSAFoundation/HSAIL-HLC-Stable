//===-- SpirVerifier.cpp - Implement the SPIR Verifier -----------*- C++ -*-==//
//
// Copyright (c) 2012 The Khronos Group Inc.  All rights reserved.
//
// NOTICE TO KHRONOS MEMBER:
//
// AMD has assigned the copyright for this object code to Khronos.
// This object code is subject to Khronos ownership rights under U.S. and
// international Copyright laws.
//
// Permission is hereby granted, free of charge, to any Khronos Member
// obtaining a copy of this software and/or associated documentation files
// (the "Materials"), to use, copy, modify and merge the Materials in object
// form only and to publish, distribute and/or sell copies of the Materials
// solely in object code form as part of conformant OpenCL API implementations,
// subject to the following conditions:
//
// Khronos Members shall ensure that their respective ICD implementation,
// that is installed over another Khronos Members' ICD implementation, will
// continue to support all OpenCL devices (hardware and software) supported
// by the replaced ICD implementation. For the purposes of this notice, "ICD"
// shall mean a library that presents an implementation of the OpenCL API for
// the purpose routing API calls to different vendor implementation.
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Materials.
//
// KHRONOS AND AMD MAKE NO REPRESENTATION ABOUT THE SUITABILITY OF THIS
// SOURCE CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
// IMPLIED WARRANTY OF ANY KIND.  KHRONOS AND AMD DISCLAIM ALL WARRANTIES
// WITH REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
// IN NO EVENT SHALL KHRONOS OR AMD BE LIABLE FOR ANY SPECIAL, INDIRECT,
// INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
// FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
// NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH
// THE USE OR PERFORMANCE OF THIS SOURCE CODE.
//
// U.S. Government End Users.   This source code is a "commercial item" as
// that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting of
// "commercial computer software" and "commercial computer software
// documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995)
// and is provided to the U.S. Government only as a commercial end item.
// Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
// 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
// source code with only those rights set forth herein.
//
//===----------------------------------------------------------------------===//
//
// This file defines the function verifier interface, that can be used for some
// sanity checking of input SPIR to the system.
//
// This does not provide LLVM style verification. It instead assumes that the
// LLVM verifier has already been run and the IR is well formed.
//
// In lightweight mode:
//    *All extensions in the spir.extensions metadata are supported by the
//     vendor and device combination.
//    *Optional core features specified in the spir.used.optional.core.features
//     are valid for the vendor and device combination.
//    *The SPIR version is valid for the vendor.
//    *The OpenCL version is valid for the specific SPIR version.
//    *The target triple is the SPIR target triple.
//    *The target data layout matches the SPIR data layout.
//
// In heavyweight mode:
//    *The lightweight mode checks are executed.
//    *Only the data types that are specified by the SPIR specification are
//     used.
//    *Only the spir builtins specified in the SPIR specification are used.
//    *Only the instructions specified in the SPIR specification are used.
//    *Only alwaysinline, inlinehint, noinline, nounwind, readnone, readonly
//     are specified as function attributes.
//    *Only zeroext, signext, byval, sret and nocapture parameter attributes
//     are used.
//    *No visibility types other than 'default' is used.
//    *Only private, available_externally, internal and external linkage modes
//     are used.
//    *All functions are marked with spirkrnl or spirfnc calling conventions.
//    *All intrinsics begin with @llvm.spir or @spir.
//    *All functions have the function attribute nounwind.
//    *No inline assembly is used.
//    *Only sampler values as specified in Table14 of the spec are used.
//    *Only __spir_get_null_ptrN are used to assign NULL to a pointer.
//    *All alignments comply with the OpenCL spec.
//    *Only address spaces 0 through 5 are valid.
//    *All functions in the spir.kernels metdata have the spirkrnl calling
//     convention.
//    *All functions marked with spirkrnl calling convention exist in the
//     spir.kernels metadata.
//    *Work group size hint metadata must have 3 integer values.
//    *Reqd work group size metadata must have 3 integer values.
//    *vector type hint metadata can only have the string types in section
//     2.4.2.2 of the SPIR spec.
//      *It can only be attached to non-kernel functions.
//      *double[N] types are only valid if cl_double is specified as an optional
//       core feature.
//    *Kernel arg metadata only contains values specified in section 2.5.
//    *Only constant, no alias and volatile are valid type qualifiers.
//    *Only compiler options as specified in the SPIR spec are valid.
//    *All functions are properly name mangled.
//    *Only the size_t functions in the SPIR spec are used and are valid.
//    *All load/store operations have an alignment valid.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/SPIRVerifier.h"
#include "llvm/CallingConv.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/GlobalValue.h"
#include "llvm/Intrinsics.h"
#include "llvm/InlineAsm.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Metadata.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstVisitor.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/DataLayout.h"
#include <algorithm>
#include <cstdarg>
#include "cxxabi.h"
#include "khrext.h"
using namespace llvm;

static bool isOpaqueNamedStruct(const Type *t, StringRef name);
static bool isAddressSpace(const Type*, unsigned);
namespace SPIR {
  // SPIR Address space enumerations. See 2.2 of the SPIR spec for details.
  enum _spiras_enum {
    SPIRAS_PRIVATE  = 0, // OpenCL Private address space
    SPIRAS_GLOBAL   = 1, // OpenCL Global address space
    SPIRAS_CONSTANT = 2, // OpenCL Constant address space
    SPIRAS_LOCAL    = 3, // OpenCL Local address space
    SPIRAS_GLOBAL_HOST = 4, // OpenCL Global address space
    // with endian(host) attribute
    SPIRAS_CONSTANT_HOST = 5 // OpenCL Constant address space
      // with endian(host) attribute
  } SPIRAS;

  enum _error_codes_enum {
    InvalidType,
    RequiresDouble,
    RequiresImages,
    InvalidVectorSize,
    InvalidAddressSpace,
    BooleanVector,
    InvalidInstr,
    NoAtomicLoadStore,
    UnalignedLoadStore,
    VectorConversion,
    InvalidPtrInst,
    NoTailCall,
    BadCallingConv,
    NoNounwind,
    InvalidFuncAttribute,
    InvalidArgAttribute,
    InlineAsm,
    IndirectCall,
    InvalidTriple,
    InvalidDataLayout,
    InvalidSPIRVersion,
    InvalidOCLVersion,
    InvalidOptionalCore,
    InvalidKHRExtension,
    UnsupportedSPIRVersion,
    UnsupportedOCLVersion,
    UnsupportedOptionalCore,
    UnsupportedKHRExtension,
    InvalidLinkage,
    InvalidVisibility,
    InvalidVarArg,
    InvalidGarbageCollection,
    InvalidNameMangling,
    InvalidIntrRet,
    InvalidIntrArg,
    InvalidIntrFunc,
    IllegalAShrExact,
    InvalidCompilerOpts,
    InvalidExtCompilerOpts,
    InvalidSamplerMD,
    InvalidFunctionMD,
    InvalidSPIRFunction,
    InvalidCompilerOptsMD,
    InvalidExtCompilerOptsMD,
    InvalidSPIRVersionMD,
    InvalidOCLVersionMD,
    InvalidOptionalCoreMD,
    InvalidKHRExtensionMD,
    InvalidNamedMD,
    InvalidPtrBitcast,
    InvalidClStdOpt,
    InvalidMangling,
    InvalidReservedName
  } ErrorCodes;


  struct SPIRVerifier : public FunctionPass,
  public InstVisitor<SPIRVerifier> {
    static char ID; // Pass ID, replacement for typeid
    int Bitness;          // Is this module 32 bit or 64 bit?
    bool Broken;          // Is this module found to be broken?
    bool RealPass;        // Are we not being run by a PassManager?
    bool LightWeight;     // Are we running the lightweight pass?
    bool HasDouble;       // Do we support doubles?
    bool HasImages;       // Do we support images?
    bool HasHalf;         // Do we support half type?
    VerifierFailureAction action;
    // What to do if verification fails.
    Module *Mod;          // Module we are verifying right now
    LLVMContext *Context; // Context within which we are verifying
    std::string OptCore;  // List of space seperated support optional core features.
    std::string KhrExt;   // List of space seperated support KHR extensions.
    unsigned SPIRVer[2];  // SPIR version information.
    unsigned OCLVer[2];   // OpenCL version information.

    std::string Messages;
    raw_string_ostream MessagesStr;

    SPIRVerifier()
      : FunctionPass(ID), Broken(false), RealPass(true), LightWeight(true),
      HasDouble(false), HasImages(false), HasHalf(false),
      action(AbortProcessAction), Mod(0), Context(0),
      MessagesStr(Messages) {
        initializeSPIRVerifierPass(*PassRegistry::getPassRegistry());
      }
    explicit SPIRVerifier(VerifierFailureAction ctn, bool lw)
      : FunctionPass(ID), Broken(false), RealPass(true), LightWeight(lw),
      HasDouble(false), HasImages(false), HasHalf(false),
      action(ctn), Mod(0), Context(0), MessagesStr(Messages) {
        initializeSPIRVerifierPass(*PassRegistry::getPassRegistry());
      }

    bool doInitialization(Module &M)
    {
      Mod = &M;
      Context = &M.getContext();
      visitSPIRHeader(M);

      // If this is a real pass, in a pass manager, we must abort before
      // returning back to the pass manager, or else the pass manager may try to
      // run other passes on the broken module.
      if (RealPass)
        return abortIfBroken();
      return false;
    }

    bool runOnFunction(Function &F)
    {
      Mod = F.getParent();
      if (!Context) Context = &F.getContext();

      if (!LightWeight) {
        visit(F);
      }

      // If this is a real pass, in a pass manager, we must abort before
      // returning back to the pass manager, or else the pass manager may try to
      // run other passes on the broken module.
      if (RealPass)
        return abortIfBroken();

      return false;
    }

    bool doFinalization(Module &M)
    {
      // Scan through, checking all of the external function's linkage now...
      for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
        visitGlobalValue(*I);

        // Check to make sure function prototypes are okay.
        if (I->isDeclaration()) visitFunction(*I);
      }

      for (Module::named_metadata_iterator I = M.named_metadata_begin(),
          E = M.named_metadata_end(); I != E; ++I)
        visitNamedMDNode(*I);

      // If the module is broken, abort at this time.
      return abortIfBroken();
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const
    {
      AU.setPreservesAll();
    }

    virtual const char *getPassName() const
    {
      return "Verification pass";
    }

    /// abortIfBroken - If the module is broken and we are supposed to abort on
    /// this condition, do so.
    ///
    bool abortIfBroken()
    {
      if (!Broken) return false;
      MessagesStr << "Broken SPIR module found, ";
      switch (action) {
        default: llvm_unreachable("Unknown action");
        case AbortProcessAction:
                 MessagesStr << "compilation aborted!\n";
                 dbgs() << MessagesStr.str();
                 // Client should choose different reaction if abort is not desired
                 abort();
        case PrintMessageAction:
                 MessagesStr << "verification continues.\n";
                 dbgs() << MessagesStr.str();
                 return false;
        case ReturnStatusAction:
                 MessagesStr << "compilation terminated.\n";
                 return true;
      }
    }


    void visitSPIRHeader(Module &M);
    // Verification methods...
    void visitGlobalValue(GlobalValue &GV);
    void visitFunction(Function &F);
    void visitNamedMDNode(NamedMDNode &NMD);
    using InstVisitor<SPIRVerifier>::visit;

    void visit(Instruction &I);
    void visitInstruction(Instruction &I);

    bool isValidType(const Type *t);
    bool isValidType(const VectorType *t);
    bool isValidType(const ArrayType *t);
    bool isValidType(const StructType *t);
    bool isValidType(const PointerType *t);
    bool isValidType(const FunctionType *t);
    bool isTypeAligned(const Type* t, unsigned align);
    bool isInvalidSPIRBuiltin(const Function* func);
    bool isInvalidMangling(const Function* func);

    template <class T>
      bool verifyOperands(Instruction &I);
    const char *ErrorMessages(unsigned err_code);

    void WriteInst(const Instruction *V)
    {
      if (!V) return;
      MessagesStr << *V << '\n';
    }

    void WriteValue(const Value *V)
    {
      if (!V) return;
      if (isa<Instruction>(V)) {
        MessagesStr << *V << '\n';
      } else {
        WriteAsOperand(MessagesStr, V, true, Mod);
        MessagesStr << '\n';
      }
    }

    void WriteType(Type *T)
    {
      if (!T) return;
      MessagesStr << ' ' << *T;
    }

    void WriteType(const Type *T)
    {
      if (!T) return;
      WriteType(const_cast<Type*>(T));
    }


    // CheckFailed - A check failed, so print out the condition and the message
    // that failed.  This provides a nice place to put a breakpoint if you want
    // to see why something is not correct.
    void CheckFailed(const Twine &Message,
        const Twine &str)
    {
      MessagesStr << Message.str() << "\n";
      MessagesStr << str.str() << "\n";
      Broken = true;
    }

    void CheckFailed(const Twine &Message,
        const Instruction *V1 = 0,
        const Instruction *V2 = 0,
        const Instruction *V3 = 0,
        const Instruction *V4 = 0)
    {
      MessagesStr << Message.str() << "\n";
      WriteInst(V1);
      WriteInst(V2);
      WriteInst(V3);
      WriteInst(V4);
      Broken = true;
    }
    void CheckFailed(const Twine &Message,
        const Type *V1 = 0, const Type *V2 = 0,
        const Type *V3 = 0, const Type *V4 = 0)
    {
      MessagesStr << Message.str() << "\n";
      WriteType(V1);
      WriteType(V2);
      WriteType(V3);
      WriteType(V4);
      Broken = true;
    }

    void CheckFailed(const Twine &Message,
        const Value *V1 = 0, const Value *V2 = 0,
        const Value *V3 = 0, const Value *V4 = 0)
    {
      MessagesStr << Message.str() << "\n";
      WriteValue(V1);
      WriteValue(V2);
      WriteValue(V3);
      WriteValue(V4);
      Broken = true;
    }

    void CheckFailed(const Twine &Message, const Value *V1,
        Type *T2, const Value *V3 = 0)
    {
      MessagesStr << Message.str() << "\n";
      WriteValue(V1);
      WriteType(T2);
      WriteValue(V3);
      Broken = true;
    }

    void CheckFailed(const Twine &Message, Type *T1,
        Type *T2 = 0, Type *T3 = 0)
    {
      MessagesStr << Message.str() << "\n";
      WriteType(T1);
      WriteType(T2);
      WriteType(T3);
      Broken = true;
    }
    static const std::string SPIR_DATA_LAYOUT_32BIT;
    static const std::string SPIR_DATA_LAYOUT_64BIT;
    private:
    void visitOptionalCoreMD(NamedMDNode*);
    void visitUsedExtensionsMD(NamedMDNode*);
    void visitSPIRVersionMD(NamedMDNode*);
    void visitOCLVersionMD(NamedMDNode*);
    void visitCompilerExtOptsMD(NamedMDNode*);
    void visitCompilerOptsMD(NamedMDNode*);
    void visitFunctionsMD(NamedMDNode*);
  };
} // End SPIR namespace

using namespace SPIR;
const std::string SPIRVerifier::SPIR_DATA_LAYOUT_32BIT =
"p:32:32:32-i1:8:8-i8:8:8"
"-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16"
"-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128"
"-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024";
const std::string SPIRVerifier::SPIR_DATA_LAYOUT_64BIT =
"p:64:64:64-i1:8:8-i8:8:8"
"-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16"
"-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128"
"-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024";

char SPIRVerifier::ID = 0;
INITIALIZE_PASS_BEGIN(SPIRVerifier, "spirverify", "SPIR Module Verifier", false, false)
INITIALIZE_PASS_END(SPIRVerifier, "spirverify", "SPIR Module Verifier", false, false)

// Assert - We know that cond should be true, if not print an error message.
#define Assert(C, M) \
  do { if (!(C)) { CheckFailed(M); return; } } while (0)
#define Assert1(C, M, V1) \
  do { if (!(C)) { CheckFailed(M, V1); return; } } while (0)
#define Assert2(C, M, V1, V2) \
  do { if (!(C)) { CheckFailed(M, V1, V2); return; } } while (0)
#define Assert3(C, M, V1, V2, V3) \
  do { if (!(C)) { CheckFailed(M, V1, V2, V3); return; } } while (0)
#define Assert4(C, M, V1, V2, V3, V4) \
  do { if (!(C)) { CheckFailed(M, V1, V2, V3, V4); return; } } while (0)

  void
SPIRVerifier::visit(Instruction &I)
{
  for (unsigned i = 0, e = I.getNumOperands(); i != e; ++i)
    Assert1(I.getOperand(i) != 0, "Operand is null", &I);
  InstVisitor<SPIRVerifier>::visit(I);
}

// Verify that the module is a SPIR module and that it has
// the basic information correct. This is the lightweight check.
  void
SPIRVerifier::visitSPIRHeader(Module &M)
{
  if (M.getTargetTriple() == "spir-unknown-unknown") {
    Bitness = 32;
    if (M.getDataLayout() != SPIR_DATA_LAYOUT_32BIT) {
      CheckFailed(ErrorMessages(InvalidDataLayout), M.getDataLayout());
      return;
    }
  } else if (M.getTargetTriple() == "spir64-unknown-unknown") {
    Bitness = 64;
    if (M.getDataLayout() != SPIR_DATA_LAYOUT_64BIT) {
      CheckFailed(ErrorMessages(InvalidDataLayout), M.getDataLayout());
      return;
    }
  } else {
    CheckFailed(ErrorMessages(InvalidTriple), M.getTargetTriple());
    return;
  }

  NamedMDNode *SPIRVersion = M.getNamedMetadata("spir.version");
  visitSPIRVersionMD(SPIRVersion);

  NamedMDNode *OCLVersion = M.getNamedMetadata("spir.ocl.version");
  visitOCLVersionMD(OCLVersion);

  NamedMDNode *OptFeat =
    M.getNamedMetadata("spir.used.optional.core.features");
  visitOptionalCoreMD(OptFeat);

  NamedMDNode *KHRExt = M.getNamedMetadata("spir.used.extensions");
  visitUsedExtensionsMD(KHRExt);
}

  const char *
SPIRVerifier::ErrorMessages(unsigned err_code)
{
  switch(err_code) {
    default:
      return "Unknown error.";
    case InvalidType:
      return "Invalid type, SPIR Section 2.1.1.";
    case RequiresDouble:
      return "Requires cl_doubles optional core feature,"
        " SPIR section 2.12.2.";
    case RequiresImages:
      return "Requires cl_images optional core feature,"
        " SPIR section 2.12.2.";
    case InvalidVectorSize:
      return "Invalid vector size, SPIR Section 2.1.2.";
    case InvalidAddressSpace:
      return "Invalid address space, SPIR section 2.2.";
    case BooleanVector:
      return "Vector of booleans invalid, SPIR Section 2.1.2.";
    case InvalidInstr:
      return "Invalid instruction, SPIR Section 3.3.";
    case NoAtomicLoadStore:
      return "Invalid atomic attribute, SPIR section 3.3.";
    case UnalignedLoadStore:
      return "Unaligned load/store, SPIR section 2.1.4.";
    case VectorConversion:
      return "Invalid vector conversion, SPIR Section 3.3.";
    case InvalidPtrInst:
      return "Invalid Pointer Instruction, SPIR section 3.3.";
    case NoTailCall:
      return "No tail calls allowed, SPIR section 3.10.";
    case BadCallingConv:
      return "Invalid calling convention, SPIR section 3.6.";
    case NoNounwind:
      return "Nounwind must be specified, SPIR section 3.10.";
    case InvalidFuncAttribute:
      return "Invalid function attribute, SPIR section 3.10.";
    case InvalidArgAttribute:
      return "Invalid argument attribute, SPIR section 3.8.";
    case InlineAsm:
      return "Invalid inline assembly, SPIR section 3.12.";
    case IndirectCall:
      return "Invalid indirect call, SPIR section 3.3.";
    case InvalidTriple:
      return "Invalid target triple, SPIR section 3.1.";
    case InvalidDataLayout:
      return "Invalid data layout, SPIR section 3.2.";
    case InvalidSPIRVersion:
      return "Invalid SPIR version, SPIR section 2.13.";
    case InvalidOCLVersion:
      return "Invalid OCL version, SPIR section 2.14.";
    case InvalidOptionalCore:
      return "Invalid optional core features, SPIR section 2.12.1.";
    case InvalidKHRExtension:
      return "Invalid KHR Extensions, SPIR section 2.12.2.";
    case UnsupportedSPIRVersion:
      return "Unsupported SPIR version.";
    case UnsupportedOCLVersion:
      return "Unsupported OCL version.";
    case UnsupportedOptionalCore:
      return "Unsupported optional core features.";
    case UnsupportedKHRExtension:
      return "Unsupported KHR Extensions.";
    case InvalidLinkage:
      return "Invalid linkage mode, SPIR section 3.5.";
    case InvalidVisibility:
      return "Invalid visibility, SPIR section 3.7.";
    case InvalidVarArg:
      return "Var Args are illegal, SPIR section 3.3.";
    case InvalidGarbageCollection:
      return "Garbage colletion is illegal, SPIR section 3.9.";
    case InvalidNameMangling:
      return "Name mangling is invalid, SPIR appendix A.";
    case InvalidIntrRet:
      return "Return value for intrincis is invalid, SPIR spec 2.1.1.X/2.11.1.";
    case InvalidIntrArg:
      return "Argument for intrincis is invalid, SPIR spec 2.1.1.X/2.11.1.";
    case InvalidIntrFunc:
      return "The intrinsic function is invalid, SPIR spec 2.1.1.X/2.11.1.";
    case IllegalAShrExact:
      return "The AShr instruction cannot have 'exact', SPIR spec 3.3.";
    case InvalidCompilerOpts:
      return "Invalid compiler options in metadata, SPIR spec 2.9.";
    case InvalidCompilerOptsMD:
      return "Named compiler options metadata format is invalid, SPIR spec 2.9.";
    case InvalidExtCompilerOpts:
      return "Invalid extended compiler options metadata, SPIR spec 2.9.";
    case InvalidExtCompilerOptsMD:
      return "Named ext compiler options metadata format is invalid, SPIR spec 2.9.";
    case InvalidSamplerMD:
      return "Invalid sampler MD detected, SPIR spec 2.1.3.1.";
    case InvalidFunctionMD:
      return "Function metadata is malformed, SPIR spec 2.4.1/2.4.2.";
    case InvalidSPIRFunction:
      return "The SPIR function specified is invalid, SPIR spec 2.1.1.1.";
    case InvalidSPIRVersionMD:
      return "The SPIR Version named metadata format is invalid, SPIR spec 2.13.";
    case InvalidOCLVersionMD:
      return "The OpenCL Version named metadata format is invalid, SPIR spec 2.14.";
    case InvalidOptionalCoreMD:
      return "The Optional Core metadata format is invalid, SPIR spec 2.12.1.";
    case InvalidKHRExtensionMD:
      return "The KHR Extension named metadata format is invalid, SPIR spec 2.12.2.";
    case InvalidNamedMD:
      return "Found an invalid named metadata node.";
    case InvalidPtrBitcast:
      return "It is illegal to bitcast between address spaces, SPIR spec 2.8.2.2.";
    case InvalidClStdOpt:
      return "-cl-std should not exist in the compiler options metadata, SPIR spec 2.9 footnote.";
    case InvalidMangling:
      return "The name mangling of the function was invalid, SPIR spec appendix A.";
    case InvalidReservedName:
      return "The __spir and spir. names are reserved, SPIR Spec 3.11.";
  }
}
// A pointer type is valid if the address spaces are valid.
  bool
SPIRVerifier::isValidType(const PointerType *t)
{
  switch (t->getAddressSpace()) {
    default:
      CheckFailed(ErrorMessages(InvalidAddressSpace), t);
      return false;
    case SPIRAS_PRIVATE:
    case SPIRAS_GLOBAL:
    case SPIRAS_CONSTANT:
    case SPIRAS_LOCAL:
    case SPIRAS_GLOBAL_HOST:
    case SPIRAS_CONSTANT_HOST:
      return true;
  }
  // FIXME: Should we verify that we are pointing to a valid
  // type? Things get tricky in this case when we try to
  // verify a pointer in structure with a loop in the
  // pointer chain.
}

// An array type is valid if the element types are all valid.
  bool
SPIRVerifier::isValidType(const ArrayType *type)
{
  for (Type::subtype_iterator sti = type->subtype_begin(),
      ste = type->subtype_end(); sti != ste; ++sti) {
    if (!isValidType(*sti)) {
      CheckFailed(ErrorMessages(InvalidType), type);
      return false;
    }
  }
  return true;
}

// A vector type is valid if the element types are valid
// and the size is 2, 3, 4, 8 or 16 elements.
  bool
SPIRVerifier::isValidType(const VectorType *type)
{
  unsigned numEle = type->getNumElements();
  switch (numEle) {
    default:
      CheckFailed(ErrorMessages(InvalidVectorSize), type);
      return false;
    case 2:
    case 3:
    case 4:
    case 8:
    case 16:
      break;
  }
  for (llvm::Type::subtype_iterator sti = type->subtype_begin(),
      ste = type->subtype_end(); sti != ste; ++sti) {
    if ((*sti)->isIntegerTy(1)) {
      CheckFailed(ErrorMessages(BooleanVector), type);
      return false;
    }
    if (!isValidType(*sti)) {
      CheckFailed(ErrorMessages(InvalidType), type);
      return false;
    }
  }
  return true;
}

// A struct type is valid if all of its element types are valid
// or if it is an opaque type with a reserved SPIR name.
bool SPIRVerifier::isValidType(const StructType *type)
{
  if (type->isOpaque()) {
    if (type->hasName()) {
      StringRef name = type->getName();
      if (name.startswith("spir.image1d_t")) return HasImages;
      if (name.startswith("spir.image1d_array_t")) return HasImages;
      if (name.startswith("spir.image1d_buffer_t")) return HasImages;
      if (name.startswith("spir.image2d_t")) return HasImages;
      if (name.startswith("spir.image2d_array_t")) return HasImages;
      if (name.startswith("spir.image3d_t")) return HasImages;
      if (name.startswith("spir.sampler_t")) return true;
      if (name.startswith("spir.event_t")) return true;
      if (name.startswith("spir.half_t")) return true;
      if (name.startswith("spir.size_t")) return true;
    }
    CheckFailed(ErrorMessages(InvalidType), type);
    return false;
  }
  for (Type::subtype_iterator sti = type->subtype_begin(),
      ste = type->subtype_end(); sti != ste; ++sti) {
    if (!isValidType(*sti)) {
      CheckFailed(ErrorMessages(InvalidType), type);
      return false;
    }
  }
  return true;
}

// A Function type is valid if all of its operands are valid,
// it has a valid return type and is not a variable argument.
  bool
SPIRVerifier::isValidType(const FunctionType *type)
{
  for (FunctionType::param_iterator pib = type->param_begin(),
      pie = type->param_end(); pib != pie; ++pib) {
    if (!isValidType(*pib)) {
      CheckFailed(ErrorMessages(InvalidType), *pib);
      return false;
    }
  }
  if (!isValidType(type->getReturnType())) {
    CheckFailed(ErrorMessages(InvalidType), type);
    return false;
  }
  // FIXME: What do we do about printf?
  return !type->isVarArg();
}
  bool
SPIRVerifier::isValidType(const Type *type)
{
  // Verify the scalar types
  if (type->isIntegerTy(1) || type->isIntegerTy(8)
      || type->isIntegerTy(16) || type->isIntegerTy(32)
      || type->isIntegerTy(64) || type->isVoidTy()
      || type->isFloatTy() || type->isLabelTy())  return true;
  if (type->isDoubleTy()) {
    if (HasDouble) return true;
    CheckFailed(ErrorMessages(RequiresDouble), type);
    return false;
  }
  if (type->isVectorTy())
    return isValidType(dyn_cast<VectorType>(type));
  if (type->isArrayTy())
    return isValidType(dyn_cast<ArrayType>(type));
  if (type->isPointerTy())
    return isValidType(dyn_cast<PointerType>(type));
  if (type->isStructTy())
    return isValidType(dyn_cast<StructType>(type));
  if (type->isFunctionTy())
    return isValidType(dyn_cast<FunctionType>(type));
  if (type->isMetadataTy()) return true;

  CheckFailed(ErrorMessages(InvalidType), type);
  return false;
}

// Verify that the optional core features are valid.
// Format:
// !spir.used.optional.core.features = !{!N}
// !N = metadata !{metadata !"feat1", ..., metadata !"featN"}
// SPIR Spec section 2.12.1
  void
SPIRVerifier::visitOptionalCoreMD(NamedMDNode *OptFeat)
{
  // Lets check the Optional features metadata for both valid
  // features and supported features. This has to be parsed
  // before the extensions since we require to know if
  // images are supported to determine if some image extensions
  // are also supported.
  if (!OptFeat) return;
  if (OptFeat->getNumOperands() != 1) {
    CheckFailed(ErrorMessages(InvalidOptionalCoreMD), OptFeat->getName());
    return;
  }
  MDNode *optfeat = OptFeat->getOperand(0);
  for (unsigned x = 0, y = optfeat->getNumOperands(); x < y; ++x) {
    MDString *ptr = dyn_cast<MDString>(optfeat->getOperand(x));
    if (!ptr) {
      CheckFailed(ErrorMessages(InvalidOptionalCore), optfeat->getName());
      return;
    }
    if (ptr->getString() == "cl_doubles") {
      HasDouble = true;
      if (OptCore.find("cl_doubles") == std::string::npos) {
        CheckFailed(ErrorMessages(UnsupportedOptionalCore), ptr);
        return;
      }
    } else if (ptr->getString() == "cl_images") {
      HasImages = true;
      if (OptCore.find("cl_images") == std::string::npos) {
        CheckFailed(ErrorMessages(UnsupportedOptionalCore), ptr);
        return;
      }
    } else {
      CheckFailed(ErrorMessages(InvalidOptionalCore), ptr);
    }
  }
}

// Verify that the list of used extensions is valid and
// only includes the KHR extensions.
// Format:
// !spir.used.extensions = !{!N}
// !N = metadata !{metadata !"ext1", ..., metadata !"extN"}
// SPIR Spec 2.12.2
  void
SPIRVerifier::visitUsedExtensionsMD(NamedMDNode *KHRExt)
{
  if (!KHRExt) return;

  KhrExtensions* checker = KhrExtensions::getInstance();

  if (KHRExt->getNumOperands() != 1) {
    CheckFailed(ErrorMessages(InvalidKHRExtensionMD), KHRExt->getName());
    return;
  }
  MDNode *khrext = KHRExt->getOperand(0);
  // TODO: since the list of valid extensions is known, should
  // we generate a perfect hash function for this to speed up
  // detection?
  // Lets check the list of 1.2 KHR extensions to make sure that the
  // ones declared in the module are valid and also check to
  // make sure that the ones declared in the module are supported
  // by the vendor/device.
  for (unsigned x = 0, y = khrext->getNumOperands(); x < y; ++x) {
    MDString *ptr = dyn_cast<MDString>(khrext->getOperand(x));
    if (!ptr) {
      CheckFailed(ErrorMessages(InvalidKHRExtension), ptr);
      continue;
    }
    std::string ext = ptr->getString();
    bool found = checker->contains(ext);
    if (!found) {
      CheckFailed(ErrorMessages(InvalidKHRExtension), ptr);
      return;
    }
    if (ext == "cl_khr_3d_image_writes" && !HasImages) {
      CheckFailed(ErrorMessages(UnsupportedKHRExtension), ptr);
      return;
    }
    if (KhrExt.find(ext) == std::string::npos) {
      CheckFailed(ErrorMessages(UnsupportedKHRExtension), ptr);
      return;
    }
  }
}

// Verify that the OCL version metadata follows the pattern:
// !spir.ocl.version = !{!N}
// !N = metadata !{i32, i32}
// SPIR Spec 2.14
  void
SPIRVerifier::visitOCLVersionMD(NamedMDNode *OCLVersion)
{
  if (!OCLVersion) return;
  if (OCLVersion->getNumOperands() != 1) {
    CheckFailed(ErrorMessages(InvalidOCLVersionMD), OCLVersion->getName());
    return;
  }
  MDNode *ver = OCLVersion->getOperand(0);
  if (ver->getNumOperands() != 2) {
    CheckFailed(ErrorMessages(InvalidOCLVersionMD), ver);
    return;  
  }
  ConstantInt *major = dyn_cast<ConstantInt>(ver->getOperand(0));
  ConstantInt *minor = dyn_cast<ConstantInt>(ver->getOperand(1));
  if (0 == major || 0 == minor) {
    CheckFailed(ErrorMessages(InvalidOCLVersion), ver);
    return;
  }
  if (major->getZExtValue() > OCLVer[0]
      || minor->getZExtValue() > OCLVer[1]) {
    CheckFailed(ErrorMessages(UnsupportedOCLVersion), ver);
    return;
  }
}

// Verify that the SPIR version metadata follows the pattern:
// !spir.version = !{!N}
// !N = metadata !{i32, i32}
// SPIR Spec 2.13
  void
SPIRVerifier::visitSPIRVersionMD(NamedMDNode *SPIRVersion)
{
  if (!SPIRVersion) return;
  if (SPIRVersion->getNumOperands() != 1) {
    CheckFailed(ErrorMessages(InvalidSPIRVersionMD), SPIRVersion->getName());
    return;
  }
  MDNode *ver = SPIRVersion->getOperand(0);
  if (ver->getNumOperands() != 2) {
    CheckFailed(ErrorMessages(InvalidSPIRVersionMD), ver);
    return;
  }
  ConstantInt *major = dyn_cast<ConstantInt>(ver->getOperand(0));
  ConstantInt *minor = dyn_cast<ConstantInt>(ver->getOperand(1));
  if (0 == major || 0 == minor) {
    CheckFailed(ErrorMessages(InvalidSPIRVersion), ver);
    return;
  }
  if (major->getZExtValue() > SPIRVer[0]
      || minor->getZExtValue() > SPIRVer[1]) {
    CheckFailed(ErrorMessages(UnsupportedSPIRVersion), ver);
    return;
  }
}

// Format is:
// !spir.compiler.ext.options = !{!N}
// !N = metadata !{metadata !"opt1", ..., metadata !"optN"}
  void
SPIRVerifier::visitCompilerExtOptsMD(NamedMDNode *ExtOpts)
{
  if (!ExtOpts) return;
  if (ExtOpts->getNumOperands() != 1) {
    CheckFailed(ErrorMessages(InvalidExtCompilerOptsMD), ExtOpts->getName());
    return;
  }
  MDNode *opts = ExtOpts->getOperand(0);
  for (unsigned x = 0, y = opts->getNumOperands(); x < y; ++x) {
    MDString *ptr = dyn_cast<MDString>(opts->getOperand(x));
    if (!ptr) {
      CheckFailed(ErrorMessages(InvalidExtCompilerOpts),
          opts->getOperand(x));
      continue;
    }
  }
}
// Format is:
// !spir.compiler.options = !{!N}
// !N = metadata !{metadata !"opt1", ..., metadata !"optN"}
  void
SPIRVerifier::visitCompilerOptsMD(NamedMDNode *CompOpts)
{
  if (!CompOpts) return;
  const char *valid12_options[] = {
    "-cl-single-precision-constant",
    "-cl-denorms-are-zero",
    "-cl-fp32-correctly-rounded-divide-sqrt",
    "-cl-opt-disable",
    "-cl-mad-enable",
    "-cl-no-signed-zeros",
    "-cl-unsafe-math-optimizations",
    "-cl-finite-math-only",
    "-cl-fast-relaxed-math",
    "-w",
    "-Werror",
    "-cl-kernel-arg-info",
    "-create-library",
    "-enable-link-options"
  };
  if (CompOpts->getNumOperands() != 1) {
    CheckFailed(ErrorMessages(InvalidCompilerOptsMD), CompOpts->getName());
    return;
  }
  MDNode *opts = CompOpts->getOperand(0);
  // Lets check the options to make sure that they are the ones that
  // are allowed by the SPIR spec section 2.9.
  for (unsigned x = 0, y = opts->getNumOperands(); x < y; ++x) {
    MDString *ptr = dyn_cast<MDString>(opts->getOperand(x));
    if (!ptr) {
      CheckFailed(ErrorMessages(InvalidCompilerOpts), opts->getOperand(x));
      continue;
    }
    StringRef ext = ptr->getString();
    bool found = false;
    for (unsigned z = 0;
        z < sizeof(valid12_options) / sizeof(valid12_options[0]); ++z) {
      if (ext == valid12_options[z]) {
        found = true;
        break;
      }
    }
    if (!found) {
      if (ptr->getString().startswith("-cl-std")) {
        CheckFailed(ErrorMessages(InvalidClStdOpt), ptr);
      } else {
        CheckFailed(ErrorMessages(InvalidCompilerOpts), ptr);
      }
      return;
    }
  }
}

  void
SPIRVerifier::visitFunctionsMD(NamedMDNode *kernelfuncs)
{
  if (!kernelfuncs) return;
  // A kernel is everything that is in user funcs plus more.
  for (unsigned x = 0, y = kernelfuncs->getNumOperands(); x < y; ++x) {
	// Each operand is for a different function.
    MDNode *ptr = kernelfuncs->getOperand(x);
    if (ptr->getNumOperands() == 0) {
      CheckFailed(ErrorMessages(InvalidFunctionMD), ptr);
      continue;
    }
    Function *F = dyn_cast<Function>(ptr->getOperand(0));
    if (!F) {
      CheckFailed(ErrorMessages(InvalidFunctionMD), ptr);
      continue;
    }
    FunctionType *FT = F->getFunctionType();
    for (unsigned w = 1, z = ptr->getNumOperands(); w < z; ++w) {
      MDNode *mdptr = dyn_cast<MDNode>(ptr->getOperand(w));
      if (!mdptr || mdptr->getNumOperands() == 0) {
        CheckFailed(ErrorMessages(InvalidFunctionMD), ptr->getOperand(w));
        continue;
      }
      MDString *mdname = dyn_cast<MDString>(mdptr->getOperand(0));
      if (!mdname) {
        CheckFailed(ErrorMessages(InvalidFunctionMD), mdptr->getOperand(0));
        continue;
      }
      StringRef id = mdname->getString();
      if (id == "work_group_size_hint" || id == "reqd_work_group_size") {
        if (mdptr->getNumOperands() == 4
            && mdptr->getOperand(1)->getType()->isIntegerTy(32)
            && mdptr->getOperand(2)->getType()->isIntegerTy(32)
            && mdptr->getOperand(3)->getType()->isIntegerTy(32))
          continue;
      } else if (id == "vec_type_hint") {
        if (mdptr->getNumOperands() == 3
            && isValidType(mdptr->getOperand(1)->getType())
            && mdptr->getOperand(2)->getType()->isIntegerTy(1)
            && dyn_cast<ConstantInt>(mdptr->getOperand(2))
            && dyn_cast<ConstantInt>(mdptr->getOperand(2))->getZExtValue() < 2)
          continue;
      } else if (id == "cl-kernel-arg-info") {
    	// Each operand is for a category of arg info.
        for (unsigned d = 1, e = mdptr->getNumOperands(); d < e; ++d) {
          MDNode *kmdptr = dyn_cast<MDNode>(mdptr->getOperand(d));
          if (!kmdptr || kmdptr->getNumOperands() == 0) {
            CheckFailed(ErrorMessages(InvalidFunctionMD), kmdptr);
            continue;
          }
          MDString *kmdname = dyn_cast<MDString>(kmdptr->getOperand(0));
          if (!kmdname) {
            CheckFailed(ErrorMessages(InvalidFunctionMD), kmdptr);
            continue;
          }
          // get name of the arg info category
          StringRef kid = kmdname->getString();

          unsigned numMDOp = kmdptr->getNumOperands();
          unsigned numFunPar = FT->getNumParams();
          if (numMDOp != numFunPar + 1) {
            CheckFailed(ErrorMessages(InvalidFunctionMD), kmdptr);
            continue;
          }

          // Check metadata for each arg
          for (unsigned b = 1; b < numMDOp; ++b) {
            if (kid == "address_qualifier") {
              if (kmdptr->getOperand(b)->getType()->isIntegerTy(32)
                  && dyn_cast<ConstantInt>(kmdptr->getOperand(b))
                  && dyn_cast<ConstantInt>(kmdptr->getOperand(b))
                     ->getZExtValue() <= SPIRAS_CONSTANT_HOST)
                continue;
            } else if (kid == "access_qualifier") {
              bool fail = false;
              if (!kmdptr->getOperand(b)->getType()->isIntegerTy(32)
                  || !dyn_cast<ConstantInt>(kmdptr->getOperand(b))) {
                fail = true;
                break;
              }
              uint64_t val = dyn_cast<ConstantInt>(kmdptr->getOperand(b))
                ->getZExtValue();
              if (val >= 4) {
                fail = true;
                break;
              }
              if (val != 3
                && !isOpaqueNamedStruct(FT->getParamType(b-1), "spir.image")) {
                fail = true;
                break;
              }
              if (!fail) continue;
            } else if (kid == "arg_type_name") {
              if (dyn_cast<MDString>(kmdptr->getOperand(b)))
                continue;
            } else if (kid == "arg_type_qualifier") {
              if (kmdptr->getOperand(b)->getType()->isIntegerTy(32)
                  && dyn_cast<ConstantInt>(kmdptr->getOperand(b))
                  && dyn_cast<ConstantInt>(kmdptr->getOperand(b))
                     ->getZExtValue() < 8)
                continue;
            } else if (kid == "arg_name") {
              if (dyn_cast<MDString>(kmdptr->getOperand(b)))
                continue;
            }
          }
        }
        continue;
      } else if (id == "address_qualifier") {
        if (mdptr->getNumOperands() == 2
            && mdptr->getOperand(1)->getType()->isIntegerTy(32)
            && dyn_cast<ConstantInt>(mdptr->getOperand(1))
            && dyn_cast<ConstantInt>(mdptr->getOperand(1))->getZExtValue()
            <= SPIRAS_CONSTANT_HOST)
          continue;
      } else if (id == "access_qualifier") {
        bool fail = false;
        if (FT->getNumParams() != mdptr->getNumOperands() - 1) {
          CheckFailed(ErrorMessages(InvalidFunctionMD), mdptr->getOperand(0));
          continue;
        }
        for (unsigned b = 1, c = mdptr->getNumOperands(); b < c; ++b) {
          if (!mdptr->getOperand(b)->getType()->isIntegerTy(32)
              || !dyn_cast<ConstantInt>(mdptr->getOperand(b))) {
            fail = true;
            break;
          }
          uint64_t val = dyn_cast<ConstantInt>(mdptr->getOperand(b))
            ->getZExtValue();
          if (val >= 4) {
            fail = true;
            break;
          }
          if (val != 3
              && !isOpaqueNamedStruct(FT->getParamType(b-1), "spir.image")) {
            fail = true;
            break;
          }
        }
        if (!fail) continue;
      } else if (id == "arg_type_name") {
        CheckFailed(ErrorMessages(InvalidFunctionMD), mdname);
        if (mdptr->getNumOperands() == 2
            && dyn_cast<MDString>(mdptr->getOperand(1)))
          continue;
      } else if (id == "arg_type_qualifier") {
        if (mdptr->getNumOperands() == 2
            && mdptr->getOperand(1)->getType()->isIntegerTy(32)
            && dyn_cast<ConstantInt>(mdptr->getOperand(1))
            && dyn_cast<ConstantInt>(mdptr->getOperand(1))->getZExtValue() < 8)
          continue;
      } else if (id == "arg_name") {
        if (mdptr->getNumOperands() == 2
            && dyn_cast<MDString>(mdptr->getOperand(1)))
          continue;
      }
      CheckFailed(ErrorMessages(InvalidFunctionMD), mdname);
    }
  }
}
  void
SPIRVerifier::visitNamedMDNode(NamedMDNode &NMD)
{
  StringRef name = NMD.getName();
  // We only need to investigate spir related named
  // metadata nodes.
  if (!name.startswith("spir.")) return;
  if (name == "spir.functions") {
    visitFunctionsMD(&NMD);
  } else if (name == "spir.compiler.options") {
    visitCompilerOptsMD(&NMD);
  } else if (name == "spir.compiler.ext.options") {
    visitCompilerExtOptsMD(&NMD);
  } else if (name == "spir.ocl.version") {
    visitOCLVersionMD(&NMD);
  } else if (name == "spir.version") {
    visitSPIRVersionMD(&NMD);
  } else if (name == "spir.used.extensions") {
    visitUsedExtensionsMD(&NMD);
  } else if (name == "spir.used.optional.core.features") {
    visitOptionalCoreMD(&NMD);
  } else if (name == "spir.disable.FP_CONTRACT") {
  } else {
    CheckFailed(ErrorMessages(InvalidNamedMD), name);
  }

}
  void
SPIRVerifier::visitGlobalValue(GlobalValue &GV)
{
  if (!GV.hasExternalLinkage()
      && !GV.hasAvailableExternallyLinkage()
      && !GV.hasPrivateLinkage()
      && !GV.hasInternalLinkage()) {
    CheckFailed(ErrorMessages(InvalidLinkage), &GV);
  }
  if (!GV.hasDefaultVisibility()) {
    CheckFailed(ErrorMessages(InvalidVisibility), &GV);
  }
  if (!isValidType(GV.getType())) {
    CheckFailed(ErrorMessages(InvalidType), &GV);
  }
}
void
SPIRVerifier::visitFunction(Function &F) {
  Function *func = &F;
  if (!isValidType(func->getReturnType())) {
    CheckFailed(ErrorMessages(InvalidType), func->getReturnType());
  }
  if (!isValidType(func->getFunctionType())) {
    CheckFailed(ErrorMessages(InvalidType), func->getFunctionType());
  }
  if (func->isVarArg()) {
    CheckFailed(ErrorMessages(InvalidVarArg), func);
  }
  if (func->getCallingConv() != CallingConv::SPIR_KERNEL
      && func->getCallingConv() != CallingConv::SPIR_FUNC) {
    CheckFailed(ErrorMessages(BadCallingConv), func);
  }

  // Anything other than NoUnwind, AlwaysInline, InlineHint, NoInline, ReadNone, ReadOnly is an error
  llvm::Attributes::AttrVal vals[22] = {llvm::Attributes::AddressSafety, llvm::Attributes::Alignment,
                                        llvm::Attributes::ByVal, llvm::Attributes::InReg,
                                        llvm::Attributes::MinSize, llvm::Attributes::Naked,
                                        llvm::Attributes::Nest, llvm::Attributes::NoAlias,
                                        llvm::Attributes::NoCapture, llvm::Attributes::NoImplicitFloat,
                                        llvm::Attributes::NonLazyBind, llvm::Attributes::NoRedZone,
                                        llvm::Attributes::NoReturn, llvm::Attributes::OptimizeForSize,
                                        llvm::Attributes::ReturnsTwice, llvm::Attributes::SExt,
                                        llvm::Attributes::StackAlignment, llvm::Attributes::StackProtect,
                                        llvm::Attributes::StackProtectReq, llvm::Attributes::StructRet,
                                        llvm::Attributes::UWTable, llvm::Attributes::ZExt};

  if (func->getAttributes().getFnAttributes().hasAttributes(llvm::Attributes::get(*Context, vals))) {
    CheckFailed(ErrorMessages(InvalidFuncAttribute), func);
  }
  // Anything other than ZExt, SExt, ByVal, StructRet, NoCapture is an error
  llvm::Attributes::AttrVal retVals[23] = {llvm::Attributes::AddressSafety, llvm::Attributes::Alignment,
                                           llvm::Attributes::AlwaysInline, llvm::Attributes::InReg,
                                           llvm::Attributes::MinSize, llvm::Attributes::Naked,
                                           llvm::Attributes::Nest, llvm::Attributes::NoAlias,
                                           llvm::Attributes::NoImplicitFloat, llvm::Attributes::ReadNone,
                                           llvm::Attributes::NonLazyBind, llvm::Attributes::NoRedZone,
                                           llvm::Attributes::NoReturn, llvm::Attributes::OptimizeForSize,
                                           llvm::Attributes::ReturnsTwice, llvm::Attributes::ReadOnly,
                                           llvm::Attributes::StackAlignment, llvm::Attributes::StackProtect,
                                           llvm::Attributes::StackProtectReq, llvm::Attributes::NoUnwind,
                                           llvm::Attributes::InlineHint, llvm::Attributes::NoInline,
                                           llvm::Attributes::UWTable};

  if (func->getAttributes().getRetAttributes().hasAttributes(llvm::Attributes::get(*Context, retVals))) {
    CheckFailed(ErrorMessages(InvalidArgAttribute), func);
  }
  for (Function::const_arg_iterator caib = func->arg_begin(),
      caie = func->arg_end(); caib != caie; ++caib) {
    if (caib->hasNestAttr()) {
      const Argument *arg = caib;
      CheckFailed(ErrorMessages(InvalidArgAttribute), arg);
    }
  }
  if (func->hasGC()) {
    CheckFailed(ErrorMessages(InvalidGarbageCollection), func);
  }

  if (!func->isIntrinsic() && isInvalidMangling(func)) {
    CheckFailed(ErrorMessages(InvalidNameMangling), func->getFunctionType());
  }

  if (isInvalidSPIRBuiltin(func)) {
    CheckFailed(ErrorMessages(InvalidSPIRFunction), func->getFunctionType());
  }
}
// Verify that all builtins that have '__spir_' in their name are valid SPIR 
// builtins. This is different than the name mangling as we don't do any type
// verification, only that we make sure the builtins are not specified incorrectly.
  bool
SPIRVerifier::isInvalidSPIRBuiltin(const Function *func)
{
  const StringRef funcName = func->getName();
  int status = 0;
  const char* data = __cxxabiv1::__cxa_demangle(funcName.data(), 0, 0, &status);
  // If we aren't mangled, move on.
  if (status || !data) return true;
  const char* ptr = strstr(data, "__spir_");
  // If we don't contain '__spir_', move on.
  if (!ptr) return false;
  ptr += 7;
  // There are two major classes of functions.
  // Class one:
  // __spir_s*:
  //  - __spir_sizet_*
  //  - __spir_size_of_*
  //  - __spir_sampler_initialize
  // __spir_g*:
  //  - __spir_globals_initializer
  //  - __spir_get_null_ptr*
  // __spir_eventt_null:
#define STRNCMPSTR(A, B) (!strncmp((A), (B), strlen((B))))
  signed size = strlen(ptr);
  unsigned id = 0;
  switch (*ptr) {
    case 's':
      if (size < 2) break;
      // Match against __spir_sampler_initialize. 
      if (ptr[1] == 'a') return !STRNCMPSTR(ptr, "sampler_initialize");
      if (size < 5) break;
      // Match against __spir_size_of_{pointer|sizet}. 
      if (ptr[4] == '_') return !STRNCMPSTR(ptr, "size_of_") && size > 8
        && (!STRNCMPSTR(ptr + 8, "pointer") || !STRNCMPSTR(ptr + 8, "sizet"));
      if (ptr[4] == 't') {
        if (!STRNCMPSTR(ptr, "sizet_") || size < 12) break;
        ptr += 6;
        size = strlen(ptr);
        switch (*ptr) {
          default: break;
          case 'c':
             if (size < 3) break;
             // Match against __spir_sizet_cmp. 
             if (ptr[1] == 'm') return !STRNCMPSTR(ptr, "cmp");
             // Match against __spir_sizet_convert_*. 
             if (size < 8) break;
             if (!STRNCMPSTR(ptr, "convert_")) break;
             ptr += 8;
             size -= 8;
             if (!size) break;
             switch (*ptr) {
               default: break;
               case 'i':
                  if (size < 2) break;
                  id = atoi(ptr + 1);
                  // Match against __spir_sizet_convert_i{1|8|16|32|64}. 
                  // This works because the types we want to check are all
                  // powers of 2, so one of the bits will match
                  // and then we just have to make sure that ID is a power of two
                  // to make sure things like i48 or i24 don't trigger success.
                  return !((id & 0x7f) && !(id & (id - 1)));
               case 'f':
                  // Match against __spir_sizet_convert_float. 
                  return !STRNCMPSTR(ptr, "float");
               case 's':
                  // Match against __spir_sizet_convert_size_t. 
                  return !STRNCMPSTR(ptr, "size_t");
               case 'd':
                  // Match against __spir_sizet_convert_double. 
                  return !STRNCMPSTR(ptr, "double");
               case 'h':
                  // Match against __spir_sizet_convert_half. 
                  return !STRNCMPSTR(ptr, "half");
               case 'p':
                  if (size < 4) break;
                  id = atoi(ptr + 3);
                  // Match against __spir_sizet_convert_ptr[0-5]. 
                  return !STRNCMPSTR(ptr, "ptr") && id <= 5;
             }
             break;
          case 'x':
                   // Match against __spir_sizet_xor. 
                   return !STRNCMPSTR(ptr, "xor");
          case 's':
                   if (size < 3) break;
                   // Match against __spir_sizet_sub. 
                   if (ptr[1] == 'u') return !STRNCMPSTR(ptr, "sub");
                   // Match against __spir_sizet_shl. 
                   if (ptr[1] == 'h') return !STRNCMPSTR(ptr, "shl");
                   // Match against __spir_sizet_srem. 
                   if (ptr[1] == 'r') return !STRNCMPSTR(ptr, "srem");
                   // Match against __spir_sizet_sdiv. 
                   if (ptr[1] == 'd') return !STRNCMPSTR(ptr, "sdiv");
                   break;
          case 'u':
                   if (size < 4) break;
                   // Match against __spir_sizet_urem. 
                   if (ptr[1] == 'r') return !STRNCMPSTR(ptr, "urem");
                   // Match against __spir_sizet_udiv. 
                   if (ptr[1] == 'd') return !STRNCMPSTR(ptr, "udiv");
                   break;
          case 'o':
                   // Match against __spir_sizet_or. 
                   return !STRNCMPSTR(ptr, "or");
          case 'a':
                   if (size < 3) break;
                   // Match against __spir_sizet_add*. 
                   if (ptr[1] == 'd') {
                     if (ptr[3] != '_') {
                       // Match against __spir_sizet_add. 
                       return !STRNCMPSTR(ptr, "add");
                     }
                     if (size < 8) break;
                     id = atoi(ptr + 7);
                     // Match against __spir_sizet_add_ptr[0-5]. 
                     return !STRNCMPSTR(ptr, "add_ptr") && id <= 5;
                   }
                   // Match against __spir_sizet_alloca. 
                   if (ptr[1] == 'l') return !STRNCMPSTR(ptr, "alloca");
                   // Match against __spir_sizet_ashr. 
                   if (ptr[1] == 's') return !STRNCMPSTR(ptr, "ashr");
                   // Match against __spir_sizet_and. 
                   if (ptr[1] == 'n') return !STRNCMPSTR(ptr, "and");
                   break;
          case 'm':
                   // Match against __spir_sizet_mul. 
                   return !STRNCMPSTR(ptr, "mul");
          case 'n':
                   // Match against __spir_sizet_not. 
                   return !STRNCMPSTR(ptr, "not");
          case 'l':
                   // Match against __spir_sizet_lshr. 
                   return !STRNCMPSTR(ptr, "lshr");
        }
      }
      break; 
    case 'g':
      if (size < 2) break;
      // Match against __spir_globals_initializer. 
      if (ptr[1] == 'l') return !STRNCMPSTR(ptr, "globals_initializer");
      // Match against __spir_get_null_ptr[0-5]. 
      if (ptr[1] == 'e' && strlen(ptr) > 12) {
        id = atoi(ptr + 12);
        return !STRNCMPSTR(ptr, "get_null_ptr") && id <= 5;
      }
      break;
    case 'e':
      // Match against __spir_eventt_null. 
      return !STRNCMPSTR(ptr, "eventt_null");
    default: break;
  }
#undef STRNCMPSTR
  return false;
}
static const char *functionNames[] =
{
  "__spir_sizet_convert_size_t",  "__spir_sizet_convert_i1",
  "__spir_sizet_convert_i8",      "__spir_sizet_convert_i16",
  "__spir_sizet_convert_i32",     "__spir_sizet_convert_i64",
  "__spir_sizet_convert_half",    "__spir_sizet_convert_float",
  "__spir_sizet_convert_double",  "__spir_sizet_convert_ptr0",
  "__spir_sizet_convert_ptr1",    "__spir_sizet_convert_ptr2",
  "__spir_sizet_convert_ptr3",    "__spir_sizet_convert_ptr4",
  "__spir_sizet_convert_ptr5",    "__spir_sizet_add",
  "__spir_sizet_sub",             "__spir_sizet_mul",
  "__spir_sizet_udiv",            "__spir_sizet_sdiv",
  "__spir_sizet_urem",            "__spir_sizet_srem",
  "__spir_sizet_or",              "__spir_sizet_and",
  "__spir_sizet_xor",             "__spir_sizet_not",
  "__spir_sizet_shl",             "__spir_sizet_lshr",
  "__spir_sizet_ashr",            "__spir_sizet_cmp",
  "__spir_sizet_add_ptr0",        "__spir_sizet_add_ptr1",
  "__spir_sizet_add_ptr2",        "__spir_sizet_add_ptr3",
  "__spir_sizet_add_ptr4",        "__spir_sizet_add_ptr5",
  "__spir_sizet_alloca",          "__spir_size_of_sizet",
  "__spir_size_of_pointer",       "__spir_sampler_initialize",
  "__spir_globals_initializer",   "__spir_eventt_null",
  "__spir_get_null_ptr0",         "__spir_get_null_ptr1",
  "__spir_get_null_ptr2",         "__spir_get_null_ptr3",
  "__spir_get_null_ptr4",         "__spir_get_null_ptr5",
}; // end function names table.

// Verify that the mangling was done correctly.
  bool
SPIRVerifier::isInvalidMangling(const Function *func)
{
  assert(!func->isIntrinsic() && "Must not be an Intrinsic Instruction.");
  StringRef mangledName = func->getName();
  if (!mangledName.startswith("_Z")) {
    CheckFailed("Invalid Prefix, C++ ABI Spec 5.1.2.", func);
    return true;
  }
  unsigned offset = 0, size = mangledName.size();
  unsigned state = 0;
  int status = 0;
  const char* name = __cxxabiv1::__cxa_demangle(mangledName.str().c_str(), 0, 0, &status);
  if (status) {
    CheckFailed(ErrorMessages(InvalidMangling), func);
    return true;
  }
  StringRef unmangledName(name);
  StringRef funcName(unmangledName.data(), unmangledName.find('('));
  if (funcName.startswith("__spir")) {
    bool fail = true;
    for (unsigned x = 0, y = sizeof(functionNames)/sizeof(functionNames[0]);
        x < y; ++x) {
      if (funcName.equals(functionNames[x])) {
        fail = false;
        break;
      }
    }
    if (fail) {
      CheckFailed(ErrorMessages(InvalidReservedName), func);
      free((void*)name);
      return true;
    }
  }
  dbgs() << status << ": " << mangledName << " ==> " << unmangledName << "\n";
  free((void*)name);
  return false;
}

  bool
SPIRVerifier::isTypeAligned(const Type *T, unsigned align)
{
  if (!align) return true;
  if (!T->isSized()) return align == 1;
  DataLayout data(Mod);
  return data.getABITypeAlignment(const_cast<Type*>(T)) % align == 0;
}

// This function returns true if the number in the
// tok StringRef is equal to the vector number of
// elements.
bool isVectorTokenEqual(StringRef tok, const Type *T)
{
  const VectorType *VT = dyn_cast<VectorType>(T);
  return VT && (tok == itostr(VT->getNumElements()));
}

// This function returns true if the type is an
// opaque structure with a name that is equal
// to the name passed in the second argument.
static bool isOpaqueNamedStruct(const Type *t, StringRef name)
{
  const StructType *ST = dyn_cast<StructType>(t);
  return ST && ST->isOpaque() && ST->hasName()
    && ST->getName().startswith(name);
}

static bool isAddressSpace(const Type *t, unsigned as)
{
  const PointerType *PT = dyn_cast<PointerType>(t);
  return PT && PT->getAddressSpace() == as;
}

// SPIR Spec section 3.3
  template <class T>
bool SPIRVerifier::verifyOperands(Instruction &I)
{
  for (typename T::op_iterator oib = static_cast<T&>(I).op_begin(),
      oie = static_cast<T&>(I).op_end();
      oib != oie; ++oib) {
    if (!isValidType(oib->get()->getType())) {
      CheckFailed(ErrorMessages(InvalidType), *oib);
      return false;
    }
  }
  return true;
}

  void
SPIRVerifier::visitInstruction(Instruction &I)
{
  switch(I.getOpcode()) {
    case Instruction::Br:
    case Instruction::Ret:
    case Instruction::Switch:
    case Instruction::Unreachable:
      if (!verifyOperands<TerminatorInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::Add:
    case Instruction::Sub:
    case Instruction::Mul:
    case Instruction::UDiv:
    case Instruction::URem:
    case Instruction::SDiv:
    case Instruction::SRem:
    case Instruction::FAdd:
    case Instruction::FSub:
    case Instruction::FMul:
    case Instruction::FDiv:
    case Instruction::FRem:
    case Instruction::Shl:
    case Instruction::LShr:
    case Instruction::AShr:
    case Instruction::And:
    case Instruction::Or:
    case Instruction::Xor:
      if (!verifyOperands<BinaryOperator>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      if (I.getOpcode() == Instruction::AShr) {
        if (static_cast<BinaryOperator&>(I).isExact()) {
          CheckFailed(ErrorMessages(IllegalAShrExact), &I);
        }
      }
      break;
    case Instruction::ExtractElement:
      if (!verifyOperands<ExtractElementInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::InsertElement:
      if (!verifyOperands<InsertElementInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::ShuffleVector:
      if (!verifyOperands<ShuffleVectorInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::ExtractValue:
      if (!verifyOperands<ExtractValueInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::InsertValue:
      if (!verifyOperands<InsertValueInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::Load:
      if (!verifyOperands<LoadInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      if (dyn_cast<LoadInst>(&I)->isAtomic()) {
        CheckFailed(ErrorMessages(NoAtomicLoadStore), &I);
      }
      if (!isTypeAligned(
            dyn_cast<PointerType>(dyn_cast<LoadInst>(&I)->
              getPointerOperand()->getType())->getElementType(),
            dyn_cast<LoadInst>(&I)->getAlignment())) {
        CheckFailed(ErrorMessages(UnalignedLoadStore), &I);
      }
      break;
    case Instruction::Store:
      if (!verifyOperands<StoreInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      if (dyn_cast<StoreInst>(&I)->isAtomic()) {
        CheckFailed(ErrorMessages(NoAtomicLoadStore), &I);
      }
      {
        const Type* type =  dyn_cast<PointerType>(dyn_cast<StoreInst>(&I)->
              getPointerOperand()->getType())->getElementType();
        if (!isTypeAligned(type, dyn_cast<StoreInst>(&I)->getAlignment())) {
          CheckFailed(ErrorMessages(UnalignedLoadStore), &I);
        }
      }
      break;
    case Instruction::GetElementPtr:
      if (!verifyOperands<GetElementPtrInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::Alloca:
      if (!verifyOperands<AllocaInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::Trunc:
    case Instruction::ZExt:
    case Instruction::SExt:
    case Instruction::FPToSI:
    case Instruction::SIToFP:
    case Instruction::FPToUI:
    case Instruction::UIToFP:
    case Instruction::FPTrunc:
    case Instruction::FPExt:
      if (!verifyOperands<UnaryInstruction>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      for (UnaryInstruction::op_iterator oib = I.op_begin(),
          oie = I.op_end(); oib != oie; ++oib) {
        if (oib->get()->getType()->isVectorTy()) {
          CheckFailed(ErrorMessages(VectorConversion), &I);
        }
      }
      break;
    case Instruction::ICmp:
    case Instruction::FCmp:
      if (!verifyOperands<CmpInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::BitCast:
      if (!verifyOperands<BitCastInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      if (I.getOperand(0)->getType()->isPointerTy()
          && I.getType()->isPointerTy()
          && dyn_cast<PointerType>(I.getOperand(0)->getType())
          ->getAddressSpace()
          != dyn_cast<PointerType>(I.getType())
          ->getAddressSpace()) {
        CheckFailed(ErrorMessages(InvalidPtrBitcast), &I);
      }
      break;
    case Instruction::PHI:
      if (!verifyOperands<PHINode>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::Select:
      if (!verifyOperands<SelectInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::Call:
      if (!verifyOperands<CallInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      {
        CallInst &CI = cast<CallInst>(I);
        if (CI.isTailCall()) {
          CheckFailed(ErrorMessages(NoTailCall), &I);
        }
        if (CI.getCallingConv() != CallingConv::SPIR_KERNEL
            && CI.getCallingConv() != CallingConv::SPIR_FUNC
            && dyn_cast<IntrinsicInst>(&CI) == NULL) {
          CheckFailed(ErrorMessages(BadCallingConv), &I);
        }
        if (!CI.paramHasAttr(AttrListPtr::FunctionIndex, Attributes::NoUnwind)
            && dyn_cast<IntrinsicInst>(&CI) == NULL) {
          CheckFailed(ErrorMessages(NoNounwind), &I);
        }

        // Anything other than NoUnwind, AlwaysInline, InlineHint, NoInline, ReadNone, ReadOnly is an error
        llvm::Attributes::AttrVal vals[22] = {llvm::Attributes::AddressSafety, llvm::Attributes::Alignment,
                                              llvm::Attributes::ByVal, llvm::Attributes::InReg,
                                              llvm::Attributes::MinSize, llvm::Attributes::Naked,
                                              llvm::Attributes::Nest, llvm::Attributes::NoAlias,
                                              llvm::Attributes::NoCapture, llvm::Attributes::NoImplicitFloat,
                                              llvm::Attributes::NonLazyBind, llvm::Attributes::NoRedZone,
                                              llvm::Attributes::NoReturn, llvm::Attributes::OptimizeForSize,
                                              llvm::Attributes::ReturnsTwice, llvm::Attributes::SExt,
                                              llvm::Attributes::StackAlignment, llvm::Attributes::StackProtect,
                                              llvm::Attributes::StackProtectReq, llvm::Attributes::StructRet,
                                              llvm::Attributes::UWTable, llvm::Attributes::ZExt};
        ArrayRef<llvm::Attributes::AttrVal> valArray(vals, 22);

        if (CI.getAttributes().getFnAttributes().hasAttributes(llvm::Attributes::get(*Context,
                                                                                     valArray))) {
          CheckFailed(ErrorMessages(InvalidFuncAttribute), &I);
        }
        // Anything other than ZExt, SExt, ByVal, StructRet, NoCapture is an error
        llvm::Attributes::AttrVal retVals[23] = {llvm::Attributes::AddressSafety, llvm::Attributes::Alignment,
                                                 llvm::Attributes::AlwaysInline, llvm::Attributes::InReg,
                                                 llvm::Attributes::MinSize, llvm::Attributes::Naked,
                                                 llvm::Attributes::Nest, llvm::Attributes::NoAlias,
                                                 llvm::Attributes::NoImplicitFloat, llvm::Attributes::ReadNone,
                                                 llvm::Attributes::NonLazyBind, llvm::Attributes::NoRedZone,
                                                 llvm::Attributes::NoReturn, llvm::Attributes::OptimizeForSize,
                                                 llvm::Attributes::ReturnsTwice, llvm::Attributes::ReadOnly,
                                                 llvm::Attributes::StackAlignment, llvm::Attributes::StackProtect,
                                                 llvm::Attributes::StackProtectReq, llvm::Attributes::NoUnwind,
                                                 llvm::Attributes::InlineHint, llvm::Attributes::NoInline,
                                                 llvm::Attributes::UWTable};
        ArrayRef<llvm::Attributes::AttrVal> retValArray(vals, 23);
        if (CI.getAttributes().getRetAttributes().hasAttributes(llvm::Attributes::get(*Context,
                                                                                      retValArray))) {
          CheckFailed(ErrorMessages(InvalidArgAttribute), I.getOperand(0));
        }

        if (CI.isInlineAsm()) {
          CheckFailed(ErrorMessages(InlineAsm), &I);
        }
        Function *fn = CI.getCalledFunction();
        if (0 == fn) {
          CheckFailed(ErrorMessages(IndirectCall), &I);
        }
        for (unsigned x = 0, y = CI.getNumArgOperands(); x < y; ++x) {
          if (CI.getAttributes().getParamAttributes(x + 1).hasAttributes(llvm::Attributes::get(*Context,
                                                                                               retValArray))) {
            CheckFailed(ErrorMessages(InvalidArgAttribute), I.getOperand(x + 1));
          }
        }
      }
      break;
    case Instruction::IntToPtr:
    case Instruction::PtrToInt:
      CheckFailed(ErrorMessages(InvalidPtrInst), &I);
      break;
    default:
      CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
  }
}

//===----------------------------------------------------------------------===//
//  Implement the public interfaces to this file...
//===----------------------------------------------------------------------===//

llvm::FunctionPass *
llvm::createLightweightSPIRVerifierPass(
    llvm::VerifierFailureAction action,
    llvm::SPIRState &state)
{
  SPIRVerifier *V = new SPIRVerifier(action, true);
  V->OptCore = state.CoreFeat;
  V->KhrExt = state.KhrFeat;
  V->SPIRVer[0] = state.SPIRMajor;
  V->SPIRVer[1] = state.SPIRMinor;
  V->OCLVer[0] = state.OCLMajor;
  V->OCLVer[1] = state.OCLMinor;
  return V;
}

llvm::FunctionPass *
llvm::createHeavyweightSPIRVerifierPass(
    llvm::VerifierFailureAction action,
    SPIRState &state)
{
  SPIRVerifier *V = new SPIRVerifier(action, false);
  V->OptCore = state.CoreFeat;
  V->KhrExt = state.KhrFeat;
  V->SPIRVer[0] = state.SPIRMajor;
  V->SPIRVer[1] = state.SPIRMinor;
  V->OCLVer[0] = state.OCLMajor;
  V->OCLVer[1] = state.OCLMinor;
  return V;
}

bool
llvm::verifySPIRModule(
    const llvm::Module &M,
    llvm::VerifierFailureAction action,
    llvm::SPIRState &State, bool lw,
    std::string *ErrorInfo)
{
  PassManager PM;
  FunctionPass *V = (lw ? createLightweightSPIRVerifierPass(action, State)
      : createHeavyweightSPIRVerifierPass(action, State));
  PM.add(V);
  PM.run(const_cast<Module&>(M));

  if (ErrorInfo && reinterpret_cast<SPIRVerifier*>(V)->Broken)
    *ErrorInfo = reinterpret_cast<SPIRVerifier*>(V)->MessagesStr.str();
  return reinterpret_cast<SPIRVerifier*>(V)->Broken;
}

