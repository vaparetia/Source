#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.12 2002/02/14 04:26:14 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../..

# ライブラリ名

LIBNAME = libhzx.a

# オブジェクトリスト
OBJECTS =	hzxd.o loadhzx.o \
		route.o navigate.o trap.o bind.o \
		nearx.o vu0nearx.o levelx.o vu0levelx.o \
		onlinex.o vu0onlinex.o dynamic.o \
		trp_view.o bhd_view.o debug.o

XOBJECTS = bind.obj debug.obj dynamic_cpp.obj hzxd.obj level_cpp.obj loadhzx.obj \
		navigate.obj near_cpp.obj online_cpp.obj route.obj trap.obj trp_view.obj\
		bhd_view.obj


# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJECTS) :	libhzx.h

vu0nearx.o : vu0nearx.dsm
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ $< $(LISTOPTION)
vu0levelx.o : vu0levelx.dsm
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ $< $(LISTOPTION)
vu0onlinex.o : vu0onlinex.dsm
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ $< $(LISTOPTION)
