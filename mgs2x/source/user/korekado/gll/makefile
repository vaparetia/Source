#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 2002/04/04 Y.Korekado
# $Id$
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = korekado.a

# オブジェクトリスト
OBJECTS = gll.o eyebeem.o searchlt.o gl_light.o vrobj.o gllheart.o sonic.o \
			missile.o gllblast.o head.o gll_drop_shadow.o gllcom.o tub.o \
			shelf.o gll_achoo.o gllblur.o gll_flour_sara.o gllspread.o \
			gl_minilit.o footwall.o gllpika.o
XOBJECTS = gll.obj eyebeem.obj searchlt.obj gl_light.obj vrobj.obj gllheart.obj sonic.obj \
			missile.obj gllblast.obj head.obj gll_drop_shadow.obj gllcom.obj tub.obj  \
			shelf.obj gll_achoo.obj gllblur.obj gll_flour_sara.obj gllspread.obj \
			gl_minilit.obj footwall.obj gllpika.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../action ../enemy ../conv ../npc
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

gll.o : gll.c gllpre.c gllact.c gllthink.c gllfight.c gmefight.c gnofight.c gll.h gll_def.h
gllcom.o : gllcom.c gll_def.h gll_def.h
gllheart.o : gllheart.c gllheart.h gll_def.h
missile.o : missile.c missile.h gll_def.h

gll.obj : gll.c gllpre.c gllact.c gllthink.c gllfight.c gmefight.c gnofight.c gll.h gll_def.h
gllcom.obj : gllcom.c gll_def.h gll_def.h
gllheart.obj : gllheart.c gllheart.h gll_def.h
missile.obj : missile.c missile.h gll_def.h
