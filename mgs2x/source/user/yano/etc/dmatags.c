//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    dmatags.cc

    2001/03/08 T.Shibata

    $Id: dmatags.c,v 1.1.1.3 2002/11/19 11:51:59 Yoshizawa1 Exp $
*/

//////// インクルード ////////
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"camera.h"
#include	"def_dma.h"

#include "../include/dmatags.h"

///////////////// 初期化DMAパケット
void *InitDmaVifCodeTags( void *addr, u_int dmatag, void *send_addr, u_int vifcode0, u_int vifcode1 )
{
	DG_DMATAG *dg_dmatag = (DG_DMATAG*)addr;
	dg_dmatag->qwc = dmatag;//DMATAG_SET_QWC( dmatag_id, size );
	dg_dmatag->addr = send_addr;
	dg_dmatag->vifcode[0] = vifcode0;
	dg_dmatag->vifcode[1] = vifcode1;
	dg_dmatag++;
	return dg_dmatag;
}

#if 0 /////////////////
void *InitTexFlush( void *addr )
{
	TEX_FLUSH	*tex_flush = (TEX_FLUSH*)addr;
	tex_flush->giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(DG_GSREG), 1, 0, 0, 0, 1);
	tex_flush->giftag.regs = GS_REGS_AD;
	tex_flush->texflush.reg = SCE_GS_TEXFLUSH;
	tex_flush->texflush.data = 0;
	tex_flush++;
	return tex_flush;
}

void *AddDmaTagsTexFlush( void *addr )
{
	addr = InitDmaVifCodeTags( addr,
							   DMATAG_SET_QWC(DMATAG_ID_CNT, SIZEOF_QWORD(TEX_FLUSH) ),
							   NULL,
							   SCE_VIF1_SET_NOP( 0 ),
							   SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(TEX_FLUSH), 0 ) );
	addr = InitTexFlush( addr );
	
	return addr;
}

void *InitZbuffSet( void *addr, u_long64 zbuff )
{
	ZBUFF_SET	*zbuff_set = (ZBUFF_SET*)addr;
	zbuff_set->giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(DG_GSREG), 1, 0, 0, 0, 1);
	zbuff_set->giftag.regs = GS_REGS_AD;
	zbuff_set->zbuff.reg = SCE_GS_ZBUF_1;
	zbuff_set->zbuff.data = zbuff;
	zbuff_set++;
	return zbuff_set;
}

void *InitFrameSet( void *addr, u_long64 frame )
{
	FRAME_SET	*frame_set = (FRAME_SET*)addr;
	frame_set->giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(DG_GSREG), 1, 0, 0, 0, 1);
	frame_set->giftag.regs = GS_REGS_AD;
	frame_set->frame.reg = SCE_GS_FRAME_1;
	frame_set->frame.data = frame;
	frame_set++;
	return frame_set;
}

void *AddDmaTagsFrameSet( void *addr, u_long64 frame )
{
	addr = InitDmaVifCodeTags( addr,
							   DMATAG_SET_QWC(DMATAG_ID_CNT, SIZEOF_QWORD(FRAME_SET) ),
							   NULL,
							   SCE_VIF1_SET_NOP( 0 ),
							   SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(FRAME_SET), 0 ) );
	addr = InitFrameSet( addr, frame );
	
	return addr;
}

void *InitScreenDraw( void *addr, u_long64 prim, u_long64 alpha, u_long64 rgbaq )
{
	SCREEN_DRAW			*scr_draw = (SCREEN_DRAW*)addr;
	SCREEN_DRAW_DATA	*data = &scr_draw->data;

	scr_draw->giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(SCREEN_DRAW_DATA), 1, 0, 0, 0, 1);
	scr_draw->giftag.regs = GS_REGS_AD;

	data->texflush.reg = SCE_GS_TEXFLUSH;
	data->texflush.data = 0;
	data->test.reg = SCE_GS_TEST_1;
	data->test.data = SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 );
	data->prim.reg = SCE_GS_PRIM;
	data->prim.data = prim;
	data->alpha.reg = SCE_GS_ALPHA_1;
	data->alpha.data = alpha;
	data->rgbaq.reg = SCE_GS_RGBAQ;
	data->rgbaq.data = rgbaq;

	data->xyz0.reg = SCE_GS_XYZ3;
	data->xyz0.data = SCE_GS_SET_XYZ(MIN_X,MIN_Y,0);
	data->xyz1.reg = SCE_GS_XYZ2;
	data->xyz1.data = SCE_GS_SET_XYZ(MAX_X,MAX_Y,0);

	scr_draw++;
	return scr_draw;
}

void *AddDmaTagsScreenDraw( void *addr, u_long64 prim, u_long64 alpha, u_long64 rgbaq )
{
	addr = InitDmaVifCodeTags( addr,
							   DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(SCREEN_DRAW) ), NULL,
							   SCE_VIF1_SET_NOP( 0 ), SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(SCREEN_DRAW), 0 ) );
	addr = InitScreenDraw( addr, prim, alpha, rgbaq );

	return addr;
}

static void SetVertsScrDrawDiv( SPRT_PARTS *sprt, int z )
{
	SPRT_PARTS *verts = sprt;
	int		i;
	int		x,xw,u,uw;

	x = ( 2048 - DRAW_WIDTH  / 2 ) * 16;
	u = 16;
	xw = ( DRAW_WIDTH  ) * 16 / DRAW_DIV;
	uw = ( DRAW_WIDTH  ) * 16 / DRAW_DIV;

	for( i = 0; i < DRAW_DIV; i++ ){
		verts->xyz0.reg = SCE_GS_XYZ3;
		verts->xyz1.reg = SCE_GS_XYZ2;
		verts->uv0.reg = SCE_GS_UV;
		verts->uv1.reg = SCE_GS_UV;

		verts->xyz0.data = SCE_GS_SET_XYZ( x   , MIN_Y, z );
		verts->xyz1.data = SCE_GS_SET_XYZ( x+xw, MAX_Y, z );
		verts->uv0.data = SCE_GS_SET_UV( u   , MIN_UV );
		verts->uv1.data = SCE_GS_SET_UV( u+uw, MAX_V  );

		x += xw;
		u += uw;
		verts++;
	}
}

void *InitScrDrawDiv( void *addr,
					  u_long64 prim, u_long64 alpha, u_long64 rgbaq,
					  u_long64 tex, u_long64 test, int z )
{
	SCR_DRAW_DIV				*scr_draw_div = (SCR_DRAW_DIV*)addr;
	SCR_DRAW_DIV_DATA			*data = &scr_draw_div->data;

	scr_draw_div->giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(SCR_DRAW_DIV_DATA), 1, 0, 0, 0, 1);
	scr_draw_div->giftag.regs = GS_REGS_AD;

	data->alpha.reg = SCE_GS_ALPHA_1;
	data->alpha.data = alpha;
	data->tex0.reg = SCE_GS_TEX0_1;
	data->tex0.data = tex;
	data->test.reg = SCE_GS_TEST_1;
	data->test.data = test;
	data->rgbaq.reg = SCE_GS_RGBAQ;
	data->rgbaq.data = rgbaq;
	data->prim.reg = SCE_GS_PRIM;
	data->prim.data = prim;

	SetVertsScrDrawDiv( data->sprt, z );
	scr_draw_div++;
	return scr_draw_div;
}

#endif ////

//汎用プリミティブ設定
void *InitSetDrawPrim( void *addr, u_long64 prim, u_long64 alpha, u_long64 tex, u_long64 clamp, int test )
{	
	SET_DRAW_PRIM		*set_draw_prim = (SET_DRAW_PRIM*)addr;
	SET_DRAW_PRIM_DATA	*data = &set_draw_prim->data;
	
	set_draw_prim->giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(SET_DRAW_PRIM_DATA), 0, 0, 0, 0, 1);/* このあと、
																									頂点情報が必ず来る
																									のでEOP=0 */
	set_draw_prim->giftag.regs = GS_REGS_AD;

	data->texflush.reg = SCE_GS_TEXFLUSH;
	data->texflush.data = 0;
	data->clamp.reg = SCE_GS_CLAMP_1;
	data->clamp.data = clamp;
	data->test.reg = SCE_GS_TEST_1;
	data->test.data = SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, test );
	data->tex0.reg = SCE_GS_TEX0_1;
	data->tex0.data = tex;
	data->alpha.reg = SCE_GS_ALPHA_1;
	data->alpha.data = alpha;
	data->prim.reg = SCE_GS_PRIM;
	data->prim.data = prim;


	set_draw_prim++;
	return set_draw_prim;
}

#if 0 ////////////
void *InitTexDraw( void *addr, u_long64 prim, u_long64 alpha, u_long64 rgbaq, u_long64 tex, u_long64 clamp, int test )
{
	TEX_DRAW		*tex_draw = (TEX_DRAW*)addr;
	TEX_DRAW_DATA	*data = &tex_draw->data;
	
	tex_draw->giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(TEX_DRAW_DATA), 1, 0, 0, 0, 1);
	tex_draw->giftag.regs = GS_REGS_AD;

	data->texflush.reg = SCE_GS_TEXFLUSH;
	data->texflush.data = 0;
	data->clamp.reg = SCE_GS_CLAMP_1;
	data->clamp.data = clamp;//SCE_GS_SET_CLAMP( 1, 1, 0, 0, 0, 0 );
	data->test.reg = SCE_GS_TEST_1;
	data->test.data = SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, test );
	data->tex0.reg = SCE_GS_TEX0_1;
	data->tex0.data = tex;//SCE_GS_SET_TEX0( 0,0,0,10,10,0,0,0,0,0,0,0);
	data->alpha.reg = SCE_GS_ALPHA_1;
	data->alpha.data = alpha;
	data->rgbaq.reg = SCE_GS_RGBAQ;
	data->rgbaq.data = rgbaq;
	data->prim.reg = SCE_GS_PRIM;
	data->prim.data = prim;


//	printf("%x:s_mode %d:t mode %d:u_min_max %d_%d:v_min_max %d_%d\n",
//		   clamp,clamp&3,(clamp>>2)&3,(clamp>>4)&0x3ff,(clamp>>14)&0x3ff,(clamp>>24)&0x3ff,(clamp>>34)&0x3ff);
	tex_draw++;
	return tex_draw;
}

void *AddDmaTagsTexDraw( void *addr, u_long64 prim, u_long64 alpha, u_long64 rgbaq, u_long64 tex, u_long64 clamp, int test )
{
	addr = InitDmaVifCodeTags( addr,
							   DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(TEX_DRAW) ), NULL,
							   SCE_VIF1_SET_NOP( 0 ), SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(TEX_DRAW), 0 ) );
	addr = InitTexDraw( addr, prim, alpha, rgbaq, tex, clamp, test );

	return addr;
}


static void *MakeTexturePacket32( void *_addr, DEF_TEXLIST *texlist )
{
	void	*addr = _addr;
	const UINT32 IMAGE_SIZE_MAX = 496 * 1024;
	DEF_TEXTURE*	_ptex;
	sceGsLoadImage*	_ploadimage;
	UINT16 _u16Width, _u16Height, _u16OffsetH;
	UINT32 _u32Size;
//	u_long64			*ul_data;

	addr = InitDmaVifCodeTags( addr, DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ), NULL,
							   SCE_VIF1_SET_FLUSH(0), SCE_VIF1_SET_NOP( 0 ) );
	
	//一応入れておく ---------------------------
#if 0
	addr = InitDmaVifCodeTags( addr, DMATAG_SET_QWC(DMATAG_ID_CNT, 2 ), NULL,
							   SCE_VIF1_SET_NOP( 0 ), SCE_VIF1_SET_DIRECT( 2, 0 ) );
	ul_data = (u_long64*)addr;
	ul_data[0] = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
	ul_data[1] = GS_REGS_AD;
	//ul_data[2] = 0;
	ul_data[3] = SCE_GS_TEXFLUSH;
	ul_data += 4;
	addr = ul_data;
#else
	addr = AddDmaTagsTexFlush( addr );
#endif
	//ここまで ----------------------------
	
	_ptex = texlist->ptex;
	_u16Width = _ptex->u16Width;
	_u16Height = _ptex->u16Height;
	_u16OffsetH = 0;

	do {
		UINT8* _pData;
		// テクスチャをロードする
		addr = InitDmaVifCodeTags( addr,
								   DMATAG_SET_QWC(DMATAG_ID_CNT, SIZEOF_QWORD(sceGsLoadImage)),
								   NULL,
								   SCE_VIF1_SET_NOP( 0 ),
								   SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(sceGsLoadImage), 0) );

		_ploadimage = (sceGsLoadImage*)addr;		

		_u16Height = _ptex->u16Height - _u16OffsetH;
		_u32Size = _u16Width * _u16Height * 4;

		// 転送サイズ制限オーバーなら分割転送を行う
		if (_u32Size > IMAGE_SIZE_MAX) {
			// 幅/高さを計算し直す
			_u16Height = IMAGE_SIZE_MAX / (_u16Width * 4);
			_u32Size = _u16Width * _u16Height * 4;
		}

		sceGsSetDefLoadImage(_ploadimage, _ptex->regTex.TBP0,
							 _ptex->regTex.TBW, _ptex->regTex.PSM, 0,0,
							 _u16Width, _u16Height);
		_ploadimage->trxpos.DSAY = _u16OffsetH; // オフセット位置を修正
		_u32Size = (_u32Size + 0xf) >> 4;       // 転送サイズを１６バイトの倍数に
		_ploadimage->giftag1.NLOOP = _u32Size;

		_ploadimage++;
		addr = _ploadimage;

		_pData = (UINT8*) _ptex->pvData + _u16OffsetH * _u16Width * 4;

		addr = InitDmaVifCodeTags( addr,
								   DMATAG_SET_QWC( DMATAG_ID_REF, _u32Size),
								   _pData,
								   SCE_VIF1_SET_NOP( 0 ),
								   SCE_VIF1_SET_DIRECT(_u32Size, 0) );
		
		_u16OffsetH += _u16Height; // オフセット位置更新
	} while (_u16OffsetH != _ptex->u16Height);

#if 0
	// TEXFLUSH をいじる
	addr = InitDmaVifCodeTags( addr, DMATAG_SET_QWC(DMATAG_ID_CNT, 2 ), NULL,
							   SCE_VIF1_SET_NOP( 0 ), SCE_VIF1_SET_DIRECT( 2, 0 ) );
	ul_data = (u_long64*)addr;
	ul_data[0] = SCE_GIF_SET_TAG(1, 1, 0, 0, SCE_GIF_PACKED, 1);
	ul_data[1] = GS_REGS_AD;
	//ul_data[2] = 0;
	ul_data[3] = SCE_GS_TEXFLUSH;
	ul_data += 4;
	addr = ul_data;
#else
	addr = AddDmaTagsTexFlush( addr );
#endif
	
	return addr;
}

static void *MakeTexturePacket( void *_addr, DEF_TEXLIST *texlist )
{
	void	*addr = _addr;
	DEF_TEXTURE*	_ptex = texlist->ptex;
	DEF_CLUT*		_pclut = texlist->pclut;
	sceGsLoadImage*	_ploadimage;
	int		i;
	//u_long64			*ul_data;
	addr = InitDmaVifCodeTags( addr, DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ), NULL,
							   SCE_VIF1_SET_FLUSH(0), SCE_VIF1_SET_NOP( 0 ) );
	
	//一応入れておく ------------------------------
#if 0
	addr = InitDmaVifCodeTags( addr,
							   DMATAG_SET_QWC(DMATAG_ID_CNT, SIZEOF_QWORD(TEX_FLUSH) ),
							   NULL,
							   SCE_VIF1_SET_NOP( 0 ),
							   SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(TEX_FLUSH), 0 ) );	
	addr = InitTexFlush( addr );
#else
	addr = AddDmaTagsTexFlush( addr );
#endif
	//ここまで ---------------------------------

	for ( i = 0; i < texlist->u16TexNum; i++ ) {
		UINT32	_u32Size;	// バイト:転送の際
							// １６バイトの倍数にしなくてはならない
		addr = InitDmaVifCodeTags( addr,
								   DMATAG_SET_QWC(DMATAG_ID_CNT, SIZEOF_QWORD(sceGsLoadImage)),
								   NULL,
								   SCE_VIF1_SET_NOP( 0 ),
								   SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(sceGsLoadImage), 0) );
		_ploadimage = (sceGsLoadImage*)addr;

		if (_ptex->regTex.PSM == SCE_GS_PSMT4) {
			sceGsSetDefLoadImage(_ploadimage, _ptex->regTex.TBP0,
								 _ptex->regTex.TBW, _ptex->regTex.PSM, 0,0,
								 _ptex->regTex.CBP, (_ptex->u16Height + 3) & ~3);
				_u32Size = _ptex->regTex.CBP * ((_ptex->u16Height + 3)  & ~3) / 2;
		} else if (_ptex->regTex.PSM <= SCE_GS_PSMCT24) {
			// PSMCT32 or PSMCT24
			sceGsSetDefLoadImage(_ploadimage, _ptex->regTex.TBP0,
								 _ptex->regTex.TBW, _ptex->regTex.PSM, 0,0,
								 _ptex->u16Width, _ptex->u16Height);
			_u32Size = _ptex->u16Width * _ptex->u16Height * 4;
		} else {
			sceGsSetDefLoadImage(_ploadimage, _ptex->regTex.TBP0,
								 _ptex->regTex.TBW, _ptex->regTex.PSM, 0, 0,
								 _ptex->regTex.CBP, (_ptex->u16Height + 1) & ~1);
			_u32Size = _ptex->regTex.CBP * ((_ptex->u16Height + 1) & ~1);
		}
		_u32Size = (_u32Size + 0xf) >> 4; // １６バイトの倍数に。
		_ploadimage->giftag1.NLOOP = _u32Size;

		_ploadimage++;
		addr = _ploadimage;

		addr = InitDmaVifCodeTags( addr,
								   DMATAG_SET_QWC( DMATAG_ID_REF, _ploadimage->giftag1.NLOOP),
								   _ptex->pvData,
								   SCE_VIF1_SET_NOP( 0 ),
								   SCE_VIF1_SET_DIRECT(_ploadimage->giftag1.NLOOP, 0) );
		_ptex++;
	}

	for ( i = texlist->u16ClutNum; i > 0; i-- ) {
		addr = InitDmaVifCodeTags( addr,
								   DMATAG_SET_QWC(DMATAG_ID_CNT, SIZEOF_QWORD(sceGsLoadImage)),
								   NULL,
								   SCE_VIF1_SET_NOP( 0 ),
								   SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(sceGsLoadImage), 0) );
		_ploadimage = (sceGsLoadImage*)addr;

		if (_pclut->ptex->regTex.PSM & 4) { // PSMT4 系
			sceGsSetDefLoadImage(_ploadimage, _pclut->regClut.CBP,
								 1, _pclut->regClut.CPSM, 0, 0, 8, 2);
		} else {
			sceGsSetDefLoadImage(_ploadimage, _pclut->regClut.CBP,
								 1, _pclut->regClut.CPSM, 0, 0, 16, 16);
		}

		_ploadimage++;
		addr = _ploadimage;

		addr = InitDmaVifCodeTags( addr,
								   DMATAG_SET_QWC( DMATAG_ID_REF, _ploadimage->giftag1.NLOOP),
								   _pclut->pvData,
								   SCE_VIF1_SET_NOP( 0 ),
								   SCE_VIF1_SET_DIRECT(_ploadimage->giftag1.NLOOP, 0) );
		_pclut++;
	}

#if 0
	addr = InitDmaVifCodeTags( addr,
							   DMATAG_SET_QWC(DMATAG_ID_CNT, SIZEOF_QWORD(TEX_FLUSH) ),
							   NULL,
							   SCE_VIF1_SET_NOP( 0 ),
							   SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(TEX_FLUSH), 0 ) );
	addr = InitTexFlush( addr );
#else
	addr = AddDmaTagsTexFlush( addr );
#endif
	return addr;
}


//指定メモリ上にテクスチャー転送パケット作成
void *MakeDmaPacketTexPack( void *_addr, DEF_TEXLIST *texlist )
{
	void			*addr = _addr;//SYS_SCR_TOP;

	//テクスチャー転送パケット作成
	if( texlist ){
		if( texlist->u16Flag != 0 ){
			addr = MakeTexturePacket32( addr, texlist );
		}else{
			addr = MakeTexturePacket( addr, texlist );
		}
	}
	
	return addr;
}

void *InitVertsDatas_VERT_ST( void *addr, int n_verts, int z, int gifregs )
{
	DG_GIFTAG	*giftag = (DG_GIFTAG*)addr;
	_VERT_ST	*verts = (_VERT_ST*)(giftag+1);
	int				i;

	giftag->tag = SCE_GIF_SET_TAG( n_verts, 1, 0, 0, 0, SIZEOF_QWORD(_VERT_ST));
	giftag->regs = gifregs;

	for( i = 0; i < n_verts; i++ ){
		verts->xyz.x = 0;
		verts->xyz.y = 0;
		verts->xyz.z = z;
		verts->xyz.flags = 0x0000;
		verts->stq.s = 0.0f;
		verts->stq.t = 0.0f;
		verts->stq.q = 1.0f;
		verts++;
	}	
	return verts;
}
#endif ////////////////
