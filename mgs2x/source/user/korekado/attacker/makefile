#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.12 2002/02/01 08:50:22 usr01749 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = korekado.a

# オブジェクトリスト
OBJECTS = attacker.o atsneak.o atavoid.o enedamag.o atavocom.o atsearch.o

XOBJECTS = attacker.obj atsneak.obj atavoid.obj enedamag.obj atavocom.obj atsearch.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../action ../enemy ../conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

attacker.o : attacker.c atprepro.c atthink.c \
					atalert.c atsneak.c \
					ataction.c atactsub.c attacker.h 
atavoid.o : atavoid.c atavoact.c atavoid.h modechng.c
atsneak.o : atsneak.c atavoact.c atavoid.h modechng.c
enedamag.o : enedamag.c endamact.c enedamag.h 
attacker.obj : attacker.c atprepro.c atthink.c \
					atalert.c atsneak.c \
					ataction.c atactsub.c attacker.h 
atavoid.obj : atavoid.c atavoact.c atavoid.h modechng.c
atsneak.obj : atsneak.c atavoact.c atavoid.h modechng.c
enedamag.obj : enedamag.c endamact.c enedamag.h 
$(OBJECTS) : ../enemy/enemy.h
