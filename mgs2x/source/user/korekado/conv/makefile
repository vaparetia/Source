#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.28 2002/07/19 11:16:35 usr03875 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = korekado.a

# オブジェクトリスト
OBJECTS = util.o putobj.o putmodel.o cnctobj.o ceiling.o sling.o slinggun.o radio.o \
			sdmanage.o bgmanage.o n_sight.o eneequip.o eye.o navites.o corp.o gcl.o \
			cnctobj2.o chksling.o ceilchk.o

XOBJECTS = util.obj putobj.obj putmodel.obj cnctobj.obj ceiling.obj sling.obj slinggun.obj radio.obj \
			sdmanage.obj bgmanage.obj n_sight.obj eneequip.obj eye.obj navites.obj corp.obj gcl.obj \
			cnctobj2.obj chksling.obj  ceilchk.obj
 
# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../action ../enemy ../conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

util.o	: util.c korekado.x
util.obj	: util.c korekado.x
