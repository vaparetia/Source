#
#	Makefile for hzx_conv2
#
#	by M.Sonoyama 1999.Nov ¡Á
#	$Id: Makefile,v 1.2 1999/11/18 01:23:11 usr02011 Exp $

TARGET = hzx_conv2
CSRC = main.c
OBJS = $(CSRC:.c=.o)

MDLLIB = /u/develop/mj001b/tools/vrs/lib
#MDLLIB = /usr/local/develop/tools/vrs
INCPATH = $(MDLLIB)/include
LIBPATH = $(MDLLIB)/lib
SUPPORTPATH = $(MDLLIB)/support/include

#MDUDIR = /u/develop/mj001b/tools
#INCPATH2 = $(MDUDIR)/include
#LIBPATH2 = $(MDUDIR)/lib
MDUDIR = /usr/local/develop/tools/libMDU
INCPATH2 = $(MDUDIR)
LIBPATH2 = $(MDUDIR)

LIBS = -lMDU -lm -lc

TOOLDIR = /u/develop/mj001b/tools
INSTDIR = $(TOOLDIR)/bin/linux

DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN

CC = gcc
CFLAGS = -g -O2 -m486 -Wall $(DEFINES) -pipe -I. -I$(INCPATH) -I$(INCPATH2) \
	 -L$(LIBPATH) -L$(LIBPATH2) -malign-double

all: $(TARGET)

.SUFFIXES: .o
.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	-rm -f $@
	$(CC) -o $(TARGET) $(OBJS) -I. -I$(INCPATH) -I$(INCPATH2) \
		-L$(LIBPATH) -L$(LIBPATH2) $(LIBS)
	-rm -f *.o

install:
	cp $(TARGET) $(INSTDIR)

clean:
	-rm -f *.o
	-rm -f $(TARGET)

#-----------------------------------------------------------------



