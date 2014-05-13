#ifndef DAG_WALKER_H
#define DAG_WALKER_H

#include "../../include/llvm/TableGen/Record.h"
#include "../../include/llvm/CodeGen/ISDOpcodes.h"
#include "../lib/Target/HSAIL/HSAILISDNodes.h"
#include "CodeGenInstruction.h"
#include <sstream>
#ifdef ANDROID
#include <stdio.h>
#endif

using namespace llvm;
using namespace llvm::ISD;
using namespace llvm::HSAILISD;

class DAGWalker {

public:
  DAGWalker(std::ostringstream & O, int vec_size = 1):
      printer(O),
      m_state(PS_START),
      m_offset(0),
      m_opNum(0),
      m_vec_size(vec_size)
      {
        Node["ISD::LOAD"] = LOAD;
        Node["ISD::STORE"] = STORE;
        Node["ISD::ADD"] = ADD;
        Node["ISD::BRCOND"] = BRCOND;
        Node["ISD::BR"] = BR;
        Node["ISD::BasicBlock"] = BasicBlock;
        Node["ISD::Constant"] = Constant;
        Node["ISD::ConstantFP"] = ConstantFP;
        Node["ISD::ExternalSymbol"] = ExternalSymbol;
        Node["ISD::GlobalAddress"] = GlobalAddress;
        Node["ISD::TargetExternalSymbol"] = TargetExternalSymbol;
        Node["ISD::TargetGlobalAddress"] = TargetGlobalAddress;
        Node["REGISTER"] = Register;
        Node["ValueType"] = VALUETYPE;

        Node["HSAILISD::LOAD_PARAM_U8"] = LOAD_PARAM_U8;
        Node["HSAILISD::LOAD_PARAM_U16"] =  LOAD_PARAM_U16;
        Node["HSAILISD::LOAD_PARAM_IMAGE"] = LOAD_PARAM_IMAGE;
        Node["HSAILISD::LOAD_PARAM_SAMP"] = LOAD_PARAM_SAMP;
        Node["HSAILISD::LOAD_PARAM"] = LOAD_PARAM;
        Node["HSAILISD::LOAD_PARAM_PTR"] = LOAD_PARAM_PTR;
        Node["HSAILISD::LOAD_PARAM_PTR_STRUCT_BY_VAL"] = LOAD_PARAM_PTR_STRUCT_BY_VAL;
        Node["HSAILISD::LDA_FLAT"] = LDA_FLAT;
        Node["HSAILISD::LDA_PRIVATE"] = LDA_PRIVATE;
        Node["HSAILISD::LDA_GLOBAL"] = LDA_GLOBAL;
        Node["HSAILISD::LDA_GROUP"] = LDA_GROUP;
        Node["HSAILISD::LDA_READONLY"] = LDA_READONLY;
        Node["HSAILISD::LD_SCALAR_RET_U8"] = LD_SCALAR_RET_U8;
        Node["HSAILISD::LD_SCALAR_RET_U16"] = LD_SCALAR_RET_U16;
        Node["HSAILISD::LD_SCALAR_RET"] = LD_SCALAR_RET;
        Node["HSAILISD::ST_SCALAR_ARG_U8"] = ST_SCALAR_ARG_U8;
        Node["HSAILISD::ST_SCALAR_ARG_U16"] = ST_SCALAR_ARG_U16;
        Node["HSAILISD::ST_SCALAR_ARG"] = ST_SCALAR_ARG;
        Node["HSAILISD::ARG_DECL_U8"] = ARG_DECL_U8;
        Node["HSAILISD::ARG_DECL_U16"] = ARG_DECL_U16;
        Node["HSAILISD::ARG_DECL_U32"] = ARG_DECL_U32;
        Node["HSAILISD::ARG_DECL_U64"] = ARG_DECL_U64;
        Node["HSAILISD::ARG_DECL_F32"] = ARG_DECL_F32;
        Node["HSAILISD::ARG_DECL_F64"] = ARG_DECL_F64;
        Node["HSAILISD::ST_SCALAR_RET_U8"] = ST_SCALAR_RET_U8;
        Node["HSAILISD::ST_SCALAR_RET_U16"] = ST_SCALAR_RET_U16;
        Node["HSAILISD::ST_SCALAR_RET"] = ST_SCALAR_RET;
        Node["HSAILISD::RET_DECL_U8"] = RET_DECL_U8;
        Node["HSAILISD::RET_DECL_U16"] = RET_DECL_U16;
        Node["HSAILISD::RET_DECL_U32"] = RET_DECL_U32;
        Node["HSAILISD::RET_DECL_U64"] = RET_DECL_U64;
        Node["HSAILISD::RET_DECL_F32"] = RET_DECL_F32;
        Node["HSAILISD::RET_DECL_F64"] = RET_DECL_F64;
        Node["HSAILISD::LD_SCALAR_ARG_U8"] = LD_SCALAR_ARG_U8;
        Node["HSAILISD::LD_SCALAR_ARG_U16"] = LD_SCALAR_ARG_U16;
        Node["HSAILISD::LD_SCALAR_ARG"] = LD_SCALAR_ARG;
        Node["Intrinsic"] = INTRINSIC;

        Node["EOL"] = DELETED_NODE;
    };

  ~DAGWalker(){};

private:
  enum ParseState {
    PS_ERROR = -1,
    PS_START,
    PS_EXPECT_LDST_ADDR, // Expect address for load or store
    PS_EXPECT_STORE_DST, // st_***  ...
    PS_BR_EXPECT_COND,   // brcond ...
    PS_BR_EXPECT_BB,     // brcond cond ...  'expects address of BasicBlock to jump to'
    PS_LD_ARG_EXPECT_SRC,   // ld|st_arg_***  $dest, ... (expect: PSEUDO_REGISTER [p0,...pN][offset])
    PS_ST_ARG_EXPECT_SRC,   // ld_arg_***  ... (expect: PSEUDO_REGISTER [p0,...pN][offset])
    PS_ST_ARG_EXPECT_DST,  // ld|st_arg_***  $src, ... (expect: PSEUDO_REGISTER [p0,...pN][offset])
    PS_LD_RET_EXPECT_SRC,   // ld_rarg_***  $dest, ... (expect: PSEUDO_REGISTER [p0,...pN][offset])
    PS_ST_RET_EXPECT_SRC,   // st_rarg_*** ... (expect: REGISTER)
    PS_ST_RET_EXPECT_DST,  // ld|st_rarg_***  $src, ... (expect: PSEUDO_REGISTER [p0,...pN][offset])
    PS_VEC_ARG_DECL_EXPECT_PARAMREG,  // arg_arr_decl (expect: PSEUDO_REGISTER [p0,...pN])
    PS_VEC_RET_DECL_EXPECT_PARAMREG,  // ret_arr_decl (expect: PSEUDO_REGISTER [r0,...rN])
    PS_VEC_DECL_EXPECT_SIZE,  // ( ret_arr_decl | arg_arr_decl ) ( %retV_rK | %param_pK )[ ... ( expect: Constant ) -- immediate size

    PS_END,
  };
  //raw_ostream & printer;
  std::ostringstream & printer;
  ParseState m_state;

  // Offset between BRIG operand number and llvm Machine operand number
  // Now it's always negative. 
  // For example in case of vector operations it will be negative
  // And in case of operations with width argument it will be positive
  int m_offset;

  unsigned m_opNum;

  void ProcessDef( DefInit * );
  void ProcessIntrinsic( DefInit * );

  void EmitVectorOrScalarOperand();

  std::map<std::string, unsigned> Node;

  int m_vec_size;

public:
  void WalkDAG( DagInit * , int& );
};

# endif // DAG_WALKER_H
