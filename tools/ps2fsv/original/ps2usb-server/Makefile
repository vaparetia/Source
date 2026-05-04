SWITCH	=	-D_DEBUG_
OPTION	=	-O0 $(SWITCH)
LIB	=

CC	=	gcc

PROG	=	ps2d

OBJECT	=	main.o \
		linQio.o \
		txresource.o \
		util.o

all:	$(PROG)

clean:;	rm -f $(PROG) $(OBJECT) *~ *.bak core

$(PROG):	$(OBJECT)
	$(CC) -o $(PROG) $(OBJECT) $(LIB)

.c.o:;	$(CC) $(OPTION) -c $*.c -o $*.o

main.o:		main.c config.h txresource.h util.h linQio.h
linQio.o:	linQio.c linQio.h config.h
txresource.o:	txresource.c txresource.h
util.o:		util.c util.h
