@echo off

:: Setup

:: Setup default working directory if not already set by calling script
IF "%WORKING_PATH%"=="" SET WORKING_PATH=%~dp0

CALL %WORKING_PATH%\cp4_environment.bat
TITLE '%WORKING_PATH%' Build Tools MGS2
@echo Building '%WORKING_PATH%' Tools
@echo ---

@echo off

:: Build
MOVE /Y "%WORKING_PATH%\Build_MGS2_TOOLS_Output.txt" "%WORKING_PATH%\Build_MGS2_TOOLS_Output_prev.txt"
"%VS90COMNTOOLS%..\IDE\devenv.com" "%BPE_ROOT%\Source\Everything.sln" /Build "Release" /Out "%WORKING_PATH%\Build_MGS2_TOOLS_Output.txt"
IF ERRORLEVEL 1 IF "%1"=="REPORTERROR" EXIT /B 1 
IF "%1"=="REPORTERROR" EXIT /B 0

@echo.
@echo ---

:: PAUSE unless 'NOPAUSE' is specified
IF NOT "%1"=="NOPAUSE" PAUSE
