%skeleton "lalr1.cc"
%defines
%locations
%debug
%define namespace "llvm"
%define parser_class_name "AMDILMDParser"
%parse-param { llvm::AMDILMDScanner &scanner }
%parse-param { CompUnit* root }
%parse-param { bool _merror }
%parse-param { std::string _mmsg }
%lex-param { llvm::AMDILMDScanner &scanner }
%{
#include <string>
#include <cstdio>
#include <cstring>
#include <cassert>
#include "AMDILMDInterface.h"
//#define YYERROR_VERBOSE
  // Forward-declare the Scanner class; the Parser needs to be assigned
  // a Scanner, but the Scanner can't be declared without the parser
#if 0
#define MATCH(rule)                    \
  do {                                 \
    fprintf(stderr, "\nScanner: Matching rule %s\n", #rule); \
  } while (0)
#else
#define MATCH(rule)
#endif
%}
%code requires {
#include <string>
#include <sstream>
  namespace llvm {
    class AMDILMDScanner;
  }
  }

%code {
  // prototype for the yylex function
  static int yylex(llvm::AMDILMDParser::semantic_type * yylval,
                   llvm::AMDILMDParser::location_type * yylloc,
                   llvm::AMDILMDScanner &scanner);
}

/* types that data can currently be represented as */
%union {
    unsigned token;
    std::string* str;
    MDType* MDNode;
    MDBlock*  mdBlockNode;
    FuncBase* funcNode;
    MainFunc* ilMain;
    DBSection* dbNode;
    DataSection* dsNode;
    Data*        dsEntry;
    std::vector<Data*>* dsEntryVec;
    std::vector<unsigned>* intVec;
    std::vector<class MDType*>* mdNodeVec;
    std::list<std::string*>* strList;
    std::list<Component*>* compVec;
    CompUnit*   CU;
}

%token <token> INT_TOKEN
%token <str> STR_TOKEN
/* Metadata tokens */
%token <str> MD_VALUE MD_SAMPLER MD_MEMORY MD_COUNTER MD_INT MD_INTLIST
%token <str> MD_POINTER MD_IMAGE MD_PRINTF MD_STRING MD_LGS MD_SEMAPHORE MD_REFLECTION MD_CONSTARG MD_ERRWAR
%token <str> ARG_START ARG_END
/* IL tokens */
%token <str> IL_VERSION IL_FUNC IL_ENDFUNC IL_ENDMAIN
/* Macro tokens */
%token <str> MACRO_START MACRO_END
/* Debug tokens */
%token <str> DBG_START DBG_END
/* Data Segment tokens */
%token <str> DATA_START DATA_END

%type <MDNode> MDStmt
%type <intVec> IntList
%type <str> CompoundToken CompUnitEnd SpaceCompoundToken
%type <funcNode> ILState
%type <ilMain> ILMain
%type <mdNodeVec> MDStmts
%type <funcNode> MacroState
%type <dbNode> DBGState
%type <mdBlockNode> MDState
%type <dsNode> DataState
%type <dsEntry> DataEntry
%type <dsEntryVec> DataEntries
%type <strList> Statements StrList
%type <compVec> MDStates DBGStates MacroStates ILStates DataStates
%type <CU> CompUnit CompUnits ILProgram

%start ILProgram
%%

ILProgram
    : CompUnits
    ;

CompUnits
    : CompUnit
    | CompUnits CompUnit
    ;

CompUnit
    : CompUnitEnd
    {
        MATCH(CompUnit/CompUnitEnd);
        $$ = root;
        delete $1;
    }
    | ILStates
    {
        MATCH(CompUnit/ILStates);
        $$ = root;
        $$->addComponents($1, CompUnit::IL_STATE);
        delete $1;
    }
    | MacroStates
    {
        MATCH(CompUnit/MacroStates);
        $$ = root;
        $$->addComponents($1, CompUnit::IL_STATE);
        delete $1;
    }
    | DBGStates
    {
        MATCH(CompUnit/DBGStates);
        $$ = root;
        $$->addComponents($1, CompUnit::DBG_STATE);
        delete $1;
    }
    | MDStates
    {
        MATCH(CompUnit/MDStates);
        $$ = root;
        $$->addComponents($1, CompUnit::MD_STATE);
        delete $1;
    }
    | ILMain
    {
        MATCH(CompUnit/ILMain);
        $$ = root;
        $$->setMain($1);
    }
    | DataStates
    {
        MATCH(CompUnit/DataStates);
        $$ = root;
        $$->addComponents($1, CompUnit::DATA_STATE);
        delete $1;
    }
    ;

ILMain
    : IL_VERSION Statements IL_ENDMAIN
    {
        MATCH(ILMain/IL_VERSION Statements IL_ENDMAIN);
        MainFunc *mainFunc = new MainFunc(*$1);
        $2->push_front($1);
        $2->push_back($3);
        mainFunc->setStatements($2);
        $$ = mainFunc;
    }
    ;

CompUnitEnd
    : STR_TOKEN
    ;


ILStates
    : ILState
    {
        MATCH(ILStates/ILState);
        $$ = new std::list<Component*>;
        $$->push_back($1);
    }
    | ILStates ILState
    {
        MATCH(ILStates/ILStates ILState);
        $1->push_back($2);
        $$ = $1;
    }
    ;

MacroStates
    : MacroState
    {
        MATCH(MacroStates/MacroState);
        $$ = new std::list<Component*>;
        $$->push_back($1);
    }
    | MacroStates MacroState
    {
        MATCH(MacroStates/MacroStates MacroState);
        $1->push_back($2);
        $$ = $1;
    }
    ;

DBGStates
    : DBGState
    {
        MATCH(DBGStates/DBGState);
        $$ = new std::list<Component*>;
        if ($1) {
            $$->push_back($1);
        }
    }
    | DBGStates DBGState
    {
        MATCH(DBGStates/DBGStates DBGState);
        if ($2) {
            $1->push_back($2);
        }
        $$ = $1;
    }
    ;

DataStates
    : DataState
    {
        MATCH(DataStates/DataState);
        $$ = new std::list<Component*>;
        $$->push_back($1);
    }
    | DataStates DataState
    {
        MATCH(DataStates/DataStates DataState);
        $1->push_back($2);
        $$ = $1;
    }
    ;


MDStates
    : MDState
    {
        MATCH(MDStates/MDState);
        $$ = new std::list<Component*>;
        $$->push_back($1);
    }
    | MDStates MDState
    {
        MATCH(MDStates/MDStates MDState);
        $1->push_back($2);
        $$ = $1;
    }
    ;

ILState
    : IL_FUNC STR_TOKEN ';' STR_TOKEN Statements IL_ENDFUNC ';' STR_TOKEN
    {
        MATCH(ILState/IL_FUNC STR_TOKEN ';' STR_TOKEN Statements IL_ENDFUNC ';' STR_TOKEN);
        $5->push_back(new std::string(*$6));
        $5->push_back(new std::string(";" + *$8));
        $$ = new ILFunc(atoi($2->c_str()), *$4);
        $5->push_front(new std::string(";" + *$4));
        $5->push_front(new std::string("func " + *$2));
        $$->setStatements($5);
        delete $2;
        delete $4;
        delete $6;
        delete $8;
    }
    | IL_FUNC STR_TOKEN ';' STR_TOKEN ';' STR_TOKEN Statements IL_ENDFUNC ';' STR_TOKEN
    {
        MATCH(ILState/IL_FUNC STR_TOKEN ';' STR_TOKEN ';' STR_TOKEN Statements IL_ENDFUNC ';' STR_TOKEN);
        $7->push_back(new std::string(*$8));
        $7->push_back(new std::string(";" + *$10));
        $7->push_front(new std::string(";" + *$4));
        $7->push_front(new std::string("func " + *$2));
        $$ = new ILFunc(atoi($2->c_str()), *$4);
        $$->setStatements($7);
        delete $2;
        delete $4;
        delete $6;
        delete $8;
        delete $10;
    }
    ;

MacroState
    : MACRO_START STR_TOKEN Statements MACRO_END
    {
        MATCH(MacroState/MACRO_START STR_TOKEN Statements MACRO_END);
        Macro *NewMacro;
        if ((*$2).find("16383", 0) == std::string::npos) {
          NewMacro = new Macro(*$2);
        } else {
          NewMacro = new DummyMacro(*$2);
        }
        NewMacro->setStatements($3);
        NewMacro->header_ = *$1;
        NewMacro->footer_ = *$4;
        $$ = NewMacro;
        delete $1;
        delete $2;
        delete $4;
    }
    | MACRO_START STR_TOKEN ';' STR_TOKEN Statements MACRO_END ';' STR_TOKEN
    {
        MATCH(MacroState/MACRO_START STR_TOKEN ';' STR_TOKEN Statements MACRO_END ';' STR_TOKEN);
/*
        assert((*$2).substr((*$2).length-9).compare("_outline")==0 &&
               "expecting outlined macro");
*/
        assert($2->at(0) == '(' && "unexpected syntax");
        $5->push_back(new std::string(*$6));
        $5->push_back(new std::string(";" + *$8));
        $$ = new ILFunc(atoi(&$2->c_str()[1]), *$4);
        $5->push_front(new std::string(";" + *$4));
        $5->push_front(new std::string("mdef" + *$2));
        $$->setStatements($5);
        delete $2;
        delete $4;
        delete $6;
        delete $8;
    }
    | MACRO_START STR_TOKEN ';' STR_TOKEN ';' STR_TOKEN Statements MACRO_END ';' STR_TOKEN
    {
        MATCH(MacroState/MACRO_START STR_TOKEN ';' STR_TOKEN ';' STR_TOKEN Statements MACRO_END ';' STR_TOKEN);
        assert($2->at(0) == '(' && "unexpected syntax");
        $7->push_back(new std::string(*$8));
        $7->push_back(new std::string(";" + *$10));
        $7->push_front(new std::string(";" + *$4));
        $7->push_front(new std::string("mdef" + *$2));
        $$ = new ILFunc(atoi(&$2->c_str()[1]), *$4);
        $$->setStatements($7);
        delete $2;
        delete $4;
        delete $6;
        delete $8;
        delete $10;
    }
    ;

DBGState
    : DBG_START Statements DBG_END
    {
        MATCH(DBGState/DBG_START Statements DBG_END);
        $$ = new DBSection("");
        $$->setStatements($2);
    }
    | DBG_START DBG_END
    {
        MATCH(DBGState/DBG_START DBG_END);
        $$ = new DBSection("");
    }
    ;

DataState
    : DATA_START ':' STR_TOKEN ':' STR_TOKEN DataEntries ';' '#' DATA_END ':' INT_TOKEN
    {
        MATCH(DataState/DATA_START ':' STR_TOKEN ':' STR_TOKEN DataEntries ';' '#' DATA_END ':' INT_TOKEN);
        assert((unsigned)atoi($3->c_str()) == $11 && "Error in parsing DATA segment. "
        "Start ID and End ID differ!");
        $$ = new DataSection(*$3);
        $$->Size_ = atoi($5->c_str());
        $$->Data_ = $6;
        delete $3;
        delete $5;
        delete $9;
    }
    | DATA_START ':' STR_TOKEN DataEntries ';' '#' DATA_END
    {
        MATCH(DataState/DATA_START ':' STR_TOKEN DataEntries ';' '#' DATA_END);
        $$ = new DataSection("Software");
        $$->Size_ = atoi($3->c_str());
        $$->Data_ = $4;
        delete $3;
        delete $7;
    }
    ;

DataEntries
    : DataEntry
    {
        MATCH(DataEntries/DataEntry);
        $$ = new std::vector<Data*>;
        $$->push_back($1);
    }
    | DataEntries DataEntry
    {
        MATCH(DataEntries/DataEntries DataEntry);
        $1->push_back($2);
        $$ = $1;
    }
    ;

DataEntry
    : ';' '#' CompoundToken ':' StrList
    {
        MATCH(DataEntry/';' '#' CompoundToken ':' StrList);
        $$ = new Data(*$3);
        $$->Data_ = $5;
        delete $3;
    }
    ;

Statements
    : STR_TOKEN
    {
        MATCH(Statements/STR_TOKEN);
        $$ = new std::list<std::string*>;
        $$->push_back($1);
    }
    | Statements STR_TOKEN
    {
        MATCH(Statements/Statements STR_TOKEN);
        $1->push_back($2);
        $$ = $1;
    }
    ;

MDState
    : ARG_START ':' STR_TOKEN MDStmts  ARG_END ':' STR_TOKEN
    {
        MATCH(MDState/ARG_START ':' STR_TOKEN MDStmts  ARG_END ':' STR_TOKEN);
        assert(*$3 == *$7 && "Error in parsing ARG segment. Start ID and End ID"
        " differ!");
        $$ = (class MDBlock*)(new MDBlock(*$3));
        $$->MDVals = $4;
        delete $3;
        delete $7;
    }
    ;

MDStmts
    : MDStmt
    {
        MATCH(MDStmts/MDStmt);
        $$ = new std::vector<class MDType*>;
        $$->push_back($1);
    }
    | MDStmts MDStmt
    {
        MATCH(MDStmts/MDStmts MDStmt);
        $1->push_back($2);
        $$ = $1;
    }
    ;

MDStmt
    : MD_STRING ':' CompoundToken
    {
        MATCH(MDStmt/MD_STRING ':' CompoundToken);
        MDStrings* strNode = new MDStrings(*$1);
        strNode->Str_ = *$3;
        $$ = strNode;
        delete $1;
        delete $3;
    }
    | MD_ERRWAR ':' CompoundToken ':' SpaceCompoundToken
    {
        MATCH(MDStmt/MD_ERRWAR ':' CompoundToken ':' SpaceCompoundToken);
        MDStrings* strNode = new MDStrings(*$1);
        strNode->Str_ = *$3 + ':' + *$5;
        $$ = strNode;
        delete $1;
        delete $3;
        delete $5;
    }
    | MD_REFLECTION ':' INT_TOKEN ':' CompoundToken
    {
      MATCH(MDStmt/MD_REFLECTION ':' INT_TOKEN ':' CompoundToken);
      MDReflection *refNode = new MDReflection(*$5);
      refNode->Int_ = $3;
      $$ = refNode;
      delete $1;
      delete $5;
    }
    | MD_CONSTARG ':' INT_TOKEN ':' CompoundToken
    {
      MATCH(MDStmt/MD_CONSTARG ':' INT_TOKEN ':' CompoundToken);
      MDConstArg *refNode = new MDConstArg(*$5);
      refNode->Int_ = $3;
      $$ = refNode;
      delete $1;
      delete $5;
    }
    | MD_MEMORY ':' STR_TOKEN ':' INT_TOKEN
    {
        MATCH(MDStmt/MD_MEMORY ':' STR_TOKEN ':' INT_TOKEN);
        MDMemory* memNode = new MDMemory(*$1);
        memNode->AS_ = *$3;
        memNode->Size_ = $5;
        $$ = memNode;
        delete $1;
        delete $3;
    }
    | MD_MEMORY ':' STR_TOKEN
    {
        MATCH(MDStmt/MD_MEMORY ':' STR_TOKEN);
        MDFlag* boolNode = new MDFlag(*$3);
        boolNode->Flag_ = true;
        $$ = boolNode;
        delete $1;
        delete $3;
    }
    | MD_INT ':' INT_TOKEN
    {
        MATCH(MDStmt/MD_INT ':' INT_TOKEN);
        MDInt* intNode = new MDInt(*$1);
        intNode->Int_ = $3;
        $$ = intNode;
        delete $1;
    }
    | MD_SAMPLER ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN
    {
        MATCH(MDStmt/MD_SAMPLER ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN);
        MDSampler* samplerNode = new MDSampler(*$1);
        samplerNode->Arg_ = *$3;
        samplerNode->ID_ = $5;
        samplerNode->isArg_ = !$7;
        samplerNode->Val_ = $9;
        $$ = samplerNode;
        delete $1;
        delete $3;
    }
    | MD_IMAGE ':' CompoundToken ':' CompoundToken ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN
    {
        MATCH(MDStmt/MD_IMAGE ':' CompoundToken ':' CompoundToken ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN);
        MDImage* imageNode = new MDImage(*$1);
        imageNode->Arg_ = *$3;
        imageNode->Dim_ = *$5;
        imageNode->Type_ = *$7;
        imageNode->ID_ = $9;
        imageNode->CBNum_ = $11;
        imageNode->CBOffset_ = $13;
        $$ = imageNode;
        delete $1;
        delete $3;
        delete $5;
        delete $7;
    }
    | MD_SEMAPHORE ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN
    {
        MATCH(MDStmt/MD_SEMAPHORE ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN);
        MDSemaphore* semaNode = new MDSemaphore(*$1);
        semaNode->Arg_ = *$3;
        semaNode->ID_ = $5;
        semaNode->CBNum_ = $7;
        semaNode->CBOffset_ = $9;
        $$ = semaNode;
        delete $1;
        delete $3;
    }
    | MD_COUNTER ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN
    {
        MATCH(MDStmt/MD_COUNTER ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN);
        MDCounter* counterNode = new MDCounter(*$1);
        counterNode->Arg_ = *$3;
        counterNode->Size_ = $5;
        counterNode->ID_ = $7;
        counterNode->CBNum_ = $9;
        counterNode->CBOffset_ = $11;
        $$ = counterNode;
        delete $1;
        delete $3;
    }
    | MD_VALUE ':' CompoundToken ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN
    {
        MATCH(MDStmt/MD_VALUE ':' CompoundToken ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN);
        MDValue* valueNode = new MDValue(*$1);
        valueNode->Arg_ = *$3;
        valueNode->Type_ = *$5;
        valueNode->Size_ = $7;
        valueNode->CBNum_ = $9;
        valueNode->CBOffset_ = $11;
        $$ = valueNode;
        delete $1;
        delete $3;
        delete $5;
    }
    | MD_POINTER ':' CompoundToken ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN
    {
        MATCH(MDStmt/MD_POINTER ':' CompoundToken ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN);
        MDPointer* pointerNode = new MDPointer(*$1);
        pointerNode->Arg_ = *$3;
        pointerNode->Type_ = *$5;
        pointerNode->Size_ = $7;
        pointerNode->CBNum_ = $9;
        pointerNode->CBOffset_ = $11;
        pointerNode->MemType_ = *$13;
        pointerNode->BufNum_ = $15;
        pointerNode->Alignment_ = $17;
        $$ = pointerNode;
        delete $1;
        delete $3;
        delete $5;
        delete $13;
    }
    | MD_POINTER ':' CompoundToken ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN
    {
        MATCH(MDStmt/MD_POINTER ':' CompoundToken ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN);
        MDPointer* pointerNode = new MDPointer(*$1);
        pointerNode->Arg_ = *$3;
        pointerNode->Type_ = *$5;
        pointerNode->Size_ = $7;
        pointerNode->CBNum_ = $9;
        pointerNode->CBOffset_ = $11;
        pointerNode->MemType_ = *$13;
        pointerNode->BufNum_ = $15;
        pointerNode->Alignment_ = $17;
        pointerNode->AccessType_ = *$19;
        $$ = pointerNode;
        delete $1;
        delete $3;
        delete $5;
        delete $13;
        delete $19;
    }
    | MD_POINTER ':' CompoundToken ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN
    {
        MATCH(MDStmt/MD_POINTER ':' CompoundToken ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN);
        MDPointer* pointerNode = new MDPointer(*$1);
        pointerNode->Arg_ = *$3;
        pointerNode->Type_ = *$5;
        pointerNode->Size_ = $7;
        pointerNode->CBNum_ = $9;
        pointerNode->CBOffset_ = $11;
        pointerNode->MemType_ = *$13;
        pointerNode->BufNum_ = $15;
        pointerNode->Alignment_ = $17;
        pointerNode->AccessType_ = *$19;
        pointerNode->Volatile_ = $21;
        pointerNode->Restrict_ = $23;
        $$ = pointerNode;
        delete $1;
        delete $3;
        delete $5;
        delete $13;
        delete $19;
    }
    | MD_INTLIST ':' IntList
    {
        MATCH(MDStmt/MD_INTLIST ':' IntList);
        MDIntList* ilNode = new MDIntList(*$1);
        ilNode->IntList_ = $3;
        $$ = ilNode;
        delete $1;
    }
    | MD_PRINTF ':' IntList ':' STR_TOKEN
    {
        MATCH(MDStmt/MD_PRINTF ':' IntList ':' STR_TOKEN);
        MDPrintf* printfNode = new MDPrintf(*$1);
        printfNode->IntList_ = $3;
        printfNode->StrLen_ = $5->length();
        printfNode->Str_ = new char[$5->length() + 1];
        memset(printfNode->Str_, 0, $5->length() + 1);
        memcpy(printfNode->Str_, $5->data(), printfNode->StrLen_);
        $$ = printfNode;
        delete $1;
        delete $5;
    }
    | MD_LGS
    {
        MATCH(MDStmt/MD_LGS);
        MDFlag* flagNode = new MDFlag(*$1);
        flagNode->Flag_ = true;
        $$ = flagNode;
        delete $1;
    }
    ;

CompoundToken
    : CompoundToken INT_TOKEN
    {
        MATCH(CompoundToken/CompoundToken INT_TOKEN);
        std::stringstream out;
        out << *$1 << $2;
        $$ = new std::string(out.str());
        delete $1;
    }
    | CompoundToken STR_TOKEN
    {
        MATCH(CompoundToken/CompoundToken STR_TOKEN);
        std::stringstream out;
        out << *$1 << *$2;
        $$ = new std::string(out.str());
        delete $1;
        delete $2;
    }
    | STR_TOKEN
    {
        MATCH(CompoundToken/STR_TOKEN);
        $$ = new std::string(*$1);
        delete $1;
    }
    | INT_TOKEN
    {
        MATCH(CompoundToken/INT_TOKEN);
        std::stringstream out;
        out << $1;
        $$ = new std::string(out.str());
    }
    ;

SpaceCompoundToken
    : SpaceCompoundToken INT_TOKEN
    {
        MATCH(SpaceCompoundToken/SpaceCompoundToken INT_TOKEN);
        std::stringstream out;
        out << *$1 << " " << $2;
        $$ = new std::string(out.str());
        delete $1;
    }
    | SpaceCompoundToken STR_TOKEN
    {
        MATCH(SpaceCompoundToken/SpaceCompoundToken STR_TOKEN);
        std::stringstream out;
        out << *$1 << " " << *$2;
        $$ = new std::string(out.str());
        delete $1;
        delete $2;
    }
    | STR_TOKEN
    {
        MATCH(SpaceCompoundToken/STR_TOKEN);
        $$ = new std::string(*$1);
        delete $1;
    }
    | INT_TOKEN
    {
        MATCH(SpaceCompoundToken/INT_TOKEN);
        std::stringstream out;
        out << $1;
        $$ = new std::string(out.str());
    }
    ;

IntList
    : INT_TOKEN
    {
        MATCH(IntList/INT_TOKEN);
        $$ = new std::vector<unsigned>;
        $$->push_back($1);
    }
    | IntList ':' INT_TOKEN
    {
        MATCH(IntList/IntList ':' INT_TOKEN);
        $1->push_back($3);
        $$ = $1;
    }
    ;

StrList
    : STR_TOKEN
    {
        MATCH(StrList/STR_TOKEN);
        $$ = new std::list<std::string*>;
        $$->push_back($1);
    }
    | StrList ':' STR_TOKEN
    {
        MATCH(StrList/StrList:STR_TOKEN);
        $1->push_back($3);
        $$ = $1;
    }

%%

// We have to implement the error function
void llvm::AMDILMDParser::error(const llvm::AMDILMDParser::location_type &loc, const std::string &msg)
{
    _merror = true;
    _mmsg = msg;

}

// Now that we have the parser declared, we can declare the scanner and implement the yylex function
#include "AMDILMDScanner.h"
static int yylex(llvm::AMDILMDParser::semantic_type * yylval,
llvm::AMDILMDParser::location_type * yylloc, llvm::AMDILMDScanner &scanner) {
    return scanner.yylex(yylval, yylloc);
}


