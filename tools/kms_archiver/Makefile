#
# Kms archiver 
# 
# $Id: Makefile,v 1.2 2002/04/10 05:04:49 usr04098 Exp $
#

TARGET = kms_archiver
OBJS = main.o conv.o prog_utl.o

ifeq ($(MGS2_WIN32),1)
INSTALL_DIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox
else
INSTALL_DIR = /u/develop/mj001b/tools/bin/linux
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
	cp $(TARGET) $(INSTALL_DIR)
