#
#	makefile for convinf
#	$Id: makefile,v 1.5 2002/04/05 07:29:44 usr01475 Exp $
#

TARGET	= convinf
OBJECTS	= main.o infload.o mymalloc.o strcode.o fontconv.o caption.o

TARGET2 = listinf
OBJECTS2 = listinf.o infload.o mymalloc.o strcode.o

TARGET3 = inf2txt
OBJECTS3 = inf2txt.o

TARGET4 = chginfstr
OBJECTS4 = chginfstr.o

TARGET5 = capcheck
OBJECTS5 = capcheck.o font.o

COPTION = -O2 -g
LDOPTION = -g

.c.o :
	$(CC) $(COPTION) -Wall -c $<

all : $(TARGET) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5)

$(TARGET) : $(OBJECTS)
	$(CC) $(LDOPTION) $^ -o $@

$(TARGET2) : $(OBJECTS2)
	$(CC) $(LDOPTION) $^ -o $@

$(TARGET3) : $(OBJECTS3)
	$(CC) $(LDOPTION) $^ -o $@

$(TARGET4) : $(OBJECTS4)
	$(CC) $(LDOPTION) $^ -o $@

$(TARGET5) : $(OBJECTS5)
	$(CC) $(LDOPTION) $^ -o $@

clean :
	-rm *.o $(TARGET) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5)

install:
	cp $(TARGET) $(TARGET2) $(TARGET4) $(TARGET5) /u/develop/mj001b/tools/bin/linux

font.o : font.c font.h
	$(CC) $(COPTION) -c -DCHECK_ONLY $< -o $@

