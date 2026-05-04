TARGET = mar
OBJS = main.o

LIBS = 
CC = gcc

ifeq ($(MGS2_XBOX),1)
CFLAGS = -g -O2 -m486 -pipe -D_XBOX
else
CFLAGS = -g -O2 -m486 -pipe
endif

# Put your desired make options here.
.MAKEOPTS: -O

# This Makefile uses the Bourne shell.
SHELL = /bin/sh

all: $(TARGET)

# Default rule.
.c.o:
	$(CC) -c $(CFLAGS) $<

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) -lm $(OBJS) -lm

install :
ifeq ($(MGS2_WIN32),1)
	cp $(TARGET) /u/develop/mj001b/exports/tools/bin/mgs2xbox
else
	cp $(TARGET) /u/develop/mj001b/tools/bin/linux
endif

# Rule to remove intermediate files but leave the target intact.
clean:
	rm -f $(OBJS) $(TARGET)

