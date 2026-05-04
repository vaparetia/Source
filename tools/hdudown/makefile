#
#	EE Standard Makefile
#

SHELL	    = /bin/sh

TARGET	    = hdudown
OBJS	    = crt0.o main.o cons.o
SYSTEMCNF   = image/system.cnf
IDFILE      = image/kcejinst.id
BOOTFILE    = SLPS_999.99

TOP         = /usr/local/sce/ee
LIBDIR	    = $(TOP)/lib
INCDIR	    = $(TOP)/include

ifeq ($(MGS2_PATH),)
MODULES		= /usr/local/home/kaz/work/mgs2/module
else
MODULES		= $(MGS2_PATH)/module/
endif
LOCALLIB	= $(MODULES)/lib
LOCALINC	= $(MODULES)/include

SYSTEMPATH = /usr/local/sce/iop/modules
USERPATH = $(MODULES)/irx

LCFILE	    = ./app.cmd
LIBS	    = $(LIBDIR)/libgraph.a \
			  $(LOCALLIB)/usbfs.a \
			  $(LOCALLIB)/jsifman.a \
			  $(LOCALLIB)/zlibdec.a \
		      $(LIBDIR)/libdma.a \
		      $(LIBDIR)/libdev.a \
		      $(LIBDIR)/libpkt.a \
		      $(LIBDIR)/libpad.a \
		      $(LIBDIR)/libvu0.a \
		      $(LIBDIR)/libmc.a \
			  $(LIBDIR)/libcdvd.a \

PREFIX	    = ee
AS	    = $(PREFIX)-gcc
CC	    = $(PREFIX)-gcc
LD	    = $(PREFIX)-gcc
DVPASM	    = $(PREFIX)-dvp-as
OBJDUMP	    = $(PREFIX)-objdump
RUN	    = dsedb -r run
RM	    = /bin/rm -f

CFLAGS	    = -O2 -Wall -Wno-paste

CXXFLAGS    = -O2 -Wall -fno-exceptions -fno-common
ASFLAGS	    = -c -xassembler-with-cpp
DVPASMFLAGS = -g
#LDFLAGS	    = -nostartfiles -L$(LIBDIR)
LDFLAGS	    = -mno-crt0 -L$(LIBDIR)
TMPFLAGS    =

IPKS = image/modules/syshd.ipk image/modules/usb.ipk image/modules/net.ipk
COPYS = image/modules/pld.irx
#SCOPYS = image/modules/ioprp21C.img
#SCOPYS = image/modules/ioprp224.img
SCOPYS = image/modules/ioprp23.img

.SUFFIXES: .c .s .cc .dsm

all: $(TARGET).elf $(IPKS) $(COPYS) $(SCOPYS)

$(TARGET).elf: $(OBJS) $(LIBS)
	$(LD) -o $@ -T $(LCFILE) $(OBJS) $(LIBS) $(LDFLAGS)

crt0.o: $(LIBDIR)/crt0.s
	$(AS) $(ASFLAGS) $(TMPFLAGS) -o $@ $<

.s.o:
	$(AS) $(ASFLAGS) $(TMPFLAGS) -I$(INCDIR) -o $@ $<

.dsm.o:
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ $<

.c.o:
	$(CC) $(CFLAGS) $(TMPFLAGS) -I$(INCDIR) -I$(LOCALINC) -c $< -o $*.o

.cc.o:
	$(CC) $(CXXFLAGS) $(TMPFLAGS) -I$(INCDIR) -c $< -o $*.o

run: all
	$(RUN) $(TARGET).elf

clean:
	$(RM) *.o *.map core *.dis *.elf

$(IPKS) : image/modules/%.ipk : %.ils ../../work/mgs2/module/irx/*.irx
	irxar a $@ $< -D SYSTEMPATH=$(SYSTEMPATH) -D USERPATH=$(USERPATH)

$(SCOPYS) : image/modules/% : /usr/local/sce/iop/modules/%
	-cp $< $@

$(COPYS) : image/modules/% : $(MODULES)/irx/%
	cp $< $@


cd :
	make clean
	make "CFLAGS=$(CFLAGS) -DCDBOOT=1"
	-mkdir image
	-mkdir image/modules
	ee-strip $(TARGET).elf -o image/$(BOOTFILE)
	echo "BOOT2 = cdrom0:\\$(strip $(BOOTFILE));1" > $(SYSTEMCNF)
	echo "VER = 1.00" >> $(SYSTEMCNF)
	echo "VMODE = NTSC" >> $(SYSTEMCNF)
	echo "KCEJ INSTALL AND BOOT CD" > $(IDFILE)
	make clean


