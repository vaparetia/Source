@echo off

IF "%CP4_ENV_INIT%"=="1" GOTO :EOF

PUSHD .

CD /d %~dp0

set MGS_ROOT=%CD%

set BPE_ROOT=%CD%\BP
set BPE_REPOSITORY=%CD%\GameData

:: Set PATH for BP Tools
set PATH=%BPE_ROOT%\bin;%PATH%
set PATH=%BPE_ROOT%\bin\Tools;%PATH%
set PATH=%BPE_ROOT%\bin\Tools\Release;%PATH%

:: Set PATH for Vita tools
set PATH=%PATH%;%SCE_PSP2_SDK_DIR%\host_tools\graphics\lib
set PATH=%PATH%;%SCE_PSP2_SDK_DIR%\host_tools\build\bin
set PATH=%PATH%;%SCE_PSP2_SDK_DIR%\host_tools\fios2
set PATH=%PATH%;%SCE_PSP2_SDK_DIR%\host_tools\Audio\at9tool\bin
set PATH=%PATH%;%SCE_PSP2_SDK_DIR%\host_tools\Audio\at9tool\bin

:: Set PATH for MGS tools
SET PATH=%PATH%;%MGS_ROOT%\tools\Release
SET PATH=%PATH%;%MGS_ROOT%\..\tools\bin\Release

:: Define the file and folder this game uses for a "slot"
SET CP4_SLOTDIR=face
SET MGS_VERSION=2

::Allows the developer to override enviro variables on a per project/computer basis.
::For example, the SCE_PS3_ROOT can be overriden to use multiple SDKs on a single machine.
IF EXIST cp4_customenv.bat CALL cp4_customenv

:: Set up so we don't init env twice
SET CP4_ENV_INIT=1

POPD
