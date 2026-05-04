@echo off

:: Setup

SETLOCAL

SET REPORTERROR=0
SET BUILDCONFIG=VTA_Develop

:PROCESSARGS

IF "%1"=="" GOTO ENDPROCESSARGS
IF /I "%1"=="REPORTERROR" SET REPORTERROR=1
IF /I "%1"=="SHIP" SET BUILDCONFIG=VTA_Ship
IF /I "%1"=="DEBUG" SET BUILDCONFIG=VTA_Debug
SHIFT
GOTO PROCESSARGS

:ENDPROCESSARGS

:: Setup default working directory if not already set by calling script
IF "%WORKING_PATH%"=="" SET WORKING_PATH=%~dp0

CALL %WORKING_PATH%\cp4_environment.bat
TITLE '%WORKING_PATH%' Build Game MGS2 - %BUILDCONFIG%
@echo Building '%WORKING_PATH%' Game - %BUILDCONFIG%
@echo ---

@echo off


:: Build
MOVE /Y "%WORKING_PATH%\Build_MGS2_%BUILDCONFIG%_Output.txt" "%WORKING_PATH%\Build_MGS2_%BUILDCONFIG%_Output_prev.txt"
VsiBuild "%WORKING_PATH%\mgs2x\source\MGS2.sln" "%BUILDCONFIG%|Win32" /out "%WORKING_PATH%\Build_MGS2_%BUILDCONFIG%_Output.txt"
IF ERRORLEVEL 1 IF "%REPORTERROR%"=="1" EXIT /B 1 
IF "%REPORTERROR%"=="1" EXIT /B 0
@echo.
@echo ---

IF NOT "%CP4_USER%"=="build_cp4a" start "MGS2 Build Results" "%WORKING_PATH%\Build_MGS2_%BUILDCONFIG%_Output.txt"