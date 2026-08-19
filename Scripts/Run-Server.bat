@echo off
REM Headless dedicated server: no window, no GPU, no Steam.
REM Logs go to this console and to server.log in the output folder.

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Debug

set TARGET=%~dp0..\Binaries\windows-x86_64\%CONFIG%\Project-Steel

if not exist "%TARGET%\Project-Steel.exe" (
    echo Could not find "%TARGET%\Project-Steel.exe".
    echo Build the solution first, or pass a configuration: Run-Server.bat Release
    pause
    exit /b 1
)

REM pushd sets the working directory so assets resolve; the exe is still invoked by
REM full path, because cmd does not always search the current directory for it.
pushd "%TARGET%"
REM --insecure skips Steam authentication and takes each client's claimed identity at
REM face value. Development and LAN only -- drop it for anything reachable publicly,
REM and pass --gslt <token> so the server logs on under your appid.
"%TARGET%\Project-Steel.exe" --server --insecure
popd
pause
