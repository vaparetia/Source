#
#	makefile for makexti6
#
#	2001/04/27
#	
#	NISINO Motoaki
#


TARGET = makexti6

#LIBDIR = ../libMDU
INCDIR = ../libMDU

ifeq ($(MGS2_WIN32),1)
LIBDIR = ../libMDU
INSTALL_DIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox
else
LIBDIR = /u/develop/mj001b/tools/lib
INSTALL_DIR = /u/develop/mj001b/tools/bin/linux
endif

CC     = gcc
CPU    = -m486

ifeq ($(TOOL_RELEASE),1)
DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN -D_RELEASE_
else
DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN
endif

CFLAGS = -c -g -O3 $(CPU) -Wall $(DEFINES) -I. -malign-double

OBJECTS = main.o arrangerect.o wrapfunc.o tex_utl.o bmp.o file.o dxtcmp.o dds.o \
mipmap.o nisi_utl.o


#コンパイル
#.cがあったら.oにせよ
.c.o :
	$(CC) -I$(INCDIR) $(CFLAGS) $<


all: $(TARGET)
	

# オブジェファイルをリンクして実行ファイルをつくりましょう
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) -I$(INCDIR) -L$(LIBDIR) -lMDU -lm


clean:
	-rm -f *.o
	-rm -f $(TARGET)


install:
	cp $(TARGET) $(INSTALL_DIR)


