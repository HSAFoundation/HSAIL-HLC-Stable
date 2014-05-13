fc /L /N /W %1 %2 1>%TSDL_TESTID%.ts_diff 2>&1
set TSDL_DIFF_RC=%ERRORLEVEL%
if %TSDL_DIFF_RC%x == 0x (
	echo * OK   [%TSDL_TESTID%] diff [%1 == %2]
	goto exit
) else (
	echo * * * ERROR [%TSDL_TESTID%] diff [%1 != %2]
	if not "%TSDL_DIFF_INTERACTIVE%"x==""x (
		echo on
		%TSDL_DIFF_INTERACTIVE% %1 %2
		@echo off
		pause
	) else (
		type %TSDL_TESTID%.ts_diff
		pause
	)
)
:exit
