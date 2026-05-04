#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.2 2002/02/01 04:43:33 usr01475 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = mode.a

# オブジェクトリスト
OBJECTS = codec.o codectbl.o cjimaku.o codecitp.o \
		codecmem.o \
		cdc_mind.o \
		codeccrt.o \
		cdc_face.o \
		cdc_load.o \
		c_indemo.o \
		c_layout.o \
		c_hsync.o \
		codeccap.o \
		3d_util.o \
		codecmtn.o \
		memcall.o \
		handycam.o \
		c_nfocus.o \
		c_ffocus.o \
		c_edge.o \
		cdcobj.o \
		edge.o

# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト


codec.o:	codec.c codec.h cjimaku.h cdc_face.h codecmem.h c_indemo.h \
		c_layout.h cdc_load.h codec_config.h cdc_hair.h cdc_mind.h \
		codec_signal.h

codectbl.o:	codectbl.c

cjimaku.o:	cjimaku.c cjimaku.h codecmem.h codec_config.h

codecitp.o:	codecitp.c codec.h cdc_face.h cdc_load.h codec_config.h \
		cdc_hair.h

codecmem.o:	codecmem.c codecmem.h codec.h cjimaku.h

codeccrt.o:	codeccrt.c codeccrt.h codec.h cjimaku.h codecmem.h \
		codec_config.h

c_hsync.o:	c_hsync.c c_hsync.h codecmem.h cjimaku.h codec_config.h

cdc_face.o:	cdc_face.c cdc_face.h codec.h cjimaku.h cdc_load.h codecmem.h \
		codeccrt.h c_hsync.h codec_config.h 3d_util.h codecmtn.h

cdc_load.o:	cdc_load.c cdc_load.h codec.h cjimaku.h codecmem.h \
		codec_config.h

c_indemo.o:	c_indemo.c c_indemo.h codec.h cjimaku.h codecmem.h \
		codec_config.h 3d_util.h

c_fade_io.o:	c_fade_io.c

c_d_fd_io.o:	c_d_fd_io.c

c_layout.o:	c_layout.c c_layout.h codec.h cjimaku.h cdc_face.h \
		codecmem.h c_indemo.h codec_config.h
codeccap.o:	codeccap.c

3d_util.o:	3d_util.c 3d_util.h

codecmtn.o:	codecmtn.c codecmtn.h codec.h cdc_load.h \
		codecmem.h codec_config.h

memcall.o:	memcall.c memcall.h codec.h cjimaku.h codecmem.h codec_config.h

handycam.o:	handycam.c handycam.h codecmem.h cdc_load.h codec_config.h

c_nfocus.o:	c_nfocus.c codecmem.h codec_config.h
c_ffocus.o:	c_ffocus.c codecmem.h codec_config.h

c_edge.o:	c_edge.c c_edge.h codecmem.h cjimaku.h codec_config.h edge.h
edge.o:		edge.c edge.h

cdc_mind.o:	cdc_mind.h cdc_mind.h codec.h codec_signal.h
cdcobj.o:	cdcobj.c cdcobj.h
