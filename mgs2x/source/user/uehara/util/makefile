#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:51:37 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = uehara.a

# オブジェクトリスト
OBJECTS = select.o delay.o sound.o arrayset.o \
		 foreach.o rand.o kbdexec.o pause.o gclutil.o \
		 vox.o cancel.o varclear.o gettime.o soundtest.o reboot.o

#XOBJECTS = arrayset.obj cancel.obj delay.obj foreach.obj pause.obj \
#			 rand.obj select.obj varclear.obj 
XOBJECTS = select.obj delay.obj sound.obj arrayset.obj \
		 foreach.obj rand.obj kbdexec.obj pause.obj gclutil.obj \
		 vox.obj cancel.obj varclear.obj gettime.obj soundtest.obj reboot.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の読み込み
include $(BASEDIR)/makefile.sub

