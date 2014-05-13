//===- aacl.cpp - AMD AMP C++ Compiler driver -----------------------------===//
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
#include "aacl.h"
#include "../aalink/aalink.h"
#include "HSAILItems.h"
#include "HSAILBrigContainer.h"
#include "HSAILBrigObjectFile.h"

using namespace std;
using namespace aa;
using namespace aacl;

Aacl        *aacl_tool      = Aacl::Instance();
Ampfe_host  *ampfe_host     = Ampfe_host::Instance();
Ampfe       *ampfe          = Ampfe::Instance();
Opt         *opt            = Opt::Instance();
Llc         *llc            = Llc::Instance();
HSAILasm    *hsailasm       = HSAILasm::Instance();
Llvm_as     *llvm_as        = Llvm_as::Instance();
Llvm_link   *llvm_link      = Llvm_link::Instance();

// verifies if the brig file is empty
// it should have empty .code & .operands sections
// and only 2 derectives (version & function directives) or empty .directives section
bool IsBrigFileEmpty(tstring s_filename)
{
    bool b_empty = false;
    HSAIL_ASM::BrigContainer c;
    std::string s_brigfile(s_filename.begin(), s_filename.end());
    if (0 == HSAIL_ASM::AutoBinaryStreamer::load(c, s_brigfile.c_str()))
    {
        if (c.operands().isEmpty() && c.insts().isEmpty())
        {
            b_empty = true;
            if (c.directives().isEmpty())
                b_empty = true;
            else
            {
                int i_count = 0;
                for (HSAIL_ASM::Directive d = c.directives().begin(); d != c.directives().end(); d = d.next())
                {
                    if ((d.kind() != Brig::BrigEDirectiveVersion) && (d.kind() != Brig::BrigEDirectiveFunction))
                        return false;
                    i_count++;
                }
                if (i_count > 2)
                    return false;
            }
        }
    }
    return b_empty;
}
//
bool Aacl::HelpOption(const tstring &str, bool bPrint)
{
    size_t i_size = str.size();
    bool b_ok = false;
    bool b_hidden = false;
    if (0 == i_size)
        return false;
    if (1 == str.find(help_opt))
    {
        if (i_size != help_opt.size()+1)
        {
            // check the help-hidden option
            if (1 == str.find(help_hidden_opt))
            {
                if (i_size != help_hidden_opt.size()+1)
                    return false;
                b_hidden = true;
            }
            else
                return false;
        }
        b_ok = true;
    }
    else if (1 == str.find(s_question))
    {               
        if (i_size != s_question.size()+1)
            return false;
        b_ok = true;
    }
    if ((b_ok) && (bPrint))
    {
        // HELP OUTPUT
        tcout << s_OVERVIEW << s_aacl_description << endl << endl;
        tcout << s_USAGE << s_aacl << SpaceBefore(s_aacl_syntax) << endl << endl;
        tcout << s_OPTIONS << endl << endl;
        if (b_hidden)
        {
            // -color
            tcout << s_space << s_space << Dash(color_opt) 
                  << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space 
                  << SpaceBefore(Dash(Space(color_opt_descr))) << endl;
            // -debug1
            tcout << s_space << s_space << Dash(debug1_opt) 
                  << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space 
                  << SpaceBefore(Dash(Space(debug1_opt_descr))) << endl;
            // -debug2
            tcout << s_space << s_space << Dash(debug2_opt) 
                  << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space 
                  << SpaceBefore(Dash(Space(debug2_opt_descr))) << endl;
            // -no-default-lib:
            tcout << s_space << s_space << Dash(no_default_lib_opt) << s_space << s_space << s_space << s_space << SpaceBefore(Dash(Space(no_default_lib_opt_descr))) << endl;
        }
        // -help
        tcout << s_space << s_space << Dash(help_opt) 
              << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space
              << SpaceBefore(Dash(Space(help_opt_descr))) << endl;
        if (b_hidden)
        {
            // -keeptmp
            tcout << s_space << s_space << Dash(keeptmp_opt) 
                  << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space 
                  << SpaceBefore(Dash(Space(keeptmp_opt_descr))) << endl;
            // -temp-disable-brig-lowering
            tcout << s_space << s_space << Dash(temp_disable_brig_lowering_opt) << s_space << SpaceBefore(Dash(Space(temp_disable_brig_lowering_opt_descr))) << endl;
            // -tool=tool_name tool_opt
            tcout << s_space << s_space << Dash(tool_opt) 
                  << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space
                  << SpaceBefore(Dash(Space(tool_opt_descr))) << endl;
        }
        // -version
        tcout << s_space << s_space << Dash(version_opt) 
              << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space
              << SpaceBefore(Dash(Space(version_opt_descr))) << endl;
        return true;
    }
    if (b_ok)
        return true;
    return false;
}
// Ensure if the string is the name of the tool, called from aacl or not
bool Aacl::IsToolName(const tstring &str)
{
    if (str.empty())
        return false;
    tstring s_lowed = ToLowerCase(str);
    if (ToLowerCase(s_ampfe_host) == s_lowed)
        return true;
    else if (ToLowerCase(s_ampfe) == s_lowed)
        return true;
    else if (ToLowerCase(s_llvm_as) == s_lowed)
        return true;
    else if (ToLowerCase(s_llvm_link) == s_lowed)
        return true;
    else if (ToLowerCase(s_opt) == s_lowed)
        return true;
    else if (ToLowerCase(s_llc) == s_lowed)
        return true;
    else if (ToLowerCase(s_HSAILasm) == s_lowed)
        return true;
    else if (ToLowerCase(s_inflate) == s_lowed)
        return true;
    else if (ToLowerCase(s_fatobj) == s_lowed)
        return true;
    else if (ToLowerCase(s_aalink) == s_lowed)
        return true;
    else if (ToLowerCase(s_cl) == s_lowed)
        return true;
    else if (ToLowerCase(s_link) == s_lowed)
        return true;
    return false;
}
// verify if the argument is argument of MS /fp option = {fast | except[-] | precise | strict} or not
bool Is_fp_argument(const tstring &str)
{
    if (str.empty())
        return false;
    else if (precise_opt == str)
        return true;
    else if (fast_opt == str)
        return true;
    else if (except_opt == str)
        return true;
    else if (except_opt + s_dash == str)
        return true;
    else if (strict == str)
        return true;
    return false;
}
//
tstring Aacl::GetOutputPath()
{
    tstring s_path = cl->GetObjFileName(this);
    if (s_path.empty())
    {
        tstring s_curr_path;
        if (EXIT_SUCCESS == GetCurDir(s_curr_path))
            return s_curr_path;
    }   
    return GetPathWitoutFileName(s_path);
}
//
Tool* Aacl::GetToolByName(const tstring &name)
{
    if (ampfe_host->GetName() == name)
        return ampfe_host;
    if (ampfe->GetName() == name)
        return ampfe;
    if (opt->GetName() == name)
        return opt;
    if (llc->GetName() == name)
        return llc;
    if (cl->GetName() == name)
        return cl;
    if (link->GetName() == name)
        return link;
    return nullptr;
}
//
tstring Aacl::GetOutputFileName(bool b_wo_extension)
{
    tstring s_path = cl->GetObjFileName(this);
    if (s_path.empty())
    {
        tstring s_curr_path;
        if (EXIT_SUCCESS == GetCurDir(s_curr_path))
            return s_curr_path;
    }   
    if (b_wo_extension)
        return GetFileNameOnly(s_path);
    return GetFileNameWithExtension(s_path);
}
//
errno_t Aacl::Parse() 
{
    errno_t iRet = ParseCL(AACL_NOANYMESSAGE);
    iRet = ParseArgs(argc, argv); 
    if (EXIT_SUCCESS != iRet)
    {
        if ((AA_EXIT_SUCCESS_AND_STOP != iRet) && (AA_EXIT_ERROR_PRINTED != iRet))
            return CoutError(iRet);
        else
            return iRet;
    }
    SetVSIDEtoPath();
    // Common diagnostics: paths, environment
    Diagnostics();
    return CheckConstraints();
}
//
errno_t Aacl::ParseCL(unsigned int cout_type)
{
    errno_t iRet = GetEnv(s_CL, s_CL_options, cout_type);
    if (EXIT_SUCCESS != iRet)
        return CoutError(iRet, s_empty, cout_type);
    tstring sCmdLine = s_abs_path + s_space + s_CL_options;
    tchar** szArglist;
    int nArgs = 0;            
    szArglist = CommandLineToArgv_my((tchar*)sCmdLine.c_str(), &nArgs);
    if(szArglist)
    {
        iRet = ParseArgs(nArgs, szArglist, false);
        if (iRet != EXIT_SUCCESS)
            return CoutError(iRet);
    }
    return EXIT_SUCCESS;
}
// NOTICE: First arg is executable filepath
errno_t Aacl::ParseArgs(int argc, tchar **argv, bool b_main_command_line)
{
    // previous option name to compare on the next cycle step
    tstring prev_opt; 
    // if option has arguments it is needed to save option with arguments
    tstring prev_opt_full; 
    // tool name (if prev option was -tool=tool_name); become empty after parsing of the following: tool_option 
    tstring prev_tool_name;
    // the size of the current option
    size_t iOptSize = 0;
    bool bOptNeedsArg = false;
    bool b_ignore_for_cl = false;
    bool b_ignore_for_ampfe = false;
    bool b_ignore_for_ampfe_host = false;
    // if option is ignored, but must have the argument, the next option will be also ignored
    bool b_ignore_next_option = false;
    errno_t iRet = 0;
    bool b_the_rest_is_linker = false;
    bool b_unquoted = false;
    // NOTICE: all "" in command-line arguments are truncated by win shell
    if (1 == argc)
    {
        if (!b_main_command_line)
            return EXIT_SUCCESS;
        CoutError(s_not_enough_cmd_args, s_must_specify_at_least_1);
        return AA_EXIT_ERROR_PRINTED;
    }
    tstring s_uppered_currstr;
    tstring s_unquoted_currstr;
    for (int num = 1; num < argc; ++num)
    {
        tstring currstr(argv[num]);
        iOptSize = currstr.size();
        if (0 == iOptSize)
            continue;
        b_ignore_for_cl = false;  
        b_ignore_for_ampfe = false;
        b_ignore_for_ampfe_host = false;
        b_unquoted = false;
        s_uppered_currstr = currstr;
        UpperCase(s_uppered_currstr);
        s_unquoted_currstr = Unquote(currstr, false);
        if (s_unquoted_currstr != currstr)
        {
            currstr = s_unquoted_currstr;
            b_unquoted = true;
        }
        if (b_ignore_next_option)
        {
            s_ignored_options += SpaceBefore(currstr);
            b_ignore_next_option = false;
            continue;
        }
        // (0) Checking for Microsoft BOM (Byte order mark) as the first symbol
        // (0.1) Checking for -help option
        if (1 == num)
        {
            iOptSize = currstr.size();
            // /V
            if (VOption(currstr))
                return AA_EXIT_SUCCESS_AND_STOP;     
            // -help
            if (HelpOption(currstr))
                return AA_EXIT_SUCCESS_AND_STOP;
            // -version
            if (VersionOption(currstr))
                return AA_EXIT_SUCCESS_AND_STOP;     
        }
        // (1) MICROSOFT OPTION (/ or -) &  NOT MICROSOFT OPTION (- or --)
        if (((ch_slash == currstr[0]) || (ch_dash == currstr[0])) && (!bOptNeedsArg))
        {
            // /link
            if (1 == currstr.find(link_opt))
            {
                b_the_rest_is_linker = true;
                prev_opt = prev_opt_full = s_empty;
                bOptNeedsArg = false;
                continue;
            }
            // /WX
            // TODO_HSA: Add support of /WX:[n]
            else if (1 == currstr.find(WX_opt))
            {
                if (iOptSize == WX_opt.size()+1)
                    link->AddOption(currstr);
                else if (iOptSize == WX_opt.size()+2)
                    if (ch_dash == currstr[iOptSize-1])
                        link->AddOption(Slash(WX_opt) + s_colon + _T("NO"));
            }
            // /D name [ = | # [{string | number}] ]
            else if (1 == currstr.find(D_opt))
            { 
                tstring s_option = currstr.substr(1, iOptSize-1);
                switch (iOptSize)
                {
                    // /D
                    case 2:
                    {                        
                        bOptNeedsArg = true;
                        prev_opt = prev_opt_full = D_opt;
                        break;
                    }
                    // /D= || /D# - impossible, because of absence of space after = or #
                    default:
                        break;
                }
                tstring s_dashed_opt = s_option;
                tstring s_slashed_opt = s_option;
                size_t i_equal = s_dashed_opt.find(s_equal);
                if (tstring::npos == i_equal)
                    i_equal = s_dashed_opt.find(s_sharp);
                if (tstring::npos != i_equal)
                {
                    tstring s_left = s_dashed_opt.substr(0, i_equal+1);
                    tstring s_right = s_dashed_opt.substr(i_equal+1, s_dashed_opt.size()-i_equal-1);
                    s_right = QuoteIfSpaces(s_right);
                    s_dashed_opt = s_left + s_right;
                }
                // add to cl explicitly
                s_slashed_opt = Slash(s_dashed_opt);
                if (b_unquoted)
                    s_slashed_opt = Quote(s_slashed_opt);
                cl->AddOption(s_slashed_opt);
                b_ignore_for_cl = true;
                // searching for # to map it to =
                size_t i_sharp = s_dashed_opt.find(s_sharp);
                if (tstring::npos != i_sharp)
                   s_dashed_opt.replace(i_sharp, 1, s_equal); 
                s_dashed_opt = Dash(s_dashed_opt);
                if (b_unquoted)
                    s_dashed_opt = Quote(s_dashed_opt);
                // add to ampfe_host & ampfe explicitly
                ampfe_host->AddOption(s_dashed_opt); 
                if (!b_ignore_for_ampfe)
                    ampfe->AddOption(s_dashed_opt); 
            }
            // /I
            else if (1 == currstr.find(I_opt)) 
            {
                if (2 == iOptSize)
                {
                    bOptNeedsArg = true;
                    prev_opt = prev_opt_full = I_opt;
                }
                else
                {
                    if (ch_quote != currstr[2])
                    {
                        currstr.replace(0, I_opt.size()+1, s_empty); 
                        if (ch_quote != currstr[currstr.size()-1])
                            BackslashIfNeeded(currstr);
                        currstr = QuoteIfSpaces(currstr);
                        AddIncludes(s_slash + I_opt + currstr);
                        AddSysIncludes((DDash(sys_include_opt)) + SpaceBefore(currstr));
                        // for cl
                        currstr = SpaceBefore(s_slash + I_opt + currstr);
                    }
                }
            }
            // OPTOMIZATION OPTIONS
            // /O1 /O2 /Ob<n> /Od /Og /Oi[-] /Os /Ot /Ox /Oy[-]
            else if (1 == currstr.find(O_opt))
            {
                MapOptimizationOptions(currstr);
            }
            // /MD[d]
            else if (1 == currstr.find(MD_opt))
            {
                tstring s_defines;
                s_defines = SpaceBefore(Dash(D_opt) + Underscope(s_MT));
                s_defines += SpaceBefore(Dash(D_opt) + Underscope(s_DLL));
                s_defines += SpaceBefore(Dash(D_opt) + Underscope(s_DLL_CPPLIB));
                bool b_add_defines = true;
                switch (iOptSize)
                {
                    // MD
                    case 3:
                    {
                        break;
                    }
                    // MDd
                    case 4:
                    {
                        if (_T('d') == currstr[3])
                        {
                            s_defines += SpaceBefore(Dash(D_opt) + Underscope(s_DEBUG));
                        }
                        break;
                    }
                    // will be cl : Command line warning D9002 : ignoring unknown option '/MDxxxx'
                    default:
                    {
                        b_add_defines = false;
                        break;
                    }
                }
                if (b_add_defines)
                {
                    b_Use_RT_Library = true;
                    ampfe_host->s_defines = s_defines;
                    ampfe->s_defines = s_defines;
                }
            }
            // /MT[d]
            else if (1 == currstr.find(MT_opt))
            {
                tstring s_defines;
                s_defines = SpaceBefore(Dash(D_opt) + Underscope(s_MT));
                bool b_add_defines = true;
                switch (iOptSize)
                {
                    // MT
                    case 3:
                    {
                        break;
                    }
                    // MTd
                    case 4:
                    {
                        if (_T('d') == currstr[3])
                        {
                            s_defines += SpaceBefore(Dash(D_opt) + Underscope(s_DEBUG));
                        }
                        break;
                    }
                    // will be cl : Command line warning D9002 : ignoring unknown option '/MTxxxx'
                    default:
                    {
                        b_add_defines = false;
                        break;
                    }
                }
                if (b_add_defines)
                {
                    b_Use_RT_Library = true;
                    ampfe_host->s_defines = s_defines;
                    ampfe->s_defines = s_defines;
                }
            }
            // /LD[d]
            // TODO_HSA: http://msdn.microsoft.com/en-us/library/2kzt1wy3(v=vs.110)
            // 1) Interprets /Fe (Name EXE File) as naming a DLL rather than an .exe file; 
            // 2) Passes the /DLL option to the linker. 
            else if (1 == currstr.find(LD_opt))
            {
                tstring s_defines;
                s_defines = SpaceBefore(Dash(D_opt) + Underscope(s_MT));
                bool b_add_defines = true;
                switch (iOptSize)
                {
                    // LD
                    case 3:
                    {
                        aalink_tool->b_DLL = true;
                        aalink_tool->AddOption(Slash(s_DLL));
                        break;
                    }
                    // LDd
                    case 4:
                    {
                        if (_T('d') == currstr[3])
                        {
                            aalink_tool->b_DLL = true;
                            aalink_tool->AddOption(Slash(s_DLL));
                            s_defines += SpaceBefore(Dash(D_opt) + Underscope(s_DEBUG));
                            s_defines += SpaceBefore(Dash(D_opt) + Underscope(s_MT));
                        }
                        break;
                    }
                    // will be cl : Command line warning D9002 : ignoring unknown option '/LDxxxx'
                    default:
                    {
                        b_add_defines = false;
                        break;
                    }
                }
                if (b_add_defines)
                {
                    b_Use_RT_Library = true;
                    ampfe_host->s_defines = s_defines;
                    ampfe->s_defines = s_defines;
                }
            }
            // /fp:[precise | except[-] | fast | strict ]
            // mapped to opt and llc: enable-unsafe-fp-math
            else if (1 == currstr.find(fp_opt))
            {
                size_t fp_opt_size = fp_opt.size();
                // if -fp only
                bool b_err = false;
                if (iOptSize < fp_opt_size + 2) 
                    b_err = true;
                else if (iOptSize == fp_opt_size + 2)
                {
                    // if -fp: only
                    if (ch_colon == currstr[fp_opt_size+1])
                    {
                        s_ignored_options += SpaceBefore(currstr);
                        tstring err = Squote(currstr) + SpaceAfter(s_colon) + s_syntax_error + Space(s_in) + SpaceAfter(s_option) + Squote(Dash(fp_opt)) + s_dot;
                        err += SpaceBefore(s_Option_argument_was_not_specified);
                        err += SpaceBefore(s_Option_is_ignored);
                        CoutError(err, s_empty, AACL_WARNING);
                        continue;                    
                    }
                    // if -fp!':' (if not colon after fp)
                    else
                        b_err = true;
                }
                if (b_err)
                {
                    s_ignored_options += SpaceBefore(currstr);
                    tstring err = Squote(currstr) + SpaceAfter(s_colon) + s_syntax_error + Space(s_in) + SpaceAfter(s_option) + Squote(Dash(fp_opt)) + s_dot;
                    err += SpaceBefore(s_Option_argument_was_not_specified);
                    err += Space(Squote(s_colon)) + s_must_follow + Space(Squote(Dash(fp_opt))) + s_option + s_dot;
                    err += SpaceBefore(s_Option_is_ignored);
                    CoutError(err, s_empty, AACL_WARNING);
                    continue;
                }
                if (iOptSize > fp_opt_size + 2)
                {
                    tstring arg_name = currstr.substr(fp_opt_size+2, iOptSize-fp_opt_size-2);
                    if (Is_fp_argument(arg_name))
                    {
                        // OK: simple pass them to cl
                        // if fp:fast then map to opt & llc 'enable-unsafe-fp-math'
                        if (fast_opt == arg_name)
                        {
                            opt->AddSpecificOption(Dash(enable_unsafe_fp_math_opt));
                            llc->AddSpecificOption(Dash(enable_unsafe_fp_math_opt));
                        }
                    }
                    else 
                    {
                        s_ignored_options += SpaceBefore(currstr);
                        tstring err = Squote(currstr) + SpaceAfter(s_colon) + s_syntax_error + Space(s_in) + SpaceAfter(s_option) + Squote(Dash(fp_opt)) + s_dot;
                        err += SpaceBefore(s_Wrong_option_argument) + SpaceBefore(Squote(arg_name)) + s_dot;
                        err += SpaceBefore(s_Option_is_ignored);
                        CoutError(err, s_empty, AACL_WARNING);
                        continue;
                    }
                }
            }
            // /EH{s|a}[c][-] : /EHa /EHs /EHac /EHsc /EHa- /EHs- /EHac- /EHas-
            else if (1 == currstr.find(EH_opt)) 
            {
                bool bOk = false;
                if (4 <= iOptSize)
                {
                    // /EHa /EHs
                    if ((_T('a') == currstr[3]) || (_T('s') == currstr[3]))
                    {
                        if (4 == iOptSize)
                        {
                            bOk = true;
                        }                
                        else if (5 == iOptSize)
                        {
                            // /EHac /EHsc /EHa- /EHs-
                            if ((_T('c') == currstr[4]) || (ch_dash == currstr[4]))
                                bOk = true;
                        }
                        else if (6 == iOptSize) 
                        {
                            // /EHac- /EHas-
                            if ((_T('c') == currstr[4]) && (ch_dash == currstr[5]))
                                bOk = true;
                        }
                    }
                }
                if (bOk)
                {
                    slashEH += currstr; 
                    slashEH += s_space;
                }
            }
            // /Fo"FILEPATH"
            else if (1 == currstr.find(Fo_opt)) 
            {
                cl->slashFo = currstr; 
                b_ignore_for_cl = true;
            }
            // /Z7 /ZI /Zi
            else if (((1 == currstr.find(Z7_opt)) || (1 == currstr.find(ZI_opt)) || (1 == currstr.find(Zi_opt))) 
                  && (iOptSize == Zi_opt.size()+1))
            {
                MapDebugInformationFormatOptions(currstr);
            }
            // /Zc:arg1[-][,arg2[-]]
            else if (1 == currstr.find(slashZc_opt)) 
            {
                 if (iOptSize > 4)
                 {
                     if (ch_colon == currstr[3])
                     {
                         // Notice: also works with comma ',' separator
                         // forScope
                         size_t iFind = currstr.find(forScope_opt, 4);
                         if (iFind >= 4)
                         {
                             slashZc += slashZc_opt + ch_colon + forScope_opt;
                             // forScope-
                             iFind += forScope_opt.size();
                             if (iOptSize > iFind)
                             {
                                 if (ch_dash == currstr[iFind])
                                     slashZc += ch_dash;
                             }
                             slashZc += s_space;
                         }
                         // wchar_t
                         iFind = currstr.find(wchar_t_opt, 4);
                         if (iFind >= 4)
                         {
                             slashZc += slashZc_opt + ch_colon + wchar_t_opt + s_space;
                             // wchar_t-
                             iFind += wchar_t_opt.size();
                             if (iOptSize > iFind)
                             {
                                 if (ch_dash == currstr[iFind])
                                     slashZc += ch_dash;
                             }
                             slashZc += s_space;
                         }
                     }
                 }
            }
            // /c
            else if ((1 == currstr.find(c_opt)) && (iOptSize == c_opt.size()+1)) 
            {
                b_c_only = true;
            }
            ////////////////////////
            // aalink OPTIONS
            ////////////////////////            
            // /OUT:"FILEPATH"
            else if (1 == s_uppered_currstr.find(OUT_opt) && (iOptSize > OUT_opt.size()+1))
            {
                aalink_tool->b_OUT = true;
                link->AddOption(QuoteIfSpaces(currstr));
            }
            // /LIBPATH:"FILEPATH"
            else if (1 == s_uppered_currstr.find(LIBPATH_opt))
            {
                slashLIBPATH += LIBPATH_opt; 
                size_t slashLIBPATH_opt_size = LIBPATH_opt.size();
                if (iOptSize > slashLIBPATH_opt_size)
                {
                    if (ch_quote != currstr[slashLIBPATH_opt_size])
                    {
                        currstr.replace(0, slashLIBPATH_opt_size + 1, s_empty); 
                        currstr = QuoteIfSpaces(currstr); 
                        slashLIBPATH += currstr;
                        currstr = s_slash + LIBPATH_opt + currstr;
                    }
                }
            }
            // /DLL
            else if (1 == s_uppered_currstr.find(s_DLL) && (iOptSize == s_DLL.size()+1))
            {
                aalink_tool->b_DLL = true;
            }
            ////////////////////////
            // aacl OPTIONS ONLY
            ////////////////////////
            else if ((1 == currstr.find(temp_disable_brig_lowering_opt))  && (iOptSize == temp_disable_brig_lowering_opt.size()+1))
            {
                b_ignore_for_cl = true;
                b_brig_lowering = false;
                AddSpecificOption(Dash(temp_disable_brig_lowering_opt), false);
            }
            // color        aacl/aalink specific
            else if (1 == currstr.find(color_opt) && (iOptSize == color_opt.size()+1))
            {
                b_ignore_for_cl = true;
                b_color = true;
                AddSpecificOption(Dash(color_opt));
            }
            // debug1        aacl/aalink specific
            else if ((1 == currstr.find(debug1_opt))  && (iOptSize == debug1_opt.size()+1))
            {
                b_ignore_for_cl = true;
                debugSetLevel = 1;
                AddSpecificOption(Dash(debug1_opt));
            }
            // debug2        aacl/aalink specific
            else if ((1 == currstr.find(debug2_opt)) && (iOptSize == debug2_opt.size()+1))
            {
                b_ignore_for_cl = true;
                debugSetLevel = 2;
                AddSpecificOption(Dash(debug2_opt));
            }
            // no-default-lib:      aacl specific
            else if ((1 == currstr.find(no_default_lib_opt))  && (iOptSize > no_default_lib_opt.size()+1))
            {
                b_ignore_for_cl = true;
                tstring s_lib = currstr.substr(no_default_lib_opt.size()+1, iOptSize-no_default_lib_opt.size()-1);
                llvm_link->RemoveBcLib(s_lib);
                AddSpecificOption(currstr, false);
            }
            // keeptmp        aacl/aalink specific
            else if (1 == currstr.find(keeptmp_opt) && (iOptSize <= keeptmp_opt.size()+2))
            {
                b_ignore_for_cl = true;
                dashkeeptmp = Dash(keeptmp_opt);
                if (currstr[currstr.size()-1] == ch_dash)
                    dashkeeptmp += s_dash;
                AddSpecificOption(dashkeeptmp);
            }
            // tool        aacl/aalink specific
            else if (1 == currstr.find(tool_opt))
            {
                // 4
                size_t tool_opt_size = tool_opt.size();
                // 0123456
                // -tool=X
                if (iOptSize < tool_opt_size+3)
                {
                    // TO_DECIDE: if wrong option throw error or message
                    // Message & continue
                    // warning: '-tool=': syntax error in option '-tool'. Tool name was not specified. Option is ignored.
                    s_ignored_options += SpaceBefore(currstr);
                    tstring err = Squote(currstr) + SpaceAfter(s_colon) + s_syntax_error + Space(s_in) + SpaceAfter(s_option) + Squote(Dash(tool_opt)) + s_dot;
                    err += SpaceBefore(s_Tool_name_was_not_specified);
                    err += SpaceBefore(s_Option_is_ignored);
                    CoutError(err, s_empty, AACL_WARNING);
                    b_ignore_next_option = true;
                    continue;
                }
                if (ch_equal != currstr[tool_opt_size+1])
                {
                    // TO_DECIDE: if wrong option throw error or message
                    // Message & continue
                    // warning: '-tool-llc': syntax error in option '-tool'. '=' must follow '-tool' option. Option is ignored.
                    s_ignored_options += SpaceBefore(currstr);
                    tstring err = Squote(currstr) + SpaceAfter(s_colon) + s_syntax_error + Space(s_in) + SpaceAfter(s_option) + Squote(Dash(tool_opt)) + s_dot;
                    err += Space(Squote(s_equal)) + s_must_follow + Space(Squote(Dash(tool_opt))) + s_option + s_dot;
                    err += SpaceBefore(s_Option_is_ignored);
                    CoutError(err, s_empty, AACL_WARNING);
                    b_ignore_next_option = true;
                    continue;
                }
                // TODO_HSA: verify: Do other tools support spaces before and|or after =
                tstring tool_name = currstr.substr(tool_opt_size+2, iOptSize-tool_opt_size-2);
                if (!IsToolName(tool_name))
                {
                    // wrong tool name error generation
                    s_ignored_options += SpaceBefore(currstr);
                    tstring err = Squote(currstr) + SpaceAfter(s_colon) + s_syntax_error + Space(s_in) + SpaceAfter(s_option) + Squote(Dash(tool_opt)) + s_dot;
                    err += SpaceBefore(s_Wrong_tool_name) + SpaceBefore(Squote(tool_name)) + s_dot;
                    err += SpaceBefore(s_Option_is_ignored);
                    CoutError(err, s_empty, AACL_WARNING);
                    b_ignore_next_option = true;
                    continue;
                }
                prev_tool_name = tool_name;
                b_ignore_for_cl = true;
                bOptNeedsArg = true;
                prev_opt = tool_opt;
                prev_opt_full = currstr.substr(1, iOptSize-1);
            }
            ////////////////////////
            // HLC OPTIONS ONLY --
            ////////////////////////
            else if (0 == currstr.find(s_dbl_dash))
            {
                // all dbl dash is not supported by MS cl
                b_ignore_for_cl = true;
                // --diag_remark XXX
                if (2 == currstr.find(diag_remark_opt))
                {
                    if (currstr.size() == diag_remark_opt.size() + 2)
                    {
                        b_ignore_for_ampfe = true;
                        bOptNeedsArg = true;
                        prev_opt = diag_remark_opt;
                        // tstring s_dbl_dashed_opt = SpaceBefore(DDash(diag_remark_opt));
                        tstring s_dbl_dashed_opt = DDash(diag_remark_opt);
                        ampfe_host->AddSpecificOption(s_dbl_dashed_opt);
                    }
                    // TODO_HSA: wrong option error !!
                    else
                    {
                        // error;
                    }
                }
            }
            ///////////////////////////////////////
            // adding current option to cl (link)
            ///////////////////////////////////////
            if (!b_ignore_for_cl)
            {
                if (b_unquoted)
                    currstr = Quote(currstr);
                if (!b_the_rest_is_linker)
                {
                    cl->AddOption(currstr);
                }
                else
                    link->AddOption(currstr);
            }
        }
        // (2) *.rsp INPUT COMMAND-LINE FILE
        else if (_T('@') == currstr[0])
        {
            // Check for access the file
            currstr.replace(0, 1, s_empty);
            iRet = CheckPath(currstr);
            if (iRet != EXIT_SUCCESS)
                return CoutError(iRet, currstr);
            // read the file
            tstring sCmdLine;
            iRet = ReadFileToString(currstr, sCmdLine);
            if (iRet != EXIT_SUCCESS)
                return CoutError(iRet, currstr);
            // read the command-line options
            if (sCmdLine.size() > 0)
            {
                aacl_tool->s_rsp_cmdline = sCmdLine;
                sCmdLine = aacl_tool->s_abs_path + s_space + sCmdLine;
                tchar** szArglist;
                int nArgs = 0;            
                szArglist = CommandLineToArgv_my((tchar*)sCmdLine.c_str(), &nArgs);
                if(szArglist)
                {
                    iRet = ParseArgs(nArgs, szArglist);
                    if (iRet != EXIT_SUCCESS)
                        return CoutError(iRet);
                }
            }
        }
        // (3) PREVIOUS OPTION ARGUMENT || FILE
        else 
        {
            // firstly verify if it is the argument 
            if (bOptNeedsArg)        
            {
                // D || D= || D#
                if (D_opt == prev_opt)
                {                    
                    tstring s_maped_opt = currstr;
                    size_t i_sharp = s_maped_opt.find(s_sharp);
                    if (tstring::npos != i_sharp)
                       s_maped_opt.replace(i_sharp, 1, s_equal); 
                    ampfe_host->s_options += QuoteIfSpaces(s_maped_opt);
                    ampfe->s_options += QuoteIfSpaces(s_maped_opt);
                    b_ignore_for_ampfe = true;
                    b_ignore_for_ampfe_host = true;
                }
                // /I
                else if (I_opt == prev_opt)
                {
                    // all include options for ampfe are generated by aacl
                    b_ignore_for_ampfe = true;
                    b_ignore_for_ampfe_host = true;
                    if (ch_quote != currstr[currstr.size()-1])
                        BackslashIfNeeded(currstr);
                    currstr = QuoteIfSpaces(currstr);
                    AddSysIncludes(DDash(sys_include_opt) + SpaceBefore(currstr));
                }
                ////////////////////////
                // HLC OPTIONS ONLY
                ////////////////////////
                // -tool=tool_name tool_opt
                else if (tool_opt == prev_opt)
                {
                    b_ignore_for_cl = true;
                    b_ignore_for_ampfe = true;
                    b_ignore_for_ampfe_host = true;
                    bool b_add_to_aalink = false;
                    if (aalink_tool->IsToolName(prev_tool_name))
                        b_add_to_aalink = true;
                    AddSpecificOption(Dash(prev_opt_full) + SpaceBefore(currstr), b_add_to_aalink);
                    AddSpecificOptionForTool(prev_tool_name, currstr);
                    prev_tool_name = s_empty;
                    prev_opt_full = s_empty;
                }
                // --diag_remark
                else if (diag_remark_opt == prev_opt)
                {
                    b_ignore_for_cl = true;
                    b_ignore_for_ampfe = true;
                    AddSpecificOptionForTool(ampfe_host->GetName(), currstr);
                }
                currstr = SpaceBefore(currstr);
                if (!b_ignore_for_cl)
                    cl->AddOption(currstr);
                if (!b_ignore_for_ampfe_host)
                    ampfe_host->s_options += currstr;
                if (!b_ignore_for_ampfe)
                    ampfe->s_options += currstr;
            }
            // if previous option is not really an option - it's a file
            else 
            {
                if (!b_the_rest_is_linker)
                {
                    // NOTICE: HERE ARE The Compulsory Options of MSVC++ cl
                    // In the absence of any of the /LD[d], /MD[d], /MT[d] options
                    // Added compulsory /MT just before filenames
                    if (!b_Use_RT_Library)
                    {
                        s_compulsory_options += SpaceBefore(Slash(MT_opt));
                        tstring s_defines;
                        s_defines = SpaceBefore(Dash(D_opt) + Underscope(s_MT));
                        ampfe_host->s_compulsory_options += s_defines;
                        ampfe->s_compulsory_options += s_defines;
                        b_Use_RT_Library = true;
                    }
                    tstring unquoted_currstr = Unquote(currstr);
                    tstring s_ext = GetExtension(currstr);
                    // NOTICE: file might be filepath => it must be enclosed in quotes!
                    currstr = QuoteIfSpaces(currstr);
                    // *.bc
                    if (unquoted_currstr.size()-s_bc_ext.size() == unquoted_currstr.rfind(s_bc_ext))
                    {
                        llvm_link->AddBcLib(currstr);
                        s_input_files += SpaceBefore(QuoteIfSpaces(currstr));
                    }
                    else if (IsCompilerInputFileExtension(s_ext))
                    {
                        input_files.push_back(currstr);
                        s_input_files += SpaceBefore(currstr);
                    }
                    else
                    {
                        link->s_options += SpaceBefore(currstr);
                        link->s_input_files += SpaceBefore(currstr);
                    }
                }
                else
                    link->s_options += Space(currstr);
            }
            prev_opt = s_empty;
            bOptNeedsArg = false;
        }
    }  
    return EXIT_SUCCESS;
}
// OPTOMIZATION OPTIONS
// mapped:   /O1 /O2 /Od /Os /Ot 
// unmapped: /Ob<n> /Og /Oi[-] /Ox /Oy[-]
void Aacl::MapOptimizationOptions(tstring sOption)
{
    if (sOption.empty())
        return;
    if ((ch_dash != sOption[0]) && (ch_slash != sOption[0]))
        return;
    if (1 != sOption.find(O_opt))
        return;
    tstring sO = sOption.substr(2, sOption.size()-2);
    // /O  == /Ot
    if (sO.empty())
    {
        opt->s_optimizations = Dash(O3_opt);
        llc->s_optimizations = Dash(O2_opt);
    }
    size_t i_size = sO.size();
    for (size_t i=0; i<i_size; ++i)
    {
        // /Od
        if (_T('d') == sO[i])
        {
            opt->s_optimizations = Dash(O0_opt);
            llc->s_optimizations = Dash(O0_opt);
        }
        // /O1 /O2 /Os /Ot
        else if ((_T('1') == sO[i]) || (_T('2') == sO[i]) || (_T('s') == sO[i]) || (_T('t') == sO[i]))
        {
            opt->s_optimizations = Dash(O3_opt);
            llc->s_optimizations = Dash(O2_opt);
        }
        // /Ox
        else if (_T('x') == sO[i])
        {
            opt->s_optimizations = Dash(O3_opt);
            llc->s_optimizations = Dash(O3_opt);
        }
    }
}
//
void Aacl::MapDebugInformationFormatOptions(tstring sOption)
{
    if (sOption.empty())
        return;
    if ((ch_dash != sOption[0]) && (ch_slash != sOption[0]))
        return;
    s_debug_options = sOption;
    ampfe->s_debug_options = Dash(g_opt);
    opt->b_debug = true;
    opt->s_optimizations = Dash(O0_opt);
    llc->b_debug = true;
    llc->s_optimizations = Dash(O0_opt);
}
//
errno_t Aacl::CheckConstraints()
{
    if ((!b_brig_lowering) && (!s_debug_options.empty()))
    {
        CoutError(s_Command_line_error, Quote(s_debug_options, ch_squote) + _T(" and ") 
                + Quote(temp_disable_brig_lowering_opt, ch_squote) + s_space + s_command_line_options_are_incompatible); 
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
// also adds that option to s_options IN PLACE
void Aacl::AddSpecificOption(const tstring &option, bool b_add_also_for_aalink)
{
    Tool::AddSpecificOption(option);
    if (b_add_also_for_aalink)
        s_aalink_specific_options += SpaceBefore(Trim(option));
}
//
errno_t Aacl::Get_VCPP_includes(tstring &s_VCPP_inc, bool b_in_sys_include_syntax)
{
    s_VCPP_inc.clear();
    tstring s_include_paths;
    errno_t iRet = GetEnv(s_INCLUDE, s_VCPP_inc, AACL_WARNING);
    if (!b_in_sys_include_syntax)
        return iRet;
    if (EXIT_SUCCESS == iRet)
    {
        tstring s_sys_includes;
        size_t iFind = 0;
        size_t i_size = s_VCPP_inc.size();
        tstring s_inc;
        while (tstring::npos != iFind)
        {
            bool bAdd = false;
            iFind = s_VCPP_inc.find(s_semicolon);
            if (tstring::npos != iFind)
            {
                s_inc = BackslashIfNeeded(s_VCPP_inc.substr(0, iFind)); // + s_backslash;
                s_VCPP_inc = s_VCPP_inc.substr(iFind+1, i_size-iFind-1);
                bAdd = true;
            }
            else
            {
                s_inc = s_VCPP_inc;
                bAdd = true;
            }
            s_inc = Trim(s_inc);
            if ((bAdd) && (s_inc.size() > 0))
            {
                s_sys_includes += DDash(sys_include_opt) + Space(QuoteIfSpaces(BackslashIfNeeded(s_inc)));
            }
        }
        s_sys_includes = Trim(s_sys_includes);
        s_VCPP_inc = s_sys_includes;
    }
    return iRet;
}
//
errno_t Aacl::Diagnostics()
{
    if (2 != debugSetLevel) 
        return EXIT_SUCCESS;
    errno_t iRet = Driver::Diagnostics();
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (IsMightBeColored())
        SetConsoleTextAttribute(hOut, FOREGROUND_INTENSITY);
    // not for driver command line launching (only in case of exec from under MS VS IDE)
    if (!s_rsp_cmdline.empty())
    {
        s_options = Trim(s_options);
        tcout << GetName() << aa::s_command_line << s_options << endl;    
        s_rsp_cmdline = Trim(s_rsp_cmdline);
        tcout << GetName() << aa::s_rsp_file << s_rsp_cmdline << endl;
    }
    s_CL_options = Trim(s_CL_options);
    tcout << GetName() << aa::s_CL_options << s_CL_options << endl;    
    if (IsMightBeColored())
        SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    return iRet;
}
//////////////// ampfe_host Execute /////////////////////////////////////////
// Example: "ampfe_host -D_DEBUG --diag_remark 954 matrix_multiply_amp.cpp"
/////////////////////////////////////////////////////////////////////////////
errno_t Ampfe_host::Execute(Tool *parent_tool)
{
    Aacl *p_drv = dynamic_cast<Aacl*>(parent_tool);
    if (!p_drv)
        return EXIT_FAILURE;

    if (IsExecutedOnce())
    {
        s_hardcoded_options.clear();
        s_includes.clear();
        s_input_files.clear();
        s_output_files.clear();
        s_current_run_options.clear();
        s_all_options.clear();
    }

    // finding tool executable
    errno_t iRet = FindExecutable();
    if (EXIT_SUCCESS != iRet)
        return iRet; 
    // output calculation
    tstring s_out_dir = GetOutputPath(p_drv);

    tstring s_out_file_name = GetFileNameOnly(s_input_file) + s_host_ext + GetExtension(s_input_file);
    // if not a current dir then -o option
    if (!s_out_dir.empty())
    {
        tstring s_o_file = s_out_dir + s_out_file_name;
        AddOption(Dash(o_opt) + SpaceBefore(s_o_file), true);
    }
    AddHardcodedOption(DDash(diag_remark_opt));
    AddHardcodedOption(_T("954"));

    tstring s_cl_std_includes;
    iRet = p_drv->Get_VCPP_includes(s_cl_std_includes);
    if (!s_cl_std_includes.empty())
        AddIncludes(s_cl_std_includes);

    tstring additional_include = Space(DDash(sys_include_opt)) + QuoteIfSpaces(path_to_aa_include);
    AddIncludes(additional_include);

    // source cpp file which aacl gets and redirects to ampfe_host
    errno_t i_ret_file_check = 0;
    s_amp_host_output_cpp_filename = s_out_dir + s_out_file_name;
    AddOutputFile(QuoteIfSpaces(s_amp_host_output_cpp_filename));
    AddInputFile(s_input_file);
   
    BundleAllOptions();

    iRet = Output();
    if (EXIT_SUCCESS != iRet)
        return iRet;
    iRet = ExecProcess();

    // Checking file existence
    i_ret_file_check = p_drv->CheckAndAddIntermediateFile(s_amp_host_output_cpp_filename);

    if ((EXIT_SUCCESS != iRet) || (EXIT_SUCCESS != i_ret_file_check))
    {
        CoutReturnCode(GetName(), iRet);
        errno_t i_ret = p_drv->DeleteIntermediateFiles();
        if (EXIT_SUCCESS != i_ret)
            return CoutError(i_ret);
        return iRet;
    }
    if (debugSetLevel) 
        CoutReturnCode(GetName(), iRet);
    return iRet;
}
//////////////// ampfe Execute //////////////////////////////////////////////
// Example: "ampfe -o matrix_multiply_amp -D_DEBUG --sys_include \"../../../../../fsa/api/fsa/include\" matrix_multiply_amp_amp_host.cpp"
/////////////////////////////////////////////////////////////////////////////
errno_t Ampfe::Execute(Tool *parent_tool)
{ 
    Aacl *p_drv = dynamic_cast<Aacl*>(parent_tool);
    if (!p_drv)
        return EXIT_FAILURE;
    
    if (IsExecutedOnce())
    {
        s_hardcoded_options.clear();
        s_includes.clear();
        s_input_files.clear();
        s_output_files.clear();
        s_current_run_options.clear();
        s_all_options.clear();
    }
    // finding tool executable
    errno_t iRet = FindExecutable();
    if (EXIT_SUCCESS != iRet)
        return iRet; 
    // output calculation
    tstring s_out_dir = GetOutputPath(p_drv);
    tstring s_o_file = s_out_dir + GetFileNameOnly(aa::s_input_file);
    tstring s_o_ext = GetExtension(aa::s_input_file);
 
    AddHardcodedOption(DDash(diag_warning_opt));
    AddHardcodedOption(_T("268"));
    AddHardcodedOption(Dash(o_opt) + SpaceBefore(QuoteIfSpaces(s_o_file)));

    tstring s_cl_std_includes;
    iRet = p_drv->Get_VCPP_includes(s_cl_std_includes);
    s_includes = s_cl_std_includes + SpaceBefore(Trim(s_includes));

    tstring additional_include = DDash(sys_include_opt) + SpaceBefore(QuoteIfSpaces(path_to_aa_include));
    PrependIncludes(additional_include);

    AddInputFile(QuoteIfSpaces(aa::s_input_file));

    // adding the intermediate .ll name to the temp files list (to be deleted)
    s_amp_ll_filename = s_o_file + s_ll_ext;    
    AddOutputFile(QuoteIfSpaces(s_amp_ll_filename));
    s_amp_ll_filename = GetFullPath(s_amp_ll_filename);

    BundleAllOptions();

    // output tool diagnostics (if debug2) and real command line
    iRet = Output();
    if (EXIT_SUCCESS != iRet)
        return iRet;
    iRet = ExecProcess();

    errno_t i_ret_file_check_ll = p_drv->CheckAndAddIntermediateFile(s_amp_ll_filename);

    if ((EXIT_SUCCESS != iRet) || (EXIT_SUCCESS != i_ret_file_check_ll))
    {
        CoutReturnCode(GetName(), iRet);
        errno_t i_ret = p_drv->DeleteIntermediateFiles();
        if (EXIT_SUCCESS != i_ret)
            return CoutError(i_ret);
        return iRet;
    }
    if (debugSetLevel) 
        CoutReturnCode(GetName(), iRet);
    return iRet;
}
//////////////// llvm-as Execute ////////////////////////////////////////////
// Example: "llvm-as matrix_multiply_amp.ll matrix_multiply_amp.bc"
/////////////////////////////////////////////////////////////////////////////
errno_t Llvm_as::Execute(Tool *parent_tool)
{
    Aacl *p_drv = dynamic_cast<Aacl*>(parent_tool);
    if (!p_drv)
        return EXIT_FAILURE;
    if (IsExecutedOnce())
        ClearAllOptions();
    // finding tool executable
    errno_t iRet = FindExecutable();
    if (EXIT_SUCCESS != iRet)
        return iRet; 
    // output calculation
    tstring s_out_dir = GetOutputPath(p_drv);
    tstring s_out_file_name_only = GetFileNameOnly(aa::s_input_file);
    tstring s_out_file_ext_only = GetExtension(aa::s_input_file);
    s_out_file_name_only = s_out_dir + s_out_file_name_only;
    s_output_file = s_out_file_name_only + s_bc_ext;
    AddOutputFile(QuoteIfSpaces(s_output_file));
    AddInputFile(QuoteIfSpaces(s_out_file_name_only + s_ll_ext));    
    return ExecProcessAndExit(p_drv);
}
//////////////// llvm-link Execute //////////////////////////////////////////////////
// Example: "llvm-link source.bc amp_libm.bc builtins-hsail.bc -o source.linked.bc"
/////////////////////////////////////////////////////////////////////////////////////
errno_t Llvm_link::Execute(Tool *parent_tool)
{
    Aacl *p_drv= dynamic_cast<Aacl*>(parent_tool);
    if (!p_drv)
        return EXIT_FAILURE;
    if (IsExecutedOnce())
        ClearAllOptions();
    // finding tool executable
    errno_t iRet = FindExecutable();
    if (EXIT_SUCCESS != iRet)
        return iRet; 
    // output calculation
    tstring s_out_dir = GetOutputPath(p_drv);
    // input & output files
    tstring s_input_bc_files;
    tstring s_output_bc_file;
    tstring s_out_file_name_only = GetFileNameOnly(aa::s_input_file);
    s_out_file_name_only = s_out_dir + s_out_file_name_only;
    tstring s_i_file = s_out_file_name_only + s_bc_ext;
    AddInputFile(QuoteIfSpaces(s_i_file));
    s_output_file = s_out_file_name_only + s_dot + s_linked + s_bc_ext;
    AddOutputFile(QuoteIfSpaces(s_output_file));
    AddHardcodedOption(Dash(o_opt) + SpaceBefore(QuoteIfSpaces(s_output_file)));
    aalink_tool->FindLibraries(additional_bc_libs, additional_bc_libs_in_abs_paths, s_additional_bc_libs);
    return ExecProcessAndExit(p_drv);
}
//
void Llvm_link::BundleAllOptions()
{
    s_additional_bc_libs = Trim(s_additional_bc_libs);
    if (!s_additional_bc_libs.empty())
        s_input_files += SpaceBefore(s_additional_bc_libs);
    Tool::BundleAllOptions();
}
//////////////// opt Execute ///////////////////////////////////////////////////////////////
// Example: "opt -O3 -gpu -whole -verify -o matrix_multiply_amp.bc matrix_multiply_amp.ll"
////////////////////////////////////////////////////////////////////////////////////////////
errno_t Opt::Execute(Tool *parent_tool)
{
    // calling driver casting
    Aacl *p_drv = dynamic_cast<Aacl*>(parent_tool);
    if (!p_drv)
        return EXIT_FAILURE;
    if (IsExecutedOnce())
        ClearAllOptions();
    // finding tool executable
    errno_t iRet = FindExecutable();
    if (EXIT_SUCCESS != iRet)
        return iRet; 
    // output calculation
    tstring s_out_dir = GetOutputPath(p_drv);    
    tstring s_out_file_name_only = GetFileNameOnly(aa::s_input_file);
    s_out_file_name_only = s_out_dir + s_out_file_name_only;
    // -O3
    AddHardcodedOption(Dash(O3_opt));
    // -gpu
    AddHardcodedOption(Dash(gpu_opt));
    // -whole
    AddHardcodedOption(Dash(whole_opt));
    // -verify
    AddHardcodedOption(Dash(verify_opt));
    // input files
    tstring s_i_file = s_out_file_name_only + s_dot + s_linked + s_bc_ext;
    AddInputFile(QuoteIfSpaces(s_i_file));
    // output files
    s_output_file = s_out_file_name_only + s_dot + s_tmp + s_bc_ext;
    AddOutputFile(QuoteIfSpaces(s_output_file));
    AddHardcodedOption(Dash(o_opt) + SpaceBefore(QuoteIfSpaces(s_output_file)));
    return ExecProcessAndExit(p_drv);
}
//
void Opt::BundleAllOptions()
{
    if (b_debug)
        s_optimizations = Dash(O0_opt);
    Tool::BundleAllOptions();
}
//////////////// llc Execute ///////////////////////////////////////////////////////
// Example: "llc -march=hsail -o matrix_multiply_amp.hsail matrix_multiply_amp.bc"
////////////////////////////////////////////////////////////////////////////////////
errno_t Llc::Execute(Tool *parent_tool)
{
    Aacl *p_drv = dynamic_cast<Aacl*>(parent_tool);
    if (!p_drv)
        return EXIT_FAILURE;
    if (IsExecutedOnce())
        ClearAllOptions();
    // finding tool executable
    errno_t iRet = FindExecutable();
    if (EXIT_SUCCESS != iRet)
        return iRet;
    // output calculation
    tstring s_out_dir = GetOutputPath(p_drv);
    tstring s_out_file_name_only = GetFileNameOnly(aa::s_input_file);
    s_out_file_name_only = s_out_dir + s_out_file_name_only;
    // -O2
    AddHardcodedOption(Dash(O2_opt));
    if (Is32bitMachine())
        AddHardcodedOption(Dash(march_opt) + s_equal + s_hsail);
    else if (Is64bitMachine())
        AddHardcodedOption(Dash(march_opt) + s_equal + s_hsail64);
    if (b_brig_lowering)
    {
        AddHardcodedOption(Dash(disable_brig_lowering_opt) + s_equal + _T("0"));
        AddHardcodedOption(Dash(filetype_opt) + s_equal + s_obj);
        tstring s_obj_output_file = cl->GetObjFileName(p_drv, true);
        s_output_file = s_obj_output_file + s_brig_ext;
    }
    else
    {
        AddHardcodedOption(Dash(disable_brig_lowering_opt) + s_equal + _T("1"));
        s_output_file = s_out_file_name_only + s_hsail_ext;
    }
    AddOutputFile(QuoteIfSpaces(s_output_file));
    AddHardcodedOption(Dash(o_opt) + SpaceBefore(QuoteIfSpaces(s_output_file)));
    tstring s_i_file = s_out_file_name_only + s_dot + s_tmp + s_bc_ext;
    AddInputFile(QuoteIfSpaces(s_i_file));    
    iRet = ExecProcessAndExit(p_drv);
    if ((EXIT_SUCCESS == iRet) && (b_brig_lowering))
        b_no_device_code = IsBrigFileEmpty(s_output_file);
    return iRet;
}
//
void Llc::BundleAllOptions()
{
    if (b_debug)
        s_optimizations = Dash(O0_opt);
    Tool::BundleAllOptions();
}
//////////////// hsailasm Execute //////////////////////////////////////
// Example: "hsailasm -o matrix_multiply_amp.brig matrix_multiply_amp.hsail"
////////////////////////////////////////////////////////////////////////
errno_t HSAILasm::Execute(Tool *parent_tool)
{
    Aacl *p_drv = dynamic_cast<Aacl*>(parent_tool);
    if (!p_drv)
        return EXIT_FAILURE;
    if (IsExecutedOnce())
        ClearAllOptions();   
    // finding tool executable
    errno_t iRet = FindExecutable();
    if (EXIT_SUCCESS != iRet)
        return iRet; 
    // output calculation
    tstring s_out_dir = GetOutputPath(p_drv);    
    tstring s_out_file_name_wo_ext = GetFileNameOnly(aa::s_input_file);
    tstring s_obj_output_file = cl->GetObjFileName(p_drv, true);
    s_fatobj_filename = s_out_dir + s_obj_output_file;
    s_output_file = s_fatobj_filename + s_brig_ext;
    AddOutputFile(QuoteIfSpaces(s_output_file));
    AddHardcodedOption(Dash(o_opt) + SpaceBefore(QuoteIfSpaces(s_output_file)));
    AddInputFile(QuoteIfSpaces(s_out_file_name_wo_ext + s_hsail_ext));    
    iRet = ExecProcessAndExit(p_drv);
    b_no_device_code = IsBrigFileEmpty(s_output_file);
    return iRet;
}
//
int aacl_Main(int argc, tchar **argv)
{
    SetErrorMode(SEM_FAILCRITICALERRORS |
                 SEM_NOGPFAULTERRORBOX |
                 SEM_NOOPENFILEERRORBOX);
    
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
    _set_error_mode(_OUT_TO_STDERR);

    errno_t iRet = EXIT_SUCCESS;
    p_current_driver = aacl_tool;
    aacl_tool->s_abs_path = argv[0];
    if (argc == 2)
    {
        if ((argv[1] == Dash(V_opt)) || (argv[1] == Slash(V_opt)))
        {
            p_current_driver->VOption(argv[1]);        
            return AA_EXIT_SUCCESS_AND_STOP;
        }
    }
    // if only name
    size_t i_find = aacl_tool->s_abs_path.rfind(s_backslash);
    if (tstring::npos == i_find)
    {
        DWORD dwRet = GetExecutablePath(aacl_tool->s_abs_path);
        if (0 == dwRet)
            return CoutDWORDError(GetLastError());
    }
    i_find = aacl_tool->s_abs_path.rfind(s_backslash);
    if (tstring::npos == i_find)
        return CoutError(s_Wrong_path, aacl_tool->s_abs_path);
    path_to_exe = aacl_tool->s_abs_path.substr(0, i_find);

    aacl_tool->s_options = aacl_tool->s_abs_path;

    input_files.clear();
    aacl_tool->all_tmp_files.clear();

    iRet = GetAMD_bin(path_to_aa_bin);
    if (EXIT_SUCCESS != iRet)
        return CoutError(iRet, path_to_aa_bin);

    iRet = GetAMD_include(path_to_aa_include);
    if (EXIT_SUCCESS != iRet)
        return CoutError(iRet, path_to_aa_include);

    iRet = GetAMD_lib(path_to_aa_lib);
    if (EXIT_SUCCESS != iRet)
        return CoutError(iRet, path_to_aa_lib);

    // 0. aacl
    aacl_tool->SetArgs(argc, argv);
    // parsing arguments
    iRet = aacl_tool->Parse();
    if (EXIT_SUCCESS != iRet)
    {
        if (EXIT_FAILURE == iRet)
            return iRet;
        else if ((AA_EXIT_SUCCESS_AND_STOP != iRet) && (AA_EXIT_ERROR_PRINTED != iRet))
            return CoutError(iRet);
        else
            return EXIT_SUCCESS;
    }
    if (!input_files.empty())
    {
        std::list<tstring>::iterator l_i;
        for (l_i = input_files.begin(); l_i != input_files.end(); ++l_i)
        {
            s_input_file = *l_i;
            // 1. ampfe_host
            iRet = ampfe_host->Execute(aacl_tool);
            if (EXIT_SUCCESS != iRet)
                return iRet;
            ampfe_host->ExecutedOnce();
            // 2. ampfe
            iRet = ampfe->Execute(aacl_tool);
            if (EXIT_SUCCESS != iRet)
                return iRet;
            ampfe->ExecutedOnce();
            // 2.a llvm-as
            iRet = llvm_as->Execute(aacl_tool);
            if (EXIT_SUCCESS != iRet)
                return iRet;
            llvm_as->ExecutedOnce();
            // 2.b llvm-link
            iRet = llvm_link->Execute(aacl_tool);
            if (EXIT_SUCCESS != iRet)
                return iRet;
            llvm_link->ExecutedOnce();
            // 3. opt
            iRet = opt->Execute(aacl_tool);
            if (EXIT_SUCCESS != iRet)
                return iRet;
            opt->ExecutedOnce();
            // 4. llc
            iRet = llc->Execute(aacl_tool);
            if (EXIT_SUCCESS != iRet)
                return iRet;
            llc->ExecutedOnce();
            // 5. HSAILasm 
            if (!b_brig_lowering)
            {
                iRet = hsailasm->Execute(aacl_tool);
                if (EXIT_SUCCESS != iRet)
                    return iRet;
                hsailasm->ExecutedOnce();
            }
            // 6. cl
            iRet = cl->Execute(aacl_tool);
            if (EXIT_SUCCESS != iRet)
                return iRet;
            cl->ExecutedOnce();
            // 7. fatobj -join
            if (!b_no_device_code)
            {
                iRet = fatobj->Execute(aacl_tool);
                if (EXIT_SUCCESS != iRet)
                    return iRet;
                fatobj->ExecutedOnce();
            }
        }
        // deletion of all intermediate files
        iRet = aacl_tool->DeleteIntermediateFiles();
        if (EXIT_SUCCESS != iRet)
            return CoutError(iRet); 
        // 8. aalink
        if (!b_c_only)
        {
            aa::aalink_tool->AddSpecificOption(aacl_tool->s_aalink_specific_options);
            aa::aalink_tool->AddOption(link->s_options);
            iRet = aa::aalink_tool->Execute();
            if (EXIT_SUCCESS != iRet)
                return iRet;
            aa::aalink_tool->ExecutedOnce();
        }
    }
    // if no source files for cl; cl is not running, is only preparing link command line
    else
    {
        // 1. cl
        iRet = cl->Execute(aacl_tool);
        if (EXIT_SUCCESS != iRet)
            return iRet;
        cl->ExecutedOnce();
        // deletion of all intermediate files
        iRet = aacl_tool->DeleteIntermediateFiles();
        if (EXIT_SUCCESS != iRet)
            return CoutError(iRet); 
        // 2. aalink
        if (!b_c_only)
        {
            aa::aalink_tool->AddSpecificOption(aacl_tool->s_aalink_specific_options);
            aa::aalink_tool->AddOption(link->s_options);
            iRet = aa::aalink_tool->Execute();
            if (EXIT_SUCCESS != iRet)
                return iRet;
            aa::aalink_tool->ExecutedOnce();
        }
    }
    // -- LONG TERM --
    // TODO_HSA: avoid predecessor steps when prompted to start from an intermediate file. -- POSTPONED to LONG TERM
    // TODO_HSA: handle Microsoft options from:
    // http://msdn.microsoft.com/en-us/library/fwkeyyhe.aspx
    return EXIT_SUCCESS;
}

int tmain(int argc, tchar **argv)
{
// for attaching the aacl process only, when called by msbuild with temporary rsp file
#if defined _DEBUG
    Sleep(AA_DELAY_FOR_DEBUG);
#endif
    SetEncoding();
    SaveConsoleState();
    if (IsMightBeColored())
        SetConsoleTextAttribute(g_hOut, FOREGROUND_INTENSITY);
    return RestoreBeforeReturn(aacl_Main(argc, argv));
}
