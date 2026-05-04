#
# Makefile for UVMap 
#	
#	Tue Nov 16 07:45:11 JST 1999
#	T.Morita
#
#	$Id: Makefile,v 5.4 2002/06/07 10:32:28 usr04098 Exp $
#
TARGET = dpack dpack.xbox dpack.def dpack.int dpack.kmy prepro replace_src

ifeq ($(MGS2_WIN32),1)
INSTDIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox
else
INSTDIR = /u/develop/mj001b/tools/bin/linux
endif

install:
	cp $(TARGET) $(INSTDIR)

