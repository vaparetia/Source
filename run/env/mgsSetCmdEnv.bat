@echo off

CALL %~dp0mgsSetEnv.bat

:: Set up window
TITLE MGS Command Prompt

:: Use obvious color to prevent confusion
COLOR 1E

echo ==================
echo MGS Command Prompt
echo ==================
echo.
echo MGS_ROOT=%MGS_ROOT%
echo P4PORT=%P4PORT%
echo.
echo --------------------------------
