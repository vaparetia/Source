# SWITCH		=	-D_DEBUG_
SWITCH		=	
CC		=	gcc
OPTION		=	-Wall -O2 $(SWITCH)
INCLUDE		=
LIB		=	-lm

BASICS		=	token.o \
			label.o \
			fname.o

SYNTAX		=	command.o \
			objdef.o \
			objstat.o \
			objact.o

OBJECTS		=	main.o \
			util.o \
			output.o \
			$(BASICS) \
			$(SYNTAX)

PROG		=	layout2d

TESTOBJ		=	test.o token.o util.o

TESTPROG	=	test

VIEWOBJ		=	view.o

VIEW		=	view

all:	$(PROG) $(TESTPROG) $(VIEW)

clean:;		rm -f $(PROG) $(OBJECTS) $(TESTPROG) $(TESTOBJ) $(VIEW) $(VIEWOBJ) *~ core

install:
	/usr/bin/install -c $(PROG)  /u/develop/mj001b/tools/bin/linux


$(PROG):	$(OBJECTS)
	$(CC) $(INCLUDE) $(LIB) -o $(PROG) $(OBJECTS)

$(TESTPROG):	$(TESTOBJ)
	$(CC) $(INCLUDE) $(LIB) -o $(TESTPROG) $(TESTOBJ)

$(VIEW):	$(VIEWOBJ)
	$(CC) $(INCLUDE) $(LIB) -o $(VIEW) $(VIEWOBJ)

.c.o:;	$(CC) $(OPTION) -c $*.c -o $*.o


#
# °ÍÂ¸´Ø·¸
#
main.o:		main.c main.h token.h debug.h label.h \
		command.h fname.h my_malloc.h

util.o:		util.c util.h my_malloc.h debug.h

# BASICS
token.o:	token.c token.h util.h my_malloc.h debug.h
label.o:	label.c label.h my_malloc.h util.h debug.h

# SYNTAX
command.o:	command.c command.h \
		my_malloc.h util.h  \
		token.h label.h     \
		objdef.h objstat.h objact.h output.h \
		debug.h

objdef.o:	objdef.c objdef.h  \
		my_malloc.h util.h \
		token.h label.h    \
		command.h          \
		debug.h

objstat.o:	objstat.c objstat.h \
		my_malloc.h util.h  \
		token.h label.h     \
		command.h objdef.h  \
		debug.h

objact.o:	objact.c objact.h   \
		my_malloc.h util.h  \
		token.h label.h     \
		command.h           \
		debug.h

output.o:	output.c output.h  \
		my_malloc.h util.h \
		token.h label.h    \
		command.h objdef.h \
		objstat.h objact.h \
		debug.h

fname.o:	fname.c fname.h my_malloc.h


test.o:		test.c token.h command.h label.h

view.o:		view.c view.h
