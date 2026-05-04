# 
# makfile for arkms
# 
# arkms2 : Get objects you need from KMSs for MGS2 on linux
#	
# Programmed by K.Kano , 12/8/1999
#
# $Id: makefile,v 1.4 2002/04/17 11:46:38 usr04098 Exp $ 
# Konami Computer Entertainment Japan West CS1
#

SHELL = /bin/sh


ifeq ($(MGS2_XBOX),1)
TARGET = arkms.xbox
else
TARGET = arkms
endif
CSRC   = arkms2.c linux_p.c option.c

OBJS   = $(CSRC:.c=.o)

CPU    = -m486

ifeq ($(MGS2_WIN32),1)
MDLLIB = /u/develop/mj001b/exports/tools/Developer/lib
MDUDIR = ../libMDU
INCPATH2 = $(MDUDIR)
LIBPATH2 = $(MDUDIR)
INSTDIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox
else
MDLLIB = /u/develop/mj001b/tools/vrs/lib
MDUDIR = /u/develop/mj001b/tools
INCPATH2 = $(MDUDIR)/include
LIBPATH2 = $(MDUDIR)/lib
INSTDIR = /u/develop/mj001b/tools/bin/linux
endif

INCPATH = $(MDLLIB)/include
LIBPATH = $(MDLLIB)/lib
SUPPORTPATH = $(MDLLIB)/support/include

#MDUDIR = /usr/local/develop/tools/libMDU
#INCPATH2 = $(MDUDIR)
#LIBPATH2 = $(MDUDIR)

#MDUDIR = ../libMDU
#INCPATH2 = $(MDUDIR)
#LIBPATH2 = $(MDUDIR)

LIBS = -lMDU -lSFDCL$(CPU) -lP3DCL$(CPU) -lSFDCL$(CPU) -lm -lc


ifeq ($(MGS2_XBOX),1)
DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN -D_XBOX
else
DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN
endif

CC     = gcc
AR     = ar
CFLAGS = -c -g -O2 $(CPU) -Wall $(DEFINES) -malign-double

all: $(TARGET)

.SUFFIXES: .o
.c.o:
	$(CC) $(CFLAGS) -I. -I$(INCPATH) -I$(INCPATH2) -I$(SUPPORTPATH) $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) -L$(LIBPATH2) -L$(LIBPATH) $(LIBS)

install:
	cp $(TARGET) $(INSTDIR)
clean:
	-rm -f *.o
	-rm -f $(TARGET)

#---------------------------------------------------------------

