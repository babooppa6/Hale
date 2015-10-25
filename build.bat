@echo off
pushd src\hale
set CompilerOptions=/MTd /Gm- /GR- /Od /EHa /DHALE_STU /D_DEBUG
set LinkerOptions=/subsystem:windows,6 /incremental:no /opt:ref

cl /nologo %CompilerOptions% hale_stu.cpp /link %LinkerOptions% /OUT:hale.exe
popd