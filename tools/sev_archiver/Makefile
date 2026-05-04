#
# SEV to SAR(sequencer achives file) コンパータ
# sev_archiver
#
# $Id: Makefile,v 1.2 2002/02/22 03:58:21 usr04098 Exp $
#
TARGET = sev_archiver
OBJS = main.o str_util.o sar_make.o

INSTALL_DIR = /u/develop/mj001b/tools/bin/linux

LIBS = -lm 
CC = gcc
#CFLAGS = -g -O2 -m486 -pipe
CFLAGS = -O2 -m486 -pipe

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
	cp $(TARGET) $(INSTALL_DIR)
