// -*-C++-*-
// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// If you use the software (in whole or in part), you shall adhere to all
// applicable U.S., European, and other export laws, including but not limited
// to the U.S. Export Administration Regulations (“EAR”), (15 C.F.R. Sections
// 730 through 774), and E.U. Council Regulation (EC) No 1334/2000 of 22 June
// 2000.  Further, pursuant to Section 740.6 of the EAR, you hereby certify
// that, except pursuant to a license granted by the United States Department
// of Commerce Bureau of Industry and Security or as otherwise permitted
// pursuant to a License Exception under the U.S. Export Administration
// Regulations ("EAR"), you will not (1) export, re-export or release to a
// national of a country in Country Groups D:1, E:1 or E:2 any restricted
// technology, software, or source code you receive hereunder, or (2) export to
// Country Groups D:1, E:1 or E:2 the direct product of such technology or
// software, if such foreign produced direct product is subject to national
// security controls as identified on the Commerce Control List (currently
// found in Supplement 1 to Part 774 of EAR).  For the most current Country
// Group listings, or for additional information about the EAR or your
// obligations under those regulations, please refer to the U.S. Bureau of
// Industry and Security’s website at http://www.bis.doc.gov/.
//
// ==-----------------------------------------------------------------------===//
// FlexLexer.h -- define interfaces for lexical analyzer classes generated
// by flex

// Copyright (c) 1993 The Regents of the University of California.
// All rights reserved.
//
// This code is derived from software contributed to Berkeley by
// Kent Williams and Tom Epperly.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:

//  1. Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.

//  Neither the name of the University nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.

//  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE.

// This file defines FlexLexer, an abstract class which specifies the
// external interface provided to flex C++ lexer objects, and yyFlexLexer,
// which defines a particular lexer class.
//
// If you want to create multiple lexer classes, you use the -P flag
// to rename each yyFlexLexer to some other xxFlexLexer.  You then
// include <FlexLexer.h> in your other sources once per lexer class:
//
//	#undef yyFlexLexer
//	#define yyFlexLexer xxFlexLexer
//	#include <FlexLexer.h>
//
//	#undef yyFlexLexer
//	#define yyFlexLexer zzFlexLexer
//	#include <FlexLexer.h>
//	...

#ifndef __FLEX_LEXER_H
// Never included before - need to define base class.
#define __FLEX_LEXER_H

#include <iostream>
#  ifndef FLEX_STD
#    define FLEX_STD std::
#  endif

extern "C++" {

struct yy_buffer_state;
typedef int yy_state_type;

class FlexLexer {
public:
	virtual ~FlexLexer()	{ }

	const char* YYText() const	{ return yytext; }
	int YYLeng()	const	{ return yyleng; }

	virtual void
		yy_switch_to_buffer( struct yy_buffer_state* new_buffer ) = 0;
	virtual struct yy_buffer_state*
		yy_create_buffer( FLEX_STD istream* s, int size ) = 0;
	virtual void yy_delete_buffer( struct yy_buffer_state* b ) = 0;
	virtual void yyrestart( FLEX_STD istream* s ) = 0;

	virtual int yylex() = 0;

	// Call yylex with new input/output sources.
	int yylex( FLEX_STD istream* new_in, FLEX_STD ostream* new_out = 0 )
		{
		switch_streams( new_in, new_out );
		return yylex();
		}

	// Switch to new input/output streams.  A nil stream pointer
	// indicates "keep the current one".
	virtual void switch_streams( FLEX_STD istream* new_in = 0,
					FLEX_STD ostream* new_out = 0 ) = 0;

	int lineno() const		{ return yylineno; }

	int debug() const		{ return yy_flex_debug; }
	void set_debug( int flag )	{ yy_flex_debug = flag; }

protected:
	char* yytext;
	int yyleng;
	int yylineno;		// only maintained if you use %option yylineno
	int yy_flex_debug;	// only has effect with -d or "%option debug"
};

}
#endif // FLEXLEXER_H

#if defined(yyFlexLexer) || ! defined(yyFlexLexerOnce)
// Either this is the first time through (yyFlexLexerOnce not defined),
// or this is a repeated include to define a different flavor of
// yyFlexLexer, as discussed in the flex manual.
#define yyFlexLexerOnce

extern "C++" {

class yyFlexLexer : public FlexLexer {
public:
	// arg_yyin and arg_yyout default to the cin and cout, but we
	// only make that assignment when initializing in yylex().
	yyFlexLexer( FLEX_STD istream* arg_yyin = 0, FLEX_STD ostream* arg_yyout = 0 );

	virtual ~yyFlexLexer();

	void yy_switch_to_buffer( struct yy_buffer_state* new_buffer );
	struct yy_buffer_state* yy_create_buffer( FLEX_STD istream* s, int size );
	void yy_delete_buffer( struct yy_buffer_state* b );
	void yyrestart( FLEX_STD istream* s );

	void yypush_buffer_state( struct yy_buffer_state* new_buffer );
	void yypop_buffer_state();

	virtual int yylex();
	virtual void switch_streams( FLEX_STD istream* new_in, FLEX_STD ostream* new_out = 0 );
	virtual int yywrap();

protected:
	virtual int LexerInput( char* buf, int max_size );
	virtual void LexerOutput( const char* buf, int size );
	virtual void LexerError( const char* msg );

	void yyunput( int c, char* buf_ptr );
	int yyinput();

	void yy_load_buffer_state();
	void yy_init_buffer( struct yy_buffer_state* b, FLEX_STD istream* s );
	void yy_flush_buffer( struct yy_buffer_state* b );

	int yy_start_stack_ptr;
	int yy_start_stack_depth;
	int* yy_start_stack;

	void yy_push_state( int new_state );
	void yy_pop_state();
	int yy_top_state();

	yy_state_type yy_get_previous_state();
	yy_state_type yy_try_NUL_trans( yy_state_type current_state );
	int yy_get_next_buffer();

	FLEX_STD istream* yyin;	// input source for default LexerInput
	FLEX_STD ostream* yyout;	// output sink for default LexerOutput

	// yy_hold_char holds the character lost when yytext is formed.
	char yy_hold_char;

	// Number of characters read into yy_ch_buf.
	int yy_n_chars;

	// Points to current character in buffer.
	char* yy_c_buf_p;

	int yy_init;		// whether we need to initialize
	int yy_start;		// start state number

	// Flag which is used to allow yywrap()'s to do buffer switches
	// instead of setting up a fresh yyin.  A bit of a hack ...
	int yy_did_buffer_switch_on_eof;


	size_t yy_buffer_stack_top; /**< index of top of stack. */
	size_t yy_buffer_stack_max; /**< capacity of stack. */
	struct yy_buffer_state ** yy_buffer_stack; /**< Stack as an array. */
	void yyensure_buffer_stack(void);

	// The following are not always needed, but may be depending
	// on use of certain flex features (like REJECT or yymore()).

	yy_state_type yy_last_accepting_state;
	char* yy_last_accepting_cpos;

	yy_state_type* yy_state_buf;
	yy_state_type* yy_state_ptr;

	char* yy_full_match;
	int* yy_full_state;
	int yy_full_lp;

	int yy_lp;
	int yy_looking_for_trail_begin;

	int yy_more_flag;
	int yy_more_len;
	int yy_more_offset;
	int yy_prev_more_offset;
};

}

#endif // yyFlexLexer || ! yyFlexLexerOnce

