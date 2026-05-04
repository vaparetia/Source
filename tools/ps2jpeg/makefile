#
# JPEG File Viewer 
# 
# $Id: makefile,v 1.1 2001/05/07 01:58:44 usr04761 Exp $
#

all :   jpegenc jpegdec

jpegenc :mainenc.o encode.o common.o
	gcc -g -o $@  $^ -lm

jpegdec :maindec.o decode.o common.o
	gcc -g -o $@  $^ -lm

%.o : %.c
	gcc -g -c -Wall $<  -o $@

clean:
	rm *.o
