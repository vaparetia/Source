#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.37 2002/02/14 05:15:15 usr01749 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = shibata.a

# オブジェクトリスト

OBJECTS = 	cigarette.o \
			traffic.o traffic_flush.o \
			cinema_scr.o telop_trial.o \
			debris_cm.o debris_tex.o \
			debris_cm_snd.o \
			lens_flare.o \
			ray_console.o mesg_bomb.o mesg_bomb2.o mesg_bomb3.o \
			eye_ctrl2.o orn_face.o ray_waterpollute.o \
			scr_water_demo.o scr_conblur.o ray_colwater.o \
			cinema_scr_e3.o blood_water_demo.o depend_arms.o \
			depend_arms2.o depend_arms3.o ray_monoeye.o \
			ray_monoeye_demo.o scr_goggles_demo.o scr_drop_demo.o \
			depend_arms4.o

XOBJECTS = 	cigarette.obj \
			traffic.obj traffic_flush.obj \
			cinema_scr.obj telop_trial.obj \
			debris_cm.obj debris_tex.obj \
			debris_cm_snd.obj \
			lens_flare.obj \
			ray_console.obj mesg_bomb.obj mesg_bomb2.obj mesg_bomb3.obj \
			eye_ctrl2.obj orn_face.obj ray_waterpollute.obj \
			scr_water_demo.obj scr_conblur.obj ray_colwater.obj \
			cinema_scr_e3.obj blood_water_demo.obj depend_arms.obj \
			depend_arms2.obj depend_arms3.obj ray_monoeye.obj \
			ray_monoeye_demo.obj scr_goggles_demo.obj scr_drop_demo.obj \
			depend_arms4.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

cinema_scr.o : ../util/dma_set.h ../util/dma_set.o
scr_goggles_demo.o : ../util/dma_set.h ../util/dma_set.o
scr_drop_demo.o : ../util/dma_set.h ../util/dma_set.o

cinema_scr.obj : ../util/dma_set.h ../util/dma_set.obj
scr_goggles_demo.obj : ../util/dma_set.h ../util/dma_set.obj
scr_drop_demo.obj : ../util/dma_set.h ../util/dma_set.obj
