//===----- AMDSPIRMutator.h - helper objects that mutate SPIR types -------===//
//===----------------------------------------------------------------------===//
// Mutate spir data types into device specific LLVM datatypes.
//
//===----------------------------------------------------------------------===//
#ifndef _AMD_SPIR_MUTATOR_H_
#define _AMD_SPIR_MUTATOR_H_
#include "llvm/Value.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/GlobalValue.h"
#include "llvm/InstrTypes.h"
#include "llvm/Instructions.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include <set>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <functional>
using namespace llvm;

namespace spir {
  
  // This struct mutates SPIR types into non-SPIR types.
  struct ValueMutator {
    public:
      // Constructor that takes a lookup table of Type pointers where
      // the table has even entries as the key and odd entries as the value
      // to mutate to.
      ValueMutator(Type **tbl);

      // Copy constructor so that the object can be used by STL functions.
      ValueMutator(const ValueMutator &tm);

      virtual void operator()(Value *V);
      virtual void operator()(Value &V);
    protected:
      Type **type_table;
      void mutate(Value* V);
      void mutate(CallInst *CI);
      bool mDebug;

  }; // end ValueMutator class

  // The type mutator classes mutates the types of specific values
  // if they match a type in the table that is passed in as
  // the second operand.
  template <typename T>
    struct TypeMutator : protected ValueMutator {
      TypeMutator(); // Do Not Implement.
      public:
      TypeMutator(Type **tbl) : ValueMutator(tbl) {}
      void operator()(T *V) {
        mutate(V);
      }
      void operator()(T &F) {
        operator()(&F);
      }
    }; // end generic TypeMutator class

  // Specialize the call instruction version so that we can
  // handle certain functions and overload the return value.
  template <>
    struct TypeMutator<Instruction> : protected ValueMutator {
      TypeMutator(); // Do Not Implement.
      private:
      std::set<Instruction*> deadInsts;
      bool typesMatch(LLVMContext &ctx, Type *type, uint32_t &curIdx);
      void clear();
      public:
      TypeMutator(Type **tbl) : ValueMutator(tbl) {}
      // overload the copy constructor so that we don't duplicate the deadInsts
      // data structure.
      TypeMutator(const TypeMutator &tm) : ValueMutator(tm) {}
      virtual ~TypeMutator() {
        clear();
      }
      // A mutate that is unique for call instructions.
      void mutate(CallInst *CI);

      void operator()(Instruction *CI) {
        if (dyn_cast<CallInst>(CI)) {
          mutate(dyn_cast<CallInst>(CI));
        } else {
          ValueMutator::mutate(CI);
        }
      }
      void operator()(Instruction &F) {
        operator()(&F);
      }

    }; // end Function Specialized TypeMutator class

  // Specialize the function version so that we can
  // iterator over all of the instructions.
  template <>
    struct TypeMutator<Function> : protected ValueMutator {
      TypeMutator(); // Do Not Implement.
      public:
      TypeMutator(Type **tbl) : ValueMutator(tbl) {}
      void operator()(Function *F) {
        mutate(F);
        TypeMutator<Instruction> inst_tm(type_table);
        // iterator over all of the instructions in a function.
        std::for_each(inst_begin(F), inst_end(F), inst_tm);
      }
      void operator()(Function &F) {
        operator()(&F);
      }
    }; // end Function Specialized TypeMutator class
}; // end spir namespace

#endif // _AMD_SPIR_MUTATOR_H_
