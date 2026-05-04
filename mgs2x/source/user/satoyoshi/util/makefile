#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.3 2002/02/13 04:39:23 usr04250 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = satoyoshi.a

# オブジェクトリスト
ifneq ($(MGS2_XBOX),1)
OBJECTS = sato_util.o
else
XOBJECTS = sato_util.obj
endif

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



