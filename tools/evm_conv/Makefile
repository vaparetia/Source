#
# evm converter 
# 
# 
#


ifeq ($(MGS2_XBOX),1)
TARGET = evm_conv.xbox
else
TARGET = evm_conv
endif

OBJS = main.o conv.o prog_utl.o conv2.o

#####ifeq ($(MGS2_WIN32),1)
ifeq ("$(OSTYPE)","linux")
INSTALL_DIR = /u/develop/mj001b/tools/bin/linux
else
INSTALL_DIR = /u/develop/mj001b/exports/tools/bin/mgs2xbox
endif

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
	$(CC) -o $(TARGET) $(LIBS) $(OBJS)

$(TARGET): $(OBJS)

# Rule to remove intermediate files but leave the target intact.
clean:
	rm -f $(OBJS) $(TARGET)

install:
	cp $(TARGET) $(INSTALL_DIR)
