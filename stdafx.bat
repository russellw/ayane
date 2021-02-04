rem https://stackoverflow.com/questions/4726155/what-is-stdafx-h-used-for-in-visual-studio
rem https://www.viva64.com/en/b/0265/
if "%VCINSTALLDIR%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
cl /DDEBUG /EHsc /I\mpir /J /MTd /Yc /Zi /c stdafx.cpp
