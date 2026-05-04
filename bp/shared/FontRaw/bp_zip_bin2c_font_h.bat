@echo off

call ..\..\..\cp4_environment.bat

set FONTFILENAME=%BPE_REPOSITORY%/EngineSupport/Fonts/MGS_Font.raw

gzip.exe -f -k -9 %FONTFILENAME%

bin2c %FONTFILENAME%.gz MGS_FONT.raw.h MGS_FONT_RAW
