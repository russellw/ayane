if "%VCINSTALLDIR%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
move *.asm \temp
cl /DDEBUG /EHsc /Feayane /I\mpir /J /MP4 /MTd /Yustdafx.h /Zi *.cc stdafx.obj \mpir\dll\x64\Debug\mpir.lib dbghelp.lib
if errorlevel 1 goto :eof
ayane %*
echo %errorlevel%
