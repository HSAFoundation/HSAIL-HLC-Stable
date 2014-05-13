//===-----------------------------------------------------------*- C++ -*--===//
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
//==-----------------------------------------------------------------------===//
#ifndef _AMDIL_MD_SCANNER_H_
#define _AMDIL_MD_SCANNER_H_

// Only include FlexLexer.h if it hasn't been already included
#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

// Override the interface for yylex since we namespaced it
#undef YY_DECL
#define YY_DECL int llvm::AMDILMDScanner::yylex()
#include "AMDILMDInterface.h"
#include "AMDILMDParser.tab.hpp"
namespace llvm {
		class AMDILMDScanner : public yyFlexLexer {
			public:
				// constructor accepts the input and output streams
				// 0 means std equivilant (stdin, stdout)
				AMDILMDScanner(std::istream * in, std::ostream * out = 0) : yyFlexLexer(in, out), _merror(false), _mmsg("") { }

				// overloaded version of yylex - we need a pointer to yylval and yylloc
				inline int yylex(AMDILMDParser::semantic_type * lval,
				                 AMDILMDParser::location_type * lloc);

        inline void printMessage();
        bool hasError() { return _merror; }
        std::string getErrorStr() { return _mmsg; }

			private:
				// Scanning function created by Flex; make this private to force usage
				// of the overloaded method so we can get a pointer to Bison's yylval
				int yylex();

				// point to yylval (provided by Bison in overloaded yylex)
				AMDILMDParser::semantic_type * yylval;

				// pointer to yylloc (provided by Bison in overloaded yylex)
				AMDILMDParser::location_type * yylloc;

				// block default constructor
				AMDILMDScanner();
				// block default copy constructor
				AMDILMDScanner(AMDILMDScanner const &rhs);
				// block default assignment operator
				AMDILMDScanner &operator=(AMDILMDScanner const &rhs);


        bool _merror;
        std::string _mmsg;
		};

		// all our overloaded version does is save yylval and yylloc to member variables
		// and invoke the generated scanner
		int AMDILMDScanner::yylex(AMDILMDParser::semantic_type * lval,
		                   AMDILMDParser::location_type * lloc) {
			yylval = lval;
			yylloc = lloc;
			return yylex();
		}

    void
      AMDILMDScanner::printMessage() {
        std::cout<<"MessageHere!\n";
        abort();
      }

	}
#endif // _AMDIL_MD_SCANNER_H_
