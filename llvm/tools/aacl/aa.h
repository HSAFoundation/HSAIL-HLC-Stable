//===- aa.h - AMD AMP C++ Compiler driver ---------------------------------===//
//                                                                             //
//              (c) AMD 2013, all rights reserved.                             //
//                                                                             //
//===---------------------------------------------------------------------===//
#pragma once
#if !defined INC_AA_H__
#define INC_AA_H__

#if !defined AA_EXIT_ERROR_PRINTED
    #define AA_EXIT_ERROR_PRINTED -1
#endif
#if !defined AA_MAX_BUF_SIZE
    #define AA_MAX_BUF_SIZE 2048
#endif

#define AA_EXIT_SUCCESS_AND_STOP -512*512

#if !defined AA_DELAY_FOR_DEBUG
    #define AA_DELAY_FOR_DEBUG 1
#endif

#include "mydefs.h"
#include "winbase.h"
#include <locale>
#include <codecvt>

extern "C" const IMAGE_DOS_HEADER __ImageBase;

namespace aa
{   
    // colors before driver's work
    WORD g_colors_old;
    // STD_OUTPUT_HANDLE
    HANDLE g_hOut;
    // STD_INPUT_HANDLE
    HANDLE g_hIn;
    // STD_ERROR_HANDLE
    HANDLE g_hErr;
    //
    bool b_color = false;
    //
    bool IsMightBeColored() { if(aa::b_color) if (_isatty(_fileno(stdout))) return true; return false;}
    //
    bool Is32bitMachine() {return sizeof(nullptr) == 4;} 
    //
    bool Is64bitMachine() {return sizeof(nullptr) == 8;} 
    //
    class Driver;
    // ENV VARS
    const tstring s_amd_env_var = _T("AMD_CPP_COMPILER01");
    //
    const tstring s_AMD_copyrignt       = _T("(C) AMD 2012, all rights reserved.");
    const tstring s_cl_description      = _T("Microsoft C++ Compiler");
    const tstring s_link_description    = _T("Microsoft C++ Linker");
    const tstring s_aalink              = _T("aalink");
    const tstring s_aalink_description  = _T("AMD AMP Linker");
    // aacl/aalink debug level {1,2}
    unsigned int debugSetLevel = 0;
    // SYMBOLS
    // empty string const
    const tstring s_empty       = _T("");
    //   space as string 
    const tstring s_space       = _T(" ");
    //   space as char
    const tchar ch_space        = _T(' ');
    // " quote as string 
    const tstring s_quote       = _T("\"");
    // " quote as char 
    const tchar ch_quote        = _T('"');
    // ' single quote as string 
    const tstring s_squote      = _T("'");
    // ' single quote as char 
    const tchar ch_squote       = _T('\'');
    // \ back slash as string 
    const tstring s_backslash   = _T("\\");
    // \ back slash as char
    const tchar ch_backslash    = _T('\\');
    // / slash as string
    const tstring s_slash       = _T("/");
    // / slash as char
    const tchar ch_slash        = _T('/');
    // point as string
    const tstring s_dot         = _T(".");
    // point as char
    const tchar ch_dot          = _T('.');
    // dash as string
    const tstring s_dash        = _T("-");
    // dash as char
    const tchar ch_dash         = _T('-');
    // underscope as string
    const tstring s_underscope  = _T("_");
    // underscope as char
    const tchar ch_underscope   = _T('_');
    // colon as string
    const tstring s_colon       = _T(":");
    // colon as char
    const tchar ch_colon        = _T(':');
    // equal as string
    const tstring s_equal       = _T("=");
    // equal as char
    const tchar ch_equal        = _T('=');
    // semicolon as string
    const tstring s_semicolon   = _T(";");
    // semicolon as char
    const tchar ch_semicolon    = _T(';');
    // question as string
    const tstring s_question    = _T("?");
    // question as char
    const tchar ch_question     = _T('?');
    // exclam as string
    const tstring s_exclam      = _T("!");
    // exclam as char
    const tchar ch_exclam       = _T('!');
    // sharp as string
    const tstring s_sharp       = _T("#");
    // sharp as char
    const tchar ch_sharp        = _T('#');
    // dbl dash --
    const tstring s_dbl_dash    = _T("--");
    // dbl backslash
    const tstring s_dbl_backslash = s_backslash + s_backslash;
    // dbl pont
    const tstring s_dbl_point   = _T("..");
    // up on 1 direcory level
    tstring s_up = s_dbl_point + s_backslash;
    // EXTENSIONS
    const tstring s_exe_ext     = _T(".exe"); 
    const tstring s_dll_ext     = _T(".dll");
    const tstring s_cpp_ext     = _T(".cpp"); 
    const tstring s_c_ext       = _T(".c"); 
    const tstring s_cc_ext      = _T(".cc"); 
    const tstring s_cxx_ext     = _T(".cxx"); 
    const tstring s_ll_ext      = _T(".ll"); 
    const tstring s_bc_ext      = _T(".bc"); 
    const tstring s_hsail_ext   = _T(".hsail");
    const tstring s_brig_ext    = _T(".brig");
    const tstring s_bif_ext     = _T(".bif");
    const tstring s_bin_ext     = _T(".bin");
    const tstring s_host_ext    = _T(".host");
    const tstring s_obj_ext     = _T(".obj");
    const tstring s_lib_ext     = _T(".lib");
    const tstring s_fatobj_ext  = _T(".fatobj");
    // if /Fo is not set the default extension of aacl output file will be .obj;
    // but actually it will be fatobj; 
    // the extension for cl output file (aacl phase) will be .obj.obj
    const tstring s_default_aacl_output_file_ext  = s_obj_ext;
    const tstring s_opt_ext     = _T(".opt");
    const tstring s_dev_ext     = _T(".dev"); 
    // Paths
    const tstring s_VC_bin = _T("\\VC\\bin\\");
    // name of the directory for amd amp binaries
    const tstring s_amd_bin_dir_name = _T("bin");
    // name of the directory for amd includes
    const tstring s_amd_include_dir_name = _T("include");
    const tstring s_VS          = _T("VS");
    const tstring s_COMNTOOLS   = _T("COMNTOOLS");
    const tstring s_VC          = _T("VC");
    const tstring s_bin         = _T("bin");
    const tstring s_lib         = _T("lib");
    const tstring s_IDE         = _T("IDE");
    const tstring s_PATH        = _T("Path");
    const tstring s_include     = _T("include");
    const tstring s_INCLUDE     = _T("INCLUDE");
    const tstring s_CL          = _T("CL");
    const tstring s_cl_exe      = _T("cl.exe");
    const tstring s_LIB         = _T("LIB");
    const tstring s_LINK        = _T("LINK");
    const tstring s_link_exe    = _T("link.exe");
    const tstring s_x86         = _T("x86");
    const tstring s_x64         = _T("x64");
    const tstring s_64          = _T("64");
    const tstring s_amd64       = _T("amd64");
    const tstring s_x86_64      = _T("x86_64");
    const tstring s_w7          = _T("w7");
    const tstring s_w764a       = _T("w764a");
    const tstring s_B_dbg       = _T("B_dbg");
    const tstring s_B_rel       = _T("B_rel");
    const tstring s_library     = _T("library");
    const tstring s_build       = _T("build");
    const tstring s_tmp         = _T("tmp");
    const tstring s_obj         = _T("obj");

    const tstring s_hsa         = _T("hsa");
    const tstring s_hsail       = _T("hsail");
    const tstring s_hsail64     = _T("hsail-64");
    const tstring s_amd         = _T("amd");
    const tstring s_amp         = _T("amp");
    const tstring s_amp_host    = _T("amp_host");

    const tstring s_command_line        = _T(" command line       = ");
    const tstring s_specific_options    = _T(" specific options   = ");
    const tstring s_ignored_options     = _T(" ignored options    = ");
    const tstring s_compulsory_options  = _T(" compulsory options = ");
    const tstring s_hardcoded_options   = _T(" hard-coded options = ");
    const tstring s_includes            = _T(" includes           = ");
    const tstring s_input_files         = _T(" input files        = ");
    const tstring s_output_files        = _T(" output files       = ");
    const tstring s_rsp_file            = _T(" @*.rsp file        = ");

    const tstring s_error				= _T(" error: ");
    const tstring s_warning				= _T(" warning: ");
    const tstring s_message				= _T(" message: ");
    const tstring s_syntax_error		= _T("syntax error");
    const tstring s_internal_error		= _T("internal error");
    const tstring s_Unknown_error		= _T("Unknown error");    
    const tstring s_return_code			= _T("return code:");
    const tstring s_Env_var				= _T("Environment variable");
    const tstring s_doesnt_exist		= _T("does not exist!");
    const tstring s_failed              = _T("failed");
    const tstring s_Failed_to_allocate_memory = _T("Failed to allocate memory");
    const tstring s_Wrong_path			=_T("Wrong path");
    const tstring s_Version             = _T("Version");
    const tstring s_Built               = _T("Built");
    const tstring s_MS_VS               = _T("Microsoft Visual Studio");
    const tstring s_supports			= _T("supports");
    const tstring s_or_higher			= _T(" or higer");
    const tstring s_wasnt_found			= _T("wasn't found");
    const tstring s_was_deleted_suc		= _T("was deleted successfully");
    const tstring s_of					= _T("of");
    const tstring s_in					= _T("in");
    const tstring s_deleted_files		= _T("deleted files");
    const tstring s_OVERVIEW			= _T("OVERVIEW: ");
    const tstring s_USAGE				= _T("USAGE: ");
    const tstring s_OPTIONS				= _T("OPTIONS: ");
    const tstring s_aacl_syntax			= _T("[options] <input file>");
    const tstring s_option				= _T("option");
    const tstring s_Option				= _T("Option");
    const tstring s_Option_is_ignored	= _T("Option is ignored.");
    const tstring s_more_than_1_file	= _T("More than one source file is not yet supported by");
    const tstring s_Tool_name_was_not_specified = _T("Tool name was not specified.");
    const tstring s_Option_argument_was_not_specified = _T("Option argument was not specified.");
    const tstring s_must_follow			= _T("must follow");
    const tstring s_Wrong_tool_name		= _T("Wrong tool name:");
    const tstring s_Wrong_option_argument = _T("Wrong option argument:");
    const tstring s_diagnostics			= _T("diagnostics");
    const tstring s_not_enough_cmd_args = _T("Not enough positional command line arguments specified!");
    const tstring s_must_specify_at_least_1 = _T("Must specify at least 1 positional argument");
    const tstring s_works_only_together_with = _T("works only together with");
    const tstring s_doesnt_work_with    = _T("doesn't work with");
    const tstring s_linked      		= _T("linked");
    const tstring s_Command_line_error  = _T("Command line error");
    const tstring s_command_line_options_are_incompatible = _T("command-line options are incompatible");
    const tstring s_CL_options          = _T(" CL options         = ");
    const tstring s_LINK_options        = _T(" LINK options       = ");
    const tstring s_LIB_paths           = _T(" LIB paths          = ");

    // both: aacl & aalink called tools
    const tstring s_cl          = _T("cl");
    const tstring s_link        = _T("link");
    const tstring s_inflate     = _T("inflate");
    const tstring s_fatobj      = _T("fatobj");
    // aalink only called tools    
    const tstring s_devlink     = _T("devlink");
    //
    // MICROSOFT COMPILER OPTIONS
    const tstring D_opt         = _T("D");
    const tstring I_opt         = _T("I");
    const tstring O_opt         = _T("O");
    const tstring EH_opt        = _T("EH");
    const tstring MD_opt        = _T("MD");
    const tstring MT_opt        = _T("MT");
    const tstring LD_opt        = _T("LD");
    const tstring Fo_opt        = _T("Fo");
    const tstring slashZc_opt   = _T("Zc");
    const tstring Zi_opt        = _T("Zi");
    const tstring ZI_opt        = _T("ZI");
    const tstring Z7_opt        = _T("Z7");
    const tstring WX_opt        = _T("WX");
    const tstring forScope_opt  = _T("forScope");
    const tstring wchar_t_opt   = _T("wchar_t");
    const tstring link_opt      = _T("link");
    const tstring fp_opt        = _T("fp");
    const tstring precise_opt   = _T("precise");
    const tstring except_opt    = _T("except");
    const tstring fast_opt      = _T("fast");
    const tstring strict        = _T("strict");
    const tstring s_DEBUG       = _T("DEBUG");
    const tstring s_DLL         = _T("DLL");
    const tstring s_DLL_CPPLIB  = _T("DLL_CPPLIB");
    const tstring s_MT          = _T("MT");
    // MICROSOFT LINKER OPTIONS
    // for all linker options all comparisions should be done in upcased mode!
    const tstring DEBUG_opt   = _T("DEBUG");
    const tstring IMPLIB_opt  = _T("IMPLIB:");
    const tstring LIBPATH_opt = _T("LIBPATH:");
    const tstring MACHINE_opt = _T("MACHINE:");
    const tstring OUT_opt     = _T("OUT:");
    //////////////////////////////
    // AMD AMP COMPILER OPTIONS //
    //////////////////////////////
    ///////////////////////////
    // aacl specific options //
    ///////////////////////////
    const tstring debug1_opt                    = _T("debug1");
    const tstring debug1_opt_descr              = _T("Enable debug output");
    const tstring debug2_opt                    = _T("debug2");
    const tstring debug2_opt_descr              = _T("Enable debug output with extended diagnostics");
    const tstring keeptmp_opt                   = _T("keeptmp");
    const tstring keeptmp_opt_descr             = _T("Keep all AMD AMP Compiler related intermediate files");
    const tstring tmpdir_opt                    = _T("tmpdir");
    const tstring help_opt                      = _T("help");
    const tstring help_hidden_opt               = _T("help-hidden");
    const tstring help_opt_descr                = _T("Display available options (") + help_hidden_opt  + _T(" for more)");
    const tstring version_opt_descr             = _T("Display the version of this program");
    const tstring color_opt                     = _T("color");
    const tstring color_opt_descr               = _T("Color the output messages, warnings and errors");
    const tstring tool_opt                      = _T("tool");
    const tstring tool_opt_descr                = _T("Direct transfer the option to the tool: -tool=<tool name> -<tool specific option>");
    const tstring temp_disable_brig_lowering_opt        = _T("temp-disable-brig-lowering");
    const tstring temp_disable_brig_lowering_opt_descr  = _T("Temporary option for disabling brig lowering path (leads to invocation of HSAILasm)");
    const tstring no_default_lib_opt            = _T("no-default-lib:");
    const tstring no_default_lib_opt_descr      = _T("Demand not to link specified library (*.bc) by llvm-link");
    const tstring hidden_aacl_driven_opt                = _T("hidden-aacl-driven");
    const tstring hidden_aacl_driven_opt_descr          = _T("Hidden option which says that aalink is executed from under aacl");

    const tstring V_opt             = _T("V"); // version number    /Vstring    deprecated
    const tstring version_opt       = _T("version");
    const tstring o_opt             = _T("o");
    const tstring c_opt             = _T("c");
    const tstring h_opt             = _T("h");
    const tstring d_opt             = _T("d");
    ////////////////////////////
    // ampfe specific options //
    ////////////////////////////
    const tstring g_opt             = _T("g");
    //////////////////////////
    // llc specific options //
    //////////////////////////
    const tstring disable_brig_lowering_opt = _T("disable-brig-lowering");
    const tstring filetype_opt      = _T("filetype");
    //////////////////////////////
    // inflate specific options //
    //////////////////////////////
    const tstring obsolete_opt      = _T("obsolete");
    /////////////////////////////
    // fatobj specific options //
    /////////////////////////////
    const tstring join_opt          = _T("join");
    const tstring split_opt         = _T("split");
    const tstring create_opt        = _T("create");
    const tstring extract_opt       = _T("extract");
    const tstring detect_type_opt   = _T("detect-type");
    //////////////////////////////
    // devlink specific options //
    //////////////////////////////
    const tstring obif_opt      = _T("obif");

    const tstring s_amp_libm            = _T("amp_libm");
    const tstring s_builtins_hsail      = _T("builtins-hsail");

    // the list of input source (C++) files
    std::list<tstring> input_files;
    // C++ source file (from input_files list), which is currently compiling
    tstring s_input_file;
    // flags
    bool b_c_only = false;

    // aacl/aalink specific
    tstring dashkeeptmp;    /*  -keeptmp[-]                               Keep or not Intermediate files     */
 
    // Byte Order Mark types
    enum BOM_TYPE
    {
        BOM_NO = 0,
        BOM_UTF_8 = 1,
        BOM_UTF_16_LE = 2,
        BOM_UTF_16_BE = 3,
        BOM_UTF_32_LE = 4,
        BOM_UTF_32_BE = 5
    };
    // type of cout-ed text
    enum AACL_COUT_TYPE
    {
        AACL_NOANYMESSAGE = 0,
        AACL_MESSAGE,
        AACL_WARNING,
        AACL_ERROR,
    };
    // path to aacl dir
    tstring path_to_exe;
    // path to the whole amd amp tools stack dir
    tstring path_to_aa_bin; 
    // path to additional amd amp includes dir
    tstring path_to_aa_include;
    // path to additional amd amp libs dir
    tstring path_to_aa_lib;
    // all adeded or modified env. vars.
    tstring s_all_env_vars;   
    //
    tstring s_obj_filename;
    //
    tstring s_opt_filename;
    //
    tstring s_fatobj_filename;
    //
    tstring s_brig_filename;
    //
    tstring s_bif_filename;
    //
    tstring s_bif_cpp_filename;
    //
    bool b_no_device_code = false;
    //
    bool b_brig_lowering = true;
    //
    Driver *p_current_driver = nullptr;
    //
    tstring s_tmp_prefix = _T("########"); 
    // Singleton base class
    template <class T>
    class Singleton_tool
    {   
        //
        static T* _self;
        //
        static int _refcount;
    protected:
        //
        Singleton_tool() {}
        //
        virtual ~Singleton_tool() {_self = nullptr;}
    public:
        //
        static T* Instance();
        //
        void FreeInstance();
    };
    // Tool class for creating hierarchy of tool classes
    // to support polymorphism (for ex.: Tool* GetToolByName(const tstring name))
    class Tool
    {
    public:
        //
        tstring GetVersion() {return _T("0.0.3.8");}
        // all options
        tstring s_options;
        // all options (including hardcoded, specific, current) just before tool execution
        tstring s_all_options;
        // options specific only for current execution of tool
        // (almost input/output file specifics)
        tstring s_current_run_options;
        // all specific options
        tstring s_specific_options;
        // all ignored options
        tstring s_ignored_options;
        // all hardcoded options
        tstring s_hardcoded_options;
        // all input files
        tstring s_input_files;
        // all output files
        tstring s_output_files;
        // all includes
        tstring s_includes;
        // all compulsory options
        tstring s_compulsory_options;
        // generated defines (for ampfe_host & ampfe), for example from /MD & /MT options
        tstring s_defines;
        // all optimization options
        tstring s_optimizations;
        // /Zi or /ZI or /Z7 (/Zd is depricated since VS2005)
        tstring s_debug_options;
        //
        bool b_stop_adding_option;
        // absolute path to the tool's executable
        tstring s_abs_path;
        //
        virtual errno_t Diagnostics();
        //
        virtual tstring GetName() = 0;
        //
        virtual errno_t Execute(Tool *parent_tool = nullptr) = 0;
        //
        virtual errno_t ExecProcess();
        //
        virtual errno_t ExecProcessAndExit(Driver *parent_driver);
        //
        virtual errno_t Where(tstring &abs_file_name);
        //
        errno_t FindExecutable();
        //
        virtual void AddOption(const tstring &option, bool b_current_run_only_opt = false);
        //
        virtual void AddSpecificOption(const tstring &option);
        //
        virtual void AddCompulsoryOption(const tstring &option);
        //
        virtual void AddIncludes(const tstring &option);
        //
        virtual void PrependIncludes(const tstring &option);
        //
        virtual void AddInputFile(const tstring &option);
        //
        virtual void AddOutputFile(const tstring &option);
        //
        virtual void AddHardcodedOption(const tstring &option);
        //
        virtual void BundleAllOptions();
        //
        errno_t Output(bool b_tool_diagnostics = true);
        //
        void ExecutedOnce() {b_executed_once = true;}
        //
        bool IsExecutedOnce() {return b_executed_once;}
        //
        tstring  GetAbsPath() {return s_abs_path;}
        //
        tstring GetOutputPath(Driver *parent_driver);
        //
        void ClearAllOptions();

    protected:
        //
        bool b_remove_output_files_from_diagnostics;
        //
        bool b_remove_output_files_from_options;
        //
        bool b_input_files_before_output_files;
        //
        bool b_remove_input_files_from_options;
        //
        bool b_remove_includes_from_diagnostics;
        //
        bool b_remove_ignored_options_from_diagnostics;
        //
        bool b_remove_optios_from_diagnostics;
        //
        bool b_remove_compulsory_options_from_diagnostics;
        // add or not the parent driver's includes (saved from cmd line) just before other includes
        bool b_add_parent_driver_includes;
        // if includes are in sys_include format
        bool b_sys_includes;
        // must be set in true after first successful run
        bool b_executed_once;
        //
        tstring s_output_file;
        //
        Tool(): b_remove_optios_from_diagnostics(false)
              , b_remove_includes_from_diagnostics(false)
              , b_remove_compulsory_options_from_diagnostics(true)
              , b_remove_output_files_from_diagnostics(false)
              , b_remove_input_files_from_options(false)
              , b_remove_ignored_options_from_diagnostics(true)
              , b_remove_output_files_from_options(false)
              , b_stop_adding_option(false)
              , b_executed_once(false)
              , b_input_files_before_output_files(false)
              , b_add_parent_driver_includes(false)
              , b_sys_includes(true)
        {}
        //
        virtual void BundleInputFiles();
        //
        virtual void BundleOutputFiles();
        //
        virtual ~Tool() {}
    };
    //
    template <class T>
    T *Singleton_tool<T>::_self = nullptr;
    //
    template <class T>
    int  Singleton_tool<T>::_refcount = 0;
    //
    template <class T>
    T *Singleton_tool<T>::Instance()
    {
        if(!_self)
            _self = new T;
        _refcount++;
        return _self;
    }
    //
    template <class T>
    void Singleton_tool<T>::FreeInstance()
    {
        if(--_refcount == 0)
            delete this;
    }
    //
    class Driver: public Tool
    {
    protected:
        //
        Driver() {}
        //
        virtual ~Driver() {}
        //
        int argc;
        //
        tchar **argv;

    public:
        // the list of all intermediate and temporary files
        // might be absolute or relative, and also with or without extension
        // every filename must be verified on existance and access before any manipulation with it
        std::list<tstring> all_tmp_files;        
        //
        tstring s_rsp_cmdline;
        // all sys includes (for ampfe_host & ampfe)
        tstring s_sys_includes;
        //
        virtual tstring GetDescription() = 0;
        //
        bool VersionOption(const tstring &str, bool bPrint = true);
        // adding ..Microsoft Visual Studio 11.0\Common7\IDE to PATH env. var.  
        errno_t SetVSIDEtoPath(unsigned int cout_type = aa::AACL_ERROR);
        //
        errno_t GetVSpath(tstring &env_val);
        //
        errno_t GetVC_include_paths(std::list<tstring> &cl_include_list);
        // sets command line arguments
        void SetArgs(int argc, tchar **argv) {this->argc = argc; this->argv = argv;}
        //
        virtual errno_t Parse() {return EXIT_SUCCESS;}
        //
        virtual Tool* GetToolByName(const tstring &name) = 0;
        //
        void AddSpecificOptionForTool(const tstring &tool, const tstring &option);
        //
        virtual tstring GetOutputPath() {return s_empty;}
        //
        virtual bool HelpOption(const tstring &str, bool bPrint = true) {if (str.size()>0) bPrint = true; return true;}
        //
        virtual bool VOption(const tstring &str, bool bPrint = true);
        //
        virtual errno_t Diagnostics();
        //
        errno_t DeleteIntermediateFiles();
        //
        errno_t CheckAndAddIntermediateFile(const tstring &s_file, unsigned int cout_type = aa::AACL_ERROR);
        //
        virtual bool IsToolName(const tstring &str) = 0;
        //
        virtual tstring GetOutputFileName(bool b_wo_extension = true) {if (b_wo_extension) return s_empty; return s_empty;}
        //
        virtual void AddSysIncludes(const tstring &option);
    };
    // class for Aalink
    class Aalink : public aa::Singleton_tool<Aalink>, public aa::Driver
    {
    protected:
        //
        Aalink() 
        {  
            b_remove_optios_from_diagnostics = true;
            b_remove_includes_from_diagnostics = true;
            b_remove_compulsory_options_from_diagnostics = false;
            b_remove_output_files_from_diagnostics = true;
            b_remove_ignored_options_from_diagnostics = false;
            b_OUT = false;
            b_DLL = false;
            b_run_separate_link = true;
        }
        //
        friend class aa::Singleton_tool<Aalink>;
        //
        virtual ~Aalink() {}
        //
        std::list<tstring> lib_paths;
        //
        std::list<tstring> fatobj_files;
        //
        std::list<tstring> fatobj_files_in_abs_paths; 

    public:
        // Is output file DLL?
        bool b_DLL;
        // Is output path specified?
        bool b_OUT;
        // if hidden option hidden_aacl_driven option is set then false
        // true (by default) is only in case of separate from aacl invocation of aalink
        bool b_run_separate_link;
        // filename under /OUT option (PE executable)
        tstring s_OUT_filename;
        //
        tstring s_rsp_cmdline;
        //
        tstring s_LINK_options;
        //
        tstring s_LIB_paths;
        //
        virtual tstring GetName() {return s_aalink;}
        // parses LINK Env.var. before parsing command line
        errno_t ParseLINK(unsigned int cout_type = aa::AACL_NOANYMESSAGE);
        //
        virtual errno_t ParseArgs(int argc, tchar **argv, bool b_main_command_line = true);
        // parses LINK Env.var. before parsing command line
        // errno_t ParseLINK(unsigned int cout_type = aa::AACL_NOANYMESSAGE);
        //
        virtual errno_t Execute(Tool *parent_tool = nullptr);
        //
        virtual errno_t Parse();
        //
        virtual errno_t Diagnostics();
        // returns the path to the *.obj file, specified in /Fo option (path w/o filename)
        virtual tstring GetDescription() {return s_aalink_description;}
        //
        virtual Tool* GetToolByName(const tstring &name);
        //
        virtual tstring GetOutputPath();
        //
        virtual bool HelpOption(const tstring &str, bool bPrint = true);
        //
        virtual bool IsToolName(const tstring &str);
        //
        void AddLibPath(const tstring &s_lib_path);
        //
        errno_t Get_LIB();
        //
        std::list<tstring> Get_lib_paths(); 
        //
        void AddFatobjFile(const tstring &s_file) {fatobj_files.push_back(s_file);}
        //
        errno_t ExecuteFatobjs();
        //
        void FindLibraries(const std::list<tstring> &libs, std::list<tstring> &libs_in_abs_paths, tstring &cmd_string);
        //
        void FindLibrary(const tstring &lib, tstring &lib_in_abs_path, tstring &cmd_string);
        //
        bool IsOUTspecified() {return b_OUT;}
        // returns filename w/o extension from option /OUT:filename
        virtual tstring GetOutputFileName() {return s_OUT_filename;}
        //
        virtual void AddSpecificOption(const tstring &option);
        //
        bool IsContainingDeviceCode();
    };
    // class for cl
    class Cl : public aa::Singleton_tool<Cl>, public aa::Driver
    {
    public:
        // MICROSOFT COMPILER OPTIONS
        tstring slashFo;        /* /Fo              /Fo"FILEPATH"                                               */
        //
        virtual tstring GetName() {return s_cl;}
        //
        virtual errno_t Execute(Tool *parent_tool = nullptr);
        //
        virtual tstring GetDescription() {return s_cl_description;}
        //
        virtual Tool* GetToolByName(const tstring &name);
        // Cl specific functions ///////////////////////////////////////////
        //
        // these options go to *.opt file which is joined into *.fatobj 
        // further (after spliting by fatobj -split) these options is used by aalink for compiling device code by cl
        virtual void AddDeviceCodeAffectingOption(const tstring &option);
        //
        errno_t CreateOptFile(tstring s_file_path, unsigned int cout_type = AACL_MESSAGE);
        //
        errno_t ReadOptFile(tstring s_file_path, unsigned int cout_type = AACL_MESSAGE);
        //
        virtual bool IsToolName(const tstring &str);
        // gets the name of obj file specified in /Fo option
        // if b_produce_default_name_if_empty = true then produces default filename
        tstring GetObjFileName(Tool *parent_tool, bool b_produce_default_name_if_empty = false);
        // gets the name of obj file specified in /Fo option
        tstring GetObjFileNameMappedForCl(Tool *parent_tool);
        
    protected:
        // options common for both host code: aacl->cl and device code: aalink->cl invocations
        tstring s_device_code_affecting_options;
        //
        Cl() 
        {
            b_remove_output_files_from_options = true;
        }
        //
        friend class aa::Singleton_tool<Cl>;
        //
        virtual ~Cl() {}
        //
        virtual errno_t Where(tstring &abs_file_name);
        //
        virtual void BundleAllOptions();
    };

    // class for link
    class Link : public aa::Singleton_tool<Link>, public aa::Driver
    {
    protected:
        //
        bool b_link_option_added;
        //
        Link() {b_link_option_added = false;}
        //
        friend class aa::Singleton_tool<Link>;
        //
        virtual ~Link() {}
        //
        virtual errno_t Where(tstring &abs_file_name);
        //
        virtual void BundleAllOptions();

    public:
        //
        tstring slashOUT;
        //
        virtual tstring GetName() {return s_link;}
        //
        virtual errno_t Execute(Tool *parent_tool = nullptr);
        //
        virtual tstring GetDescription() {return s_link_description;}
        //
        virtual Tool* GetToolByName(const tstring &name);
        //
        virtual bool IsToolName(const tstring &str);
        //
        bool IsLinkOptionAdded() {return b_link_option_added;}
        // /link
        void AddLinkOption();
        // /MACHINE:x86 (x64)
        void AddMachineOption();
        // /IMPLIB:<*.lib>
        errno_t AddImplibOption(Tool *parent_tool);
    };
    // class for inflate
    class Inflate : public aa::Singleton_tool<Inflate>, public aa::Tool
    {
    protected:
        //
        Inflate() 
        {
            b_remove_includes_from_diagnostics = true;
            b_remove_input_files_from_options = true;
            b_remove_output_files_from_options = true;
        }
        //
        friend class aa::Singleton_tool<Inflate>;
        //
        virtual ~Inflate() {}
    public:
        //
        virtual tstring GetName() {return s_inflate;}
        //
        virtual errno_t Execute(Tool *parent_tool = nullptr);
    };
    // class for fatobj
    class Fatobj : public aa::Singleton_tool<Fatobj>, public aa::Tool
    {
    protected:
        //
        Fatobj() 
        {
            b_remove_includes_from_diagnostics = true;
            b_remove_output_files_from_options = true;
            b_remove_input_files_from_options = true;
        }
        //
        friend class aa::Singleton_tool<Fatobj>;
        //
        virtual ~Fatobj() {}

    public:
        //
        virtual tstring GetName() {return s_fatobj;}
        //
        virtual errno_t Execute(Tool *parent_tool = nullptr);
        //
        virtual errno_t DetectType(Tool *parent_tool, tstring s_filename);
    };
    // class for devlink
    class Devlink : public aa::Singleton_tool<Devlink>, public aa::Tool
    {
    protected:
        //
        Devlink() 
        {
            b_remove_includes_from_diagnostics = true;
            b_remove_output_files_from_options = true;
            b_obif = true;
        }
        //
        friend class aa::Singleton_tool<Devlink>;
        friend class Aalink;
        //
        virtual ~Devlink() {}
        //
        std::list<tstring> additional_brig_files;
        //
        std::list<tstring> additional_brig_files_in_abs_paths; 
        //
        tstring s_brig_files;

    public:
        //
        bool b_obif;
        //
        virtual void BundleAllOptions();
        //
        virtual tstring GetName() {return aa::s_devlink;}
        //
        virtual errno_t Execute(Tool *parent_tool = nullptr);
        //
        void AddBrigFile(const tstring &s_file) {additional_brig_files.push_back(s_file);}
    };

    // Replaces in s_source all occurences of s1 with s2
    tstring ReplaceMultiple(const tstring &s_source, const tstring &s1, const tstring &s2)
    {
        if (0 == s_source.size())
            return s_source;
        size_t s1_size = s1.size();
        if (0 == s1_size)
            return s_source;
        tstring s_dest = s_source;
        size_t iFind = 0;
        while (tstring::npos != iFind)
        {
            iFind = s_dest.find(s1);
            if (tstring::npos != iFind)
                s_dest.replace(iFind, s1_size, s2);
        }
        return s_dest;
    }
    //
    size_t GetCharsCount(const tstring &s_source, tchar ch1)
    {
        if (0 == s_source.size())
            return 0;
        size_t iFind = 0;
        size_t iFind_prev = 0;
        size_t iCount = 0;
        while (tstring::npos != iFind)
        {
            iFind = s_source.find(ch1, iFind_prev);
            if (tstring::npos != iFind)
            {
                iCount++;
                iFind_prev = iFind+1;
            }
        }
        return iCount;
    }
    // FUNCTIONS
    //
    tstring Unquote(const tstring &str, bool b_all = true)
    {
        if (str.size() < 2)
            return str;
        if (b_all)
            return ReplaceMultiple(str, s_quote, s_empty);
        if ((ch_quote == str[0]) && (ch_quote == str[str.size()-1]))
            return str.substr(1, str.size()-1);
        return str;
    }
    // Quote anyway (with implicit unquoting) if b_even_if_quoted_somehow = true
    tstring Quote(const tstring &str, const tchar &quote_char = ch_quote, bool b_even_if_quoted_somehow = true)
    {
        if (!b_even_if_quoted_somehow)
        {
            size_t iCount = GetCharsCount(str, quote_char);
            iCount = 0;
            if (str.size() > 1)
            {
                switch (iCount)
                {
                    case 0:
                        return quote_char + str + quote_char;
                    case 1:
                        if (ch_quote == str[0])
                            return str + quote_char;
                        if (ch_quote == str[str.size()-1])
                            return quote_char + str;
                    default:    
                        return str;
                }
            }
        }
        return quote_char + Unquote(str) + quote_char;
    }
    // TO_DECIDE: if quotes found inside and spaces outside of quotes is it needed to "expand" quotes to its' ends.
    tstring QuoteIfSpaces(const tstring &str)
    {
        if (tstring::npos == str.find(s_space))
            return str;
        else
        {
            size_t iCount = GetCharsCount(str, ch_quote);
            if (0 == iCount)
                return s_quote + str + s_quote;
            size_t iFirstSpace = str.find(ch_space);
            size_t iLastSpace = str.rfind(ch_space);
            bool b_1_Space = false;
            if (iFirstSpace == iLastSpace)
                b_1_Space = true;
            size_t iFirstQuote = str.find(ch_quote);
            size_t iLastQuote = str.rfind(ch_quote);
            bool b_1_Quote = false;
            if (iFirstQuote == iLastQuote)
                b_1_Quote = true;
            bool bAddFirstQuote = false;
            if (iFirstSpace < iFirstQuote)
                bAddFirstQuote = true;
            bool bAddLastQuote = false;
            if (iLastSpace > iLastQuote)
                bAddLastQuote = true;
            tstring s = str;
            bool bDelLastQuote = false;
            bool bDelFirstQuote = false;
            bool bDelBothQuotes = false;
            if (bAddFirstQuote)
            {
                if (bAddLastQuote)
                {
                    if (b_1_Quote)
                        bDelFirstQuote = true;
                    else
                        bDelBothQuotes = true;
                }
                else
                {
                    if (!b_1_Quote)
                        bDelFirstQuote = true;
                }
            }
            // !bAddFirst & bAddLast
            else if (bAddLastQuote)
            {
                if (!b_1_Quote)
                    bDelLastQuote = true;
            }
            // deleting of Quotes (if needed)
            if (bDelFirstQuote)
                s.replace(iFirstQuote, 1, s_empty);
            else if (bDelLastQuote)
                s.replace(iLastQuote, 1, s_empty);
            else if (bDelBothQuotes)
            {
                s.replace(iFirstQuote, 1, s_empty);
                s.replace(iLastQuote-1, 1, s_empty);
            }   
            // adding Quotes at the begining or|and at the end (if needed)
            if (bAddFirstQuote)
                s = s_quote + s;
            if (bAddLastQuote)
                s = s + s_quote;
        }
        return str;
    }
    //
    tstring Space(const tstring &str)
    {
        return s_space + str + s_space;
    }
    //
    tstring Squote(const tstring &str)
    {
        return s_squote + str + s_squote;
    }
    //
    tstring Trim(const tstring &str, const tstring &ws = _T(" \t"))
    {
        const size_t beginStr = str.find_first_not_of(ws);
        if (beginStr == tstring::npos)
        {
            // no content
            return s_empty;
        }
        const size_t endStr = str.find_last_not_of(ws);
        const size_t range = endStr - beginStr + 1;
        return str.substr(beginStr, range);
    }
    //
    tstring TrimLeft(const tstring &str, const tstring &ws = _T(" \t"))
    {
        
        const size_t beginStr = str.find_first_not_of(ws);
        if (tstring::npos == beginStr)
        {
            // no content
            return s_empty;
        }
        if (0 == beginStr)
            return str;
        // if not in the beginning
        if (beginStr > ws.size())
            return str;
        const size_t range = str.size() - beginStr + 1;
        return str.substr(beginStr, range);
    }
    //
    tstring SpaceBefore(const tstring &str)
    {
        return s_space + str;
    }

    tstring SpaceAfter(const tstring &str)
    {
        return str + s_space;
    }

    tstring Underscope(const tstring &str)
    {
        return s_underscope + str;
    }

    tstring Dash(const tstring &str)
    {
        return s_dash + str;
    }

    tstring DDash(const tstring &str)
    {
        return s_dash + s_dash + str;
    }

    tstring Slash(const tstring &str)
    {
        return s_slash + str;
    }

    tstring BackslashIfNeeded(const tstring &str)
    {
        if (str.empty())
            return str;
        size_t i_last = str.size()-1;        
        if (ch_backslash == str[i_last])
        {
            if (i_last > 0)
            {
                int i_bs_count = 1;
                for (size_t i=i_last-1; i >= 0; --i)
                {
                    if (ch_backslash == str[i])
                        i_bs_count++;
                    else
                        break;
                }
                if (i_bs_count % 2 != 0)
                    return str + s_backslash;
            }
            else
                return str + s_backslash;
        }
        return str;
    }

    void RemoveLastSlashes(tstring &str)
    {
        if (str.empty())
            return;
        size_t i_last = str.size()-1;        
        while ((ch_backslash == str[i_last]) || (ch_slash == str[i_last]))
        {
            str = str.substr(0, i_last);
            i_last = str.size()-1;
        }
    }
    //
    void LowerCase(tstring &str)
    {
        transform(str.begin(), str.end(), str.begin(), tolower);
    }
    //
    tstring ToLowerCase(const tstring &str)
    {
        tstring ret_str = str;
        LowerCase(ret_str);
        return ret_str;
    }
    //
    void UpperCase(tstring &str)
    {
        transform(str.begin(), str.end(), str.begin(), toupper);
    }
    //
    tstring ToUpperCase(const tstring &str)
    {
        tstring ret_str = str;
        UpperCase(ret_str);
        return ret_str;
    }
    //
    bool IsCompilerInputFileExtension(const tstring &ext)
    {
        tstring ex = ext;
        LowerCase(ex);
        if (s_c_ext == ex)
            return true;
        if (s_cpp_ext == ex)
            return true;
        if (s_cc_ext == ex)
            return true;
        if (s_cxx_ext == ex)
            return true;    
        return false;
    }
    //
    void Cout_tool_name()
    {
        HANDLE hOut = 0;
//        int b_is_console = _isatty(_fileno(stdout));
        if (IsMightBeColored())
        {
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hOut, FOREGROUND_INTENSITY);
        }
        tcout << std::endl << s_tmp_prefix + s_space + p_current_driver->GetName() + s_space + p_current_driver->GetVersion() + s_space + s_tmp_prefix << std::endl; 
        if (IsMightBeColored())
            SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
    //
    errno_t CoutError(tstring serror, tstring serror_content = s_empty, unsigned int cout_type = AACL_ERROR)
    {        
        if ((s_empty == serror) && (s_empty == serror_content))
            return EXIT_SUCCESS;
        if ((AA_EXIT_ERROR_PRINTED == cout_type) || (AACL_NOANYMESSAGE == cout_type))
            return AA_EXIT_ERROR_PRINTED;
        tstring s_level;
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (IsMightBeColored())
            SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        Cout_tool_name();
        switch (cout_type)
        {
            case AACL_ERROR:
                s_level = s_error;
                if (IsMightBeColored())
                    SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_INTENSITY);
                break;
            case AACL_WARNING:
                s_level = s_warning;
                if (IsMightBeColored())
                    SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                break;
            case AACL_MESSAGE:
                s_level = s_message;
                if (IsMightBeColored())
                    SetConsoleTextAttribute(hOut, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                break;
            default:
                return AA_EXIT_ERROR_PRINTED;
        }
        tcout << s_level;
        if (!serror.empty())
        {
            serror = SpaceAfter(serror);
            tcout << serror;
        }
        if (!serror_content.empty())
            tcout << Quote(serror_content) << std::endl;
        else
            tcout << std::endl;
        if (IsMightBeColored())
            SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        return AA_EXIT_ERROR_PRINTED;
    }

    tstring GetDWORDErrorDescription(DWORD err_no)
    {
        void* lpMsgBuf = NULL;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER 
                       | FORMAT_MESSAGE_FROM_SYSTEM 
                       | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      err_no,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (tchar*) &lpMsgBuf,
                      0,
                      NULL);
        tchar* lpWideCharStr = reinterpret_cast<tchar*>(lpMsgBuf);    
        tstring sErr(lpWideCharStr);
        LocalFree((void*)lpWideCharStr);
        return sErr;
    }
    // 
    tstring GetErrorDescription(errno_t err_no)
    {
        if (AA_EXIT_ERROR_PRINTED == err_no)
            return s_empty;
        tchar buf[AA_MAX_BUF_SIZE];
        // the length of the longest error text is 72
        // http://msdn.microsoft.com/en-us/library/t3ayayh1(v=vs.110).aspx
        if (tstrerror_s(buf, AA_MAX_BUF_SIZE, err_no) != 0)
            return s_Unknown_error;
        return tstring(buf);
    }
    //
    errno_t CoutError(errno_t err_no, tstring serror_content = s_empty, unsigned int cout_type = AACL_ERROR)
    {
        
        if ((AA_EXIT_ERROR_PRINTED == err_no) || (EXIT_SUCCESS == err_no))
            return err_no;
        return CoutError(GetErrorDescription(err_no), serror_content, cout_type);
    }
    //
    errno_t CoutDWORDError(DWORD err_no, tstring serror_content = s_empty)
    {
        if ((AA_EXIT_ERROR_PRINTED == err_no) || (EXIT_SUCCESS == err_no))
            return err_no;
        return CoutError(GetDWORDErrorDescription(err_no), serror_content);
    }
    //
    BOM_TYPE CheckBOM(tstring &s_filename, unsigned int cout_type = AACL_ERROR)
    {
        char buf[4];
        std::ifstream file(s_filename, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            CoutError(-1, s_filename, cout_type);
            return BOM_NO;
        }
        file.read(buf, 4);
        std::streamsize bytes = file.gcount();
        unsigned char b0 = 0;
        unsigned char b1 = 0;
        unsigned char b2 = 0;
        unsigned char b3 = 0;
        if (bytes > 1)
        {
            b0 = (unsigned char)buf[0];
            b1 = (unsigned char)buf[1];
            if (0xFF == b0 && 0xFE == b1)
            {
                if (bytes > 3)
                {
                    b2 = (unsigned char)buf[2];
                    b3 = (unsigned char)buf[3];
                    if (0x00 == b2 && 0x00 == b3)
                        // UTF-32 (LE) (FF FE 00 00)
                        return BOM_UTF_32_LE;
                }
                // UTF-16 (LE) (FF FE)
                return BOM_UTF_16_LE;
            }
            else if (0xFE == b0 && 0xFF == b1)
            {
                // UTF-16 (BE) (FE FF)
                return BOM_UTF_16_BE;
            }
        }
        if (bytes > 2)
        {
            b2 = (unsigned char)buf[2];
            if (0xEF == b0 && 0xBB == b1 && 0xBF == b2) 
                // UTF-8 (EF BB BF)
                return BOM_UTF_8;
        }
        if (bytes > 3)
        {
            b3 = (unsigned char)buf[3];
            if (0x00 == b0 && 0x00 == b1 && 0xFE == b2 && 0xFF == b3) 
                // UTF-32 (BE) (00 00 FE FF)
                return BOM_UTF_32_BE;
        }
        return BOM_NO;
    }
    //
    DWORD GetExecutablePath(tstring &exe_path) 
    {
        std::vector<tchar> executablePath(AA_MAX_BUF_SIZE);
        // Try to get the executable path with a buffer of MAX_PATH characters.
        DWORD dwRet = GetModuleFileName(nullptr, &executablePath[0], static_cast<DWORD>(executablePath.size()));
        // As long the function returns the buffer size, it is indicating that the buffer
        // was too small. Keep enlarging the buffer by a factor of 2 until it fits.
        while (dwRet == executablePath.size())     
        {
            executablePath.resize(executablePath.size() * 2);
            dwRet = GetModuleFileName(nullptr, &executablePath[0], static_cast<DWORD>(executablePath.size()));
        }
        // If the function returned 0, something went wrong
        if (0 == dwRet)
            return CoutDWORDError(GetLastError());   
        // We've got the path, construct a standard string from it
        exe_path = tstring(executablePath.begin(), executablePath.begin() + dwRet);
        return dwRet;
    }
    //
    tstring GetFullPath(const tstring &path)
    {
        tchar abs_path[_MAX_PATH];
        if (t_fullpath(abs_path, path.c_str(), _MAX_PATH))
            return tstring(abs_path);
        return s_empty;
    }
    //
    bool IsAbsolutePath(const tstring &path)
    {
        tstring full_path = GetFullPath(path);
        if (full_path != path)
            if (full_path.size() != path.size())
                return false;
        return true;
    }
    //
    bool IsFinishedWithSlashOrBackslash(const tstring &path)
    {
       const size_t path_size = path.size();
        if (0 == path_size)
            return false;
        if (path_size-1 == path.rfind(ch_slash))
            return true;
        if (path_size-1 == path.rfind(ch_backslash))
            return true;
        return false;
    }
    // gets drive, path (dir) with slashes/backslashes, file name only w/o path and extension, extension with leading dot
    errno_t SplitPath(const tstring &sPath, tstring sDrive, tstring sDir, tstring sFileNameOnly, tstring sExt, bool b_PathToFullPath = true)
    {
        tchar ch_drive[_MAX_DRIVE];
        tchar ch_dir[_MAX_DIR];
        tchar ch_file[_MAX_FNAME];
        tchar ch_ext[_MAX_EXT];
        tstring s_path = sPath; 
        if (b_PathToFullPath)
            s_path = GetFullPath(sPath);
        errno_t iRet = t_splitpath_s(Unquote(s_path).c_str(), 
                                     ch_drive, _MAX_DRIVE,
                                     ch_dir, _MAX_DIR,
                                     ch_file, _MAX_FNAME,
                                     ch_ext, _MAX_EXT);
        if (EXIT_SUCCESS != iRet)
            return CoutError(iRet);
        return iRet;
    }
    //
    tstring GetFileNameOnly(const tstring &path)
    {
        if (0 == path.size())
            return s_empty;
        tchar ch_file[_MAX_FNAME];
        errno_t iRet = t_splitpath_s(Unquote(path).c_str(), 
                                     nullptr, 0,
                                     nullptr, 0,
                                     ch_file, _MAX_FNAME,
                                     nullptr, 0);
        if (EXIT_SUCCESS != iRet)
            return s_empty;
        return tstring(ch_file);
    }
    //
    tstring GetFileNameWithExtension(const tstring &path)
    {
        if (0 == path.size())
            return s_empty;
        tchar ch_file[_MAX_FNAME];
        tchar ch_ext[_MAX_EXT];
        errno_t iRet = t_splitpath_s(Unquote(path).c_str(), 
                                     nullptr, 0,
                                     nullptr, 0,
                                     ch_file, _MAX_FNAME,
                                     ch_ext, _MAX_EXT);
        if (EXIT_SUCCESS != iRet)
            return s_empty;
        return tstring(ch_file) + tstring(ch_ext);
    }
    //
    tstring GetExtension(const tstring &path, bool b_with_point = true)
    {
        tstring ext = path;
        ext = Unquote(ext);
        size_t iFind = ext.rfind(s_dot);
        if (tstring::npos != iFind)
        {
            if (!b_with_point)
                iFind++;
            size_t path_size = ext.size();
            if (iFind <= path_size + 1)
            ext = ext.substr(iFind, path_size - iFind);
            return ext;
        }
        return s_empty;
    }
    //
    tstring GetPathWitoutFileName(const tstring &path, bool b_without_last_slash = true)
    {
        if (0 == path.size())
            return s_empty;
        tchar ch_dir[_MAX_DIR];
        tchar ch_drive[_MAX_DRIVE];
        errno_t iRet = t_splitpath_s(path.c_str(), 
                                     ch_drive, _MAX_DRIVE,
                                     ch_dir, _MAX_DIR,
                                     nullptr, 0,
                                     nullptr, 0);
        if (EXIT_SUCCESS != iRet)
            return s_empty;
        tstring s_path = tstring(ch_drive) + tstring(ch_dir);
        if (b_without_last_slash)
        {
            size_t iFind = 0;
            while (tstring::npos != iFind)
            {
                iFind = s_path.rfind(s_slash);
                if (iFind != s_path.size()-1)
                    iFind = s_path.rfind(s_backslash);
                if (iFind != s_path.size()-1)
                    break;
                s_path = s_path.substr(0, iFind);
            }
        }
        return s_path;
    }
    //
    tstring GetPathWitoutExtension(const tstring &path)
    {
        tstring path_wo_ext = path;
        tstring s_ext = GetExtension(path);
        if (s_ext.empty())
            return path_wo_ext;
        size_t iFind = path_wo_ext.rfind(s_ext);
        if (tstring::npos != iFind)
        {            
            path_wo_ext.replace(path_wo_ext.begin() + iFind, path_wo_ext.begin() + iFind + s_ext.size(), s_empty);
        }
        return path_wo_ext;
    }
    // STL GetCurrentDirectory wrapper
    errno_t GetCurDir(tstring & s_cur_dir)
    {
        tchar curDir[AA_MAX_BUF_SIZE];
        DWORD dwRet = GetCurrentDirectory(AA_MAX_BUF_SIZE, curDir);
        // dwRet contains the length of curDir, otherwise look at the LastError
        if (0 == dwRet)
            return CoutDWORDError(GetLastError());
        s_cur_dir = tstring(curDir);
        return EXIT_SUCCESS;
    }
    // Checks the Path (file or dir) whatever it exists and access is allowed
    errno_t CheckPath(const tstring & s_path, unsigned int cout_type = AACL_ERROR)
    {
        return CoutError(t_access_s(Unquote(s_path).c_str(), 0), s_path, cout_type);
    }
    // Checks the Path which is filename (whatever absolute or relative)
    errno_t CheckFile(const tstring & s_path, unsigned int cout_type = AACL_ERROR)
    {
        return CheckPath(GetFullPath(s_path), cout_type);  
    }

    tchar** CommandLineToArgv_my(tchar *CmdLine, int *_argc)
    {   
        tchar** argv;
        tchar*  _argv;
        ULONG   len;
        ULONG   argc;
        tchar   a;
        ULONG   i, j;
        BOOLEAN  in_QM;
        BOOLEAN  in_TEXT;
        BOOLEAN  in_SPACE;
        BOOLEAN in_QM_in_TEXT;
        len = ULONG(_tcsclen(CmdLine));
        i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);
        argv = (tchar**)GlobalAlloc(GMEM_FIXED, i + (len+2)*sizeof(tchar));
        _argv = (tchar*)(((PUCHAR)argv)+i);
        argc = 0;
        argv[argc] = _argv;
        in_QM = FALSE;
        in_QM_in_TEXT = FALSE;
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        i = 0;
        j = 0;
        while(i<len) 
        {
            a = CmdLine[i];
            if(in_QM) 
            {
                if(a == _T('\"')) 
                {
                    in_QM = FALSE;
                    if (in_QM_in_TEXT)
                    {
                        in_QM_in_TEXT = FALSE;
                        _argv[j] = a;
                        j++;
                    }
                } 
                else 
                {
                    _argv[j] = a;
                    j++;
                }
            } 
            else 
            {
                switch(a) 
                {
                case _T('\"'):
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if(in_SPACE) 
                    {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    else
                    {
                        in_QM_in_TEXT = TRUE;
                        _argv[j] = a;
                        j++;
                    }
                    in_SPACE = FALSE;
                    break;
                case _T(' '):
                case _T('\t'):
                case _T('\n'):
                case _T('\r'):
                    if(in_TEXT) 
                    {
                        _argv[j] = _T('\0');
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if(in_SPACE) 
                    {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
                }
            }
            i++;
        }
        _argv[j] = _T('\0');
        argv[argc] = NULL;
        (*_argc) = argc;
        return (tchar**)argv;
    }
    // Returns the environment variable value env_val if it exists and EXIT_SUCCESS, 
    // otherwise returns empty env_val and error code 
    errno_t GetEnv(const tstring env_var, tstring &env_val, unsigned int cout_type = AACL_ERROR)
    {    
        tchar* val = 0;
        size_t requiredSize = 0;
        tgetenv_s(&requiredSize, NULL, 0, env_var.c_str());
        if (0 == requiredSize)
            return CoutError(s_Env_var + Space(Quote(env_var)) + s_doesnt_exist, s_empty, cout_type);
        val = (tchar*)malloc(requiredSize * sizeof(tchar));
        if (!val)
            return CoutError(s_Failed_to_allocate_memory, s_empty, cout_type);
        // Get the value of the env_var environment variable.
        errno_t iRet = tgetenv_s(&requiredSize, val, requiredSize, env_var.c_str());
        if (EXIT_SUCCESS == iRet)
        {
            env_val = tstring(val);
            return EXIT_SUCCESS;
        }
        return CoutError(iRet, s_empty, cout_type);
    }
    //
    errno_t SetEnv(const tstring &env_var, const tstring &env_val, unsigned int cout_type = AACL_ERROR)
    {
        errno_t iRet = tputenv_s(env_var.c_str(), env_val.c_str());
        if (EXIT_SUCCESS == iRet)
            return iRet;
        return CoutError(iRet, s_empty, cout_type);
    }
    // TODO_HSA: finish and test the function
    // bPath: if env var to check contains path(s)
    // bOccurence: if occurrence of env_val in env var is checked
    bool CheckEnv(const tstring &env_var, const tstring &env_val, bool bPath = true, bool bOccurrence = true, unsigned int cout_type = AACL_ERROR)
    {
        errno_t iRet = EXIT_SUCCESS;
        // TODO_HSA: just finish the function
        bOccurrence = false;
        // //
        tstring s_val_to_check = env_val;
        tstring s_val;
        iRet = GetEnv(env_var, s_val, cout_type);
        if (EXIT_SUCCESS != iRet)
            return false;
        if (bPath)
        {
            LowerCase(s_val_to_check);
            s_val_to_check = GetFullPath(s_val_to_check);
            RemoveLastSlashes(s_val_to_check);
            size_t iFind = 0;
            size_t iPrevFind = 0;
            size_t i_size = s_val.size();
            tstring s_path;
            while (tstring::npos != iFind)
            {
                iFind = s_val.find(s_semicolon);
                if (tstring::npos != iFind)
                {
                    s_path = s_val.substr(0, iFind);
                    s_val = s_val.substr(iFind+1, i_size-iFind-1);
                }
                // Check
                if (s_path.empty())
                    continue;
                LowerCase(s_path);
                s_path = GetFullPath(s_path);
                RemoveLastSlashes(s_path);
                if (s_path == s_val_to_check)
                    return true;
                iPrevFind = iFind;
            }
        }
        return false;
    }
    // read text file to string provided
    errno_t ReadFileToString(tstring s_filename, tstring &s_output_string, unsigned int cout_type = AACL_ERROR)
    {
        s_output_string.clear();
        // open as a byte stream
        BOM_TYPE iBOM = CheckBOM(s_filename);
        tfstream fin(s_filename, std::ios::in | std::ios::binary);
        if (!fin.is_open())
            return CoutError(-1, s_filename, cout_type);
        tstringstream ss;
        ss.imbue(std::locale(ss.getloc(), new std::codecvt_utf8<wchar_t>));
        // apply BOM-sensitive translation
        switch (iBOM)
        {
            case BOM_UTF_8:
                fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf8<wchar_t>));
                break;
            case BOM_UTF_16_BE:
                fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf16<wchar_t>));
                break;
            case BOM_UTF_16_LE:
                fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>));
                break;
        }
        tchar tch;
        tstring tline;
        // remove BOM (1 wchar_t symbol, even for UTF-8 after translation by imbue)
        if (iBOM != BOM_NO)
            fin.get(tch);
        while (getline(fin, tline))
            ss << tline;
        fin.close();
        s_output_string = ss.str().c_str();
        return EXIT_SUCCESS;
    }
    // Writes file even if input string is empty (because *.opt file should always exist)
    errno_t WriteFileFromString(tstring s_filename, tstring &s_input_string, unsigned int cout_type = AACL_ERROR)
    {
        tofstream fout(s_filename, std::ios::out | std::ios::binary);
        if (!fout.is_open())
            return CoutError(-1, s_filename, cout_type);
        fout.imbue(std::locale(fout.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
        fout << s_input_string;
        fout.close();
        return EXIT_SUCCESS;
    }
    //
    void CoutReturnCode(tstring toolname, errno_t iret)
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (IsMightBeColored())
        {
            if (0 == iret)
                SetConsoleTextAttribute(hOut, FOREGROUND_GREEN);
            else
                SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_INTENSITY);
        }
        tcout << std::endl << toolname << s_exe_ext << Space(s_return_code) << iret << std::endl; 
        if (IsMightBeColored())
            SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
    //
    void CoutToolName(tstring toolname)
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (IsMightBeColored())
            SetConsoleTextAttribute(hOut, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        tcout << toolname << s_exe_ext << s_colon << std::endl;
        if (IsMightBeColored())
            SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
    // 1. Searches for "AMD_CPP_COMPILER01" env. var only with first call
    // 2. Searces for "AMDAPPSDKROOT" if AMD_CPP_COMPILER01 is missing or files were not not found 
    // 3. Uses path_to_exe
    // and stores it in path_to_aa_bin 
    // If env var is wrong or absent then suppose the current dir (aacl.exe dir)
    errno_t GetAMD_bin(tstring& amd_bin_abs_path)
    {
        errno_t iRet = EXIT_SUCCESS;
        if (!path_to_aa_bin.empty())
        {
            amd_bin_abs_path = path_to_aa_bin;
            return CheckPath(amd_bin_abs_path);
        }
        else
        {
            iRet = GetEnv(s_amd_env_var, amd_bin_abs_path, AACL_NOANYMESSAGE);
        }
        // if not found or wrong, suppose the Cur Dir (aacl.exe dir)
        if (EXIT_SUCCESS != iRet)
        {
            amd_bin_abs_path = path_to_exe;
            iRet = CheckPath(path_to_exe, true);
        }
        if (EXIT_SUCCESS != iRet)
        {
            amd_bin_abs_path = s_empty;
            return CoutError(iRet, s_amd_env_var);
        }
        return CheckPath(amd_bin_abs_path);
    }
    // ?? if include is always beside bin dir?
    errno_t GetAMD_include(tstring & amd_include_abs_path)
    {
        errno_t iRet = GetAMD_bin(amd_include_abs_path);
        if (EXIT_SUCCESS != iRet)
        {
            amd_include_abs_path = s_empty;
            return CoutError(iRet, amd_include_abs_path);
        }
        // ugly hack to get include path from bin path
        // The rule: bin & include on the same level
        size_t aacl_pos = amd_include_abs_path.rfind(s_amd_bin_dir_name);
        if (tstring::npos != aacl_pos)
        {
            amd_include_abs_path = amd_include_abs_path.substr(0, aacl_pos);
            amd_include_abs_path += s_amd_include_dir_name;
        }
        else
            // wrong path
            return CoutError(s_Wrong_path, amd_include_abs_path);
        return CheckPath(amd_include_abs_path);
    }
    // ?? if lib is always beside bin dir?
    errno_t GetAMD_lib(tstring & amd_lib_abs_path)
    {
        errno_t iRet = GetAMD_bin(amd_lib_abs_path);
        if (EXIT_SUCCESS != iRet)
        {
            amd_lib_abs_path = s_empty;
            return CoutError(iRet, amd_lib_abs_path);
        }
        // ugly hack to get include path from bin path
        // The rule: bin & include on the same level
        // TO DECIDE: Do we need to set up the ENV VAR for AMD AMP LIB Path?
        size_t aacl_pos = amd_lib_abs_path.rfind(s_amd_bin_dir_name);
        if (aacl_pos > 0)
        {
            amd_lib_abs_path = amd_lib_abs_path.substr(0, aacl_pos);
            amd_lib_abs_path += s_lib + s_backslash;
            if (Is32bitMachine())
                amd_lib_abs_path += s_x86;
            else if (Is64bitMachine())
                amd_lib_abs_path += s_x86_64;
        }
        else
            // wrong path
            return CoutError(s_Wrong_path, amd_lib_abs_path);
        return CheckPath(amd_lib_abs_path);
    }
    // Executes Process existed by ExeFullPath with cmd parapeters Parameters
    errno_t ExecuteProcess(tstring ExeFullPath, tstring Parameters, size_t SecondsToWait = INFINITE) 
    { 
        errno_t iRet = EXIT_SUCCESS; 
        DWORD dwExitCode = 0; 
        tstring sTempStr = s_empty; 
        /* - NOTE - You should check here to see if the exe even exists */ 
        /* Add a space to the beginning of the Parameters */ 
        if (Parameters.size() != 0) 
            if (Parameters[0] != ch_space) 
                Parameters.insert(0, s_space); 
        /* The first parameter needs to be the exe itself */ 
        sTempStr = ExeFullPath; 
        Parameters = sTempStr.append(Parameters); 
         /* CreateProcessW can modify Parameters thus we allocate needed memory */ 
        tchar* pwszParam = new tchar[Parameters.size() + 1]; 
        if (!pwszParam) 
            return CoutError(s_Failed_to_allocate_memory);
        const tchar* pchrTemp = Parameters.c_str(); 

        iRet = _tcsncpy_s(pwszParam, Parameters.size() + 1, pchrTemp, Parameters.size());
        if (EXIT_SUCCESS != iRet)
            return CoutError(iRet);
        /* CreateProcess API initialization */ 
        STARTUPINFO siStartupInfo; 
        PROCESS_INFORMATION piProcessInfo; 
        memset(&siStartupInfo, 0, sizeof(siStartupInfo)); 
        memset(&piProcessInfo, 0, sizeof(piProcessInfo)); 
        siStartupInfo.cb = sizeof(siStartupInfo); 
        siStartupInfo.hStdOutput = g_hOut;
        siStartupInfo.hStdInput = g_hIn;
        siStartupInfo.hStdError  = g_hErr;
        siStartupInfo.dwFlags |= STARTF_USESTDHANDLES;
        // create child process with handles' inheritence
        if (FALSE != CreateProcess(NULL, 
                                    reinterpret_cast<TLPSTR>(pwszParam), // command line  
                                    nullptr,            // process security attributes 
                                    nullptr,            // primary thread security attributes 
                                    TRUE,               // handles are inherited 
                                    0,                  // creation flags 
                                    nullptr,            // use parent's environment 
                                    nullptr,            // use parent's current directory  
                                    &siStartupInfo,     // STARTUPINFO pointer 
                                    &piProcessInfo))    // receives PROCESS_INFORMATION 
        { 
            // Watch the process
            dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, DWORD(SecondsToWait)); 
            if (WAIT_OBJECT_0 == dwExitCode)
            {
                if (!GetExitCodeProcess(piProcessInfo.hProcess, &dwExitCode))
                {
                    /* Free memory */ 
                    if (pwszParam)
                    {
                        delete[]pwszParam; 
                        pwszParam = 0; 
                    }
                    // Waiting of process failed
                    return CoutDWORDError(dwExitCode);
                }
                iRet = int(dwExitCode);
            }
        } 
        // ERROR
        else 
        { 
            /* Free memory */ 
            if (pwszParam)
            {
                delete[]pwszParam; 
                pwszParam = 0; 
            }
            /* CreateProcess failed */ 
            return CoutDWORDError(GetLastError());
        } 
        /* Free memory */ 
        if (pwszParam)
        {
            delete[]pwszParam; 
            pwszParam = 0; 
        }
        /* Release handles */ 
        CloseHandle(piProcessInfo.hProcess); 
        CloseHandle(piProcessInfo.hThread); 
        return iRet; 
    } 
    //
    errno_t Driver::CheckAndAddIntermediateFile(const tstring &s_file, unsigned int cout_type)
    {
        errno_t iRet = CheckFile(s_file, cout_type);
        if (EXIT_SUCCESS != iRet)
            return CoutError(iRet, s_empty, cout_type);
        all_tmp_files.push_back(s_file);
        return iRet;
    }
    //
    errno_t Driver::DeleteIntermediateFiles()
    {
        errno_t iRet = EXIT_SUCCESS;
        // if -keeptmp do not perform deletion
        // if -keeptmp- perform deletion
        if (Dash(keeptmp_opt) == dashkeeptmp)
            return iRet;
        tstring s_curr_dir;
        iRet = GetCurDir(s_curr_dir);
        if (EXIT_SUCCESS != iRet)
            return CoutError(iRet);
        // Verify every file name (is it absolute or relative)
        std::list<tstring>::iterator l_i;
        tstring s_filename;
        tstringstream ss;
        size_t all_files_to_be_deleted = all_tmp_files.size();
        size_t all_files_being_deleted = 0;
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (2 == debugSetLevel)
        {
            Cout_tool_name();
            if (IsMightBeColored())
                SetConsoleTextAttribute(hOut, FOREGROUND_INTENSITY);
            ss << s_diagnostics << Space(s_deleted_files + s_colon) << std::endl << std::endl;
        }
        for (l_i = all_tmp_files.begin(); l_i != all_tmp_files.end(); ++l_i)
        {
            s_filename = *l_i;
            if (!IsAbsolutePath(s_filename))
            {
                s_filename = s_curr_dir + s_backslash + s_filename;
            }
            if (0 == CheckPath(s_filename))
            {
                const tchar* pchrTemp = s_filename.c_str(); 
                if (!DeleteFile(pchrTemp))
                    iRet = CoutDWORDError(GetLastError()); 
            else
                if (2 == debugSetLevel)
                {
                    ss << Quote(s_filename) << std::endl;
                    all_files_being_deleted++;
                }
            }
        }
        if (2 == debugSetLevel)
        {
            ss << all_files_being_deleted << Space(s_of) << all_files_to_be_deleted << Space(s_was_deleted_suc);
            tcout << ss.str() << std::endl << std::endl;
            if (IsMightBeColored())
                SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
        return iRet;
    }
    //
    bool Driver::VOption(const tstring &str, bool bPrint)
    {
        if (str.empty())
            return false;
        bool b_ok = false;
        size_t i_size = str.size();
        if (1 == str.find(V_opt))
        {
            if (i_size != V_opt.size()+1)
                return false;
            b_ok = true;
        }
        if ((b_ok) && (bPrint))
        {
            tcout << GetVersion() << std::endl;
            return true;
        }
        return false;
    }
    //
    void Tool::AddOption(const tstring &option, bool b_current_run_only_opt)
    {   
        if (b_current_run_only_opt)
        {
            s_current_run_options += SpaceBefore(Trim(option));
            s_current_run_options = Trim(s_current_run_options);
        }
        else
        {
            s_options += SpaceBefore(Trim(option));
            s_options = Trim(s_options);
        }
    }
    //
    void Tool::AddOutputFile(const tstring &option)
    {
        s_output_files += SpaceBefore(Trim(option));
    }
    //
    void Tool::AddInputFile(const tstring &option)
    {
        s_input_files += SpaceBefore(Trim(option));
    }
    //
    void Tool::AddIncludes(const tstring &option)
    {
        s_includes += SpaceBefore(Trim(option));
    }
    //
    void Tool::PrependIncludes(const tstring &option)
    {
        s_includes = SpaceAfter(Trim(option)) + s_includes;
    }
    // also adds that option to s_options IN PLACE
    void Tool::AddSpecificOption(const tstring &option)
    {
        s_specific_options += SpaceBefore(Trim(option));
        AddOption(option);
    }
    //
    void Tool::AddCompulsoryOption(const tstring &option)
    {
        s_compulsory_options += SpaceBefore(Trim(option));
    }
    // Add specific option and also add that option to s_options
    void Tool::AddHardcodedOption(const tstring &option)
    {
        s_hardcoded_options += SpaceBefore(Trim(option));
    }
    void Tool::BundleInputFiles()
    {
        if (!b_remove_input_files_from_options)
            if (!s_input_files.empty())
                s_all_options += SpaceBefore(s_input_files);
    }
    void Tool::BundleOutputFiles()
    {
        if (!b_remove_output_files_from_options)
            if (!s_output_files.empty())
                s_all_options += SpaceBefore(Trim(s_output_files));
    }
    //
    void Tool::BundleAllOptions()
    {
        s_hardcoded_options = Trim(s_hardcoded_options);
        s_compulsory_options = Trim(s_compulsory_options);
        s_defines = Trim(s_defines);
        s_optimizations = Trim(s_optimizations);
        s_debug_options = Trim(s_debug_options);
        s_options = Trim(s_options);
        s_input_files = Trim(s_input_files);
        s_output_files = Trim(s_output_files);
        if (!s_hardcoded_options.empty())
            s_all_options += SpaceBefore(s_hardcoded_options);
        if (!s_compulsory_options.empty())
            s_all_options += SpaceBefore(s_compulsory_options);
        if (!s_defines.empty())
            s_all_options += SpaceBefore(s_defines);
        if (!s_optimizations.empty())
            s_all_options += SpaceBefore(s_optimizations);
        if (!s_debug_options.empty())
            s_all_options += SpaceBefore(s_debug_options);
        if (!s_options.empty())
            s_all_options += SpaceBefore(s_options);
        if (b_add_parent_driver_includes)
        {
            if (b_sys_includes)
            {
                if (!p_current_driver->s_sys_includes.empty())
                    s_includes += SpaceBefore(Trim(p_current_driver->s_sys_includes));
            }
            else
            {
                if (!p_current_driver->s_includes.empty())
                    s_includes += SpaceBefore(Trim(p_current_driver->s_includes));
            }
        }
        if (!s_includes.empty())
            s_all_options += SpaceBefore(s_includes);
        if (!s_current_run_options.empty())
            s_all_options += SpaceBefore(s_current_run_options);
        // for most of tools
        if (!b_input_files_before_output_files) 
        {
            BundleOutputFiles();
            BundleInputFiles();
        }
        else
        {
            BundleInputFiles();
            BundleOutputFiles();
        }
        s_all_options = Trim(s_all_options);
    }
    //
    errno_t Tool::Where(tstring &abs_file_name)
    {
        errno_t iRet = GetAMD_bin(abs_file_name);
        if (EXIT_SUCCESS != iRet)
            return CoutError(iRet);
        if (Is32bitMachine())
            abs_file_name += s_backslash + GetName() + s_exe_ext;
        else if (Is64bitMachine())
            abs_file_name += _T("\\..\\") + s_x86_64 + s_backslash + GetName() + s_exe_ext;
        return CheckPath(abs_file_name, AACL_NOANYMESSAGE);
    }
    //
    errno_t Tool::FindExecutable()
    {
        errno_t iRet = Where(s_abs_path);
        if (EXIT_SUCCESS != iRet)
            return CoutError(GetName() + Space(s_wasnt_found)); 
        s_abs_path = Quote(s_abs_path);
        return EXIT_SUCCESS;
    }
    //
    void Tool::ClearAllOptions()
    {
        s_hardcoded_options.clear();
        s_includes.clear();
        s_options.clear();
        s_current_run_options.clear();
        s_input_files.clear();
        s_output_files.clear();
        s_all_options.clear();
    }
    //
    tstring Tool::GetOutputPath(Driver *parent_driver)
    {
        tstring s_out_dir;
        if (parent_driver) 
            s_out_dir = parent_driver->GetOutputPath();
        else
            return s_empty;
        tstring s_path_out = GetFullPath(s_out_dir);
        tstring s_curr_dir;
        GetCurDir(s_curr_dir);
        if (ToLowerCase(s_curr_dir) == ToLowerCase(s_path_out))
            return s_empty;
        if (s_out_dir.size() != 0)
            s_out_dir += s_backslash;
        return s_out_dir;
    }
    // tool diagnostics
    errno_t Tool::Diagnostics()
    {
        if (2 != debugSetLevel) 
            return EXIT_SUCCESS;
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (IsMightBeColored())
            SetConsoleTextAttribute(hOut, FOREGROUND_INTENSITY);
        tstring s_name = GetName();
        tcout << s_name << s_colon << std::endl;
        tstring s_temp;
        s_temp = Trim(s_hardcoded_options);
        tcout << s_name << aa::s_hardcoded_options << s_temp << std::endl;
        if (!b_remove_compulsory_options_from_diagnostics)
        {
            s_temp = Trim(s_compulsory_options);
            tcout << s_name << aa::s_compulsory_options << s_temp << std::endl;
        }
        s_temp = Trim(s_specific_options);
        tcout << s_name << aa::s_specific_options << s_temp << std::endl;
        if (!b_remove_ignored_options_from_diagnostics)
        {
            s_temp = Trim(s_ignored_options);
            tcout << s_name << aa::s_ignored_options << s_temp << std::endl;
        }
        if (!b_remove_includes_from_diagnostics)
        {
            s_temp = Trim(s_includes);
            tcout << s_name << aa::s_includes << s_temp << std::endl;
        }
        s_temp = Trim(s_input_files);
        tcout << s_name << aa::s_input_files << s_temp << std::endl;
        if (!b_remove_output_files_from_diagnostics)
        {
            s_temp = Trim(s_output_files);
            tcout << s_name << aa::s_output_files << s_temp << std::endl;
        }
        if (IsMightBeColored())
            SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        return EXIT_SUCCESS;
    }
    //
    errno_t Tool::Output(bool b_tool_diagnostics)
    {
        if (debugSetLevel) 
        {
            Cout_tool_name(); 
            if (2 == debugSetLevel)
            {
                if (b_tool_diagnostics)
                    Tool::Diagnostics();
                else
                    Diagnostics();
                tcout << std::endl;
            }
            CoutToolName(GetName());
            tcout << s_abs_path << s_space << s_all_options << std::endl; // << std::endl;
        }
        return EXIT_SUCCESS;
    }
    //
    errno_t Tool::ExecProcess()
    {
        return ExecuteProcess(s_abs_path, s_all_options);
    }
    errno_t Tool::ExecProcessAndExit(Driver *parent_driver)
    {
        BundleAllOptions();
        // output tool diagnostics (if debug2) and real command line
        errno_t iRet = Output();
        if (EXIT_SUCCESS != iRet)
            return iRet;
        iRet = ExecProcess();
        // Checking file existence
        errno_t i_ret_file_check = EXIT_SUCCESS;
        if (!s_output_file.empty())
            i_ret_file_check = parent_driver->CheckAndAddIntermediateFile(s_output_file);
        if ((EXIT_SUCCESS != iRet) || (EXIT_SUCCESS != i_ret_file_check))
        {
            CoutReturnCode(GetName(), iRet);
            errno_t i_ret = parent_driver->DeleteIntermediateFiles();
            if (EXIT_SUCCESS != i_ret)
                return CoutError(i_ret);
            return iRet;
        }
        if (debugSetLevel) 
            CoutReturnCode(GetName(), iRet);
        return iRet;
    }
    //
    void Driver::AddSysIncludes(const tstring &option)
    {
        s_sys_includes += SpaceBefore(Trim(option));
    }
    //
    errno_t Driver::Diagnostics()
    {
        if (2 != debugSetLevel) 
            return EXIT_SUCCESS;
        Cout_tool_name();
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (IsMightBeColored())
            SetConsoleTextAttribute(hOut, FOREGROUND_INTENSITY);
        tcout << s_diagnostics << s_colon << std::endl << std::endl;
        tcout << _T("Path to driver executable = ") << s_abs_path << std::endl;
        tcout << _T("Path to AMD AMP binaries  = ") << path_to_aa_bin << std::endl;
        tcout << _T("Path to AMD AMP includes  = ") << path_to_aa_include << std::endl;
        tcout << _T("Path to AMD AMP libraries = ") << path_to_aa_lib << std::endl;
        tcout << _T("Environment variables     = ") << s_all_env_vars << std::endl;
        tcout << std::endl;
        errno_t iRet = Tool::Diagnostics();
        if (IsMightBeColored())
            SetConsoleTextAttribute(hOut, FOREGROUND_INTENSITY);
        return iRet;
    }
    //
    void Driver::AddSpecificOptionForTool(const tstring &tool, const tstring &option)
    {
        if (tool.empty() || option.empty())
            return;
        Tool *p_tool = GetToolByName(tool);
        if (!p_tool)
            return;
        p_tool->AddSpecificOption(option);
    }
    //
    errno_t Driver::GetVSpath(tstring &env_val)
    {
        errno_t iRet = EXIT_SUCCESS;
        // Verify the Visual Studio version number: it should be >= 11
        unsigned int iVSVersion = 11;
        tstringstream ss;
        iVSVersion *= 10;
        tstring sVSXXXComnTools = s_VS;
        ss << iVSVersion;
        sVSXXXComnTools += ss.str() + s_COMNTOOLS;
        iRet = GetEnv(sVSXXXComnTools, env_val);
        if (EXIT_SUCCESS != iRet)
        {
            env_val = s_empty;
            return CoutError(iRet);
        }
        int iCount = 3;
        size_t iFind = env_val.rfind(s_backslash); 
        size_t iSize = env_val.size();
        if ((iSize == iFind) || (iSize == iFind+1))
            iCount = 3;
        else
            iCount = 2;
        // copying to print if error
        tstring s_path = env_val;
        for (int i=0; i < iCount; ++i)
        {
            iFind = env_val.rfind(s_backslash); 
            if (iFind > 0)
                env_val = env_val.substr(0, iFind);
            else
            {
                env_val = s_empty;
                // wrong path
                return CoutError(s_Wrong_path, s_path);
            }
        }
        return iRet;
    }
    // ugly huck from VS110COMNTOOLS env var
    errno_t Driver::GetVC_include_paths(std::list<tstring> &cl_include_list)
    {
        cl_include_list.clear();
        tstring sVS;
        errno_t iRet = GetVSpath(sVS);
        if (EXIT_SUCCESS != iRet)
            return CoutError(iRet);
        tstring sInc = sVS;
        sInc += s_backslash + s_VC + s_backslash + s_include + s_backslash;
        iRet = CheckPath(sInc);
        if (EXIT_SUCCESS == iRet)
            cl_include_list.push_back(sInc);
        return iRet;
    }
    //
    bool Driver::VersionOption(const tstring &str, bool bPrint)
    {
        if (str.empty())
            return false;
        bool b_ok = false;
        size_t i_size = str.size();
        if (1 == str.find(version_opt))
        {
            if (i_size != version_opt.size()+1)
                return false;
            b_ok = true;
        }
        if ((b_ok) && (bPrint))
        {
            tstring s_capacity;
            if (Is32bitMachine())
                s_capacity = s_x86;
            else if (Is64bitMachine())
                s_capacity = s_x64;
            s_capacity = SpaceBefore(s_capacity);
            tcout << GetDescription() << s_capacity << std::endl;
            tcout << s_space << s_space << s_AMD_copyrignt << std::endl;
            // linkage date time
            unsigned t = ((IMAGE_NT_HEADERS*)(((LPBYTE)(&__ImageBase))+__ImageBase.e_lfanew))->FileHeader.TimeDateStamp;
            // t: unix timestamp (UTC)
            unsigned __int64 ft = (unsigned __int64)t * 10000000 + 116444736000000000;
            // ft: 100-nanosecond intervals since January 1, 1601 (UTC)
            unsigned __int64 lft;
            BOOL rc = FileTimeToLocalFileTime((FILETIME*)&ft, (FILETIME*)&lft);
            if (0 == rc) 
            {
                CoutError(s_failed);
                return false;
            }
            // ft: 100-nanosecond intervals since January 1, 1601 (local)
            SYSTEMTIME st;
            rc = FileTimeToSystemTime((FILETIME*)&lft, &st);
            if (0 == rc) 
            {
                CoutError(s_failed);
                return false;
            }
            // st: time split into elements (local)
            tcout << s_space << Space(s_Built) << st.wYear << s_dash << st.wMonth << s_dash << st.wDay 
                << s_space << st.wHour << s_colon << st.wMinute << s_colon << st.wSecond << std::endl;
            tcout << s_space << s_space << s_Version << SpaceBefore(GetVersion()) << std::endl;
            return true;
        }
        return b_ok;
    }
    //
    errno_t Driver::SetVSIDEtoPath(unsigned int cout_type)
    {
        errno_t iRet = EXIT_SUCCESS;
        // getting VS110COMNTOOLS
        unsigned int iVSVersion = 11;
        tstringstream ss;
        iVSVersion *= 10;
        tstring sVSXXXComnTools = s_VS;
        ss << iVSVersion;
        sVSXXXComnTools += ss.str() + s_COMNTOOLS;
        tstring s_IDE_path;
        iRet = GetEnv(sVSXXXComnTools, s_IDE_path, cout_type);
        if (EXIT_SUCCESS != iRet)
            return CoutError(iRet, s_empty, cout_type);
        // setting to \\Tools\..\IDE
        s_IDE_path += _T("\\..\\") + s_IDE;
        // getting PATH
        tstring s_PATH_val;
        iRet = GetEnv(s_PATH, s_PATH_val, cout_type);
        if (EXIT_SUCCESS != iRet)
            return CoutError(iRet, s_empty, cout_type);
        // checking PATH: if it has the path to IDE
        if (!CheckEnv(s_PATH, s_IDE_path, true, true, AACL_WARNING))
        {
            CoutError(s_empty, _T("Path to \"Microsoft Visual Studio 11.0\\Common7\\IDE\" in PATH environment variable is absent! Microsoft link might crash!"), AACL_WARNING);
            // setting PATH with path to IDE before
            s_PATH_val = s_IDE_path + s_semicolon + s_PATH_val;
            iRet = SetEnv(s_PATH, s_PATH_val, cout_type);
            if (EXIT_SUCCESS == iRet)
            {
                tstringstream ss;
                ss << s_all_env_vars;
                if (!s_all_env_vars.empty())
                    ss << std::endl;
                ss << s_PATH << Space(_T("+=")) << s_IDE_path;
                s_all_env_vars += ss.str();
            }
        }
        return iRet;
    }
    // initialisation of tools
    Aalink      *aalink_tool    = Aalink::Instance();
    Devlink     *devlink        = Devlink::Instance();
    Inflate     *inflate        = Inflate::Instance();
    Fatobj      *fatobj         = Fatobj::Instance();
    Cl          *cl             = Cl::Instance();
    Link        *link           = Link::Instance();   
    //
    tstring Aalink::GetOutputPath()
    {
        if (link->slashOUT.empty())
        {
            tstring s_path;
            if (EXIT_SUCCESS == GetCurDir(s_path))
                return s_path;
        }
        tstring s_exe_file = Trim(link->slashOUT);
        s_exe_file = Unquote(s_exe_file.substr(OUT_opt.size()+1, link->slashOUT.size()-OUT_opt.size()-1));
        return GetPathWitoutFileName(s_exe_file);
    }
    //
    Tool* Aalink::GetToolByName(const tstring &name)
    {
        if (name.empty())
            return false;
        tstring name_lowed = ToLowerCase(name);
        if (ToLowerCase(fatobj->GetName()) == name_lowed)
            return fatobj;
        if (ToLowerCase(devlink->GetName()) == name_lowed)
            return devlink;
        if (ToLowerCase(inflate->GetName()) == name_lowed)
            return inflate;
        if (ToLowerCase(cl->GetName()) == name_lowed)
            return cl;
        if (ToLowerCase(link->GetName()) == name_lowed)
            return link;
        return nullptr;
    }
    // Ensure if the string is the name of the tool, called from aalink or not
    bool Aalink::IsToolName(const tstring &str)
    {
        if (str.empty())
            return false;
        tstring s_lowed = ToLowerCase(str);
        if (ToLowerCase(s_fatobj) == s_lowed)
            return true;
        else if (ToLowerCase(s_devlink) == s_lowed)
            return true;
        else if (ToLowerCase(s_inflate) == s_lowed)
            return true;
        else if (ToLowerCase(s_cl) == s_lowed)
            return true;
        else if (ToLowerCase(s_link) == s_lowed)
            return true;
        return false;
    }
    //
    void Cl::AddDeviceCodeAffectingOption(const tstring &option)
    {
        if ((Slash(c_opt) == Trim(option)) || (Dash(c_opt) == Trim(option)))
            return;
        s_device_code_affecting_options += SpaceBefore(Trim(option));
        // there is shouldn't be an /c option in *.opt file
        s_device_code_affecting_options = ReplaceMultiple(s_device_code_affecting_options, Space(Slash(c_opt)), s_space);
        s_device_code_affecting_options = ReplaceMultiple(s_device_code_affecting_options, Space(Dash(c_opt)), s_space);
        if ((0 == s_device_code_affecting_options.find(SpaceAfter(Slash(c_opt)))) ||
            (0 == s_device_code_affecting_options.find(SpaceAfter(Dash(c_opt)))))
            s_device_code_affecting_options.replace(0, 3, s_empty);
        size_t i_size = s_device_code_affecting_options.size();
        if ((i_size-3 == s_device_code_affecting_options.find(SpaceBefore(Slash(c_opt)))) ||
            (i_size-3 == s_device_code_affecting_options.find(SpaceBefore(Dash(c_opt)))))
            s_device_code_affecting_options.replace(i_size-3, 3, s_empty);
    }
    //
    errno_t Cl::CreateOptFile(tstring s_file_path, unsigned int cout_type)
    {
        return WriteFileFromString(s_file_path, s_device_code_affecting_options, cout_type);
    }
    //
    errno_t Cl::ReadOptFile(tstring s_file_path, unsigned int cout_type)
    {
        return ReadFileToString(s_file_path, s_device_code_affecting_options, cout_type);
    }
    // Ensure if the string is the name of the tool, called from CL or not
    bool Cl::IsToolName(const tstring &str)
    {
        if (str.empty())
            return false;
        tstring s_lowed = ToLowerCase(str);
        if (ToLowerCase(s_cl) == s_lowed)
            return true;
        else if (ToLowerCase(s_link) == s_lowed)
            return true;
        return false;
    }
    //
    Tool* Cl::GetToolByName(const tstring &name)
    {
        if (cl->GetName() == name)
            return cl;
        if (link->GetName() == name)
            return link;
        return nullptr;
    }
    //
    tstring Cl::GetObjFileName(Tool *parent_tool, bool b_produce_default_name_if_empty)
    {
        if (slashFo.empty())
        {
            Driver *p_drv = dynamic_cast<Driver*>(parent_tool);
            if (!p_drv)
                return s_empty;
            if (b_produce_default_name_if_empty)
                return Tool::GetOutputPath(p_drv) + GetFileNameOnly(aa::s_input_file) + s_default_aacl_output_file_ext;
           else
                return s_empty;
        }
        else
        {
            tstring s_obj = Unquote(slashFo.substr(Fo_opt.size()+1, slashFo.size()-Fo_opt.size()-1));
            // if directory is specified in /Fo option
            if (IsFinishedWithSlashOrBackslash(s_obj))
                return s_obj + GetFileNameOnly(aa::s_input_file) + s_obj_ext;
            // if filename is specified in /Fo option
            else
                return s_obj;
        }
    }
    //
    tstring Cl::GetObjFileNameMappedForCl(Tool *parent_tool)
    {
        Driver *p_drv = dynamic_cast<Driver*>(parent_tool);
        if (!p_drv)
            return s_empty;
        bool b_aacl_driven = true;
        if (dynamic_cast<Aalink*>(parent_tool))
            b_aacl_driven = false;        
        tstring s_out_file_name_wo_ext = GetFileNameOnly(aa::s_input_file);
        tstring s_default_obj_file_name;
        if (b_aacl_driven)
        {
            if (!b_no_device_code)
                s_default_obj_file_name = s_out_file_name_wo_ext + s_default_aacl_output_file_ext + s_obj_ext;
            else
                s_default_obj_file_name = s_out_file_name_wo_ext + s_obj_ext;
        }
        else
            s_default_obj_file_name = s_out_file_name_wo_ext + s_obj_ext;
        tstring s_out_dir = Tool::GetOutputPath(p_drv);
        tstring s_out_file_ext_only = GetExtension(aa::s_input_file);
        tstring s_out_file_name_only = GetFileNameOnly(aa::s_input_file);
        tstring s_mapped_obj_filename;        
        if (slashFo.empty())
        {
            if (b_aacl_driven)
                s_mapped_obj_filename = s_default_obj_file_name;
            else
                s_mapped_obj_filename = s_out_dir + s_default_obj_file_name;
        }
        // TODO_HSA: Decide: /Fo is placing in the end. Check multipe /Fo and theirs overrides
        // Code for /Fo is here because unknown of cpp file name on the stage of options parsing 
        // cpp goes further after all options as a rule        

        // if dir
        // TO_DECIDE: might it be dir w/o slash or backslash at the end? 
        //            ANSWER: NO! By default VC++ sets $(IntDir) macros, which finishes with /
        // TO_DECIDE: should driver create the dir or not in case of its absence?
        else
        {
            s_mapped_obj_filename = Unquote(slashFo.substr(Fo_opt.size()+1, slashFo.size()-Fo_opt.size()-1));
            // if directory is specified in /Fo option
            if (IsFinishedWithSlashOrBackslash(s_mapped_obj_filename))
                s_mapped_obj_filename += s_out_file_name_only + s_obj_ext + s_obj_ext;
            // if filename is specified in /Fo option
            else
            {
                // if device code, then generate *.obj.obj by default
                if (!b_no_device_code)
                    s_mapped_obj_filename += s_obj_ext;
            }
        }
        return QuoteIfSpaces(Trim(s_mapped_obj_filename));
    }
    //
    void Cl::BundleAllOptions()
    {
        s_hardcoded_options = Trim(s_hardcoded_options);
        s_compulsory_options = Trim(s_compulsory_options);
        s_options = Trim(s_options);
        s_input_files = Trim(s_input_files);
        s_output_files = Trim(s_output_files);
        if (!s_hardcoded_options.empty())
            s_all_options += SpaceBefore(s_hardcoded_options);
        if (!s_compulsory_options.empty())
            s_all_options += SpaceBefore(s_compulsory_options);
        if (!s_options.empty())
            s_all_options += SpaceBefore(s_options);
        if (!p_current_driver->s_includes.empty())
            s_includes += SpaceBefore(Trim(p_current_driver->s_includes));
        if (!s_includes.empty())
            s_all_options += SpaceBefore(s_includes);
        AddDeviceCodeAffectingOption(s_all_options);
        if (!s_current_run_options.empty())
            s_all_options += SpaceBefore(s_current_run_options);
        BundleInputFiles();
        if (p_current_driver == aalink_tool)
        {
            // if link is not run separately
            if (!b_c_only)
            {
                if (tstring::npos == link->s_options.find(link_opt + s_space))
                    s_all_options += SpaceBefore(Slash(link_opt));
                s_all_options += SpaceBefore(Trim(link->s_options));
                if (!link->s_input_files.empty()) 
                    s_all_options += SpaceBefore(Trim(link->s_input_files));
            }
        }
        s_all_options = Trim(s_all_options);
    }
    //  Gets cl.exe absolute path, checks its existance and access
    errno_t Cl::Where(tstring &cl_exe_abs_path)
    {
        cl_exe_abs_path = s_empty;
        errno_t iRet = GetVSpath(cl_exe_abs_path);
        if (EXIT_SUCCESS != iRet)
            return CoutError(iRet, cl_exe_abs_path);
        unsigned int iVSVersion = 11;
        if (iVSVersion >= 11)
        {
            cl_exe_abs_path += s_VC_bin;
            if (Is32bitMachine())
                cl_exe_abs_path += s_cl_exe;
            else if (Is64bitMachine())
                cl_exe_abs_path += s_amd64 + s_backslash + s_cl_exe;
        }
        return CheckPath(cl_exe_abs_path);
    }
    //  Gets link.exe absolute path, checks its existance and access
    errno_t Link::Where(tstring &link_exe_abs_path)
    {
        link_exe_abs_path = s_empty;
        errno_t iRet = GetVSpath(link_exe_abs_path);
        if (EXIT_SUCCESS != iRet)
            return CoutError(iRet, link_exe_abs_path);
        unsigned int iVSVersion = 11;
        if (iVSVersion >= 11)
        {
            link_exe_abs_path += s_VC_bin;
            if (Is32bitMachine())
                link_exe_abs_path += s_link_exe;
            else if (Is64bitMachine())
                link_exe_abs_path += s_amd64 + s_backslash + s_link_exe;
        }
        return CheckPath(link_exe_abs_path);
    }
    // Ensure if the string is the name of the tool, called from LINK or not
    bool Link::IsToolName(const tstring &str)
    {
        if (str.empty())
            return false;
        tstring s_lowed = ToLowerCase(str);
        if (ToLowerCase(s_link) == s_lowed)
            return true;
        return false;
    }
    //
    void Link::AddLinkOption() 
    {
        if (!b_link_option_added)
        {
            s_options = Slash(link_opt) + SpaceBefore(Trim(s_options)); 
            b_link_option_added = true;
        }
    }
    //
    void Link::AddMachineOption()
    {
        tstring s_options_uc = ToUpperCase(s_options);
        tstring s_machine_uc = ToUpperCase(MACHINE_opt);
        std::size_t i_find = s_options_uc.find(s_machine_uc);
        if (tstring::npos == i_find)
        {
            tstring sMACHINE = s_x86;
            if (Is64bitMachine())
                sMACHINE = s_x64;
            AddOption(Slash(MACHINE_opt + sMACHINE));
        }
    }
    //
    errno_t Link::AddImplibOption(Tool *parent_tool)
    {
        Aalink *p_drv = dynamic_cast<Aalink*>(parent_tool);
        if (!p_drv)
            return EXIT_FAILURE;
        tstring s_out = p_drv->GetOutputFileName();
        if (s_out.empty())
            return EXIT_FAILURE;
        s_out = GetFileNameOnly(s_out);
        AddOption(Slash(IMPLIB_opt + s_out + s_lib_ext));
        return EXIT_SUCCESS;
    }
    //
    errno_t Link::Execute(Tool *parent_tool)
    {
        Aalink *p_drv = dynamic_cast<Aalink*>(parent_tool);
        if (!p_drv)
            return EXIT_FAILURE;
        // finding tool executable
        errno_t iRet = FindExecutable();
        if (EXIT_SUCCESS != iRet)
            return iRet; 
        AddMachineOption();
        return ExecProcessAndExit(p_drv);
    }
    //
    void Link::BundleAllOptions()
    {
        s_input_files = Trim(s_input_files);
        s_options = Trim(s_options);
        s_all_options = s_options;
        s_all_options += SpaceBefore(s_input_files);
    }
    //
    Tool* Link::GetToolByName(const tstring &name)
    {
        if (link->GetName() == name)
            return link;
        return nullptr;
    }
    //////////////// inflate Execute ///////////////////////////////////////////////////
    // Example: "inflate -d matrix_multiply_amp.brig -o matrix_multiply_amp.brig.cpp"
    ////////////////////////////////////////////////////////////////////////////////////
    errno_t Inflate::Execute(Tool *parent_tool)
    {
        Driver *p_drv = dynamic_cast<Driver*>(parent_tool);
        if (!p_drv)
            return EXIT_FAILURE;
        if (!dynamic_cast<Aalink*>(parent_tool))
            return EXIT_FAILURE;
        if (IsExecutedOnce())
            ClearAllOptions();
        // finding tool executable
        errno_t iRet = FindExecutable();
        if (EXIT_SUCCESS != iRet)
            return iRet; 
        // output calculation
        tstring s_out_dir = GetOutputPath(p_drv);
        tstring s_input_file_name_only = GetFileNameOnly(aa::s_input_file);
        tstring s_input_file_ext_only = GetExtension(aa::s_input_file);

        tstring s_bif = s_out_dir + s_bif_filename;
        AddInputFile(QuoteIfSpaces(s_bif));
        AddHardcodedOption(Dash(d_opt) + SpaceBefore(QuoteIfSpaces(s_bif)));
        s_bif_cpp_filename = s_bif + s_cpp_ext;
        s_output_file = s_bif_cpp_filename;
        aa::s_input_file = s_bif_cpp_filename;

        AddOutputFile(QuoteIfSpaces(s_output_file));
        AddHardcodedOption(Dash(o_opt) + SpaceBefore(QuoteIfSpaces(s_output_file)));      
        return ExecProcessAndExit(p_drv);
    }
    //////////////// fatobj Execute /////////////////////////////////
    // Example (aacl):   "fatobj -create matrix_multiply_amp.fatobj"
    // Example (aalink): "fatobj -extract matrix_multiply_amp.fatobj"
    /////////////////////////////////////////////////////////////////
    errno_t Fatobj::Execute(Tool *parent_tool)
    {
        Driver *p_drv = nullptr;
        bool b_aalink_driven = false;
        p_drv = dynamic_cast<Driver*>(parent_tool);
        if (!p_drv)
            return EXIT_FAILURE;
        if (dynamic_cast<Aalink*>(parent_tool))
            b_aalink_driven = true;
        if (IsExecutedOnce())
            ClearAllOptions();
        // finding tool executable
        errno_t iRet = FindExecutable();
        if (EXIT_SUCCESS != iRet)
            return iRet; 
        // output calculation
        tstring s_out_dir = GetOutputPath(p_drv);
        tstring s_input_file_name = GetFileNameWithExtension(aa::s_input_file);
        tstring s_input_file_name_wo_ext = GetFileNameOnly(aa::s_input_file);
        errno_t i_ret_file_check = EXIT_SUCCESS;
        if (b_aalink_driven)
        {
            // checking of input fatobj file existance (aalink side)
            i_ret_file_check = CheckFile(s_out_dir + s_fatobj_filename, AACL_NOANYMESSAGE);
            // if not found, then fatobj file should be in current dir (fatobj filename stayed without path)
            if (EXIT_SUCCESS == i_ret_file_check)
                s_fatobj_filename = s_out_dir + s_fatobj_filename;
            AddHardcodedOption(Dash(extract_opt));
            // input *.fatobj
            AddInputFile(QuoteIfSpaces(s_fatobj_filename));
            // output *.brig
            s_brig_filename = s_fatobj_filename + s_brig_ext;
            AddOutputFile(QuoteIfSpaces(s_brig_filename));
            devlink->AddBrigFile(s_brig_filename);
            // output *.obj
            s_obj_filename = s_fatobj_filename + s_obj_ext;
            AddOutputFile(QuoteIfSpaces(s_obj_filename));
            link->AddInputFile(QuoteIfSpaces(s_obj_filename));
            // output *.opt
            s_opt_filename = s_fatobj_filename + s_opt_ext;
            AddOutputFile(QuoteIfSpaces(s_opt_filename));
        }
        else
        {
            s_fatobj_filename = cl->GetObjFileName(p_drv, true);
            AddHardcodedOption(Dash(create_opt));
            AddOutputFile(QuoteIfSpaces(s_fatobj_filename));
        }
        AddOption(QuoteIfSpaces(s_fatobj_filename));
        BundleAllOptions();
        // output tool diagnostics (if debug2) and real command line
        iRet = Output();
        if (EXIT_SUCCESS != iRet)
            return iRet;
        iRet = ExecProcess();
        // Checking file existence
        errno_t i_ret_file_check_2 = EXIT_SUCCESS;
        errno_t i_ret_file_check_3 = EXIT_SUCCESS;
        if (b_aalink_driven)
        {
            i_ret_file_check   = p_drv->CheckAndAddIntermediateFile(s_obj_filename);
            i_ret_file_check_2 = p_drv->CheckAndAddIntermediateFile(s_brig_filename);
            i_ret_file_check_3 = p_drv->CheckAndAddIntermediateFile(s_opt_filename, aa::AACL_NOANYMESSAGE);
        }
        s_fatobj_filename = QuoteIfSpaces(s_fatobj_filename);
        if ((EXIT_SUCCESS != iRet) || (EXIT_SUCCESS != i_ret_file_check) || (EXIT_SUCCESS != i_ret_file_check_2))
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
    //
    errno_t Fatobj::DetectType(Tool *parent_tool, tstring s_filename)
    {
        Driver *p_drv = nullptr;
        bool b_aalink_driven = false;
        p_drv = dynamic_cast<Driver*>(parent_tool);
        if (!p_drv)
            return EXIT_FAILURE;
        if (dynamic_cast<Aalink*>(parent_tool))
            b_aalink_driven = true;
        ClearAllOptions();
        // finding tool executable
        errno_t iRet = FindExecutable();
        if (EXIT_SUCCESS != iRet)
            return iRet; 
        // output calculation
        tstring s_out_dir = GetOutputPath(p_drv);
        tstring s_input_file_name = GetFileNameWithExtension(s_filename);
        tstring s_input_file_name_wo_ext = GetFileNameOnly(s_filename);
        tstring s_file_to_check = s_filename;
        // checking the file existance in current dir
        errno_t i_ret_file_check = CheckFile(s_file_to_check, AACL_NOANYMESSAGE);
        // checking the file existance in output dir
        if (EXIT_SUCCESS != i_ret_file_check)
            s_file_to_check = s_out_dir + s_filename;
        // trying to find the file in library paths
        if (EXIT_SUCCESS != i_ret_file_check)
        {
            tstring s_abs_path, s_cmd;
            s_file_to_check = s_filename;
            aalink_tool->FindLibrary(s_file_to_check, s_abs_path, s_cmd);
            s_file_to_check = s_abs_path;
        }
        AddOption(Dash(detect_type_opt));
        AddOption(QuoteIfSpaces(s_file_to_check));
        BundleAllOptions();
        return ExecProcess();
    }
    //////////////// cl Execute ////////////////////////////////////////////////////////////////
    // Example: "cl /D_DEBUG /I"C:\Program Files (x86)\Microsoft C++ AMP Prerelease"\include /EHsc /Fo"Debug\HelloAMP.obj" /c HelloAMP.cpp"
    ////////////////////////////////////////////////////////////////////////////////////////////
    errno_t Cl::Execute(Tool *parent_tool)
    {
        Driver *p_drv = dynamic_cast<Driver*>(parent_tool);
        if (!p_drv)
            return EXIT_FAILURE;
        bool b_aacl_driven = true;
        if (dynamic_cast<Aalink*>(p_drv))
            b_aacl_driven = false;
        if (IsExecutedOnce())
        {
            s_current_run_options.clear();
            s_hardcoded_options.clear();
            s_includes.clear();
            s_input_files.clear();
            s_output_files.clear();
            s_all_options.clear();
        }
        // finding tool executable
        errno_t iRet = FindExecutable();
        if (EXIT_SUCCESS != iRet)
            return iRet; 
        // output calculation
        tstring s_out_dir = Tool::GetOutputPath(p_drv);
        size_t i_aa_size = path_to_aa_include.size();
        tstring s_out_file_name_only = GetFileNameOnly(aa::s_input_file);
        tstring s_out_file_name_wo_ext = GetPathWitoutExtension(aa::s_input_file);
        tstring s_out_file_ext_only = GetExtension(aa::s_input_file);
        // check if it is an *.obj file
        if (aa::s_input_file.size()-s_obj_ext.size() == aa::s_input_file.rfind(s_obj_ext))
            s_out_file_name_only = s_empty;
        tstring additional_include;
        additional_include = Slash(I_opt) + QuoteIfSpaces(path_to_aa_include);
        AddIncludes(additional_include);
        if (!b_aacl_driven)
            if (!s_out_file_name_only.empty())
                s_out_file_name_only += s_out_file_ext_only;
        // /Fo verifying: if empty, then add /Fo option with name = input file w/o ext + .obj    
        tstring s_cl_input_filename = s_out_file_name_only;
        tstring slashFo_mapped_for_cl;
        // when s_out_file_name_only is empty it means that cl executes not source, but *.obj file (for example)
        if (!s_out_file_name_only.empty())        
        {
            slashFo_mapped_for_cl = GetObjFileNameMappedForCl(parent_tool);
            AddOption(Slash(Fo_opt) + slashFo_mapped_for_cl, true);
            AddOutputFile(slashFo_mapped_for_cl); 
            if ((!b_aacl_driven) && (aalink_tool->b_run_separate_link))
                link->AddInputFile(slashFo_mapped_for_cl);
            if (b_aacl_driven)
            {
                s_cl_input_filename += s_host_ext + s_out_file_ext_only;
                AddInputFile(QuoteIfSpaces(s_out_dir + s_cl_input_filename));
            }
        }
        else if (!link->s_input_files.empty())
        {
            if (!b_aacl_driven) 
                AddInputFile(link->s_input_files);
        }
        //linker options if no /c (!b_c_only)
        errno_t i_ret_file_check_opt = EXIT_SUCCESS;
        if (b_aacl_driven)
        {
            if (!b_c_only)
            {
                tstring s_obj = GetObjFileName(p_drv, true);
                if (!s_out_file_name_only.empty())
                    link->AddOption(QuoteIfSpaces(s_obj));
                AddOption(Slash(c_opt), true);
                if (!IsExecutedOnce())
                {
                    i_aa_size = path_to_aa_lib.size();
                    if (i_aa_size > 0)
                    {
                        tstring path_to_aa_lib_for_cl = path_to_aa_lib;
                        // remove last slash or backslash
                        if ((ch_backslash == path_to_aa_lib[i_aa_size-1]) 
                         || (ch_slash == path_to_aa_lib[i_aa_size-1]))
                            path_to_aa_lib_for_cl.replace(i_aa_size-1, 1, s_empty);
                        tstring s_hsa_lib = s_hsa;
                        if (Is64bitMachine())
                            s_hsa_lib += s_64;
                        s_hsa_lib += s_lib_ext;
                        link->AddOption(Slash(LIBPATH_opt) + QuoteIfSpaces(path_to_aa_lib_for_cl));
                        link->AddOption(Quote(s_hsa_lib));
                        if (!s_out_file_name_only.empty())
                            if (!aalink_tool->IsOUTspecified())
                            {
                                tstring s_out_filename = s_out_dir + s_out_file_name_only;
                                if (aalink_tool->b_DLL)
                                    s_out_filename = QuoteIfSpaces(s_out_filename + s_dll_ext);
                                else
                                    s_out_filename = QuoteIfSpaces(s_out_filename + s_exe_ext);
                                link->s_options += SpaceBefore(Slash(OUT_opt)) + s_out_filename;
                            }
                    }
                }
            }
        }
        // if cl is aalink driven
        else
        {
            if (!Unquote(s_cl_input_filename).empty())
                AddInputFile(QuoteIfSpaces(s_out_dir + s_cl_input_filename));
            // since 0.0.1.9
            tstring s_o = Unquote(s_fatobj_filename) + s_opt_ext;
            tstring s_opt_f = QuoteIfSpaces(s_out_dir + s_o);
            iRet = ReadOptFile(s_opt_f, AACL_NOANYMESSAGE);
            if ((EXIT_SUCCESS != iRet) && (!s_out_dir.empty()))
            {
                tstring s_curr_dir;
                GetCurDir(s_curr_dir);
                if (!s_curr_dir.empty())
                {
                    s_opt_f = QuoteIfSpaces(s_o);
                    iRet = ReadOptFile(s_opt_f);
                }
            }
            s_current_run_options += s_device_code_affecting_options;
            link->AddMachineOption();
        }
        if (s_out_file_name_only.empty())
        {
            s_options = s_empty;
            if (b_aacl_driven)
                return EXIT_SUCCESS;
        }
        BundleAllOptions();
        // output tool diagnostics (if debug2) and real command line
        iRet = Output(true);
        if (EXIT_SUCCESS != iRet)
            return iRet;
        iRet = ExecProcess();
        // creating *.opt file
        if (b_aacl_driven)
        {
            if (slashFo_mapped_for_cl.empty())
                slashFo_mapped_for_cl = s_out_file_name_only + s_default_aacl_output_file_ext;
            tstring s_opt_file = GetFileNameOnly(slashFo_mapped_for_cl) + s_opt_ext;
            if (GetPathWitoutFileName(s_opt_file).empty())
                s_opt_file = s_out_dir + s_opt_file;
            i_ret_file_check_opt = CreateOptFile(s_opt_file);
            if (EXIT_SUCCESS != iRet)
                CoutError(i_ret_file_check_opt, s_empty, AACL_WARNING);
            i_ret_file_check_opt = p_drv->CheckAndAddIntermediateFile(Unquote(s_opt_file));
            // if no /c option then add -hidden-aacl-driven option to aalink
            if (!b_c_only)    
                aalink_tool->AddOption(Dash(hidden_aacl_driven_opt));
        }        
        errno_t i_ret_file_check = EXIT_SUCCESS;
        if (!b_aacl_driven)
            i_ret_file_check = p_drv->CheckAndAddIntermediateFile(GetObjFileNameMappedForCl(parent_tool));
        if ((EXIT_SUCCESS != iRet) || (EXIT_SUCCESS != i_ret_file_check) || (EXIT_SUCCESS != i_ret_file_check_opt))
        {
            tcout << std::endl;
            CoutReturnCode(GetName(), iRet);
            errno_t i_ret = DeleteIntermediateFiles();
            if (EXIT_SUCCESS != i_ret)
                return CoutError(i_ret);
            return iRet;
        }
        if (debugSetLevel) 
        {
            tcout << std::endl;     
            CoutReturnCode(GetName(), iRet);
        }
        return iRet;
    }
    //////////////// devlink Execute /////////////////////////////////////////////////////
    // Example: "devlink -obif matrix_multiply_amp.brig -o matrix_multiply_amp.exe.brig"
    //////////////////////////////////////////////////////////////////////////////////////
    errno_t Devlink::Execute(Tool *parent_tool)
    {
        if (0 == additional_brig_files.size())
            return 1;
        Aalink *p_drv = dynamic_cast<Aalink*>(parent_tool);
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
        // gets the output file name from /OUT: option (if it is set)
        tstring s_out_file_name_only = p_drv->GetOutputFileName();
        // if /OUT: is unset, then use s_input_file (should be the first *.brig file in aalink cmdline)
        if (s_out_file_name_only.empty())
        {
            if (!additional_brig_files.empty())
            {
                s_out_file_name_only = GetPathWitoutExtension(GetPathWitoutExtension(*additional_brig_files.begin()));
                aalink_tool->s_OUT_filename = s_out_file_name_only;
            }
        }
        s_out_file_name_only = s_out_dir + s_out_file_name_only;
        // -obif
        if (b_obif)
            AddHardcodedOption(Dash(obif_opt));
        // output files
        if (b_obif)
            s_output_file = s_out_file_name_only + s_bif_ext;
        else
            s_output_file = s_out_file_name_only + s_dev_ext + s_bif_ext;
        s_bif_filename = GetFileNameWithExtension(s_output_file);
        AddOutputFile(QuoteIfSpaces(s_output_file));
        AddHardcodedOption(Dash(o_opt) + SpaceBefore(QuoteIfSpaces(s_output_file)));        
        // finding libraries
        p_drv->FindLibraries(additional_brig_files, additional_brig_files_in_abs_paths, s_brig_files);
        return ExecProcessAndExit(p_drv);
    }
    //
    void Devlink::BundleAllOptions()
    {
        s_brig_files = Trim(s_brig_files);
        if (!s_brig_files.empty())
            s_input_files += SpaceBefore(s_brig_files);
        Tool::BundleAllOptions();
    }
    //
    errno_t Aalink::Diagnostics()
    {
        if (2 != debugSetLevel) 
            return EXIT_SUCCESS;
        errno_t iRet = Driver::Diagnostics();
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (IsMightBeColored())
            SetConsoleTextAttribute(hOut, FOREGROUND_INTENSITY);
        if (!s_rsp_cmdline.empty())
        {
            s_options = Trim(s_options);
            tcout << GetName() << aa::s_command_line << s_options << std::endl;    
            s_rsp_cmdline = Trim(s_rsp_cmdline);
            tcout << GetName() << aa::s_rsp_file << s_rsp_cmdline << std::endl;
        }
        s_LINK_options = Trim(s_LINK_options);
        tcout << GetName() << aa::s_LINK_options << s_LINK_options << std::endl;
        s_LIB_paths = Trim(s_LIB_paths);
        tcout << GetName() << aa::s_LIB_paths << s_LIB_paths << std::endl;
        if (IsMightBeColored())
            SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        return iRet;
    }
    //
    errno_t Aalink::Parse() 
    {
        errno_t iRet = ParseLINK(AACL_NOANYMESSAGE);
        iRet = Get_LIB();
        iRet = ParseArgs(argc, argv);  
        if (EXIT_SUCCESS != iRet)
        {
            if ((AA_EXIT_SUCCESS_AND_STOP != iRet) && (AA_EXIT_ERROR_PRINTED != iRet))
                return CoutError(iRet);
            else
                return iRet;
        }
        SetVSIDEtoPath();
        s_all_options = s_options;
        Diagnostics();
        return iRet;
    }
    //
    errno_t Aalink::Execute(Tool * /*parent_tool*/)
    {
        // Example:
        // "aalink /debug /libpath:"%WIN_SDK%\Lib\win8\um\%WIN_PLATFORM%" /out:Debug\matrix_multiply_amp.exe Debug\matrix_multiply_amp.fatobj"
        errno_t iRet = Where(s_abs_path);
        if (EXIT_SUCCESS != iRet)
            return CoutError(s_aalink + SpaceBefore(s_wasnt_found)); 
        s_abs_path = Quote(s_abs_path);
        s_all_options = Trim(s_specific_options) + SpaceBefore(Slash(link_opt)) + SpaceBefore(Trim(s_options));
        if (debugSetLevel) 
        {
            tcout << std::endl;
            CoutToolName(s_aalink);
            tcout << s_abs_path << s_space << s_all_options << std::endl << std::endl;
        }
        iRet = ExecProcess();
        if (EXIT_SUCCESS != iRet)
        {
            CoutReturnCode(s_aalink, iRet);
            return iRet;
        }
        if (debugSetLevel) 
            CoutReturnCode(s_aalink, iRet);
        return iRet;
    }
    //
    void Aalink::AddLibPath(const tstring &s_lib_path)
    {
        lib_paths.push_back(s_lib_path);
        s_LIB_paths += SpaceBefore(s_lib_path + s_semicolon);
    }
    //
    std::list<tstring> Aalink::Get_lib_paths() {return lib_paths;}
    //
    bool Aalink::IsContainingDeviceCode()
    {
        if (fatobj_files.size() > 0)
            return true;
        if (devlink->additional_brig_files.size() > 0)
            return true;
        return false;
    }
    //
    errno_t Aalink::Get_LIB()
    {
        tstring s_LIB_paths;
        errno_t iRet = GetEnv(s_LIB, s_LIB_paths, AACL_NOANYMESSAGE);
        if (EXIT_SUCCESS == iRet)
        {
            tstring s_sys_includes;
            size_t iFind = 0;
            size_t i_size = s_LIB_paths.size();
            tstring s_inc;
            while (tstring::npos != iFind)
            {
                bool bAdd = false;
                iFind = s_LIB_paths.find(s_semicolon);
                if (tstring::npos != iFind)
                {
                    s_inc = BackslashIfNeeded(s_LIB_paths.substr(0, iFind));
                    s_LIB_paths = s_LIB_paths.substr(iFind+1, i_size-iFind-1);
                    bAdd = true;
                }
                else
                {
                    s_inc = s_LIB_paths;
                    bAdd = true;
                }
                s_inc = Trim(s_inc);
                if ((bAdd) && (s_inc.size() > 0))
                {
                    AddLibPath(s_inc);
                }
            }
        }
        return iRet;
    }
    //
    errno_t Aalink::ParseLINK(unsigned int cout_type)
    {
        errno_t iRet = GetEnv(s_LINK, s_LINK_options, cout_type);
        if (EXIT_SUCCESS != iRet)
            return CoutError(iRet, s_empty, cout_type);
        tstring sCmdLine = s_abs_path + s_space + s_LINK_options;
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
    //
    errno_t Aalink::ParseArgs(int argc, tchar **argv, bool b_main_command_line)
    {
        // previous option name to compare on the next cycle step
        tstring prev_opt; 
        // if option has arguments it is needed to save option with arguments
        tstring prev_opt_full; 
        // tool name (if prev option was -tool=tool_name); become empty after parsing of the following: tool_option 
        tstring prev_tool_name;
        // the size of the current option
        size_t iOptSize = 0;
        // if option is ignored, but must have the argument, the next option will be also ignored
        bool b_ignore_next_option = false;
        bool bOptNeedsArg = false;
        bool b_ignore_for_link = false;
        errno_t iRet = 0;
        if (argc <= 1)
        {
            if (!b_main_command_line)
                return EXIT_SUCCESS;
            CoutError(s_not_enough_cmd_args, s_must_specify_at_least_1);
            return AA_EXIT_ERROR_PRINTED;
        }
        tstring s_uppered_currstr;
        tstring s_unquoted_currstr;
        bool b_unquoted = false;
        for (int num = 1; num < argc; ++num)
        {
            tstring currstr(argv[num]);
            iOptSize = currstr.size();
            if (0 == iOptSize)
                continue;
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
            b_ignore_for_link = false;
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
                tstring s_uppered_currstr = currstr;
                UpperCase(s_uppered_currstr);
                // /link
                if (1 == currstr.find(link_opt) && (iOptSize == link_opt.size()+1))
                {
                    b_ignore_for_link = true;
                }
                // /DLL
                if (1 == currstr.find(s_DLL) && (iOptSize == s_DLL.size()+1))
                {
                    b_DLL = true;
                }
                // /OUT:"FILEPATH"
                if (1 == s_uppered_currstr.find(OUT_opt) && (iOptSize > OUT_opt.size()+1))
                {
                    b_OUT = true;
                    b_ignore_for_link = true;
                    link->slashOUT = currstr;
                    s_OUT_filename = currstr.substr(OUT_opt.size()+1, currstr.size()-OUT_opt.size()-1);
                    s_OUT_filename = GetFileNameOnly(s_OUT_filename);
                    s_OUT_filename = GetPathWitoutExtension(s_OUT_filename);
                    link->AddOption(QuoteIfSpaces(currstr));
                }
                // /LIBPATH:"FILEPATH"
                else if (1 == s_uppered_currstr.find(LIBPATH_opt) && (iOptSize > LIBPATH_opt.size()+1))
                {
                    b_ignore_for_link = true;
                    tstring s_LIB_path = currstr.substr(LIBPATH_opt.size()+1, currstr.size()-LIBPATH_opt.size()-1);
                    AddLibPath(s_LIB_path);
                    link->AddOption(QuoteIfSpaces(currstr));
                }
                /////////////////////////
                // aalink OPTIONS ONLY //
                /////////////////////////
                // color        aacl/aalink specific
                else if (1 == currstr.find(color_opt) && (iOptSize == color_opt.size()+1))
                {
                    b_ignore_for_link = true;
                    b_color = true;
                    AddSpecificOption(Dash(color_opt));
                }
                // debug1        aacl/aalink specific
                else if ((1 == currstr.find(debug1_opt))  && (iOptSize == debug1_opt.size()+1))
                {
                    b_ignore_for_link = true;
                    debugSetLevel = 1;
                    AddSpecificOption(Dash(debug1_opt));
                }
                // debug2        aacl/aalink specific
                else if ((1 == currstr.find(debug2_opt)) && (iOptSize == debug2_opt.size()+1))
                {
                    b_ignore_for_link = true;
                    debugSetLevel = 2;
                    AddSpecificOption(Dash(debug2_opt));
                }
                // hidden-aacl-driven        aalink specific
                else if (1 == currstr.find(hidden_aacl_driven_opt) && (iOptSize <= hidden_aacl_driven_opt.size()+2))
                {
                    b_ignore_for_link = true;
                    b_run_separate_link = false;
                    AddSpecificOption(Dash(hidden_aacl_driven_opt));
                }
                // keeptmp        aacl/aalink specific
                else if (1 == currstr.find(keeptmp_opt) && (iOptSize <= keeptmp_opt.size()+2))
                {
                    b_ignore_for_link = true;
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
                        // TODO_HSA: TEST IS NEEDED!
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
                        // TODO_HSA: TEST IS NEEDED!
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
                    b_ignore_for_link = true;
                    bOptNeedsArg = true;
                    prev_opt = tool_opt;
                    prev_opt_full = currstr.substr(1, iOptSize-1);
                }
                ///////////////////////////////////////
                // adding current option to cl (link)
                ///////////////////////////////////////
                if (!b_ignore_for_link)
                {
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
                    s_rsp_cmdline = sCmdLine;
                    sCmdLine = s_abs_path + s_space + sCmdLine;
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
                    ////////////////////////
                    // HLC OPTIONS ONLY
                    ////////////////////////
                    // -tool=tool_name tool_opt
                    if (tool_opt == prev_opt)
                    {
                        AddSpecificOption(Dash(prev_opt_full) + SpaceBefore(currstr));
                        AddSpecificOptionForTool(prev_tool_name, currstr);
                        prev_tool_name = s_empty;
                        prev_opt_full = s_empty;
                    }
                }
                else
                {
                    tstring unquoted_currstr = Unquote(currstr);
                    // check type
                    errno_t iType = fatobj->DetectType(this, currstr);
                    switch (iType)
                    {
                        // Unknown => MS link files (*.obj, *.lib, *.pdb etc.)
                        case 1:
                        {
                            aa::s_input_file = currstr;
                            link->AddInputFile(QuoteIfSpaces(currstr));
                            s_input_files += SpaceBefore(QuoteIfSpaces(currstr));
                            break;
                        }
                        // Fat object
                        case 2:
                        {
                            s_fatobj_filename = currstr;
                            aa::s_input_file = s_fatobj_filename;
                            s_input_files += SpaceBefore(QuoteIfSpaces(currstr));
                            AddFatobjFile(currstr);
                            break;
                        }
                        // 'old plain' ELF/BRIG
                        case 3:
                        {
                            devlink->AddBrigFile(currstr);
                            s_input_files += SpaceBefore(QuoteIfSpaces(currstr));
                        }
                        // BIF
                        // TODO_HSA: after supporting of linking of bif files in devlink (w/o -ibif) together with brig files
                        case 4:
                        {
                            break;
                        }
                    }
                }
                prev_opt = s_empty;
                bOptNeedsArg = false;
            }
        }
        return iRet;
    }
    //
    bool Aalink::HelpOption(const tstring &str, bool bPrint)
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
            tcout << s_OVERVIEW << s_aalink_description << std::endl << std::endl;
            tcout << s_USAGE << GetName() << SpaceBefore(s_aacl_syntax) << std::endl << std::endl;
            tcout << s_OPTIONS << std::endl << std::endl;
            if (b_hidden)
            {
                // -color
                tcout << s_space << s_space << Dash(color_opt) 
                      << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space 
                      << SpaceBefore(Dash(Space(color_opt_descr))) << std::endl;
                // -debug1
                tcout << s_space << s_space << Dash(debug1_opt) 
                      << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space
                      << SpaceBefore(Dash(Space(debug1_opt_descr))) << std::endl;
                // -debug2
                tcout << s_space << s_space << Dash(debug2_opt)
                      << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space
                      << SpaceBefore(Dash(Space(debug2_opt_descr))) << std::endl;
            }
            // -help
            tcout << s_space << s_space << Dash(help_opt) 
                  << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space 
                  << SpaceBefore(Dash(Space(help_opt_descr))) << std::endl;
            if (b_hidden)
            {
                // -hidden-aacl-driven
                tcout << s_space << s_space << Dash(hidden_aacl_driven_opt) 
                      << SpaceBefore(Dash(Space(hidden_aacl_driven_opt_descr))) << std::endl;
                // -keeptmp
                tcout << s_space << s_space << Dash(keeptmp_opt) 
                      << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space 
                      << SpaceBefore(Dash(Space(keeptmp_opt_descr))) << std::endl;
                // -tool=tool_name tool_opt
                tcout << s_space << s_space << Dash(tool_opt) 
                      << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space 
                      << SpaceBefore(Dash(Space(tool_opt_descr))) << std::endl;
            }
            // -version
            tcout << s_space << s_space << Dash(version_opt) 
                  << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space << s_space
                  << SpaceBefore(Dash(Space(version_opt_descr))) << std::endl;
            return true;
        }
        if (b_ok)
            return true;
        return false;
    }
    //
    void Aalink::FindLibrary(const tstring &lib, tstring &lib_in_abs_path, tstring &cmd_string)
    {
        if (IsAbsolutePath(lib))
        {
            lib_in_abs_path = lib;
            cmd_string += SpaceBefore(QuoteIfSpaces(lib));
            return;
        }
        tstring s_cur_dir;
        if (EXIT_SUCCESS != GetCurDir(s_cur_dir))
            return;
        tstring s_abs_path = s_cur_dir + s_backslash + lib;
        tstring s_lib_path;
        // TODO_HSA: find in lib_paths TEST!!!
        if (EXIT_SUCCESS == CheckFile(s_abs_path, AACL_NOANYMESSAGE))
        {
            lib_in_abs_path = lib;
            cmd_string += SpaceBefore(QuoteIfSpaces(lib));
            return;
        }
        bool b_lib_found = false;
        std::list<tstring>::const_iterator l_i, m_i;
        for (m_i = lib_paths.begin(); m_i != lib_paths.end(); ++m_i)
        {
            s_lib_path = *m_i;
            if (s_lib_path.empty())
                continue;
            RemoveLastSlashes(s_lib_path);
            s_abs_path = s_lib_path + s_backslash + lib;
            if (EXIT_SUCCESS == CheckFile(s_abs_path, AACL_NOANYMESSAGE))
            {
                lib_in_abs_path = s_abs_path;
                cmd_string += SpaceBefore(QuoteIfSpaces(s_abs_path));
                b_lib_found = true;
                break;
            }
        }
        // lib not found: try to find in AMD_lib folder
        if (!b_lib_found)
        {
            tstring s_amd_lib_path;
            GetAMD_lib(s_amd_lib_path);
            RemoveLastSlashes(s_amd_lib_path);
            s_abs_path = s_amd_lib_path + s_backslash + lib;
            if (EXIT_SUCCESS == CheckFile(s_abs_path, AACL_NOANYMESSAGE))
            {
                lib_in_abs_path = s_abs_path;
                cmd_string += SpaceBefore(QuoteIfSpaces(s_abs_path));
                b_lib_found = true;
            }
        }
        // lib still not found
        if (!b_lib_found)
        {
            CoutError(ENOENT, s_lib, AACL_WARNING);
            lib_in_abs_path = lib;
            cmd_string += SpaceBefore(QuoteIfSpaces(lib));
        }
    }
    //
    void Aalink::FindLibraries(const std::list<tstring> &libs, std::list<tstring> &libs_in_abs_paths, tstring &cmd_string)
    {
        libs_in_abs_paths.clear();
        cmd_string.clear();
        std::list<tstring> lib_paths = Get_lib_paths();
        tstring s_lib, s_abs_path, s_lib_path;
        std::list<tstring>::const_iterator l_i;
        for (l_i = libs.begin(); l_i != libs.end(); ++l_i)
        {
            s_lib = *l_i;
            if (s_lib.empty())
                continue;
            FindLibrary(s_lib, s_abs_path, cmd_string);
            libs_in_abs_paths.push_back(s_abs_path);
        }
    }
    //
    void Aalink::AddSpecificOption(const tstring &option)
    {
        s_specific_options += SpaceBefore(Trim(option));
    }
    // executes all fatobj tools on all *.fatobj files from aalink cmd line
    errno_t Aalink::ExecuteFatobjs()
    {
        errno_t iRet = EXIT_SUCCESS;
        if (0 == fatobj_files.size())
            return iRet;
        tstring s_cmd_str;
        FindLibraries(fatobj_files, fatobj_files_in_abs_paths, s_cmd_str);
        tstring s_file;
        tstring s_input = aa::s_input_file;
        std::list<tstring>::iterator l_i;
        fatobj->ClearAllOptions();
        for (l_i = fatobj_files_in_abs_paths.begin(); l_i != fatobj_files_in_abs_paths.end(); ++l_i)
        {
            s_file = *l_i;
            if (s_file.empty())
                continue;
            aa::s_input_file = s_file;
            aa::s_fatobj_filename = s_file;
            iRet = fatobj->Execute(this);
            if (EXIT_SUCCESS != iRet)
                return iRet;
            fatobj->ExecutedOnce();
            fatobj->ClearAllOptions();
        }
        return iRet;
    }
}

void SetEncoding()
{
    std::locale old_locale;
    std::locale utf8_locale(std::locale(old_locale, new std::codecvt_utf8<wchar_t>));
    std::locale::global(utf8_locale);
    tcout.imbue(utf8_locale);
    tcerr.imbue(utf8_locale);
    tclog.imbue(utf8_locale);
}

void SaveConsoleState()
{
    aa::g_hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    aa::g_hErr = GetStdHandle(STD_ERROR_HANDLE);
    aa::g_hIn  = GetStdHandle(STD_INPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi_old;
    GetConsoleScreenBufferInfo(aa::g_hOut, &csbi_old);
    aa::g_colors_old = csbi_old.wAttributes;
}

errno_t RestoreBeforeReturn(errno_t iRet)
{
    SetConsoleTextAttribute(aa::g_hOut, aa::g_colors_old);
    return iRet;
}

#endif // INC_AA_H__
