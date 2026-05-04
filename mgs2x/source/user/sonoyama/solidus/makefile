#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:51:05 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = sonoyama.a

# オブジェクトリスト
OBJECTS = solidus.o between_cam.o kwt_mst.o \
		  snakearm.o sa_missile.o pl_liftup.o \
		  flame.o body_flame.o sam_slashed.o \
		  subjectarm_chain.o sol_global.o

XOBJECTS = solidus.obj between_cam.obj kwt_mst.obj \
		  snakearm.obj sa_missile.obj pl_liftup.obj \
		  body_flame.obj sam_slashed.obj \
		  subjectarm_chain.obj sol_global.obj

#XOBJECTS = $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../raiden
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

solidus.o : solidus.h solidus.c init_sol.c event_sol.c util_sol.c \
			normal_sol.c damage_sol.c think_sol.c attack_sol.c \
	  		careful_sol.c snakearm_mot.h dbg_menu_sol.c

$(OBJECTS) :	sol_common.h
pl_liftup.o :	../raiden/pl_work.h ../raiden/pl_define.h \
				../raiden/pl_inline.c ../raiden/pl_inline.h


