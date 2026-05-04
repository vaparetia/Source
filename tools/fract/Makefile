#
# Makefile for UVMap 
#	
#	Tue Nov 16 07:45:11 JST 1999
#	T.Morita
#
#	$Id: Makefile,v 1.6 2002/06/13 01:35:48 usr04098 Exp $
#
CC     = gcc
AR     = ar


SRCS   = fract.c event.c create.c output.c bevel.c strcode.c
OBJS   = $(SRCS:.c=.o)


ifeq ($(MGS2_WIN32),1)

LIBS   = -lm
LIBDIR = 
CFLAGS = -c -g -O2 -m486 -Wall -malign-double -D_WIN32
INSTDIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox

else

LIBS   = -lX11 -lm
LIBDIR = /usr/X11R6/lib/
CFLAGS = -c -g -O2 -m486 -Wall -malign-double -I../libMDU
INSTDIR = /u/develop/mj001b/tools/bin/linux

endif


ifeq ($(MGS2_XBOX),1)
TARGET     = fract.xbox
CFLAGS    := $(CFLAGS) -D_XBOX
else
TARGET     = fract
endif


all: $(TARGET) $(XTARGET)

.SUFFIXES: .o .xo
.c.o:
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) -L$(LIBDIR) $(LIBS)

install:
	cp $(TARGET) $(XTARGET) $(INSTDIR)
#	cp makefile *.c $(BACKUPDIR)

clean:
	-rm -f *.o *~ core
	-rm -f $(TARGET)

#---------------------------------------------------------------

