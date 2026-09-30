@echo off
rem Build the user's dismod DLL (D:\Christmas\github\dismod) into build\debug.
rem See resources/docs/dismod_harness.md. dismod is NOT part of this repository.
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86 >nul
if errorlevel 1 (echo vcvarsall failed & exit /b 1)
cmake --build "D:\Christmas\github\dismod\build\debug" %*
echo dismod build exit %errorlevel%
