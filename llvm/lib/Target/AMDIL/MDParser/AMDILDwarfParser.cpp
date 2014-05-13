//===--------------- AMDILDwarfParser.cpp - AMDIL Dwarf Parser ------------===//
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
// Implementation of the AMDILDwarfParser class. The dwarf parser class
// parses the unique aspect of the dwarf code emitted by the AMDIL backend.
// This dwarf is unique as it doesn't follow the standard ELF or MACHO
// formats and has its own format specifications. Also since no
// standard assembler exists, this parser is used as the frontend to the
// assembler.
//
//===---------------------------------------------------------------------===//

#define DEBUG_TYPE "amdildwarfparser"

#include "AMDILDwarfParser.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <cassert>

using namespace llvm;


AMDILDwarfParser::AMDILDwarfParser(std::vector<DBSection*>& data, bool dbgMode)
  : mData(data)
{
  secIter = mData.begin();
  secEnd = mData.end();
  if (secIter != secEnd) {
    stmtIter = (*secIter)->begin();
    stmtEnd = (*secIter)->end();
  }
}
AMDILDwarfParser::~AMDILDwarfParser()
{
}
// Split - Splits a string of tkn separated items in to a vector of strings.
///
static void Split(std::vector<std::string> &V, const std::string &S, const char* tkn) {
    // Start at beginning of string.
    size_t Pos = 0;
    while (true) {
        // Find the next token
        size_t Token = S.find_first_of(tkn, Pos);
        // If no token found then the the rest of the string is used
        if (Token == std::string::npos) {
            // Add string to vector
            V.push_back(S.substr(Pos));
            break;
        }
        // Otherwise add substring to vector
        V.push_back(S.substr(Pos, Token - Pos));
        // Advance to next item
        Pos = Token + 1;
    }
}

bool
AMDILDwarfParser::next()
{
  // If our section iterators are equal, then all sections have been parsed
  // and the function returns false to signal an end of parsing.
  if (secIter == secEnd) {
    return false;
  }
  if (stmtIter == stmtEnd) {
    ++secIter;
    if (secIter == secEnd) {
      return false;
    }

    DEBUG(dbgs() << "----------------- New Debug Section -----------------\n");

    stmtIter = (*secIter)->begin();
    stmtEnd = (*secIter)->end();
    if (stmtIter == stmtEnd) {
        // If the iterators point to an empty section
        // lets call next again so that we can
        // move on to the next section. We don't want
        // to return false here as we only want to
        // return false when secIter == secEnd and
        // not the sub-iterator condition stmtIter == stmtEnd.
        return next();
    }
  }
  std::string curStr = *(*stmtIter);
  mTType = DWARF_UNKNOWN;
  mVType = DWARF_UNKNOWN;
  mTStr = "";
  mVStr = "";

  DEBUG(dbgs() << "Str: " << curStr << "\t");
  std::vector<std::string> v;
  // First lets split up our string into a sequence of tokens.
  Split(v, curStr, " \t\n\r");
  for (unsigned x = 0, y = v.size(); x < y; ++x) {
    std::string& curTok = v[x];
    unsigned size = curTok.size();
    if (!size) {
      continue;
    }
    mTStr = curTok;
    if (!curTok.compare(0, 2, ";.")) {
      mTType = DWARF_LABEL;
      if (!curTok.compare(2, 10, "func_begin")
          && curTok.find(":") != std::string::npos) {
        while (!v[++x].size());
        mVStr = v[++x];
        mVType = DWARF_TEXT;
      }
      break;
    } else if (curTok == ".byte") {
      mTType = DWARF_BYTE;
      mVStr = v[++x];
      mVType = DWARF_NUMBER;
      break;
    } else if (curTok == ".short") {
      mTType = DWARF_SHORT;
      mVStr = v[++x];
      mVType = DWARF_NUMBER;
      break;
    } else if (curTok == ".long") {
      mTType = DWARF_LONG;
      mVStr = v[++x];
      if (mVStr.find("-") != std::string::npos
       || mVStr.find("+") != std::string::npos) {
        mVType = DWARF_CALCULATION;
      } else if (mVStr[0] == ';') {
        mVType = DWARF_LABEL;
      } else {
        mVType = DWARF_NUMBER;
      }
      break;
     } else if (curTok == ".quad") {
      mTType = DWARF_QUAD;
      mVStr = v[++x];
      if (mVStr.find("-") != std::string::npos
       || mVStr.find("+") != std::string::npos) {
        mVType = DWARF_CALCULATION;
      } else if (mVStr[0] == ';') {
        mVType = DWARF_LABEL;
      } else {
        mVType = DWARF_NUMBER;
      }
      break;
    } else if (curTok == ".ascii") {
      mTType = DWARF_ASCII;
      ++x;
      mVStr = v[++x];
      while (++x < y) {
          if (!v[x].size()) {
              continue;
          }
        if (v[x][0] == ';') {
          break;
        }
          mVStr = mVStr + " " + v[x];
      }
      // Kill the first quote and last quote.
      mVStr = mVStr.substr(1, mVStr.length() - 2);
      mVType = DWARF_STRING;
      break;
    } else if (curTok == ".asciz") {
      mTType = DWARF_ASCIZ;
      ++x;
      mVStr = v[++x];
      while (++x < y) {
         if (!v[x].size()) {
              continue;
          }
        if (v[x][0] == ';') {
          break;
        }
          mVStr = mVStr + " " + v[x];
      }
      // Kill the first quote and last quote.
      mVStr = mVStr.substr(1, mVStr.length() - 2);
      mVType = DWARF_STRING;
      break;
    } else if (curTok == ".sleb128") {
      mTType = DWARF_SLEB128;
      mVStr = v[++x];
      mVType = DWARF_NUMBER;
      break;
    } else if (curTok == ".uleb128") {
      mTType = DWARF_ULEB128;
      mVStr = v[++x];
      mVType = DWARF_NUMBER;
      break;
    } else if (!curTok.compare(0, 7, ".global")) {
      mTType = DWARF_GLOBAL;
      // The global token always comes attached to a second name.
      // As we strip out the '@' from that name, we need to increment
      // x to make sure we don't parse it twice.
      mTStr = curTok.substr(0, 7);
      mVStr = curTok.substr(8, size);
      mVType = DWARF_GLOBAL_LABEL;
      break;
    } else if (!curTok.compare(0,7,".offset")) {
      mTType = DWARF_OFFSET;
      mTStr = curTok.substr(0, 7);
      mVStr = curTok.substr(7, size);
      mVType = DWARF_LABEL;
      break;
    } else if (curTok == ".section") {
      mTType = DWARF_SECTION;
      // If we are a section, then we need to also parse out the
      // section name, so we increment the x token by one.
      mVStr = v[++x];
      if (mVStr[0] == '.') {
        mVType = DWARF_LABEL;
      } else {
        mVType = DWARF_STRING;
      }
      break;
    } else if (curTok == ".zero") {
      mTType = DWARF_ZERO;
      break;
    } else if (curTok == ".text") {
      mTType = DWARF_TEXT;
      break;
    } else if (curTok == ".data") {
      mTType = DWARF_DATA;
      break;
    } else if (curTok == ".loc") {
      mTType = DWARF_LOC;
      break;
    } else if (curTok == ".file") {
      mTType = DWARF_FILE;
      break;
    } else if (curTok == ".debug_range") {
      mTType = DWARF_DEBUG_RANGE;
      break;
    } else if (curTok == ".align") {
      mTType = DWARF_ALIGN;
      mVStr = v[++x];
      mVType = DWARF_NUMBER;
      break;
    } else if (curTok == ".type") {
      mTType = DWARF_VAR_TYPE;
      break;
    } else if (curTok == ".size") {
      mTType = DWARF_SIZE;
      break;
    } else if (curTok == ".weakref") {
      mTType = DWARF_WEAK_REF;
      break;
    } else if (curTok == ".weakdef"
        || curTok == ".weak_definition") {
      mTType = DWARF_WEAK_DEF;
      break;
    } else if (curTok == ";") {
      // We are a comment, we can finish as the rest of the line is not important.
      mTType = DWARF_COMMENT;
      break;
    } else {
      report_fatal_error(Twine("Unknown Token: ", curTok));
    }
  }
  DEBUG(dump());
  ++stmtIter;
  return true;
}
static const char* typeToStr(DWARF_TYPE A) {
  switch (A) {
    default:                  return "unknown";
    case DWARF_ASCII:         return "ascii";
    case DWARF_ASCIZ:         return "asciz";
    case DWARF_SECTION:       return "section";
    case DWARF_LABEL:         return "label";
    case DWARF_GLOBAL:        return "global";
    case DWARF_GLOBAL_LABEL:  return "global_label";
    case DWARF_BYTE:          return "byte";
    case DWARF_SHORT:         return "short";
    case DWARF_OFFSET:        return "offset";
    case DWARF_LONG:          return "long";
    case DWARF_SLEB128:       return "sleb128";
    case DWARF_ULEB128:       return "uleb128";
    case DWARF_CALCULATION:   return "calculation";
    case DWARF_TEXT:          return "text";
    case DWARF_DATA:          return "data";
    case DWARF_STRING:        return "string";
    case DWARF_NUMBER:        return "number";
    case DWARF_LOC:           return "loc";
    case DWARF_FILE:          return "file";
    case DWARF_DEBUG_RANGE:   return "debug_range";
    case DWARF_COMMENT:       return "comment";
    case DWARF_ALIGN:         return "align";
    case DWARF_VAR_TYPE:      return "variable_type";
    case DWARF_SIZE:          return "size";
    case DWARF_WEAK_REF:      return "weak_ref";
    case DWARF_WEAK_DEF:      return "weak_def";
  };
}
void
AMDILDwarfParser::dump()
{
  dbgs() << "{Token[" << typeToStr(mTType) << "]: " << mTStr;
  if (mVType != DWARF_UNKNOWN) {
    dbgs() << "\tValue[" << typeToStr(mVType) << "]: " << mVStr;
  }
  dbgs() << "}\n";

}
std::string
AMDILDwarfParser::getSectionStr()
{
  std::string str;
  raw_string_ostream ss(str);
  ss << "\t";
  for (StmtBlock::iterator i = mStart, e = mEnd; i != e; ++i) {
    if ((*i)->compare(0, 2, "; ")) {
      ss << "\n\t";
    }
    ss << (**i);
  }
  return ss.str();
}
