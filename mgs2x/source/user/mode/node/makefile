#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.16 2002/05/21 08:20:11 usr01475 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = mode.a

# オブジェクトリスト
OBJECTS = node.o node_map.o \
		node_bg.o \
		node_bg_chm.o \
		node_bg_log.o \
		node_bg_nymap.o \
		node_bg_scroll.o \
		node_bg_fog.o \
		node_bg_grd.o \
		node_bg_plmap.o \
		node_bg_ray.o \
		node_bg_wall.o \
		node_bg_genzi.o

XOBJECTS = node.obj node_map.obj \
		node_bg.obj \
		node_bg_chm.obj \
		node_bg_log.obj \
		node_bg_nymap.obj \
		node_bg_scroll.obj \
		node_bg_fog.obj \
		node_bg_grd.obj \
		node_bg_plmap.obj \
		node_bg_ray.obj \
		node_bg_wall.obj \
		node_bg_genzi.obj


# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

node.o:		node.c node.h node_config.h node_map.h mode_act.h main_cur.h \
		node_bg_def.h
node_map.o:	node_map.c node_map.h node.h node_config.h

node_bg_chm.o:		node_bg_chm.c node_bg.h node_bg_def.h
node_bg_log.o:		node_bg_chm.c node_bg.h node_bg_def.h node_bg_log.h
node_bg_nymap.o:	node_bg_nymap.c node_bg.h node_bg_def.h
node_bg_scroll.o:	node_bg_scroll.c node_bg.h node_bg_def.h
node_bg_fog.o:		node_bg_fog.c node_bg.h node_bg_def.h

test.o:		test.c node.h

mode_act.h:	mode_act.lst
	perl ./macrobld.pl < $< > $@

node_bg_def.h:	node_bg_def.lst
	perl ./macrobld.pl < $< > $@

main_cur.h:	main_cur.lst
	perl ./macrobld.pl < $< > $@

opt_cur.h:	opt_cur.lst
	perl ./macrobld.pl < $< > $@


#------XBOX 依存関係 ------- yano 2002.03.26
node.obj:		node.c node.h node_config.h node_map.h mode_act.h main_cur.h \
		node_bg_def.h
node_map.obj:	node_map.c node_map.h node.h node_config.h mode_act.h node_bg_def.h

node_bg_chm.obj:		node_bg_chm.c node_bg.h node_bg_def.h
node_bg_log.obj:		node_bg_chm.c node_bg.h node_bg_def.h node_bg_log.h
node_bg_nymap.obj:	node_bg_nymap.c node_bg.h node_bg_def.h
node_bg_scroll.obj:	node_bg_scroll.c node_bg.h node_bg_def.h
node_bg_fog.obj:		node_bg_fog.c node_bg.h node_bg_def.h

test.obj:		test.c node.h
