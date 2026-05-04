SWITCH	=	

CC	=	gcc
LINK	=	gcc
OPTION	=	-O2 $(SWITCH)
LIB	=	
PROG	=	dictool

OBJECT	=	main.o \
		dic.o \
		vec.o \
		group.o \
		token.o \
		util.o

all:	$(PROG)

clean:;	rm -f $(OBJECT) $(PROG) *~ *.bak core

$(PROG):	$(OBJECT)
	$(LINK) $(OBJECT) -o $(PROG) $(LIB)

.c.o:;	$(CC) $(OPTION) -c $*.c -o $*.o

main.o:		main.c main.h vec.h group.h dic.h token.h
vec.o:		vec.c vec.h token.h
group.o:	group.c group.h token.h
dic.o:		dic.c dic.h vec.h group.h token.h util.h
token.o:	token.c token.h
util.o:		util.c util.h
