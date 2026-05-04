#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.18 2002/07/31 11:47:09 usr03875 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = sigeno.a

# オブジェクトリスト
OBJECTS = padproc.o zoneviewer.o safecheck.o \
	conv_act.o setjump.o routeview.o conv_view.o \
	poscheck.o hang_act.o dogtagmng.o pad_pos.o 
XOBJECTS = padproc.obj zoneviewer.obj safecheck.obj \
	conv_act.obj setjump.obj routeview.obj \
	conv_view.obj poscheck.obj hang_act.obj dogtagmng.obj pad_pos.obj 

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../../korekado/action ../../korekado/enemy \
	../../korekado/conv ../attacker
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

padproc.o : padproc.c 
safecheck.o : safecheck.c
thkview.o : thkview.c 
conv_act.o : ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
setjump.o : ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
routeview.o : ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
conv_view.o : ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
hang_act.o : ../../korekado/enemy/enemy.h ../../korekado/action/action.h 

#####XBOX#####

padproc.obj : padproc.c 
safecheck.obj : safecheck.c
thkview.obj : thkview.c 
conv_act.obj : ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
setjump.obj : ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
routeview.obj : ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
conv_view.obj : ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
hang_act.obj : ../../korekado/enemy/enemy.h ../../korekado/action/action.h 


