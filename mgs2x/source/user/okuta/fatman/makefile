#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#	ファットマン関連
# 2001/04/23 Masafumi Okuta
# $Id: makefile,v 1.10 2002/03/13 08:01:33 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okuta.a

# オブジェクトリスト
OBJECTS = fatman.o 

XOBJECTS = fatman.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト
fatman.o : fatman.c fatman.h fatman.def fatdata.c fatnpc.c fatsound.c fatarea.c fatzone.c fatintrude.c fatextern.c fatact.c fatparty.c fateye.c fatc4.c fatmove.c fatattack.c fatdamage.c fatbomb.c fatdemine.c fatshoot.c fatpre.c fatthink.c fatinit.c fatcommon.c fatutil.c fatdebug.c fatdbgmenu.c  ../../korekado/npc/npc.h ../conv/algfunc.h ../conv/maoutil.h

fatman.obj : fatman.c fatman.h fatman.def fatdata.c fatnpc.c fatsound.c fatarea.c fatzone.c fatintrude.c fatextern.c fatact.c fatparty.c fateye.c fatc4.c fatmove.c fatattack.c fatdamage.c fatbomb.c fatdemine.c fatshoot.c fatpre.c fatthink.c fatinit.c fatcommon.c fatutil.c fatdebug.c fatdbgmenu.c  ../../korekado/npc/npc.h ../conv/algfunc.h ../conv/maoutil.h



