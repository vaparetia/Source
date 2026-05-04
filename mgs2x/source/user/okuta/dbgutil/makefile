#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#	デバック関連
# 2001/05/31 Masafumi Okuta
# $Id: makefile,v 1.4 2002/02/12 11:39:31 usr01475 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okuta.a

# オブジェクトリスト
OBJECTS = posbird.o 

XOBJECTS = posbird.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

posbird.o : posbird.c posbird.h ../conv/memmot.h ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

