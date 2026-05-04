#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:49:52 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = sigeno.a

# オブジェクトリスト
OBJECTS = tng_a.o tng_b.o tngcom.o tng_act.o tngthink.o tng_pad.o \
	tngdefthink.o 
XOBJECTS = tng_a.obj tng_b.obj tngcom.obj tng_act.obj tngthink.obj tng_pad.obj \
	tngdefthink.obj 




# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../../korekado/action ../../korekado/enemy \
	../../korekado/conv ../attacker ../attacker 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

tngthink.o : tngthink.c ../attacker/check_at.c \
	../attacker/discovery_at.c ../attacker/threat.c \
	../attacker/approach.c ../attacker/intrude.c \
	../attacker/symmetry.c ../attacker/rescue.c \
	../../korekado/enemy/enemy.h ../../korekado/action/action.h   tng.h tng_a.h ../attacker/at_thk.h 
tng_a.o : tng.h tng_a.h ../../korekado/enemy/enemy.h ../../korekado/action/action.h ../attacker/at_thk.h \
	../../kira/effect/floor/floor.h
tng_b.o : tng.h tng_a.h ../../korekado/enemy/enemy.h ../../korekado/action/action.h ../attacker/at_thk.h \
	../../kira/effect/floor/floor.h
tngcom.o : tng.h tng_a.h ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
tng_act.o : tng.h tng_a.h ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
tngdefthink.o : tng.h tng_a.h  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 

######################################


tngthink.obj : tngthink.c ../attacker/check_at.c \
	../attacker/discovery_at.c ../attacker/threat.c \
	../attacker/approach.c ../attacker/intrude.c \
	../attacker/symmetry.c ../attacker/rescue.c \
	../../korekado/enemy/enemy.h ../../korekado/action/action.h   tng.h tng_a.h ../attacker/at_thk.h 
tng_a.obj : tng.h tng_a.h ../../korekado/enemy/enemy.h ../../korekado/action/action.h ../attacker/at_thk.h \
	../../kira/effect/floor/floor.h
tng_b.obj : tng.h tng_a.h ../../korekado/enemy/enemy.h ../../korekado/action/action.h ../attacker/at_thk.h \
	../../kira/effect/floor/floor.h
tngcom.obj : tng.h tng_a.h ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
tng_act.obj : tng.h tng_a.h ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
tngdefthink.obj : tng.h tng_a.h  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 


