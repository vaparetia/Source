#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.36 2002/07/08 01:52:08 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS = orga.c orga_ini.c orga_act.c orga_clb.c \
	orga_rcg.c orga_dsp.c orga_msg.c \
	orga_dbg.c

OBJS = $(SRCS:.c=.o)
XOBJECTS = $(OBJS:.o=.obj)

MODULES = weapon misc holo

# ここでのローカルフラグリスト

LOCAL_INCLUDES = include
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# コマンド

.PHONY : $(MODULES) mkdbg clean2

all : mkdbg $(MODULES)

mkdbg : action/orga_dbglst.org
action/orga_dbglst.org : \
	action/orga_list.h action/orga_act.h \
        action/orga_move.h action/orga_fire.h action/orga_peep.h \
        action/orga_wait.h action/orga_stll.h action/orga_teaz.h \
	action/orga_damg.h \
	action/orga_thk_care.h \
	action/orga_thk_attk.h \
	action/orga_thk_move.h 

	@echo 'Making Debug-List!!'
	@/usr/bin/perl < action/mk_dbglst
	@echo 'Done.'

lib : mkdbg $(MODULES)

$(MODULES):
	@$(MAKE) lib -C $@

clean2 :
	-(for i in $(MODULES); do $(MAKE) clean -C $$i; done)

clean : clean2

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJS): include/orga.h include/orga_flag.h include/orga_se.h
$(XOBJECTS): include/orga.h include/orga_flag.h include/orga_se.h
orga_act.o: orga_act.c action/orga_list.h action/orga_act.h \
	action/orga_move.h action/orga_fire.h action/orga_peep.h \
	action/orga_wait.h action/orga_stll.h action/orga_teaz.h \
	action/orga_damg.h \
	action/orga_dbug.h \
	action/orga_thk_care.h \
	action/orga_thk_attk.h \
	action/orga_thk_move.h 
