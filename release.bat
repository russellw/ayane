if "%VCINSTALLDIR%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
cl /Feyane /I\mpir /J /O2 *.cc \mpir\lib\x64\Release\mpir.lib
