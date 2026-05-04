# Microsoft Developer Studio Project File - Name="libhzx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Static Library" 0x0104
# TARGTYPE "Xbox Static Library" 0x0b04

CFG=libhzx - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "libhzx.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "libhzx.mak" CFG="libhzx - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "libhzx - Xbox Release" ("Xbox Static Library" 用)
!MESSAGE "libhzx - Xbox Debug" ("Xbox Static Library" 用)
!MESSAGE "libhzx - Win32 Debug" ("Win32 (x86) Static Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/mgs2x/libhzx", FABAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "libhzx - Xbox Release"

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
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /G6 /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /G6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libhzx - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\lib\Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /G6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\..\include" /I "..\libgv" /I "..\libdg" /I "..\libgcl" /I "..\libmt" /I "..\..\game" /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "DEBUG" /D "DEBUG_MODE" /D "NTSC" /YX /FD /Zvc6 /G6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libhzx - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libhzx___Win32_Debug"
# PROP BASE Intermediate_Dir "libhzx___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\lib\w32Debug"
# PROP Intermediate_Dir "w32Debug"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /G6 /W3 /Gm /GX /Zi /Od /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /D "DEBUG_MODE" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "DEBUG" /D "DEBUG_MODE" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libhzx - Xbox Release"
# Name "libhzx - Xbox Debug"
# Name "libhzx - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bind.c

!IF  "$(CFG)" == "libhzx - Xbox Release"

!ELSEIF  "$(CFG)" == "libhzx - Xbox Debug"

!ELSEIF  "$(CFG)" == "libhzx - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\debug.c

!IF  "$(CFG)" == "libhzx - Xbox Release"

!ELSEIF  "$(CFG)" == "libhzx - Xbox Debug"

!ELSEIF  "$(CFG)" == "libhzx - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dynamic.cpp

!IF  "$(CFG)" == "libhzx - Xbox Release"

!ELSEIF  "$(CFG)" == "libhzx - Xbox Debug"

!ELSEIF  "$(CFG)" == "libhzx - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hzxd.c

!IF  "$(CFG)" == "libhzx - Xbox Release"

!ELSEIF  "$(CFG)" == "libhzx - Xbox Debug"

!ELSEIF  "$(CFG)" == "libhzx - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\level.cpp

!IF  "$(CFG)" == "libhzx - Xbox Release"

!ELSEIF  "$(CFG)" == "libhzx - Xbox Debug"

!ELSEIF  "$(CFG)" == "libhzx - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loadhzx.c

!IF  "$(CFG)" == "libhzx - Xbox Release"

!ELSEIF  "$(CFG)" == "libhzx - Xbox Debug"

!ELSEIF  "$(CFG)" == "libhzx - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\navigate.c

!IF  "$(CFG)" == "libhzx - Xbox Release"

!ELSEIF  "$(CFG)" == "libhzx - Xbox Debug"

!ELSEIF  "$(CFG)" == "libhzx - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\near.cpp

!IF  "$(CFG)" == "libhzx - Xbox Release"

!ELSEIF  "$(CFG)" == "libhzx - Xbox Debug"

!ELSEIF  "$(CFG)" == "libhzx - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\online.cpp

!IF  "$(CFG)" == "libhzx - Xbox Release"

!ELSEIF  "$(CFG)" == "libhzx - Xbox Debug"

!ELSEIF  "$(CFG)" == "libhzx - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\route.c

!IF  "$(CFG)" == "libhzx - Xbox Release"

!ELSEIF  "$(CFG)" == "libhzx - Xbox Debug"

!ELSEIF  "$(CFG)" == "libhzx - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\trap.c

!IF  "$(CFG)" == "libhzx - Xbox Release"

!ELSEIF  "$(CFG)" == "libhzx - Xbox Debug"

!ELSEIF  "$(CFG)" == "libhzx - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\trp_view.c

!IF  "$(CFG)" == "libhzx - Xbox Release"

!ELSEIF  "$(CFG)" == "libhzx - Xbox Debug"

!ELSEIF  "$(CFG)" == "libhzx - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
