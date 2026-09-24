@echo off
rem Opens the VS 2022 x86 environment and runs the given cmake preset.
rem   run-vcvars.cmd x86-debug            configure + build
rem   run-vcvars.cmd x86-debug configure  configure only
setlocal
set PRESET=%1
if "%PRESET%"=="" set PRESET=x86-debug
set VSDEVCMD="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
if not exist %VSDEVCMD% set VSDEVCMD="C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"
call %VSDEVCMD% -arch=x86 -host_arch=x64 -no_logo || exit /b 1
cd /d "%~dp0"
cmake --preset %PRESET% || exit /b 1
if /i "%2"=="configure" exit /b 0
cmake --build --preset %PRESET%
