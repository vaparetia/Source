# SWITCH	=	-D_DEBUG_
SWITCH	=

CC	=	gcc
OPTION	=	-O2 $(SWITCH)

PROG	=	vmtn

LIB	=	-lm

# INSTDIR	=	/usr/local/bin
INSTDIR	=	/u/develop/mj001b/tools/bin/linux


MTNLIB	=	gte.o		\
		mtn_util.o	\
		quat.o		\
		mtn_blend.o

MTNHEAD	=	gte.h		\
		quat.h		\
		motion.h	\
		mtn_blend.h	\
		mtnlib.h

OBJECT	=	main.o		\
		proc.o		\
		make_motion.o	\
		read_seq.o	\
		mtn_list.o	\
		vmotion.o	\
		vmtn_multi.o	\
		inf_seq.o	\
		mem_util.o	\
		mtn_filter.o	\
		util.o		\
		code.o		\
		$(MTNLIB)

all:	$(PROG)

install:;	install -c $(PROG) $(INSTDIR)

clean:;	rm -f $(OBJECT) $(PROG) *~ *.bak core


$(PROG):	$(OBJECT)
	$(CC) $(OBJECT) $(LIB) -o $(PROG)

.c.o:
	$(CC) $(OPTION) -c $*.c -o $*.o


# Motion Library
gte.o:		gte.c $(MTNHEAD)
mtn_util.o:	mtn_util.c $(MTNHEAD)
quat.o:		quat.c $(MTNHEAD)
mtn_blend.o:	mtn_blend.c $(MTNHEAD)

main.o:		main.c main.h
proc.o:		proc.c proc.h make_motion.h read_seq.h vmotion.h vmtn_multi.h \
		inf_seq.h mem_util.h mtn_list.h mtn_filter.h $(MTNHEAD)

make_motion.o:	make_motion.c make_motion.h mtn_list.h mtn_blend.h vmotion.h \
		$(MTNHEAD)

read_seq.o:	read_seq.c read_seq.h mem_util.h

vmotion.o:	vmotion.c vmotion.h util.h $(MTNHEAD)

vmtn_multi.o:	vmtn_multi.c vmtn_multi.h vmotion.h mem_util.h

inf_seq.o:	inf_seq.c inf_seq.h infdata.h mem_util.h code.h

mtn_list.o:	mtn_list.c mtn_list.h
mem_util.o:	mem_util.c mem_util.h

mtn_filter.o:	mtn_filter.c mtn_filter.h $(MTNHEAD)

util.o:		util.c util.h
