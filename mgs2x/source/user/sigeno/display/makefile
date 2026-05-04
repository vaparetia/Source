#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:49:30 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = sigeno.a

# オブジェクトリスト
#OBJECTS = radar1.o vuradar.o radsprite.o
#OBJECTS = radar.o vuradar.o radsprite.o timer.o
OBJECTS = timer.o timer2.o 
XOBJECTS = timer.obj timer2.obj 

# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

timer.o : timer.c 

timer.obj : timer.c 

