@echo off
if not exist test.bat goto helpenv
rem set TSDL_DIFF_INTERACTIVE=call wm
set TSDL_ALL_OPT_KEEP=
set TSDL_ALL_OPT_REBUILD=
set TSDL_ALL_OPT_SMOKE=
if not _%1==_ if not _%1==_rebuild if not _%1==_smoke if not _%1==_keep goto errbadparam
if not _%2==_ if not _%2==_rebuild if not _%2==_smoke if not _%2==_keep goto errbadparam
if not _%3==_ goto errbadparam
if _%1==_rebuild set TSDL_ALL_OPT_REBUILD=y
if _%2==_rebuild set TSDL_ALL_OPT_REBUILD=y
if _%1==_smoke set TSDL_ALL_OPT_SMOKE=y
if _%2==_smoke set TSDL_ALL_OPT_SMOKE=y
if _%1==_keep set TSDL_ALL_OPT_KEEP=y
if _%2==_keep set TSDL_ALL_OPT_KEEP=y
::
:: defaults
set TSDL_RUN_PFX_SMOKE=
set TSDL_RUN_PFX_SLOW=@rem
set TSDL_RUN_PFX_NORMAL=
::
if not _%TSDL_ALL_OPT_REBUILD%==_ (
set TSDL_RUN_PFX_SMOKE=
set TSDL_RUN_PFX_SLOW=
set TSDL_RUN_PFX_NORMAL=
)
if not _%TSDL_ALL_OPT_SMOKE%==_ (
set TSDL_RUN_PFX_SMOKE=
set TSDL_RUN_PFX_SLOW=@rem
set TSDL_RUN_PFX_NORMAL=@rem
)
goto run
echo TSDL_RUN_PFX_SMOKE =%TSDL_RUN_PFX_SMOKE%
echo TSDL_RUN_PFX_SLOW  =%TSDL_RUN_PFX_SLOW%
echo TSDL_RUN_PFX_NORMAL=%TSDL_RUN_PFX_NORMAL%
goto exit
::
:run
%TSDL_RUN_PFX_SMOKE%  call test clean
%TSDL_RUN_PFX_SLOW%   call test clean_all
%TSDL_RUN_PFX_SMOKE%  call test test_2m_simplest
%TSDL_RUN_PFX_SMOKE%  call test matrix_multiply_amp
%TSDL_RUN_PFX_SMOKE%  call test vp1
%TSDL_RUN_PFX_SMOKE%  call test vp2
%TSDL_RUN_PFX_SMOKE%  call test lib1
%TSDL_RUN_PFX_SMOKE%  call test a1
%TSDL_RUN_PFX_SMOKE%  call test a1bif
%TSDL_RUN_PFX_SMOKE%  call test bug20120716_min
%TSDL_RUN_PFX_SMOKE%  call test decl_glob_f1
%TSDL_RUN_PFX_SMOKE%  call test decl_loc_f1
%TSDL_RUN_PFX_SMOKE%  call test decl_glob_s1
%TSDL_RUN_PFX_SMOKE%  call test decl_loc_s1
%TSDL_RUN_PFX_SMOKE%  call test mp1
%TSDL_RUN_PFX_SMOKE%  call test invalid_lib1
%TSDL_RUN_PFX_SMOKE%  call test invalid_in1
%TSDL_RUN_PFX_SMOKE%  call test invalid_out1
%TSDL_RUN_PFX_SMOKE%  call test bug20120912a_org
%TSDL_RUN_PFX_SMOKE%  call test dirblock_opt
%TSDL_RUN_PFX_SMOKE%  call test o_function_list_1
%TSDL_RUN_PFX_SMOKE%  call test mult_defs_01neg
%TSDL_RUN_PFX_SMOKE%  call test unopt
%TSDL_RUN_PFX_SMOKE%  call test dpcel
%TSDL_RUN_PFX_SMOKE%  call test ddo_immed
%TSDL_RUN_PFX_SMOKE%  call test ddo_indirect
%TSDL_RUN_PFX_SLOW%   call test amp_libm2
%TSDL_RUN_PFX_NORMAL% call test aa1_libm2
%TSDL_RUN_PFX_NORMAL% call test aa2_libm2
%TSDL_RUN_PFX_NORMAL% call test unopt_big
%TSDL_RUN_PFX_SLOW%   call test bug20120716_org
%TSDL_RUN_PFX_SMOKE%  call test decl_glob_neg
%TSDL_RUN_PFX_SMOKE%  call test versions_neg
%TSDL_RUN_PFX_SMOKE%  call test debug_info
%TSDL_RUN_PFX_SMOKE%  call test kernel_neg
::
if not _%TSDL_ALL_OPT_KEEP%==_y call test clean
goto exit
::
:helpenv
echo.
echo * * * ERROR: TEST.BAT not found in the current directory.
echo * * * INFO : CD to the directory where tests reside, then re-run tests.
goto exit
::
:errbadparam
echo.
echo * * * ERROR: Bad parameter.
echo * * * Use "rebuild", "smoke" and "keep". Max 2 parameters allowed.
goto exit
::
:exit
::env cleanup
set TSDL_ALL_OPT_KEEP=
set TSDL_ALL_OPT_REBUILD=
set TSDL_ALL_OPT_SMOKE=
set TSDL_RUN_PFX_SMOKE=
set TSDL_RUN_PFX_SLOW=
set TSDL_RUN_PFX_NORMAL=
rem set TSDL_DIFF_INTERACTIVE=
