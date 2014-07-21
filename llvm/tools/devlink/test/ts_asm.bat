if %TSDL_TESTID%x==x (
	set TSDL_ERROR=TSDL_TESTID not set
	goto error
)
if %TSDL_TESTCASE_INIT_DONE%x==x (
	set TSDL_ERROR=
	if exist %TSDL_TESTID%.all_hsail del %TSDL_TESTID%.all_hsail
	if exist %TSDL_TESTID%.all_dump  del %TSDL_TESTID%.all_dump
        if not _%2==_KEEP_ALL_DASM_HSAIL (
		if exist %TSDL_TESTID%.all_dasm_hsail del %TSDL_TESTID%.all_dasm_hsail
	)
	set TSDL_TESTCASE_INIT_DONE=YES
)
if %1x == x (
	set TSDL_ERROR=No file suffix for %TSDL_TESTID%
	goto error
)
if not "%TSDL_ERROR%"x == ""x (
goto exit
)
if not _==_%TSDL_DEVLINK_IBIF% set TSDL_ASM_OPTS=%TSDL_ASM_OPTS% -bif
if not _==_%TSDL_DEVLINK_IBIF% set TSDL_DASM_OPTS=%TSDL_DASM_OPTS% -bif
set TSDL_ASM_FILE=%TSDL_TESTID%%1
if exist %TSDL_ASM_FILE%.brig del %TSDL_ASM_FILE%.brig
%TSDL_SHOW_ON%
%TSDL_BIN%\hsailasm -assemble %TSDL_ASM_FILE%.hsail -o=%TSDL_ASM_FILE%.brig %TSDL_ASM_OPTS% -debug 1>%TSDL_ASM_FILE%.asm.log
%TSDL_SHOW_OFF%
if not exist %TSDL_ASM_FILE%.brig (
	set TSDL_ERROR=Assembly failed, %TSDL_ASM_FILE%.brig not created
	type %TSDL_ASM_FILE%.asm.log
	goto error
)
%TSDL_SHOW_ON%
type %TSDL_ASM_FILE%.hsail >> %TSDL_TESTID%.all_hsail
%TSDL_BIN%\hsailasm -disassemble %TSDL_ASM_FILE%.brig -o=%TSDL_ASM_FILE%.dasm_hsail -debug %TSDL_DASM_OPTS% >> %TSDL_TESTID%.all_dump
%TSDL_SHOW_OFF%
if not exist %TSDL_ASM_FILE%.dasm_hsail (
	set TSDL_ERROR=Disassembly failed, %TSDL_ASM_FILE%.dasm_hsail not reconstructed
	goto error
)
%TSDL_SHOW_ON%
type %TSDL_ASM_FILE%.dasm_hsail >> %TSDL_TESTID%.all_dasm_hsail
%TSDL_SHOW_OFF%
set TSDL_DEVLINK_FILES=%TSDL_DEVLINK_FILES% %TSDL_ASM_FILE%.brig
goto exit
:error
echo * * * ERROR [%TSDL_TESTID%] %TSDL_ERROR%
:exit
