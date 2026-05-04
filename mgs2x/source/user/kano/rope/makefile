#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 1999/09/21 K.Kano (Modified)
# $Id: makefile,v 1.10 2002/02/04 07:55:17 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kano.a

# オブジェクトリスト
OBJECTS = ropemain.o ropemain2.o rope.o rope1.o rope2.o rope3.o ropeserver.o
XOBJECTS= $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

ropemain.o : ropemain.c rope.h
ropemain2.o : ropemain2.c rope.h
rope.o : rope.c rope.h
rope1.o : rope1.c rope.h
rope2.o : rope2.c rope.h
rope3.o : rope3.c rope.h
ropeserver.o : ropeserver.c rope.h
