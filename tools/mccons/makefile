#
#	EE Standard Makefile
#

SHELL	    = /bin/sh

TOP         = /usr/local/sce/ee
LIBDIR	    = $(TOP)/lib
INCDIR	    = $(TOP)/include

TARGET	    = memcard
OBJS	    = crt0.o main.o

LCFILE	    = $(LIBDIR)/app.cmd
LIBS	    = $(LIBDIR)/libgraph.a \
		      $(LIBDIR)/libdma.a \
		      $(LIBDIR)/libdev.a \
		      $(LIBDIR)/libpkt.a \
		      $(LIBDIR)/libpad.a \
		      $(LIBDIR)/libmc.a \
		      $(LIBDIR)/libvu0.a \
			  $(LIBDIR)/libcdvd.a

PREFIX	    = ee
AS	    = $(PREFIX)-gcc
CC	    = $(PREFIX)-gcc
LD	    = $(PREFIX)-gcc
DVPASM	    = $(PREFIX)-dvp-as
OBJDUMP	    = $(PREFIX)-objdump
RUN	    = dsedb -r run
RM	    = /bin/rm -f

CFLAGS	    = -O2 -Wall -G0
CXXFLAGS    = -O2 -Wall -fno-exceptions -fno-common
ASFLAGS	    = -c -xassembler-with-cpp
DVPASMFLAGS = -g
#LDFLAGS	    = -nostartfiles -L$(LIBDIR) -lm -lc -s
LDFLAGS	    = -mno-crt0 -L$(LIBDIR) -lm -lc -s
TMPFLAGS    =

.SUFFIXES: .c .s .cc .dsm

all: $(TARGET).elf

$(TARGET).elf: $(OBJS) $(LIBS)
	$(LD) -o $@ -T $(LCFILE) $(OBJS) $(LIBS) $(LDFLAGS)

crt0.o: $(LIBDIR)/crt0.s
	$(AS) $(ASFLAGS) $(TMPFLAGS) -o $@ $<

.s.o:
	$(AS) $(ASFLAGS) $(TMPFLAGS) -I$(INCDIR) -o $@ $<

.dsm.o:
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ $<

.c.o:
	$(CC) $(CFLAGS) $(TMPFLAGS) -I$(INCDIR) -c $< -o $*.o

.cc.o:
	$(CC) $(CXXFLAGS) $(TMPFLAGS) -I$(INCDIR) -c $< -o $*.o

clean:
	$(RM) *.o *.map *.lst core *.dis *.elf

run:
	$(RUN) $(TARGET).elf

install:
	cp $(TARGET).elf KCEJ-NETCONF /u/develop/mj001b/develop/tools/saveconf/
	cp saveconf /u/develop/mj001b/tools/bin/linux

