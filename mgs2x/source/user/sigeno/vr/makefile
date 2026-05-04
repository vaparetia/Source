#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:49:56 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = sigeno.a

# オブジェクトリスト
OBJECTS = vr_target1.o 3dprint.o vr_goal.o vr_disp.o vr_wall.o vr_cube.o vr_sys.o vr_child.o \
	fadeobj.o posprint.o trg_cmd.o inc_trg.o vr_status_cmd.o 

XOBJECTS = vr_target1.obj 3dprint.obj vr_goal.obj vr_disp.obj vr_wall.obj vr_cube.obj vr_sys.obj vr_child.obj \
	fadeobj.obj posprint.obj trg_cmd.obj inc_trg.obj vr_status_cmd.obj 

# ここでのローカルフラグリスト


LOCAL_INCLUDES = ../../korekado/action \
	../../korekado/enemy \
	../../korekado/conv ../attacker ../tng 
#	../../../../scn

LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

vr_target1.o : vr.h inc_trg.c inc_trg.h
vr_child.o : vr.h inc_trg.c inc_trg.h
3dprint.o : vr.h
vr_goal.o : vr.h
vr_disp.o : vr.h
vr_wall.o : vr.h
vr_cube.o : vr.h inc_cube.c
vr_sys.o : vr.h
posprint.o : vr.h 
trg_cmd.o : vr.h 
inc_trg.o : vr.h 

############################################
vr_target1.obj : vr.h inc_trg.c inc_trg.h
vr_child.obj : vr.h inc_trg.c inc_trg.h
3dprint.obj : vr.h
vr_goal.obj : vr.h
vr_disp.obj : vr.h
vr_wall.obj : vr.h
vr_cube.obj : vr.h inc_cube.c
vr_sys.obj : vr.h
posprint.obj : vr.h 
trg_cmd.obj : vr.h 
inc_trg.obj : vr.h 

