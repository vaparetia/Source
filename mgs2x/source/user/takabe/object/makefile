#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/10/22 K.Takabe
# $Id: makefile,v 1.1.1.3 2002/11/19 11:51:14 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = takabe.a

ifeq ($(MGS2_XBOX),1)

.CFILES =  shdwdraw.c spotdraw.c putspot.c mirror.c spottex.c wave2.c shdwctrl.c wave4.c \
		wave5.c de_wave5.c f_bridge.c shdwdrw2.c \
		wave6.c mirror2.c huge_sea.c fk_gover.c eddogtag.c ipupanel.c
# オブジェクトリスト
XOBJECTS = $(.CFILES:.c=.obj)

else

# オブジェクトリスト
OBJECTS = shdwdraw.o spotdraw.o putspot.o mirror.o spottex.o wave2.o shdwctrl.o wave4.o \
		wave5.o de_wave5.o f_bridge.o wave5_ex.o shdwdrw2.o \
		wave6.o mirror2.o huge_sea.o fk_gover.o eddogtag.o ipupanel.o \
		wave5_vu.o
NO_USEOBJ = wave3.o


endif

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

#%.vsm: %.vcl
#	vcl	-g -t8 -A -I$(INCDIR) -o$@ $<

vcl:
	vcl	-g -t8 -A -I$(INCDIR) -owave5vcl.vsm wave5vcl.vcl
	

wave5_vu.o: wave5_vu.dsm wave5vcl.vsm
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ $<
#	ee-dvp-gasp -I$(INCDIR) $< | $(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@
