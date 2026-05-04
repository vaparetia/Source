#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
#	1999/07/07 K.Takabe
#	$Id: makefile,v 1.6 2002/12/26 12:40:32 takaki Exp $
#

# ベースディレクトリ
BASEDIR = ../..

# ライブラリ名

LIBNAME = libx2w.a

# オブジェクトリスト
OBJECTS =	
XOBJECTS =  
WOBJECTS = xbox2win.obj x2w_util.obj x2w_app_config.obj \
			error_util.obj error_code.obj	\
			OptionIO.obj sChkDisk_cpp.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

# ifeq ($(MGS2_XBOX),1)
# all : $(XSHADE_OBJS)
# lib : $(XSHADE_OBJS)
# endif

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

x2w_app_config.obj :	x2w_app_config.h
error_code.obj :		error_code_equ.h error_code_str.h

