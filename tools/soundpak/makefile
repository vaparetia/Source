#
#	makefile for soundpak
#

TARGET	= soundpak
OBJECTS	= soundpak.o generate.o print.o parse.o
CFLAGS = -c -I ../gclconv

.c.o :
	gcc $(CFLAGS) $<

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	gcc $(OBJECTS) -o $(TARGET)

clean :
	-rm $(OBJECTS) $(TARGET)

install :
ifeq ($(MGS2_WIN32),1)
	cp $(TARGET) /u/develop/mj001b/tools/bin/linux
else
	cp $(TARGET) /u/develop/mj001b/exports/tools/bin/mgs2xbox
endif
