# LINUX 用メイクファイル
#
# $(CPU)     : -m486 等のオブジェクトタイプ(gcc オプション)を指定します
# $(LIBDIR)  : ライブラリディレクトリのあるディレクトリを指定します
# $(INSTDIR) : インストール先ディレクトリを指定します
#
# 例
# setenv CPU     -m486
# setenv LIBDIR  /usr/home/lib : /usr/home/lib と仮定したばあい
# setenv INSTDIR /usr/bin      : /usr/bin と仮定したばあい
#
# 注意：LINUX はワイルドカード展開できない
# ×：if [ -f *.o ]; then cp *.o ../LINUX$(CPU)/; fi
# ○：if [ -f main.o ]; then cp *.o ../LINUX$(CPU)/; fi

CPU   	= -m486
LIBDIR 	= /u/develop/mj001b/tools/vrs/lib
INSTDIR	= /u/develop/mj001b/tools/bin/linux

SHELL = /bin/sh

TARGET = scn2hzx
CSRC   = main.c \
	 Scn2Hzx.c \
	 ZoneLib.c \
	 Near.c \
	 Online.c \
	 AnalyzeCommand.c \
	 GetHZXObject.c \
	 GetGroupObject.c \
	 GetHazardObject.c \
	 GetFloorObject.c \
	 GetTrapObject.c \
	 GetCameraObject.c \
	 GetZoneObject.c \
	 GetRouteObject.c \
	 GetCharacterObject.c \
	 GetClearingObject.c \
	 MakeHZXBlockObject.c \
	 DestroyScene2HZXObject.c \
	 OESPublic.c



OBJS   = $(CSRC:.c=.o)
LIBS   = $(LIBDIR)/lib/libP3DCL$(CPU).a \
	 $(LIBDIR)/lib/libSFDCL$(CPU).a \
	 $(LIBDIR)/lib/libP3DCL$(CPU).a \
	 $(LIBDIR)/lib/libSFDCL$(CPU).a

CC     = gcc
CFLAGS = -c -malign-double $(CPU) -D_LINUX -I$(LIBDIR)/include -I.
LFLAGS = $(CPU)
LFLIBS = -lm

all: cpstart $(TARGET)

.SUFFIXES: .o
.c.o: $(INCLUDE)
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJS) $(LIBS)
	if [ -f main.o ]; then cp *.o LINUX$(CPU)/; fi
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) $(LFLIBS) -o $(TARGET)
#	-rm -f *.o

cpstart:
	if [ ! -d LINUX$(CPU) ]; then mkdir LINUX$(CPU); fi
#	-rm -f $(TARGET)
#	-rm -f *.o
	if [ -f LINUX$(CPU)/main.o ]; then cp -p LINUX$(CPU)/*.o ./; fi

clean:
	-rm -f *.o
	-rm -f LINUX$(CPU)/*.o
	-rm -f $(TARGET)

install: cpstart $(INSTDIR)/$(TARGET) $(TARGET) $(INSTDIR)
	@echo done
$(INSTDIR)/$(TARGET): $(TARGET)
	@-rm -f $@
	cp $(TARGET) $@
$(INSTDIR):
	if [ ! -d $(INSTDIR) ]; then mkdir $(INSTDIR); fi
