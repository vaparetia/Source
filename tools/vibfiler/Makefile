#
# 振動データ管理ファイラー
# vibfiler
#
# $Id: Makefile,v 1.1 2000/06/23 06:16:49 usr02774 Exp $
#
TARGET = vibfiler
OBJS = main.o 

INSTALL_DIR = /u/develop/mj001b/tools/bin/linux

LIBS = -lm -lncurses /usr/lib/libreadline.so.3.0
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

