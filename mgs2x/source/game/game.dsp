# Microsoft Developer Studio Project File - Name="game" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** 編集しないでください **

# TARGTYPE "Xbox Static Library" 0x0b04

CFG=game - Xbox Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "game.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "game.mak" CFG="game - Xbox Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "game - Xbox Release" ("Xbox Static Library" 用)
!MESSAGE "game - Xbox Debug" ("Xbox Static Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/mgs2x/game", RUAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "game - Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "game - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib\Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\include" /I "..\system\libgv" /I "..\system\libdg" /I "..\system\libfs" /I "..\system\libmt" /I "..\system\libgcl" /I "..\system\libhzx" /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "DEBUG" /D "DEBUG_MODE" /D "NTSC" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "game - Xbox Release"
# Name "game - Xbox Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\area.c
# End Source File
# Begin Source File

SOURCE=.\cam_set.c
# End Source File
# Begin Source File

SOURCE=.\cam_util.c
# End Source File
# Begin Source File

SOURCE=.\camera.c
# End Source File
# Begin Source File

SOURCE=.\camerad.c
# End Source File
# Begin Source File

SOURCE=.\caption.c
# End Source File
# Begin Source File

SOURCE=.\chara.c
# End Source File
# Begin Source File

SOURCE=.\control.c
# End Source File
# Begin Source File

SOURCE=.\debugcam.c
# End Source File
# Begin Source File

SOURCE=.\debugmenu.c
# End Source File
# Begin Source File

SOURCE=.\dumstream.c
# End Source File
# Begin Source File

SOURCE=.\extern.c
# End Source File
# Begin Source File

SOURCE=.\font.c
# End Source File
# Begin Source File

SOURCE=.\gamed.c
# End Source File
# Begin Source File

SOURCE=.\gameinit.c
# End Source File
# Begin Source File

SOURCE=.\getitem.c
# End Source File
# Begin Source File

SOURCE=.\hazard.c
# End Source File
# Begin Source File

SOURCE=.\loader.c
# End Source File
# Begin Source File

SOURCE=.\map.c
# End Source File
# Begin Source File

SOURCE=.\mapcnct.c
# End Source File
# Begin Source File

SOURCE=.\menupri2.cpp
# End Source File
# Begin Source File

SOURCE=.\no_use.c
# End Source File
# Begin Source File

SOURCE=.\object.c
# End Source File
# Begin Source File

SOURCE=.\padctrl.c
# End Source File
# Begin Source File

SOURCE=.\radarctrl.c
# End Source File
# Begin Source File

SOURCE=.\resource.c
# End Source File
# Begin Source File

SOURCE=.\script.c
# End Source File
# Begin Source File

SOURCE=.\sparkd.c
# End Source File
# Begin Source File

SOURCE=.\syscllbk.c
# End Source File
# Begin Source File

SOURCE=.\target.c
# End Source File
# Begin Source File

SOURCE=.\temp.cpp
# End Source File
# Begin Source File

SOURCE=.\viewer.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
