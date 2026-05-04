#
# make_miptpk
#
# $Id: Makefile,v 1.2 2002/06/19 09:00:59 usr02774 Exp $
#
ifeq ("$(OSTYPE)","linux")
#linux ver.
TARGET = make_mipxti
TARGET2 = make_mipxti_script.pl
INSTALL_DIR = /u/develop/mj001b/tools/bin/linux
else
#cygwin ver.
TARGET = make_mipxti.exe
TARGET2 = make_mipxti_script.pl
INSTALL_DIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox
endif
OBJS = main.o loadtpk.o writexti.o

OBJ1 = 

LIBS = 
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
	$(CC) -o $(TARGET) $(OBJS) -lm

$(TARGET): $(OBJS)

# Rule to remove intermediate files but leave the target intact.
clean:
	rm -f $(OBJS) $(TARGET)

install:
	cp $(TARGET) $(INSTALL_DIR)
	cp $(TARGET2) $(INSTALL_DIR)
