@echo off
setlocal enabledelayedexpansion

REM === Define Paths ===
set "PYTHON_DIR=..\..\Tools\Python"
set "PYTHON_EXE=%PYTHON_DIR%\python.exe"
set "PYTHON_ZIP_URL=https://www.python.org/ftp/python/3.12.2/python-3.12.2-embed-amd64.zip"
set "PYTHON_ZIP=%PYTHON_DIR%\python-embed.zip"

REM === Log file ===
set "LOG_FILE=reflect_log.txt"
echo [INFO] Reflection script started at %date% %time% > %LOG_FILE%
echo Working directory: %cd% >> %LOG_FILE%

REM === Check if Python exists ===
if exist "!PYTHON_EXE!" (
    echo [INFO] Found Python at !PYTHON_EXE!
    echo Found Python at !PYTHON_EXE! >> %LOG_FILE%
) else (
    echo [WARN] Python not found at !PYTHON_EXE!
    echo Python not found. Downloading... >> %LOG_FILE%

    if not exist "!PYTHON_DIR!" (
        mkdir "!PYTHON_DIR!"
    )

    powershell -Command "Invoke-WebRequest -Uri '%PYTHON_ZIP_URL%' -OutFile '%PYTHON_ZIP%'" >> %LOG_FILE% 2>&1
    if exist "!PYTHON_ZIP!" (
        powershell -Command "Expand-Archive -Path '%PYTHON_ZIP%' -DestinationPath '%PYTHON_DIR%' -Force" >> %LOG_FILE% 2>&1
        del "%PYTHON_ZIP%"
    ) else (
        echo [ERROR] Failed to download Python! >> %LOG_FILE%
        pause
        exit /b 1
    )
)

REM === Test Python ===
echo [INFO] Testing Python... >> %LOG_FILE%
"!PYTHON_EXE!" --version >> %LOG_FILE% 2>&1
if errorlevel 1 (
    echo [ERROR] Python did not run properly. >> %LOG_FILE%
    pause
    exit /b 1
)

REM === Run for The Forge ===
echo Running C++ reflection generator for The Forge...
echo [INFO] Reflecting The Forge... >> %LOG_FILE%
"!PYTHON_EXE!" reflect_generator.py ..\..\TheForge >> %LOG_FILE% 2>&1

REM === Run for Games ===
echo Running C++ reflection generator for Games...
echo [INFO] Reflecting Games... >> %LOG_FILE%
"!PYTHON_EXE!" reflect_generator.py ..\..\Games >> %LOG_FILE% 2>&1

REM === Complete ===
echo [SUCCESS] Reflection completed. Check %LOG_FILE% for details.
echo [SUCCESS] Reflection completed at %date% %time%. >> %LOG_FILE%

pause
