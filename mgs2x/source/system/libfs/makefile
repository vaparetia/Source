#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.15 2002/07/15 10:58:39 usr01475 Exp $
#

# ベースディレクトリ
BASEDIR = ../..

# ライブラリ名

LIBNAME = libfs.a

# オブジェクトリスト
OBJECTS = fshd.o fsusb.o fscd.o stageld.o stagezcd.o stream.o

SELECTOBJS = link_hd.o link_hdc.o link_cd.o link_cdd.o \
			 link_usb.o link_usbd.o link_hdu.o link_hdud.o

#XOBJECTS = fshdx.obj stageldx.obj stream.obj fsxdev.obj cdbiosx.obj cdbiosx_dev.obj
XOBJECTS = cdbios.obj cdbios_dev.obj cdbios_xdat.obj fshdx.obj stageldx.obj stream.obj \
			 fsxdev.obj cdutil.obj stagezcd.obj 
XSELECTOBJS = link_hd.obj link_hdu.obj link_cd.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# ターゲット

TARGET = $(foreach A, $(OBJECTS), $(BASEDIR)/lib/$(LIBNAME)($(A))) 

# コマンド

ifneq ($(MGS2_XBOX),1)	# -------------------------- for PS2

.PHONY : all lib link clean clean2

all : $(SELECTOBJS)
lib : $(SELECTOBJS)
clean: clean2

else					# -------------------------- for XBOX

.PHONY : all lib link clean clean2

all : $(XSELECTOBJS)
lib : $(XSELECTOBJS)
clean: xclean2

endif

# 設定の実体

include $(BASEDIR)/makefile.sub

clean2:
	-rm -f $(SELECTOBJS)

xclean2:
	-rm -f $(XSELECTOBJS)

# 依存関係のリスト

# ------- fs

fsusb.o : fshd.c
	$(CC) $(CFLAGS) -I$(INCDIR) -D _USB_ -c $< -o $@

# ------- MODULESELECT

# 基本モード

link_hd.o : select.c file.cnf
	$(CC) $(CFLAGS) -I$(INCDIR) -D _HD_ -c $< -o $@

link_cd.o : select.c file.cnf
	$(CC) $(CFLAGS) -I$(INCDIR) -D _CD_ -c $< -o $@

link_usb.o : select.c file.cnf
	$(CC) $(CFLAGS) -I$(INCDIR) -D _USB_ -c $< -o $@

link_hdu.o : select.c file.cnf
	$(CC) $(CFLAGS) -I$(INCDIR) -D _HDU_ -c $< -o $@

# デバッグ用モード

link_hdc.o : select.c file.cnf
	$(CC) $(CFLAGS) -I$(INCDIR) -D _HD_ -D _STREAM_CD_ -c $< -o $@

link_cdd.o : select.c file.cnf
	$(CC) $(CFLAGS) -I$(INCDIR) -D _CD_ -D _HDDEBUG_ -c $< -o $@

link_usbd.o : select.c file.cnf
	$(CC) $(CFLAGS) -I$(INCDIR) -D _USB_ -D _HDDEBUG_ -c $< -o $@

link_hdud.o : select.c file.cnf
	$(CC) $(CFLAGS) -I$(INCDIR) -D _HDU_ -D _HDDEBUG_ -c $< -o $@

# XBOX用モード
link_hd.obj : selectx.c file.cnf
	$(CC) $(CFLAGS) -c -D_DEV_ $< -Fo$@

link_hdu.obj : selectx.c file.cnf
	$(CC) $(CFLAGS) -c -D_HDU_ $< -Fo$@

link_cd.obj : selectx.c file.cnf
	$(CC) $(CFLAGS) -c -D_XDAT_ $< -Fo$@


cdbiosx_dev.obj : cdbiosx.c
	$(CC) $(CFLAGS) -c -D_DEV_ $< -Fo$@

cdbiosx.obj : cdbiosx.c
	$(CC) $(CFLAGS) -c -D_HDU_ $< -Fo$@

cdbios_dev.obj : cdbios.c
	$(CC) $(CFLAGS) -c -D_DEV_ $< -Fo$@

cdbios_xdat.obj : cdbios.c
	$(CC) $(CFLAGS) -c -D_XDAT_ $< -Fo$@

cdbios.obj : cdbios.c
	$(CC) $(CFLAGS) -c -D_HDU_ $< -Fo$@
	
