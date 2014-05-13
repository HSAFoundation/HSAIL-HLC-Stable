//===- aalink.cpp - AMD AMP C++ Compiler driver ---------------------------===//
//                                                                             //
//              (c) AMD 2013, all rights reserved.                             //
//                                                                             //
//===---------------------------------------------------------------------===//
#include <stdio.h>
#include <io.h>
#include <assert.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <cctype>
#include "aalink.h"

#if !defined AA_DELAY_FOR_DEBUG
    #define AA_DELAY_FOR_DEBUG 1
#endif

#define AA_EXIT_SUCCESS_AND_STOP -512*512

using namespace std;
using namespace aa;

int aalink_Main(int argc, tchar **argv)
{
    SetErrorMode(SEM_FAILCRITICALERRORS |
                 SEM_NOGPFAULTERRORBOX |
                 SEM_NOOPENFILEERRORBOX);
    
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
    _set_error_mode(_OUT_TO_STDERR);

    b_c_only = false;

    errno_t iRet = EXIT_SUCCESS;
    p_current_driver = aalink_tool;
    aalink_tool->s_abs_path = argv[0];
    for (int num = 1; num < argc; ++num) 
        aalink_tool->s_options += Space(argv[num]);
    // 
    if (argc == 2)
    {
        if ((argv[1] == Dash(V_opt)) || (argv[1] == Slash(V_opt)))
        {
            p_current_driver->VOption(argv[1]);        
            return AA_EXIT_SUCCESS_AND_STOP;
        }
    }
    // if only name
    size_t i_find = aalink_tool->s_abs_path.rfind(s_backslash);
    if (tstring::npos == i_find)
    {
        DWORD dwRet = GetExecutablePath(aalink_tool->s_abs_path);
        if (0 == dwRet)
            return CoutDWORDError(GetLastError());
    }
    i_find = aalink_tool->s_abs_path.rfind(s_backslash);
    if (tstring::npos == i_find)
        return CoutError(s_Wrong_path, aalink_tool->s_abs_path);
    path_to_exe = aalink_tool->s_abs_path.substr(0, i_find);

    iRet = GetAMD_bin(path_to_aa_bin);
    if (EXIT_SUCCESS != iRet)
        return CoutError(iRet, path_to_aa_bin);

    iRet = GetAMD_include(path_to_aa_include);
    if (EXIT_SUCCESS != iRet)
        return CoutError(iRet, path_to_aa_include);

    iRet = GetAMD_lib(path_to_aa_lib);
    if (EXIT_SUCCESS != iRet)
        return CoutError(iRet, path_to_aa_lib);

    // 0. aalink
    aalink_tool->SetArgs(argc, argv);
    // parsing arguments
    iRet = aalink_tool->Parse();
    if (EXIT_SUCCESS != iRet)
    {
        if ((AA_EXIT_SUCCESS_AND_STOP != iRet) && (AA_EXIT_ERROR_PRINTED != iRet))
            return CoutError(iRet);
        else
            return EXIT_SUCCESS;
    }
    // AMD AMP compiler doesn't support dlls with device code
    if (aalink_tool->b_DLL && aalink_tool->IsContainingDeviceCode())
    {
        CoutError(s_empty, _T("Linking of DLL with device code is not supported!"));
        return EXIT_FAILURE;
    }
    // 1. fatobj                      multiple
    iRet = aalink_tool->ExecuteFatobjs();
    if (EXIT_SUCCESS != iRet)
        return iRet;
    if (aalink_tool->IsContainingDeviceCode())
    {
        // 2. devlink
        devlink->b_obif = true;
        iRet = devlink->Execute(aalink_tool);
        if (EXIT_SUCCESS != iRet)
            return iRet;
        devlink->ExecutedOnce();
        // 3. inflate
        iRet = inflate->Execute(aalink_tool);
        if (EXIT_SUCCESS != iRet)
            return iRet;
        inflate->ExecutedOnce();
        // 4. cl
        if (aalink_tool->b_run_separate_link)
        {
            cl->AddOption(Slash(c_opt), true);
            b_c_only = true;
        }
        iRet = cl->Execute(aalink_tool);
        if (EXIT_SUCCESS != iRet)
            return iRet;
        cl->ExecutedOnce();
        // 5. link (optional if no -hidden-aacl-driven option)
        if (aalink_tool->b_run_separate_link)
        {
            iRet = link->Execute(aalink_tool);
            if (EXIT_SUCCESS != iRet)
                return iRet;
            link->ExecutedOnce();
        }
    }
    // no device code files (no *.brig or *.fatobj files)
    else
    {
        iRet = link->Execute(aalink_tool);
        if (EXIT_SUCCESS != iRet)
            return iRet;
        link->ExecutedOnce();
    }
    // deletion of all intermediate files
    iRet = aalink_tool->DeleteIntermediateFiles();
    if (EXIT_SUCCESS != iRet)
        return CoutError(iRet); 
    return EXIT_SUCCESS;
}

int tmain(int argc, tchar **argv)
{
// for attaching aalink process only, when called by msbuild with temporary rsp file
#if defined _DEBUG
    Sleep(AA_DELAY_FOR_DEBUG);
#endif
    SetEncoding(); 
    SaveConsoleState();
    if (IsMightBeColored())
        SetConsoleTextAttribute(g_hOut, FOREGROUND_INTENSITY);
    return RestoreBeforeReturn(aalink_Main(argc, argv));
}