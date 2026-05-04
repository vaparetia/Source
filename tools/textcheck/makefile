#
#	makefile for textcheck
#	$Id: makefile,v 1.3 2000/12/11 11:00:25 usr01475 Exp $
#

TARGET	= textcheck
OBJECTS	= textcheck.o

.c.o :
	$(CC) -c -O -Wall $<

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) -static $(OBJECTS) -o $(TARGET)

clean :
	-rm $(OBJECTS) $(TARGET)

install:
	cp $(TARGET) /u/develop/mj001b/tools/bin/linux


