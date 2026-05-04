:: Launches MGS2 Text Tool with correct environment
CALL %~dp0\..\..\cp4_environment.bat
CD /D %~dp0\..\..\
ECHO Please Wait...
start "MGS2 Text Tool" TextTool.exe