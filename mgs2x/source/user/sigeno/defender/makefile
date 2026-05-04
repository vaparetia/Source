#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.24 2002/07/31 11:47:09 usr03875 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = sigeno.a

# オブジェクトリスト
OBJECTS = defcomm.o defthink.o defender.o  w32def.o \
	w32think.o w32com.o w25def.o

XOBJECTS = defcomm.obj defthink.obj defender.obj  w32def.obj \
	w32think.obj w32com.obj w25def.obj

#defsneak.o
#defthink.o 

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../../korekado/action ../../korekado/enemy \
	../../korekado/conv ../attacker ../conv 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト



defender.o : defender.h eve_a.h ../attacker/at_thk.h \
	../../korekado/enemy/enemy.h def_enum.h \
	../../korekado/enemy/motion.h 
defcomm.o : eve_a.h  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
defthink.o : defthink.c eve_a.h  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
w25def.o : ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
w32def.o : ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
enemy.h : action.h 
w32think.o :  eve_a.h ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
w32com.o :  eve_a.h ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
w25def.o :  eve_a.h ../../korekado/enemy/enemy.h ../../korekado/action/action.h 

##############################################################3

defender.obj : defender.h eve_a.h ../attacker/at_thk.h \
	../../korekado/enemy/enemy.h def_enum.h \
	../../korekado/enemy/motion.h 
defcomm.obj : eve_a.h  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
defthink.obj : defthink.c eve_a.h  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
w25def.obj : ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
w32def.obj : ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
enemy.h : action.h 
w32think.obj :  eve_a.h ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
w32com.obj :  eve_a.h ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
w25def.obj :  eve_a.h ../../korekado/enemy/enemy.h ../../korekado/action/action.h 



