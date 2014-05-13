fc /B %1 %2 1>%TSFO_TID%.ts_diff 2>&1
set TSFO_DIFF_RC=%ERRORLEVEL%
if %TSFO_DIFF_RC%x == 0x (
	echo * OK [%TSFO_TID%]: binary diff [%1 == %2]
	goto exit
) else (
	echo * * * ERROR [%TSFO_TID%]: binary diff [%1 != %2]
	pause
)
:exit
