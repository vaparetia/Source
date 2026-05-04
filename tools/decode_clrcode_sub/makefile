#
#	makefile for decode_clrcode_sub
#

TARGET	= decode_clrcode_sub

ifeq ("$(OSTYPE)","linux")
OBJECTS	= main.o clearcode_encode.o display.o vrstagename.o
else
OBJECTS	= main.o clearcode_encode.o display_sjis.o vrstagename.o
endif

TARGET2	= get_vr_hash
OBJECTS2= get_vr_hash.o clearcode_encode.o

TARGET3	= web_decode
OBJECTS3 = main.o clearcode_encode.o display_web.o vrstagename.o

TARGET4 = web_vr_hash
OBJECTS4 = web_vr_hash.o clearcode_encode.o

TARGET5 = encode_sub
OBJECTS5 = encode.o clearcode_encode.o

TARGET6 = web_vr_stage
OBJECTS6 = web_vr_stage.o vrstagename.o

SCRIPT1 = clrcheck

CFLAGS = -c -DTOOL

.c.o :
	gcc $(CFLAGS) $<

ifeq ("$(OSTYPE)","linux")
all : $(TARGET) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) $(TARGET6) 
else
all : $(TARGET) $(TARGET2) $(TARGET3) $(TARGET4)
endif

$(TARGET) : $(OBJECTS)
	gcc -static $(OBJECTS) -o $(TARGET) -lm

$(TARGET2) : $(OBJECTS2)
	gcc -static $(OBJECTS2) -o $(TARGET2) -lm

$(TARGET3) : $(OBJECTS3)
	gcc -static $(OBJECTS3) -o $(TARGET3) -lm

$(TARGET4) : $(OBJECTS4)
	gcc -static $(OBJECTS4) -o $(TARGET4) -lm

$(TARGET5) : $(OBJECTS5)
	gcc -static $(OBJECTS5) -o $(TARGET5) -lm

$(TARGET6) : $(OBJECTS6)
	gcc -static $(OBJECTS6) -o $(TARGET6) -lm

%_sjis.c: %.c
	nkf -sO $< $@

clean :
	-rm $(OBJECTS) $(TARGET) $(OBJECTS2) $(TARGET2) $(OBJECTS3) $(TARGET3)

install :
ifeq ("$(OSTYPE)","linux")
	cp $(TARGET) /u/develop/mj001b/tools/bin/linux
	cp $(TARGET2) /u/develop/mj001b/tools/bin/linux
	cp $(SCRIPT1) /u/develop/mj001b/tools/bin/linux
else
	cp $(TARGET) $(TARGET2) /u/develop/mj001b/exports/tools/bin/mgs2xbox
	cp $(SCRIPT1) /u/develop/mj001b/exports/tools/bin/mgs2xbox
endif


