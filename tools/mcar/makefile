# 
# makfile for st_anime
# 
# mcar
#	
# by T.Morita 2000 Feb. ¡Á
# $Id: makefile,v 1.5 2001/11/16 05:38:35 usr04761 Exp $
#

SHELL  = /bin/sh
CC     = gcc

DUMP   = mcar
SRCS  = mcar.c
OBJS  = $(SRCS:.c=.o)

INCDIR = 
LIB    = -lm

CPU    = -m486
INSTDIR = /u/develop/mj001b/tools/bin/linux
DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN
CFLAGS = -c -g -O2 $(CPU) -Wall $(DEFINES) -I. -malign-double


all: $(DUMP)

.SUFFIXES: .o
.c.o:
	$(CC) $(CFLAGS) $< -o $@

$(DUMP): $(OBJS)
	$(CC) -o $(DUMP) $(OBJS) $(LIB)

clean:
	-rm -f *.o core *~
	-rm -f $(TARGET)

install:
	-cp -pf * /u/develop/mj001b/develop/tools/mcar
	cp mcget.sh /u/develop/mj001b/tools/bin/linux/mcget
	cp mcput.sh /u/develop/mj001b/tools/bin/linux/mcput
	cp mcformat.sh /u/develop/mj001b/tools/bin/linux/mcformat
	cp mcar /u/develop/mj001b/tools/bin/linux/

#---------------------------------------------------------------

