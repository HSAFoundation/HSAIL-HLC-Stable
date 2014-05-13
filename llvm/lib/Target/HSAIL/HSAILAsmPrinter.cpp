//===- HSAILAsmPrinter.cpp - Convert HSAIL LLVM code to assembly ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "HSAILAsmPrinter.h"
#include "HSAILKernelManager.h"
#include "HSAILMachineFunctionInfo.h"
#include "HSAILModuleInfo.h"
#include "HSAILTargetMachine.h"
#include "HSAILLLVMVersion.h"
#include "HSAILUtilityFunctions.h"
#include "HSAILOpaqueTypes.h"
#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Metadata.h"
#include "llvm/Module.h"
#include "llvm/Type.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/DerivedTypes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCObjectStreamer.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/InstIterator.h"
#include "../lib/CodeGen/AsmPrinter/DwarfDebug.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/Mangler.h"
#include <sstream>
#include <iostream>
#ifdef ANDROID
#include <ctype.h>
#endif
using namespace llvm;

#include "HSAILGenAsmWriter.inc"
#include "RawVectorOstream.h"

#ifndef HSAIL_SPEC_UPDATE_1_0
#define HSAIL_SPEC_UPDATE_1_0
#endif

static inline std::string
toHSAILHexString(uint64_t ui, unsigned int len) {
  std::string hexstr = utohexstr(ui);
  if (hexstr.length() < len) {
    std::string str;
    str.resize(len - hexstr.length(), '0');
    hexstr = str + hexstr;
  }
  assert(hexstr.length() == len && "invalid HSAIL hex constant");
  return hexstr;
}

static Type* printGVType(Type *ty, DataLayout& DL, std::string &str) {
  if (const ArrayType *ATy = dyn_cast<ArrayType>(ty)) {
    std::stringstream ss;
    ss << "[";
    uint64_t numElems = ATy->getNumElements();

    // Flatten multi-dimensional array declaration
    while (ATy->getElementType()->isArrayTy()) {
      ATy = dyn_cast<ArrayType>(ATy->getElementType());
      numElems *= ATy->getNumElements();
    }

    // There are no vector types in HSAIL, so global declarations for
    // arrays of composite types (vector, struct) are emitted as a single
    // array of a scalar type.

    // Flatten array of vector declaration
    if (ATy->getElementType()->isVectorTy()) {
      const VectorType *VTy = cast<VectorType>(ATy->getElementType());
      numElems *= VTy->getNumElements();
      ss << numElems << "]";
      str += ss.str();
      return printGVType(VTy->getElementType(), DL, str);
    }
    // Flatten array of struct declaration
    if (ATy->getElementType()->isStructTy()) {
      StructType *ST = cast<StructType>(ATy->getElementType());
      const StructLayout *layout = DL.getStructLayout(ST);
      numElems *= layout->getSizeInBytes();
    }
    ss << numElems << "]";
    str += ss.str();
    return printGVType(ATy->getElementType(), DL, str);
  } else if (const VectorType *VTy = dyn_cast<VectorType>(ty)) {
    std::stringstream ss;
    uint64_t numElems = VTy->getNumElements();
    ss << "[" << numElems << "]";
    str += ss.str();
    return VTy->getElementType();
  } else
    return ty;
}

static  Type* getGVType(Type *ty) {
  if (const ArrayType *ATy = dyn_cast<ArrayType>(ty)) {
    return getGVType(ATy->getElementType());
  } else
    return ty;
}

static void printGVInitialValue(
  const GlobalValue *GV,
  Constant *CV,
  raw_ostream &O,
  bool emit_braces = true)
{
  if (const ConstantArray *CA = dyn_cast<ConstantArray>(CV)) {
    O << (emit_braces ? "{" : "");
    for (unsigned i = 0, e = CA->getNumOperands(); i != e; ++i) {
      O << (i > 0 ? ", " : "");
      printGVInitialValue(GV, cast<Constant>(CV->getOperand(i)), O, false);
    }
    O << (emit_braces ? "}" : "");
  } else  if (const ConstantDataArray *CDA = dyn_cast<ConstantDataArray>(CV)) {
    O << (emit_braces ? "{" : "");
    for (unsigned i = 0, e = CDA->getNumOperands(); i != e; ++i) {
      O << (i > 0 ? ", " : "");
      printGVInitialValue(GV, cast<Constant>(CV->getOperand(i)), O, false);
    }
    O << (emit_braces ? "}" : "");
  } else if (const ConstantVector *CVE = dyn_cast<ConstantVector>(CV)) {
    for (unsigned i = 0, e = CVE->getType()->getNumElements(); i != e; ++i) {
      O << (i > 0 ? ", " : "");
      printGVInitialValue(GV, cast<Constant>(CVE->getOperand(i)), O, false);
    }
  } else if (const ConstantInt *CI = dyn_cast<ConstantInt>(CV)) {
    if (CI->getType()->isIntegerTy(1))
      O << (CI->getZExtValue() ? "true" : "false");
    else
      O << CI->getValue();
  } else if (const ConstantFP *CFP = dyn_cast<ConstantFP>(CV)) {
    uint64_t ui = CFP->getValueAPF().bitcastToAPInt().getZExtValue();
    if (CFP->getType()->isFloatTy()) {
      O << "0F" << toHSAILHexString(ui, 8);
    } else if (CFP->getType()->isDoubleTy()) {
      O << "0D" << toHSAILHexString(ui, 16);
    }
  } else if (dyn_cast<ConstantAggregateZero>(CV)) {
    const Type *ty = CV->getType();
    unsigned numElems = 1;
    // If this is a zero initializer for a vector type,
    // get the # elements and element type.
    if (ty->isVectorTy()) {
      const VectorType *VTy = dyn_cast<VectorType>(ty);
      ty = VTy->getElementType();
      numElems = VTy->getNumElements();
    }
    // numElems == 1 for scalar and array types, > 1 for vector types.
    for (unsigned i = 0; i < numElems; ++i) {
      O << (i > 0 ? ", " : "");
      switch(ty->getTypeID()) {
      case Type::IntegerTyID:
      case Type::PointerTyID:
        O << "0";
        break;
      case Type::FloatTyID:
        O << "0F" << toHSAILHexString(0, 8);
        break;
      case Type::DoubleTyID:
        O << "0D" << toHSAILHexString(0, 16);
        break;
      case Type::ArrayTyID:
        // Array type with a single zero initializer.
        assert(numElems == 1 && "Unexpected zero initializer");
        O << (emit_braces ? "{0}" : "0");
        break;
      default:
        assert(!"Unhandled zero initializer");
        break;
      }
    }
  } else if (dyn_cast<ConstantPointerNull>(CV)) {
    O << "0";
  } else {
    assert(!"Unhandled initializer");
  }
}

HSAILAsmPrinter::HSAILAsmPrinter(HSAIL_ASM_PRINTER_ARGUMENTS)
  : AsmPrinter(ASM_PRINTER_ARGUMENTS)
{
  Subtarget = &TM.getSubtarget<HSAILSubtarget>();
  FuncArgsStr = "";
  FuncRetValStr = "";
  retValCounter = 0;
  paramCounter = 0;
  reg1Counter = 0;
  reg32Counter = 0;
  reg64Counter = 0;
  mTM = reinterpret_cast<HSAILTargetMachine*>(&TM);
  mMeta = new HSAILKernelManager(mTM);
  mMFI = NULL;
  mBuffer = 0;
}

HSAILAsmPrinter::~HSAILAsmPrinter()
{
  delete mMeta;
}

static void printAlignTypeQualifier(Type *ty, DataLayout& DL,
  raw_ostream &O) 
{
  if (ArrayType *ATy = dyn_cast<ArrayType>(ty)) {
    // Print align-type qualifier for structs and arrays of structs.
    if (ATy->getElementType()->isStructTy()) {
      StructType *STy = cast<StructType>(ATy->getElementType());
      unsigned align = 1;
       // Scan members to find type with strictest alignment requirement.
       for (StructType::element_iterator I = STy->element_begin(),
        E = STy->element_end(); I != E; ++I) {
          Type* elemTy = *I;
          if (elemTy->getScalarSizeInBits() / 8 > align) {
            align = elemTy->getScalarSizeInBits() / 8;
          }
          if (align == 8) {
            break;
          }
      }
      assert ((align == 1 || align == 2 || align == 4 || align == 8) &&
        "invalid align-type qualifier");
      O << "align " << align << " ";
    }
    // TODO_HSA: Print align-type qualifiers for other types here (if required).
  }
}

std::string
HSAILAsmPrinter::getHSAILAddressSpace(const GlobalVariable* gv)
{
  std::string str = "";
  switch (gv->getType()->getAddressSpace()) {
  case HSAILAS::GLOBAL_ADDRESS: str += "global"; break;
  case HSAILAS::CONSTANT_ADDRESS: str += "readonly"; break;
  case HSAILAS::GROUP_ADDRESS: str += "group"; break;
  case HSAILAS::PRIVATE_ADDRESS: str += "private"; break;
  // TODO_HSA: default to global until all possible actions are resolved
  default: str += "global"; break;
  }
  return str;
}

bool
HSAILAsmPrinter::canInitHSAILAddressSpace(const GlobalVariable* gv) const
{
  bool canInit;
  switch (gv->getType()->getAddressSpace()) {
  case HSAILAS::GLOBAL_ADDRESS:
  case HSAILAS::CONSTANT_ADDRESS:
    canInit = true;
    break;
  default:
    canInit = false;
    break;
  }
  return canInit;
}

/// EmitGlobalVariable - Emit the specified global variable to the .s file.
void
HSAILAsmPrinter::EmitGlobalVariable(const GlobalVariable *GV)
{
  if (isIgnoredGV(GV))
    return;

  StringRef GVname = GV->getName();
  SmallString<1024> Str;
  raw_svector_ostream O(Str);
  DataLayout DL = getDataLayout();
  printAlignTypeQualifier(GV->getType()->getElementType(), DL, O);
  std::string str = "";
  O << getHSAILAddressSpace(GV)
    << getHSAILArgType(printGVType(GV->getType()->getElementType(), DL, str))
    << " &" << GVname << str;
  // TODO_HSA: if group memory has initializer, then emit instructions to
  // initialize dynamically
  if (GV->hasInitializer() && canInitHSAILAddressSpace(GV)) {
    O << " = ";
    printGVInitialValue(GV, (Constant *)GV->getInitializer(), O);
  }
  O << ";\n";
  O << "\n";
  O.flush();
  OutStreamer.EmitRawText(O.str());
}

static bool isHSAILInstrinsic(StringRef str) {
  if(str.startswith("__hsail_"))
    return true;
  return false;
}

// Returns true if StringRef is LLVM intrinsic function that define a mapping
// between LLVM program objects and the source-level objects.
// See http://llvm.org/docs/SourceLevelDebugging.html#format_common_intrinsics
// for more details.
//
static bool isLLVMDebugIntrinsic(StringRef str) {
  return str.equals("llvm.dbg.declare") || str.equals("llvm.dbg.value");
}

static bool isLLVMMemoryUseMarkerIntrinsic(StringRef str) {
  return str.startswith("llvm.lifetime");
}


void
HSAILAsmPrinter::EmitFunctionLabel(const Function &rF)
{
  std::string FunStr;
  raw_string_ostream OFunStr(FunStr);
  formatted_raw_ostream O(OFunStr);
  if (isLLVMDebugIntrinsic(rF.getName()) || isLLVMMemoryUseMarkerIntrinsic(rF.getName())) {
    return; //nothing to do with LLVM debug-related intrinsics
  }
  const Function *F = &rF;
  Type *retType = F->getReturnType();
  const FunctionType *funcType = F->getFunctionType();
  bool isKernel = isKernelFunc(F);
  O << (isKernel ? "kernel " : "function ");
  O << "&" << F->getName() << "(";
  // functions with kernel linkage cannot have output args
  paramCounter = 0;
  if ( !isKernel ) {
    // TODO_HSA: Need "good" names for the formal arguments and returned value
    // TODO_HSA: Need to emit alignment information.
    if (retType && (retType->getTypeID() != Type::VoidTyID)) {
      EmitFunctionReturn(retType, isKernel, O);

    }
    O << ") (";
  }
  if (funcType) {
    // Loop through all of the parameters and emit the types and
    // corresponding names.
    reg1Counter = 0;
    reg32Counter = 0;
    reg64Counter = 0;
    for (FunctionType::param_iterator pb = funcType->param_begin(),
        pe = funcType->param_end(); pb != pe; ++pb) {
      Type* type = *pb;
      EmitFunctionArgument(type, isKernel, O);
      if ((pb + 1) != pe) {
        O << ", ";
      }
    }
  }
  O << ");";
  O << "\n";
  O << "\n";
  O.flush();
  OutStreamer.EmitRawText(StringRef(FunStr));
}

//===------------------------------------------------------------------===//
// Overridable Hooks
//===------------------------------------------------------------------===//

static
std::string HSAILStrip(const std::string &name)
{
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

/** 
 * 
 * 
 * @param lMF MachineFunction to print the assembly for
 * @brief parse the specified machine function and print
 * out the assembly for all the instructions in the function
 * 
 * @return 
 */

bool
HSAILAsmPrinter::runOnMachineFunction(MachineFunction &lMF)
{
  std::string Str;
  raw_string_ostream OStr(Str);
  formatted_raw_ostream O(OStr);
  this->MF = &lMF;
  mMeta->setMF(&lMF);
  mMFI = lMF.getInfo<HSAILMachineFunctionInfo>();
  mAMI = &(lMF.getMMI().getObjFileInfo<HSAILModuleInfo>());
  SetupMachineFunction(lMF);
  const Function *F = MF->getFunction();
  OutStreamer.SwitchSection(getObjFileLowering().SectionForGlobal(F, Mang, TM));
  std::string kernelName = MF->getFunction()->getName();
  mName = HSAILStrip(kernelName);
  mKernelName = kernelName;
  // TODO_HSA: perhaps better to position this along with emit of version
  // directive. we put it here because usesGCNAtomicCounter() needs the
  // MachineFunction
  if (usesGCNAtomicCounter()) {
    O << "extension \"amd:gcn\";\n";
    O << "\n";
    O.flush();
    OutStreamer.EmitRawText(StringRef(Str));
  }

  // The need to define global samplers is discovered during instruction selection, 
  // so we emit them at file scope just before a kernel function is emitted.
  Subtarget->getImageHandles()->finalize();
  EmitSamplerDefs();

  EmitFunctionEntryLabel();
  EmitFunctionBody();

  // Clear local handles from image handles
  Subtarget->getImageHandles()->clearImageArgs();

  return false;
}

bool
HSAILAsmPrinter::isMacroFunc(const MachineInstr *MI) {
  if (MI->getOpcode() != HSAIL::target_call) {
    return false;
  }
  const llvm::StringRef &nameRef = MI->getOperand(0).getGlobal()->getName();
  if (nameRef.startswith("barrier")) {
    return true;
  }
  return false;
}

bool
HSAILAsmPrinter::isIdentityCopy(const MachineInstr *MI) const {
  if (MI->getNumOperands() != 2) {
    return false;
  }
  switch(MI->getOpcode()) {
    // Bitconvert is a copy instruction
  case HSAIL::bitcvt_f32_u32:
  case HSAIL::bitcvt_u32_f32:
  case HSAIL::bitcvt_f64_u64:
  case HSAIL::bitcvt_u64_f64:
    return MI->getOperand(0).getReg() == MI->getOperand(1).getReg();
  default:
    return false;
  }
}

void
HSAILAsmPrinter::emitMacroFunc(const MachineInstr *MI, raw_ostream &O)
{
  const char *name = "unknown";
  llvm::StringRef nameRef;
  nameRef = MI->getOperand(0).getGlobal()->getName();
  if (nameRef.startswith("barrier")) {
    name = nameRef.data();
    O << '\t';
    O << name;
    O << ';';
    return;
  }
}

// Targets can, or in the case of EmitInstruction, must implement these to
// customize output.
void
HSAILAsmPrinter::EmitInstruction(const MachineInstr *II)
{
  std::string FunStr;
  raw_string_ostream OFunStr(FunStr);
  formatted_raw_ostream O(OFunStr);

  if(II->getOpcode() == HSAIL::arg_scope_start) {
    O << "\t{ ";
  } else if (II->getOpcode() == HSAIL::arg_scope_end) {
    O << "\t} ";
  } else if (II->getOpcode() == HSAIL::target_call) {

    if (isMacroFunc(II)) {
      emitMacroFunc(II, O);
      O.flush();
      OutStreamer.EmitRawText(StringRef(FunStr));
      return;
    } 
    /*
else {
      const GlobalValue *gv = II->getOperand(0).getGlobal();        
      const PointerType *PTy = cast<PointerType>(gv->getType());
      const FunctionType *Fty = cast<FunctionType>(PTy->getElementType());
      Type *retTy = Fty->getReturnType();
      bool isKernel = isKernelFunc(gv->getName());

      if (retTy && (retTy->getTypeID() != Type::VoidTyID)) {
	EmitCallerReturn(retTy, isKernel, O);
      }
    }
    */
  } else if (isIdentityCopy(II)) {
    return;
  }

  printInstruction(II, O);  

  O.flush();
  OutStreamer.EmitRawText(StringRef(FunStr));
}

bool HSAILAsmPrinter::doFinalization(Module &M) {
  DwarfDebug *mDD = getDwarfDebug();
  if (mDD) {
    //NamedRegionTimer T(DbgTimerName, DWARFGroupName, TimePassesIsEnabled);
    mDD->endModule();
    delete mDD;
    setDwarfDebug(0);
  }

  // LLVM Bug 9761. Nothing should be emitted after EmitEndOfAsmFile()
  OutStreamer.FinishImpl();

  // Allow the target to emit any magic that it wants at the end of the file,
  // after everything else has gone out.
  EmitEndOfAsmFile(M);

  delete Mang; Mang = 0;
  MMI = 0;

  return false;
}

/// EmitStartOfAsmFile - This virtual method can be overridden by targets
/// that want to emit something at the start of their file.
void
HSAILAsmPrinter::EmitStartOfAsmFile(Module &M)
{
  std::string Str;
  raw_string_ostream OStr(Str);
  formatted_raw_ostream O(OStr);

  std::string mmode = Subtarget->is64Bit() ? "$large" : "$small";

  O << "version 1:0:";
  O << mmode << ";\n";
  O << "\n";
  O.flush();
  OutStreamer.EmitRawText(StringRef(Str));

  for (Module::const_global_iterator I = M.global_begin(), E = M.global_end();
       I != E; ++I)
	if (HSAILAS::PRIVATE_ADDRESS != I->getType()->getAddressSpace()
      && HSAILAS::GROUP_ADDRESS != I->getType()->getAddressSpace())
      EmitGlobalVariable(I);

  // Emit function declarations
  for (Module::const_iterator I = M.begin(), E = M.end(); I != E; ++I) {
    const Function &F = *I;
    // no declaration for kernel functions or HSAIL instrinsic
    if (!isKernelFunc(&F) && !isHSAILInstrinsic(F.getName())) { 
      if (F.isDeclaration()) {
        if (F.getLinkage() == GlobalValue::ExternalLinkage) {
          EmitFunctionLabel(F);
        }
      } else {
        EmitFunctionLabel(F);
      }
    }
  }
}

/// EmitEndOfAsmFile - This virtual method can be overridden by targets that
/// want to emit something at the end of their file.
void
HSAILAsmPrinter::EmitEndOfAsmFile(Module &M)
{
}


/// EmitFunctionBodyStart - Targets can override this to emit stuff before
/// the first basic block in the function.
void
HSAILAsmPrinter::EmitFunctionBodyStart()
{
  std::string FunStr;
  raw_string_ostream OFunStr(FunStr);
  formatted_raw_ostream O(OFunStr);
  O << "{\n";
  DwarfDebug *mDD = getDwarfDebug();
  if (mDD) {
    //NamedRegionTimer T(DbgTimerName, DWARFGroupName, TimePassesIsEnabled);
    mDD->beginFunction(MF);
  }

  const Function *F = MF->getFunction();
 
  bool isKernel = isKernelFunc(F);
  if (isKernel) {
    // emitting block data inside of kernel
    uint32_t id = 0;
    mMeta->setID(id);
    mMeta->setKernel(isKernel);
    ++mBuffer;
    mMeta->printHeader(mKernelName);
    mMeta->processArgMetadata(O, mBuffer, isKernel);
    mMeta->printMetaData(O, id, isKernel);
  }

  SmallPtrSet<const GlobalVariable*,16> thisFuncPvtVarsSet;
  SmallPtrSet<const GlobalVariable*,16> thisFuncGrpVarsSet;
  for (MachineFunction::const_iterator I = MF->begin(), E = MF->end(); 
                                                        I != E; ++I){
    for (MachineBasicBlock::const_iterator II = I->begin(), IE = I->end();
                                                         II != IE; ++II) {
      const MachineInstr *LastMI = II;
      for (unsigned int opNum = 0; opNum < LastMI->getNumOperands(); opNum++) {
        const MachineOperand &MO = LastMI->getOperand(opNum);
        if (MO.getType() == MachineOperand::MO_GlobalAddress){
          if (const GlobalVariable *GV =  dyn_cast<GlobalVariable>(MO.getGlobal())){
            if  (GV->getType()->getAddressSpace() == HSAILAS::PRIVATE_ADDRESS){
              thisFuncPvtVarsSet.insert(GV);
            }
            if  (GV->getType()->getAddressSpace() == HSAILAS::GROUP_ADDRESS){
              thisFuncGrpVarsSet.insert(GV);
            }
          }
        }
      }
    }
  }

  // Emit group variables declaration
  const Module* M = MF->getMMI().getModule();
  for (Module::const_global_iterator I = M->global_begin(), E = M->global_end();
    I != E; ++I){
      if ( HSAILAS::GROUP_ADDRESS == I->getType()->getAddressSpace()){
        const GlobalVariable *GV = I;
        if (thisFuncGrpVarsSet.count(GV)){
          DataLayout DL = getDataLayout();
          std::string str = "";
          O << "\t";
          printAlignTypeQualifier(GV->getType()->getElementType(), DL, O);
          O << getHSAILAddressSpace(GV)
            << getHSAILArgType(printGVType(GV->getType()->getElementType(), DL, str))
            << " %" << GV->getName() << str;
          if (GV->hasInitializer() && canInitHSAILAddressSpace(GV)) {
            O << " = ";
            printGVInitialValue(GV, (Constant *)GV->getInitializer(), O);
          }
          O << ";\n";
        }
      }
  }

  // Emit private variables declaration
  for (Module::const_global_iterator I = M->global_begin(), E = M->global_end();
                                     I != E; ++I){
    if ( HSAILAS::PRIVATE_ADDRESS == I->getType()->getAddressSpace()){
      const GlobalVariable *GV = I;
      if (thisFuncPvtVarsSet.count(GV)){
        StringRef GVname = GV->getName();
        bool bChangeName = false;
        // from 3.1 SmallVectorImpl constructor is a protected member
        SmallVector<StringRef, 10> NameParts;
        const char *tmp_opt_name = "tmp_opt_var";
        std::string str = "";
        if (GVname.empty()){
          str = tmp_opt_name;
          bChangeName = true;
        } else if(!isalpha(GVname[0]) && GVname[0] != '_'){
          str = tmp_opt_name;
          str.append(GVname);
          bChangeName = true;
        }
        { // replace all '.' with '_'
          size_t pos = 0;
          pos = str.find('.', pos);
          if (pos != std::string::npos) bChangeName = true;
          while (pos != std::string::npos){
            str.replace(pos++, 1, "_");
            pos = str.find('.', pos);
          }
        }
        if (bChangeName){
          (const_cast<GlobalVariable*>(GV))->setName(str);
        }

        DataLayout DL = getDataLayout();
        O << "\t";
        printAlignTypeQualifier(GV->getType()->getElementType(), DL, O);
        str = "";
        O << getHSAILAddressSpace(GV)
          << getHSAILArgType(printGVType(GV->getType()->getElementType(), DL, str))
          << " %" << GV->getName() << str;
        if (GV->hasInitializer() && canInitHSAILAddressSpace(GV)) {
          O << " = ";
          printGVInitialValue(GV, (Constant *)GV->getInitializer(), O);
        }
        O << ";\n";
      }
    }
  }

  FuncArgsStr = "";
  const MachineFrameInfo *MFI = MF->getFrameInfo();
  size_t stack_size = MFI->getOffsetAdjustment() + MFI->getStackSize();
  if (stack_size) {
    // dimension is in units of type length
    O <<  "\tspill_u32 %stack[" << (stack_size >> 2) << "];\n";
  }
  retValCounter = 0;
  paramCounter = 0;
  O << "@" << MF->getFunction()->getName() << "_entry:\n";
  // Allocate gcn region for gcn atomic counter, if required
  if (usesGCNAtomicCounter()) {
    O << "\tgcn_region_alloc 4;\n";
  }
  O.flush();
  OutStreamer.EmitRawText(StringRef(FunStr));
}

/// EmitFunctionBodyEnd - Targets can override this to emit stuff after
/// the last basic block in the function.
void
HSAILAsmPrinter::EmitFunctionBodyEnd()
{
  std::string FunStr;
  raw_string_ostream OFunStr(FunStr);
  formatted_raw_ostream O(OFunStr);
  O << "};\n";
  O.flush();
  OutStreamer.EmitRawText(StringRef(FunStr));
}

void
HSAILAsmPrinter::EmitFunctionReturn(
  Type* type,
  bool isKernel,
  formatted_raw_ostream &O)
{
  bool isVector = false;
  if (const VectorType *VT = dyn_cast<VectorType>(type)) {
    type = VT->getElementType();
    isVector = true;
  } 
  std::string argTypeStr =
    getHSAILArgType(type, isKernel? ARG_TYPE_KERNEL:ARG_TYPE_FUNC);
  
  O << argTypeStr << " %ret_r0";
  if(isVector)
    O << "[]";

  reg1Counter = 0;
  reg32Counter = 0;
  reg64Counter = 0;
}

void
HSAILAsmPrinter::EmitFunctionArgument(
  Type* type,
  bool isKernel,
  formatted_raw_ostream &O)
{   
  bool isVector = false;
  if (const VectorType *VT = dyn_cast<VectorType>(type)) {
    type = VT->getElementType();
    if(isKernel) {
      for(unsigned x = 0, y = VT->getNumElements(); x < y; ++x) {
	EmitFunctionArgument(type, isKernel, O);
	if( x != ( y - 1)) {
	  O << ", ";
	}
      }
    } else
      isVector = true;
  }

  // TODO_HSA: Need to emit alignment information.
  std::string argTypeStr = getHSAILArgType(type, 
      isKernel ? ARG_TYPE_KERNEL : ARG_TYPE_FUNC);
  O << argTypeStr << " ";

  std::stringstream stream;
  stream << "%arg_p" << paramCounter++;

  // for vector args, we ll use HSA IL array
  if(!isKernel && isVector)  
    stream << "[]";

  std::string ArgName = stream.str();
  O << ArgName;
}

/**
 * Sets up arguments prior to a call. Setup involves:
 * - declaration of variable
 * - initializion. For HSAIL, a memory op to parameter (Arg) memory.
 *
 * @param type
 * @param isKernel
 * @param tempPCounter
 * @param O
 */
/*
void
HSAILAsmPrinter::EmitFunctionParam(Type* type, bool isKernel,
    unsigned &tempPCounter,
    formatted_raw_ostream &O)
{
  if (const VectorType *VT = dyn_cast<VectorType>(type)) {
    type = VT->getElementType();
    for (unsigned x = 0, y = VT->getNumElements(); x < y; ++x) {
      EmitFunctionParam(type, isKernel, tempPCounter, O);
      // TODO_HSA:
    }
  } else {
    std::string paramTyStr =
      getHSAILArgType(type, isKernel? ARG_TYPE_KERNEL:ARG_TYPE_FUNC);
    std::stringstream stream;
    stream << "%param_val";
    stream << tempPCounter++;
    O << "\t" << paramTyStr << " " << stream.str() << ";\n";
    O << "\tst_" << paramTyStr << " " << getHSAILReg(type)
      << ", [" << stream.str() << "];\n";
  }
}
*/
void
HSAILAsmPrinter::EmitCallerReturn(
  Type* type,
  bool isKernel,
  /*  std::string &calleeStr, */
  formatted_raw_ostream &O)
{
  if (const VectorType *VT = dyn_cast<VectorType>(type)) {
    type = VT->getElementType();
    for (unsigned x = 0, y = VT->getNumElements(); x < y; ++x) {
      EmitCallerReturn(type, isKernel, O);
      // TODO_HSA: HSAIL admits one return value.
      break;
    }
  } else {
    std::string retValTyStr =
      getHSAILArgType(type, isKernel? ARG_TYPE_KERNEL:ARG_TYPE_FUNC);
    std::stringstream stream;
    stream << "%retV_r0";
    
    std::string RetValName = stream.str();
    O << "\t" << retValTyStr << " " << RetValName << ";\n";
  }
}

// Emit the function signature
void
HSAILAsmPrinter::EmitFunctionEntryLabel()
{
  std::string FunStr;
  raw_string_ostream OFunStr(FunStr);
  formatted_raw_ostream O(OFunStr);
  const Function *F = MF->getFunction();
  Type *retType = F->getReturnType();
  const FunctionType *funcType = F->getFunctionType();
  bool isKernel = isKernelFunc(F);
  O << (isKernel ? "kernel " : "function ");
  O << "&" << F->getName() << "(";
  // functions with kernel linkage cannot have output args
  if ( !isKernel ) {
    // TODO_HSA: Need "good" names for the formal arguments and returned value
    // TODO_HSA: Need to emit alignment information.
    if (retType && (retType->getTypeID() != Type::VoidTyID)) {
      EmitFunctionReturn(retType, isKernel, O);
    }
    O << ") (";
  }
  if (funcType) {
    // Loop through all of the parameters and emit the types and
    // corresponding names.
    reg1Counter = 0;
    reg32Counter = 0;
    reg64Counter = 0;
    paramCounter = 0;
    for (FunctionType::param_iterator pb = funcType->param_begin(),
        pe = funcType->param_end(); pb != pe; ++pb) {
      O << "\n";
      O << "\t";
      Type* type = *pb;
     
      const VectorType *VT = dyn_cast<VectorType>(type);
      if (isKernel && (VT != NULL)) {
        type = VT->getElementType();
        for (unsigned x = 0, y = VT->getNumElements(); x < y; ++x) {
          EmitFunctionArgument(type, isKernel, O);
          if (x < (y -1 )) {
            O << ", ";
	    O << "\n";
	    O << "\t";
	  }
        }
      } else {
      	EmitFunctionArgument(type, isKernel, O);
      }

      if ((pb + 1) !=  pe) {
	O << ", ";
      }
    }
  }
  O << ")\n";

  O.flush();
  OutStreamer.EmitRawText(StringRef(FunStr));
}

void
HSAILAsmPrinter::EmitMachineConstantPoolValue(MachineConstantPoolValue *MCPV)
{
  assert(!"When do we hit this?");
}

int
HSAILAsmPrinter::getHSAILParameterSize(Type* type, HSAIL_ARG_TYPE arg_type)
{
  int type_size = 0;

  switch (type->getTypeID()) {
  case Type::VoidTyID:
    break;
  case Type::FloatTyID:
    type_size = 4;
    break;
  case Type::DoubleTyID:
    type_size = 8;
    break;
  case Type::IntegerTyID:
    if (type->isIntegerTy(8)) {
      type_size = 1;
    } else if (type->isIntegerTy(16)) {
      type_size = 2;
    } else if (type->isIntegerTy(32)) {
      type_size = 4;
    } else if (type->isIntegerTy(64)) {
      type_size = 8;
    } else if (type->isIntegerTy(1)) {
      type_size = 1;
    } else {
      type->dump();
      assert(!"Found a case we don't handle!");
    }
    break;
  case Type::PointerTyID:
    if (Subtarget->is64Bit())
      type_size = 8;
    else
      type_size = 4;
    break;
  case Type::VectorTyID:
    type_size = getHSAILParameterSize(type->getScalarType(), arg_type);
    break;
  default:
    type->dump();
    assert(!"Found a case we don't handle!");
    break;
  }
  return type_size;
}

std::string
HSAILAsmPrinter::getHSAILArgType(Type* type, HSAIL_ARG_TYPE arg_type)
{
  std::string str = "";
  if (arg_type == ARG_TYPE_KERNEL) {
    str += "kernarg";
  } else if (arg_type == ARG_TYPE_FUNC) {
    str += "arg";
  }
  switch (type->getTypeID()) {
  case Type::VoidTyID:
    break;
  case Type::FloatTyID:
    str += "_f32";
    break;
  case Type::DoubleTyID:
    str += "_f64";
    break;
  case Type::IntegerTyID:
    if (type->isIntegerTy(8)) {
      str += "_u8";
    } else if (type->isIntegerTy(16)) {
      str += "_u16";
    } else if (type->isIntegerTy(32)) {
      str += "_u32";
    } else if (type->isIntegerTy(64)) {
      str += "_u64";
    } else if (type->isIntegerTy(1)) {
      str += "_b1";
    } else {
      type->dump();
      assert(!"Found a case we don't handle!");
    }
    break;
  case Type::PointerTyID:
  {
    OpaqueType OT = GetOpaqueType(type);
    if (IsImage(OT)) {
      // TODO_HSA: Need a way to discover read and write attributes of
      // an image in order to emit read-only/read-write type
      // accurately. At present, all images are defined as read-write.
      // This isn't wrong -- it's just not accurate.
#ifdef HSAIL_SPEC_UPDATE_1_0
            str += "_RWImg";
#else
            str += "_iob";
#endif
    } else if (OT == Sampler) {
#ifdef HSAIL_SPEC_UPDATE_1_0
        str += "_Samp";
#else
        str += "_sob";
#endif
    } else {
      if (Subtarget->is64Bit())
        str += "_u64";
      else
        str += "_u32";
    }
    break;
  }
  case Type::StructTyID:
    // Treat struct as array of bytes.
    str += "_u8";
    break;
  case Type::VectorTyID:
    str += getHSAILArgType(type->getScalarType(), arg_type);
    break;
  default:
    type->dump();
    assert(!"Found a case we don't handle!");
    break;
  }

  return str;
}

std::string
HSAILAsmPrinter::getHSAILReg(Type* type)
{
  std::stringstream stream;

  switch (type->getTypeID()) {
  case Type::VoidTyID:
    break;
  case Type::FloatTyID:
    stream << "$s" << reg32Counter++;
    break;
  case Type::DoubleTyID:
    stream << "$d" << reg64Counter++;
    break;
  case Type::IntegerTyID:
    if (type->isIntegerTy(1)) {
      stream << "$c" << reg1Counter++;
    } else if (type->isIntegerTy()
      && type->getScalarSizeInBits() <= 32){
        stream << "$s" << reg32Counter++;
    } else if (type->isIntegerTy()
      && type->getScalarSizeInBits() <= 64) {
        stream << "$d" << reg64Counter++;
    } else {
      type->dump();
      assert(!"Found a case we don't handle!");
    }
    break;
  case Type::PointerTyID:
    if (Subtarget->is64Bit())
      stream << "$d" << reg64Counter++;
    else
      stream << "$s" << reg32Counter++;
    break;
  default:
    type->dump();
    assert(!"Found a case we don't handle!");
    break;
};

  return stream.str();
}

/// isBlockOnlyReachableByFallthough - Return true if the basic block has
/// exactly one predecessor and the control transfer mechanism between
/// the predecessor and this block is a fall-through.
bool
HSAILAsmPrinter::isBlockOnlyReachableByFallthrough(const MachineBasicBlock *MBB)
                                                   const
{
  return AsmPrinter::isBlockOnlyReachableByFallthrough(MBB);
}

//===------------------------------------------------------------------===//
// Dwarf Emission Helper Routines
//===------------------------------------------------------------------===//

/// getDebugValueLocation - Get location information encoded by DBG_VALUE
/// operands.
MachineLocation
HSAILAsmPrinter::getDebugValueLocation(const MachineInstr *MI) const
{
  MachineLocation Location;
  assert(!"When do we hit this?");
  return Location;
}

/// getISAEncoding - Get the value for DW_AT_APPLE_isa. Zero if no isa
/// encoding specified.
unsigned
HSAILAsmPrinter::getISAEncoding()
{
  return 0;
}

//===------------------------------------------------------------------===//
// Inline Asm Support
//===------------------------------------------------------------------===//
// These are hooks that targets can override to implement inline asm
// support.  These should probably be moved out of AsmPrinter someday.

/** 
 * PrintSpecial - Print information related to the specified machine instr that
 * is independent of the operand, and may be independent of the instr itself.
 * This can be useful for portably encoding the comment character or other bits
 * of target-specific knowledge into the asmstrings.  The syntax used is
 * ${:comment}.  Targets can override this to add support for their own strange
 * codes.
 * 
 * @param MI 
 * @param OS 
 * @param Code 
 */

void
HSAILAsmPrinter::PrintSpecial(const MachineInstr *MI,
                              raw_ostream &OS,
                              const char *Code) const
{
  assert(!"When do we hit this?");
}

/** 
 * PrintAsmOperand - Print the specified operand of MI, an INLINEASM
 * instruction, using the specified assembler variant.  Targets should override
 * this to format as appropriate.  This method can return true if the operand is
 * erroneous.
 * 
 * 
 * @param MI 
 * @param OpNo 
 * @param AsmVariant 
 * @param ExtraCode 
 * @param OS 
 * 
 * @return 
 */

bool
HSAILAsmPrinter::PrintAsmOperand(const MachineInstr *MI,
                                 unsigned OpNo,
                                 unsigned AsmVariant,
                                 const char *ExtraCode,
                                 raw_ostream &OS)
{
  assert(!"When do we hit this?");
  return false;
}

/** 
 * PrintAsmMemoryOperand - Print the specified operand of MI, an INLINEASM
 * instruction, using the specified assembler variant as an address.
 * Targets should override this to format as appropriate.  This method can
 * return true if the operand is erroneous.
 * 
 * @param MI 
 * @param OpNo 
 * @param AsmVariant 
 * @param ExtraCode 
 * @param OS 
 * 
 * @return 
 */

bool
HSAILAsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                       unsigned OpNo,
                                       unsigned AsmVariant,
                                       const char *ExtraCode,
                                       raw_ostream &OS)
{
  assert(!"When do we hit this?");
  return false;
}

/** 
 * 
 * 
 * @param MI Machine instruction to print the operand of
 * @param opNum operand to print from the specified machine instruciton
 * @param OS The output stream for the operand
 * @brief Based on the register type, print out register specific information
 * and add swizzle information in the cases that require it
 */

void
HSAILAsmPrinter::printOperand(const MachineInstr *MI,
                              unsigned opNum,
                              raw_ostream &OS)
{
  const MachineOperand &MO = MI->getOperand(opNum);
  MCOperand MCOp;
  int64_t off;

  switch (MO.getType()) {
  default:
    OS << "<unknown operand type>"; break;
  case MachineOperand::MO_Register:
    if (MO.isReg()) {
      OS << getRegisterName(MO.getReg());
    } else {
      assert(0 && "Invalid Register type");
    }
    break;
  case MachineOperand::MO_Immediate:
    {
      const int op = MI->getOpcode();
      // Since DL has no FP literals we have to encode these as ints.
      // Code below checks insns for which that might be the case and
      // print this as a correct FP operand.
      switch (op) {
      case HSAIL::cmp_i1_eq_i1_ri:
      case HSAIL::cmp_i1_ne_i1_ri:
      case HSAIL::cmp_i32_eq_i1_ri:
      case HSAIL::cmp_i32_ne_i1_ri:
      case HSAIL::cmp_i64_eq_i1_ri:
      case HSAIL::cmp_i64_ne_i1_ri:
        // Print truncated literal value in control register comparisons.
        OS << (MO.getImm() == 0 ? "0" : "1");
        break;
      default:
        OS << MO.getImm();
      }
    }
    break;
  case MachineOperand::MO_FPImmediate:
    {
      const APFloat &flt = MO.getFPImm()->getValueAPF();
      const fltSemantics& flt_type = flt.getSemantics();
      uint64_t ui = flt.bitcastToAPInt().getZExtValue();
      if (&flt_type == &APFloat::IEEEdouble) {
        OS << "0D" << toHSAILHexString(ui, 16);
      } else if (&flt_type == &APFloat::IEEEsingle) {
        OS << "0F" << toHSAILHexString(ui, 8);
      } else {
        assert(0 && "Invalid literal/constant type");
      }
    }
    break;
  case MachineOperand::MO_MachineBasicBlock:
    OS << *MO.getMBB()->getSymbol();
    break;
  case MachineOperand::MO_GlobalAddress:
    {
      const GlobalValue *gv = MO.getGlobal();
      if (MI->getOpcode() == HSAIL::target_call) {
        const PointerType *PTy = cast<PointerType>(gv->getType());
        const FunctionType *Fty = cast<FunctionType>(PTy->getElementType());
        const Type *retTy = Fty->getReturnType();
        llvm::StringRef name = gv->getName();
        OS << "&" << name << " (";
        if (retTy->getTypeID() != Type::VoidTyID) {
          std::stringstream sstr;
          sstr << "%retV_r0";
          OS << sstr.str() << ") (";
        }
        for (unsigned i = 0; i < Fty->getNumParams(); ++i) {
          if (i != 0)
            OS << ", ";
          std::stringstream sstr;
          sstr << "%param_p" << i;
          OS << sstr.str();
        }
        OS << ")";
      } else if (
                 MI->getOpcode() == HSAIL::lda_flat_addr32 ||
                 MI->getOpcode() == HSAIL::ldas_global_addr32 ||
                 MI->getOpcode() == HSAIL::ldas_constant_addr32 ||
                 MI->getOpcode() == HSAIL::ldas_group_addr32 ||
                 MI->getOpcode() == HSAIL::ldas_private_addr32 ||
                 MI->getOpcode() == HSAIL::lda_flat_addr64 ||
                 MI->getOpcode() == HSAIL::ldas_global_addr64 ||
                 MI->getOpcode() == HSAIL::ldas_constant_addr64 ||
                 MI->getOpcode() == HSAIL::ldas_group_addr64 ||
                 MI->getOpcode() == HSAIL::ldas_private_addr64 ||
                 MI->getOpcode() == HSAIL::ldas_global_flat_addr32 ||
                 MI->getOpcode() == HSAIL::ldas_group_flat_addr32 ||
                 MI->getOpcode() == HSAIL::ldas_private_flat_addr32 ||
                 MI->getOpcode() == HSAIL::ldas_global_flat_addr64 ||
                 MI->getOpcode() == HSAIL::ldas_group_flat_addr64 ||
                 MI->getOpcode() == HSAIL::ldas_private_flat_addr64
                 ) {
        OS << ((HSAILAS::PRIVATE_ADDRESS == gv->getType()->getAddressSpace()
                || HSAILAS::GROUP_ADDRESS == gv->getType()->getAddressSpace())
                ? "[%" : "[&") << gv->getName() << "]";
        off = MO.getOffset();
        if (off != 0) OS << "[" << off << "]";
      } else {
        OS << ((HSAILAS::PRIVATE_ADDRESS == gv->getType()->getAddressSpace()
                || HSAILAS::GROUP_ADDRESS == gv->getType()->getAddressSpace())
                ? "[%" : "[&") << gv->getName() << "]";
        off = MO.getOffset();
        if (off != 0) OS << "[" << off << "]";
      }
    }
    break;
  case MachineOperand::MO_ExternalSymbol:
    if (
        MI->getOpcode() == HSAIL::lda_flat_addr32 ||
        MI->getOpcode() == HSAIL::ldas_global_addr32 ||
        MI->getOpcode() == HSAIL::ldas_constant_addr32 ||
        MI->getOpcode() == HSAIL::ldas_group_addr32 ||
        MI->getOpcode() == HSAIL::ldas_private_addr32 ||
        MI->getOpcode() == HSAIL::lda_flat_addr64 ||
        MI->getOpcode() == HSAIL::ldas_global_addr64 ||
        MI->getOpcode() == HSAIL::ldas_constant_addr64 ||
        MI->getOpcode() == HSAIL::ldas_group_addr64 ||
        MI->getOpcode() == HSAIL::ldas_private_addr64 ||
        MI->getOpcode() == HSAIL::ldas_global_flat_addr32 ||
        MI->getOpcode() == HSAIL::ldas_group_flat_addr32 ||
        MI->getOpcode() == HSAIL::ldas_private_flat_addr32 ||
        MI->getOpcode() == HSAIL::ldas_global_flat_addr64 ||
        MI->getOpcode() == HSAIL::ldas_group_flat_addr64 ||
        MI->getOpcode() == HSAIL::ldas_private_flat_addr64
        ) {
      OS << "[%" << MO.getSymbolName() << "]";
    } else {
      OS << "%" << MO.getSymbolName();
    }
    off = MO.getOffset();
    if (off != 0) OS << "[" << off << "]";
    break;
  case MachineOperand::MO_ConstantPoolIndex:
    assert(!"When do we hit this?");
    break;
  }
}

/** 
 * 
 * 
 * @param MI Machine instruction to print memory operand of 
 * @param opNum operand to print from the specified machine instrucion
 * @param OS Modifier optional modifier for the memory operand
 * @param Modifier Print the memory operand based on the register type
 */

void
HSAILAsmPrinter::printMemOperand(const MachineInstr *MI,
                                 unsigned opNum,
                                 raw_ostream &OS,
                                 const char *Modifier)
{
  // Note: this method does not emit correct HSAIL code.
  //       it is only suitable for human reading in debug purposes
  assert(opNum + 2 < MI->getNumOperands());
  const MachineOperand &base = MI->getOperand(opNum),
    &reg  = MI->getOperand(opNum+1),
    &offset_op  = MI->getOperand(opNum+2);

  if (base.isGlobal())
  {
    const GlobalValue *gv = base.getGlobal();
    std::stringstream ss;
    const unsigned AddrSpace = gv->getType()->getAddressSpace();

    OS << "%" << gv->getName().str();
  }
  else if (base.isImm())
    OS << "%" << base.getImm();

  bool need_close_bracket = false;
  bool need_plus_sign = false;
  if (reg.isReg() && reg.getReg() != 0)
  {
    OS << "[" << getRegisterName(reg.getReg());
    need_close_bracket = true;
    need_plus_sign = true;
  }

  if (offset_op.getImm() != 0)
  {
    if (!need_close_bracket)
      OS << "[";
    if (need_plus_sign)
      OS << "+ ";
    OS << offset_op.getImm();
    need_close_bracket = true;
  }

  if (need_close_bracket)
    OS << "]";
}

void
HSAILAsmPrinter::printImageMemOperand(const MachineInstr *MI,
                                      unsigned opNum,
                                      raw_ostream &OS,
                                      const char *Modifier)
{
  const MachineOperand &MO = MI->getOperand(opNum);

  switch (MO.getType()) {
  default:
    assert(0 && "Unknown image operand type");
    break;
  case MachineOperand::MO_Immediate:
    unsigned index = MO.getImm();
    // Indices for image_t and sampler_t args are biased, so now we un-bias them.
    // Note that the biased values rely on biasing performed by 
    // HSAILPropagateImageOperands and HSAILISelLowering::LowerFormalArguments.
    if (index >= IMAGE_ARG_BIAS) {
      index -= IMAGE_ARG_BIAS;
      // This is an image arg
      std::string sym = Subtarget->getImageHandles()->getImageSymbol(index);
      assert(!sym.empty() && "Expected symbol here");
      OS << "%" << sym.c_str();
    } else {
      // This is an initilized sampler
      HSAILSamplerHandle* sampler = 
        Subtarget->getImageHandles()->getSamplerHandle(index);
      assert(sampler && "Invalid sampler handle");
      std::string sym = sampler->getSym();
      assert(!sym.empty() && "Expected symbol here");
      OS << "&" << sym.c_str();
    }
    //OS << MO.getImm();
    break;
  }
}

// OCL sampler inits
//
// from macros in FE: hsa/compiler/edg/src/amd_ocl_sys_predef.c
// also see: hsa/library/x86/common/src/image.h

// normalized coords 
#define CLK_NORMALIZED_COORDS_TRUE 1
#define CLK_NORMALIZED_COORDS_FALSE 0
// addressing mode 
#define CLK_ADDRESS_NONE 0
#define CLK_ADDRESS_REPEAT 2
#define CLK_ADDRESS_CLAMP_TO_EDGE 4
#define CLK_ADDRESS_CLAMP 6
#define CLK_ADDRESS_MIRRORED_REPEAT 8
// filter mode
#define CLK_FILTER_NEAREST 16
#define CLK_FILTER_LINEAR 32

// notes on address modes

// HSAIL APR13
// coord: 
//   normalized (coordinates are in range [0.0 to 1.0])
//   unnormalized (coordinates are integers)
// filter: nearest or linear
// address mode for [U,V,W] component: wrap, clamp, mirror, mirroronce, border

// OpenCL
// Specifies the image addressing-mode i.e. how out-ofrange
// image coordinates are handled. This must be a
// literal value and can be one of the following predefined
// enums:

// CLK_ADDRESS_MIRRORED_REPEAT - Flip the image coordinate at every
// integer junction. This addressing mode can only be used with
// normalized coordinates. If normalized coordinates are not used,
// this addressing mode may generate image coordinates that are
// undefined.

// CLK_ADDRESS_REPEAT – out-of-range image coordinates are wrapped
// to the valid range. This addressing mode can only be used with
// normalized coordinates. If normalized coordinates are not used,
// this addressing mode may generate image coordinates that are
// undefined.

// CLK_ADDRESS_CLAMP_TO_EDGE – out-of-range image coordinates are
// clamped to the extent.

// CLK_ADDRESS_CLAMP – out-of-range image coordinates will return
// a border color.  This is similar to the
// GL_ADDRESS_CLAMP_TO_BORDER addressing mode.

// CLK_ADDRESS_NONE – for this addressing mode the programmer
// guarantees that the image coordinates used to sample elements of
// the image refer to a location inside the image; otherwise the
// results are undefined.

// For 1D and 2D image arrays, the addressing mode applies only to
// the x and (x, y) coordinates. The addressing mode for the
// coordinate which specifies the array index is always
// CLK_ADDRESS_CLAMP_TO_EDGE.

/**
 * Emit set of constant sampler initializations collected during instruction
 * lowering.
 * 
 */

void
HSAILAsmPrinter::EmitSamplerDefs() {
  std::string Str;
  raw_string_ostream OStr(Str);
  formatted_raw_ostream O(OStr);
  bool emitted = false;

  HSAILImageHandles* handles = Subtarget->getImageHandles();
  SmallVector<HSAILSamplerHandle*, 16> samplers = 
    handles->getSamplerHandles();

  // Emit global sampler defs
  for (unsigned i = 0; i < samplers.size(); i++) {
    // All sampler defs (samplers with initializers) are global, so we emit
    // them only once.
    if (samplers[i]->isEmitted()) {
      continue;
    }

    if (samplers[i]->getVal() != 0) {
#ifdef HSAIL_SPEC_UPDATE_1_0
      if (samplers[i]->isRO()) {
        O << "readonly_samp &";
      } else {
        O << "global_samp &";
      }
#else
      O <<  "global_sob &";
#endif
      O << samplers[i]->getSym().c_str();
      O << " = {";
      int ocl_init = handles->getSamplerValue(i);

      if (ocl_init & 0x1) { // CLK_NORMALIZED_COORDS_TRUE
        O << "coord = normalized";
      } else { // CLK_NORMALIZED_COORDS_FALSE
        O << "coord = unnormalized";
      }
      O << ", ";
      if (ocl_init & 0x10) { // CLK_FILTER_NEAREST
        O << "filter = nearest";
      } else if (ocl_init & 0x20) { // CLK_FILTER_LINEAR
        O << "filter = linear";
      } 
      // TODO_HSA: assert if filter mode is not chosen
      O << ", ";
      // HSA_TODO: verify OCL -> HSAIL mapping for address modes
      if ((ocl_init & 0x4) && (ocl_init & 0x2)) { // CLK_ADDRESS_CLAMP
        O << "addressU = border, addressV = border, addressW = border";
      } else if (ocl_init & 0x2) { // CLK_ADDRESS_REPEAT
        O << "addressU = wrap, addressV = wrap, addressW = wrap";
      } else if (ocl_init & 0x4) { // CLK_ADDRESS_CLAMP_TO_EDGE
        O << "addressU = clamp, addressV = clamp, addressW = clamp";
      } else if (ocl_init & 0x8 ) { // CLK_ADDRESS_MIRRORED_REPEAT
        O << "addressU = mirror, addressV = mirror, addressW = mirror";
      } else { // CLK_ADDRESS_NONE
        O << "addressU = mirroronce, addressV = mirroronce, addressW = mirroronce";
      }
      O << "};\n";
      //samplers[i]->setGlobal();
      samplers[i]->setEmitted();
    }
    emitted = true;
  }
  if (emitted) {
    O << "\n";
    O.flush();
    OutStreamer.EmitRawText(StringRef(Str));
  }
}

// Scan instrs for use of gcn atomic counter.
bool HSAILAsmPrinter::usesGCNAtomicCounter(void) {
  // TODO_HSA: This introduces another pass over all the instrs in the
  // kernel. Need to find a more efficient way to get this info.
  for (MachineFunction::const_iterator I = MF->begin(), E = MF->end();
       I != E; ++I) {
    for (MachineBasicBlock::const_iterator II = I->begin(), IE = I->end();
         II != IE; ++II) {
      switch (II->getOpcode()) {
      default:
        continue;
      case HSAIL::gcn_atomic_append:
      case HSAIL::gcn_atomic_consume:
        return true;
      }
    }
  }
  return false;
}

//===----------------------------------------------------------------------===//
// Target Registry Stuff
//===----------------------------------------------------------------------===//
static MCInstPrinter *createHSAILMCInstPrinter(const Target &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI,
                                               const MCSubtargetInfo &STI) {
  return new HSAILInstPrinter(MAI);
}

extern cl::opt<bool> disableBrigLowering;

// Force static initialization.
extern "C" void LLVMInitializeHSAILAsmPrinter() {
  // HSAIL asm printer and BRIG asm printer are mutually exclusive, only one can
  // run
  if (!disableBrigLowering)
    return;

  RegisterAsmPrinter<HSAILAsmPrinter> X(TheHSAIL_32Target);
  RegisterAsmPrinter<HSAILAsmPrinter> Y(TheHSAIL_64Target);

  TargetRegistry::RegisterMCInstPrinter(TheHSAIL_32Target,
                                        createHSAILMCInstPrinter);
  TargetRegistry::RegisterMCInstPrinter(TheHSAIL_64Target,
                                        createHSAILMCInstPrinter);
}
