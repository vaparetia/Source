#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.18 2002/05/16 09:51:13 usr01749 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = korekado.a

# オブジェクトリスト
OBJECTS = enemy.o command.o enaction.o corps.o corpcom.o oldcorp.o dbview.o eyeview.o enecorps.o campose.o

XOBJECTS = enemy.obj command.obj enaction.obj corps.obj corpcom.obj oldcorp.obj dbview.obj eyeview.obj enecorps.obj campose.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../action ../enemy ../conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

enemy.o : enemy.c enemy.h enemy.x security.h
command.o : command.c clearing.c clearing.h command.h enemy.h security.h speak.c speak.h
corps.o : corps.c enemy.h security.h
campose.o : corps.c enemy.h security.h

enemy.obj : enemy.c enemy.h enemy.x security.h
command.obj : command.c clearing.c clearing.h command.h enemy.h security.h speak.c speak.h
corps.obj : corps.c enemy.h security.h
campose.obj : corps.c enemy.h security.h

$(OBJECTS) : enemy.h ../action/action.h
