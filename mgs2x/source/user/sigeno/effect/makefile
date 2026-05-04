#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:49:47 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = sigeno.a

# オブジェクトリスト


OBJECTS = eyeanim.o handanim.o demohand.o mouth.o sysmouth.o sig_vanim.o breakobj.o rgbblur.o \
	homsight.o breakbody.o 3dtex.o vr_noise.o stagebreak.o breakobj2.o fall_line.o warptag.o \
	vr_lens.o shuchu_line.o grav.o katana_ap.o itembreak.o negafilter.o homsight2.o play_disap.o \
	breakpart.o scnbreakpart.o dm_fr.o padvib4.o scnskirt.o scnvapor.o
XOBJECTS = eyeanim.obj handanim.obj demohand.obj mouth.obj sysmouth.obj sig_vanim.obj xbreakobj.obj rgbblur.obj \
	homsight.obj xbreakbody.obj 3dtex.obj vr_noise.obj xstagebreak.obj xbreakobj2.obj fall_line.obj warptag.obj \
	vr_lens.obj shuchu_line.obj grav.obj xkatana_ap.obj itembreak.obj negafilter.obj homsight2.obj play_disap.obj \
	breakpart.obj scnbreakpart.obj  dm_fr.obj padvib4.obj scnskirt.obj scnvapor.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../../korekado/action ../../korekado/enemy ../../korekado/conv ../vr 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

eyeanim.o : eyeanim.c  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
handanim.o : handanim.c  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
demohand.o : handanim.c demohand.c  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
mouth.o : mouth.c  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
sysmouth.o : sysmouth.c   ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
homsight.o : ../vr/vr.h
itembreak.o : ../../korekado/enemy/enemy.h

###################
eyeanim.obj : eyeanim.c  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
handanim.obj : handanim.c  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
demohand.obj : handanim.c demohand.c  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
mouth.obj : mouth.c  ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
sysmouth.obj : sysmouth.c   ../../korekado/enemy/enemy.h ../../korekado/action/action.h 
homsight.obj : ../vr/vr.h
itembreak.obj : ../../korekado/enemy/enemy.h


