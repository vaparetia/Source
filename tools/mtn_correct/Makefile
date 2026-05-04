#
# 変換プログラム
# mtn_correct
#
# $Id: Makefile,v 1.1 1999/11/16 08:30:31 usr02774 Exp $
#
TARGET = mtn_correct
OBJS = main.o loadmt1.o mtn_util.o mt3_enc.o quat.o gte.o

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
