if "%VCINSTALLDIR%"=="" call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cl /DDEBUG /Feayane /IC:\mpir /MP /MTd /WX /Zi /std:c++17 C:\ayane\*.cc C:\mpir\debug.lib dbghelp.lib
