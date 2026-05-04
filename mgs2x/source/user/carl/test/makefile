#-----------------------------------------------------------
#	MGS2X-Gnumake用 仮メイクファイル
#-----------------------------------------------------------

# 

#-----------------------------------------------------------
#	デバッグバージョン
#-----------------------------------------------------------

OUTDIR=.
INTDIR=.

LIB_NAME = carl

# gnumakeは "   "をうまく認識してくれない？
OUTPUT_LIB_GNU = ..\..\lib\Debug\$(LIB_NAME).lib
OUTPUT_LIB = "$(OUTPUT_LIB_GNU)"


PCH = $(LIB_NAME).pch



# インクルードファイルフォルダへのパス
SYS_INCLUDE_DIR=/I "C:\Program Files\Microsoft Xbox SDK\xbox\include"

INCLUDE_DIR=/I "..\..\..\include" /I "..\..\..\system\libgv" /I "..\..\..\system\libdg" /I "..\..\..\system\libgcl" /I "..\..\..\system\libfs" /I "..\..\..\system\libhzx" /I "..\..\..\game" /I "..\..\..\system\libmt"




# コンパイラのオプション設定
CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od $(SYS_INCLUDE_DIR) $(INCLUDE_DIR) /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "DEBUG" /D "DEBUG_MODE" /D "NTSC" /Fp"$(INTDIR)\$(PCH)" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /G6 /Zvc6 /c 


# リンカのオプション設定
#LIB32=link.exe -lib
#LIB32_FLAGS=/nologo /out:$(OUTPUT_LIB)
LIB32_OBJS= \
	$(INTDIR)\testply.obj


all : $(OUTPUT_LIB_GNU)


$(OUTPUT_LIB_GNU) :  $(LIB32_OBJS)




%.obj :: %.cpp
	$(CPP) $(CPP_PROJ) "$<"


%.obj :: %.c
	$(CPP) $(CPP_PROJ) "$<"




#-----------------------------------------------------------

clean :
	rm -f  $(INTDIR)/*.obj
	rm -f  $(OUTPUT_LIB)





