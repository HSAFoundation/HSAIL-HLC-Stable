#ifndef HSAILISDNODES_H
#define HSAILISDNODES_H

namespace llvm {
  namespace HSAILISD {
    // HSAIL Specific DAG Nodes
    enum NodeType {
      // Start the numbering where the builtin ops leave off.
      FIRST_NUMBER = ISD::BUILTIN_OP_END,
      CALL,        // Function call based on a single integer
      /// Return with a flag operand. Operand 0 is the chain operand, operand
      /// 1 is the number of bytes of stack to pop.
      RET_FLAG,
      LOAD_PARAM_U8,
      LOAD_PARAM_U16,
      LOAD_PARAM_IMAGE,
      LOAD_PARAM_SAMP,
      LOAD_PARAM,
      LOAD_PARAM_PTR,
      LDA_FLAT,
      LDA_GLOBAL,
      LDA_GROUP,
      LDA_PRIVATE,
      LDA_READONLY,
      WRAPPER,
      TRUNC_B1,
      LD_SCALAR_RET_U8,
      LD_SCALAR_RET_U16,
      LD_SCALAR_RET,
      ST_SCALAR_ARG_U8,
      ST_SCALAR_ARG_U16,
      ST_SCALAR_ARG,
      LD_SCALAR_ARG_U8,
      LD_SCALAR_ARG_U16,
      LD_SCALAR_ARG,
      ARG_DECL_U8,
      ARG_DECL_U16,
      ARG_DECL_U32,
      ARG_DECL_U64,
      ARG_DECL_F32,
      ARG_DECL_F64,
      ST_SCALAR_RET_U8,
      ST_SCALAR_RET_U16,
      ST_SCALAR_RET,
      RET_DECL_U8,
      RET_DECL_U16,
      RET_DECL_U32,
      RET_DECL_U64,
      RET_DECL_F32,
      RET_DECL_F64,
      INTRINSIC,
      LOAD_PARAM_PTR_STRUCT_BY_VAL
    };
  }
}
#endif
