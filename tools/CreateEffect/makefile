# 
# makfile for st_anime
# 
# st_anime
#	
# by T.Morita 2000 Feb. ¡Á
# $Id: makefile,v 1.2 2002/04/10 02:34:56 usr04098 Exp $
#

SHELL  = /bin/sh
LEX    = flex
YACC   = bison
CC     = gcc
AR     = ar

SRCY   = ParseGrm.y
SRCL   = ParseLex.l

SRCLL  = ParseLex.c
SRCYY  = ParseGrm.c
SRCYH  = $(SRCYY:.c=.h)

YYFLAGS = -v -d -o $(SRCYY)
LLFLAGS = -o$(SRCLL)

TARGET = ef_create
SRCS   = CreateEffect.c $(SRCYY) $(SRCLL)
OBJS   = $(SRCS:.c=.o)

CPU    = -m486

ifeq ($(MGS2_WIN32),1)
INSTDIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox
else
INSTDIR = /u/develop/mj001b/tools/bin/linux
endif
DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN -DC_LANG -DYYERROR_VERBOSE
CFLAGS = -c -g -O2 $(CPU) -Wall $(DEFINES) -I. -malign-double


all: $(TARGET)

$(SRCLL): $(SRCL)
	$(LEX) $(LLFLAGS) $(SRCL)

$(SRCYY): $(SRCY)
	$(YACC) $(YYFLAGS) $(SRCY)

CreateEffect.c: $(SRCYH)
$(SRCYH):$(SRCYY)
$(SRCLL):$(SRCYH)

.SUFFIXES: .o
.c.o:
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

install:
	cp $(TARGET) $(INSTDIR)

clean:
	-rm -f *.o core *~
	-rm -f $(TARGET)
	-rm -f $(SRCYY) $(SRCYH) $(SRCLL) *.output

#---------------------------------------------------------------
