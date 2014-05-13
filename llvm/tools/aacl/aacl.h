//===- aacl.h - AMD AMP C++ Compiler driver -------------------------------===//
//                                                                             //
//              (c) AMD 2013, all rights reserved.                             //
//                                                                             //
//===---------------------------------------------------------------------===//
#pragma once
#if !defined INC_AACL_H__
#define INC_AACL_H__

#include "aa.h"
#include <list> 

namespace aacl
{   
    const tstring s_aacl                = _T("aacl");
    const tstring s_aacl_description    = _T("AMD AMP Compiler");

    tstring slashLIBPATH;   /* /LIBPATH:        /LIBPATH:"FILEPATH"                                         */
    tstring slashEH;        /* /EH              /EH{s|a}[c][-]                  Exception Handling          */
    tstring slashZc;        /* /Zc              /Zc:arg1[-][,arg2[-]]           Conformance                 */

    // AMD AMP COMPILER OPTIONS
    const tstring march_opt     = _T("march");
    const tstring gpu_opt       = _T("gpu");
    const tstring verify_opt    = _T("verify");
    const tstring whole_opt     = _T("whole");
    const tstring d_opt         = _T("d");
    const tstring O3_opt        = _T("O3");
    const tstring O2_opt        = _T("O2");
    const tstring O1_opt        = _T("O1");
    const tstring O0_opt        = _T("O0");
    // ampfe specific options
    const tstring diag_remark_opt   = _T("diag_remark");
    const tstring diag_warning_opt  = _T("diag_warning");
    const tstring sys_include_opt   = _T("sys_include");
    // opt specific options
    const tstring enable_unsafe_fp_math_opt =_T("enable-unsafe-fp-math");
    // HSAILasm specific options
    const tstring bif_opt       = _T("bif");
    // aacl only called tools    
    const tstring s_ampfe_host  = _T("ampfe_host");
    const tstring s_ampfe       = _T("ampfe");
    const tstring s_opt         = _T("opt");
    const tstring s_llc         = _T("llc");
    const tstring s_llvm_as     = _T("llvm-as");
    const tstring s_llvm_link   = _T("llvm-link");
    const tstring s_hsailasm    = _T("hsailasm");
    const tstring s_HSAILasm    = _T("HSAILasm");

    // intermediate file names (relative or absolute whatever)
    // <source file w/o ext>_amp_host<.cpp ext> - produced by ampfe_host.exe
    tstring s_amp_host_output_cpp_filename;
    // .ll file produced by ampfe.exe
    tstring s_amp_ll_filename;
    // .host.cpp file produced by ampfe.exe
    tstring s_amp_point_host_filename;
    // .bc file produced by opt.exe
    tstring s_amp_bc_filename;
    // .hsail file produced by llc.exe
    tstring s_amp_hsail_filename;
    // .bin file produced by objgen.exe
    tstring s_amp_bin_filename;
    // set to true in case of any of the following options:
    // /LD, /LDd, /MD, /MDd, /ML, /MLd, /MT or /MTd 
    // NOTICE: /ML & /MLd - depricated since VS 2005 (last used in .Net 2003)
    // http://msdn.microsoft.com/en-us/library/2kzt1wy3(v=vs.71).aspx
    bool b_Use_RT_Library = false;

    // class for Aacl
    class Aacl : public aa::Singleton_tool<Aacl>, public aa::Driver
    {
    protected:
        //
        Aacl() 
        {  
            b_remove_optios_from_diagnostics = true;
            b_remove_includes_from_diagnostics = true;
            b_remove_compulsory_options_from_diagnostics = false;
            b_remove_output_files_from_diagnostics = true;
            b_remove_ignored_options_from_diagnostics = false;
            b_no_device_code = false;
        }
        //
        friend class aa::Singleton_tool<Aacl>;
        //
        virtual ~Aacl() {}
        //
        tstring s_additional_bc_libs;
        // mapping optimization options /O to corresponding options in aa tool chain (opt, llc)
        void MapOptimizationOptions(tstring sOption);
        // mapping /Z7 /ZI Zi options to corresponding options in aa tool chain
        void MapDebugInformationFormatOptions(tstring sOption);
        // checks the constraints (incompatible options, for example)
        errno_t CheckConstraints();

    public:
        //
        bool b_no_device_code;
        //
        tstring s_aalink_specific_options;
        //
        tstring s_CL_options;
        //
        virtual tstring GetName() {return s_aacl;}
        //
        virtual errno_t ParseArgs(int argc, tchar **argv, bool b_main_command_line = true);
        // parses CL Env.var. before parsing command line
        errno_t ParseCL(unsigned int cout_type = aa::AACL_NOANYMESSAGE);
        //
        virtual errno_t Execute(Tool * /*parent_tool = nullptr*/) {return EXIT_SUCCESS;}
        //
        virtual errno_t Parse();
        //
        virtual errno_t Diagnostics();
        // returns the path to the *.obj file, specified in /Fo option (path w/o filename)
        // if /Fo is not specified returns GetCurDir()
        virtual tstring GetOutputPath();
        // returns the filename only (w/o extension & path) for the *.obj file specified in /Fo
        virtual tstring GetOutputFileName(bool b_wo_extension = true);
        //
        virtual tstring GetDescription() {return s_aacl_description;}
        //
        virtual Tool* GetToolByName(const tstring &name);
        //
        virtual bool HelpOption(const tstring &str, bool bPrint = true);
        //
        virtual bool IsToolName(const tstring &str);
        //
        errno_t Get_VCPP_includes(tstring &s_VCPP_inc, bool b_in_sys_include_syntax = true);
        //
        virtual void AddSpecificOption(const tstring &option, bool b_add_also_for_aalink = true);
    };
    // base abstract class for every tool driven by aacl
    // class for ampfe_host 
    class Ampfe_host : public aa::Singleton_tool<Ampfe_host>, public aa::Tool
    {
    protected:
        //
        Ampfe_host()
        {
            b_remove_compulsory_options_from_diagnostics = false;
            b_remove_output_files_from_options = true;
            b_add_parent_driver_includes = true;
            b_sys_includes = true;
        }
        //
        friend class aa::Singleton_tool<Ampfe_host>;
        //
        virtual ~Ampfe_host() {}
    public:
        //
        virtual tstring GetName() {return s_ampfe_host;}
        //
        virtual errno_t Execute(Tool *parent_tool = nullptr);
    };
    // class for ampfe
    class Ampfe : public aa::Singleton_tool<Ampfe>, public aa::Tool
    {
    protected:
        //
        Ampfe() 
        {
            b_remove_compulsory_options_from_diagnostics = false;
            b_remove_output_files_from_options = true;
            b_add_parent_driver_includes = true;
            b_sys_includes = true;
        }
        //
        friend class aa::Singleton_tool<Ampfe>;
        //
        virtual ~Ampfe() {}
    public:
        //
        virtual tstring GetName() {return s_ampfe;}
        //
        virtual errno_t Execute(Tool *parent_tool = nullptr);
    };
    // class for opt
    class Opt : public aa::Singleton_tool<Opt>, public aa::Tool
    {
    protected:
        //
        Opt() 
        {
            b_remove_includes_from_diagnostics = true;
            b_remove_output_files_from_options = true;
            b_debug = false;
        }
        //
        friend class aa::Singleton_tool<Opt>;
        //
        virtual ~Opt() {}
    public:
        //
        bool b_debug;
        //
        virtual tstring GetName() {return s_opt;}
        //
        virtual errno_t Execute(Tool *parent_tool = nullptr);
        //
        virtual void BundleAllOptions();
    };
    // class for llc
    class Llc : public aa::Singleton_tool<Llc>, public aa::Tool
    {
    protected:
        //
        Llc() 
        {
            b_remove_includes_from_diagnostics = true;
            b_remove_output_files_from_options = true;
            b_debug = false;
        }
        //
        friend class aa::Singleton_tool<Llc>;
        //
        virtual ~Llc() {}
    public:
        //
        bool b_debug;
        //
        virtual tstring GetName() {return s_llc;}
        //
        virtual errno_t Execute(Tool *parent_tool = nullptr);
        //
        virtual void BundleAllOptions();
    };
    // class for llvm-as
    class Llvm_as : public aa::Singleton_tool<Llvm_as>, public aa::Tool
    {
    protected:
        //
        Llvm_as() 
        {
            b_remove_includes_from_diagnostics = true;
            b_remove_output_files_from_options = true;
        }
        //
        friend class aa::Singleton_tool<Llvm_as>;
        //
        virtual ~Llvm_as() {}
    public:
        //
        virtual tstring GetName() {return s_llvm_as;}
        //
        virtual errno_t Execute(Tool *parent_tool = nullptr);
    };
    // class for llvm-link
    class Llvm_link : public aa::Singleton_tool<Llvm_link>, public aa::Tool
    {
    protected:
        //
        Llvm_link() 
        {
            b_remove_includes_from_diagnostics = true;
            b_remove_output_files_from_options = true;
            AddBcLib(aa::s_amp_libm + aa::s_bc_ext);
            AddBcLib(aa::s_builtins_hsail + aa::s_bc_ext);
        }
        //
        friend class aa::Singleton_tool<Llvm_link>;
        //
        friend class aacl::Aacl;
        //
        virtual ~Llvm_link() {}
        //
        tstring s_additional_bc_libs;
        //
        std::list<tstring> additional_bc_libs;
        //
        std::list<tstring> additional_bc_libs_in_abs_paths; 

    public:
        //
        virtual tstring GetName() {return s_llvm_link;}
        //
        virtual errno_t Execute(Tool *parent_tool = nullptr);
        //
        virtual void BundleAllOptions();
        //
        void AddBcLib(const tstring &s_file) {additional_bc_libs.push_back(s_file);}
        //
        void RemoveBcLib(const tstring &s_file) {additional_bc_libs.remove(s_file);}
    };
    // class for HSAILasm
    class HSAILasm : public aa::Singleton_tool<HSAILasm>, public aa::Tool
    {
    protected:
        //
        HSAILasm() 
        {
            b_remove_includes_from_diagnostics = true;
            b_remove_output_files_from_options = true;
        }
        //
        friend class aa::Singleton_tool<HSAILasm>;
        //
        virtual ~HSAILasm() {}

    public:
        //
        virtual tstring GetName() {return s_HSAILasm;}
        //
        virtual errno_t Execute(Tool *parent_tool = nullptr);
    };
}
#endif // INC_AACL_H__
