# Microsoft Developer Studio Project File - Name="sonoyama" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** 編集しないでください **

# TARGTYPE "Xbox Static Library" 0x0b04

CFG=sonoyama - Xbox Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "sonoyama.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "sonoyama.mak" CFG="sonoyama - Xbox Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "sonoyama - Xbox Release" ("Xbox Static Library" 用)
!MESSAGE "sonoyama - Xbox Debug" ("Xbox Static Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/mgs2x/sonoyama", HWBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "sonoyama - Xbox Release"

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

!ELSEIF  "$(CFG)" == "sonoyama - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sonoyama___Xbox_Debug"
# PROP BASE Intermediate_Dir "sonoyama___Xbox_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\lib\Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\..\include" /I "..\..\system\libgv" /I "..\..\system\libdg" /I "..\..\system\libgcl" /I "..\..\system\libfs" /I "..\..\system\libhzx" /I "..\..\game" /I "..\..\system\libmt" /I "..\..\system\libutl" /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "DEBUG" /D "DEBUG_MODE" /D "NTSC" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "sonoyama - Xbox Release"
# Name "sonoyama - Xbox Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "raiden"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\raiden\command.c
# End Source File
# Begin Source File

SOURCE=.\raiden\command2.c
# End Source File
# Begin Source File

SOURCE=.\raiden\command3.c
# End Source File
# Begin Source File

SOURCE=.\raiden\command4.c
# End Source File
# Begin Source File

SOURCE=.\raiden\command5.c
# End Source File
# Begin Source File

SOURCE=.\raiden\dbviewpl.c
# End Source File
# Begin Source File

SOURCE=.\raiden\pl_arm.c
# End Source File
# Begin Source File

SOURCE=.\raiden\pl_bul_splash.c
# End Source File
# Begin Source File

SOURCE=.\raiden\pl_deadmode.c
# End Source File
# Begin Source File

SOURCE=.\raiden\pl_dummy.c
# End Source File
# Begin Source File

SOURCE=.\raiden\pl_global.c
# End Source File
# Begin Source File

SOURCE=.\raiden\pl_inline.c
# End Source File
# Begin Source File

SOURCE=.\raiden\pl_plugin.c
# End Source File
# Begin Source File

SOURCE=.\raiden\pl_subcam_mng.c
# End Source File
# Begin Source File

SOURCE=.\raiden\pl_subject_demo.c
# End Source File
# Begin Source File

SOURCE=.\raiden\rai_action_blur.c
# End Source File
# Begin Source File

SOURCE=.\raiden\rai_equip.c
# End Source File
# Begin Source File

SOURCE=.\raiden\rai_init.c
# End Source File
# Begin Source File

SOURCE=.\raiden\rai_shadow_hair.c
# End Source File
# Begin Source File

SOURCE=.\raiden\raiden.c
# End Source File
# Begin Source File

SOURCE=.\raiden\temp_resolve.c
# End Source File
# End Group
# Begin Group "etc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\etc\belt.c
# End Source File
# Begin Source File

SOURCE=.\etc\beltconv.c
# End Source File
# Begin Source File

SOURCE=.\etc\beltobj.c
# End Source File
# Begin Source File

SOURCE=.\etc\boss_survival.c
# End Source File
# Begin Source File

SOURCE=.\etc\bul_fall.c
# End Source File
# Begin Source File

SOURCE=.\etc\demo_snakearm.c
# End Source File
# Begin Source File

SOURCE=.\etc\dsegment.c
# End Source File
# Begin Source File

SOURCE=.\etc\dtrap.c
# End Source File
# Begin Source File

SOURCE=.\etc\etc.c
# End Source File
# Begin Source File

SOURCE=.\etc\evt_wall.c
# End Source File
# Begin Source File

SOURCE=.\etc\fallflr.c
# End Source File
# Begin Source File

SOURCE=.\etc\lodctrl.c
# End Source File
# Begin Source File

SOURCE=.\etc\multi.c
# End Source File
# Begin Source File

SOURCE=.\etc\o2gage.c
# End Source File
# Begin Source File

SOURCE=.\etc\objhzx.c
# End Source File
# Begin Source File

SOURCE=.\etc\paddemo.c
# End Source File
# Begin Source File

SOURCE=.\etc\pl_vib.c
# End Source File
# Begin Source File

SOURCE=.\etc\psg_sight.c
# End Source File
# Begin Source File

SOURCE=.\etc\scncamvib.c
# End Source File
# Begin Source File

SOURCE=.\etc\sunglasses.c
# End Source File
# Begin Source File

SOURCE=.\etc\t_sight.c
# End Source File
# Begin Source File

SOURCE=.\etc\targettrap.c
# End Source File
# Begin Source File

SOURCE=.\etc\zoomcam.c
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
