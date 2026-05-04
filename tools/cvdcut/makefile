# 
# makfile for mdl2kms
# 
# cvdcut : cv2 data cutter for MGS2 on linux
#	
# by M.Sonoyama 1999 Sep. ¡Á
# Modified by K.Kano , 11/13/1999
#
# $Id: makefile,v 1.4 2002/04/10 01:23:00 usr04098 Exp $ 
# Konami Computer Entertainment Japan West CS1
#

SHELL = /bin/sh

TARGET = cvdcut
CSRC   = cvdcut.c linux_p.c option.c

OBJS   = $(CSRC:.c=.o)

CPU    = -m486

ifeq ($(MGS2_WIN32),1)
MDLLIB = /u/develop/mj001b/exports/tools/Developer/lib
INSTDIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox
else
MDLLIB = /u/develop/mj001b/tools/vrs/lib
INSTDIR = /u/develop/mj001b/tools/bin/linux
endif
INCPATH = $(MDLLIB)/include
LIBPATH = $(MDLLIB)/lib
SUPPORTPATH = $(MDLLIB)/support/include

#MDUDIR = /u/develop/mj001b/tools
#INCPATH2 = $(MDUDIR)/include
#LIBPATH2 = $(MDUDIR)/lib
#MDUDIR = /usr/local/develop/tools/libMDU
#INCPATH2 = $(MDUDIR)
#LIBPATH2 = $(MDUDIR)

MDUDIR = ../libMDU
INCPATH2 = $(MDUDIR)
LIBPATH2 = $(MDUDIR)

LIBS = -lMDU -lSFDCL$(CPU) -lP3DCL$(CPU) -lSFDCL$(CPU) -lm -lc

DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN

CC     = gcc
AR     = ar
CFLAGS = -c -g -O2 $(CPU) -Wall $(DEFINES) -I. -I$(INCPATH) -I$(INCPATH2) \
	 -I$(SUPPORTPATH) -L$(LIBPATH) -L$(LIBPATH2) -malign-double

all: $(TARGET)

.SUFFIXES: .o
.c.o:
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) -I. -I$(INCPATH) -I$(INCPATH2) \
		-L$(LIBPATH2) -L$(LIBPATH) $(LIBS)

install:
	cp $(TARGET) $(INSTDIR)
clean:
	-rm -f *.o
	-rm -f $(TARGET)

#---------------------------------------------------------------

