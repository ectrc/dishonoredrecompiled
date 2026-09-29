@echo off
rem Build the final playable executable and stage it into the retail tree.
rem
rem   resources\build-play.cmd              -> Release configuration, switches baked in
rem   resources\build-play.cmd shipping     -> adds FINAL_RELEASE/SHIPPING_PC_GAME/NO_LOGGING
rem
rem The result is DishonoredGame-Win64-Shipping.exe in the retail Binaries\Win32, and it needs no
rem command line: appInit appends -gfxuimenu -nosteam -skipnativepkgs=OnlineSubsystemPC -nomovie
rem -startmap=Dishonored_MainMenu -startmapopen -windowed -ResX=1600 -ResY=900 when they are absent
rem (Core/Src/UnMisc.cpp, DISHONORED_PLAY_DEFAULTS). Anything you pass yourself wins, so
rem   DishonoredGame-Win64-Shipping.exe -startmap=L_Pub_Day_P
rem opens the pub instead of the menu.
rem
rem NOTE ON THE NAME: this executable is 32-bit (x86), like the retail game. The middleware it links
rem against - PhysX 2.8.4, Bink, Wwise 2012, Scaleform 3.3 - ships only as 32-bit DLLs in the retail
rem tree, and the 2,314 layout static_asserts that keep this a faithful recompilation are written
rem against retail's 32-bit struct offsets under /Zp4. The Win64 in the name is the requested name,
rem not a description of the binary.
setlocal
set CFG=%1
if "%CFG%"=="" set CFG=release

set SHIPPING=OFF
if /I "%CFG%"=="shipping" set SHIPPING=ON

if "%BUILD_DIR%"=="" set BUILD_DIR=build\play
set VSDEVCMD="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
if not exist %VSDEVCMD% set VSDEVCMD="C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"
call %VSDEVCMD% -arch=x86 -host_arch=x64 -no_logo || exit /b 1
cd /d "%~dp0.."

cmake -S . -B %BUILD_DIR% -G Ninja -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-x86.cmake ^
  -DDISHONORED_REFERENCE_DIR=D:/RecompileDishonored/UnrealEngine3 ^
  -DDISHONORED_REAL_LAUNCH=ON -DDISHONORED_ENABLE_GFXUI=ON -DDISHONORED_ENABLE_AKAUDIO=ON ^
  -DDISHONORED_ENABLE_OSS=ON -DDISHONORED_ENABLE_DISHONOREDGAME=ON ^
  -DDISHONORED_PLAY_DEFAULTS=ON -DDISHONORED_SHIPPING=%SHIPPING% || exit /b 1
cmake --build %BUILD_DIR% --target DishonoredGame -- -k 0 || exit /b 1

python resources\tools\stage_retail.py --build-dir %BUILD_DIR% --exe-name DishonoredGame-Win64-Shipping.exe || exit /b 1

echo.
echo Playable build staged:
echo   D:\RecompileDishonored\Dishonored_Latest2026\Binaries\Win32\DishonoredGame-Win64-Shipping.exe
echo Run it with no arguments.
