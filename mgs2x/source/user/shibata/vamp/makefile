#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.6 2002/02/14 10:13:02 usr04761 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = shibata.a

# オブジェクトリスト
OBJECTS =	vamp.o \
			vmp_action.o vmp_think.o \
			vmp_etc.o vmp_zone.o \
			vmp_knife.o vmp_shdwtrg.o

XOBJECTS = $(OBJECTS:.o=.obj)

#XOBJECTS =	vamp.obj \
#			vmp_action.obj vmp_think.obj \
#			vmp_etc.obj vmp_zone.obj \
#			vmp_knife.obj vmp_shdwtrg.obj


# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

vamp.o : vamp.h vmp_inline.h
vmp_think.o : vamp.h vmp_inline.h
vmp_action.o : vamp.h vmp_inline.h
vmp_etc.o : vamp.h vmp_inline.h
vmp_zone.o : vamp.h vmp_inline.h

vamp.obj : vamp.h vmp_inline.h
vmp_think.obj : vamp.h vmp_inline.h
vmp_action.obj : vamp.h vmp_inline.h
vmp_etc.obj : vamp.h vmp_inline.h
vmp_zone.obj : vamp.h vmp_inline.h


