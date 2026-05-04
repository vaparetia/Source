
DECI2LIB = ../deci2app/deci2lib

INSTALL_DIR = /u/develop/mj001b/tools/bin/linux

TARGETS = ps2_dma_check
OBJECTS = main.o dma.o vif.o gif.o

CC = gcc
CFLAGS = -O2 -Wall -I$(DECI2LIB)
LDFLAGS = -L /usr/X11R6/lib -lm -lX11

DECI2OBJS = $(addprefix $(DECI2LIB)/, list.o elf.o dbgp.o netmp.o dcmp.o connect.o )

all: $(TARGETS)

$(TARGETS) : $(OBJECTS) $(DECI2OBJS)
	$(CC) -o $@ $(LDFLAGS) $^

clean:
	rm -f $(TARGETS) *.o $(DECI2LIB)/*.o core

install:
	cp $(TARGETS) $(INSTALL_DIR)
