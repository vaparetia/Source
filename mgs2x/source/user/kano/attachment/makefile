#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 1999/09/21 K.Kano (Modified)
# $Id: makefile,v 1.9 2002/02/04 06:16:16 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kano.a

# オブジェクトリスト
OBJECTS = attachment.o attachact.o attachment2.o attachact2.o attachment3.o attachact3.o \
	attachment4.o attachact4.o attachments.o attachment3a.o attachactf.o
XOBJECTS= $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

attachment.o : attachment.c attachment.h
attachact.o : attachact.c attachment.h
attachment2.o : attachment2.c attachment2.h
attachact2.o : attachact2.c attachment2.h
attachment3.o : attachment3.c attachment3.h
attachact3.o : attachact3.c attachment3.h
attachment4.o : attachment4.c attachment4.h
attachact4.o : attachact4.c attachment4.h
attachments.o : attachments.c attachment.h attachment2.h attachment3.h
attachment3a.o : attachment3a.c attachment.h attachment2.h attachment3.h attachment4.h attachmentf.h
attachactf.o : attachactf.c attachmentf.h
