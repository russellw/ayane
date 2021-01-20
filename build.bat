if "%VCINSTALLDIR%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"

clang -I\mpir -Werror -Wno-deprecated-declarations -c *.cc
if errorlevel 1 goto :eof

cl /DDEBUG /Feayane /I\mpir /J /MTd /Zi *.cc dbghelp.lib
if errorlevel 1 goto :eof
