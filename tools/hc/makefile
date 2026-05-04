#
#	makefile for hc
#	$Id: makefile,v 1.2 2001/04/10 01:55:22 usr01475 Exp $
#

TARGET	= hc
OBJECTS	= hc.o expr.o print.o parse.o mymalloc.o strcode.o
TARGET2 = char
OBJECTS2 = char.o

.c.o :
	$(CC) -c -O -Wall $<

all : $(TARGET) $(TARGET2)

$(TARGET) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

$(TARGET2) : $(OBJECTS2)
	$(CC) $(OBJECTS2) -o $(TARGET2)

clean :
	-rm $(OBJECTS) $(TARGET)
	-rm $(OBJECTS2) $(TARGET2)

install:
	cp $(TARGET) /u/develop/mj001b/tools/bin/linux
	cp $(TARGET2) /u/develop/mj001b/tools/bin/linux


