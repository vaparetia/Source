#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.47 2002/07/31 11:47:09 usr03875 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = sigeno.a

# オブジェクトリスト
OBJECTS = attacker.o  atcomm.o atthink.o atress.o sigavoid.o \
	atanes.o ataction.o sig_conv.o enetrapchk.o
XOBJECTS = attacker.obj  atcomm.obj atthink.obj atress.obj sigavoid.obj \
	atanes.obj ataction.obj sig_conv.obj enetrapchk.obj 


# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../../korekado/action ../../korekado/enemy ../../korekado/conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

attacker.o : attacker.c atprepro.c ataction.c \
	attacker.h at_act.c at_act.h viewprim.c \
	../../korekado/enemy/enemy.h at_enum.h \
	../../korekado/action/action.h 


atcomm.o : atcomm.c at_enum.h viewprim.c at_thk.h \
		../../korekado/action/action.h \
		../../korekado/enemy/enemy.h 
ataction.o : at_act.c ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
atthink.o : atthink.c at_enum.h at_act.c attacker.h viewprim.c \
		check_at.c threat.c discovery_at.c \
		approach.c intrude.c at_thk.h symmetry.c rescue.c \
		atanes.c pbreak_at.c \
		../../korekado/enemy/enemy.h \
		../../korekado/enemy/motion.h at_thk.h
atress.o : atress.c at_thk.h ../../korekado/enemy/enemy.h 
sigavoid.o : sigavoid.c sigavoact.c ../../korekado/enemy/enemy.h at_thk.h
atanes.o : atanes.c at_thk.h ../../korekado/enemy/enemy.h 
enemy.h : action.h at_thk.h
sig_conv.o : ../tng/tng.h at_enum.h at_thk.h \
		../../korekado/action/action.h ../../korekado/enemy/enemy.h 

#########XBOX################
attacker.obj : attacker.c atprepro.c ataction.c \
	attacker.h at_act.c at_act.h viewprim.c \
	../../korekado/enemy/enemy.h at_enum.h \
	../../korekado/action/action.h 


atcomm.obj : atcomm.c at_enum.h viewprim.c at_thk.h \
		../../korekado/action/action.h \
		../../korekado/enemy/enemy.h 
ataction.obj : at_act.c ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
atthink.obj : atthink.c at_enum.h at_act.c attacker.h viewprim.c \
		check_at.c threat.c discovery_at.c \
		approach.c intrude.c at_thk.h symmetry.c rescue.c \
		atanes.c pbreak_at.c \
		../../korekado/enemy/enemy.h \
		../../korekado/enemy/motion.h at_thk.h
atress.obj : atress.c at_thk.h ../../korekado/enemy/enemy.h 
sigavoid.obj : sigavoid.c sigavoact.c ../../korekado/enemy/enemy.h at_thk.h
atanes.obj : atanes.c at_thk.h ../../korekado/enemy/enemy.h 
enemy.h : action.h at_thk.h
sig_conv.obj : ../tng/tng.h at_enum.h at_thk.h \
		../../korekado/action/action.h ../../korekado/enemy/enemy.h 

