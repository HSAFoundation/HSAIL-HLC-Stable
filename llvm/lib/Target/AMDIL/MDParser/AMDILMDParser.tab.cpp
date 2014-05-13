
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++
   
      Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008 Free Software
   Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* First part of user declarations.  */

/* Line 311 of lalr1.cc  */
#line 12 "AMDILMDParser.y"

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


/* Line 311 of lalr1.cc  */
#line 61 "AMDILMDParser.tab.cpp"


#include "AMDILMDParser.tab.hpp"

/* User implementation prologue.  */


/* Line 317 of lalr1.cc  */
#line 70 "AMDILMDParser.tab.cpp"
/* Unqualified %code blocks.  */

/* Line 318 of lalr1.cc  */
#line 38 "AMDILMDParser.y"

  // prototype for the yylex function
  static int yylex(llvm::AMDILMDParser::semantic_type * yylval,
                   llvm::AMDILMDParser::location_type * yylloc,
                   llvm::AMDILMDScanner &scanner);



/* Line 318 of lalr1.cc  */
#line 84 "AMDILMDParser.tab.cpp"

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* Enable debugging if requested.  */
#if YYDEBUG

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)	\
do {							\
  if (yydebug_)						\
    {							\
      *yycdebug_ << Title << ' ';			\
      yy_symbol_print_ ((Type), (Value), (Location));	\
      *yycdebug_ << std::endl;				\
    }							\
} while (false)

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug_)				\
    yy_reduce_print_ (Rule);		\
} while (false)

# define YY_STACK_PRINT()		\
do {					\
  if (yydebug_)				\
    yystack_print_ ();			\
} while (false)

#else /* !YYDEBUG */

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_REDUCE_PRINT(Rule)
# define YY_STACK_PRINT()

#endif /* !YYDEBUG */

#define yyerrok		(yyerrstatus_ = 0)
#define yyclearin	(yychar = yyempty_)

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


/* Line 380 of lalr1.cc  */
#line 5 "AMDILMDParser.y"
namespace llvm {

/* Line 380 of lalr1.cc  */
#line 152 "AMDILMDParser.tab.cpp"
#if YYERROR_VERBOSE

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  AMDILMDParser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              /* Fall through.  */
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

#endif

  /// Build a parser object.
  AMDILMDParser::AMDILMDParser (llvm::AMDILMDScanner &scanner_yyarg, CompUnit* root_yyarg, bool _merror_yyarg, std::string _mmsg_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      scanner (scanner_yyarg),
      root (root_yyarg),
      _merror (_merror_yyarg),
      _mmsg (_mmsg_yyarg)
  {
  }

  AMDILMDParser::~AMDILMDParser ()
  {
  }

#if YYDEBUG
  /*--------------------------------.
  | Print this symbol on YYOUTPUT.  |
  `--------------------------------*/

  inline void
  AMDILMDParser::yy_symbol_value_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yyvaluep);
    switch (yytype)
      {
         default:
	  break;
      }
  }


  void
  AMDILMDParser::yy_symbol_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    *yycdebug_ << (yytype < yyntokens_ ? "token" : "nterm")
	       << ' ' << yytname_[yytype] << " ("
	       << *yylocationp << ": ";
    yy_symbol_value_print_ (yytype, yyvaluep, yylocationp);
    *yycdebug_ << ')';
  }
#endif

  void
  AMDILMDParser::yydestruct_ (const char* yymsg,
			   int yytype, semantic_type* yyvaluep, location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yymsg);
    YYUSE (yyvaluep);

    YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

    switch (yytype)
      {
  
	default:
	  break;
      }
  }

  void
  AMDILMDParser::yypop_ (unsigned int n)
  {
    yystate_stack_.pop (n);
    yysemantic_stack_.pop (n);
    yylocation_stack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  AMDILMDParser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  AMDILMDParser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  AMDILMDParser::debug_level_type
  AMDILMDParser::debug_level () const
  {
    return yydebug_;
  }

  void
  AMDILMDParser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif

  int
  AMDILMDParser::parse ()
  {
    /// Lookahead and lookahead in internal form.
    int yychar = yyempty_;
    int yytoken = 0;

    /* State.  */
    int yyn;
    int yylen = 0;
    int yystate = 0;

    /* Error handling.  */
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// Semantic value of the lookahead.
    semantic_type yylval;
    /// Location of the lookahead.
    location_type yylloc;
    /// The locations where the error started and ended.
    location_type yyerror_range[2];

    /// $$.
    semantic_type yyval;
    /// @$.
    location_type yyloc;

    int yyresult;

    YYCDEBUG << "Starting parse" << std::endl;


    /* Initialize the stacks.  The initial state will be pushed in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystate_stack_ = state_stack_type (0);
    yysemantic_stack_ = semantic_stack_type (0);
    yylocation_stack_ = location_stack_type (0);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* New state.  */
  yynewstate:
    yystate_stack_.push (yystate);
    YYCDEBUG << "Entering state " << yystate << std::endl;

    /* Accept?  */
    if (yystate == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    /* Backup.  */
  yybackup:

    /* Try to take a decision without lookahead.  */
    yyn = yypact_[yystate];
    if (yyn == yypact_ninf_)
      goto yydefault;

    /* Read a lookahead token.  */
    if (yychar == yyempty_)
      {
	YYCDEBUG << "Reading a token: ";
	yychar = yylex (&yylval, &yylloc, scanner);
      }


    /* Convert token to internal form.  */
    if (yychar <= yyeof_)
      {
	yychar = yytoken = yyeof_;
	YYCDEBUG << "Now at end of input." << std::endl;
      }
    else
      {
	yytoken = yytranslate_ (yychar);
	YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
      }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken)
      goto yydefault;

    /* Reduce or error.  */
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
	if (yyn == 0 || yyn == yytable_ninf_)
	goto yyerrlab;
	yyn = -yyn;
	goto yyreduce;
      }

    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

    /* Discard the token being shifted.  */
    yychar = yyempty_;

    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus_)
      --yyerrstatus_;

    yystate = yyn;
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystate];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.  Otherwise, use the top of the stack.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  */
    if (yylen)
      yyval = yysemantic_stack_[yylen - 1];
    else
      yyval = yysemantic_stack_[0];

    {
      slice<location_type, location_stack_type> slice (yylocation_stack_, yylen);
      YYLLOC_DEFAULT (yyloc, slice, yylen);
    }
    YY_REDUCE_PRINT (yyn);
    switch (yyn)
      {
	  case 5:

/* Line 678 of lalr1.cc  */
#line 109 "AMDILMDParser.y"
    {
        MATCH(CompUnit/CompUnitEnd);
        (yyval.CU) = root;
        delete (yysemantic_stack_[(1) - (1)].str);
    }
    break;

  case 6:

/* Line 678 of lalr1.cc  */
#line 115 "AMDILMDParser.y"
    {
        MATCH(CompUnit/ILStates);
        (yyval.CU) = root;
        (yyval.CU)->addComponents((yysemantic_stack_[(1) - (1)].compVec), CompUnit::IL_STATE);
        delete (yysemantic_stack_[(1) - (1)].compVec);
    }
    break;

  case 7:

/* Line 678 of lalr1.cc  */
#line 122 "AMDILMDParser.y"
    {
        MATCH(CompUnit/MacroStates);
        (yyval.CU) = root;
        (yyval.CU)->addComponents((yysemantic_stack_[(1) - (1)].compVec), CompUnit::IL_STATE);
        delete (yysemantic_stack_[(1) - (1)].compVec);
    }
    break;

  case 8:

/* Line 678 of lalr1.cc  */
#line 129 "AMDILMDParser.y"
    {
        MATCH(CompUnit/DBGStates);
        (yyval.CU) = root;
        (yyval.CU)->addComponents((yysemantic_stack_[(1) - (1)].compVec), CompUnit::DBG_STATE);
        delete (yysemantic_stack_[(1) - (1)].compVec);
    }
    break;

  case 9:

/* Line 678 of lalr1.cc  */
#line 136 "AMDILMDParser.y"
    {
        MATCH(CompUnit/MDStates);
        (yyval.CU) = root;
        (yyval.CU)->addComponents((yysemantic_stack_[(1) - (1)].compVec), CompUnit::MD_STATE);
        delete (yysemantic_stack_[(1) - (1)].compVec);
    }
    break;

  case 10:

/* Line 678 of lalr1.cc  */
#line 143 "AMDILMDParser.y"
    {
        MATCH(CompUnit/ILMain);
        (yyval.CU) = root;
        (yyval.CU)->setMain((yysemantic_stack_[(1) - (1)].ilMain));
    }
    break;

  case 11:

/* Line 678 of lalr1.cc  */
#line 149 "AMDILMDParser.y"
    {
        MATCH(CompUnit/DataStates);
        (yyval.CU) = root;
        (yyval.CU)->addComponents((yysemantic_stack_[(1) - (1)].compVec), CompUnit::DATA_STATE);
        delete (yysemantic_stack_[(1) - (1)].compVec);
    }
    break;

  case 12:

/* Line 678 of lalr1.cc  */
#line 159 "AMDILMDParser.y"
    {
        MATCH(ILMain/IL_VERSION Statements IL_ENDMAIN);
        MainFunc *mainFunc = new MainFunc(*(yysemantic_stack_[(3) - (1)].str));
        (yysemantic_stack_[(3) - (2)].strList)->push_front((yysemantic_stack_[(3) - (1)].str));
        (yysemantic_stack_[(3) - (2)].strList)->push_back((yysemantic_stack_[(3) - (3)].str));
        mainFunc->setStatements((yysemantic_stack_[(3) - (2)].strList));
        (yyval.ilMain) = mainFunc;
    }
    break;

  case 14:

/* Line 678 of lalr1.cc  */
#line 176 "AMDILMDParser.y"
    {
        MATCH(ILStates/ILState);
        (yyval.compVec) = new std::list<Component*>;
        (yyval.compVec)->push_back((yysemantic_stack_[(1) - (1)].funcNode));
    }
    break;

  case 15:

/* Line 678 of lalr1.cc  */
#line 182 "AMDILMDParser.y"
    {
        MATCH(ILStates/ILStates ILState);
        (yysemantic_stack_[(2) - (1)].compVec)->push_back((yysemantic_stack_[(2) - (2)].funcNode));
        (yyval.compVec) = (yysemantic_stack_[(2) - (1)].compVec);
    }
    break;

  case 16:

/* Line 678 of lalr1.cc  */
#line 191 "AMDILMDParser.y"
    {
        MATCH(MacroStates/MacroState);
        (yyval.compVec) = new std::list<Component*>;
        (yyval.compVec)->push_back((yysemantic_stack_[(1) - (1)].funcNode));
    }
    break;

  case 17:

/* Line 678 of lalr1.cc  */
#line 197 "AMDILMDParser.y"
    {
        MATCH(MacroStates/MacroStates MacroState);
        (yysemantic_stack_[(2) - (1)].compVec)->push_back((yysemantic_stack_[(2) - (2)].funcNode));
        (yyval.compVec) = (yysemantic_stack_[(2) - (1)].compVec);
    }
    break;

  case 18:

/* Line 678 of lalr1.cc  */
#line 206 "AMDILMDParser.y"
    {
        MATCH(DBGStates/DBGState);
        (yyval.compVec) = new std::list<Component*>;
        if ((yysemantic_stack_[(1) - (1)].dbNode)) {
            (yyval.compVec)->push_back((yysemantic_stack_[(1) - (1)].dbNode));
        }
    }
    break;

  case 19:

/* Line 678 of lalr1.cc  */
#line 214 "AMDILMDParser.y"
    {
        MATCH(DBGStates/DBGStates DBGState);
        if ((yysemantic_stack_[(2) - (2)].dbNode)) {
            (yysemantic_stack_[(2) - (1)].compVec)->push_back((yysemantic_stack_[(2) - (2)].dbNode));
        }
        (yyval.compVec) = (yysemantic_stack_[(2) - (1)].compVec);
    }
    break;

  case 20:

/* Line 678 of lalr1.cc  */
#line 225 "AMDILMDParser.y"
    {
        MATCH(DataStates/DataState);
        (yyval.compVec) = new std::list<Component*>;
        (yyval.compVec)->push_back((yysemantic_stack_[(1) - (1)].dsNode));
    }
    break;

  case 21:

/* Line 678 of lalr1.cc  */
#line 231 "AMDILMDParser.y"
    {
        MATCH(DataStates/DataStates DataState);
        (yysemantic_stack_[(2) - (1)].compVec)->push_back((yysemantic_stack_[(2) - (2)].dsNode));
        (yyval.compVec) = (yysemantic_stack_[(2) - (1)].compVec);
    }
    break;

  case 22:

/* Line 678 of lalr1.cc  */
#line 241 "AMDILMDParser.y"
    {
        MATCH(MDStates/MDState);
        (yyval.compVec) = new std::list<Component*>;
        (yyval.compVec)->push_back((yysemantic_stack_[(1) - (1)].mdBlockNode));
    }
    break;

  case 23:

/* Line 678 of lalr1.cc  */
#line 247 "AMDILMDParser.y"
    {
        MATCH(MDStates/MDStates MDState);
        (yysemantic_stack_[(2) - (1)].compVec)->push_back((yysemantic_stack_[(2) - (2)].mdBlockNode));
        (yyval.compVec) = (yysemantic_stack_[(2) - (1)].compVec);
    }
    break;

  case 24:

/* Line 678 of lalr1.cc  */
#line 256 "AMDILMDParser.y"
    {
        MATCH(ILState/IL_FUNC STR_TOKEN ';' STR_TOKEN Statements IL_ENDFUNC ';' STR_TOKEN);
        (yysemantic_stack_[(8) - (5)].strList)->push_back(new std::string(*(yysemantic_stack_[(8) - (6)].str)));
        (yysemantic_stack_[(8) - (5)].strList)->push_back(new std::string(";" + *(yysemantic_stack_[(8) - (8)].str)));
        (yyval.funcNode) = new ILFunc(atoi((yysemantic_stack_[(8) - (2)].str)->c_str()), *(yysemantic_stack_[(8) - (4)].str));
        (yysemantic_stack_[(8) - (5)].strList)->push_front(new std::string(";" + *(yysemantic_stack_[(8) - (4)].str)));
        (yysemantic_stack_[(8) - (5)].strList)->push_front(new std::string("func " + *(yysemantic_stack_[(8) - (2)].str)));
        (yyval.funcNode)->setStatements((yysemantic_stack_[(8) - (5)].strList));
        delete (yysemantic_stack_[(8) - (2)].str);
        delete (yysemantic_stack_[(8) - (4)].str);
        delete (yysemantic_stack_[(8) - (6)].str);
        delete (yysemantic_stack_[(8) - (8)].str);
    }
    break;

  case 25:

/* Line 678 of lalr1.cc  */
#line 270 "AMDILMDParser.y"
    {
        MATCH(ILState/IL_FUNC STR_TOKEN ';' STR_TOKEN ';' STR_TOKEN Statements IL_ENDFUNC ';' STR_TOKEN);
        (yysemantic_stack_[(10) - (7)].strList)->push_back(new std::string(*(yysemantic_stack_[(10) - (8)].str)));
        (yysemantic_stack_[(10) - (7)].strList)->push_back(new std::string(";" + *(yysemantic_stack_[(10) - (10)].str)));
        (yysemantic_stack_[(10) - (7)].strList)->push_front(new std::string(";" + *(yysemantic_stack_[(10) - (4)].str)));
        (yysemantic_stack_[(10) - (7)].strList)->push_front(new std::string("func " + *(yysemantic_stack_[(10) - (2)].str)));
        (yyval.funcNode) = new ILFunc(atoi((yysemantic_stack_[(10) - (2)].str)->c_str()), *(yysemantic_stack_[(10) - (4)].str));
        (yyval.funcNode)->setStatements((yysemantic_stack_[(10) - (7)].strList));
        delete (yysemantic_stack_[(10) - (2)].str);
        delete (yysemantic_stack_[(10) - (4)].str);
        delete (yysemantic_stack_[(10) - (6)].str);
        delete (yysemantic_stack_[(10) - (8)].str);
        delete (yysemantic_stack_[(10) - (10)].str);
    }
    break;

  case 26:

/* Line 678 of lalr1.cc  */
#line 288 "AMDILMDParser.y"
    {
        MATCH(MacroState/MACRO_START STR_TOKEN Statements MACRO_END);
        Macro *NewMacro;
        if ((*(yysemantic_stack_[(4) - (2)].str)).find("16383", 0) == std::string::npos) {
          NewMacro = new Macro(*(yysemantic_stack_[(4) - (2)].str));
        } else {
          NewMacro = new DummyMacro(*(yysemantic_stack_[(4) - (2)].str));
        }
        NewMacro->setStatements((yysemantic_stack_[(4) - (3)].strList));
        NewMacro->header_ = *(yysemantic_stack_[(4) - (1)].str);
        NewMacro->footer_ = *(yysemantic_stack_[(4) - (4)].str);
        (yyval.funcNode) = NewMacro;
        delete (yysemantic_stack_[(4) - (1)].str);
        delete (yysemantic_stack_[(4) - (2)].str);
        delete (yysemantic_stack_[(4) - (4)].str);
    }
    break;

  case 27:

/* Line 678 of lalr1.cc  */
#line 305 "AMDILMDParser.y"
    {
        MATCH(MacroState/MACRO_START STR_TOKEN ';' STR_TOKEN Statements MACRO_END ';' STR_TOKEN);
/*
        assert((*$2).substr((*$2).length-9).compare("_outline")==0 &&
               "expecting outlined macro");
*/
        assert((yysemantic_stack_[(8) - (2)].str)->at(0) == '(' && "unexpected syntax");
        (yysemantic_stack_[(8) - (5)].strList)->push_back(new std::string(*(yysemantic_stack_[(8) - (6)].str)));
        (yysemantic_stack_[(8) - (5)].strList)->push_back(new std::string(";" + *(yysemantic_stack_[(8) - (8)].str)));
        (yyval.funcNode) = new ILFunc(atoi(&(yysemantic_stack_[(8) - (2)].str)->c_str()[1]), *(yysemantic_stack_[(8) - (4)].str));
        (yysemantic_stack_[(8) - (5)].strList)->push_front(new std::string(";" + *(yysemantic_stack_[(8) - (4)].str)));
        (yysemantic_stack_[(8) - (5)].strList)->push_front(new std::string("mdef" + *(yysemantic_stack_[(8) - (2)].str)));
        (yyval.funcNode)->setStatements((yysemantic_stack_[(8) - (5)].strList));
        delete (yysemantic_stack_[(8) - (2)].str);
        delete (yysemantic_stack_[(8) - (4)].str);
        delete (yysemantic_stack_[(8) - (6)].str);
        delete (yysemantic_stack_[(8) - (8)].str);
    }
    break;

  case 28:

/* Line 678 of lalr1.cc  */
#line 324 "AMDILMDParser.y"
    {
        MATCH(MacroState/MACRO_START STR_TOKEN ';' STR_TOKEN ';' STR_TOKEN Statements MACRO_END ';' STR_TOKEN);
        assert((yysemantic_stack_[(10) - (2)].str)->at(0) == '(' && "unexpected syntax");
        (yysemantic_stack_[(10) - (7)].strList)->push_back(new std::string(*(yysemantic_stack_[(10) - (8)].str)));
        (yysemantic_stack_[(10) - (7)].strList)->push_back(new std::string(";" + *(yysemantic_stack_[(10) - (10)].str)));
        (yysemantic_stack_[(10) - (7)].strList)->push_front(new std::string(";" + *(yysemantic_stack_[(10) - (4)].str)));
        (yysemantic_stack_[(10) - (7)].strList)->push_front(new std::string("mdef" + *(yysemantic_stack_[(10) - (2)].str)));
        (yyval.funcNode) = new ILFunc(atoi(&(yysemantic_stack_[(10) - (2)].str)->c_str()[1]), *(yysemantic_stack_[(10) - (4)].str));
        (yyval.funcNode)->setStatements((yysemantic_stack_[(10) - (7)].strList));
        delete (yysemantic_stack_[(10) - (2)].str);
        delete (yysemantic_stack_[(10) - (4)].str);
        delete (yysemantic_stack_[(10) - (6)].str);
        delete (yysemantic_stack_[(10) - (8)].str);
        delete (yysemantic_stack_[(10) - (10)].str);
    }
    break;

  case 29:

/* Line 678 of lalr1.cc  */
#line 343 "AMDILMDParser.y"
    {
        MATCH(DBGState/DBG_START Statements DBG_END);
        (yyval.dbNode) = new DBSection("");
        (yyval.dbNode)->setStatements((yysemantic_stack_[(3) - (2)].strList));
    }
    break;

  case 30:

/* Line 678 of lalr1.cc  */
#line 349 "AMDILMDParser.y"
    {
        MATCH(DBGState/DBG_START DBG_END);
        (yyval.dbNode) = new DBSection("");
    }
    break;

  case 31:

/* Line 678 of lalr1.cc  */
#line 357 "AMDILMDParser.y"
    {
        MATCH(DataState/DATA_START ':' STR_TOKEN ':' STR_TOKEN DataEntries ';' '#' DATA_END ':' INT_TOKEN);
        assert((unsigned)atoi((yysemantic_stack_[(11) - (3)].str)->c_str()) == (yysemantic_stack_[(11) - (11)].token) && "Error in parsing DATA segment. "
        "Start ID and End ID differ!");
        (yyval.dsNode) = new DataSection(*(yysemantic_stack_[(11) - (3)].str));
        (yyval.dsNode)->Size_ = atoi((yysemantic_stack_[(11) - (5)].str)->c_str());
        (yyval.dsNode)->Data_ = (yysemantic_stack_[(11) - (6)].dsEntryVec);
        delete (yysemantic_stack_[(11) - (3)].str);
        delete (yysemantic_stack_[(11) - (5)].str);
        delete (yysemantic_stack_[(11) - (9)].str);
    }
    break;

  case 32:

/* Line 678 of lalr1.cc  */
#line 369 "AMDILMDParser.y"
    {
        MATCH(DataState/DATA_START ':' STR_TOKEN DataEntries ';' '#' DATA_END);
        (yyval.dsNode) = new DataSection("Software");
        (yyval.dsNode)->Size_ = atoi((yysemantic_stack_[(7) - (3)].str)->c_str());
        (yyval.dsNode)->Data_ = (yysemantic_stack_[(7) - (4)].dsEntryVec);
        delete (yysemantic_stack_[(7) - (3)].str);
        delete (yysemantic_stack_[(7) - (7)].str);
    }
    break;

  case 33:

/* Line 678 of lalr1.cc  */
#line 381 "AMDILMDParser.y"
    {
        MATCH(DataEntries/DataEntry);
        (yyval.dsEntryVec) = new std::vector<Data*>;
        (yyval.dsEntryVec)->push_back((yysemantic_stack_[(1) - (1)].dsEntry));
    }
    break;

  case 34:

/* Line 678 of lalr1.cc  */
#line 387 "AMDILMDParser.y"
    {
        MATCH(DataEntries/DataEntries DataEntry);
        (yysemantic_stack_[(2) - (1)].dsEntryVec)->push_back((yysemantic_stack_[(2) - (2)].dsEntry));
        (yyval.dsEntryVec) = (yysemantic_stack_[(2) - (1)].dsEntryVec);
    }
    break;

  case 35:

/* Line 678 of lalr1.cc  */
#line 396 "AMDILMDParser.y"
    {
        MATCH(DataEntry/';' '#' CompoundToken ':' StrList);
        (yyval.dsEntry) = new Data(*(yysemantic_stack_[(5) - (3)].str));
        (yyval.dsEntry)->Data_ = (yysemantic_stack_[(5) - (5)].strList);
        delete (yysemantic_stack_[(5) - (3)].str);
    }
    break;

  case 36:

/* Line 678 of lalr1.cc  */
#line 406 "AMDILMDParser.y"
    {
        MATCH(Statements/STR_TOKEN);
        (yyval.strList) = new std::list<std::string*>;
        (yyval.strList)->push_back((yysemantic_stack_[(1) - (1)].str));
    }
    break;

  case 37:

/* Line 678 of lalr1.cc  */
#line 412 "AMDILMDParser.y"
    {
        MATCH(Statements/Statements STR_TOKEN);
        (yysemantic_stack_[(2) - (1)].strList)->push_back((yysemantic_stack_[(2) - (2)].str));
        (yyval.strList) = (yysemantic_stack_[(2) - (1)].strList);
    }
    break;

  case 38:

/* Line 678 of lalr1.cc  */
#line 421 "AMDILMDParser.y"
    {
        MATCH(MDState/ARG_START ':' STR_TOKEN MDStmts  ARG_END ':' STR_TOKEN);
        assert(*(yysemantic_stack_[(7) - (3)].str) == *(yysemantic_stack_[(7) - (7)].str) && "Error in parsing ARG segment. Start ID and End ID"
        " differ!");
        (yyval.mdBlockNode) = (class MDBlock*)(new MDBlock(*(yysemantic_stack_[(7) - (3)].str)));
        (yyval.mdBlockNode)->MDVals = (yysemantic_stack_[(7) - (4)].mdNodeVec);
        delete (yysemantic_stack_[(7) - (3)].str);
        delete (yysemantic_stack_[(7) - (7)].str);
    }
    break;

  case 39:

/* Line 678 of lalr1.cc  */
#line 434 "AMDILMDParser.y"
    {
        MATCH(MDStmts/MDStmt);
        (yyval.mdNodeVec) = new std::vector<class MDType*>;
        (yyval.mdNodeVec)->push_back((yysemantic_stack_[(1) - (1)].MDNode));
    }
    break;

  case 40:

/* Line 678 of lalr1.cc  */
#line 440 "AMDILMDParser.y"
    {
        MATCH(MDStmts/MDStmts MDStmt);
        (yysemantic_stack_[(2) - (1)].mdNodeVec)->push_back((yysemantic_stack_[(2) - (2)].MDNode));
        (yyval.mdNodeVec) = (yysemantic_stack_[(2) - (1)].mdNodeVec);
    }
    break;

  case 41:

/* Line 678 of lalr1.cc  */
#line 449 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_STRING ':' CompoundToken);
        MDStrings* strNode = new MDStrings(*(yysemantic_stack_[(3) - (1)].str));
        strNode->Str_ = *(yysemantic_stack_[(3) - (3)].str);
        (yyval.MDNode) = strNode;
        delete (yysemantic_stack_[(3) - (1)].str);
        delete (yysemantic_stack_[(3) - (3)].str);
    }
    break;

  case 42:

/* Line 678 of lalr1.cc  */
#line 458 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_ERRWAR ':' CompoundToken ':' SpaceCompoundToken);
        MDStrings* strNode = new MDStrings(*(yysemantic_stack_[(5) - (1)].str));
        strNode->Str_ = *(yysemantic_stack_[(5) - (3)].str) + ':' + *(yysemantic_stack_[(5) - (5)].str);
        (yyval.MDNode) = strNode;
        delete (yysemantic_stack_[(5) - (1)].str);
        delete (yysemantic_stack_[(5) - (3)].str);
        delete (yysemantic_stack_[(5) - (5)].str);
    }
    break;

  case 43:

/* Line 678 of lalr1.cc  */
#line 468 "AMDILMDParser.y"
    {
      MATCH(MDStmt/MD_REFLECTION ':' INT_TOKEN ':' CompoundToken);
      MDReflection *refNode = new MDReflection(*(yysemantic_stack_[(5) - (5)].str));
      refNode->Int_ = (yysemantic_stack_[(5) - (3)].token);
      (yyval.MDNode) = refNode;
      delete (yysemantic_stack_[(5) - (1)].str);
      delete (yysemantic_stack_[(5) - (5)].str);
    }
    break;

  case 44:

/* Line 678 of lalr1.cc  */
#line 477 "AMDILMDParser.y"
    {
      MATCH(MDStmt/MD_CONSTARG ':' INT_TOKEN ':' CompoundToken);
      MDConstArg *refNode = new MDConstArg(*(yysemantic_stack_[(5) - (5)].str));
      refNode->Int_ = (yysemantic_stack_[(5) - (3)].token);
      (yyval.MDNode) = refNode;
      delete (yysemantic_stack_[(5) - (1)].str);
      delete (yysemantic_stack_[(5) - (5)].str);
    }
    break;

  case 45:

/* Line 678 of lalr1.cc  */
#line 486 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_MEMORY ':' STR_TOKEN ':' INT_TOKEN);
        MDMemory* memNode = new MDMemory(*(yysemantic_stack_[(5) - (1)].str));
        memNode->AS_ = *(yysemantic_stack_[(5) - (3)].str);
        memNode->Size_ = (yysemantic_stack_[(5) - (5)].token);
        (yyval.MDNode) = memNode;
        delete (yysemantic_stack_[(5) - (1)].str);
        delete (yysemantic_stack_[(5) - (3)].str);
    }
    break;

  case 46:

/* Line 678 of lalr1.cc  */
#line 496 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_MEMORY ':' STR_TOKEN);
        MDFlag* boolNode = new MDFlag(*(yysemantic_stack_[(3) - (3)].str));
        boolNode->Flag_ = true;
        (yyval.MDNode) = boolNode;
        delete (yysemantic_stack_[(3) - (1)].str);
        delete (yysemantic_stack_[(3) - (3)].str);
    }
    break;

  case 47:

/* Line 678 of lalr1.cc  */
#line 505 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_INT ':' INT_TOKEN);
        MDInt* intNode = new MDInt(*(yysemantic_stack_[(3) - (1)].str));
        intNode->Int_ = (yysemantic_stack_[(3) - (3)].token);
        (yyval.MDNode) = intNode;
        delete (yysemantic_stack_[(3) - (1)].str);
    }
    break;

  case 48:

/* Line 678 of lalr1.cc  */
#line 513 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_SAMPLER ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN);
        MDSampler* samplerNode = new MDSampler(*(yysemantic_stack_[(9) - (1)].str));
        samplerNode->Arg_ = *(yysemantic_stack_[(9) - (3)].str);
        samplerNode->ID_ = (yysemantic_stack_[(9) - (5)].token);
        samplerNode->isArg_ = !(yysemantic_stack_[(9) - (7)].token);
        samplerNode->Val_ = (yysemantic_stack_[(9) - (9)].token);
        (yyval.MDNode) = samplerNode;
        delete (yysemantic_stack_[(9) - (1)].str);
        delete (yysemantic_stack_[(9) - (3)].str);
    }
    break;

  case 49:

/* Line 678 of lalr1.cc  */
#line 525 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_IMAGE ':' CompoundToken ':' CompoundToken ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN);
        MDImage* imageNode = new MDImage(*(yysemantic_stack_[(13) - (1)].str));
        imageNode->Arg_ = *(yysemantic_stack_[(13) - (3)].str);
        imageNode->Dim_ = *(yysemantic_stack_[(13) - (5)].str);
        imageNode->Type_ = *(yysemantic_stack_[(13) - (7)].str);
        imageNode->ID_ = (yysemantic_stack_[(13) - (9)].token);
        imageNode->CBNum_ = (yysemantic_stack_[(13) - (11)].token);
        imageNode->CBOffset_ = (yysemantic_stack_[(13) - (13)].token);
        (yyval.MDNode) = imageNode;
        delete (yysemantic_stack_[(13) - (1)].str);
        delete (yysemantic_stack_[(13) - (3)].str);
        delete (yysemantic_stack_[(13) - (5)].str);
        delete (yysemantic_stack_[(13) - (7)].str);
    }
    break;

  case 50:

/* Line 678 of lalr1.cc  */
#line 541 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_SEMAPHORE ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN);
        MDSemaphore* semaNode = new MDSemaphore(*(yysemantic_stack_[(9) - (1)].str));
        semaNode->Arg_ = *(yysemantic_stack_[(9) - (3)].str);
        semaNode->ID_ = (yysemantic_stack_[(9) - (5)].token);
        semaNode->CBNum_ = (yysemantic_stack_[(9) - (7)].token);
        semaNode->CBOffset_ = (yysemantic_stack_[(9) - (9)].token);
        (yyval.MDNode) = semaNode;
        delete (yysemantic_stack_[(9) - (1)].str);
        delete (yysemantic_stack_[(9) - (3)].str);
    }
    break;

  case 51:

/* Line 678 of lalr1.cc  */
#line 553 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_COUNTER ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN);
        MDCounter* counterNode = new MDCounter(*(yysemantic_stack_[(11) - (1)].str));
        counterNode->Arg_ = *(yysemantic_stack_[(11) - (3)].str);
        counterNode->Size_ = (yysemantic_stack_[(11) - (5)].token);
        counterNode->ID_ = (yysemantic_stack_[(11) - (7)].token);
        counterNode->CBNum_ = (yysemantic_stack_[(11) - (9)].token);
        counterNode->CBOffset_ = (yysemantic_stack_[(11) - (11)].token);
        (yyval.MDNode) = counterNode;
        delete (yysemantic_stack_[(11) - (1)].str);
        delete (yysemantic_stack_[(11) - (3)].str);
    }
    break;

  case 52:

/* Line 678 of lalr1.cc  */
#line 566 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_VALUE ':' CompoundToken ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN);
        MDValue* valueNode = new MDValue(*(yysemantic_stack_[(11) - (1)].str));
        valueNode->Arg_ = *(yysemantic_stack_[(11) - (3)].str);
        valueNode->Type_ = *(yysemantic_stack_[(11) - (5)].str);
        valueNode->Size_ = (yysemantic_stack_[(11) - (7)].token);
        valueNode->CBNum_ = (yysemantic_stack_[(11) - (9)].token);
        valueNode->CBOffset_ = (yysemantic_stack_[(11) - (11)].token);
        (yyval.MDNode) = valueNode;
        delete (yysemantic_stack_[(11) - (1)].str);
        delete (yysemantic_stack_[(11) - (3)].str);
        delete (yysemantic_stack_[(11) - (5)].str);
    }
    break;

  case 53:

/* Line 678 of lalr1.cc  */
#line 580 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_POINTER ':' CompoundToken ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN);
        MDPointer* pointerNode = new MDPointer(*(yysemantic_stack_[(17) - (1)].str));
        pointerNode->Arg_ = *(yysemantic_stack_[(17) - (3)].str);
        pointerNode->Type_ = *(yysemantic_stack_[(17) - (5)].str);
        pointerNode->Size_ = (yysemantic_stack_[(17) - (7)].token);
        pointerNode->CBNum_ = (yysemantic_stack_[(17) - (9)].token);
        pointerNode->CBOffset_ = (yysemantic_stack_[(17) - (11)].token);
        pointerNode->MemType_ = *(yysemantic_stack_[(17) - (13)].str);
        pointerNode->BufNum_ = (yysemantic_stack_[(17) - (15)].token);
        pointerNode->Alignment_ = (yysemantic_stack_[(17) - (17)].token);
        (yyval.MDNode) = pointerNode;
        delete (yysemantic_stack_[(17) - (1)].str);
        delete (yysemantic_stack_[(17) - (3)].str);
        delete (yysemantic_stack_[(17) - (5)].str);
        delete (yysemantic_stack_[(17) - (13)].str);
    }
    break;

  case 54:

/* Line 678 of lalr1.cc  */
#line 598 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_POINTER ':' CompoundToken ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN);
        MDPointer* pointerNode = new MDPointer(*(yysemantic_stack_[(19) - (1)].str));
        pointerNode->Arg_ = *(yysemantic_stack_[(19) - (3)].str);
        pointerNode->Type_ = *(yysemantic_stack_[(19) - (5)].str);
        pointerNode->Size_ = (yysemantic_stack_[(19) - (7)].token);
        pointerNode->CBNum_ = (yysemantic_stack_[(19) - (9)].token);
        pointerNode->CBOffset_ = (yysemantic_stack_[(19) - (11)].token);
        pointerNode->MemType_ = *(yysemantic_stack_[(19) - (13)].str);
        pointerNode->BufNum_ = (yysemantic_stack_[(19) - (15)].token);
        pointerNode->Alignment_ = (yysemantic_stack_[(19) - (17)].token);
        pointerNode->AccessType_ = *(yysemantic_stack_[(19) - (19)].str);
        (yyval.MDNode) = pointerNode;
        delete (yysemantic_stack_[(19) - (1)].str);
        delete (yysemantic_stack_[(19) - (3)].str);
        delete (yysemantic_stack_[(19) - (5)].str);
        delete (yysemantic_stack_[(19) - (13)].str);
        delete (yysemantic_stack_[(19) - (19)].str);
    }
    break;

  case 55:

/* Line 678 of lalr1.cc  */
#line 618 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_POINTER ':' CompoundToken ':' CompoundToken ':' INT_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN ':' STR_TOKEN ':' INT_TOKEN ':' INT_TOKEN);
        MDPointer* pointerNode = new MDPointer(*(yysemantic_stack_[(23) - (1)].str));
        pointerNode->Arg_ = *(yysemantic_stack_[(23) - (3)].str);
        pointerNode->Type_ = *(yysemantic_stack_[(23) - (5)].str);
        pointerNode->Size_ = (yysemantic_stack_[(23) - (7)].token);
        pointerNode->CBNum_ = (yysemantic_stack_[(23) - (9)].token);
        pointerNode->CBOffset_ = (yysemantic_stack_[(23) - (11)].token);
        pointerNode->MemType_ = *(yysemantic_stack_[(23) - (13)].str);
        pointerNode->BufNum_ = (yysemantic_stack_[(23) - (15)].token);
        pointerNode->Alignment_ = (yysemantic_stack_[(23) - (17)].token);
        pointerNode->AccessType_ = *(yysemantic_stack_[(23) - (19)].str);
        pointerNode->Volatile_ = (yysemantic_stack_[(23) - (21)].token);
        pointerNode->Restrict_ = (yysemantic_stack_[(23) - (23)].token);
        (yyval.MDNode) = pointerNode;
        delete (yysemantic_stack_[(23) - (1)].str);
        delete (yysemantic_stack_[(23) - (3)].str);
        delete (yysemantic_stack_[(23) - (5)].str);
        delete (yysemantic_stack_[(23) - (13)].str);
        delete (yysemantic_stack_[(23) - (19)].str);
    }
    break;

  case 56:

/* Line 678 of lalr1.cc  */
#line 640 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_INTLIST ':' IntList);
        MDIntList* ilNode = new MDIntList(*(yysemantic_stack_[(3) - (1)].str));
        ilNode->IntList_ = (yysemantic_stack_[(3) - (3)].intVec);
        (yyval.MDNode) = ilNode;
        delete (yysemantic_stack_[(3) - (1)].str);
    }
    break;

  case 57:

/* Line 678 of lalr1.cc  */
#line 648 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_PRINTF ':' IntList ':' STR_TOKEN);
        MDPrintf* printfNode = new MDPrintf(*(yysemantic_stack_[(5) - (1)].str));
        printfNode->IntList_ = (yysemantic_stack_[(5) - (3)].intVec);
        printfNode->StrLen_ = (yysemantic_stack_[(5) - (5)].str)->length();
        printfNode->Str_ = new char[(yysemantic_stack_[(5) - (5)].str)->length() + 1];
        memset(printfNode->Str_, 0, (yysemantic_stack_[(5) - (5)].str)->length() + 1);
        memcpy(printfNode->Str_, (yysemantic_stack_[(5) - (5)].str)->data(), printfNode->StrLen_);
        (yyval.MDNode) = printfNode;
        delete (yysemantic_stack_[(5) - (1)].str);
        delete (yysemantic_stack_[(5) - (5)].str);
    }
    break;

  case 58:

/* Line 678 of lalr1.cc  */
#line 661 "AMDILMDParser.y"
    {
        MATCH(MDStmt/MD_LGS);
        MDFlag* flagNode = new MDFlag(*(yysemantic_stack_[(1) - (1)].str));
        flagNode->Flag_ = true;
        (yyval.MDNode) = flagNode;
        delete (yysemantic_stack_[(1) - (1)].str);
    }
    break;

  case 59:

/* Line 678 of lalr1.cc  */
#line 672 "AMDILMDParser.y"
    {
        MATCH(CompoundToken/CompoundToken INT_TOKEN);
        std::stringstream out;
        out << *(yysemantic_stack_[(2) - (1)].str) << (yysemantic_stack_[(2) - (2)].token);
        (yyval.str) = new std::string(out.str());
        delete (yysemantic_stack_[(2) - (1)].str);
    }
    break;

  case 60:

/* Line 678 of lalr1.cc  */
#line 680 "AMDILMDParser.y"
    {
        MATCH(CompoundToken/CompoundToken STR_TOKEN);
        std::stringstream out;
        out << *(yysemantic_stack_[(2) - (1)].str) << *(yysemantic_stack_[(2) - (2)].str);
        (yyval.str) = new std::string(out.str());
        delete (yysemantic_stack_[(2) - (1)].str);
        delete (yysemantic_stack_[(2) - (2)].str);
    }
    break;

  case 61:

/* Line 678 of lalr1.cc  */
#line 689 "AMDILMDParser.y"
    {
        MATCH(CompoundToken/STR_TOKEN);
        (yyval.str) = new std::string(*(yysemantic_stack_[(1) - (1)].str));
        delete (yysemantic_stack_[(1) - (1)].str);
    }
    break;

  case 62:

/* Line 678 of lalr1.cc  */
#line 695 "AMDILMDParser.y"
    {
        MATCH(CompoundToken/INT_TOKEN);
        std::stringstream out;
        out << (yysemantic_stack_[(1) - (1)].token);
        (yyval.str) = new std::string(out.str());
    }
    break;

  case 63:

/* Line 678 of lalr1.cc  */
#line 705 "AMDILMDParser.y"
    {
        MATCH(SpaceCompoundToken/SpaceCompoundToken INT_TOKEN);
        std::stringstream out;
        out << *(yysemantic_stack_[(2) - (1)].str) << " " << (yysemantic_stack_[(2) - (2)].token);
        (yyval.str) = new std::string(out.str());
        delete (yysemantic_stack_[(2) - (1)].str);
    }
    break;

  case 64:

/* Line 678 of lalr1.cc  */
#line 713 "AMDILMDParser.y"
    {
        MATCH(SpaceCompoundToken/SpaceCompoundToken STR_TOKEN);
        std::stringstream out;
        out << *(yysemantic_stack_[(2) - (1)].str) << " " << *(yysemantic_stack_[(2) - (2)].str);
        (yyval.str) = new std::string(out.str());
        delete (yysemantic_stack_[(2) - (1)].str);
        delete (yysemantic_stack_[(2) - (2)].str);
    }
    break;

  case 65:

/* Line 678 of lalr1.cc  */
#line 722 "AMDILMDParser.y"
    {
        MATCH(SpaceCompoundToken/STR_TOKEN);
        (yyval.str) = new std::string(*(yysemantic_stack_[(1) - (1)].str));
        delete (yysemantic_stack_[(1) - (1)].str);
    }
    break;

  case 66:

/* Line 678 of lalr1.cc  */
#line 728 "AMDILMDParser.y"
    {
        MATCH(SpaceCompoundToken/INT_TOKEN);
        std::stringstream out;
        out << (yysemantic_stack_[(1) - (1)].token);
        (yyval.str) = new std::string(out.str());
    }
    break;

  case 67:

/* Line 678 of lalr1.cc  */
#line 738 "AMDILMDParser.y"
    {
        MATCH(IntList/INT_TOKEN);
        (yyval.intVec) = new std::vector<unsigned>;
        (yyval.intVec)->push_back((yysemantic_stack_[(1) - (1)].token));
    }
    break;

  case 68:

/* Line 678 of lalr1.cc  */
#line 744 "AMDILMDParser.y"
    {
        MATCH(IntList/IntList ':' INT_TOKEN);
        (yysemantic_stack_[(3) - (1)].intVec)->push_back((yysemantic_stack_[(3) - (3)].token));
        (yyval.intVec) = (yysemantic_stack_[(3) - (1)].intVec);
    }
    break;

  case 69:

/* Line 678 of lalr1.cc  */
#line 753 "AMDILMDParser.y"
    {
        MATCH(StrList/STR_TOKEN);
        (yyval.strList) = new std::list<std::string*>;
        (yyval.strList)->push_back((yysemantic_stack_[(1) - (1)].str));
    }
    break;

  case 70:

/* Line 678 of lalr1.cc  */
#line 759 "AMDILMDParser.y"
    {
        MATCH(StrList/StrList:STR_TOKEN);
        (yysemantic_stack_[(3) - (1)].strList)->push_back((yysemantic_stack_[(3) - (3)].str));
        (yyval.strList) = (yysemantic_stack_[(3) - (1)].strList);
    }
    break;



/* Line 678 of lalr1.cc  */
#line 1361 "AMDILMDParser.tab.cpp"
	default:
          break;
      }
    YY_SYMBOL_PRINT ("-> $$ =", yyr1_[yyn], &yyval, &yyloc);

    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();

    yysemantic_stack_.push (yyval);
    yylocation_stack_.push (yyloc);

    /* Shift the result of the reduction.  */
    yyn = yyr1_[yyn];
    yystate = yypgoto_[yyn - yyntokens_] + yystate_stack_[0];
    if (0 <= yystate && yystate <= yylast_
	&& yycheck_[yystate] == yystate_stack_[0])
      yystate = yytable_[yystate];
    else
      yystate = yydefgoto_[yyn - yyntokens_];
    goto yynewstate;

  /*------------------------------------.
  | yyerrlab -- here on detecting error |
  `------------------------------------*/
  yyerrlab:
    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_)
      {
	++yynerrs_;
	error (yylloc, yysyntax_error_ (yystate));
      }

    yyerror_range[0] = yylloc;
    if (yyerrstatus_ == 3)
      {
	/* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

	if (yychar <= yyeof_)
	  {
	  /* Return failure if at end of input.  */
	  if (yychar == yyeof_)
	    YYABORT;
	  }
	else
	  {
	    yydestruct_ ("Error: discarding", yytoken, &yylval, &yylloc);
	    yychar = yyempty_;
	  }
      }

    /* Else will try to reuse lookahead token after shifting the error
       token.  */
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;

    yyerror_range[0] = yylocation_stack_[yylen - 1];
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    yystate = yystate_stack_[0];
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;	/* Each real token shifted decrements this.  */

    for (;;)
      {
	yyn = yypact_[yystate];
	if (yyn != yypact_ninf_)
	{
	  yyn += yyterror_;
	  if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
	    {
	      yyn = yytable_[yyn];
	      if (0 < yyn)
		break;
	    }
	}

	/* Pop the current state because it cannot handle the error token.  */
	if (yystate_stack_.height () == 1)
	YYABORT;

	yyerror_range[0] = yylocation_stack_[0];
	yydestruct_ ("Error: popping",
		     yystos_[yystate],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);
	yypop_ ();
	yystate = yystate_stack_[0];
	YY_STACK_PRINT ();
      }

    yyerror_range[1] = yylloc;
    // Using YYLLOC is tempting, but would change the location of
    // the lookahead.  YYLOC is available though.
    YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yyloc);

    /* Shift the error token.  */
    YY_SYMBOL_PRINT ("Shifting", yystos_[yyn],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);

    yystate = yyn;
    goto yynewstate;

    /* Accept.  */
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    /* Abort.  */
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (yychar != yyempty_)
      yydestruct_ ("Cleanup: discarding lookahead", yytoken, &yylval, &yylloc);

    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (yystate_stack_.height () != 1)
      {
	yydestruct_ ("Cleanup: popping",
		   yystos_[yystate_stack_[0]],
		   &yysemantic_stack_[0],
		   &yylocation_stack_[0]);
	yypop_ ();
      }

    return yyresult;
  }

  // Generate an error message.
  std::string
  AMDILMDParser::yysyntax_error_ (int yystate)
  {
    std::string res;
    YYUSE (yystate);
#if YYERROR_VERBOSE
    int yyn = yypact_[yystate];
    if (yypact_ninf_ < yyn && yyn <= yylast_)
      {
	/* Start YYX at -YYN if negative to avoid negative indexes in
	   YYCHECK.  */
	int yyxbegin = yyn < 0 ? -yyn : 0;

	/* Stay within bounds of both yycheck and yytname.  */
	int yychecklim = yylast_ - yyn + 1;
	int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
	int count = 0;
	for (int x = yyxbegin; x < yyxend; ++x)
	  if (yycheck_[x + yyn] == x && x != yyterror_)
	    ++count;

	// FIXME: This method of building the message is not compatible
	// with internationalization.  It should work like yacc.c does it.
	// That is, first build a string that looks like this:
	// "syntax error, unexpected %s or %s or %s"
	// Then, invoke YY_ on this string.
	// Finally, use the string as a format to output
	// yytname_[tok], etc.
	// Until this gets fixed, this message appears in English only.
	res = "syntax error, unexpected ";
	res += yytnamerr_ (yytname_[tok]);
	if (count < 5)
	  {
	    count = 0;
	    for (int x = yyxbegin; x < yyxend; ++x)
	      if (yycheck_[x + yyn] == x && x != yyterror_)
		{
		  res += (!count++) ? ", expecting " : " or ";
		  res += yytnamerr_ (yytname_[x]);
		}
	  }
      }
    else
#endif
      res = YY_("syntax error");
    return res;
  }


  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
  const signed char AMDILMDParser::yypact_ninf_ = -68;
  const short int
  AMDILMDParser::yypact_[] =
  {
        62,   -68,   -22,    23,    26,    77,    46,    50,    86,    62,
     -68,   -68,   -68,    68,    79,   115,    94,    87,   -68,   -68,
     -68,   -68,   -68,    89,   -68,    73,   112,     1,   -68,    49,
     141,   -68,   -68,   -68,   -68,   -68,   -68,   -68,   121,   -68,
     -68,   142,   143,    -3,   -68,    47,   116,   117,   118,   119,
     120,   122,   123,   124,   125,   126,   -68,   127,   128,   129,
     130,   104,   -68,    30,    44,   -68,   114,   150,   132,   -68,
      92,    92,   161,    92,   163,   164,    92,    92,   164,    92,
      92,   165,   166,    92,   137,   -68,   167,    63,   168,    67,
      92,   144,   139,   -68,   -68,   -68,     4,    11,   145,    14,
     -68,   -68,   146,    16,    19,   147,    96,    22,   148,   149,
      28,   170,    23,   151,    23,   152,    32,   153,    25,   -68,
     -68,    92,   172,   174,   183,   184,    92,    92,    98,   185,
      92,    92,   100,   -68,    65,   186,    70,   187,   188,   155,
     -68,    35,   160,   -68,   162,   -68,    37,    39,   -68,   169,
      96,    96,   -68,   -68,   138,   171,   -68,   173,   -68,   -68,
     175,    42,   191,   193,   194,   195,   196,   198,   -68,   -68,
     200,   202,   203,   176,   177,   178,   179,   180,   181,   182,
     -68,   -68,   -68,   213,   214,   215,   216,   217,   218,   219,
     -68,   190,   -68,   192,   197,   199,   -68,   221,   223,   224,
     225,   -68,   -68,   201,   204,   227,   226,   205,   -68,   230,
     206,   232,   207,   237,   209,   233,   210,   241,   -68
  };

  /* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
     doesn't specify something else to do.  Zero means the default is an
     error.  */
  const unsigned char
  AMDILMDParser::yydefact_[] =
  {
         0,    13,     0,     0,     0,     0,     0,     0,     0,     2,
       3,    10,     5,     6,     7,     8,    11,     9,    14,    16,
      18,    20,    22,     0,    36,     0,     0,     0,    30,     0,
       0,     1,     4,    15,    17,    19,    21,    23,     0,    37,
      12,     0,     0,     0,    29,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    58,     0,     0,     0,
       0,     0,    39,     0,     0,    26,     0,     0,     0,    33,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,    34,    62,    61,     0,     0,    46,     0,
      47,    67,    56,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    59,
      60,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    38,     0,     0,     0,     0,     0,     0,
      32,     0,     0,    45,     0,    68,     0,     0,    57,     0,
      43,    44,    66,    65,    42,     0,    24,     0,    27,    69,
      35,     0,     0,     0,     0,     0,     0,     0,    63,    64,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      25,    28,    70,     0,     0,     0,     0,     0,     0,     0,
      31,     0,    48,     0,     0,     0,    50,     0,     0,     0,
       0,    52,    51,     0,     0,     0,     0,     0,    49,     0,
       0,     0,    53,     0,    54,     0,     0,     0,    55
  };

  /* YYPGOTO[NTERM-NUM].  */
  const short int
  AMDILMDParser::yypgoto_[] =
  {
       -68,   -68,   -68,   236,   -68,   -68,   -68,   -68,   -68,   -68,
     -68,   234,   235,   231,   238,   108,   -66,    -6,   239,   -68,
     189,   -67,   -68,   208,   -68
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const short int
  AMDILMDParser::yydefgoto_[] =
  {
        -1,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    68,    69,    25,    22,    61,
      62,   116,   154,   102,   160
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If zero, do what YYDEFACT says.  */
  const signed char AMDILMDParser::yytable_ninf_ = -1;
  const unsigned char
  AMDILMDParser::yytable_[] =
  {
        29,    39,    93,    96,    97,    24,    99,   119,   120,   103,
     104,    23,   106,   107,   119,   120,   110,   119,   120,   119,
     120,    43,   119,   120,    65,   119,   120,    24,    94,    95,
      26,   119,   120,    42,    24,   119,   120,   121,   119,   120,
     119,   120,   119,   120,   122,    94,    95,   124,    24,   126,
      24,    93,   127,    39,   141,   129,   140,    87,    89,   146,
     147,   132,    86,   150,   151,   138,     1,    39,   162,    39,
     165,    39,   166,   173,    39,    28,    88,    39,    44,    66,
      67,    27,     2,    30,     3,     4,    31,   113,     5,   155,
       6,     4,     7,    38,   115,    94,    95,   157,    40,   119,
     120,   145,   148,   152,   153,     5,   134,     2,   136,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,     7,    84,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,   168,   169,     6,    41,    45,    63,    64,    90,    70,
      71,    72,    73,    74,    91,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    92,    98,   100,   101,   108,   109,
     111,   112,   114,   118,   133,   142,    66,   143,   123,   125,
     128,   130,   131,   135,   137,   139,   144,   145,   149,   161,
     156,   158,   159,   163,   174,   164,   175,   176,   177,   117,
     178,   179,   167,   170,   180,   171,   181,   182,   172,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   201,   198,   202,   203,   204,   208,
     199,   207,   200,   210,   205,   212,   216,   206,   209,   211,
     213,   214,   215,   217,   218,    32,    35,    33,     0,    34,
      85,     0,     0,     0,    36,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   105
  };

  /* YYCHECK.  */
  const short int
  AMDILMDParser::yycheck_[] =
  {
         6,     4,    68,    70,    71,     4,    73,     3,     4,    76,
      77,    33,    79,    80,     3,     4,    83,     3,     4,     3,
       4,    27,     3,     4,    27,     3,     4,     4,     3,     4,
       4,     3,     4,    32,     4,     3,     4,    33,     3,     4,
       3,     4,     3,     4,    33,     3,     4,    33,     4,    33,
       4,   117,    33,     4,   121,    33,    31,    63,    64,   126,
     127,    33,    32,   130,   131,    33,     4,     4,    33,     4,
      33,     4,    33,    31,     4,    29,    32,     4,    29,    32,
      33,     4,    20,    33,    22,    23,     0,    24,    26,    24,
      28,    23,    30,     4,    27,     3,     4,    27,    25,     3,
       4,     3,     4,     3,     4,    26,   112,    20,   114,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    30,    21,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,     3,     4,    28,    32,     4,     4,     4,    34,    33,
      33,    33,    33,    33,     4,    33,    33,    33,    33,    33,
      33,    33,    33,    33,    32,     4,     3,     3,     3,     3,
      33,     4,     4,    34,     4,     3,    32,     3,    33,    33,
      33,    33,    33,    32,    32,    32,     3,     3,     3,    34,
       4,     4,     4,    33,     3,    33,     3,     3,     3,    91,
       4,     3,    33,    32,     4,    32,     4,     4,    33,    33,
      33,    33,    33,    33,    33,    33,     3,     3,     3,     3,
       3,     3,     3,    33,     3,    33,     3,     3,     3,     3,
      33,     4,    33,     3,    33,     3,     3,    33,    33,    33,
      33,     4,    33,    33,     3,     9,    15,    13,    -1,    14,
      61,    -1,    -1,    -1,    16,    -1,    17,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    78
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  AMDILMDParser::yystos_[] =
  {
         0,     4,    20,    22,    23,    26,    28,    30,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    53,    33,     4,    52,     4,     4,    29,    52,
      33,     0,    38,    46,    47,    48,    49,    53,     4,     4,
      25,    32,    32,    52,    29,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    54,    55,     4,     4,    27,    32,    33,    50,    51,
      33,    33,    33,    33,    33,    33,    33,    33,    33,    33,
      33,    33,    33,    33,    21,    55,    32,    52,    32,    52,
      34,     4,    32,    51,     3,     4,    56,    56,     4,    56,
       3,     3,    58,    56,    56,    58,    56,    56,     3,     3,
      56,    33,     4,    24,     4,    27,    56,    50,    34,     3,
       4,    33,    33,    33,    33,    33,    33,    33,    33,    33,
      33,    33,    33,     4,    52,    32,    52,    32,    33,    32,
      31,    56,     3,     3,     3,     3,    56,    56,     4,     3,
      56,    56,     3,     4,    57,    24,     4,    27,     4,     4,
      59,    34,    33,    33,    33,    33,    33,    33,     3,     4,
      32,    32,    33,    31,     3,     3,     3,     3,     4,     3,
       4,     4,     4,    33,    33,    33,    33,    33,    33,    33,
       3,     3,     3,     3,     3,     3,     3,    33,    33,    33,
      33,     3,     3,     3,     3,    33,    33,     4,     3,    33,
       3,    33,     3,    33,     4,    33,     3,    33,     3
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  AMDILMDParser::yytoken_number_[] =
  {
         0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,    59,    58,    35
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  AMDILMDParser::yyr1_[] =
  {
         0,    35,    36,    37,    37,    38,    38,    38,    38,    38,
      38,    38,    39,    40,    41,    41,    42,    42,    43,    43,
      44,    44,    45,    45,    46,    46,    47,    47,    47,    48,
      48,    49,    49,    50,    50,    51,    52,    52,    53,    54,
      54,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    56,
      56,    56,    56,    57,    57,    57,    57,    58,    58,    59,
      59
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  AMDILMDParser::yyr2_[] =
  {
         0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     1,     2,     1,     2,     1,     2,
       1,     2,     1,     2,     8,    10,     4,     8,    10,     3,
       2,    11,     7,     1,     2,     5,     1,     2,     7,     1,
       2,     3,     5,     5,     5,     5,     3,     3,     9,    13,
       9,    11,    11,    17,    19,    23,     3,     5,     1,     2,
       2,     1,     1,     2,     2,     1,     1,     1,     3,     1,
       3
  };

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const AMDILMDParser::yytname_[] =
  {
    "$end", "error", "$undefined", "INT_TOKEN", "STR_TOKEN", "MD_VALUE",
  "MD_SAMPLER", "MD_MEMORY", "MD_COUNTER", "MD_INT", "MD_INTLIST",
  "MD_POINTER", "MD_IMAGE", "MD_PRINTF", "MD_STRING", "MD_LGS",
  "MD_SEMAPHORE", "MD_REFLECTION", "MD_CONSTARG", "MD_ERRWAR", "ARG_START",
  "ARG_END", "IL_VERSION", "IL_FUNC", "IL_ENDFUNC", "IL_ENDMAIN",
  "MACRO_START", "MACRO_END", "DBG_START", "DBG_END", "DATA_START",
  "DATA_END", "';'", "':'", "'#'", "$accept", "ILProgram", "CompUnits",
  "CompUnit", "ILMain", "CompUnitEnd", "ILStates", "MacroStates",
  "DBGStates", "DataStates", "MDStates", "ILState", "MacroState",
  "DBGState", "DataState", "DataEntries", "DataEntry", "Statements",
  "MDState", "MDStmts", "MDStmt", "CompoundToken", "SpaceCompoundToken",
  "IntList", "StrList", 0
  };
#endif

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const AMDILMDParser::rhs_number_type
  AMDILMDParser::yyrhs_[] =
  {
        36,     0,    -1,    37,    -1,    38,    -1,    37,    38,    -1,
      40,    -1,    41,    -1,    42,    -1,    43,    -1,    45,    -1,
      39,    -1,    44,    -1,    22,    52,    25,    -1,     4,    -1,
      46,    -1,    41,    46,    -1,    47,    -1,    42,    47,    -1,
      48,    -1,    43,    48,    -1,    49,    -1,    44,    49,    -1,
      53,    -1,    45,    53,    -1,    23,     4,    32,     4,    52,
      24,    32,     4,    -1,    23,     4,    32,     4,    32,     4,
      52,    24,    32,     4,    -1,    26,     4,    52,    27,    -1,
      26,     4,    32,     4,    52,    27,    32,     4,    -1,    26,
       4,    32,     4,    32,     4,    52,    27,    32,     4,    -1,
      28,    52,    29,    -1,    28,    29,    -1,    30,    33,     4,
      33,     4,    50,    32,    34,    31,    33,     3,    -1,    30,
      33,     4,    50,    32,    34,    31,    -1,    51,    -1,    50,
      51,    -1,    32,    34,    56,    33,    59,    -1,     4,    -1,
      52,     4,    -1,    20,    33,     4,    54,    21,    33,     4,
      -1,    55,    -1,    54,    55,    -1,    14,    33,    56,    -1,
      19,    33,    56,    33,    57,    -1,    17,    33,     3,    33,
      56,    -1,    18,    33,     3,    33,    56,    -1,     7,    33,
       4,    33,     3,    -1,     7,    33,     4,    -1,     9,    33,
       3,    -1,     6,    33,    56,    33,     3,    33,     3,    33,
       3,    -1,    12,    33,    56,    33,    56,    33,     4,    33,
       3,    33,     3,    33,     3,    -1,    16,    33,    56,    33,
       3,    33,     3,    33,     3,    -1,     8,    33,    56,    33,
       3,    33,     3,    33,     3,    33,     3,    -1,     5,    33,
      56,    33,    56,    33,     3,    33,     3,    33,     3,    -1,
      11,    33,    56,    33,    56,    33,     3,    33,     3,    33,
       3,    33,     4,    33,     3,    33,     3,    -1,    11,    33,
      56,    33,    56,    33,     3,    33,     3,    33,     3,    33,
       4,    33,     3,    33,     3,    33,     4,    -1,    11,    33,
      56,    33,    56,    33,     3,    33,     3,    33,     3,    33,
       4,    33,     3,    33,     3,    33,     4,    33,     3,    33,
       3,    -1,    10,    33,    58,    -1,    13,    33,    58,    33,
       4,    -1,    15,    -1,    56,     3,    -1,    56,     4,    -1,
       4,    -1,     3,    -1,    57,     3,    -1,    57,     4,    -1,
       4,    -1,     3,    -1,     3,    -1,    58,    33,     3,    -1,
       4,    -1,    59,    33,     4,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned short int
  AMDILMDParser::yyprhs_[] =
  {
         0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    28,    30,    32,    35,    37,    40,    42,
      45,    47,    50,    52,    55,    64,    75,    80,    89,   100,
     104,   107,   119,   127,   129,   132,   138,   140,   143,   151,
     153,   156,   160,   166,   172,   178,   184,   188,   192,   202,
     216,   226,   238,   250,   268,   288,   312,   316,   322,   324,
     327,   330,   332,   334,   337,   340,   342,   344,   346,   350,
     352
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned short int
  AMDILMDParser::yyrline_[] =
  {
         0,    99,    99,   103,   104,   108,   114,   121,   128,   135,
     142,   148,   158,   170,   175,   181,   190,   196,   205,   213,
     224,   230,   240,   246,   255,   269,   287,   304,   323,   342,
     348,   356,   368,   380,   386,   395,   405,   411,   420,   433,
     439,   448,   457,   467,   476,   485,   495,   504,   512,   524,
     540,   552,   565,   579,   597,   617,   639,   647,   660,   671,
     679,   688,   694,   704,   712,   721,   727,   737,   743,   752,
     758
  };

  // Print the state stack on the debug stream.
  void
  AMDILMDParser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (state_stack_type::const_iterator i = yystate_stack_.begin ();
	 i != yystate_stack_.end (); ++i)
      *yycdebug_ << ' ' << *i;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  AMDILMDParser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
	       << " (line " << yylno << "):" << std::endl;
    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
		       yyrhs_[yyprhs_[yyrule] + yyi],
		       &(yysemantic_stack_[(yynrhs) - (yyi + 1)]),
		       &(yylocation_stack_[(yynrhs) - (yyi + 1)]));
  }
#endif // YYDEBUG

  /* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
  AMDILMDParser::token_number_type
  AMDILMDParser::yytranslate_ (int t)
  {
    static
    const token_number_type
    translate_table[] =
    {
           0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,    34,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    33,    32,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int AMDILMDParser::yyeof_ = 0;
  const int AMDILMDParser::yylast_ = 286;
  const int AMDILMDParser::yynnts_ = 25;
  const int AMDILMDParser::yyempty_ = -2;
  const int AMDILMDParser::yyfinal_ = 31;
  const int AMDILMDParser::yyterror_ = 1;
  const int AMDILMDParser::yyerrcode_ = 256;
  const int AMDILMDParser::yyntokens_ = 35;

  const unsigned int AMDILMDParser::yyuser_token_number_max_ = 286;
  const AMDILMDParser::token_number_type AMDILMDParser::yyundef_token_ = 2;


/* Line 1054 of lalr1.cc  */
#line 5 "AMDILMDParser.y"
} // llvm

/* Line 1054 of lalr1.cc  */
#line 1969 "AMDILMDParser.tab.cpp"


/* Line 1056 of lalr1.cc  */
#line 765 "AMDILMDParser.y"


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



