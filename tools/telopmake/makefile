#
#	makefile
#	$Id: makefile,v 1.1 2002/07/12 04:10:58 usr01475 Exp $
#

TARGET	= telopmake
OBJECTS	= telopmake.o parse.o mymalloc.o print.o

.c.o :
	$(CC) -c -O -malign-double -Wall $<

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) -lz

clean :
	-rm $(OBJECTS) $(TARGET)

install:
	cp $(TARGET) /u/develop/mj001b/tools/bin/linux


