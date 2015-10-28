@echo off

if not exist %~dp0build mkdir %~dp0build
pushd %~dp0build

rem Compiler options: https://msdn.microsoft.com/en-us/library/fwkeyyhe.aspx
rem Linker options:   https://msdn.microsoft.com/en-US/library/y0zzbyt4.aspx

set DebugCompilerOptions=/MTd /Od /Oi /Z7
set ReleaseCompilerOptions=/MT /Ox
set CompilerOptions=%ReleaseCompilerOptions% /Gm- /GR- /EHa- /FC -fp:fast /D_HAS_EXCEPTIONS=0 /DHALE_STU
rem /opt:ref
set LinkerOptions=/subsystem:windows,6 /incremental:no

cl /nologo %CompilerOptions% ../src/hale/stu.cpp /link %LinkerOptions% /OUT:hale.exe

echo Done.

popd