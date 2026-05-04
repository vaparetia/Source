#
# mtn lip blend
# 
# $Id: Makefile,v 1.2 2001/12/04 10:20:48 usr02774 Exp $
#

TARGET = mtn_lip_blend
TARGET2 = lip_jp2eng.pl
OBJS = main.o mtn_util.o quat.o gte.o rateconv.o prog_utl.o

# program install dir
INSTALL_DIR = /u/develop/mj001b/tools/bin/linux

# GTK+ library setting
GTK_DIR = 
GTK_LIB = `gtk-config --libs`
GTK_CFLAGS = `gtk-config --cflags`
# MESA library setting
MESA_DIR = /u/develop/mj001b/tools
MESA_LIB = -lglut -lMesaGLU -lMesaGL -L$(MESA_DIR)/lib
MESA_CFLAGS = -I$(MESA_DIR)/include
# MDU library setting
MDU_DIR = ../libMDU
MDU_LIB = $(MDU_DIR)/libMDU.a
MDU_CFLAGS = -I$(MDU_DIR)
# VRS library setting
CPU = -m486
VRS_DIR = /u/develop/mj001b/tools/vrs/lib
VRS_LIB = -lP3DCL$(CPU) -lSFDCL$(CPU) -lm -lc -L$(VRS_DIR)/lib
VRS_CFLAGS = -D_LINUX -DIS_LITTLE_ENDIAN -I$(VRS_DIR)/include -malign-double

LIBS = -lm
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
	$(CC) -o $(TARGET) $(OBJS) $(LIBS) 

$(TARGET): $(OBJS)

# Rule to remove intermediate files but leave the target intact.
clean:
	rm -f $(OBJS) $(TARGET)

install:
	cp $(TARGET) $(INSTALL_DIR)
	cp $(TARGET2) $(INSTALL_DIR)
