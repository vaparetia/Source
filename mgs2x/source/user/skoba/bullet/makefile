#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:50:02 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = skoba.a

# オブジェクトリスト
OBJECTS = bullet.o bul_c4.o bul_gre.o bul_clay.o blast.o \
	  bul_nkt.o bul_rgb6.o bul_stg.o bul_spray.o \
	  blst_chf.o blst_stn.o water_mine.o bul_c4_mng.o \
	  bul_m4demo.o bul_fatman_c4_mng.o bul_book.o \
	  blast_water.o bmbctrl.o

XOBJECTS = bullet.obj bul_c4.obj bul_gre.obj bul_clay.obj blast.obj \
	  bul_nkt.obj bul_rgb6.obj bul_stg.obj bul_spray.obj \
	  blst_chf.obj blst_stn.obj water_mine.obj bul_c4_mng.obj \
	  bul_m4demo.obj bul_fatman_c4_mng.obj bul_book.obj \
	  blast_water.obj bmbctrl.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJECTS) : ../../../game/player.h
bul_c4.o :	bul_c4.h
bul_c4_mng.o : bul_c4.h

