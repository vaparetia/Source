#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.11 2002/04/24 04:12:44 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS = fort.c fort_ini.c fort_act.c fort_clb.c \
	fort_rcg.c fort_dsp.c fort_msg.c \
	fort_aim.c fort_var.c

OBJS = $(SRCS:.c=.o)
XOBJECTS = $(OBJS:.o=.obj)

MODULES = weapon misc hide hang_lgt effect

# ここでのローカルフラグリスト

LOCAL_INCLUDES = include
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# コマンド

.PHONY : $(MODULES) clean2 mkdbg

all : mkdbg $(MODULES)

mkdbg : action/fort_dbglst.org
action/fort_dbglst.org : \
	action/fort_list.h action/fort_act.h \
        action/fort_move.h action/fort_fire.h \
        action/fort_wait.h action/fort_stll.h \
	action/fort_thk_attk.h \
	action/fort_thk_move.h 

	@echo 'Making Debug-List!!'
	@/usr/bin/perl < action/mk_dbglst
	@echo 'Done.'

lib : mkdbg $(MODULES)

$(MODULES):
	@$(MAKE) lib -C $@

clean2 :
	-(for i in $(MODULES); do $(MAKE) clean -C $$i; done)

clean: clean2

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJS): include/fort.h include/fort_flag.h include/fort_se.h
fort_act.o: fort_act.c action/fort_list.h action/fort_act.h \
	action/fort_move.h action/fort_fire.h \
	action/fort_wait.h action/fort_stll.h \
	action/fort_dbug.h \
	action/fort_thk_attk.h \
	action/fort_thk_move.h 
