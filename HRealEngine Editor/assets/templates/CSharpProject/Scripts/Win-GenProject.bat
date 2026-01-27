@echo off
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PREMAKE_EXE="

REM 1) Prefer a premake shipped with the template (recommended)
if exist "%SCRIPT_DIR%Tools\Premake\premake5.exe" (
    set "PREMAKE_EXE=%SCRIPT_DIR%Tools\Premake\premake5.exe"
)

REM 2) If user provided PREMAKE5 env var
if "%PREMAKE_EXE%"=="" (
    if not "%PREMAKE5%"=="" (
        if exist "%PREMAKE5%" set "PREMAKE_EXE=%PREMAKE5%"
    )
)

REM 3) If premake is in PATH
if "%PREMAKE_EXE%"=="" (
    where premake5.exe >nul 2>nul
    if !errorlevel! EQU 0 (
        for /f "delims=" %%i in ('where premake5.exe') do (
            set "PREMAKE_EXE=%%i"
            goto :found
        )
    )
)

:found
if "%PREMAKE_EXE%"=="" (
    echo [ERROR] Premake was not found.
    echo         Expected one of:
    echo         - "%SCRIPT_DIR%Tools\Premake\premake5.exe"
    echo         - Environment variable PREMAKE5 pointing to premake5.exe
    echo         - premake5.exe available in PATH
    echo.
    echo [HINT] Easiest fix: ship premake5.exe in Scripts\Tools\Premake\
    pause
    exit /b 1
)

echo [INFO] Using Premake: "%PREMAKE_EXE%"
pushd "%SCRIPT_DIR%"

REM Optional: allow custom workspace name via first arg
REM Example: Win-GenProject.bat MyGame
set "WS_NAME=%~1"
if "%WS_NAME%"=="" (
    for %%i in ("%SCRIPT_DIR%..") do set "WS_NAME=%%~nxi"
)


"%PREMAKE_EXE%" vs2022 --file=premake5.lua --workspace="%WS_NAME%"
if errorlevel 1 (
    echo [ERROR] Premake generation failed.
    popd
    pause
    exit /b 1
)

echo [OK] Generated Visual Studio solution for workspace: %WS_NAME%
popd
pause
exit /b 0
