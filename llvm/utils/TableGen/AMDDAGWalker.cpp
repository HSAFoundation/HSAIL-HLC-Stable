#if defined(AMD_OPENCL) || 1

#include "AMDDAGWalker.h"

#ifdef ANDROID
#include <stdio.h>
#endif

bool IsSubclassOf( std::string sSuperclassName, DefInit * def ) {
  std::vector<Record*> superClasses = def->getDef()->getSuperClasses();
  for ( std::vector<Record*>::const_iterator i = superClasses.begin(); i != superClasses.end(); i++ ) {
    Record * r = *i;
    if ( r->getName() == sSuperclassName ) {
      return true;
    }
  }
  return false;
}

std::string getOpcode( DefInit * def ) {
  std::string sOpcode = "EOL";
  if ( def ) {
    if ( IsSubclassOf("SDNode", def ) ) {
      sOpcode = def->getDef()->getValueAsString("Opcode");
    } else	if ( IsSubclassOf("PatFrag", def ) ) {
      if ( DagInit * frag = def->getDef()->getValueAsDag("Fragment") ) {
        Init * val = frag->getOperator();
        if ( DefInit * def = dynamic_cast<DefInit*>(val) ) {
          if ( IsSubclassOf("ValueType", def) ) {
            sOpcode = getOpcode(dynamic_cast<DefInit*>(frag->getArg(0)));
          } else {
            sOpcode = getOpcode(def);
          }
        }
      }
    } else if  ( IsSubclassOf("RegisterClass", def ) ) {
      sOpcode = "REGISTER";
    } else if  ( IsSubclassOf("ValueType", def) ) {
      sOpcode = "ValueType";
    } else if  ( IsSubclassOf("Intrinsic", def) ) {
      sOpcode = "Intrinsic";
    }
  }
  return sOpcode;
}

void DAGWalker::ProcessIntrinsic( DefInit * def ) {
  std::string sIntrnName = def->getDef()->getName();
  std::string sHSAILIntrnPrefix("int_HSAIL_");
  assert( 0 == sIntrnName.compare(0, sHSAILIntrnPrefix.length(), sHSAILIntrnPrefix) );
  if ( ( sIntrnName.find("img") != sIntrnName.npos ) || ( sIntrnName.find("image") != sIntrnName.npos ) ) 
  {  
    /* image load, store or read */
    printer << "\t\tBrigEmitImageInst(MI, inst, 0);\n";
  } else if  ( sIntrnName.find("query") != sIntrnName.npos ) {
    /* query width, height, order etc */
    printer << "\t\t{\n";
    printer << "\t\t\tunsigned MONum=1, BRIGopNum=1;\n"; 
    printer << "\t\tBrigEmitOperandImage(MI, MONum, inst, BRIGopNum);\n\t\t}\n";
  } else {
    /* not image intrinsic */
    ListInit * LI = def->getDef()->getValueAsListInit("ParamTypes");
    ListInit::const_iterator i = LI->begin();
    ListInit::const_iterator e = LI->end();
    for (;i != e; i++ ){ 
      Init * init = *i;
      DefInit * defParamType = dynamic_cast<DefInit*>(init);
      if ( defParamType ) {
        std::string sParamType = defParamType->getDef()->getName();
        if ( "llvm_ptr_ty" == sParamType ) {
          printer << "\t\tBrigEmitOperandLdStAddress( MI, " << m_opNum << ", inst, " << m_offset << " );\n\t";
          m_opNum += 3;
          m_offset -= 2;
        } else {
          printer << "\t\tBrigEmitOperand( MI, " << m_opNum++ << ", inst, " << m_offset  << " );\n\t";
        }
      }
    }
  }
  m_state = PS_END;
}

void DAGWalker::EmitVectorOrScalarOperand() {
  if (m_vec_size == 1)
    printer << "  BrigEmitOperand( MI, " << m_opNum++ << ", inst);\n\t";
  else if (m_vec_size == 2)
  {
    printer << "{ unsigned op_start = 0, operand = " << m_opNum + m_offset << ";  BrigEmitOperandV2( MI, op_start, inst, operand ); }\n\t";
    m_opNum += 2;
    m_offset -= 1;
  }
  else if (m_vec_size == 3)
  {
    printer << "{ unsigned op_start = 0, operand = " << m_opNum + m_offset << ";  BrigEmitOperandV3( MI, op_start, inst, operand ); }\n\t";
    m_opNum += 3;
    m_offset -= 2;
  }
  else if (m_vec_size == 4)
  {
    printer << "{ unsigned op_start = 0, operand = " << m_opNum + m_offset << ";  BrigEmitOperandV4( MI, op_start, inst, operand ); }\n\t";
    m_opNum += 4;
    m_offset -= 3;
  }
  else
    printf("INCORRECT VECTOR SIZE \n");
}

void DAGWalker::ProcessDef( DefInit * def ) {

  switch( m_state ) {
  case PS_START:
    {
      switch(Node[getOpcode(def)]) {
      case LDA_FLAT:
      case LDA_GLOBAL:
      case LDA_GROUP:
      case LDA_PRIVATE:
      case LDA_READONLY:
        m_state = PS_EXPECT_LDST_ADDR;
        break;
      case LOAD:
        printer << " BrigEmitQualifiers( MI, " << m_opNum + 3 << ", inst );\n\t";
        m_state = PS_EXPECT_LDST_ADDR;
        break;
      case LOAD_PARAM_U8:
      case LOAD_PARAM_U16:
      case LOAD_PARAM_PTR:
      case LOAD_PARAM_PTR_STRUCT_BY_VAL:
      case LOAD_PARAM:
      case LOAD_PARAM_IMAGE:
      case LOAD_PARAM_SAMP:
        printer << "  BrigEmitOperandAddress( MI, " << m_opNum << ", inst, " << m_offset << " );\n\t";
        m_opNum++;
        m_state = PS_END;
        break;
      case STORE:	
        m_state = PS_EXPECT_STORE_DST;
        break;
      case BRCOND:
        m_state = PS_BR_EXPECT_COND;
        break;
      case BR:
        m_state = PS_BR_EXPECT_BB;
        break;
      case Register:
      case Constant:
      case ConstantFP:
        //	Don't change state - just emit what we have 
        EmitVectorOrScalarOperand();
        break;
      case INTRINSIC:
        {
          ProcessIntrinsic(def);
        }
        break;
      case ST_SCALAR_ARG_U8:
      case ST_SCALAR_ARG_U16:
      case ST_SCALAR_ARG:
        m_state = PS_ST_ARG_EXPECT_SRC;
        break;
      case LD_SCALAR_RET_U8:
      case LD_SCALAR_RET_U16:
      case LD_SCALAR_RET:
        m_state = PS_LD_RET_EXPECT_SRC;
        break;
      case ST_SCALAR_RET_U8:
      case ST_SCALAR_RET_U16:
      case ST_SCALAR_RET:
        m_state = PS_ST_RET_EXPECT_SRC;
        break;
      case LD_SCALAR_ARG_U8:
      case LD_SCALAR_ARG_U16:
      case LD_SCALAR_ARG:
        m_state = PS_LD_ARG_EXPECT_SRC;
        break;
      case ARG_DECL_U8:
      case ARG_DECL_U16:
      case ARG_DECL_U32:
      case ARG_DECL_U64:
      case ARG_DECL_F32:
      case ARG_DECL_F64:
        m_state = PS_VEC_ARG_DECL_EXPECT_PARAMREG;
        break;
      case RET_DECL_U8:
      case RET_DECL_U16:
      case RET_DECL_U32:
      case RET_DECL_U64:
      case RET_DECL_F32:
      case RET_DECL_F64:
        m_state = PS_VEC_RET_DECL_EXPECT_PARAMREG;
        break;
      }
    }
    break;
  case PS_VEC_ARG_DECL_EXPECT_PARAMREG:
    {
      switch(Node[getOpcode(def)]) {
      case Constant:
        m_state = PS_VEC_DECL_EXPECT_SIZE;
        printer << m_opNum++ << ", \"%param_\", ";
        break;
      case VALUETYPE: 
        printer << "BrigEmitVecArgDeclaration( MI, ";
      break;
      default:
        printf("PATTERN DAG HAS INCORRECT FORMAT \n");
        exit(1);
      }
    }
    break;
  case PS_VEC_RET_DECL_EXPECT_PARAMREG:
    {
      switch(Node[getOpcode(def)]) {
      case Constant:
        m_state = PS_VEC_DECL_EXPECT_SIZE;
        printer << m_opNum++ << ", \"%ret_\", ";
        break;
      case VALUETYPE:  
        printer << "BrigEmitVecArgDeclaration( MI, ";
      break;
      default:
        printf("PATTERN DAG HAS INCORRECT FORMAT \n");
        exit(1);
      }
    }
    break;
  case PS_VEC_DECL_EXPECT_SIZE:
    {
      switch(Node[getOpcode(def)]) {
      case Constant:
        printer << m_opNum++ << " );\n";
        m_state = PS_END;
        break;
      case VALUETYPE:  // do nothing, expect value itself 
      break;
      default:
        printf("PATTERN DAG HAS INCORRECT FORMAT \n");
        exit(1);
      }
    }
    break;
  case PS_ST_ARG_EXPECT_SRC:
    {
      switch(Node[getOpcode(def)]) {
      case Register:
        m_state = PS_ST_ARG_EXPECT_DST;
        printer << "  BrigEmitOperand( MI, " << m_opNum++ << ", inst);\n\t";
        break;
      case VALUETYPE:  // do nothing, expect value itself 
      break;
      default:
        printf("PATTERN DAG HAS INCORRECT FORMAT \n");
        exit(1);
      }
    }
    break;
  case PS_ST_RET_EXPECT_SRC:
    {
      switch(Node[getOpcode(def)]) {
      case Register:
        m_state = PS_ST_RET_EXPECT_DST;
        printer << "  BrigEmitOperand( MI, " << m_opNum++ << ", inst );\n\t";
        break;
      case VALUETYPE:  // do nothing, expect value itself 
      break;
      default:
        printf("PATTERN DAG HAS INCORRECT FORMAT \n");
        exit(1);
      }
    }
    break;
  case PS_ST_RET_EXPECT_DST:
  case PS_LD_RET_EXPECT_SRC:
    {
      switch(Node[getOpcode(def)]) {
      case Register:
        {
          printer << "  BrigEmitOperandArgRef( MI, " << m_opNum++ << ", \"%ret_\", inst, " << m_offset << " );\n\t";
          m_state = PS_END;
        }
        break;
      case VALUETYPE:  // do nothing, expect value itself 
      break;
      default:
        printf("PATTERN DAG HAS INCORRECT FORMAT \n");
        exit(1);
      }
    }
    break;
  case PS_ST_ARG_EXPECT_DST:
    {
      switch(Node[getOpcode(def)]) {
      case Register:
        {
          printer << "  BrigEmitOperandArgRef( MI, " << m_opNum++ << ", \"%param_\", inst, " << m_offset << " );\n\t";
          m_state = PS_END;
        }
        break;
      case VALUETYPE:  // do nothing, expect value itself 
      break;
      default:
        printf("PATTERN DAG HAS INCORRECT FORMAT \n");
        exit(1);
      }
    }
    break;
  case PS_LD_ARG_EXPECT_SRC:
    {
      switch(Node[getOpcode(def)]) {
      case Register:
        {
          printer << "  BrigEmitOperandArgRef( MI, " << m_opNum++ << ", \"%arg_\", inst, " << m_offset << " );\n\t";
          m_state = PS_END;
        }
        break;
      case VALUETYPE:  // do nothing, expect value itself 
      break;
      default:
        printf("PATTERN DAG HAS INCORRECT FORMAT \n");
        exit(1);
      }
    }
    break;
  case PS_EXPECT_LDST_ADDR:
    {
      printer << "  BrigEmitOperandLdStAddress( MI, " << 
        m_opNum << 
        ", inst, " << 
        m_offset << " );\n\t";
      m_state = PS_END;
    }
    break;
  case PS_EXPECT_STORE_DST:
    {
      switch(Node[getOpcode(def)]) {
      case Register:
        EmitVectorOrScalarOperand();
        printer << " BrigEmitQualifiers( MI, " << m_opNum + 3 << ", inst );\n\t";
        m_state = PS_EXPECT_LDST_ADDR;
        break;
      case VALUETYPE:  // do nothing, expect value itself TODO: check type/size against operation type/size
      case ADD:        // we likely in [$reg + off] story, expect operands
      case LDA_GLOBAL: // load $reg %addr - here we're retreiving the '%addr' with LDA_***, we don't need to react, just await for LDA operand
      case LDA_FLAT:   // same story as above...
      case LDA_GROUP:
      case LDA_PRIVATE:
        break;
      default:
        m_state = PS_ERROR;
      }
    }
    break;
  case PS_BR_EXPECT_COND:
    {
      switch(Node[getOpcode(def)]) {
      case Register: // TODO: check for the appropriate register class C*
        printer << "  BrigEmitOperand( MI, " << m_opNum++ << ", inst );\n\t";
        m_state = PS_BR_EXPECT_BB;
        break;
      case VALUETYPE:  // do nothing, expect value itself TODO: check type/size against operation type/size
        break;
      default:
        m_state = PS_ERROR;
      }
    }
    break;
  case PS_BR_EXPECT_BB:
    {
      switch(Node[getOpcode(def)]) {
      case BasicBlock:
        printer << "  BrigEmitOperandAddress( MI, " << m_opNum++ << ", inst, " << m_offset << " );\n\t";
        break;
      case DELETED_NODE:
        {
          m_state = PS_END;
        }
        break;
      default:
        m_state = PS_ERROR;
      }
    }
    break;
  case PS_END:
  switch(Node[getOpcode(def)]) {
      case DELETED_NODE:    // we're done. only EOL accepted to let recursion rollout
        {
          m_state = PS_END;
        }
        break;
      default:
        printf("PATTERN DAG HAS INCORRECT FORMAT \n");
        exit(1);
  }
  break;

  default:
    {
      printf("PATTERN DAG HAS INCORRECT FORMAT \n");
      exit(1);
    }
  }
}

void DAGWalker::WalkDAG( DagInit * dag, int& nNumOperands ) {
  if ( ( 0 > nNumOperands ) || ( PS_END == m_state ) ) return;
  Init * val = dag->getOperator();
  if ( DefInit * def = dynamic_cast<DefInit*>(val) ) {
    ProcessDef(def);
  }
  for ( DagInit::const_arg_iterator ai = dag->arg_begin(), ae = dag->arg_end(); ai != ae; ai++ ) {
    if ( DefInit * def = dynamic_cast<DefInit*>(*ai) ) { 
      if ( ( 0 > --nNumOperands ) || ( PS_END == m_state )) return;    
      ProcessDef(def);
    } else if ( DagInit * d = dynamic_cast<DagInit*>(*ai) ) {
      WalkDAG( d, nNumOperands);
    }
  }
  if ( nNumOperands == 0 )  { // we're done - generate EOL
    ProcessDef(NULL);
  }
}

#endif // AMD_OPENCL
