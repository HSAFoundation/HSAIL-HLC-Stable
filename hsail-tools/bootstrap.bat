set BLDMODE=Debug_Win32
: set HSAILASM=HSAILAsm\build\w7\B_dbg\HSAILasm.exe 
set HSAILASM=build\Debug\x86\HSAILAsm.exe
rd /s bootstrap_test

mkdir bootstrap_test\
mkdir bootstrap_test\brig1
mkdir bootstrap_test\brig2
mkdir bootstrap_test\asm1out
mkdir bootstrap_test\asm2out
mkdir bootstrap_test\dis1
mkdir bootstrap_test\dis2
mkdir bootstrap_test\dis1out
mkdir bootstrap_test\dis2out

for %%i in (oclsdk\*.hsail) do call :sub %%i

:sub
%HSAILASM% -assemble -o bootstrap_test\brig1\%~n1.brig %1 >bootstrap_test\asm1out\%~n1.txt 2>&1
%HSAILASM% -disassemble -o bootstrap_test\dis1\%~n1.hsail bootstrap_test\brig1\%~n1.brig >bootstrap_test\dis1out\%~n1.txt 2>&1
%HSAILASM% -assemble -o bootstrap_test\brig2\%~n1.brig bootstrap_test\dis1\%~n1.hsail >bootstrap_test\asm2out\%~n1.txt 2>&1
%HSAILASM% -disassemble -o bootstrap_test\dis2\%~n1.hsail bootstrap_test\brig2\%~n1.brig >bootstrap_test\dis2out\%~n1.txt 2>&1
