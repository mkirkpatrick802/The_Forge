@echo off
setlocal

set PREMAKE_URL=https://github.com/premake/premake-core/releases/download/v5.0.0-beta5/premake-5.0.0-beta5-windows.zip
set PREMAKE_DIR=%CD%\..\Tools\Premake\Windows

PowerShell -ExecutionPolicy Bypass -File %CD%\BuildTools\Setup-Premake.ps1 "%PREMAKE_URL%" "%PREMAKE_DIR%"

pushd ..
%PREMAKE_DIR%\premake5.exe --file=Build.lua vs2022
popd
pause