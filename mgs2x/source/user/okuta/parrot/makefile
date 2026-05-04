#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#	おうむ関連
# 2001/05/08 Masafumi Okuta
# $Id: makefile,v 1.4 2002/02/07 07:44:52 usr04761 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okuta.a

# オブジェクトリスト
OBJECTS = parrot.o 

XOBJECTS = parrot.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

parrot.o : parrot.c parrot.h parrot.def pardata.c parparty.c parsound.c parpre.c paract.c parthk.c parinit.c


