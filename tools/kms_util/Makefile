#
# Kms Util 
# 
# $Id: Makefile,v 1.3 2000/01/22 10:33:00 usr02774 Exp $
#

TARGET = kms_util
OBJS = main.o conv1.o conv2.o conv3.o prog_utl.o poly_utl.o

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
