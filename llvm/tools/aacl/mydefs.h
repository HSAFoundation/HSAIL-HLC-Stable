/***
*mydefs.h - common macroses and defines (UTF-8 w/o BOM)
*
*Purpose:
*       This file contains macroses, defines and typedefs for Unicode/MB STL support
***/
#pragma once
#if !defined INC_MYDEFS_H__
#define INC_MYDEFS_H__

#if (defined WIN32) && (!defined _WIN32)
    #define _WIN32
#endif

#if (defined _WIN32) && (!defined WIN32)
    #define WIN32
#endif

#if (defined UNICODE) && (!defined _UNICODE)
    #define _UNICODE
#endif

#if (defined _UNICODE) && (!defined UNICODE)
    #define UNICODE
#endif

#if defined WIN32
    #include <windows.h>
    #include <tchar.h>
#else // !WIN32
    #define _T(x) x
#endif

#include <string>
#include <ios>
#include <io.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>

#if defined UNICODE
    #define tmain _tmain
    #define tsystem _wsystem
    #define tcout std::wcout
    #define tcerr std::wcerr
    #define tclog std::wclog
    #define tgetenv_s _wgetenv_s
    #define tputenv_s _wputenv_s
    #define t_access_s _waccess_s
    #define tfopen_s _wfopen_s
    #define tstrerror_s _wcserror_s
    #define t_splitpath_s _wsplitpath_s
    #define t_fullpath _wfullpath
    #define TLPSTR LPWSTR
    typedef wchar_t tchar;
    typedef std::wstring tstring;
    typedef std::allocator<wchar_t> tchar_allocator; 
    typedef std::char_traits<wchar_t> tchar_traits;
    typedef std::basic_ios<wchar_t, tchar_traits> tios;
    typedef std::basic_streambuf<wchar_t, tchar_traits> tstreambuf;
    typedef std::basic_istream<wchar_t, tchar_traits> tistream;
    typedef std::basic_ostream<wchar_t, tchar_traits> tostream;
    typedef std::basic_iostream<wchar_t, tchar_traits> tiostream;
    typedef std::basic_stringbuf<wchar_t, tchar_traits, tchar_allocator> tstringbuf;
    typedef std::basic_istringstream<wchar_t, tchar_traits, tchar_allocator> tistringstream;
    typedef std::basic_ostringstream<wchar_t, tchar_traits, tchar_allocator> tostringstream;
    typedef std::basic_stringstream<wchar_t, tchar_traits, tchar_allocator> tstringstream;
    typedef std::basic_filebuf<wchar_t, tchar_traits> tfilebuf;
    typedef std::basic_ifstream<wchar_t, tchar_traits> tifstream;
    typedef std::basic_ofstream<wchar_t, tchar_traits> tofstream;
    typedef std::basic_fstream<wchar_t, tchar_traits> tfstream;
    typedef std::basic_ostream<wchar_t, tchar_traits> tostream;
#else // ! UNICODE
    #define tmain main
    #define tsystem system
    #define tcout std::cout
    #define tcerr std::cerr
    #define tclog std::clog
    #define tgetenv_s getenv_s
    #define tputenv_s _putenv_s
    #define t_access_s _access_s
    #define tfopen_s fopen_s
    #define tstrerror_s strerror_s
    #define t_splitpath_s _splitpath_s
    #define t_fullpath _fullpath
    #define TLPSTR LPSTR
    typedef char tchar;
    typedef std::string tstring;
    typedef std::allocator<char> tchar_allocator; 
    typedef std::char_traits<char> tchar_traits;
    typedef std::basic_ios<char, tchar_traits> tios;
    typedef std::basic_streambuf<char, tchar_traits> tstreambuf;
    typedef std::basic_istream<char, tchar_traits> tistream;
    typedef std::basic_ostream<char, tchar_traits> tostream;
    typedef std::basic_iostream<char, tchar_traits> tiostream;
    typedef std::basic_stringbuf<char, tchar_traits, tchar_allocator> tstringbuf;
    typedef std::basic_istringstream<char, tchar_traits, tchar_allocator> tistringstream;
    typedef std::basic_ostringstream<char, tchar_traits, tchar_allocator> tostringstream;
    typedef std::basic_stringstream<char, tchar_traits, tchar_allocator> tstringstream;
    typedef std::basic_filebuf<char, tchar_traits> tfilebuf;
    typedef std::basic_ifstream<char, tchar_traits> tifstream;
    typedef std::basic_ofstream<char, tchar_traits> tofstream;
    typedef std::basic_fstream<char, tchar_traits> fstream;
    typedef std::basic_ostream<char, tchar_traits> tostream;
#endif // UNICODE

#endif // INC_MYDEFS_H__

