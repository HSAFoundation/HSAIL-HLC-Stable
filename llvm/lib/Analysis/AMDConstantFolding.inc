#ifndef AMD_CONSTANT_FOLDING_H
#define AMD_CONSTANT_FOLDING_H

static Constant *ConstantFoldFP(double (*NativeFP)(double), double V,
                                Type *Ty);
static Constant *ConstantFoldBinaryFP(double (*NativeFP)(double, double),
                                      double V, double W, Type *Ty);

enum IntrKind {
  IK_GENERAL,
  IK_AMDIL,
  IK_HSAIL,
  IK_NATIVE,
  IK_GCN
};

/// Checks if name formed as intrinsic and strips and prefixes
/// Returns VoidTyID if invalid
static const Type::TypeID GetStrippedIntrinsicName(StringRef& Name, IntrKind& Ik)
{
    Type::TypeID Ty;
    Ik = IK_GENERAL;
    if (Name.startswith("__")) {
      Name = Name.substr(2);
      if (Name.startswith("amdil_")) {
        Ik = IK_AMDIL;
        Name = Name.substr(6);
      } else if (Name.startswith("hsail_")) {
        Ik = IK_HSAIL;
        Name = Name.substr(6);
      } else if (Name.startswith("native_")) {
        Ik = IK_NATIVE;
        Name = Name.substr(7);
      } else if (Name.startswith("gcn_")) {
        Ik = IK_GCN;
        Name = Name.substr(4);
      }
      if (Name.endswith("_f32")) Ty = Type::FloatTyID;
      else if (Name.endswith("_f64")) Ty = Type::DoubleTyID;
      else return Type::VoidTyID;
      Name = Name.substr(0, Name.size() - 4);
      return Ty;
    }
    return Type::VoidTyID;
}

/// AMDCanConstantFoldCallTo - canConstantFoldCallTo() helper for
/// AMD specific functions.
/// It will also remove any specifc AMD name prefixes on return.
static bool AMDCanConstantFoldCallTo(StringRef Name) {
  IntrKind Ik;
  GetStrippedIntrinsicName(Name, Ik);

  switch (Name[0]) {
  case 'a':
    return Name == "abs";
  case 'c':
    return (Name == "class" && Ik != IK_GENERAL);
  case 'f':
    return Name == "fldexp" ||
           ((Name == "fma"  || Name == "ftz") && Ik != IK_GENERAL);
  case 'i':
    return Name == "isnan";
  case 'l':
    return Name == "ldexp";
  case 'm':
    return Name == "min" || Name == "max" ||
       ((Name == "mad" && Ik != IK_GENERAL) ||
       ((Name == "min_s32" || Name == "min_s64" ||
         Name == "max_s32" || Name == "max_s64" ||
         Name == "min_u32" || Name == "min_u64" ||
         Name == "max_u32" || Name == "max_u64") && Ik == IK_HSAIL));
  case 'n':
    return (Name == "nfma" || Name == "ncos" || Name == "nsin" ||
      Name == "nsqrt" || Name == "nlog2" || Name == "nexp2" || Name == "nrcp")
      && Ik == IK_HSAIL;
  }
  return false;
}

static Constant *ConstantFoldTernaryFP(double (*NativeFP)(double, double,
                                                          double),
                                      double V, double W, double Z,
                                      const Type *Ty) {
  sys::llvm_fenv_clearexcept();
  V = NativeFP(V, W, Z);
  if (sys::llvm_fenv_testexcept()) {
    sys::llvm_fenv_clearexcept();
    return 0;
  }

  if (Ty->isFloatTy())
    return ConstantFP::get(Ty->getContext(), APFloat((float)V));
  if (Ty->isDoubleTy())
    return ConstantFP::get(Ty->getContext(), APFloat(V));
  llvm_unreachable("Can only constant fold float/double");
  return 0; // dummy return to suppress warning
}

/// Native functions absent from standard library
namespace llvm {
static double mad(double V, double W, double Z)
{
  return V * W + Z;
}

static double ftz(double V)
{
  return V + 0.0;
}

static double min(double V, double W)
{
  return (V > W) ? W : V;
}

static double max(double V, double W)
{
  return (V < W) ? W : V;
}

static double rcp(double V)
{
  return 1.0 / V;
}

enum {
  HSAIL_SNAN = 0x001,
  HSAIL_QNAN = 0x002,
  HSAIL_NINF = 0x004,
  HSAIL_NNOR = 0x008,
  HSAIL_NSUB = 0x010,
  HSAIL_NZER = 0x020,
  HSAIL_PZER = 0x040,
  HSAIL_PSUB = 0x080,
  HSAIL_PNOR = 0x100,
  HSAIL_PINF = 0x200
};

static int hsail_classify(const APFloat& V, int test)
{
  APFloat::fltCategory category = V.getCategory();
  if ((test & (HSAIL_SNAN | HSAIL_QNAN)) && (category == APFloat::fcNaN)) {
    int64_t i = V.bitcastToAPInt().getSExtValue();
    const fltSemantics* pSem = &V.getSemantics();
    int qnan = 1;
    if (pSem == &APFloat::IEEEdouble) qnan = (i & 0x8000000000000LL) ? 1 : 0;
    if (pSem == &APFloat::IEEEdouble) qnan = (i & 0x400000LL) ? 1 : 0;
    if (pSem == &APFloat::IEEEhalf)   qnan = (i & 0x200LL) ? 1 : 0;
    if ((test & HSAIL_QNAN) && qnan )  return 1;
    if ((test & HSAIL_SNAN) && !qnan ) return 1;
  }
  if ((test & HSAIL_NINF) && V.isNegative() && (category == APFloat::fcInfinity))
    return 1;
  if ((test & HSAIL_NNOR) && V.isNegative() && (category == APFloat::fcNormal))
    return 1;
  if ((test & HSAIL_NSUB) && V.isNegative() && (category != APFloat::fcNormal))
    return 1;
  if ((test & HSAIL_NZER) && V.isNegZero())
    return 1;
  if ((test & HSAIL_PZER) && V.isPosZero())
    return 1;
  if ((test & HSAIL_PSUB) && !V.isNegative() && (category != APFloat::fcNormal))
    return 1;
  if ((test & HSAIL_PNOR) && !V.isNegative() && (category == APFloat::fcNormal))
    return 1;
  if ((test & HSAIL_PINF) && !V.isNegative() && (category == APFloat::fcInfinity))
    return 1;
  return 0;
}
} // namespace llvm

/// AMD Specific call constant folding
/// Note, the call will strip any AMD specific prefixes from the Name,
/// so further general constant folding will have chance to process it as well.
static Constant *
AMDConstantFoldCall(Function *F, StringRef Name, ArrayRef<Constant *> Operands) {
  IntrKind Ik;
  GetStrippedIntrinsicName(Name, Ik);
  Type *Ty = F->getReturnType();

  if (Operands.size() == 1) {
    if (ConstantFP *Op = dyn_cast<ConstantFP>(Operands[0])) {
      if (Ty->isIntegerTy()) {
        if (Name == "isnan")
          return ConstantInt::get(Ty, Op->getValueAPF().isNaN() ? 1 : 0, true);
      }

      if (!Ty->isFloatTy() && !Ty->isDoubleTy())
        return 0;

      if (Op->getValueAPF().isNaN() || Op->getValueAPF().isInfinity())
        return 0;

      double V = Ty->isFloatTy() ? (double)Op->getValueAPF().convertToFloat() :
                                     Op->getValueAPF().convertToDouble();
      switch (Name[0]) {
      case 'a':
        if (Name == "abs")
          return ConstantFoldFP(fabs, V, Ty);
        break;
      case 'f':
        if (Ik != IK_GENERAL && Name == "ftz")
          return ConstantFoldFP(ftz, V, Ty);
        break;
      case 'n':
        if (Ik == IK_HSAIL) {
          if (Name == "ncos")
            return ConstantFoldFP(cos, V, Ty);
          else if (Name == "nsin")
            return ConstantFoldFP(sin, V, Ty);
          else if (Name == "nlog2" && V > 0)
            return ConstantFoldFP(llvm::log2, V, Ty);
          else if (Name == "nexp2")
            return ConstantFoldBinaryFP(pow, 2.0, V, Ty);
          else if (Name == "nsqrt" && V >= 0)
            return ConstantFoldFP(sqrt, V, Ty);
          else if (Name == "nrcp" && V != 0.0)
            return ConstantFoldFP(rcp, V, Ty);
        }
        break;
      default:
        break;
      }
      return 0;
    }
    return 0;
  }

  if (Operands.size() == 2) {
    if (ConstantFP *Op1 = dyn_cast<ConstantFP>(Operands[0])) {
      if (Ty->isIntegerTy()) {
        if (ConstantInt *Op2 = dyn_cast<ConstantInt>(Operands[1])) {
          if(Ik != IK_GENERAL && Name == "class")
            return ConstantInt::get(Ty, hsail_classify(Op1->getValueAPF(),
                                     int(Op2->getLimitedValue())), true);
        }
      }
      if (!Ty->isFloatTy() && !Ty->isDoubleTy())
        return 0;
      double Op1V = Ty->isFloatTy() ?
                      (double)Op1->getValueAPF().convertToFloat() :
                      Op1->getValueAPF().convertToDouble();
      if (ConstantFP *Op2 = dyn_cast<ConstantFP>(Operands[1])) {
        if (Op2->getType() != Op1->getType())
          return 0;

        double Op2V = Ty->isFloatTy() ?
                      (double)Op2->getValueAPF().convertToFloat():
                      Op2->getValueAPF().convertToDouble();

        if (Name == "min")
          return ConstantFoldBinaryFP(llvm::min, Op1V, Op2V, Ty);
        if (Name == "max")
          return ConstantFoldBinaryFP(llvm::max, Op1V, Op2V, Ty);
      } else if (ConstantInt *Op2C = dyn_cast<ConstantInt>(Operands[1])) {
        if (Name == "ldexp" || Name == "fldexp") {
          if (Ty->isFloatTy()) {
            return ConstantFP::get(F->getContext(),
                                   APFloat((float)ldexp((float)Op1V,
                                                (int)Op2C->getZExtValue())));
          } else if (Ty->isDoubleTy()) {
            return ConstantFP::get(F->getContext(),
                                 APFloat((double)ldexp((double)Op1V,
                                                 (int)Op2C->getZExtValue())));
          }
        }
      }
      return 0;
    }

    if (ConstantInt *Op1 = dyn_cast<ConstantInt>(Operands[0])) {
      if (ConstantInt *Op2 = dyn_cast<ConstantInt>(Operands[1])) {
        if (Ik == IK_HSAIL) {
          switch(Name[0]) {
          case 'm':
            if(Name == "min_s32" || Name == "min_s64") {
              int64_t v1 = (int64_t)Op1->getSExtValue();
              int64_t v2 = (int64_t)Op2->getSExtValue();
              return ConstantInt::get(Ty, (v1 > v2) ? v2 : v1, true);
            }
            if(Name == "min_u32" || Name == "min_u64") {
              uint64_t v1 = Op1->getZExtValue();
              uint64_t v2 = Op2->getZExtValue();
              return ConstantInt::get(Ty, (v1 > v2) ? v2 : v1, false);
            }
            if(Name == "max_s32" || Name == "max_s64") {
              int64_t v1 = (int64_t)Op1->getSExtValue();
              int64_t v2 = (int64_t)Op2->getSExtValue();
              return ConstantInt::get(Ty, (v1 > v2) ? v1 : v2, true);
            }
            if(Name == "max_u32" || Name == "max_u64") {
              uint64_t v1 = Op1->getZExtValue();
              uint64_t v2 = Op2->getZExtValue();
              return ConstantInt::get(Ty, (v1 > v2) ? v1 : v2, false);
            }
            break;
          default:
            break;
          }
        }
      }
      return 0;
    }
    return 0;
  }

  if (Operands.size() == 3) {
    if (ConstantFP *Op1 = dyn_cast<ConstantFP>(Operands[0])) {
      if (!Ty->isFloatTy() && !Ty->isDoubleTy())
        return 0;
      double Op1V = Ty->isFloatTy() ?
                      (double)Op1->getValueAPF().convertToFloat() :
                      Op1->getValueAPF().convertToDouble();
      if (ConstantFP *Op2 = dyn_cast<ConstantFP>(Operands[1])) {
        if (Op2->getType() != Op1->getType())
          return 0;

        double Op2V = Ty->isFloatTy() ?
                      (double)Op2->getValueAPF().convertToFloat():
                      Op2->getValueAPF().convertToDouble();

        if (ConstantFP *Op3 = dyn_cast<ConstantFP>(Operands[2])) {
          if (Op3->getType() != Op1->getType())
            return 0;

          double Op3V = Ty->isFloatTy() ?
                        (double)Op3->getValueAPF().convertToFloat():
                        Op3->getValueAPF().convertToDouble();

          if (((Name == "mad" || Name == "fma") && (Ik != IK_GENERAL)) ||
              ((Name == "nfma") && (Ik == IK_HSAIL)))
            return ConstantFoldTernaryFP(mad, Op1V, Op2V, Op3V, Ty);
        }
        return 0;
      }
      return 0;
    }
    return 0;
  }

  return 0;
}
#endif // AMD_CONSTANT_FOLDING_H
