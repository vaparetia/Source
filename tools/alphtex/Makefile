#
# 新ライトデータ（.lt2）グループ分割プログラム
# lt2_div
#
# $Id: Makefile,v 1.1 2000/05/24 13:51:36 usr04098 Exp $
#
TARGET = alphtex
OBJS = main.o

INSTALL_DIR = /u/develop/mj001b/tools/bin/linux

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
	cc -o $(TARGET) $(LIBS) $(OBJS)

$(TARGET): $(OBJS)

# Rule to remove intermediate files but leave the target intact.
clean:
	rm -f $(OBJS) $(TARGET)

install:
	cp $(TARGET) $(INSTALL_DIR)
