@echo off
if %HSAROOT%x==x goto helpenv
set TSFO_BIN=%HSAROOT%\dist\windows\debug\bin\x86_64
set TSFO_FIXME_VALIDATOR_CALL_UNSUPPORTED=-disable-validator
set TSFO_OPTS=
set TSFO_NEGATIVE_TEST=
set TSFO_RC=

if %1x==x (
set TSFO_TID=help
) else (
set TSFO_TID=%1
)
if not %TSFO_TID%x==helpx (
echo.
echo ###########################################################################
echo #
echo # TARGET: %TSFO_TID%
echo #
echo ###########################################################################
)
goto %TSFO_TID%
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:raw_1
:raw_2
set TSFO_OPTS=%TSFO_OPTS% -debug-elf-writeback %TSFO_TID%.brig -debug-o=%TSFO_TID%.out_elf
echo on
%TSFO_BIN%\fatobj %TSFO_OPTS%
@echo off
goto raw_compare
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:j_nohost
:j_none
set TSFO_NEGATIVE_TEST=y
:j_3
:j_2
set TSFO_OPTS=%TSFO_OPTS% -create %TSFO_TID%.fatobj
echo on
%TSFO_BIN%\fatobj %TSFO_OPTS%
@echo off
goto fat_compare
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:js1
:js2
set TSFO_OPTS=-join %TSFO_TID%
echo on
%TSFO_BIN%\fatobj %TSFO_OPTS% -debug-o=%TSFO_TID%_out
@echo off
set TSFO_OPTS=-split %TSFO_TID%_out
echo on
%TSFO_BIN%\fatobj %TSFO_OPTS%
@echo off
goto js_compare
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:js1opt
set TSFO_OPTS=-create %TSFO_TID%
echo on
%TSFO_BIN%\fatobj %TSFO_OPTS% -debug-o=%TSFO_TID%_out
@echo off
set TSFO_OPTS=-extract %TSFO_TID%_out
echo on
%TSFO_BIN%\fatobj %TSFO_OPTS%
@echo off
goto js_compare
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:d_nofile
set TSFO_GOLD_RC=130
goto d_compare
:d_bif
set TSFO_GOLD_RC=4
goto d_compare
:d_brig
set TSFO_GOLD_RC=3
goto d_compare
:d_fatobj
set TSFO_GOLD_RC=2
goto d_compare
:d_unknown
set TSFO_GOLD_RC=1
goto d_compare
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:d_compare
set TSFO_OPTS=-detect-type %TSFO_TID%.detect
echo on
%TSFO_BIN%\fatobj %TSFO_OPTS%
set TSFO_RC=%ERRORLEVEL%
@echo off
if %TSFO_RC%_==%TSFO_GOLD_RC%_ (
	echo * OK [%TSFO_TID%]: file type detection [%TSFO_RC% == %TSFO_GOLD_RC%]
) else (
	echo * * * ERROR [%TSFO_TID%]: file type detection [%TSFO_RC% != %TSFO_GOLD_RC%]
)
goto exit
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:js_compare
call ts_bindiff %TSFO_TID%.brig  %TSFO_TID%_out.brig
call ts_bindiff %TSFO_TID%.obj   %TSFO_TID%_out.obj
if exist %TSFO_TID%.opt call ts_bindiff %TSFO_TID%.opt  %TSFO_TID%_out.opt
goto exit
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:raw_compare
:: generate and compare readelf output
if not exist %TSFO_TID%.out_elf (
	echo * * * ERROR [%TSFO_TID%]: failed, .out_elf does not exist
	pause
	goto exit
)
readelf -a %TSFO_TID%.out_elf  > %TSFO_TID%.out_readelf
if not exist %TSFO_TID%.out_readelf (
	echo * * * ERROR [%TSFO_TID%]: failed, .out_readelf does not exist
	pause
	goto exit
)
if exist %TSFO_TID%.gold_readelf (
	call ts_diff %TSFO_TID%.gold_readelf %TSFO_TID%.out_readelf
) else (
	echo * * * INFO [%TSFO_TID%]: There is no golden readelf
	readelf -a %TSFO_TID%.brig > %TSFO_TID%.readelf
	if not exist %TSFO_TID%.readelf (
		echo * * * ERROR [%TSFO_TID%]: failed, .readelf does not exist
		echo * * * Input data for this test case is broken.
		pause
		goto exit
	)
	call ts_diff %TSFO_TID%.readelf %TSFO_TID%.out_readelf
)
:: generate and compare disassembled elf
if exist %TSFO_TID%.out_dasm_hsail del %TSFO_TID%.out_dasm_hsail
echo on
%TSFO_BIN%\hsailasm -disassemble %TSFO_TID%.out_elf -o=%TSFO_TID%.out_dasm_hsail %TSFO_FIXME_VALIDATOR_CALL_UNSUPPORTED%
@echo off
if not exist %TSFO_TID%.out_dasm_hsail (
	echo * * * ERROR [%TSFO_TID%]: disassembly failed, .out_dasm_hsail does not exist
	pause
	goto exit
)
if exist %TSFO_TID%.dasm_hsail del %TSFO_TID%.dasm_hsail
echo on
%TSFO_BIN%\hsailasm -disassemble %TSFO_TID%.brig -o=%TSFO_TID%.dasm_hsail %TSFO_FIXME_VALIDATOR_CALL_UNSUPPORTED%
@echo off
if not exist %TSFO_TID%.dasm_hsail (
	echo * * * ERROR [%TSFO_TID%]: disassembly failed, .dasm_hsail does not exist
	echo * * * Input data for this test case is broken.
	pause
	goto exit
)
call ts_diff %TSFO_TID%.dasm_hsail %TSFO_TID%.out_dasm_hsail
goto exit
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:fat_compare
:: generate and compare readelf output
if %TSFO_NEGATIVE_TEST%_==_ (
	if not exist %TSFO_TID%.fatobj (
		echo * * * ERROR [%TSFO_TID%]: failed, .fatobj does not exist
		pause
		goto exit
	)
) else (
	if not exist %TSFO_TID%.fatobj (
		echo * OK [NEG %TSFO_TID%]: .fatobj does not exist
	) else (
		echo * * * ERROR [NEG %TSFO_TID%]: .fatobj exists!
		pause
		del %TSFO_TID%.fatobj
	)
	goto exit
)
readelf -a %TSFO_TID%.fatobj  > %TSFO_TID%.out_readelf
if not exist %TSFO_TID%.out_readelf (
	echo * * * ERROR [%TSFO_TID%]: failed, .out_readelf does not exist
	pause
	goto exit
)
if exist %TSFO_TID%.gold_readelf (
	call ts_diff %TSFO_TID%.gold_readelf %TSFO_TID%.out_readelf
) else (
	echo * * * INFO [%TSFO_TID%]: There is no golden readelf.
	pause
	type %TSFO_TID%.out_readelf
	rem call ts_diff %TSFO_TID%.out_readelf
)
goto exit
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:clean_all
echo on
@goto clean_com
:clean
echo on
:clean_com
del *.out_elf 		1>nul 2>&1
del *.out_readelf 	1>nul 2>&1
del *.readelf      	1>nul 2>&1
del *.out_dump     	1>nul 2>&1
del *.out_dasm_hsail 	1>nul 2>&1
del *.dasm_hsail 	1>nul 2>&1
del j_*.fatobj 		1>nul 2>&1
del *_out.fatobj     	1>nul 2>&1
del *_out.brig     	1>nul 2>&1
del *_out.obj     	1>nul 2>&1
del *_out.opt     	1>nul 2>&1
del *.ts_diff		1>nul 2>&1
:: clean obsolete tempfiles
del temp.elf		1>nul 2>&1
@echo off
goto exit
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:clean_vars
echo on
set TSFO_BIN=
set TSFO_FIXME_VALIDATOR_CALL_UNSUPPORTED=
set TSFO_OPTS=
set TSFO_TID=
set TSFO_DIFF_RC=
set TSFO_NEGATIVE_TEST=
set TSFO_RC=
@echo off
goto exit
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:clean_obsolete_vars
echo on
:: none yet
@echo off
goto exit
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:helpenv
echo.
echo * * * ERROR: Envronment variables are not set up.
goto help
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:help
echo.
echo USAGE:
echo     %0.bat ^<TEST_ID^>
echo.
echo ^<TEST_ID^> is one of:
echo     raw_1, raw_2, j_3, j_none, js1, js2, js1opt, j_nohost, j_2
echo     d_brig, d_bif, d_fatobj, d_unknown, d_nofile
echo.
echo     --- special targets ---
echo     help, clean, clean_all, 
echo     clean_vars, clean_obsolete_vars
goto exit
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
echo.
:exit
