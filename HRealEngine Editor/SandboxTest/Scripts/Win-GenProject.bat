@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "PREMAKE_EXE=%SCRIPT_DIR%Tools\Premake\premake5.exe"

if not exist "%PREMAKE_EXE%" (
    echo [ERROR] premake5.exe not found at: "%PREMAKE_EXE%"
    exit /b 1
)

set "WS_NAME=%~1"
if "%WS_NAME%"=="" (
    for %%i in ("%SCRIPT_DIR%..") do set "WS_NAME=%%~nxi"
)

pushd "%SCRIPT_DIR%"
"%PREMAKE_EXE%" vs2022 --file=premake5.lua --workspace="%WS_NAME%"
if errorlevel 1 (
    echo [ERROR] Premake generation failed.
    popd
    exit /b 1
)

echo [OK] Generated Visual Studio solution: %WS_NAME%
popd
pause
