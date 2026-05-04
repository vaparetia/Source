#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#	大統領イベント関連
# 2001/04/23 Masafumi Okuta
# $Id: makefile,v 1.7 2002/02/07 07:44:52 usr04761 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okuta.a

# オブジェクトリスト
OBJECTS = prez.o

XOBJECTS = prez.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

prez.o : prez.c prez.h prez.def prezact.c prezdata.c preznikita.c prezpre.c prezthk.c preznoise.c prezatra.c prezpanic.c prezarea.c prezsound.c prezparty.c prezinit.c prezdbg.c ../../korekado/npc/npc.h
