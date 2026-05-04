#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:50:16 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = skoba.a

# オブジェクトリスト
OBJECTS = c_box.o scope.o dg_cam.o bsensB.o \
	  	  uniform.o eq_magazine.o stealth.o \
	  	  bandana.o wig.o \
		  box_hidden.o goggles.o dg_cam_mng.o capture.o \
		  evm_skirt.o demo_dgcam_mng.o demo_evm_skirt.o

XOBJECTS = c_box.obj scope.obj bsensB.obj \
	  	  uniform.obj stealth.obj dg_cam_mng.obj \
	  	  bandana.obj wig.obj dg_cam.obj\
		  box_hidden.obj goggles.obj eq_magazine.obj \
		  evm_skirt.obj demo_dgcam_mng.obj demo_evm_skirt.obj \
		  capture.obj 

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト


capture.obj	: ../../kano/xmcman/xmcman.h
