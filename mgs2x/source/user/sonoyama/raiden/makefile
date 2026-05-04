#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:50:56 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = sonoyama.a

# オブジェクトリスト
OBJECTS = raiden.o rai_init.o dbviewpl.o command.o \
	 	  pl_inline.o pl_plugin.o pl_global.o pl_arm.o \
		  command2.o pl_subject_demo.o command3.o \
	 	  command4.o command5.o \
	      pl_deadmode.o rai_equip.o pl_dummy.o \
	      pl_bul_splash.o rai_shadow_hair.o rai_action_blur.o \
		  rai_nude_ik.o pl_subcam_mng.o pl_pad.o \
		temp_resolve.o

XOBJECTS = $(OBJECTS:.o=.obj)
XOBJECTS := $(XOBJECTS) dbg_wp_it_chng.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES =
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

raiden.o :	raiden.c raiden.h motion.h weapon.h \
		routine.c normal.c event.c attack.c peep.c subject.c \
		equip.c special.c force.c item.h rai_arm.h \
		pl_inline.h adjust.c dbg_menu.c
command.o :	command.c
rai_arm.o :	rai_arm.c rai_arm.h arm_motion.h
rai_init.o :	rai_init.c raiden.h
pl_inline.o : 	pl_inline.c raiden.h rai_arm.h
pl_arm.o :		pl_arm.c pl_arm.h pl_arm_mot.h
$(OBJECTS) :	pl_work.h pl_define.h pl_inline.h pl_inline.c ../../../game/player.h

raiden.obj :	raiden.c raiden.h motion.h weapon.h \
		routine.c normal.c event.c attack.c peep.c subject.c \
		equip.c special.c force.c item.h rai_arm.h \
		pl_inline.h adjust.c dbg_menu.c
