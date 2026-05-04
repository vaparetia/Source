#
# 新ライトデータ（.lt2）グループ分割プログラム
# lt2_div
#
# $Id: Makefile,v 1.3 2002/04/10 01:51:39 usr04098 Exp $
#
TARGET = lt2_div
OBJS = main.o convert.o 

ifeq ($(MGS2_WIN32),1)
INSTALL = /u/develop/mj001b/exports/tools/bin/mgs2xbox
else
INSTALL	= /u/develop/mj001b/tools/bin/linux
endif

LIBS = -lm 
CC = gcc
CFLAGS = -g -O2 -m486 -pipe

# Put your desired make options here.
.MAKEOPTS: -O

# This Makefile uses the Bourne shell.
SHELL = /bin/sh

# Default rule.

.c.o:
	$(CC) -c $(CFLAGS) $<

compile: $(TARGET)
	$(CC) -o $(TARGET) $(LIBS) $(OBJS)

$(TARGET): $(OBJS)

# Rule to remove intermediate files but leave the target intact.
clean:
	rm -f $(OBJS) $(TARGET)

install:
	cp $(TARGET) $(INSTALL)
