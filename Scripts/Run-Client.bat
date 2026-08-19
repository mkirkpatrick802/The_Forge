@echo off
REM Windowed client pointed at a dedicated server. No Steam.
REM Usage: Run-Client.bat [address] [config]     e.g. Run-Client.bat 127.0.0.1 Debug
REM
REM NOTE: the UDP transport does not exist yet, so this does not actually connect
REM to anything. It runs as an isolated client until that milestone lands.

set ADDRESS=%1
if "%ADDRESS%"=="" set ADDRESS=127.0.0.1

set CONFIG=%2
if "%CONFIG%"=="" set CONFIG=Debug

set TARGET=%~dp0..\Binaries\windows-x86_64\%CONFIG%\Project-Steel

if not exist "%TARGET%\Project-Steel.exe" (
    echo Could not find "%TARGET%\Project-Steel.exe".
    echo Build the solution first.
    pause
    exit /b 1
)

pushd "%TARGET%"
"%TARGET%\Project-Steel.exe" --connect %ADDRESS% --windowed
popd
