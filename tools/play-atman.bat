@echo off
setlocal
rem Plays Atman (the Advaita Vedanta game on the Strata engine) from the
rem latest main branch, building it first. There are no saves yet: each run
rem starts at 4:30 in the afternoon outside your house.
rem Controls: mouse look, W A S D walk, Shift hurry, E use (chores, eat, drink,
rem sleep), F lamp, 1-4 throw at the wheel, B break a pot, C clay lens, Tab
rem codex, V watch in the after-mode, F1 help, Esc frees the mouse.
rem Extra arguments are passed to the game, e.g.:
rem   play-atman.bat --tour                  watch the fixed tour through the valley and the codex
rem   play-atman.bat --autopilot --speed 10  watch the autopilot play the whole slice, ten times as fast

set "AT_DIR=%USERPROFILE%\atman-play"
set "AT_REPO=https://github.com/Preponderous-Software/atman.git"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "INSTALLED="

where git >nul 2>nul || (echo Git is not installed: https://git-scm.com/download/win & pause & exit /b 1)

where cmake >nul 2>nul || (
    echo Atman needs CMake 3.24 or newer. Installing it with winget ...
    winget install --id Kitware.CMake -e --accept-source-agreements --accept-package-agreements || (echo Install it from https://cmake.org/download/ & pause & exit /b 1)
    set "INSTALLED=1"
)

set "HAS_MSVC="
if exist "%VSWHERE%" (
    for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "HAS_MSVC=1"
)
if not defined HAS_MSVC (
    echo Atman needs the Visual Studio C++ build tools. Installing them with winget, this takes a while ...
    winget install --id Microsoft.VisualStudio.2022.BuildTools -e --accept-source-agreements --accept-package-agreements --override "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended" || (echo Install "Desktop development with C++" from https://visualstudio.microsoft.com/visual-cpp-build-tools/ & pause & exit /b 1)
    set "INSTALLED=1"
)

if defined INSTALLED (
    echo Tools installed. Close this window and run the script again.
    pause
    exit /b 0
)

if not exist "%AT_DIR%\.git" (
    echo Cloning Atman into %AT_DIR% ...
    git clone -b main "%AT_REPO%" "%AT_DIR%" || (pause & exit /b 1)
) else (
    echo Updating %AT_DIR% ...
    pushd "%AT_DIR%"
    git fetch origin || (popd & pause & exit /b 1)
    git checkout -B main origin/main || (popd & pause & exit /b 1)
    popd
)

pushd "%AT_DIR%"
echo Building (the first time fetches and builds Strata and SDL3, a few minutes) ...
cmake -S . -B build -DATMAN_BUILD_TESTS=OFF -Wno-dev || (popd & pause & exit /b 1)
cmake --build build --config Release --parallel --target atman || (popd & pause & exit /b 1)

build\Release\atman.exe %*
set "RC=%ERRORLEVEL%"
popd
if not "%RC%"=="0" pause
exit /b %RC%
