#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
#	1999/07/07 K.Takabe
#	$Id: makefile,v 1.70 2002/09/24 12:03:58 usr02774 Exp $
#

# ベースディレクトリ
BASEDIR = ../..

# ライブラリ名

LIBNAME = libdg.a

# オブジェクトリスト
OBJECTS =	dgd.o chanl.o frame.o screen.o chain.o dma_ctrl.o opack.o packet.o \
			pos.o light.o objs.o loader.o text.o pshade.o \
			prim2.o chain2.o \
			vanime.o \
			command.o \
			shdwchin.o \
			pshadevu.o \
			comdl.o comdl_vu.o \
			bound_vu.o \
			trans1.o trans2.o shdwtrn2.o trans3.o  \
			evmobjs.o mwtrans3.o mwtrans4.o \
			plugin.o \
			dmapack.o opt_cmf.o optcmfvu.o \
			scn_shot.o store.o debug.o \
			m_prim2.o \
			patch.o curvsurf.o reflsurf.o reflsuf2.o \
			curvsurf_debug.o covershw.o

XOBJECTS =  xchain.obj xchain2.obj xchanl.obj xcomdl.obj \
			xd3d.obj xdgd.obj xdmapack.obj xevmobjs.obj xframe.obj \
			xlight.obj xloader.obj m_weight_cpp.obj xobjs.obj \
			xpatch.obj plugin_cpp.obj xpos.obj xprim2.obj xpshade.obj \
			xscreen.obj xshdwchn.obj xtext.obj xvanime.obj \
			xdebug.obj xvbuffer.obj xm_prim2.obj xvshader.obj ximage.obj xopt_cmf.obj \
			xalpha.obj command.obj xdmactrl.obj xdgmem.obj xstore.obj \
			$(XSHADE_OBJS)

.XVSHADER_SRC = kms_sl.vsh kms_sp.vsh kms_ml.vsh kms_emap.vsh kms_smap.vsh \
				evm.vsh evm_emap.vsh evm_bmap.vsh \
				prm2poly.vsh prm2sprt.vsh \
				comdl.vsh \
				optcmf00.vsh \
				patch.vsh patch_emap.vsh patch2_emap.vsh \
				dmapk_2d.vsh dmapk_2d4.vsh \
				shdw_mk.vsh shdw_wt.vsh 
.XPSHADER_SRC = shdwmap0.psh  mtex_1tex.psh mtex_emap.psh mtex_bmap.psh mtex_gray.psh \
				mtex_smap.psh mtex_emap2.psh mtex_emap3.psh mtex_emap4.psh
XSHADE_OBJS = $(.XVSHADER_SRC:.vsh=_vsh.obj) $(.XPSHADER_SRC:.psh=_psh.obj)
#XSHADE_OBJS =  test_psh.obj shadow_vsh.obj plight_psh.obj patchl_vsh.obj patchbl_vsh.obj \
#				patchbl_psh.obj objwl_vsh.obj objw_vsh.obj objp_vsh.obj \
#				objl_vsh.obj objawl_vsh.obj objaw_vsh.obj objal_vsh.obj obja_vsh.obj \
#				obj_vsh.obj normal_psh.obj evml_vsh.obj \
#				evm_vsh.obj evm_emap_vsh.obj \
#				comdl_vsh.obj \
#				patch_vsh.obj patch_emap_vsh.obj \
#				kms_sl_vsh.obj kms_sp_vsh.obj \
#				prm2poly_vsh.obj prm2sprt_vsh.obj \
#				dmapk_2d_vsh.obj dmapk_2d4_vsh.obj \
#				optcmf00_vsh.obj \
#				shdw_mk_vsh.obj shdw_wt_vsh.obj \
#				shdwmap0_psh.obj \
#				kms_ml_vsh.obj kms_emap_vsh.obj kms_smap_vsh.obj \
#				mtex_1tex_psh.obj mtex_emap_psh.obj \
#				mtex_gray_psh.obj 

# 未使用オブジェクトリスト
NO_USE_OBJS = shadowvl.o prim.o sort.o m_weight.o \
			trans0.o shdwtrns.o mw_env.o bumpsurf.o 

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = UVADJUST
LOCAL_OPTIONS = 

# 設定の実体

# ifeq ($(MGS2_XBOX),1)
# all : $(XSHADE_OBJS)
# lib : $(XSHADE_OBJS)
# endif

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

# 旧ルーチン（現在は未使用）
trans0.o: trans0.dsm t0_clip.vu1 t0_fast.vu1 t0_shdw.vu1
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ trans0.dsm $(LISTOPTION)

# 旧ルーチン（現在は未使用）
trans1.o: trans1.dsm t1_clip.vu1 t1_fast.vu1 t1_shdw.vu1 t1_prim.vu1
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ trans1.dsm $(LISTOPTION)

# 旧ルーチン（現在は未使用）
shdwtrns.o: shdwtrns.dsm t1_clip.vu1 t1_fast.vu1 t1_shdw.vu1
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ shdwtrns.dsm $(LISTOPTION)

# 現在のデフォルトモデル＆プリミティブ描画ルーチン
trans2.o: trans2.dsm t2_fast.vu1 t2_clip.vu1 t2_prim.vu1 t4_clip.vu1 t4_fast.vu1 t5_clip.vu1 t5_fast.vu1
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ trans2.dsm $(LISTOPTION)

# 影＆スポットライト投影用
shdwtrn2.o: shdwtrn2.dsm t2_clip.vu1 t2_fast.vu1 t2_shdw.vu1 t5_clip.vu1 t5_fast.vu1 t5_shdw.vu1
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ shdwtrn2.dsm $(LISTOPTION)

# マルチテクスチャ用
trans3.o: trans3.dsm t3_clip.vu1 t3_cull.vu1 t3_fast.vu1
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ trans3.dsm $(LISTOPTION)

# XBOX Shader

%_vsh.c : %_vsh.v
	echo unsigned char VERTEX_SHADER_$*\[\] = { > $@
	od -An -tx1 -v $< | sed 's/\([a-z0-9][a-z0-9]\)/0x\1,/g'>> $@
	echo }\; >> $@

%_psh.c : %_psh.u
	echo unsigned char PIXEL_SHADER_$*\[\] = { > $@
	od -An -tx1 -v $< | sed 's/\([a-z0-9][a-z0-9]\)/0x\1,/g' >> $@
	echo }\; >> $@


%_vsh.v : %.vsh
	xsasm -nologo -D _XBOX -D UVADJUST $<  $@

%_psh.u : %.psh
	xsasm -nologo -D _XBOX $<  $@




