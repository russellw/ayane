rem MPIR is the Windows-friendly port of GMP for arbitrary precision arithmetic
rem At the time of writing, the latest version can be downloaded from:
rem https://github.com/BrianGladman/mpir
rem The following parameters can be adjusted for architecture and compiler version

pushd \mpir\msvc\vs19
call msbuild.bat gc DLL x64 Debug
call msbuild.bat gc LIB x64 Release
popd
copy \mpir\dll\x64\Debug\mpir.dll
