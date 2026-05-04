#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
#	1999/08/16 K.Takabe
#	$Id: makefile,v 1.31 2002/05/07 12:56:22 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../..

# ライブラリ名

LIBNAME = libutl.a

# オブジェクトリスト
OBJECTS = util_dmy.o utl_dma.o scratch.o rand.o makequat.o \
	objcollision.o objcollisionv.o objcollisionb.o boundary.o ctrlcollision.o \
	vertex_animation.o motion_animation.o human_ma.o vertex_search.o \
	malti_matrix.o objchange.o calcrgb.o tmplight.o vectomat.o utl_lzh.o \
	partchg.o partchg_evm.o \
	utl_vib.o \
	jpeg_common.o jpeg_encode.o jpeg_decode.o thumbnail.o \
	utl_anmtex.o utl_eft_bound.o


XOBJECTS = util_dmy.obj  utl_dma.obj scratch.obj rand.obj makequat.obj \
	objcollision.obj objcollisionv.obj objcollisionb.obj boundary.obj ctrlcollision.obj \
	vertex_animation.obj motion_animation.obj human_ma.obj vertex_search.obj \
	objchange.obj calcrgb.obj tmplight.obj vectomat.obj utl_lzh.obj \
	partchg.obj partchg_evm.obj \
	utl_vib.obj \
	jpeg_common.obj jpeg_encode.obj jpeg_decode.obj thumbnail.obj \
	utl_anmtex.obj utl_eft_bound.obj sse_pack_xbox.obj utlx_cpp.obj \
	malti_matrix.obj 

#まだやっていない　T.Morita 20002.02.01
# part_sub.obj  でもDG_PRIMなので移植必要なし？



# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

trans0.o: trans0.dsm t0_clip.vu1 t0_fast.vu1 t0_shdw.vu1
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ trans0.dsm $(LISTOPTION)

trans1.o: trans1.dsm t1_clip.vu1 t1_fast.vu1 t1_shdw.vu1
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ trans1.dsm $(LISTOPTION)

jpeg_encode.o jpeg_decode.o jpeg_common.o : jpeg.h
