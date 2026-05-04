# 
# makfile for mdl2kms
# 
# mdl2kms : mdl вк kms converter for MGS2 on linux
#	
# by M.Sonoyama 1999 Sep. б┴
# $Id: makefile,v 1.8 2001/01/19 08:12:33 usr02011 Exp $ 
# Konami Computer Entertainment Japan West CS1
#

SHELL = /bin/sh

#TARGET = mdl2kms
#CSRC   = main.c linux_p.c 

#OBJS   = $(CSRC:.c=.o)

TARGETS	= dumptex strcode texconv tsearch dumpkms dumpkm3 dumpcv2 \
		  mt3size

CPU    = -m486
MDLLIB = /u/develop/mj001b/tools/vrs/lib
INCPATH = $(MDLLIB)/include
LIBPATH = $(MDLLIB)/lib
SUPPORTPATH = $(MDLLIB)/support/include

MDUDIR = /u/develop/mj001b/tools
INCPATH2 = $(MDUDIR)/include
LIBPATH2 = $(MDUDIR)/lib
#MDUDIR = /usr/local/develop/tools/libMDU
#INCPATH2 = $(MDUDIR)
#LIBPATH2 = $(MDUDIR)

LIBS = -lMDU -lSFDCL$(CPU) -lP3DCL$(CPU) -lSFDCL$(CPU) -lm -lc

TOOLDIR = /u/develop/mj001b/tools
INSTDIR = $(TOOLDIR)/bin/linux

DEFINES = -D_UNIX -D_LINUX -DIS_LITTLE_ENDIAN

CC     = gcc
AR     = ar
CFLAGS = -c -g -O2 $(CPU) -Wall $(DEFINES) -I. -I$(INCPATH) -I$(INCPATH2) \
	 -I$(SUPPORTPATH) -L$(LIBPATH) -L$(LIBPATH2) -malign-double

.SUFFIXES: .o
.c.o:
	$(CC) $(CFLAGS) $< -o $@

all: 
	$(CC) $(CFLAGS) linux_p.c -o linux_p.o
	$(CC) $(CFLAGS) dumptex.c -o dumptex.o
	$(CC) -o dumptex dumptex.o -I. -I$(INCPATH) -I$(INCPATH2) \
		-L$(LIBPATH2) -L$(LIBPATH) $(LIBS)
	$(CC) $(CFLAGS) strcode.c -o strcode.o
	$(CC) -o strcode strcode.o -I. -I$(INCPATH) -I$(INCPATH2) \
		-L$(LIBPATH2) -L$(LIBPATH) $(LIBS)
	$(CC) $(CFLAGS) texconv.c -o texconv.o
	$(CC) -o texconv texconv.o -I. -I$(INCPATH) -I$(INCPATH2) \
		-L$(LIBPATH2) -L$(LIBPATH) $(LIBS)
	$(CC) -o texconv texconv.o -I. -I$(INCPATH) -I$(INCPATH2) \
		-L$(LIBPATH2) -L$(LIBPATH) $(LIBS)
	$(CC) $(CFLAGS) tsearch.c -o tsearch.o
	$(CC) -o tsearch tsearch.o linux_p.o -I. -I$(INCPATH) -I$(INCPATH2) \
		-L$(LIBPATH2) -L$(LIBPATH) $(LIBS)
	$(CC) $(CFLAGS) dumpkms.c -o dumpkms.o
	$(CC) -o dumpkms dumpkms.o -I. -I$(INCPATH) -I$(INCPATH2) \
		-L$(LIBPATH2) -L$(LIBPATH) $(LIBS)
	$(CC) $(CFLAGS) dumpkm3.c -o dumpkm3.o
	$(CC) -o dumpkm3 dumpkm3.o -I. -I$(INCPATH) -I$(INCPATH2) \
		-L$(LIBPATH2) -L$(LIBPATH) $(LIBS)
cv2:
	$(CC) $(CFLAGS) dumpcv2.c -o dumpcv2.o
	$(CC) -o dumpcv2 dumpcv2.o -I. -I$(INCPATH) -I$(INCPATH2) \
		-L$(LIBPATH2) -L$(LIBPATH) $(LIBS)
	$(CC) $(CFLAGS) mt3size.c -o mt3size.o
	$(CC) -o mt3size mt3size.o -I. -I$(INCPATH) -I$(INCPATH2) \
		-L$(LIBPATH2) -L$(LIBPATH) $(LIBS)
	-rm -f *.o

install:
	cp $(TARGETS) $(INSTDIR)
#	-cp makefile *.c $(BACKUPDIR)
clean:
	-rm -f *.o
	-rm -f $(TARGETS)

#---------------------------------------------------------------

