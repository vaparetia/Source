SWITCH	=

CC	=	gcc
LINK	=	gcc
OPTION	=	-O2 $(SWITCH)
LIB	=	
INSTDIR	=	/usr/local/bin

OBJECT	=	main.o \
		split.o \
		util.o

PROG	=	vstream

all:	$(PROG)

clean:;	rm -f $(PROG) $(OBJECT) *~ *.bak core

install:
	install -c $(PROG) $(INSTDIR)

$(PROG):	$(OBJECT)
	$(LINK) -o $(PROG) $(OBJECT) $(LIB)

.c.o:;	$(CC) $(OPTION) -c $*.c -o $*.o

main.o:		main.c main.h split.h seq.h
split.o:	split.c split.h seq.h util.h
util.o:		util.c util.h
