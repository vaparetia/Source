#
# khm converter 
# 
# $Id: Makefile,v 1.1 2002/11/18 07:22:48 usr02774 Exp $
#

ifeq ($(MGS2_XBOX),1)
TARGET = khm_conv.xbox
else
TARGET = khm_conv
endif

OBJS = main.o conv.o prog_utl.o poly_utl.o

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
