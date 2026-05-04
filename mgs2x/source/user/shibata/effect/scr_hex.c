//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scr_drop.c
	歪みモデルを使った奴(歪み変化)

	2001/07/10 T.Shibata
	
	$Id: scr_hex.c,v 1.5 2002/11/23 12:16:40 Yoshizawa1 Exp $

*/

#ifdef PSX2
#define REDEFINEPSX2
#undef PSX2
#endif

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
#include	"../util/ts_util.h"
#include	"../util/dma_set.h"


#define CLOCK_COUNT	(BP_BASE_TICK())

#define MDL_PRIM		(SCE_GS_SET_PRIM( 4, 0, 1, 0, 1, 0, 1, 0, 0 ))
#define MDL_TEST		(SCE_GS_SET_TEST( 0, 7, 0, 0, 0, 0, 1, 1 ))
#define MDL_ALPHA		(SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 ))

#define LINE_PRIM		(SCE_GS_SET_PRIM( 2, 1, 1, 0, 1, 0, 1, 0, 0 ))
#define LINE_TEST		(SCE_GS_SET_TEST( 0, 7, 0, 0, 0, 0, 1, 1 ))
#define LINE_ALPHA		(SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 ))

#define		SCR_ROOT3	(1.732050808f*720.0f/DRAW_HEIGHT)

#ifdef PSX2
#define		HEX_LEN		(24.0f * (float)DRAW_HEIGHT/448.0f)		//一片の長さ(一画面512に対して)
#define		N_HEXS_W	((int)((float)DRAW_WIDTH/(HEX_LEN*3.0f)*2.0f+1.0f))
#define		N_HEXS_H	((int)((float)DRAW_HEIGHT/(HEX_LEN*SCR_ROOT3)+1.0f))
#else
#define		HEX_LEN		(24.0f * (float)DRAW_HEIGHT/448.0f)		//一片の長さ(一画面512に対して)
#define		N_HEXS_W	((int)((float)DRAW_WIDTH/(HEX_LEN*3.0f)*5.0f/4.0f*2.0f+1.0f))
#define		N_HEXS_H	((int)((float)DRAW_HEIGHT/(HEX_LEN*SCR_ROOT3)+1.0f))
#endif

#define		N_HEXS		(N_HEXS_W*N_HEXS_H-N_HEXS_W/2)
#define		N_VERTS		(N_HEXS*6)

#define		INIT_X		(0.0f)
#define		INIT_Y		(0.0f - HEX_LEN*SCR_ROOT3/2.0f)

#define		N_CMN_VERTS_W	(N_HEXS_W+1)
#define		N_CMN_VERTS_H	(N_HEXS_H*2+1)

#define		DEF_COLOR		(0x80808040)

extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );

//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

typedef struct _hex_data {
	short		count,bright;
	u_short	v[6];
	float	x,y;
	struct _hex_data *next;
} HEX_DATA;

typedef struct {
	float	x,y;
//	int		col;
//	int		pad0;
} SCR_VERTS;


typedef struct {
	DG_DMATAG		dmatag_flush0;		// ＧＩＦ接続ＤＭＡタグ			フラッシュ
	DROW_FLUSH 		drow_flush0;		//								フラッシュ
	DG_DMATAG		dmatag_sys;			// ＧＩＦ接続ＤＭＡタグ			退避
	DG_DMATAG		dmatag_flush1;		// ＧＩＦ接続ＤＭＡタグ			フラッシュ
	DROW_FLUSH 		drow_flush1;		//								フラッシュ
	
	DG_DMATAG		dmatag_mdl;			// ＧＩＦ接続ＤＭＡタグ
	MDL_DRAW		mdl_draw;			//								ロッカク描画の設定
	DG_DMATAG		dmatag_mverts0;		// ＧＩＦ接続ＤＭＡタグ			ロッカク頂点(tex0,alpha入り)
	DG_DMATAG		dmatag_mverts1;		// ＧＩＦ接続ＤＭＡタグ			ロッカク頂点(tex0,alpha入り)
	DG_DMATAG		dmatag_mverts2;		// ＧＩＦ接続ＤＭＡタグ			ロッカク頂点(tex0,alpha入り)
#if 1
	DG_DMATAG		dmatag_line;		// ＧＩＦ接続ＤＭＡタグ
	MDL_DRAW		line_draw;			//								ライン描画の設定
	DG_DMATAG		dmatag_mverts3;		// ＧＩＦ接続ＤＭＡタグ			ライン頂点
#endif
	DG_DMATAG		dmatag_test;		// ＧＩＦ接続ＤＭＡタグ
	PACKET_END		end_paket;			//								ピクセルテスト復元
	DG_DMATAG		dmatag_offset;		// オフセット環境復元用
	DG_DMATAG		dmatag_end;			// RETタグ
} HEXS_PACK;

/*
prim


verts	ずれ
verts	ずれ
verts	カラー


verts	カラー
verts	ずれ
verts	ずれ

こんな感じで
*/

#define		SCRHEX_FLAGS_FADEOUT	(0x0001)
#define		SCRHEX_FLAGS_PREACTEND	(0x0002)
#define		SCRHEX_FLAGS_ACTEND		(0x0004)

typedef	struct	{
	GV_ACT_EX		actor ;

	ALIGN16_PRE HEXS_PACK		*hexs_pack[2] ALIGN16_POST;
	ALIGN16_PRE void			*drow_verts[2] ALIGN16_POST;
	ALIGN16_PRE void			*line_verts[2] ALIGN16_POST;
	ALIGN16_PRE HEX_DATA		hex_data[N_HEXS] ALIGN16_POST;
	SCR_VERTS		scr_verts_tbl[N_CMN_VERTS_W*N_CMN_VERTS_H];
	DG_DMAPACK		*dmapack;

	int				name;
	int				timer;
	int				flags;
	int				sub_time;
	int				col;
	int				end_proc;

#ifndef PSX2
	void				*packet_mem;
	void                *prim ;
	int xbox_packet_size;
#endif

} Work;

static Work *g_work = NULL;

#define		MEM_SCR_VERTS1		((void *)(SCRPAD_ADDR))

static void	SetDMATsgs( DG_DMATAG *dmatag, void *data, int size )
{
#ifdef PSX2
	dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, size/sizeof(u_long128) );
	dmatag->addr = data;
	dmatag->vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	dmatag->vifcode[1] = SCE_VIF1_SET_DIRECT( size/sizeof(u_long128), 0);
#endif
}

static void *SetGifTag( void *data, int size )
{
	DG_GIFTAG	*giftag = (DG_GIFTAG*)data;

#ifdef PSX2
	giftag->tag  = SCE_GIF_SET_TAG( size/sizeof(u_long128), 1, 0, 0, 0, 1);
	giftag->regs = GS_REGS_AD;
	giftag++;
#endif

	return giftag;
}

static void *SetDataHead( void *data, int tex, long64 alpha )
{

#ifdef PSX2
	DG_GSREG	*regs = (DG_GSREG*)data;

	regs->reg = SCE_GS_PRIM;
	regs->data = SCE_GS_SET_PRIM(4,0,tex,0,1,0,1,0,0);
	regs++;

	regs->reg = SCE_GS_ALPHA_1;
	regs->data = alpha;
	regs++;

	return regs;
#else
	return DG_SetDmapackAlpha( data, alpha ) ;
#endif

}

static void *SetHexVertsFadeOut( void *data, HEX_DATA *hex, int n_hexs, SCR_VERTS *verts, int col, int mode )
{
#ifdef PSX2
	MVERT_DATA	*mverts = (MVERT_DATA*)data;
#else
	void	*mverts = data;
#endif
	int			i,j;
	int			r,g,b,a;

	float		offset_x;
	float		offset_y;
	float		size_x;
	float		size_y;
#if 0
	float		debug_uv[6][2] = {
		{ 85.0f, 0.0f },
		{ 0.0f, 224.0f },
		{ 512.0f-85.0f, 0.0f },
		{ 85.0f, 448.0f },
		{ 512.0f, 224.0f },
		{ 512.0f-85.0f, 448.0f },
	};
#endif

	if( mode == 0 ){
		offset_x = 32.0f;
		offset_y = 32.0f;
		size_x = (float)DRAW_WIDTH - 64.0f;
		size_y = (float)DRAW_HEIGHT - 64.0f;
	}else if( mode == 1 ){
		offset_x = 32.0f;
		offset_y = 32.0f;
		size_x = (float)DRAW_WIDTH - 64.0f;
		size_y = (float)DRAW_HEIGHT - 64.0f;
	}else if( mode == 2 ){
		offset_x = 0.0f;
		offset_y = 0.0f;
		size_x = (float)DRAW_WIDTH;
		size_y = (float)DRAW_HEIGHT;
	}else{
		offset_x = 0.0f;
		offset_y = 0.0f;
		size_x = (float)DRAW_WIDTH;
		size_y = (float)DRAW_HEIGHT;
	}

	r = (col>>24)&0xff;
	g = (col>>16)&0xff;
	b = (col>> 8)&0xff;
	a = (col>> 0)&0xff;
	for( i = 0; i < n_hexs; i++ ){

		if( mode == 0 ){
			a = (int)(128.0f - 128.0f*(float)(hex->count-(2048+512))/512.0f);
#if 0
			r = hex->bright*2/3;
			g = hex->bright;
			b = hex->bright/3;
#else
			r = hex->bright;//-0;
			g = hex->bright;//-0;
			b = hex->bright;//-7;
#endif
		}else if( mode == 1 ){
			a = (int)(128.0f*(float)(hex->count-2048)/512.0f);
#if 0
			r = hex->bright*2/3;
			g = hex->bright;
			b = hex->bright/3;
#else
			r = hex->bright;//-0;
			g = hex->bright;//-0;
			b = hex->bright;//-7;
#endif
		}else if( mode == 2 ){
			a = (int)(128.0f - 128.0f*(float)(hex->count)/2048.0f);
		}else{
			a = 0;
		}
//		printf("count = %d\n",hex->count);
#if 0
		r = (i*8)%256;
		g = (i*16)%256;
		b = (i*12)%256;
		a = 128;
#endif
		
		for( j = 0; j < 6; j++ ){
			float		x = verts[hex->v[j]].x;
			float		y = verts[hex->v[j]].y;
#ifdef PSX2
			int			ix,iy,iu,iv;
#else
			float       u,v ;
#endif
			if( x < 0.0f ) x = 0.0f;
			else if( x > (float)DRAW_WIDTH ) x = (float)DRAW_WIDTH;
			if( y < 0.0f ) y = 0.0f;
			else if( y > (float)DRAW_HEIGHT ) y = (float)DRAW_HEIGHT;
#ifdef PSX2
			ix = (int)(2048.0f-(float)(DRAW_WIDTH>>1) + x ) << 4;
			iy = (int)(2048.0f-(float)(DRAW_HEIGHT>>1) + y ) << 4;
#if 0
			iu = (int)(debug_uv[j][0]) << 4;
			iv = (int)(debug_uv[j][1]) << 4;
#else
			iu = (int)(x*(size_x/(float)DRAW_WIDTH)+offset_x) << 4;
			iv = (int)(y*(size_y/(float)DRAW_HEIGHT)+offset_y) << 4;
#endif
			
			if( iu > MAX_U ) iu = MAX_U;
			else if( iu < MIN_UV ) iu = MIN_UV;
			if( iv > MAX_V ) iv = MAX_V;
			else if( iv < MIN_UV ) iv = MIN_UV;
			
			if( ix > MAX_X ) ix = MAX_X;
			else if( ix < MIN_X ) ix = MIN_X;
			if( iy > MAX_Y ) iy = MAX_Y;
			else if( iy < MIN_Y ) iy = MIN_Y;

			mverts->rgbq.reg = SCE_GS_RGBAQ;
			mverts->uv.reg = SCE_GS_UV;
			mverts->xyz.reg = ( j < 2 )?SCE_GS_XYZ3:SCE_GS_XYZ2;
			mverts->rgbq.data = SCE_GS_SET_RGBAQ(r,g,b,a,0);
			mverts->uv.data = SCE_GS_SET_UV(iu,iv);
			mverts->xyz.data = SCE_GS_SET_XYZ(ix,iy,0);
			mverts++;
#else
			x *= 5.0f/4.0f ;
			u = DG_FRAME_U( x*(size_x/(float)DRAW_WIDTH ) + offset_x ) ;
			v = DG_FRAME_V( y*(size_y/(float)DRAW_HEIGHT) + offset_y ) ;
			if ( !j ) {
				mverts = DG_SetDmapackTriangleStrip( mverts, 6 ) ;
			}
			mverts = DG_SetDmapackVertex( mverts, x,y, u,v,
										 DG_MakeDMAPackColor(r,g,b,a) ) ;
#endif
		}
		hex = hex->next;
	}
	return mverts;
}

static void *SetHexVertsFadeIn( void *data, HEX_DATA *hex, int n_hexs, SCR_VERTS *verts, int col, int mode )
{
#ifdef PSX2
	MVERT_DATA	*mverts = (MVERT_DATA*)data;
#else
	void	*mverts = data;
#endif
	int			i,j;
	int			r,g,b,a;

	float		offset_x;
	float		offset_y;
	float		size_x;
	float		size_y;
#if 0
	float		debug_uv[6][2] = {
		{ 85.0f, 0.0f },
		{ 0.0f, 224.0f },
		{ 512.0f-85.0f, 0.0f },
		{ 85.0f, 448.0f },
		{ 512.0f, 224.0f },
		{ 512.0f-85.0f, 448.0f },
	};
#endif

	if( mode == 0 ){
		offset_x = 0.0f;
		offset_y = 0.0f;
		size_x = (float)DRAW_WIDTH;
		size_y = (float)DRAW_HEIGHT;
	}else if( mode == 1 ){
		offset_x = 32.0f;
		offset_y = 32.0f;
		size_x = (float)DRAW_WIDTH - 64.0f;
		size_y = (float)DRAW_HEIGHT - 64.0f;
	}else if( mode == 2 ){
		offset_x = 32.0f;
		offset_y = 32.0f;
		size_x = (float)DRAW_WIDTH - 64.0f;
		size_y = (float)DRAW_HEIGHT - 64.0f;
	}else{
		offset_x = 0.0f;
		offset_y = 0.0f;
		size_x = (float)DRAW_WIDTH;
		size_y = (float)DRAW_HEIGHT;
	}

	r = (col>>24)&0xff;
	g = (col>>16)&0xff;
	b = (col>> 8)&0xff;
	a = (col>> 0)&0xff;
//printf("mode %d\n", mode );
	for( i = 0; i < n_hexs; i++ ){

		if( mode == 0 ){
			// 最初 テクスチャー無し
			a = (int)(128.0f*(float)(hex->count-(2048+512))/512.0f);
			if( a > 128 ) a = 128;
		}else if( mode == 1 ){
			// 次
			a = (int)(128.0f - 128.0f*(float)(hex->count-2048)/512.0f);
#if 0
			r = hex->bright*2/3;
			g = hex->bright;
			b = hex->bright/3;
#else
			r = hex->bright;//-0;
			g = hex->bright;//-0;
			b = hex->bright;//-7;
#endif
		}else if( mode == 2 ){
			// 最後
			a = (int)(128.0f*(float)(hex->count)/2048.0f);
#if 0
			r = hex->bright*2/3;
			g = hex->bright;
			b = hex->bright/3;
#else
			r = hex->bright;//-0;
			g = hex->bright;//-0;
			b = hex->bright;//-7;
#endif
		}else{
			a = 0;
		}
//		printf("count = %d\n",hex->count);
#if 0
		r = (i*8)%256;
		g = (i*16)%256;
		b = (i*12)%256;
		a = 128;
#endif
		
		for( j = 0; j < 6; j++ ){
			float		x = verts[hex->v[j]].x;
			float		y = verts[hex->v[j]].y;
#ifdef PSX2
			int			ix,iy,iu,iv;
#else
			float		u,v ;
#endif

			if( x < 0.0f ) x = 0.0f;
			else if( x > (float)DRAW_WIDTH ) x = (float)DRAW_WIDTH;
			if( y < 0.0f ) y = 0.0f;
			else if( y > (float)DRAW_HEIGHT ) y = (float)DRAW_HEIGHT;
			
#ifdef PSX2
			ix = (int)(2048.0f-(float)(DRAW_WIDTH>>1) + x ) << 4;
			iy = (int)(2048.0f-(float)(DRAW_HEIGHT>>1) + y ) << 4;
			iu = (int)(x*(size_x/(float)DRAW_WIDTH)+offset_x) << 4;
			iv = (int)(y*(size_y/(float)DRAW_HEIGHT)+offset_y) << 4;
			
			if( iu > MAX_U ) iu = MAX_U;
			else if( iu < MIN_UV ) iu = MIN_UV;
			if( iv > MAX_V ) iv = MAX_V;
			else if( iv < MIN_UV ) iv = MIN_UV;
			
			if( ix > MAX_X ) ix = MAX_X;
			else if( ix < MIN_X ) ix = MIN_X;
			if( iy > MAX_Y ) iy = MAX_Y;
			else if( iy < MIN_Y ) iy = MIN_Y;

			mverts->rgbq.reg = SCE_GS_RGBAQ;
			mverts->uv.reg = SCE_GS_UV;
			mverts->xyz.reg = ( j < 2 )?SCE_GS_XYZ3:SCE_GS_XYZ2;
			mverts->rgbq.data = SCE_GS_SET_RGBAQ(r,g,b,a,0);
			mverts->uv.data = SCE_GS_SET_UV(iu,iv);
			mverts->xyz.data = SCE_GS_SET_XYZ(ix,iy,0);
			mverts++;
#else
			x *= 5.0f/4.0f ;
			u = DG_FRAME_U( x*(size_x/(float)DRAW_WIDTH ) + offset_x ) ;
			v = DG_FRAME_V( y*(size_y/(float)DRAW_HEIGHT) + offset_y ) ;
			if ( !j ) {
				mverts = DG_SetDmapackTriangleStrip( mverts, 6 ) ;
			}    
			mverts = DG_SetDmapackVertex( mverts, x,y, u,v,
										  DG_MakeDMAPackColor(r,g,b,a) ) ;
#endif
		}
		hex = hex->next;
	}
	return mverts;
}

#define		FG_COS60	(0.500000000f)
#define		FG_SIN60	(0.866025404f)
#define		FG_COS00	(1.0f)
#define		FG_SIN00	(0.0f)


static void *SetHexVertsLineFadeOut( void *data, HEX_DATA *hex, int n_hexs, SCR_VERTS *verts, int col, int mode )
{
#ifdef PSX2
	MVERT_DATA	*mverts = (MVERT_DATA*)data;
#else
	void	*mverts = data;
#endif
	int			i,j;
	float			r,g,b,a;
	static int index_tbl[7] = { 0, 1, 3, 5, 4, 2, 0 };
	static float norm[7][2] = {
		{ -FG_COS60, -FG_SIN60 },
		{ -FG_COS00,  FG_SIN00 },
		{ -FG_COS60,  FG_SIN60 },
		{  FG_COS60,  FG_SIN60 },
		{  FG_COS00,  FG_SIN00 },
		{  FG_COS60, -FG_SIN60 },
		{ -FG_COS60, -FG_SIN60 },
	};

//	a = (float)((col>> 0)&0xff);
//printf("mode %d\n", mode );
	for( i = 0; i < n_hexs; i++ ){
		float	bright_x = TS_COSs( (hex->count*4/3)%4096 );
		float	bright_y = TS_SINs( (hex->count*4/3)%4096 );
#if 0
		r = (float)((i*8)%256);
		g = (float)((i*16)%256);
		b = (float)((i*12)%256);
#else
		r = (float)((col>>24)&0xff);
		g = (float)((col>>16)&0xff);
		b = (float)((col>> 8)&0xff);
#endif
		if( hex->count > 3072 ){
			a = 0;
		}else if( hex->count > 3072 - 256 ){
			a = 128 - ((hex->count - (3072 - 256))/(256/128) );
		}else if( hex->count > 3072 - 512 ){
			a = 64 + ((hex->count - (3072 - 512))/(256/64) );
		}else if( hex->count > 1024+256 ){
			a = 64;
		}else if( hex->count > 1024 ){
			a = 128 - ((hex->count - (1024))/(256/64) );
		}else{
			a = hex->count / (1024/128);
		}

		for( j = 0; j < 7; j++ ){
			float		x = verts[hex->v[index_tbl[j]]].x;
			float		y = verts[hex->v[index_tbl[j]]].y;
			int			ir,ig,ib;
#ifdef PSX2
			int			ix,iy,iu,iv ;
#endif
			float		inner;

			inner = (bright_x * norm[j][0] + bright_y * norm[j][1])*0.5f;

			ir = (int)( r + r*inner );
			ig = (int)( g + g*inner );
			ib = (int)( b + b*inner );

			if( ir < 0 ) ir = 0;
			else if( ir > 255 ) ir = 255;
			if( ig < 0 ) ig = 0;
			else if( ig > 255 ) ig = 255;
			if( ib < 0 ) ib = 0;
			else if( ib > 255 ) ib = 255;
			
			x = (x - hex->x)*(HEX_LEN-0.5f)/HEX_LEN + hex->x;
			y = (y - hex->y)*(HEX_LEN-0.5f)/HEX_LEN + hex->y;
			if( x < 0.0f ) x = 0.0f;
			else if( x > (float)DRAW_WIDTH ) x = (float)DRAW_WIDTH;
			if( y < 0.0f ) y = 0.0f;
			else if( y > (float)DRAW_HEIGHT ) y = (float)DRAW_HEIGHT;
#ifdef PSX2
			ix = (int)(2048.0f-(float)(DRAW_WIDTH>>1) + x ) << 4;
			iy = (int)(2048.0f-(float)(DRAW_HEIGHT>>1) + y ) << 4;

			iu = (int)(x) << 4;
			iv = (int)(y) << 4;

			if( iu > MAX_U ) iu = MAX_U;
			else if( iu < MIN_UV ) iu = MIN_UV;
			if( iv > MAX_V ) iv = MAX_V;
			else if( iv < MIN_UV ) iv = MIN_UV;
			
			if( ix > MAX_X ) ix = MAX_X;
			else if( ix < MIN_X ) ix = MIN_X;
			if( iy > MAX_Y ) iy = MAX_Y;
			else if( iy < MIN_Y ) iy = MIN_Y;

			mverts->rgbq.reg = SCE_GS_RGBAQ;
			mverts->uv.reg = SCE_GS_UV;
			mverts->xyz.reg = ( j )?SCE_GS_XYZ2:SCE_GS_XYZ3;
			mverts->rgbq.data = SCE_GS_SET_RGBAQ(ir,ig,ib,a,0);
			mverts->uv.data = SCE_GS_SET_UV(iu,iv);
			mverts->xyz.data = SCE_GS_SET_XYZ(ix,iy,0);
			mverts++;
#else
			x *= 5.0f/4.0f ;
			if ( !j ) {
				mverts = DG_SetDmapackLineStrip( mverts, 7 ) ;
			}
			mverts = DG_SetDmapackVertex( mverts,
										  x,y, DG_FRAME_U(x), DG_FRAME_V(y),
										  DG_MakeDMAPackColor(ir,ig,ib,a) ) ;
			
#endif
		}
		hex++;// = hex->next;
	}

	return mverts;
}


static int ActHexData( HEX_DATA *hex, int n_hex, int sub )
{
	int		i,cnt = n_hex;

	for( i = 0; i < n_hex; i++ ){
		hex->count -= sub;
		if( hex->count < 0 ){
			hex->count = 0;
			cnt--;
		}
		hex++;
	}
//printf("cnt %d\n",cnt);
	return cnt;
}


static void SetMVerts_FadeOut( HEXS_PACK *packet, void *pdata, void *pline, HEX_DATA *phex, int n_hexs, SCR_VERTS *verts, int col )
{
	void *data = pdata;
#ifndef PSX2
	Work *work = pline;
#endif
	HEX_DATA *hex = phex;
	HEX_DATA *fst = NULL;
	HEX_DATA *snd = NULL;
	HEX_DATA *thd = NULL;
	int		n_fst = 0;
	int		n_snd = 0;
	int		n_thd = 0;
	int		i;

	for( i = 0; i < n_hexs; i++ ){
		switch( hex->count/512 ){
		  case 0:
		  case 1:
		  case 2:
		  case 3:
			hex->next = thd;
			thd = hex;
			n_thd++;
			break;
		  case 4:
			hex->next = snd;
			snd = hex;
			n_snd++;
			break;
		  case 5:
			hex->next = fst;
			fst = hex;
			n_fst++;
			break;
		  default:
			break;
		}
		hex++;
	}

	//printf( "0 %d: 1 %d: 2 %d\n", n_fst, n_snd, n_thd );
	SetDMATsgs( &packet->dmatag_mverts0, data,
				sizeof(MVERT_DATA)*6*n_fst + sizeof(DG_GIFTAG) + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetGifTag( data, sizeof(MVERT_DATA)*6*n_fst + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetDataHead( data, 1, SCE_GS_SET_ALPHA(0,2,0,1,0x80) );
	data = SetHexVertsFadeOut( data, fst, n_fst, verts, DEF_COLOR, 0 );
	
	SetDMATsgs( &packet->dmatag_mverts1, data,
				sizeof(MVERT_DATA)*6*n_snd + sizeof(DG_GIFTAG) + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetGifTag( data, sizeof(MVERT_DATA)*6*n_snd + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetDataHead( data, 1, SCE_GS_SET_ALPHA(0,2,0,1,0x80) );
	data = SetHexVertsFadeOut( data, snd, n_snd, verts, DEF_COLOR, 1 );

	SetDMATsgs( &packet->dmatag_mverts2, data,
				sizeof(MVERT_DATA)*6*n_thd + sizeof(DG_GIFTAG) + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetGifTag( data, sizeof(MVERT_DATA)*6*n_thd + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetDataHead( data, 0, SCE_GS_SET_ALPHA(0,1,0,1,0x80) );
	data = SetHexVertsFadeOut( data, thd, n_thd, verts, col, 2 );

#ifdef PSX2 /* XBOXでは、パケットを１つにすべてつなげるので、これは要らない。*/
	data = pline;
#endif
	SetDMATsgs( &packet->dmatag_mverts3, data, sizeof(DG_GIFTAG) + sizeof(MVERT_DATA)*7*n_hexs );
	data = SetGifTag( data, sizeof(MVERT_DATA)*7*n_hexs );
	data = SetHexVertsLineFadeOut( data, phex, n_hexs, verts, DEF_COLOR, 0 );

#ifndef PSX2
	work->prim = data ;
#endif
}


static void SetMVerts_FadeIn( HEXS_PACK *packet, void *pdata, void *pline, HEX_DATA *phex, int n_hexs, SCR_VERTS *verts, int col )
{
	void *data = pdata;
#ifndef PSX2
	Work *work = pline;
#endif
	HEX_DATA *hex = phex;
	HEX_DATA *fst = NULL;
	HEX_DATA *snd = NULL;
	HEX_DATA *thd = NULL;
	int		n_fst = 0;
	int		n_snd = 0;
	int		n_thd = 0;
	int		i;

	for( i = 0; i < n_hexs; i++ ){
		switch( hex->count/512 ){
		  case 0:
		  case 1:
		  case 2:
		  case 3:
			hex->next = thd;
			thd = hex;
			n_thd++;
			break;
		  case 4:
			//ずれ
			hex->next = snd;
			snd = hex;
			n_snd++;
			break;
		  case 5:
		  case 6:
		  case 7:
		  case 8:
			//テクスチャー無し
			hex->next = fst;
			fst = hex;
			n_fst++;
			break;
		  default:
			break;
		}
		hex++;
	}

	//printf( "0 %d: 1 %d: 2 %d\n", n_fst, n_snd, n_thd );
	SetDMATsgs( &packet->dmatag_mverts0, data,
				sizeof(MVERT_DATA)*6*n_fst + sizeof(DG_GIFTAG) + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetGifTag( data, sizeof(MVERT_DATA)*6*n_fst + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetDataHead( data, 0, SCE_GS_SET_ALPHA(0,1,0,1,0x80) );
	data = SetHexVertsFadeIn( data, fst, n_fst, verts, col, 0 );

	SetDMATsgs( &packet->dmatag_mverts1, data,
				sizeof(MVERT_DATA)*6*n_snd + sizeof(DG_GIFTAG) + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetGifTag( data, sizeof(MVERT_DATA)*6*n_snd + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetDataHead( data, 1, SCE_GS_SET_ALPHA(0,2,0,1,0x80) );
	data = SetHexVertsFadeIn( data, snd, n_snd, verts, DEF_COLOR, 1 );

	SetDMATsgs( &packet->dmatag_mverts2, data,
				sizeof(MVERT_DATA)*6*n_thd + sizeof(DG_GIFTAG) + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetGifTag( data, sizeof(MVERT_DATA)*6*n_thd + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetDataHead( data, 1, SCE_GS_SET_ALPHA(0,2,0,1,0x80) );
	data = SetHexVertsFadeIn( data, thd, n_thd, verts, DEF_COLOR, 2 );

#ifdef PSX2 /* XBOXでは、パケットを１つにすべてつなげるので、これは要らない。*/
	data = pline;
#endif
	SetDMATsgs( &packet->dmatag_mverts3, data, sizeof(DG_GIFTAG) + sizeof(MVERT_DATA)*7*n_hexs );
	data = SetGifTag( data, sizeof(MVERT_DATA)*7*n_hexs );
	data = SetHexVertsLineFadeOut( data, phex, n_hexs, verts, DEF_COLOR, 0 );

#ifndef PSX2
	work->prim = data ;
#endif
}

static void Act( Work *work )
{
	int	ret = 1;
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case 0:
			GV_DestroyActor( work );
			break;
		  default:
			break;
		}
		msg--;
	}

	if( !(work->flags & SCRHEX_FLAGS_ACTEND) ){//&& !(GV_PauseLevel & GV_PAUSE_PAUSE) ){
		if( !(work->flags & SCRHEX_FLAGS_PREACTEND) ){
			ret = ActHexData( work->hex_data, N_HEXS, work->sub_time );
			if( !ret ) work->flags |= SCRHEX_FLAGS_PREACTEND;
		}else{
			work->flags |= SCRHEX_FLAGS_ACTEND;
			if( work->end_proc){
				GCL_ExecProc( work->end_proc, NULL );
			}
			if( !(work->flags & SCRHEX_FLAGS_FADEOUT) ){
				//死亡
				GV_DestroyActor( work );
			}
		}
		
	}


#ifdef PSX2
	if( work->flags & SCRHEX_FLAGS_FADEOUT ){
		SetMVerts_FadeOut( work->hexs_pack[DG_Clock], work->drow_verts[DG_Clock], work->line_verts[DG_Clock],
						   work->hex_data, N_HEXS, work->scr_verts_tbl, work->col );
	}else{
		SetMVerts_FadeIn( work->hexs_pack[DG_Clock], work->drow_verts[DG_Clock], work->line_verts[DG_Clock],
						  work->hex_data, N_HEXS, work->scr_verts_tbl, work->col );
	}

#else

	work->prim = work->dmapack->autopacket ;
	work->prim = DG_SetDmapackUseFrameTex( work->prim, 1 ) ;
	/* フレームパッファのアルファを無視する */
	work->prim = DG_SetDmapackModeEnable( work->prim, DG_DMAPACK_MODE_NO_TEXALPHA );

	if( work->flags & SCRHEX_FLAGS_FADEOUT ){
		SetMVerts_FadeOut( work->hexs_pack[DG_Clock], work->prim, work,
						   work->hex_data, N_HEXS, work->scr_verts_tbl, work->col );
	}else{
		SetMVerts_FadeIn( work->hexs_pack[DG_Clock], work->prim, work,
						  work->hex_data, N_HEXS, work->scr_verts_tbl, work->col );
	}

	/* フレームパッファのアルファを無視する設定を終了 */
	work->prim = DG_SetDmapackModeDisable( work->prim, DG_DMAPACK_MODE_NO_TEXALPHA );

	work->prim = DG_SetDmapackEnd( work->prim ) ;

	//printf( "scr_hex.c need Size%d\n",  (int)work->prim-(int)work->packet_mem  ) ;
	if( work->xbox_packet_size < ((int)work->prim-(int)work->packet_mem) ){
		printf("xbox_packet_size not enough!!\n");
		ASSERT( 0 );
	}
#endif
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	if(work->dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}

#ifdef PSX2	
#else
	if(work->packet_mem) GV_DelayedFree( work->packet_mem );
#endif
	
	if(work->hexs_pack[0]) GV_DelayedFree(work->hexs_pack[0]);
	if(work->drow_verts[0]) GV_DelayedFree(work->drow_verts[0]);
	if(work->line_verts[0]) GV_DelayedFree(work->line_verts[0]);
	g_work = NULL;
}
/* ---------------------------------------------------------------- */



#ifdef PSX2  /*!!!!!!!!!!!!!!!!!!!!!!! PSX2でしか使用しない関数!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
static void InitDmaTags( Work *work, HEXS_PACK *packet, int which, int verts_size )
{
	// --------  DMATag  ----------------------------
	// ＤＭＡタグ		
	packet->dmatag_flush0.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DROW_FLUSH) );
	packet->dmatag_flush0.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_flush0.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DROW_FLUSH), 0);
	
	packet->dmatag_flush1.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DROW_FLUSH) );
	packet->dmatag_flush1.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_flush1.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DROW_FLUSH), 0);
	// ＤＭＡタグ		モデル描画設定
	packet->dmatag_mdl.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(MDL_DRAW) );
	packet->dmatag_mdl.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_mdl.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(MDL_DRAW), 0);

	// ＤＭＡタグ		モデル描画設定
	packet->dmatag_line.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(MDL_DRAW) );
	packet->dmatag_line.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_line.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(MDL_DRAW), 0);

	// ＤＭＡタグ		ピクセルテスト復元
	packet->dmatag_test.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(PACKET_END) );
	packet->dmatag_test.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_test.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(PACKET_END), 0);
	// オフセット環境復元用
	packet->dmatag_offset.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWOFFSET) );
	packet->dmatag_offset.addr = &( DG_Chanl( 0 )->draw_offset[ which ] );
	packet->dmatag_offset.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_offset.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_DRAWOFFSET), 0);
	// RETタグ
	packet->dmatag_end.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 );
	packet->dmatag_end.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_end.vifcode[1] = SCE_VIF1_SET_NOP( 0 );

	// ＤＭＡタグ		モデル頂点0
	packet->dmatag_mverts0.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 0 );
	packet->dmatag_mverts0.addr = work->drow_verts[which];
	packet->dmatag_mverts0.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_mverts0.vifcode[1] = SCE_VIF1_SET_DIRECT( 0, 0);
	
	// ＤＭＡタグ		モデル頂点1
	packet->dmatag_mverts1.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 0 );
	packet->dmatag_mverts1.addr = work->drow_verts[which];
	packet->dmatag_mverts1.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_mverts1.vifcode[1] = SCE_VIF1_SET_DIRECT( 0, 0);
	
	// ＤＭＡタグ		モデル頂点2
	packet->dmatag_mverts2.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 0 );
	packet->dmatag_mverts2.addr = work->drow_verts[which];
	packet->dmatag_mverts2.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_mverts2.vifcode[1] = SCE_VIF1_SET_DIRECT( 0, 0);
	
	// ＤＭＡタグ		モデル頂点2
	packet->dmatag_mverts3.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 0 );
	packet->dmatag_mverts3.addr = work->drow_verts[which];
	packet->dmatag_mverts3.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_mverts3.vifcode[1] = SCE_VIF1_SET_DIRECT( 0, 0);
}

static void InitMVerts( HEXS_PACK *packet, void *pdata, HEX_DATA *hex, SCR_VERTS *verts )
{
	void *data = pdata;
#if 0
	SetDMATsgs( &packet->dmatag_mverts0, data,
				sizeof(MVERT_DATA)*N_HEXS*6 + sizeof(DG_GIFTAG) + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetGifTag( data, sizeof(MVERT_DATA)*N_HEXS*6 + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetDataHead( data, 1, SCE_GS_SET_ALPHA(0,1,0,1,0x80) );
	data = SetHexVertsFadeOut( data, hex, N_HEXS, verts, 0x80808000, 0 );
#else
	SetDMATsgs( &packet->dmatag_mverts0, data, sizeof(DG_GIFTAG) + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetGifTag( data, sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetDataHead( data, 1, SCE_GS_SET_ALPHA(0,1,0,1,0x80) );
	data = SetHexVertsFadeOut( data, hex, 0, verts, 0x80808000, 0 );
#endif
	SetDMATsgs( &packet->dmatag_mverts1, data, sizeof(DG_GIFTAG) + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetGifTag( data, sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetDataHead( data, 1, SCE_GS_SET_ALPHA(0,1,0,1,0x80) );
	data = SetHexVertsFadeOut( data, hex, 0, verts, 0x80808000, 0 );
	
	SetDMATsgs( &packet->dmatag_mverts2, data, sizeof(DG_GIFTAG) + sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetGifTag( data, sizeof(DG_GSREG) + sizeof(DG_GSREG) );
	data = SetDataHead( data, 1, SCE_GS_SET_ALPHA(0,1,0,1,0x80) );
	data = SetHexVertsFadeOut( data, hex, 0, verts, 0x80808000, 0 );
	
}

static void InitLineVerts( HEXS_PACK *packet, void *pdata )
{
	void *data = pdata;

	SetDMATsgs( &packet->dmatag_mverts3, data, sizeof(DG_GIFTAG) );
	data = SetGifTag( data, 0 );
	data = SetHexVertsLineFadeOut( data, NULL, 0, NULL, 0x80808000, 0 );
}
#endif  /*!!!!!!!!!!!!!!!!!!!!!!! PSX2でしか使用しない関数!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/



static void InitScrVerts( SCR_VERTS *out )
{
	SCR_VERTS *data = out;
	int		i,j;
	float		temp_wx[2][N_CMN_VERTS_W];
	float		temp_hy[N_CMN_VERTS_H+1];
#if 0
	static FVECTOR debug_hex_verts[N_CMN_VERTS_W*N_CMN_VERTS_H];
#endif
	temp_wx[0][0] = INIT_X;
	temp_wx[1][0] = INIT_X - HEX_LEN/2.0f;
	//printf("x0 %f: x1 %f\n", temp_wx[0][0], temp_wx[1][0] );
	
	for( i = 1; i < N_CMN_VERTS_W; i++ ){
		temp_wx[0][i] = temp_wx[0][i-1] + ( (i&1)?HEX_LEN:HEX_LEN*2.0f );
		temp_wx[1][i] = temp_wx[1][i-1] + ( (i&1)?HEX_LEN*2.0f:HEX_LEN );

		//printf("x0 %f: x1 %f\n", temp_wx[0][i], temp_wx[1][i] );
	}
	//printf("\n");

	temp_hy[0] = INIT_Y - HEX_LEN*SCR_ROOT3/2.0f;
	
	for( i = 1; i < N_CMN_VERTS_H+1; i++ ){
		temp_hy[i] = temp_hy[i-1] + HEX_LEN*SCR_ROOT3/2.0f;
	}
	
	for( i = 0; i < N_CMN_VERTS_H+1; i++ ){
		temp_hy[i] *= (float)DRAW_HEIGHT/400.0f;
	}
	
	
	for( i = 0; i < N_CMN_VERTS_W; i++ ){
		for( j = 0; j < N_CMN_VERTS_H; j++ ){
			data->x = temp_wx[j&1][i];
//			data->y = temp_hy[(j+1) - ((i&2)>>1)];
			data->y = temp_hy[(j+1)];
#if 0
			debug_hex_verts[i*N_CMN_VERTS_H+j].vx = data->x;
			debug_hex_verts[i*N_CMN_VERTS_H+j].vy = data->y;
			debug_hex_verts[i*N_CMN_VERTS_H+j].vz = 0.0f;
			debug_hex_verts[i*N_CMN_VERTS_H+j].vw = 1.0f;

			NewDbugSprite( &debug_hex_verts[i*N_CMN_VERTS_H+j], 10.0f );
#endif
			data++;
		}
	}

	//printf("w %d:h %d:a %d:vh %d\n",N_HEXS_W,N_HEXS_H,N_HEXS, N_CMN_VERTS_H );
}


static void InitHexData( HEX_DATA *hex, int n_hex_w, int n_hex_h, SCR_VERTS *verts, int flags )
{
	int		i,j,k;
	float		count_max = (float)(n_hex_w * n_hex_h);
	float		x,y;

	//ここで音も鳴らしてしまう。
	//SD_S_HEX_IN01
	//SD_S_HEXOUT01
	if( flags & SCRHEX_FLAGS_FADEOUT ){
		GM_SeSetMode( SD_S_HEXOUT01, (FVECTOR*)DG_Chanls[0].eye.m[3], GM_SEMODE_BOMB );
	}else{
		GM_SeSetMode( SD_S_HEX_IN01, (FVECTOR*)DG_Chanls[0].eye.m[3], GM_SEMODE_BOMB );
	}
	for( i = 0; i < n_hex_w; i++ ){
		for( j = 0; j < n_hex_h - (i&1); j++ ){
			//0,1,2, N_CMN_VERTS_H,N_CMN_VERTS_H+1,N_CMN_VERTS_H+2
			//1,2,3, 
			//i&1 == 0
			if( irnd()&0x001000 )irnd();
			
			
			//hex->count = 3072 + irnd()%(128) + (int)((512.0f) * (float)((i+1)*(j+1))/count_max);
			hex->count = 3072 + irnd()%(1024);// - 128);
			if( 0 && flags & SCRHEX_FLAGS_FADEOUT ){
				hex->bright = (128) + irnd()%(128);
			}else{
				hex->bright = ((128) + irnd()%(128))*4/10;
			}
			hex->v[0] = 0					+(j*2)+(i&1)+(i*N_CMN_VERTS_H);
			hex->v[1] = 1					+(j*2)+(i&1)+(i*N_CMN_VERTS_H);
			hex->v[2] = (N_CMN_VERTS_H)		+(j*2)+(i&1)+(i*N_CMN_VERTS_H);
			hex->v[3] = 2					+(j*2)+(i&1)+(i*N_CMN_VERTS_H);
			hex->v[4] = (N_CMN_VERTS_H+1)	+(j*2)+(i&1)+(i*N_CMN_VERTS_H);
			hex->v[5] = (N_CMN_VERTS_H+2)	+(j*2)+(i&1)+(i*N_CMN_VERTS_H);
			if( !(i == 0 && j == 0) )
				(hex-1)->next = hex;
			hex->next = NULL;

			x = y = 0.0f;
			for( k = 0; k < 6; k++ ){
				x += verts[hex->v[k]].x;
				y += verts[hex->v[k]].y;
			}
			hex->x = x/6.0f;
			hex->y = y/6.0f;
			hex++;
		}
	}
}

static int GetOption( Work *work )
{
	int		time;
	int		rgba;
	int		mode;
	int		end_proc;


	time = DIRECT_TICK(GCL_GetOptionValue( 't', 100 ));
	mode = GCL_GetOptionValue( 'm', 0 );
	end_proc = GCL_GetOptionValue( 'p', 0 );

	if(GCL_GetOption('c')){
		rgba  = (GCL_GetNextInt()&0xff)<<24;
		rgba |= (GCL_GetNextInt()&0xff)<<16;
		rgba |= (GCL_GetNextInt()&0xff)<< 8;
	}else{
		rgba = 0x80808000;
	}

	work->sub_time = (int)((3072.0f+(1024.0f-128.0f)/2)/(float)time);
	if( mode ) work->flags = SCRHEX_FLAGS_FADEOUT;
	else work->flags = 0;
	work->col = rgba;
	work->end_proc = end_proc;

	//printf("flags %x: sub_count %d\n",work->flags,work->sub_time);
	return end_proc;
}

static int GetResources( Work *work )
{
	DG_DMAPACK		*dmapack;
	u_char			*verts,*lines;
	int				n_verts,verts_size,lines_size;
#ifdef PSX2
	int             i ;
	HEXS_PACK		*packet;
#endif


	GetOption( work );
	n_verts = N_VERTS;
	verts_size = sizeof(DG_GIFTAG)*3 + sizeof(DG_GSREG)*3 + sizeof(DG_GSREG)*3 + sizeof(MVERT_DATA)*n_verts;
	lines_size = sizeof(DG_GIFTAG) + sizeof(MVERT_DATA)*N_HEXS*7;
	
	// 頂点データメモり確保
	verts = GV_Malloc( verts_size * 2 );
	if(verts == NULL){ printf("ERR!! MALLOC MVERTS!!\n"); return -1; }
	work->drow_verts[0] = verts;
	work->drow_verts[1] = verts + verts_size;

	// 頂点データメモり確保
	lines = GV_Malloc( lines_size * 2 );
	if(lines == NULL){ printf("ERR!! MALLOC MVERTS!!\n"); return -1; }
	work->line_verts[0] = lines;
	work->line_verts[1] = lines + lines_size;

	// ＤＭＡパケット型オブジェクト作成
	if( GCL_GetOption('e') == NULL ){
	work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU|DG_DMAPACK_INVISIBLE1|DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3,
											  DG_DMAPACK_PHASE_NORMAL, 95 );
	}else{
	work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU|DG_DMAPACK_INVISIBLE1|DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3,
											  DG_PLUGIN_PHASE_AFTER, 97 );
	}
	if(dmapack == NULL){ printf("ERR!! MAKE DMAPACK!!\n"); return -1; }
	//dmapack->flag |= DG_DMAPACK_INVISIBLE0;
	DG_QueueDmapack( dmapack );


	InitScrVerts( work->scr_verts_tbl );
	InitHexData( work->hex_data, N_HEXS_W, N_HEXS_H, work->scr_verts_tbl, work->flags );

#ifdef PSX2
	// パケットメモリ割り当て
	packet = GV_Malloc( sizeof(HEXS_PACK) * 2 );
	if(packet == NULL){ printf("ERR!! MALLOC HEXS_PACK!!\n"); return -1; }
	work->hexs_pack[0] = dmapack->packet[0] = &packet[0];
	work->hexs_pack[1] = dmapack->packet[1] = &packet[1];

	for ( i = 0 ; i < 2 ; i++ ){
		InitDmaTags( work, &packet[i], i, verts_size );
		InitDrowFlush( &packet[i].drow_flush0 );
		DG_WritePacket_SoftImageTurnOut( &packet[i].dmatag_sys, i );
		InitDrowFlush( &packet[i].drow_flush1 );
		InitMdlDraw( &packet[i].mdl_draw, MDL_TEST, MDL_ALPHA, MDL_PRIM );
		InitMdlDraw( &packet[i].line_draw, LINE_TEST, LINE_ALPHA, LINE_PRIM );
		InitMVerts( &packet[i], work->drow_verts[i], work->hex_data, work->scr_verts_tbl );
		InitLineVerts( &packet[i], work->line_verts[i] );
		InitEndPacket( &packet[i].end_paket );

		packet[i].end_paket.data.test.data = SCE_GS_SET_TEST( 0, 7, 0, 0, 0, 0, 1, 1 );

#if 1
		if( 1 || !(work->flags & SCRHEX_FLAGS_FADEOUT) ){
		//ブラーモード
			packet[i].mdl_draw.data.tex0.data = SCE_GS_SET_TEX0( BUFFER_PAGE(1-i)/64,
																 BUFFER_WIDTH/64,
																 FRAME_BUFFER_COLOR_MODE(),
																 10,10,0,0,0,0,0,0,0);
		
		//ブラーモード
			packet[i].line_draw.data.tex0.data = SCE_GS_SET_TEX0( BUFFER_PAGE(1-i)/64,
																  BUFFER_WIDTH/64,
																  FRAME_BUFFER_COLOR_MODE(),
																  10,10,0,0,0,0,0,0,0);
		}
#endif
	}

#else /* PSX2 */
	
	work->xbox_packet_size = 55000;
	if ( !(work->packet_mem = GV_Malloc( work->xbox_packet_size )) ) {
		printf("ERR!! MALLOC scr_hex.c!!\n");
		return -1 ;
	}
	dmapack->autopacket = work->packet_mem ;
	DG_SetDmapackEnd( dmapack->autopacket ) ;

#endif /* PSX2 */

	return (0);
}

void *NewScrHexFade( int name, int map )
{
	Work		*work ;

	//return (void*)1;
	if(g_work){
		g_work->name = name;
		GetOption( g_work );
		InitHexData( g_work->hex_data, N_HEXS_W, N_HEXS_H, g_work->scr_verts_tbl, g_work->flags );
#if 0
		if( g_work->flags & SCRHEX_FLAGS_FADEOUT ){
			//ブラーモード
			g_work->hexs_pack[0]->mdl_draw.data.tex0.data = SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE()/64,
																			BUFFER_WIDTH/64,
																			FRAME_BUFFER_COLOR_MODE(),
																			10,10,0,0,0,0,0,0,0);
			g_work->hexs_pack[1]->mdl_draw.data.tex0.data = SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE()/64,
																			BUFFER_WIDTH/64,
																			FRAME_BUFFER_COLOR_MODE(),
																			10,10,0,0,0,0,0,0,0);
		
			//ブラーモード
			g_work->hexs_pack[0]->line_draw.data.tex0.data = SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE()/64,
																			 BUFFER_WIDTH/64,
																			 FRAME_BUFFER_COLOR_MODE(),
																			 10,10,0,0,0,0,0,0,0);
			g_work->hexs_pack[1]->line_draw.data.tex0.data = SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE()/64,
																			 BUFFER_WIDTH/64,
																			 FRAME_BUFFER_COLOR_MODE(),
																			 10,10,0,0,0,0,0,0,0);
		}else{
			//ブラーモード
			g_work->hexs_pack[0]->mdl_draw.data.tex0.data = SCE_GS_SET_TEX0( BUFFER_PAGE(1)/64,
																			BUFFER_WIDTH/64,
																			FRAME_BUFFER_COLOR_MODE(),
																			10,10,0,0,0,0,0,0,0);
			g_work->hexs_pack[1]->mdl_draw.data.tex0.data = SCE_GS_SET_TEX0( BUFFER_PAGE(0)/64,
																			BUFFER_WIDTH/64,
																			FRAME_BUFFER_COLOR_MODE(),
																			10,10,0,0,0,0,0,0,0);
		
			//ブラーモード
			g_work->hexs_pack[0]->line_draw.data.tex0.data = SCE_GS_SET_TEX0( BUFFER_PAGE(1)/64,
																			 BUFFER_WIDTH/64,
																			 FRAME_BUFFER_COLOR_MODE(),
																			 10,10,0,0,0,0,0,0,0);
			g_work->hexs_pack[1]->line_draw.data.tex0.data = SCE_GS_SET_TEX0( BUFFER_PAGE(0)/64,
																			  BUFFER_WIDTH/64,
																			  FRAME_BUFFER_COLOR_MODE(),
																			  10,10,0,0,0,0,0,0,0);
		}
#endif
		return g_work;
	}

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	//work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->name = name;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
		g_work = work;
	}

	return (void *)work ;
}

#ifdef REDEFINEPSX2
#define PSX2
#undef REDEFINEPSX2
#endif

