#
#	makefile for makesetbl
#	$Id: makefile,v 1.1 2000/03/09 12:07:39 usr01749 Exp $
#


SHELL = /bin/sh

TARGET = makesetbl
CSRC   = makesetbl.c

DEF_TARGET = makesedef
DEF_CSRC   = makesedef.c

LST_TARGET = makeselst
LST_CSRC   = makeselst.c

OBJS   = $(CSRC:.c=.o)
LST_OBJS   = $(LST_CSRC:.c=.o)
DEF_OBJS   = $(DEF_CSRC:.c=.o)

CPU    = -m486

LIBDIR = /u/develop/mj001b/tools/lib
INCDIR = $(LIBDIR)/../include

INSTDIR = /u/develop/mj001b/tools/bin/linux
DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN

CC     = gcc
AR     = ar
CFLAGS = -c -g -O2 $(CPU) -Wall $(DEFINES) -I. -malign-double

all: $(TARGET) $(LST_TARGET) $(DEF_TARGET)

.SUFFIXES: .o
.c.o:
	$(CC) $(CFLAGS) -I$(INCDIR) $< -o $@

$(TARGET): $(OBJS)
	-rm -f $@
	$(CC) -o $(TARGET) $(OBJS) -I$(INCDIR) -L$(LIBDIR) -lMDU
	-rm -f *.o

$(LST_TARGET): $(LST_OBJS)
	-rm -f $@
	$(CC) -o $(LST_TARGET) $(LST_OBJS) -I$(INCDIR) -L$(LIBDIR) -lMDU
	-rm -f *.o

$(DEF_TARGET): $(DEF_OBJS)
	-rm -f $@
	$(CC) -o $(DEF_TARGET) $(DEF_OBJS) -I$(INCDIR) -L$(LIBDIR) -lMDU
	-rm -f *.o



install:
	cp $(TARGET) $(LST_TARGET) $(DEF_TARGET) $(INSTDIR)

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f $(LST_TARGET)
	-rm -f $(DEF_TARGET)

#---------------------------------------------------------------

