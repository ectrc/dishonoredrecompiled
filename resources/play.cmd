@echo off
rem Launch our build into a level, windowed, against the retail content.
rem
rem   resources\play.cmd                  -> L_Tower_P (first mission map), 1280x720
rem   resources\play.cmd L_Pub_Day_P      -> another map
rem   resources\play.cmd L_Tower_P 1920 1080
rem
rem The exe must sit next to the retail Dishonored.exe, because the engine finds ..\..\DishonoredGame and
rem ..\..\Engine relative to itself exactly as retail does; stage_retail.py copies it there and touches
rem nothing else. Set BUILD_DIR to launch a different build (default: the Release build, which is what long
rem rendered runs need - a _DEBUG build uses FMallocDebug and exhausts the 32-bit heap with the shader
rem caches resident).
rem
rem What works today: the map loads and streams its sub-levels, the world renders, the player pawn is
rem possessed, the mouse turns the view and the keys are bound.
rem What does not yet: the main menu (Scaleform is off, hence -startmap), audio is a silent backend, and the
rem pawn falls through the floor because it spawns before the first sub-level becomes visible.
setlocal
set MAP=%1
if "%MAP%"=="" set MAP=L_Tower_P
set RESX=%2
if "%RESX%"=="" set RESX=1280
set RESY=%3
if "%RESY%"=="" set RESY=720
if "%BUILD_DIR%"=="" set BUILD_DIR=build\release
set RETAIL=D:\RecompileDishonored\Dishonored_Latest2026

cd /d "%~dp0.."
python resources\tools\stage_retail.py --build-dir %BUILD_DIR% --exe-name DishonoredGame_Play.exe || exit /b 1

echo.
echo Launching %MAP% at %RESX%x%RESY% from %BUILD_DIR% ...
echo Log: %RETAIL%\DishonoredGame\Logs\Launch.log
echo.
cd /d "%RETAIL%\Binaries\Win32"
DishonoredGame_Play.exe -startmap=%MAP% -startmapopen -nosteam -skipnativepkgs=OnlineSubsystemPC ^
  -windowed -ResX=%RESX% -ResY=%RESY% -nomovie -log -forcelogflush
