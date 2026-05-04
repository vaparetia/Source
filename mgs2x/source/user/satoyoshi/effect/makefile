#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.4 2002/02/13 04:38:34 usr04250 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = satoyoshi.a

# オブジェクトリスト
ifneq ($(MGS2_XBOX),1)
OBJECTS = umi_ex.o sara_umi.o
else
XOBJECTS = umi_ex.obj sara_umi.obj
endif

# ここでのローカルフラグリスト
LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト
umi_ex.o : umi_ex.c

