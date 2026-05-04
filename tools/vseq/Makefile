SWITCH	=

OPTION	=	-O2 $(SWITCH)
LIB	=

CC	=	gcc
INSTDIR	=	/usr/local/bin

PROG	=	vseq

OBJECT	=	main.o \
		binseq.o \
		lang_env.o

all:	$(PROG)

clean:;	rm -rf $(PROG) $(OBJECT) *~ *.bak core

install:
	install -c $(PROG) $(INSTDIR)


$(PROG):	$(OBJECT)
	$(CC) $(OBJECT) -o $(PROG) $(LIB)

.c.o:;	$(CC) $(OPTION) -c $*.c -o $*.o


main.o:		main.c main.h lip.h binseq.h lang_env.h

binseq.o:	binseq.c binseq.h main.h lip.h lip_japanese.h lip_english.h \
		lang_env.h

lang_env.o:	lang_env.c lang_env.h main.h lip_japanese.h lip_english.h \
		lip.h
