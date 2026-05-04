#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.4 2002/02/07 02:25:04 usr04761 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kunibe.a

# オブジェクトリスト
OBJECTS = act_test.o prm_test.o prm_test2.o line_test.o round_test.o cul_test.o particle_test.o

XOBJECTS = $(OBJECTS:.o=.obj)


# ここでのローカルフラグリスト


#LOCAL_INCLUDES = ../../korekado/action ../../korekado/enemy \
#	../../korekado/conv 
#../attacker 
#LOCAL_INCLUDES = ../../korekado/enemy 

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

cypher.o : cypher.c eyecheck.h 
sv_camera.o : sv_camera.c eyecheck.h 
eyecheck.o : eyecheck.c eyecheck.h 
guncam.o : guncam.c eyecheck.h 
enemy.h : action.h 

