#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.23 2002/08/28 02:25:13 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS = emma.c emma_ini.c emma_act.c emma_thk.c \
	emma_msg.c emma_rcg.c emma_dsp.c \
	emma_clb.c emma_utl.c emma_slp.c emma_dbg.c 
OBJS     = $(SRCS:.c=.o)
XOBJECTS = $(OBJS:.o=.obj)


# ここでのローカルフラグリスト

LOCAL_INCLUDES = include
LOCAL_DEFINES = 
LOCAL_OPTIONS =


MODULES = command effect misc

.PHONY : $(MODULES) clean2

all : $(MODULES)

lib : $(MODULES)

$(MODULES):
	@$(MAKE) lib -C $@

clean2:
	-(for i in $(MODULES); do $(MAKE) clean -C $$i; done)
clean: clean2

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト
emma_thk.o : $(wildcard think/*.h)
emma_thk.obj : $(wildcard think/*.h)
emma_act.o : $(wildcard action/*.h)
emma_act.obj : $(wildcard action/*.h)
$(OBJS): include/emma.h include/emma_flag.h include/emma.mh ../../korekado/npc/npc.h
$(XOBJECTS): include/emma.h include/emma_flag.h include/emma.mh ../../korekado/npc/npc.h
