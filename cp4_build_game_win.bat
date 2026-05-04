@echo off

:: Setup

:: Setup default working directory if not already set by calling script
IF "%WORKING_PATH%"=="" SET WORKING_PATH=%~dp0\mgs2x

CALL %WORKING_PATH%\..\cp4_environment.bat
TITLE '%WORKING_PATH%' Build Game MGS2 - Windows %1
@echo Building '%WORKING_PATH%' Game %1
@echo ---

@echo off

:: Build
"%VS90COMNTOOLS%..\IDE\devenv.com" "%WORKING_PATH%\source\MGS2.sln" /Build "%1|Win32" /Project main
IF ERRORLEVEL 1 EXIT /B 1
@echo.
@echo ---
