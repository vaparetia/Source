# Microsoft Developer Studio Project File - Name="korekado" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** 編集しないでください **

# TARGTYPE "Xbox Static Library" 0x0b04

CFG=korekado - Xbox Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "korekado.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "korekado.mak" CFG="korekado - Xbox Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "korekado - Xbox Release" ("Xbox Static Library" 用)
!MESSAGE "korekado - Xbox Debug" ("Xbox Static Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/mgs2x/korekado", NGBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "korekado - Xbox Release"

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

!ELSEIF  "$(CFG)" == "korekado - Xbox Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\..\include" /I "..\..\system\libgv" /I "..\..\system\libdg" /I "..\..\system\libgcl" /I "..\..\system\libfs" /I "..\..\system\libutl" /I "..\..\system\libhzx" /I "..\..\game" /I "..\..\system\libmt" /I "conv" /I "action" /I "enemy" /I "npc" /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "DEBUG" /D "DEBUG_MODE" /D "NTSC" /Fp"Debug/okajima.pch" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "korekado - Xbox Release"
# Name "korekado - Xbox Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "conv"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\conv\bgmanage.c
# End Source File
# Begin Source File

SOURCE=.\conv\ceiling.c
# End Source File
# Begin Source File

SOURCE=.\conv\chksling.c
# End Source File
# Begin Source File

SOURCE=.\conv\cnctobj.c
# End Source File
# Begin Source File

SOURCE=.\conv\cnctobj2.c
# End Source File
# Begin Source File

SOURCE=.\conv\corp.c
# End Source File
# Begin Source File

SOURCE=.\conv\eneequip.c
# End Source File
# Begin Source File

SOURCE=.\conv\eye.c
# End Source File
# Begin Source File

SOURCE=.\conv\gcl.c
# End Source File
# Begin Source File

SOURCE=.\conv\imagetrg.c
# End Source File
# Begin Source File

SOURCE=.\conv\n_sight.c
# End Source File
# Begin Source File

SOURCE=.\conv\navites.c
# End Source File
# Begin Source File

SOURCE=.\conv\putmodel.c
# End Source File
# Begin Source File

SOURCE=.\conv\putobj.c
# End Source File
# Begin Source File

SOURCE=.\conv\radio.c
# End Source File
# Begin Source File

SOURCE=.\conv\sdmanage.c
# End Source File
# Begin Source File

SOURCE=.\conv\sling.c
# End Source File
# Begin Source File

SOURCE=.\conv\slinggun.c
# End Source File
# Begin Source File

SOURCE=.\conv\util.c
# End Source File
# End Group
# Begin Group "acion"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\action\action.c
# End Source File
# End Group
# Begin Group "attacker"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\attacker\atavocom.c
# End Source File
# Begin Source File

SOURCE=.\attacker\atavoid.c
# End Source File
# Begin Source File

SOURCE=.\attacker\atsearch.c
# End Source File
# Begin Source File

SOURCE=.\attacker\atsneak.c
# End Source File
# Begin Source File

SOURCE=.\attacker\attacker.c
# End Source File
# Begin Source File

SOURCE=.\attacker\enedamag.c
# End Source File
# End Group
# Begin Group "effect"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\effect\putatach.c
# End Source File
# Begin Source File

SOURCE=.\effect\putatach_scn.c
# End Source File
# End Group
# Begin Group "hold"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hold\camera.c
# End Source File
# Begin Source File

SOURCE=.\hold\checkene.c
# End Source File
# Begin Source File

SOURCE=.\hold\holdene.c
# End Source File
# Begin Source File

SOURCE=.\hold\rootene.c
# End Source File
# End Group
# Begin Group "hostage"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hostage\hostage.c
# End Source File
# Begin Source File

SOURCE=.\hostage\hostcomm.c
# End Source File
# Begin Source File

SOURCE=.\hostage\skirt.c
# End Source File
# End Group
# Begin Group "notice"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\notice\notice.c
# End Source File
# Begin Source File

SOURCE=.\notice\thkaccid.c
# End Source File
# Begin Source File

SOURCE=.\notice\thkacrepo.c
# End Source File
# Begin Source File

SOURCE=.\notice\thkbox.c
# End Source File
# Begin Source File

SOURCE=.\notice\thkcorps.c
# End Source File
# Begin Source File

SOURCE=.\notice\thkdamag.c
# End Source File
# Begin Source File

SOURCE=.\notice\thkdiscv.c
# End Source File
# Begin Source File

SOURCE=.\notice\thkendam.c
# End Source File
# Begin Source File

SOURCE=.\notice\thkfound.c
# End Source File
# Begin Source File

SOURCE=.\notice\thkhold.c
# End Source File
# Begin Source File

SOURCE=.\notice\thkinves.c
# End Source File
# Begin Source File

SOURCE=.\notice\thknoise.c
# End Source File
# Begin Source File

SOURCE=.\notice\thkoboro.c
# End Source File
# Begin Source File

SOURCE=.\notice\thkradio.c
# End Source File
# Begin Source File

SOURCE=.\notice\thktouch.c
# End Source File
# Begin Source File

SOURCE=.\notice\thktrace.c
# End Source File
# Begin Source File

SOURCE=.\notice\thkunifo.c
# End Source File
# End Group
# Begin Group "npc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\npc\npc.c
# End Source File
# Begin Source File

SOURCE=.\npc\npcact.c
# End Source File
# Begin Source File

SOURCE=.\npc\npcsub.c
# End Source File
# Begin Source File

SOURCE=.\npc\sample.c
# End Source File
# End Group
# Begin Group "support"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\support\spalert.c
# End Source File
# Begin Source File

SOURCE=.\support\spavoid.c
# End Source File
# Begin Source File

SOURCE=.\support\spcautio.c
# End Source File
# Begin Source File

SOURCE=.\support\support.c
# End Source File
# End Group
# Begin Group "thing"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\thing\asiato.c
# End Source File
# Begin Source File

SOURCE=.\thing\asitest.c
# End Source File
# Begin Source File

SOURCE=.\thing\bgmfader.c
# End Source File
# Begin Source File

SOURCE=.\thing\db_box.c
# End Source File
# Begin Source File

SOURCE=.\thing\db_spher.c
# End Source File
# Begin Source File

SOURCE=.\thing\db_tex.c
# End Source File
# Begin Source File

SOURCE=.\thing\enemem.c
# End Source File
# Begin Source File

SOURCE=.\thing\item.c
# End Source File
# Begin Source File

SOURCE=.\thing\moveobj.c
# End Source File
# Begin Source File

SOURCE=.\thing\namepri.c
# End Source File
# Begin Source File

SOURCE=.\thing\strmfadr.c
# End Source File
# Begin Source File

SOURCE=.\thing\windnois.c
# End Source File
# End Group
# Begin Group "watcher"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\watcher\watcher.c
# End Source File
# Begin Source File

SOURCE=.\watcher\wcalert.c
# End Source File
# Begin Source File

SOURCE=.\watcher\wcavoid.c
# End Source File
# Begin Source File

SOURCE=.\watcher\wccomm.c
# End Source File
# Begin Source File

SOURCE=.\watcher\wcress.c
# End Source File
# Begin Source File

SOURCE=.\watcher\wcsneak.c
# End Source File
# End Group
# Begin Group "enemy"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\enemy\command.c
# End Source File
# Begin Source File

SOURCE=.\enemy\corpcom.c
# End Source File
# Begin Source File

SOURCE=.\enemy\corps.c
# End Source File
# Begin Source File

SOURCE=.\enemy\dbview.c
# End Source File
# Begin Source File

SOURCE=.\enemy\enaction.c
# End Source File
# Begin Source File

SOURCE=.\enemy\enecorps.c
# End Source File
# Begin Source File

SOURCE=.\enemy\enemy.c
# End Source File
# Begin Source File

SOURCE=.\enemy\eyeview.c
# End Source File
# Begin Source File

SOURCE=.\enemy\oldcorp.c
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "conv No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\conv\cnctobj.h
# End Source File
# Begin Source File

SOURCE=.\conv\corps.h
# End Source File
# Begin Source File

SOURCE=.\conv\define.h
# End Source File
# End Group
# Begin Group "action"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\action\action.h
# End Source File
# End Group
# Begin Group "attacker No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\attacker\atavoid.h
# End Source File
# Begin Source File

SOURCE=.\attacker\attacker.h
# End Source File
# Begin Source File

SOURCE=.\attacker\enedamag.h
# End Source File
# End Group
# Begin Group "hold No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hold\checkene.h
# End Source File
# Begin Source File

SOURCE=.\hold\chkmot.h
# End Source File
# Begin Source File

SOURCE=.\hold\holdene.h
# End Source File
# Begin Source File

SOURCE=.\hold\rootene.h
# End Source File
# End Group
# Begin Group "hostage No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hostage\hostage.h
# End Source File
# End Group
# Begin Group "npc No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\npc\npc.h
# End Source File
# End Group
# Begin Group "support No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\support\support.h
# End Source File
# End Group
# Begin Group "thing No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\thing\enemem.h
# End Source File
# Begin Source File

SOURCE=.\thing\item.h
# End Source File
# End Group
# Begin Group "watcher No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\watcher\watcher.h
# End Source File
# End Group
# Begin Group "enemy No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\enemy\command.h
# End Source File
# Begin Source File

SOURCE=.\enemy\data.h
# End Source File
# Begin Source File

SOURCE=.\enemy\enemy.h
# End Source File
# Begin Source File

SOURCE=.\enemy\motion.h
# End Source File
# Begin Source File

SOURCE=.\enemy\mt_array.h
# End Source File
# Begin Source File

SOURCE=.\enemy\route.h
# End Source File
# Begin Source File

SOURCE=.\enemy\security.h
# End Source File
# Begin Source File

SOURCE=.\enemy\speak.h
# End Source File
# End Group
# End Group
# End Target
# End Project
