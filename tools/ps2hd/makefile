#
#
#
#

ifeq ($(wildcard PathDefs),)
PathDefs:
	iop-path-setup > PathDefs || (rm -f PathDefs ; exit 1)
	$(MAKE) all
else
include PathDefs
endif

#include $(word 1, $(wildcard Makedefs ../Makedefs ../../Makedefs \
#		../../../Makedefs ) )

IRX      = main.irx
SRCS     = main.c
OBJS    := $(SRCS:%.c=%.o)

CFLAGS   = -g -G 0 -Wall -O2 -I./include -Wall
ASFLAGS  = $(ASCOMFLAGS) -I./include
#LDFLAGS  = -noiopilb
LDFLAGS =

#EXILBS   = ../../lib/stdio.ilb \
#	   ../../lib/sysclib.ilb \
#	   ../../lib/loadcore.ilb \
#	   ../../lib/moduleloader.ilb \
#	   ../../lib/threadman.ilb \
#	   ../../lib/timerman.ilb \
#	   ../../lib/ioman.ilb
EXILBS =

LOADLIBES= $(LDFLAGS) $(EXILBS:%=-ilb=%)

all:	$(IRX)

$(IRX): $(OBJS)
	$(LINK.o) -o $(@:.irx=.sym) $^ $(LOADLIBES) 
	$(IFIXUP) -o $@ $(@:.irx=.sym)

clean:
	rm -f *.o *.irx *.sym

#run: $(IRX)
#	dsreset 0 2
#	dsistart $(IRX)
run: $(IRX)
	dsidb -r mstart -d $(IRX)
