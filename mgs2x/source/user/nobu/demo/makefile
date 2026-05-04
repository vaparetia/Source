#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/12/26 N.Tanaka
# $Id: makefile,v 1.8 2002/02/01 04:43:47 usr01475 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = nobu.a

# オブジェクトリスト
OBJECTS = demothrd.o demoexec.o demoact.o demoeffect.o common.o

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

demothrd.o : demothrd.c demo_.h demo.h demoeffect.h
demoexec.o : demoexec.c demo_.h demo.h demoeffect.h common.h
demoact.o : demoact.c demo_.h demo.h
demoeffect.o : demoeffect.c demo_.h demo.h demoeffect.h
common.o : common.c demo_.h demo.h common.h