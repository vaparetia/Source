#
# 旧ライトデータ（.lit）＞新ライトデータ（.lt2）変換プログラム
# lt2_conv
#
# $Id: Makefile,v 1.2 1999/09/27 12:38:10 usr02774 Exp $
#
TARGET = lt2_conv
OBJS = main.o convert.o 

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
