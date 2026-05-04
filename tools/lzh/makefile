#
# Makefile for UVMap 
#	
#	Tue Nov 16 07:45:11 JST 1999
#	T.Morita
#
#	$Id: makefile,v 1.1 2001/11/30 04:26:54 usr04098 Exp $
#
TARGET = lzsh


INSTDIR = /u/develop/mj001b/tools/bin/linux

all: $(TARGET)

$(TARGET): lzh.c
	gcc -o $(TARGET) -g lzh.c

install:
	cp $(TARGET) $(INSTDIR)

clean :
	rm $(TARGET)

