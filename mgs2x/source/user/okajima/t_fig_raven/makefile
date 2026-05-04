#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okajima.a

# オブジェクトリスト
OBJECTS = fig_raven.o fig_raven_init.o
XOBJECTS = $(OBJECTS:.o=.obj)
# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

fig_raven.o:		fig_raven.h
fig_raven_init.o:	fig_raven.h


