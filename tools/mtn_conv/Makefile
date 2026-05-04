#
# 無圧縮モーション（.mt1 or .mtn）＞圧縮モーション（.mt3）変換プログラム
# mtn_conv
#
# $Id: Makefile,v 1.2 2000/05/15 00:46:45 usr02774 Exp $
#
TARGET = mtn_conv
OBJS = mtnconv.o loadmt1.o mtn_util.o mt3_enc.o quat.o gte.o hermite.o

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
