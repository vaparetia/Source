#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:49:51 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = sigeno.a

# オブジェクトリスト
OBJECTS = shield.o sig_grd.o tng_ktn.o 	blurpoint.o
XOBJECTS = shield.obj sig_grd.obj tng_ktn.obj 	blurpoint.obj

# ここでのローカルフラグリスト


LOCAL_INCLUDES = ../../korekado/action \
	../../korekado/enemy \
	../../korekado/conv ../attacker ../tng

LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

shield.o : shield.c ../../korekado/enemy/enemy.h \
	../../korekado/action/action.h 
sig_grd.o :  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
tng_ktn.o :  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 

#################################################################
shield.obj : shield.c ../../korekado/enemy/enemy.h \
	../../korekado/action/action.h 
sig_grd.obj :  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
tng_ktn.obj :  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 

