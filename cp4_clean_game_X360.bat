@echo off

:: Setup

:: Setup default working directory if not already set by calling script
IF "%WORKING_PATH%"=="" SET WORKING_PATH=%~dp0\mgs2x

CALL %WORKING_PATH%\..\cp4_environment.bat
TITLE '%WORKING_PATH%' Cleaning Game MGS2 - X360
@echo Cleaning '%WORKING_PATH%' Game
@echo ---

@echo off

:: Clean
"%VS90COMNTOOLS%..\IDE\devenv.com" "%WORKING_PATH%\source\MGS2.sln" /Clean "X360_Develop|Win32" /Project main
@echo.
@echo ---
