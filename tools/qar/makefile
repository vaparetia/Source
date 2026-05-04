#
#	makefile for qar
#	$Id: makefile,v 1.2 2002/04/10 02:35:39 usr04098 Exp $
#

TARGET	= qar
OBJECTS	= qar.o strcode.o

.c.o :
	$(CC) -c -O -Wall $<

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

clean :
	-rm -f $(OBJECTS) $(TARGET)

install:
ifeq ($(MGS2_WIN32),1)
	cp $(TARGET) /u/develop/mj001b/exports/tools/bin/mgs2xbox
else
	cp $(TARGET) /u/develop/mj001b/tools/bin/linux
endif
