# 
# makfile for id_check
# 
# txm : id conflict checker for MGS on linux
#	
# by M.Sonoyama 1999 Oct. ¡Á
# $Id: makefile,v 1.5 2002/02/07 06:55:05 usr04098 Exp $
# Konami Computer Entertainment Japan West CS1
#

SHELL = /bin/sh

TARGET = id_check
CSRC   = id_check.c

OBJS   = $(CSRC:.c=.o)

CPU    = -m486
LIBDIR = /u/develop/mj001b/tools
INSTDIR = /u/develop/mj001b/tools/bin/linux

DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN

CC     = gcc
AR     = ar
CFLAGS = -c -g -O2 $(CPU) -Wall $(DEFINES) -I. -malign-double

all: $(TARGET)

.SUFFIXES: .o
.c.o:
	$(CC) $(CFLAGS) -I$(LIBDIR)/include $< -o $@

$(TARGET): $(OBJS)
	-rm -f $@
	$(CC) -o $(TARGET) $(OBJS) -I$(LIBDIR)/include -L$(LIBDIR)/lib -lMDU
	-rm -f *.o

install:
	cp $(TARGET) id_checkall $(INSTDIR)
	cp check_kms_bmp check_dar_bmp $(INSTDIR)

clean:
	-rm -f *.o
	-rm -f $(TARGET)

#---------------------------------------------------------------

