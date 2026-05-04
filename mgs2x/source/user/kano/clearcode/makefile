#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 2001/08/01 K.Kano (Modified)
# $Id: makefile,v 1.6 2002/08/06 05:05:55 usr03700 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kano.a

# オブジェクトリスト
OBJECTS = clearcode.o clearcode_layout.o get_newitem.o confirm.o clearcode_encode.o
XOBJECTS = $(OBJECTS:.o=.obj)


# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJECTS) : clearcode.h
