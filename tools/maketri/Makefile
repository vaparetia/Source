#
# make_tri
#
# $Id: Makefile,v 1.2 2000/01/12 12:19:45 usr10478 Exp $
#
TARGET = make_tri
OBJS = main.o loadcm2.o tex_pack.o vramctrl.o

INSTALL_DIR = /u/develop/mj001b/tools/bin/linux

OBJ1 = 

LIBS = 
CC = gcc
CFLAGS = -g -O2 -m486 -pipe

# Put your desired make options here.
.MAKEOPTS: -O

# This Makefile uses the Bourne shell.
SHELL = /bin/sh

# Default rule.

.c.o:
	$(CC) -c $(CFLAGS) $<

compile: $(TARGET)
	cc -o $(TARGET) -lm $(OBJS) -lm

$(TARGET): $(OBJS)

# Rule to remove intermediate files but leave the target intact.
clean:
	rm -f $(OBJS) $(TARGET)

install:
	cp $(TARGET) $(INSTALL_DIR)
