# 
# makefile for mdloutline
#
# by S.Yamashita 2002/03/08
#
# $Id: makefile,v 1.3 2002/03/19 06:18:09 usr03692 Exp $
# Konami Computer Entertainment Japan West CS1
#

SHELL       = /bin/sh
TARGET      = mdloutline
CSRC        = _outline.c create.c scale.c output.c mdloutline.c
OBJS        = $(CSRC:.c=.o)
CPU         = -m486
MDLLIB      = /u/develop/mj001b/tools/vrs/lib
INCPATH     = $(MDLLIB)/include
LIBPATH     = $(MDLLIB)/lib
SUPPORTPATH = $(MDLLIB)/support/include
LIBS        = -Wl,-\( -lP3DCL$(CPU) -lSFDCL$(CPU) -Wl,-\) -lm -lc
TOOLDIR     = /u/develop/mj001b/tools
INSTDIR     = $(TOOLDIR)/bin/linux
DEFINES     = -D_LINUX -DIS_LITTLE_ENDIAN
CC          = gcc
AR          = ar
CFLAGS      = -c -g -O2 $(CPU) -Wall $(DEFINES) -I. -I$(INCPATH) -I$(SUPPORTPATH) -L$(LIBPATH) -malign-double

all: $(TARGET)

.SUFFIXES: .o
.c.o:
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	-rm -f $@
	$(CC) -o $(TARGET) $(OBJS) -I. -I$(INCPATH) \
		-L$(LIBPATH) $(LIBS)
	-rm -f *.o

install:
	cp $(TARGET) $(INSTDIR)

clean:
	-rm -f *.o
	-rm -f $(TARGET)

#-------------------------------------------------------------------------------

