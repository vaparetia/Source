#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:50:44 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = sonoyama.a

# オブジェクトリスト
OBJECTS = gaged.o \
	  pl_vib.o dsegment.o multi.o breath.o evt_wall.o \
	  objhzx.o lodctrl.o dtrap.o etc.o padrec.o \
	  paddemo.o zoomcam.o \
	  beltconv.o beltobj.o o2gage.o fallflr.o \
	  psg_sight.o dv_goggles.o targettrap.o \
	  bul_fall.o belt.o demo_snakearm.o \
	  scncamvib.o shavedsnake.o sunglasses.o \
      boss_survival.o invisible.o

XOBJECTS = $(OBJECTS:.o=.obj)

#XOBJECTS = pl_vib.obj dsegment.obj multi.obj breath.obj evt_wall.obj \
#	  objhzx.obj lodctrl.obj dtrap.obj etc.obj padrec.obj \
#	  paddemo.obj zoomcam.obj \
#	  beltconv.obj beltobj.obj o2gage.obj fallflr.obj \
#	  psg_sight.obj dv_goggles.obj targettrap.obj \
#	  bul_fall.obj belt.obj demo_snakearm.obj \
#	  scncamvib.obj shavedsnake.obj sunglasses.obj \
#      boss_survival.obj

#XOBJECTS = $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../raiden
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

pl_vib.o : pl_vib.c ../raiden/pl_work.h
o2gage.o : o2gage.c ../raiden/pl_work.h