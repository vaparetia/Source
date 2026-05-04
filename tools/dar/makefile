# 
# makfile for dar
# 
# dar : file archiver for MGS on linux
#	
# by M.Sonoyama 1999 Aug. ¡Á
# $Id: makefile,v 1.5 2002/04/10 04:22:38 usr04098 Exp $
# Konami Computer Entertainment Japan West CS1
#

SHELL = /bin/sh

TARGET = dar
CSRC   = dar.c

OBJS   = $(CSRC:.c=.o)

CPU    = -i386
ifeq ($(MGS2_WIN32),1)
MDUDIR = ../libMDU
LIBDIR = $(MDUDIR)
INCDIR = $(MDUDIR)
INSTDIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox
else
MDUDIR = /u/develop/mj001b/tools
LIBDIR = $(MDUDIR)/lib
INCDIR = $(MDUDIR)/include
INSTDIR	= /u/develop/mj001b/tools/bin/linux
endif

DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN

CC     = gcc
AR     = ar
CFLAGS = -c -g -O2 $(CPU) -Wall $(DEFINES) -malign-double

all: $(TARGET)

.SUFFIXES: .o
.c.o:
	$(CC) $(CFLAGS) -I. -I$(INCDIR) $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) -L$(LIBDIR) -lMDU

install:
	cp $(TARGET) $(INSTDIR)

clean:
	-rm -f *.o
	-rm -f $(TARGET)

#---------------------------------------------------------------

