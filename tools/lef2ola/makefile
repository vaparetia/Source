# lef2ola
#
# $Id: makefile,v 1.2 2002/12/13 11:49:14 usr04761 Exp $
#

TARGET = lef2ola
OBJS =  main.o \
		Analyze.o \
		ReadLefPicture.o \
		ReadLefAction.o \
		ReadLefObject.o \
		ReadLefAnime.o \
		SetKeyData.o \
		DumpSceneData.o \
		DeleteSceneData.o \
		Lef2SceneData.o \
		SceneData2Ola.o

INSTALL_DIR = /u/develop/mj001b/tools/bin/linux

LIBS = 
CC = gcc
CFLAGS = -g -O2 -m486 -pipe
#DEFINE = -DDEBUG_DUMP -DDEBUG
#DEFINE = -DDEBUG

# Put your desired make options here.
.MAKEOPTS: -O

# This Makefile uses the Bourne shell.
SHELL = /bin/sh

# Default rule.

.c.o:
	$(CC) -c $(CFLAGS) $(DEFINE) $<

compile: $(TARGET)
	cc -o $(TARGET) -lm $(OBJS) -lm

$(TARGET): $(OBJS)

# Rule to remove intermediate files but leave the target intact.
clean:
	rm -f $(OBJS) $(TARGET)

install:
	cp $(TARGET) $(INSTALL_DIR)


main.o:		main.c data.h func.h
Analyze.o:	Analyze.c data.h func.h
Lef2SceneData.o: Lef2SceneData.c data.h func.h LESceneStruct20.h
ReadLefPicture.o: ReadLefPicture.c data.h func.h LESceneStruct20.h
ReadLefAction.o: ReadLefAction.c data.h func.h LESceneStruct20.h
ReadLefObject.o: ReadLefObject.c data.h func.h LESceneStruct20.h
ReadLefAnime.o: ReadLefAnime.c data.h func.h LESceneStruct20.h
SetKeyData.o: SetKeyData.c data.h func.h LESceneStruct20.h
DumpSceneData.o: DumpSceneData.c data.h func.h
DeleteSceneData.o: DeleteSceneData.c data.h func.h
SceneData2Ola.o : SceneData2Ola.c data.h
