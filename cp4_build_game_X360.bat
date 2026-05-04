@echo off

:: Setup

:: Setup default working directory if not already set by calling script
IF "%WORKING_PATH%"=="" SET WORKING_PATH=%~dp0\mgs2x

CALL %WORKING_PATH%\..\cp4_environment.bat
TITLE '%WORKING_PATH%' Build Game MGS2 - X360
@echo Building '%WORKING_PATH%' Game
@echo ---

@echo off

:: Build
MOVE /Y "%WORKING_PATH%\..\Build_MGS2_X360_Output.txt" "%WORKING_PATH%\..\Build_MGS2_X360_Output_prev.txt"
"%VS90COMNTOOLS%..\IDE\devenv.com" "%WORKING_PATH%\source\MGS2.sln" /Build "X360_Develop|Win32" /Project main /Out "%WORKING_PATH%\..\Build_MGS2_X360_Output.txt"
@echo.
@echo ---

start "MGS2 X360 Build Results" "%WORKING_PATH%\..\Build_MGS2_X360_Output.txt"