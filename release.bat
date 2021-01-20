if "%VCINSTALLDIR%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"
cl /Feyane /I\mpir /J /O2 *.cc \mpir\lib\Win32\Release\mpir.lib
