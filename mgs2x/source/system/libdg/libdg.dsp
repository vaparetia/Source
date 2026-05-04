# Microsoft Developer Studio Project File - Name="libdg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Static Library" 0x0104
# TARGTYPE "Xbox Static Library" 0x0b04

CFG=libdg - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "libdg.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "libdg.mak" CFG="libdg - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "libdg - Win32 Release" ("Win32 (x86) Static Library" 用)
!MESSAGE "libdg - Win32 Debug" ("Win32 (x86) Static Library" 用)
!MESSAGE "libdg - Xbox Debug" ("Xbox Static Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/mgs2x/libdg", RBCAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "libdg - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "DEBUG" /D "DEBUG_MODE" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libdg___Win32_Debug"
# PROP BASE Intermediate_Dir "libdg___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\lib\w32Debug"
# PROP Intermediate_Dir "w32Debug"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "DEBUG" /D "DEBUG_MODE" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
IntDir=.\w32Debug
WkspDir=.
SOURCE="$(InputPath)"
PostBuild_Desc=copying shader images
PostBuild_Cmds=copy $(IntDir)\*.utn $(WkspDir)\..\cdrom.img\stage\init	copy $(IntDir)\*.vtn $(WkspDir)\..\cdrom.img\stage\init
# End Special Build Tool

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libdg___Xbox_Debug"
# PROP BASE Intermediate_Dir "libdg___Xbox_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\lib\Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\include" /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /D "DEBUG_MODE" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\..\include" /I "..\libgv" /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "DEBUG" /D "DEBUG_MODE" /D "NTSC" /FR /YX /FD /Zvc6 /G6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
IntDir=.\Debug
WkspDir=.
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(IntDir)\*.u $(WkspDir)\..\cdrom.img\stage\init	copy $(IntDir)\*.v $(WkspDir)\..\cdrom.img\stage\init
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "libdg - Win32 Release"
# Name "libdg - Win32 Debug"
# Name "libdg - Xbox Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\2d_prim.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\callback.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chain.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chanl.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\comdl.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d3d.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgd.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dmapack.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\evmobjs.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\frame.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\light.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loader.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\m_weight.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\particle.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\patch.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\plugin.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pos.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\prim2.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pshade.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\screen.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shadowvl.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shdwchin.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\text.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vanime.cpp

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\dgconf.h
# End Source File
# Begin Source File

SOURCE=.\fmt_bmp.h
# End Source File
# Begin Source File

SOURCE=.\fmt_dds.h
# End Source File
# Begin Source File

SOURCE=.\private.h
# End Source File
# Begin Source File

SOURCE=.\shader.h
# End Source File
# End Group
# Begin Group "Vertex Shader"

# PROP Default_Filter "vsh"
# Begin Source File

SOURCE=.\comdl.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\comdl.vsh
InputName=comdl

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\comdl.vsh
InputName=comdl

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\evm.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\evm.vsh
InputName=evm

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\evm.vsh
InputName=evm

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\evml.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\evml.vsh
InputName=evml

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\evml.vsh
InputName=evml

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\obj.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\obj.vsh
InputName=obj

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\obj.vsh
InputName=obj

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\obja.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\obja.vsh
InputName=obja

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\obja.vsh
InputName=obja

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objal.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\objal.vsh
InputName=objal

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\objal.vsh
InputName=objal

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objaw.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\objaw.vsh
InputName=objaw

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\objaw.vsh
InputName=objaw

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objawl.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\objawl.vsh
InputName=objawl

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\objawl.vsh
InputName=objawl

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objl.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\objl.vsh
InputName=objl

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\objl.vsh
InputName=objl

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objp.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\objp.vsh
InputName=objp

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\objp.vsh
InputName=objp

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objw.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\objw.vsh
InputName=objw

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\objw.vsh
InputName=objw

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objwl.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\objwl.vsh
InputName=objwl

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\objwl.vsh
InputName=objwl

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\patch.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\patch.vsh
InputName=patch

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\patch.vsh
InputName=patch

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\patchbl.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\patchbl.vsh
InputName=patchbl

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\patchl.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\patchl.vsh
InputName=patchl

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\patchl.vsh
InputName=patchl

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shadow.vsh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\shadow.vsh
InputName=shadow

"$(IntDir)\$(InputName).vtn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).vtn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\shadow.vsh
InputName=shadow

"$(IntDir)\$(InputName).v" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).v

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Pixel Shader"

# PROP Default_Filter "psh"
# Begin Source File

SOURCE=.\normal.psh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\normal.psh
InputName=normal

"$(IntDir)\$(InputName).utn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).utn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\normal.psh
InputName=normal

"$(IntDir)\$(InputName).u" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).u

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\patchbl.psh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\patchbl.psh
InputName=patchbl

"$(IntDir)\$(InputName).u" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).u

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\plight.psh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\plight.psh
InputName=plight

"$(IntDir)\$(InputName).utn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).utn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\plight.psh
InputName=plight

"$(IntDir)\$(InputName).u" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).u

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\test.psh

!IF  "$(CFG)" == "libdg - Win32 Release"

!ELSEIF  "$(CFG)" == "libdg - Win32 Debug"

# Begin Custom Build
IntDir=.\w32Debug
InputPath=.\test.psh
InputName=test

"$(IntDir)\$(InputName).utn" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nvasm -b $(InputPath) $(IntDir)\$(InputName).utn

# End Custom Build

!ELSEIF  "$(CFG)" == "libdg - Xbox Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\test.psh
InputName=test

"$(IntDir)\$(InputName).u" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm -nologo -D _XBOX $(InputPath) $(IntDir)\$(InputName).u

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
