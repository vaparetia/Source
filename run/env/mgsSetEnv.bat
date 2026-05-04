@echo off

IF "%MGS_ENV_INIT%"=="1" GOTO :EOF

:: Store current directory
PUSHD .

:: SET UP ROOT
cd /d %~dp0..\..\

:: Write root directory to main MGS_ROOT env var
FOR /f "tokens=1" %%B in ('CHDIR') do set MGS_ROOT=%%B

:: Strip trailing backspace
IF "%MGS_ROOT:~-1%"=="\" SET MGS_ROOT=%MGS_ROOT:~0,-1%

:: Set up path

SET PATH=%PATH%;%MGS_ROOT%\tools\Release
SET PATH=%PATH%;%MGS_ROOT%\..\tools\bin\Release
SET PATH=%PATH%;%MGS_ROOT%\Run

:: SET UP PERFORCE
SET P4PORT=BP-SERVER2:1701

:: Set up direct x SDK
SET DXSDK_DIR=%MGS_ROOT%\mgs2x\module\dxsdk

:: Set up so we don't init env twice
SET MGS_ENV_INIT=1

:: Return to starting dir
POPD