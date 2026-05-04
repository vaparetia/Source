
SHELL       = /bin/sh

TOP         = /usr/local/sce/ee
LIBDIR      = $(TOP)/lib
INCDIR      = $(TOP)/include -I$(TOP)/include/libeenet -I$(TOP)/common/include

TARGET      = ipu2mpeg
OBJS        = crt0.o \
              main.o \

LCFILE      = $(LIBDIR)/app.cmd
LIBS        = $(LIBDIR)/libgraph.a \
              $(LIBDIR)/libdma.a \
              $(LIBDIR)/libdev.a \
              $(LIBDIR)/libpkt.a \
		        $(LIBDIR)/libvu0.a \
		        $(LIBDIR)/libeenet.a \
		        $(LIBDIR)/libscf.a \
	      	  $(LIBDIR)/libipu.a 

LIBOPT	  = -lgraph -ldma -ldev -lpkt -lvu0 -lipu -lm -leenet -lscf

PREFIX      = ee
AS          = $(PREFIX)-g++
CC          = $(PREFIX)-g++
LD          = $(PREFIX)-g++
DVPASM      = $(PREFIX)-dvp-as
OBJDUMP     = $(PREFIX)-objdump
RUN         = dsedb -r run
RM          = /bin/rm -f

CFLAGS      = -O2 -Wall -Wa,-al
CXXFLAGS    = -O2 -Wall -Wa,-al -fno-exceptions -fno-common
ASFLAGS     = -c -xassembler-with-cpp -Wa,-al
DVPASMFLAGS =  
LDFLAGS     = -Wl,-Map,$(TARGET).map -mno-crt0 -L$(LIBDIR) $(LIBOPT)
TMPFLAGS    =

.SUFFIXES: .s .cpp .dsm

all: $(TARGET).elf

$(TARGET).elf: $(OBJS) $(LIBS)
	$(LD) -o $@ -T $(LCFILE) $(OBJS) $(LDFLAGS)

crt0.o: $(LIBDIR)/crt0.s
	$(AS) $(ASFLAGS) $(TMPFLAGS) -o $@ $< > $*.lst

.s.o:
	$(AS) $(ASFLAGS) $(TMPFLAGS) -I$(INCDIR) -o $@ $< > $*.lst

.dsm.o:
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ $< > $*.lst

.cpp.o:
	$(CC) $(CXXFLAGS) $(TMPFLAGS) -I$(INCDIR) -c $< -o $*.o > $*.lst

run: $(TARGET).elf
	$(RUN) $(TARGET).elf sample.ipu

clean:
	$(RM) *.o *.map *.lst core *.dis *.elf

########
compare:
	${MAKE} "CFLAGS=${CFLAGS} -DCOMPARE_ON"

g++:
	${MAKE} "CC=ee-g++" "LD=ee-g++"


