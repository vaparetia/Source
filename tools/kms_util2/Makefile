#
# Kms Util2 
# 
# $Id: Makefile,v 1.6 2002/04/10 02:44:45 usr04098 Exp $
#

ifeq ($(MGS2_XBOX),1)
TARGET = kms_util2.xbox
else
TARGET = kms_util
endif

#OBJS = main.o conv1.o conv2.o conv3.o prog_utl.o poly_utl.o
OBJS = main.o conv2.o prog_utl.o conv3.o conv4.o poly_utl.o conv5.o conv6.o

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
