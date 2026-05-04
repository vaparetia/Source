#
#	KCEJ DEBUG UTILITY for EE
#

TARGET      = mts.a
TARGET_M	= mts_m.a

HEADER		= mts.h debugutil.h
COMMON_OBJS = exception_in.o eventflag.o delay.o
OBJECTS     = mts.o exception.o stackcheck.o $(COMMON_OBJS)
OBJECTS_M	= mts_m.o exception_m.o stackcheck_m.o $(COMMON_OBJS)

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

CFLAGS      = -O2 -Wall -D EE -G0
MASTERFLAGS = -D MASTER
ASFLAGS     = -c -xassembler-with-cpp
DVPASMFLAGS = -g 

.SUFFIXES: .a .c .s .cc .dsm

all: $(TARGET) $(TARGET_M) copy

clean:
	$(RM) $(TARGET) $(TARGET_M) $(OBJECTS) $(OBJECTS_M)

$(TARGET) : $(OBJECTS)
	$(AR) r $(TARGET) $(OBJECTS)

$(TARGET_M) : $(OBJECTS_M)
	$(AR) r $(TARGET_M) $(OBJECTS_M)

copy:
	cp $(TARGET) $(MODULE_LIBDIR)
	cp $(TARGET_M) $(MODULE_LIBDIR)
	cp $(HEADER) $(MODULE_INCDIR)

.s.o:
	$(AS) $(ASFLAGS) $(TMPFLAGS) -I$(INCDIR) -o $@ $<

.dsm.o:
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(TMPFLAGS) -I$(INCDIR) -c $< -o $*.o

%_m.o : %.c
	$(CC) $(CFLAGS) $(MASTERFLAGS) $(TMPFLAGS) -I$(INCDIR) -c $< -o $@
