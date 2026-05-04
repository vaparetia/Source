#
#	makefile for stream
#

TARGET	= stream
OBJECTS	= stream.o generate.o print.o parse.o

TARGET2 = dumpstream
OBJECTS2 = dumpstream.o

CFLAGS = -c -I ../gclconv

.c.o :
	gcc $(CFLAGS) $<

all : $(TARGET) $(TARGET2)

$(TARGET) : $(OBJECTS)
	gcc -static $(OBJECTS) -o $(TARGET)

$(TARGET2) : $(OBJECTS2)
	gcc -static $(OBJECTS2) -o $(TARGET2)

clean :
	-rm $(OBJECTS) $(TARGET) $(OBJECTS2) $(TARGET2)

install :
ifeq ("$(OSTYPE)","linux")
	cp $(TARGET) $(TARGET2) /u/develop/mj001b/tools/bin/linux
else
	cp $(TARGET) $(TARGET2) /u/develop/mj001b/exports/tools/bin/mgs2xbox
endif


