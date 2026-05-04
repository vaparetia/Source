CC	=	gcc
OPTION	=	-O2
LIB	=
INSTDIR	=	/u/develop/mj001b/tools/bin/linux

PROG	=	bdmp
OBJECT	=	bdmp.o

all:	$(PROG)

clean:;	rm -f $(PROG) $(OBJECT) *~ *.bak core

install:
	install -c $(PROG) $(INSTDIR)

$(PROG):	$(OBJECT)
	$(CC) -o $(PROG) $(OBJECT) $(LIB)

.c.o:;	$(CC) $(OPTION) $(INCLUDE) -c $*.c -o $*.o

bdmp.o:		bdmp.c
