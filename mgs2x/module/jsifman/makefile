OBJECTS = libmain.o rvman.o

IOP_INCDIR = ../include
IOP_LIBDIR = ../lib
IOP_IRXDIR = ../irx

export OBJECTS IOP_INCDIR IOP_LIBDIR IOP_IRXDIR

all:
	make -f makefile.iop
	make -f makefile.ee

clean:
	make -f makefile.iop clean
	make -f makefile.ee clean
