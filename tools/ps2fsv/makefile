SWITCH	=	-D_DEBUG_
OPTION	=	-O2 $(SWITCH)
LIB	=

CC	=	gcc

PROG	=	ps2fsv

OBJECT	=	main.o linQio.o

all:	$(PROG)

clean:;	rm -f $(PROG) $(OBJECT) *~ *.bak core

$(PROG):	$(OBJECT)
	$(CC) -o $(PROG) $(OBJECT) $(LIB)

.c.o:;	$(CC) $(OPTION) -c $*.c -o $*.o

main.o:		main.c
linQio.o:	linQio.h

install:
	cp $(PROG) /u/develop/mj001b/tools/bin/linux

