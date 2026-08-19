@echo off
REM Editor / offline authoring. Skips Steam entirely.
REM The editor is enabled automatically in Debug builds.

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Debug

set TARGET=%~dp0..\Binaries\windows-x86_64\%CONFIG%\Project-Steel

if not exist "%TARGET%\Project-Steel.exe" (
    echo Could not find "%TARGET%\Project-Steel.exe".
    echo Build the solution first.
    pause
    exit /b 1
)

pushd "%TARGET%"
"%TARGET%\Project-Steel.exe" --no-steam
popd
