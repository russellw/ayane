if "%VCINSTALLDIR%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
move *.asm \temp
cl /DDEBUG /EHsc /Feayane /I\mpir /J /MP4 /MTd /WX /Yustdafx.h /Zi *.cc stdafx.obj \mpir\dll\x64\Debug\mpir.lib dbghelp.lib >r:1
if errorlevel 1 goto :err
ayane %*
echo %errorlevel%
goto :eof

:err
head -n 50 r:1
