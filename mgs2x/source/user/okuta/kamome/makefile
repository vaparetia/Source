#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#	かもめさん関連
# 2001/05/01 Masafumi Okuta
# $Id: makefile,v 1.10 2002/02/07 07:44:51 usr04761 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okuta.a

# オブジェクトリスト
OBJECTS = kmmng.o

XOBJECTS = kmmng.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト
kmmng.o : kmmng.c kamome.def kmtest.c kmmess.c kmsend.c kmact.c kmmove.c kmthink.c kmpre.c kmsound.c kmdbg.c kmginfo.c kamome.h ../conv/memmot.h ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h







