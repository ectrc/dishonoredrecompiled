@echo off
rem Configure + build the RELEASE game exe (NDEBUG, FINAL_RELEASE=0: logging kept, FMallocBinned, no FMallocDebug).
rem Long d3d9 runs need this: the debug allocator plus the resident cooked shader caches exhaust the 32-bit heap.
rem Configure + build the full game exe (real Launch, D3D9Drv, and the generated GFxUI/AkAudio/OSS/DishonoredGame
rem registrant modules) inside the VS 2022 x86 environment. Works from cmd, PowerShell or Git Bash:
rem   resources\build-game.cmd                 -> build\game, target DishonoredGame
rem   resources\build-game.cmd CoreSmoke       -> another target
rem   set BUILD_DIR=build\other before calling to use a different build directory
setlocal
set TARGET=%1
if "%TARGET%"=="" set TARGET=DishonoredGame
if "%BUILD_DIR%"=="" set BUILD_DIR=build\release
set VSDEVCMD="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
if not exist %VSDEVCMD% set VSDEVCMD="C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"
call %VSDEVCMD% -arch=x86 -host_arch=x64 -no_logo || exit /b 1
cd /d "%~dp0.."
cmake -S . -B %BUILD_DIR% -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-x86.cmake -DDISHONORED_REFERENCE_DIR=D:/RecompileDishonored/UnrealEngine3 -DDISHONORED_REAL_LAUNCH=ON -DDISHONORED_ENABLE_GFXUI=ON -DDISHONORED_ENABLE_AKAUDIO=ON -DDISHONORED_ENABLE_OSS=ON -DDISHONORED_ENABLE_DISHONOREDGAME=ON || exit /b 1
cmake --build %BUILD_DIR% --target %TARGET% -- -k 0
