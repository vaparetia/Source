# 
# makfile for makecm2
# 
# makcm2 : texture archiver for MGS2 on linux
#	
# by M.Sonoyama 1999 Sep. ¡Á
# $Id: makefile,v 1.1 2000/10/12 11:38:54 usr02011 Exp $
# Konami Computer Entertainment Japan West CS1
#

SHELL = /bin/sh

TARGET = makevar
CSRC   = main.c

OBJS   = $(CSRC:.c=.o)

CPU    = -m486

LIBDIR = /u/develop/mj001b/tools/lib
INCDIR = $(LIBDIR)/../include

INSTDIR = /u/develop/mj001b/tools/bin/linux
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

clean:
	-rm -f *.o
	-rm -f $(TARGET)

#---------------------------------------------------------------

