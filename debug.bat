if "%VCINSTALLDIR%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
move *.asm \temp

rem Try building with both clang and Microsoft C++
rem for two chances to find compile-time errors
clang -DDEBUG -I\mpir -Wall -Werror -Wno-deprecated-declarations -c *.cc
if errorlevel 1 goto :eof
del *.o

cl /DDEBUG /EHsc /Feayane /I\mpir /J /MTd /Zi *.cc \mpir\dll\x64\Debug\mpir.lib dbghelp.lib
if errorlevel 1 goto :eof

ayane %*
