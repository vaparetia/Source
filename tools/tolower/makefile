#
#	makefile for tolower
#	$Id: makefile,v 1.1 1999/11/24 07:17:04 usr01475 Exp $
#

TARGET	= tolower
OBJECTS	= tolower.o

.c.o :
	$(CC) -c -O -Wall $<

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

clean :
	-rm $(OBJECTS) $(TARGET)

install:
	cp $(TARGET) /u/develop/mj001b/tools/bin/linux


