#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.7 2002/02/01 04:43:19 usr01475 Exp $
#

# ベースディレクトリ
BASEDIR = ../..

# ライブラリ名

LIBNAME = libgv.a

# オブジェクトリスト
OBJECTS =	gvd.o actor.o memory.o pad.o debug.o strcode.o inline.o cache.o \
			util.o near.o message.o resident.o memlist.o

XOBJECTS = actor.obj cache.obj debug.obj gvd.obj inline.obj memlist.obj memory.obj \
			message.obj near.obj pad_cpp.obj resident.obj strcode.obj util.obj


#OBJECTS := $(OBJECTS) profile.o

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト


