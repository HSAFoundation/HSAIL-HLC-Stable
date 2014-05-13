@echo off
if not exist test.bat goto helpenv
::
::  -no-deduper                         - Do not perform the DeDuper step
::  -no-dedeader
::  -debug-parser                       - Debug Parser
::  -debug-dedeader                     - Debug DeDeader
::  -debug-deduper                      - Debug DeDuper
::  -detail-debug-parser                - Detail Debug Parser
::  -detail-debug-dedeader              - Detail Debug DeDeader
::
::Remove "rem" for debugging this batch
set TSDL_SHOW=rem
set TSDL_SHOW_ON=%TSDL_SHOW% echo on
set TSDL_SHOW_OFF=%TSDL_SHOW% @echo off
::
set TSDL_BIN=..\..\..\..\..\dist\windows\debug\bin\x86_64
if %TSDL_BUILD_UNDER_TEST%_==_ (
	set TSDL_BIN_UNDER_TEST=%TSDL_BIN%
) else (
	echo ###########################################################################
	if %TSDL_BUILD_UNDER_TEST_RELEASE%_==_ (
		set TSDL_BIN_UNDER_TEST=..\..\..\..\..\dist\windows\debug\bin\%TSDL_BUILD_UNDER_TEST%
		echo # Testing binary in debug\bin\%TSDL_BUILD_UNDER_TEST%
	) else (
		set TSDL_BIN_UNDER_TEST=..\..\..\..\..\dist\windows\release\bin\%TSDL_BUILD_UNDER_TEST%
		echo # Testing binary in release\bin\%TSDL_BUILD_UNDER_TEST%
	)
)
set TSDL_DEVLINK_OPTS=-detail-debug-parser -debug-deduper -detail-debug-dedeader
set TSDL_DEVLINK_OPTS_4HUGE=-debug-parser -debug-deduper -debug-dedeader
if %TSDL_BUILD_UNDER_TEST_RELEASE%_==_ set TSDL_DEVLINK_OPTS=%TSDL_DEVLINK_OPTS% -debug
if %TSDL_BUILD_UNDER_TEST_RELEASE%_==_ set TSDL_DEVLINK_OPTS_4HUGE=%TSDL_DEVLINK_OPTS_4HUGE% -debug
set TSDL_ASM_FILE=
set TSDL_DEVLINK_FILES=
set TSDL_TESTCASE_INIT_DONE=
set TSDL_ONE_PASS_OPTIMIZATION=
set TSDL_PRODUCE_LIBRARY=
set TSDL_PRODUCE_INVALID_BRIG=
set TSDL_DEVLINK_OBIF=
set TSDL_DEVLINK_IBIF=
set TSDL_ASM_OPTS=
set TSDL_DASM_OPTS=
set TSDL_LINKED_DASM_OPTS=
set TSDL_LINKED_ASM_OPTS=
set TSDL_NEGATIVE_TEST=
if %1x==x (
set TSDL_TESTID=help
rem set TSDL_TESTID=test_2m_simplest
) else (
set TSDL_TESTID=%1
)
if not %TSDL_TESTID%x==helpx (
echo ###########################################################################
echo #
echo # TARGET: %TSDL_TESTID%
echo #
echo ###########################################################################
)
goto %TSDL_TESTID%
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:unopt
set TSDL_DEVLINK_OPTS=%TSDL_DEVLINK_OPTS% -no-dedeader -no-deduper
call ts_asm _1
call ts_asm _2
goto link
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:debug_info
set TSDL_ASM_OPTS=%TSDL_ASM_OPTS% -g
set TSDL_DEVLINK_OPTS=%TSDL_DEVLINK_OPTS% -link-dwarf
call ts_asm _1
call ts_asm _2
goto link
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:unopt_big
if not exist amp_libm2.lib_brig cmd /c %0 amp_libm2 & echo.
call ts_asm _1
type amp_libm2.gold_dasm_hsail >> %TSDL_TESTID%.all_dasm_hsail
set TSDL_DEVLINK_FILES=%TSDL_DEVLINK_FILES% amp_libm2.lib_brig
set TSDL_DEVLINK_OPTS=%TSDL_DEVLINK_OPTS% -no-dedeader -no-deduper
goto link
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:aa1_libm2
:aa2_libm2
if not exist amp_libm2.lib_brig cmd /c %0 amp_libm2 & echo.
set TSDL_DEVLINK_OPTS=%TSDL_DEVLINK_OPTS_4HUGE%
set TSDL_DEVLINK_FILES=%TSDL_DEVLINK_FILES% amp_libm2.lib_brig
call ts_asm _1
goto link
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:invalid_lib1
set TSDL_PRODUCE_INVALID_BRIG=YES
set TSDL_PRODUCE_LIBRARY=YES
set TSDL_DEVLINK_OPTS=%TSDL_DEVLINK_OPTS% -disable-output-validator
call ts_asm _1
goto link
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:amp_libm2
set TSDL_DEVLINK_OPTS=%TSDL_DEVLINK_OPTS_4HUGE%
set TSDL_PRODUCE_LIBRARY=YES
call ts_asm _1
goto link
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:lib1
set TSDL_PRODUCE_LIBRARY=YES
call ts_asm _1
call ts_asm _2
goto link
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:vp1
set TSDL_PRODUCE_LIBRARY=YES
:vp2
call ts_asm _1
call ts_asm _2
call ts_asm _3
goto link
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:invalid_out1
set TSDL_PRODUCE_INVALID_BRIG=YES
set TSDL_NEGATIVE_TEST=yes
:matrix_multiply_amp
:mp1
:bug20120912a_org
:dirblock_opt
:o_function_list_1
:dpcel
call ts_asm _1
goto link
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:invalid_in1
if not exist invalid_lib1.lib_brig cmd /c %0 invalid_lib1 & echo.
set TSDL_DEVLINK_FILES=%TSDL_DEVLINK_FILES% invalid_lib1.lib_brig
set TSDL_NEGATIVE_TEST=yes
call ts_asm _1
goto link
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:a1bif
set TSDL_DEVLINK_IBIF=yes
set TSDL_DEVLINK_OBIF=yes
:a1
:test_2m_simplest
:bug20120716_org
:bug20120716_min
:decl_glob_f1
:decl_loc_s1
:decl_glob_s1
:ddo_immed
:ddo_indirect
call ts_asm _1
call ts_asm _2
goto link
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:mult_defs_01neg
set TSDL_NEGATIVE_TEST=yes
call ts_asm _1
call ts_asm _2
call ts_asm _3
call ts_asm _4
goto link
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:decl_glob_neg
set TSDL_NEGATIVE_TEST=yes
call ts_asm _1
call ts_asm _2
call ts_asm _3
goto link
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:versions_neg
:kernel_neg
set TSDL_NEGATIVE_TEST=yes
:decl_loc_f1
call ts_asm _1
call ts_asm _2
goto link
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:link
if exist %TSDL_TESTID%.dasm_brig  del %TSDL_TESTID%.dasm_brig 
if exist %TSDL_TESTID%.dasm_hsail del %TSDL_TESTID%.dasm_hsail
if exist %TSDL_TESTID%.brig del %TSDL_TESTID%.brig
if exist %TSDL_TESTID%.dump del %TSDL_TESTID%.dump
if exist %TSDL_TESTID%.devlink.log del %TSDL_TESTID%.devlink.log
if not "%TSDL_ERROR%"x == ""x (
	if _==_%TSDL_NEGATIVE_TEST% (
		echo * * * ERROR [%TSDL_TESTID%] Asm or Dasm failed. Test canceled.
	) else (
		echo * * * ERROR [NEG %TSDL_TESTID%] Asm or Dasm failed. Test canceled.
	)
	pause
	goto exit
)
set TSDL_DEVLINK_OPTS=%TSDL_DEVLINK_OPTS% -o=%TSDL_TESTID%.brig
if not _==_%TSDL_ONE_PASS_OPTIMIZATION%			set TSDL_DEVLINK_OPTS=%TSDL_DEVLINK_OPTS% -disable-recursive-optimization
if not _==_%TSDL_PRODUCE_LIBRARY%			set TSDL_DEVLINK_OPTS=%TSDL_DEVLINK_OPTS% -library
if     _==_%TSDL_PRODUCE_LIBRARY%			set TSDL_LINKED_DASM_OPTS=%TSDL_LINKED_DASM_OPTS% -validate-linked-code
if not _==_%TSDL_DEVLINK_IBIF%				set TSDL_DEVLINK_OPTS=%TSDL_DEVLINK_OPTS% -ibif
if not _==_%TSDL_DEVLINK_OBIF%				set TSDL_DEVLINK_OPTS=%TSDL_DEVLINK_OPTS% -obif
if not _==_%TSDL_DEVLINK_OBIF%				set TSDL_LINKED_DASM_OPTS=%TSDL_LINKED_DASM_OPTS% -bif
if not _==_%TSDL_PRODUCE_INVALID_BRIG%			set TSDL_DEVLINK_OPTS=%TSDL_DEVLINK_OPTS% -debug-remove-declarations
::FIXME
if not _==_%TSDL_PRODUCE_INVALID_BRIG%			set TSDL_DEVLINK_OPTS=%TSDL_DEVLINK_OPTS% -debug-corrupt-offsets

%TSDL_SHOW_ON%
%TSDL_BIN_UNDER_TEST%\devlink %TSDL_DEVLINK_FILES% %TSDL_DEVLINK_OPTS% > %TSDL_TESTID%.devlink.log 2>%TSDL_TESTID%.devlink.2_log
%TSDL_SHOW_OFF%
if not exist %TSDL_TESTID%.devlink.gold_2_log (
	type %TSDL_TESTID%.devlink.2_log
) else (
	if %TSDL_BUILD_UNDER_TEST_RELEASE%_==_ (
		tail -n+2 %TSDL_TESTID%.devlink.2_log > %TSDL_TESTID%.devlink.2_log.4diff
	) else (
		copy %TSDL_TESTID%.devlink.2_log %TSDL_TESTID%.devlink.2_log.4diff >nul
	)
	call ts_diff %TSDL_TESTID%.devlink.gold_2_log %TSDL_TESTID%.devlink.2_log.4diff
)
if exist %TSDL_TESTID%.devlink.gold_log (
	call ts_diff %TSDL_TESTID%.devlink.gold_log %TSDL_TESTID%.devlink.log
)
if not exist %TSDL_TESTID%.brig (
	if _==_%TSDL_NEGATIVE_TEST% (
		echo * * * ERROR [%TSDL_TESTID%] Devlink failed [.brig not found]
		type %TSDL_TESTID%.devlink.log
		pause
	) else (
		echo * OK   [NEG %TSDL_TESTID%] Devlink failed
		goto exit
	)
) else (
	if _==_%TSDL_NEGATIVE_TEST% (
		echo * OK   [%TSDL_TESTID%] Devlink succeeded [.brig created]
	) else (
		echo * * * ERROR [NEG %TSDL_TESTID%] Devlink succeeded [.brig created]
		pause
		goto exit
	)
	if not _==_%TSDL_PRODUCE_LIBRARY% (
		copy %TSDL_TESTID%.brig %TSDL_TESTID%.lib_brig 1>nul
		if not exist %TSDL_TESTID%.lib_brig (
			echo * * * ERROR [%TSDL_TESTID%] File copy failed [.brig to .lib_brig]
			pause
		)
	)
	%TSDL_SHOW_ON%
	%TSDL_BIN%\hsailasm -disassemble %TSDL_TESTID%.brig -o=%TSDL_TESTID%.dasm_hsail -debug %TSDL_LINKED_DASM_OPTS% > %TSDL_TESTID%.dump
	%TSDL_SHOW_OFF%
	if exist %TSDL_TESTID%.dasm_hsail (
		if _==_%TSDL_PRODUCE_INVALID_BRIG% (
			echo * OK   [%TSDL_TESTID%] Dasm OK
		) else (
			echo * * * ERROR [INV %TSDL_TESTID%] Dasm OK
			pause
			goto exit
		)
	) else (
		if _==_%TSDL_PRODUCE_INVALID_BRIG% (
			echo * * * ERROR [%TSDL_TESTID%] Dasm failed [.dasm_hsail not found]
			pause
		) else (
			echo * OK   [INV %TSDL_TESTID%] Dasm failed [.dasm_hsail not found]
			goto exit
		)
	)
	%TSDL_SHOW_ON%
	%TSDL_BIN%\hsailasm -assemble %TSDL_TESTID%.dasm_hsail -o=%TSDL_TESTID%.dasm_brig -debug %TSDL_LINKED_ASM_OPTS% > %TSDL_TESTID%.dasm_dump
	%TSDL_SHOW_OFF%
	if exist %TSDL_TESTID%.dasm_brig (
		echo * OK   [%TSDL_TESTID%] Asm linked/dasmed HSAIL
	) else (
		echo * * * ERROR [%TSDL_TESTID%] Asm linked/dasmed HSAIL [.dasm_brig not found]
		pause
	)
)
if exist %TSDL_TESTID%.gold_dasm_hsail (
	call ts_diff %TSDL_TESTID%.gold_dasm_hsail %TSDL_TESTID%.dasm_hsail
)
if exist %TSDL_TESTID%.dump (
	if exist %TSDL_TESTID%.gold_dump (
	        %TSDL_SHOW% echo ^> fc /L /N /W %TSDL_TESTID%.gold_dump %TSDL_TESTID%.dump
		fc /L /N /W %TSDL_TESTID%.gold_dump %TSDL_TESTID%.dump >nul
		if ERRORLEVEL 1 (
			%TSDL_SHOW% echo ^> p4 edit %TSDL_TESTID%.gold_dump 
			p4 edit %TSDL_TESTID%.gold_dump 
			if not exist %TSDL_TESTID%.gold_dasm_hsail (
				echo * * * INFO [%TSDL_TESTID%] .gold_dasm_hsail does not exist in the testsuite.
				echo * * * INFO [%TSDL_TESTID%] .all_dasm_hsail used instead.
				pause
				call ts_diff %TSDL_TESTID%.all_dasm_hsail %TSDL_TESTID%.dasm_hsail
			)
		)
		call ts_diff %TSDL_TESTID%.gold_dump %TSDL_TESTID%.dump
	) else (
		rem ======================================================================
		rem When .dump file is too big or manual verification of .dump file is too
		rem complicated, it is OK to use golden disassembled file for validation.
		rem Example of such a case is libm-related testcases, which produce very
		rem big dumps which is impractical to analyze.
		rem ======================================================================
		echo * INFO [%TSDL_TESTID%] There is no golden dump.
		if not exist %TSDL_TESTID%.gold_dasm_hsail (
			echo * * * WARN [%TSDL_TESTID%] .gold_dasm_hsail does not exist in the testsuite.
			echo * * * INFO [%TSDL_TESTID%] .all_dasm_hsail used instead.
			pause
			call ts_diff %TSDL_TESTID%.all_dasm_hsail %TSDL_TESTID%.dasm_hsail
		)
	)
)
goto exit
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:clean_all
%TSDL_SHOW_ON%
del *.lib_brig  1>nul 2>&1
%TSDL_SHOW_OFF% 
:clean
%TSDL_SHOW_ON%
del *.brig  1>nul 2>&1
del *.dump  1>nul 2>&1
del *.all_dump       1>nul 2>&1
del *.all_hsail      1>nul 2>&1
del *.dasm_hsail     1>nul 2>&1
del *.dasm_brig      1>nul 2>&1
del *.dasm_dump      1>nul 2>&1
del *.all_dasm_hsail 1>nul 2>&1
del *.devlink.log    1>nul 2>&1
del *.devlink.2_log  1>nul 2>&1
del *.devlink.2_log.4diff  1>nul 2>&1
del *.ts_diff        	   1>nul 2>&1
del *.asm.log              1>nul 2>&1
:: clean obsolete tempfiles
if exist __tmp__.* del  __tmp__.*      1>nul 2>&1
if exist linked.brig  del linked.brig  1>nul 2>&1
if exist linked.hsail del linked.hsail 1>nul 2>&1
if exist allsrc.hsail del allsrc.hsail 1>nul 2>&1
if exist tmp.tmp del tmp.tmp           1>nul 2>&1
@echo.
%TSDL_SHOW_OFF%
goto exit
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:clean_vars
echo on
set TSDL_BIN=
set TSDL_DEVLINK_OPTS=
set TSDL_ASM_FILE=
set TSDL_DEVLINK_FILES=
set TSDL_TESTID=
set TSDL_DIFF_RC=
set TSDL_ERROR=
set TSDL_DIFF_INTERACTIVE=
set TSDL_TESTCASE_INIT_DONE=
set TSDL_PRODUCE_LIBRARY=
set TSDL_DEVLINK_OBIF=
set TSDL_DEVLINK_IBIF=
set TSDL_LINKED_DASM_OPTS=
set TSDL_SHOW=
set TSDL_SHOW_ON=
set TSDL_SHOW_OFF=
set TSDL_ONE_PASS_OPTIMIZATION=
set TSDL_NEGATIVE_TEST=
set TSDL_DEVLINK_OPTS_4HUGE=
set TSDL_LINKED_ASM_OPTS=
set TSDL_ASM_OPTS=
set TSDL_DASM_OPTS=
set TSDL_PRODUCE_INVALID_BRIG=
set TSDL_BIN_UNDER_TEST=
@echo.
@echo off
goto exit
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:clean_obsolete_vars
echo on
set TSDL_DIFF_MORE=
set TSDL_FIXME_VALIDATOR_UNSUPPORTED_LINKED=
set TSDL_FIXME_VALIDATOR_CALL_UNSUPPORTED=
set TSDL_DIFF=
set TSDL_FIXME_DEVLINK_UNUSED_EXTREF_NOT_REMOVED=
set FIXME_ASM_CALL_WORKAROUND=
set FIXME_LINKED_BRIG_DASM_WORKAROUND=
set FIXME_WORKAROUND_ASM_CALL_NOT_SUPPORTED=
set FIXME_WORKAROUND_DASM_CALL_NOT_SUPPORTED=
set HSABIN=
set DEVLINK_OPTS=
set DEVLINK_FILES=
set ASM_FILE=
set TEST_ID=
@echo.
@echo off
goto exit
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:helpenv
echo.
echo * * * ERROR: TEST.BAT not found in the current directory.
echo * * * INFO : CD to the directory where tests reside, then re-run tests.
goto exit
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:help
echo.
echo USAGE:
echo     %0.bat ^<TSDL_TESTID^>
echo.
echo ^<TSDL_TESTID^> is one of:
echo     test_2m_simplest    - very basic test, kernel+external function, 2 files.
echo     matrix_multiply_amp - copied from AMP test case, 1 file
echo     vp1                 - first test from Valery Pykhtin, 3 files
echo     vp2                 - similar to vp1, but in different order
echo     lib1                - first simple library test
echo     a1                  - >1 function agruments
echo     a1bif               - a1, but bif3.0 input/output
echo     decl_glob_f1        - resolving ^& decl removal of global functions
echo     decl_loc_f1         - resolving ^& decl removal of local functions
echo     decl_glob_s1        - resolving ^& decl removal of global symbols
echo     decl_loc_s1         - resolving ^& decl removal of local symbols
echo     mp1                 - multi-pass optimization
echo     invalid_lib1        - produces bad library
echo     invalid_in1         - trying to link with bad library (negative test)
echo     invalid_out1        - trying to produce bad brig (negative test)
echo     dirblock_opt        - optimizations of Brig Blocks
echo     o_function_list_1   - support for DirectiveFunctionList
echo     mult_defs_01neg     - multiple external definitions (negative)
echo     unopt               - linking with optimizations disabled
echo     dpcel               - for DirectivePragma/Control/Extension/Loc
echo     ddo_immed           - deDuping of immediate operands
echo     ddo_indirect        - deDuping of indirect operands
echo     amp_libm2           - produces libm library (most probably, obsolete)
echo     aa1_libm2           - link-with-libm test, created by aalanov
echo     aa2_libm2           - link-with-libm test, created by aalanov
echo     unopt_big           - big test of linking with optimizations disabled
echo     decl_glob_neg       - non-matching externals
echo     versions_neg        - non-matching module versions
echo     debug_info          - smoke for dwarf-linking
echo     kernel_neg          - multiple kernels (negative)
echo.
echo     bug20120716_min     - reduced bug20120716_org.
echo     bug20120912a_org    - bug found Sep 12 2012, original.
echo     bug20120716_org     - bug found Jul 16 2012, original.
echo.
echo     ---- special targets ---
echo     help, clean, clean_all (this also deletes device libs), 
echo     clean_vars, clean_obsolete_vars
echo.
echo Tips: What you may want to see/change in this file (%0.bat):
echo     - By default, x64 binaries are tested. 
echo       Set TSDL_BUILD_UNDER_TEST (and also TSDL_BUILD_UNDER_TEST_RELEASE
echo       to test release versions) env var before runnning the test
echo       if you want to change this. Example for testing binary from 
echo       "...release\bin\x86":
echo         ^> set TSDL_BUILD_UNDER_TEST=x86
echo         ^> set TSDL_BUILD_UNDER_TEST_RELEASE=y
echo     - Edit definition of TSDL_DEVLINK_OPTS if you want to change 
echo       the set of default options passed to devlink during tests.
echo     - See how TSDL_DIFF_INTERACTIVE is used.
goto exit
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
echo.
:exit
