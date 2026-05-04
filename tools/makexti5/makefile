#
#	makefile for makexti3
#
#	2001/04/27
#	
#	NISINO Motoaki
#


TARGET = makexti5

ifeq ($(MGS2_WIN32),1)
LIBDIR = ../libMDU
INCDIR = ../libMDU
INSTALL_DIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox
else
LIBDIR = /u/develop/mj001b/tools/lib
INCDIR = ../libMDU
INSTALL_DIR = /u/develop/mj001b/tools/bin/linux
endif

CC     = gcc
CPU    = -m486
DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN
CFLAGS = -c -g -O2 $(CPU) -Wall $(DEFINES) -malign-double

OBJECTS = main.o bmp.o bmppos.o file.o dxtcmp.o


#コンパイル
#.cがあったら.oにせよ
.c.o :
	$(CC) -I. -I$(INCDIR) $(CFLAGS) $<


all: $(TARGET)
	

# オブジェファイルをリンクして実行ファイルをつくりましょう
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) -L$(LIBDIR) -lMDU


clean:
	-rm -f *.o
	-rm -f $(TARGET)


install:
	cp $(TARGET) $(INSTALL_DIR)


