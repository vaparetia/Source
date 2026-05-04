#
#	makefile
#	$Id: makefile,v 1.1 2002/03/27 04:16:04 usr01475 Exp $
#

TARGET	= zlibenc
OBJECTS	= zlibenc.o

.c.o :
	$(CC) -c -O -Wall $<

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) -lz

clean :
	-rm $(OBJECTS) $(TARGET)

install:
	cp $(TARGET) /u/develop/mj001b/tools/bin/linux


