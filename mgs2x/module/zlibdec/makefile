#
#	KCEJ DEBUG UTILITY for EE
#

TARGET      = zlibdec.a

HEADER      = zlib.h zconf.h
OBJECTS     = infblock.o infcodes.o inffast.o inflate.o inftrees.o infutil.o adler32.o

MODULE_INCDIR = ../include
MODULE_LIBDIR = ../lib

SHELL       = /bin/sh
TOP         = /usr/local/sce/ee
LIBDIR      = $(TOP)/lib
INCDIR      = $(TOP)/include 
SYSINCDIR   = $(TOP)/src/kernel/include

PREFIX      = ee
AS          = $(PREFIX)-gcc
CC          = $(PREFIX)-gcc
LD          = $(PREFIX)-gcc
DVPASM      = $(PREFIX)-dvp-as
OBJDUMP     = $(PREFIX)-objdump
RUN         = dsedb -r run
RM          = /bin/rm -f
AR          = $(PREFIX)-ar

CFLAGS      = -O2 -Wall -DEE -G0 -DPS2_EE -g
ASFLAGS     = -c -xassembler-with-cpp
DVPASMFLAGS = -g 

.SUFFIXES: .a .c .s .cc .dsm

all: $(TARGET) copy

clean:
	$(RM) $(TARGET) $(OBJECTS)

$(TARGET) : $(OBJECTS)
	$(AR) r $(TARGET) $(OBJECTS)

copy:
	cp $(TARGET) $(MODULE_LIBDIR)
	cp $(HEADER) $(MODULE_INCDIR)

.s.o:
	$(AS) $(ASFLAGS) $(TMPFLAGS) -I$(INCDIR) -o $@ $<

.dsm.o:
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(TMPFLAGS) -I$(INCDIR) -c $< -o $*.o
