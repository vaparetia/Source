#
#	makefile
#	$Id: makefile,v 1.2 2001/05/03 08:43:40 usr01475 Exp $
#

TARGET	= pss2stream
OBJECTS	= main.o generate.o

TARGET2 = ipu2stream
OBJECT2 = ipu2stream.o

.c.o :
	$(CC) -c -O -Wall $<

all : $(TARGET) $(TARGET2)

$(TARGET) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

$(TARGET2) : $(OBJECT2)
	$(CC) $(OBJECT2) -o $(TARGET2)

clean :
	-rm $(OBJECTS) $(TARGET)
	-rm $(OBJECT2) $(TARGET2)

install:
	cp $(TARGET) $(TARGET2) /u/develop/mj001b/tools/bin/linux
	cp mpv2sst.sh /u/develop/mj001b/tools/bin/linux/mpv2sst
	cp mpv2ist.sh /u/develop/mj001b/tools/bin/linux/mpv2ist



