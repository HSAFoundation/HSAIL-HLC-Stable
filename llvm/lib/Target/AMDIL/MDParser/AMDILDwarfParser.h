//===--------------- AMDILDwarfParser.h - AMDIL Dwarf Parser ---*- C++ -*--===//
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
//
// Interface of the AMDILDwarfParser class. The dwarf parser class parses the
// dwarf code emitted by the AMDIL backend.
//
//===---------------------------------------------------------------------===//
#ifndef _AMDIL_DWARF_PARSER_H_
#define _AMDIL_DWARF_PARSER_H_

#include "AMDILMDTypes.h"

#include <vector>


namespace llvm {
  typedef enum {
    DWARF_ASCII,
    DWARF_ASCIZ,
    DWARF_SECTION,
    DWARF_LABEL,
    DWARF_GLOBAL,
    DWARF_GLOBAL_LABEL,
    DWARF_ZERO,
    DWARF_BYTE,
    DWARF_SHORT,
    DWARF_LONG,
    DWARF_QUAD,
    DWARF_SLEB128,
    DWARF_ULEB128,
    DWARF_OFFSET,
    DWARF_CALCULATION,
    DWARF_TEXT,
    DWARF_DATA,
    DWARF_STRING,
    DWARF_NUMBER,
    DWARF_LOC,
    DWARF_FILE,
    DWARF_DEBUG_RANGE,
    DWARF_COMMENT,
    DWARF_ALIGN,
    DWARF_VAR_TYPE,
    DWARF_SIZE,
    DWARF_WEAK_REF,
    DWARF_WEAK_DEF,
    DWARF_UNKNOWN
  } DWARF_TYPE;

  class AMDILDwarfParser {
    private:
      std::vector<DBSection*>& mData;
      std::vector<DBSection*>::iterator secIter, secEnd;
      StmtBlock::iterator stmtIter, stmtEnd, mStart, mEnd;
      DWARF_TYPE mTType, mVType;
      std::string mTStr, mVStr;

    public:
      AMDILDwarfParser(std::vector<DBSection*>&, bool dbgMode = false);
      ~AMDILDwarfParser();

      bool next();
      void dump();
      std::string getSectionStr();
      void markStart() { mStart = stmtIter; mEnd = stmtEnd; }
      void markStop() { mEnd = stmtIter; }
      DBSection* getSection() { return *secIter; }
      DWARF_TYPE& getTokenType() { return mTType; }
      DWARF_TYPE& getValueType() { return mVType; }
      const std::string& getToken() { return mTStr; }
      const std::string& getValue() { return mVStr; }
      bool isASCII(DWARF_TYPE &A) { return A == DWARF_ASCII; }
      bool isASCIZ(DWARF_TYPE &A) { return A == DWARF_ASCIZ; }
      bool isSection(DWARF_TYPE &A) { return A == DWARF_SECTION; }
      bool isLabel(DWARF_TYPE &A) { return A == DWARF_LABEL; }
      bool isGlobal(DWARF_TYPE &A) { return A == DWARF_GLOBAL; }
      bool isGlobalLabel(DWARF_TYPE &A) { return A == DWARF_GLOBAL_LABEL; }
      bool isZero(DWARF_TYPE &A) { return A == DWARF_ZERO; }
      bool isByte(DWARF_TYPE &A) { return A == DWARF_BYTE; }
      bool isShort(DWARF_TYPE &A) { return A == DWARF_SHORT; }
      bool isLong(DWARF_TYPE &A) { return A == DWARF_LONG; }
      bool isQuad(DWARF_TYPE &A) { return A == DWARF_QUAD; }
      bool isCalc(DWARF_TYPE &A) { return A == DWARF_CALCULATION; }
      bool isSLEB128(DWARF_TYPE &A) { return A == DWARF_SLEB128; }
      bool isULEB128(DWARF_TYPE &A) { return A == DWARF_ULEB128; }
      bool isText(DWARF_TYPE &A) { return A == DWARF_TEXT; }
      bool isData(DWARF_TYPE &A) { return A == DWARF_DATA; }
      bool isString(DWARF_TYPE &A) { return A == DWARF_STRING; }
      bool isNumber(DWARF_TYPE &A) { return A == DWARF_NUMBER; }
      bool isLoc(DWARF_TYPE &A) { return A == DWARF_LOC; };
      bool isFile(DWARF_TYPE &A) { return A == DWARF_LOC; };
      bool isDebugRange(DWARF_TYPE &A) { return A == DWARF_LOC; }
      bool isComment(DWARF_TYPE &A) { return A == DWARF_COMMENT; }
      bool isAlign(DWARF_TYPE &A) { return A == DWARF_ALIGN; }
      bool isSize(DWARF_TYPE &A) { return A == DWARF_SIZE; }
      bool isVarType(DWARF_TYPE &A) { return A == DWARF_VAR_TYPE; }
      bool isWeakRef(DWARF_TYPE &A) { return A == DWARF_WEAK_REF; }
      bool isWeakDef(DWARF_TYPE &A) { return A == DWARF_WEAK_DEF; }
      bool isUnknown(DWARF_TYPE &A) { return A == DWARF_UNKNOWN; }

  }; // class AMDILDwarfParser
} // end namespace llvm

#endif // _AMDIL_DWARF_PARSER_H_
