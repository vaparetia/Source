# 
# makfile for mdl2kc2
# 
# mdl2kc2 : mdl вк kms, cv2 converter for MGS2 on linux
#	
# by M.Sonoyama 1999 Sep. б┴
# Modified by K.Kano , 11/13/1999
#
# $Id: makefile,v 1.8 2000/10/11 05:42:44 usr01363 Exp $ 
# Konami Computer Entertainment Japan West CS1
#

SHELL = /bin/sh

TARGET = mdl2kc
CSRC   = main.c linux_p.c 

OBJS   = $(CSRC:.c=.o)

CPU    = -m486
MDLLIB = /u/develop/mj001b/tools/vrs/lib
#MDLLIB = /usr/local/develop/tools/vrs
INCPATH = $(MDLLIB)/include
LIBPATH = $(MDLLIB)/lib
SUPPORTPATH = $(MDLLIB)/support/include

MDUDIR = /u/develop/mj001b/tools
INCPATH2 = $(MDUDIR)/include
LIBPATH2 = $(MDUDIR)/lib
#MDUDIR = /usr/local/develop/tools/libMDU
#INCPATH2 = $(MDUDIR)
#LIBPATH2 = $(MDUDIR)

#MDUDIR = ../libMDU
#INCPATH2 = $(MDUDIR)
#LIBPATH2 = $(MDUDIR)

LIBS = -lMDU -lSFDCL$(CPU) -lP3DCL$(CPU) -lSFDCL$(CPU) -lm -lc

TOOLDIR = /u/develop/mj001b/tools
INSTDIR = $(TOOLDIR)/bin/linux
BACKUPDIR = $(TOOLDIR)/src/m_sonoyama/mdl2kms

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
	-rm -f $@
	$(CC) -o $(TARGET) $(OBJS) -I. -I$(INCPATH) -I$(INCPATH2) \
		-L$(LIBPATH2) -L$(LIBPATH) $(LIBS)
	-rm -f *.o

install:
	cp $(TARGET) $(INSTDIR)
#	-cp makefile *.c $(BACKUPDIR)
clean:
	-rm -f *.o
	-rm -f $(TARGET)

#---------------------------------------------------------------

