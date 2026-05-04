@echo off

:: Setup

:: Setup default working directory if not already set by calling script
IF "%WORKING_PATH%"=="" SET WORKING_PATH=%~dp0

CALL %WORKING_PATH%\cp4_environment.bat
TITLE '%WORKING_PATH%' Cleaning Game MGS2 - PS3
@echo Cleaning '%WORKING_PATH%' Game
@echo ---

@echo off

:: Build
VsiBuild "%WORKING_PATH%\mgs2x\source\MGS2.sln" "PS3_SHIP|Win32" /clean
@echo.
@echo ---