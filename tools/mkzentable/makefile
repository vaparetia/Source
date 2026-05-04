#
#	makefile for mkzentable
#	$Id: makefile,v 1.1.1.1 1999/12/08 05:12:43 usr01475 Exp $
#

TARGET	= mkzentable
OBJECTS	= mkzentable.o

.c.o :
	$(CC) -c -O -Wall $<

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

clean :
	-rm $(OBJECTS) $(TARGET)

install:
	cp $(TARGET) /u/develop/mj001b/tools/bin/linux


