fc /L /N /W %1 %2 1>%TSFO_TID%.ts_diff 2>&1
set TSFO_DIFF_RC=%ERRORLEVEL%
if %TSFO_DIFF_RC%x == 0x (
	echo * OK [%TSFO_TID%]: diff [%1 == %2]
	goto exit
) else (
	echo * * * ERROR [%TSFO_TID%]: diff [%1 != %2]
	if not "%TSFO_DIFF_INTERACTIVE%"x==""x (
		echo on
		%TSFO_DIFF_INTERACTIVE% %1 %2
		@echo off
	) else (
		type %TSFO_TID%.ts_diff
	)
	pause
)
:exit
