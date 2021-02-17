if "%VCINSTALLDIR%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
if [%~1]==[] goto :eof
cl /EHsc /Feayane /I\mpir /J /MTd /O2 /Zi *.cc \mpir\dll\x64\Debug\mpir.lib dbghelp.lib
"C:\Program Files (x86)\Intel\oneAPI\vtune\latest\bin64\vtune" -collect hotspots -user-data-dir \temp ayane.exe %*
