#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:49:29 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = satoyoshi.a

# オブジェクトリスト
#OBJECTS = cypher.o sv_camera.o eyecheck.o guncam.o 


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

cypher.o : cypher.c eyecheck.h 
sv_camera.o : sv_camera.c eyecheck.h 
eyecheck.o : eyecheck.c eyecheck.h 
guncam.o : guncam.c eyecheck.h 
enemy.h : action.h 

###########################################
cypher.obj : cypher.c eyecheck.h 
sv_camera.obj : sv_camera.c eyecheck.h 
eyecheck.obj : eyecheck.c eyecheck.h 
guncam.obj : guncam.c eyecheck.h 
enemy.h : action.h 

