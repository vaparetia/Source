#
# 無圧縮モーション（.mt1 or .mtn）＞圧縮モーション（.mt3）変換プログラム
# mtn_conv
#
# $Id: Makefile,v 1.1 2000/06/08 06:22:04 usr04098 Exp $
#
TARGET = dumpmtn
OBJS = dumpmtn.o mtn_util.o

INSTALL_DIR = /u/develop/mj001b/tools/bin/linux

LIBS = -lm /usr/lib/libreadline.so.3.0
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
