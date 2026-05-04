#
# Mot file to Mtn file convert program
# 
# $Id: Makefile,v 1.2 2000/03/28 00:26:50 usr02774 Exp $
#

# target setting
TARGET = mot2mtn

# object setting
OBJS = main.o str_util.o mot2mtn.o wrt_mtn.o wrt_mtn2.o
LINK_OBJS = $(OBJS)

# directory setting
INSTALL_DIR = /u/develop/mj001b/tools/bin/linux/
MESA_DIR = /u/develop/mj001b/tools
MDU_DIR = /u/develop/mj001b/tools
VRS_DIR = /u/develop/mj001b/exports/tools/Developer/lib

# include path setting
APP_INCLUDE = 
STD_INCLUDE = 
GTK_INCLUDE = `gtk-config --cflags`
MESA_INCLUDE = -I$(MESA_DIR)/include
MDU_INCLUDE = -I$(MDU_DIR)/include
VRS_INCLUDE = -I$(VRS_DIR)/include
INCLUDE_PATH = $(STD_INCLUDE) $(VRS_INCLUDE) $(MDU_INCLUDE)

# library setting
APP_LIB = 
STD_LIB = -lm
GTK_LIB = `gtk-config --libs`
MESA_LIB = -lglut -lMesaGLU -lMesaGL -L$(MESA_DIR)/lib
MDU_LIB = -lMDU -L$(MDU_DIR)/lib/
VRS_LIB = -L$(VRS_DIR)/lib/ -lSFDCL-m486 -lP3DCL-m486 -lSFDCL-m486 
LIBS = $(STD_LIB) $(VRS_LIB)

# compile setting
CC = gcc
CFLAGS = -g -O2 -m486 -pipe  $(INCLUDE_PATH) -D_LINUX -malign-double

# Put your desired make options here.
.MAKEOPTS: -O

# This Makefile uses the Bourne shell.
SHELL = /bin/sh

# Default rule.

.c.o:
	$(CC) -c $(CFLAGS) $<

compile: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(LINK_OBJS) $(LIBS) 

# Rule to remove intermediate files but leave the target intact.
clean:
	rm -f $(OBJS) $(TARGET)

install:
	cp $(TARGET) $(INSTALL_DIR)
