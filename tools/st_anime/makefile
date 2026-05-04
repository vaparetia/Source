# 
# makfile for st_anime
# 
# st_anime
#	
# by T.Morita 2000 Feb. ¡Á
# $Id: makefile,v 1.7 2002/04/19 02:14:30 usr04098 Exp $
#

SHELL  = /bin/sh
LEX    = flex
YACC   = bison
CC     = gcc
AR     = ar

SRCY   = xsi.y
SRCL   = xsi.l

SRCLL  = lex.yy.c
SRCYY  = y.tab.c
SRCYH  = $(SRCYY:.c=.h)

ifeq ($(MGS2_XBOX),1)

TARGET = st_anime.xbox
DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN -DYYERROR_VERBOSE -D_XBOX

else

TARGET = st_anime
DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN -DYYERROR_VERBOSE

endif

ifeq ($(MGS2_WIN32),1)
INSTDIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox
else
INSTDIR = /u/develop/mj001b/tools/bin/linux
endif

SRCS   = main.c create.c parse.c output.c list.c  misc.c  $(SRCYY) $(SRCLL)
OBJS   = $(SRCS:.c=.o)

#MDUDIR = /u/develop/mj001b/tools
#MDU_INC = $(MDUDIR)/include
#MDU_LIB = $(MDUDIR)/lib
MDUDIR = ../libMDU
MDU_INC = $(MDUDIR)
MDU_LIB	 = $(MDUDIR)

LIB    = -lMDU -lm

CPU    = -m486
CFLAGS = -c -g -O2 $(CPU) -Wall $(DEFINES) -I. -I$(MDU_INC) -malign-double


all: $(TARGET)


diag:
	$(YACC) -d $(SRCY) -v -o $(SRCYY)

$(SRCLL): $(SRCL)
	$(LEX) $(SRCL)

$(SRCYY): $(SRCY)
	$(YACC) -d $(SRCY) -o $(SRCYY)


.SUFFIXES: .o
.c.o:
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) -L$(MDU_LIB) $(LIB)

install:
	cp $(TARGET) $(INSTDIR)

clean:
	-rm -f *.o core *~
	-rm -f $(TARGET)
	-rm -f $(SRCYY) $(SRCYH) $(SRCLL) y.output

#---------------------------------------------------------------
