#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.14 2002/02/22 07:04:09 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = mode.a

# オブジェクトリスト
OBJECTS = demod.o demo_pkt.o \
	demo_cmd.o demo_frm.o demo_obj.o demo_mtn.o demo_eft.o \
	func_tbl.o demo_var.o demo_lst.o \
	obj_act.o eft_act.o cam_act.o \
	eft_con.o
XOBJECTS = $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

MODULES =  effect

# コマンド

.PHONY : $(MODULES) clean2

all : $(MODULES)
lib : $(MODULES)

clean2 :
	-(for i in $(MODULES); do $(MAKE) clean -C $$i; done)
clean : clean2

$(MODULES) :
	@$(MAKE) lib -C $@

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJECTS) : libdemo.h
