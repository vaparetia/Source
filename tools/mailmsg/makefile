#
#	makefile for mailmsg
#	$Id: makefile,v 1.3 2002/05/21 08:29:34 usr01475 Exp $
#

TARGET	= mailmsg
OBJECTS	= mailmsg.o

ifeq ($(MGS2_WIN32),1)
INSTDIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox
else
INSTDIR = /u/develop/mj001b/tools/bin/linux
endif

.c.o :
	$(CC) -c -O -Wall $<

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) -static $(OBJECTS) -o $(TARGET)

clean :
	-rm $(OBJECTS) $(TARGET)

install:
	cp $(TARGET) $(INSTDIR)


