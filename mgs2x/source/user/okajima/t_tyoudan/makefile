#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.5 2002/02/07 10:33:39 usr03875 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okajima.a

# スクリプト変換をかけるオブジェクト
#OBJANIM = tyou_smo.o
#OBJANIM = 

# オブジェクトリスト
#OBJECTS = ../anime/anime.o $(OBJANIM) tyoudan.o
#dankon.o
OBJECTS = tyoudan.o
XOBJECTS = tyoudan.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

$(OBJANIM) : %.o : %.c
	animbin $< > $(<:.c=.tmp)
	$(CC) -c $(CFLAGS) $(TMPFLAGS) -I$(INCDIR) -x c $(<:.c=.tmp) -o $*.o
	rm $(<:.c=.tmp)
