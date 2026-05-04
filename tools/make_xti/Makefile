#
# make_xti
#
# $Id: Makefile,v 1.6 2002/08/06 09:46:37 usr02774 Exp $
#
ifeq ("$(OSTYPE)","linux")
#linux ver.
TARGET = make_xti
INSTALL_DIR = /u/develop/mj001b/tools/bin/linux
else
#cygwin ver.
TARGET = make_xti.exe
INSTALL_DIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox
endif
OBJS = main.o loadtpk.o writexti.o

OBJ1 = 

LIBS = 
CC = gcc
CFLAGS = -g -O3 -m486 -pipe

# Put your desired make options here.
.MAKEOPTS: -O

# This Makefile uses the Bourne shell.
SHELL = /bin/sh

# Default rule.

.c.o:
	$(CC) -c $(CFLAGS) $<

compile: $(TARGET)
	$(CC) -o $(TARGET) $(OBJS) -lm

$(TARGET): $(OBJS)

# Rule to remove intermediate files but leave the target intact.
clean:
	rm -f $(OBJS) $(TARGET)

install:
	cp $(TARGET) $(INSTALL_DIR)
