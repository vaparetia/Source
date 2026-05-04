#
#	makefile for mkcdimg
#	$Id: makefile,v 1.4 2002/04/09 06:17:07 usr01475 Exp $
#

TARGET	= mkcdimg
OBJECTS	= mkcdimg.o bldstage.o parse.o print.o mymalloc.o

TARGET2 = convhd
OBJECTS2 = main_convhd.o convhd.o

.c.o :
	$(CC) -c -g -O -Wall $<

all : $(TARGET) $(TARGET2)

$(TARGET) : $(OBJECTS)
	$(CC) $^ -g -o $@ -lz

$(TARGET2) : $(OBJECTS2)
	$(CC) $^ -g -o $@ -lz

clean :
	-rm -f $(OBJECTS) $(TARGET)
	-rm -f $(OBJECTS2) $(TARGET2)

install:
	cp $(TARGET) /u/develop/mj001b/tools/bin/linux


