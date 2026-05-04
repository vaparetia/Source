#
# Kmx2kmy converter
# 
# $Id: Makefile,v 1.1 2002/06/05 05:23:18 usr02774 Exp $
#

ifeq ("$(OSTYPE)","linux")
#linux ver.
TARGET = kmx2kmy
INSTALL_DIR = /u/develop/mj001b/tools/bin/linux
else
#cygwin ver.
TARGET = kmx2kmy.exe
INSTALL_DIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox
endif

OBJS = main.o prog_utl.o conv.o

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
