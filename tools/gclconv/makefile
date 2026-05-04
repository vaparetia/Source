#
#	makefile for gclconv
#

COMMON_OBJECTS = parse.o print.o strcode.o expr.o mymalloc.o symbol.o hash.o

CV_TARGET	= gclconv
CV_OBJECTS	= gclconv.o group.o prototable.o convert.o fontconv.o font.o $(COMMON_OBJECTS)

LK_TARGET	= gclk
LK_OBJECTS	= gclk.o variable.o $(COMMON_OBJECTS)

PL_TARGET = stridconv.pl gcltags.pl

#STRCODEOPT = -DSTRCODE16		# 16ビット文字列ID
STRCODEOPT =					# 24ビット文字列ID

COPTIONS  = -c -O2 -g $(STRCODEOPT) -Wall
LKOPTIONS = -g

.c.o :
	gcc $(COPTIONS) $<

all : $(CV_TARGET) $(LK_TARGET)

$(CV_TARGET) : $(CV_OBJECTS)
	gcc $(LKOPTIONS) $(CV_OBJECTS) -o $(CV_TARGET)

$(LK_TARGET) : $(LK_OBJECTS)
	gcc $(LKOPTIONS) $(LK_OBJECTS) -o $(LK_TARGET)

clean :
	-rm $(CV_OBJECTS) $(CV_TARGET)
	-rm $(LK_OBJECTS) $(LK_TARGET)
	-rm .#*

ifeq ("$(OSTYPE)","linux")
INSTALL_PATH = /u/develop/mj001b/tools/bin/linux
else
INSTALL_PATH = /u/develop/mj001b/exports/tools/bin/mgs2xbox
endif

install :
	chmod +x $(PL_TARGET)
	cp $(CV_TARGET) $(LK_TARGET) $(PL_TARGET) $(INSTALL_PATH)

gclconv.o : version.h
gclk.o : version.h

font.o : font.c font.h
	gcc $(COPTIONS) -DCHECK_ONLY -w $<
