//===-- HSAILTestGenFilter.cpp - HSAIL Test Generator Filter -----------===//
//
//===----------------------------------------------------------------------===//
//
// HSAIL Test Generator Filter. (C) 2012 AMD Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDED_HSAIL_TESTGEN_FILTER_H
#define INCLUDED_HSAIL_TESTGEN_FILTER_H

#include "HSAILTestGenOptions.h"
#include "HSAILItems.h"

#include <algorithm>
#include <cctype>

using std::string;
using std::vector;

using HSAIL_ASM::opcode2str;

// ============================================================================
// ============================================================================

namespace TESTGEN {

//=============================================================================
// Test Filtering
//=============================================================================
//
// Currently, each element of a filter may have one of the following formats:
//
//  -------------------------------------------------------------------------------------
//   Format          Meaning                                            Encoded as
//  -------------------------------------------------------------------------------------
//   "prop=value"    this string must present in test description       "prop=value"
//   "prop!=value"   "prop=value" must not present in test description  " prop=value"
//   "value"         equivalent to "opcode=value"                       "opcode=value"
//  -------------------------------------------------------------------------------------
//

#define OPCODE_PREF ("opcode=")

static string normalize(string s) 
{ 
    s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
    
    if (s.find_first_of("=!") == string::npos) s = OPCODE_PREF + s;
    
    // Handling of negative conditions
    string::size_type pos = s.find("!=");
    if (pos != string::npos) {
        s = s.erase(pos, 1);    // erase '!'
        s = " " + s;
    }

    return s;
}

static bool eqChIgnoreCase(char c1, char c2) { return std::tolower(c1) == std::tolower(c2); }

static bool eqStrIgnoreCase(string s1, string s2) 
{ 
    return s1.length() == s2.length() && std::equal(s1.begin(), s1.end(), s2.begin(), eqChIgnoreCase); 
}

static bool isOpcodeProp(string s)
{
    const string::size_type len = strlen(OPCODE_PREF);
    return s.length() > len && eqStrIgnoreCase(s.substr(0, len), OPCODE_PREF);
}

//==========================================================================

class FilterComparator
{
private:
    unsigned index;
    const vector<string> &filter;

public:
    FilterComparator(const vector<string> &f) : index(0), filter(f) {}

    bool isEmpty()     const { return index == filter.size(); }
    bool isPositive()  const { return filter[index].length() == 0 || filter[index][0] != ' '; }
    string getFilter() const { return isPositive()? filter[index] : filter[index].substr(1); }
    void next()              { ++index; }

    bool operator()(const string& val) const { return eqStrIgnoreCase(val, getFilter()); }
};

//==========================================================================

class TestGenFilter
{
private:
    vector<string> filter;
    string opcode;

public:
    TestGenFilter()
    {
        std::transform(testFilter.begin(), testFilter.end(), back_inserter(filter), normalize);
        vector<string>::iterator result = find_if(filter.begin(), filter.end(), isOpcodeProp);
        if (result != filter.end()) opcode = *result;
    }

public:
    bool isTestEnabled(const vector<string> &testProps)
    {
        for (FilterComparator c(filter); !c.isEmpty(); c.next()) 
        {
            bool found = (find_if(testProps.begin(), testProps.end(), c) != testProps.end());
            if (found != c.isPositive()) return false; // ok == (found for positive) || (not found for negative)
        }
        return true;
    }

    bool isOpcodeEnabled(const unsigned opc) const
    {
        string prop = string(OPCODE_PREF) + opcode2str(opc);
        return opcode.empty() || eqStrIgnoreCase(prop, opcode); 
    }
};

//=============================================================================
//=============================================================================
//=============================================================================

} // namespace TESTGEN

#endif // INCLUDED_HSAIL_TESTGEN_FILTER_H

