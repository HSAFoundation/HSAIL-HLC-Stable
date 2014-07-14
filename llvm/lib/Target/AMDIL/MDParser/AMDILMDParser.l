%top{
#include "llvm/ADT/StringRef.h"
}

%{

#include "AMDILMDInterface.h"
#include "AMDILMDScanner.h"

#undef ECHO

#if 0

#include <cstdio>

#define MATCH(rule)                    \
  do {                                 \
    fprintf(stderr, "\nMatching rule %s: '%s'\n", #rule, yytext); \
  } while (0)
#define ECHO                           \
  do {                                 \
    fprintf(stderr, "[%s]\n", yytext); \
  } while (0)
#define LEAVE_STATE(A)                             \
  do {                                             \
    fprintf(stderr, "Leaving State - [%s] (%d)\n", #A, (int) A); \
    yy_pop_state();                                \
  } while (0)
#define ENTER_STATE(A)                              \
  do {                                              \
    yy_push_state(A);                               \
    fprintf(stderr, "Entering State - [%s] (%d)\n", #A, (int) A); \
  } while (0)
#else
#define MATCH(rule)
#define ECHO
#define LEAVE_STATE(A) yy_pop_state()
#define ENTER_STATE(A) yy_push_state(A)
#endif
#define YY_USER_ACTION yylloc->columns(YYLeng());
#define SAVE_STR_TOKEN yylval->str = new std::string(YYText(), YYLeng());
#define TOKEN(t) (yylval->token = t);
%}

%option c++
%option yyclass="AMDILMDScanner"
%option yylineno
%option stack
%option nodefault
%option noyywrap

NewLine      [\n\r]
TabSpace     [[:blank:]]
WhiteSpace   [ \t\r\n]
DecimalNumber [[:digit:]]+
HexNumber    [[:xdigit:]]+
TextLine     [[:alnum:]\._,()\[\]\-:@~#"/\\ `\t+\$]*
DbgTextLine  [[:alnum:]\._,()\[\]\-:@~#"\/\"`\\ \t+;!\f&*=+{}|<>,.?$%^~']*
MDTextLine   [[:alnum:]\._,()\[\]\-@#"/\\`&*!$%+]*
DSTextLine   [[:alnum:]]*
Comment      ";"
MDDelimiter ":"
DSDelimiter "#"
DelimitedHexNumber {MDDelimiter}{HexNumber}
DelimitedDecimalNumber {MDDelimiter}{DecimalNumber}
SingleLineComment [[:blank:]]*{Comment}.*
MainCallSpot ;$$$$$$$$$$

  /* The various global state tokens */
%x IL_STATE
%x DBG_STATE
%x MD_STATE
%x PROG_START
%x MACRO_STATE
%x DBG_NAME
%x IL_NAME
%x IL_NAME_SUB
%x MD_NAME
%x MD_ARG
%x DS_STATE
%x DS_NAME

%%
   /* All the IL state parsing goes here */
il_cs_2_0 {
  MATCH(il_cs_2_0);
  SAVE_STR_TOKEN;
  ENTER_STATE(IL_STATE);
  ECHO;
  return llvm::AMDILMDParser::token::IL_VERSION;
}

{WhiteSpace} {
  MATCH(WhiteSpace);
  ECHO; /* eat white space */
}

<IL_STATE>endmain {
  MATCH(IL_STATE/endmain);
  SAVE_STR_TOKEN;
  LEAVE_STATE(IL_STATE);
  ECHO;
  return llvm::AMDILMDParser::token::IL_ENDMAIN;
}

func {
  MATCH(func);
  ENTER_STATE(IL_STATE);
  ENTER_STATE(IL_NAME);
  ECHO;
  return llvm::AMDILMDParser::token::IL_FUNC;
}

<IL_STATE>{Comment} {
  MATCH(IL_STATE/Comment);
  ECHO;
  return ';';
}

<IL_STATE>{SingleLineComment} {
  MATCH(IL_STATE/SingleLineComment);
  SAVE_STR_TOKEN;
  ECHO;
  return llvm::AMDILMDParser::token::STR_TOKEN;
}

<IL_STATE>{TextLine} {
  MATCH(IL_STATE/TextLine);
  ECHO;

  llvm::StringRef text(YYText(), YYLeng());

  if (text.startswith("endfunc")) {
    SAVE_STR_TOKEN;
    LEAVE_STATE(IL_STATE);
    ENTER_STATE(IL_NAME);
    return llvm::AMDILMDParser::token::IL_ENDFUNC;
  }

  if (text.startswith("mend")) {
    SAVE_STR_TOKEN;
    LEAVE_STATE(IL_STATE);
    ENTER_STATE(IL_NAME);
    return llvm::AMDILMDParser::token::MACRO_END;
  }

  if (text != "\t") {
    SAVE_STR_TOKEN;
    return llvm::AMDILMDParser::token::STR_TOKEN;
  }

  /* If it is only a tab, let's eat it.
     We want to allow tabs within a instruction,
     but not only a tab by itself. */
}

<IL_STATE>{WhiteSpace} {
  MATCH(IL_STATE/WhiteSpace);
  ECHO; /* eat white space */
}

<IL_NAME>{Comment} {
  MATCH(IL_NAME/Comment);
  ECHO;
  ENTER_STATE(IL_NAME_SUB);
  return ';';
}

<IL_NAME>{MDTextLine} {
  MATCH(IL_NAME/MDTextLine);
  SAVE_STR_TOKEN;
  ECHO;
  return llvm::AMDILMDParser::token::STR_TOKEN;
}

<IL_NAME_SUB>{MDTextLine} {
  MATCH(IL_NAME_SUB/MDTextLine);
  SAVE_STR_TOKEN;
  ECHO;
  return llvm::AMDILMDParser::token::STR_TOKEN;
}

<IL_NAME_SUB>{Comment} {
  MATCH(IL_NAME_SUB/Comment);
  ECHO;
  return ';';
}

<IL_NAME_SUB>{TabSpace} {
  MATCH(IL_NAME_SUB/TabSpace);
  ECHO; /* eat tabs and spaces */
}

<IL_NAME_SUB>{WhiteSpace} {
  MATCH(IL_NAME_SUB/WhiteSpace);
  ECHO;
  LEAVE_STATE(IL_NAME_SUB);
  LEAVE_STATE(IL_NAME);/* eat newlines and carriage returns */
}

<IL_NAME>{TabSpace} {
  MATCH(IL_NAME/TabSpace);
  ECHO; /* eat white space */
}

<IL_NAME>{NewLine} {
  MATCH(IL_NAME/NewLine);
  ECHO;
  LEAVE_STATE(IL_NAME);/* eat newlines and carriage returns */
}

end {
  MATCH(end);
  ECHO;
}

   /* All the Macro state parsing goes here*/
mdef {
  MATCH(mdef);
  SAVE_STR_TOKEN;
  ENTER_STATE(IL_STATE);
  ENTER_STATE(IL_NAME);
  ECHO;
  return llvm::AMDILMDParser::token::MACRO_START;
}

  /* All the Debug state parsing goes here */
;DEBUGSTART {
  MATCH(;DEBUGSTART);
  ENTER_STATE(DBG_STATE);
  ENTER_STATE(DBG_NAME);
  ECHO;
  return llvm::AMDILMDParser::token::DBG_START;
}

<DBG_NAME>{DbgTextLine} {
  MATCH(DBG_NAME/DbgTextLine);
  SAVE_STR_TOKEN;
  ECHO;
  return llvm::AMDILMDParser::token::STR_TOKEN;
}

<DBG_NAME>{NewLine} {
  MATCH(DBG_NAME/NewLine);
  LEAVE_STATE(DBG_NAME);
}

<DBG_STATE>;DEBUGEND {
  LEAVE_STATE(DBG_STATE);
  ECHO;
  return llvm::AMDILMDParser::token::DBG_END;
}

<DBG_STATE>{WhiteSpace} {
  MATCH(DBG_STATE/WhiteSpace);
  ECHO; /* eat white space */
}

<DBG_STATE>{DbgTextLine} {
  MATCH(DBG_STATE/DbgTextLine);
  ECHO;
  // Linux detects DEBUGEND here
  if (!strncmp(yytext, ";DEBUGEND", 9)) {
    LEAVE_STATE(DBG_STATE);
    return llvm::AMDILMDParser::token::DBG_END;
  } else {
    SAVE_STR_TOKEN;
    return llvm::AMDILMDParser::token::STR_TOKEN;
  }
}
   /* All the Metadata state parsing goes here*/
;ARGSTART {
  MATCH(MD_STATE/;ARGSTART);
  ENTER_STATE(MD_STATE);
  ENTER_STATE(MD_NAME);
  ECHO;
  return llvm::AMDILMDParser::token::ARG_START;
}

<MD_STATE>;ARGEND {
  MATCH(MD_STATE/;ARGEND);
  LEAVE_STATE(MD_STATE);
  ENTER_STATE(MD_NAME);
  ECHO;
  return llvm::AMDILMDParser::token::ARG_END;
}

<MD_STATE>;version {
  MATCH(MD_STATE/;version);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_INTLIST;
}

<MD_STATE>;device {
  MATCH(MD_STATE/;device);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_STRING;
}

<MD_STATE>;error {
  MATCH(MD_STATE/;error);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_ERRWAR;
}

<MD_STATE>;warning {
  MATCH(MD_STATE/;warning);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_ERRWAR;
}

<MD_STATE>;memory {
  MATCH(MD_STATE/;memory);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_MEMORY;
}

<MD_STATE>;uniqueid {
  MATCH(MD_STATE/;uniqueid);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_INT;
}

<MD_STATE>;sampler {
  MATCH(MD_STATE/;sampler);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_SAMPLER;
}

<MD_STATE>;image {
  MATCH(MD_STATE/;image);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_IMAGE;
}

<MD_STATE>;counter {
  MATCH(MD_STATE/;counter);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_COUNTER;
}

<MD_STATE>;sema {
  MATCH(MD_STATE/;sema);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO; return llvm::AMDILMDParser::token::MD_COUNTER;
}

<MD_STATE>;value {
  MATCH(MD_STATE/;value);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_VALUE;
}

<MD_STATE>;pointer {
  MATCH(MD_STATE/;pointer);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_POINTER;
}

<MD_STATE>;uavid {
  MATCH(MD_STATE/;uavid);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_INT;
}

<MD_STATE>;printfid {
  MATCH(MD_STATE/;printfid);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_INT;
}

<MD_STATE>;cbid {
  MATCH(MD_STATE/;cbid);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_INT;
}

<MD_STATE>;privateid {
  MATCH(MD_STATE/;privateid);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_INT;
}

<MD_STATE>;enqueue_kernel {
  MATCH(MD_STATE/;enqueue_kernel);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_INT;
}
<MD_STATE>;kernel_index {
  MATCH(MD_STATE/;kernel_index);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_INT;
}
<MD_STATE>;printf_fmt {
  MATCH(MD_STATE/;printf_fmt);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_PRINTF;
}

<MD_STATE>;function {
  MATCH(MD_STATE/;function);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_INTLIST;
}

<MD_STATE>;intrinsic {
  MATCH(MD_STATE/;intrinsic);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_INTLIST;
}

<MD_STATE>;cws {
  MATCH(MD_STATE/;cws);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_INTLIST;
}

<MD_STATE>;lws {
  MATCH(MD_STATE/;lws);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_INT;
}

<MD_STATE>;limitgroupsize {
  MATCH(MD_STATE/;limitgroupsize);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_LGS;
}

<MD_STATE>;reflection {
  MATCH(MD_STATE/;reflection);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_REFLECTION;
}

<MD_STATE>;constarg {
  MATCH(MD_STATE/;constarg);
  SAVE_STR_TOKEN;
  ENTER_STATE(MD_ARG);
  ECHO;
  return llvm::AMDILMDParser::token::MD_CONSTARG;
}

<MD_STATE>{WhiteSpace} {
  MATCH(MD_STATE/WhiteSpace);
  ECHO; /* eat white space */
}

<MD_NAME>{MDTextLine} {
  MATCH(MD_NAME/MDTextLine);
  SAVE_STR_TOKEN;
  ECHO;
  return llvm::AMDILMDParser::token::STR_TOKEN;
}

<MD_NAME>{NewLine} {
  MATCH(MD_NAME/NewLine);
  LEAVE_STATE(MD_NAME);
}

<MD_NAME>{TabSpace} {
  MATCH(MD_NAME/TabSpace);
  ECHO; /* eat white space */
}

<MD_NAME>{MDDelimiter} {
  MATCH(MD_NAME/MDDelimiter);
  ECHO;
  return ':';
}

<MD_ARG>{MDDelimiter} {
  MATCH(MD_ARG/MDDelimiter);
  ECHO;
  return ':';
}

<MD_ARG>{DecimalNumber} {
  MATCH(MD_ARG/DecimalNumber);
  TOKEN(atoi(yytext));
  ECHO;
  return llvm::AMDILMDParser::token::INT_TOKEN;
}

<MD_ARG>{MDTextLine} {
  MATCH(MD_ARG/MDTextLine);
  SAVE_STR_TOKEN;
  ECHO;
  return llvm::AMDILMDParser::token::STR_TOKEN;
}

<MD_ARG>{TabSpace} {
  MATCH(MD_ARG/TabSpace);
  ECHO; /* eat white space */
}

<MD_ARG>{NewLine} {
  MATCH(MD_ARG/NewLine);
  LEAVE_STATE(MD_ARG);
}

<MD_ARG>;{NewLine} {
  MATCH(MD_ARG/NewLine);
  LEAVE_STATE(MD_ARG);
}

   /* All the data segment state parsing goes here */
;#DATASTART {
  ENTER_STATE(DS_STATE);
  ECHO;
  return llvm::AMDILMDParser::token::DATA_START;
}

<DS_STATE>{DSTextLine} {
  MATCH(DS_STATE/DSTextLine);
  SAVE_STR_TOKEN; ECHO;
  if (!strncmp(yytext, ";#DATAEND", 9) ||
      !strncmp(yytext, "DATAEND", 7)) {
    LEAVE_STATE(DS_STATE);
    ENTER_STATE(DS_NAME);
    return llvm::AMDILMDParser::token::DATA_END;
  } else {
    return llvm::AMDILMDParser::token::STR_TOKEN;
  }
}

<DS_STATE>{WhiteSpace} {
  MATCH(DS_STATE/WhiteSpace);
  ECHO; /* eat white space */
}

<DS_STATE>{Comment} {
  MATCH(DS_STATE/Comment);
  ECHO;
  return ';';
}

<DS_STATE>{MDDelimiter} {
  MATCH(DS_STATE/MDDelimiter);
  ECHO;
  return ':';
}

<DS_STATE>{DSDelimiter} {
  MATCH(DS_STATE/DSDelimiter);
  ECHO;
  return '#';
}

<DS_NAME>{MDDelimiter} {
  MATCH(DS_NAME/MDDelimiter);
  ECHO;
  return ':';
}

<DS_NAME>{DecimalNumber} {
  MATCH(DS_NAME/DecimalNumber);
  TOKEN(atoi(yytext));
  ECHO;
  return llvm::AMDILMDParser::token::INT_TOKEN;
}

<DS_NAME>{NewLine} {
  MATCH(DS_NAME/NewLine);
  LEAVE_STATE(DS_NAME);
}
%%
