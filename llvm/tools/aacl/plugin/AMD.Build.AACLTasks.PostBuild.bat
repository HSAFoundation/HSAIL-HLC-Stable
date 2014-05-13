REM %1 - PLATFORM (x64 or win32)
REM %2 - $(TargetPath) - absolute path to output .dll including filename
REM %3 - $(TargetFileName) - only filename of output .dll
REM %4 - $(ProjectDir) - absolute path to project dir where .csproj resides

SET PLATF=%1
SET TARGETPATH="%2"
SET TARGETFILENAME=%3
SET PROJECTDIR=%4

SET TOOLSETDIR=%ProgramFiles(x86)%\MSBuild\Microsoft.Cpp\v4.0\Platforms\%PLATF%\PlatformToolsets\AMD AMP Compiler 0.1\
SET XML_DIR=%PROJECTDIR%..\MSBuild\%PLATF%\AMD AMP Compiler 0.1\
SET PROPS_FILE_NAME=Microsoft.Cpp.%PLATF%.AMD AMP Compiler 0.1.props
SET PROPS_FILE="%XML_DIR%%PROPS_FILE_NAME%"
SET TARGETS_FILE_NAME=Microsoft.Cpp.%PLATF%.AMD AMP Compiler 0.1.targets
SET TARGETS_FILE="%XML_DIR%%TARGETS_FILE_NAME%"

IF NOT EXIST "%TOOLSETDIR%" MKDIR "%TOOLSETDIR%"
IF NOT EXIST "%TOOLSETDIR%1033" MKDIR "%TOOLSETDIR%1033"
IF %PLATF%==x64 (IF NOT EXIST "%TOOLSETDIR%ImportBefore" MKDIR "%TOOLSETDIR%ImportBefore")

ECHO.
ECHO ======== AMD AMP Compiler %PLATF% VS Plugin Installation ========
REM  ======== DLL COPYING ========
ECHO COPYING: %TARGETPATH% TO
ECHO          "%TOOLSETDIR%%TARGETFILENAME%" 
COPY %TARGETPATH% "%TOOLSETDIR%%TARGETFILENAME%"

REM ======== .PROPS COPYING ========
ECHO COPYING: %PROPS_FILE% TO
ECHO          "%TOOLSETDIR%%PROPS_FILE_NAME%" 
COPY %PROPS_FILE% "%TOOLSETDIR%%PROPS_FILE_NAME%"

REM ======== .TARGETS COPYING ========
ECHO COPYING: %TARGETS_FILE% TO
ECHO          "%TOOLSETDIR%%TARGETS_FILE_NAME%" 
COPY %TARGETS_FILE% "%TOOLSETDIR%%TARGETS_FILE_NAME%"

REM ======== aacl.xml COPYING ========
ECHO COPYING: "%XML_DIR%1033\aacl.xml" TO
ECHO          "%TOOLSETDIR%1033\aacl.xml" 
COPY "%XML_DIR%1033\aacl.xml" "%TOOLSETDIR%1033\aacl.xml"

REM ======== aalink.xml COPYING ========
ECHO COPYING: "%XML_DIR%1033\aalink.xml" TO
ECHO          "%TOOLSETDIR%1033\aalink.xml" 
COPY "%XML_DIR%1033\aalink.xml" "%TOOLSETDIR%1033\aalink.xml"

REM ======== AMD.Project.General.xml COPYING ========
ECHO COPYING: "%XML_DIR%1033\AMD.Project.General.xml" TO
ECHO          "%TOOLSETDIR%1033\AMD.Project.General.xml" 
COPY "%XML_DIR%1033\AMD.Project.General.xml" "%TOOLSETDIR%1033\AMD.Project.General.xml"

REM ======== aalink.xml COPYING ========
IF %PLATF%==x64 (
ECHO COPYING: "%XML_DIR%ImportBefore\AMD.Lib.x64.AMD AMP Compiler 0.1.targets" TO
ECHO          "%TOOLSETDIR%ImportBefore\AMD.Lib.x64.AMD AMP Compiler 0.1.targets" 
COPY "%XML_DIR%ImportBefore\AMD.Lib.x64.AMD AMP Compiler 0.1.targets" "%TOOLSETDIR%ImportBefore\AMD.Lib.x64.AMD AMP Compiler 0.1.targets" )