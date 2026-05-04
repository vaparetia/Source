#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.8 2002/02/12 11:07:26 usr01475 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = shibata.a

# スクリプト変換をかけるオブジェクト

OBJANIM = blastsgl.o headmark.o testeye.o blood2.o bullet.o
XOBJANIM = blastsgl.obj headmark.obj testeye.obj blood2.obj bullet.obj

# オブジェクトリスト
OBJECTS = anime.o $(OBJANIM)

XOBJECTS = anime.obj $(XOBJANIM)

#anime2.o
# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト


$(OBJANIM) : %.o : %.c
	animbin $< > $(<:.c=.tmp)
	$(CC) -c $(CFLAGS) $(TMPFLAGS) -I$(INCDIR) -x c $(<:.c=.tmp) -o $*.o
	rm $(<:.c=.tmp)

$(XOBJANIM) : %.obj : %.c
	animbin $< > _tmp_$<
	$(CC) -c $(CFLAGS) $(TMPFLAGS) -I$(INCDIR) _tmp_$< /Fo$*.obj
	rm _tmp_$<
