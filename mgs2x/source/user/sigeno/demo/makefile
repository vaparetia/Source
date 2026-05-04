#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:49:21 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = sigeno.a

# オブジェクトリスト
OBJECTS = putmodel.o lockercp.o routemdl.o facedemo.o \
	doll.o route3d.o sinktank.o demo_eye.o rad_point.o \
	gunspin.o play_ap.o routemdl2.o ghost.o gll_photo.o gll_ef.o scn_break.o \
	scn_padvib.o etc_sight.o 

XOBJECTS = putmodel.obj lockercp.obj routemdl.obj facedemo.obj \
	doll.obj route3d.obj sinktank.obj demo_eye.obj rad_point.obj \
	gunspin.obj play_ap.obj routemdl2.obj ghost.obj gll_photo.obj gll_ef.obj  scn_break.obj \
	scn_padvib.obj etc_sight.obj 

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../../korekado/enemy ../../korekado/conv \
			../../korekado/action
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

putmodel.o : putmodel.c 
lockercp.o : lockercp.c 
routemdl.o : routemdl.c 
facedemo.o : facedemo.c 
doll.o     : doll.c 
route3d.o :  route3d.c
##########################

putmodel.obj : putmodel.c 
lockercp.obj : lockercp.c 
routemdl.obj : routemdl.c 
facedemo.obj : facedemo.c 
doll.obj     : doll.c 
route3d.obj :  route3d.c

