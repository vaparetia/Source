SWITCH	=	-D_DEBUG_
OPTION	=	-O2 $(SWITCH)
CC	=	gcc
LINK	=	gcc
LIB	=

INSTALL	=	/usr/local/bin

VXCODEC	=	decode.o \
		decxa.o  \
		enctbl.o \
		tools.o  \
		dectbl.o \
		encode.o \
		encxa.o  \
		quantize.o

OBJECT	=	pcm2wav.o util.o vagdata.o $(VXCODEC)

PROG	=	pcm2wav

all:	$(PROG)

install:
	install -c $(PROG) $(INSTALL)

clean:;	rm -f $(PROG) $(OBJECT) *~ *.bak core

$(PROG):	$(OBJECT)
	$(LINK) -o $(PROG) $(OBJECT) $(LIB)

.c.o:;	$(CC) $(OPTION) -c $*.c -o $*.o
