How to build and install AMD CPP compiler 0.1 plugin (debug version)* into Microsoft Visual Studio 11 (2012):

0. Build HSA tree.
   Make sure that the following tools executables exist in "hsa\dist\windows\debug\bin\x86" directory for 32bit & in "hsa\dist\windows\debug\bin\x86_64" directory for 64bit:

     aacl.exe
     aalink.exe
     ampfe.exe
     ampfe_host.exe
     devlink.exe
     fatobj.exe
     HSAILasm.exe
     inflate.exe
     llc.exe
     llvm-as.exe
     llvm-link.exe
     opt.exe

   Also make sure that the following libraries exist in "hsa\dist\windows\debug\lib\x86" directory for 32bit:

     amp_libm.bc
     builtins-hsail.bc
     hsa.lib
     hsacore.lib
     hsagpu.lib
     hsaperfmarker.lib

   and the following libraries exist in "hsa\dist\windows\debug\lib\x86_64" directory for 64bit:	

     amp_libm.bc
     builtins-hsail.bc
     hsa64.lib
     hsacore64.lib
     hsagpu64.lib
     hsaperfmarker64.lib

1. Set env AMD_CPP_COMPILER01 to point to <your_workspace_root>\drivers\hsa\dist\windows\debug\bin\x86\

2. Close all opened VS 11 IDEs.

3. Rebuild plugin\AMD.Build.AACLTasks.sln in VC10 SP1 under (Debug | AnyCPU)* Configuration | Platform
   In case of successful build plugin assembly and all plugin configuration files will be installed into corresponding MSBuild folder automatically on post-build event.

   1) win32 plugin files will be copied in %ProgramFiles(x86)%\MSBuild\Microsoft.Cpp\v4.0\Platforms\Win32\PlatformToolsets\
      subdir "AMD AMP Compiler 0.1"
              AMD.Build.AACLTasks.Win32.dll
              Microsoft.Cpp.win32.AMD AMP Compiler 0.1.props
              Microsoft.Cpp.win32.AMD AMP Compiler 0.1.targets
              subdir "1033"
                      aacl.xml
                      aalink.xml
                      AMD.Project.General.xml
	
   2) x64 plugin files will be copied in %ProgramFiles(x86)%\MSBuild\Microsoft.Cpp\v4.0\Platforms\x64\PlatformToolsets\
      subdir "AMD AMP Compiler 0.1"
              AMD.Build.AACLTasks.X64.dll	
              Microsoft.Cpp.X64.AMD AMP Compiler 0.1.props
              Microsoft.Cpp.X64.AMD AMP Compiler 0.1.targets
              subdir "1033"
                      aacl.xml
                      aalink.xml
                      AMD.Project.General.xml		
              ImportBefore
                      AMD.Lib.x64.AMD AMP Compiler 0.1.targets

4. Open your *.vcxproj in VS 11 and select "AMD AMP Compiler 0.1" in:
   Property Pages -> Configuration Properties -> General -> Platform Toolset

5. Rebuild your project.

* For release version please make sure that the foregoing files exist in corresponding "release" subfolders [0.]
  and rebuild plugin\AMD.Build.AACLTasks.sln under (Release | AnyCPU) Configuration | Platform [3.]
