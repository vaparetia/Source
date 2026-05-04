#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:51:59 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = yano.a

# オブジェクトリスト
OBJECTS = yn_utl.o dmatags.o vr_book.o show_pic.o show_codename.o gover_end.o

XOBJECTS = $(OBJECTS:.o=.obj)


# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体
include $(BASEDIR)/makefile.sub

# 依存関係のリスト
vr_book.o   : vr_book.c vr_book.h
vr_book.obj : vr_book.c vr_book.h
