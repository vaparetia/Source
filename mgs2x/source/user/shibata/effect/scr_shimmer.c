//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scr_shimmer.c
	画面壊れ
	
	2001/07/28 T.Shibata
	
	$Id: scr_shimmer.c,v 1.1.1.3 2002/11/19 11:48:41 Yoshizawa1 Exp $

*/
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

#include	"def_dma.h"
#include	"../util/ts_util.h"
#include	"../util/dma_set.h"

//#define TEST

#define CLOCK_COUNT	(BP_BASE_TICK())


#define MDL_TEST	(SCE_GS_SET_TEST( 1, 7, 0, 0, 0, 0, 1, 1 ))

#ifdef TEST
#define MDL_ALPHA	(SCE_GS_SET_ALPHA(0,1,1,1,64))
#define MDL_PRIM	(SCE_GS_SET_PRIM(6,0,1,0,1,0,1,0,0))
#else
#define MDL_ALPHA	(SCE_GS_SET_ALPHA(0,1,0,1,64))
#define MDL_PRIM	(SCE_GS_SET_PRIM(4,1,1,0,1,0,1,0,0))
#endif

#define SPR_PRIM	(SCE_GS_SET_PRIM(6,0,1,0,1,0,0,0,0))

#define SPR_RGBA	(SCE_GS_SET_RGBAQ(0x80,0x80,0x80,0x80,0x3F800000))

#define SPR_ALPHA_MODE	(SCE_GS_SET_ALPHA(2,2,0,1,0x08))
#define ALP_CLEAR_TEST	(SCE_GS_SET_TEST( 0, 0, 0, 0, 0, 0, 1, 1 ))

//#define SPR_ALPHA_MODE	(SCE_GS_SET_ALPHA(0,1,2,1,0x08))

#define	ALP_CLEAR_RGBA	(SCE_GS_SET_RGBAQ(0x00,0x00,0x00,0x08,0x3F800000))

#define		PIX_SHIFT_U	(6<<4)
#define		PIX_SHIFT_V	(4<<4)

extern void *DG_WritePacket_SoftImageTurnOut( void *addr, int which );

//スプライトの頂点

typedef struct {
	DG_GSREG	st0;
	DG_GSREG	xyz0;
	DG_GSREG	st1;
	DG_GSREG	xyz1;
} SPRT_VERTS_F;


#define N_SHMMR_SPRT_ALP	(32)
#define	SHMMR_SPRT_ALP_SIZE	(96.0f)

typedef struct _shimmer_alpha {
	DG_GIFTAG		giftag;
	SPRT_VERTS_F	sprt_verts[N_SHMMR_SPRT_ALP];
} SHIMMER_ALPHA;

#define N_SHMMR_SPRT	(2)

typedef struct _shimmer_verts {
	DG_GIFTAG		giftag;
	MVERT_DATA		verts[N_SHMMR_SPRT][4];
} SHIMMER_VERTS;

typedef struct {
	DG_DMATAG		dmatag_flush0;		// ＧＩＦ接続ＤＭＡタグ			フラッシュ
	DROW_FLUSH 		drow_flush0;		//								フラッシュ

	DG_DMATAG		dmatag_alp_clear;	// ＧＩＦ接続ＤＭＡタグ			アルファクリアー
	FRAME_CLEAR		alp_clear;			//								アルファクリアー
	DG_DMATAG		dmatag_flush1;		// ＧＩＦ接続ＤＭＡタグ			フラッシュ
	DROW_FLUSH 		drow_flush1;		//								フラッシュ

	//テクスチャー転送とアルファのみ描画	
	u_long128		tex_packet_alp[2];		//								テクスチャー転送
	DG_DMATAG		dmatag_spr_alp;			// ＧＩＦ接続ＤＭＡタグ			スプライト描画の設定
	TEX_DRAW		spr_draw_alp;			//								スプライト描画の設定

	DG_DMATAG		dmatag_alpha;			// ＧＩＦ接続ＤＭＡタグ			スプライト頂点
	SHIMMER_ALPHA	shimmer_alpha;			// ＧＩＦ接続ＤＭＡタグ			スプライト頂点
#if 0
	DG_DMATAG		dmatag_flush1;		// ＧＩＦ接続ＤＭＡタグ			フラッシュ
	DROW_FLUSH 		drow_flush1;		//								フラッシュ
	DG_DMATAG		dmatag_sys;			// ＧＩＦ接続ＤＭＡタグ			退避
	DG_DMATAG		dmatag_flush2;		// ＧＩＦ接続ＤＭＡタグ			フラッシュ
	DROW_FLUSH 		drow_flush2;		//								フラッシュ
#else
	DG_DMATAG		dmatag_tr;			// ＧＩＦ接続ＤＭＡタグ			退避
	TR_BUFFER		tr_buffer;			//								退避(フレームをテクスチャーエリアへ)
#endif
	
	DG_DMATAG		dmatag_mdl;			// ＧＩＦ接続ＤＭＡタグ			モデル描画の設定
	MDL_DRAW		mdl_draw;			// 								モデル描画の設定
	DG_DMATAG		dmatag_shimmer;		// ＧＩＦ接続ＤＭＡタグ			スプライト頂点
	SHIMMER_VERTS	shimmer_verts;		//								スプライト頂点
	
	DG_DMATAG		dmatag_test;		// ＧＩＦ接続ＤＭＡタグ			ピクセルテスト復元
	PACKET_END		end_paket;			//								ピクセルテスト復元
	DG_DMATAG		dmatag_offset;		// オフセット環境復元用

	DG_DMATAG		dmatag_end;			// RETタグ
}SHIMMER_PACK;

typedef struct {
	float		vx;
	float		vy;
	float		dx;
	float		dy;
	float		size;
//	int			pad;
} SPRT_SCR_DATA;

typedef	struct	{
	GV_ACT_EX			actor ;
	
	int					name;
	int					map;
	int					flags;
	DG_TEX				*tex;
	DG_TEX_PACKET		*tex_packet[2];
	DG_DMAPACK			*dmapack;
	SHIMMER_PACK		*shimmer_pack[2];
	SPRT_SCR_DATA		alp_data[N_SHMMR_SPRT_ALP];
	//float				timer;
	float				check_h;

#ifndef PSX2
	void				*packet_mem;
#endif
} Work ;


#define		DROW_SHIFT_X	((float)( 2048 - DRAW_WIDTH/2 ))
#define		DROW_SHIFT_Y	((float)( 2048 - DRAW_HEIGHT/2 ))

static void CalcSprtVerts( SPRT_SCR_DATA *data, FVECTOR *d_shift )
{
	data->vx += (data->dx + d_shift->vx);
	if( data->vx < -data->size ){
		data->vx = (float)DRAW_WIDTH + data->size;
		//data->dx *= -1.0f;
	}else if( data->vx - data->size > (float)DRAW_WIDTH ){
		data->vx = -data->size;
		//data->dx *= -1.0f;
	}
	
	data->vy += (data->dy + d_shift->vy);
	if( data->vy < -data->size ){
		data->vy = (float)DRAW_HEIGHT + data->size;
		//data->dy *= -1.0f;
	}else if( data->vy - data->size > (float)DRAW_HEIGHT ){
		data->vy = -data->size;
		//data->dy *= -1.0f;
	}
}


#define		SHIMMER_RGB		(0x808080)

#ifdef PSX2
static void SetSprtAlpVerts( SPRT_VERTS_F *out, SPRT_SCR_DATA *data, int num, FVECTOR *d_shift, int flag )
{
	int		i;

	for( i = 0; i < num; i++ ){
		int x0, y0, x1, y1 ;

		if( !flag ){
			CalcSprtVerts( data, d_shift ) ;
		}
		x0 = DG_FTOI((DROW_SHIFT_X + data->vx - data->size)*SHIBATA_FSHIFT);
		y0 = DG_FTOI((DROW_SHIFT_Y + data->vy - data->size)*SHIBATA_FSHIFT);
		x1 = DG_FTOI((DROW_SHIFT_X + data->vx + data->size)*SHIBATA_FSHIFT);
		y1 = DG_FTOI((DROW_SHIFT_Y + data->vy + data->size)*SHIBATA_FSHIFT);
		
		out->xyz0.data = SCE_GS_SET_XYZ( x0, y0, 0);
		out->xyz1.data = SCE_GS_SET_XYZ( x1, y1, 0);

		out++;
		data++;
	}
}
static void SetShimmerVerts( MVERT_DATA *verts, int num, float check_h )
{
	FVECTOR		y_vec = { 0.0f, 1.0f, 0.0f, 0.0f };
	int			alpha0;
	int			alpha1;
	float		inner,len;
	int			i;

	len = DG_FABS( check_h - DG_Chanls[0].eye.m[3][1] ) - 2000.0f;
	if( len < 0.0f ) len = 0.0f;
	else if( len > 16000.0f ) len = 16000.0f;

	len = 1.0f - len/16000.0f;

	inner = _sceVu0InnerProduct( &y_vec, (FVECTOR*)DG_Chanls[0].eye.m[2] );
	alpha0 = DG_FABS( 128.0f * inner ) * len;
	alpha1 = 128.0f * len;
	
	for( i = 0; i < num; i++ ){
		verts[0].rgbq.data = SCE_GS_SET_RGBAQ(SHIMMER_RGB,0,0,alpha0,0x3F800000);
		verts[1].rgbq.data = SCE_GS_SET_RGBAQ(SHIMMER_RGB,0,0,alpha0,0x3F800000);
		
		verts[2].rgbq.data = SCE_GS_SET_RGBAQ(SHIMMER_RGB,0,0,alpha1,0x3F800000);
		verts[3].rgbq.data = SCE_GS_SET_RGBAQ(SHIMMER_RGB,0,0,alpha1,0x3F800000);
		verts += 4;
	}
}
#else
static void *SetSprtAlpVerts( void *prim,
							  SPRT_SCR_DATA *data, int num,
							  FVECTOR *d_shift, int flag )
{
	int		i;

	for( i = 0; i < num; i++ ){
		int x0, y0, x1, y1 ;

		if( !flag ){
			CalcSprtVerts( data, d_shift ) ;
		}
		x0 = 0*DRAW_WIDTH /2 + data->vx - data->size ;
		y0 = 0*DRAW_HEIGHT/2 + data->vy - data->size ;
		x1 = 0*DRAW_WIDTH /2 + data->vx + data->size ;
		y1 = 0*DRAW_HEIGHT/2 + data->vy + data->size ;
		
		prim = DG_SetDmapackSprt( prim,
								  x0,y0, MIN_UV, MIN_UV + PIX_SHIFT_V,
								  x1,y1, MAX_U , MAX_V  + PIX_SHIFT_V,
								  SCE_GS_SET_RGBAQ(SHIMMER_RGB,0,0,128,0)) ;
		data++;
	}
	return prim ;
}

static void *SetShimmerVerts( void *prim, int num, float check_h )
{
	FVECTOR		y_vec = { 0.0f, 1.0f, 0.0f, 0.0f };
	int			alpha0;
	int			alpha1;
	float		inner,len;
	int			i;


	len = DG_FABS( check_h - DG_Chanls[0].eye.m[3][1] ) - 2000.0f;
	if( len < 0.0f ) len = 0.0f;
	else if( len > 16000.0f ) len = 16000.0f;

	len = 1.0f - len/16000.0f;

	inner = _sceVu0InnerProduct( &y_vec, (FVECTOR*)DG_Chanls[0].eye.m[2] );
	alpha0 = DG_FABS( 128.0f * inner ) * len;
	alpha1 = 128.0f * len;
	
	prim = DG_SetDmapackSprt( prim,
							 MIN_X, MIN_Y, MIN_UV, MIN_UV + PIX_SHIFT_V,
							 MAX_X, MAX_Y, MAX_U , MAX_V,
							 SCE_GS_SET_RGBAQ(SHIMMER_RGB,0,0,alpha0*0+128,0)) ;
	prim = DG_SetDmapackSprt( prim,
							 MIN_X, MIN_Y, MIN_UV + PIX_SHIFT_U, MIN_UV,
							 MAX_X, MAX_Y, MAX_U               , MAX_V ,
							 SCE_GS_SET_RGBAQ(SHIMMER_RGB,0,0,alpha0*0+128,0)) ;

	return prim ;
}
#endif

static inline void _RotVector( FVECTOR *out, FMATRIX *world, FVECTOR *in )
{
#ifdef BP_PSX2_ASM
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	vmulax.xyzw			ACC, vf4,vf8
	vmadday.xyzw		ACC, vf5,vf8
	vmaddz.xyzw			vf8, vf6,vf8
	sqc2				vf8,0x00(%0)
	": : "r"(out), "r"(world), "r"(in) );//:"memory" );
#else
	DG_SetPos( world ) ;
	DG_RotVector(in, out, 1 ) ;
#endif
}

static void Act( Work *work )
{
	static FVECTOR	y_vec = { 0.0f, 2.5f, 0.0f, 0.0f };
	FVECTOR			d_shift;

	if( !(work->flags & 0x0002) ){

#ifdef PSX2
		_RotVector( &d_shift, &DG_Chanls[0].eye, &y_vec );
		SetSprtAlpVerts( work->shimmer_pack[DG_Clock]->shimmer_alpha.sprt_verts,
						 work->alp_data, N_SHMMR_SPRT_ALP, &d_shift, work->flags & 0x0001 );
		SetShimmerVerts( work->shimmer_pack[DG_Clock]->shimmer_verts.verts[0], N_SHMMR_SPRT, work->check_h );		
#else
		void *prim ;

		_RotVector( &d_shift, &DG_Chanls[0].eye, &y_vec );
		prim = work->packet_mem ;
		prim = DG_SetDmapackModeEnable( prim, DG_DMAPACK_MODE_NO_ALPHATEST ) ;
		prim = InitFrameClear( prim, ALP_CLEAR_TEST, ALP_CLEAR_RGBA,
							   SPR_ALPHA_MODE, 0.0f );
		prim = DG_SetDmapackTex( prim, work->tex ) ;
		prim = DG_SetDmapackAlpha( prim, SPR_ALPHA_MODE ) ;
		prim = SetSprtAlpVerts( prim,
							    work->alp_data,
							    N_SHMMR_SPRT_ALP,
							   &d_shift,
							    work->flags & 0x0001 ) ;
		prim = InitTrBuffer( prim, 0 );
		prim = InitMdlDraw( prim, MDL_TEST, MDL_ALPHA, MDL_PRIM );
		prim = SetShimmerVerts( prim, N_SHMMR_SPRT, work->check_h );
		prim = DG_SetDmapackModeDisable( prim, DG_DMAPACK_MODE_NO_ALPHATEST ) ;
		prim = DG_SetDmapackEnd( prim ) ;
#endif
	}
	if (GV_PauseLevel & GV_PAUSE_PAUSE ||
		GM_CheckGameStatus( STATE_DISP_GAMEOVER ) ||
	   ( GM_CheckMenuStatus( MENU_WEAPON_OPEN | MENU_ITEM_OPEN | MENU_RADIO_ON ))){
		if( work->flags & 0x0001 )
		  work->flags |= 0x0002 ;
		work->flags |= 0x0001 ;
		return;
	}else{
		work->flags &= ~0x0003;
	}
}

static void Die( Work *work )
{
	if(work->dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
	
#ifdef PSX2
	if(work->shimmer_pack[0]) GV_DelayedFree(work->shimmer_pack[0]);
#else
	if(work->packet_mem) GV_DelayedFree( work->packet_mem );
#endif
}



#ifdef PSX2 /*!!!!!!!!!!!!PSX2 ここから下３つの関数はPSX2のみの関数!!!!!!!!!!!!!!!!!!!!!!*/

static void InitDmaTags( Work *work, SHIMMER_PACK *packet, int which )
{
	// --------  DMATag  ----------------------------
	// ＤＭＡタグ		フラッシュ
	packet->dmatag_flush0.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DROW_FLUSH) );
	packet->dmatag_flush0.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_flush0.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DROW_FLUSH), 0);

	packet->dmatag_flush1.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DROW_FLUSH) );
	packet->dmatag_flush1.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_flush1.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DROW_FLUSH), 0);
#if 0	
	packet->dmatag_flush2.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DROW_FLUSH) );
	packet->dmatag_flush2.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_flush2.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DROW_FLUSH), 0);
#else
	packet->dmatag_tr.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(TR_BUFFER) ) ;
	packet->dmatag_tr.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_tr.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(TR_BUFFER), 0) ;
#endif
	// ＤＭＡタグ		フレームのアルファ値クリアー
	packet->dmatag_alp_clear.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(FRAME_CLEAR) );
	packet->dmatag_alp_clear.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_alp_clear.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(FRAME_CLEAR), 0);
	
	// ＤＭＡタグ		スプライト描画設定
	packet->dmatag_spr_alp.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(TEX_DRAW) ) ;
	packet->dmatag_spr_alp.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_spr_alp.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(TEX_DRAW), 0) ;
	
	// ＤＭＡタグ		スプライト描画設定
	packet->dmatag_mdl.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(MDL_DRAW) ) ;
	packet->dmatag_mdl.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_mdl.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(MDL_DRAW), 0) ;

	// ＤＭＡタグ		ピクセルテスト復元
	packet->dmatag_test.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(PACKET_END) ) ;
	packet->dmatag_test.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_test.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(PACKET_END), 0) ;
	
	// オフセット環境復元用
	packet->dmatag_offset.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWOFFSET) ) ;
	packet->dmatag_offset.addr = &( DG_Chanl( 0 )->draw_offset[ which ] ) ;
	packet->dmatag_offset.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_offset.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_DRAWOFFSET), 0) ;

	// RETタグ
	packet->dmatag_end.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 ) ;
	packet->dmatag_end.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_end.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;

	// ＤＭＡタグ		スプライト頂点
	packet->dmatag_shimmer.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(SHIMMER_VERTS) ) ;
	packet->dmatag_shimmer.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_shimmer.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(SHIMMER_VERTS), 0) ;
	
	// ＤＭＡタグ		スプライト頂点
	packet->dmatag_alpha.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(SHIMMER_ALPHA) ) ;
	packet->dmatag_alpha.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_alpha.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(SHIMMER_ALPHA), 0) ;

}


static void InitShimmerVerts( SHIMMER_VERTS *shimmer_verts )
{
	int		i;
	
	shimmer_verts->giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD( MVERT_DATA )*N_SHMMR_SPRT*4, 1, 0, 0, 0, 1);
	shimmer_verts->giftag.regs = GS_REGS_AD;

	for( i = 0; i < N_SHMMR_SPRT; i++ ){
/*		int		u0 = MIN_UV;
		int		v0 = MIN_UV;
		int		u1 = MAX_U;
		int		v1 = MAX_V;
	*/	
		shimmer_verts->verts[i][0].xyz.reg = SCE_GS_XYZ3;
		shimmer_verts->verts[i][1].xyz.reg = SCE_GS_XYZ3;
		shimmer_verts->verts[i][2].xyz.reg = SCE_GS_XYZ2;
		shimmer_verts->verts[i][3].xyz.reg = SCE_GS_XYZ2;
		
		shimmer_verts->verts[i][0].uv.reg = SCE_GS_UV;
		shimmer_verts->verts[i][1].uv.reg = SCE_GS_UV;
		shimmer_verts->verts[i][2].uv.reg = SCE_GS_UV;
		shimmer_verts->verts[i][3].uv.reg = SCE_GS_UV;

		shimmer_verts->verts[i][0].rgbq.reg = SCE_GS_RGBAQ;
		shimmer_verts->verts[i][1].rgbq.reg = SCE_GS_RGBAQ;
		shimmer_verts->verts[i][2].rgbq.reg = SCE_GS_RGBAQ;
		shimmer_verts->verts[i][3].rgbq.reg = SCE_GS_RGBAQ;

		//右上から
		shimmer_verts->verts[i][0].xyz.data = SCE_GS_SET_XYZ( MIN_X, MIN_Y, 0);
		shimmer_verts->verts[i][1].xyz.data = SCE_GS_SET_XYZ( MAX_X, MIN_Y, 0);
		shimmer_verts->verts[i][2].xyz.data = SCE_GS_SET_XYZ( MIN_X, MAX_Y, 0);
		shimmer_verts->verts[i][3].xyz.data = SCE_GS_SET_XYZ( MAX_X, MAX_Y, 0);

		shimmer_verts->verts[i][0].rgbq.data = SCE_GS_SET_RGBAQ(0x80,0x80,0x80,0x00,0x3F800000);
		shimmer_verts->verts[i][1].rgbq.data = SCE_GS_SET_RGBAQ(0x80,0x80,0x80,0x00,0x3F800000);
		shimmer_verts->verts[i][2].rgbq.data = SCE_GS_SET_RGBAQ(0x80,0x80,0x80,0x80,0x3F800000);
		shimmer_verts->verts[i][3].rgbq.data = SCE_GS_SET_RGBAQ(0x80,0x80,0x80,0x80,0x3F800000);

		shimmer_verts->verts[i][0].rgbq.data = SCE_GS_SET_RGBAQ(SHIMMER_RGB,0,0,0x00,0x3F800000);
		shimmer_verts->verts[i][1].rgbq.data = SCE_GS_SET_RGBAQ(SHIMMER_RGB,0,0,0x00,0x3F800000);
		shimmer_verts->verts[i][2].rgbq.data = SCE_GS_SET_RGBAQ(SHIMMER_RGB,0,0,0x80,0x3F800000);
		shimmer_verts->verts[i][3].rgbq.data = SCE_GS_SET_RGBAQ(SHIMMER_RGB,0,0,0x80,0x3F800000);

	}

	shimmer_verts->verts[0][0].uv.data = SCE_GS_SET_UV( MIN_UV,	MIN_UV + PIX_SHIFT_V );		//上
	shimmer_verts->verts[0][1].uv.data = SCE_GS_SET_UV( MAX_U,	MIN_UV + PIX_SHIFT_V );
	shimmer_verts->verts[0][2].uv.data = SCE_GS_SET_UV( MIN_UV,	MAX_V );
	shimmer_verts->verts[0][3].uv.data = SCE_GS_SET_UV( MAX_U,	MAX_V );
#if 1
	shimmer_verts->verts[1][0].uv.data = SCE_GS_SET_UV( MIN_UV + PIX_SHIFT_U,	MIN_UV );	//右
	shimmer_verts->verts[1][1].uv.data = SCE_GS_SET_UV( MAX_U,					MIN_UV );
	shimmer_verts->verts[1][2].uv.data = SCE_GS_SET_UV( MIN_UV + PIX_SHIFT_U,	MAX_V );
	shimmer_verts->verts[1][3].uv.data = SCE_GS_SET_UV( MAX_U,					MAX_V );
#else
	shimmer_verts->verts[1][0].uv.data = SCE_GS_SET_UV( MIN_UV,	MIN_UV );					//下
	shimmer_verts->verts[1][1].uv.data = SCE_GS_SET_UV( MAX_U,	MIN_UV );
	shimmer_verts->verts[1][2].uv.data = SCE_GS_SET_UV( MIN_UV,	MAX_V - PIX_SHIFT_V );
	shimmer_verts->verts[1][3].uv.data = SCE_GS_SET_UV( MAX_U,	MAX_V - PIX_SHIFT_V );

	shimmer_verts->verts[2][0].uv.data = SCE_GS_SET_UV( MIN_UV + PIX_SHIFT_U,	MIN_UV );	//右
	shimmer_verts->verts[2][1].uv.data = SCE_GS_SET_UV( MAX_U,					MIN_UV );
	shimmer_verts->verts[2][2].uv.data = SCE_GS_SET_UV( MIN_UV + PIX_SHIFT_U,	MAX_V );
	shimmer_verts->verts[2][3].uv.data = SCE_GS_SET_UV( MAX_U,					MAX_V );

	shimmer_verts->verts[3][0].uv.data = SCE_GS_SET_UV( MIN_UV,					MIN_UV );	//左
	shimmer_verts->verts[3][1].uv.data = SCE_GS_SET_UV( MAX_U - PIX_SHIFT_U,	MIN_UV );
	shimmer_verts->verts[3][2].uv.data = SCE_GS_SET_UV( MIN_UV,				 	MAX_V );
	shimmer_verts->verts[3][3].uv.data = SCE_GS_SET_UV( MAX_U - PIX_SHIFT_U,	MAX_V );
#endif
}

static void InitShimmerAlpha( SHIMMER_ALPHA *shimmer_alpha, DG_TEX *tex )
{
	float			st[4];
	int				*ist,i;
	SPRT_VERTS_F	*data = shimmer_alpha->sprt_verts;

	st[0] = 0.0f * tex->u_scale+tex->u_offset;
	st[1] = 0.0f * tex->v_scale+tex->v_offset;
	st[2] = 1.0f * tex->u_scale+tex->u_offset;
	st[3] = 1.0f * tex->v_scale+tex->v_offset;

	ist = (int*)st;

	shimmer_alpha->giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD( SPRT_VERTS_F )*N_SHMMR_SPRT_ALP, 1, 0, 0, 0, 1);
	shimmer_alpha->giftag.regs = GS_REGS_AD;

	for( i = 0; i < N_SHMMR_SPRT_ALP; i++ ){
		data->xyz0.reg = SCE_GS_XYZ2;
		data->xyz1.reg = SCE_GS_XYZ2;
		data->st0.reg = SCE_GS_ST;
		data->st1.reg = SCE_GS_ST;

		data->xyz0.data = SCE_GS_SET_XYZ( MIN_X, MIN_Y, 0);
		data->xyz1.data = SCE_GS_SET_XYZ( MAX_X, MAX_Y, 0);
//		if( irnd()&0x00800000 ){
			data->st0.data = SCE_GS_SET_ST( ist[0], ist[1] );
			data->st1.data = SCE_GS_SET_ST( ist[2], ist[3] );
//		}else{
//			data->st1.data = SCE_GS_SET_ST( ist[0], ist[1] );
//			data->st0.data = SCE_GS_SET_ST( ist[2], ist[3] );
//		}
		data++;
	}
}


#endif /*!!!!!!!!!!!!!!!!!!!!!!!!!PSX2 ここから上３つの関数はPSX2のみの関数!!!!!!!!!!!!!!!!!!!!!!!!!*/

static void InitAlpSprtData( SPRT_SCR_DATA *data, int num )
{
	int		i;
	for( i = 0; i < num; i++ ){
		data->size = SHMMR_SPRT_ALP_SIZE;
		data->vx = rnd()*(float)DRAW_WIDTH;
		data->vy = rnd()*(float)DRAW_HEIGHT;
		data->dx = ( irnd()&0x0040000 )? -(rnd()*1.5f+1.0f):(rnd()*1.5f+1.0f);
		data->dy = ( irnd()&0x0040000 )? -(rnd()*1.5f+1.0f):(rnd()*1.5f+1.0f);
		data++;
	}
}


static int GetResources( Work *work )
{
	DG_DMAPACK		*dmapack;
	SHIMMER_PACK	*packet;
//	u_char			*verts;
	DG_TEXTURE_LIST *texlist;
	int				i;
#define		TRI_CODE	(GV_StrCode("shimmer"))
//#define		TEX_CODE	(GV_StrCode("haze01_alp_ovl"))
//#define		TEX_CODE	(GV_StrCode("haze02_alp_ovl"))
#define		TEX_CODE	(GV_StrCode("haze03_alp_ovl"))

	
	if(GCL_GetOption('h')){
		work->check_h = (float)GCL_GetNextInt();
	}else{
		work->check_h = 0.0f;
	}
	
	texlist = DG_GetTextureList( TRI_CODE );
	if(texlist==NULL){
		printf("ERR!! NO TRI!!\n");
		return -1;
	}

	work->tex = DG_GetTexture2( TRI_CODE, TEX_CODE );
	if( !work->tex ){
		printf("ERR!! NO TEX!!\n");
		return -1;
	}
	work->tex_packet[0] = &texlist->tex_packet[0];
	work->tex_packet[1] = &texlist->tex_packet[1];
	
	// ＤＭＡパケット型オブジェクト作成
	work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL, DG_DMAPACK_PHASE_AFTER, 144 );
	if(dmapack==NULL){ printf("ERR!! MAKE DMAPACK!!\n"); return -1; }
	DG_QueueDmapack( dmapack );

	// パケットメモリ割り当て
#ifdef PSX2
	packet = GV_Malloc( sizeof(SHIMMER_PACK) * 2 );
	if(packet == NULL){ printf("ERR!! MALLOC SHIMMER_PACK!!\n"); return -1; }
	
	work->shimmer_pack[0] = dmapack->packet[0] = &packet[0] ;
	work->shimmer_pack[1] = dmapack->packet[1] = &packet[1] ;
	
	for ( i = 0 ; i < 2 ; i++ ){
		InitDmaTags( work, &packet[i], i );
		InitShimmerVerts( &packet[i].shimmer_verts );
		InitMdlDraw( &packet[i].mdl_draw, MDL_TEST, MDL_ALPHA, MDL_PRIM );
		InitEndPacket( &packet[i].end_paket );
		InitDrowFlush( &packet[i].drow_flush0 );

		InitFrameClear( &packet[i].alp_clear, ALP_CLEAR_TEST, ALP_CLEAR_RGBA, SPR_ALPHA_MODE, 0.0f );
		InitDrowFlush( &packet[i].drow_flush1 );
#if 0
		DG_WritePacket_SoftImageTurnOut( &packet[i].dmatag_sys, i );

		InitDrowFlush( &packet[i].drow_flush2 );
#else
		InitTrBuffer( &packet[i].tr_buffer, i );
#endif
		InitSetTexDraw( &packet[i].spr_draw_alp, SPR_RGBA, SPR_PRIM );
		TexDataSet( packet[i].tex_packet_alp, work->tex_packet[i],
					&packet[i].spr_draw_alp, &work->tex->tex_trans, SPR_ALPHA_MODE );
		InitShimmerAlpha( &packet[i].shimmer_alpha, work->tex );

		packet[i].spr_draw_alp.data.test.data = SCE_GS_SET_TEST( 1, 6, 4, 0, 0, 0, 1, 1 );
		packet[i].mdl_draw.data.tex0.data = SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE()/64,
															 BUFFER_WIDTH/64,
															 FRAME_BUFFER_COLOR_MODE(),
															 10,10,1,0,0,0,0,0,0);
#ifdef TEST
		//ブラーモード
		packet[i].mdl_draw.data.tex0.data = SCE_GS_SET_TEX0( BUFFER_PAGE(1-i)/64,
															 BUFFER_WIDTH/64,
															 FRAME_BUFFER_COLOR_MODE(),
															 10,10,0,0,0,0,0,0,0);
#endif
	}
#else /*PSX2*/

	if ( !(work->packet_mem = GV_Malloc( sizeof(SHIMMER_PACK) )) ) {
		return -1 ;
	}
	dmapack->autopacket = work->packet_mem ;
	DG_SetDmapackEnd( work->packet_mem );

#endif /*PSX2*/

	InitAlpSprtData( work->alp_data, N_SHMMR_SPRT_ALP );
	
	return (0);
}


static Work *g_work = NULL;

void *NewScrShimmer( int name, int map )
{
	Work		*work ;

	//printf("SCR_BOTTOM = %p\n", MEM_SCR_BOTTOM);
	//if(g_work) return NULL;
		
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
//	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
		g_work = work;
	}
	
	//printf("size/128 = %f\n",(float)work->m_size/(float)sizeof(u_long128));
	//printf("Points Num = %d\n", N_POINTS);
	//printf("babababababab\n");

	return (void *)work ;
}


