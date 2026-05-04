# 
# makfile for makecm2
# 
# makcm2 : texture archiver for MGS2 on linux
#	
# by M.Sonoyama 1999 Sep. ¡Á
# $Id: makefile,v 1.5 1999/11/05 08:28:28 usr02011 Exp $
# Konami Computer Entertainment Japan West CS1
#

SHELL = /bin/sh

TARGET = makecm2
CSRC   = makecm2.c

OBJS   = $(CSRC:.c=.o)

CPU    = -m486

LIBDIR = /u/develop/mj001b/tools/lib
INCDIR = $(LIBDIR)/../include
#LIBDIR = /usr/local/develop/tools/libMDU
#INCDIR = $(LIBDIR)

INSTDIR = /u/develop/mj001b/tools/bin/linux
BACKUPDIR = /u/develop/mj001b/tools/src/m_sonoyama/makecm2
DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN

CC     = gcc
AR     = ar
CFLAGS = -c -g -O2 $(CPU) -Wall $(DEFINES) -I. -malign-double

all: $(TARGET)

.SUFFIXES: .o
.c.o:
	$(CC) $(CFLAGS) -I$(INCDIR) $< -o $@

$(TARGET): $(OBJS)
	-rm -f $@
	$(CC) -o $(TARGET) $(OBJS) -I$(INCDIR) -L$(LIBDIR) -lMDU
	-rm -f *.o

install:
	cp $(TARGET) $(INSTDIR)
#	cp makefile *.c $(BACKUPDIR)

clean:
	-rm -f *.o
	-rm -f $(TARGET)

#---------------------------------------------------------------

