@echo off
REM One entry point for building The Forge.
REM
REM   Build.bat [Config]        Config is Debug (default), Release or Dist
REM
REM Runs three steps in the order that matters:
REM   1. reflection codegen  -- may emit new .reflected.cpp files
REM   2. premake             -- picks up any new source files, including those
REM   3. msbuild             -- builds what premake just described
REM
REM Doing it in this order is why new files never silently go missing from the
REM build. Running msbuild on its own after adding a source file will link against
REM a stale project and fail with confusing unresolved externals.

setlocal enabledelayedexpansion

set ROOT=%~dp0..
set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Debug

echo.
echo === [1/3] Reflection codegen ===
set PYTHON=%ROOT%\Tools\Python\python.exe
if not exist "%PYTHON%" set PYTHON=python
"%PYTHON%" "%ROOT%\Scripts\BuildTools\reflect_generator.py" "%ROOT%\TheForge" >nul 2>&1
if errorlevel 1 (
    echo   [warn] reflection codegen skipped -- no usable Python found.
    echo          Run Scripts\BuildTools\run_reflection.bat once to install it.
) else (
    "%PYTHON%" "%ROOT%\Scripts\BuildTools\reflect_generator.py" "%ROOT%\Games" >nul 2>&1
    echo   ok
)

echo.
echo === [2/3] Generating project files ===
pushd "%ROOT%"
"%ROOT%\Tools\Premake\Windows\premake5.exe" --file=Build.lua vs2022
if errorlevel 1 (
    popd
    echo   [error] premake failed.
    exit /b 1
)
popd
echo   ok

echo.
echo === [3/3] Building %CONFIG% ^| x64 ===
set VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
set MSBUILD=
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set MSBUILD=%%i
)
if not defined MSBUILD (
    echo   [error] Could not locate MSBuild. Is Visual Studio with the C++ workload installed?
    exit /b 1
)

REM nodeReuse:false stops MSBuild leaving worker processes alive that hold on to
REM Engine.pdb and fail the next build with C1041.
"!MSBUILD!" "%ROOT%\The Forge.sln" /p:Configuration=%CONFIG% /p:Platform=x64 /m /nodeReuse:false /v:minimal
if errorlevel 1 (
    echo.
    echo   [error] Build failed.
    exit /b 1
)

echo.
echo === Build succeeded: %CONFIG% ===
echo   Output: Binaries\windows-x86_64\%CONFIG%\
echo   Run it: Scripts\Run-Editor.bat  ^|  Run-Server.bat  ^|  Run-Client.bat
exit /b 0
