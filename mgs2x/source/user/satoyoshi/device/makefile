#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.13 2002/03/20 05:47:46 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = satoyoshi.a

# オブジェクトリスト

ifneq ($(MGS2_XBOX),1)
OBJECTS = cypher.o sv_camera.o eyecheck.o cypher4snipe.o \
	ub_camera.o

else

XOBJECTS = cypher.obj sv_camera.obj eyecheck.obj cypher4snipe.obj ub_camera.obj


endif

# ここでのローカルフラグリスト


LOCAL_INCLUDES = ../../korekado/action ../../korekado/enemy \
	../../korekado/conv 
#../attacker 
#LOCAL_INCLUDES = ../../korekado/enemy 


LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

cypher.o : cypher.c eyecheck.h eye_check.c cypher.h cyp_act.c cyp_target.c cyp_trace.c cyp_funcs.c cyp_alert.c cyp_call.c cyp_wbk.c
cypher4snipe.o : cypher4snipe.c eyecheck.h cypher.h
sv_camera.o : eyecheck.h camera.h layer_check.h cam_target.c cam_act.c eye_check.c cam_check.c cam_alert.c cam_call.c
eyecheck.o : eyecheck.c eyecheck.h cam_wbk.c
ub_camera.o : ub_camera.c ub_camera.h

#Xbox用 依存関係のリスト
cypher.obj : cypher.c eyecheck.h eye_check.c cypher.h cyp_act.c cyp_target.c cyp_trace.c cyp_funcs.c cyp_alert.c cyp_call.c cyp_wbk.c
cypher4snipe.obj : cypher4snipe.c eyecheck.h cypher.h
sv_camera.obj : eyecheck.h camera.h layer_check.h cam_target.c cam_act.c eye_check.c cam_check.c cam_alert.c cam_call.c
eyecheck.obj : eyecheck.c eyecheck.h cam_wbk.c
ub_camera.obj : ub_camera.c ub_camera.h

