# SWITCH	=	-DDEBUG -DYYDEBUG=9
SWITCH	=	
OPTION	=	$(SWITCH) -O2
CC	=	gcc

INSTALL	=	/u/develop/mj001b/tools/bin/linux

## yacc & lex を使用する場合
# YACC	=	yacc
# LEX	=	lex

# bison & flex 使用する場合
YACC	=	bison -y
LEX	=	flex

LIB	=	-lfl

YYSRC	=	y.tab.c
LEXSRC	=	lex.yy.c

OBJECT	=	main.o \
		2d_data.o \
		util.o \
		futil.o \
		fname.o \
		mgs2.o \
		y.tab.o

PROG	=	layout2d

all:	$(PROG)

install:
	install -c $(PROG) $(INSTALL)

clean:;	rm -rf $(PROG) $(OBJECT) $(YYSRC) $(LEXSRC) *~ core

$(PROG):	$(OBJECT)
	$(CC) -o $(PROG) $(OBJECT) $(LIB)

y.tab.c:	2d_layout.y
	$(YACC) 2d_layout.y

lex.yy.c:	2d_layout.l
	$(LEX) 2d_layout.l

.c.o:;	$(CC) $(OPTION) -c $*.c -o $*.o

y.tab.o:	y.tab.c lex.yy.c 2d_data.h util.h

main.o:		main.c main.h 2d_data.h yacc_parser.h futil.h fname.h \
		util.h mgs2.h

2d_data.o:	2d_data.c 2d_data.h yacc_parser.h
util.o:		util.c util.h
futil.o:	futil.c futil.h
fname.o:	fname.c fname.h
mgs2.o:		mgs2.c mgs2.h futil.h 2d_data.h

